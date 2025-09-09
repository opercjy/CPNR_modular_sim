#include "DetectorConstruction.hh"

#include "G4NistManager.hh"
#include "G4Material.hh"
#include "G4Element.hh"
#include "G4LogicalVolume.hh"
#include "G4PVPlacement.hh"
#include "G4Box.hh"
#include "G4Tubs.hh"
#include "G4Sphere.hh"
#include "G4Polycone.hh"
#include "G4SDManager.hh"
#include "G4VisAttributes.hh"
#include "G4Colour.hh"
#include "G4MaterialPropertiesTable.hh"
#include "G4OpticalSurface.hh"
#include "G4LogicalSkinSurface.hh"
#include "G4SystemOfUnits.hh"
#include "G4RotationMatrix.hh"

#include "LSSD.hh"
#include "PMTSD.hh"
#include <cmath>
#include <vector>

DetectorConstruction::DetectorConstruction()
 : G4VUserDetectorConstruction(),
   fWorldMaterial(nullptr), fGdLsMaterial(nullptr), fLsMaterial(nullptr),
   fPmmaMaterial(nullptr), fGlassMaterial(nullptr), fPhotocathodeMaterial(nullptr),
   fVacuumMaterial(nullptr), fSiliconeGrease(nullptr), fTeflonMaterial(nullptr),
   fTeflonSurface(nullptr),
   fLogicInnerPmma(nullptr), fLogicOuterPmma(nullptr),
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
    fGlassMaterial = nist->FindOrBuildMaterial("G4_Pyrex_Glass");
    fVacuumMaterial = nist->FindOrBuildMaterial("G4_Galactic");
    fSiliconeGrease = nist->FindOrBuildMaterial("G4_SILICON_DIOXIDE");
    fSiliconeGrease->SetName("SiliconeGrease");
    
    fTeflonMaterial = new G4Material("Teflon", 2.2*g/cm3, 2);
    fTeflonMaterial->AddElement(nist->FindOrBuildElement("C"), 2);
    fTeflonMaterial->AddElement(nist->FindOrBuildElement("F"), 4);

    fPhotocathodeMaterial = new G4Material("Bialkali", 3.0*g/cm3, 3);
    fPhotocathodeMaterial->AddElement(nist->FindOrBuildElement("K"), 2);
    fPhotocathodeMaterial->AddElement(nist->FindOrBuildElement("Cs"), 1);
    fPhotocathodeMaterial->AddElement(nist->FindOrBuildElement("Sb"), 1);

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

    G4Element* elGd = nist->FindOrBuildElement("Gd");
    fGdLsMaterial = new G4Material("GdLS", 0.865*g/cm3, 2);
    fGdLsMaterial->AddMaterial(fLsMaterial, 99.8*perCent);
    fGdLsMaterial->AddElement(elGd, 0.2*perCent);

    const std::vector<G4double> photonEnergies = {2.0*eV, 2.5*eV, 2.8*eV, 3.1*eV, 3.5*eV};

    auto airMPT = new G4MaterialPropertiesTable();
    airMPT->AddProperty("RINDEX", photonEnergies, std::vector<G4double>(photonEnergies.size(), 1.00));
    fWorldMaterial->SetMaterialPropertiesTable(airMPT);
    
    auto vacuumMPT = new G4MaterialPropertiesTable();
    vacuumMPT->AddProperty("RINDEX", photonEnergies, std::vector<G4double>(photonEnergies.size(), 1.0));
    fVacuumMaterial->SetMaterialPropertiesTable(vacuumMPT);

    auto glassMPT = new G4MaterialPropertiesTable();
    glassMPT->AddProperty("RINDEX", photonEnergies, std::vector<G4double>(photonEnergies.size(), 1.47));
    fGlassMaterial->SetMaterialPropertiesTable(glassMPT);

    auto greaseMPT = new G4MaterialPropertiesTable();
    greaseMPT->AddProperty("RINDEX", photonEnergies, std::vector<G4double>(photonEnergies.size(), 1.45));
    fSiliconeGrease->SetMaterialPropertiesTable(greaseMPT);

    auto lsMPT = new G4MaterialPropertiesTable();
    lsMPT->AddProperty("RINDEX", photonEnergies, std::vector<G4double>(photonEnergies.size(), 1.50));
    std::vector<G4double> ls_emission = {0.1, 0.4, 1.0, 0.4, 0.1};
    lsMPT->AddProperty("SCINTILLATIONCOMPONENT1", photonEnergies, ls_emission);
    lsMPT->AddConstProperty("SCINTILLATIONYIELD", 10000./MeV);
    lsMPT->AddConstProperty("SCINTILLATIONTIMECONSTANT1", 10.*ns);
    lsMPT->AddConstProperty("RESOLUTIONSCALE", 1.0);
    fLsMaterial->SetMaterialPropertiesTable(lsMPT);
    fGdLsMaterial->SetMaterialPropertiesTable(lsMPT);
    
    fTeflonSurface = new G4OpticalSurface("TeflonSurface");
    fTeflonSurface->SetType(dielectric_dielectric);
    fTeflonSurface->SetModel(unified);
    fTeflonSurface->SetFinish(ground);
    auto teflonMPT = new G4MaterialPropertiesTable();
    teflonMPT->AddProperty("REFLECTIVITY", photonEnergies, std::vector<G4double>(photonEnergies.size(), 0.99));
    fTeflonSurface->SetMaterialPropertiesTable(teflonMPT);
}

G4VPhysicalVolume* DetectorConstruction::Construct()
{
    auto solidWorld = new G4Box("SolidWorld", kWorldX/2, kWorldY/2, kWorldZ/2);
    auto logicWorld = new G4LogicalVolume(solidWorld, fWorldMaterial, "LogicWorld");
    auto physWorld = new G4PVPlacement(nullptr, G4ThreeVector(), logicWorld, "PhysWorld", nullptr, false, 0, true);
    logicWorld->SetVisAttributes(G4VisAttributes::GetInvisible());

    G4LogicalVolume* logicSegment = ConstructSegment();
    G4LogicalVolume* logicPMT = ConstructPMT();
    
    auto logicGrease = new G4LogicalVolume(new G4Tubs("SolidGrease", 0, kPmtFaceRadius, kGreaseThickness/2.0, 0, CLHEP::twopi),
                                           fSiliconeGrease, "LogicGrease");
    logicGrease->SetVisAttributes(new G4VisAttributes(G4Colour(0.8, 0.8, 0.8, 0.2)));

    G4double pitchX = kSegmentWidth + 5.0*cm;
    G4double pitchY = kSegmentHeight + 5.0*cm;

    for (G4int j = 0; j < kNy; ++j) {
        for (G4int i = 0; i < kNx; ++i) {
            G4double x = (i - (kNx-1)/2.0) * pitchX;
            G4double y = (j - (kNy-1)/2.0) * pitchY;
            G4int copyNo = j * kNx + i;

            new G4PVPlacement(nullptr, G4ThreeVector(x, y, 0), logicSegment, "PhysSegment", logicWorld, false, copyNo, true);
            
            G4double grease_pos_z = kSegmentLength/2.0 + kGreaseThickness/2.0;
            G4double pmt_pos_z = kSegmentLength/2.0 + kGreaseThickness + kPmtHeight;
            
            auto rot_180_y = new G4RotationMatrix();
            rot_180_y->rotateY(180.0*deg);

            // +z 방향 PMT (머리가 -z를 향하도록 180도 회전)
            new G4PVPlacement(rot_180_y, G4ThreeVector(x, y, pmt_pos_z), logicPMT, "PhysPMT", logicWorld, false, copyNo*2, true);
            new G4PVPlacement(rot_180_y, G4ThreeVector(x, y, grease_pos_z), logicGrease, "PhysGrease", logicWorld, false, copyNo*2, true);

            // -z 방향 PMT (머리가 +z를 향하도록 회전 없음)
            new G4PVPlacement(nullptr, G4ThreeVector(x, y, -pmt_pos_z), logicPMT, "PhysPMT", logicWorld, false, copyNo*2+1, true);
            new G4PVPlacement(nullptr, G4ThreeVector(x, y, -grease_pos_z), logicGrease, "PhysGrease", logicWorld, false, copyNo*2+1, true);
        }
    }
    return physWorld;
}

G4LogicalVolume* DetectorConstruction::ConstructPMT()
{
    const G4int num_z_planes = 4;
    G4double z_planes[num_z_planes] = {
        0.0,
        kPmtNeckLength,
        kPmtNeckLength + kPmtTransitionLength,
        kPmtHeight
    };
    G4double r_inner_zeros[num_z_planes] = {0,0,0,0};
    
    G4double r_outer_glass[num_z_planes] = {
        kPmtNeckRadius,
        kPmtNeckRadius,
        kPmtFaceRadius,
        kPmtFaceRadius
    };
    
    auto solidPmtGlass = new G4Polycone("SolidPmtGlass", 0, CLHEP::twopi, num_z_planes, z_planes, r_inner_zeros, r_outer_glass);
    auto logicPmtGlass = new G4LogicalVolume(solidPmtGlass, fGlassMaterial, "LogicPmtGlass");
    logicPmtGlass->SetVisAttributes(new G4VisAttributes(G4Colour(0.0, 1.0, 1.0, 0.05)));

    G4double r_outer_vacuum[num_z_planes];
    for(int i=0; i<num_z_planes; ++i) {
      r_outer_vacuum[i] = std::max(0.0, r_outer_glass[i] - kPmtGlassThickness);
    }
    
    auto solidPmtVacuum = new G4Polycone("SolidPmtVacuum", 0, CLHEP::twopi, num_z_planes, z_planes, r_inner_zeros, r_outer_vacuum);
    auto logicPmtVacuum = new G4LogicalVolume(solidPmtVacuum, fVacuumMaterial, "LogicPmtVacuum");
    logicPmtVacuum->SetVisAttributes(new G4VisAttributes(G4Colour::White()));
    new G4PVPlacement(nullptr, G4ThreeVector(), logicPmtVacuum, "PhysPmtVacuum", logicPmtGlass, false, 0, true);

    G4double vac_neck_radius = kPmtNeckRadius - kPmtGlassThickness;
    auto solidPhotocathode = new G4Tubs("SolidPhotocathode", 0, vac_neck_radius, kPhotocathodeThickness/2.0, 0, CLHEP::twopi);
    fLogicPhotocathode = new G4LogicalVolume(solidPhotocathode, fPhotocathodeMaterial, "LogicPhotocathode");
    fLogicPhotocathode->SetVisAttributes(new G4VisAttributes(G4Colour::Red()));
    
    G4double pc_pos_z = kPmtNeckLength + kPhotocathodeThickness/2.0;
    new G4PVPlacement(nullptr, G4ThreeVector(0,0,pc_pos_z), fLogicPhotocathode, "PhysPhotocathode", logicPmtVacuum, false, 0, true);
    
    auto photocathode_opsurf = new G4OpticalSurface("Photocathode_OpSurface");
    photocathode_opsurf->SetType(dielectric_metal);
    photocathode_opsurf->SetModel(unified);
    photocathode_opsurf->SetFinish(polished);
    
    const std::vector<G4double> energies_qe = {2.0*eV, 2.5*eV, 2.8*eV, 3.1*eV, 3.5*eV};
    const std::vector<G4double> efficiency = {0.15, 0.22, 0.28, 0.22, 0.10};
    auto pmtMPT = new G4MaterialPropertiesTable();
    pmtMPT->AddProperty("EFFICIENCY", energies_qe, efficiency);
    fPhotocathodeMaterial->SetMaterialPropertiesTable(pmtMPT);
    photocathode_opsurf->SetMaterialPropertiesTable(pmtMPT);
    new G4LogicalSkinSurface("Photocathode_SkinSurface", fLogicPhotocathode, photocathode_opsurf);

    return logicPmtGlass;
}

G4LogicalVolume* DetectorConstruction::ConstructSegment()
{
    auto solidOuterPmma = new G4Box("SolidOuterPmma", kSegmentWidth/2, kSegmentHeight/2, kSegmentLength/2);
    auto logicOuterPmma = new G4LogicalVolume(solidOuterPmma, fPmmaMaterial, "LogicOuterPmma");
    logicOuterPmma->SetVisAttributes(new G4VisAttributes(G4Colour(0.0, 1.0, 1.0, 0.1)));

    G4double outerLS_width = kSegmentWidth - 2 * kOuterPmmaThickness;
    G4double outerLS_height = kSegmentHeight - 2 * kOuterPmmaThickness;
    G4double outerLS_length = kSegmentLength - 2 * kOuterPmmaThickness;
    auto solidOuterLS = new G4Box("SolidOuterLS", outerLS_width/2, outerLS_height/2, outerLS_length/2);
    fLogicLS_outer = new G4LogicalVolume(solidOuterLS, fLsMaterial, "LogicLS_outer");
    fLogicLS_outer->SetVisAttributes(new G4VisAttributes(G4Colour(1.0, 1.0, 0.0, 0.2)));
    new G4PVPlacement(nullptr, G4ThreeVector(0,0,0), fLogicLS_outer, "PhysLS_outer", logicOuterPmma, false, 0, true);

    auto solidInnerPmma = new G4Box("SolidInnerPmma", kInnerContainerWidth/2, kInnerContainerHeight/2, kInnerContainerLength/2);
    auto logicInnerPmma = new G4LogicalVolume(solidInnerPmma, fPmmaMaterial, "LogicInnerPmma");
    logicInnerPmma->SetVisAttributes(new G4VisAttributes(G4Colour(0.0, 0.8, 0.8, 0.3)));
    new G4PVPlacement(nullptr, G4ThreeVector(0,0,0), logicInnerPmma, "PhysInnerPmma", fLogicLS_outer, false, 0, true);

    G4double innerLS_width = kInnerContainerWidth - 2 * kInnerPmmaThickness;
    G4double innerLS_height = kInnerContainerHeight - 2 * kInnerPmmaThickness;
    G4double innerLS_length = kInnerContainerLength - 2 * kInnerPmmaThickness;
    auto solidInnerLS = new G4Box("SolidInnerLS", innerLS_width/2, innerLS_height/2, innerLS_length/2);
    fLogicLS_inner = new G4LogicalVolume(solidInnerLS, fGdLsMaterial, "LogicLS_inner");
    fLogicLS_inner->SetVisAttributes(new G4VisAttributes(G4Colour(1.0, 0.5, 0.0, 0.4)));
    new G4PVPlacement(nullptr, G4ThreeVector(0,0,0), fLogicLS_inner, "PhysLS_inner", logicInnerPmma, false, 0, true);
    
    G4double pillar_length = (outerLS_height - kInnerContainerHeight) / 2.0;
    auto solidPillar = new G4Tubs("SolidPillar", 0, kPillarRadius, pillar_length/2, 0, CLHEP::twopi);
    auto logicPillar = new G4LogicalVolume(solidPillar, fPmmaMaterial, "LogicPillar");
    logicPillar->SetVisAttributes(new G4VisAttributes(G4Colour(0.0, 1.0, 1.0, 0.2)));

    auto pillar_rot = new G4RotationMatrix();
    pillar_rot->rotateX(90.0 * deg);

    G4double pillar_y_pos = kInnerContainerHeight/2 + pillar_length/2;
    G4double start_z = -kInnerContainerLength/2 + 20*cm;
    G4double end_z = kInnerContainerLength/2 - 20*cm;
    G4double z_spacing = (end_z - start_z) / (kPillarCount - 1);
    
    for (G4int i=0; i<kPillarCount; ++i)
    {
        G4double pillar_z_pos = start_z + i*z_spacing;
        G4ThreeVector pos;
        if (i % 2 == 0) {
            pos = G4ThreeVector(0, pillar_y_pos, pillar_z_pos);
        } else {
            pos = G4ThreeVector(0, -pillar_y_pos, pillar_z_pos);
        }
        new G4PVPlacement(pillar_rot, pos, logicPillar, "PhysPillar", fLogicLS_outer, false, i, true);
    }

    return logicOuterPmma;
}

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
