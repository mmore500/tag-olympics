#pragma once

#include <iostream>
#include <limits>
#include <string>

#include "tools/MatchBin.h"
#include "tools/matchbin_utils.h"
#include "tools/Random.h"
#include "config/ArgManager.h"
#include "data/DataFile.h"
#include "tools/string_utils.h"
#include "tools/keyname_utils.h"

#include "Config.h"
#include "Metrics.h"

void LowMutationalWalk(const Metrics::collection_t &metrics, const Config &cfg) {

  emp::Random rand(cfg.SEED());

  size_t s;
  size_t r;
  size_t step;
  std::string name;
  double match;

  emp::DataFile df(emp::keyname::pack({
    {"bitweight", emp::to_string(cfg.LMW_BITWEIGHT())},
    {"title", cfg.LMW_TITLE()},
    {"seed", emp::to_string(cfg.SEED())},
    // {"_emp_hash=", STRINGIFY(EMPIRICAL_HASH_)},
    // {"_source_hash=", STRINGIFY(DISHTINY_HASH_)},
    {"ext", ".csv"}
  }));
  df.AddVar(s, "Sample");
  df.AddVar(r, "Replicate");
  df.AddVar(step, "Mutational Step");
  df.AddVar(name, "Metric");
  df.AddVar(match, "Match Score");
  df.PrintHeaderKeys();

  for (const auto & mptr : metrics) {
    const auto & metric = *mptr;

    // filter out non-interesting data
    if (metric.name().find("Inverse") != std::string::npos) continue;
    std::cout << "metric " << name << std::endl;


    for (s = 0; s < cfg.LMW_NSAMPLES(); ++s) {

      const emp::BitSet<Config:BS_WIDTH()> orig_bs(rand, cfg.LMW_BITWEIGHT());

      for (r = 0; r < cfg.LMW_NREPS(); ++r) {

        emp::BitSet<Config:BS_WIDTH()> walker(orig_bs);

        for (step = 0; step < cfg.LMW_NSTEPS(); ++step) {

          name = metric.name() + " Distance";
          match = metric(orig_bs, walker);

          df.Update();

          // no mutations are possible in an absolute regime
          if (cfg.LMW_BITWEIGHT() == 0.0 || cfg.LMW_BITWEIGHT() == 1.0) {
            continue;
          }

          const size_t num_ones = walker.CountOnes();

          if (
            num_ones * (1.0 - cfg.LMW_BITWEIGHT())
            <
            rand.GetDouble() * (
              num_ones * (1.0 - cfg.LMW_BITWEIGHT())
              + (walker.GetSize() - num_ones) * cfg.LMW_BITWEIGHT()
            )
          ) {
            // toggle a randomly chosen 1
            const size_t targetone = rand.GetUInt(num_ones);
            size_t onecount = 0;
            size_t pos;
            for (pos = 0; onecount < targetone; ++pos) {
              if (walker[pos]) ++onecount;
            }
            walker.Toggle(pos);
          } else {
            // togggle a randomly chosen 0
            const size_t targetzero = rand.GetUInt(walker.GetSize() - num_ones);
            size_t zerocount = 0;
            size_t pos;
            for (pos = 0; zerocount < targetzero; ++pos) {
              if (!walker[pos]) ++zerocount;
            }
            walker.Toggle(pos);
          }

        }

      }

    }

  }

}
