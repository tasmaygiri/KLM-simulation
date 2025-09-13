#ifndef MYLARSD_HH
#define MYLARSD_HH

#include "G4VSensitiveDetector.hh"
#include "MylarHit.hh" // Include the Hit class definition
#include <vector>

// Forward declarations
class G4Step;
class G4HCofThisEvent;
class DetectorConstruction; // To get dimensions

class MylarSD : public G4VSensitiveDetector
{
public:
  // Constructor needs the name of the SD and its hits collection
  // Also pass DetectorConstruction to get geometry info
  MylarSD(const G4String& name,
          const G4String& hitsCollectionName,
          DetectorConstruction* detConstruction);
  virtual ~MylarSD();

  // Called at the beginning of each event
  virtual void Initialize(G4HCofThisEvent* hce);

  // Called for each step in a sensitive volume
  virtual G4bool ProcessHits(G4Step* aStep, G4TouchableHistory* ROhist);

  // Called at the end of each event (optional)
  // virtual void EndOfEvent(G4HCofThisEvent* hce);

private:
  MylarHitsCollection* fHitsCollection;
  DetectorConstruction* fDetConstruction; // To get KLM dimensions for grid
};

#endif