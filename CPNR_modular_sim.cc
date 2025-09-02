// CPNR_modular_sim.cc
// 시뮬레이션의 시작점(entry point)이며, 전체 실행 흐름을 제어합니다.

#include "G4RunManagerFactory.hh"
#include "G4UImanager.hh"
#include "G4VisExecutive.hh"
#include "G4UIExecutive.hh"
#include "G4Threading.hh"
#include "G4ScoringManager.hh"

#include "DetectorConstruction.hh"
#include "MyShieldingPhysList.hh" 
#include "ActionInitialization.hh"

int main(int argc, char** argv)
{
  // UI 세션 감지 (터미널 인자가 없으면 GUI 모드로 판단)
  G4UIExecutive* ui = nullptr;
  if (argc == 1) {
    ui = new G4UIExecutive(argc, argv);
  }

  // 실행 모드에 따라 적합한 RunManager 생성
  auto* runManager = G4RunManagerFactory::CreateRunManager(
      (ui) ? G4RunManagerType::SerialOnly : G4RunManagerType::Default
  );
  if (!ui) {
    runManager->SetNumberOfThreads(G4Threading::G4GetNumberOfCores());
  }

  // 스코어링 매니저 활성화
  G4ScoringManager::GetScoringManager();

  // 시각화 관리자 생성 및 초기화
  G4VisManager* visManager = new G4VisExecutive;
  visManager->Initialize();

  // 필수 사용자 클래스들을 RunManager에 등록
  runManager->SetUserInitialization(new DetectorConstruction());
  runManager->SetUserInitialization(new MyShieldingPhysList()); 
  runManager->SetUserInitialization(new ActionInitialization());
  
  // Geant4 커널 초기화
  runManager->Initialize();

  // UI 관리자 포인터 가져오기
  G4UImanager* UImanager = G4UImanager::GetUIpointer();

  // 실행 모드에 따라 적절한 매크로 실행
  if (ui) {
    // ## 인터랙티브(GUI) 모드 ##
    UImanager->ApplyCommand("/control/execute vis.mac"); // GUI용 기본 매크로
    ui->SessionStart();
    delete ui;
  }
  else {
    // ## 배치 모드 ##
    G4String command = "/control/execute ";
    G4String fileName = argv[1];
    UImanager->ApplyCommand(command + fileName);
  }

  // 프로그램 종료 전 메모리 해제
  delete visManager;
  delete runManager;

  return 0;
}
