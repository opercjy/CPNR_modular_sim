// PMTHit.hh (리팩토링 제안)
#ifndef PMTHit_h
#define PMTHit_h 1
#include "G4VHit.hh"
#include "G4THitsCollection.hh"
#include "G4Allocator.hh"

class PMTHit : public G4VHit
{
public:
  PMTHit();
  virtual ~PMTHit();

  // --- [리팩토링] 세그먼트 ID 저장을 위한 멤버 추가 ---
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
#endif
