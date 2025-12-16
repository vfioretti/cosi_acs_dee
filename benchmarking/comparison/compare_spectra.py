import os
import json
import sys
import matplotlib.pyplot as plt
from matplotlib import gridspec
from scipy.optimize import curve_fit

from functions import *
from models import *

# Import the input parameters
arg_list = sys.argv
json_file = arg_list[1]

# Load the JSON file with the information about the sources
with open(json_file, 'r') as f:
    data = json.load(f)

gs = gridspec.GridSpec(5, 1)
gs.update(hspace = 0.)

fig1 = plt.figure(1, figsize=(10,7))
ax1 = plt.Subplot(fig1, gs[0:4, 0:1])
fig1.add_subplot(ax1)
ax11 = plt.Subplot(fig1, gs[4:5, 0:1])
fig1.add_subplot(ax11)

plots_dir = 'plots/'
if not os.path.exists(plots_dir): os.mkdir(plots_dir)

energy, resolution, resolution_err, dataset_name = [], [], [], []
with open('sim_resolution.dat', 'r') as f:
    for line in f:
        line = line.strip()
        if not line.startswith('#'):
            columns = line.split()
            energy.append(float(columns[0]))
            resolution.append(float(columns[1]))
            resolution_err.append(float(columns[2]))
            dataset_name.append(columns[3])
        else:
            columns = line.split()
            if line.startswith('# a = '):
                a = float(columns[3])
                a_err = float(columns[5])
            elif line.startswith('# b = '):
                b = float(columns[3])
                b_err = float(columns[5])
            elif line.startswith('# c = '):
                c = float(columns[3])
                c_err = float(columns[5])

energy, resolution, resolution_err, dataset_name = np.array(energy), np.array(resolution), np.array(resolution_err), np.array(dataset_name)
opt_params_sim = [a, b, c]
opt_params_sim_err = [a_err, b_err, c_err]

ax1.errorbar(energy, resolution, yerr=resolution_err, fmt='o', markersize=6, zorder=2, color='r', label=f'Simulation')

energy_fit = np.linspace(1, 2000, 10000)
ax1.plot(energy_fit, w_func(energy_fit, *opt_params_sim)*100/energy_fit, color="r", lw=1.2, zorder=0, label="Best fit (sim)")

energy, resolution, resolution_err, dataset_name = [], [], [], []
with open('exp_resolution.dat', 'r') as f:
    for line in f:
        line = line.strip()
        if not line.startswith('#'):
            columns = line.split()
            energy.append(float(columns[0]))
            resolution.append(float(columns[1]))
            resolution_err.append(float(columns[2]))
            dataset_name.append(columns[3])
        else:
            columns = line.split()
            if line.startswith('# a = '):
                a = float(columns[3])
                a_err = float(columns[5])
            elif line.startswith('# b = '):
                b = float(columns[3])
                b_err = float(columns[5])
            elif line.startswith('# c = '):
                c = float(columns[3])
                c_err = float(columns[5])


energy, resolution, resolution_err, dataset_name = np.array(energy), np.array(resolution), np.array(resolution_err), np.array(dataset_name)
opt_params_lab = [a, b, c]
opt_params_lab_err = [a_err, b_err, c_err]

ax1.errorbar(energy, resolution, yerr=resolution_err, fmt='o', markersize=6, zorder=2, color='k', label=f'Experiment')

energy_fit = np.linspace(1, 2000, 10000)
ax1.plot(energy_fit, w_func(energy_fit, *opt_params_lab)*100/energy_fit, color="k", lw=1.2, zorder=0, label="Best fit (exp)")

fit_sim, fit_lab = w_func(energy_fit, *opt_params_sim), w_func(energy_fit, *opt_params_lab)
fit_sim_err, fit_lab_err = w_func(energy_fit, *opt_params_sim_err), w_func(energy_fit, *opt_params_lab_err)
res = (fit_sim - fit_lab)*100/fit_lab
res_err = np.sqrt((fit_sim_err / fit_lab * 100) ** 2 + ((fit_sim - fit_lab) / fit_lab ** 2 * fit_lab_err * 100) ** 2)
ax11.plot(energy_fit, res, color='k')
ax11.fill_between(energy_fit, res - res_err, res + res_err, color='k', alpha=0.2)
ax11.errorbar([0, 2000], [0, 0], fmt = 'k--')

ax1.set_title('With noise', fontsize=20)
ax11.set_xlabel("Energy [keV]", fontsize=20)
ax11.set_ylabel("Residuals (%)", fontsize=15)
ax1.set_ylabel("Resolution (%)", fontsize=20)
ax1.set_xlim([0, 2000])
ax1.set_ylim([0, 70])
ax11.set_xlim([0, 2000])
ax11.set_ylim([-50, 50])
#ax1.set_xscale("log")
#ax1.set_yscale("log")
ax1.set_xticklabels([])
ax11.set_yticks([-20, 20])
ax1.legend(loc="upper right", fontsize=20)
ax1.grid(which="minor", alpha=0.5)
ax1.grid(which="major", alpha=0.5)
ax11.grid(which="minor", alpha=0.5)
ax11.grid(which="major", alpha=0.5)
ax1.tick_params(axis='both', which='major', labelsize=15)
ax1.tick_params(axis='both', which='minor', labelsize=15)
ax11.tick_params(axis='both', which='major', labelsize=15)
ax11.tick_params(axis='both', which='minor', labelsize=15)
fig1.tight_layout()
fig1.savefig(os.path.join(plots_dir, 'resolution_SSL_NRL.pdf'))

plt.show()



#####################################
### Compare energy spectra ###
#####################################

sigmas, sigmas_photopeaks, sigmas_compton = [], [], []
accuracy, accuracy_photopeaks, accuracy_compton = [], [], []
datasets = data['uncollimated']
for dataset in datasets:
    path, name, sources = dataset['path_lab'], dataset['name'], dataset['sources']
    lab_dir = "Spectrums_lab_"+name
    sim_dir = "Spectrums_sim_"+name

    for source in sources:
        # Read files
        source_name = source['source_name']
        bkg_spectrums = source['file_bkg']
        file_source_sim = os.path.join(sim_dir, 'data_'+source_name+'.dat')
        file_source_lab = os.path.join(lab_dir, 'data_'+source_name+'.dat')
        energy_sim, counts_sim, counts_sim_err = np.genfromtxt(file_source_sim, usecols=(0,1,2), unpack=True)
        energy_lab, counts_lab, energy_lab_err = np.genfromtxt(file_source_lab, usecols=(0,1,2), unpack=True)

        # Rebin lab and bkg data
        energy = energy_sim
        counts_lab_rebin = rebin_counts(energy_lab, counts_lab, energy)
        counts_lab_top = rebin_counts(energy_lab+energy_lab_err, counts_lab, energy)
        counts_lab_bot = rebin_counts(energy_lab-energy_lab_err, counts_lab, energy)
        counts_lab = counts_lab_rebin
        counts_lab_sys = abs(counts_lab_top - counts_lab_bot)/2
        counts_lab_err = np.sqrt((np.sqrt(counts_lab))**2 + (counts_lab_sys)**2)
        
        counts_bkgs, counts_bkgs_top, counts_bkgs_bot = [], [], []
        for bkg_spectrum in bkg_spectrums:
            file_bkg = os.path.join(lab_dir, bkg_spectrum)
            energy_bkg, counts_bkg, energy_bkg_err = np.genfromtxt(file_bkg, usecols=(0,1,2), unpack=True)
            counts_bkg_rebin = rebin_counts(energy_bkg, counts_bkg, energy)
            counts_bkg_top = rebin_counts(energy_bkg+energy_bkg_err, counts_bkg, energy)
            counts_bkg_bot = rebin_counts(energy_bkg-energy_bkg_err, counts_bkg, energy)
            counts_bkgs.append(counts_bkg_rebin)
            counts_bkgs_top.append(counts_bkg_top)
            counts_bkgs_bot.append(counts_bkg_bot)
        counts_bkgs, counts_bkgs_top, counts_bkgs_bot = np.stack(counts_bkgs), np.stack(counts_bkgs_top), np.stack(counts_bkgs_bot)
        counts_bkg, counts_bkg_top, counts_bkg_bot = np.average(counts_bkgs, axis=0), np.average(counts_bkgs_top, axis=0), np.average(counts_bkgs_bot, axis=0)
        counts_bkg_stat = np.average(np.sqrt(counts_bkgs))
        counts_sys_temp = np.max(counts_bkgs, axis=0)-np.min(counts_bkgs, axis=0)
        counts_sys_cal = np.abs(counts_bkg_top - counts_bkg_bot)/2
        counts_bkg_err = np.sqrt((counts_bkg_stat)**2 + (counts_sys_temp)**2 + (counts_sys_cal)**2) # statistical + systematic (temp. variations) + systematic (energy calibration)

        # Subtract background
        counts_lab_notsub = counts_lab
        counts_lab = counts_lab - counts_bkg
        counts_lab[counts_lab < 0] = 0
        counts_lab_err = np.sqrt((counts_lab_err)**2 + (counts_bkg_err)**2)
        #sys_cont = np.nanmean((counts_lab_err - np.sqrt((np.sqrt(counts_lab_notsub))**2 + (np.sqrt(counts_bkg))**2))/ counts_lab_err)
        #print(f"SYSYEMATIC: {sys_cont}")

        # Normalize sim and lab spectra
        lines = source['lines']
        first_line = lines[0]
        energy_line = first_line['energy']
        data_sim = np.genfromtxt('sim_resolution.dat', dtype=[('energy_resol_sim', float), ('resolution_sim', float), ('resolution_sim_err', float), ('dataset_name', 'U20')],  usecols=(0, 1, 2, 3))
        data_lab = np.genfromtxt('exp_resolution.dat', dtype=[('energy_resol_lab', float), ('resolution_lab', float), ('resolution_lab_err', float), ('dataset_name', 'U20')],  usecols=(0, 1, 2, 3))
        energy_resol_sim, resolution_sim, resolution_sim_err, dataset_name = data_sim['energy_resol_sim'], data_sim['resolution_sim'], data_sim['resolution_sim_err'], data_sim['dataset_name']
        energy_resol_lab, resolution_lab, resolution_lab_err, dataset_name_lab = data_lab['energy_resol_lab'], data_lab['resolution_lab'], data_lab['resolution_lab_err'], data_lab['dataset_name']
        energy_resol_sim_sel = energy_resol_sim[(energy_resol_sim == energy_line) & (dataset_name == name)][0]
        resolution_sim_sel = resolution_sim[(energy_resol_sim == energy_line) & (dataset_name == name)][0]
        energy_resol_lab_sel = energy_resol_lab[(energy_resol_lab == energy_line) & (dataset_name == name)][0]
        resolution_lab_sel = resolution_lab[(energy_resol_lab == energy_line) & (dataset_name == name)][0]
        fwhm_sim = resolution_sim_sel * energy_resol_sim_sel / 100
        fwhm_lab = resolution_lab_sel * energy_resol_lab_sel / 100
        if energy_line == 60:
            fwhm_sim = fwhm_sim / 2
            fwhm_lab = fwhm_lab / 2
        ene_bot_sim, ene_top_sim = energy_line - fwhm_sim/2, energy_line + fwhm_sim/2
        ene_bot_lab, ene_top_lab = energy_line - fwhm_lab/2, energy_line + fwhm_lab/2
        counts_sim_fwhm = np.sum(counts_sim[(ene_bot_sim < energy) & (energy < ene_top_sim)])
        counts_lab_fwhm = np.sum(counts_lab[(ene_bot_lab < energy) & (energy < ene_top_lab)])
        bin_width = energy[1] - energy[0]

        counts_sim = counts_sim / counts_sim_fwhm / bin_width
        counts_sim_err = counts_sim_err / counts_sim_fwhm / bin_width
        counts_lab = counts_lab / counts_lab_fwhm / bin_width
        counts_lab_err = counts_lab_err / counts_lab_fwhm / bin_width

        # Compare sim and lab spectra
        fig1 = plt.figure(1, figsize=(10,7))
        ax1 = plt.Subplot(fig1, gs[0:4, 0:1])
        fig1.add_subplot(ax1)
        ax11 = plt.Subplot(fig1, gs[4:5, 0:1])
        fig1.add_subplot(ax11)
        
        if name == "NRL":
            if source_name == "Am241": energy_left, energy_right, bottom_y = 50, 90, 2e-3
            if source_name == "Na22": energy_left, energy_right, bottom_y = 60, 1400, 2e-5
            if source_name == "Cs137": energy_left, energy_right, bottom_y = 60, 800, 8e-6
        if name == "SSL":
            if source_name == "Am241": energy_left, energy_right, bottom_y = 50, 90, 2e-3
            if source_name == "Cd109": energy_left, energy_right, bottom_y = 60, 130, 3e-4
            if source_name == "Co57": energy_left, energy_right, bottom_y = 60, 180, 2e-5
            if source_name == "Ba133": energy_left, energy_right, bottom_y = 60, 450, 2e-5
            if source_name == "Na22": energy_left, energy_right, bottom_y = 60, 1400, 8e-6
            if source_name == "Cs137": energy_left, energy_right, bottom_y = 60, 800, 8e-6
            if source_name == "Co60": energy_left, energy_right, bottom_y = 60, 1500, 2e-4
            if source_name == "Y88": energy_left, energy_right, bottom_y = 60, 2000, 2e-4     

        energy, counts_sim, counts_lab, counts_sim_err, counts_lab_err = energy[(counts_lab > 0) & (counts_sim > 0)], counts_sim[(counts_lab > 0) & (counts_sim > 0)], counts_lab[(counts_lab > 0) & (counts_sim > 0)], counts_sim_err[(counts_lab > 0) & (counts_sim > 0)], counts_lab_err[(counts_lab > 0) & (counts_sim > 0)]

        ax1.errorbar(energy, counts_lab, xerr=bin_width/2, yerr=counts_lab_err, fmt=".", color="k", zorder=1, label="Experiment")
        ax1.errorbar(energy, counts_sim, xerr=bin_width/2, yerr=counts_sim_err, fmt=".", color="red", zorder=2, label="Simulation")
        
        residuals = (counts_sim-counts_lab)*100/counts_lab
        residuals_err = (counts_sim_err + counts_lab_err)*100/counts_lab + abs((counts_sim-counts_lab)*counts_lab_err*100/counts_lab**2)
        ax11.errorbar(energy, residuals, xerr=bin_width/2, yerr=residuals_err, fmt=".", color="red")
        ax11.errorbar([energy_left, energy_right], [0, 0], fmt = 'k--')

        #sigma = np.abs(counts_sim - counts_lab) / np.sqrt(counts_sim_err**2 + counts_lab_err**2)
        #ax11.errorbar(energy, sigma, xerr=bin_width/2, yerr=0, fmt=".", color="red")
        #ax11.errorbar([energy_left, energy_right], [0, 0], fmt = 'k--')

        # Write to file
        binned_dir_sim = f'binned_spectrums/sim/{name}/'
        if not os.path.exists(binned_dir_sim): os.makedirs(binned_dir_sim)
        filename = os.path.join(binned_dir_sim, 'data_'+source_name+'.dat')
        with open(filename, 'w') as f:
            f.write('# Energy[keV] Counts Counts_err\n')
            for e, c, ec in zip(energy, counts_sim, counts_sim_err):
                f.write('{0:<30.0f}'.format(e))
                f.write('{0:<30.5f}'.format(c))
                f.write('{0:<30.5f}'.format(ec))
                f.write('\n')
        binned_dir_lab = f'binned_spectrums/lab/{name}/'
        if not os.path.exists(binned_dir_lab): os.makedirs(binned_dir_lab)
        filename = os.path.join(binned_dir_lab, 'data_'+source_name+'.dat')
        with open(filename, 'w') as f:
            f.write('# Energy[keV] Counts Counts_err\n')
            for e, c, ec in zip(energy, counts_lab, counts_lab_err):
                f.write('{0:<30.0f}'.format(e))
                f.write('{0:<30.5f}'.format(c))
                f.write('{0:<30.5f}'.format(ec))
                f.write('\n')

        # Calculate the sigmas and average accuracy
        counts_sim = counts_sim[(energy_left < energy) & (energy < energy_right)]
        counts_lab = counts_lab[(energy_left < energy) & (energy < energy_right)]
        counts_sim_err = counts_sim_err[(energy_left < energy) & (energy < energy_right)]
        counts_lab_err = counts_lab_err[(energy_left < energy) & (energy < energy_right)]
        energy = energy[(energy_left < energy) & (energy < energy_right)]
        sigma = np.abs(counts_sim - counts_lab) / np.sqrt(counts_sim_err**2 + counts_lab_err**2)
        sigmas.append(sigma)
        residuals = abs((counts_sim-counts_lab)*100/counts_lab)
        accuracy.append(np.mean(residuals[np.isfinite(residuals)]))
        lines = source['lines']
        for line in lines:
            energy_line = line['energy']
            data_sim = np.genfromtxt('sim_resolution.dat', dtype=[('energy_resol_sim', float), ('resolution_sim', float), ('resolution_sim_err', float), ('dataset_name', 'U20')],  usecols=(0, 1, 2, 3))
            data_lab = np.genfromtxt('exp_resolution.dat', dtype=[('energy_resol_lab', float), ('resolution_lab', float), ('resolution_lab_err', float), ('dataset_name', 'U20')],  usecols=(0, 1, 2, 3))
            energy_resol_sim, resolution_sim, resolution_sim_err, dataset_name = data_sim['energy_resol_sim'], data_sim['resolution_sim'], data_sim['resolution_sim_err'], data_sim['dataset_name']
            energy_resol_lab, resolution_lab, resolution_lab_err, dataset_name_lab = data_lab['energy_resol_lab'], data_lab['resolution_lab'], data_lab['resolution_lab_err'], data_lab['dataset_name']
            energy_resol_sim_sel = energy_resol_sim[(energy_resol_sim == energy_line) & (dataset_name == name)][0]
            resolution_sim_sel = resolution_sim[(energy_resol_sim == energy_line) & (dataset_name == name)][0]
            energy_resol_lab_sel = energy_resol_lab[(energy_resol_lab == energy_line) & (dataset_name == name)][0]
            resolution_lab_sel = resolution_lab[(energy_resol_lab == energy_line) & (dataset_name == name)][0]
            fwhm_sim = resolution_sim_sel * energy_resol_sim_sel / 100
            fwhm_lab = resolution_lab_sel * energy_resol_lab_sel / 100
            sigma_sim, sigma_lab = fwhm_sim / (2 * np.sqrt(2 * np.log(2))), fwhm_lab / (2 * np.sqrt(2 * np.log(2)))
            ene_bot_sim, ene_top_sim = energy_line - 2*sigma_sim, energy_line + 2*sigma_sim
            ene_bot_lab, ene_top_lab = energy_line - 2*sigma_lab, energy_line + 2*sigma_lab
            counts_sim_photo = counts_sim[(ene_bot_sim < energy) & (energy < ene_top_sim)]
            counts_lab_photo = counts_lab[(ene_bot_sim < energy) & (energy < ene_top_sim)]
            counts_sim_compton = counts_sim[(ene_bot_sim > energy) | (energy > ene_top_sim)]
            counts_lab_compton = counts_lab[(ene_bot_sim > energy) | (energy > ene_top_sim)]
            counts_sim_err_photo = counts_sim_err[(ene_bot_sim < energy) & (energy < ene_top_sim)]
            counts_lab_err_photo = counts_lab_err[(ene_bot_sim < energy) & (energy < ene_top_sim)]
            counts_sim_err_compton = counts_sim_err[(ene_bot_sim > energy) | (energy > ene_top_sim)]
            counts_lab_err_compton = counts_lab_err[(ene_bot_sim > energy) | (energy > ene_top_sim)]
            sigma = np.abs(counts_sim_photo - counts_lab_photo) / np.sqrt(counts_sim_err_photo**2 + counts_lab_err_photo**2)
            sigmas_photopeaks.append(sigma)
            sigma = np.abs(counts_sim_photo - counts_lab_photo) / np.sqrt(counts_sim_err_photo**2 + counts_lab_err_photo**2)
            sigmas_compton.append(sigma)
            residuals = abs((counts_sim_photo-counts_lab_photo)*100/counts_lab_photo)
            accuracy_photopeaks.append(np.mean(residuals[np.isfinite(residuals)]))
            residuals = abs((counts_sim_compton-counts_lab_compton)*100/counts_lab_compton)
            accuracy_compton.append(np.mean(residuals[np.isfinite(residuals)]))

        ax1.set_title(f'{source_name} ({name})', fontsize=20)
        ax11.set_xlabel("Energy [keV]", fontsize=20)
        ax1.set_ylabel("Normalized counts / keV", fontsize=20)
        ax11.set_ylabel("Residuals (%)", fontsize=20)
        ax1.set_xlim([energy_left, energy_right])
        ax11.set_xlim([energy_left, energy_right])
        ax1.set_ylim(bottom=bottom_y)
        ax11.set_ylim([-130, 130])
        ax11.set_yticks([-100, 0, 100])
        #ax11.set_ylim([0, 5])
        #ax11.set_yticks([1, 2, 3])
        ax1.set_xticklabels([])
        #ax1.set_xscale("log")
        ax1.set_yscale("log")
        ax1.legend(loc='lower left', fontsize=20)
        ax1.grid(which="minor", alpha=0.5)
        ax1.grid(which="major", alpha=0.5)
        ax11.grid(which="minor", alpha=0.5)
        ax11.grid(which="major", alpha=0.5)
        ax1.tick_params(axis='both', which='major', labelsize=15)
        ax1.tick_params(axis='both', which='minor', labelsize=15)
        ax11.tick_params(axis='both', which='major', labelsize=15)
        ax11.tick_params(axis='both', which='minor', labelsize=15)
        fig1.tight_layout()
        fig1.savefig(os.path.join(plots_dir, source_name+'_comp_'+name+'.pdf'))

        plt.show()

print(f'AVERAGE ACCURACY (TOTAL): {np.mean(accuracy)}')
print(f'AVERAGE ACCURACY (PHOTOPEAKS): {np.mean(accuracy_photopeaks)}')
print(f'AVERAGE ACCURACY (COMPTON): {np.mean(accuracy_compton)}')