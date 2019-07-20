#!/bin/bash -l

##############################
#       Job blueprint        #
##############################

# Give your job a name, so you can recognize it in the queue overview
#SBATCH --job-name=LowMutationalWalk

# Define, how many nodes you need. Here, we ask for 1 node.
# Each node has 16 or 20 CPU cores.
#SBATCH --nodes=1
# You can further define the number of tasks with --ntasks-per-*
# See "man sbatch" for details. e.g. --ntasks=4 will ask for 4 cpus.

# Define, how long the job will run in real time. This is a hard cap meaning
# that if the job runs longer than what is written here, it will be
# force-stopped by the server. If you make the expected time too long, it will
# take longer for the job to start. Here, we say the job will take 5 minutes.
#              d-hh:mm:ss
#SBATCH --time=0-48:00:00

# Define the partition on which the job shall run. May be omitted.
##SBATCH --partition normal

# How much memory you need.
# --mem will define memory per node and
# --mem-per-cpu will define memory per CPU/core. Choose one of those.
##SBATCH --mem-per-cpu=1500MB
#SBATCH --mem=8GB

# Turn on mail notification. There are many possible self-explaining values:
# NONE, BEGIN, END, FAIL, ALL (including all aforementioned)
# For more values, check "man sbatch"
#SBATCH --mail-type=END,FAIL

# You may not place any commands before the last SBATCH directive

module purge; module load gcc/8.2.0-2.31.1

# Define and create a unique scratch directory for this job
SCRATCH_DIRECTORY=/mnt/scratch/${USER}/test
mkdir -p ${SCRATCH_DIRECTORY}
cd ${SCRATCH_DIRECTORY}


# You can copy everything you need to the scratch directory
# ${SLURM_SUBMIT_DIR} points to the path where this script was submitted from
# modulea

git clone git@github.com:devosoft/Empirical -b match-bin

git clone git@github.com:mmore500/tag-olympics
cd tag-olympics && make && cp tag-olympics .. && cd ..

./tag-olympics MMK | tee -a log.txt
./tag-olympics LMW | tee -a log.txt
module purge
source ~/pyEnv/bin/activate
time python tag-olympics/script/LowMutationalWalkPlot.py metric-key.csv low-mutational-walk.csv | tee -a log.txt

# After the job is done we copy our output back to $SLURM_SUBMIT_DIR
cp ${SCRATCH_DIRECTORY}/*.pdf ${SLURM_SUBMIT_DIR}

# In addition to the copied files, you will also find a file called
# slurm-1234.out in the submit directory. This file will contain all output that
# was produced during runtime, i.e. stdout and stderr.

# After everything is saved to the home directory, delete the work directory to
# save space on /global/work
#cd ${SLURM_SUBMIT_DIR}
#rm -rf ${SCRATCH_DIRECTORY}

# Finish the script
exit 0
