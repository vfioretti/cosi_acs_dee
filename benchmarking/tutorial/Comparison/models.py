import math
import numpy as np

def gaus(x, a, w, xc):
    if a < 0:
        a = 0
    return a/w * math.sqrt(4*math.log(2)/math.pi)*np.exp(-4*math.log(2)*(x-xc)**2/w**2)

def double_gaus(x, a1, w1, xc1, a2, w2, xc2):
    return gaus(x, a1, w1, xc1) + gaus(x, a2, w2, xc2)

def double_gaus_lin(x, a1, w1, xc1, a2, w2, xc2, m, q):
    return gaus(x, a1, w1, xc1) + gaus(x, a2, w2, xc2) + lin(x, m, q)

def quad(x, a, b, c):
    return a*x**2 + b*x + c

def gaus_quad(x, a, w, xc, a1, a2, a3):
    return gaus(x, a, w, xc) + quad(x, a1, a2, a3)

def gaus_lin(x, a, w, xc, m, q):
    return gaus(x, a, w, xc) + lin(x, m, q)

def four_gaus(x, a, w1, xc, m, q):
    return gaus(x,a*0.14,w1,0.77*xc) + gaus(x,a*0.31,w1,0.85*xc) + gaus(x,a,w1,xc) + gaus(x,a*0.15,w1,1.08*xc) + lin(x, m, q)

def lin(x, m, q):
    return m*x + q

def w_func(x, a, b, c):
    return np.sqrt(a**2 + b**2*x + c**2*x**2)

def get_model(model):
    if model == "gaus":
        return gaus
    if model == "double_gaus":
        return double_gaus
    if model == double_gaus_lin:
        return double_gaus_lin
    if model == "gaus_lin":
        return gaus_lin
    if model == "four_gaus":
        return four_gaus
    if model == "double_gaus_lin":
        return double_gaus_lin
    return None

def plot_model(ax, x, model, popt):
    if model == double_gaus:
        ax.plot(x, double_gaus(x, *popt), color="r", lw=2, zorder=3, label="Total fit")
        ax.plot(x, gaus(x, popt[0], popt[1], popt[2]), lw=1, zorder=3, label="Photopeak")
        ax.plot(x, gaus(x, popt[3], popt[4], popt[5]), ls='--', lw=1, zorder=3, label="Non-photopeak")
    if model == gaus_lin:
        ax.plot(x, gaus_lin(x, *popt), color="r", lw=2, zorder=3, label="Total fit")
        ax.plot(x, gaus(x, popt[0], popt[1], popt[2]), lw=1, zorder=3, label="Gaussian")
        ax.plot(x, lin(x, popt[3], popt[4]), lw=1, zorder=3, label="Linear")
    if model == four_gaus:
        ax.plot(x, four_gaus(x, *popt), color="r", lw=2, zorder=3, label="Total fit")
        gaus1, = ax.plot(x, gaus(x, popt[0]*0.14, popt[1], 0.77*popt[2]), lw=1, zorder=3, label="Gaussian")
        ax.plot(x, gaus(x, popt[0]*0.31, popt[1], 0.85*popt[2]), color=gaus1.get_color(), lw=1, zorder=3)
        ax.plot(x, gaus(x, popt[0], popt[1], popt[2]), color=gaus1.get_color(), lw=1, zorder=3)
        ax.plot(x, gaus(x, popt[0]*0.15, popt[1], 1.08*popt[2]), color=gaus1.get_color(), lw=1, zorder=3)
        ax.plot(x, lin(x, popt[3], popt[4]), ls='--', lw=1, zorder=3, label="Linear")
    if model == double_gaus_lin:
        ax.plot(x, double_gaus_lin(x, *popt), color="r", lw=2, zorder=3, label="Total fit")
        gaus1, = ax.plot(x, gaus(x, popt[0], popt[1], popt[2]), lw=1, zorder=3, label="Gaussian")
        ax.plot(x, gaus(x, popt[3], popt[4], popt[5]), color=gaus1.get_color(), lw=1, zorder=3)
        ax.plot(x, lin(x, popt[6], popt[7]), lw=1, zorder=3, label="Linear")