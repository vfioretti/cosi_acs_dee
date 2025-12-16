import json
import sys, os
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


##################
### Experiment ###
##################

datasets = data['collimated']
for dataset in datasets:
    path, name, sources, positions = dataset['path_lab'], dataset['name'], dataset['sources'], dataset['positions']
    resp_lab, respErr_lab, ene = [], [], []
    for source in sources:
        source_name, file_lab, file_bkg, file_sim, lines = source['source_name'], source['file_lab'], source['file_bkg'], source['file_sim'], source['lines']
        file_lab = os.path.join(path, file_lab)
        files_bkg = [os.path.join(path, f) for f in file_bkg]

        for line in lines:
            energy, channel_lower, channel_upper, model, initial_params, bin_width = line['energy'], line['channel_range'][0], line['channel_range'][1], line['model'], line['initial_params'], line['bin_width']
            model = get_model(model)
            resp_source, respErr_source = [], []

            for pos in positions:

                fig1 = plt.figure(1, figsize=(10,7))
                ax1 = fig1.add_subplot(111)

                # Read file
                file_pos = file_lab.replace('*', str(pos))
                channel, counts, time = read_spectrum(file_pos)

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
                counts_bkg = counts_bkg_tot / len(file_bkg)

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

                # Cut
                counts = counts_norm[(channel_norm > channel_lower) & (channel_norm < channel_upper)]
                counts_err = counts_err_norm[(channel_norm > channel_lower) & (channel_norm < channel_upper)]
                channel = channel_norm[(channel_norm > channel_lower) & (channel_norm < channel_upper)]

                # Fit
                initial_guess = [mean for (mean, std) in initial_params]
                popt, perr, flat_samples = fit_model_mcmc(channel, counts, counts_err, model, initial_guess = initial_guess, bounds=None, priors = initial_params)
                ax1.errorbar(channel, counts, yerr=counts_err, fmt=".", alpha=0.2)
                plot_model(ax1, channel, model, popt)

                resp_source.append(popt[2])
                respErr_source.append(perr[2])

                ax1.set_title(source_name, fontsize=20)
                ax1.set_xlabel("Channel", fontsize=20)
                ax1.set_ylabel("Counts", fontsize=20)
                ax1.set_xlim([channel_lower, channel_upper])
                ax1.set_ylim(bottom=0)
                #ax1.set_yscale("log")
                ax1.legend(loc="upper right", fontsize=20)
                ax1.grid(which="minor", alpha=0.5)
                ax1.grid(which="major", alpha=0.5)
                fig1.tight_layout()
            
                #plt.show()
            
            resp_lab.append(resp_source)
            respErr_lab.append(respErr_source)
            ene.append(energy)

    plt.close('all')

    fig1 = plt.figure(1, figsize=(10,7))
    ax1 = fig1.add_subplot(111)

    resp_lab = np.array(resp_lab)
    respErr_lab = np.array(respErr_lab)
    relResponseLab = np.array([r/np.sum(r) for r in resp_lab])
    relResponseErrLab = np.array([np.sqrt((er/np.sum(r))**2 + (r*np.sqrt(np.sum(er)**2)/np.sum(r)**2)**2) for r, er in zip(resp_lab, respErr_lab)])

    positions_sorted = sorted(positions) 
    for r, er, e in zip(relResponseLab, relResponseErrLab, ene):
        ax1.errorbar(positions_sorted, r, yerr=er, fmt="o-", label=f'{e} keV')

    ax1.set_xlabel("Position", fontsize=20)
    ax1.set_ylabel("BGO relative response", fontsize=20)
    ax1.set_xlim([0.5, len(positions)+0.5])
    ax1.set_ylim([0.07, 0.1])
    #ax1.set_yscale("log")
    ax1.set_xticks([1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12], labels=["1", "10", "2", "11", "3", "12", "4", "5", "6", "7", "8", "9"])
    ax1.legend(loc="upper right", fontsize=20)
    ax1.grid(which="minor", alpha=0.5)
    ax1.grid(which="major", alpha=0.5)
    ax1.tick_params(axis='both', which='major', labelsize=15)
    ax1.tick_params(axis='both', which='minor', labelsize=15)
    fig1.tight_layout()
    fig1.savefig(f'exp_relative_response.pdf')

    plt.show()

##################
### Simulation ###
##################
            
    name, sources, positions = dataset['name'], dataset['sources'], dataset['positions']
    resp_sim, respErr_sim = [], []
    response, responseErr = [], []
    responseRel_min, responseRel_max = [], []
    for source in sources:
        source_name, file_sim, lines = source['source_name'], source['file_sim'], source['lines']

        for line in lines:
            energy, bin_width = line['energy'], line['bin_width']
            response_source, responseErr_source = [], []

            eventID, pos, edep, Nopt = np.genfromtxt(file_sim, usecols=(0,1,2,3), unpack=True)
            eventID = eventID[np.isclose(edep, energy, rtol = 1e-2, atol=1)]
            pos = pos[np.isclose(edep, energy, rtol = 1e-2, atol=1)]
            Nopt = Nopt[np.isclose(edep, energy, rtol = 1e-2, atol=1)]
            edep = edep[np.isclose(edep, energy, rtol = 1e-2, atol=1)]
            
            fig2 = plt.figure(2, figsize=(10,7))
            ax2 = fig2.add_subplot(111)

            colors = [
                "#E6194B",  # Red  
                "#FABEBE",  # Pink  
                "#3CB44B",  # Green  
                "#008080",  # Teal  
                "#FFE119",  # Yellow
                "#E6BEFF",   # Lavender   
                "#4363D8",  # Blue  
                "#F58231",  # Orange  
                "#911EB4",  # Purple  
                "#46F0F0",  # Cyan  
                "#F032E6",  # Magenta  
                "#BCF60C"  # Lime  
            ]
            for i, p in enumerate(positions):
                Nopt_pos = Nopt[pos == p]
                
                # Bin
                nbins = int((np.max(Nopt_pos)-np.min(Nopt_pos))/bin_width)
                hist, bin_edges = np.histogram(Nopt_pos, bins = nbins)
                bin_centers = 0.5 * (bin_edges[1:] + bin_edges[:-1])
                spectrum2, = ax2.step(bin_centers, hist,  where="mid", alpha=0.2, color=colors[i])
                ax2.errorbar(bin_centers, hist, yerr=np.sqrt(hist), fmt=".", color = spectrum2.get_color(), alpha=0.2)

                # Fit
                mean_exp = np.average(bin_centers[hist!=0], weights=hist[hist!=0])
                fwhm_exp = np.sqrt(np.average((bin_centers[hist!=0] - mean_exp)**2, weights=hist[hist!=0])) * 2.3548
                amp_exp = np.max(hist[hist!=0])*(fwhm_exp/np.sqrt(4*np.log(2)/np.pi))
                popt, pcov = curve_fit(gaus, bin_centers[hist!=0], hist[hist!=0], sigma=np.sqrt(hist[hist!=0]), p0=[amp_exp, fwhm_exp, mean_exp])
                perr = np.sqrt(np.diag(pcov)) 
                bin_fit = np.arange(bin_centers[hist!=0][0], bin_centers[hist!=0][-1], 0.1)
                bin_fit = np.arange(0, 1000, 0.1)
                ax2.plot(bin_fit, gaus(bin_fit, *popt), color=spectrum2.get_color(), label="pos"+str(p))
                
                response_source.append(popt[2])
                responseErr_source.append(perr[2])

            ax2.set_title(source_name, fontsize=20)
            ax2.set_xlabel("Detected optical photons", fontsize=20)
            ax2.set_ylabel("Counts", fontsize=20)
            ax2.set_xlim([120, 250])
            ax2.set_ylim(bottom=0)
            #ax2.set_yscale("log")
            ax2.legend(loc="upper right", fontsize=20)
            ax2.grid(which="minor", alpha=0.5)
            ax2.grid(which="major", alpha=0.5)
            ax2.tick_params(axis='both', which='major', labelsize=15)
            ax2.tick_params(axis='both', which='minor', labelsize=15)
            fig2.tight_layout()
        
            plt.show()

            # Systematics
            fig5 = plt.figure(5, figsize=(10,7))
            ax5 = fig5.add_subplot(111)
            biases = ["left", "right", "forward", "back"]
            response_min = []
            response_max = []
            responses_bias = []
            for p in positions:
                response_bias = []
                for bias in biases:
                    dirs = file_sim.split("/")
                    for j in range(len(dirs)):
                        if "coll" in dirs[j]:
                                dirs[j] = dirs[j] + "_" + bias
                    file_new = "/".join(dirs)

                    eventID, pos, edep, Nopt = np.genfromtxt(file_new, usecols=(0,1,2,3), unpack=True)

                    eventID = eventID[np.isclose(edep, energy, rtol = 1e-2, atol=1)]
                    pos = pos[np.isclose(edep, energy, rtol = 1e-2, atol=1)]
                    Nopt = Nopt[np.isclose(edep, energy, rtol = 1e-2, atol=1)]
                    edep = edep[np.isclose(edep, energy, rtol = 1e-2, atol=1)]

                    Nopt_pos = Nopt[pos == p]

                    # Histogram
                    nbins = int((np.max(Nopt_pos)-np.min(Nopt_pos))/bin_width)
                    hist, bin_edges = np.histogram(Nopt_pos, bins = nbins)
                    bin_centers = 0.5 * (bin_edges[1:] + bin_edges[:-1])
                    if p == 2: spectrum, = ax5.step(bin_centers, hist,  where="mid", alpha=0.2)
                    # Fit
                    mean_exp = np.average(bin_centers[hist!=0], weights=hist[hist!=0])
                    fwhm_exp = np.sqrt(np.average((bin_centers[hist!=0] - mean_exp)**2, weights=hist[hist!=0])) * 2.3548
                    amp_exp = np.max(hist[hist!=0])*(fwhm_exp/np.sqrt(4*np.log(2)/np.pi))
                    popt, pcov = curve_fit(gaus, bin_centers[hist!=0], hist[hist!=0], sigma=np.sqrt(hist[hist!=0]), p0=[amp_exp, fwhm_exp, mean_exp])
                    perr = np.sqrt(np.diag(pcov))
                    bin_fit = np.arange(0, 1000, 0.1)
                    if p == 2: ax5.plot(bin_fit, gaus(bin_fit, *popt), color=spectrum.get_color(), label="pos"+str(p))

                    response_bias.append(popt[2])

                response_bias = np.array(response_bias)
                response_min.append(np.min(response_bias))
                response_max.append(np.max(response_bias))
                responses_bias.append(response_bias)

            plt.show()

            response.append(response_source)
            responseErr.append(responseErr_source)

            response_min = np.array(response_min)
            response_max = np.array(response_max)
            j = 0
            responseMin = []
            responseMax = []
            for r4_bias in responses_bias:
                responseMin.append(response_min[j]/(np.sum(response_max)-response_max[j]+response_min[j]))
                responseMax.append(response_max[j]/(np.sum(response_min)-response_min[j]+response_max[j]))
                j += 1 
            responseRel_min.append(responseMin)
            responseRel_max.append(responseMax)

    ene = np.array(ene)
    response = np.array(response)
    responseErr = np.array(responseErr)
    relResponseSim = np.array([r/np.sum(r) for r in response])
    relResponseErrSim = np.array([np.sqrt((er/np.sum(r))**2 + (r*np.sqrt(np.sum(er)**2)/np.sum(r)**2)**2) for r, er in zip(response, responseErr)])

    positions_sorted = sorted(positions) 
    for rs, ers, rl, erl, rmin, rmax, e in zip(relResponseSim, relResponseErrSim, relResponseLab, relResponseErrLab, responseRel_min, responseRel_max, ene):
        fig1 = plt.figure(1, figsize=(10,7))
        ax1 = plt.Subplot(fig1, gs[0:4, 0:1])
        fig1.add_subplot(ax1)
        ax11 = plt.Subplot(fig1, gs[4:5, 0:1])
        fig1.add_subplot(ax11)
        
        #ax1.errorbar(positions_sorted, rl, yerr=erl, color="k", fmt="o-", label="Experiment")
        #ax1.errorbar(positions_sorted, rs, yerr=ers, color="red", fmt="o-", label="Simulation")
        err = np.sqrt(((ers+erl)*100/rl)**2 + ((rs-rl)*erl*100/rl**2)**2)
        err_sys_bot = abs(rs - rmin)
        err_sys_up = abs(rs - rmax)
        err_sys_tot_sim = abs(err_sys_up+err_sys_bot)/2
        err_tot_bot = np.sqrt(ers**2 + err_sys_bot**2)
        err_tot_up = np.sqrt(ers**2 + err_sys_up**2)
        err_tot_sim = (err_tot_up + err_tot_bot) / 2
        ax1.errorbar(positions_sorted, rs, yerr=err_tot_sim, color="red", fmt='o-', label="Simulation")
        err_sys_lab = (err_sys_tot_sim / rs) * rl
        err_tot_lab = np.sqrt(erl**2 + err_sys_lab**2)
        ax1.errorbar(positions_sorted, rl, yerr=err_tot_lab, color="k", fmt="o-", zorder=0, label="Experiment")
        res_err_tot = (err_tot_sim+err_tot_lab)*100/rl + (rs-rl)*err_tot_lab*100/rl**2
        ax11.errorbar(sorted(positions), (rs-rl)*100/rl, yerr=res_err_tot, color="red", fmt='o')
        ax11.errorbar([0, 1000], [0, 0], fmt = 'k--')

        with open(f'{e}keV_response_comparison.dat', 'w') as f:
            f.write('# Position Experiment Experiment_error Simulation Simulation_error Residuals(%) Residuals_error\n')
            for pos, rll, erl, rss, ers, res, res_err in zip(positions_sorted, rl, err_tot_lab, rs, err_tot_sim, (rs-rl)*100/rl, res_err_tot):
                f.write(f'{pos} {rll} {erl} {rss} {ers} {res} {res_err}\n')

        ax1.set_title(f'{e} keV', fontsize=20)
        ax11.set_xlabel("Position", fontsize=20)
        ax11.set_ylabel("Residuals (%)", fontsize = 15)
        ax1.set_ylabel("BGO relative response", fontsize=20)
        ax1.set_xlim([0.5, len(positions)+0.5])
        ax11.set_xlim([0.5, len(positions)+0.5])
        ax11.set_ylim([-30, 30])
        ax1.set_ylim([0.05, 0.125])
        ax1.set_yticks(ax1.get_yticks()[1:])
        #ax1.set_yscale("log")
        ax1.set_xticklabels([])
        ax11.set_xticks([1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12], labels=["1", "10", "2", "11", "3", "12", "4", "5", "6", "7", "8", "9"])
        ax11.set_yticks([-30, -10, 10, 30])
        #ax11.set_xticklabels(["1", "10", "2", "11", "3", "12", "4", "5", "6", "7", "8", "9"])
        ticks = ax1.get_yticks()
        ax1.set_yticks(ticks[ticks != 0])
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
        fig1.savefig(f'{e}keV_response_comparison.pdf')

        plt.show()


        
