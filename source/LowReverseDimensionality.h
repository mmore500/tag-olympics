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

void LowReverseDimensionality(const Metrics::collection_t &metrics, const Config &cfg) {

  emp::Random rand(cfg.SEED());

  size_t s;
  std::string name;
  double score;

  emp::DataFile df(emp::keyname::pack({
    {"bitweight", emp::to_string(cfg.LRD_BITWEIGHT())},
    {"title", cfg.LRD_TITLE()},
    {"seed", emp::to_string(cfg.SEED())},
    // {"_emp_hash=", STRINGIFY(EMPIRICAL_HASH_)},
    // {"_source_hash=", STRINGIFY(DISHTINY_HASH_)},
    {"ext", ".csv"}
  }));
  df.AddVar(s, "Sample");
  df.AddVar(name, "Metric");
  df.AddVar(score, "Match Distance");
  df.PrintHeaderKeys();


  for (s = 0; s < cfg.LRD_NSAMPLES(); ++s) {

    emp::BitSet<Config::BS_WIDTH()> bs_target(rand, cfg.LSD_BITWEIGHT());

    for (const auto & mptr : metrics) {
      const auto & metric = *mptr;
      name = metric.name() + " Distance";

      emp::vector<emp::BitSet<Config::BS_WIDTH()>> antisets;
      while (antisets.size() < 1) {
        emp::BitSet<Config::BS_WIDTH()> bs_test(rand, cfg.LSD_BITWEIGHT());
        if (metric(bs_target, bs_test) > 0.99) {
          antisets.push_back(bs_test);
        }
      }

      emp::vector<emp::BitSet<Config::BS_WIDTH()>> bitsets;

      while (bitsets.size() < 1) {
        emp::BitSet<Config::BS_WIDTH()> bs_test(rand, cfg.LSD_BITWEIGHT());
        if (metric(bs_target, bs_test) < 0.01) {
          bitsets.push_back(bs_test);
        }
      }

      score = metric(antisets[0], bitsets[0]);
      df.Update();

    }

  }

}
