#include "G4HepMCInterface.hh"

#include "G4Event.hh"
#include "G4PrimaryParticle.hh"
#include "G4PrimaryVertex.hh"
#include "G4Log.hh"
#include "G4RunManager.hh"
#include "G4SystemOfUnits.hh"
#include "G4PhysicalConstants.hh"

// HepMC2 Includes
#include "HepMC/IO_GenEvent.h"
#include "HepMC/GenEvent.h"
#include "HepMC/GenParticle.h"
#include "HepMC/GenVertex.h"
// #include "HepMC/Units.h" // We use G4 units

#include <iostream>
#include <map>
#include <ios> // Required for std::ios::iostate constants

// Constructor: Initialize the reader
G4HepMCInterface::G4HepMCInterface(const G4String& hepmcFileName)
{
    m_asciiInput = new HepMC::IO_GenEvent(hepmcFileName.c_str(), std::ios::in);

    // Check if the file stream is good *immediately* after opening
    if (m_asciiInput->rdstate() != std::ios::goodbit) {
         G4Exception("G4HepMCInterface::G4HepMCInterface",
                    "CannotOpenFile", FatalException,
                    ("Could not open HepMC file: " + hepmcFileName + " or stream is bad.").c_str());
    } else {
        G4cout << "G4HepMCInterface: Opened HepMC file: " << hepmcFileName << G4endl;
        G4cout << "G4HepMCInterface: Assuming HepMC units are GeV and mm." << G4endl;
    }
}

// Destructor: Clean up the reader
G4HepMCInterface::~G4HepMCInterface()
{
    delete m_asciiInput;
    G4cout << "G4HepMCInterface: Reader deleted." << G4endl;
}

// GeneratePrimaries: Called by Geant4 for each event
void G4HepMCInterface::GeneratePrimaries(G4Event* anEvent)
{
    if (!m_asciiInput) {
         G4Exception("G4HepMCInterface::GeneratePrimaries",
                    "ReaderNotInitialized", FatalException,
                    "HepMC::IO_GenEvent reader is not initialized!");
        return;
    }

    // Use read_next_event() which allocates a new GenEvent object
    HepMC::GenEvent* hepmcEvt = m_asciiInput->read_next_event();

    // Check if event reading failed (returns NULL on error or EOF)
    if (!hepmcEvt) {
        int state = m_asciiInput->rdstate(); // Check stream state after failed read
        bool is_eof = bool(state & std::ios::eofbit);
        bool is_bad = bool(state & std::ios::badbit);
        bool is_fail= bool(state & std::ios::failbit);

        if (is_eof && !is_bad && !is_fail) { // Clean End-Of-File
            G4cout << "G4HepMCInterface: End of HepMC file reached." << G4endl;
        } else { // Actual read error
            G4Exception("G4HepMCInterface::GeneratePrimaries",
                        "ReadError", JustWarning, // JustWarning allows run to potentially continue if desired
                        "Error reading HepMC event. File might be corrupted or ended unexpectedly.");
             G4cout << "     Stream State Bits: eof=" << is_eof << " fail=" << is_fail << " bad=" << is_bad << G4endl;
        }
        // Signal G4RunManager to stop the run smoothly in either case (EOF or error)
        G4RunManager::GetRunManager()->AbortRun(true); // Soft abort
        return; // Do not proceed to conversion
    }

    // Optionally print the event (matches verbose > 0 in the example)
    if(verboseLevel > 0) {
        G4cout << "================= HepMC Event ==================" << G4endl;
        hepmcEvt->print();
        G4cout << "================================================" << G4endl;
    }

    // Convert the read HepMC event to Geant4 primaries
    if (!ConvertHepMCEvent(hepmcEvt, anEvent)) {
         G4Exception("G4HepMCInterface::GeneratePrimaries",
                    "ConversionError", JustWarning,
                    "Failed to convert HepMC event to Geant4 primaries.");
         // Decide if you want to skip event or stop run
         // G4RunManager::GetRunManager()->AbortRun(true);
    }

    // *** CRUCIAL: Delete the event object allocated by read_next_event() ***
    delete hepmcEvt;
    hepmcEvt = nullptr; // Good practice to nullify pointer after delete
}

// --- Conversion Method --- (Modified to accept event pointer)
// Converts the HepMC event (hepmcEvt) into Geant4 primaries
// Focuses on *final state* particles (status == 1).
// ASSUMES HepMC units are GeV and mm.
G4bool G4HepMCInterface::ConvertHepMCEvent(const HepMC::GenEvent* hepmcEvt, G4Event* g4Evt)
{
    if (!hepmcEvt) {
        G4cout<< "G4HepMCInterface::ConvertHepMCEvent - null HepMC event pointer passed." << G4endl;
        return false;
    }
    if (!g4Evt) {
         G4cout << "G4HepMCInterface::ConvertHepMCEvent - null G4Event pointer passed." << G4endl;
         return false;
    }

    // Map to store G4PrimaryVertex pointers, keyed by HepMC vertex barcode
    std::map<int, G4PrimaryVertex*> geant4Vertices;

    // Iterate over HepMC particles using HepMC iterators
    for (HepMC::GenEvent::particle_const_iterator piter = hepmcEvt->particles_begin();
         piter != hepmcEvt->particles_end(); ++piter)
    {
        HepMC::GenParticle* hepmcParticle = *piter;

        // Select particles to be converted to G4PrimaryParticle (status == 1)
        if (hepmcParticle->status() == 1) {
            HepMC::GenVertex* prodVertexHepMC = hepmcParticle->production_vertex();
            if (!prodVertexHepMC) {
                G4cout << "G4HepMCInterface: Final state particle (PDG: "
                               << hepmcParticle->pdg_id() << ", Barcode: " << hepmcParticle->barcode()
                               << ") has no production vertex! Skipping." << G4endl;
                continue;
            }

            int vertexBarcode = prodVertexHepMC->barcode();
            G4PrimaryVertex* g4Vertex = nullptr;

            // Check if G4 vertex already exists for this barcode
            auto vtxIt = geant4Vertices.find(vertexBarcode);
            if (vtxIt == geant4Vertices.end()) {
                // Create new G4PrimaryVertex
                HepMC::FourVector pos = prodVertexHepMC->position();
                G4double x = pos.x() * lengthUnit; // Assumes mm
                G4double y = pos.y() * lengthUnit; // Assumes mm
                G4double z = pos.z() * lengthUnit; // Assumes mm
                G4double t = (pos.t() * lengthUnit) / c_light; // Convert t_mm to G4 time (ns)

                g4Vertex = new G4PrimaryVertex(x, y, z, t);
                geant4Vertices[vertexBarcode] = g4Vertex;
                g4Evt->AddPrimaryVertex(g4Vertex); // Add to the G4Event
            } else {
                g4Vertex = vtxIt->second; // Use existing vertex
            }

            // Create G4PrimaryParticle
            HepMC::FourVector mom = hepmcParticle->momentum();
            G4double px = mom.px() * momentumUnit; // Assumes GeV -> MeV
            G4double py = mom.py() * momentumUnit; // Assumes GeV -> MeV
            G4double pz = mom.pz() * momentumUnit; // Assumes GeV -> MeV
            G4int pdgCode = hepmcParticle->pdg_id();

            G4PrimaryParticle* g4Particle = new G4PrimaryParticle(pdgCode, px, py, pz);
            g4Vertex->SetPrimary(g4Particle); // Add particle to vertex
        }
    } // End loop over particles

    if (geant4Vertices.empty() && hepmcEvt->particles_size() > 0) {
         G4cout<< "G4HepMCInterface: No final state particles (status=1) found in HepMC event "
                        << hepmcEvt->event_number() << "." << G4endl;
    }

    return true; // Conversion successful (or event had no final state particles)
}