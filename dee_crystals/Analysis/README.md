# Analysis

Analysis and plot for the COSI ACS correction matrix

## Description

- `filter.py`: filter the simulation output and store the relevant information in 'SiPM.dat'
- `reduce.py`: collect all the files 'SiPM.dat' from the simulation runs to a unique file 'SiPM_final_{BGO_name}.dat'
- `compute_response.py`: perform the spatial analysis and compute the correction matrix
- `centroids_fwhm_map.py`: compute the centroids and FWHM map and fit them as a function of the energy
- `config_{BGO_name}.json`: parameter file for the individual crystal
- `unify_files.py`: unifies all correction files from each crystal into a single file
- `SiPM_PDE`: file with the Photon Detection Efficiency (PDE) of the SiPMs

You need to configure a 'json' file for each crystal. These are the parameters inside the json file:

- `BGO_name`: name of the crystal
- `fwhm_noise`: fwhm of the electronic noise in keV
- `m_cal`, `q_cal`: fit parameters of the photon-energy relation (assuming linear relation: E = m * N_photons + q) 
- `em_cal`, `eq_cal`: error on fit parameters of the photon-energy relation
- `x_len_mm`, `y_len_mm`, `z_len_mm`: x, y and z length of the BGO crystal in mm
- `NbinX`, `NbinY`, `NbinZ`: number of spatial bins in the x, y and z directions
- `x_to_megalib`: conversion from bogemms 'x' to megalib 'x' coordinate
- `y_to_megalib`: conversion from bogemms 'y' to megalib 'y' coordinate
- `z_to_megalib`: conversion from bogemms 'z' to megalib 'z' coordinate
- `E_min_keV`, `E_max_keV`: minimum and maximum energy to analyze (in keV)
- `num_bins`: number of energetic bins

## Usage

Use filterACS.py to filter the output files from simulation (.fits.gz files) and write the relevant information to SiPM.dat file.
In a directory (different from the directory of the simulations), launch:

```
python3 filter.py SIM_DIR run_start run_stop
```

Example:

```
python3 filter.py ../Simulations/SIM0/194.0x118.0x23.0/index_0.0/10.0-1000.0/1000/ 1 1
```

This will create a directory like 

```
SIM{sim_id}/SIM{sim_id}/{xlen}x{ylen}x{zlen}/index_{index}/{E_min}-{E_max}/{N_in}/run_{run_id}/
```

where 'SiPM.dat' is created.

Then, you need to collect all the simulation runs and perform a spatial binning of the energy deposits in the BGO. To do so, run:

```
python3 reduce.py config.json SIM0
```

The script reads all the 'SiPM.dat' files inside 'SIM0' (you can also give as input more than one simulation directory), and gives as output 'SiPM_final_{BGO_name}.dat' containing the following columns:

- `EventID`: event ID
- `E_ent`: energy (in keV) of the incoming photon
- `x`, `y`, `z`: central x, y, z coordinates of the spatial bin where the energy deposit occurred
- `E_dep`: true energy deposit (in keV) in the corresponding spatial bin
- `N_abs`: number of detected optical photons
- `N_abs_err`: error on the number of detected optical photons (sqrt(N_abs))

To compute the response, run:

```
python3 compute_response.py SiPM_final_{BGO_name}.dat config_{BGO_name}.json
```

This script fits, for each energy and spatial bin, the distribution of measured energies (obtained after converting N_abs into energy) with a Gaussian model and computes the corresponding centroid and full width at half maximum (FWHM) (both in keV). A file 'response_{BGO_name}.dat' will be created, with the following columns:

- `posID`: ID of the spatial bin
- `x`: central x coordinate of the spatial bin (in mm)
- `y`: central y coordinate of the spatial bin (in mm)
- `E_true`: true energy deposit, i.e. the central energy of the energy bin (in keV)
- `E_meas`: measured energy deposit, i.e. the centroid obtained from the fit (in keV)
- `FWHM`: measured FWHM, i.e. the FWHM obtained from the fit (in keV)

Also, it performs the fit of the centroids and FWHM against energy. The fit are performed using the following models for the centroids and energy resolution: `E_meas = m * E_true + q` and `FWHM = sqrt(a^2 + b^2 E + c^2 E^2)`. The results of the fit are saved in 'correction_file_{BGO_name}.dat' file.

Then, by running:

```
python3 centroids_fwhm_map.py response.dat config.json
```

you plot the centroid and FWHM maps for each energetic bin. All the plots will be saved in *plots_{BGO_name}/*.

To produce a unique correction file for all the crystal, just run

```
python3 unify_files.py
```

A file, called `ACS_correction_file.dat`, will be produced, having the following columns:

- `DetectorName`: the name of the crystal
- `voxel_X`: x-voxel ID matching the MEGAlib voxelization
- `voxel_Y`: y-voxel ID matching the MEGAlib voxelization
- `x[mm]`: central x coordinate of the spatial bin (in mm)
- `y[mm]`: central y coordinate of the spatial bin (in mm)
- `m`: *m* parameter from the fit of the centroids
- `q`: *q* parameter from the fit of the centroids
- `a`: *a* parameter from the fit of the energy resolution
- `b`: *b* parameter from the fit of the energy resolution
- `c`: *c* parameter from the fit of the energy resolution

**Note**: the name of the crystals must match those in MEGAlib, in order to use this file for the ACS DEE.


