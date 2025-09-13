#include "EventAction.hh"
#include "RunAction.hh"
#include "SteppingAction.hh" // If used
#include "MylarHit.hh"       // For MylarHitsCollection and MylarHit

#include "G4Event.hh"
#include "G4RunManager.hh"
// #include "G4EventManager.hh" // Not directly needed
#include "G4HCofThisEvent.hh"
// #include "G4VHitsCollection.hh" // G4HCofThisEvent is enough
#include "G4SDManager.hh"
#include "G4ios.hh"
#include "G4UnitsTable.hh"
#include "G4SystemOfUnits.hh"
#include <iomanip>
#include <fstream>      // For std::ofstream

// Constructor
EventAction::EventAction(RunAction* runAction, SteppingAction* steppingAction)
 : G4UserEventAction(),
   fRunAction(runAction),
   fSteppingAction(steppingAction),
   fMylarHitsCollectionID(-1)
{
    G4cout << "EventAction created." << G4endl;
}

EventAction::~EventAction()
{}

// AddTrackData is REMOVED

void EventAction::BeginOfEventAction(const G4Event* event)
{
  if (fSteppingAction) {
    fSteppingAction->Reset();
  }
  // fEventTrackInfo.clear(); // REMOVED

  // Clear the cell energy map for the new event
  fCellEnergyMap.clear();

  // Get Hits Collection ID for Mylar hits (do this once)
  if (fMylarHitsCollectionID < 0) {
      G4SDManager* sdManager = G4SDManager::GetSDMpointer();
      if (sdManager) {
          fMylarHitsCollectionID = sdManager->GetCollectionID("MylarHitsCollection");
      } else {
          G4cerr << "ERROR in EventAction: SDManager not found!" << G4endl;
      }
  }

  G4int eventID = event->GetEventID();
  if (eventID % 100 == 0) { // Or every event for debugging
    G4cout << "\n-----------------------------------------------------" << G4endl;
    G4cout << "---> Begin of Event: " << eventID << G4endl;
    G4cout << "-----------------------------------------------------" << G4endl;
  }
}

void EventAction::EndOfEventAction(const G4Event* event)
{
  G4int eventID = event->GetEventID();

  // --- Retrieve Mylar Hits and SUMMARIZE them into fCellEnergyMap ---
  if (fMylarHitsCollectionID >= 0) {
    G4HCofThisEvent* hce = event->GetHCofThisEvent();
    MylarHitsCollection* mylarHC = nullptr;
    if (hce) {
      mylarHC = static_cast<MylarHitsCollection*>(hce->GetHC(fMylarHitsCollectionID));
    }

    if (mylarHC) {
      G4int n_hit_steps = mylarHC->entries(); // Number of individual steps recorded as hits
      // G4cout << "Event " << eventID << ": Number of Mylar hit steps = " << n_hit_steps << G4endl;

      for (G4int i = 0; i < n_hit_steps; i++) {
        MylarHit* hit = (*mylarHC)[i];

        // Create cell identifier from the hit information
        CellIdentifier cellID = std::make_tuple(
            hit->GetSectorNumber(),
            hit->GetStackNumber(),
            hit->GetZCellID(),    // Should be 0-95
            hit->GetPhiCellID()   // Should be 0-35
        );
        // Accumulate energy in the map
        fCellEnergyMap[cellID] += hit->GetEnergyDeposited();
      }
    }
  } else {
    G4cerr << "EventAction Warning (Event " << eventID << "): MylarHitsCollectionID not set or invalid!" << G4endl;
  }

  // --- Write SUMMARIZED Mylar Cell Energies from fCellEnergyMap to file ---
  if (fRunAction && fRunAction->GetOutputFileStream().is_open()) {
    std::ofstream& outFile = fRunAction->GetOutputFileStream();

    if (!fCellEnergyMap.empty()) {
      G4cout << "EventAction: Writing " << fCellEnergyMap.size() << " summarized cell energy entries for Event " << eventID << G4endl;
      for (const auto& pair : fCellEnergyMap) {
        const CellIdentifier& cell = pair.first;
        G4double totalEdep = pair.second;

        outFile << eventID << " "
                << std::get<0>(cell) << " " // Sector
                << std::get<1>(cell) << " " // Stack
                << std::get<2>(cell) << " " // ZCell (0-95)
                << std::get<3>(cell) << " " // PhiCell (0-35)
                << totalEdep / keV      // Write energy in MeV
                << "\n";
      }
    }
  } else {
      if (!fRunAction) G4cerr << "EventAction Error (Event " << eventID << "): RunAction pointer is null! Cannot write cell energies." << G4endl;
      else if (fRunAction && !fRunAction->GetOutputFileStream().is_open()) {
          G4cerr << "EventAction Error (Event " << eventID << "): Output file stream is not open! Cannot write cell energies." << G4endl;
      }
  }

  // --- Remove Particle Table Summary (from TrackingAction) ---
  // G4cout << "\n--- Track Summary for Event ... (REMOVED)

  G4cout << "---> End of Event: " << eventID << G4endl;
}