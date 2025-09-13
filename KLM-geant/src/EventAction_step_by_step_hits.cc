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
  G4int eventID = event->GetEventID();

  // --- Process and Write Mylar Hits ---
  if (fMylarHitsCollectionID >= 0 && fRunAction && fRunAction->GetOutputFileStream().is_open()) {
    G4HCofThisEvent* hce = event->GetHCofThisEvent();
    MylarHitsCollection* mylarHC = nullptr;
    if (hce) {
      mylarHC = static_cast<MylarHitsCollection*>(hce->GetHC(fMylarHitsCollectionID));
    }

    if (mylarHC) {
      G4int n_hit = mylarHC->entries();
      // G4cout << "Event " << eventID << ": Number of Mylar hits = " << n_hit << G4endl;
      std::ofstream& outFile = fRunAction->GetOutputFileStream();

      for (G4int i = 0; i < n_hit; i++) {
        MylarHit* hit = (*mylarHC)[i];
        // hit->Print(); // Optional: for console debug

        outFile 
                << eventID << " "
                << hit->GetTrackID() << " "
                << hit->GetParentID() << " "
                << hit->GetPDGCode() << " "
                << "\"" << hit->GetParticleName() << "\"" << " " // Enclose string in quotes for CSV
                << hit->GetEnergyDeposited() / MeV << " " // Store in MeV
                << hit->GetGlobalTime() / ns << " "      // Store in ns
                << hit->GetPosition().x() / mm << " "
                << hit->GetPosition().y() / mm << " "
                << hit->GetPosition().z() / mm << " "
                << hit->GetSectorNumber() << " "
                << hit->GetStackNumber() << " "
                // << "\"" << hit->GetMylarLayerType() << "\"" << " " // Enclose string
                << hit->GetPhiCellID() << " "
                << hit->GetZCellID() << " "
                << "\"" << hit->GetVolumeName() << "\"" // Enclose string
                << "\n";
      }
    }
  } else {
      if (fMylarHitsCollectionID < 0) G4cerr << "EventAction: MylarHitsCollectionID not set!" << G4endl;
      if (!fRunAction) G4cerr << "EventAction: RunAction pointer is null!" << G4endl;
      else if (fRunAction && !fRunAction->GetOutputFileStream().is_open()) G4cerr << "EventAction: Output file stream is not open!" << G4endl;
  }


  // --- Print Particle Table Summary (from TrackingAction) ---
  G4cout << "\n--- Track Summary for Event " << eventID << " (" << fEventTrackInfo.size() << " tracks) ---" << G4endl;
  if (!fEventTrackInfo.empty()) {
      G4cout << std::setw(5) << "TrkID" << " | " << std::setw(5) << "ParID" << " | "
             << std::setw(8) << "PDG ID" << " | " << std::setw(14) << "Particle" << " | "
             << std::setw(15) << "Creator Proc" << " | " << std::setw(18) << "Start Energy [MeV]" << " | "
             << std::setw(25) << "Start Pos (x,y,z) [mm]" << G4endl;
      G4cout << "-------------------------------------------------------------------------------------------------------------------------------" << G4endl;
      for (const auto& trackInfo : fEventTrackInfo) {
          G4cout << std::setw(5) << trackInfo.trackID << " | " << std::setw(5) << trackInfo.parentID << " | "
                 << std::setw(8) << trackInfo.pdgID << " | " << std::setw(14) << trackInfo.particleName << " | "
                 << std::setw(15) << trackInfo.processName << " | "
                 << std::fixed << std::setprecision(3) << std::setw(18) << trackInfo.initialKineticEnergy
                 << std::setprecision(6) << std::defaultfloat << " | ("
                 << std::fixed << std::setprecision(1) << std::setw(7) << trackInfo.initialPosition.x() / mm << ","
                 << std::setw(7) << trackInfo.initialPosition.y() / mm << ","
                 << std::setw(7) << trackInfo.initialPosition.z() / mm << ")"
                 << std::setprecision(6) << std::defaultfloat << G4endl;
      }
      G4cout << "-------------------------------------------------------------------------------------------------------------------------------" << G4endl;
  } else {
      G4cout << "No tracks were processed/recorded in this event for the summary table." << G4endl;
  }
  G4cout << "---> End of Event: " << eventID << G4endl;
}