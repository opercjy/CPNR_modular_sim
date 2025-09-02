# CPNR_modular_sim
# Geant4 기반 모듈형 검출기 시뮬레이션: CPNR_modular_sim
<img width="1320" height="1020" alt="image" src="https://github.com/user-attachments/assets/7f61aa08-f10d-4450-8f7b-e7df066cb323" />

<img width="1320" height="1020" alt="image" src="https://github.com/user-attachments/assets/a0cb7f6a-d35e-4c8c-9937-1430881596ce" />

## 1. 프로젝트 개요

본 프로젝트는 **모듈형 섬광체 검출기 배열**을 시뮬레이션하기 위한 Geant4 애플리케이션입니다. 각 검출기 모듈(세그먼트)은 중성자 포획 효율을 높이기 위해 내부에 **가돌리늄(Gadolinium)이 첨가된 액체섬광체(Gd-LS)**를, 그 주변을 **일반 액체섬광체(LS)**가 감싸는 이중 구조로 설계되었습니다.

이 시뮬레이션은 각 세그먼트 양 끝에 위치한 광전증배관(PMT)에서 검출되는 빛의 양과 시간 정보를 통해 입자의 에너지 증착 및 상호작용 위치를 재구성하는 것을 목표로 합니다.

### 주요 특징

* **모듈형 검출기 설계**: 전체 검출기는 사용자가 정의한 수의 개별 세그먼트(예: 14x11 배열)로 구성되어 있어 확장성이 용이합니다.
* **이중 섬광체 구조**: 각 세그먼트는 Gd-LS 코어와 일반 LS 쉘로 구성되어, 중성자 신호와 다른 입자 신호를 효과적으로 구분할 수 있도록 설계되었습니다.
* **상세한 데이터 출력**: Geant4의 `G4AnalysisManager`를 통해 각 상호작용(Hit)에 대한 상세 정보(위치, 시간, 에너지 등)와 PMT에서 검출된 광자 정보를 ROOT 파일 형식으로 저장합니다.

---

## 2. 빌드 및 실행 방법

### 2.1. 빌드

C++ 코드를 수정한 후에는 항상 다시 빌드해야 합니다. 프로젝트 최상위 디렉토리에서 다음 명령어를 실행하십시오.

```bash
# 빌드 디렉토리 생성 및 이동
mkdir build && cd build

# CMake 실행
cmake ..

# 컴파일 (시스템의 모든 코어 사용)
make -j$(nproc)
```
### 2.2. 실행
GUI 모드 (시각화 및 디버깅)
build 디렉토리에서 인자 없이 실행하면 Geant4의 그래픽 사용자 인터페이스(GUI)가 실행됩니다.

```bash
./CPNR_modular_sim
```
배치 모드 (데이터 생산)
매크로 파일을 인자로 전달하여 시뮬레이션을 실행합니다.

```bash
./CPNR_modular_sim ../run.mac
```
## 3. 코드 구조
CPNR_modular_sim.cc: 시뮬레이션의 시작점(main 함수).
include/: 모든 C++ 클래스의 헤더 파일(.hh).
src/: 모든 C++ 클래스의 소스 파일(.cc).

현재 개발중
