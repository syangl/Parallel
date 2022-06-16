# test.sh
# !/bin/sh
# PBS -N test
# PBS -l nodes=2:ppn=1  
/usr/local/bin/mpiexec -np 2 -machinefile $PBS_NODEFILE /home/s2013458/mpi/mpi-omp-simd-x86
