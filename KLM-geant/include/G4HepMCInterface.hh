#ifndef G4HepMCInterface_h
#define G4HepMCInterface_h 1

#include "G4VUserPrimaryGeneratorAction.hh"
#include "G4String.hh"
#include "globals.hh"
#include "G4SystemOfUnits.hh"


// Forward declarations for HepMC2 classes
namespace HepMC {
    class IO_GenEvent;
    class GenEvent; // We no longer store one, but we use the type
}
class G4Event;


class G4HepMCInterface : public G4VUserPrimaryGeneratorAction
{
public:
    // Constructor now takes the filename
    G4HepMCInterface(const G4String& hepmcFileName);
    virtual ~G4HepMCInterface();

    // The core method called by Geant4
    virtual void GeneratePrimaries(G4Event* anEvent);

private:
    // --- HepMC specific (Version 2) ---
    HepMC::IO_GenEvent* m_asciiInput = nullptr; // HepMC file reader object

    // --- Helper method ---
    // Converts a given HepMC event to Geant4 primaries in the G4Event
    G4bool ConvertHepMCEvent(const HepMC::GenEvent* hepmcEvt, G4Event* g4Evt);

    // --- Unit conversion factors (assuming GeV and mm input) ---
    const G4double momentumUnit = GeV; // Assume HepMC momentum is in GeV
    const G4double lengthUnit   = mm;  // Assume HepMC length is in mm

    // Optional verbosity control (matches the example)
    G4int verboseLevel = 0; // Set to 1 for event printout
};

#endif