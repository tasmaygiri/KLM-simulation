#ifndef STEPPINGACTION_HH
#define STEPPINGACTION_HH

#include "G4UserSteppingAction.hh"
#include "globals.hh"
#include <set> // To keep track of tracks that have entered the RPC

// Forward declarations
class G4Step;
class G4LogicalVolume;

class SteppingAction : public G4UserSteppingAction
{
public:
  // Constructor needs access to the RPC material name or pointer,
  // and potentially other classes like EventAction if needed.
  // Let's pass the material name for simplicity.
  SteppingAction(const G4String& rpcMaterialName);
  virtual ~SteppingAction();

  // Method called at the end of every step
  virtual void UserSteppingAction(const G4Step* step);

  // Method called at the beginning of each event (to clear the set)
  void Reset();

private:
  G4String fRpcMaterialName;
  // Use a set to store track IDs that have already entered the RPC layer ONCE
  // We use a static member for simplicity in this example,
  // but managing this via EventAction is often cleaner for MT runs.
  static std::set<G4int> fTracksInRPC;
};

#endif // STEPPINGACTION_HH