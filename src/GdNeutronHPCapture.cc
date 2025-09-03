#include "GdNeutronHPCapture.hh"
#include <cstdlib> // getenv

// Geant4 includes
#include "G4GenericMessenger.hh"
#include "G4Threading.hh"
#include "G4ParticleHPManager.hh"
#include "G4NeutronHPCaptureFS.hh"
#include "G4Element.hh"
#include "G4Isotope.hh"
#include "G4IonTable.hh"
#include "G4SystemOfUnits.hh"
#include "G4HadronicException.hh"
#include "G4ParticleHPThermalBoost.hh"
#include "G4ParticleHPChannel.hh"

// ANNRI-Gd includes
#include "GdNeutronHPCaptureFS.hh"
#include "ANNRIGd_GdNCaptureGammaGenerator.hh"
#include "ANNRIGd_GeneratorConfigurator.hh"

G4ThreadLocal GdNeutronHPCapture* GdNeutronHPCapture::fInstance = nullptr;

GdNeutronHPCapture* GdNeutronHPCapture::GetInstance() {
    if (fInstance == nullptr) {
        fInstance = new GdNeutronHPCapture();
    }
    return fInstance;
}

GdNeutronHPCapture::GdNeutronHPCapture() 
  : G4HadronicInteraction("NeutronHPCapture_ANNRI"),
    fIsGeneratorInitialized(false),
    fCurrentTargetIsotope(nullptr),
    fCaptureMode(1),
    fCascadeMode(1),
    fVerboseLevel(1)
{
    SetMinEnergy(0.0);
    SetMaxEnergy(20. * MeV);
    DefineCommands();
}

GdNeutronHPCapture::~GdNeutronHPCapture() {
    if (!G4Threading::IsWorkerThread()) {
        if (theCapture != nullptr) {
            for (auto& ite : *theCapture) {
                delete ite;
            }
            theCapture->clear();
        }
    }
}

void GdNeutronHPCapture::DefineCommands() {
    fMessenger = std::make_unique<G4GenericMessenger>(this, "/myApp/phys/gd/", "ANNRI-Gd Model Control");

    auto& verboseCmd = fMessenger->DeclareProperty("verbose", fVerboseLevel, "Set verbosity level (0:silent, 1:default)");
    verboseCmd.SetParameterName("level", false);

    auto& captureCmd = fMessenger->DeclareProperty("captureMode", fCaptureMode, "Set Gd capture mode (1:nat, 2:157Gd, 3:155Gd)");
    captureCmd.SetParameterName("mode", false);
    captureCmd.SetGuidance(" 1: natural Gd\n 2: enriched 157Gd\n 3: enriched 155Gd");
    
    auto& cascadeCmd = fMessenger->DeclareProperty("cascadeMode", fCascadeMode, "Set Gd cascade mode (1:all, 2:discrete, 3:continuum)");
    cascadeCmd.SetParameterName("mode", false);
    cascadeCmd.SetGuidance(" 1: discrete + continuum\n 2: discrete only\n 3: continuum only");

    fMessenger->DeclareProperty("dataFile155", fGd155DataFile, "Data file for 155Gd(n,g) -> 156Gd continuum");
    fMessenger->DeclareProperty("dataFile157", fGd157DataFile, "Data file for 157Gd(n,g) -> 158Gd continuum");
}

void GdNeutronHPCapture::InitializeGenerator() {
    if (fIsGeneratorInitialized) return;

    const char* dataDirEnv = getenv("GD_CAPTURE_DATA_DIR");
    if (!dataDirEnv) {
        G4String msg = "Environment variable GD_CAPTURE_DATA_DIR is not set!";
        G4Exception("GdNeutronHPCapture::InitializeGenerator()", "FatalError", FatalException, msg);
    }
    
    G4String dataDir = dataDirEnv;
    fGd155DataFile = dataDir + "/" + "156GdContTbl__E1SLO4__HFB.root";
    fGd157DataFile = dataDir + "/" + "158GdContTbl__E1SLO4__HFB.root";

    if (fVerboseLevel > 0) {
        G4cout << "GdNeutronHPCapture: Initializing ANNRI-Gd Generator..." << G4endl;
        G4cout << ">> Using 155Gd data file: " << fGd155DataFile << G4endl;
        G4cout << ">> Using 157Gd data file: " << fGd157DataFile << G4endl;
    }
    
    fAnnriGammaGen = std::make_unique<ANNRIGdGammaSpecModel::ANNRIGd_GdNCaptureGammaGenerator>();
    ANNRIGdGammaSpecModel::ANNRIGd_GeneratorConfigurator::Configure(
        *fAnnriGammaGen, 1, 1, fGd155DataFile, fGd157DataFile
    );
    fIsGeneratorInitialized = true;
    
    if (fVerboseLevel > 0) {
        G4cout << "GdNeutronHPCapture: ANNRI-Gd Generator Initialized." << G4endl;
    }
}

ANNRIGdGammaSpecModel::ANNRIGd_GdNCaptureGammaGenerator* GdNeutronHPCapture::GetANNRIGdGenerator() {
    if (!fIsGeneratorInitialized && G4Threading::IsMasterThread()) {
        InitializeGenerator();
    }
    return fAnnriGammaGen.get();
}

void GdNeutronHPCapture::BuildPhysicsTable(const G4ParticleDefinition&) {
    if (!G4Threading::IsMasterThread()) return;
    
    InitializeGenerator();

    G4ParticleHPManager* hpmanager = G4ParticleHPManager::GetInstance();
    theCapture = hpmanager->GetCaptureFinalStates();

    if (theCapture == nullptr) theCapture = new std::vector<G4ParticleHPChannel*>;
    if (theCapture->size() == G4Element::GetNumberOfElements()) return;

    if (G4FindDataDir("G4NEUTRONHPDATA") == nullptr)
      throw G4HadronicException(__FILE__, __LINE__, "Please setenv G4NEUTRONHPDATA.");
    dirName = G4FindDataDir("G4NEUTRONHPDATA");
    dirName += "/Capture";

    auto theFS = new G4NeutronHPCaptureFS;
    auto theGdFS = new GdNeutronHPCaptureFS;

    for (size_t i = theCapture->size(); i < G4Element::GetNumberOfElements(); ++i) {
        theCapture->push_back(new G4ParticleHPChannel);
        if ((*(G4Element::GetElementTable()))[i]->GetZ() == 64) { // Z=64, 가돌리늄
            if (fVerboseLevel > 0) {
                G4cout << "ANNRI-Gd Physics: Registering custom model for " << (*(G4Element::GetElementTable()))[i]->GetName() << G4endl;
            }
            ((*theCapture)[i])->Init((*(G4Element::GetElementTable()))[i], dirName);
            ((*theCapture)[i])->Register(theGdFS);
        } else {
            ((*theCapture)[i])->Init((*(G4Element::GetElementTable()))[i], dirName);
            ((*theCapture)[i])->Register(theFS);
        }
    }
    delete theFS;
    delete theGdFS;
    hpmanager->RegisterCaptureFinalStates(theCapture);
}

G4HadFinalState* GdNeutronHPCapture::ApplyYourself(const G4HadProjectile& aTrack, G4Nucleus& aNucleus) {
    G4ParticleHPManager::GetInstance()->OpenReactionWhiteBoard();
    
    const G4Material* theMaterial = aTrack.GetMaterial();
    auto nElements = theMaterial->GetNumberOfElements();
    size_t index = 0;

    if (nElements > 1) {
        auto xSec = new G4double[nElements];
        G4double sum = 0;
        const G4double* NumAtomsPerVolume = theMaterial->GetVecNbOfAtomsPerVolume();
        G4ParticleHPThermalBoost aThermalE;
        for (size_t i = 0; i < nElements; ++i) {
            index = theMaterial->GetElement(i)->GetIndex();
            xSec[i] = ((*theCapture)[index])->GetXsec(aThermalE.GetThermalEnergy(aTrack, theMaterial->GetElement(i), theMaterial->GetTemperature()));
            xSec[i] *= NumAtomsPerVolume[i];
            sum += xSec[i];
        }
        G4double random = G4UniformRand();
        G4double running = 0;
        for (size_t i = 0; i < nElements; ++i) {
            running += xSec[i];
            index = theMaterial->GetElement(i)->GetIndex();
            if (sum == 0 || random <= running / sum) break;
        }
        delete[] xSec;
    } else {
        index = theMaterial->GetElement(0)->GetIndex();
    }
    
    const G4Element* target_element = (*G4Element::GetElementTable())[index];
    G4int targA = G4ParticleHPManager::GetInstance()->GetReactionWhiteBoard()->GetTargA();
    const G4Isotope* target_isotope = nullptr;
    if(targA > 0) {
        for (size_t j = 0; j < target_element->GetNumberOfIsotopes(); ++j) {
            if (target_element->GetIsotope(j)->GetN() == targA) {
                target_isotope = target_element->GetIsotope(j);
                break;
            }
        }
    }
    
    this->SetCurrentTargetIsotope(target_isotope);
    if (target_isotope) {
        aNucleus.SetIsotope(target_isotope);
    }
    
    G4HadFinalState* result = ((*theCapture)[index])->ApplyYourself(aTrack);

    G4ParticleHPManager::GetInstance()->CloseReactionWhiteBoard();
    return result;
}

G4int GdNeutronHPCapture::GetCaptureModeForIsotope(const G4Isotope* iso) {
    if (fCaptureMode != 1) {
        return fCaptureMode;
    }
    if (iso) {
        if (iso->GetName() == "Gd155") return 3;
        if (iso->GetName() == "Gd157") return 2;
    }
    return 1;
}

const std::pair<G4double, G4double> GdNeutronHPCapture::GetFatalEnergyCheckLevels() const {
  return std::pair<G4double, G4double>(10.0 * perCent, 350.0 * CLHEP::GeV);
}
