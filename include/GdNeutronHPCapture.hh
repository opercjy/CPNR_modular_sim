#ifndef GdNeutronHPCapture_h
#define GdNeutronHPCapture_h 1

#include "G4HadronicInteraction.hh"
#include <memory>
#include <vector>

// Forward declarations
class G4ParticleHPChannel;
class G4GenericMessenger;
class G4Isotope;
namespace ANNRIGdGammaSpecModel {
    class ANNRIGd_GdNCaptureGammaGenerator;
}

class GdNeutronHPCapture : public G4HadronicInteraction {
public:
    static GdNeutronHPCapture* GetInstance();
    ~GdNeutronHPCapture() override;

    G4HadFinalState* ApplyYourself(const G4HadProjectile& aTrack, G4Nucleus& aTargetNucleus) override;
    void BuildPhysicsTable(const G4ParticleDefinition&) override;
    const std::pair<G4double, G4double> GetFatalEnergyCheckLevels() const override;

    ANNRIGdGammaSpecModel::ANNRIGd_GdNCaptureGammaGenerator* GetANNRIGdGenerator();
    G4int GetCaptureModeForIsotope(const G4Isotope* iso);
    G4int GetCascadeMode() const { return fCascadeMode; }
    
    void SetCurrentTargetIsotope(const G4Isotope* iso) { fCurrentTargetIsotope = iso; }
    const G4Isotope* GetCurrentTargetIsotope() const { return fCurrentTargetIsotope; }

private:
    GdNeutronHPCapture();

    static G4ThreadLocal GdNeutronHPCapture* fInstance;
    
    std::unique_ptr<G4GenericMessenger> fMessenger;
    std::unique_ptr<ANNRIGdGammaSpecModel::ANNRIGd_GdNCaptureGammaGenerator> fAnnriGammaGen;
    bool fIsGeneratorInitialized;

    const G4Isotope* fCurrentTargetIsotope; 

    G4int    fCaptureMode;
    G4int    fCascadeMode;
    G4String fGd155DataFile;
    G4String fGd157DataFile;
    G4int    fVerboseLevel;

    void DefineCommands();
    void InitializeGenerator();

    std::vector<G4ParticleHPChannel*>* theCapture{nullptr};
    G4String dirName;
    G4int numEle{0};
};

#endif
