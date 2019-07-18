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

  emp::DataFile df(cfg.LSA_FILE());
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

      emp::BitSet<32> samp(rand);
      // emp::BitSet<32> samp_w(rand, 0.75);

      for(size_t r = 0; r < cfg.LSA_NREPS(); ++r) {

        emp::BitSet<32> rep(rand);
        // emp::BitSet<32> rep_w(rand, 0.75);

        match_node.Add(metric(samp, rep));

      }

      df.Update();

    }
  }
}
