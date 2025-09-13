// MylarHit.hh or MylarSD.hh

#ifndef MYLARHIT_HH
#define MYLARHIT_HH

#include "G4VHit.hh"
#include "G4THitsCollection.hh"
#include "G4Allocator.hh" // For G4Allocator
#include "G4ThreeVector.hh"
#include "globals.hh"

class MylarHit : public G4VHit
{
public:
  MylarHit();
  MylarHit(const MylarHit& right);
  virtual ~MylarHit();

  const MylarHit& operator=(const MylarHit& right);
  int operator==(const MylarHit& right) const;

  // <<< ADD DECLARATIONS FOR CUSTOM MEMORY MANAGEMENT >>>
  inline void* operator new(size_t);
  inline void  operator delete(void* aHit);
  // <<< END OF ADDED DECLARATIONS >>>

  virtual void Draw();
  virtual void Print();

  // Setters
  void SetTrackID(G4int id) { fTrackID = id; }
  void SetParentID(G4int id) { fParentID = id; }
  void SetPDGCode(G4int code) { fPDGCode = code; }
  void SetParticleName(const G4String& name) { fParticleName = name; }
  void SetEnergyDeposited(G4double edep) { fEnergyDeposited = edep; }
  void SetGlobalTime(G4double time) { fGlobalTime = time; }
  void SetPosition(const G4ThreeVector& pos) { fPosition = pos; }
  void SetVolumeName(const G4String& name) { fVolumeName = name; }
  void SetMylarLayerType(const G4String& type) { fMylarLayerType = type; }
  void SetSectorNumber(G4int num) { fSectorNumber = num; }
  void SetStackNumber(G4int num) { fStackNumber = num; }
  void SetZCellID(G4int id) { fZCellID = id; }
  void SetPhiCellID(G4int id) { fPhiCellID = id; }

  // Getters
  G4int GetTrackID() const { return fTrackID; }
  G4int GetParentID() const { return fParentID; }
  G4int GetPDGCode() const { return fPDGCode; }
  const G4String& GetParticleName() const { return fParticleName; }
  G4double GetEnergyDeposited() const { return fEnergyDeposited; }
  G4double GetGlobalTime() const { return fGlobalTime; }
  const G4ThreeVector& GetPosition() const { return fPosition; }
  const G4String& GetVolumeName() const { return fVolumeName; }
  const G4String& GetMylarLayerType() const { return fMylarLayerType; }
  G4int GetSectorNumber() const { return fSectorNumber; }
  G4int GetStackNumber() const { return fStackNumber; }
  G4int GetZCellID() const { return fZCellID; }
  G4int GetPhiCellID() const { return fPhiCellID; }

private:
  G4int fTrackID;
  G4int fParentID;
  G4int fPDGCode;
  G4String fParticleName;
  G4double fEnergyDeposited;
  G4double fGlobalTime;
  G4ThreeVector fPosition;
  G4String fVolumeName;
  G4String fMylarLayerType;
  G4int fSectorNumber;
  G4int fStackNumber;
  G4int fZCellID;
  G4int fPhiCellID;
};

typedef G4THitsCollection<MylarHit> MylarHitsCollection;

// The G4ThreadLocal G4Allocator should be declared extern here
// and defined once in MylarHit.cc (or a relevant .cc file).
extern G4ThreadLocal G4Allocator<MylarHit>* MylarHitAllocator;

// Definitions of new and delete can stay inline here AFTER the class definition
// as long as they are declared inside the class.
inline void* MylarHit::operator new(size_t)
{
  if(!MylarHitAllocator) MylarHitAllocator = new G4Allocator<MylarHit>;
  return (void*) MylarHitAllocator->MallocSingle();
}

inline void MylarHit::operator delete(void* aHit)
{
  MylarHitAllocator->FreeSingle((MylarHit*) aHit);
}

#endif