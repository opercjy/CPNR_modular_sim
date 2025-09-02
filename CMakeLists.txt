# ============================================================================
# CPNR_modular_sim Geant4 프로젝트용 CMakeLists.txt
# ============================================================================

# 최소 CMake 버전 요구사항 지정
cmake_minimum_required(VERSION 3.16)

# 프로젝트 이름 설정 (실행 파일 이름으로 사용됨)
project(CPNR_modular_sim)

# --- 패키지 검색 ---
# Geant4와 ROOT 라이브러리를 찾고, 필요한 컴포넌트를 지정합니다.
find_package(Geant4 REQUIRED ui_all vis_all)
find_package(ROOT REQUIRED COMPONENTS Core Graf Tree)

# --- Geant4 및 프로젝트 헤더 파일 경로 설정 ---
# Geant4의 헤더 파일 및 라이브러리 설정을 현재 프로젝트에 포함시킵니다.
include(${Geant4_USE_FILE})

# 우리 프로젝트의 헤더 파일이 있는 'include' 디렉토리를 경로에 추가합니다.
include_directories(${PROJECT_SOURCE_DIR}/include)

# --- 소스 파일 목록 정의 ---
set(PROJECT_SOURCES
    ${PROJECT_SOURCE_DIR}/src/ActionInitialization.cc
    ${PROJECT_SOURCE_DIR}/src/DetectorConstruction.cc
    ${PROJECT_SOURCE_DIR}/src/EventAction.cc
    ${PROJECT_SOURCE_DIR}/src/LSHit.cc
    ${PROJECT_SOURCE_DIR}/src/LSSD.cc
    ${PROJECT_SOURCE_DIR}/src/PMTHit.cc
    ${PROJECT_SOURCE_DIR}/src/PMTSD.cc
    ${PROJECT_SOURCE_DIR}/src/PhysicsList.cc
    ${PROJECT_SOURCE_DIR}/src/PrimaryGeneratorAction.cc
    ${PROJECT_SOURCE_DIR}/src/RunAction.cc
    ${PROJECT_SOURCE_DIR}/src/SteppingAction.cc
    ${PROJECT_SOURCE_DIR}/src/TrackingAction.cc
)

# --- 실행 파일 생성 및 라이브러리 연결 ---
add_executable(${PROJECT_NAME} ${PROJECT_NAME}.cc ${PROJECT_SOURCES})
target_link_libraries(${PROJECT_NAME} PRIVATE ${Geant4_LIBRARIES} ${ROOT_LIBRARIES})

# --- [수정된 부분] 매크로 파일 복사 ---
# 시뮬레이션 실행에 필요한 매크로(.mac) 파일들을
# 소스 디렉토리에서 빌드 디렉토리로 자동으로 복사합니다.
set(MACRO_FILES
  vis.mac
  run.mac
)

foreach(macro ${MACRO_FILES})
  configure_file(
    ${PROJECT_SOURCE_DIR}/${macro}
    ${PROJECT_BINARY_DIR}/${macro}
    COPYONLY
  )
endforeach()


# --- 설치 (선택 사항) ---
install(TARGETS ${PROJECT_NAME} RUNTIME DESTINATION bin)
