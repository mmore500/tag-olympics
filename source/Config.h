#pragma once

#include <float.h>
#include <string>

#include "config/config.h"

EMP_BUILD_CONFIG(
  Config,

  GROUP(BASE, "Base settings for all experiments"),
  VALUE(SEED, int, 1, "Random number seed"),
  VALUE(FILENAME, std::string, "configs.cfg", "Default config filename"),

  GROUP(LOW_MUTATIONAL_WALK, "Settings for low-mutational-walk experiment"),
  VALUE(NSTEPS, size_t, 500, "Number of mutational steps in each walk"),
  VALUE(NREPS, size_t, 1, "Number of replicates of each walk"),
  VALUE(NSAMPLES, size_t, 1000, "Number of sample walks to conduct"),

)
