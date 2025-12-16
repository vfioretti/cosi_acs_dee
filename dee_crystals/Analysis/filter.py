"""
 filter.py  -  description
 ---------------------------------------------------------------------------------
 filtering the events of ACS simulation
 ---------------------------------------------------------------------------------
 copyright            : (C) 2023 Alex Ciabattoni
 email                : alex.ciabattoni@inaf.it
 ----------------------------------------------
 Usage:
 python filter.py PATH_SIM run_start run_stop
 example:
 python filter.py ../Simulations/SIM_DIR/10000/122keV/ 1 1
 ---------------------------------------------------------------------------------
 Parameters:
 - PATH_SIM  = path where the BoGEMMS simulation runs are stored
 - run_start = initial run number
 - run_start = initial run number
 --------------------------------------------------------------------------------
 Caveats:
 None
 ---------------------------------------------------------------------------------
 Modification history:
 - 2024/02/26: creation date
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

from matplotlib import gridspec
import matplotlib as mpl
from mpl_toolkits.axes_grid1 import make_axes_locatable

from astropy.table import Table, Column

import glob

# Help
if len(sys.argv) != 4:
	print("n")
	print(" filter.py  -  description\n")
	print(" ----------------------------------------------------------\n")
	print(" Usage:\n\tpython filter.py PATH_SIM run_start run_stop\n example:\n\tpython filterACS.py ../Simulations/SIM_DIR/10000/122keV/ 1 1\n")
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
file_input = PATH_SIM + "/run" + str(run_start) + "/runResponse.txt"
with open(file_input, "r") as f_in:
	for line in f_in:
		line = line.strip()
		if not line.startswith("#") and line != "":
			columns = line.split("=")
			input_params.append(columns[-1].strip())

#run_start = int(input_params[0])
#run_stop = int(input_params[1])
G4PATH = input_params[2]
sim_id = int(input_params[3])
N_in = int(input_params[4])
E_min = float(input_params[5]) # keV
E_max = float(input_params[6]) # keV
index = float(input_params[7])
xlen = float(input_params[8])
ylen = float(input_params[9])
zlen = float(input_params[10])
isslurm = int(input_params[11]) # 0: nohup, 1: slurm
job_name = input_params[12]
num_threads = int(input_params[13])
num_tasks = int(input_params[14])

# Volume IDs
scint_copyno = 15
ej560_copyno = 16
external_copyno = 10
chamber_copyno = 0

N_runs = run_stop - run_start + 1
Ene_dir = str(E_min)+"-"+str(E_max)

path_sim = PATH_SIM

crystal_dir = str(xlen) + "x" + str(ylen) + "x" + str(zlen)
index_dir = "index_" + str(index)
num_runs = run_stop + 1 - run_start

if "Leonardo" in PATH_SIM: sim_dir = "/Leonardo_SIM"
else: sim_dir = "/SIM"

# Reading PDE
wl = []
pde_file = []
with open("SiPM_PDE.txt", "r") as f:
	for line in f:
		line = line.strip()
		if not line.startswith("#"):
			columns = line.split()
			wl.append(float(columns[0]))
			pde_file.append(float(columns[1])/100)
wl = np.array(wl)
pde_file = np.array(pde_file)
f = interp1d(wl, pde_file)

# Reading FITS files
for jrun in range(run_start, run_stop + 1):

	evtID = [] # event ID
	e_ent = [] # initial energy
	x_enter = [] # initial x position
	y_enter = [] # initial y position
	edep_scint = [] # energy deposit in scintillator
	x_edep = [] # x coordinate of energy deposit in scintillator ((x_ent + x_exit) / 2)
	y_edep = [] # y coordinate of energy deposit in scintillator ((y_ent + y_exit) / 2)
	z_edep = [] # z coordinate of energy deposit in scintillator ((z_ent + z_exit) / 2)
	Nopt = [] # total number of optical photons that reach the SiPMs
	Nopt_pde = [] # number of optical photons detected by the SiPMs (using the PDE)

	rundir = "/run"+str(jrun)
	path_output = "."+sim_dir+str(sim_id)+"/"+crystal_dir+"/"+index_dir+"/"+Ene_dir+"/"+str(N_in)+"/run"+str(jrun)+"/"
	N_tasks = len(glob.glob1(path_sim+rundir,"*0.task*.fits*"))
	N_in_per_task = int(N_in / N_tasks) # number of events per task

	for task in range(N_tasks):
		N_fits = len(glob.glob1(path_sim+rundir,"*.task"+str(task)+".fits.gz"))
	
		for jfits in range(N_fits):

			print('%%%%%%%%%%%%%% READING THELSim FILE: ', path_sim+rundir+'/xyz.'+str(jfits)+'.task'+str(task)+'.fits.gz')
			hdulist = fits.open(path_sim+rundir+'/xyz.'+str(jfits)+'.task'+str(task)+'.fits.gz')

			tbdata = hdulist[1].data
			evt_id = tbdata.field('EVT_ID') + N_in_per_task * task
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

			if evt_id[0] != evt_id[-1]:
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
						sameev_parent_trk_id = parent_trk_id[where_sameevent]
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
						where_SiPM = np.where((sameev_vol_id == ej560_copyno) & (sameev_ene_dep > 0) & (np.round(sameev_x_exit,2) == np.round(xlen/2.+1.,2)) & (sameev_part_id == -22))
						where_ent = np.where((sameev_vol_id == chamber_copyno) & (sameev_trk_id == 1) & (sameev_parent_trk_id == 0) & (sameev_mdz_ent == -1))

						if where_scint[0].size: # check whether there was an energy deposit in scintillator for this event
							evt_id_value = sameev_evt_id[where_scint][0] # take event
							e_ent_value = sameev_ene_ent[where_ent][0] # take initial energy
							x_ent_value, y_ent_value = sameev_x_ent[where_ent][0], sameev_y_ent[where_ent][0] # take initial position
							edep_scint_values = sameev_ene_dep[where_scint] # take all energy deposits
							trk_id_edep = sameev_trk_id[where_scint] # take the track ID of particles that deposit energy
							x_exit_edep = sameev_x_exit[where_scint] # take x_exit of particles that deposit energy
							y_exit_edep = sameev_y_exit[where_scint] # take y_exit of particles that deposit energy
							z_exit_edep = sameev_z_exit[where_scint] # take z_exit of particles that deposit energy

							trk_id_opt = sameev_trk_id[where_SiPM] # take optical photons that reach the SiPMs
							parent_trk_id_opt = sameev_parent_trk_id[where_SiPM] # take parent ID of optical photons that reach the SiPMs
							edep_opt = sameev_ene_dep[where_SiPM] # take energy deposit by optical photons in SiPMs
							for trkid, e_scint, x_ex, y_ex, z_ex in zip(trk_id_edep, edep_scint_values, x_exit_edep, y_exit_edep, z_exit_edep):
								where_SiPM_parent = np.where(parent_trk_id_opt == trkid)
								if not where_SiPM_parent[0].size: # check whether there are optical photons that reach the SiPMs for this parent trkID
									evtID.append(sameev_evt_id[0])
									e_ent.append(e_ent_value)
									x_enter.append(x_ent_value)
									y_enter.append(y_ent_value)
									edep_scint.append(e_scint)
									x_edep.append(x_ex)
									y_edep.append(y_ex)
									z_edep.append(z_ex)
									Nopt.append(0)
									Nopt_pde.append(0) 
								else:
									trk_id_opt_parent = trk_id_opt[where_SiPM_parent] # take trkID of optical photons that have this parent trkID
									Nopt_value = len(trk_id_opt_parent) # take their number
									edep_opt_parent = edep_opt[where_SiPM_parent] # take edep
									Nopt_pde_value = 0 # number of detected optical photons (using PDE)
									for e_opt in edep_opt_parent: # take PDE for each optical photon
										e_opt_eV = e_opt * 1000 * u.eV # convert in eV
										wave_nm = e_opt_eV.to(u.nm, equivalencies=u.spectral()) # take the corresponding wavelength
										Nopt_pde_value += f(wave_nm) # add the corresponding pde
									evtID.append(sameev_evt_id[0])
									e_ent.append(e_ent_value)
									x_enter.append(x_ent_value)
									y_enter.append(y_ent_value)
									edep_scint.append(e_scint)
									x_edep.append(x_ex)
									y_edep.append(y_ex)
									z_edep.append(z_ex)
									Nopt.append(Nopt_value)
									Nopt_pde.append(Nopt_pde_value) 
								

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
							sameev_parent_trk_id = parent_trk_id[where_sameevent]
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
							where_SiPM = np.where((sameev_vol_id == ej560_copyno) & (sameev_ene_dep > 0) & (np.round(sameev_x_exit,2) == np.round(xlen/2.+1.,2)) & (sameev_part_id == -22))
							where_ent = np.where((sameev_vol_id == chamber_copyno) & (sameev_trk_id == 1) & (sameev_parent_trk_id == 0) & (sameev_mdz_ent == -1))

							if where_scint[0].size: # check whether there was an energy deposit in scintillator for this event
								evt_id_value = sameev_evt_id[where_scint][0] # take event
								e_ent_value = sameev_ene_ent[where_ent][0] # take initial energy
								x_ent_value, y_ent_value = sameev_x_ent[where_ent][0], sameev_y_ent[where_ent][0] # take initial position
								edep_scint_values = sameev_ene_dep[where_scint] # take all energy deposits
								trk_id_edep = sameev_trk_id[where_scint] # take the track ID of particles that deposit energy
								x_exit_edep = sameev_x_exit[where_scint] # take x_exit of particles that deposit energy
								y_exit_edep = sameev_y_exit[where_scint] # take y_exit of particles that deposit energy
								z_exit_edep = sameev_z_exit[where_scint] # take z_exit of particles that deposit energy

								trk_id_opt = sameev_trk_id[where_SiPM] # take optical photons that reach the SiPMs
								parent_trk_id_opt = sameev_parent_trk_id[where_SiPM] # take parent ID of optical photons that reach the SiPMs
								edep_opt = sameev_ene_dep[where_SiPM] # take energy deposit by optical photons in SiPMs
								for trkid, e_scint, x_ex, y_ex, z_ex in zip(trk_id_edep, edep_scint_values, x_exit_edep, y_exit_edep, z_exit_edep):
									where_SiPM_parent = np.where(parent_trk_id_opt == trkid)
									if not where_SiPM_parent[0].size: # check whether there are optical photons that reach the SiPMs for this parent trkID
										evtID.append(sameev_evt_id[0])
										e_ent.append(e_ent_value)
										x_enter.append(x_ent_value)
										y_enter.append(y_ent_value)
										edep_scint.append(e_scint)
										x_edep.append(x_ex)
										y_edep.append(y_ex)
										z_edep.append(z_ex)
										Nopt.append(0)
										Nopt_pde.append(0) 
									else:
										trk_id_opt_parent = trk_id_opt[where_SiPM_parent] # take trkID of optical photons that have this parent trkID
										Nopt_value = len(trk_id_opt_parent) # take their number
										edep_opt_parent = edep_opt[where_SiPM_parent] # take edep
										Nopt_pde_value = 0 # number of detected optical photons (using PDE)
										for e_opt in edep_opt_parent: # take PDE for each optical photon
											e_opt_eV = e_opt * 1000 * u.eV # convert in eV
											wave_nm = e_opt_eV.to(u.nm, equivalencies=u.spectral()) # take the corresponding wavelength
											Nopt_pde_value += f(wave_nm) # add the corresponding pde
										evtID.append(sameev_evt_id[0])
										e_ent.append(e_ent_value)
										x_enter.append(x_ent_value)
										y_enter.append(y_ent_value)
										edep_scint.append(e_scint)
										x_edep.append(x_ex)
										y_edep.append(y_ex)
										z_edep.append(z_ex)
										Nopt.append(Nopt_value)
										Nopt_pde.append(Nopt_pde_value) 
			
							break

				hdulist.close()
			else:
				where_sameevent = range(len(evt_id))
				sameev_evt_id = evt_id[where_sameevent]
				sameev_trk_id = trk_id[where_sameevent]
				sameev_parent_trk_id = parent_trk_id[where_sameevent]
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
				where_SiPM = np.where((sameev_vol_id == ej560_copyno) & (sameev_ene_dep > 0) & (np.round(sameev_x_exit,2) == np.round(xlen/2.+1.,2)) & (sameev_part_id == -22))
				where_ent = np.where((sameev_vol_id == chamber_copyno) & (sameev_trk_id == 1) & (sameev_parent_trk_id == 0) & (sameev_mdz_ent == -1))

				if where_scint[0].size: # check whether there was an energy deposit in scintillator for this event
					evt_id_value = sameev_evt_id[where_scint][0] # take event
					e_ent_value = sameev_ene_ent[where_ent][0] # take initial energy
					x_ent_value, y_ent_value = sameev_x_ent[where_ent][0], sameev_y_ent[where_ent][0] # take initial position
					edep_scint_values = sameev_ene_dep[where_scint] # take all energy deposits
					trk_id_edep = sameev_trk_id[where_scint] # take the track ID of particles that deposit energy
					x_exit_edep = sameev_x_exit[where_scint] # take x_exit of particles that deposit energy
					y_exit_edep = sameev_y_exit[where_scint] # take y_exit of particles that deposit energy
					z_exit_edep = sameev_z_exit[where_scint] # take z_exit of particles that deposit energy

					trk_id_opt = sameev_trk_id[where_SiPM] # take optical photons that reach the SiPMs
					parent_trk_id_opt = sameev_parent_trk_id[where_SiPM] # take parent ID of optical photons that reach the SiPMs
					edep_opt = sameev_ene_dep[where_SiPM] # take energy deposit by optical photons in SiPMs
					for trkid, e_scint, x_ex, y_ex, z_ex in zip(trk_id_edep, edep_scint_values, x_exit_edep, y_exit_edep, z_exit_edep):
						where_SiPM_parent = np.where(parent_trk_id_opt == trkid)
						if not where_SiPM_parent[0].size: # check whether there are optical photons that reach the SiPMs for this parent trkID
							evtID.append(sameev_evt_id[0])
							e_ent.append(e_ent_value)
							x_enter.append(x_ent_value)
							y_enter.append(y_ent_value)
							edep_scint.append(e_scint)
							x_edep.append(x_ex)
							y_edep.append(y_ex)
							z_edep.append(z_ex)
							Nopt.append(0)
							Nopt_pde.append(0) 
						else:
							trk_id_opt_parent = trk_id_opt[where_SiPM_parent] # take trkID of optical photons that have this parent trkID
							Nopt_value = len(trk_id_opt_parent) # take their number
							edep_opt_parent = edep_opt[where_SiPM_parent] # take edep
							Nopt_pde_value = 0 # number of detected optical photons (using PDE)
							for e_opt in edep_opt_parent: # take PDE for each optical photon
								e_opt_eV = e_opt * 1000 * u.eV # convert in eV
								wave_nm = e_opt_eV.to(u.nm, equivalencies=u.spectral()) # take the corresponding wavelength
								Nopt_pde_value += f(wave_nm) # add the corresponding pde
							evtID.append(sameev_evt_id[0])
							e_ent.append(e_ent_value)
							x_enter.append(x_ent_value)
							y_enter.append(y_ent_value)
							edep_scint.append(e_scint)
							x_edep.append(x_ex)
							y_edep.append(y_ex)
							z_edep.append(z_ex)
							Nopt.append(Nopt_value)
							Nopt_pde.append(Nopt_pde_value) 

				hdulist.close()

	evtID = np.array(evtID)
	e_ent = np.array(e_ent)
	x_enter = np.array(x_enter)
	y_enter = np.array(y_enter)
	edep_scint = np.array(edep_scint)
	x_edep = np.array(x_edep)
	y_edep = np.array(y_edep)
	z_edep = np.array(z_edep)
	Nopt = np.array(Nopt)
	Nopt_pde = np.array(Nopt_pde)

	if not os.path.exists(path_output):
		os.makedirs(path_output)

	# Open and write file 
	f_out = open(path_output+"SiPM.dat", 'w')
	print('... Writing '+path_output+"SiPM.dat")

	f_out.write("### BoGEMMS simulation ### \n")
	f_out.write("# EventID E_ent[keV] Edep[keV] x_dep[mm] y_dep[mm] z_dep[mm] Nopt Nopt_PDE \n")
	for je in range(len(evtID)):

		f_out.write('{0:30}'.format(str(evtID[je])))
		f_out.write('{0:30}'.format(str(e_ent[je])))
		f_out.write('{0:30}'.format(str(edep_scint[je])))
		f_out.write('{0:30}'.format(str(x_edep[je])))
		f_out.write('{0:30}'.format(str(y_edep[je])))
		f_out.write('{0:30}'.format(str(z_edep[je])))
		f_out.write('{0:30}'.format(str(Nopt[je])))
		f_out.write('{0:30}'.format(str(Nopt_pde[je])))

		f_out.write("\n")

	f_out.close()

	# Copy config file
	if not os.path.exists(path_output+"/config"):
		os.makedirs(path_output+"/config")
	subprocess.call(["cp", "./"+file_input, path_output+"/config/."])
