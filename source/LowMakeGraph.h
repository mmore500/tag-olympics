#pragma once

#include <iostream>
#include <limits>

#include "tools/MatchBin.h"
#include "tools/matchbin_utils.h"
#include "tools/Random.h"
#include "config/ArgManager.h"
#include "data/DataFile.h"

#include "Config.h"
#include "Metrics.h"

void LowMakeGraph(const Metrics &metrics, const Config &cfg) {

  emp::Random rand(cfg.SEED());

  size_t s;
  size_t from;
  size_t to;
  std::string name;
  double match;

  emp::DataFile df(cfg.LMG_FILE());
  df.AddVar(s, "Sample");
  df.AddVar(from, "From");
  df.AddVar(to, "To");
  df.AddVar(name, "Metric");
  df.AddVar(match, "Match Score");
  df.PrintHeaderKeys();

  for(s = 0; s < cfg.LMG_NSAMPLES(); ++s) {
    std::cout << "sample " << s << std::endl;

    emp::vector<emp::BitSet<32>> bs;
    bs.reserve(cfg.LMG_NNODES());

    for(size_t n = 0; n < cfg.LMG_NNODES(); ++n) {
      bs.emplace_back(rand, cfg.LMG_BITWEIGHT());
    }

    for (const auto & mptr : metrics.mets) {
      const auto & metric = *mptr;
      name = metric.name() + " Distance";

      for (from = 0; from < cfg.LMG_NNODES(); ++from) {
        for (to = from; to < cfg.LMG_NNODES(); ++to) {
          match = metric(bs[from], bs[to]);
          df.Update();
        }
      }

    }

  }
}