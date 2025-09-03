#include "MyHadronPhysics.hh"
#include "GdNeutronHPCapture.hh" // 우리가 만든 싱글턴 모델

// Geant4 헤더 파일
#include "G4ParticleDefinition.hh"
#include "G4ProcessManager.hh"
#include "G4Neutron.hh"
#include "G4NeutronCaptureProcess.hh" // [수정] G4HadronCaptureProcess.hh -> G4NeutronCaptureProcess.hh
#include "G4ParticleHPCapture.hh"   // 표준 HP 캡처 모델
#include "G4HadronPhysicsFTFP_BERT_HP.hh"
#include "G4PhysListUtil.hh"

MyHadronPhysics::MyHadronPhysics(G4int verbose)
  : G4VPhysicsConstructor("MyHadronPhysics"), fVerbose(verbose)
{}

MyHadronPhysics::~MyHadronPhysics()
{}

void MyHadronPhysics::ConstructParticle()
{
    // 표준 강입자 물리 리스트가 생성하는 모든 입자를 그대로 생성합니다.
    G4HadronPhysicsFTFP_BERT_HP basePhysics(fVerbose);
    basePhysics.ConstructParticle();
}

void MyHadronPhysics::ConstructProcess()
{
    // 1. 기반이 될 표준 강입자 물리(FTFP_BERT_HP)를 먼저 생성합니다.
    G4HadronPhysicsFTFP_BERT_HP basePhysics(fVerbose);
    basePhysics.ConstructProcess();

    // 2. 중성자의 프로세스 매니저를 가져옵니다.
    G4ProcessManager* pManager = G4Neutron::Neutron()->GetProcessManager();

    // 3. 중성자의 여러 프로세스 중, 'nCapture'라는 이름의 포획 프로세스를 찾습니다.
    // [수정] G4HadronCaptureProcess* -> G4NeutronCaptureProcess* 로 타입 변경
    G4NeutronCaptureProcess* captureProcess = nullptr;
    G4VProcess* process = pManager->GetProcess("nCapture");
    if (process) {
        captureProcess = static_cast<G4NeutronCaptureProcess*>(process);
    } else {
        captureProcess = new G4NeutronCaptureProcess();
        pManager->AddDiscreteProcess(captureProcess);
    }
    
    // 4. [핵심] 포획 프로세스에 우리 커스텀 모델과 표준 모델을 함께 등록합니다.
    if (captureProcess) {
        if (fVerbose > 0) {
            G4cout << "MyHadronPhysics: Registering ANNRI-Gd model to nCapture process." << G4endl;
        }
        // 표준 고정밀(HP) 캡처 모델도 등록합니다.
        captureProcess->RegisterMe(new G4ParticleHPCapture());
        
        // GdNeutronHPCapture 싱글턴 인스턴스를 가져와 등록합니다.
        captureProcess->RegisterMe(GdNeutronHPCapture::GetInstance());
    }
}
