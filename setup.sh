#!/usr/bin/env bash

# ============================================================
#  COSI ACS DEE setup script
#  This script must be SOURCED:
#     source setup.sh
# ============================================================

# --- Safety check: must be sourced ---
if [[ "${BASH_SOURCE[0]}" == "${0}" ]]; then
    echo "ERROR: This script must be sourced:"
    echo "  source setup.sh"
    return 1
fi

# --- Non-interactive mode (for SLURM / container exec) ---
# If SETUP_NON_INTERACTIVE=1, any prompt will default to "No".
SETUP_NON_INTERACTIVE="${SETUP_NON_INTERACTIVE:-0}"

# --- Determine number of parallel build jobs ---
if command -v nproc >/dev/null 2>&1; then
    NPROC=$(nproc)
elif command -v sysctl >/dev/null 2>&1; then
    NPROC=$(sysctl -n hw.ncpu)
else
    NPROC=1
fi
echo "Using ${NPROC} parallel build jobs"

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
if [[ ":$PYTHONPATH:" != *":$COSI_ACS_DEE_DIR:"* ]]; then
    export PYTHONPATH="${COSI_ACS_DEE_DIR}:${PYTHONPATH}"
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
# External dependency
# ============================================================

BOGEMMS_REPO_NAME="BoGEMMS-HPC"
BOGEMMS_REPO_URL="https://www.ict.inaf.it/gitlab/icsc_g4_hpc/BoGEMMS-HPC"

EXTERNAL_BASE_DIR="${COSI_ACS_DEE_DIR}/external"
BOGEMMS_REPO_DIR="${EXTERNAL_BASE_DIR}/${BOGEMMS_REPO_NAME}"
BOGEMMS_BUILD_DIR="${EXTERNAL_BASE_DIR}/${BOGEMMS_REPO_NAME}-build"

mkdir -p "${EXTERNAL_BASE_DIR}"

if [[ ! -d "${BOGEMMS_REPO_DIR}" ]]; then
    echo
    echo "Required BoGEMMS-HPC repository not found:"
    echo "  ${BOGEMMS_REPO_NAME}"
    echo

    if [[ "${SETUP_NON_INTERACTIVE}" == "1" ]]; then
        answer="N"
        echo "Non-interactive mode: defaulting to 'No' (skip clone/build)."
    else
        read -p "Do you want to clone and build it now? [y/N] " answer
    fi

    case "$answer" in
        y|Y )
            echo "Cloning ${BOGEMMS_REPO_NAME}..."
            git clone "${BOGEMMS_REPO_URL}" "${BOGEMMS_REPO_DIR}" || {
                echo "ERROR: git clone failed"
                return 1
            }
            ;;
        * )
            echo "Skipping external dependency installation."
            return 0
            ;;
    esac
else
    echo "BoGEMMS-HPC repository already present:"
    echo "  ${BOGEMMS_REPO_DIR}"
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
    echo "       Please set Geant4_DIR (or G4INSTALL) before sourcing setup.sh."
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

# ============================================================
# Build BoGEMMS-HPC with user classes
# ============================================================

echo
echo "Configuring build for ${BOGEMMS_REPO_NAME}..."

mkdir -p "${BOGEMMS_BUILD_DIR}"
cd "${BOGEMMS_BUILD_DIR}" || return 1

if [[ ! -f "Makefile" ]]; then
    cmake \
        -DGeant4_DIR="${Geant4_DIR}" \
        -DUSER_CLASSES=ON \
        -DUSER_CLASSES_PATH="${BOGEMMS_USER_DIR}" \
        "${BOGEMMS_REPO_DIR}" || {
            echo "ERROR: cmake configuration failed"
            return 1
        }
else
    echo "CMake already configured, skipping configuration."
fi

echo "Building ${BOGEMMS_REPO_NAME}..."
make -j"${NPROC}" || {
    echo "ERROR: build failed"
    return 1
}

cd "${COSI_ACS_DEE_DIR}"

# ============================================================
# Add CAD files into BoGEMMS-HPC (non-destructive)
# ============================================================

CAD_SRC_DIR="${COSI_ACS_DEE_DIR}/cad_files"
CAD_DST_DIR="${BOGEMMS_REPO_DIR}/cad_files"

if [[ ! -d "${CAD_SRC_DIR}" ]]; then
    echo "WARNING: CAD source directory not found:"
    echo "         ${CAD_SRC_DIR}"
else
    mkdir -p "${CAD_DST_DIR}"
    rsync -av --ignore-existing "${CAD_SRC_DIR}/" "${CAD_DST_DIR}/"
fi

echo
echo "${BOGEMMS_REPO_NAME} successfully built."

# --- exporting PATH ---
export PATH="${PATH}:${BOGEMMS_BUILD_DIR}"

echo "Setup completed."

