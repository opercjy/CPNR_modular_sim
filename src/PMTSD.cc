#include "PMTSD.hh"
#include "G4HCofThisEvent.hh"
#include "G4Step.hh"
#include "G4Track.hh"
#include "G4OpticalPhoton.hh"
#include "G4SystemOfUnits.hh"
#include "G4SDManager.hh"
#include "G4MaterialPropertiesTable.hh"
#include "Randomize.hh"

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

  // 양자 효율(QE) 기반으로 광자 검출 확률 계산
  G4MaterialPropertiesTable* pmtMPT = aStep->GetPreStepPoint()->GetMaterial()->GetMaterialPropertiesTable();
  if (!pmtMPT) return false;
  G4MaterialPropertyVector* qeVector = pmtMPT->GetProperty("EFFICIENCY");
  if (!qeVector) return false;

  if (G4UniformRand() > qeVector->Value(track->GetKineticEnergy())) {
    track->SetTrackStatus(fStopAndKill);
    return false;
  }
  
  // --- [리팩토링] 세그먼트 ID와 PMT ID를 정확히 추출 ---
  auto touchable = aStep->GetPreStepPoint()->GetTouchable();
  G4int pmtID = touchable->GetCopyNumber(0);      // Level 0: Photocathode의 copy number (0 또는 1)
  G4int segmentID = touchable->GetCopyNumber(1);  // Level 1: PhysSegment의 copy number (0 ~ 153)

  PMTHit* newHit = new PMTHit();
  newHit->SetSegmentID(segmentID);
  newHit->SetPMTID(pmtID);
  newHit->SetTime(aStep->GetPostStepPoint()->GetGlobalTime() / ns);
  
  fHitsCollection->insert(newHit);
  track->SetTrackStatus(fStopAndKill);

  return true;
}
