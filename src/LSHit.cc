#include "LSHit.hh"

G4ThreadLocal G4Allocator<LSHit>* LSHitAllocator = nullptr;

LSHit::LSHit()
: G4VHit(),
  fTrackID(0), fParentID(0),
  fParticleName(""), fProcessName(""), fVolumeName(""),
  fPosition(0,0,0), fTime(0.),
  fKineticEnergy(0.), fEnergyDeposit(0.)
{}

LSHit::~LSHit()
{}
