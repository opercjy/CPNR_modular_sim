# Geant4 기반 모듈형 검출기 시뮬레이션: CPNR_modular_sim (개발 중)
<img width="1320" height="1020" alt="image" src="https://github.com/user-attachments/assets/7f61aa08-f10d-4450-8f7b-e7df066cb323" />
<img width="1320" height="1020" alt="image" src="https://github.com/user-attachments/assets/9fe6eec4-ed72-4ccb-aa1d-d822ee1581be" />
<img width="1320" height="1020" alt="image" src="https://github.com/user-attachments/assets/dfaae9f5-c414-4877-8435-efccdc57eee8" />


## 1. 프로젝트 개요

본 프로젝트는 **모듈형 섬광체 검출기 배열**을 시뮬레이션하기 위한 Geant4 애플리케이션입니다. 각 검출기 모듈(세그먼트)은 중성자 포획 효율을 높이기 위해 내부에 \*\*가돌리늄(Gadolinium)이 첨가된 액체섬광체(Gd-LS)\*\*를, 그 주변을 \*\*일반 액체섬광체(LS)\*\*가 감싸는 이중 구조로 설계되었습니다.

시뮬레이션의 핵심은 표준 Geant4 물리 모델을 확장하여, 가돌리늄의 열중성자 포획 반응에 대해 정밀한 감마선 캐스케이드 데이터를 생성하는 **ANNRI-Gd 모델**을 선택적으로 적용하는 것입니다. 이를 통해 보다 현실에 가까운 검출기 응답을 연구할 수 있습니다.

### 주요 특징

  * **모듈형 검출기 설계**: 5x5 배열의 개별 세그먼트로 구성되며, `DetectorConstruction.hh`에서 쉽게 크기 조절이 가능합니다.
  * **견고한 기하구조**: `G4SubtractionSolid` 불리언 연산을 통해 모든 구성요소가 겹치지 않도록 설계하여 'Stuck Track'과 같은 네비게이션 오류를 원천적으로 방지합니다.
  * **사실적인 PMT 모델**: 곡면 입사창과 광음극을 가진 5인치 하마마츠 PMT와 광학적 연결을 위한 실리콘 구리스를 구현합니다.
  * **커스텀 물리 모델**: 표준 차폐 물리 리스트를 기반으로, 가돌리늄(Z=64)의 중성자 포획 반응에만 **ANNRI-Gd 모델** (https://www.physics.okayama-u.ac.jp/~sakuda/ANNRI-Gd_ver1.html) 을동적으로 적용하는 사용자 정의 물리 리스트(`MyShieldingPhysList`)를 사용합니다.
  * **유연한 제어**: Geant4 메신저를 통해 매크로 파일에서 ANNRI-Gd 모델의 상세 옵션(캡처 모드, 데이터 파일 경로 등)을 C++ 코드 수정 없이 제어할 수 있습니다.
  * **상세한 데이터 출력**: `G4AnalysisManager`를 통해 각 상호작용(Hit)과 PMT 광자 검출 정보를 ROOT 파일 형식으로 저장합니다.

-----

## 2. 사전 준비

### 2.1. ANNRI-Gd 데이터 파일

이 시뮬레이션은 ANNRI-Gd의 연속 스펙트럼 모델을 위해 외부 ROOT 데이터 파일이 필요합니다.
http://www.physics.okayama-u.ac.jp/~sakuda/ANNRI-Gd/cont_dat.zip

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

### 2.3. 외부 도구 (동영상 제작 시)

시뮬레이션 결과로 동영상을 제작하려면 `ImageMagick`과 `ffmpeg`이 필요합니다. (Docker/Linux 환경 기준)

```bash
sudo dnf install ImageMagick ffmpeg -y
```

-----

## 3. 빌드 및 실행

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
    # ../vis.mac 파일을 자동으로 실행합니다.
    ./CPNR_modular_sim
    ```
  * **배치 모드 (데이터 생성)**
    ```bash
    # ../run.mac 파일을 실행합니다.
    ./CPNR_modular_sim run.mac
    ```

-----

## 4. 고급 사용법

### 4.1. 물리 모델 제어

`run.mac`과 같은 매크로 파일 안에서 Geant4 메신저 명령어를 사용하여 ANNRI-Gd 모델의 동작을 제어할 수 있습니다. 이 명령어들은 `/run/initialize` 이전에 실행되어야 합니다.

  * **/myApp/phys/gd/setCaptureMode [mode]**: 중성자 포획 반응을 시뮬레이션할 핵종을 선택합니다.
      * `1`: Natural Gd (기본값)
      * `2`: Enriched ¹⁵⁷Gd
      * `3`: Enriched ¹⁵⁵Gd
  * **/myApp/phys/gd/setCascadeMode [mode]**: 생성할 감마선 캐스케이드 종류를 선택합니다.
      * `1`: 연속 + 이산 스펙트럼 모두 (기본값)
      * `2`: 이산 스펙트럼만
      * `3`: 연속 스펙트럼만
  * **/myApp/phys/gd/verbose [level]**: ANNRI-Gd 모델의 초기화 과정에서 출력되는 메시지 레벨을 조절합니다.
      * `0`: 메시지 없음
      * `1`: 기본 정보 출력 (기본값)

### 4.2. 동영상 제작

시뮬레이션 이벤트의 시간 흐름을 동영상으로 만들 수 있습니다.

1.  **프레임 생성**: `movie.mac`을 실행하여 이벤트의 시간대별 스냅샷을 `.eps` 이미지 파일로 생성합니다.
    ```bash
    ./CPNR_modular_sim movie.mac
    ```
2.  **이미지 형식 변환**: `ImageMagick`을 사용하여 `.eps` 파일들을 `.jpg`로 일괄 변환합니다.
    ```bash
    for f in g4_*.eps; do convert "$f" "${f%.eps}.jpg"; done
    ```
3.  **동영상 인코딩**: `ffmpeg`을 사용하여 `.jpg` 프레임들을 하나의 `.mp4` 동영상으로 합칩니다.
    ```bash
    ffmpeg -r 25 -i g4_%04d.jpg -c:v libx264 -pix_fmt yuv420p -y simulation_movie.mp4
    ```

-----

## 5. 코드 구조

  * `CPNR_modular_sim.cc`: 시뮬레이션의 시작점(main 함수).
  * `include/`, `src/`:
      * `DetectorConstruction`: 검출기 기하구조와 물질 정의.
      * `MyShieldingPhysList`: 메인 물리 리스트. 표준 물리 모듈과 커스텀 모듈을 조립.
      * `MyHadronPhysics`: 커스텀 강입자 물리 모듈. 중성자 포획 프로세스를 제어.
      * `GdNeutronHPCapture`, `GdNeutronHPCaptureFS`: ANNRI-Gd 모델을 Geant4에 연결하는 핵심 인터페이스 클래스.

-----

## 6. 개발 현황

  * 2025년 9월 3일 현재, ANNRI-Gd 핵반응 물리 모델 이식 및 리팩토링 완료. 안정적인 시뮬레이션 기반 확보하기 위해 검증 중.
