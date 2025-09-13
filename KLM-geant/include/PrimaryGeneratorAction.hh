#ifndef PRIMARYGENERATORACTION_HH
#define PRIMARYGENERATORACTION_HH

#include "G4VUserPrimaryGeneratorAction.hh"
#include "globals.hh"
#include <fstream> // Required for ifstream
#include <string>  // Required for string
#include <vector>  // Required for storing particle data temporarily

// Structure to hold particle data read from custom file
struct ParticleData { // Keep this structure
    G4int eventID = -1;
    G4int pdgID = 0;
    G4double px = 0.0, py = 0.0, pz = 0.0, E = 0.0;
    G4double x = 0.0, y = 0.0, z = 0.0, t = 0.0;
    G4int motherPID = 0;
    std::string daughtersStr = "";
    bool isValid = false;
};

class PrimaryGeneratorAction : public G4VUserPrimaryGeneratorAction
{
  public:
    // Constructor only takes filename for custom format
    PrimaryGeneratorAction(const G4String& filename = "particles.txt");
    virtual ~PrimaryGeneratorAction();

    virtual void GeneratePrimaries(G4Event* anEvent);

  private:
    // Members ONLY for Custom File Format
    std::ifstream fCustomParticleFile;
    ParticleData fNextCustomParticleData;
    G4bool fCustomFileEOF;
    G4bool fCustomFileFirstCall;
    G4bool ReadNextCustomParticle(); // Method to read custom file
};

#endif