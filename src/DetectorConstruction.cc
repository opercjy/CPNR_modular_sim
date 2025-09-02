#include "DetectorConstruction.hh"
#include "G4NistManager.hh"
#include "G4Material.hh"
#include "G4Element.hh"
#include "G4LogicalVolume.hh"
#include "G4PVPlacement.hh"
#include "G4Box.hh"
#include "G4Tubs.hh"
#include "G4SDManager.hh"
#include "G4VisAttributes.hh"
#include "G4Colour.hh"
#include "G4MaterialPropertiesTable.hh"
#include "G4OpticalSurface.hh"
#include "G4LogicalSkinSurface.hh"

#include "LSSD.hh"
#include "PMTSD.hh"

DetectorConstruction::DetectorConstruction()
 : G4VUserDetectorConstruction(),
   fWorldMaterial(nullptr), fGdLsMaterial(nullptr), fLsMaterial(nullptr),
   fPmmaMaterial(nullptr), fPmtMaterial(nullptr),
   fLogicLS_inner(nullptr), fLogicLS_outer(nullptr), fLogicPhotocathode(nullptr)
{
    DefineMaterials();
}

DetectorConstruction::~DetectorConstruction() {}

void DetectorConstruction::DefineMaterials()
{
    auto nist = G4NistManager::Instance();
    fWorldMaterial = nist->FindOrBuildMaterial("G4_AIR");
    fPmmaMaterial = nist->FindOrBuildMaterial("G4_PLEXIGLASS");
    fPmtMaterial = nist->FindOrBuildMaterial("G4_GLASS_LEAD"); 

    // --- 기본 액체 섬광체 (LS) 정의 (Unloaded) ---
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

    // --- 가돌리늄(Gd) 첨가 액체 섬광체 (Gd-LS) 정의 ---
    G4Element* elGd = nist->FindOrBuildElement("Gd");
    
    // 가정: Gd를 0.2% 질량비로 첨가
    fGdLsMaterial = new G4Material("GdLS", 0.865*g/cm3, 2);
    fGdLsMaterial->AddMaterial(fLsMaterial, 99.8*perCent);
    fGdLsMaterial->AddElement(elGd, 0.2*perCent);

    // --- 광학 속성 정의 (간략화) ---
    // 실제로는 LS와 GdLS의 특성이 약간 다르지만, 여기서는 동일하게 가정.
    // 추후 이 부분에 더 정밀한 데이터를 추가할 수 있음.
    const std::vector<G4double> photonEnergies = {2.0*eV, 3.1*eV};
    const std::vector<G4double> rindex_ls = {1.50, 1.50};
    const std::vector<G4double> absorption_ls = {20*m, 20*m};
    const std::vector<G4double> emission_ls = {1.0, 1.0};
    
    auto lsMPT = new G4MaterialPropertiesTable();
    lsMPT->AddProperty("RINDEX", photonEnergies, rindex_ls);
    lsMPT->AddProperty("ABSLENGTH", photonEnergies, absorption_ls);
    lsMPT->AddProperty("SCINTILLATIONCOMPONENT1", photonEnergies, emission_ls);
    lsMPT->AddConstProperty("SCINTILLATIONYIELD", 10000./MeV);
    lsMPT->AddConstProperty("SCINTILLATIONTIMECONSTANT1", 10.*ns);
    lsMPT->AddConstProperty("RESOLUTIONSCALE", 1.0);
    
    fLsMaterial->SetMaterialPropertiesTable(lsMPT);
    fGdLsMaterial->SetMaterialPropertiesTable(lsMPT);
}


G4VPhysicalVolume* DetectorConstruction::Construct()
{
    // 월드 생성
    auto solidWorld = new G4Box("SolidWorld", kWorldX/2, kWorldY/2, kWorldZ/2);
    auto logicWorld = new G4LogicalVolume(solidWorld, fWorldMaterial, "LogicWorld");
    auto physWorld = new G4PVPlacement(nullptr, G4ThreeVector(), logicWorld, "PhysWorld", nullptr, false, 0, true);
    logicWorld->SetVisAttributes(G4VisAttributes::GetInvisible());

    // 검출기 세그먼트 배열 생성
    G4LogicalVolume* logicSegment = ConstructSegment();
    G4double pitchX = kSegmentWidth + 1.0*cm;
    G4double pitchY = kSegmentHeight + 1.0*cm;

    for (G4int i = 0; i < kNx; ++i) {
        for (G4int j = 0; j < kNy; ++j) {
            G4double x = (i - (kNx-1)/2.0) * pitchX;
            G4double y = (j - (kNy-1)/2.0) * pitchY;
            G4int copyNo = j * kNx + i;
            new G4PVPlacement(nullptr, G4ThreeVector(x, y, 0), logicSegment, "PhysSegment", logicWorld, false, copyNo, true);
        }
    }
    return physWorld;
}

G4LogicalVolume* DetectorConstruction::ConstructSegment()
{
    // --- 세그먼트 모체 볼륨 ---
    auto solidAssembly = new G4Box("SolidAssembly", kSegmentWidth/2, kSegmentHeight/2, kSegmentLength/2 + kPhotocathodeHalfZ);
    auto logicAssembly = new G4LogicalVolume(solidAssembly, fWorldMaterial, "LogicAssembly");
    logicAssembly->SetVisAttributes(G4VisAttributes::GetInvisible());

    // --- PMMA 하우징 ---
    G4double pmmThickness = 0.5*cm;
    auto solidPmma = new G4Box("SolidPmma", kSegmentWidth/2, kSegmentHeight/2, kSegmentLength/2);
    auto logicPmma = new G4LogicalVolume(solidPmma, fPmmaMaterial, "LogicPmma");
    logicPmma->SetVisAttributes(new G4VisAttributes(G4Colour(0.0, 1.0, 1.0, 0.1)));
    new G4PVPlacement(nullptr, G4ThreeVector(), logicPmma, "PhysPmma", logicAssembly, false, 0, true);

    // --- 이중 섬광체 구조 생성 ---
    // 1. 외부 LS 쉘 (Unloaded LS)
    auto solidLS_outer = new G4Box("SolidLS_outer", kSegmentWidth/2 - pmmThickness, kSegmentHeight/2 - pmmThickness, kSegmentLength/2 - pmmThickness);
    fLogicLS_outer = new G4LogicalVolume(solidLS_outer, fLsMaterial, "LogicLS_outer");
    fLogicLS_outer->SetVisAttributes(new G4VisAttributes(G4Colour(1.0, 1.0, 0.0, 0.2))); // 연한 노란색
    new G4PVPlacement(nullptr, G4ThreeVector(), fLogicLS_outer, "PhysLS_outer", logicPmma, false, 0, true);

    // 2. 내부 Gd-LS 코어
    auto solidLS_inner = new G4Box("SolidLS_inner", kInnerCoreWidth/2, kInnerCoreHeight/2, kSegmentLength/2 - pmmThickness);
    fLogicLS_inner = new G4LogicalVolume(solidLS_inner, fGdLsMaterial, "LogicLS_inner");
    fLogicLS_inner->SetVisAttributes(new G4VisAttributes(G4Colour(1.0, 0.5, 0.0, 0.4))); // 주황색
    // 외부 LS 쉘의 자식 볼륨으로 배치
    new G4PVPlacement(nullptr, G4ThreeVector(), fLogicLS_inner, "PhysLS_inner", fLogicLS_outer, false, 0, true);

    // --- 양 끝에 PMT 배치 ---
    auto solidPhotocathode = new G4Tubs("SolidPhotocathode", 0, kPmtRadius, kPhotocathodeHalfZ, 0, CLHEP::twopi);
    fLogicPhotocathode = new G4LogicalVolume(solidPhotocathode, fPmtMaterial, "LogicPhotocathode");
    fLogicPhotocathode->SetVisAttributes(new G4VisAttributes(G4Colour::Red()));

    G4double pmt_pos_z = kSegmentLength/2 + kPhotocathodeHalfZ;
    new G4PVPlacement(nullptr, G4ThreeVector(0, 0, pmt_pos_z), fLogicPhotocathode, "PhysPhotocathode", logicAssembly, false, 0, true);
    auto rotPMT = new G4RotationMatrix();
    rotPMT->rotateY(180.0 * deg);
    new G4PVPlacement(rotPMT, G4ThreeVector(0, 0, -pmt_pos_z), fLogicPhotocathode, "PhysPhotocathode", logicAssembly, false, 1, true);

    // --- 광음극 광학 표면 정의 ---
    auto photocathode_opsurf = new G4OpticalSurface("Photocathode_OpSurface");
    photocathode_opsurf->SetType(dielectric_metal);
    photocathode_opsurf->SetModel(unified);
    photocathode_opsurf->SetFinish(polished);
    
    const std::vector<G4double> energies_qe = {2.0*eV, 3.5*eV};
    const std::vector<G4double> efficiency = {0.25, 0.25};
    auto pmtMPT = new G4MaterialPropertiesTable();
    pmtMPT->AddProperty("EFFICIENCY", energies_qe, efficiency);
    photocathode_opsurf->SetMaterialPropertiesTable(pmtMPT);
    new G4LogicalSkinSurface("Photocathode_SkinSurface", fLogicPhotocathode, photocathode_opsurf);

    return logicAssembly;
}

void DetectorConstruction::ConstructSDandField()
{
    auto sdManager = G4SDManager::GetSDMpointer();
    auto lsSD = new LSSD("LSSD");
    sdManager->AddNewDetector(lsSD);

    // 내부 코어와 외부 쉘 모두에 동일한 LSSD를 할당
    if (fLogicLS_inner) SetSensitiveDetector(fLogicLS_inner, lsSD);
    if (fLogicLS_outer) SetSensitiveDetector(fLogicLS_outer, lsSD);
    
    if (fLogicPhotocathode) {
        auto pmtSD = new PMTSD("PMTSD");
        sdManager->AddNewDetector(pmtSD);
        SetSensitiveDetector(fLogicPhotocathode, pmtSD);
    }
}
