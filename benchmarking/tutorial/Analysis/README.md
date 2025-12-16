# Analysis of simulation output

Once we have simulated a radioactive source, we need to analyze and filter the information contained in the output FITS files. 

You need two files:
- *filterACS.py*: python script to filter the simulation output with uncollimated sources
- *filterACS_pos.py*: python script to filter the simulation output with collimated sources
- *SiPM_PDE.txt*: text file containing the photon detection efficiency (PDE) of the SiPM

Place these two files in a different directory than the one dedicated to the simulations. 

## Uncollimated sources
Use filterACS.py to filter the output files from the simulation with uncollimated sources and write the relevant information to a .dat file by running

```
python3 filterACS.py SIM_DIR run_start run_stop
```

where *SIM_DIR* is the directory containing the simulation runs and *run_start*, *run_stop* define the range of runs you want to analyze (*run_stop* included). In this example you need to run

```
python3 filterACS.py ../Simulations/SIM0/GEOM_TYPE8/BGO_ABSL_TYPE6/REFL_TYPE1/REFL2_TYPE4/200000/Cs137/not_collimated/ 1 1
```

This will create a directory like

*SIM{sim_id}/GEOM_TYPE{geom_type}/BGO_ABSL_TYPE{bgo_absl_type}/REFL_TYPE{refl_type}/REFL2_TYPE{refl2_type}/{N_in}/{source}/not_collimated/{run_start}-{run_stop}/*

In this directory a file called **'SiPM.dat'** is created with the following columns:
- *EventID*: ID of the event
- *Edep_scin*: energy deposit (in keV) in the BGO
- *Nabs_SiPM_PDE*: number of detected optical photons after applying the SiPM PDE
- *Nabs_PDE*: number of absorbed optical photons without applying the SiPM PDE
- *N_opt*: number of generated optical photons in the BGO
- *Edep_SiPM*: total energy of the optical photons detected by the SiPMs

## Collimated sources
For collimated sources, you need to run

```
python3 filterACS_pos.py SIM_DIR run_start run_stop n_pos
```

where *n_pos* is the number of collimated positions. For this example you need to run

```
python3 filterACS_pos.py ../Simulations/SIM0/GEOM_TYPE8/BGO_ABSL_TYPE6/REFL_TYPE1/REFL2_TYPE4/100000/Cs137/collimated/ 1 1 12
```

and the same applies also for simulations with shifts (i.e. substituting *collimated/* with *collimated_right*, *collimated_left* etc).

In this directory a file called **'SiPM.dat'** is created with the following columns:
- *EventID*: ID of the event
- *Pos*: position ID
- *Edep_scin*: energy deposit (in keV) in the BGO
- *Nabs_SiPM_PDE*: number of detected optical photons after applying the SiPM PDE
- *Nabs_PDE*: number of absorbed optical photons without applying the SiPM PDE
- *N_opt*: number of generated optical photons in the BGO
- *Edep_SiPM*: total energy of the optical photons detected by the SiPMs