#include "EventAction.hh"
#include "RunAction.hh"         // <<< Include RunAction
#include "SteppingAction.hh"
#include "MylarHit.hh"          // <<< Include MylarHit

#include "G4Event.hh"
#include "G4RunManager.hh"
#include "G4EventManager.hh"    // For G4EventManager
#include "G4HCofThisEvent.hh"   // For G4HCofThisEvent
#include "G4VHitsCollection.hh" // For G4VHitsCollection
#include "G4SDManager.hh"       // For G4SDManager
#include "G4ios.hh"
#include "G4UnitsTable.hh"
#include "G4SystemOfUnits.hh"   // For unit conversions (MeV, ns, mm)
#include <iomanip>

// Constructor now takes RunAction
EventAction::EventAction(RunAction* runAction, SteppingAction* steppingAction)
 : G4UserEventAction(),
   fRunAction(runAction),         // <<< Store RunAction
   fSteppingAction(steppingAction),
   fMylarHitsCollectionID(-1)     // <<< Initialize collection ID
{}

EventAction::~EventAction()
{}

void EventAction::AddTrackData(const TrackInfo& info) {
    fEventTrackInfo.push_back(info);
}

void EventAction::BeginOfEventAction(const G4Event* event)
{
  if (fSteppingAction) {
    fSteppingAction->Reset();
  }
  fEventTrackInfo.clear();

  // Get Hits Collection ID for Mylar hits
  // This ID is constant for the whole run once initialized by SD
  if (fMylarHitsCollectionID < 0) { // Get it only once
      G4SDManager* sdManager = G4SDManager::GetSDMpointer();
      if (sdManager) { // Check if SDManager exists
          fMylarHitsCollectionID = sdManager->GetCollectionID("MylarHitsCollection");
          // G4cout << "EventAction: MylarHitsCollection ID = " << fMylarHitsCollectionID << G4endl;
      } else {
          G4cerr << "ERROR in EventAction: SDManager not found!" << G4endl;
      }
  }

  G4int eventID = event->GetEventID();
  // Reduced print frequency for BeginOfEvent
  if (eventID % 100 == 0) {
    G4cout << "\n-----------------------------------------------------" << G4endl;
    G4cout << "---> Begin of Event: " << eventID << G4endl;
    G4cout << "-----------------------------------------------------" << G4endl;
  }
}

void EventAction::EndOfEventAction(const G4Event* event)
{
  // ... (your existing Track Summary table printing if any) ...

  G4int eventID = event->GetEventID();

  // --- Process and Write SUMMARIZED Mylar Hits ---
  if (fMylarHitsCollectionID >= 0 && fRunAction && fRunAction->GetOutputFileStream().is_open()) {
    G4HCofThisEvent* hce = event->GetHCofThisEvent();
    MylarHitsCollection* mylarHC = nullptr;
    if (hce) {
      mylarHC = static_cast<MylarHitsCollection*>(hce->GetHC(fMylarHitsCollectionID));
    }

    if (mylarHC && mylarHC->entries() > 0) {
      std::ofstream& outFile = fRunAction->GetOutputFileStream();

      // Use a map to group hits by TrackID.
      // If you want to distinguish hits in different physical volumes by the same track,
      // the key could be std::pair<G4int, G4String> (TrackID, VolumeName)
      std::map<G4int, SummarizedMylarHit> summarizedHitsMap;

      for (G4int i = 0; i < mylarHC->entries(); i++) {
        MylarHit* hit = (*mylarHC)[i];
        G4int currentTrackID = hit->GetTrackID();

        // Get or create the summary for this trackID
        SummarizedMylarHit& summary = summarizedHitsMap[currentTrackID]; // Creates if not exists

        if (!summary.isInitialized) { // First step for this track in a Mylar volume
          summary.eventID = eventID;
          summary.trackID = currentTrackID;
          summary.parentID = hit->GetParentID();
          summary.pdgCode = hit->GetPDGCode();
          summary.particleName = hit->GetParticleName();
          summary.entryTime = hit->GetGlobalTime();
          summary.entryPosition = hit->GetPosition();
          summary.sectorNumber = hit->GetSectorNumber(); // Assuming these are properties of the hit/volume
          summary.stackNumber = hit->GetStackNumber();
          // summary.mylarLayerType = hit->GetMylarLayerType();
          summary.phiCellID = hit->GetPhiCellID();
          summary.zCellID = hit->GetZCellID();
          summary.volumeName = hit->GetVolumeName(); // Assumes MylarHit stores this
          summary.isInitialized = true;
        }

        // Accumulate energy
        summary.totalEnergyDeposited += hit->GetEnergyDeposited();

        // Update exit time and position with each step's data
        summary.exitTime = hit->GetGlobalTime();
        summary.exitPosition = hit->GetPosition();
      }

      // Now write the summarized hits to the file
      for (const auto& pair : summarizedHitsMap) {
        const SummarizedMylarHit& summary = pair.second;
        outFile
                << summary.eventID << " "
                << summary.trackID << " "
                << summary.parentID << " "
                << summary.pdgCode << " "
                << "\"" << summary.particleName << "\"" << " "
                << summary.totalEnergyDeposited / MeV << " " // Already accumulated, convert if necessary
                << summary.entryTime / ns << " "           // Entry time
                << summary.entryPosition.x() / mm << " "
                << summary.entryPosition.y() / mm << " "
                << summary.entryPosition.z() / mm << " "
                // Optional: Write exit time and position as well
                // << summary.exitTime / ns << " "
                // << summary.exitPosition.x() / mm << " "
                // << summary.exitPosition.y() / mm << " "
                // << summary.exitPosition.z() / mm << " "
                << summary.sectorNumber << " "
                << summary.stackNumber << " "
                // << "\"" << summary.mylarLayerType << "\"" << " "
                << summary.phiCellID << " "
                << summary.zCellID << " "
                << "\"" << summary.volumeName << "\""
                << "\n";
      }
    }
  } else {
      if (fMylarHitsCollectionID < 0) G4cerr << "EventAction: MylarHitsCollectionID not set for event " << eventID << G4endl;
      if (!fRunAction) G4cerr << "EventAction: RunAction pointer is null for event " << eventID << G4endl;
      else if (fRunAction && !fRunAction->GetOutputFileStream().is_open()) {
          G4cerr << "EventAction: Output file stream is not open for event " << eventID << G4endl;
      }
  }
  G4cout << "---> End of Event: " << eventID << G4endl; // Keep your end of event marker
}