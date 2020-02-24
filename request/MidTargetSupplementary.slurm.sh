#!/bin/bash
########## Define Resources Needed with SBATCH Lines ##########
#SBATCH --time=4:00:00
#SBATCH --array=200-399
#SBATCH --mem=2G
#SBATCH --ntasks 1
#SBATCH --cpus-per-task 1
#SBATCH --job-name mid-target
#SBATCH --account=devolab
#SBATCH --output="/mnt/home/mmore500/slurmlogs/slurm-%A_%a.out"
#SBATCH --mail-type=FAIL
#SBATCH --mail-user=mmore500@msu.edu

################################################################################
echo
echo "Setup Exit and Error Traps"
echo "--------------------------"
################################################################################

function on_exit() {

  echo
  echo "Run Exit Trap"
  echo "-------------"

  qstat -f ${SLURM_JOB_ID}

  # prepare python environment
  module purge; module load GCC/7.3.0-2.30 OpenMPI/3.1.1 Python/3.6.6
  source "/mnt/home/mmore500/myPy/bin/activate"

  echo "   SECONDS" $SECONDS
  echo "   MINUTES" $(python3 -c "print( ${SECONDS}/60 )")
  echo "   HOURS  " $(python3 -c "print( ${SECONDS}/3600 )")

  cp ${SLURM_LOGPATH} .

}

function on_error() {

  echo
  echo "Run Error Trap (FAIL)"
  echo "---------------------"

  echo "   EXIT STATUS ${1}"
  echo "   LINE NO ${2}"
  echo "---------------------"
  echo

  cp ${SLURM_LOGPATH} "/mnt/home/mmore500/err_slurmlogs"

  qstat -f ${SLURM_JOB_ID}                                                     \
    >> "/mnt/home/mmore500/err_slurmlogs/${SLURM_LOGFILE}"

  echo "---------------------"
  echo

}

trap 'on_error $? $LINENO' ERR
trap "on_exit" EXIT

################################################################################
echo
echo "Prepare Env Vars"
echo "----------------"
################################################################################

read -r                                                                        \
MUTATION_RATE TARGET_REGULAR_DEGREE TARGET_IRREGULAR_DEGREE TARGET_STRUCTURE   \
<<< $(python3 -c "
import itertools as it
import random
random.seed(1)

mutation_rates = ['0.75', '1.5', '3.0', '6.0', '12.0', ]
target_degrees = [ '1', '2', ]
target_structures = [ 'Regular', 'Irregular', ]
meta_replicates = range(10)

trials = list(
  it.product(mutation_rates, target_degrees, target_structures, meta_replicates)
)

mutation_rate, target_degree, target_structure, __ = next(
  trial for i, trial in enumerate(trials) if i == ${SLURM_ARRAY_TASK_ID} - 200
)
print(
  mutation_rate,
  target_degree if target_structure == 'Regular' else 0,
  target_degree if target_structure == 'Irregular' else 0,
  target_structure
)
")
SEED_OFFSET=1000
SEED=$((SLURM_ARRAY_TASK_ID * 10 + SEED_OFFSET))

OUTPUT_DIR="/mnt/scratch/mmore500/tag-olympics-midtarget/run=${SLURM_ARRAY_TASK_ID}"
CONFIG_DIR="/mnt/home/mmore500/tag-olympics/request/"

echo "   SEED" $SEED
echo "   OUTPUT_DIR" $OUTPUT_DIR
echo "   CONFIG_DIR" $CONFIG_DIR

export SLURM_LOGFILE="slurm-${SLURM_ARRAY_JOB_ID}_${SLURM_ARRAY_TASK_ID}.out"
export SLURM_LOGPATH="/mnt/home/mmore500/slurmlogs/${SLURM_LOGFILE}"

echo "   SLURM_LOGFILE" $SLURM_LOGFILE
echo "   SLURM_LOGPATH" $SLURM_LOGPATH

################################################################################
echo
echo "Setup Work Dir"
echo "--------------"
################################################################################

rm -rf ${OUTPUT_DIR}/* || echo "   not a redo"
mkdir -p ${OUTPUT_DIR}
cp -r ${CONFIG_DIR}/* ${OUTPUT_DIR}
cd ${OUTPUT_DIR}
echo "   PWD" $PWD

################################################################################
echo
echo "Do Work"
echo "-------"
################################################################################

for REP in {0..9}; do

  echo "   REP " $REP

  module purge; module load GCC/7.3.0-2.30 OpenMPI/3.1.1 Python/3.6.6
  source "/mnt/home/mmore500/myPy/bin/activate"

  python3 /mnt/home/mmore500/tag-olympics/script/MidGenerateBiGraph.py         \
    -s $(( $SEED + $REP )) --lefts 16 --rights 16                              \
    --regular $(( $TARGET_REGULAR_DEGREE * 16 ))                               \
    --irregular $(( $TARGET_IRREGULAR_DEGREE * 16 ))                           \

  echo "   RUN EXECUTABLE"

  module purge; module load GCC/8.2.0-2.31.1 OpenMPI/3.1.3 HDF5/1.10.4;

  ./mid-tag-olympics MBM                                                       \
    -SEED $(( $SEED + $REP ))                                                  \
    -MFM_TARGET_DEGREE $(( $TARGET_REGULAR_DEGREE + $TARGET_IRREGULAR_DEGREE ))\
    -MFM_TARGET_STRUCTURE $TARGET_STRUCTURE                                    \
    -MFM_GENS 515 -MO_LENGTH 32 -MO_MUT_EXPECTED_REDRAWS $MUTATION_RATE        \
    >"title=run+rep=${REP}+ext=.log" 2>&1

  echo "   REP COMPLETE"

done

################################################################################
echo
echo "Done! (SUCCESS)"
echo "---------------"
################################################################################
