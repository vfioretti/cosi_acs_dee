"""
 filterACS.py  -  description
 ---------------------------------------------------------------------------------
 filtering the events of ACS simulation
 ---------------------------------------------------------------------------------
 copyright            : (C) 2023 Alex Ciabattoni
 email                : alex.ciabattoni@inaf.it
 ----------------------------------------------
 Usage:
 python filterACS.py PATH_SIM run_start run_stop N_beams
 example:
 python filterACS.py ../Simulations/SIM_DIR/10000/122keV/ 1 1 12
 ---------------------------------------------------------------------------------
 Parameters:
 - PATH_SIM  = path where the BoGEMMS simulation runs are stored
 - run_start = initial run number
 - run_start = initial run number
 - N_beams   = number of beams
 --------------------------------------------------------------------------------
 Caveats:
 None
 ---------------------------------------------------------------------------------
 Modification history:
 - 2023/04/18: creation date
"""

from astropy.io import fits
from astropy import units as u
from astropy.constants import h, c
import numpy as np
import math
import sys, os
import subprocess
import matplotlib.pyplot as plt
from scipy.optimize import curve_fit
from scipy.interpolate import interp1d
import shutil

from matplotlib import gridspec
import matplotlib as mpl
from mpl_toolkits.axes_grid1 import make_axes_locatable

from astropy.table import Table, Column

import glob

# Help
if len(sys.argv) != 4:
	print("n")
	print(" filterACS.py  -  description\n")
	print(" ----------------------------------------------------------\n")
	print(" Usage:\n\tpython filterACS.py PATH_SIM run_start run_stop\n example:\n\tpython filterACS.py ../Simulations/SIM_DIR/10000/122keV/ 1 1\n")
	print(" ----------------------------------------------------------\n")
	print(" Parameters:\n\t- PATH_SIM = path where the BoGEMMS simulation runs are stored\n")
	print(" \t- run_start = initial run number\n")
	print(" \t- run_start = final run number\n")
	print("\n")
	exit()

# Import line command parameters
arg_list = sys.argv
PATH_SIM = arg_list[1]
run_start = int(arg_list[2])
run_stop = int(arg_list[3])

# Read input parameters from config file
input_params = []
file_input = PATH_SIM + "/run" + str(run_start) + "/runSiPM.txt"
with open(file_input, "r") as f_in:
	for line in f_in:
		line = line.strip()
		if not line.startswith("#") and line != "":
			columns = line.split("=")
			input_params.append(columns[-1].strip())

G4PATH = input_params[2]
sim_id = int(input_params[3])
geom_type = int(input_params[4])
bgo_absl_type = int(input_params[5])
refl_type = int(input_params[6])
refl2_type = int(input_params[7]) # SiPM face (geom4)
N_in = int(input_params[8])
source = input_params[9]
ismorgana = int(input_params[10]) # 0: nohup, 1: slurm
bias = input_params[11]
job_name = input_params[12]
num_threads = int(input_params[13])
num_tasks = int(input_params[14])

N_runs = run_stop - run_start + 1
source_dir = source
Ene_dir = ""

# Volume IDs
scint_copyno = 15
ej560_copyno = 16
shelf_copyno = 50
Al_shelf_copyno = 51
coat_copyno = 70
Al_copyno = 9
wrapper1_copyno = 10
wrapper2_copyno = 11
wrapper3_copyno = 12
wrapper4_copyno = 13

# absorbed
vecScintEventID = []
vecScintEdep = []
vecSiPMEdep = []
vecScintEventIDSingle = []
vecSiPMEdepSingle = []
vecSiPMNabs = []
vecSiPMNabsPDE = []
vecSiPM11Nabs = []
vecScintNopt = []
vecScintNoptErr = []

path_sim = PATH_SIM
N_tot = N_in

refl2_dir = "/REFL2_TYPE" + str(refl2_type)

path_output = "."+"/SIM"+str(sim_id)+"/GEOM_TYPE"+str(geom_type)+"/BGO_ABSL_TYPE"+str(bgo_absl_type)+"/REFL_TYPE"+str(refl_type)+refl2_dir+"/"+str(N_tot)+"/"+source_dir+"/not_collimated/"+str(run_start)+"-"+str(run_stop)+"/"
path_runs = "."+"/SIM"+str(sim_id)+"/GEOM_TYPE"+str(geom_type)+"/BGO_ABSL_TYPE"+str(bgo_absl_type)+"/REFL_TYPE"+str(refl_type)+refl2_dir+"/"+str(N_tot)+"/"+source_dir+"/not_collimated/"
if os.path.exists(path_runs): shutil.rmtree(path_runs)

# PDE
wl = []
pde = []
with open("SiPM_PDE.txt", "r") as f:
	for line in f:
		line = line.strip()
		if not line.startswith("#"):
			columns = line.split()
			wl.append(float(columns[0]))
			pde.append(float(columns[1])/100)
wl = np.array(wl)
pde = np.array(pde)
f = interp1d(wl, pde)

for jrun in range(run_start, run_stop + 1):
	rundir = "/run"+str(jrun)
	N_tasks = len(glob.glob1(path_sim+rundir,"*0.task*.fits*"))
	N_in_per_task = int(N_in / N_tasks) # number of events per task

	for task in range(N_tasks):
		N_fits = len(glob.glob1(path_sim+rundir,"*.task"+str(task)+".fits.gz"))

		for jfits in range(N_fits):

			print('%%%%%%%%%%%%%% READING BoGEMMS-HPC FILE: ', path_sim+rundir+'/xyz.'+str(jfits)+'.task'+str(task)+'.fits.gz')
			hdulist = fits.open(path_sim+rundir+'/xyz.'+str(jfits)+'.task'+str(task)+'.fits.gz')

			tbdata = hdulist[1].data
			evt_id = tbdata.field('EVT_ID') + N_in_per_task * task + N_in * (jrun-1)
			trk_id = tbdata.field('TRK_ID')
			parent_trk_id = tbdata.field('PARENT_TRK_ID')
			vol_id = tbdata.field('VOLUME_ID')
			moth_id = tbdata.field('MOTHER_ID')
			ene_ent = tbdata.field('E_KIN_ENT')
			ene_exit = tbdata.field('E_KIN_EXIT')
			e_dep = tbdata.field('E_DEP')
			mdx_ent = tbdata.field('MDX_ENT')
			mdy_ent = tbdata.field('MDY_ENT')
			mdz_ent = tbdata.field('MDZ_ENT')
			mdx_exit = tbdata.field('MDX_EXIT')
			mdy_exit = tbdata.field('MDY_EXIT')
			mdz_exit = tbdata.field('MDZ_EXIT')
			x_ent = tbdata.field('X_ENT')
			y_ent = tbdata.field('Y_ENT')
			z_ent = tbdata.field('Z_ENT')
			x_exit = tbdata.field('X_EXIT')
			y_exit = tbdata.field('Y_EXIT')
			z_exit = tbdata.field('Z_EXIT')
			part_id = tbdata.field('PARTICLE_ID')
			gtime_ent = tbdata.field('GTIME_ENT')

			index = 0
			where_sameevent = [index]
			temp_index = index
			while (where_sameevent[-1] < (len(evt_id)-1)):
				while evt_id[temp_index] == evt_id[temp_index+1]:
					where_sameevent.append(temp_index+1)
					temp_index += 1
				else:
					sameev_evt_id = evt_id[where_sameevent]
					sameev_trk_id = trk_id[where_sameevent]
					sameev_vol_id = vol_id[where_sameevent]
					sameev_moth_id = moth_id[where_sameevent]
					sameev_ene_ent = ene_ent[where_sameevent]
					sameev_ene_exit = ene_exit[where_sameevent]
					sameev_ene_dep = e_dep[where_sameevent]
					sameev_mdx_ent = mdx_ent[where_sameevent]
					sameev_mdy_ent = mdy_ent[where_sameevent]
					sameev_mdz_ent = mdz_ent[where_sameevent]
					sameev_mdx_exit = mdx_exit[where_sameevent]
					sameev_mdy_exit = mdy_exit[where_sameevent]
					sameev_mdz_exit = mdz_exit[where_sameevent]
					sameev_x_ent = x_ent[where_sameevent]
					sameev_y_ent = y_ent[where_sameevent]
					sameev_z_ent = z_ent[where_sameevent]
					sameev_x_exit = x_exit[where_sameevent]
					sameev_y_exit = y_exit[where_sameevent]
					sameev_z_exit = z_exit[where_sameevent]
					sameev_part_id = part_id[where_sameevent]
					sameev_gtime_ent = gtime_ent[where_sameevent]


					where_scint = np.where((sameev_vol_id == scint_copyno) & (sameev_ene_dep > 0) & (sameev_part_id != -22))
					where_SiPM = np.where((sameev_vol_id == ej560_copyno) & (sameev_ene_dep > 0) & (sameev_x_exit == 100.) & (sameev_part_id == -22))

					# Energy deposits in scintillators and SiPMs
					if (where_scint[0].size):
						evt_id_scint = sameev_evt_id[where_scint]
						moth_id_scint = sameev_moth_id[where_scint]
						edep_scint = sameev_ene_dep[where_scint]

						# Count of optical photons generated
						where_scint_opt = np.where((sameev_vol_id == scint_copyno) & (sameev_part_id == -22))
						if (where_scint_opt[0].size):
							trk_id_scint_opt = sameev_trk_id[where_scint_opt]
							vecScintNopt.append(len(np.unique(trk_id_scint_opt)))
							vecScintNoptErr.append(np.sqrt(len(np.unique(trk_id_scint_opt))))
						else:
							vecScintNopt.append(0)
							vecScintNoptErr.append(0)

						# Count of optical photons that reach the SiPMs
						if (where_SiPM[0].size):
							evt_id_SiPM = sameev_evt_id[where_SiPM]
							moth_id_SiPM = sameev_moth_id[where_SiPM]
							edep_SiPM = sameev_ene_dep[where_SiPM]
							nabs_SiPM = len(edep_SiPM)
							
							vecScintEventID.append(evt_id_scint[0])
							vecSiPMEdep.append(np.sum(edep_SiPM))
							vecScintEdep.append(np.sum(edep_scint))
							vecSiPMNabs.append(nabs_SiPM)
							# Apply PDE
							summ = 0
							for edep in edep_SiPM: 
								energy = edep * 1000 * u.eV
								wave_nm = energy.to(u.nm, equivalencies=u.spectral())
								pde_interp = f(wave_nm)
								summ = summ + pde_interp
							vecSiPMNabsPDE.append(summ)
						else:
							vecScintEventID.append(evt_id_scint[0])
							vecSiPMEdep.append(0)
							vecScintEdep.append(np.sum(edep_scint))
							vecSiPMEdep.append(0)
							vecSiPMNabs.append(0)
							vecSiPMNabsPDE.append(0)
							

					N_event_eq = len(where_sameevent)
					if (evt_id[where_sameevent[-1]+1] != evt_id[-1]):
						index = where_sameevent[N_event_eq-1] + 1
						where_sameevent = [index]
						temp_index = index
					else:
						first_sameevent = where_sameevent[-1]+1
						where_sameevent = np.arange(first_sameevent, len(evt_id), 1)
		
						sameev_evt_id = evt_id[where_sameevent]
						sameev_trk_id = trk_id[where_sameevent]
						sameev_vol_id = vol_id[where_sameevent]
						sameev_moth_id = moth_id[where_sameevent]
						sameev_ene_ent = ene_ent[where_sameevent]
						sameev_ene_exit = ene_exit[where_sameevent]
						sameev_ene_dep = e_dep[where_sameevent]
						sameev_mdx_ent = mdx_ent[where_sameevent]
						sameev_mdy_ent = mdy_ent[where_sameevent]
						sameev_mdz_ent = mdz_ent[where_sameevent]
						sameev_mdx_exit = mdx_exit[where_sameevent]
						sameev_mdy_exit = mdy_exit[where_sameevent]
						sameev_mdz_exit = mdz_exit[where_sameevent]
						sameev_x_ent = x_ent[where_sameevent]
						sameev_y_ent = y_ent[where_sameevent]
						sameev_z_ent = z_ent[where_sameevent]
						sameev_x_exit = x_exit[where_sameevent]
						sameev_y_exit = y_exit[where_sameevent]
						sameev_z_exit = z_exit[where_sameevent]
						sameev_part_id = part_id[where_sameevent]
						sameev_gtime_ent = gtime_ent[where_sameevent]


						where_scint = np.where((sameev_vol_id == scint_copyno) & (sameev_ene_dep > 0) & (sameev_part_id != -22))
						where_SiPM = np.where((sameev_vol_id == ej560_copyno) & (sameev_ene_dep > 0) & (sameev_x_exit == 100.) & (sameev_part_id == -22))

						# Energy deposits in scintillators and SiPMs
						if (where_scint[0].size):
							evt_id_scint = sameev_evt_id[where_scint]
							moth_id_scint = sameev_moth_id[where_scint]
							edep_scint = sameev_ene_dep[where_scint]

							# Count of optical photons generated
							where_scint_opt = np.where((sameev_vol_id == scint_copyno) & (sameev_part_id == -22))
							if (where_scint_opt[0].size):
								trk_id_scint_opt = sameev_trk_id[where_scint_opt]
								vecScintNopt.append(len(np.unique(trk_id_scint_opt)))
								vecScintNoptErr.append(np.sqrt(len(np.unique(trk_id_scint_opt))))
							else:
								vecScintNopt.append(0)
								vecScintNoptErr.append(0)

							# Count of optical photons that reach the SiPMs
							if (where_SiPM[0].size):
								evt_id_SiPM = sameev_evt_id[where_SiPM]
								moth_id_SiPM = sameev_moth_id[where_SiPM]
								edep_SiPM = sameev_ene_dep[where_SiPM]
								nabs_SiPM = len(edep_SiPM)
								
								vecScintEventID.append(evt_id_scint[0])
								vecSiPMEdep.append(np.sum(edep_SiPM))
								vecScintEdep.append(np.sum(edep_scint))
								vecSiPMNabs.append(nabs_SiPM)
								# Apply PDE
								summ = 0
								for edep in edep_SiPM: 
									energy = edep * 1000 * u.eV
									wave_nm = energy.to(u.nm, equivalencies=u.spectral())
									pde_interp = f(wave_nm)
									summ = summ + pde_interp
								vecSiPMNabsPDE.append(summ)
							else:
								vecScintEventID.append(evt_id_scint[0])
								vecSiPMEdep.append(0)
								vecScintEdep.append(np.sum(edep_scint))
								vecSiPMEdep.append(0)
								vecSiPMNabs.append(0)
								vecSiPMNabsPDE.append(0)
		
						break

			hdulist.close()

vecScintEventID = np.array(vecScintEventID)
vecScintEdep = np.array(vecScintEdep)
vecSiPMEdep = np.array(vecSiPMEdep)
vecSiPMNabs = np.array(vecSiPMNabs)
vecScintNopt = np.array(vecScintNopt)
vecScintNoptErr = np.array(vecScintNoptErr)


# Write output
if not os.path.exists(path_output):
	os.makedirs(path_output)

# Open and write file 
f_out = open(path_output+"SiPM.dat", 'w')
print('... Writing '+path_output+"SiPM.dat")

f_out.write("### BoGEMMS simulation ### \n")
f_out.write("# EventID Edep_scint[keV] Nabs_SiPM_PDE Nabs_SiPM N_opt Edep_SiPM[keV]\n")
for je in range(len(vecScintEventID)):

	f_out.write('{0:30}'.format(str(vecScintEventID[je])))
	f_out.write('{0:30}'.format(str(vecScintEdep[je])))
	f_out.write('{0:30}'.format(str(vecSiPMNabsPDE[je])))
	f_out.write('{0:30}'.format(str(vecSiPMNabs[je])))
	f_out.write('{0:30}'.format(str(vecScintNopt[je])))
	f_out.write('{0:30}'.format(str(vecSiPMEdep[je])))

	f_out.write("\n")

f_out.close()

# Copy config file
if not os.path.exists(path_output+"/config"):
	os.makedirs(path_output+"/config")
subprocess.call(["cp", "./"+file_input, path_output+"/config/."])

plt.show()
