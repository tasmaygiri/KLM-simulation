#ifndef ACTIONINITIALIZATION_HH
#define ACTIONINITIALIZATION_HH

#include "G4VUserActionInitialization.hh"
#include "globals.hh" 

class ActionInitialization : public G4VUserActionInitialization
{
  public:

    ActionInitialization(const G4String& inputFilename = "particles.txt");
    virtual ~ActionInitialization();

    virtual void BuildForMaster() const;
    virtual void Build() const;

  private:
    G4String fInputFilename; // Storing filename
};

#endif

