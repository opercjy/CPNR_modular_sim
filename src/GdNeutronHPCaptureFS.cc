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
#include "G4PhysicsModelCatalog.hh"
#include "G4ParticleHPThermalBoost.hh"

// ANNRI-Gd includes (RAT 경로 제거)
#include "ANNRIGd_GdNCaptureGammaGenerator.hh"
#include "ANNRIGd_OutputConverter.hh"

GdNeutronHPCaptureFS::GdNeutronHPCaptureFS() 
{
    secID = G4PhysicsModelCatalog::GetModelID("model_NeutronHPCapture_ANNRI_FS");
    hasXsec = false;
}

/**
 * @brief 메인 함수: 역할을 분리하여 코드 흐름을 명확하게 보여줍니다.
 */
G4HadFinalState* GdNeutronHPCaptureFS::ApplyYourself(const G4HadProjectile& theTrack) 
{
    if (theResult.Get() == nullptr) theResult.Put(new G4HadFinalState);
    theResult.Get()->Clear();

    // 1. 초기 상태 운동량 계산
    G4ReactionProduct theNeutron;
    G4ReactionProduct theTarget;
    G4LorentzVector pInitial;
    CalculateInitialState(theTrack, theNeutron, theTarget, pInitial);

    // 2. ANNRI-Gd 생성기 호출 및 감마선 생성
    auto manager = GdNeutronHPCapture::GetInstance();
    auto annriGenerator = manager->GetANNRIGdGenerator();
    G4int targA = G4ParticleHPManager::GetInstance()->GetReactionWhiteBoard()->GetTargA();
    G4int targZ = G4ParticleHPManager::GetInstance()->GetReactionWhiteBoard()->GetTargZ();
    const G4Isotope* target_isotope = G4IonTable::GetIonTable()->GetIsotope(targZ, targA, 0.0);
    int captureMode = manager->GetCaptureModeForIsotope(target_isotope);
    int cascadeMode = manager->GetCascadeMode();
    
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
    
    // 3. 2차 입자(감마, 전자)들을 Final State에 추가
    G4LorentzVector pFinalProducts(0,0,0,0);
    AddSecondariesToFinalState(products, theTarget, pFinalProducts);
    
    // 4. 운동량 보존을 위해 반도일 핵 추가
    G4LorentzVector pRecoil = pInitial - pFinalProducts;
    AddRecoilToFinalState(pRecoil, targZ, targA);

    theResult.Get()->SetStatusChange(stopAndKill);
    return theResult.Get();
}

/**
 * @brief Helper: 초기 상태(중성자+타겟)의 운동량을 계산
 */
void GdNeutronHPCaptureFS::CalculateInitialState(const G4HadProjectile& theTrack, G4ReactionProduct& theNeutron, G4ReactionProduct& theTarget, G4LorentzVector& pInitial) 
{
    theNeutron.SetDefinition(theTrack.GetDefinition());
    theNeutron.SetMomentum(theTrack.Get4Momentum().vect());
    theNeutron.SetKineticEnergy(theTrack.GetKineticEnergy());

    G4Nucleus aNucleus;
    G4double targetMass = G4NucleiProperties::GetNuclearMass(theBaseA, theBaseZ);
    G4ParticleHPThermalBoost aThermalE;
    theTarget = aNucleus.GetBiasedThermalNucleus(targetMass, aThermalE.GetThermalCentralMomentum(theTrack.GetKineticEnergy(), targetMass, theTrack.GetMaterial()->GetTemperature()), theTrack.GetMaterial()->GetTemperature());
    
    pInitial = theNeutron.Get4Momentum() + theTarget.Get4Momentum();
}

/**
 * @brief Helper: ANNRI-Gd가 생성한 입자들을 Geant4의 2차 입자로 추가
 */
void GdNeutronHPCaptureFS::AddSecondariesToFinalState(const ANNRIGdGammaSpecModel::ReactionProductVector& products, const G4ReactionProduct& theTarget, G4LorentzVector& pFinalProducts) 
{
    auto g4products = ANNRIGdGammaSpecModel::ANNRIGd_OutputConverter::ConvertToG4(products);

    for (size_t i = 0; i < g4products->size(); ++i) {
        G4ReactionProduct* product = (*g4products)[i];
        
        // ANNRI-Gd 생성기는 핵의 정지 좌표계에서 입자를 생성하므로, 실험실 좌표계로 변환(Lorentz boost)
        product->Lorentz(*product, -1. * theTarget);

        auto theOne = new G4DynamicParticle;
        theOne->SetDefinition(product->GetDefinition());
        theOne->Set4Momentum(product->Get4Momentum());
        
        theResult.Get()->AddSecondary(theOne, secID);
        pFinalProducts += theOne->Get4Momentum();
        delete product;
    }
    delete g4products;
}

/**
 * @brief Helper: 운동량 보존을 위해 반도일 핵을 추가
 */
void GdNeutronHPCaptureFS::AddRecoilToFinalState(const G4LorentzVector& pRecoil, G4int targZ, G4int targA) 
{
    G4ParticleDefinition* recoil_def = G4IonTable::GetIonTable()->GetIon(targZ, targA + 1, 0.0);
    auto recoil_particle = new G4DynamicParticle(recoil_def, pRecoil);
    theResult.Get()->AddSecondary(recoil_particle, secID);
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
