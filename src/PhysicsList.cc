#include "PhysicsList.hh"

#include "G4DecayPhysics.hh"
#include "G4RadioactiveDecayPhysics.hh"
#include "G4EmStandardPhysics.hh"
#include "G4HadronPhysicsFTFP_BERT_HP.hh"
#include "G4HadronElasticPhysicsHP.hh"
#include "G4IonPhysics.hh"
#include "G4StoppingPhysics.hh"
#include "G4OpticalPhysics.hh"
#include "G4SystemOfUnits.hh"

PhysicsList::PhysicsList() : G4VModularPhysicsList()
{
  // 1. 표준 전자기 물리
  RegisterPhysics(new G4EmStandardPhysics());

  // 2. 붕괴 물리 (일반 및 방사성)
  RegisterPhysics(new G4DecayPhysics());
  RegisterPhysics(new G4RadioactiveDecayPhysics());

  // 3. 강입자, 이온, 정지 물리 (HP 옵션으로 정밀한 중성자 추적 포함)
  RegisterPhysics(new G4HadronPhysicsFTFP_BERT_HP());
  RegisterPhysics(new G4HadronElasticPhysicsHP());
  RegisterPhysics(new G4IonPhysics());
  RegisterPhysics(new G4StoppingPhysics());
  
  // 4. 광학 물리 (섬광, 체렌코프, 반사, 굴절 등)
  RegisterPhysics(new G4OpticalPhysics());
  
  SetDefaultCutValue(1.0*mm);
}

PhysicsList::~PhysicsList() {}
