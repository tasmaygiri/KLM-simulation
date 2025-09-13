cmake_minimum_required(VERSION 3.16)
project(KLM_Barrel_Sim)

find_package(Geant4 REQUIRED)

include(${Geant4_USE_FILE})



message(STATUS "HepMC3 not found. HepMC file reading will not be available.")
# Optionally, try to find HepMC2 here
find_package(HepMC)
if(HepMC_FOUND)
  message(STATUS "Found HepMC (version 2): ${HepMC_VERSION_STRING}")
  include_directories(${HepMC_INCLUDE_DIR})
  set(HEPMC_LIBRARIES ${HepMC_LIBRARIES})
else()
  message(STATUS "HepMC (version 2) also not found.")
endif()

# ... (add_executable, target_include_directories) ...

# Link libraries


set(HEADER_FILES
  include/DetectorConstruction.hh
  include/ActionInitialization.hh
  include/PrimaryGeneratorAction.hh
  include/RunAction.hh          # Assuming you have this
  include/EventAction.hh
  include/MylarSD.hh
  include/MylarHit.hh         # <<< ADD MylarHit.hh if it's separate
  include/SteppingAction.hh   # If you use it
  include/G4HepMCInterface.hh
  # include/TrackingAction.hh # If removed
)

set(SOURCE_FILES
  klm_barrel.cc
  src/DetectorConstruction.cc
  src/ActionInitialization.cc
  src/PrimaryGeneratorAction.cc
  src/RunAction.cc            # Assuming you have this
  src/EventAction.cc
  src/MylarSD.cc
  src/MylarHit.cc           # <<< ADD MylarHit.cc HERE
  src/SteppingAction.cc     # If you use it
  src/G4HepMCInterface.cc
  # src/TrackingAction.cc   # If removed
)

# Add the executable using the variables
add_executable(klm_barrel ${SOURCE_FILES})

# Include directories
target_include_directories(klm_barrel PUBLIC ${PROJECT_SOURCE_DIR}/include)

# Link libraries
target_link_libraries(klm_barrel
    ${Geant4_LIBRARIES}
    ${HEPMC_LIBRARIES} # Add HepMC libraries
)

# Define source groups for IDEs (optional)
source_group(Source FILES ${SOURCE_FILES})
source_group(Headers FILES ${HEADER_FILES})

# ... (rest of CMakeLists.txt, e.g., copying macros) ...

message(STATUS "Geant4 libraries: ${Geant4_LIBRARIES}")





# ... (Find Geant4) ...

# --- Find HepMC (Version 3 example) ---
# You might need to set HepMC3_DIR if it's not in a standard location
# e.g., cmake .. -DHepMC3_DIR=/path/to/hepmc3/install/lib/cmake/HepMC3
