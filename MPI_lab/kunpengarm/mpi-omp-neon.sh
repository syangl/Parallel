mpicxx -g -Wall -fopenmp -o mpi-omp-simd-neon mpi-omp-simd-neon.cpp 

#qsub-mpi.sh
#!/bin/sh
#PBS -N qsub-mpi
#PBS -l nodes=2
pssh -h $PBS_NODEFILE -i "if [ ! -d \"/home/s2013458/mpi-gauss\" ];then mkdir -p \"/home/s2013458/mpi-gauss\"; fi" 1>&2
pscp -h $PBS_NODEFILE /home/s2013458/mpi-gauss/mpi-omp-simd-neon /home/s2013458/mpi-gauss/ 1>&2
mpiexec -np 2 /home/s2013458/mpi-gauss/mpi-omp-simd-neon