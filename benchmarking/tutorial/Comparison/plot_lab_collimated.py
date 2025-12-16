# Plot spectra vs channels

import numpy as np
import math
import sys, os
import matplotlib.pyplot as plt

from matplotlib import gridspec
from matplotlib import cm
import matplotlib as mpl
from mpl_toolkits.axes_grid1 import make_axes_locatable
from astropy.table import Table, Column
from astropy import units as u
from scipy.optimize import curve_fit

from functions import *
from models import *

# Import the input parameters
arg_list = sys.argv
dir = arg_list[1]

fig1 = plt.figure(1, figsize=(10,7))
ax1 = fig1.add_subplot(111)

fig2 = plt.figure(2, figsize=(10,7))
ax2 = fig2.add_subplot(111)


# Read background
file = dir+"/data_bkg.dat"
channel_bkg, counts_bkg, time_bkg = read_spectrum(file)

colors = [
    "#E6194B",  # Red  
    "#3CB44B",  # Green  
    "#FFE119",  # Yellow  
    "#4363D8",  # Blue  
    "#F58231",  # Orange  
    "#911EB4",  # Purple  
    "#46F0F0",  # Cyan  
    "#F032E6",  # Magenta  
    "#BCF60C",  # Lime  
    "#FABEBE",  # Pink  
    "#008080",  # Teal  
    "#E6BEFF"   # Lavender  
]
response = []
responseErr = []
#fact = [1.7, 1.3, 1.5, 1.35, 1.2, 1.5, 1.8, 1.5, 1.5]
for pos in range(1, 13):
    file = dir+"/data_Am241_pos"+str(pos)+".dat"
    channel, counts, time = read_spectrum(file)
    # Rescale counts
    counts = counts * time_bkg / time

    # Subtract background
    counts_sub = counts - counts_bkg
    counts_sub[counts_sub < 0] = 0

    ax1.step(channel, counts_sub, lw=1.2, color=colors[pos-1], label="Pos. "+str(pos))

response = []
responseErr = []
for pos in range(1, 13):
    file = dir+"/data_Cs137_pos"+str(pos)+".dat"
    channel, counts, time = read_spectrum(file)
    # Rescale counts
    counts = counts * time_bkg / time

    # Subtract background
    counts_sub = counts - counts_bkg
    counts_sub[counts_sub < 0] = 0

    ax2.step(channel, counts_sub, lw=1.2, color=colors[pos-1], label="Pos. "+str(pos))

ax1.set_title("Am241 (60 keV)", fontsize=20)
ax1.set_xlabel("ADC channel", fontsize=20)
ax1.set_ylabel("Counts", fontsize=20)
ax1.set_xlim([30, 90])
ax1.set_ylim([0, 10000])
#ax1.set_yscale("log")
ax1.legend(loc="upper right", fontsize=20)
ax1.grid(which="minor", alpha=0.5)
ax1.grid(which="major", alpha=0.5)
ax1.tick_params(axis='both', which='major', labelsize=15)
ax1.tick_params(axis='both', which='minor', labelsize=15)

ax2.set_title("Cs137 (662 keV)", fontsize=20)
ax2.set_xlabel("ADC channel", fontsize=20)
ax2.set_ylabel("Counts", fontsize=20)
ax2.set_xlim([500, 800])
ax2.set_ylim([0, 1400])
#ax2.set_yscale("log")
ax2.legend(loc="upper right", fontsize=20)
ax2.grid(which="minor", alpha=0.5)
ax2.grid(which="major", alpha=0.5)
ax2.tick_params(axis='both', which='major', labelsize=15)
ax2.tick_params(axis='both', which='minor', labelsize=15)

fig1.tight_layout()
fig2.tight_layout()

fig1.savefig("Am241_pos_NRL2.pdf")
fig2.savefig("Cs137_pos_NRL2.pdf")

plt.show()
