# COSI_ACS_DEE


## Setup

```bash
# clone project
git clone https://github.com/vfioretti/cosi_acs_dee.git
cd cosi_acs_dee

# copy and edit env_bogemms_hpc.sh depending if using local installation or the singularity container
cp env_bogemms_hpc.sh.template env_bogemms_hpc.sh

### Local installation

source env_bogemms_hpc.sh

# project setup (builds BoGEMMS with user classes)
source setup.sh

The setup script also defines:

- `COSI_ACS_DEE_DIR` – main repository directory
- `BOGEMMS_USER_DIR` – user BoGEMMS classes (`bogemms_hpc_classes/`)

### Using the container as toolchain for the BoGEMMS-HPC installation

singularity shell g4_11_1_cosi.sif
source env_bogemms_hpc.sh
source setup.sh

Then exit container before running the simulations.
