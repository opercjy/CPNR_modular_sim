#ifndef PMTHit_h
#define PMTHit_h 1

#include "G4VHit.hh"
#include "G4THitsCollection.hh"
#include "G4Allocator.hh"

/**
 * @class PMTHit
 * @brief PMT의 광음극에 도달한 광학 광자(hit) 정보를 저장하는 데이터 클래스입니다.
 */
class PMTHit : public G4VHit
{
public:
  PMTHit();
  virtual ~PMTHit();

  inline void* operator new(size_t);
  inline void  operator delete(void*);

  void SetSegmentID(G4int id) { fSegmentID = id; }
  G4int GetSegmentID() const { return fSegmentID; }

  void SetPMTID(G4int id) { fPMTID = id; }
  G4int GetPMTID() const { return fPMTID; }

  void SetTime(G4double time) { fTime = time; }
  G4double GetTime() const { return fTime; }

private:
  G4int fSegmentID; // 광자를 검출한 세그먼트의 번호
  G4int fPMTID;     // 세그먼트 내 PMT의 번호 (0 또는 1)
  G4double fTime;   // 광자 검출 시간
};

typedef G4THitsCollection<PMTHit> PMTHitsCollection;
extern G4ThreadLocal G4Allocator<PMTHit>* PMTHitAllocator;

inline void* PMTHit::operator new(size_t)
{
  if (!PMTHitAllocator) PMTHitAllocator = new G4Allocator<PMTHit>;
  return (void*)PMTHitAllocator->MallocSingle();
}

inline void PMTHit::operator delete(void* aHit)
{
  PMTHitAllocator->FreeSingle((PMTHit*)aHit);
}

#endif
