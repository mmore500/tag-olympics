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
  CONST(BS_WIDTH, size_t, 32, "BitSet width?"),

  GROUP(LOW_MUTATIONAL_WALK, "Settings for low-mutational-walk experiment"),
  VALUE(LMW_BITWEIGHT, double, 0.5, "Per-position probability of 1 in a bitstring"),
  VALUE(LMW_NSTEPS, size_t, 65, "Number of mutational steps in each walk"),
  VALUE(LMW_NREPS, size_t, 1, "Number of replicates of each walk"),
  VALUE(LMW_NSAMPLES, size_t, 1000, "Number of sample walks to conduct"),
  VALUE(LMW_TITLE, std::string, "low-mutational-walk", "Default out filename"),

  GROUP(LOW_MUTATIONAL_STEP, "Settings for low-mutational-step experiment"),
  VALUE(LMS_BITWEIGHT, double, 0.5, "Per-position probability of 1 in a bitstring"),
  VALUE(LMS_NSAMPLES, size_t, 5000, "Number of sample walks to conduct"),
  VALUE(LMS_TITLE, std::string, "low-mutational-step", "Default out filename"),

  GROUP(LOW_DIMENSIONALITY, "Settings for dimensionality experiments"),
  VALUE(LD_TITLE, std::string, "low-dimensionality", "TODO"),
  VALUE(LD_BITWEIGHT, double, 0.5, "Per-position probability of 1 in a BitSet"),
  VALUE(LD_NSAMPLES, size_t, 5000, "TODO"),

  GROUP(LOW_REVERSE_DIMENSIONALITY, "Settings for dimensionality experiments"),
  VALUE(LRD_TITLE, std::string, "low-revese-dimensionality", "TODO"),
  VALUE(LRD_BITWEIGHT, double, 0.5, "Per-position probability of 1 in a BitSet"),
  VALUE(LRD_NSAMPLES, size_t, 5000, "TODO"),

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
  VALUE(LTA_NSAMPLES, size_t, 5000, "Number of sample graphs to study"),
  VALUE(LTA_TITLE, std::string, "low-triplet-analysis", "Default out filename"),

  GROUP(LOW_SPECIFICITY_ANALYSIS, "Settings for low-specificty-analysis experiment"),
  VALUE(LSA_BITWEIGHT, double, 0.5, "Per-position probability of 1 in a bitstring"),
  VALUE(LSA_NSAMPLES, size_t, 500, "Number of samples to characterize"),
  VALUE(LSA_NREPS, size_t, 20000, "Number of replicates per sample"),
  VALUE(LSA_TITLE, std::string, "low-specificty-analysis", "Default out filename"),

  GROUP(LOW_SCORE_DISTRIBUTION, "Settings for low-score-distribution experiment"),
  VALUE(LSD_BITWEIGHT, double, 0.5, "Per-position probability of 1 in a bitstring"),
  VALUE(LSD_NSAMPLES, size_t, 5000, "Number of samples to take"),
  VALUE(LSD_TITLE, std::string, "low-score-distribution", "Default out filename"),

  GROUP(MAKE_METRIC_KEY, "Settings for make-metric-key utility"),
  VALUE(MMK_TITLE, std::string, "metric-key", "Default out filename"),

  GROUP(MID_ORGANISM, "Settings for mid-level organism"),
  VALUE(MO_LENGTH, size_t, 48, "TODO"),
  VALUE(MO_MUT_EXPECTED_REDRAWS, double, 1, "TODO"),
  VALUE(MO_MUT_PROB, double, 1.0, "TODO"),
  VALUE(MO_MUT_NORMAL, bool, false, "TODO"),
  VALUE(MO_MUT_NORMAL_SD, double, 1.0, "TODO"),

  GROUP(MID_FLEX_MATCH, "Settings for mid-flex-match experiment"),
  VALUE(MFM_GENS, size_t, 4096, "TODO"),
  VALUE(MFM_NK_K, size_t, 1, "TODO"),
  VALUE(MFM_POP_SIZE, size_t, 500, "TODO"),
  VALUE(MFM_SUBGRID_DIM, size_t, 0, "alt 10 TODO"),
  VALUE(MFM_SUBGRID_TRANSFERS, size_t, 0, "alt 72 TODO"),
  VALUE(MFM_TOURNEY_SIZE, size_t, 7, "TODO"),
  VALUE(MFM_TOURNEY_REPS, size_t, 500, "TODO"),
  VALUE(MFM_ZEROINIT, bool, false, "Should genomes be zero-initialized?"),
  VALUE(
    MFM_COMPONENT_FREQ,
    size_t,
    0,
    "How often to take a modularity census?"
  ),
  VALUE(
    MFM_COMPONENT_WALK_REPS,
    size_t,
    512,
    "How many mutational walk reps to perform?"
  ),
  VALUE(
    MFM_TARGET_MAX,
    double,
    0.0,
    "What should the maximum randomly generated match target be?"
  ),
  VALUE(
    MFM_MATCH_THRESH,
    double,
    0.0,
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
  VALUE(
    MFM_SWAPS_PER_GEN,
    size_t,
    0,
    "How many target edges should be swapped"
    "within each component between each generation?"
  ),
  VALUE(
    MFM_GENS_PER_TARGET,
    size_t,
    0,
    "After how many generations should we switch the target? If 0, never switch."
  ),
  VALUE(
    MFM_NEUT_INTERVAL,
    size_t,
    20,
    "How many often to output data while stepping back through systematics."
  ),
  VALUE(MFM_TITLE, std::string, "mid-flex-match", "Default out filename"),
  VALUE(MFM_TARGET_DEGREE, std::string, "unspecified", "How are target nodes connected to one another?"),
  VALUE(MFM_TARGET_STRUCTURE, std::string, "unspecified", "How are target nodes connected to one another?")

)
