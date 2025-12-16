import numpy as np
import emcee
import corner

# Spectrum file reader
def read_spectrum(filename):
    channel = []
    counts = []
    c = 0
    with open(filename, "r") as f:
        for line in f:
            line = line.strip()
            if c == 0:
                columns = line.split()
                time = float(columns[-2])
            else:
                if not line.startswith("#"):
                    columns = line.split()
                    channel.append(float(columns[0]))
                    counts.append(int(columns[1]))
            c += 1
    return np.array(channel), np.array(counts), time

# MCMC fit
def fit_model_mcmc(x, y, yerr, model, initial_guess, param_names=None, bounds=None, 
                   priors=None, nwalkers=50, nsteps=5000, burn_in=300, thin=15):
    """
    Fit data with an arbitrary model using MCMC, with flat or Gaussian priors.

    Parameters:
    - x (array): The x data.
    - y (array): The y data.
    - yerr (array): The uncertainties on the y data.
    - model (function): The model function to fit, e.g., model(x, *params).
    - initial_guess (list): Initial guess for the parameters.
    - param_names (list): Optional, names of the parameters for plotting.
    - bounds (list of tuples): Optional, [(min, max), ...] bounds for each parameter (used for flat priors).
    - priors (list of tuples): Optional, [(mean, std), ...] for Gaussian priors, or `None` for flat priors.
    - nwalkers (int): Number of MCMC walkers.
    - nsteps (int): Number of MCMC steps.
    - burn_in (int): Number of steps to discard as burn-in.
    - thin (int): Thinning factor for the chain.

    Returns:
    - best_params (array): Best-fit parameters.
    - param_uncertainties (array): Uncertainties on the parameters.
    - flat_samples (array): Flattened MCMC samples.
    """

    ndim = len(initial_guess)

    # Define the log-likelihood function
    def log_likelihood(params, x, y, yerr):
        model_vals = model(x, *params)
        return -0.5 * np.sum(((y - model_vals) / yerr) ** 2)

    # Define the log-prior function
    def log_prior(params):
        lp = 0.0
        for i, p in enumerate(params):
            if bounds and bounds[i]:
                lower, upper = bounds[i]
                if not (lower < p < upper):
                    return -np.inf  # Outside bounds for flat prior
            
            if priors and priors[i]:
                mean, std = priors[i]
                lp += -0.5 * ((p - mean) / std) ** 2  # Gaussian prior
                
        return lp

    # Define the log-posterior function
    def log_posterior(params, x, y, yerr):
        lp = log_prior(params)
        if not np.isfinite(lp):
            return -np.inf
        return lp + log_likelihood(params, x, y, yerr)

    # Set up initial positions of walkers
    pos = initial_guess + 1e-4 * np.random.randn(nwalkers, ndim)

    # Set up the sampler
    sampler = emcee.EnsembleSampler(nwalkers, ndim, log_posterior, args=(x, y, yerr))

    # Run the MCMC
    sampler.run_mcmc(pos, nsteps, progress=True)

    '''
    samples = sampler.get_chain()  # Shape (nsteps, nwalkers, ndim)
    for i in range(ndim):
        plt.plot(samples[:, :, i], alpha=0.5)
        plt.xlabel("Step")
        plt.ylabel(f"Parameter {i}")
        plt.show()
    '''

    # Flatten the chain, removing burn-in and thinning
    flat_samples = sampler.get_chain(discard=burn_in, thin=thin, flat=True)

    # Calculate the best-fit parameters and uncertainties from percentiles
    percentiles = np.percentile(flat_samples, [16, 50, 84], axis=0)
    best_params = percentiles[1]  # 50th percentile (median)
    lower_uncertainties = percentiles[1] - percentiles[0]  # 50th - 16th percentile
    upper_uncertainties = percentiles[2] - percentiles[1]  # 84th - 50th percentile
    param_uncertainties = (lower_uncertainties + upper_uncertainties) / 2  # Symmetric uncertainty

    # Plot corner plot
    if param_names:
        fig = corner.corner(flat_samples, labels=param_names, truths=best_params)

    return best_params, param_uncertainties, flat_samples


def rebin_counts(original_centers, original_counts, new_centers):
    # Calculate bin edges for original bins
    original_widths = np.diff(original_centers, prepend=original_centers[0], append=original_centers[-1])
    original_edges = np.concatenate(([original_centers[0] - original_widths[0] / 2],
                                      original_centers + original_widths[1:] / 2))
    
    # Calculate new bin width
    new_width = new_centers[1] - new_centers[0]
    new_edges = np.concatenate(([new_centers[0] - new_width / 2],
                                 new_centers + new_width / 2))
    
    # Initialize new counts
    new_counts = np.zeros_like(new_centers)
    
    # Redistribute counts
    for i in range(len(new_centers)):
        new_bin_start, new_bin_end = new_edges[i], new_edges[i+1]
        for j in range(len(original_centers)):
            orig_bin_start, orig_bin_end = original_edges[j], original_edges[j+1]
            
            # Calculate overlap between bins
            overlap_start = max(new_bin_start, orig_bin_start)
            overlap_end = min(new_bin_end, orig_bin_end)
            overlap = max(0, overlap_end - overlap_start)
            
            if overlap > 0:
                # Redistribute proportional to overlap
                fraction = overlap / (orig_bin_end - orig_bin_start)
                new_counts[i] += fraction * original_counts[j]
    
    return new_counts

