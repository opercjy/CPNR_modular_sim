#include "LSSD.hh"
#include "G4HCofThisEvent.hh"
#include "G4Step.hh"
#include "G4Track.hh"
#include "G4VProcess.hh"
#include "G4SystemOfUnits.hh"
#include "G4SDManager.hh"

LSSD::LSSD(const G4String& name)
: G4VSensitiveDetector(name), fHitsCollection(nullptr)
{
  collectionName.insert("LSHitsCollection");
}

LSSD::~LSSD()
{}

void LSSD::Initialize(G4HCofThisEvent* hce)
{
  fHitsCollection = new LSHitsCollection(SensitiveDetectorName, collectionName[0]);
  G4int hcID = GetCollectionID(0);
  hce->AddHitsCollection(hcID, fHitsCollection);
}

G4bool LSSD::ProcessHits(G4Step* aStep, G4TouchableHistory* /*ROhist*/)
{
  if (aStep->GetTotalEnergyDeposit() == 0.) return false;

  LSHit* newHit = new LSHit();
  G4Track* track = aStep->GetTrack();
  auto preStepPoint = aStep->GetPreStepPoint();

  // --- 기본 정보 저장 ---
  newHit->SetTrackID(track->GetTrackID());
  newHit->SetParentID(track->GetParentID());
  newHit->SetParticleName(track->GetDefinition()->GetParticleName());
  newHit->SetVolumeName(track->GetVolume()->GetLogicalVolume()->GetName());

  const G4VProcess* creatorProcess = track->GetCreatorProcess();
  if (creatorProcess) {
    newHit->SetProcessName(creatorProcess->GetProcessName());
  } else {
    newHit->SetProcessName("primary");
  }

  newHit->SetPosition(preStepPoint->GetPosition());
  newHit->SetTime(preStepPoint->GetGlobalTime());
  newHit->SetKineticEnergy(preStepPoint->GetKineticEnergy());
  newHit->SetEnergyDeposit(aStep->GetTotalEnergyDeposit());

  // --- [추가] 확장된 물리 정보 저장 ---
  newHit->SetPDGID(track->GetDefinition()->GetPDGEncoding());
  newHit->SetMomentum(preStepPoint->GetMomentum());
  newHit->SetEnergy(preStepPoint->GetTotalEnergy());
  // ------------------------------------

  fHitsCollection->insert(newHit);

  return true;
}
