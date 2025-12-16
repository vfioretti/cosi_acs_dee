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

# Plot spectrum for Am241+Na22 and bkg
fig1 = plt.figure(1, figsize=(12,7))
ax1 = fig1.add_subplot(111)

fig2 = plt.figure(2, figsize=(12,7))
ax2 = fig2.add_subplot(111)

fig3 = plt.figure(3, figsize=(12,7))
ax3 = fig3.add_subplot(111)

fig4 = plt.figure(4, figsize=(12,7))
ax4 = fig4.add_subplot(111)

fig5 = plt.figure(5, figsize=(12,7))
ax5 = fig5.add_subplot(111)

fig6 = plt.figure(6, figsize=(12,7))
ax6 = fig6.add_subplot(111)

# Read files
filename = dir+"/data_Am241.dat"
channel_Am241, counts_Am241, time_Am241 = read_spectrum(filename)

filename = dir+"/data_Na22.dat"
channel_Na22, counts_Na22, time_Na22 = read_spectrum(filename)

filename = dir+"/data_Cs137.dat"
channel_Cs137, counts_Cs137, time_Cs137 = read_spectrum(filename)

filename = dir+"/data_bkg.dat"
channel_bkg, counts_bkg, time_bkg = read_spectrum(filename)

# Rescale counts
counts_Am241 = counts_Am241 * time_bkg / time_Am241
counts_Na22 = counts_Na22 * time_bkg / time_Na22
counts_Cs137 = counts_Cs137 * time_bkg / time_Cs137

#print(f"Total number of channels for Am241&Na22, Cs137 and bkg: {len(channel_Am241)}, {len(channel_Cs137)}, {len(channel_bkg)}")

ax1.step(channel_Am241, counts_Am241, color="k", lw=0.7, label=r"source + bkg")
ax1.step(channel_bkg, counts_bkg, color="c", lw=0.7, label="bkg")

ax2.step(channel_Na22, counts_Na22, color="k",  lw=0.7,label=r"source + bkg")
ax2.step(channel_bkg, counts_bkg, color="c", lw=0.7, label="bkg")

ax3.step(channel_Cs137, counts_Cs137, color="k",  lw=0.7,label=r"source + bkg")
ax3.step(channel_bkg, counts_bkg, color="c", lw=0.7, label="bkg")

counts_Am241_sub = counts_Am241 - counts_bkg
counts_Na22_sub = counts_Na22 - counts_bkg
counts_Cs137_sub = counts_Cs137 - counts_bkg

ax4.step(channel_Am241, counts_Am241_sub, color="k", lw=0.7)

ax5.step(channel_Na22, counts_Na22_sub, color="k", lw=0.7)

ax6.step(channel_Cs137, counts_Cs137_sub, color="k", lw=0.7)

ax1.set_title(r"$^{241}$Am + bkg", fontsize=20)
ax1.set_xlabel("ADC channel", fontsize=20)
ax1.set_ylabel("Counts", fontsize=20)
ax1.set_xlim([0, 250])
ax1.set_ylim(bottom=1)
ax1.set_yscale("log")
ax1.legend(loc="upper right", fontsize=20)
ax1.grid(which="minor", alpha=0.5)
ax1.grid(which="major", alpha=0.5)

ax2.set_title(r"$^{22}$Na + bkg", fontsize=20)
ax2.set_xlabel("ADC channel", fontsize=20)
ax2.set_ylabel("Counts", fontsize=20)
ax2.set_xlim([0, 2047])
ax2.set_ylim(bottom=1, top=1e3)
ax2.set_yscale("log")
ax2.legend(loc="upper right", fontsize=20)
ax2.grid(which="minor", alpha=0.5)
ax2.grid(which="major", alpha=0.5)

ax3.set_title(r"$^{137}$Cs + bkg", fontsize=20)
ax3.set_xlabel("ADC channel", fontsize=20)
ax3.set_ylabel("Counts", fontsize=20)
ax3.set_xlim([0, 1500])
ax3.set_ylim(bottom=1, top=1e4)
ax3.set_yscale("log")
ax3.legend(loc="upper right", fontsize=20)
ax3.grid(which="minor", alpha=0.5)
ax3.grid(which="major", alpha=0.5)

ax4.set_title(r"$^{241}$Am", fontsize=20)
ax4.set_xlabel("ADC channel", fontsize=20)
ax4.set_ylabel("Counts", fontsize=20)
ax4.set_xlim([0, 250])
ax4.set_ylim(bottom=1)
ax4.set_yscale("log")
ax4.legend(loc="upper right", fontsize=20)
ax4.grid(which="minor", alpha=0.5)
ax4.grid(which="major", alpha=0.5)

ax5.set_title(r"$^{22}$Na", fontsize=20)
ax5.set_xlabel("ADC channel", fontsize=20)
ax5.set_ylabel("Counts", fontsize=20)
ax5.set_xlim([0, 2047])
ax5.set_ylim(bottom=1, top=1e3)
ax5.set_yscale("log")
ax5.legend(loc="upper right", fontsize=20)
ax5.grid(which="minor", alpha=0.5)
ax5.grid(which="major", alpha=0.5)

ax6.set_title(r"$^{137}$Cs", fontsize=20)
ax6.set_xlabel("ADC channel", fontsize=20)
ax6.set_ylabel("Counts", fontsize=20)
ax6.set_xlim([0, 1500])
ax6.set_ylim(bottom=1, top=1e4)
ax6.set_yscale("log")
ax6.legend(loc="upper right", fontsize=20)
ax6.grid(which="minor", alpha=0.5)
ax6.grid(which="major", alpha=0.5)

fig1.tight_layout()
fig2.tight_layout()
fig3.tight_layout()
fig4.tight_layout()
fig5.tight_layout()
fig6.tight_layout()

fig1.savefig("Am241_bkg_NRL2.pdf")
fig2.savefig("Na22_bkg_NRL2.pdf")
fig3.savefig("Cs137_bkg_NRL2.pdf")
fig4.savefig("Am241_NRL2.pdf.pdf")
fig5.savefig("Na22_NRL2.pdf.pdf")
fig6.savefig("Cs137_NRL2.pdf")

plt.show()
