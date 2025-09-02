#include "MyShieldingPhysList.hh"
#include "MyHadronPhysics.hh" // 우리가 만든 커스텀 강입자 물리

// Geant4 표준 물리 리스트 부품들
#include "G4DecayPhysics.hh"
#include "G4RadioactiveDecayPhysics.hh"
#include "G4EmStandardPhysics_option4.hh" // 정밀한 EM 물리 모델
#include "G4OpticalPhysics.hh"
#include "G4SystemOfUnits.hh"

MyShieldingPhysList::MyShieldingPhysList(G4int verbose)
{
    G4cout << "<<< Geant4 Physics List: MyShieldingPhysList" << G4endl;
    SetVerboseLevel(verbose);
    defaultCutValue = 0.7 * mm;

    // 1. 붕괴 물리 (Decay Physics)
    RegisterPhysics(new G4DecayPhysics(verbose));

    // 2. 방사성 붕괴 물리 (Radioactive Decay Physics)
    RegisterPhysics(new G4RadioactiveDecayPhysics(verbose));

    // 3. 전자기 물리 (Electromagnetic Physics) - Option 4는 정밀도가 높은 모델
    RegisterPhysics(new G4EmStandardPhysics_option4(verbose));

    // 4. 광학 물리 (Optical Physics) - 섬광, 체렌코프 등
    G4OpticalPhysics* opticalPhysics = new G4OpticalPhysics(verbose);
    RegisterPhysics(opticalPhysics);

    // 5. 강입자 물리 (Hadronic Physics) - ANNRI-Gd 모델이 포함된 우리 커스텀 모듈
    RegisterPhysics(new MyHadronPhysics(verbose));
}

MyShieldingPhysList::~MyShieldingPhysList()
{}

void MyShieldingPhysList::ConstructParticle()
{
    // 각 물리 부품들이 필요로 하는 입자들을 생성합니다.
    G4VModularPhysicsList::ConstructParticle();
}

void MyShieldingPhysList::ConstructProcess()
{
    // 각 물리 부품들이 입자에 맞는 물리 프로세스를 등록합니다.
    G4VModularPhysicsList::ConstructProcess();
}
