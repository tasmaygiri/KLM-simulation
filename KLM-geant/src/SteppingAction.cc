#include "SteppingAction.hh"
#include "DetectorConstruction.hh" // Might need if accessing geometry info directly

#include "G4Step.hh"
#include "G4Track.hh"
#include "G4LogicalVolume.hh"
#include "G4VPhysicalVolume.hh"
#include "G4Material.hh"
#include "G4RunManager.hh" // If needed, e.g., to access detector construction
#include "G4ios.hh"

// Initialize the static set
std::set<G4int> SteppingAction::fTracksInRPC;

// Constructor stores the target material name
SteppingAction::SteppingAction(const G4String& rpcMaterialName)
 : G4UserSteppingAction(),
   fRpcMaterialName(rpcMaterialName)
{
    G4cout << "SteppingAction initialized to kill particles after entering material: "
           << fRpcMaterialName << G4endl;
}

SteppingAction::~SteppingAction()
{}

// Method called at the beginning of each event by EventAction
void SteppingAction::Reset() {
    fTracksInRPC.clear();
}

// This method is called by Geant4 at the end of each step
void SteppingAction::UserSteppingAction(const G4Step* step)
{
    // Get the track associated with this step
    G4Track* track = step->GetTrack();

    // --- Check if track should be killed ---
    // If the track ID is already in our set, kill it immediately.
    if (fTracksInRPC.count(track->GetTrackID())) {
        track->SetTrackStatus(fStopAndKill);
        // G4cout << "Killing track " << track->GetTrackID()
        //        << " (" << track->GetParticleDefinition()->GetParticleName() << ")"
        //        << " because it previously entered RPC." << std::endl;
        return; // Stop further checks for this step
    }

    // --- Check if the step *ended* in the RPC layer ---
    // Get the volume where the step ended (PostStepPoint)
    G4VPhysicalVolume* postVolume = step->GetPostStepPoint()->GetPhysicalVolume();

    // Check if the post-step volume exists and get its logical volume's material
    if (postVolume) {
        G4Material* postMaterial = postVolume->GetLogicalVolume()->GetMaterial();
        // Compare the material name with the target RPC material name
        if (postMaterial->GetName() == fRpcMaterialName) {
            // This is the first time this track has ended a step in the RPC material.
            // Add its ID to the set so it will be killed on the *next* step
            // (or immediately if it already was in the set, handled above).
            fTracksInRPC.insert(track->GetTrackID());
            // G4cout << "Track " << track->GetTrackID()
            //        << " (" << track->GetParticleDefinition()->GetParticleName() << ")"
            //        << " entered RPC material (" << fRpcMaterialName << "). Will kill on next step." << std::endl;
        }
    }
    // --- End of check ---

    // The track continues for this step unless it was killed above.
    // It will be killed at the start of its *next* step if its ID is now in fTracksInRPC.
}