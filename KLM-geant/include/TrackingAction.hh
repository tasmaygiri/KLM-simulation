#ifndef TRACKINGACTION_HH
#define TRACKINGACTION_HH

#include "G4UserTrackingAction.hh"
#include "globals.hh"

// Forward declarations
class G4Track;
class EventAction; // Needs to know about EventAction to store data

class TrackingAction : public G4UserTrackingAction
{
public:
  // Constructor needs pointer to EventAction
  TrackingAction(EventAction* eventAction);
  virtual ~TrackingAction();

  // Method called at the beginning of tracking a new particle
  // This is where we will collect the initial information for the table
  virtual void PreUserTrackingAction(const G4Track* track);

  // Method called at the end of tracking (optional, we might not need it for this table)
  // virtual void PostUserTrackingAction(const G4Track* track);

private:
  EventAction* fEventAction; // Pointer to the event action instance
};

#endif // TRACKINGACTION_HH