#include "RunAction.hh"
#include "G4Run.hh"
#include "G4RunManager.hh"
#include "G4ios.hh"
// #include "G4UnitsTable.hh" // Not strictly needed here anymore
#include "G4SystemOfUnits.hh"

RunAction::RunAction(const G4String& outputFileName)
 : G4UserRunAction(),
   fOutputFileName(outputFileName)
{
  G4cout << "RunAction created. Output file for cell energies: " << fOutputFileName << G4endl;
}

RunAction::~RunAction()
{
  if (fOutputFile.is_open()) {
    fOutputFile.close();
  }
}

void RunAction::BeginOfRunAction(const G4Run* aRun)
{
  G4cout << "### Run " << aRun->GetRunID() << " start." << G4endl;
  fOutputFile.open(fOutputFileName.c_str(), std::ios::out | std::ios::trunc);

  if (fOutputFile.is_open()) {
    G4cout << "Output file for cell energies opened: " << fOutputFileName << G4endl;
    // <<< MODIFIED HEADER >>>
    fOutputFile << "# EventID Sector Stack ZCell(0-95) PhiCell(0-35) TotalEnergyDep_keV\n";
  } else {
    G4cerr << "ERROR: Could not open output file for cell energies: " << fOutputFileName << G4endl;
  }
}

void RunAction::EndOfRunAction(const G4Run* aRun)
{
  G4int nofEvents = aRun->GetNumberOfEvent();
  if (nofEvents == 0) {
    G4cout << "Run " << aRun->GetRunID() << " had no events." << G4endl;
  } else {
    G4cout << "### Run " << aRun->GetRunID() << " end. Number of events: " << nofEvents << G4endl;
  }

  if (fOutputFile.is_open()) {
    fOutputFile.close();
    G4cout << "Output file for cell energies closed: " << fOutputFileName << G4endl;
  }
}