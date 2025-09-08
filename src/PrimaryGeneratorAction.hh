// PrimaryGeneratorAction.hh
#ifndef PrimaryGeneratorAction_h
#define PrimaryGeneratorAction_h 1

#include "G4VUserPrimaryGeneratorAction.hh"
#include "globals.hh"

class G4GeneralParticleSource;
class G4Event;

class PrimaryGeneratorAction : public G4VUserPrimaryGeneratorAction
{
public:
  PrimaryGeneratorAction();
  virtual ~PrimaryGeneratorAction();
  virtual void GeneratePrimaries(G4Event* anEvent) override;
private:
  G4GeneralParticleSource* fGPS;
};
#endif

// SteppingAction.hh
#ifndef SteppingAction_h
#define SteppingAction_h 1
#include "G4UserSteppingAction.hh"
class SteppingAction : public G4UserSteppingAction
{
public:
  SteppingAction();
  virtual ~SteppingAction();
  virtual void UserSteppingAction(const G4Step*) override;
};
#endif

// TrackingAction.hh
#ifndef TrackingAction_h
#define TrackingAction_h 1
#include "G4UserTrackingAction.hh"
#include "globals.hh"
class TrackingAction : public G4UserTrackingAction
{
public:
  TrackingAction();
  virtual ~TrackingAction();
  virtual void PreUserTrackingAction(const G4Track* track) override;
  virtual void PostUserTrackingAction(const G4Track* track) override;
};
#endif
