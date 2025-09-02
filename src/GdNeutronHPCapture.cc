#include "GdNeutronHPCapture.hh"

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

// ANNRI-Gd includes (RAT 경로 제거)
#include "GdNeutronHPCaptureFS.hh"
#include "ANNRIGd_GdNCaptureGammaGenerator.hh"
#include "ANNRIGd_GeneratorConfigurator.hh"

// 싱글턴 인스턴스 초기화
G4ThreadLocal GdNeutronHPCapture* GdNeutronHPCapture::fInstance = nullptr;

/**
 * @brief 싱글턴 인스턴스를 반환하는 static 메소드
 */
GdNeutronHPCapture* GdNeutronHPCapture::GetInstance() {
    if (fInstance == nullptr) {
        fInstance = new GdNeutronHPCapture();
    }
    return fInstance;
}

/**
 * @brief 생성자 (private)
 */
GdNeutronHPCapture::GdNeutronHPCapture() 
  : G4HadronicInteraction("NeutronHPCapture_ANNRI"),
    fIsGeneratorInitialized(false),
    fCaptureMode(1), // 기본값: natural Gd
    fCascadeMode(1), // 기본값: discrete + continuum
    fGd155DataFile("data/156GdContTbl_E1SLO4_HFB.root"), // 기본 데이터 파일 경로
    fGd157DataFile("data/158GdContTbl_E1SLO4_HFB.root")
{
    SetMinEnergy(0.0);
    SetMaxEnergy(20. * MeV);
    DefineCommands(); // 메신저 초기화
}

/**
 * @brief 소멸자
 */
GdNeutronHPCapture::~GdNeutronHPCapture() {
    // fMessenger와 fAnnriGammaGen은 unique_ptr이므로 자동 삭제됩니다.
    if (!G4Threading::IsWorkerThread()) {
        if (theCapture != nullptr) {
            for (auto& ite : *theCapture) {
                delete ite;
            }
            theCapture->clear();
        }
    }
}

/**
 * @brief G4GenericMessenger를 이용해 UI 명령어를 정의하는 함수
 */
void GdNeutronHPCapture::DefineCommands() {
    fMessenger = std::make_unique<G4GenericMessenger>(this, "/myApp/phys/gd/", "ANNRI-Gd Model Control");

    auto& captureCmd = fMessenger->DeclareProperty("captureMode", fCaptureMode, "Set Gd capture mode (1:nat, 2:157Gd, 3:155Gd)");
    captureCmd.SetParameterName("mode", false);
    captureCmd.SetGuidance(" 1: natural Gd\n 2: enriched 157Gd\n 3: enriched 155Gd");
    
    auto& cascadeCmd = fMessenger->DeclareProperty("cascadeMode", fCascadeMode, "Set Gd cascade mode (1:all, 2:discrete, 3:continuum)");
    cascadeCmd.SetParameterName("mode", false);
    cascadeCmd.SetGuidance(" 1: discrete + continuum\n 2: discrete only\n 3: continuum only");

    auto& file155Cmd = fMessenger->DeclareProperty("dataFile155", fGd155DataFile, "Data file for 155Gd(n,g) -> 156Gd continuum");
    file155Cmd.SetParameterName("filename", false);

    auto& file157Cmd = fMessenger->DeclareProperty("dataFile157", fGd157DataFile, "Data file for 157Gd(n,g) -> 158Gd continuum");
    file157Cmd.SetParameterName("filename", false);
}

/**
 * @brief ANNRI-Gd 생성기를 멤버 변수에 설정된 값으로 초기화하는 함수
 */
void GdNeutronHPCapture::InitializeGenerator() {
    if (!fIsGeneratorInitialized) {
        G4cout << "GdNeutronHPCapture: Initializing ANNRI-Gd Generator with user settings..." << G4endl;
        fAnnriGammaGen = std::make_unique<ANNRIGdGammaSpecModel::ANNRIGd_GdNCaptureGammaGenerator>();
        
        // Configure 함수는 모든 모델을 생성해야 하므로, 항상 모든 데이터 파일을 전달합니다.
        // 실제 어떤 모델을 사용할지는 ApplyYourself 단계에서 결정됩니다.
        ANNRIGdGammaSpecModel::ANNRIGd_GeneratorConfigurator::Configure(
            *fAnnriGammaGen, 1, 1, fGd155DataFile, fGd157DataFile
        );
        fIsGeneratorInitialized = true;
        G4cout << "GdNeutronHPCapture: ANNRI-Gd Generator Initialized." << G4endl;
    }
}

/**
 * @brief ANNRI-Gd 생성기 인스턴스를 반환하는 public getter
 */
ANNRIGdGammaSpecModel::ANNRIGd_GdNCaptureGammaGenerator* GdNeutronHPCapture::GetANNRIGdGenerator() {
    if (!fIsGeneratorInitialized) {
        InitializeGenerator();
    }
    return fAnnriGammaGen.get();
}

/**
 * @brief Geant4 물리 테이블 빌드 시 호출되는 함수. Gd에 커스텀 모델을 등록합니다.
 */
void GdNeutronHPCapture::BuildPhysicsTable(const G4ParticleDefinition&) {
    if (!G4Threading::IsMasterThread()) return;

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
            G4cout << "ANNRI-Gd Physics: Registering custom model for " << (*(G4Element::GetElementTable()))[i]->GetName() << G4endl;
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

/**
 * @brief 상호작용 발생 시 Geant4 커널에 의해 호출되는 메인 함수
 */
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

    // 선택된 원소에 대해 Final State 모델 호출
    G4HadFinalState* result = ((*theCapture)[index])->ApplyYourself(aTrack);
    
    // 타겟 동위원소 정보를 aNucleus에 설정 (FS에서 사용 가능하도록)
    const G4Element* target_element = (*G4Element::GetElementTable())[index];
    G4int targA = G4ParticleHPManager::GetInstance()->GetReactionWhiteBoard()->GetTargA();
    const G4Isotope* target_isotope = nullptr;
    for (size_t j = 0; j < target_element->GetNumberOfIsotopes(); ++j) {
        if (target_element->GetIsotope(j)->GetN() == targA) {
            target_isotope = target_element->GetIsotope(j);
            break;
        }
    }
    if (target_isotope) {
        aNucleus.SetIsotope(target_isotope);
    }

    G4ParticleHPManager::GetInstance()->CloseReactionWhiteBoard();
    return result;
}

/**
 * @brief FS 클래스에서 사용할 captureMode를 결정하는 함수
 */
G4int GdNeutronHPCapture::GetCaptureModeForIsotope(const G4Isotope* iso) {
    if (fCaptureMode != 1) { // natural Gd 모드가 아니면, 사용자가 설정한 값을 따름
        return fCaptureMode;
    }

    if (iso) {
        if (iso->GetName() == "Gd155") return 3;
        if (iso->GetName() == "Gd157") return 2;
    }
    return 1; // 기본값은 natural Gd
}

const std::pair<G4double, G4double> GdNeutronHPCapture::GetFatalEnergyCheckLevels() const {
  return std::pair<G4double, G4double>(10.0 * perCent, 350.0 * CLHEP::GeV);
}
