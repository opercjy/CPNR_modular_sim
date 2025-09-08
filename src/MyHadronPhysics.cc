#include "MyHadronPhysics.hh"
#include "G4ParticleDefinition.hh"
#include "G4ProcessManager.hh"
#include "G4Neutron.hh"
#include "G4HadronPhysicsFTFP_BERT_HP.hh"

// 필요한 프로세스 및 모델 헤더
#include "G4NeutronCaptureProcess.hh"
#include "GdNeutronHPCapture.hh"
#include "G4SystemOfUnits.hh"


MyHadronPhysics::MyHadronPhysics(G4int verbose)
  : G4VPhysicsConstructor("MyHadronPhysics")
{
    fStdPhysics = std::make_unique<G4HadronPhysicsFTFP_BERT_HP>(verbose);
    G4cout << "<<< Hadronic Physics Constructor: MyHadronPhysics (Process Re-creation Pattern)" << G4endl;
}

MyHadronPhysics::~MyHadronPhysics()
{}

void MyHadronPhysics::ConstructParticle()
{
    fStdPhysics->ConstructParticle();
}

void MyHadronPhysics::ConstructProcess()
{
    // 1. 표준 물리 리스트가 모든 입자에 대한 물리 프로세스를 생성하도록 합니다.
    fStdPhysics->ConstructProcess();

    G4cout << "\nMyHadronPhysics: Starting custom neutron capture physics construction..." << G4endl;

    G4ProcessManager* pManager = G4Neutron::Neutron()->GetProcessManager();

    // 2. 표준 물리 리스트가 생성한 기존 'nCapture' 프로세스를 찾습니다.
    G4VProcess* process = pManager->GetProcess("nCapture");
    if (process) {
        // 3. 찾았다면, 프로세스 목록에서 완전히 제거합니다.
        pManager->RemoveProcess(process);
        G4cout << "MyHadronPhysics: Standard 'nCapture' process has been removed." << G4endl;
    }

    // 4. 우리가 원하는 모델만 포함하는 깨끗한 'nCapture' 프로세스를 새로 생성합니다.
    G4NeutronCaptureProcess* captureProcess = new G4NeutronCaptureProcess();
    
    // 5. 우리의 커스텀 모델 'GdNeutronHPCapture'를 등록합니다.
    //    이 모델은 Gd일때는 ANNRI-Gd 로직을, 다른 원소일때는 표준 G4NeutronHPCapture 로직을 수행합니다.
    captureProcess->RegisterMe(new GdNeutronHPCapture());
    
    // 6. 새로 만든 프로세스를 중성자의 프로세스 목록에 추가합니다.
    pManager->AddDiscreteProcess(captureProcess);
    G4cout << "MyHadronPhysics: Custom 'nCapture' process with GdNeutronHPCapture has been registered." << G4endl;
    G4cout << "MyHadronPhysics: Custom neutron physics construction finished.\n" << G4endl;
}
