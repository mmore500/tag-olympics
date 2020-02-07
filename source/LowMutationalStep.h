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

void LowMutationalStep(const Metrics::collection_t &metrics, const Config &cfg) {

  emp::Random rand(cfg.SEED());

  size_t s;
  std::string name;
  std::string affinity;
  double match;

  emp::DataFile df(emp::keyname::pack({
    {"bitweight", emp::to_string(cfg.LMS_BITWEIGHT())},
    {"title", cfg.LMS_TITLE()},
    {"seed", emp::to_string(cfg.SEED())},
    // {"_emp_hash=", STRINGIFY(EMPIRICAL_HASH_)},
    // {"_source_hash=", STRINGIFY(DISHTINY_HASH_)},
    {"ext", ".csv"}
  }));
  df.AddVar(s, "Sample");
  df.AddVar(name, "Metric");
  df.AddVar(match, "Match Distance Change");
  df.AddVar(affinity, "Affinity");
  df.PrintHeaderKeys();

  for (const auto & mptr : metrics) {
    const auto & metric = *mptr;

    // filter out non-interesting data
    if (metric.name().find("Inverse") != std::string::npos) continue;
    std::cout << "metric " << name << std::endl;
    name = metric.name() + " Distance";

    for (const size_t sample : {1, 100}) {

      affinity = sample == 1 ? "Loose" : "Tight";
      for (s = 0; s < cfg.LMS_NSAMPLES(); ++s) {

        const emp::BitSet<Config::BS_WIDTH()> target(rand, cfg.LMS_BITWEIGHT());

        emp::vector<emp::BitSet<Config::BS_WIDTH()>> options;
        for (size_t opt = 0; opt < sample; ++opt) {
          options.emplace_back(rand, cfg.LMS_BITWEIGHT());
        }

        const auto & best = *std::min_element(
          std::begin(options),
          std::end(options),
          [&](const auto & a, const auto & b){
            return metric(target, a) < metric(target, b);
          }
        );

        auto walker = best;

        const size_t num_ones = walker.CountOnes();

        if (
          num_ones * (1.0 - cfg.LMS_BITWEIGHT())
          <
          rand.GetDouble() * (
            num_ones * (1.0 - cfg.LMS_BITWEIGHT())
            + (walker.GetSize() - num_ones) * cfg.LMS_BITWEIGHT()
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

        match = metric(target, best) - metric(target, walker);
        df.Update();

      }

    }

  }

}
