// LSHit.hh (변경 없음)
#ifndef LSHit_h
#define LSHit_h 1
#include "G4VHit.hh"
#include "G4THitsCollection.hh"
#include "G4Allocator.hh"
#include "G4ThreeVector.hh"
#include "G4String.hh"

class LSHit : public G4VHit
{
public:
  LSHit();
  virtual ~LSHit();
  // ... Getters and Setters ...
  void SetVolumeName(const G4String& name) { fVolumeName = name; }
  const G4String& GetVolumeName() const { return fVolumeName; }
  // ...
private:
  G4String fVolumeName;
  // ...
};
typedef G4THitsCollection<LSHit> LSHitsCollection;
#endif
