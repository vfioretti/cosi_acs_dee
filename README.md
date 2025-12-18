# COSI_ACS_DEE


## Setup

From the main directory:

```bash
# clone project
git clone https://github.com/vfioretti/cosi_acs_dee.git
cd cosi_acs_dee

# copy and edit env_bogemms_hpc.sh depending on the environment
cp env_bogemms_hpc.sh.template env_bogemms_hpc.sh

# project setup (builds BoGEMMS with user classes)
source setup.sh

The setup script also defines:

- `COSI_ACS_DEE_DIR` – main repository directory
- `BOGEMMS_USER_DIR` – user BoGEMMS classes (`bogemms_hpc_classes/`)
