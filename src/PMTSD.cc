#include "PMTSD.hh"
#include "G4HCofThisEvent.hh"
#include "G4Step.hh"
#include "G4Track.hh"
#include "G4OpticalPhoton.hh"
#include "G4SystemOfUnits.hh"
#include "G4SDManager.hh"
#include "G4MaterialPropertiesTable.hh"
#include "Randomize.hh"

// --- [수정] G4RunManager 사용을 위한 헤더 추가 ---
#include "G4RunManager.hh" 
// ---------------------------------------------

PMTSD::PMTSD(const G4String& name)
: G4VSensitiveDetector(name), fHitsCollection(nullptr)
{
  collectionName.insert("PMTHitsCollection");
}

PMTSD::~PMTSD() {}

void PMTSD::Initialize(G4HCofThisEvent* hce)
{
  fHitsCollection = new PMTHitsCollection(SensitiveDetectorName, collectionName[0]);
  G4int hcID = G4SDManager::GetSDMpointer()->GetCollectionID(collectionName[0]);
  hce->AddHitsCollection(hcID, fHitsCollection);
}

G4bool PMTSD::ProcessHits(G4Step* aStep, G4TouchableHistory* /*ROhist*/)
{
  G4Track* track = aStep->GetTrack();
  if (track->GetDefinition() != G4OpticalPhoton::Definition()) return false;

  // 디버깅 메시지를 위해 eventID를 가져옵니다.
  G4int eventID = G4RunManager::GetRunManager()->GetCurrentEvent()->GetEventID();
  G4cout << "DEBUG (Event " << eventID << "): OpticalPhoton reached photocathode." << G4endl;


  G4MaterialPropertiesTable* pmtMPT = aStep->GetPreStepPoint()->GetMaterial()->GetMaterialPropertiesTable();
  if (!pmtMPT) return false;
  G4MaterialPropertyVector* qeVector = pmtMPT->GetProperty("EFFICIENCY");
  if (!qeVector) return false;

  if (G4UniformRand() > qeVector->Value(track->GetKineticEnergy())) {
    track->SetTrackStatus(fStopAndKill);
    return false;
  }
  
  G4cout << "DEBUG (Event " << eventID << "): Photon DETECTED!" << G4endl;

  // --- [수정] 기하구조 계층에 따른 정확한 ID 추출 ---
  // DetectorConstruction.cc를 기준으로, 광음극(photocathode)의 부모 계층은 다음과 같습니다:
  // Level 0: PhysPhotocathode (자신)
  // Level 1: PhysPMT (CopyNo: 0 또는 1 -> pmtID)
  // Level 2: PhysSegment (CopyNo: 0~24 -> segmentID)
  auto touchable = aStep->GetPreStepPoint()->GetTouchable();
  G4int pmtID = touchable->GetCopyNumber(1);
  G4int segmentID = touchable->GetCopyNumber(2);
  // ----------------------------------------------------

  PMTHit* newHit = new PMTHit();
  newHit->SetSegmentID(segmentID);
  newHit->SetPMTID(pmtID);
  newHit->SetTime(aStep->GetPostStepPoint()->GetGlobalTime() / ns);
  
  fHitsCollection->insert(newHit);
  track->SetTrackStatus(fStopAndKill);

  return true;
}
