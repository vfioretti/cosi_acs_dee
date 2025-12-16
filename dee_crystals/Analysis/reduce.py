import numpy as np
import math
import sys, os
import matplotlib.pyplot as plt
import fnmatch
import yaml

def gaus(x, a, sigma, mu):
    return a/(sigma*math.sqrt(2*math.pi))*np.exp(-((x-mu)**2)/(2*sigma**2))

# Import the input parameters
arg_list = sys.argv
yaml_file = arg_list[1]
dirs = []
for arg in arg_list[2:]:
    dirs.append(arg)

# Load the YAML file
with open(yaml_file, 'r') as f:
    params = yaml.safe_load(f)
name = params['name']
fwhm_noise = params['fwhm_noise']
m = params['m_cal']
q = params['q_cal']
em = params['em_cal']
eq = params['eq_cal']
x_len_mm = params['x_len_mm']
y_len_mm = params['y_len_mm']
z_len_mm = params['z_len_mm']
NbinX = params['NbinX']
NbinY = params['NbinY']
xlen = params['x_len_mm']
ylen = params['y_len_mm']
E_min = params['E_min_keV']
E_max = params['E_max_keV']
num_bins = params['num_bins']

file_output = f"SiPM_final_{name}.txt"
with open(file_output, "w") as f_out:
    f_out.write("# EventID E_ent[keV] x[mm] y[mm] z[mm] E_dep[keV] Nabs Nabs_err\n")

filename_sim = "SiPM.dat"

# Position bins
edgesX = np.linspace(-xlen/2, xlen/2, NbinX + 1)
edgesY = np.linspace(-ylen/2, ylen/2, NbinY + 1)
centersX = (edgesX[:-1] + edgesX[1:]) / 2
centersY = (edgesY[:-1] + edgesY[1:]) / 2

global_EventID = 0
nfile = 0
dirs_panel = [dir+f"/{float(x_len_mm)}x{float(y_len_mm)}x{float(z_len_mm)}/" for dir in dirs]
for dir in dirs_panel:
    for root, dirnames, filenames in os.walk(dir):
        for file_match in fnmatch.filter(filenames, filename_sim):
            file = os.path.join(root, file_match)

            print("%%%%%%%%%%%%% READING OUTPUT FILE: " + file)

            EventID, E_ent, x, y, z, E_dep, Nabs = [], [], [], [], [], [], []

            current_event = 0
            with open(file, "r") as f:
                for line in f:
                    line = line.strip()
                    if not line.startswith("#"):
                        first_line = line
                        break
            with open(file, "r") as f:
                last_line = f.readlines()[-1].strip()
            with open(file, "r") as f:
                for line in f:
                    line = line.strip()
                    if not line.startswith("#"):
                        columns = line.split()
                        if line == first_line:
                            #print("Reading first line\n") 
                            current_event = float(columns[0])
                        event = float(columns[0])
                        if event == current_event:
                            EventID.append(float(columns[0]))
                            E_ent.append(float(columns[1]))
                            E_dep.append(float(columns[2]))
                            x.append(float(columns[3]))
                            y.append(float(columns[4]))
                            z.append(float(columns[5]))
                            Nabs.append(float(columns[7]))
                        if event != current_event or line == last_line:
                            EventID, E_ent, x, y, z, E_dep, Nabs = np.array(EventID), np.array(E_ent), np.array(x), np.array(y), np.array(z), np.array(E_dep), np.array(Nabs)
                            #check
                            if len(np.unique(EventID)) > 1 or len(np.unique(E_ent)) > 1: 
                                print(EventID)
                                print(E_ent)
                                print("ERROR!")
                            for xmin, xmax in zip(edgesX[:-1], edgesX[1:]):
                                for ymin, ymax in zip(edgesY[:-1], edgesY[1:]):
                                    Edep_bin = E_dep[(xmin <= x) & (x < xmax) & (ymin <= y) & (y < ymax)]
                                    Nabs_bin = Nabs[(xmin <= x) & (x < xmax) & (ymin <= y) & (y < ymax)]
                                    if len(Edep_bin) > 0:
                                        Edep_sum = np.sum(Edep_bin)
                                        Nabs_sum = np.sum(Nabs_bin)
                                        Nabs_sum_err = np.sqrt(Nabs_sum)
                                        xc = (xmin + xmax) / 2
                                        yc = (ymin + ymax) / 2
                                        zc = 0
                                        with open(file_output, "a") as f_out:
                                            f_out.write('{0:30}'.format(str(global_EventID)))
                                            f_out.write('{0:30}'.format(str(E_ent[0])))
                                            f_out.write('{0:30}'.format(str(xc)))
                                            f_out.write('{0:30}'.format(str(yc)))
                                            f_out.write('{0:30}'.format(str(zc)))
                                            f_out.write('{0:30}'.format(str(Edep_sum)))
                                            f_out.write('{0:30}'.format(str(Nabs_sum)))
                                            f_out.write('{0:30}'.format(str(Nabs_sum_err)))
                                            f_out.write('\n')
                                        global_EventID += 1

                            if line != last_line:  
                                EventID, E_ent, x, y, z, E_dep, Nabs = [], [], [], [], [], [], []
                                EventID.append(float(columns[0]))
                                E_ent.append(float(columns[1]))
                                E_dep.append(float(columns[2]))
                                x.append(float(columns[3]))
                                y.append(float(columns[4]))
                                z.append(float(columns[5]))
                                Nabs.append(float(columns[7]))
                                current_event = EventID[-1]
            
            nfile += 1