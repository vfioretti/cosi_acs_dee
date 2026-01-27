# COSI_ACS_DEE

This repository collects all scripts and classes to build a Geant4 simulation of the COSI (https://cosi.ssl.berkeley.edu/) ACS crystals illuminated by calibration sources.
The Geant4 simulation uses the BoGEMMS-HPC simulation framework and includes the optical physics to reproduce the generation of optical photons in the crystal. The goal is to benchmark the simulation by comparing it with the calibration measurements, and the build the energy correction file to be used in the Nuclearizer Detector Effect Engine.
The repository also includes tools to analyse the calibration measurements and build the ADC-energy relation.

## References
- A. Ciabattoni et al. "Benchmarking of Geant4 simulations for the COSI Anticoincidence System", Exp. Astr. 60, 1, 2025

## Setup

### clone project
```bash
git clone https://github.com/vfioretti/cosi_acs_dee.git
cd cosi_acs_dee
```

### copy and edit env_bogemms_hpc.sh depending if using local installation or the singularity container
```bash
cp env_bogemms_hpc.sh.template env_bogemms_hpc.sh
```
## Local installation

### Loading the environment
```bash
source env_bogemms_hpc.sh
```
### project setup (builds BoGEMMS with user classes)
```bash
source setup.sh
```
The setup script also defines:

- `COSI_ACS_DEE_DIR` – main repository directory
- `BOGEMMS_USER_DIR` – user BoGEMMS classes (`bogemms_hpc_classes/`)

## Using the container as toolchain for the BoGEMMS-HPC installation
```bash
singularity shell g4_11_1_cosi.sif
source env_bogemms_hpc.sh
source setup.sh
```
Then exit container before running the simulations.
