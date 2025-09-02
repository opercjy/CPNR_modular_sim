#ifndef GdNeutronHPCaptureFS_h
#define GdNeutronHPCaptureFS_h 1

#include "G4ParticleHPFinalState.hh"
#include "G4ParticleHPPhotonDist.hh"

class GdNeutronHPCaptureFS : public G4ParticleHPFinalState {
 public:
  GdNeutronHPCaptureFS();
  ~GdNeutronHPCaptureFS() override = default;

  void Init(G4double A, G4double Z, G4int M, G4String& dirName, G4String& aFSType, G4ParticleDefinition*) override;
  G4HadFinalState* ApplyYourself(const G4HadProjectile& theTrack) override;
  
  G4ParticleHPFinalState* New() override {
    return new GdNeutronHPCaptureFS;
  }
  
 private:
  G4ParticleHPPhotonDist theFinalStatePhotons;
};
#endif
