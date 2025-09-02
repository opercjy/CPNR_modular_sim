#include "EventAction.hh"
#include "G4AnalysisManager.hh"
#include "G4Event.hh"
#include "G4HCofThisEvent.hh"
#include "G4SDManager.hh"
#include "G4SystemOfUnits.hh"

// 데이터 저장을 위해 Hit 클래스 헤더들을 포함합니다.
#include "LSHit.hh"
#include "PMTHit.hh"

/**
 * @brief 생성자
 */
EventAction::EventAction() : G4UserEventAction() {}

/**
 * @brief 소멸자
 */
EventAction::~EventAction() {}

/**
 * @brief 각 이벤트가 끝날 때마다 호출되는 함수입니다.
 * @param event 현재 이벤트에 대한 정보를 담고 있는 G4Event 객체 포인터
 *
 * 이 함수는 LSSD와 PMTSD에서 수집된 HitsCollection을 분석하여,
 * 1) 상세 에너지 증착 정보를 'Hits' TTree에 저장하고,
 * 2) PMT에서 검출된 광자 정보를 'PMTHits' TTree에 저장하는 역할을 수행합니다.
 */
void EventAction::EndOfEventAction(const G4Event* event)
{
  auto analysisManager = G4AnalysisManager::Instance();
  G4int eventID = event->GetEventID();

  // --- LS 데이터 처리 (LSHitsCollection) ---
  G4int lsHcID = G4SDManager::GetSDMpointer()->GetCollectionID("LSHitsCollection");
  if (lsHcID >= 0) {
    auto lsHitsCollection = static_cast<LSHitsCollection*>(event->GetHCofThisEvent()->GetHC(lsHcID));
    if (lsHitsCollection && lsHitsCollection->entries() > 0) {
      // Hits TTree (Ntuple ID=0)에 상세 정보 저장
      for (size_t i = 0; i < lsHitsCollection->entries(); ++i) {
        auto hit = (*lsHitsCollection)[i];
        analysisManager->FillNtupleIColumn(0, 0, eventID);
        analysisManager->FillNtupleIColumn(0, 1, hit->GetTrackID());
        analysisManager->FillNtupleIColumn(0, 2, hit->GetParentID());
        analysisManager->FillNtupleSColumn(0, 3, hit->GetParticleName());
        analysisManager->FillNtupleSColumn(0, 4, hit->GetProcessName());
        analysisManager->FillNtupleSColumn(0, 5, hit->GetVolumeName());
        analysisManager->FillNtupleDColumn(0, 6, hit->GetPosition().x() / mm);
        analysisManager->FillNtupleDColumn(0, 7, hit->GetPosition().y() / mm);
        analysisManager->FillNtupleDColumn(0, 8, hit->GetPosition().z() / mm);
        analysisManager->FillNtupleDColumn(0, 9, hit->GetTime() / ns);
        analysisManager->FillNtupleDColumn(0, 10, hit->GetKineticEnergy() / MeV);
        analysisManager->FillNtupleDColumn(0, 11, hit->GetEnergyDeposit() / MeV);
        analysisManager->AddNtupleRow(0);
      }
    }
  }
  
  // --- PMT 데이터 처리 (PMTHitsCollection) ---
  G4int pmtHcID = G4SDManager::GetSDMpointer()->GetCollectionID("PMTHitsCollection");
  if (pmtHcID >= 0) {
    auto pmtHitsCollection = static_cast<PMTHitsCollection*>(event->GetHCofThisEvent()->GetHC(pmtHcID));
    if (pmtHitsCollection && pmtHitsCollection->entries() > 0) {
      // PMTHits TTree (Ntuple ID=1)에 저장
      for (size_t i = 0; i < pmtHitsCollection->entries(); ++i) {
        auto pmtHit = (*pmtHitsCollection)[i];
        
        analysisManager->FillNtupleIColumn(1, 0, eventID);
        analysisManager->FillNtupleIColumn(1, 1, pmtHit->GetSegmentID());
        analysisManager->FillNtupleIColumn(1, 2, pmtHit->GetPMTID());
        analysisManager->FillNtupleDColumn(1, 3, pmtHit->GetTime());
        analysisManager->AddNtupleRow(1);
      }
    }
  }
}
