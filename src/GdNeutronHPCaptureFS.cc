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
#include "G4RandomDirection.hh"
#include "G4ParticleHPDataUsed.hh"
#include "G4ParticleHPNames.hh"
#include "G4PhysicsModelCatalog.hh"
#include "G4ParticleHPThermalBoost.hh"
#include "G4LorentzVector.hh"

// ANNRI-Gd includes
#include "ANNRIGd_GdNCaptureGammaGenerator.hh"
#include "ANNRIGd_OutputConverter.hh"

GdNeutronHPCaptureFS::GdNeutronHPCaptureFS() 
{
    secID = G4PhysicsModelCatalog::GetModelID("model_NeutronHPCapture_ANNRI_FS");
    hasXsec = false;
    targetMass = 0.0;
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

    ANNRIGdGammaSpecModel::ReactionProductVector products;
    if (fCaptureMode == 1) {
        products = fAnnriGammaGen->Generate_NatGd();
    } else if (fCaptureMode == 2) {
        if(fCascadeMode == 1)      products = fAnnriGammaGen->Generate_158Gd();
        else if(fCascadeMode == 2) products = fAnnriGammaGen->Generate_158Gd_Discrete();
        else                       products = fAnnriGammaGen->Generate_158Gd_Continuum();
    } else if (fCaptureMode == 3) {
        if(fCascadeMode == 1)      products = fAnnriGammaGen->Generate_156Gd();
        else if(fCascadeMode == 2) products = fAnnriGammaGen->Generate_156Gd_Discrete();
        else                       products = fAnnriGammaGen->Generate_156Gd_Continuum();
    }
    
    // --- [확인용 출력문] 생성된 감마선 개수를 출력합니다 ---
    G4cout << "GdNeutronHPCaptureFS: ANNRI-Gd Generator produced " << products.size() << " secondaries." << G4endl;
    // ---------------------------------------------------------
    
    G4ReactionProduct theNeutron;
    G4ReactionProduct theTarget;
    G4LorentzVector pInitial;
    CalculateInitialState(theTrack, theNeutron, theTarget, pInitial);

    G4LorentzVector pFinalProducts(0,0,0,0);
    AddSecondariesToFinalState(products, theTarget, pFinalProducts);
    
    G4LorentzVector pRecoil = pInitial - pFinalProducts;
    AddRecoilToFinalState(pRecoil, targZ, targA);

    theResult.Get()->SetStatusChange(stopAndKill);
    return theResult.Get();
}

void GdNeutronHPCaptureFS::CalculateInitialState(const G4HadProjectile& theTrack, G4ReactionProduct& theNeutron, G4ReactionProduct& theTarget, G4LorentzVector& pInitial) 
{
    theNeutron.SetDefinition(theTrack.GetDefinition());
    theNeutron.SetMomentum(theTrack.Get4Momentum().vect());
    theNeutron.SetKineticEnergy(theTrack.GetKineticEnergy());

    // --- [최종 수정] 리액션 화이트보드에서 A와 Z를 가져옵니다 ---
    G4int targA = G4ParticleHPManager::GetInstance()->GetReactionWhiteBoard()->GetTargA();
    G4int targZ = G4ParticleHPManager::GetInstance()->GetReactionWhiteBoard()->GetTargZ();
    // -----------------------------------------------------------

    G4Nucleus aNucleus;
    // --- [최종 수정] 올바른 A와 Z를 사용합니다 ---
    targetMass = G4NucleiProperties::GetNuclearMass(targA, targZ);

    G4ThreeVector neutronVelocity = (1./theNeutron.GetMass()) * theNeutron.GetMomentum();
    theTarget = aNucleus.GetBiasedThermalNucleus(targetMass, neutronVelocity, theTrack.GetMaterial()->GetTemperature());
    
    G4LorentzVector p_neutron(theNeutron.GetMomentum(), theNeutron.GetTotalEnergy());
    G4LorentzVector p_target(theTarget.GetMomentum(), theTarget.GetTotalEnergy());
    pInitial = p_neutron + p_target;
}

void GdNeutronHPCaptureFS::AddSecondariesToFinalState(const ANNRIGdGammaSpecModel::ReactionProductVector& products, const G4ReactionProduct& theTarget, G4LorentzVector& pFinalProducts) 
{
    auto g4products = ANNRIGdGammaSpecModel::ANNRIGd_OutputConverter::ConvertToG4(products);
    for (size_t i = 0; i < g4products->size(); ++i) {
        G4ReactionProduct* product = (*g4products)[i];
        
        // --- [수정] 이 불필요한 좌표 변환 라인을 제거(주석 처리)합니다 ---
        // product->Lorentz(*product, -1. * theTarget);
        // ---------------------------------------------------------------

        auto theOne = new G4DynamicParticle;
        theOne->SetDefinition(product->GetDefinition());
        theOne->Set4Momentum(G4LorentzVector(product->GetMomentum(), product->GetTotalEnergy()));
        
        theResult.Get()->AddSecondary(theOne, secID);
        pFinalProducts += theOne->Get4Momentum();
        delete product;
    }
    delete g4products;
}

void GdNeutronHPCaptureFS::AddRecoilToFinalState(const G4LorentzVector& pRecoil, G4int targZ, G4int targA) 
{
    // 반동핵의 에너지가 너무 작아 추적할 수 없는 경우를 대비한 방어 코드
    if (pRecoil.e() - pRecoil.m() <= 0.0) return;

    G4ParticleDefinition* recoil_def = G4IonTable::GetIonTable()->GetIon(targZ, targA + 1, 0.0);
    auto recoil_particle = new G4DynamicParticle(recoil_def, pRecoil);
    theResult.Get()->AddSecondary(recoil_particle, secID);
}

void GdNeutronHPCaptureFS::Init(G4double A, G4double Z, G4int M, const G4String& dirName, const G4String&, G4ParticleDefinition*) 
{
    G4String tString = "/FS";
    G4bool dbool;
    G4ParticleHPNames theNames;
    G4ParticleHPDataUsed aFile = theNames.GetName(static_cast<G4int>(A), static_cast<G4int>(Z), M, dirName, tString, dbool);
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
