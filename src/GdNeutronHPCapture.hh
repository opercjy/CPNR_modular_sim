#ifndef GdNeutronHPCapture_h
#define GdNeutronHPCapture_h 1

#include "G4NeutronHPCapture.hh" // **중요**: G4HadronicInteraction 대신 G4NeutronHPCapture를 상속
#include <memory>

// Forward declarations
class G4GenericMessenger;
namespace ANNRIGdGammaSpecModel {
    class ANNRIGd_GdNCaptureGammaGenerator;
}

// G4NeutronHPCapture를 상속받는 클래스로 변경
class GdNeutronHPCapture : public G4NeutronHPCapture {
public:
    GdNeutronHPCapture();
    ~GdNeutronHPCapture() override;

    // 부모 클래스의 ApplyYourself를 재정의(override)
    G4HadFinalState* ApplyYourself(const G4HadProjectile& aTrack, G4Nucleus& aTargetNucleus) override;
    
private:
    std::unique_ptr<G4GenericMessenger> fMessenger;
    std::unique_ptr<ANNRIGdGammaSpecModel::ANNRIGd_GdNCaptureGammaGenerator> fAnnriGammaGen;
    
    // 이 클래스 내에서만 사용할 Final State 모델
    std::unique_ptr<G4HadFinalState> fFinalState;
    std::unique_ptr<class GdNeutronHPCaptureFS> fGdCaptureFS;

    bool fIsGeneratorInitialized;
    G4int fCaptureMode;
    G4int fCascadeMode;
    G4int fVerboseLevel;
    G4String fGd155DataFile;
    G4String fGd157DataFile;

    void DefineCommands();
    void InitializeGenerator();
};

#endif
