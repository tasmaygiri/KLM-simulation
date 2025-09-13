# Geant4-based simulation of the Belle II KLM (K Long and Muon) detector subsystem

This project contains a C++ Geant4 application to simulate particle interactions in the Belle II KLM detector. It implements detector geometry, event generation, hit processing, and visualization macros for studying detector response.

---

## Features
- Detector construction for the KLM barrel geometry (`DetectorConstruction.cc` / `DetectorConstruction.hh`).  
- Primary particle generation (`PrimaryGeneratorAction`).  
- Event, run, stepping, and tracking actions for flexible control of the simulation.  
- Sensitive detector and hit classes for Mylar scintillator strips (`MylarSD`, `MylarHit`).  
- Visualization setup via Geant4 macro (`init_vis.mac`).  
- Example main application (`klm_barrel.cc`).  

---
## Requirements
- [Geant4](https://geant4.web.cern.ch/) (with data libraries and visualization options installed)  
- CMake (≥ 3.10)  
- A C++17 compiler (e.g., `g++`, `clang`)  

---


## Note (when uploading)
Still work is going on, hence some files are just there and not being used while cmake-ing the application. Check CMakeLists.txt. Regardless of that, current state should work.

## Building
Clone the repository and build with CMake:

```bash
git clone https://github.com/username/KLM-geant.git
cd KLM-geant
mkdir build && cd build
cmake ..
make -j4
```

## Running

The program can take particle list files, HepMC files, and optional macro files as input.

1. Run with a text file of particles:
```bash
./klm_barrel particles.txt
```

The particles.txt file should contain particles in the following format (one per line):
```
EvtID PID Px Py Pz E X Y Z T Parent_ID Sisters
1 130 -0.309704 0.574027 0.375492 0.902238 -0.035242 0.032906 0.205898 0.837699 311 130
```

2. Run with a HepMC file

```bash
./klm_barrel events.hepmc
```

This allows you to provide events generated externally (e.g., from Pythia or another generator).

To use macro:

```bash
./klm_barrel events.hepmc init_vis.mac
```

