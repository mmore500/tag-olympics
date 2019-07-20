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
  VALUE(LMW_NSTEPS, size_t, 100, "Number of mutational steps in each walk"),
  VALUE(LMW_NREPS, size_t, 1, "Number of replicates of each walk"),
  VALUE(LMW_NSAMPLES, size_t, 1000, "Number of sample walks to conduct"),
  VALUE(LMW_FILE, std::string, "low-mutational-walk.csv", "Default out filename"),

  GROUP(LOW_GRAPH_ANALYSIS, "Settings for low-graph-analysis experiment"),
  VALUE(LGA_NNODES, size_t, 100, "Number of nodes in each graph"),
  VALUE(LGA_NSAMPLES, size_t, 100, "Number of sample graphs to study"),
  VALUE(LGA_FILE, std::string, "low-graph-analysis.csv", "Default out filename"),

  GROUP(LOW_TRIPLET_ANALYSIS, "Settings for low-triplet-analysis experiment"),
  VALUE(LTA_NSAMPLES, size_t, 2000, "Number of sample graphs to study"),
  VALUE(LTA_FILE, std::string, "low-triplet-analysis.csv", "Default out filename"),

  GROUP(LOW_SPECIFICITY_ANALYSIS, "Settings for low-specificty-analysis experiment"),
  VALUE(LSA_NSAMPLES, size_t, 500, "Number of samples to characterize"),
  VALUE(LSA_NREPS, size_t, 20000, "Number of replicates per sample"),
  VALUE(LSA_FILE, std::string, "low-specificty-analysis.csv", "Default out filename"),

  GROUP(LOW_SCORE_DISTRIBUTION, "Settings for low-score-distribution experiment"),
  VALUE(LSD_NSAMPLES, size_t, 500, "Number of samples to take"),
  VALUE(LSD_FILE, std::string, "low-score-distribution.csv", "Default out filename"),

  GROUP(MAKE_METRIC_KEY, "Settings for make-metric-key utility"),
  VALUE(MMK_FILE, std::string, "metric-key.csv", "Default out filename"),

)
