#ifndef DetectorConstruction_h
#define DetectorConstruction_h 1

#include "G4VUserDetectorConstruction.hh"
#include "globals.hh"
#include "G4ThreeVector.hh"
#include "G4SystemOfUnits.hh"

class G4OpticalSurface;
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
    G4LogicalVolume* ConstructPMT();

    // 물질 및 광학 표면 포인터
    G4Material* fWorldMaterial;
    G4Material* fGdLsMaterial;
    G4Material* fLsMaterial;
    G4Material* fPmmaMaterial;
    G4Material* fGlassMaterial;
    G4Material* fPhotocathodeMaterial;
    G4Material* fVacuumMaterial;
    G4Material* fSiliconeGrease;
    G4Material* fTeflonMaterial;
    G4OpticalSurface* fTeflonSurface;

    // 논리 볼륨 포인터
    G4LogicalVolume* fLogicInnerPmma;
    G4LogicalVolume* fLogicOuterPmma;
    G4LogicalVolume* fLogicLS_inner;
    G4LogicalVolume* fLogicLS_outer;
    G4LogicalVolume* fLogicPhotocathode;

public:
    // 지오메트리 상수 (3x3 배열)
    static constexpr G4int kNx = 3;
    static constexpr G4int kNy = 3;
    
    static constexpr G4double kWorldX = 2.0 * m;
    static constexpr G4double kWorldY = 2.0 * m;
    static constexpr G4double kWorldZ = 4.0 * m;

    // 파라미터 기반 단일 세그먼트 크기 정의
    static constexpr G4double kSegmentLength = 200.0 * cm;
    static constexpr G4double kSegmentWidth  = 20.0 * cm;
    static constexpr G4double kSegmentHeight = 20.0 * cm;
    
    static constexpr G4double kOuterPmmaThickness = 3.0 * cm;
    static constexpr G4double kInnerContainerWidth  = 12.0 * cm;
    static constexpr G4double kInnerContainerHeight = 12.0 * cm;
    static constexpr G4double kInnerContainerLength = 190.0 * cm;
    static constexpr G4double kInnerPmmaThickness = 1.0 * cm;
    
    static constexpr G4int kPillarCount = 5;
    static constexpr G4double kPillarRadius = 1.5 * cm;

    // [수정] 평평한 면을 가진 PMT 상세 스펙
    static constexpr G4double kPmtGlassThickness = 2.0 * mm;
    static constexpr G4double kPmtNeckRadius = 3.8 * cm;
    static constexpr G4double kPmtNeckLength = 8.0 * cm;
    static constexpr G4double kPmtFaceRadius = 6.8 * cm; // PMT의 평평한 면의 반지름
    static constexpr G4double kPmtFaceThickness = 0.5 * cm; // 평평한 면의 두께
    static constexpr G4double kPmtTransitionLength = 6.0 * cm; // 목에서 면으로 변하는 곡선부 길이
    static constexpr G4double kPhotocathodeThickness = 1.0 * nm;
    static constexpr G4double kGreaseThickness = 1.0 * mm;
    static constexpr G4double kPmtHeight = kPmtNeckLength + kPmtTransitionLength + kPmtFaceThickness;
};

#endif
