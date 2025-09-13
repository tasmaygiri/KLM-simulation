#ifndef RUNACTION_HH
#define RUNACTION_HH

#include "G4UserRunAction.hh"
#include "globals.hh"
#include <fstream> // For std::ofstream

class G4Run;

class RunAction : public G4UserRunAction
{
public:
  RunAction(const G4String& outputFilename = "cell_energy_summary.txt");
  virtual ~RunAction();

  virtual void BeginOfRunAction(const G4Run* run);
  virtual void EndOfRunAction(const G4Run* run);

  std::ofstream& GetOutputFileStream() { return fOutputFile; }
  // bool IsFirstEvent() const { return fIsFirstEventFlagsSetForEvent0; } // Optional helper

private:
  std::ofstream fOutputFile;
  G4String fOutputFileName;
  // bool fIsFirstEventFlagsSetForEvent0; // Optional
};

#endif // RUNACTION_HH