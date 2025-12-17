# COSI_ACS_DEE


## Setup

From the main directory:

```bash
# clone project
git clone https://github.com/vfioretti/cosi_acs_dee.git
cd cosi_acs_dee

# edit env_bogemms_hpc.sh if needed
source env_bogemms_hpc.sh

# project setup (builds BoGEMMS with user classes)
source setup.sh

The setup script also defines:

- `COSI_ACS_DEE_DIR` – main repository directory
- `BOGEMMS_USER_DIR` – user BoGEMMS classes (`bogemms_hpc_classes/`)
