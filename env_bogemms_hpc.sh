# ============================================================
# BoGEMMS-HPC dependencies environment
# ============================================================

# --- Geant4 11.1 ---
export G4INSTALL="$HOME/Programs/geant4/11.1/geant4.11.1.p01-install"
. "$G4INSTALL/bin/geant4.sh"

# --- CFITSIO ---
export DYLD_LIBRARY_PATH="$HOME/G4_Projects/cfitsio_install/lib:${DYLD_LIBRARY_PATH}"
export LD_LIBRARY_PATH="$HOME/G4_Projects/cfitsio_install/lib:${LD_LIBRARY_PATH}"

# --- OpenMPI ---
export PATH="$PATH:$HOME/Programs/open-mpi-install/bin"

# --- Geant4 libraries ---
export DYLD_LIBRARY_PATH="$G4INSTALL/lib:${DYLD_LIBRARY_PATH}"
export LD_LIBRARY_PATH="$G4INSTALL/lib:${LD_LIBRARY_PATH}"

