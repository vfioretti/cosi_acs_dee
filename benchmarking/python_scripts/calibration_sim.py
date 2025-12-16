import os
import json
import sys
import numpy as np
import matplotlib.pyplot as plt
from matplotlib import gridspec
from scipy.optimize import curve_fit

from functions import *
from models import *

# Import the input parameters
arg_list = sys.argv
json_file = arg_list[1]
if len(arg_list) == 3:
    electron_resolution = float(arg_list[2])
    electron_resolution_manual = True
else:
    electron_resolution_manual = False

# Load the JSON file with the information about the sources
with open(json_file, 'r') as f:
    data = json.load(f)

# Setup the plots directory
plots_dir = './calibration_sim_plots/'
if not os.path.exists(plots_dir): os.mkdir(plots_dir)

energy_resol, resolution, resolution_err, dataset_resol = [], [], [], []
datasets = data['uncollimated']
# Loop over the datasets
for dataset in datasets:
    xc, exc, energy_calibration = [], [], []
    # Read the dataset information
    path, name, sources = dataset['path_lab'], dataset['name'], dataset['sources']
    # Loop over the sources
    for source in sources:
        source_name, file_sim, bin_width, lines = source['source_name'], source['file_sim'], source['bin_width'], source['lines']

        # Read energy deposit and number of detected photons for the corresponding source
        Edep_scint, Nabs = np.genfromtxt(file_sim, usecols = (1, 2), unpack = True)
        
        # Plot histograms with all events
        fig1 = plt.figure(1, figsize=(10,7))
        ax1 = fig1.add_subplot(111)

        edges = np.arange(np.min(Nabs), np.max(Nabs)+bin_width, bin_width)
        hist, bin_edges = np.histogram(Nabs, bins = edges)
        hist_err = np.sqrt(hist)
        bin_centers = 0.5 * (bin_edges[1:] + bin_edges[:-1])
        ax1.errorbar(bin_centers, hist, yerr= hist_err, fmt='.')

        ax1.set_title(f"{source_name} spectrum", fontsize = 15)
        ax1.set_xlabel("Number of photons detected by SiPMs", fontsize = 15)
        ax1.set_ylabel("Counts", fontsize = 15)
        ax1.set_ylim(bottom=0)
        ax1.legend(fontsize = 15)
        fig1.tight_layout()
        #fig1.savefig(os.path.join(plots_dir, source_name + '_All.pdf'))

        #plt.show()

        # Loop over emission lines
        for line in lines:
        
            # Read the energy of the line emitted by the source
            energy = line['energy']

            # Plot histograms with only photopeaks
            fig2 = plt.figure(2, figsize=(10,7))
            ax2 = fig2.add_subplot(111)

            Nabs_photo = Nabs[np.isclose(Edep_scint, energy, rtol = 1e-2, atol=1)]
            edges = np.arange(np.min(Nabs_photo), np.max(Nabs_photo)+bin_width, bin_width)
            hist, bin_edges = np.histogram(Nabs_photo, bins = edges)
            hist_err = np.sqrt(hist)
            bin_centers = 0.5 * (bin_edges[1:] + bin_edges[:-1])
            ax2.errorbar(bin_centers, hist, yerr= hist_err, fmt='.',label=str(energy)+" keV")

            ax2.set_title(f"{source_name} spectrum ({energy} keV photopeak)", fontsize = 15)
            ax2.set_xlabel("Number of photons detected by SiPMs", fontsize = 15)
            ax2.set_ylabel("Counts", fontsize = 15)
            ax2.set_ylim(bottom=0)
            ax2.legend(fontsize = 15)
            fig2.tight_layout()
            #fig2.savefig(os.path.join(plots_dir, source_name + "_" + str(ene) + "_Photopeaks.pdf"))

            #plt.show()

            # Save bins with counts > 0
            bins, counts, counts_err = bin_centers[hist > 0], hist[hist > 0], hist_err[hist > 0]
            # Normalize
            counts_err = counts_err / np.sum(counts)
            counts = counts / np.sum(counts)
            # Fit with Gaussian
            mean_exp = np.average(bins, weights=counts)
            fwhm_exp = np.sqrt(np.average((bins - mean_exp)**2, weights=counts)) * 2.3548
            amp_exp = np.max(counts)*(fwhm_exp/np.sqrt(4*np.log(2)/np.pi))
            popt, pcov = curve_fit(gaus, bins[counts>0], counts[counts>0], sigma=counts_err[counts>0], p0=[amp_exp, fwhm_exp, mean_exp])
            # Save fit results
            xc.append(popt[2])
            exc.append(np.absolute(pcov[2][2])**0.5)
            energy_calibration.append(energy)

            # Plot photopeak and fit
            fig1 = plt.figure(1, figsize=(10,7))
            ax1 = fig1.add_subplot(111)

            ax1.errorbar(bins, counts, yerr=counts_err, fmt=".", label="simulation")
            ax1.plot(bins, gaus(bins, *popt), color="r", label="fit")

            ax1.set_title("E = "+str(energy)+" keV", fontsize = 12)
            ax1.set_xlabel("Detected optical photons", fontsize = 12)
            ax1.set_ylabel("Normalized counts", fontsize = 12)
            ax1.set_xlim(left=0)
            ax1.set_ylim(bottom=0)
            ax1.legend(loc="upper right", fontsize = 12)
            fig1.tight_layout()
            fig1.savefig(os.path.join(plots_dir, f"{name}_{source_name}_photons_fit.pdf"))
            
            #plt.show()

    #######################################
    ### Energy calibration ###
    #######################################

    gs = gridspec.GridSpec(5, 1)
    gs.update(hspace = 0.)

    plt.close('all')
    # Plot: Nabs vs Energy
    fig1 = plt.figure(1, figsize=(10,7))
    ax1 = plt.Subplot(fig1, gs[0:4, 0:1])
    fig1.add_subplot(ax1)
    ax11 = plt.Subplot(fig1, gs[4:5, 0:1])
    fig1.add_subplot(ax11)

    xc, exc, energy_calibration = np.array(xc), np.array(exc), np.array(energy_calibration)

    # Fit energy-photon relation with linear model
    priors = [(1, 0.3), (-10, 2)]
    initial_guess = [mean for (mean, std) in priors]
    popt, perr, flat_samples = fit_model_mcmc(energy_calibration, xc, exc, lin, initial_guess = initial_guess, bounds=None, priors = priors)
    a, b = popt[0], popt[1]
    ea, eb = perr[0], perr[1]

    print('###########################################')
    print(f'### Energy calibration best-fit parameters energy -> photons ({name}):')
    print(f'a = {a} p/m {ea}')
    print(f'b = {b} p/m {eb}')
    print('###########################################')

    # These are the inverse parameters
    m, q = 1/a, -b/a
    em, eq = ea / a**2, np.sqrt((eb / a)**2 + (b * ea / a**2)**2)

    print('###########################################')
    print(f'### Energy calibration best-fit parameters photons -> energy ({name}):')
    print(f'm = {m} p/m {em}')
    print(f'q = {q} p/m {eq}')
    print('###########################################')

    # chi-squared
    dof = len(xc) - 2
    chi_red = np.sum(((xc-lin(energy_calibration, *popt))/exc)**2)/dof
    #print(f"Reduced chi-squared: {chi_red}")

    ax1.errorbar(energy_calibration, xc, yerr=exc, fmt="o", label="Simulation")
    ene_fit = np.arange(0, 2000, 1)
    ax1.plot(ene_fit, lin(ene_fit, *popt), color="r", label="Linear fit")
    ax11.errorbar(energy_calibration, (xc-lin(energy_calibration, *popt))*100/lin(energy_calibration, *popt), yerr=exc*100/lin(energy_calibration, *popt), fmt='o')
    ax11.errorbar([0, 2000], [0, 0], fmt = 'k--')

    ax11.set_xlabel("Energy [keV]", fontsize=20)
    ax1.set_ylabel("Detected optical photons", fontsize=20)
    ax11.set_ylabel("Residuals (%)", fontsize = 20)
    ax1.set_xlim([0, 2000])
    ax11.set_xlim([0, 2000])
    #ax1.set_ylim(top=1000)
    ax11.set_ylim([-2.5, 2.5])
    ax1.set_xticklabels([])
    #ax1.set_xscale("log")
    #ax1.set_yscale("log")
    ax1.legend(loc="upper left", fontsize=20)
    ax1.grid(which="minor", alpha=0.5)
    ax1.grid(which="major", alpha=0.5)
    ax11.grid(which="minor", alpha=0.5)
    ax11.grid(which="major", alpha=0.5)
    ax1.tick_params(axis='both', which='major', labelsize=15)
    ax1.tick_params(axis='both', which='minor', labelsize=15)
    ax11.tick_params(axis='both', which='major', labelsize=15)
    ax11.tick_params(axis='both', which='minor', labelsize=15)
    fig1.tight_layout()
    fig1.savefig(os.path.join(plots_dir, f"{name}_energy_calibration_sim.pdf"))

    plt.show()

    ### Convert spectra into energy spectra ###
    energy_resol_dataset, resolution_dataset, resolution_err_dataset = [], [], []
    # Loop over the sources
    for source in sources:
        source_name, file_sim, bin_width, lines = source['source_name'], source['file_sim'], source['bin_width'], source['lines']
        bin_width = m * bin_width

        # Read energy deposit and number of detected photons for the corresponding source
        Edep_scint, Nabs = np.genfromtxt(file_sim, usecols = (1, 2), unpack = True)
        # Convert Nabs into energy
        Eabs = m * Nabs + q
        # Propagate errors on fit parameters
        Eabs_err = np.sqrt((em*Nabs)**2 + (eq)**2)

        # Read experimental electronic resolution
        # If the user did not provide a manual value, read it from the file
        if not electron_resolution_manual:
            with open('exp_resolution.dat', 'r') as f:
                for line in f:
                    line = line.strip()
                    if line.startswith('# a ='):
                        columns = line.split()
                        fwhm_noise = float(columns[3])
            sigma_noise = fwhm_noise / (2 * np.sqrt(2 * np.log(2)))
        else:
            # If the user provided a manual value, use it
            fwhm_noise = electron_resolution
            sigma_noise = fwhm_noise / (2 * np.sqrt(2 * np.log(2)))

        # Add noise to the energy spectrum 
        # Do it multiple iterations to get the average spectrum and add standard deviation to the total error
        Eabs_iter, Eabs_top_iter, Eabs_bot_iter = [], [], []
        for i in range(1000):
            # Add noise to the energies (Eabs) and also to energies p/m their errors (Eabs_top, Eabs_bot)
            Eabs_iter.append(np.random.normal(Eabs, sigma_noise))
            Eabs_top_iter.append(np.random.normal(Eabs + Eabs_err, sigma_noise))
            Eabs_bot_iter.append(np.random.normal(Eabs - Eabs_err, sigma_noise))

        edges = np.arange(np.min(Eabs_iter), np.max(Eabs_iter)+bin_width, bin_width)
        hist_tot, hist_tot_err = [], []
        for Eabs, Eabs_top, Eabs_bot in zip(Eabs_iter, Eabs_top_iter, Eabs_bot_iter):
            # Bin Eabs, Eabs_top and Eabs_bot
            hist, bin_edges = np.histogram(Eabs, bins = edges)
            hist_top, bin_edges_top = np.histogram(Eabs_top, bins = edges)
            hist_bot, bin_edges_bot = np.histogram(Eabs_bot, bins = edges)
            # Systematic error due to energy calibration
            hist_sys = (hist_top - hist_bot)/2
            # Statistical error
            hist_stat = np.sqrt(hist)
            # Total error = sqrt(statistical error^2 + systematic error^2)
            hist_err = np.sqrt(hist_stat**2 + hist_sys**2)
            hist_tot.append(hist)
            hist_tot_err.append(hist_err)
        hist_tot, hist_tot_err = np.array(hist_tot), np.array(hist_tot_err)
        # Get average spectrum from all iterations
        hist = np.average(hist_tot, axis=0)
        # Get average error from all iterations
        hist_err = np.average(hist_tot_err, axis=0)
        # For each bin, add to its error the standard deviation of the counts from all iterations
        hist_err = np.sqrt(hist_err**2 + np.std(hist_tot, axis=0))
        #sys_contrib = np.nanmean((hist_err - np.sqrt(hist)) / hist_err)
        #print(f"SYSYEMATIC: {sys_contrib}")

        # Plot histograms with all events
        fig1 = plt.figure(1, figsize=(10,7))
        ax1 = fig1.add_subplot(111)
        bin_centers = 0.5 * (bin_edges[1:] + bin_edges[:-1])
        ax1.errorbar(bin_centers, hist, yerr= hist_err, fmt='.',label=str(energy)+" keV")

        # Write to file
        spectrums_ene_dir = "Spectrums_sim_"+name
        filename = os.path.join(spectrums_ene_dir, 'data_'+source_name+'.dat')
        if not os.path.exists(spectrums_ene_dir): os.mkdir(spectrums_ene_dir)
        with open(filename, "w") as f:
            f.write("# Energy[keV] Counts Counts_err\n")
            for b, c, ec in zip(bin_centers, hist, hist_err):
                f.write(f"{b} {c} {ec}\n")

        ax1.set_title(f"{source_name} spectrum", fontsize = 15)
        ax1.set_xlabel("Energy [keV]", fontsize = 15)
        ax1.set_ylabel("Counts", fontsize = 15)
        ax1.set_ylim(bottom=0)
        ax1.legend(fontsize = 15)
        fig1.tight_layout()
        #fig1.savefig(os.path.join(plots_dir, source_name + '_All.pdf'))

        #plt.show()

        for line in lines:

            # Plot histograms with only photopeaks
            fig2 = plt.figure(2, figsize=(10,7))
            ax2 = fig2.add_subplot(111)

            energy = line['energy']

            Eabs_photo = Eabs[np.isclose(Edep_scint, energy, rtol = 1e-2, atol=1)]
            edges = np.arange(np.min(Eabs_photo), np.max(Eabs_photo)+bin_width, bin_width)
            hist, bin_edges = np.histogram(Eabs_photo, bins = edges)
            hist_err = np.sqrt(hist)
            bin_centers = 0.5 * (bin_edges[1:] + bin_edges[:-1])
            ax2.errorbar(bin_centers, hist, yerr= hist_err, fmt='.',label=str(energy)+" keV")

            ax2.set_title(f"{source_name} spectrum ({energy} keV photopeak)", fontsize = 15)
            ax2.set_xlabel("Energy [keV]", fontsize = 15)
            ax2.set_ylabel("Counts", fontsize = 15)
            ax2.set_ylim(bottom=0)
            ax2.legend(fontsize = 15)
            fig2.tight_layout()
            #fig2.savefig(os.path.join(plots_dir, source_name + "_" + str(ene) + "_Photopeaks.pdf"))

            #plt.show()


            bins, counts, counts_err = bin_centers[hist > 0], hist[hist > 0], hist_err[hist > 0]

            counts_err = counts_err / np.sum(counts)
            counts = counts / np.sum(counts)

            # Fit energy photopeaks
            mean_exp = np.average(bins, weights=counts)
            fwhm_exp = np.sqrt(np.average((bins - mean_exp)**2, weights=counts)) * 2.3548
            amp_exp = np.max(counts)*(fwhm_exp/np.sqrt(4*np.log(2)/np.pi))
            priors = [[amp_exp, amp_exp*0.1], [fwhm_exp, fwhm_exp*0.1], [mean_exp, mean_exp*0.1]]
            initial_guess = [mean for (mean, std) in priors]
            popt, pcov = curve_fit(gaus, bins, counts, sigma=counts_err, p0=[amp_exp, fwhm_exp, mean_exp])
            perr = np.sqrt(np.diag(pcov))
            #popt, perr, flat_samples = fit_model_mcmc(bins, counts, counts_err, gaus, initial_guess = initial_guess, bounds=None, priors = priors)
            # Save fit results
            fwhm = popt[1]
            efwhm = perr[1]
            ec = popt[2]
            eec = perr[2]
            res = fwhm * 100 / ec
            res_err = np.sqrt((efwhm*100/ec)**2 + (fwhm*100*eec/ec**2)**2)
            resolution_dataset.append(res)
            resolution_err_dataset.append(res_err)
            energy_resol_dataset.append(energy)
            dataset_resol.append(name)

            fig1 = plt.figure(1, figsize=(10,7))
            ax1 = fig1.add_subplot(111)

            ax1.errorbar(bins, counts, yerr=counts_err, fmt=".", label="simulation")
            ax1.plot(bins, gaus(bins, *popt), color="r", label="fit")

            ax1.set_title("E = "+str(energy)+" keV", fontsize = 12)
            ax1.set_xlabel("Energy [keV]", fontsize = 12)
            ax1.set_ylabel("Normalized counts", fontsize = 12)
            ax1.set_xlim(left=0)
            ax1.set_ylim(bottom=0)
            ax1.legend(loc="upper right", fontsize = 12)
            fig1.tight_layout()
            #fig1.savefig(os.path.join(plots_dir, str(ene) + '_fitted.pdf'))
            
            #plt.show()

    # Store energy resolutions
    energy_resol.append(energy_resol_dataset)
    resolution.append(resolution_dataset)
    resolution_err.append(resolution_err_dataset)

plt.close('all')

#######################################
### Energy resolution ###
#######################################

fig1 = plt.figure(1, figsize=(10,7))
ax1 = fig1.add_subplot(111)

for dataset, ene, res, res_err in zip(datasets, energy_resol, resolution, resolution_err):
    name = dataset['name']
    ax1.errorbar(ene, res, yerr=res_err, fmt="o", markersize=10, zorder=2, label=name)

energy_resol, resolution, resolution_err  = np.array([e for ene in energy_resol for e in ene]), np.array([r for res in resolution for r in res]), np.array([er for eres in resolution_err for er in eres]), 

# Fit energy resolution
priors = [(20, 1), (3, 1), (0, 0.5)]
initial_guess = [mean for (mean, std) in priors]
opt_params_lab, param_errors, flat_samples = fit_model_mcmc(energy_resol, resolution*energy_resol/100, resolution_err*energy_resol/100, w_func, initial_guess = initial_guess, bounds=[(0, np.inf), (0, np.inf), (0, np.inf)], priors = priors)
print('###########################################')
print(f'### Energy resolution fit parameters:')
print(f'a = {opt_params_lab[0]} p/m {param_errors[0]}')
print(f'b = {opt_params_lab[1]} p/m {param_errors[1]}')
print(f'c = {opt_params_lab[2]} p/m {param_errors[2]}')
print('###########################################')
energy_fit = np.linspace(1, 2000, 10000)
ax1.plot(energy_fit, w_func(energy_fit, *opt_params_lab)*100/energy_fit, color="k", lw=1.2, zorder=0, label="Best fit")
ax1.plot(energy_fit, w_func(energy_fit, opt_params_lab[0], 0, 0)*100/energy_fit, lw=1, ls="--", zorder=0, label="Noise")
ax1.plot(energy_fit, w_func(energy_fit, 0, opt_params_lab[1], 0)*100/energy_fit, lw=1, ls="--", zorder=0, label="Statistical")
ax1.plot(energy_fit, w_func(energy_fit, 0, 0, opt_params_lab[2])*100/energy_fit, lw=1, ls="--", zorder=0, label="Inhomogeneity")


# Chi-squared
dof = len(resolution) - 3
model = w_func(energy_resol, *opt_params_lab)*100/energy_resol
chi_red = np.sum(((resolution-model)/resolution_err)**2)/dof
#print(f"Reduced chi-squared: {chi_red}\n")

# Save fit parameters and resolutions to file
with open('sim_resolution.dat', 'w') as f:
    f.write(f'# Fit parameters:\n')
    f.write(f'# a = {abs(opt_params_lab[0]):.5f} p\m {abs(param_errors[0]):.5f}\n')
    f.write(f'# b = {abs(opt_params_lab[1]):.5f} p\m {abs(param_errors[1]):.5f}\n') 
    f.write(f'# c = {abs(opt_params_lab[2]):.5f} p\m {abs(param_errors[2]):.5f}\n')
    f.write('# Energy[keV] Resolution(%) Resolution_err(%) Dataset\n')
    for e, r, er, n in zip(energy_resol, resolution, resolution_err, dataset_resol):
        f.write('{0:<30.0f}'.format(e))
        f.write('{0:<30.5f}'.format(r))
        f.write('{0:<30.5f}'.format(er))
        f.write(n)

        f.write('\n')

ax1.set_xlabel("Energy [keV]", fontsize=20)
ax1.set_ylabel("Resolution (%)", fontsize=20)
ax1.set_xlim([0, 2000])
ax1.set_ylim([0, 70])
#ax1.set_xscale("log")
#ax1.set_yscale("log")
#ax1.set_xticklabels([])
ax1.legend(loc='upper right', fontsize=20)
ax1.grid(which="minor", alpha=0.5)
ax1.grid(which="major", alpha=0.5)
ax1.tick_params(axis='both', which='major', labelsize=15)
ax1.tick_params(axis='both', which='minor', labelsize=15)
fig1.tight_layout()
fig1.savefig('resolution_sim.pdf')

plt.show()




