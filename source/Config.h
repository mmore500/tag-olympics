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
  VALUE(LMW_BITWEIGHT, double, 0.5, "Per-position probability of 1 in a bitstring"),
  VALUE(LMW_NSTEPS, size_t, 100, "Number of mutational steps in each walk"),
  VALUE(LMW_NREPS, size_t, 1, "Number of replicates of each walk"),
  VALUE(LMW_NSAMPLES, size_t, 20000, "Number of sample walks to conduct"),
  VALUE(LMW_TITLE, std::string, "low-mutational-walk", "Default out filename"),

  GROUP(LOW_MAKE_GRAPH, "Settings to generate weighted graph from match scores"),
  VALUE(LMG_BITWEIGHT, double, 0.5, "Per-position probability of 1 in a BitSet"),
  VALUE(LMG_NNODES, size_t, 100, "Number of nodes in each graph"),
  VALUE(LMG_NSAMPLES, size_t, 100, "Number of sample graphs to study"),
  VALUE(LMG_TITLE, std::string, "low-graph-analysis", "Default out filename"),

  GROUP(LOW_TRIPLET_ANALYSIS, "Settings for low-triplet-analysis experiment"),
  VALUE(LTA_BITWEIGHT, double, 0.5, "Per-position probability of 1 in a bitstring"),
  VALUE(LTA_NSAMPLES, size_t, 20000, "Number of sample graphs to study"),
  VALUE(LTA_TITLE, std::string, "low-triplet-analysis", "Default out filename"),

  GROUP(LOW_SPECIFICITY_ANALYSIS, "Settings for low-specificty-analysis experiment"),
  VALUE(LSA_BITWEIGHT, double, 0.5, "Per-position probability of 1 in a bitstring"),
  VALUE(LSA_NSAMPLES, size_t, 500, "Number of samples to characterize"),
  VALUE(LSA_NREPS, size_t, 20000, "Number of replicates per sample"),
  VALUE(LSA_TITLE, std::string, "low-specificty-analysis", "Default out filename"),

  GROUP(LOW_SCORE_DISTRIBUTION, "Settings for low-score-distribution experiment"),
  VALUE(LSD_BITWEIGHT, double, 0.5, "Per-position probability of 1 in a bitstring"),
  VALUE(LSD_NSAMPLES, size_t, 500, "Number of samples to take"),
  VALUE(LSD_TITLE, std::string, "low-score-distribution", "Default out filename"),

  GROUP(MAKE_METRIC_KEY, "Settings for make-metric-key utility"),
  VALUE(MMK_TITLE, std::string, "metric-key", "Default out filename"),

)
