#include "GdNeutronHPCaptureFS.hh"
#include "GdNeutronHPCapture.hh" // 싱글턴에 접근하기 위해 추가

// Geant4 includes
#include "G4SystemOfUnits.hh"
#include "G4PhysicalConstants.hh"
#include "G4ParticleHPManager.hh"
#include "G4ReactionProduct.hh"
#include "G4Nucleus.hh"
#include "G4IonTable.hh"
#include "G4Gamma.hh"
#include "G4Electron.hh"
#include "G4RandomDirection.hh"
#include "G4ParticleHPDataUsed.hh"
#include "G4ParticleHPNames.hh"

// ANNRI-Gd includes (RAT 경로 제거)
#include "ANNRIGd_GdNCaptureGammaGenerator.hh"
#include "ANNRIGd_OutputConverter.hh"

GdNeutronHPCaptureFS::GdNeutronHPCaptureFS() 
{
    // secID는 G4VSecondaryGenerator에서 상속받은 멤버 변수입니다.
    // 모델을 식별하는 고유 ID를 할당할 수 있습니다.
    secID = G4PhysicsModelCatalog::GetModelID("model_NeutronHPCapture_ANNRI_FS");
    hasXsec = false;
}

G4HadFinalState* GdNeutronHPCaptureFS::ApplyYourself(const G4HadProjectile& theTrack) 
{
    if (theResult.Get() == nullptr) theResult.Put(new G4HadFinalState);
    theResult.Get()->Clear();

    // --- 1. GdNeutronHPCapture 싱글턴 인스턴스에서 ANNRI-Gd 생성기와 설정을 가져옴 ---
    auto manager = GdNeutronHPCapture::GetInstance();
    auto annriGenerator = manager->GetANNRIGdGenerator();

    // Geant4의 "화이트보드"에서 현재 반응하는 타겟 동위원소 정보를 가져옵니다.
    G4int targA = G4ParticleHPManager::GetInstance()->GetReactionWhiteBoard()->GetTargA();
    G4int targZ = G4ParticleHPManager::GetInstance()->GetReactionWhiteBoard()->GetTargZ();
    const G4Isotope* target_isotope = G4IonTable::GetIonTable()->GetIsotope(targZ, targA, 0.0);

    // 동위원소 정보와 사용자 설정을 바탕으로 어떤 모드를 사용할지 결정합니다.
    G4int captureMode = manager->GetCaptureModeForIsotope(target_isotope);
    G4int cascadeMode = manager->GetCascadeMode();
    
    // --- 2. ANNRI-Gd 생성기를 사용하여 감마선 생성 ---
    ANNRIGdGammaSpecModel::ReactionProductVector products;

    if (captureMode == 1) { // Natural Gd
        products = annriGenerator->Generate_NatGd();
    } else if (captureMode == 2) { // 157Gd -> 158Gd*
        if(cascadeMode == 1)      products = annriGenerator->Generate_158Gd();
        else if(cascadeMode == 2) products = annriGenerator->Generate_158Gd_Discrete();
        else                      products = annriGenerator->Generate_158Gd_Continuum();
    } else if (captureMode == 3) { // 155Gd -> 156Gd*
        if(cascadeMode == 1)      products = annriGenerator->Generate_156Gd();
        else if(cascadeMode == 2) products = annriGenerator->Generate_156Gd_Discrete();
        else                      products = annriGenerator->Generate_156Gd_Continuum();
    }
    
    auto theProducts = ANNRIGdGammaSpecModel::ANNRIGd_OutputConverter::ConvertToG4(products);

    // --- 3. 생성된 입자들을 Geant4의 최종 상태(Final State)에 추가 ---
    // 운동량 보존을 위해 초기 상태의 운동량을 계산합니다.
    G4ReactionProduct theNeutron(theTrack.GetDefinition());
    theNeutron.SetMomentum(theTrack.Get4Momentum().vect());
    theNeutron.SetKineticEnergy(theTrack.GetKineticEnergy());

    G4ReactionProduct theTarget;
    G4Nucleus aNucleus;
    G4double targetMass = G4NucleiProperties::GetNuclearMass(theBaseA, theBaseZ);
    theTarget = aNucleus.GetBiasedThermalNucleus(targetMass, theNeutron.GetMomentum(), theTrack.GetMaterial()->GetTemperature());

    G4LorentzVector pInitial = theNeutron.Get4Momentum() + theTarget.Get4Momentum();
    G4LorentzVector pFinalProducts(0,0,0,0);

    // 생성된 모든 감마선/전자를 2차 입자로 추가하고 최종 운동량을 합산합니다.
    for (size_t i = 0; i < theProducts->size(); ++i) {
        auto theOne = new G4DynamicParticle;
        theOne->SetDefinition((*theProducts)[i]->GetDefinition());
        theOne->SetMomentum((*theProducts)[i]->GetMomentum());
        
        // Geant4 좌표계로 변환 (Lorentz boost)
        G4ReactionProduct temp;
        temp.SetDefinition(theOne->GetDefinition());
        temp.SetTotalEnergy(theOne->GetTotalEnergy());
        temp.SetMomentum(theOne->GetMomentum());
        temp.Lorentz(temp, -1. * theTarget);
        theOne->Set4Momentum(temp.Get4Momentum());

        theResult.Get()->AddSecondary(theOne, secID);
        pFinalProducts += theOne->Get4Momentum();
        delete (*theProducts)[i];
    }
    delete theProducts;
    
    // --- 4. 되튐 핵(Recoil Nucleus) 추가 및 운동량 보존 ---
    G4ParticleDefinition* recoil_def = G4IonTable::GetIonTable()->GetIon(targZ, targA + 1, 0.0);
    G4LorentzVector pRecoil = pInitial - pFinalProducts;
    auto recoil_particle = new G4DynamicParticle(recoil_def, pRecoil);
    theResult.Get()->AddSecondary(recoil_particle, secID);

    // 초기 중성자는 소멸시킴
    theResult.Get()->SetStatusChange(stopAndKill);
    return theResult.Get();
}

/**
 * @brief Geant4 NeutronHP 데이터를 로드하는 표준 초기화 함수
 */
void GdNeutronHPCaptureFS::Init(G4double A, G4double Z, G4int M, G4String& dirName, G4String&, G4ParticleDefinition*) 
{
    G4String tString = "/FS";
    G4bool dbool;
    G4ParticleHPDataUsed aFile = G4ParticleHPNames::GetInstance()->GetName(static_cast<G4int>(A), static_cast<G4int>(Z), M, dirName, tString, dbool);
    G4String filename = aFile.GetName();

    SetAZMs(static_cast<G4int>(A), static_cast<G4int>(Z), M, aFile);

    if(!dbool) {
        hasAnyData = false;
        hasFSData = false;
        hasXsec = false;
        return;
    }
    
    std::istringstream theData(std::ios::in);
    G4ParticleHPManager::GetInstance()->GetDataStream(filename, theData);
    hasFSData = theFinalStatePhotons.InitMean(theData);
    if(hasFSData) {
        targetMass = theFinalStatePhotons.GetTargetMass();
        theFinalStatePhotons.InitAngular(theData);
        theFinalStatePhotons.InitEnergies(theData);
    }
}
