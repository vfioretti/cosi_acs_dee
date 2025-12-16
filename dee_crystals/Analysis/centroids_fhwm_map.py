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

# Import the input parameters
arg_list = sys.argv
yaml_file = arg_list[1]

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
NbinZ = params['NbinZ']
x_to_megalib = params['x_to_megalib']
y_to_megalib = params['y_to_megalib']
z_to_megalib = params['z_to_megalib']
xlen = params['x_len_mm']
ylen = params['y_len_mm']
E_min = params['E_min_keV']
E_max = params['E_max_keV']
num_bins = params['num_bins']

# Position bins
edgesX = np.linspace(-xlen/2, xlen/2, NbinX + 1)
edgesY = np.linspace(-ylen/2, ylen/2, NbinY + 1)
centersX = (edgesX[:-1] + edgesX[1:]) / 2
centersY = (edgesY[:-1] + edgesY[1:]) / 2
# Energy bins
edgesE = np.logspace(np.log10(E_min), np.log10(E_max), num_bins + 1)
centersE = np.sqrt(edgesE[:-1] * edgesE[1:])

x_to_megalib, y_to_megalib, z_to_megalib = np.array(x_to_megalib), np.array(y_to_megalib), np.array(z_to_megalib)
index_x, index_y, index_z = np.where(x_to_megalib != 0)[0], np.where(y_to_megalib != 0)[0], np.where(z_to_megalib != 0)[0]
if len(index_x) != 1 or len(index_y) != 1 or len(index_z) != 1: raise ValueError('x_to_megalib, y_to_megalib and z_to_megalib must be 1D arrays with only one non-zero element each.')
elif len({index_x[0], index_y[0], index_z[0]}) != 3: raise ValueError('x_to_megalib, y_to_megalib and z_to_megalib must be 1D arrays having the non-zero element in different positions.')
else: index_x, index_y, index_z = index_x[0], index_y[0], index_z[0]
swap_x = False
if x_to_megalib[index_x] < 0: swap_x = True

plots_dir = f'./plots_{name}/'

posID, vx, vy, vz, x, y, Etrue, Emeas, fwhm, Emeas_err, fwhm_err = np.genfromtxt("response_"+name+".dat", unpack=True)

if index_z == 0: cond = vx == 0
elif index_z == 1: cond = vy == 0
else: cond = vz == 0
posID, vx, vy, vz, x, y, Etrue, Emeas, fwhm, Emeas_err, fwhm_err = posID[cond], vx[cond], vy[cond], vz[cond], x[cond], y[cond], Etrue[cond], Emeas[cond], fwhm[cond], Emeas_err[cond], fwhm_err[cond]

fig1 = plt.figure(1, figsize=(10,7))
ax1 = fig1.add_subplot(111)

fig2 = plt.figure(2, figsize=(10,7))
ax2 = fig2.add_subplot(111)

colors = plt.cm.viridis(np.linspace(0, 1, NbinX*NbinY))
norm = Normalize(vmin=0, vmax=NbinX*NbinY)

i = 0
for pos in np.unique(posID):
    Etrue_pos = Etrue[posID == pos]
    Emeas_pos = Emeas[posID == pos]
    fwhm_pos = fwhm[posID == pos]
    Emeas_pos_err = Emeas_err[posID == pos]
    fwhm_pos_err = fwhm_err[posID == pos]

    resol = fwhm_pos*100/Emeas_pos
    resol_err = np.sqrt(100**2*((fwhm_pos_err/Emeas_pos)**2 + (fwhm_pos*Emeas_pos_err/Emeas_pos**2)**2))
    x_err = (edgesE[1:] - edgesE[:-1])/2
    centroids = ax1.errorbar(Etrue_pos, Emeas_pos, xerr=x_err, yerr=Emeas_pos_err, fmt='.', label=f"pos. {pos}", color=colors[i])
    resolutions = ax2.errorbar(Etrue_pos, resol, xerr=x_err, yerr=resol_err, fmt='.', label=f"pos. {pos}", color=colors[i])

    i += 1

sm1 = ScalarMappable(cmap='viridis', norm=norm)
sm1.set_array([])
cbar1 = plt.colorbar(sm1, ax=ax1, orientation='vertical')
cbar1.set_label('Spatial bins')
sm2 = ScalarMappable(cmap='viridis', norm=norm)
sm2.set_array([])
cbar2 = plt.colorbar(sm2, ax=ax2, orientation='vertical')
cbar2.set_label('Spatial bins')

ax1.plot(Etrue_pos, Etrue_pos, lw=0.7)

ax1.set_xlabel(r"$E_{true}$ [keV]")
ax1.set_ylabel(r"$E_{meas}$ [keV]")
ax1.set_ylim([10, 12000])
ax1.set_xscale("log")
ax1.set_yscale("log")
#ax1.legend()
fig1.tight_layout()

ax2.set_xlabel(r"$E_{true}$ [keV]")
ax2.set_ylabel(r"Resolution (%)")
ax2.set_ylim([0,100])
ax2.set_xscale("log")
#ax2.legend()
fig2.tight_layout()

fig1.savefig(os.path.join(plots_dir,'centrShift.pdf'))
fig2.savefig(os.path.join(plots_dir,'resolution.pdf'))

for energy_bin in range(len(centersE)):
    fig3 = plt.figure(3, figsize=(10,7))
    ax3 = fig3.add_subplot(111)

    fig4 = plt.figure(4, figsize=(10,7))
    ax4 = fig4.add_subplot(111)

    centrShift_map = []
    fwhm_map = []
    energy_center = centersE[energy_bin]
    pos = 0
    for y_min, y_max, cY in zip(edgesY[:-1], edgesY[1:], centersY):
        centr_array, fwhm_array = [], []
        #if sym == 1 and pos >= NbinX*NbinY/2: break
        for x_min, x_max, cX in zip(edgesX[:-1], edgesX[1:], centersX):
            cond_pos = (x_min < x) & (x < x_max) & (y_min < y) & (y < y_max)
            posID_pos = posID[cond_pos]
            centr_array.append((Emeas[cond_pos][energy_bin]-energy_center)*100/energy_center)
            fwhm_array.append(fwhm[cond_pos][energy_bin])

            pos += 1

            #if posID_pos[0] == 94:
                #print(centr_array[-1], fwhm_array[-1])
        
        centrShift_map.append(centr_array)
        fwhm_map.append(fwhm_array)
    centrShift_map, fwhm_map = np.array(centrShift_map), np.array(fwhm_map)
    resolution_map = fwhm_map*100/(centrShift_map*energy_center/100 + energy_center)
    map = ax3.imshow(centrShift_map, extent = (edgesX[0], edgesX[-1], edgesY[0], edgesY[-1]), cmap = 'seismic', norm=TwoSlopeNorm(vmin=-centrShift_map.max(),vcenter=0, vmax=centrShift_map.max()))
    divider = make_axes_locatable(ax3)
    cax = divider.append_axes("right", size="5%", pad=0.1)
    colorbar = fig3.colorbar(map, cax=cax)
    colorbar.set_label(r'Centroid shift (%) @ '+str(round(energy_center,1))+' keV', fontsize = 20)
    colorbar.ax.tick_params(labelsize=15)
    map = ax4.imshow(resolution_map, extent = (edgesX[0], edgesX[-1], edgesY[0], edgesY[-1]), origin = 'lower', cmap = 'viridis')
    divider = make_axes_locatable(ax4)
    cax = divider.append_axes("right", size="5%", pad=0.1)
    colorbar = fig4.colorbar(map, cax=cax)
    colorbar.set_label(r'Resolution (%) @ '+str(round(energy_center,1))+' keV', fontsize = 20)
    colorbar.ax.tick_params(labelsize=15)

    ax3.set_xlabel("x[mm]", fontsize = 20)
    ax3.set_ylabel("y[mm]", fontsize = 20)
    ax4.set_xlabel("x[mm]", fontsize = 20)
    ax4.set_ylabel("y[mm]", fontsize = 20)
    ax3.tick_params(axis='both', which='major', labelsize=15)
    ax3.tick_params(axis='both', which='minor', labelsize=15)
    ax4.tick_params(axis='both', which='major', labelsize=15)
    ax4.tick_params(axis='both', which='minor', labelsize=15)
    fig3.tight_layout()
    fig4.tight_layout()

    fig3.savefig(os.path.join(plots_dir,'centrShift_map_'+str(round(energy_center,1))+'.pdf'))
    fig4.savefig(os.path.join(plots_dir,'resolution_map_'+str(round(energy_center,1))+'.pdf'))

    plt.show()


### Fit of centroids and FWHMs ###

def lin(x, m, q):
    return m * x + q

def w_func(x, b, c):
    a = 21
    return np.sqrt(a**2 + b**2*x + c**2*x**2)

fig1 = plt.figure(1, figsize=(10,7))
ax1 = fig1.add_subplot(111)

fig2 = plt.figure(2, figsize=(10,7))
ax2 = fig2.add_subplot(111)

fig3 = plt.figure(3, figsize=(10,7))
ax3 = fig3.add_subplot(111)

fig4 = plt.figure(4, figsize=(10,7))
ax4 = fig4.add_subplot(111)

fig5 = plt.figure(5, figsize=(10,7))
ax5 = fig5.add_subplot(111)

fig6 = plt.figure(6, figsize=(10,7))
ax6 = fig6.add_subplot(111)

fig7 = plt.figure(7, figsize=(10,7))
ax7 = fig7.add_subplot(111)

fig8 = plt.figure(8, figsize=(10,7))
ax8 = fig8.add_subplot(111)

fig9 = plt.figure(9, figsize=(10,7))
ax9 = fig9.add_subplot(111)

posID_fit, vx_fit, vy_fit, vz_fit, x_fit, y_fit, m_fit, q_fit, a_fit, b_fit, c_fit = np.genfromtxt(f'correction_file_{name}.dat', unpack=True)

if index_z == 0: cond = vx_fit == 0
elif index_z == 1: cond = vy_fit == 0
else: cond = vz_fit == 0
posID_fit, vx_fit, vy_fit, vz_fit, x_fit, y_fit, m_fit, q_fit, a_fit, b_fit, c_fit = posID_fit[cond], vx_fit[cond], vy_fit[cond], vz_fit[cond], x_fit[cond], y_fit[cond], m_fit[cond], q_fit[cond], a_fit[cond], b_fit[cond], c_fit[cond]

i = 0
m = []
q = []
m_map = []
q_map = []
a_map = []
b_map = []
c_map = []
for y_min, y_max, cY in zip(edgesY[:-1], edgesY[1:], centersY):
    m_array = []
    q_array = []
    a_array = []
    b_array = []
    c_array = []
    for x_min, x_max, cX in zip(edgesX[:-1], edgesX[1:], centersX):
        cond_pos = (x_min < x_fit) & (x_fit < x_max) & (y_min < y_fit) & (y_fit < y_max)
        cond_pos2 = (x_min < x) & (x < x_max) & (y_min < y) & (y < y_max)
        popt_centroids = [m_fit[cond_pos][0], q_fit[cond_pos][0]]
        popt_fwhm = [b_fit[cond_pos][0], c_fit[cond_pos][0]]
        m_array.append(m_fit[cond_pos][0])
        q_array.append(q_fit[cond_pos][0])
        a_array.append(a_fit[cond_pos][0])
        b_array.append(b_fit[cond_pos][0])
        c_array.append(c_fit[cond_pos][0])        
        Etrue_pos = Etrue[cond_pos2]
        Emeas_pos = Emeas[cond_pos2]
        Emeas_err_pos = Emeas_err[cond_pos2]
        fwhm_pos = fwhm[cond_pos2]
        fwhm_err_pos = fwhm_err[cond_pos2]


        x_err = (edgesE[1:] - edgesE[:-1])/2
        centroids = ax1.errorbar(Etrue_pos, Emeas_pos, xerr=x_err, yerr=Emeas_pos_err, fmt='.', color=colors[i], alpha=0.2)
        energy_fit = np.arange(10, edgesE[-1], 0.1)
        ax1.plot(energy_fit, lin(energy_fit, *popt_centroids), lw=0.7, color=centroids[0].get_color())

        resol = fwhm_pos*100/Emeas_pos
        resol_err = np.sqrt(100**2*((fwhm_err_pos/Emeas_pos)**2 + (fwhm_pos*Emeas_err_pos/Emeas_pos**2)**2))
        x_err = (edgesE[1:] - edgesE[:-1])/2
        resolution = ax6.errorbar(Etrue_pos, resol, xerr=x_err, yerr=resol_err, fmt='.', color=colors[i], alpha=0.2)
        energy_fit = np.arange(10, edgesE[-1], 0.1)
        ax6.plot(energy_fit, w_func(energy_fit, *popt_fwhm)*100/energy_fit, lw=0.7, color=resolution[0].get_color())

        i += 1

    m_map.append(m_array)
    q_map.append(q_array)
    a_map.append(a_array)
    b_map.append(b_array)
    c_map.append(c_array)

sm1 = ScalarMappable(cmap='viridis', norm=norm)
sm1.set_array([])
cbar1 = plt.colorbar(sm1, ax=ax1, orientation='vertical')
cbar1.set_label('Spatial bins', fontsize=20)
cbar1.ax.tick_params(labelsize=15)

sm6 = ScalarMappable(cmap='viridis', norm=norm)
sm6.set_array([])
cbar6 = plt.colorbar(sm6, ax=ax6, orientation='vertical')
cbar6.set_label('Spatial bins', fontsize=20)
cbar6.ax.tick_params(labelsize=15)

ax1.set_xlabel(r"$E_{true}$ [keV]", fontsize=20)
ax1.set_ylabel(r"$E_{meas}$ [keV]", fontsize=20)
ax1.set_xlim([edgesE[0], edgesE[-1]])
ax1.set_ylim([10, 12000])
ax1.set_xscale("log")
ax1.set_yscale("log")
ax1.tick_params(axis='both', which='major', labelsize=15)
ax1.tick_params(axis='both', which='minor', labelsize=15)
fig1.tight_layout()
fig1.savefig(os.path.join(plots_dir,'fit_centroids_DEE.pdf'))

ax6.set_xlabel(r"$E_{true}$ [keV]", fontsize=20)
ax6.set_ylabel(r"Resolution (%)", fontsize=20)
ax6.set_xlim([edgesE[0], edgesE[-1]])
ax6.set_ylim(bottom=0)
ax6.set_xscale("log")
ax6.tick_params(axis='both', which='major', labelsize=15)
ax6.tick_params(axis='both', which='minor', labelsize=15)
fig6.tight_layout()
fig6.savefig(os.path.join(plots_dir,'fit_resolution_DEE.pdf'))

ax2.hist(m, 10)
ax3.hist(q, 10)

ax2.set_xlabel(r"Angular coefficients")
ax2.set_ylabel(r"Counts")
ax3.set_xlabel(r"Intercepts")
ax3.set_ylabel(r"Counts")

map = ax4.imshow(m_map, extent = (edgesX[0], edgesX[-1], edgesY[0], edgesY[-1]), origin = 'lower', cmap = 'viridis')
colorbar = fig4.colorbar(map, ax=ax4)
colorbar.set_label(r'Angular coefficients')

map = ax5.imshow(q_map, extent = (edgesX[0], edgesX[-1], edgesY[0], edgesY[-1]), origin = 'lower', cmap = 'viridis')
colorbar = fig5.colorbar(map, ax=ax5)
colorbar.set_label(r'Intercepts')

map = ax7.imshow(a_map, extent = (edgesX[0], edgesX[-1], edgesY[0], edgesY[-1]), origin = 'lower', cmap = 'viridis')
colorbar = fig7.colorbar(map, ax=ax7)
colorbar.set_label(r'a')

map = ax8.imshow(b_map, extent = (edgesX[0], edgesX[-1], edgesY[0], edgesY[-1]), origin = 'lower', cmap = 'viridis')
colorbar = fig8.colorbar(map, ax=ax8)
colorbar.set_label(r'b')

map = ax9.imshow(c_map, extent = (edgesX[0], edgesX[-1], edgesY[0], edgesY[-1]), origin = 'lower', cmap = 'viridis')
colorbar = fig9.colorbar(map, ax=ax9)
colorbar.set_label(r'c')

ax5.set_xlabel("x[mm]")
ax5.set_ylabel("y[mm]")
ax7.set_xlabel("x[mm]")
ax7.set_ylabel("y[mm]")
ax8.set_xlabel("x[mm]")
ax8.set_ylabel("y[mm]")
ax9.set_xlabel("x[mm]")
ax9.set_ylabel("y[mm]")

plt.show()