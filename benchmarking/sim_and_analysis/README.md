# Simulation and analysis

In this part there are two directories ('SSL' and 'NRL') for the two calibration campaigns. In each of them, there are two directories, 'Simulations' for the Geant4 simulation, 'Analysis' for the analysis of the simulation output.

The directories 'geom/', 'phys/' and 'cad_files' contain the geometry files, the physics list and the CAD modules which are read by BoGEMMS-HPC, respectively.

## Simulation

### Description

- **{source_name}.mac**: macro file for each uncollimated source
- **Coll_{source_name}.mac**: macro file for each collimated source
- **SiPM.conf**: configuration file for the simulation
- **runSiPM.py**: script to start the simulation with uncollimated sources
- **runSiPM_coll.py**: script to start the simulation with collimated sources
- **runSiPM.txt**: input file for runSiPM.py
- **thelsim_container.slurm**: slurm file

### Usage

Update 'runSiPM.txt' to define the parameters of the simulation. Make sure that the 'SiPM.conf' file contains the right geometry name, physics name and the right path to the directory with the CAD modules.

Then, to start the simulation, for the uncollimated sources run:

```
python3 runSiPM.py runSiPM.txt
```

For the collimated sources run:

```
python3 runSiPM_coll.py runSiPM.txt
```

At the end of the simulation, a directory like 

SIM{sim_id}/GEOM_TYPE{geom_type}/BGO_ABSL_TYPE{bgo_absl_type}/REFL_TYPE{refl_type}/REFL2_TYPE{refl2_type}/{N_in}/{source}/not_collimated/run{run_id}/

will be created, where each {} depends on the parameters set in 'runSiPM.txt'.


## Analysis

### Description

- filterACS.py: python script to filter the simulation output
- SiPM_PDE.txt: photon detection efficiency (PDE) of SiPMs

### Usage

Use filterACS.py to filter the output files from simulation (.fits.gz files) and write the relevant information to a .dat file.
In a directory (different from the directory of the simulations), launch:

```
python3 filterACS.py SIM_DIR run_start run_stop
```

Example:

```
python3 filterACS.py ../Simulations/SIM0/GEOM_TYPE9/BGO_ABSL_TYPE6/REFL_TYPE1/REFL2_TYPE4/1000000/Cs137/not_collimated/ 1 1
```

This will create a directory like 

SIM{sim_id}/GEOM_TYPE{geom_type}/BGO_ABSL_TYPE{bgo_absl_type}/REFL_TYPE{refl_type}/REFL2_TYPE{refl2_type}/{N_in}/{source}/not_collimated/{run_start}-{run_stop}/

In this directory a file called 'SiPM.dat' is created with the following columns:

- EventID: ID of the event
- Edep_scin: energy deposit (in keV) in the BGO
- Nabs_SiPM_PDE: number of detected optical photons after applying the SiPM PDE
- Nabs_PDE: number of absorbed optical photons without applying the SiPM PDE
- N_opt: number of generated optical photons in the BGO
- Edep_SiPM: total energy of the optical photons detected by the SiPMs