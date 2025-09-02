#include "EventAction.hh"
#include "G4AnalysisManager.hh"
#include "G4Event.hh"
#include "G4HCofThisEvent.hh"
#include "G4SDManager.hh"
#include "LSHit.hh"
#include "PMTHit.hh"

// std::set을 사용하여 중복된 트랙을 효율적으로 제거하기 위해 헤더를 포함합니다.
#include <set>

EventAction::EventAction() : G4UserEventAction() {}
EventAction::~EventAction() {}

void EventAction::EndOfEventAction(const G4Event* event)
{
  auto analysisManager = G4AnalysisManager::Instance();
  G4int eventID = event->GetEventID();

  // --- LS 데이터 처리 (LSHitsCollection) ---
  G4int lsHcID = G4SDManager::GetSDMpointer()->GetCollectionID("LSHitsCollection");
  if (lsHcID >= 0) {
    auto lsHitsCollection = static_cast<LSHitsCollection*>(event->GetHCofThisEvent()->GetHC(lsHcID));
    // ... (기존과 동일한 로직으로 Hits TTree 채우기) ...
  }
  
  // --- PMT 데이터 처리 (PMTHitsCollection) ---
  G4int pmtHcID = G4SDManager::GetSDMpointer()->GetCollectionID("PMTHitsCollection");
  if (pmtHcID >= 0) {
    auto pmtHitsCollection = static_cast<PMTHitsCollection*>(event->GetHCofThisEvent()->GetHC(pmtHcID));
    if (pmtHitsCollection) {
      for (size_t i = 0; i < pmtHitsCollection->entries(); ++i) {
        auto pmtHit = (*pmtHitsCollection)[i];
        
        // --- [리팩토링] PMTHits TTree 채우기 (Ntuple ID=1) ---
        analysisManager->FillNtupleIColumn(1, 0, eventID);
        analysisManager->FillNtupleIColumn(1, 1, pmtHit->GetSegmentID()); // 추가된 segmentID 저장
        analysisManager->FillNtupleIColumn(1, 2, pmtHit->GetPMTID());
        analysisManager->FillNtupleDColumn(1, 3, pmtHit->GetTime());
        analysisManager->AddNtupleRow(1);
      }
    }
  }
}
