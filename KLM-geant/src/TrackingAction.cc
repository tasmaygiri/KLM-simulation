#include "TrackingAction.hh"
#include "EventAction.hh"     // Include EventAction to access its methods/members (like AddTrackData)
                              // and the TrackInfo struct definition.

#include "G4TrackingManager.hh"
#include "G4Track.hh"
#include "G4ParticleDefinition.hh"
#include "G4VProcess.hh"       // To get the creator process name
#include "G4SystemOfUnits.hh"  // For unit conversions if needed (e.g., GetKineticEnergy is in MeV)

// Constructor stores pointer to EventAction
TrackingAction::TrackingAction(EventAction* eventAction)
:G4UserTrackingAction(),
 fEventAction(eventAction) // Store the passed EventAction pointer
{
    if (fEventAction) {
        G4cout << "TrackingAction created and linked to EventAction." << G4endl;
    } else {
        G4cerr << "ERROR: TrackingAction created with a null EventAction pointer!" << G4endl;
    }
}

TrackingAction::~TrackingAction()
{}

// This method is called by Geant4 at the beginning of processing *every* track
// (both primary particles from the input file and any secondary particles produced)
void TrackingAction::PreUserTrackingAction(const G4Track* track)
{
    // Ensure the EventAction pointer is valid before trying to use it
    if (!fEventAction) return;

    // Create a TrackInfo structure to hold data for the current track
    TrackInfo info;

    // Populate the TrackInfo structure with data from the G4Track object
    info.trackID = track->GetTrackID();          // Unique ID for this track
    info.parentID = track->GetParentID();        // ID of the parent track (0 for primaries)
    info.pdgID = track->GetParticleDefinition()->GetPDGEncoding(); // PDG code
    info.particleName = track->GetParticleDefinition()->GetParticleName(); // Particle name (e.g., "e-", "gamma")

    // Get the position where this track was created (its vertex)
    info.initialPosition = track->GetVertexPosition(); // Position is in mm

    // Get the kinetic energy with which this track was created
    // GetVertexKineticEnergy() is energy at the vertex. GetKineticEnergy() is current.
    info.initialKineticEnergy = track->GetVertexKineticEnergy(); // Energy is in MeV

    // Get the momentum direction at the vertex
    info.initialMomentumDirection = track->GetVertexMomentumDirection();

    // Get the name of the physics process that created this track
    const G4VProcess* creatorProcess = track->GetCreatorProcess();
    if (creatorProcess) {
        info.processName = creatorProcess->GetProcessName();
    } else {
        // Primary particles (parentID == 0) don't have a "creator process" in the same sense
        if (info.parentID == 0) {
            info.processName = "Primary"; // Label it as a primary particle
        } else {
            // This case should ideally not happen for a secondary particle
            info.processName = "Unknown";
        }
    }

    // Pass this TrackInfo object to the EventAction to be stored
    fEventAction->AddTrackData(info);
}

// PostUserTrackingAction is called at the end of a track.
// We don't strictly need it for the requested table of initial particle states,
// but it's available if you want to collect information about where/how a track ends.
// void TrackingAction::PostUserTrackingAction(const G4Track* track)
// {
//     // Example:
//     // if (fEventAction) {
//     //     G4cout << "Track " << track->GetTrackID() << " (" << track->GetParticleDefinition()->GetParticleName()
//     //            << ") ended. Length: " << G4BestUnit(track->GetTrackLength(), "Length")
//     //            << " in volume: " << (track->GetVolume() ? track->GetVolume()->GetName() : "OutOfWorld")
//     //            << G4endl;
//     // }
// }