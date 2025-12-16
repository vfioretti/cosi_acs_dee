import os
import sys
import json
import matplotlib.pyplot as plt
from matplotlib import gridspec
from scipy.interpolate import interp1d
import shutil

from functions import *
from models import *

# Import the input parameters
arg_list = sys.argv
json_file = arg_list[1]

# Load the JSON file with the information about the sources
with open(json_file, 'r') as f:
    data = json.load(f)

# Path to plots
plots_path = './calibration_lab_plots/'
if not os.path.exists(plots_path):
    os.makedirs(plots_path)

j = 1
energy_resol, resolution, resolution_err, dataset_resol = [], [], [], []
datasets = data['uncollimated']
for dataset in datasets:
    energies, a, ea, w, ew, xc, exc, chi_red_list = [], [], [], [], [], [], [], []
    energy_resol_dataset, resolution_dataset, resolution_err_dataset = [], [], []
    path, name, sources = dataset['path_lab'], dataset['name'], dataset['sources']
    for source in sources:
        source_name, file, files_bkg, lines = source['source_name'], source['file_lab'], source['file_bkg'], source['lines']
        file = os.path.join(path, file)
        files_bkg = [os.path.join(path, file_bkg) for file_bkg in files_bkg]

        # Read source spectrum
        channel, counts, time = read_spectrum(file)

        # Read background spectra
        for i, file_bkg in enumerate(files_bkg):
            channel_bkg, counts_bkg, time_bkg = read_spectrum(file_bkg)
            if i == 0: 
                time_ref = time_bkg
                counts_ref = counts_bkg
                counts_bkg_tot = counts_bkg
            else: 
                counts_bkg = (counts_bkg * time_ref / time_bkg).astype(int)
                counts_bkg_tot += counts_bkg
        counts_bkg = counts_bkg_tot / len(files_bkg)

        # Rescale counts
        counts_err = np.sqrt(counts) * time_ref / time
        counts = counts * time_ref / time

        # Subtract background and set to zero negative values
        counts_sub = counts - counts_bkg
        counts_sub[counts_sub < 0] = 0
        counts_sub_err = np.sqrt(counts_err**2 + np.sqrt(counts_bkg)**2)

        # Normalize counts
        counts_err_norm = counts_sub_err/np.sum(counts_sub)
        counts_norm = counts_sub/np.sum(counts_sub)
        channel_norm = channel

        for line in lines:
            energy, channel_lower, channel_upper, model, priors = line['energy'], line['channel_range'][0], line['channel_range'][1], line['model'], line['initial_params_channel']
            model = get_model(model)

            fig1 = plt.figure(j, figsize=(10,7))
            ax1 = fig1.add_subplot(111)

            # Cut spectrums
            counts = counts_norm[(channel_norm >= channel_lower) & (channel_norm <= channel_upper)]
            counts_err = counts_err_norm[(channel_norm >= channel_lower) & (channel_norm <= channel_upper)]
            channel = channel_norm[(channel_norm >= channel_lower) & (channel_norm <= channel_upper)]
            ax1.errorbar(channel, counts, yerr=counts_err, fmt=".", color="k", zorder=1)

            # Perform the fit
            initial_guess = [mean for (mean, std) in priors]
            popt, perrtot, flat_samples = fit_model_mcmc(channel, counts, counts_err, model, initial_guess = initial_guess, bounds=None, priors = priors)
            plot_model(ax1, channel, model, popt)

            energies.append(energy)
            a.append(popt[0])
            ea.append(perrtot[0])
            w.append(popt[1])
            ew.append(perrtot[1])
            xc.append(popt[2])
            exc.append(perrtot[2])

            # Chi-squared
            num_params = len(initial_guess)
            dof = len(counts) - num_params
            chi_red_list.append(np.sum(((counts-model(channel, *popt))/counts_err)**2)/dof)

            ax1.set_title(source_name, fontsize = 20)
            ax1.set_xlabel("Channel", fontsize = 20)
            ax1.set_ylabel("Normalized counts", fontsize = 20)
            ax1.set_xlim([channel_lower, channel_upper])
            ax1.set_ylim(bottom=0)
            plt.xticks(fontsize = 15)
            plt.yticks(fontsize = 15)
            ax1.legend(fontsize = 20)
            ax1.grid()
            fig1.tight_layout()
            fig1.savefig(os.path.join(plots_path, f"{name}_{source_name}_channel_fit.pdf"))

            j += 1
        
    plt.show()

    ### Fit energy - channel relation ###

    energies, w, ew, xc, exc = np.array(energies), np.array(w), np.array(ew), np.array(xc), np.array(exc)

    # Nabs vs Energy
    gs = gridspec.GridSpec(5, 1)
    gs.update(hspace = 0.)
    fig1 = plt.figure(1, figsize=(10,7))
    ax1 = plt.Subplot(fig1, gs[0:4, 0:1])
    fig1.add_subplot(ax1)
    ax11 = plt.Subplot(fig1, gs[4:5, 0:1])
    fig1.add_subplot(ax11)

    priors = [(1, 0.3), (-10, 2)]
    initial_guess = [mean for (mean, std) in priors]
    popt, perr, flat_samples = fit_model_mcmc(energies, xc, exc, lin, initial_guess = initial_guess, bounds=None, priors = priors)
    popt_max = popt + perr
    popt_min = popt - perr
    a, b = popt[0], popt[1]
    ea, eb = perr[0], perr[1]

    print('###########################################')
    print(f'### Energy calibration best-fit parameters energy -> channels ({name}):')
    print(f'a = {a} p/m {ea}')
    print(f'b = {b} p/m {eb}')
    print('###########################################')

    m, q = 1/a, -b/a
    em, eq = ea / a**2, np.sqrt((eb / a)**2 + (b * ea / a**2)**2)

    print('###########################################')
    print(f'### Energy calibration best-fit parameters photons -> energy ({name}):')
    print(f'm = {m} p/m {em}')
    print(f'q = {q} p/m {eq}')
    print('###########################################')

    # chi-squared
    dof = len(xc) - 2
    chi_red = np.sum(((xc-lin(energies, *popt))/exc)**2)/dof
    #print(f"Reduced chi-squared: {chi_red}")

    energy_cal = np.arange(1, 2000, 0.1)

    ax1.errorbar(energies, xc, yerr=exc, fmt="o", label="Data")
    ene = np.arange(0, 2000, 1)
    ax1.plot(ene, lin(ene, *popt), color="r", label="Linear fit")
    ax11.errorbar(energies, (xc-lin(energies, *popt))*100/lin(energies, *popt), yerr=exc*100/lin(energies, *popt), fmt='o')
    ax11.errorbar([0, 2000], [0, 0], fmt = 'k--')

    # Convert channels into energies and write to file
    spectrums_ene_dir = "./Spectrums_lab_"+name
    if os.path.exists(spectrums_ene_dir): shutil.rmtree(spectrums_ene_dir)
    os.mkdir(spectrums_ene_dir)
    for f in os.listdir(path):
        file = os.path.join(path, f)
        if os.path.isfile(file):
            channel, counts, time = read_spectrum(file)
            energy = m * channel + q
            energy_err = np.sqrt((em*channel)**2 + (eq)**2)
            energy_up, energy_down = energy + energy_err, energy - energy_err
            counts_sys = np.interp(energy, energy_up, counts) - np.interp(energy, energy_down, counts)
            counts_err = np.sqrt(np.sqrt(counts)**2 + counts_sys**2)

            # Write to file
            filename = os.path.join(spectrums_ene_dir, f)
            with open(filename, "w") as ff:
                ff.write(f"# TIME: {time} s\n")
                ff.write("# Energy[keV] Counts Energy_err[keV] Counts_err\n")
                for b, c, eb, ec in zip(energy, counts, energy_err, counts_err):
                    ff.write(f"{b} {c} {eb} {ec}\n")

    ax11.set_xlabel("Energy [keV]", fontsize = 20)
    ax1.set_ylabel("ADC channel", fontsize = 20)
    ax11.set_ylabel("Residuals (%)", fontsize = 15)
    ax1.set_xlim([0, 2000])
    #ax1.set_ylim([0, 50000])
    ax11.set_xlim([0, 2000])
    #ax1.set_ylim(top=1000)
    if name == 'NRL': ax11.set_ylim([-15, 15])
    if name == 'SSL': ax11.set_ylim([-2.5, 2.5])
    ax1.set_xticklabels([])
    #ax1.set_xscale("log")
    #ax1.set_yscale("log")
    ax1.legend(loc="upper left", fontsize = 20)
    ax1.grid(which="minor", alpha=0.5)
    ax1.grid(which="major", alpha=0.5)
    ax11.grid(which="minor", alpha=0.5)
    ax11.grid(which="major", alpha=0.5)
    ax1.tick_params(axis='both', which='major', labelsize=15)
    ax1.tick_params(axis='both', which='minor', labelsize=15)
    ax11.tick_params(axis='both', which='major', labelsize=15)
    ax11.tick_params(axis='both', which='minor', labelsize=15)
    fig1.tight_layout()
    fig1.savefig(os.path.join(plots_path, f"{name}_energy_calibration_lab.pdf"))

    plt.show()



    ### Energy resolution ###
    j = 1
    for source in sources:
        source_name, file, files_bkg, lines = source['source_name'], source['file_lab'], source['file_bkg'], source['lines']
        file = os.path.join(spectrums_ene_dir, file)
        files_bkg = [os.path.join(spectrums_ene_dir, file_bkg) for file_bkg in files_bkg]

        # Read source spectrum
        energy, counts, time = read_spectrum(file)
        counts_err = np.genfromtxt(file, usecols=(3), unpack=True)

        # Read background spectra
        for i, file_bkg in enumerate(files_bkg):
            energy_bkg, counts_bkg, time_bkg = read_spectrum(file_bkg)
            if i == 0: 
                time_ref = time_bkg
                counts_ref = counts_bkg
                counts_bkg_tot = counts_bkg
            else: 
                counts_bkg = (counts_bkg * time_ref / time_bkg).astype(int)
                counts_bkg_tot += counts_bkg
        counts_bkg = counts_bkg_tot / len(files_bkg)

        # Rescale counts
        counts_err = counts_err * time_ref / time
        counts = counts * time_ref / time

        # Subtract background and set to zero negative values
        counts_sub = counts - counts_bkg
        counts_sub[counts_sub < 0] = 0
        counts_sub_err = np.sqrt(counts_err**2 + np.sqrt(counts_bkg)**2)

        # Normalize counts
        counts_err_norm = counts_sub_err/np.sum(counts_sub)
        counts_norm = counts_sub/np.sum(counts_sub)
        energy_norm = energy

        for line in lines:
            energy_line, channel_lower, channel_upper, model, priors = line['energy'], line['channel_range'][0], line['channel_range'][1], line['model'], line['initial_params_energy']
            model = get_model(model)

            fig1 = plt.figure(j, figsize=(10,7))
            ax1 = fig1.add_subplot(111)

            # Convert channel into energy
            energy_lower = m * channel_lower + q
            energy_upper = m * channel_upper + q

            # Cut spectrums
            counts = counts_norm[(energy_norm >= energy_lower) & (energy_norm <= energy_upper)]
            counts_err = counts_err_norm[(energy_norm >= energy_lower) & (energy_norm <= energy_upper)]
            energy = energy_norm[(energy_norm >= energy_lower) & (energy_norm <= energy_upper)]
            ax1.errorbar(energy, counts, yerr=counts_err, fmt=".", color="k", zorder=1)

            # Perform the fit
            initial_guess = [mean for (mean, std) in priors]
            popt, perrtot, flat_samples = fit_model_mcmc(energy, counts, counts_err, model, initial_guess = initial_guess, bounds=None, priors = priors)
            plot_model(ax1, energy, model, popt)

            energy_resol_dataset.append(energy_line)
            fwhm = popt[1]
            efwhm = perrtot[1]
            ec = popt[2]
            eec = perrtot[2]
            res = fwhm * 100 / ec
            res_err = np.sqrt((efwhm*100/ec)**2 + (fwhm*100*eec/ec**2)**2)
            resolution_dataset.append(res)
            resolution_err_dataset.append(res_err)
            dataset_resol.append(name)

            # Chi-squared
            num_params = len(initial_guess)
            dof = len(counts) - num_params
            chi_red_list.append(np.sum(((counts-model(energy, *popt))/counts_err)**2)/dof)

            ax1.set_title(source_name, fontsize = 20)
            ax1.set_xlabel("Energy [keV]", fontsize = 20)
            ax1.set_ylabel("Normalized counts", fontsize = 20)
            ax1.set_xlim([energy_lower, energy_upper])
            ax1.set_ylim(bottom=0)
            plt.xticks(fontsize = 15)
            plt.yticks(fontsize = 15)
            ax1.legend(loc="upper left", fontsize = 20)
            ax1.grid()

            j += 1

    with open(name+'_exp_resolution.dat', 'w') as f:
        f.write('# Energy[keV] Resolution(%) Resolution_err(%)\n')
        for e, r, er in zip(energy_resol_dataset, resolution_dataset, resolution_err_dataset):
            f.write('{0:<30.0f}'.format(e))
            f.write('{0:<30.5f}'.format(r))
            f.write('{0:<30.5f}'.format(er))

            f.write('\n')

    energy_resol.append(energy_resol_dataset)
    resolution.append(resolution_dataset)
    resolution_err.append(resolution_err_dataset)
        
    plt.show()


fig1 = plt.figure(1, figsize=(10,7))
ax1 = fig1.add_subplot(111)

for dataset, ene, res, res_err in zip(datasets, energy_resol, resolution, resolution_err):
    name = dataset['name']
    ax1.errorbar(ene, res, yerr=res_err, fmt="o", markersize=10, zorder=2, label=name)

energy_resol, resolution, resolution_err, dataset_resol  = np.array([e for ene in energy_resol for e in ene]), np.array([r for res in resolution for r in res]), np.array([er for eres in resolution_err for er in eres]), np.array(dataset_resol)

priors = [(25, 5), (3, 1), (0, 0.5)]
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

with open('exp_resolution.dat', 'w') as f:
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
fig1.savefig('resolution_exp.pdf')

plt.show()

