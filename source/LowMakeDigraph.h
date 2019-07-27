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

void LowMakeDigraph(const Metrics::collection_t &metrics, const Config &cfg) {

  emp::Random rand(cfg.SEED());

  size_t s;
  size_t from;
  size_t to;
  std::string name;
  double distance;

  emp::DataFile df(emp::keyname::pack({
    {"bitweight", emp::to_string(cfg.LMD_BITWEIGHT())},
    {"title", cfg.LMD_TITLE()},
    {"seed", emp::to_string(cfg.SEED())},
    // {"_emp_hash=", STRINGIFY(EMPIRICAL_HASH_)},
    // {"_source_hash=", STRINGIFY(DISHTINY_HASH_)},
    {"ext", ".csv"}
  }));
  df.AddVar(s, "Sample");
  df.AddVar(from, "From");
  df.AddVar(to, "To");
  df.AddVar(name, "Metric");
  df.AddVar(distance, "Distance");
  df.PrintHeaderKeys();

  for(s = 0; s < cfg.LMD_NSAMPLES(); ++s) {
    std::cout << "sample " << s << std::endl;

    emp::vector<emp::BitSet<32>> lefts;
    emp::vector<emp::BitSet<32>> rights;
    lefts.reserve(cfg.LMD_NNODES());
    rights.reserve(cfg.LMD_NNODES());

    for(size_t i = 0; i < cfg.LMD_NNODES(); ++i) {
      lefts.emplace_back(rand, cfg.LMD_BITWEIGHT());
      rights.emplace_back(rand, cfg.LMD_BITWEIGHT());
    }

    for (const auto & mptr : metrics) {
      const auto & metric = *mptr;
      name = metric.name() + " Distance";

      // lefts broadcast to all rights
      for (size_t l = 0; l < cfg.LMD_NNODES(); ++l) {
        for (size_t r = 0; r < cfg.LMD_NNODES(); ++r) {
          from = r;
          to = l + cfg.LMD_NNODES();
          distance = metric(rights[r], lefts[l]);
          df.Update();
        }
      }

      // rights are tied directly to a corresponding left
      for (size_t pair = 0; pair < cfg.LMD_NNODES(); ++pair) {
        from = pair + cfg.LMD_NNODES();
        to = pair;
        distance = 0.0;
        df.Update();
      }

    }

  }
}
