#pragma once

#include <iostream>
#include <limits>

#include "base/vector.h"
#include "tools/MatchBin.h"
#include "tools/matchbin_utils.h"
#include "tools/Random.h"
#include "config/ArgManager.h"
#include "data/DataFile.h"

#include "Config.h"
#include "Metrics.h"

void LowSpecificityAnalysis(const Metrics &metrics, const Config &cfg) {

  emp::Random rand(cfg.SEED());

  size_t s;
  size_t r;
  std::string name;
  double match;

  emp::DataFile df(cfg.LSA_FILE());
  df.AddVar(s, "Sample");
  df.AddVar(r, "Replicate");
  df.AddVar(name, "Metric");
  df.AddVar(match, "Match Distance");
  df.PrintHeaderKeys();

  for(s = 0; s < cfg.LSA_NSAMPLES(); ++s) {
    std::cout << "sample " << s << std::endl;

    emp::BitSet<32> samp(rand);
    emp::BitSet<32> samp_w(rand, 0.75);

    for(r = 0; r < cfg.LSA_NREPS(); ++r) {

      emp::BitSet<32> rep(rand);
      emp::BitSet<32> rep_w(rand, 0.75);


      for (const auto & mptr : metrics.mets) {
        const auto & metric = *mptr;
        name = metric.name() + " Distance";
        match = metric(samp, rep);
        df.Update();
      }

    }
  }

}
