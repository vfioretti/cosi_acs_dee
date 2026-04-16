#!/usr/bin/env bash

# ============================================================
#  COSI ACS DEE full setup wrapper
#  This script must be SOURCED:
#     source setup.sh
# ============================================================

# --- Safety check: must be sourced ---
if [[ "${BASH_SOURCE[0]}" == "${0}" ]]; then
    echo "ERROR: This script must be sourced:"
    echo "  source setup.sh"
    return 1
fi

SETUP_DIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd )"

source "${SETUP_DIR}/setup_paths.sh" || return 1
source "${SETUP_DIR}/setup_bogemms.sh" || return 1

echo "Full setup completed."

