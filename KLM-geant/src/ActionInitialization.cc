#include "ActionInitialization.hh"
#include "PrimaryGeneratorAction.hh"
#include "RunAction.hh"
#include "EventAction.hh"
#include "SteppingAction.hh" // If still used
#include "G4HepMCInterface.hh"
// #include "TrackingAction.hh" // <<< REMOVE or comment out

ActionInitialization::ActionInitialization(const G4String& inputFilename)
 : G4VUserActionInitialization(),
   fInputFilename(inputFilename)
{}

ActionInitialization::~ActionInitialization()
{}

void ActionInitialization::Build() const
{
  if (fInputFilename.contains(".hepmc")) {
    G4cout << "ActionInitialization: Using G4HepMCInterface for file: " << fInputFilename << G4endl;
    SetUserAction(new G4HepMCInterface(fInputFilename)); // Use YOUR HepMC interface
} else {
    G4cout << "ActionInitialization: Using PrimaryGeneratorAction (custom format) for file: " << fInputFilename << G4endl;
    SetUserAction(new PrimaryGeneratorAction(fInputFilename)); // Use your custom format reader
}

  RunAction* runAction = new RunAction("summarized_cell_energy.txt"); // New output file name
  SetUserAction(runAction);

  SteppingAction* steppingAction = nullptr;
  // if (you_need_stepping_action) {
  //   steppingAction = new SteppingAction("RPCSuperlayer");
  //   SetUserAction(steppingAction);
  // }

  EventAction* eventAction = new EventAction(runAction, steppingAction);
  SetUserAction(eventAction);

  // TrackingAction* trackingAction = new TrackingAction(eventAction); // <<< REMOVE or comment out
  // SetUserAction(trackingAction); // <<< REMOVE or comment out
}

void ActionInitialization::BuildForMaster() const
{
  SetUserAction(new RunAction("master_summarized_cell_energy.txt"));
}