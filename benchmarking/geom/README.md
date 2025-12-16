# Experimental setups and simulated mass model

## NRL

This is the experimental setup used for the tutorial.

The measurements involve the Engineering Model (EM) X-wall, i.e. a lateral ACS panel consisting of 3 BGO crystals and read-out each by SiPM arrays. Each BGO crystal consists of a 198 x 118 x 23 mm scintillator block, and a 3 x 3 array of SiPMs (onsemi J-series 60035) is attached to the 118 x 23 mm face with the use of a silicone optical coupler. The BGO is wrapped with two layers of VM2000, one layer of Tetratex and one layer of Teflon PTFE. The face where the SiPMs are attached is instead covered only with two layers of Tetratex. These reflective layers prevent scintillation light from escaping the BGO and maximize the SiPM response. 

The X-wall is placed on a metal table and covered with a black cloth to prevent light leakage. The uncollimated spectra are taken with Am241 and Cs137 placed on top of a foam block that sits on an aluminum frame, with the sources at 32 cm above the central BGO crystal. The Na22 source is instead positioned directly on top of the aluminum housing and centered with the crystal. The collimated measurements are performed using a lead cylinder with a 16 mm diameter hole and a 3D printed aligner in order to place the collimator at 12 different positions to scan the BGO surface.

<img src="images/NRL_Am241_Cs137.png" alt="Logo" width="50%"/>

<img src="images/NRL_Na22.png" alt="Logo" width="50%"/>

<img src="images/NRL_coll.png" alt="Logo" width="50%"/>

### CAD files

For the NRL seutp, we use CAD files for the following geometries:
- **aluminum housing** (*ACS-ST-0101revB_ACS_X-WALL_CHASSIS.stl*)
- **3D printed aligner** (*CollimatorSheet.stl*)

## SSL

At SSL, the measurements involved the same EM X-wall, but using a larger number of radioactive sources.

The X-wall is placed on a wooden table and anti-static mat and covered with a black cloth. A stand is used to hold the sources at 60 cm from the central BGO crystal, without collimation. A lead block was also present, positioned close to the X-wall during the measurements.

<img src="images/SSL.png" alt="Logo" width="50%"/>

### CAD files

For the SSL seutp, we use CAD files for the following geometries:
- **aluminum housing** (*ACS-ST-0101revB_ACS_X-WALL_CHASSIS.stl*)

