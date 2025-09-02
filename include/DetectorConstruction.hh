#ifndef DetectorConstruction_h
#define DetectorConstruction_h 1

#include "G4VUserDetectorConstruction.hh"
#include "globals.hh"
#include "G4ThreeVector.hh"
#include "G4SystemOfUnits.hh"

class G4VPhysicalVolume;
class G4LogicalVolume;
class G4Material;

class DetectorConstruction : public G4VUserDetectorConstruction
{
public:
    DetectorConstruction();
    virtual ~DetectorConstruction();
    virtual G4VPhysicalVolume* Construct() override;
    virtual void ConstructSDandField() override;

private:
    void DefineMaterials();
    G4LogicalVolume* ConstructSegment();
    // [추가] 사실적인 PMT 생성을 위한 헬퍼 함수
    G4LogicalVolume* ConstructPMT(); 

    // 물질 포인터
    G4Material* fWorldMaterial;
    G4Material* fGdLsMaterial;
    G4Material* fLsMaterial;
    G4Material* fPmmaMaterial;
    G4Material* fGlassMaterial;      // PMT 유리
    G4Material* fPhotocathodeMaterial; // 광음극 물질
    G4Material* fVacuumMaterial;
    G4Material* fSiliconeGrease;     // 실리콘 구리스

    // SD 할당을 위한 논리 볼륨 포인터
    G4LogicalVolume* fLogicLS_inner;
    G4LogicalVolume* fLogicLS_outer;
    G4LogicalVolume* fLogicPhotocathode;

public:
    // --- 지오메트리 상수 정의 ---
    static constexpr G4int kNx = 5; // 수정됨
    static constexpr G4int kNy = 5; // 수정됨
    
    static constexpr G4double kWorldX = 2.0 * m; // 배열이 작아졌으므로 월드 크기 조정
    static constexpr G4double kWorldY = 2.0 * m;
    static constexpr G4double kWorldZ = 3.0 * m;

    // 단일 세그먼트 크기
    static constexpr G4double kSegmentLength = 100.0 * cm;
    static constexpr G4double kSegmentWidth  = 15.0 * cm;
    static constexpr G4double kSegmentHeight = 15.0 * cm;
    
    // 내부 Gd-LS 코어 크기
    static constexpr G4double kInnerCoreWidth = 10.0 * cm;
    static constexpr G4double kInnerCoreHeight = 10.0 * cm;

    // [추가] 5인치 PMT 상세 스펙 (Hamamatsu R5912 기반)
    static constexpr G4double kPmtRadius = 6.35 * cm; // 5인치
    static constexpr G4double kPmtSphereRadius = 10.0 * cm; // PMT 유리의 곡률 반경 (가정)
    static constexpr G4double kPmtHeight = 15.0 * cm; // PMT 전체 높이 (가정)
    static constexpr G4double kPhotocathodeThickness = 1.0 * nm; // 매우 얇은 막
    static constexpr G4double kGreaseThickness = 1.0 * mm; // 구리스 두께
};

#endif
