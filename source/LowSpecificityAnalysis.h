#pragma once

#include <iostream>
#include <limits>

#include "base/vector.h"
#include "tools/MatchBin.h"
#include "tools/matchbin_utils.h"
#include "tools/Random.h"
#include "config/ArgManager.h"
#include "data/DataFile.h"
#include "data/DataNode.h"
#include "tools/string_utils.h"

#include "Config.h"
#include "Metrics.h"

void LowSpecificityAnalysis(const Metrics &metrics, const Config &cfg) {

  emp::Random rand(cfg.SEED());

  size_t s;
  std::string name;
  emp::DataNode<
    double,
    emp::data::Range
  > match_node;

  emp::DataFile df(
    std::string()
    + "bitweight="
    + emp::to_string(cfg.LSA_BITWEIGHT())
    + "+"
    + "title="
    + cfg.LSA_TITLE()
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
  df.AddMean(match_node, "Tag Mean Match Score", "TODO", true);
  df.PrintHeaderKeys();

  for (const auto & mptr : metrics.mets) {
    const auto & metric = *mptr;

    // filter out non-interesting data
    if (metric.name().find("Inverse") != std::string::npos) continue;

    name = metric.name() + " Distance";
    std::cout << "metric " << name << std::endl;

    for(s = 0; s < cfg.LSA_NSAMPLES(); ++s) {

      emp::BitSet<32> samp(rand, cfg.LSA_BITWEIGHT());

      for(size_t r = 0; r < cfg.LSA_NREPS(); ++r) {

        emp::BitSet<32> rep(rand, cfg.LSA_BITWEIGHT());

        match_node.Add(metric(samp, rep));

      }

      df.Update();

    }
  }
}
