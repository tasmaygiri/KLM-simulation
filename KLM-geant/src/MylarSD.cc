#include "MylarSD.hh"
#include "DetectorConstruction.hh" // To access geometry parameters

#include "G4HCofThisEvent.hh"
#include "G4Step.hh"
#include "G4ThreeVector.hh"
#include "G4SDManager.hh"
#include "G4ios.hh"
#include "G4Track.hh"
#include "G4ParticleDefinition.hh"
#include "G4VTouchable.hh"
#include "G4SystemOfUnits.hh"
#include "G4AffineTransform.hh" // For coordinate transformations
#include "G4VSolid.hh"          // To get solid extents (though less useful for Polyhedra cells)
#include "G4Polyhedra.hh"       // If needed to inspect Polyhedra solid

MylarSD::MylarSD(const G4String& name,
                 const G4String& hitsCollectionName,
                 DetectorConstruction* detConstruction)
 : G4VSensitiveDetector(name),
   fHitsCollection(nullptr),
   fDetConstruction(detConstruction) // Store pointer to detector construction
{
  collectionName.insert(hitsCollectionName); // Register the hits collection name
}

MylarSD::~MylarSD()
{}

// Called at the beginning of each event
void MylarSD::Initialize(G4HCofThisEvent* hce)
{
  // Create hits collection object
  fHitsCollection = new MylarHitsCollection(SensitiveDetectorName, collectionName[0]);

  // Add this collection to the G4HCofThisEvent
  // A G4HCofThisEvent is provided by Geant4 and holds all hits collections for an event
  G4int hcID = G4SDManager::GetSDMpointer()->GetCollectionID(collectionName[0]);
  hce->AddHitsCollection(hcID, fHitsCollection);

  // G4cout << "MylarSD: Initialized hits collection '" << collectionName[0] << "' with ID " << hcID << G4endl;
}

// This method is called by Geant4 for every step in a volume
// associated with this sensitive detector
G4bool MylarSD::ProcessHits(G4Step* aStep, G4TouchableHistory* /*ROhist*/)
{
  // Get energy deposited in this step
  G4double edep = aStep->GetTotalEnergyDeposit();
  if (edep == 0.) return false; // No energy deposited, no hit (or only if particle passes through)

  // Create a new hit
  MylarHit* newHit = new MylarHit();

  // --- Get common particle and step information ---
  G4Track* track = aStep->GetTrack();
  const G4ParticleDefinition* particleDef = track->GetParticleDefinition();
  if (particleDef->GetPDGCharge() == 0.0) { // Check if charge is zero
    return false; // Not a charged particle, do not record a hit
  }

  // G4double v = track->GetVelocity();
  // G4double KE = track->GetKineticEnergy();
  // G4ThreeVector mom_direction = track->GetMomentumDirection();
  G4ThreeVector mom = track->GetMomentum();
  G4double totalMomentum = mom.mag();

  G4double mass = track->GetParticleDefinition()->GetPDGMass();


  if(abs(track->GetParticleDefinition()->GetPDGEncoding()) == 11 and track->GetTrackID() ==1)
  {G4cout << "track ID: " << track->GetTrackID() << " parent ID: " << track->GetParentID() << " PDG ID: " << track->GetParticleDefinition()->GetPDGEncoding() << " Mass: " << mass << " Momentum: " << totalMomentum << G4endl;
  G4cout << "----------------------------------------------------------------------------------------------" << G4endl;
  }

  newHit->SetTrackID(track->GetTrackID());
  newHit->SetParentID(track->GetParentID());
  newHit->SetPDGCode(track->GetParticleDefinition()->GetPDGEncoding());
  newHit->SetParticleName(track->GetParticleDefinition()->GetParticleName());
  // newHit->SetParticleName(track->GetParticleDefinition()->GetPDGMass());
  newHit->SetEnergyDeposited(edep);
  newHit->SetGlobalTime(aStep->GetPostStepPoint()->GetGlobalTime());
  newHit->SetPosition(aStep->GetPostStepPoint()->GetPosition()); // Global position of step end

  // --- Get KLM specific information from the volume ---
  const G4VTouchable* touchable = aStep->GetPreStepPoint()->GetTouchable();
  G4VPhysicalVolume* pv = touchable->GetVolume();
  G4String volumeName = pv->GetLogicalVolume()->GetName();
  newHit->SetVolumeName(volumeName);

  // Determine Sector, Stack, and Mylar Type from Physical Volume's copy numbers or name parsing
  // This depends on how PVs are named and copy numbers are assigned in DetectorConstruction
  G4int sectorNumber = touchable->GetCopyNumber(1); // Assuming Sector is at depth 1 (KLMSectorPV_X)
  newHit->SetSectorNumber(sectorNumber);

  // For Stack and Mylar Type, more complex parsing of pvName or deeper touchable info needed
  // Example: if PV name is "OuterGPMylar_S<stackID>_PV_Sector<sectorID>"
  // Or rely on the copy number set for the sublayer PV in DetectorConstruction
  G4int sublayerCopyNo = touchable->GetCopyNumber(0); // Copy number of the Mylar sublayer PV itself
  G4int stackNumber = sublayerCopyNo / 100; // As per iStack * 100 + subLayerID

  // G4int subLayerID_inStack = sublayerCopyNo % 100; // Could be used to identify specific mylar
  newHit->SetStackNumber(stackNumber);

  // Infer Mylar type from volume name (crude example, make more robust)
  // This relies on the logical volume name set in DetectorConstruction
  if (volumeName.contains("OuterGPMylar")) newHit->SetMylarLayerType("OuterGP");
  else if (volumeName.contains("OuterCPMylar")) newHit->SetMylarLayerType("OuterCP");
  else if (volumeName.contains("InsulatorMylar")) newHit->SetMylarLayerType("Insulator");
  else if (volumeName.contains("InnerCPMylar")) newHit->SetMylarLayerType("InnerCP");
  else if (volumeName.contains("InnerGPMylar")) newHit->SetMylarLayerType("InnerGP");
  else if (volumeName.contains("InnerGassecond")) newHit->SetMylarLayerType("InnerGassecond");
  else if (volumeName.contains("InnerGas")) newHit->SetMylarLayerType("InnerGas");
  else if (volumeName.contains("OuterGassecond")) newHit->SetMylarLayerType("OuterGassecond");
  else if (volumeName.contains("OuterGas")) newHit->SetMylarLayerType("OuterGas");
  else newHit->SetMylarLayerType("UnknownMylar");


  // --- Calculate Grid Cell IDs (Phi and Z) ---
  // This requires transforming the global hit position to the local coordinate system
  // of the Mylar layer within its specific sector.
  G4AffineTransform transform = touchable->GetHistory()->GetTopTransform();
  G4ThreeVector localPos = transform.TransformPoint(newHit->GetPosition());

  // In src/MylarSD.cc - within ProcessHits
// ... (after getting localPos) ...

  // Get dimensions from DetectorConstruction
  G4double klmHalfZ = fDetConstruction->GetKLMHalfLength();
  // G4double sectorAngleRad = fDetConstruction->GetKLMSectorAngle(); // Already in radians
  G4int numPhiCells = fDetConstruction->GetNumPhiCells06();;
  if (stackNumber > 6){
    G4cout << stackNumber << " stack_number " << G4endl;
    numPhiCells = fDetConstruction->GetNumPhiCells714(); // Should be 48
  }
  G4int numZCells = fDetConstruction->GetNumZCells();     // Should be 96

  // Z-Cell Calculation: local Z ranges from -klmHalfZ to +klmHalfZ
  G4double localZ = localPos.z();
  G4int zCell = -1; // Default to invalid
  if (localZ >= -klmHalfZ && localZ < klmHalfZ) { // Use < for upper bound to align with 0-N-1 indexing
      zCell = static_cast<G4int>(((localZ + klmHalfZ) / (2. * klmHalfZ)) * numZCells);
      // Clamp to valid range [0, numZCells-1]
      if (zCell >= numZCells) zCell = numZCells - 1;
      if (zCell < 0) zCell = 0; // Should not happen if localZ is in range
  }
  newHit->SetZCellID(zCell); // Z goes from 0 to 95

  // Phi-Cell Calculation:
  G4double localPhi = localPos.phi(); // range: -pi to +pi relative to local X of the segment
  G4double segmentDeltaPhi = fDetConstruction->GetKLMSectorAngle(); // e.g., 45 deg
  G4double segmentStartPhi = -segmentDeltaPhi / 2.0; // Sector is centered around its local X-axis

  // Normalize phi within the segment [segmentStartPhi, segmentStartPhi + segmentDeltaPhi]
  // to [0, segmentDeltaPhi] then to [0, 1]
  G4double phiRelativeToSegmentStart = localPhi - segmentStartPhi;
  // Handle wraparound if localPhi is just outside segmentStartPhi (e.g. -22.6 deg for a -22.5 start)
  // or just above segmentStartPhi + segmentDeltaPhi
  while (phiRelativeToSegmentStart < 0) phiRelativeToSegmentStart += CLHEP::twopi; // Ensure positive
  while (phiRelativeToSegmentStart >= segmentDeltaPhi + 1e-9) phiRelativeToSegmentStart -= segmentDeltaPhi; // Normalize within one segment width (approx)

  G4int phiCell = -1; // Default to invalid
  if (phiRelativeToSegmentStart >= -1e-9 && phiRelativeToSegmentStart <= segmentDeltaPhi + 1e-9) { // Check within segment bounds (with tolerance)
      phiCell = static_cast<G4int>((phiRelativeToSegmentStart / segmentDeltaPhi) * numPhiCells);
       // Clamp to valid range [0, numPhiCells-1]
      if (phiCell >= numPhiCells) phiCell = numPhiCells - 1;
      if (phiCell < 0) phiCell = 0;
  }
  newHit->SetPhiCellID(phiCell); // Phi goes from 0 to 35
  if (phiCell > 35){
    G4cout << phiCell << "phiCell" << G4endl; 
  }

// ...

  // // Get dimensions from DetectorConstruction
  // G4double klmHalfZ = fDetConstruction->GetKLMHalfLength();
  // G4double sectorAngleRad = fDetConstruction->GetKLMSectorAngle(); // Already in radians
  // G4int numPhiCells = fDetConstruction->GetNumPhiCells();
  // G4int numZCells = fDetConstruction->GetNumZCells();

  // // Z-Cell Calculation: local Z ranges from -klmHalfZ to +klmHalfZ
  // G4double localZ = localPos.z();
  // G4int zCell = -1;
  // if (localZ >= -klmHalfZ && localZ <= klmHalfZ) {
  //     zCell = static_cast<G4int>(((localZ + klmHalfZ) / (2. * klmHalfZ)) * numZCells);
  //     if (zCell >= numZCells) zCell = numZCells - 1; // Boundary condition
  //     if (zCell < 0) zCell = 0;
  // }
  // newHit->SetZCellID(zCell);

  // // Phi-Cell Calculation:
  // // Local phi within the sector wedge.
  // // The sector mother volume starts at -sectorAngleRad/2.0 and spans sectorAngleRad.
  // // So localPhi ranges from -sectorAngleRad/2.0 to +sectorAngleRad/2.0
  // G4double localPhi = localPos.phi(); // This phi is w.r.t. the local X-axis of the Polyhedra segment

  // // The Polyhedra local phi usually ranges from its startPhi to startPhi + deltaPhi.
  // // For our sector mother (and layers within), phiStart is -sectorAngleRad/2.0.
  // // So localPhi (from G4ThreeVector::phi()) will be between -pi and pi.
  // // We need to map this to the range [0, sectorAngleRad] relative to the segment's start.

  // G4double phi_in_segment_frame = localPhi; // This localPos.phi() is already in the segment's frame.
  //                                           // It should range from the Polyhedra's sPhi to sPhi+dPhi
  //                                           // For our sector, sPhi = -sectorAngleRad/2.0
  //                                           // So localPos.phi() will be approx in [-sectorAngleRad/2, sectorAngleRad/2]

  // G4double normalizedPhi = (phi_in_segment_frame + sectorAngleRad / 2.0) / sectorAngleRad; // Normalize to [0, 1]

  // G4int phiCell = static_cast<G4int>(normalizedPhi * numPhiCells);
  // if (phiCell >= numPhiCells) phiCell = numPhiCells - 1; // Boundary
  // if (phiCell < 0) phiCell = 0;                         // Boundary
  // newHit->SetPhiCellID(phiCell);
  fHitsCollection->insert(newHit);


  // Add the hit to the collection
  // if(totalMomentum > 600){
  //   fHitsCollection->insert(newHit);
  // }
  // else{
  //   G4cout << "momentum less than 0.6 GeV/c, cannot pass through"  << G4endl;
  // }
  // newHit->Print(); // Optional: for debugging

  return true;
}

// void MylarSD::EndOfEvent(G4HCofThisEvent* /*hce*/)
// {}