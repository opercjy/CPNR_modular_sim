#ifndef GdNeutronHPCaptureFS_h
#define GdNeutronHPCaptureFS_h 1

#include "G4ParticleHPFinalState.hh"
#include "G4ParticleHPPhotonDist.hh"
#include "G4LorentzVector.hh"
#include "ANNRIGd_ReactionProduct.hh"

class GdNeutronHPCaptureFS : public G4ParticleHPFinalState {
 public:
  GdNeutronHPCaptureFS();
  ~GdNeutronHPCaptureFS() override = default;

  void Init(G4double A, G4double Z, G4int M, const G4String& dirName, const G4String& aFSType, G4ParticleDefinition*) override;
  G4HadFinalState* ApplyYourself(const G4HadProjectile& theTrack) override;
  
  G4ParticleHPFinalState* New() override {
    return new GdNeutronHPCaptureFS;
  }
  
 private:
  void CalculateInitialState(const G4HadProjectile& theTrack, G4ReactionProduct& theNeutron, G4ReactionProduct& theTarget, G4LorentzVector& pInitial);
  void AddSecondariesToFinalState(const ANNRIGdGammaSpecModel::ReactionProductVector& products, const G4ReactionProduct& theTarget, G4LorentzVector& pFinalProducts);
  void AddRecoilToFinalState(const G4LorentzVector& pRecoil, G4int targZ, G4int targA);

  G4ParticleHPPhotonDist theFinalStatePhotons;
  G4double targetMass;
};
#endif
