"""
 runSiPM.py  -  description
 ---------------------------------------------------------------------------------
 running the Geant4 simulation for the COSI BGO shield prototype with SiPMs
 ---------------------------------------------------------------------------------
 copyright            : (C) 2023 Alex Ciabattoni
 email                : alex.ciabattoni@inaf.it
 ----------------------------------------------
 Usage:
 python runCLAIRE.py file_input
 example:
 python runCLAIRE.py config.txt
 ---------------------------------------------------------------------------------
 Parameters:
 - file_input = input file from where input parameters are read
 --------------------------------------------------------------------------------
 Caveats:
 None
 ---------------------------------------------------------------------------------
 Modification history:
 - 2023/08/29: creation date
"""

import math
import sys, os
import subprocess
from datetime import date, datetime
import shutil
import random

# Help
if len(sys.argv) != 2:
	print("\n")
	print(" runACS.py  -  description\n")
	print(" ----------------------------------------------------------\n")
	print(" Usage:\n\tpython runACS.py file_input\n example:\n\tpython runACS.py config.txt\n")
	print(" ----------------------------------------------------------\n")
	print(" Parameters:\n\t- file_input = input file from where input parameters are read")
	print("\n")
	exit()

# Import line command parameters
arg_list = sys.argv
file_input = arg_list[1]

# Read input parameters from file
input_params = []
with open(file_input, "r") as f_in:
	for line in f_in:
		line = line.strip()
		if not line.startswith("#") and line != "":
			columns = line.split("=")
			input_params.append(columns[-1].strip())

run_start = int(input_params[0])
run_stop = int(input_params[1])
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


N_runs = run_stop - run_start + 1
Ene_dir = str(E_min)+"-"+str(E_max)
halfx = xlen/2
halfy = ylen/2

vecNBeam = []
vecXBeam = []
vecYBeam = []

index_dir = "index_" + str(index)
size_dir = f"{xlen}x{ylen}x{zlen}"

path_sim_main = G4PATH+"/SIM"+str(sim_id)+"/"+size_dir+"/"+index_dir+"/"+Ene_dir+"/"+str(N_in)+"/"

if not os.path.exists(path_sim_main):
	print("... Creating "+path_sim_main)
	os.makedirs(path_sim_main)

print("Starting simulation ...")

os.chdir(path_sim_main)

# loop in the run
for jrun in range(run_start, run_stop + 1):

	os.chdir(path_sim_main)
	rundir = 'run'+str(jrun)+'/'
	gorun = 0
	# creating directory
	if not os.path.exists(rundir):
		print("... Creating "+rundir)
		os.makedirs(rundir)
		gorun = 1
	else:
		if len(os.listdir('./'+rundir)) == 0:
			gorun = 1
		else:
			check_fits = 0
			check_fits_gz = 0
			for fname in os.listdir('./'+rundir):
				if fname.endswith('.fits'):
					check_fits = 1
				if fname.endswith('.fits.gz'):
					check_fits_gz = 1
			if check_fits == 1: 
				shutil.rmtree(rundir)
				os.makedirs(rundir)    
				gorun = 1
			else:
				if check_fits_gz == 0:
					shutil.rmtree(rundir)
					os.makedirs(rundir)    
					gorun = 1
				else:
					gorun = 0
					print("File .fits.gz already present, simulation stopped.\n")
	if gorun == 1:
		file_conf = "response.conf"
		file_mac = "beam.mac"
		
		# copying files in directory
		subprocess.call(["pwd"])
		subprocess.call(["cp", G4PATH+'/'+file_conf, rundir])
		subprocess.call(["cp", G4PATH+'/currentEvent.rndm', rundir])
		subprocess.call(["cp", G4PATH+'/'+file_mac, rundir])
		subprocess.call(["cp", G4PATH+'/'+file_input, rundir])
		
		if isslurm: 
			subprocess.call(["cp", G4PATH+'/thelsim_container.slurm', rundir])
		else:
			subprocess.call(["cp", G4PATH+'/thelsim_container.sh', rundir])
		# changing dir to the G4 run

		os.chdir(rundir)

		f = open(file_conf, "r")
		list_of_lines = f.readlines()
		l = 0
		for line in list_of_lines:
			if line.startswith('GEOM.ACS.CRYSTAL.SIZE.X'):
				list_of_lines[l] = 'GEOM.ACS.CRYSTAL.SIZE.X = '+str(xlen)+'\n'
			if line.startswith('GEOM.ACS.CRYSTAL.SIZE.Y'):
				list_of_lines[l] = 'GEOM.ACS.CRYSTAL.SIZE.Y = '+str(ylen)+'\n'
			if line.startswith('GEOM.ACS.CRYSTAL.SIZE.Z'):
				list_of_lines[l] = 'GEOM.ACS.CRYSTAL.SIZE.Z = '+str(zlen)+'\n'
			if (num_threads > 1):
				if line.startswith('RUN.MT.ACTIVATE'): 
					list_of_lines[l] = 'RUN.MT.ACTIVATE = 1\n'
				if line.startswith('MT.NUM.THREADS'):
					list_of_lines[l] = 'MT.NUM.THREADS = '+str(num_threads)+'\n'
			l = l + 1
		f = open(file_conf, "w")
		f.writelines(list_of_lines)
		f.close()

		f = open(file_mac, "r")
		list_of_lines = f.readlines()
		N_in_run = int(N_in / num_tasks)
		l = 0
		zpos = ' 80. mm\n'
		for line in list_of_lines:
			if line.startswith('/gps/pos/centre'):
				list_of_lines[l] = '/gps/pos/centre 0 0 '+zpos+'\n'
			if line.startswith('/gps/pos/halfx'):
				list_of_lines[l] = '/gps/pos/halfx '+str(halfx)+' mm\n'
			if line.startswith('/gps/pos/halfy'):
				list_of_lines[l] = '/gps/pos/halfy '+str(halfy)+' mm\n'
			if line.startswith('/gps/ene/alpha'):
				list_of_lines[l] = '/gps/ene/alpha '+str(index)+'\n'
			if line.startswith('/gps/ene/min'):
				list_of_lines[l] = '/gps/ene/min '+str(E_min)+' keV\n'
			if line.startswith('/gps/ene/max'):
				list_of_lines[l] = '/gps/ene/max '+str(E_max)+' keV\n'
			if line.startswith('/run/beamOn'):
				list_of_lines[l] = '/run/beamOn '+str(N_in_run)+'\n'
			if num_tasks > 1:
				if line.startswith('/random'):
					list_of_lines[l] = '#'+list_of_lines[l]+'\n'
			l = l + 1
		f = open(file_mac, "w")
		f.writelines(list_of_lines)
		f.close()
		
		f = open('currentEvent.rndm', "r")
		list_of_lines = f.readlines()
		list_of_lines[3] = str(random.randrange(sys.maxsize))[:6]+'\n'
		list_of_lines[4] = str(random.randrange(sys.maxsize))[:6]+'\n'
		f = open('currentEvent.rndm', "w")
		f.writelines(list_of_lines)
		f.close()

		if isslurm:
			f = open('thelsim_container.slurm', "r")
			list_of_lines = f.readlines()
			if num_threads <= 10:
				num_sockets = 1
				num_cores_per_socket = num_threads
			else:
				num_sockets = 2
				num_cores_per_socket = int(num_threads / 2)
			l = 0
			for line in list_of_lines:
				if line.startswith('#SBATCH --job-name'):
					list_of_lines[l] = '#SBATCH --job-name='+job_name
				if line.startswith('##SBATCH -n') and (num_tasks > 1):
					list_of_lines[l] = '#SBATCH -n '+str(num_tasks)+'\n'
				if line.startswith('##SBATCH --sockets') and (num_threads > 1):
					list_of_lines[l] = '#SBATCH --sockets-per-node='+str(num_sockets)+'\n'
				if line.startswith('##SBATCH --cores-per-socket') and (num_threads > 1):
					list_of_lines[l] = '#SBATCH --cores-per-socket='+str(num_cores_per_socket)+'\n'
				if line.startswith('singularity exec'):
					if (num_tasks > 1):
						list_of_lines[l] = 'singularity exec --bind=/blasco /home/ciabattoni/containers/g4_11_1_HPC_v2.sif bash -c ". /home/ciabattoni/ICSC_G4_HPC/profile_g4hpc_11_1 && mpiexec -n '+str(num_tasks)+' /home/ciabattoni/ICSC_G4_HPC/BoGEMMS-HPC/BoGEMMS-HPC-build/bogemms '+file_conf+' 0 '+file_mac+'"\n'
					else:
						list_of_lines[l] = 'singularity exec --bind=/blasco /home/ciabattoni/containers/g4_11_1_HPC_v2.sif bash -c ". /home/ciabattoni/ICSC_G4_HPC/profile_g4hpc_11_1 && /home/ciabattoni/ICSC_G4_HPC/BoGEMMS-HPC/BoGEMMS-HPC-build/bogemms '+file_conf+' 0 '+file_mac+'"\n'
				l = l + 1
			f = open("thelsim_container.slurm", "w")
			f.writelines(list_of_lines)
			f.close()

		print("... Running "+rundir)
		if isslurm == 0: subprocess.Popen(["sh", "thelsim_container.sh", ">", "out.log"])	
		if isslurm == 1: subprocess.Popen(["sbatch", "thelsim_container.slurm"])	
