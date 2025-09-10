# Geant4 기반 모듈형 검출기 시뮬레이션: CPNR_modular_sim
<img width="1320" height="1020" alt="image" src="https://github.com/user-attachments/assets/7f61aa08-f10d-4450-8f7b-e7df066cb323" />
<img width="1320" height="1020" alt="image" src="https://github.com/user-attachments/assets/d7cd7aa6-f2f3-4658-b8bc-0d77ea2ce772" />
<img width="1320" height="1020" alt="image" src="https://github.com/user-attachments/assets/e2bc1e0a-fe01-486b-baad-627cb0706cd6" />

-----

### README.md (최신 버전)

# Geant4 기반 모듈형 검출기 시뮬레이션: CPNR\_modular\_sim

*최종 수정: 2025년 9월 10일*

## 1\. 프로젝트 개요

본 프로젝트는 **3x3 모듈형 섬광체 검출기 배열**을 시뮬레이션하기 위한 Geant4 애플리케이션입니다. 각 검출기 모듈(세그먼트)은 물리적으로 안정적인 구조를 갖도록 설계되었습니다. 내부적으로는 **가돌리늄(Gadolinium)이 첨가된 액체섬광체(Gd-LS)를 담는 PMMA 용기**가 있으며, 이 용기는 **PMMA 지지 기둥(Pillar)** 에 의해 **일반 액체섬광체(LS)를 담는 외부 PMMA 용기** 내부에 고정됩니다.

시뮬레이션의 핵심은 표준 Geant4 물리 모델을 확장하여, 가돌리늄의 열중성자 포획 반응에 대해 정밀한 감마선 캐스케이드 데이터를 생성하는 **ANNRI-Gd 모델**을 선택적으로 적용하는 것입니다. 이를 통해 보다 현실에 가까운 검출기 응답을 연구할 수 있습니다.

### 주요 특징

  * **모듈형 검출기 설계**: **3x3** 배열의 개별 세그먼트로 구성되며, `DetectorConstruction.hh`에서 쉽게 크기 조절이 가능합니다.
  * **견고한 기하구조**: 물리적으로 현실적인 "용기 속 용기(container-in-container)" 구조를 채택했습니다. 내부 Gd-LS 용기는 **5개의 PMMA 지지 기둥**에 의해 외부 LS 용기 내에 안정적으로 고정됩니다. 모든 구조는 파라미터 기반으로 설계되어 중첩 오류 없이 쉽게 수정할 수 있습니다.
  * **사실적인 PMT 모델**: `G4Polycone`을 사용하여 **평평한 입사창과 완만한 곡면을 가진 몸체**를 구현하여 실제 PMT와 유사한 형태를 갖추었습니다.
  * **커스텀 물리 모델**: 표준 물리 리스트를 기반으로, 가돌리늄(Z=64)의 중성자 포획 반응에만 **ANNRI-Gd 모델**을 동적으로 적용하는 사용자 정의 물리 리스트를 사용합니다.
  * **유연한 제어**: Geant4 메신저를 통해 매크로 파일에서 ANNRI-Gd 모델의 상세 옵션을 C++ 코드 수정 없이 제어할 수 있습니다.
  * **상세한 데이터 출력**: `G4AnalysisManager`를 통해 각 상호작용(Hit) 정보를 ROOT 파일 형식으로 저장합니다. 특히 Hit 정보에는 **4-운동량 ($P\_x, P\_y, P\_z, E$)과 입자 식별 코드(PDG ID)가 포함**되어 상세한 물리 분석을 지원합니다.

-----

## 2\. 사전 준비

### 2.1. ANNRI-Gd 데이터 파일

이 시뮬레이션은 ANNRI-Gd의 연속 스펙트럼 모델을 위해 외부 ROOT 데이터 파일이 필요합니다.

  * `156GdContTbl_E1SLO4_HFB.root`
  * `158GdContTbl_E1SLO4_HFB.root`

이 파일들을 저장할 디렉토리를 생성하고, 해당 디렉토리의 경로를 환경 변수로 설정해야 합니다.

### 2.2. 환경 변수 설정

터미널에서 아래와 같이 `GD_CAPTURE_DATA_DIR` 환경 변수를 설정하십시오. 이 변수는 `GdNeutronHPCapture` 코드가 데이터 파일을 찾는 데 사용됩니다.

```bash
# 예시: /home/user/geant4_data/annri_gd 디렉토리에 데이터 파일을 저장한 경우
export GD_CAPTURE_DATA_DIR=/home/user/geant4_data/annri_gd
```

`.bashrc` 또는 `.zshrc` 파일에 이 라인을 추가하면 터미널을 열 때마다 자동으로 설정됩니다.

-----

## 3\. 빌드 및 실행

### 3.1. 빌드

프로젝트 최상위 디렉토리에서 다음 명령어를 실행하십시오.

```bash
mkdir build && cd build
cmake ..
make -j$(nproc)
```

### 3.2. 실행

모든 실행은 `build` 디렉토리 안에서 이루어집니다.

  * **GUI 모드 (시각화 및 디버깅)**
    ```bash
    ./CPNR_modular_sim
    ```
  * **배치 모드 (데이터 생성)**
    ```bash
    ./CPNR_modular_sim run.mac
    ```

-----

## 4\. 고급 사용법

### 4.1. 물리 모델 제어

`run.mac`과 같은 매크로 파일 안에서 Geant4 메신저 명령어를 사용하여 ANNRI-Gd 모델의 동작을 제어할 수 있습니다.

  * **/myApp/phys/gd/captureMode [mode]**: 중성자 포획 반응 핵종 선택.
      * `1`: Natural Gd (기본값)
      * `2`: Enriched ¹⁵⁷Gd
      * `3`: Enriched ¹⁵⁵Gd
  * **/myApp/phys/gd/cascadeMode [mode]**: 감마선 캐스케이드 종류 선택.
      * `1`: 연속 + 이산 스펙트럼 모두 (기본값)
      * `2`: 이산 스펙트럼만
      * `3`: 연속 스펙트럼만

-----

## 5\. 코드 구조

  * `CPNR_modular_sim.cc`: 시뮬레이션의 시작점(main 함수).
  * `include/`, `src/`:
      * `DetectorConstruction`: 검출기 기하구조와 물질 정의 (모든 기하구조가 파라미터 기반으로 재설계됨).
      * `MyShieldingPhysList`: 메인 물리 리스트.
      * `MyHadronPhysics`: 커스텀 강입자 물리 모듈.
      * `GdNeutronHPCapture`, `GdNeutronHPCaptureFS`: ANNRI-Gd 모델 인터페이스.
      * `RunAction`, `EventAction`: 데이터 저장 관리.
      * `LSSD`, `PMTSD`: Sensitive Detector.

-----

## 6\. 개발 현황

  * **안정화 버전**: ANNRI-Gd 핵반응 모델 이식 및 Geant4 프레임워크 통합 완료.
  * **기하구조 리팩토링 완료**: 3x3 배열, 이중 PMMA 용기, 내부 지지 구조, 사실적인 PMT 모델 등 물리적으로 현실적인 검출기 구조 구현 완료.
  * **데이터 구조 확장 완료**: 물리 분석을 위한 4-운동량 및 PDG ID 출력 기능 추가 완료.
  * 현재 안정적인 시뮬레이션 기반이 확보된 상태입니다.
