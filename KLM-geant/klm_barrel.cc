#include "G4RunManagerFactory.hh"
#include "G4UImanager.hh"
#include "G4VisExecutive.hh"
#include "G4UIExecutive.hh"
#include "G4PhysListFactory.hh"
#include "FTFP_BERT.hh"

#include "DetectorConstruction.hh"
#include "ActionInitialization.hh"

int main(int argc, char** argv)
{
    G4UIExecutive* ui = nullptr;
    G4String macroName = "";
    G4String         inputFileName = argv[1];
    // Default

    if (argc == 2) {
        ui = new G4UIExecutive(argc, argv);
    } else {
        if (argc > 2) {
            macroName = argv[2];
        }
    }

    // --- Construct the RunManager ---
    auto* runManager = G4RunManagerFactory::CreateRunManager(G4RunManagerType::Serial);

    // --- Set mandatory user initialization classes ---
    // 1. Detector construction
    runManager->SetUserInitialization(new DetectorConstruction());

    // 2. Physics list
    G4VModularPhysicsList* physicsList = new FTFP_BERT;
    physicsList->SetVerboseLevel(1); // Set verbosity before initialization if needed
    runManager->SetUserInitialization(physicsList);

    // 3. User action initialization
    // This creates instances of PrimaryGeneratorAction, RunAction, EventAction etc.
    runManager->SetUserInitialization(new ActionInitialization(inputFileName));

    // --- Initialize Visualization AFTER User Initializations ---
    G4VisManager* visManager = new G4VisExecutive;
    visManager->Initialize(); // This might internally call /run/initialize if not done yet

    // --- Get UI manager ---
    G4UImanager* UImanager = G4UImanager::GetUIpointer();

    // --- Process macro or start UI session ---
    if (macroName.size() > 0) { // Batch mode
        G4String command = "/control/execute ";
        UImanager->ApplyCommand(command + macroName);
    } else { // Interactive mode
        // It's common for init_vis.mac to contain /run/initialize
        // If not, /run/initialize might be needed here or before beamOn.
        // Geant4 often calls /run/initialize automatically before the first /run/beamOn
        // if visualization is active.
        UImanager->ApplyCommand("/control/execute init_vis.mac");
        if (ui) { // Check if ui was successfully created
            ui->SessionStart();
            delete ui;
        } else {
            G4cerr << "ERROR: Could not start UI session." << G4endl;
        }
    }

    // --- Job termination ---
    delete visManager;
    delete runManager; // This will delete ActionInitialization and its actions,
                       // triggering destructors (like PrimaryGeneratorAction's destructor)

    G4cout << "----> End of main()." << G4endl;
    return 0;
}