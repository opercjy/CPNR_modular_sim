#include "RAT/GdNeutronHPCapture.hh"

// Geant4 includes
#include "G4GenericMessenger.hh"
#include "G4Threading.hh"
#include "G4ParticleHPManager.hh"
#include "G4NeutronHPCaptureFS.hh"
#include "G4Element.hh"
#include "G4IonTable.hh"
#include "G4SystemOfUnits.hh"

// ANNRI-Gd includes
#include "RAT/GdNeutronHPCaptureFS.hh"
#include "RAT/ANNRIGd_GdNCaptureGammaGenerator.hh"
#include "RAT/ANNRIGd_GeneratorConfigurator.hh"

// 싱글턴 인스턴스 초기화
G4ThreadLocal GdNeutronHPCapture* GdNeutronHPCapture::fInstance = nullptr;

GdNeutronHPCapture* GdNeutronHPCapture::GetInstance() {
    if (fInstance == nullptr) {
        fInstance = new GdNeutronHPCapture();
    }
    return fInstance;
}

GdNeutronHPCapture::GdNeutronHPCapture() 
  : G4HadronicInteraction("NeutronHPCapture_ANNRI"),
    fCaptureMode(1), // 기본값: natural Gd
    fCascadeMode(1), // 기본값: discrete + continuum
    fGd155DataFile("data/156GdContTbl_E1SLO4_HFB.root"), // 기본 데이터 파일 경로
    fGd157DataFile("data/158GdContTbl_E1SLO4_HFB.root")
{
    SetMinEnergy(0.0);
    SetMaxEnergy(20. * MeV);
    DefineCommands();
}

GdNeutronHPCapture::~GdNeutronHPCapture() {
    // atexit으로 자동 삭제되므로 스마트 포인터가 관리하는 멤버 외에는 특별히 할 일 없음
}

void GdNeutronHPCapture::DefineCommands() {
    // G4GenericMessenger를 사용하여 UI 명령어들을 정의합니다.
    fMessenger = std::make_unique<G4GenericMessenger>(this, "/myApp/phys/gd/", "ANNRI-Gd Model Control");

    auto& captureCmd = fMessenger->DeclareProperty("setCaptureMode", fCaptureMode, "Set Gd capture mode (1:nat, 2:157Gd, 3:155Gd)");
    captureCmd.SetParameterName("mode", false);
    
    auto& cascadeCmd = fMessenger->DeclareProperty("setCascadeMode", fCascadeMode, "Set Gd cascade mode (1:all, 2:discrete, 3:continuum)");
    cascadeCmd.SetParameterName("mode", false);

    auto& file155Cmd = fMessenger->DeclareProperty("set155GdDataFile", fGd155DataFile, "Set data file for 155Gd(n,g)");
    file155Cmd.SetParameterName("filename", false);

    auto& file157Cmd = fMessenger->DeclareProperty("set157GdDataFile", fGd157DataFile, "Set data file for 157Gd(n,g)");
    file157Cmd.SetParameterName("filename", false);
}

void GdNeutronHPCapture::InitializeGenerator() {
    // 생성기가 아직 초기화되지 않았다면, 현재 설정값으로 초기화합니다.
    if (!fAnnriGammaGen) {
        fAnnriGammaGen = std::make_unique<ANNRIGdGammaSpecModel::ANNRIGd_GdNCaptureGammaGenerator>();
        ANNRIGdGammaSpecModel::ANNRIGd_GeneratorConfigurator::Configure(
            *fAnnriGammaGen, fCaptureMode, fCascadeMode, fGd155DataFile, fGd157DataFile
        );
    }
}

ANNRIGdGammaSpecModel::ANNRIGd_GdNCaptureGammaGenerator* GdNeutronHPCapture::GetANNRIGdGenerator() {
    // 생성기를 사용하기 전에 초기화되었는지 확인
    if (!fAnnriGammaGen) {
        InitializeGenerator();
    }
    return fAnnriGammaGen.get();
}

void GdNeutronHPCapture::BuildPhysicsTable(const G4ParticleDefinition&) {
    // 이 메소드는 Master 스레드에서만 호출되어야 합니다.
    if (!G4Threading::IsMasterThread()) return;

    G4ParticleHPManager* hpmanager = G4ParticleHPManager::GetInstance();
    theCapture = hpmanager->GetCaptureFinalStates();

    if (theCapture == nullptr) theCapture = new std::vector<G4ParticleHPChannel*>;
    if (theCapture->size() == G4Element::GetNumberOfElements()) return;

    dirName = G4FindDataDir("G4NEUTRONHPDATA");
    dirName += "/Capture";

    auto theFS = new G4NeutronHPCaptureFS;
    auto theGdFS = new GdNeutronHPCaptureFS; // 우리 커스텀 Final State 모델

    for (size_t i = 0; i < G4Element::GetNumberOfElements(); ++i) {
        theCapture->push_back(new G4ParticleHPChannel);
        if ((*(G4Element::GetElementTable()))[i]->GetZ() == 64) { // Z=64, 가돌리늄
            ((*theCapture)[i])->Init((*(G4Element::GetElementTable()))[i], dirName);
            ((*theCapture)[i])->Register(theGdFS); // Gd일 경우 우리 모델 등록
        } else {
            ((*theCapture)[i])->Init((*(G4Element::GetElementTable()))[i], dirName);
            ((*theCapture)[i])->Register(theFS); // 그 외에는 표준 모델 등록
        }
    }
    delete theFS;
    delete theGdFS;
    hpmanager->RegisterCaptureFinalStates(theCapture);
}

G4HadFinalState* GdNeutronHPCapture::ApplyYourself(const G4HadProjectile& aTrack, G4Nucleus& aNucleus) {
    // 이 부분의 로직은 원본과 거의 동일하게 유지하되,
    // 전역 변수 대신 이 클래스의 멤버 변수를 사용하도록 수정해야 합니다.
    // (이 예제에서는 GdNeutronHPCaptureFS가 싱글턴 인스턴스에서 직접 값을 가져가므로 수정 불필요)
    G4ParticleHPManager::GetInstance()->OpenReactionWhiteBoard();
    
    // ... 원본과 동일한 타겟 원소 결정 로직 ...
    
    // index는 반응한 원소의 인덱스입니다.
    G4HadFinalState* result = ((*theCapture)[index])->ApplyYourself(aTrack);
    
    // ... 원본과 동일한 나머지 로직 ...

    G4ParticleHPManager::GetInstance()->CloseReactionWhiteBoard();
    return result;
}

// GetFatalEnergyCheckLevels() 등 나머지 함수는 원본과 동일
