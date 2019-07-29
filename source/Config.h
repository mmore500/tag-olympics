#pragma once

#include <float.h>
#include <string>

#include "config/config.h"

EMP_BUILD_CONFIG(
  Config,

  GROUP(BASE, "Base settings for all experiments"),
  VALUE(SEED, int, 1, "Random number seed"),
  VALUE(FILENAME, std::string, "configs.cfg", "Default config filename"),
  VALUE(TREATMENT, std::string, "unspecified", "Treatment specifier"),

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

  GROUP(LOW_MAKE_DIGRAPH, "Settings to generate weighted directed graph from match scores"),
  VALUE(LMD_BITWEIGHT, double, 0.5, "Per-position probability of 1 in a BitSet"),
  VALUE(LMD_NNODES, size_t, 50, "Number of nodes in each graph"),
  VALUE(LMD_NSAMPLES, size_t, 100, "Number of sample graphs to study"),
  VALUE(LMD_TITLE, std::string, "low-digraph-analysis", "Default out filename"),

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

  GROUP(MID_ORGANISM, "Settings for mid-level organism"),
  VALUE(MO_LENGTH, size_t, 48, "TODO"),
  VALUE(MO_BITWEIGHT, double, 0.5, "TODO"),
  VALUE(MO_MUT_BIT_REDRAW_PER_BIT, double, 0.001, "TODO"),

  GROUP(MID_FLEX_MATCH, "Settings for mid-flex-match experiment"),
  VALUE(MFM_GENS, size_t, 100, "TODO"),
  VALUE(MFM_POP_SIZE, size_t, 100, "TODO"),
  VALUE(MFM_TOURNEY_SIZE, size_t, 2, "TODO"),
  VALUE(MFM_TOURNEY_REPS, size_t, 4000, "TODO"),
  VALUE(
    MFM_TARGET_MAX,
    double,
    0.0,
    "What should the maximum randomly generated match target be?"
  ),
  VALUE(
    MFM_MATCH_THRESH,
    double,
    0.25,
    "Any match score under this threshold is considered a perfect match"
  ),
  VALUE(
    MFM_NOMATCH_THRESH,
    double,
    0.5,
    "Any match score under this threshold is considered a perfect non-match"
  ),
  VALUE(
    MFM_RANKED,
    bool,
    true,
    "Should fitness be evaluated by tag match ranking instead of by match score?"
  ),
  VALUE(MFM_TITLE, std::string, "mid-flex-match", "Default out filename")

)
