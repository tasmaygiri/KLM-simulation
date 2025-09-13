#ifndef EVENTACTION_HH
#define EVENTACTION_HH

#include "G4UserEventAction.hh"
#include "globals.hh"
#include <map>      // For storing energy per cell
#include <tuple>    // For using a tuple as a map key

// Forward declarations
class G4Event;
class SteppingAction; // Optional
class RunAction;

// Define a key for our energy deposition map
// (Sector, Stack, ZCell (0-95), PhiCell (0-35))
typedef std::tuple<G4int, G4int, G4int, G4int> CellIdentifier;

class EventAction : public G4UserEventAction
{
public:
  EventAction(RunAction* runAction, SteppingAction* steppingAction = nullptr);
  virtual ~EventAction();

  virtual void BeginOfEventAction(const G4Event* event);
  virtual void EndOfEventAction(const G4Event* event);

  // // Method for MylarSD to add energy to a cell (This would be if MylarSD called EventAction directly)
  // void AddEnergyToCell(const CellIdentifier& cell, G4double energy); // We will do accumulation here

private:
  RunAction* fRunAction;
  SteppingAction* fSteppingAction; // Optional
  // Map to store total energy deposited in each cell for the current event
  std::map<CellIdentifier, G4double> fCellEnergyMap;
  G4int fMylarHitsCollectionID; // Keep this to retrieve MylarHitsCollection
};

#endif // EVENTACTION_HH