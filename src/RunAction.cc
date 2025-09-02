#include "RunAction.hh"
#include "G4AnalysisManager.hh"
#include "G4Run.hh"

RunAction::RunAction() : G4UserRunAction()
{
  auto analysisManager = G4AnalysisManager::Instance();
  analysisManager->SetVerboseLevel(1);
  analysisManager->SetNtupleMerging(true);

  // Ntuple ID=0: Hits (에너지 증착 상세 정보)
  analysisManager->CreateNtuple("Hits", "Hit-by-hit energy deposition data");
  analysisManager->CreateNtupleIColumn("eventID");
  analysisManager->CreateNtupleIColumn("trackID");
  analysisManager->CreateNtupleIColumn("parentID");
  analysisManager->CreateNtupleSColumn("particleName");
  analysisManager->CreateNtupleSColumn("processName");
  analysisManager->CreateNtupleSColumn("volumeName"); // "LogicLS_inner" 또는 "LogicLS_outer"가 저장됨
  analysisManager->CreateNtupleDColumn("x_mm");
  analysisManager->CreateNtupleDColumn("y_mm");
  analysisManager->CreateNtupleDColumn("z_mm");
  analysisManager->CreateNtupleDColumn("time_ns");
  analysisManager->CreateNtupleDColumn("kineticEnergy_MeV");
  analysisManager->CreateNtupleDColumn("energyDeposit_MeV");
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
