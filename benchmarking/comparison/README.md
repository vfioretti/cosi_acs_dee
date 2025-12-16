# Comparison between simulation and experiment

## Description

- **DATA/**: directory containing the experimental measurements
- **config.json:** configuration file 
- **calibration_lab.py:** fit the experimental photopeak for each source, perform the energy calibration, convert the channel spectra into energy spectra, fit again the experimental energy photopeak to compute the energy resolution, fit the energy resolution against energy
- **calibration_sim.py**: fit the simulated photopeak for each source, perform the energy calibration, convert the spectra into energy spectra, fit again the simulated energy photopeak to compute the energy resolution, fit the energy resolution against energy
- **compare_spectra.py**: compare energy resolution and energy spectra between experiment and simulation
- **compare_relative_response.py**: compare relative response between experiment and simulation
- **models.py** and **functions.py**: python scripts containing useful models and functions

The config.json file contains the information about the measurements and how to perform the energy calibration and has the following structure:
- **'uncollimated'**: list of sets of uncollimated measurements; for each set you need to define:
    - **'path lab'**: path of the laboratory spectra
    - **'name'**: name of the calibration campaign
    - **'sources'**: list of sources; for each source you need to define:
        - **'source_name'**: name of the source
        - **'file_lab'**: file name of the corresponding spectrum
        - **'file_bkg'**: list of file names for the background spectra (which can be more than one)
        - **'file_sim'**: path of the corresponding simulation file ('SiPM.dat')
        - **'bin_width'**: bin width used for binning the simulated spectra (in terms of number of detected photons)
        - **'lines'**: list of energy lines; for each line you need to define:
            - **'energy'**: energy in keV
            - **'channel_range'**: channel range used to select the photopeak region
            - **'model'**: model used to fit the photopeals (the models are listed in models.py)
            - **'initial_params_channel'**: priors (mean and std) used for the fit of the photopeaks in channels
            - **'initial_params_channel'**: priors (mean and std) used for the fit of the photopeaks in energy when calculating the energy resolution
- **'collimated'**: list of sets of collimated measurements; for each set you need to define:
    - **'path lab'**: path of the laboratory spectra
    - **'name'**: name of the calibration campaign
    - **'positions'**: list of position labels
    - **'sources'**: list of sources; for each soure you need to define:
        - **'source_name'**: name of the source
        - **'file_lab'**: file name of the corresponding spectrum
        - **'file_bkg'**: list of file names for the background spectra (which can be more than one)
        - **'file_sim'**: path of the corresponding simulation file ('SiPM.dat')
        - **'lines'**: list of energy lines; for each line you need to define:
            - **'energy'**: energy in keV
            - **'channel_range'**: channel range used to select the photopeak region
            - **'model'**: model used to fit the photopeals (the models are listed in models.py)
            - **'initial_params_channel'**: priors (mean and std) used for the fit of the photopeaks in channels
            - **'bin_width'**: bin width used for binning the simulated photopeak (in terms of number of detected photons)

## Usage

Before starting the analysis, you need to make sure that the experimental spectra are in the following format:

```
# TIME: XXX s
# Channel Counts
X X
X X
...
```

where XXX is the time in seconds used to take the data for the corresponding source. This quantity is used to correctly scale the background when subtracting it from the source.

### Relative response
To compare the relative response between experiment and simulation, run:

```
python3 compare_relative_response.py config.json
```

### Spectral comparison
First, use calibration_lab.py to fit the experimental photopeaks, perform the energy calibration to convert channel spectra into energy spectra and fit the energy resolution:

```
python3 calibration_lab.py config.json
```

Then, do the same thing for simulation:

```
python3 calibration_sim.py config.json
```

Finally, to compare the energy resolution and energy spectra between experiment and simulation, run:

```
python3 compare_spectra.py config.json
```


