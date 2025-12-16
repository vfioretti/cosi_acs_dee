# Simulating a BGO crystal

The simulation consists of a BGO crystal irradiated by a beam of photons, following a power-law energy spectrum. When a photon interacts with the crystal, scintillation photons are emitted as a result of the energy deposit in the BGO. These optical photons are kept inside the crystal thanks to the reflective layers coating the BGO. Finally, some of these optical photons are absorbed by the SiPMs, which are placed at the corner of each crystal. 

## Input file
The input file **runSiPM.txt** collects the input parameters for the simulation. When you launch the simulation, some of these parameters are used to modify the configuration file *SiPM.conf* and update it accordingly.

Update **runSiPM.txt** in the following way:
- **run_start** and **run_stop**: number of initial and final runs (total number of runs = run_stop - run_start + 1)
- **G4PATH**: path where to create the simulation directory
- **sim_id**: ID of the simulation
- **N_in**: number of events to simulate
- **E_min** and **E_max**: energy range to simulate [keV]
- **index**: spectral index
- **xlen**, **ylen**, **zlen**: dimensions of the BGO crystal [mm]
- **isslurm**: define how to run the simulation (0: nohup, 1: SLURM)
- **job_name**: define the job name
- **num_threads** and **num_tasks**: number of threads and MPI tasks to use

## Simulation

To  start the simulation, run:
```
python3 runSiPM.py runSiPM.txt
```

At the end of the simulation, a directory like

'SIM{sim_id}/{xlen}x{ylen}x{zlen}/index_{index}/{E_min}-{E_max}/{N_in}/'

will be created, where each {} depends on the parameters set in 'runSiPM.txt'. Inside the last directory ('not_collimated/'), you find directories like 'run{run_ID}', depending on how many runs you simulated. In this last directories you will find the output files (.fits.gz) containing the results containing the results of the simulation, and the *conf*, *mac* and *runSiPM.txt* used for this simulation.
