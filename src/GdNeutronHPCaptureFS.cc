#include "GdNeutronHPCaptureFS.hh"

// Geant4 includes
#include "G4SystemOfUnits.hh"
#include "G4PhysicalConstants.hh"
#include "G4ParticleHPManager.hh"
#include "G4ReactionProduct.hh"
#include "G4Nucleus.hh"
#include "G4IonTable.hh"
#include "G4Gamma.hh"
#include "G4Electron.hh"
#include "G4Neutron.hh"
#include "G4ParticleHPDataUsed.hh"
#include "G4ParticleHPNames.hh"
#include "G4PhysicsModelCatalog.hh"

// ANNRI-Gd includes
#include "ANNRIGd_GdNCaptureGammaGenerator.hh"
#include "ANNRIGd_OutputConverter.hh"

GdNeutronHPCaptureFS::GdNeutronHPCaptureFS()
{
    secID = G4PhysicsModelCatalog::GetModelID("model_NeutronHPCapture_ANNRI_FS");
}

// Init 함수는 G4NeutronHPCapture 클래스가 내부적으로 호출하므로 그대로 둡니다.
void GdNeutronHPCaptureFS::Init(G4double A, G4double Z, G4int M, const G4String& dirName, const G4String&, G4ParticleDefinition*)
{
    G4String tString = "/FS";
    G4bool dbool; // isObsolete
    G4ParticleHPNames theNames;
    G4ParticleHPDataUsed aFile = theNames.GetName(static_cast<G4int>(A), static_cast<G4int>(Z), M, dirName, tString, dbool);
    
    // --- [최종 수정] aFile.isNotFound 대신, 파일 이름이 비어있는지 직접 확인합니다. ---
    if (aFile.GetName().empty()) { 
        // 파일이 없으면 isNotFound 플래그를 수동으로 설정해줄 수 있습니다 (선택사항).
        // 이 예제에서는 G4ParticleHPDataUsed에 해당 멤버가 없으므로 그냥 반환합니다.
        return; 
    }
    // ---------------------------------------------------------------------------------

    // Init 함수는 표준 HP 물리(비-Gd 물질)에서 사용될 수 있으므로, 나머지 부분은 남겨둡니다.
    // 하지만 우리 커스텀 로직은 이 데이터를 직접 사용하지 않으므로, 아래 코드는 사실상 실행되지 않습니다.
    std::istringstream theData(std::ios::in);
    G4ParticleHPManager::GetInstance()->GetDataStream(aFile.GetName(), theData);
    hasFSData = theFinalStatePhotons.InitMean(theData);
    if(hasFSData) {
        targetMass = theFinalStatePhotons.GetTargetMass();
        theFinalStatePhotons.InitAngular(theData);
        theFinalStatePhotons.InitEnergies(theData);
    }
}

G4HadFinalState* GdNeutronHPCaptureFS::ApplyYourself(const G4HadProjectile& theTrack)
{
    if (theResult.Get() == nullptr) theResult.Put(new G4HadFinalState);
    theResult.Get()->Clear();

    if (!fAnnriGammaGen) {
        G4Exception("GdNeutronHPCaptureFS::ApplyYourself()", "FatalError", FatalException, "ANNRI-Gd Generator pointer is null!");
        return theResult.Get();
    }

    G4int targA = G4ParticleHPManager::GetInstance()->GetReactionWhiteBoard()->GetTargA();
    G4int targZ = G4ParticleHPManager::GetInstance()->GetReactionWhiteBoard()->GetTargZ();

    // 1. 초기 상태 계산: 타겟 핵이 정지해 있다고 가정
    G4double targetMass = G4IonTable::GetIonTable()->GetIon(targZ, targA, 0.0)->GetPDGMass();
    G4LorentzVector pInitial = theTrack.Get4Momentum() + G4LorentzVector(0,0,0, targetMass);

    // 2. ANNRI-Gd 모델로부터 감마선 목록 생성
    ANNRIGdGammaSpecModel::ReactionProductVector products;
    if (fCaptureMode == 1) { // Natural Gd
        if (targA == 155) {
            // [수정] Generate_155Gd -> Generate_156Gd
            products = (fCascadeMode == 2) ? fAnnriGammaGen->Generate_156Gd_Discrete() :
                       (fCascadeMode == 3) ? fAnnriGammaGen->Generate_156Gd_Continuum() :
                                             fAnnriGammaGen->Generate_156Gd();
        } else if (targA == 157) {
            products = (fCascadeMode == 2) ? fAnnriGammaGen->Generate_158Gd_Discrete() :
                       (fCascadeMode == 3) ? fAnnriGammaGen->Generate_158Gd_Continuum() :
                                             fAnnriGammaGen->Generate_158Gd();
        }
    } else if (fCaptureMode == 2) { // Enriched 157Gd
        products = (fCascadeMode == 2) ? fAnnriGammaGen->Generate_158Gd_Discrete() :
                   (fCascadeMode == 3) ? fAnnriGammaGen->Generate_158Gd_Continuum() :
                                         fAnnriGammaGen->Generate_158Gd();
    } else if (fCaptureMode == 3) { // Enriched 155Gd
        // [수정] Generate_155Gd -> Generate_156Gd
        products = (fCascadeMode == 2) ? fAnnriGammaGen->Generate_156Gd_Discrete() :
                   (fCascadeMode == 3) ? fAnnriGammaGen->Generate_156Gd_Continuum() :
                                         fAnnriGammaGen->Generate_156Gd();
    }

    // 3. 에너지 스케일링 팩터 계산
    G4double recoilMass = G4IonTable::GetIonTable()->GetIon(targZ, targA + 1, 0.0)->GetPDGMass();
    G4double availableEnergyForGammas = pInitial.m() - recoilMass;
    
    G4double totalGammaEnergyFromAnnri = 0.0;
    for (const auto& prod : products) {
        totalGammaEnergyFromAnnri += prod.eTot_;
    }

    G4double scale = 1.0;
    if (totalGammaEnergyFromAnnri > 0) {
        scale = availableEnergyForGammas / totalGammaEnergyFromAnnri;
    }

    // 4. 스케일링된 2차 입자 추가
    G4LorentzVector pFinalProducts(0,0,0,0);
    for (const auto& prod : products) {
        G4LorentzVector pGamma(prod.px_, prod.py_, prod.pz_, prod.eTot_);
        pGamma *= scale;

        // [수정] 삼항 연산자를 if/else 구문으로 변경
        G4ParticleDefinition* particleDef = nullptr;
        if (prod.pdgID_ == 11) { // electron
            particleDef = G4Electron::Definition();
        } else { // gamma
            particleDef = G4Gamma::Definition();
        }
        
        auto theOne = new G4DynamicParticle(particleDef, pGamma);
        theResult.Get()->AddSecondary(theOne);
        pFinalProducts += theOne->Get4Momentum();
    }

    // 5. 최종 반동핵 계산
    G4LorentzVector pRecoil = pInitial - pFinalProducts;
    G4ParticleDefinition* recoil_def = G4IonTable::GetIonTable()->GetIon(targZ, targA + 1, 0.0);
    if (recoil_def && pRecoil.e() >= recoil_def->GetPDGMass()) {
        auto recoil_particle = new G4DynamicParticle(recoil_def, pRecoil);
        theResult.Get()->AddSecondary(recoil_particle);
    }

    theResult.Get()->SetStatusChange(stopAndKill);
    return theResult.Get();
}
