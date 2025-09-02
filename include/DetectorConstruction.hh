#ifndef DetectorConstruction_h
#define DetectorConstruction_h 1

#include "G4VUserDetectorConstruction.hh"
#include "globals.hh"
#include "G4ThreeVector.hh"
#include "G4SystemOfUnits.hh"

class G4VPhysicalVolume;
class G4LogicalVolume;
class G4Material;

/**
 * @class DetectorConstruction
 * @brief Gd-LS 코어와 LS 쉘의 이중 구조를 가진 모듈형 검출기를 생성합니다.
 */
class DetectorConstruction : public G4VUserDetectorConstruction
{
public:
    DetectorConstruction();
    virtual ~DetectorConstruction();
    virtual G4VPhysicalVolume* Construct() override;
    virtual void ConstructSDandField() override;

private:
    // Helper 함수
    void DefineMaterials();
    G4LogicalVolume* ConstructSegment();

    // 물질 포인터
    G4Material* fWorldMaterial;
    G4Material* fGdLsMaterial;    // Gadolinium-loaded Liquid Scintillator
    G4Material* fLsMaterial;      // Unloaded Liquid Scintillator
    G4Material* fPmmaMaterial;    // PMMA (Acrylic)
    G4Material* fPmtMaterial;     // PMT photocathode material

    // SD 할당을 위한 논리 볼륨 포인터
    G4LogicalVolume* fLogicLS_inner; // Gd-LS (Core)
    G4LogicalVolume* fLogicLS_outer; // LS (Shell)
    G4LogicalVolume* fLogicPhotocathode;

public:
    // --- 지오메트리 상수 정의 ---
    static constexpr G4int kNx = 14;
    static constexpr G4int kNy = 11;
    
    static constexpr G4double kWorldX = 3.0 * m;
    static constexpr G4double kWorldY = 3.0 * m;
    static constexpr G4double kWorldZ = 3.0 * m;

    // 단일 세그먼트 크기
    static constexpr G4double kSegmentLength = 100.0 * cm;
    static constexpr G4double kSegmentWidth  = 15.0 * cm;
    static constexpr G4double kSegmentHeight = 15.0 * cm;
    
    // 내부 Gd-LS 코어 크기
    static constexpr G4double kInnerCoreWidth = 10.0 * cm;
    static constexpr G4double kInnerCoreHeight = 10.0 * cm;

    // 5인치 PMT
    static constexpr G4double kPmtRadius = 6.35 * cm;
    static constexpr G4double kPhotocathodeHalfZ = 0.1 * cm;
};

#endif
