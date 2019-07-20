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
  size_t step;
  std::string name;
  double match;

  emp::DataFile df(cfg.LMW_FILE());
  df.AddVar(s, "Sample");
  df.AddVar(r, "Replicate");
  df.AddVar(step, "Mutational Step");
  df.AddVar(name, "Metric");
  df.AddVar(match, "Match Score");
  df.PrintHeaderKeys();

  for (const auto & mptr : metrics.mets) {
    const auto & metric = *mptr;

    // filter out non-interesting data
    if (metric.name().find("Inverse") != std::string::npos) continue;
    std::cout << "metric " << name << std::endl;


    for (s = 0; s < cfg.LMW_NSAMPLES(); ++s) {

      const emp::BitSet<32> orig_bs(rand);

      for (r = 0; r < cfg.LMW_NREPS(); ++r) {

        emp::BitSet<32> walker(orig_bs);

        for (step = 0; step < cfg.LMW_NSTEPS(); ++step) {

          walker.Toggle(rand.GetUInt(32));

          name = metric.name() + " Distance";
          match = metric(orig_bs, walker);

          df.Update();

        }

      }

    }

  }

}
