#ifndef GdNeutronHPCapture_h
#define GdNeutronHPCapture_h 1

#include "G4HadronicInteraction.hh"
#include "G4ParticleHPChannel.hh"
#include "globals.hh"
#include <memory> // std::unique_ptr 사용을 위해 추가

// Forward declarations
class G4GenericMessenger;
namespace ANNRIGdGammaSpecModel {
    class ANNRIGd_GdNCaptureGammaGenerator;
}

/**
 * @class GdNeutronHPCapture
 * @brief [리팩토링] ANNRI-Gd 모델을 관리하고 Geant4와 연동하는 싱글턴 클래스
 *
 * 전역 변수 대신 모든 설정을 멤버 변수로 관리하며, G4GenericMessenger를 통해
 * 매크로에서 이 설정들을 제어할 수 있도록 합니다.
 */
class GdNeutronHPCapture : public G4HadronicInteraction {
public:
    // 싱글턴 인스턴스를 가져오는 public static 메소드
    static GdNeutronHPCapture* GetInstance();

    // 소멸자
    ~GdNeutronHPCapture() override;

    // Geant4 인터페이스 메소드
    G4HadFinalState* ApplyYourself(const G4HadProjectile& aTrack, G4Nucleus& aTargetNucleus) override;
    void BuildPhysicsTable(const G4ParticleDefinition&) override;
    const std::pair<G4double, G4double> GetFatalEnergyCheckLevels() const override;

    // --- public 인터페이스 ---
    // GdNeutronHPCaptureFS가 생성기와 설정에 접근할 수 있도록 public으로 제공
    ANNRIGdGammaSpecModel::ANNRIGd_GdNCaptureGammaGenerator* GetANNRIGdGenerator();
    G4int GetCaptureMode() const { return fCaptureMode; }
    G4int GetCascadeMode() const { return fCascadeMode; }

private:
    // 생성자를 private으로 만들어 외부에서 객체를 생성하지 못하게 함
    GdNeutronHPCapture();

    // --- 멤버 변수 ---
    static G4ThreadLocal GdNeutronHPCapture* fInstance;
    
    // UI 명령어 제어를 위한 메신저
    std::unique_ptr<G4GenericMessenger> fMessenger;
    
    // ANNRI-Gd 생성기 인스턴스 (스마트 포인터로 관리)
    std::unique_ptr<ANNRIGdGammaSpecModel::ANNRIGd_GdNCaptureGammaGenerator> fAnnriGammaGen;

    // 매크로로 제어될 설정값들 (전역 변수 대체)
    G4int fCaptureMode;      // 1:natural, 2:157Gd, 3:155Gd
    G4int fCascadeMode;      // 1:all, 2:discrete, 3:continuum
    G4String fGd155DataFile;
    G4String fGd157DataFile;
    G4int    fVerboseLevel; // 상세 출력 제어 변수

    // --- Helper 함수 ---
    void DefineCommands(); // 메신저 명령어 정의 함수
    void InitializeGenerator(); // ANNRI-Gd 생성기 초기화 함수

    // Geant4 NeutronHP 관련 멤버
    std::vector<G4ParticleHPChannel*>* theCapture{nullptr};
    G4String dirName;
    G4int numEle{0};
};

#endif
