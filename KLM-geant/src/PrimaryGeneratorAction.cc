#include "PrimaryGeneratorAction.hh" // Header for this class

// Geant4 includes
#include "G4Event.hh"
#include "G4ParticleTable.hh"
#include "G4ParticleDefinition.hh"
#include "G4PrimaryParticle.hh"
#include "G4PrimaryVertex.hh"
#include "G4SystemOfUnits.hh"
#include "G4ios.hh"
#include "G4RunManager.hh"
#include "G4UnitsTable.hh"
#include "G4ThreeVector.hh"

// C++ includes
#include <sstream>
#include <iomanip>

// Constructor (Only for custom file)
PrimaryGeneratorAction::PrimaryGeneratorAction(const G4String& filename)
 : G4VUserPrimaryGeneratorAction(),
   fCustomFileEOF(false),
   fCustomFileFirstCall(true)
   // All HepMC members removed
{
    // This constructor is now ONLY called for custom format files
    G4cout << "----> PrimaryGeneratorAction (Custom Format): Opening file: " << filename << G4endl;
    fCustomParticleFile.open(filename.c_str());
    if (!fCustomParticleFile.is_open()) {
        G4ExceptionDescription msg;
        msg << " PrimaryGeneratorAction (Custom Format): Cannot open input file: " << filename;
        G4Exception("PrimaryGeneratorAction::PrimaryGeneratorAction", "MyCodeCustom001", FatalException, msg);
    }
}

// Destructor (Only for custom file)
PrimaryGeneratorAction::~PrimaryGeneratorAction()
{
    if (fCustomParticleFile.is_open()) {
        fCustomParticleFile.close();
        G4cout << "----> Closed custom particle input file." << G4endl;
    }
}

// Custom File Format Reader (Renamed from ReadNextCustomParticle to keep it unique if it was in header)
// But since it's private and only one generator uses it now, name ReadNextParticle is fine.
G4bool PrimaryGeneratorAction::ReadNextCustomParticle() // Or just ReadNextParticle if header only has this one
{
    if (fCustomFileEOF || !fCustomParticleFile.is_open()) {
        fNextCustomParticleData.isValid = false;
        return false;
    }
    std::string line;
    if (std::getline(fCustomParticleFile, line)) {
        std::stringstream ss(line);
        if (ss >> fNextCustomParticleData.eventID
               >> fNextCustomParticleData.pdgID
               >> fNextCustomParticleData.px >> fNextCustomParticleData.py >> fNextCustomParticleData.pz >> fNextCustomParticleData.E
               >> fNextCustomParticleData.x >> fNextCustomParticleData.y >> fNextCustomParticleData.z >> fNextCustomParticleData.t
               >> fNextCustomParticleData.motherPID)
        {
             std::getline(ss, fNextCustomParticleData.daughtersStr);
             fNextCustomParticleData.daughtersStr.erase(0, fNextCustomParticleData.daughtersStr.find_first_not_of(" \t"));
             fNextCustomParticleData.isValid = true;
             return true;
        } else {
            G4cerr << "Warning [PrimaryGeneratorAction::ReadNextCustomParticle]: Failed to parse line: " << line << G4endl;
            fNextCustomParticleData.isValid = false;
            return ReadNextCustomParticle();
        }
    } else {
        fCustomFileEOF = true;
        fNextCustomParticleData.isValid = false;
        G4cout << "----> End of custom particle input file reached." << G4endl;
        return false;
    }
}


// Main GeneratePrimaries method - ONLY for custom file format
void PrimaryGeneratorAction::GeneratePrimaries(G4Event* anEvent)
{
    // This method is now ONLY called if this class was instantiated (i.e., for custom files)
    if (fCustomFileFirstCall || !fNextCustomParticleData.isValid) {
        if (!ReadNextCustomParticle()) {
            G4cout << "[PrimaryGeneratorAction::GeneratePrimaries] "
                   << "Custom file: No more particles. Aborting run." << G4endl;
            G4RunManager::GetRunManager()->AbortRun(true);
            anEvent->SetEventAborted();
            return;
        }
        fCustomFileFirstCall = false;
    }
    if (!fNextCustomParticleData.isValid) {
        G4cout << "[PrimaryGeneratorAction::GeneratePrimaries] "
               << "Custom file: Aborting event " << anEvent->GetEventID() << ": No valid particle data." << G4endl;
        anEvent->SetEventAborted();
        return;
    }

    G4int currentFileEventID = fNextCustomParticleData.eventID;
    G4cout << "\n[PrimaryGeneratorAction] Custom File: Processing G4Event " << anEvent->GetEventID()
           << ", for FileEventID " << currentFileEventID << "." << G4endl;

    bool printedHeader = false;
    G4int particlesInEvent = 0;

    while (fNextCustomParticleData.isValid && fNextCustomParticleData.eventID == currentFileEventID)
    {
        if (!printedHeader) {
            G4cout << "--- G4Event " << anEvent->GetEventID() << " (File Event " << currentFileEventID << ") Custom File Primaries ---" << G4endl;
            G4cout << std::setw(8) << "PDG ID" << " | "
                   << std::setw(18) << "Particle Name" << " | "
                   << std::setw(22) << "Momentum [GeV/c]" << " | "
                   << std::setw(20) << "Input Energy [GeV]" << " | "
                   << std::setw(25) << "Vertex (x,y,z) [mm]" << " | "
                   << std::setw(12) << "Time [ns]"
                   << G4endl;
            G4cout << "------------------------------------------------------------------------------------------------------------------------------------------" << G4endl;
            printedHeader = true;
        }

        G4ParticleTable* particleTable = G4ParticleTable::GetParticleTable();
        G4ParticleDefinition* particleDef = particleTable->FindParticle(fNextCustomParticleData.pdgID);

        if (!particleDef) {
            G4cerr << "Warning [PrimaryGeneratorAction]: Custom File: Unknown PDG ID " << fNextCustomParticleData.pdgID
                   << " in file event " << currentFileEventID << ". Skipping particle." << G4endl;
            if (!ReadNextCustomParticle()) break;
            continue;
        }

        G4double xPos = fNextCustomParticleData.x * mm;
        G4double yPos = fNextCustomParticleData.y * mm;
        G4double zPos = fNextCustomParticleData.z * mm;
        G4double time = fNextCustomParticleData.t * ns;
        G4PrimaryVertex* vertex = new G4PrimaryVertex(xPos, yPos, zPos, time);

        G4double pxGeV = fNextCustomParticleData.px;
        G4double pyGeV = fNextCustomParticleData.py;
        G4double pzGeV = fNextCustomParticleData.pz;
        G4PrimaryParticle* particle = new G4PrimaryParticle(particleDef,
                                                            pxGeV * GeV,
                                                            pyGeV * GeV,
                                                            pzGeV * GeV);
        vertex->SetPrimary(particle);
        anEvent->AddPrimaryVertex(vertex);
        particlesInEvent++;

        G4ThreeVector mom = particle->GetMomentum();
        G4cout << std::setw(8) << particleDef->GetPDGEncoding() << " | "
               << std::setw(18) << particleDef->GetParticleName() << " | "
               << std::fixed << std::setprecision(3)
               << "(" << std::setw(7) << mom.x()/GeV << ","
               << std::setw(7) << mom.y()/GeV << ","
               << std::setw(7) << mom.z()/GeV << ")" << std::setprecision(6) << std::defaultfloat
               << " | "
               << std::setw(20) << fNextCustomParticleData.E
               << " | ("
               << std::fixed << std::setprecision(1)
               << std::setw(7) << vertex->GetX0()/mm << ","
               << std::setw(7) << vertex->GetY0()/mm << ","
               << std::setw(7) << vertex->GetZ0()/mm << ")" << std::setprecision(6) << std::defaultfloat
               << " | "
               << std::fixed << std::setprecision(1)
               << std::setw(12) << vertex->GetT0()/ns << std::setprecision(6) << std::defaultfloat
               << G4endl;

        if (!ReadNextCustomParticle()) break;
    }

    if (printedHeader) {
         G4cout << "------------------------------------------------------------------------------------------------------------------------------------------" << G4endl;
    }
    if (particlesInEvent == 0 && fNextCustomParticleData.isValid) {
        G4cout << "Warning [PrimaryGeneratorAction]: Custom File: No particles generated for G4Event " << anEvent->GetEventID()
               << " (target file event ID " << currentFileEventID << " was processed or skipped)." << G4endl;
    } else if (particlesInEvent > 0) {
        G4cout << "[PrimaryGeneratorAction] Custom File: G4Event " << anEvent->GetEventID() << " finished processing "
               << particlesInEvent << " particles from file event " << currentFileEventID << "." << G4endl;
    }
}