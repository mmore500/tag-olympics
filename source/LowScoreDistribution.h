#pragma once

#include <iostream>
#include <limits>

#include "tools/MatchBin.h"
#include "tools/matchbin_utils.h"
#include "tools/Random.h"
#include "config/ArgManager.h"
#include "data/DataFile.h"
#include "tools/string_utils.h"

#include "Config.h"
#include "Metrics.h"

void LowScoreDistribution(const Metrics &metrics, const Config &cfg) {

  emp::Random rand(cfg.SEED());

  size_t s;
  std::string name;
  double score;

  emp::DataFile df(
    std::string()
    + "bitweight="
    + emp::to_string(cfg.LSD_BITWEIGHT())
    + "title="
    + cfg.LSD_TITLE()
    + "+"
    + "seed="
    + emp::to_string(cfg.SEED())
    // + "+"
    // + "_emp_hash="
    // + STRINGIFY(EMPIRICAL_HASH_)
    // + "+"
    // + "_source_hash="
    // + STRINGIFY(DISHTINY_HASH_)
    + "+"
    + "ext="
    + ".csv"
  );
  df.AddVar(s, "Sample");
  df.AddVar(name, "Metric");
  df.AddVar(score, "Match Score");
  df.PrintHeaderKeys();

  for(s = 0; s < cfg.LSD_NSAMPLES(); ++s) {

    emp::BitSet<32> bs_a(rand, cfg.LSD_BITWEIGHT());
    emp::BitSet<32> bs_b(rand, cfg.LSD_BITWEIGHT());

    for (const auto & mptr : metrics.mets) {
      const auto & metric = *mptr;

      // filter out non-interesting data
      if (metric.name().find("Inverse") != std::string::npos) continue;

      name = metric.name() + " Distance";
      score = metric(bs_a, bs_b);
      df.Update();
    }
  }

}
