import numpy as np
import math
import sys, os
import shutil
import matplotlib.pyplot as plt
from matplotlib.backends.backend_pdf import PdfPages
from matplotlib.gridspec import GridSpec
from matplotlib.colors import TwoSlopeNorm
from matplotlib.colors import Normalize
from matplotlib.cm import ScalarMappable
from matplotlib import gridspec
from matplotlib import cm
import matplotlib as mpl
from mpl_toolkits.axes_grid1 import make_axes_locatable
from astropy.table import Table, Column
from astropy import units as u
from astropy.stats import sigma_clip
from scipy.optimize import curve_fit
import yaml

def gaus(x, a, w, xc):
    return (a/w)*np.sqrt(4*np.log(2)/np.pi)*np.exp(-4*np.log(2)*(x-xc)**2/w**2)

# Import the input parameters
arg_list = sys.argv
file = arg_list[1]
yaml_file = arg_list[2]

# Load the YAML file
with open(yaml_file, 'r') as f:
    params = yaml.safe_load(f)
name = params['name']
fwhm_noise = params['fwhm_noise']
m = params['m_cal']
q = params['q_cal']
em = params['em_cal']
eq = params['eq_cal']
NbinX = params['NbinX']
NbinY = params['NbinY']
x_to_megalib = params['x_to_megalib']
y_to_megalib = params['y_to_megalib']
z_to_megalib = params['z_to_megalib']
NbinZ = params['NbinZ']
xlen = params['x_len_mm']
ylen = params['y_len_mm']
E_min = params['E_min_keV']
E_max = params['E_max_keV']
num_bins = params['num_bins']

dir_out = os.path.dirname(os.path.abspath(file))

print("Calculating detector response matrix ... \n")

# Read file
EventID, E_ent, x, y, z, Edep, Nabs, Nabs_err = np.genfromtxt(file, usecols=(0,1,2,3,4,5,6,7), unpack=True)

plots_dir = f'./plots_{name}/'
if not os.path.exists(plots_dir): os.makedirs(plots_dir)

sigma_noise = fwhm_noise / (2 * np.sqrt(2 * np.log(2)))

#m, q = 3.94, 2.3
#em, eq = 0.01, 0.5

Emeas = m * Nabs + q
Emeas_err = np.sqrt((em * Nabs)**2 + (m * Nabs_err)**2 + eq**2)

Emeas = np.random.normal(Emeas, sigma_noise)

x_to_megalib, y_to_megalib, z_to_megalib = np.array(x_to_megalib), np.array(y_to_megalib), np.array(z_to_megalib)
index_x, index_y, index_z = np.where(x_to_megalib != 0)[0], np.where(y_to_megalib != 0)[0], np.where(z_to_megalib != 0)[0]
if len(index_x) != 1 or len(index_y) != 1 or len(index_z) != 1: raise ValueError('x_to_megalib, y_to_megalib and z_to_megalib must be 1D arrays with only one non-zero element each.')
elif len({index_x[0], index_y[0], index_z[0]}) != 3: raise ValueError('x_to_megalib, y_to_megalib and z_to_megalib must be 1D arrays having the non-zero element in different positions.')
else: index_x, index_y, index_z = index_x[0], index_y[0], index_z[0]
swap_x = False
if x_to_megalib[index_x] < 0: swap_x = True


##########################################
### Spatial binning analysis ###
##########################################

chi2_red = []
sym = 1

# Position bins
edgesX = np.linspace(-xlen/2, xlen/2, NbinX + 1)
edgesY = np.linspace(-ylen/2, ylen/2, NbinY + 1)
centersX = (edgesX[:-1] + edgesX[1:]) / 2
centersY = (edgesY[:-1] + edgesY[1:]) / 2

# Energy bins
edgesE = np.logspace(np.log10(E_min), np.log10(E_max), num_bins + 1)
centersE = np.sqrt(edgesE[:-1] * edgesE[1:])

pdf_file1 = os.path.join(plots_dir, "Fits.pdf")
pdf1 = PdfPages(pdf_file1)
pdf_file2 = os.path.join(plots_dir, "response_matrix.pdf")
pdf2 = PdfPages(pdf_file2)
pdf_file3 = os.path.join(plots_dir, "gaussians.pdf")
pdf3 = PdfPages(pdf_file3)

save_pos = 86
#save_pos = 10000
pos = 0
if sym == 0: lim = NbinY
else: lim = NbinY/2

for y_min, y_max, cY, y_min_sym, y_max_sym, cY_sym in zip(edgesY[:-1], edgesY[1:], centersY, edgesY[-2::-1], edgesY[-1:0:-1], centersY[::-1]):
    for x_min, x_max, cX in zip(edgesX[:-1], edgesX[1:], centersX):

        print(f"%%%%%%%%%%% Analyzing position {pos} / {NbinX * NbinY} ...")

        fig1 = plt.figure(1, figsize=(12,7))
        gs = GridSpec(2, 2, figure=fig1)
        ax1 = fig1.add_subplot(gs[0, :]) 
        ax2 = fig1.add_subplot(gs[1, 0])
        ax3 = fig1.add_subplot(gs[1, 1])

        #if pos == save_pos:
        fig4 = plt.figure(4, figsize=(21,7))
        ax4 = fig4.add_subplot(111)

        
        if sym == 0: cond_pos = (x_min < x) & (x < x_max) & (y_min < y) & (y < y_max)
        else: cond_pos = ((x_min < x) & (x < x_max) & (y_min < y) & (y < y_max)) | ((x_min < x) & (x < x_max) & (y_min_sym < y) & (y < y_max_sym))
        Edep_pos = Edep[cond_pos]
        Emeas_pos = Emeas[cond_pos]

        Edep_hist, edges = np.histogram(Edep_pos, bins = edgesE)
        max_emeas = np.max(Emeas_pos)
        min_emeas = np.min(Emeas_pos)
        response, response_err, number, number_err = [], [], [], []
        response_map, response_err_map, number_map, number_err_map = [], [], [], []
        centers_meas = []
        for edep_h, edep_min, edep_max, edep_c in zip(Edep_hist, edgesE[:-1], edgesE[1:], centersE):
            Emeas_bin = Emeas_pos[(edep_min <= Edep_pos) & (Edep_pos <= edep_max)]
            Edep_bin = Edep_pos[(edep_min <= Edep_pos) & (Edep_pos <= edep_max)]
            emeas_rel = []
            for emeas, etrue in zip(Emeas_bin, Edep_bin):
                emeas_rel.append(edep_c + emeas - etrue)
            emeas_rel = np.array(emeas_rel)
            #clipped_data = sigma_clip(emeas_rel, sigma = 2.5, maxiters = 5)
            #emeas_rel = clipped_data.data[~clipped_data.mask]
            #emeas_rel = emeas_rel.compressed()
            #n = int((edgesE[0] - min_emeas) / bin_width_meas) + 1
            #edges_meas = np.arange(edgesE[0] - n * bin_width_meas, max_emeas + bin_width_meas, bin_width_meas)

            # Apply Freedman-Diaconis rule to get the optimal bin width 
            iqr = np.percentile(emeas_rel, 75) - np.percentile(emeas_rel, 25)
            bin_width = 2 * iqr * len(emeas_rel) ** (-1/3)
            bin_width_map = 5
            nbins_meas = int(np.ceil((emeas_rel.max() - emeas_rel.min()) / bin_width))
            edges_meas = np.linspace(emeas_rel.min(), emeas_rel.max(), nbins_meas + 1)
            nbins_meas_map = int(np.ceil((edgesE[-1] - edgesE[0]) / bin_width_map))
            edges_meas_map = np.linspace(edgesE[0], edgesE[-1], nbins_meas_map + 1)
            centers_meas.append((edges_meas[1:] + edges_meas[:-1]) / 2)
            Emeas_hist, egdes = np.histogram(emeas_rel, bins = edges_meas)
            Emeas_hist_map, egdes_map = np.histogram(emeas_rel, bins = edges_meas_map)
            fraction, fraction_err, num, num_err = [], [], [], []
            fraction_map, fraction_err_map, num_map, num_err_map = [], [], [], []
            for emeas_h in Emeas_hist:
                if edep_h != 0: 
                    fraction.append(emeas_h / edep_h)
                    fraction_err.append(np.sqrt((np.sqrt(emeas_h) / edep_h)**2 + (emeas_h * np.sqrt(edep_h) / edep_h**2)**2))
                    num.append(emeas_h)
                    num_err.append(np.sqrt(emeas_h))
                else: 
                    fraction.append(0)
                    fraction_err.append(0)
                    num.append(emeas_h)
                    num_err.append(np.sqrt(emeas_h))
            for emeas_h in Emeas_hist_map:
                if edep_h != 0: 
                    fraction_map.append(emeas_h / edep_h)
                    fraction_err_map.append(np.sqrt((np.sqrt(emeas_h) / edep_h)**2 + (emeas_h * np.sqrt(edep_h) / edep_h**2)**2))
                    num_map.append(emeas_h)
                    num_err_map.append(np.sqrt(emeas_h))
                else: 
                    fraction_map.append(0)
                    fraction_err_map.append(0)
                    num_map.append(emeas_h)
                    num_err_map.append(np.sqrt(emeas_h))
            response.append(np.array(fraction))
            response_err.append(np.array(fraction_err))
            number.append(np.array(num))
            number_err.append(np.array(num_err))
            response_map.append(np.array(fraction_map))
            response_err_map.append(np.array(fraction_err_map))
            number_map.append(np.array(num_map))
            number_err_map.append(np.array(num_err_map))


        fig5 = plt.figure(5, figsize=(10,7))
        ax5 = fig5.add_subplot(111)

        response_map = np.array(response_map).T # transpose
        response_err_map = np.array(response_err_map).T # transpose

        X, Y = np.meshgrid(edgesE, edges_meas_map)  # Creiamo le griglie con i bordi dei bin

        map = ax5.pcolormesh(X, Y, response_map, shading='auto', cmap='viridis', rasterized=True)
        colorbar = fig5.colorbar(map, ax=ax5)
        colorbar.set_label(r'Probability distribution', fontsize=20)
        colorbar.ax.tick_params(labelsize=15)

        ax5.plot(edgesE, edgesE, color='red', linestyle='--', linewidth=2, label=r'$E_{meas} = E_{true}$')

        ax5.set_title(f"Position {pos}", fontsize=20)
        ax5.set_xlabel(r"$E_{true}$ [keV]", fontsize=20)
        ax5.set_ylabel(r"$E_{meas}$ [keV]", fontsize=20)
        ax5.set_xlim([edgesE[0], edgesE[-1]])
        ax5.set_ylim([edgesE[0], edgesE[-1]])
        ax5.set_xscale('log')
        ax5.set_yscale('log')
        ax5.tick_params(axis='both', which='major', labelsize=15)
        ax5.tick_params(axis='both', which='minor', labelsize=15)
        ax5.legend(fontsize=20)
        fig5.tight_layout()
        #fig5.savefig(os.path.join(plots_dir, f'pos{pos}_response_matrix.pdf'), format='pdf', dpi=300)

        #plt.show()


        ### Fit with Gaussians probability distributions ###

        mean = []
        emean = []
        fwhm = []
        efwhm = []
        i = 0
        for prob_distr, err_distr, ec, em in zip(response, response_err, centersE, centers_meas):
        #for prob_distr, err_distr, ec, em in zip(number, number_err, centersE, centers_meas):
            prob_distr, err_distr, e_meas = prob_distr[prob_distr > 0], err_distr[prob_distr > 0], em[prob_distr > 0]
            prob_distr, err_distr = prob_distr/np.max(prob_distr), err_distr/np.max(prob_distr)
            #err_distr = err_distr[prob_distr > 0]
            #e_meas = em[prob_distr > 0]
            #prob_distr = prob_distr[prob_distr > 0]
            mean_exp = np.average(e_meas, weights=prob_distr)
            fwhm_exp = np.sqrt(np.average((e_meas - mean_exp)**2, weights=prob_distr)) * 2.3548
            amp_exp = np.max(prob_distr)*(fwhm_exp/np.sqrt(4*np.log(2)/np.pi))
            factors = np.linspace(0.9, 1.1, 5)
            mean_trial = [mean_exp * factor for factor in factors]
            fwhm_trial = [fwhm_exp * factor for factor in factors]
            amp_trial = [amp_exp * factor for factor in factors]
            chi2_red_trial = []
            m_list, f_list, a_list = [], [], []
            for m in mean_trial:
                for f in fwhm_trial:
                    for a in amp_trial:
                        try: 
                            popt, pcov = curve_fit(gaus, e_meas, prob_distr, sigma=err_distr, p0=[a, f, m])
                            perr = np.sqrt(np.diag(pcov))
                            residuals = prob_distr - gaus(e_meas, *popt)
                            chi_squared = np.sum((residuals / err_distr)**2)
                            num_data_points = len(e_meas)
                            num_params = len(popt)
                            degrees_of_freedom = num_data_points - num_params
                            chi_squared_red = chi_squared / degrees_of_freedom  
                            chi2_red_trial.append(chi_squared_red)
                            m_list.append(m), f_list.append(f), a_list.append(a)
                        except:
                            continue
            if len(chi2_red_trial) > 0:
                chi2_red_trial, m_list, f_list, a_list = np.array(chi2_red_trial), np.array(m_list), np.array(f_list), np.array(a_list)
                mean_best, fwhm_best, amp_best = m_list[np.argmin(chi2_red_trial)], f_list[np.argmin(chi2_red_trial)], a_list[np.argmin(chi2_red_trial)]
                sigma_mean, sigma_fwhm = np.std(m_list), np.std(f_list)
                try:
                    # Perfrom the fit
                    popt, pcov = curve_fit(gaus, e_meas, prob_distr, sigma=err_distr, p0=[amp_best, fwhm_best, mean_best])
                    perr = np.sqrt(np.diag(pcov))
                    mean.append(popt[2])
                    emean.append(perr[2]+sigma_mean)
                    fwhm.append(popt[1])
                    efwhm.append(perr[1]+sigma_fwhm)
                    energy_fit = np.linspace(1, 3e4, 100000)

                    prob = ax1.errorbar(e_meas, prob_distr, yerr=err_distr, fmt = '.', label = str(round(ec,1))+' keV')
                    ax1.plot(energy_fit, gaus(energy_fit, *popt), color = prob[0].get_color())
                    #if pos == save_pos: 
                    ax4.errorbar(e_meas, prob_distr, yerr=err_distr, fmt = '.', label = str(round(ec,1))+' keV')
                    ax4.plot(energy_fit, gaus(energy_fit, *popt), color = prob[0].get_color())
                    grid = np.zeros((NbinY, NbinX))
                    grid[int(pos / NbinX), pos % NbinX] = 1
                    ax2.imshow(grid, extent = (edgesX[0], edgesX[-1], edgesY[0], edgesY[-1]), cmap = 'seismic', origin = 'upper')
                    resol = fwhm[-1] * 100 / mean[-1]
                    resol_err = np.sqrt(100**2*((efwhm[-1]/mean[-1])**2 + (fwhm[-1]*emean[-1]/mean[-1]**2)**2))
                    ax3.errorbar(ec, resol, yerr=resol_err, fmt='.')
                    
                    #prob = plt.errorbar(e_meas, prob_distr, yerr=err_distr, fmt = '.', label = str(round(ec,1))+' keV')
                    #plt.plot(energy_fit, gaus(energy_fit, *popt), color = prob[0].get_color())
                    #plt.title("Position "+str(pos))
                    #plt.xlabel("Measured energy [keV]")
                    #plt.ylabel("Normalized distribution")
                    #plt.xlim(left=9)
                    #plt.ylim(bottom = 0)
                    #plt.xscale("log")
                    #plt.legend(loc = 'upper right')
                    #pdf2_pages.savefig()
                    #plt.close()
                    residuals = prob_distr - gaus(e_meas, *popt)
                    chi_squared = np.sum((residuals / err_distr)**2)
                    num_data_points = len(e_meas)
                    num_params = len(popt)
                    degrees_of_freedom = num_data_points - num_params
                    chi_squared_red = chi_squared / degrees_of_freedom  
                    chi2_red.append(chi_squared_red)
                    '''
                    if chi_squared_red > 2 and y_min < y_min_sym:
                        print(f"Chi-squared of {chi_squared_red} for pos {pos} and energy {ec} keV")
                        plt.figure()
                        prob = plt.errorbar(e_meas, prob_distr, yerr=err_distr, fmt = '.', label = str(ec)+' keV')
                        plt.plot(energy_fit, gaus(energy_fit, *popt), color = prob[0].get_color(), lw = 0.7)
                        plt.xlabel("Measured energy [keV]"); plt.ylabel("Normalized distribution")
                        plt.ylim(bottom=0)
                        pdf_pages.savefig()
                        plt.close()
                    '''
                except RuntimeError as e:
                    mean.append(-9999)
                    emean.append(-9999)
                    fwhm.append(-9999)
                    efwhm.append(-9999)
                    chi2_red.append(-9999)
                    print(f"%%% WARNING: fit not found for pos {pos} and energy {ec}.")
                except TypeError as e:
                    mean.append(-9999)
                    emean.append(-9999)
                    fwhm.append(-9999)
                    efwhm.append(-9999)
                    chi2_red.append(-9999)
                    print(f"%%% WARNING: not enough data points (< 3) for pos {pos} and energy {ec}.")
            else:
                mean.append(-9999)
                emean.append(-9999)
                fwhm.append(-9999)
                efwhm.append(-9999)
                chi2_red.append(-9999)
                print(f"%%% WARNING: fit not found for pos {pos} and energy {ec}.")
            i += 1
            

        mean = np.array(mean)
        emean = np.array(emean)
        fwhm = np.array(fwhm)
        efwhm = np.array(efwhm)

        # Convert from BoGEMMS posID to MEGAlib voxels
        megalib_voxel = np.zeros(3)
        if not swap_x: megalib_voxel[index_x] = pos % NbinX
        else: megalib_voxel[index_x] = NbinX - 1 - pos % NbinX
        megalib_voxel[index_y] = int(pos / NbinX)
        megalib_voxel[index_z] = 0


        file_output = f'./response_{name}.dat'
        if pos == 0: mod = "w"
        else: mod = "a"
        with open(file_output, mod) as f:
            if pos == 0: f.write("# posID voxel_X voxel_Y voxel_Z x[mm] y[mm] E_true[keV] E_meas[keV] FWHM[keV] Emeas_err[keV] FWHM_err[keV]\n")
            for i in range(len(centersE)):
                f.write('{:30.0f}'.format(pos))
                f.write('{:30.0f}'.format(megalib_voxel[0]))
                f.write('{:30.0f}'.format(megalib_voxel[1]))
                f.write('{:30.0f}'.format(megalib_voxel[2]))
                f.write('{:30.0f}'.format(cX))
                f.write('{:30.0f}'.format(cY))
                f.write('{:30.2f}'.format(centersE[i]))
                f.write('{:30.2f}'.format(abs(mean[i])))
                f.write('{:30.2f}'.format(abs(fwhm[i])))
                f.write('{:30.2f}'.format(abs(emean[i])))
                f.write('{:30.2f}'.format(abs(efwhm[i])))

                f.write('\n')

        ax1.set_title("Position "+str(pos))
        ax1.set_xlabel("Measured energy [keV]")
        ax1.set_ylabel("Normalized distribution")
        ax1.set_xlim([10, 1.5e4])
        ax1.set_ylim([0, 1.4])
        ax1.set_xscale("log")
        ax1.legend(loc = 'upper center', ncol = len(centersE), fontsize=9)

        ax2.set_xlabel("x[mm]")
        ax2.set_ylabel("y[mm]")

        ax3.set_title("Position "+str(pos))
        ax3.set_xlabel("True energy [keV]")
        ax3.set_ylabel("Resolution (%)")
        ax3.set_xlim(10, 1e4)
        ax3.set_ylim([0, 200])
        ax3.set_xscale("log")
        #ax3.legend(loc = 'upper right')

        #if pos == save_pos:
        ax4.set_title("Position "+str(pos), fontsize=20)
        ax4.set_xlabel("Measured energy [keV]", fontsize=20)
        ax4.set_ylabel("Normalized distribution", fontsize=20)
        ax4.set_xlim([10, 1.5e4])
        ax4.set_ylim([0, 1.4])
        ax4.set_xscale("log")
        ax4.legend(loc = 'upper center', ncol = len(centersE), fontsize=16)
        ax4.tick_params(axis='both', which='major', labelsize=15)
        ax4.tick_params(axis='both', which='minor', labelsize=15)
        fig4.tight_layout()
        #fig4.savefig('Gaussians_pos'+str(pos)+'.pdf')

        fig1.tight_layout()
        pdf1.savefig(fig1)
        pdf2.savefig(fig5)
        pdf3.savefig(fig4)

        plt.close(fig1)
        plt.close(fig5)
        plt.close(fig4)

        pos += 1

pdf1.close()
pdf2.close()
pdf3.close()

# Write also for depth voxels
posID, vx, vy, vz, x, y, Etrue, Emeas, fwhm, Emeas_err, fwhm_err = np.genfromtxt(file_output, unpack=True)
last_posID = posID[-1]
for j in range(NbinZ - 1):
    if index_z == 0: vx = vx + 1
    elif index_z == 1: vy = vy + 1
    else: vz = vz + 1
    posID_new = posID + last_posID + 1
    with open(file_output, 'a') as f:
        for i in range(len(posID)):
            f.write('{:30.0f}'.format(posID_new[i]))
            f.write('{:30.0f}'.format(vx[i]))
            f.write('{:30.0f}'.format(vy[i]))
            f.write('{:30.0f}'.format(vz[i]))
            f.write('{:30.0f}'.format(x[i]))
            f.write('{:30.0f}'.format(y[i]))
            f.write('{:30.2f}'.format(Etrue[i]))
            f.write('{:30.2f}'.format(Emeas[i]))
            f.write('{:30.2f}'.format(fwhm[i]))
            f.write('{:30.2f}'.format(Emeas_err[i]))
            f.write('{:30.2f}'.format(fwhm_err[i]))

            f.write('\n')
    last_posID = posID_new[-1]


##########################################
### Fit centroids and resolutions ###
##########################################

fig1 = plt.figure(1, figsize=(10,7))
ax1 = fig1.add_subplot(111)

fig2 = plt.figure(2, figsize=(10,7))
ax2 = fig2.add_subplot(111)

posID, vx, vy, vz, x, y, Etrue, Emeas, fwhm, Emeas_err, fwhm_err = np.genfromtxt(file_output, unpack=True)

def lin(x, m, q):
    return m * x + q

def w_func(x, a, b, c):
    return np.sqrt(a**2 + b**2*x + c**2*x**2)

colors = plt.cm.viridis(np.linspace(0, 1, NbinX*NbinY))
norm = Normalize(vmin=0, vmax=NbinX*NbinY)

if index_z == 0: cond = vx == 0
elif index_z == 1: cond = vy == 0
else: cond = vz == 0
posID, vx, vy, vz, x, y, Etrue, Emeas, fwhm, Emeas_err, fwhm_err = posID[cond], vx[cond], vy[cond], vz[cond], x[cond], y[cond], Etrue[cond], Emeas[cond], fwhm[cond], Emeas_err[cond], fwhm_err[cond]

i = 0
m_array = []
q_array = []
a_array = []
b_array = []
c_array = []
posID_pos, vx_pos, vy_pos, vz_pos = [], [], [], []
for y_min, y_max, cY in zip(edgesY[:-1], edgesY[1:], centersY):
    for x_min, x_max, cX in zip(edgesX[:-1], edgesX[1:], centersX):
        cond_pos = (x_min < x) & (x < x_max) & (y_min < y) & (y < y_max)
        posID_pos.append(np.unique(posID[cond_pos])[0])
        vx_pos.append(np.unique(vx[cond_pos])[0])
        vy_pos.append(np.unique(vy[cond_pos])[0])
        vz_pos.append(np.unique(vz[cond_pos])[0])
        Etrue_pos = Etrue[cond_pos]
        Emeas_pos = Emeas[cond_pos]
        Emeas_err_pos = Emeas_err[cond_pos]
        fwhm_pos = fwhm[cond_pos]
        fwhm_err_pos = fwhm_err[cond_pos]
        
        popt, pcov = curve_fit(lin, Etrue_pos, Emeas_pos, sigma=Emeas_err_pos, p0=[1, 0])
        m_array.append(popt[0])
        q_array.append(popt[1])

        x_err = (edgesE[1:] - edgesE[:-1])/2
        centroids = ax1.errorbar(Etrue_pos, Emeas_pos, xerr=x_err, yerr=Emeas_err_pos, fmt='.', color=colors[i], alpha=0.2)
        energy_fit = np.arange(10, edgesE[-1], 0.1)
        ax1.plot(energy_fit, lin(energy_fit, *popt), lw=0.7, color=centroids[0].get_color())

        popt, pcov = curve_fit(w_func, Etrue_pos, fwhm_pos, sigma=fwhm_err_pos, p0=[20, 3, 0.01])
        a_array.append(popt[0])
        b_array.append(popt[1])
        c_array.append(popt[2])

        resol = fwhm_pos*100/Emeas_pos
        resol_err = np.sqrt(100**2*((fwhm_err_pos/Emeas_pos)**2 + (fwhm_pos*Emeas_err_pos/Emeas_pos**2)**2))
        x_err = (edgesE[1:] - edgesE[:-1])/2
        resolution = ax2.errorbar(Etrue_pos, resol, xerr=x_err, yerr=resol_err, fmt='.', color=colors[i], alpha=0.2)
        energy_fit = np.arange(10, edgesE[-1], 0.1)
        ax2.plot(energy_fit, w_func(energy_fit, *popt)*100/energy_fit, lw=0.7, color=resolution[0].get_color())

        i += 1

file_fit = f'correction_file_{name}.dat'
with open(file_fit, 'w') as f:
    f.write("# posID voxel_X voxel_Y voxel_Z x[mm] y[mm] m q a b c\n")
    i = 0
    for cY in centersY:
        for cX in centersX:
            f.write('{:30.0f}'.format(posID_pos[i]))
            f.write('{:30.0f}'.format(vx_pos[i]))
            f.write('{:30.0f}'.format(vy_pos[i]))
            f.write('{:30.0f}'.format(vz_pos[i]))
            f.write('{:30.2f}'.format(cX))
            f.write('{:30.2f}'.format(cY))
            f.write('{:30.2f}'.format(m_array[i]))
            f.write('{:30.2f}'.format(q_array[i]))
            f.write('{:30.2f}'.format(abs(a_array[i])))
            f.write('{:30.2f}'.format(abs(b_array[i])))
            f.write('{:30.2f}'.format(abs(c_array[i])))

            f.write('\n')
            i += 1

# Write also for depth voxels
posID, vx, vy, vz, x, y, m, q, a, b, c = np.genfromtxt(file_fit, unpack=True)
last_posID = posID[-1]
for j in range(NbinZ - 1):
    if index_z == 0: vx = vx + 1
    elif index_z == 1: vy = vy + 1
    else: vz = vz + 1
    posID_new = posID + last_posID + 1
    with open(file_fit, 'a') as f:
        for i in range(len(posID)):
            f.write('{:30.0f}'.format(posID_new[i]))
            f.write('{:30.0f}'.format(vx[i]))
            f.write('{:30.0f}'.format(vy[i]))
            f.write('{:30.0f}'.format(vz[i]))
            f.write('{:30.2f}'.format(x[i]))
            f.write('{:30.2f}'.format(y[i]))
            f.write('{:30.2f}'.format(m[i]))
            f.write('{:30.2f}'.format(q[i]))
            f.write('{:30.2f}'.format(a[i]))
            f.write('{:30.2f}'.format(b[i]))
            f.write('{:30.2f}'.format(c[i]))

            f.write('\n')
    last_posID = posID_new[-1]

sm1 = ScalarMappable(cmap='viridis', norm=norm)
sm1.set_array([])
cbar1 = plt.colorbar(sm1, ax=ax1, orientation='vertical')
cbar1.set_label('Spatial bins', fontsize=20)

sm2 = ScalarMappable(cmap='viridis', norm=norm)
sm2.set_array([])
cbar6 = plt.colorbar(sm2, ax=ax2, orientation='vertical')
cbar6.set_label('Spatial bins', fontsize=20)

ax1.set_xlabel(r"$E_{true}$ [keV]", fontsize=20)
ax1.set_ylabel(r"$E_{meas}$ [keV]", fontsize=20)
ax1.set_ylim([10, 12000])
ax1.set_xscale("log")
ax1.set_yscale("log")
fig1.tight_layout()
fig1.savefig(os.path.join(plots_dir, 'centroids_fit.pdf'))

ax2.set_xlabel(r"$E_{true}$ [keV]", fontsize=20)
ax2.set_ylabel(r"Resolution (%)", fontsize=20)
ax2.set_ylim([0, 100])
ax2.set_xscale("log")
fig2.tight_layout()
#ax2.set_yscale("log")
fig2.savefig(os.path.join(plots_dir, 'resolutions_fit.pdf'))