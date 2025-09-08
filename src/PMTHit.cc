#include "PMTHit.hh"

G4ThreadLocal G4Allocator<PMTHit>* PMTHitAllocator = nullptr;

PMTHit::PMTHit() 
: G4VHit(), 
  fSegmentID(-1), fPMTID(-1), fTime(0.) 
{}

PMTHit::~PMTHit() {}
