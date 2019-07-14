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

void LowMutationalWalk(const Metrics &metrics, const Config &cfg) {

  emp::Random rand(cfg.SEED());

  size_t s;
  size_t r;
  size_t t;
  std::string name;
  double match;

  emp::DataFile df(cfg.LMW_FILE());
  df.AddVar(s, "Sample");
  df.AddVar(r, "Replicate");
  df.AddVar(t, "Step");
  df.AddVar(name, "Metric");
  df.AddVar(match, "Match Distance");
  df.PrintHeaderKeys();

  for(s = 0; s < cfg.LMW_NSAMPLES(); ++s) {
    std::cout << "sample " << s << std::endl;

    const emp::BitSet<32> orig_bs(rand);

    for(r = 0; r < cfg.LMW_NREPS(); ++r) {
      std::cout << ".";
      std::cout.flush();
      emp::BitSet<32> bs = orig_bs;

      for(t = 0; t < cfg.LMW_NSTEPS(); ++t) {

        bs.Toggle(rand.GetUInt(32));

        for (const auto & mptr : metrics.mets) {
          const auto & metric = *mptr;
          name = metric.name() + " Distance";
          match = metric(orig_bs, bs);
          df.Update();
        }

      }
    }
    std::cout << std::endl;
  }

}
