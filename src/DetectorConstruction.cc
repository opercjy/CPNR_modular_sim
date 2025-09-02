#include "DetectorConstruction.hh"

// Geant4 헤더 파일
#include "G4NistManager.hh"
#include "G4Material.hh"
#include "G4Element.hh"
#include "G4LogicalVolume.hh"
#include "G4PVPlacement.hh"
#include "G4Box.hh"
#include "G4Tubs.hh"
#include "G4Sphere.hh" // 곡면 PMT 구현을 위해 필요
#include "G4SubtractionSolid.hh" // 불리언 연산을 위해 필수
#include "G4SDManager.hh"
#include "G4VisAttributes.hh"
#include "G4Colour.hh"
#include "G4MaterialPropertiesTable.hh"
#include "G4OpticalSurface.hh"
#include "G4LogicalSkinSurface.hh"
#include "G4SystemOfUnits.hh"
#include "G4RotationMatrix.hh"

// 사용자 정의 클래스 헤더
#include "LSSD.hh"
#include "PMTSD.hh"

#include <cmath> // asin 함수 사용을 위해 추가

/**
 * @brief 생성자
 */
DetectorConstruction::DetectorConstruction()
 : G4VUserDetectorConstruction(),
   fWorldMaterial(nullptr), fGdLsMaterial(nullptr), fLsMaterial(nullptr),
   fPmmaMaterial(nullptr), fGlassMaterial(nullptr), fPhotocathodeMaterial(nullptr),
   fVacuumMaterial(nullptr), fSiliconeGrease(nullptr),
   fLogicLS_inner(nullptr), fLogicLS_outer(nullptr), fLogicPhotocathode(nullptr)
{
    // 시뮬레이션에 필요한 모든 물질을 정의합니다.
    DefineMaterials();
}

/**
 * @brief 소멸자
 */
DetectorConstruction::~DetectorConstruction() {}

/**
 * @brief 시뮬레이션에 사용될 모든 물질과 광학적 특성을 정의하는 함수
 */
void DetectorConstruction::DefineMaterials()
{
    auto nist = G4NistManager::Instance();

    // --- 기본 물질 정의 ---
    fWorldMaterial = nist->FindOrBuildMaterial("G4_AIR");
    fPmmaMaterial = nist->FindOrBuildMaterial("G4_PLEXIGLASS");
    fGlassMaterial = nist->FindOrBuildMaterial("G4_Pyrex_Glass"); 
    fVacuumMaterial = nist->FindOrBuildMaterial("G4_Galactic");
    fSiliconeGrease = nist->FindOrBuildMaterial("G4_SILICON_DIOXIDE");
    fSiliconeGrease->SetName("SiliconeGrease");

    // --- 사용자 정의 물질 ---
    // 광음극 물질 (Bialkali, 성분 근사치)
    fPhotocathodeMaterial = new G4Material("Bialkali", 3.0*g/cm3, 3);
    fPhotocathodeMaterial->AddElement(nist->FindOrBuildElement("K"), 2);
    fPhotocathodeMaterial->AddElement(nist->FindOrBuildElement("Cs"), 1);
    fPhotocathodeMaterial->AddElement(nist->FindOrBuildElement("Sb"), 1);

    // 기본 액체 섬광체 (LS)
    G4Material* lab = new G4Material("LAB", 0.86*g/cm3, 2);
    lab->AddElement(nist->FindOrBuildElement("C"), 18);
    lab->AddElement(nist->FindOrBuildElement("H"), 30);
    
    G4Material* ppo = new G4Material("PPO", 1.1*g/cm3, 4);
    ppo->AddElement(nist->FindOrBuildElement("C"), 15);
    ppo->AddElement(nist->FindOrBuildElement("H"), 11);
    ppo->AddElement(nist->FindOrBuildElement("N"), 1);
    ppo->AddElement(nist->FindOrBuildElement("O"), 1);
    
    G4Material* bis_msb = new G4Material("bisMSB", 1.05*g/cm3, 2);
    bis_msb->AddElement(nist->FindOrBuildElement("C"), 24);
    bis_msb->AddElement(nist->FindOrBuildElement("H"), 22);

    fLsMaterial = new G4Material("LS", 0.863*g/cm3, 3);
    fLsMaterial->AddMaterial(lab, 99.64*perCent);
    fLsMaterial->AddMaterial(ppo, 0.35*perCent);
    fLsMaterial->AddMaterial(bis_msb, 0.01*perCent);

    // 가돌리늄(Gd) 첨가 액체 섬광체 (Gd-LS)
    G4Element* elGd = nist->FindOrBuildElement("Gd");
    fGdLsMaterial = new G4Material("GdLS", 0.865*g/cm3, 2);
    fGdLsMaterial->AddMaterial(fLsMaterial, 99.8*perCent);
    fGdLsMaterial->AddElement(elGd, 0.2*perCent);

    // --- 광학 속성 정의 ---
    const std::vector<G4double> photonEnergies = {2.0*eV, 2.5*eV, 2.8*eV, 3.1*eV, 3.5*eV};

    // 굴절률(RINDEX)
    auto airMPT = new G4MaterialPropertiesTable();
    airMPT->AddProperty("RINDEX", photonEnergies, std::vector<G4double>(photonEnergies.size(), 1.00));
    fWorldMaterial->SetMaterialPropertiesTable(airMPT);
    
    auto glassMPT = new G4MaterialPropertiesTable();
    glassMPT->AddProperty("RINDEX", photonEnergies, std::vector<G4double>(photonEnergies.size(), 1.47));
    fGlassMaterial->SetMaterialPropertiesTable(glassMPT);

    auto greaseMPT = new G4MaterialPropertiesTable();
    greaseMPT->AddProperty("RINDEX", photonEnergies, std::vector<G4double>(photonEnergies.size(), 1.45));
    fSiliconeGrease->SetMaterialPropertiesTable(greaseMPT);

    // 섬광체 광학 속성 (LS와 GdLS가 동일하다고 가정)
    auto lsMPT = new G4MaterialPropertiesTable();
    lsMPT->AddProperty("RINDEX", photonEnergies, std::vector<G4double>(photonEnergies.size(), 1.50));
    std::vector<G4double> ls_emission = {0.1, 0.4, 1.0, 0.4, 0.1};
    lsMPT->AddProperty("SCINTILLATIONCOMPONENT1", photonEnergies, ls_emission);
    lsMPT->AddConstProperty("SCINTILLATIONYIELD", 10000./MeV);
    lsMPT->AddConstProperty("SCINTILLATIONTIMECONSTANT1", 10.*ns);
    lsMPT->AddConstProperty("RESOLUTIONSCALE", 1.0);
    fLsMaterial->SetMaterialPropertiesTable(lsMPT);
    fGdLsMaterial->SetMaterialPropertiesTable(lsMPT);
}

/**
 * @brief 최상위 볼륨(World)과 검출기 배열을 생성하는 메인 함수
 */
G4VPhysicalVolume* DetectorConstruction::Construct()
{
    // 월드 생성
    auto solidWorld = new G4Box("SolidWorld", kWorldX/2, kWorldY/2, kWorldZ/2);
    auto logicWorld = new G4LogicalVolume(solidWorld, fWorldMaterial, "LogicWorld");
    auto physWorld = new G4PVPlacement(nullptr, G4ThreeVector(), logicWorld, "PhysWorld", nullptr, false, 0, true);
    logicWorld->SetVisAttributes(G4VisAttributes::GetInvisible());

    // 단일 검출기 세그먼트의 논리 볼륨을 생성
    G4LogicalVolume* logicSegment = ConstructSegment();

    // 5x5 배열로 세그먼트들을 배치
    G4double pitchX = kSegmentWidth + 1.0*cm;
    G4double pitchY = kSegmentHeight + 1.0*cm;

    for (G4int j = 0; j < kNy; ++j) {
        for (G4int i = 0; i < kNx; ++i) {
            G4double x = (i - (kNx-1)/2.0) * pitchX;
            G4double y = (j - (kNy-1)/2.0) * pitchY;
            G4int copyNo = j * kNx + i;
            new G4PVPlacement(nullptr, G4ThreeVector(x, y, 0), logicSegment, "PhysSegment", logicWorld, false, copyNo, true);
        }
    }
    return physWorld;
}

/**
 * @brief 사실적인 곡면 PMT를 생성하는 헬퍼 함수
 * @return PMT의 논리 볼륨 (G4LogicalVolume*)
 */
G4LogicalVolume* DetectorConstruction::ConstructPMT()
{
    // --- PMT 전체를 감싸는 모체 볼륨 ---
    auto solidPmtAssembly = new G4Tubs("SolidPmtAssembly", 0, kPmtRadius, kPmtHeight/2, 0, CLHEP::twopi);
    auto logicPmtAssembly = new G4LogicalVolume(solidPmtAssembly, fVacuumMaterial, "LogicPmtAssembly");
    logicPmtAssembly->SetVisAttributes(G4VisAttributes::GetInvisible());
    
    // --- PMT 유리 입사창 (곡면) ---
    G4double glassThickness = 3.0 * mm;
    G4double pmt_end_angle = std::asin(kPmtRadius / kPmtSphereRadius);

    auto solidGlassWindow = new G4Sphere("SolidGlassWindow", 
                                       kPmtSphereRadius - glassThickness, kPmtSphereRadius,
                                       0, CLHEP::twopi, 0, pmt_end_angle);
    auto logicGlassWindow = new G4LogicalVolume(solidGlassWindow, fGlassMaterial, "LogicGlassWindow");
    logicGlassWindow->SetVisAttributes(new G4VisAttributes(G4Colour(0.0, 1.0, 1.0, 0.05)));

    G4double glassWindow_z = kPmtHeight/2 - kPmtSphereRadius * std::cos(pmt_end_angle);
    new G4PVPlacement(nullptr, G4ThreeVector(0, 0, glassWindow_z), logicGlassWindow, "PhysGlassWindow", logicPmtAssembly, false, 0, true);

    // --- 광음극 (Photocathode) ---
    auto solidPhotocathode = new G4Sphere("SolidPhotocathode", 
                                        kPmtSphereRadius - glassThickness - kPhotocathodeThickness, 
                                        kPmtSphereRadius - glassThickness,
                                        0, CLHEP::twopi, 0, pmt_end_angle);
    fLogicPhotocathode = new G4LogicalVolume(solidPhotocathode, fPhotocathodeMaterial, "LogicPhotocathode");
    fLogicPhotocathode->SetVisAttributes(new G4VisAttributes(G4Colour::Red()));
    new G4PVPlacement(nullptr, G4ThreeVector(0, 0, glassWindow_z), fLogicPhotocathode, "PhysPhotocathode", logicPmtAssembly, false, 0, true);

    // --- 광음극의 광학적 표면 속성 설정 ---
    auto photocathode_opsurf = new G4OpticalSurface("Photocathode_OpSurface");
    photocathode_opsurf->SetType(dielectric_metal);
    photocathode_opsurf->SetModel(unified);
    photocathode_opsurf->SetFinish(polished);
    
    // 양자 효율(QE) 설정
    const std::vector<G4double> energies_qe = {2.0*eV, 2.5*eV, 2.8*eV, 3.1*eV, 3.5*eV};
    const std::vector<G4double> efficiency = {0.15, 0.22, 0.28, 0.22, 0.10};
    auto pmtMPT = new G4MaterialPropertiesTable();
    pmtMPT->AddProperty("EFFICIENCY", energies_qe, efficiency);
    photocathode_opsurf->SetMaterialPropertiesTable(pmtMPT);
    new G4LogicalSkinSurface("Photocathode_SkinSurface", fLogicPhotocathode, photocathode_opsurf);

    return logicPmtAssembly;
}

/**
 * @brief 불리언 연산을 사용하여 단일 세그먼트를 생성하는 함수
 * @return 세그먼트의 논리 볼륨 (G4LogicalVolume*)
 */
G4LogicalVolume* DetectorConstruction::ConstructSegment()
{
    // --- 1. 각 구성요소의 '틀(Mold)'이 될 기본 솔리드들을 정의합니다. ---
    
    auto pмма_mold = new G4Box("pmma_mold", kSegmentWidth/2, kSegmentHeight/2, kSegmentLength/2);

    G4double pmmThickness = 0.5*cm;
    auto scint_region_mold = new G4Box("scint_region_mold", 
                                       kSegmentWidth/2 - pmmThickness, 
                                       kSegmentHeight/2 - pmmThickness, 
                                       kSegmentLength/2);

    auto core_mold = new G4Box("core_mold", 
                               kInnerCoreWidth/2, 
                               kInnerCoreHeight/2, 
                               kSegmentLength/2);

    // --- 2. '빼기' 연산을 통해 각 부품의 최종 모양(Solid)을 조각합니다. ---

    auto solidPmma_Hollow = new G4SubtractionSolid("SolidPmma_Hollow", pмма_mold, scint_region_mold);
    auto solidLS_outer_Shell = new G4SubtractionSolid("SolidLS_outer_Shell", scint_region_mold, core_mold);
    auto solidLS_inner_Core = core_mold;

    // --- 3. 조각된 모양들로 각각의 논리 볼륨(Logical Volume)을 생성합니다. ---
    auto logicPmma = new G4LogicalVolume(solidPmma_Hollow, fPmmaMaterial, "LogicPmma");
    logicPmma->SetVisAttributes(new G4VisAttributes(G4Colour(0.0, 1.0, 1.0, 0.1)));

    fLogicLS_outer = new G4LogicalVolume(solidLS_outer_Shell, fLsMaterial, "LogicLS_outer");
    fLogicLS_outer->SetVisAttributes(new G4VisAttributes(G4Colour(1.0, 1.0, 0.0, 0.2)));
    
    fLogicLS_inner = new G4LogicalVolume(solidLS_inner_Core, fGdLsMaterial, "LogicLS_inner");
    fLogicLS_inner->SetVisAttributes(new G4VisAttributes(G4Colour(1.0, 0.5, 0.0, 0.4)));

    // --- 4. 최종 조립: 각 부품을 제자리에 배치합니다. ---
    
    // 세그먼트 전체를 담을 모체 볼륨
    G4double assemblyHalfZ = kSegmentLength/2 + kGreaseThickness + kPmtHeight;
    auto solidAssembly = new G4Box("SolidAssembly", kSegmentWidth/2, kSegmentHeight/2, assemblyHalfZ);
    auto logicAssembly = new G4LogicalVolume(solidAssembly, fWorldMaterial, "LogicAssembly");
    logicAssembly->SetVisAttributes(G4VisAttributes::GetInvisible());
    
    // PMMA, LS, GdLS 파트들을 모체 볼륨의 중앙에 배치
    new G4PVPlacement(nullptr, G4ThreeVector(0,0,0), logicPmma, "PhysPmma", logicAssembly, false, 0, true);
    new G4PVPlacement(nullptr, G4ThreeVector(0,0,0), fLogicLS_outer, "PhysLS_outer", logicAssembly, false, 0, true);
    new G4PVPlacement(nullptr, G4ThreeVector(0,0,0), fLogicLS_inner, "PhysLS_inner", logicAssembly, false, 0, true);

    // 실리콘 구리스와 PMT
    auto logicGrease = new G4LogicalVolume(new G4Tubs("SolidGrease", 0, kPmtRadius, kGreaseThickness/2.0, 0, CLHEP::twopi),
                                           fSiliconeGrease, "LogicGrease");
    logicGrease->SetVisAttributes(new G4VisAttributes(G4Colour(0.8, 0.8, 0.8, 0.2)));
    
    G4LogicalVolume* logicPMT = ConstructPMT();
    
    G4double greasePos_z = kSegmentLength/2 + kGreaseThickness/2;
    G4double pmtPos_z = kSegmentLength/2 + kGreaseThickness + kPmtHeight/2;
    
    auto rot_180 = new G4RotationMatrix();
    rot_180->rotateY(180.0 * deg);

    // +z 방향 조립
    new G4PVPlacement(nullptr, G4ThreeVector(0, 0, greasePos_z), logicGrease, "PhysGrease", logicAssembly, false, 0, true);
    new G4PVPlacement(nullptr, G4ThreeVector(0, 0, pmtPos_z), logicPMT, "PhysPMT", logicAssembly, false, 0, true);
    
    // -z 방향 조립
    new G4PVPlacement(rot_180, G4ThreeVector(0, 0, -greasePos_z), logicGrease, "PhysGrease", logicAssembly, false, 1, true);
    new G4PVPlacement(rot_180, G4ThreeVector(0, 0, -pmtPos_z), logicPMT, "PhysPMT", logicAssembly, false, 1, true);

    return logicAssembly;
}

/**
 * @brief Sensitive Detector(SD)들을 각각의 논리 볼륨에 할당하는 함수
 */
void DetectorConstruction::ConstructSDandField()
{
    auto sdManager = G4SDManager::GetSDMpointer();
    
    auto lsSD = new LSSD("LSSD");
    sdManager->AddNewDetector(lsSD);
    if (fLogicLS_inner) SetSensitiveDetector(fLogicLS_inner, lsSD);
    if (fLogicLS_outer) SetSensitiveDetector(fLogicLS_outer, lsSD);
    
    if (fLogicPhotocathode) {
        auto pmtSD = new PMTSD("PMTSD");
        sdManager->AddNewDetector(pmtSD);
        SetSensitiveDetector(fLogicPhotocathode, pmtSD);
    }
}
