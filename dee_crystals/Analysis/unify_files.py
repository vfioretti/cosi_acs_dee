import numpy as np

panels = ["BGO_X0_0", "BGO_X0_1", "BGO_X0_2", 
         "BGO_Y0_0", "BGO_Y0_1", "BGO_Y0_2",
         "BGO_Z0_0", "BGO_Z0_1", "BGO_Z0_2", "BGO_Z0_3", "BGO_Z0_4",
         "BGO_Z1_0", "BGO_Z1_1", "BGO_Z1_2", "BGO_Z1_3", "BGO_Z1_4"]

files = ["correction_file_" + panel + ".dat" for panel in panels]

with open("ACS_correction_file.dat", "w") as f:
    f.write("# DetectorName voxel_X voxel_Y voxel_Z x[mm] y[mm] m q a b c\n")


for file, panel in zip(files, panels):
    posID, vX, vY, vZ, x, y, m, q, a, b, c = np.genfromtxt(file, unpack=True)
    
    vX = vX.astype(int)
    vY = vY.astype(int)
    vZ = vZ.astype(int)

    with open("ACS_correction_file.dat", "a") as f:
        for i in range(len(posID)):
            f.write(f'{panel} ')
            f.write('{:10.0f}'.format(vX[i]))
            f.write('{:10.0f}'.format(vY[i]))
            f.write('{:10.0f}'.format(vZ[i]))
            f.write('{:10.2f}'.format(x[i]))
            f.write('{:10.2f}'.format(y[i]))
            f.write('{:10.2f}'.format(m[i]))
            f.write('{:10.2f}'.format(q[i]))
            f.write('{:10.2f}'.format(a[i]))
            f.write('{:10.2f}'.format(b[i]))
            f.write('{:10.2f}'.format(c[i]))
            f.write('\n')




