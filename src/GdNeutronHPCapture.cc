#include "GdNeutronHPCapture.hh"

// Geant4 includes
#include "G4GenericMessenger.hh"
#include "G4SystemOfUnits.hh"
#include "G4Nucleus.hh"
#include "G4Isotope.hh"
#include "G4Element.hh"
#include "G4ParticleHPManager.hh" // 화이트보드 접근을 위해 추가
#include "G4ParticleHPReactionWhiteBoard.hh" // 화이트보드 클래스 직접 사용을 위해 추가

// ANNRI-Gd includes
#include "GdNeutronHPCaptureFS.hh"
#include "ANNRIGd_GdNCaptureGammaGenerator.hh"
#include "ANNRIGd_GeneratorConfigurator.hh"

GdNeutronHPCapture::GdNeutronHPCapture() 
  : G4NeutronHPCapture(),
    fIsGeneratorInitialized(false),
    fCaptureMode(1),
    fCascadeMode(1),
    fVerboseLevel(1)
{
    SetMinEnergy(0.0);
    SetMaxEnergy(20. * MeV);

    fFinalState = std::make_unique<G4HadFinalState>();
    fGdCaptureFS = std::make_unique<GdNeutronHPCaptureFS>();

    DefineCommands();
    InitializeGenerator();
}

GdNeutronHPCapture::~GdNeutronHPCapture() {}

void GdNeutronHPCapture::DefineCommands() {
    fMessenger = std::make_unique<G4GenericMessenger>(this, "/myApp/phys/gd/", "ANNRI-Gd Model Control");
    fMessenger->DeclareProperty("verbose", fVerboseLevel, "Set verbosity level (0:silent, 1:default)");
    fMessenger->DeclareProperty("captureMode", fCaptureMode, "Set Gd capture mode (1:nat, 2:157Gd, 3:155Gd)");
    fMessenger->DeclareProperty("cascadeMode", fCascadeMode, "Set Gd cascade mode (1:all, 2:discrete, 3:continuum)");
}

void GdNeutronHPCapture::InitializeGenerator() {
    if (fIsGeneratorInitialized) return;
    const char* dataDirEnv = getenv("GD_CAPTURE_DATA_DIR");
    if (!dataDirEnv) {
        G4Exception("GdNeutronHPCapture::InitializeGenerator()", "FatalError", FatalException, "Environment variable GD_CAPTURE_DATA_DIR is not set!");
    }
    G4String dataDir = dataDirEnv;
    fGd155DataFile = dataDir + "/" + "156GdContTbl__E1SLO4__HFB.root";
    fGd157DataFile = dataDir + "/" + "158GdContTbl__E1SLO4__HFB.root";
    
    fAnnriGammaGen = std::make_unique<ANNRIGdGammaSpecModel::ANNRIGd_GdNCaptureGammaGenerator>();
    ANNRIGdGammaSpecModel::ANNRIGd_GeneratorConfigurator::Configure(
        *fAnnriGammaGen, fCaptureMode, fCascadeMode, fGd155DataFile, fGd157DataFile);
    fIsGeneratorInitialized = true;
    if (fVerboseLevel > 0) G4cout << "GdNeutronHPCapture: ANNRI-Gd Generator Initialized." << G4endl;
}

G4HadFinalState* GdNeutronHPCapture::ApplyYourself(const G4HadProjectile& aTrack, G4Nucleus& aTargetNucleus)
{
    if (aTargetNucleus.GetZ_asInt() == 64) {
        if (fVerboseLevel > 0) {
            G4cout << "GdNeutronHPCapture: Neutron captured by Gadolinium. Using ANNRI-Gd model." << G4endl;
        }

        G4ParticleHPManager::GetInstance()->OpenReactionWhiteBoard();
        
        // --- [수정] 화이트보드에 타겟 핵의 A와 Z 정보를 직접 기록합니다 ---
        auto wb = G4ParticleHPManager::GetInstance()->GetReactionWhiteBoard();
        wb->SetTargA(aTargetNucleus.GetA_asInt());
        wb->SetTargZ(aTargetNucleus.GetZ_asInt());
        // ---------------------------------------------------------------
        
        fGdCaptureFS->SetAnnriGenerator(fAnnriGammaGen.get());
        fGdCaptureFS->SetModes(fCaptureMode, fCascadeMode);
        G4HadFinalState* result = fGdCaptureFS->ApplyYourself(aTrack);
        
        G4ParticleHPManager::GetInstance()->CloseReactionWhiteBoard();
        return result;

    } else {
        return G4NeutronHPCapture::ApplyYourself(aTrack, aTargetNucleus);
    }
}
