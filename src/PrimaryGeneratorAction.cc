#include "PrimaryGeneratorAction.hh"

#include "G4Event.hh"
#include "G4GeneralParticleSource.hh"

PrimaryGeneratorAction::PrimaryGeneratorAction()
: G4VUserPrimaryGeneratorAction(), fGPS(nullptr)
{
  fGPS = new G4GeneralParticleSource();
}

PrimaryGeneratorAction::~PrimaryGeneratorAction()
{
  delete fGPS;
}

void PrimaryGeneratorAction::GeneratePrimaries(G4Event* anEvent)
{
  fGPS->GeneratePrimaryVertex(anEvent);
}
