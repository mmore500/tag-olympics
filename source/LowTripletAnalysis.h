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

void LowTripletAnalysis(const Metrics &metrics, const Config &cfg) {

  emp::Random rand(cfg.SEED());

  size_t s;
  std::string name;
  double detdiff;

  emp::DataFile df(cfg.LTA_FILE());
  df.AddVar(s, "Sample");
  df.AddVar(name, "Metric");
  df.AddVar(detdiff, "Detour Difference");
  df.PrintHeaderKeys();

  for(s = 0; s < cfg.LTA_NSAMPLES(); ++s) {
    if (s % 10000 == 0)  std::cout << "sample " << s << std::endl;

    emp::BitSet<32> bs_x(rand);
    emp::BitSet<32> bs_y(rand);
    emp::BitSet<32> bs_z(rand);

    for (const auto & mptr : metrics.mets) {
      const auto & metric = *mptr;
      name = metric.name() + " Distance";
      detdiff = (metric(bs_x, bs_y) + metric(bs_y, bs_z) - metric(bs_x, bs_z));
      df.Update();
    }
  }

}
