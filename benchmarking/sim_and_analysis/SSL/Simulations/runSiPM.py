"""
 runSiPM.py  -  description
 ---------------------------------------------------------------------------------
 running the Geant4 simulation for the COSI BGO shield prototype with SiPMs
 ---------------------------------------------------------------------------------
 copyright            : (C) 2023 Alex Ciabattoni
 email                : alex.ciabattoni@inaf.it
 ----------------------------------------------
 Usage:
 python runSiPM.py file_input
 example:
 python runSiPM.py runSiPM.txt
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
    print(" runSiPM.py  -  description\n")
    print(" ----------------------------------------------------------\n")
    print(" Usage:\n\tpython runSiPM.py file_input\n example:\n\tpython runSiPM.py runSiPM.txt\n")
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
geom_type = int(input_params[4])
bgo_absl_type = int(input_params[5])
refl_type = int(input_params[6])
refl2_type = int(input_params[7]) # SiPM face
N_in = int(input_params[8])
source = input_params[9]
isslurm = int(input_params[10]) # 0: nohup, 1: slurm
bias = input_params[11]
job_name = input_params[12]
num_threads = int(input_params[13])
num_tasks = int(input_params[14])
num_sockets = int(input_params[15])
num_cores_per_socket = int(input_params[16])

cosi_acs_dee_repo = os.getenv("COSI_ACS_DEE_DIR")
if geom_type < 9:
   working_dir = cosi_acs_dee_repo+"/benchmarking/sim_and_analysis/NRL/Simulations"
if geom_type == 9:
   working_dir = cosi_acs_dee_repo+"/benchmarking/sim_and_analysis/SSL/Simulations"

N_runs = run_stop - run_start + 1

ang_type = "not_collimated"
refl2_dir = "/REFL2_TYPE" + str(refl2_type)
source_dir = source

path_sim_main = G4PATH+"/SIM"+str(sim_id)+"/GEOM_TYPE"+str(geom_type)+"/BGO_ABSL_TYPE"+str(bgo_absl_type)+"/REFL_TYPE"+str(refl_type)+refl2_dir+"/"+str(N_in)+"/"+source_dir+"/"+ang_type

if not os.path.exists(path_sim_main):
    print("... Creating "+path_sim_main)
    os.makedirs(path_sim_main)

print("Starting simulation ...")

os.chdir(path_sim_main)

# loop in the run
for jrun in range(run_start, run_stop + 1):
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
        file_conf = "SiPM.conf"
        file_mac = ""
        if source == "Am241": file_mac = "Am241.mac"
        if source == "Cd109": file_mac = "Cd109.mac"
        if source == "Co57": file_mac = "Co57.mac"
        if source == "Ba133": file_mac = "Ba133.mac"
        if source == "Na22": file_mac = "Na22.mac"
        if source == "Cs137": file_mac = "Cs137.mac"
        if source == "Co60": file_mac = "Co60.mac"
        if source == "Y88": file_mac = "Y88.mac"
        
        # copying files in directory
        subprocess.call(["pwd"])
        subprocess.call(["cp", working_dir+'/'+file_conf, rundir])
        subprocess.call(["cp", working_dir+'/'+file_mac, rundir])
        subprocess.call(["cp", working_dir+'/'+file_input, rundir])
        
        if isslurm: 
            subprocess.call(["cp", working_dir+'/bogemms_container.slurm', rundir])

        # changing dir to the G4 run

        os.chdir(rundir)

        f = open(file_conf, "r")
        list_of_lines = f.readlines()
        l = 0
        if source == "Cd109" or source == "Ba133" or source == "Co60" or source == "Y88": is_casing = 1
        else: is_casing = 0
        for line in list_of_lines:
            if line.startswith('GEOM.VERSION'):
               if geom_type == 9: list_of_lines[l] = 'GEOM.VERSION = GeometryCOSI_EMXwall\n'                     
            if line.startswith('GEOM.COSI.ACS.TYPE'):
                list_of_lines[l] = 'GEOM.COSI.ACS.TYPE = '+str(geom_type)+'\n'
            if line.startswith('PHYS.COSI.BGO.ABSL.TYPE'):
                list_of_lines[l] = 'PHYS.COSI.BGO.ABSL.TYPE = '+str(bgo_absl_type)+'\n'
            if line.startswith('PHYS.ACS.OPTSURFACE.WRAPPER'):
                list_of_lines[l] = 'PHYS.ACS.OPTSURFACE.WRAPPER = '+str(refl_type)+'\n'
            if line.startswith('PHYS.ACS.OPTSURFACE2.WRAPPER'):
                list_of_lines[l] = 'PHYS.ACS.OPTSURFACE2.WRAPPER = '+str(refl2_type)+'\n'
            if line.startswith('GEOM.COSI.IS.CASING'):
                list_of_lines[l] = 'GEOM.COSI.IS.CASING = '+str(is_casing)+'\n'    
            if line.startswith('GEOM.CAD.PATH'):
                list_of_lines[l] = 'GEOM.CAD.PATH = '+cosi_acs_dee_repo+'/external/BoGEMMS-HPC/cad_files \n' 
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
        N_in_task = int(N_in / num_tasks)
        l = 0
        for line in list_of_lines:
            if line.startswith('/run/beamOn'):
                list_of_lines[l] = '/run/beamOn '+str(N_in_task)+'\n'
            l = l + 1
        f = open(file_mac, "w")
        f.writelines(list_of_lines)
        f.close()

        if isslurm:
            container_path = input_params[17]
            f = open('bogemms_container.slurm', "r")
            list_of_lines = f.readlines()
            l = 0
            for line in list_of_lines:
                if line.startswith('##SBATCH -n'):
                    list_of_lines[l] = '#SBATCH -n '+str(num_tasks)+'\n'
                if line.startswith('##SBATCH --sockets'):
                    list_of_lines[l] = '#SBATCH --sockets-per-node='+str(num_sockets)+'\n'
                if line.startswith('##SBATCH --cores-per-socket'):
                    list_of_lines[l] = '#SBATCH --cores-per-socket='+str(num_cores_per_socket)+'\n'
                if line.startswith('#SBATCH --job-name'):
                    list_of_lines[l] = '#SBATCH --job-name='+job_name+'\n'
                if line.startswith('singularity exec'):
                    list_of_lines[l] = 'singularity exec --bind '+path_sim_main+'/'+rundir+':/work '+container_path+' bash -lc "cd /work && mpiexec -n ${num_tasks} bogemms -c '+file_conf+' -m '+file_mac+'"\n'
                l = l + 1
            f = open("bogemms_container.slurm", "w")
            f.writelines(list_of_lines)
            f.close()

        print("... Running "+rundir)
        #if isslurm == 0: subprocess.Popen(["sh", "thelsim_container.sh", ">", "out.log"])    
        if isslurm == 0: subprocess.Popen(["bogemms", "-c", file_conf, "-m", file_mac])    
        if isslurm == 1: subprocess.Popen(["sbatch", "bogemms_container.slurm"])    
