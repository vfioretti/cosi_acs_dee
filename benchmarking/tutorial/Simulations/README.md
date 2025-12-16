# Simulating radioactive sources

The simulation consists of a radioactive source placed at a certain distance from the COSI EM X-wall, i.e. three BGO crystals placed inside an aluminum housing. When an emitted photon from the source enter the crystal, scintillation photons are emitted as a result of the energy deposit in the BGO. These optical photons are kept inside the crystal thanks to the reflective layers coating the BGO. Finally, some of these optical photons are absorbed by the SiPMs, which are placed at the corner of each crystal. 

In this simulation we simulate the Cesium-137 (Cs137), which emits photons at 662 keV. We simulate the source both with and without collimation. The collimation is achieved with a collimator placed at 12 different positions above the central BGO crystal.

These are the steps you need to follow:
- prepare the geometry, CAD files and physics list
- prepare the configuration (**.conf**) file 
- prepare the macro (**.mac**) file
- set up the simulation parameters in the input file
- launch the simulation

## Geometry, CAD files and physics list
In **'geom'** directory you can find the source and header files (GeometryCOSI_EMXwall.cc and GeometryCOSI_EMXwall.hh) defining the geometry for the simulation (inside this directory you can find also a detailed description of the mass model used in this example).

In **'phys'** directory you can find the source and header files (OPTPhys.cc and OPTPhys.hh) defining the physics list used for the simulation. It includes several physics list, the most important ones for this example being the decay physics for the radioactive decays, electromagnetic physics (Livermore polarized) for the interaction of the high energy photons within the crystal and the optical physics for the generation of scintillation photons.

In **'cad_files'** directory you can find the CAD files that are needed to construct the geometry (the aluminum housing and 3D printer aligner for the collimated sources).

**The steps you need to do are:**
- Move the files in this 'geom' directory in the geometry directory of BoGEMMS-HPC
- Move the files in this 'phys' directory in the physics directory of BoGEMMS-HPC

## Configuration file
The configuration file is called **SiPM.conf** and contains the parameters that BoGEMMS-HPC needs to run the simulation. The parameters which are of interest for this simulation are:
- **ENERGYPROCESS.VERSION**: it is the name of the physics list and must match with the physics list class (OPTPhys)
- **PHYS.COSI.OPT.ACTIVATE**: it is a keyword to activat the optical physics and must be set to 1 (if 0 the optical physics is deactivated)
- **PHYS.COSI.BGO.ABSL.TYPE**: it determines the absorption length  of the BGO (1: 10 cm, 2: 30.3 cm, 3: 200 cm, 4: 300 cm, 5: 1000 cm, 6: 500 cm); in this simulation we set it to 500 cm (i.e. case 6)
- **GEOM.VERSION**: it is the name of the geometry class (GeometryCOSI_EMXwall)
- **GEOM.COSI.COLLIMATED**: keyword specifying if the sources are collimated (1) or not (0) (i.e. if the collimator is present or not in the geometry)
- **GEOM.COSI.BEAMX**: if the source is collimated, it specifies its x coordinate with respect to the BGO center (i.e. the position of the collimator)
- **GEOM.COSI.BEAMY**: if the source is collimated, it specifies its y coordinate with respect to the BGO center (i.e. the position of the collimator)
- **GEOM.COSI.AM241.OR.CS137**: if 1, it uses the geometry setup for either Am241 or Cs137, while if 0 it uses the setup for Na22
- **GEOM.CAD.PATH**: the path to the directory containing the cad files
- **SENSITIVE.[NAME]**: if 1, it means that the corresponding object in the geometry is sensitive

**The steps you need to do are:**
- make sure that **ENERGYPROCESS.VERSION** is set to *OPTPhys*,  **PHYS.COSI.OPT.ACTIVATE** to 1 and **GEOM.VERSION** to *GeometryCOSI_EMXwall*
- set **GEOM.CAD.PATH** to the path to your directory containing the cad files
- leave all the rest parameters unchanged

## Macro files
The macro files are called **[source_name].mac** (for uncollimated sources) and **Coll_[source_name].mac** (for collimated sources) and each of them contains the information about the corresponding radioactive source you want to simulate. 

Let's take *Cs137.mac* as example:

```
/gps/particle ion
/gps/ion 55 137
```
These two lines create a radioactive element with Z = 55 and A = 137 (Cs137).
```
/gps/pos/type Volume
/gps/pos/shape Cylinder
/gps/pos/centre 0. 0. 32.1 cm
/gps/pos/radius 2.5 mm
/gps/pos/halfz 0.5 mm
```
With this lines, you are telling BoGEMMS-HPC to place the radioactive material in a cylindrical volume, with center at {0, 0, 32.1}, radius 2.5 mm and half height 0.5 mm.
```
/gps/pos/rot1 0 1 0
/gps/pos/rot2 1 0 0
```
These lines are used to make sure that the source is oriented in the right way.
```
/gps/ang/type iso
```
This line tells the simulator to simulate the emission of photons isotropically
```
/gps/ene/type Mono
/gps/energy 0. keV
```
With this lines you are telling the simulator that the radioactive sources are at rest (they just decay).
```
/run/beamOn 100
```
With this line you specify how many radioactive decays you want to simulate.

You can leave these files unchanged.

## Input file
The input file is called **runSiPM.txt** and collects the input parameters for the simulation. When you launch the simulation, some of these parameters are used to modify the configuration file *SiPM.conf* and update it accordingly.

These are the list of parameters you can specify in the input file:
- **run_start** and **run_stop**: number of initial and final runs (total number of runs = run_stop - run_start + 1)
- **G4PATH**: path where to create the simulation directory
- **sim_id**: ID of the simulation
- **geom_type**: type of geometry (8: NRL, 9: SSL)
- **bgo_absl_type**: BGO absorption length case (1: 10cm, 2: 30.3cm, 3: 200cm, 4: 300cm, 5: 1000cm, 6: 500cm)
- **refl_type** and **reflw_type**: optical surface type for the BGO-VM2000 interface (non-SiPM faces) and BGO-Tetratex interface (SiPM face), respectively
- **N_in**: number of events to simulate
- **source**: define the source (Am241, Na22 or Cs137)
- **isslurm**: define how to run the simulation (0: nohup, 1: SLURM)
- **bias**: with this parameter you set a systematic shift in the collimated positions (left, right, forward or back); if it is empty, there is no systematic shift; if the sources are uncollimated, this parameter is ignored
- **job_name**: define the job name
- **num_threads** and **num_tasks**: number of threads and MPI tasks to use

See in the next sections how to properly set this parameters.

## Simulation

The python script to launch the simulation is **runSiPM.py** for uncollimated sources, **runSiPM_coll.py** for collimated sources. They both take as input the parameter file *runSiPM.txt*. 

Before starting the simulation, if you use the SLURM scheduler, you need to update the last line of **thelsim_container.slurm** with your path to the BoGEMMS-HPC container and the path to the environment setup script.

### Uncollimated sources

Update **runSiPM.txt** in the following way:
- **run_start** and **run_stop**: you can always leave both of them to 1 (i.e. only 1 run will be simulated).
- **G4PATH**: put the path where you want to store the simulation directory (can also be the directory where you currently are)
- **sim_id**: you can set it to 0
- **geom_type**: set it to 8
- **bgo_absl_type**: set it to 6 (BGO absorption length of 500 cm)
- **refl_type** and **reflw_type**: set it to 1 and 4, respectively
- **N_in**: you can set it to 100000
- **source**: Cs137
- **isslurm**: 1
- **bias**: leave empty
- **job_name**: set it as you want
- **num_threads** and **num_tasks**: suggested for OAS cluster: num_threads = 1, num_tasks = 20

To finally start the simulation, for the uncollimated sources just run:
```
python3 runSiPM.py runSiPM.txt
```
For the collimated sources, run:
```
python3 runSiPM_coll.py runSiPM.txt
```
At the end of the simulation, a directory like

'SIM{sim_id}/GEOM_TYPE{geom_type}/BGO_ABSL_TYPE{bgo_absl_type}/REFL_TYPE{refl_type}/REFL2_TYPE{refl2_type}/{N_in}/{source}/not_collimated/'

will be created, where each {} depends on the parameters set in 'runSiPM.txt'. Inside the last directory ('not_collimated/'), you find directories like 'run{run_ID}', depending on how many runs you simulated. In this last directories you will find the output files (.fits.gz) containing the results containing the results of the simulation, and the *conf*, *mac* and *runSiPM.txt* used for this simulation.

### Collimated sources

For the collimated sources, we need to run the simulation five times: one with the sources placed in their corresponding positions, and other four times with the sources shifted in four orthogonal directions by a certain amount *d* to estimate the systematic error due to uncertainty in the position of the collimator. The coordinates of the positions and the displacement *d* (both in cm) are defined in *runSiPM_coll.py* (in this example *d* = 1 mm)

You can leave the same parameters in **runSiPM.txt** as the uncollimated case, except for:
- **bias**: for each of the 5 cases you need to change this parameter:
-- *leave empty*: the collimated sources are placed in their exact positions
-- *'right'*: the collimated sources are shifted towards the "right" (w.r.t. the SiPMs)
-- *'left'*: the collimated sources are shifted towards the "left" (w.r.t. the SiPMs)
-- *'forward'*: the collimated sources are shifted "forward" (i.e. towards the SiPM)
-- *'back'*: the collimated sources are shifted "backward" (i.e. they move away from the SiPM)

For the collimated sources, run:
```
python3 runSiPM_coll.py runSiPM.txt
```
At the end of the simulation, directories like

'SIM{sim_id}/GEOM_TYPE{geom_type}/BGO_ABSL_TYPE{bgo_absl_type}/REFL_TYPE{refl_type}/REFL2_TYPE{refl2_type}/{N_in}/{source}/collimated{bias}'

will be created, where each {} depends on the parameters set in 'runSiPM.txt'. The last directory can be either *'collimated/'* (with no shift), or *'collimated_right/'*, *'collimated_left/'*, *'collimated_forward/'*, *'collimated_back/'* (for the corresponding shifts). Inside these directories you will find directories like *'pos1/'*, *'pos2/'* etc., depending on how many positions you have (in this example 12). Finally, inside the position directories you can find directories like *'run{run_ID}'*, depending on how many runs you simulated. In this last directories you will find the output files (.fits.gz) containing the results containing the results of the simulation, and the *conf*, *mac* and *runSiPM.txt* used for this simulation.
