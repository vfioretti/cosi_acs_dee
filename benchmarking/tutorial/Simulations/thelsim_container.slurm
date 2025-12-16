#!/bin/bash

##Indicazione del Walltime
#SBATCH --time=240:00:00
#SBATCH --nodes=1
##SBATCH --sockets-per-node=1
##Numero di processi totali che volete per mpi
##SBATCH -n 4
##Questa istruzione vi riporta praticamente il numero di threads, ovvero il numero di core che assegnate al processo praticamente dipende da quanto e' parallelizzato il codice internamente ad esempio in openmp. Con questo scheduler dovete specificare anche che a ogni thread date un core.
##SBATCH --cores-per-socket=2
##SBATCH --threads-per-core=1
##questo e' il nome della partizione dove runnate che nel nostro caso large
#SBATCH -p moon
##Nome job e file di output ed error
#SBATCH --job-name=Cs137t
#SBATCH --err=run.err
#SBATCH --out=run.out

## [OPZIONALE] per attivare l'invio di e-mail alla fine del run, aggiungere allo script
##SBATCH --mail-type=ALL
##SBATCH --mail-user=alex.ciabattoni2@unibo.it


date
 
singularity exec --bind=/blasco /home/ciabattoni/containers/g4_11_1_HPC.sif mpiexec -n 1 /home/ciabattoni/ICSC_G4_HPC/BoGEMMS-HPC/BoGEMMS-HPC-build/bogemms SiPM.conf 0 Beam.mac
