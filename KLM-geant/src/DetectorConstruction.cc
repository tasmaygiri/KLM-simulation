#include "DetectorConstruction.hh"
#include "MylarSD.hh" // Will create this later

#include "G4NistManager.hh"
#include "G4Material.hh"
#include "G4Element.hh"
#include "G4Box.hh"
#include "G4Polyhedra.hh"
#include "G4LogicalVolume.hh"
#include "G4PVPlacement.hh"
#include "G4RotationMatrix.hh"
#include "G4ThreeVector.hh"
#include "G4SystemOfUnits.hh"
#include "G4PhysicalConstants.hh"
#include "G4VisAttributes.hh"
#include "G4Colour.hh"
#include "G4LogicalVolumeStore.hh"
#include "G4SolidStore.hh"
#include "G4PhysicalVolumeStore.hh"
#include "G4SDManager.hh"       // For SD management
#include "G4RunManager.hh"
#include "G4UnitsTable.hh"
#include "G4GeometryManager.hh"
#include <numeric> // For std::accumulate if needed, though manual sum is fine

// Constructor: Initialize material pointers and calculate fRPCStackThickness
DetectorConstruction::DetectorConstruction()
: G4VUserDetectorConstruction(),
  fWorldMaterial(nullptr), fIronMaterial(nullptr),
  fMylarMaterial(nullptr), fCopperMaterial(nullptr), fFoamMaterial(nullptr),
  fRPCGasMaterial(nullptr), fGlassMaterial(nullptr),
  fWorldPV(nullptr)
{
    // Calculate total thickness of one RPC superlayer stack based on component thicknesses
    fRPCStackThickness = (t_Mylar_GP_CP * 2) * 2 +  // 2x Mylar in 2x GP/CP structures
                         (t_Copper_GP_CP * 2) * 2 + // 2x Copper in 2x GP/CP structures
                         (t_Foam * 2) +             // 2x Foam layers
                         (t_HV_Region_Glass * 4) +  // 4x Glass layers
                         (t_GasGap * 2) +           // 2x Gas Gaps
                         t_Mylar_Insulator;         // 1x Insulator Mylar
    G4cout << "Calculated fRPCStackThickness: " << G4BestUnit(fRPCStackThickness, "Length") << G4endl;
}

DetectorConstruction::~DetectorConstruction()
{
}

// DefineMaterials based on user's provided code
void DetectorConstruction::DefineMaterials()
{
  G4NistManager* nist = G4NistManager::Instance();
  G4double density;

  fWorldMaterial = nist->FindOrBuildMaterial("G4_Galactic");
  fIronMaterial = nist->FindOrBuildMaterial("G4_Fe");
  G4cout << "Iron Material: " << fIronMaterial << G4endl;

  // === Materials for RPC Superlayer Components ===
  G4Element* elH = nist->FindOrBuildElement("H");
  G4Element* elC = nist->FindOrBuildElement("C");
  G4Element* elF = nist->FindOrBuildElement("F");
  G4Material* matAr = nist->FindOrBuildMaterial("G4_Ar");

  G4Material* matNButane = new G4Material("RPC_NButane", 2.48*kg/m3, 2, kStateGas, STP_Temperature, STP_Pressure); // Corrected density from user
  matNButane->AddElement(elC, 4); matNButane->AddElement(elH, 10);
  G4Material* matIsoButane = new G4Material("RPC_IsoButane", 2.51*kg/m3, 2, kStateGas, STP_Temperature, STP_Pressure);
  matIsoButane->AddElement(elC, 4); matIsoButane->AddElement(elH, 10);

  G4double densityNButane = 2.48*kg/m3; // Store for calculation
  G4double densityIsoButane = 2.51*kg/m3;
  G4double densityButaneSilver = 1.0 / (0.70 / densityNButane + 0.30 / densityIsoButane); // Recalculate with stored densities

  G4Material* matButaneSilver = new G4Material("RPC_ButaneSilver", densityButaneSilver, 2, kStateGas, STP_Temperature, STP_Pressure);
  matButaneSilver->AddMaterial(matNButane, 0.70); // Assuming mass fractions
  matButaneSilver->AddMaterial(matIsoButane, 0.30); // Assuming mass fractions

  G4Material* matHFC134a = new G4Material("RPC_HFC134a", 4.25*kg/m3, 3, kStateGas, STP_Temperature, STP_Pressure);
  matHFC134a->AddElement(elC, 2); matHFC134a->AddElement(elH, 2); matHFC134a->AddElement(elF, 4);

  G4double molarMassHFC = 102.03*g/mole;
  G4double molarMassAr = matAr->GetA(); // Already in G4 internal units
  G4double molarMassBS = 58.12*g/mole;

  G4double moleFracHFC = 0.62; G4double moleFracAr = 0.30; G4double moleFracBS = 0.08;
  G4double avgMolarMassRPC = moleFracHFC*molarMassHFC + moleFracAr*molarMassAr + moleFracBS*molarMassBS;

  // Using Ideal Gas Law: PV = nRT => density = m/V = (n*M)/(nRT/P) = PM/RT
  G4double R_ideal_gas = Avogadro * k_Boltzmann; // Ideal gas constant J/(K*mol)
  G4double densityRPCGas = (STP_Pressure * avgMolarMassRPC) / (R_ideal_gas * STP_Temperature);

  G4Material* matRPCGas_local = new G4Material("RPCGas", densityRPCGas, 3, kStateGas, STP_Temperature, STP_Pressure);
  matRPCGas_local->AddMaterial(matHFC134a, (moleFracHFC * molarMassHFC) / avgMolarMassRPC);
  matRPCGas_local->AddMaterial(matAr,      (moleFracAr * molarMassAr) / avgMolarMassRPC);
  matRPCGas_local->AddMaterial(matButaneSilver, (moleFracBS * molarMassBS) / avgMolarMassRPC);
  fRPCGasMaterial = matRPCGas_local; // Assign to member
  G4cout << "RPC Gas Material Defined: " << fRPCGasMaterial << " with density " << G4BestUnit(densityRPCGas, "Volumic Mass") << G4endl;

  fMylarMaterial = nist->FindOrBuildMaterial("G4_MYLAR");
  G4cout << "Mylar Material: " << fMylarMaterial << G4endl;
  fCopperMaterial = nist->FindOrBuildMaterial("G4_Cu");
  G4cout << "Copper Material: " << fCopperMaterial << G4endl;

  density = 0.05 * g/cm3; // Foam density (ASSUMPTION)
  G4Material* matFoam_local = new G4Material("DielectricFoam", density, 1);
  matFoam_local->AddMaterial(nist->FindOrBuildMaterial("G4_POLYSTYRENE"), 1.0);
  fFoamMaterial = matFoam_local; // Assign to member
  G4cout << "Foam Material (Assumed Polystyrene): " << fFoamMaterial << G4endl;

  G4Element* elSi = nist->FindOrBuildElement("Si"); G4Element* elO  = nist->FindOrBuildElement("O");
  G4Element* elNa = nist->FindOrBuildElement("Na"); G4Element* elCa = nist->FindOrBuildElement("Ca");
  density = 2.5 * g/cm3; // Float glass density
  G4Material* matGlass_local = new G4Material("FloatGlass", density, 4);
  G4double massSiO2 = 0.73; G4double massNa2O = 0.14; G4double massCaO = 0.09;
  G4double M_Si_val = elSi->GetA()/(g/mole); G4double M_O_val = elO->GetA()/(g/mole); // GetA() returns in internal units
  G4double M_Na_val = elNa->GetA()/(g/mole); G4double M_Ca_val = elCa->GetA()/(g/mole);
  G4double M_SiO2 = M_Si_val + 2*M_O_val; G4double M_Na2O = 2*M_Na_val + M_O_val; G4double M_CaO = M_Ca_val + M_O_val;
  G4double totalMassCompounds = massSiO2 + massNa2O + massCaO; // Should be 0.73+0.14+0.09 = 0.96
  // Normalize component masses as they don't sum to 1 (due to 4% other)
  massSiO2 /= totalMassCompounds; massNa2O /= totalMassCompounds; massCaO /= totalMassCompounds;

  G4double fracSi = massSiO2 * (M_Si_val / M_SiO2);
  G4double fracNa = massNa2O * (2 * M_Na_val / M_Na2O);
  G4double fracCa = massCaO * (M_Ca_val / M_CaO);
  G4double fracO  = massSiO2 * (2 * M_O_val / M_SiO2) +
                    massNa2O * (M_O_val / M_Na2O) +
                    massCaO * (M_O_val / M_CaO);
  // Normalize element fractions to sum to 1.0
  G4double totalElementFrac = fracSi + fracNa + fracCa + fracO;
  matGlass_local->AddElement(elSi, fracSi/totalElementFrac);
  matGlass_local->AddElement(elO,  fracO/totalElementFrac);
  matGlass_local->AddElement(elNa, fracNa/totalElementFrac);
  matGlass_local->AddElement(elCa, fracCa/totalElementFrac);
  fGlassMaterial = matGlass_local; // Assign to member
  G4cout << "Glass Material: " << fGlassMaterial << G4endl;

  G4cout << "\n--- Final Defined Materials List ---" << G4endl;
  G4cout << *(G4Material::GetMaterialTable()) << G4endl;
  G4cout << "------------------------------------" << G4endl;
}

// Helper GetKLMSectorLayerLogical remains the same
G4LogicalVolume* DetectorConstruction::GetKLMSectorLayerLogical(
    const G4String& name, G4double innerRadius, G4double outerRadius, G4double halfLength,
    G4double phiStart, G4double phiTotal, G4Material* material)
{
  const G4int numZPlanes = 2;
  G4double zPlane[numZPlanes] = {-halfLength, halfLength};
  G4double rInner[numZPlanes] = {innerRadius, innerRadius};
  G4double rOuter[numZPlanes] = {outerRadius, outerRadius};

  G4Polyhedra* solidLayer = new G4Polyhedra(name + "_Solid",
                                            phiStart, phiTotal,
                                            1, // For a single segment/wedge
                                            numZPlanes, zPlane, rInner, rOuter);

  G4LogicalVolume* logicLayer = new G4LogicalVolume(solidLayer, material, name + "_Log"); // Added _Log for clarity
  return logicLayer;
}


// --- Construct method with Detailed RPC Stack ---
G4VPhysicalVolume* DetectorConstruction::Construct()
{
  DefineMaterials(); // Define all materials first

  // --- Basic KLM Parameters ---
  G4double klmInnerRadius = fKLMBarrelInnerRadius;
  // klmOuterRadius will be calculated dynamically based on detailed stack
  G4double klmHalfLength = fKLMBarrelHalfLength;
  G4double klmSectorAngle = (360.0/fKLMBarrelNumSides) * deg;

  // --- World Volume ---
  // Calculate estimated outer radius for world volume sizing
  G4double estimatedOuterRadius = klmInnerRadius + fNbDetectorLayers * (2 * fRPCStackThickness + fIronThickness);
  G4double worldSizeXY = estimatedOuterRadius * 5;
  G4double worldSizeZ  = klmHalfLength * 5;

  G4Box* solidWorld = new G4Box("WorldSolid", worldSizeXY, worldSizeXY, worldSizeZ);
  G4LogicalVolume* logicWorld = new G4LogicalVolume(solidWorld, fWorldMaterial, "WorldLog");
  fWorldPV = new G4PVPlacement(0, G4ThreeVector(), logicWorld, "World", 0, false, 0, true);

  // --- Visualization Attributes for Sublayers ---
  G4VisAttributes* visAttMylar  = new G4VisAttributes(G4Colour(0.9, 0.9, 0.2, 0.5)); // Yellowish
  visAttMylar->SetForceSolid(true);
  G4VisAttributes* visAttCopper = new G4VisAttributes(G4Colour(0.7, 0.4, 0.1, 0.7)); // Brownish-Copper
  visAttCopper->SetForceSolid(true);
  G4VisAttributes* visAttFoam   = new G4VisAttributes(G4Colour(0.8, 0.8, 0.8, 0.3)); // Light Grey
  visAttFoam->SetForceSolid(true);
  G4VisAttributes* visAttGas    = new G4VisAttributes(G4Colour(0.5, 0.8, 1.0, 0.2)); // Light Blue/Cyan
  visAttGas->SetForceSolid(true);
  G4VisAttributes* visAttGlass  = new G4VisAttributes(G4Colour(0.6, 0.6, 0.9, 0.4)); // Light Purple/Blue
  visAttGlass->SetForceSolid(true);
  G4VisAttributes* visAttIron   = new G4VisAttributes(G4Colour(0.5, 0.5, 0.5, 0.7)); // Grey
  visAttIron->SetForceSolid(true);

  // --- KLM Sector Mother (common for all sectors) ---
  // This mother volume's radial extent will be determined by the full stack
  // We create one logical mother and place it 8 times with rotations
  // The radial extent for the solid is just an envelope, layers are placed within.
  G4double sectorMotherInnerR = klmInnerRadius;
  G4double sectorMotherOuterR = estimatedOuterRadius; // Envelope

  const G4int numZPlanes_sector = 2;
  G4double zPlane_sector[numZPlanes_sector] = {-klmHalfLength, klmHalfLength};
  G4double rInner_sector[numZPlanes_sector] = {sectorMotherInnerR, sectorMotherInnerR};
  G4double rOuter_sector[numZPlanes_sector] = {sectorMotherOuterR, sectorMotherOuterR};

  G4Polyhedra* solidKLMSectorMother = new G4Polyhedra("KLMSectorMotherSolid",
                                                      -klmSectorAngle/2.0, klmSectorAngle,
                                                      1, // For a single segment/wedge
                                                      numZPlanes_sector, zPlane_sector,
                                                      rInner_sector, rOuter_sector);
  G4LogicalVolume* logicKLMSectorMother = new G4LogicalVolume(solidKLMSectorMother,
                                                              fWorldMaterial, // Filled with world material
                                                              "KLMSectorMotherLog");
  logicKLMSectorMother->SetVisAttributes(G4VisAttributes::GetInvisible());

  // --- Loop to build fNbDetectorLayers of (RPC Stack + Iron) ---
  G4double currentRadialPosition = klmInnerRadius; // Starting radius for the first layer

  for (G4int iStack = 0; iStack < fNbDetectorLayers; ++iStack)
  {
    G4cout << "Building RPC Stack " << iStack << " starting at R = "
           << G4BestUnit(currentRadialPosition, "Length") << G4endl;
    G4double rpcStackStartR = currentRadialPosition;

    // Define helper lambda to place a sublayer
    auto PlaceSublayer = [&](const G4String& namePrefix, G4double thickness, G4Material* mat, G4VisAttributes* visAtt, G4int subLayerID) {
        G4String volName = namePrefix + "_S" + std::to_string(iStack); // Unique name
        G4LogicalVolume* logicSub = GetKLMSectorLayerLogical(
            volName, currentRadialPosition, currentRadialPosition + thickness, klmHalfLength,
            -klmSectorAngle/2.0, klmSectorAngle, mat);
        logicSub->SetVisAttributes(visAtt);
        new G4PVPlacement(0, G4ThreeVector(), logicSub, volName + "_PV",
                          logicKLMSectorMother, false, iStack * 100 + subLayerID, true); // Unique copyNo
        currentRadialPosition += thickness;
    };

    // --- Build one RPC Superlayer Stack (following Fig 10.2 from outside in, then mirrored) ---
    // This is one "Detector Layer" which is an RPC superlayer
    // Outer Ground Plane
    PlaceSublayer("OuterGPMylar", t_Mylar_GP_CP, fMylarMaterial, visAttMylar, 0);
    PlaceSublayer("OuterGPCu", t_Copper_GP_CP, fCopperMaterial, visAttCopper, 1);
    // Outer Foam
    PlaceSublayer("OuterFoam", t_Foam, fFoamMaterial, visAttFoam, 2);
    // Outer Cathode Plane
    PlaceSublayer("OuterCPCu", t_Copper_GP_CP, fCopperMaterial, visAttCopper, 3);
    PlaceSublayer("OuterCPMylar", t_Mylar_GP_CP, fMylarMaterial, visAttMylar, 4);
    
    // Outer RPC unit (Glass - Gas - Glass)
    PlaceSublayer("OuterGlass1", t_HV_Region_Glass, fGlassMaterial, visAttGlass, 5); // This is -HV
    PlaceSublayer("OuterGas", t_GasGap, fRPCGasMaterial, visAttGas, 6);
    PlaceSublayer("OuterGlass2", t_HV_Region_Glass, fGlassMaterial, visAttGlass, 7); // This is +HV
    // Central Insulator
    PlaceSublayer("InsulatorMylar", t_Mylar_Insulator, fMylarMaterial, visAttMylar, 8);
    // Inner RPC unit (Glass - Gas - Glass)
    PlaceSublayer("InnerGlass1", t_HV_Region_Glass, fGlassMaterial, visAttGlass, 9);   // This is +HV
    PlaceSublayer("InnerGas", t_GasGap, fRPCGasMaterial, visAttGas, 10);
    PlaceSublayer("InnerGlass2", t_HV_Region_Glass, fGlassMaterial, visAttGlass, 11);  // This is -HV
    // Inner Cathode Plane
    PlaceSublayer("InnerCPMylar", t_Mylar_GP_CP, fMylarMaterial, visAttMylar, 12);
    PlaceSublayer("InnerCPCu", t_Copper_GP_CP, fCopperMaterial, visAttCopper, 13);
    // Inner Foam
    PlaceSublayer("InnerFoam", t_Foam, fFoamMaterial, visAttFoam, 14);
    // Inner Ground Plane
    PlaceSublayer("InnerGPCu", t_Copper_GP_CP, fCopperMaterial, visAttCopper, 15);
    PlaceSublayer("InnerGPMylar", t_Mylar_GP_CP, fMylarMaterial, visAttMylar, 16);
    

    // Check calculated thickness against fRPCStackThickness
    G4double builtStackThickness = currentRadialPosition - rpcStackStartR;
    if (std::abs(builtStackThickness - fRPCStackThickness) > 0.01 * mm) {
        G4cerr << "WARNING: Built RPC Stack " << iStack << " thickness "
               << G4BestUnit(builtStackThickness, "Length")
               << " differs from expected fRPCStackThickness "
               << G4BestUnit(fRPCStackThickness, "Length") << G4endl;
    }
    // Ensure currentRadialPosition reflects the defined stack thickness for placement of next iron
    currentRadialPosition = rpcStackStartR + fRPCStackThickness;

    PlaceSublayer("OuterGPMylar2", t_Mylar_GP_CP, fMylarMaterial, visAttMylar, 17);
    PlaceSublayer("OuterGPCu2", t_Copper_GP_CP, fCopperMaterial, visAttCopper, 18);
    // Outer Foam
    PlaceSublayer("OuterFoam2", t_Foam, fFoamMaterial, visAttFoam, 19);
    // Outer Cathode Plane
    PlaceSublayer("OuterCPCu2", t_Copper_GP_CP, fCopperMaterial, visAttCopper, 20);
    PlaceSublayer("OuterCPMylar2", t_Mylar_GP_CP, fMylarMaterial, visAttMylar, 21);
    
    // Outer RPC unit (Glass - Gas - Glass)
    PlaceSublayer("OuterGlass12", t_HV_Region_Glass, fGlassMaterial, visAttGlass, 22); // This is -HV
    PlaceSublayer("OuterGassecond", t_GasGap, fRPCGasMaterial, visAttGas, 23);
    PlaceSublayer("OuterGlass22", t_HV_Region_Glass, fGlassMaterial, visAttGlass, 24); // This is +HV
    // Central Insulator
    PlaceSublayer("InsulatorMylar2", t_Mylar_Insulator, fMylarMaterial, visAttMylar, 25);
    // Inner RPC unit (Glass - Gas - Glass)
    PlaceSublayer("InnerGlass12", t_HV_Region_Glass, fGlassMaterial, visAttGlass, 26);   // This is +HV
    PlaceSublayer("InnerGassecond", t_GasGap, fRPCGasMaterial, visAttGas, 27);
    PlaceSublayer("InnerGlass22", t_HV_Region_Glass, fGlassMaterial, visAttGlass, 28);  // This is -HV
    // Inner Cathode Plane
    PlaceSublayer("InnerCPMylar2", t_Mylar_GP_CP, fMylarMaterial, visAttMylar, 29);
    PlaceSublayer("InnerCPCu2", t_Copper_GP_CP, fCopperMaterial, visAttCopper, 30);
    // Inner Foam
    PlaceSublayer("InnerFoam2", t_Foam, fFoamMaterial, visAttFoam, 31);
    // Inner Ground Plane
    PlaceSublayer("InnerGPCu2", t_Copper_GP_CP, fCopperMaterial, visAttCopper, 32);
    PlaceSublayer("InnerGPMylar2", t_Mylar_GP_CP, fMylarMaterial, visAttMylar, 33);

    // --- Place Iron layer if applicable ---
    if (iStack < fNbIronLayers) {
        G4cout << "Building Iron Layer " << iStack << " after RPC Stack, starting at R = "
               << G4BestUnit(currentRadialPosition, "Length") << G4endl;
        G4String ironName = "IronLayer_S" + std::to_string(iStack);
        G4LogicalVolume* logicIron = GetKLMSectorLayerLogical(
            ironName, currentRadialPosition, currentRadialPosition + fIronThickness, klmHalfLength,
            -klmSectorAngle/2.0, klmSectorAngle, fIronMaterial);
        logicIron->SetVisAttributes(visAttIron);
        new G4PVPlacement(0, G4ThreeVector(), logicIron, ironName + "_PV",
                          logicKLMSectorMother, false, iStack, true); // Simpler copyNo for iron
        currentRadialPosition += fIronThickness;
    }
    G4cout << "Stack " << iStack << " finished. Current Radial Position: "
           << G4BestUnit(currentRadialPosition, "Length") << G4endl;
  } // End of loop over fNbDetectorLayers

  // --- Place the 8 KLM Sector Mother Volumes in the World ---
  G4double finalOuterRadius = currentRadialPosition; // Actual outer radius after construction
  G4cout << "\n--- Placing " << fKLMBarrelNumSides << " KLM Sectors into World ---" << G4endl;
  G4cout << "Final outer radius of constructed layers: " << G4BestUnit(finalOuterRadius, "Length") << G4endl;

  // If the sector mother solid was just an envelope, it's fine.
  // If it needs to be tight, it could be redefined here, but layers are placed relative to its origin.

  for (G4int iSector = 0; iSector < fKLMBarrelNumSides; ++iSector) {
    G4double phi = iSector * klmSectorAngle;
    G4RotationMatrix* rotation = new G4RotationMatrix();
    rotation->rotateZ(phi);
    // The same logicKLMSectorMother is placed multiple times,
    // its children (the sublayers) are already defined relative to its center.
    new G4PVPlacement(rotation, G4ThreeVector(), // Place at world origin, then rotate
                      logicKLMSectorMother,
                      "KLMSectorPV_" + std::to_string(iSector),
                      logicWorld, false, iSector, true);
    G4cout << "Placed KLM Sector " << iSector << " at Phi = " << phi/deg << " deg" << G4endl;
  }

  // --- Print Final Geometry Summary ---
  G4cout << "\n------------------------------------------------------------" << G4endl;
  G4cout << "---> Detailed Detector Geometry built:" << G4endl;
  G4cout << "     KLM Inner Radius          : " << G4BestUnit(fKLMBarrelInnerRadius,"Length") << G4endl;
  G4cout << "     KLM Actual Outer Radius   : " << G4BestUnit(finalOuterRadius,"Length") << G4endl;
  G4cout << "     KLM Half Length           : " << G4BestUnit(klmHalfLength,"Length") << G4endl;
  G4cout << "     Number of Sectors         : " << fKLMBarrelNumSides << G4endl;
  G4cout << "     Number of RPC Stacks      : " << fNbDetectorLayers << G4endl;
  G4cout << "     Number of Iron Plates     : " << fNbIronLayers << G4endl;
  G4cout << "     Calculated RPC Stack Thick: " << G4BestUnit(fRPCStackThickness,"Length") << G4endl;
  G4cout << "     Iron Plate Thickness      : " << G4BestUnit(fIronThickness,"Length") << G4endl;
  G4cout << "     Mylar Grid (Phi x Z)      : " << fNumPhiCells_MylarGrid06  << " for 0-6 layers "<< fNumPhiCells_MylarGrid714 << " for 7-14 layers " << " x " << fNumZCells_MylarGrid << G4endl;
  G4cout << "------------------------------------------------------------" << G4endl;

  return fWorldPV;
}

#include "DetectorConstruction.hh"
#include "MylarSD.hh" // For MylarSD
#include "G4SDManager.hh"
#include "G4LogicalVolumeStore.hh"
#include "G4LogicalVolume.hh" // For G4LogicalVolume
#include "G4Material.hh"     // For G4Material
// ... other necessary includes for geometry ...
#include "G4NistManager.hh"
#include "G4Box.hh"
#include "G4Polyhedra.hh"
#include "G4PVPlacement.hh"
#include "G4RotationMatrix.hh"
#include "G4VisAttributes.hh"
#include "G4Colour.hh"
#include "G4SystemOfUnits.hh"
#include "G4UnitsTable.hh"
#include "G4GeometryManager.hh"
// #include "G4StrUtil.hh"       // <<< ADDED for G4StrUtil::contains

// ... (rest of DetectorConstruction.hh and .cc up to ConstructSDandField) ...

void DetectorConstruction::ConstructSDandField()
{
  G4cout << "\nDetectorConstruction::ConstructSDandField() called." << G4endl;

  // Assuming MylarSD constructor is: MylarSD(const G4String& name, const G4String& hitsCollectionName, DetectorConstruction* det)
  // If it changed to just MylarSD(const G4String& name) because EventAction handles hits,
  // then the MylarSD.hh/cc would need to reflect that (no hitsCollectionName, no detConstruction ptr).
  // Based on your MylarSD.cc it seems it still takes 3 args.
  MylarSD* mylarSD = new MylarSD("KLM/MylarSD", "MylarHitsCollection", this);
  G4SDManager::GetSDMpointer()->AddNewDetector(mylarSD);
  G4cout << "MylarSD instance created and added to SDManager." << G4endl;

  std::vector<std::string> mylarLogVolBaseNames = {
    "InnerGas_Log", "OuterGas_Log",
    // "InnerGassecond_Log", "OuterGassecond_Log"
    //   "OuterGPMylar_Log", "OuterCPMylar_Log",
    //   "InnerCPMylar_Log", "InnerGPMylar_Log"
      // Add other base names for Mylar LVs if you have them, e.g., "InsulatorMylar_Log"
  };

  G4LogicalVolumeStore* lvStore = G4LogicalVolumeStore::GetInstance();
  G4int sensitiveMylarVolumesCount = 0;
  if (lvStore) { // Check if lvStore is not null
    for (auto const& lv : *lvStore) {
        if (!lv) continue; // Skip null logical volumes
        const G4String& lvName = lv->GetName();
        for (const auto& baseNameWithSuffix : mylarLogVolBaseNames) {
            // Remove "_Log" from baseNameWithSuffix to get the prefix to check
            std::string prefixToCheck = baseNameWithSuffix;
            size_t suffixPos = prefixToCheck.rfind("_Log");
            if (suffixPos != std::string::npos) {
                prefixToCheck = prefixToCheck.substr(0, suffixPos);
            }

            // Check if lvName starts with the prefix AND ends with "_Log" (or contains it meaningfully)
            if (lvName.rfind(prefixToCheck, 0) == 0 && G4StrUtil::contains(lvName, "_Log")) { // <<< MODIFIED HERE
                G4cout << "Assigning MylarSD to: " << lvName << G4endl;
                SetSensitiveDetector(lvName, mylarSD); // Use G4String lvName directly
                sensitiveMylarVolumesCount++;
                break;
            }
        }
    }
  }

  if (sensitiveMylarVolumesCount == 0) {
      G4cout << "WARNING: No Mylar logical volumes were found to assign the SD. Check naming and base names." << G4endl;
  } else {
      G4cout << "Assigned MylarSD to " << sensitiveMylarVolumesCount << " Mylar logical volumes." << G4endl;
  }
}