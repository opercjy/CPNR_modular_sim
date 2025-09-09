#include "RunAction.hh"
#include "G4AnalysisManager.hh"
#include "G4Run.hh"
#include "G4Threading.hh" // G4Threading::IsMultithreadedApplication() 사용

RunAction::RunAction() : G4UserRunAction()
{
  auto analysisManager = G4AnalysisManager::Instance();
  analysisManager->SetVerboseLevel(1);
  if (G4Threading::IsMultithreadedApplication()) {
    analysisManager->SetNtupleMerging(true);
  }

  // Ntuple ID=0: Hits (에너지 증착 상세 정보)
  analysisManager->CreateNtuple("Hits", "Hit-by-hit energy deposition data");
  analysisManager->CreateNtupleIColumn("eventID");        // col 0
  analysisManager->CreateNtupleIColumn("trackID");        // col 1
  analysisManager->CreateNtupleIColumn("parentID");       // col 2
  analysisManager->CreateNtupleSColumn("particleName");   // col 3
  analysisManager->CreateNtupleSColumn("processName");    // col 4
  analysisManager->CreateNtupleSColumn("volumeName");     // col 5
  analysisManager->CreateNtupleDColumn("x_mm");           // col 6
  analysisManager->CreateNtupleDColumn("y_mm");           // col 7
  analysisManager->CreateNtupleDColumn("z_mm");           // col 8
  analysisManager->CreateNtupleDColumn("time_ns");        // col 9
  analysisManager->CreateNtupleDColumn("kineticEnergy_MeV"); // col 10
  analysisManager->CreateNtupleDColumn("energyDeposit_MeV"); // col 11
  
  // --- [수정] 새로운 컬럼 추가 ---
  analysisManager->CreateNtupleIColumn("pdgID");          // col 12
  analysisManager->CreateNtupleDColumn("px_MeV");         // col 13
  analysisManager->CreateNtupleDColumn("py_MeV");         // col 14
  analysisManager->CreateNtupleDColumn("pz_MeV");         // col 15
  analysisManager->CreateNtupleDColumn("energy_MeV");     // col 16
  // ---------------------------------
  
  analysisManager->FinishNtuple();

  // Ntuple ID=1: PMTHits (개별 광자 검출 정보)
  analysisManager->CreateNtuple("PMTHits", "Individual photon hits in PMTs");
  analysisManager->CreateNtupleIColumn("eventID");
  analysisManager->CreateNtupleIColumn("segmentID");
  analysisManager->CreateNtupleIColumn("pmtID");
  analysisManager->CreateNtupleDColumn("time_ns");
  analysisManager->FinishNtuple();
}

RunAction::~RunAction() {}

void RunAction::BeginOfRunAction(const G4Run* run)
{
  auto analysisManager = G4AnalysisManager::Instance();
  analysisManager->OpenFile("cpnr_modular_sim.root");
  G4cout << "### Run " << run->GetRunID() << " start." << G4endl;
}

void RunAction::EndOfRunAction(const G4Run* /*run*/)
{
  auto analysisManager = G4AnalysisManager::Instance();
  analysisManager->Write();
  analysisManager->CloseFile();
}
