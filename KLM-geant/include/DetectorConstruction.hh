#ifndef DETECTORCONSTRUCTION_HH
#define DETECTORCONSTRUCTION_HH

#include "G4VUserDetectorConstruction.hh"
#include "globals.hh"
#include "G4SystemOfUnits.hh" // For units

// Forward declarations
class G4VPhysicalVolume;
class G4LogicalVolume;
class G4Material; // Forward declare G4Material

class DetectorConstruction : public G4VUserDetectorConstruction
{
  public:
    DetectorConstruction();
    virtual ~DetectorConstruction();

    virtual G4VPhysicalVolume* Construct();
    virtual void ConstructSDandField();

    // Public getters for dimensions needed by SD
    G4double GetKLMHalfLength() const { return fKLMBarrelHalfLength; }
    G4double GetKLMSectorAngle() const { return (360.0/fKLMBarrelNumSides) * deg; }
    G4int    GetNumPhiCells06() const { return fNumPhiCells_MylarGrid06; }
    G4int    GetNumPhiCells714() const { return fNumPhiCells_MylarGrid714; }
    G4int    GetNumZCells() const { return fNumZCells_MylarGrid; }


  private:
    void DefineMaterials();

    G4LogicalVolume* GetKLMSectorLayerLogical(const G4String& name,
                                              G4double innerRadius,
                                              G4double outerRadius,
                                              G4double halfLength,
                                              G4double phiStart,
                                              G4double phiTotal,
                                              G4Material* material);
    // --- Material Pointers ---
    G4Material* fWorldMaterial;
    G4Material* fIronMaterial;
    // Materials for detailed RPC Stack
    G4Material* fMylarMaterial;
    G4Material* fCopperMaterial;
    G4Material* fFoamMaterial;
    G4Material* fRPCGasMaterial;
    G4Material* fGlassMaterial;

    // --- Volumes ---
    G4VPhysicalVolume* fWorldPV;

    // --- Parameters from original setup ---
  const G4int fNbIronLayers = 14;
  const G4int fNbDetectorLayers = 15;
  const G4double fIronThickness = 4.7 * cm;
  // const G4double fRPCStackThickness = 31.6 * mm; // Total thickness from Fig 10.2
  const G4double fKLMBarrelInnerRadius = 201.586 * cm; 
  const G4double fKLMBarrelHalfLength = 220.0 * cm;    
  const G4int fKLMBarrelNumSides = 8;

    // --- Parameters for detailed RPC Stack (from Fig 10.2) ---
    // These are individual layer thicknesses
    const G4double t_Mylar_GP_CP = 0.25 * mm;
    const G4double t_Copper_GP_CP = 0.035 * mm;
    const G4double t_Foam = 7.0 * mm;
    const G4double t_HV_Region_Glass = 3.0 * mm; // "HV" region from Fig 10.2, assumed to be Glass
    const G4double t_GasGap = 2.0 * mm;
    const G4double t_Mylar_Insulator = 0.5 * mm;

    // Total thickness of one full RPC Superlayer stack (will be calculated)
    G4double fRPCStackThickness; // Calculated in constructor or DefineMaterials

    // --- Mylar Grid Parameters for SD ---
    const G4int fNumPhiCells_MylarGrid06 = 36;
    const G4int fNumPhiCells_MylarGrid714 = 48;
    const G4int fNumZCells_MylarGrid = 96;
};

#endif