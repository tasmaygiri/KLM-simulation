#include "MylarHit.hh"
#include "G4UnitsTable.hh" // For G4BestUnit
#include "G4VVisManager.hh"
#include "G4Circle.hh"
#include "G4Colour.hh"
#include "G4VisAttributes.hh"
#include "G4SystemOfUnits.hh" // For MeV etc. in Print

G4ThreadLocal G4Allocator<MylarHit>* MylarHitAllocator = nullptr; // Definition

MylarHit::MylarHit()
 : G4VHit(),
   fTrackID(-1), fParentID(-1), fPDGCode(0), fParticleName(""),
   fEnergyDeposited(0.), fGlobalTime(0.), fPosition(0,0,0),
   fVolumeName(""), fMylarLayerType(""),
   fSectorNumber(-1), fStackNumber(-1),
   fZCellID(-1), fPhiCellID(-1) // Initialize new members
{}

MylarHit::~MylarHit() {}

MylarHit::MylarHit(const MylarHit& right) : G4VHit()
{
  fTrackID          = right.fTrackID;
  fParentID         = right.fParentID;
  fPDGCode          = right.fPDGCode;
  fParticleName     = right.fParticleName;
  fEnergyDeposited  = right.fEnergyDeposited;
  fGlobalTime       = right.fGlobalTime;
  fPosition         = right.fPosition;
  fVolumeName       = right.fVolumeName;
  fMylarLayerType   = right.fMylarLayerType;
  fSectorNumber     = right.fSectorNumber;
  fStackNumber      = right.fStackNumber;
  fZCellID          = right.fZCellID;     // Copy new members
  fPhiCellID        = right.fPhiCellID;   // Copy new members
}

const MylarHit& MylarHit::operator=(const MylarHit& right)
{
  fTrackID          = right.fTrackID;
  fParentID         = right.fParentID;
  fPDGCode          = right.fPDGCode;
  fParticleName     = right.fParticleName;
  fEnergyDeposited  = right.fEnergyDeposited;
  fGlobalTime       = right.fGlobalTime;
  fPosition         = right.fPosition;
  fVolumeName       = right.fVolumeName;
  fMylarLayerType   = right.fMylarLayerType;
  fSectorNumber     = right.fSectorNumber;
  fStackNumber      = right.fStackNumber;
  fZCellID          = right.fZCellID;     // Assign new members
  fPhiCellID        = right.fPhiCellID;   // Assign new members
  return *this;
}

int MylarHit::operator==(const MylarHit& right) const
{
  return ( this == &right ) ? 1 : 0;
}

void MylarHit::Draw()
{
  G4VVisManager* pVVisManager = G4VVisManager::GetConcreteInstance();
  if(pVVisManager)
  {
    G4Circle circle(fPosition);
    circle.SetScreenSize(2.); // Smaller size
    circle.SetFillStyle(G4Circle::filled);
    G4Colour colour(1.,0.5,0.); // Orange
    G4VisAttributes attribs(colour);
    circle.SetVisAttributes(attribs);
    pVVisManager->Draw(circle);
  }
}

void MylarHit::Print()
{
  G4cout << "MylarHit -> TrkID: " << fTrackID << ", PDG: " << fPDGCode << " (" << fParticleName << ")"
         << ", Edep: " << G4BestUnit(fEnergyDeposited,"Energy")
         << ", Pos: " << G4BestUnit(fPosition,"Length")
         << ", Time: " << G4BestUnit(fGlobalTime,"Time")
         << ", Vol: " << fVolumeName << ", Type: " << fMylarLayerType
         << ", Sec: " << fSectorNumber << ", Stack: " << fStackNumber
         << ", ZCell: " << fZCellID << ", PhiCell: " << fPhiCellID
         << G4endl;
}