#pragma once

#include <iostream>
#include <limits>

#include "tools/MatchBin.h"
#include "tools/matchbin_utils.h"
#include "tools/Random.h"
#include "config/ArgManager.h"
#include "data/DataFile.h"
#include "tools/string_utils.h"
#include "tools/keyname_utils.h"

#include "Config.h"
#include "Metrics.h"

void LowTripletAnalysis(const Metrics::collection_t &metrics, const Config &cfg) {

  emp::Random rand(cfg.SEED());

  size_t s;
  std::string name;
  double detdiff;

  emp::DataFile df(emp::keyname::pack({
    {"bitweight", emp::to_string(cfg.LTA_BITWEIGHT())},
    {"title", cfg.LTA_TITLE()},
    {"seed", emp::to_string(cfg.SEED())},
    // {"_emp_hash=", STRINGIFY(EMPIRICAL_HASH_)},
    // {"_source_hash=", STRINGIFY(DISHTINY_HASH_)},
    {"ext", ".csv"}
  }));
  df.AddVar(s, "Sample");
  df.AddVar(name, "Metric");
  df.AddVar(detdiff, "Detour Difference");
  df.PrintHeaderKeys();

  for(s = 0; s < cfg.LTA_NSAMPLES(); ++s) {
    if (s % 10 == 0)  std::cout << "sample " << s << std::endl;

    emp::BitSet<32> bs_x(rand, cfg.LTA_BITWEIGHT());
    emp::BitSet<32> bs_y(rand, cfg.LTA_BITWEIGHT());
    emp::BitSet<32> bs_z(rand, cfg.LTA_BITWEIGHT());

    for (const auto & mptr : metrics) {
      const auto & metric = *mptr;
      name = metric.name() + " Distance";
      detdiff = (metric(bs_x, bs_y) + metric(bs_y, bs_z) - metric(bs_x, bs_z));
      df.Update();
    }
  }

}
