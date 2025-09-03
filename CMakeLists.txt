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

# [수정] ROOT 헤더 파일 경로를 추가합니다.
include_directories(${ROOT_INCLUDE_DIRS})

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
    ${PROJECT_SOURCE_DIR}/src/PrimaryGeneratorAction.cc
    ${PROJECT_SOURCE_DIR}/src/RunAction.cc
    ${PROJECT_SOURCE_DIR}/src/SteppingAction.cc
    ${PROJECT_SOURCE_DIR}/src/TrackingAction.cc
    
    # [추가] ANNRI-Gd 연동 클래스들
    src/GdNeutronHPCapture.cc
    src/GdNeutronHPCaptureFS.cc

    # [추가] ANNRI-Gd 라이브러리 소스 코드 
    src/ANNRIGd_156GdContinuumModelV2.cc
    src/ANNRIGd_156GdDiscreteModel.cc
    src/ANNRIGd_158GdContinuumModelV2.cc
    src/ANNRIGd_158GdDiscreteModel.cc
    src/ANNRIGd_Auxiliary.cc
    src/ANNRIGd_DummyModel.cc
    src/ANNRIGd_GdNCaptureGammaGenerator.cc
    src/ANNRIGd_GeneratorConfigurator.cc
    src/ANNRIGd_Model.cc
    src/ANNRIGd_ModelType.cc
    src/ANNRIGd_OutputConverter.cc
    src/ANNRIGd_ReactionProduct.cc

    # [추가] 새로운 물리 리스트 관련 클래스들
    src/MyHadronPhysics.cc
    src/MyShieldingPhysList.cc
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
