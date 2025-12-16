# Energy calibration and comparison with laboratory data

In this section, we see how to compare simulated data with laboratory data. 

For uncollimated sources, we compare simulated spectra of radioactive sources with corresponding laboratory data and the energy resolution.
There are three steps:
- Energy calibration for laboratory data to get experimental energy spectra and resolution
- Energy calibration for simulated data to get simulated energy spectra resolution
- Comparison between simulated and experimental energy spectra

For collimated sources, we evaluate the relative response for each position and compare it to the experimental results. In this case, we do not need to perform the energy calibration.

# Description

In this directory you can find:

- **DATA/**: directory containing the experimental measurements for uncollimated Am241, Na22, Cs137 and collimated Am241, Cs137.
- **SIM0/**: directory containing the simulation files for uncollimated Am241, Na22, and collimated Am241.
- **config.json:** configuration file 
- **calibration_lab.py:** python script to fit the experimental photopeak for each source, perform the energy calibration, convert the channel spectra into energy spectra, fit again the experimental energy photopeak to compute the energy resolution, fit the energy resolution against energy
- **calibration_sim.py**: python script to fit the simulated photopeak for each source, perform the energy calibration, convert the spectra into energy spectra, fit again the simulated energy photopeak to compute the energy resolution, fit the energy resolution against energy
- **compare_spectra.py**: compare energy resolution and energy spectra between experiment and simulation
- **compare_relative_response.py**: compare relative response between experiment and simulation
- **plot_lab_uncollimated** and **plot_lab_collimated**: plot lab spectra
- **models.py** and **functions.py**: python scripts containing useful models and functions

# Preparation of laboratory data
Before starting the analysis, you need to make sure that the experimental spectra are in the following format:

```
# TIME: XXX s
# Channel Counts
X X
X X
...
```

where XXX is the time in seconds used to take the data for the corresponding source. This quantity is used to correctly scale the background when subtracting it from the source. In 'DATA/' you can find the laboratory data already in the correct format.

You can plot them using

```
python3 plot_lab_uncollimated.py DATA/NRL/uncollimated/
python3 plot_lab_collimated.py DATA/NRL/collimated/
```


# Uncollimated sources
We use the uncollimated sources to perform an energy calibration, evaluate the energy resolution and compare the simulated spectra with the experimental one.

In 'SIM0' you can find the simulation results (i.e. SiPM.dat files) for Am241 and Na22. You have to add your Cs137 simulation file (for the energy calibration we need at least 3 sources). 

In **config.json** there are two sections, 'uncollimated' and 'collimated'. These are the parameters you need to set in the 'uncollimated' section:
- **'uncollimated'**: list of sets of uncollimated measurements (in this case only one set); for each set you need to define:
    - **'path lab'**: path of the laboratory spectra (e.g. "/path/to/DATA/NRL/uncollimated")
    - **'name'**: name of the calibration campaign (e.g. "NRL")
    - **'sources'**: list of sources; for each source you need to define:
        - **'source_name'**: name of the source (e.g. "Am241", "Na22" or "Cs137")
        - **'file_lab'**: file name of the corresponding spectrum (e.g. "data_Cs137.dat" for Cs137)
        - **'file_bkg'**: list of file names for the background spectra (e.g. ["data_bkg.dat"], it is a list because in principle you could have more than one background spectrum for each source)
        - **'file_sim'**: path of the corresponding simulation file (e.g. "/path/to/SiPM.dat")
        - **'bin_width'**: bin width used for binning the simulated spectra in terms of number of detected photons (for Cs137 you can use 2)
        - **'lines'**: list of energy lines; for each line you need to define:
            - **'energy'**: energy in keV (for Cs137 it is 662)
            - **'channel_range'**: channel range used to select the photopeak region (for Cs137 "[450, 700]")
            - **'model'**: model used to fit the photopeals (the models are listed in models.py)
            - **'initial_params_channel'**: priors (mean and std) for each parameter used for the fit of the photopeaks in channels
            - **'initial_params_channel'**: priors (mean and std) for each parameter used for the fit of the photopeaks in energy when calculating the energy resolution


## Energy calibration for experimental data
Use calibration_lab.py to fit the experimental photopeaks, perform the energy calibration to convert channel spectra into energy spectra and fit the energy resolution:

```
python3 calibration_lab.py config.json
```

You may need to adjust the parameters about the priors to make a successful fit. This script fits the experimental photopeak for each source, performs the energy calibration, converts the channel spectra into energy spectra, fits again the experimental energy photopeak to compute the energy resolution, and finally fits the energy resolution against energy.

The energy calibration is performed with a linear model, while the energy resolution is fitted with the following model (*Bissaldi et al. 2019*):

<img src="images/energy_resolution_model.png" alt="Logo" width="20%"/>

The binned energy spectra are written in a directory called "Spectrum_lab_{name}", where *'name'* is the name of the calibration campaign you put in the configuration file (in our case 'NRL'). Also, a file called "exp_resolution.dat" is created, containing the fit parameters and the energy resolution for each energy line.

## Energy calibration for simulated data

You need to do the same for the simulation:

```
python3 calibration_sim.py config.json
```

This scripts reads the electronic noise from "exp_resolution.dat" obtained from the energy calibration of experimental data and applies it to the simulated spectra. If you don't have experimental data, you can set manually the value of the electronic noise by adding an input parameter after *config.json*. For example, if you want an electronic noise with FWHM = 21 keV, you can do it in the following way:

```
python3 calibration_sim.py config.json 21
```

The binned energy spectra are written in a directory called "Spectrum_sim_{name}", where *'name'* is the name of the calibration campaign you put in the configuration file (in our case 'NRL'). Also, a file called "sim_resolution.dat" is created, containing the fit parameters and the energy resolution for each energy line.

## Comparison
Finally, to compare the energy resolution and energy spectra between experiment and simulation, run:

```
python3 compare_spectra.py config.json
```

This script plots the comparison between experimental and simulated energy resolution (with and without electronic noise for the simulated data), and the spectral comparison for each radioactive source.

The spectrum data are also saved in *binned_spectrum/sim/* and *binned_spectrum/lab/* for the simulation and experiment, rispectively. The plots of the comparison between simulation and experiment are saved in *plots/* with the name *[source_name]_comp_[calibration_campaign].pdf* (example: *Cs137_comp_NRL.pdf*).


# Collimated sources
The collimated sources are used to compare the relative response for each position.

These are the parameters in the 'collimated' section in **config.json**:
- **'collimated'**: list of sets of collimated measurements (in this case only one); for each set you need to define:
    - **'path lab'**: path of the laboratory spectra (e.g. "/path/to/DATA/collimated")
    - **'name'**: name of the calibration campaign ("NRL)
    - **'positions'**: list of position IDs (e.g. [1,10,2,11,3,12,4,5,6,7,8,9], the order is the order you want them to appear in the plot)
    - **'sources'**: list of sources; for each soure you need to define:
        - **'source_name'**: name of the source (e.g. "Am241" or "Cs137")
        - **'file_lab'**: file name of the corresponding spectrum (e.g. "data_Am241_pos*.dat", the '*' will be replaced with the number you put in the 'positions' parameter)
        - **'file_bkg'**: list of file names for the background spectra (e.g. ["data_bkg.dat"], it is a list because in principle you could have more than one background spectrum for each source)
        - **'file_sim'**: path of the corresponding simulation file (e.g. "/path/to/SiPM.dat")
        - **'lines'**: list of energy lines; for each line you need to define:
            - **'energy'**: energy in keV (e.g. 662 for Cs137)
            - **'channel_range'**: channel range used to select the photopeak region (e.g. [450, 700] for Cs137)
            - **'model'**: model used to fit the photopeals (the models are listed in models.py)
            - **'initial_params_channel'**: priors (mean and std) used for the fit of the photopeaks in channels
            - **'bin_width'**: bin width used for binning the simulated photopeak (in terms of number of detected photons)

To compare the relative response between experiment and simulation, run:

```
python3 compare_relative_response.py config.json
```

This script plots the comparison between the simulated and experimental relative response for each position.
