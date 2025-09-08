#include "MyHadronPhysics.hh"

// Geant4 필수 헤더
#include "G4ParticleDefinition.hh"
#include "G4ProcessManager.hh"
#include "G4Neutron.hh"
#include "G4HadronPhysicsFTFP_BERT_HP.hh"
#include "G4SystemOfUnits.hh"

// --- 우리가 직접 등록할 모든 프로세스와 모델 헤더 ---
// 탄성 산란 (Elastic)
#include "G4HadronElasticProcess.hh"
#include "G4NeutronHPElastic.hh"
#include "G4NeutronHPElasticData.hh"

// 비탄성 산란 (Inelastic)
#include "G4HadronInelasticProcess.hh"
#include "G4NeutronHPInelastic.hh"
#include "G4NeutronHPInelasticData.hh"

// 핵분열 (Fission)
#include "G4NeutronFissionProcess.hh"
#include "G4NeutronHPFission.hh"
#include "G4NeutronHPFissionData.hh"

// 포획 (Capture)
#include "G4NeutronCaptureProcess.hh"
#include "GdNeutronHPCapture.hh" // 우리의 커스텀 모델

MyHadronPhysics::MyHadronPhysics(G4int verbose)
  : G4VPhysicsConstructor("MyHadronPhysics")
{
    fStdPhysics = std::make_unique<G4HadronPhysicsFTFP_BERT_HP>(verbose);
    G4cout << "<<< Hadronic Physics Constructor: MyHadronPhysics (Fully Explicit Assembly)" << G4endl;
}

MyHadronPhysics::~MyHadronPhysics()
{}

void MyHadronPhysics::ConstructParticle()
{
    fStdPhysics->ConstructParticle();
}

void MyHadronPhysics::ConstructProcess()
{
    // 1. 중성자를 제외한 다른 모든 강입자에 대해 표준 물리를 먼저 적용합니다.
    fStdPhysics->ConstructProcess();

    G4cout << "\nMyHadronPhysics: Starting fully explicit neutron physics construction..." << G4endl;
    G4ProcessManager* pManager = G4Neutron::Neutron()->GetProcessManager();

    // 2. 표준 물리 리스트가 생성했을 수 있는 모든 강입자 프로세스를 깨끗하게 제거합니다.
    G4cout << "MyHadronPhysics: Removing any pre-existing standard neutron processes..." << G4endl;
    
    // --- [수정] 프로세스를 이름으로 찾아서 포인터를 얻은 후, 그 포인터로 제거합니다 ---
    G4VProcess* process = nullptr;
    process = pManager->GetProcess("hadElastic");       if(process) pManager->RemoveProcess(process);
    process = pManager->GetProcess("neutronInelastic"); if(process) pManager->RemoveProcess(process);
    process = pManager->GetProcess("nCapture");         if(process) pManager->RemoveProcess(process);
    process = pManager->GetProcess("nFission");         if(process) pManager->RemoveProcess(process);
    // ------------------------------------------------------------------------------------

    // --- 이제부터 우리가 원하는 프로세스를 하나씩 직접 조립합니다 ---

    // 3. 탄성 산란 (Elastic Scattering) 프로세스 추가
    auto elasticProcess = new G4HadronElasticProcess();
    elasticProcess->AddDataSet(new G4NeutronHPElasticData());
    elasticProcess->RegisterMe(new G4NeutronHPElastic());
    pManager->AddDiscreteProcess(elasticProcess);
    G4cout << "MyHadronPhysics: G4NeutronHPElastic registered." << G4endl;

    // 4. 비탄성 산란 (Inelastic Scattering) 프로세스 추가
    auto inelasticProcess = new G4HadronInelasticProcess("neutronInelastic");
    inelasticProcess->AddDataSet(new G4NeutronHPInelasticData());
    inelasticProcess->RegisterMe(new G4NeutronHPInelastic());
    pManager->AddDiscreteProcess(inelasticProcess);
    G4cout << "MyHadronPhysics: G4NeutronHPInelastic registered." << G4endl;

    // 5. 핵분열 (Fission) 프로세스 추가
    auto fissionProcess = new G4NeutronFissionProcess();
    fissionProcess->AddDataSet(new G4NeutronHPFissionData());
    fissionProcess->RegisterMe(new G4NeutronHPFission());
    pManager->AddDiscreteProcess(fissionProcess);
    G4cout << "MyHadronPhysics: G4NeutronHPFission registered." << G4endl;

    // 6. 포획 (Capture) 프로세스 추가 (우리의 커스텀 모델 사용)
    auto captureProcess = new G4NeutronCaptureProcess();
    captureProcess->RegisterMe(new GdNeutronHPCapture());
    pManager->AddDiscreteProcess(captureProcess);
    G4cout << "MyHadronPhysics: Custom GdNeutronHPCapture registered for nCapture process." << G4endl;
    
    G4cout << "MyHadronPhysics: Custom neutron physics construction finished.\n" << G4endl;
}
