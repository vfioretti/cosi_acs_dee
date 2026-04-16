#!/usr/bin/env bash

# ============================================================
#  COSI ACS DEE environment/path setup
#  This script must be SOURCED:
#     source setup_paths.sh
# ============================================================

# --- Safety check: must be sourced ---
if [[ "${BASH_SOURCE[0]}" == "${0}" ]]; then
    echo "ERROR: This script must be sourced:"
    echo "  source setup_paths.sh"
    return 1
fi

# --- Determine main directory ---
export COSI_ACS_DEE_DIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd )"
echo "COSI_ACS_DEE_DIR set to:"
echo "  ${COSI_ACS_DEE_DIR}"

# --- BoGEMMS user classes directory ---
export BOGEMMS_USER_DIR="${COSI_ACS_DEE_DIR}/bogemms_hpc_classes"
echo "BOGEMMS_USER_DIR set to:"
echo "  ${BOGEMMS_USER_DIR}"

if [[ ! -d "${BOGEMMS_USER_DIR}" ]]; then
    echo "WARNING: BOGEMMS_USER_DIR does not exist yet."
    echo "         Expected at: ${BOGEMMS_USER_DIR}"
fi

# --- Optional: add to PYTHONPATH ---
if [[ ":${PYTHONPATH:-}:" != *":${COSI_ACS_DEE_DIR}:"* ]]; then
    export PYTHONPATH="${COSI_ACS_DEE_DIR}:${PYTHONPATH:-}"
    echo "Added COSI_ACS_DEE_DIR to PYTHONPATH"
fi

# ============================================================
# Load BoGEMMS-HPC dependency environment (if available)
# ============================================================

if [[ -f "${COSI_ACS_DEE_DIR}/env_bogemms_hpc.sh" ]]; then
    echo "Sourcing BoGEMMS-HPC environment..."
    source "${COSI_ACS_DEE_DIR}/env_bogemms_hpc.sh"
else
    echo "WARNING: env_bogemms_hpc.sh not found"
    echo "         BoGEMMS-HPC dependencies may not be set"
fi

# ============================================================
# Helper: infer Geant4_DIR robustly
# ============================================================

infer_geant4_dir() {
    # If Geant4_DIR already set and valid, keep it
    if [[ -n "${Geant4_DIR:-}" && -d "${Geant4_DIR}" ]]; then
        echo "${Geant4_DIR}"
        return 0
    fi

    # Try from G4INSTALL
    if [[ -n "${G4INSTALL:-}" ]]; then
        if [[ -d "${G4INSTALL}/lib/cmake/Geant4" ]]; then
            echo "${G4INSTALL}/lib/cmake/Geant4"
            return 0
        fi
        if [[ -d "${G4INSTALL}/lib64/cmake/Geant4" ]]; then
            echo "${G4INSTALL}/lib64/cmake/Geant4"
            return 0
        fi
    fi

    # Try from INSTALL_PREFIX (container toolchain)
    if [[ -n "${INSTALL_PREFIX:-}" ]]; then
        if [[ -d "${INSTALL_PREFIX}/geant4-v11-1.0-install/lib/cmake/Geant4" ]]; then
            echo "${INSTALL_PREFIX}/geant4-v11-1.0-install/lib/cmake/Geant4"
            return 0
        fi
        if [[ -d "${INSTALL_PREFIX}/geant4-v11-1.0-install/lib64/cmake/Geant4" ]]; then
            echo "${INSTALL_PREFIX}/geant4-v11-1.0-install/lib64/cmake/Geant4"
            return 0
        fi
    fi

    return 1
}

Geant4_DIR_INFERRED="$(infer_geant4_dir || true)"
if [[ -z "${Geant4_DIR_INFERRED}" ]]; then
    echo "ERROR: Could not infer Geant4_DIR."
    echo "       Please set Geant4_DIR (or G4INSTALL) before sourcing setup_paths.sh."
    echo "       Examples:"
    echo "         export Geant4_DIR=/SOFTWARE/geant4-v11-1.0-install/lib/cmake/Geant4"
    echo "         export G4INSTALL=/SOFTWARE/geant4-v11-1.0-install"
    return 1
fi
export Geant4_DIR="${Geant4_DIR_INFERRED}"
echo "Geant4_DIR set to:"
echo "  ${Geant4_DIR}"

# Optional: ensure Geant4 runtime libs are in LD_LIBRARY_PATH (Linux)
if [[ -n "${G4INSTALL:-}" ]]; then
    if [[ -d "${G4INSTALL}/lib" && ":${LD_LIBRARY_PATH:-}:" != *":${G4INSTALL}/lib:"* ]]; then
        export LD_LIBRARY_PATH="${G4INSTALL}/lib:${LD_LIBRARY_PATH:-}"
    fi
    if [[ -d "${G4INSTALL}/lib64" && ":${LD_LIBRARY_PATH:-}:" != *":${G4INSTALL}/lib64:"* ]]; then
        export LD_LIBRARY_PATH="${G4INSTALL}/lib64:${LD_LIBRARY_PATH:-}"
    fi
fi

echo "Path/environment setup completed."
