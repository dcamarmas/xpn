#!/bin/bash
#SBATCH --job-name=c3_hdf5_xpn_test

#SBATCH --output=/home/dcamarma/RES/mpi_%j.out
#SBATCH --error=/home/dcamarma/RES/mpi_%j.out

### For exclusive usage ###
#SBATCH --nodes=4
#SBATCH --exclusive
#SBATCH --cpus-per-task=16

#SBATCH --time=02:00:00

echo ${SLURM_JOBID}
echo ${SLURM_NNODES}
echo ${SLURM_JOB_NODELIST}

module load mpich/4.3.0-ofi

XPN_SERVER_DATA="/tmp"
#XPN_SERVER_DATA="/tmp-xpn"
#XPN_SERVER_DATA="/dev/shm"

echo ${SLURM_JOBID}
echo ${SLURM_NNODES}
echo ${SLURM_JOB_NODELIST}

scontrol show hostnames ${SLURM_JOB_NODELIST} > $HOME/tmp/machinefile.${SLURM_JOBID}

echo "[partition]"           >  $HOME/tmp/config.${SLURM_JOBID}.xml
echo "partition_name = xpn"  >> $HOME/tmp/config.${SLURM_JOBID}.xml
echo "bsize = 512k"          >> $HOME/tmp/config.${SLURM_JOBID}.xml
echo "replication_level = 0" >> $HOME/tmp/config.${SLURM_JOBID}.xml
ITER=1
while IFS= read -r line
do
   echo "server_url = mpi_server://$line/$XPN_SERVER_DATA" >> $HOME/tmp/config.${SLURM_JOBID}.xml
   ITER=$((${ITER}+1))
done < $HOME/tmp/machinefile.${SLURM_JOBID}

pkill mpiexec

echo "---------------- SERVER ------------------"

mpiexec -np ${SLURM_NNODES} \
        -f  $HOME/tmp/machinefile.${SLURM_JOBID} \
        $HOME/bin/xpn/bin/xpn_server -s mpi -t pool -d / &

echo "---------------- CLIENT ------------------"

sleep 30

mpiexec -np 1 \
        -f $HOME/tmp/machinefile.${SLURM_JOBID} \
        -genv XPN_CONF $HOME/tmp/config.${SLURM_JOBID}.xml \
        -genv LD_PRELOAD $HOME/bin/xpn/lib/xpn_bypass.so \
        -genv XPN_LOCALITY 1 \
        -genv XPN_THREAD 1 \
        $HOME/bin/hdf5-iotest/bin/hdf5_iotest $HOME/hdf5/hdf5_iotest_strong_20GB.int

sleep 5
pkill mpiexec
