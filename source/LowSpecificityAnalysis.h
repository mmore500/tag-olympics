#include <iostream>
#include <limits>

#include "base/vector.h"
#include "tools/MatchBin.h"
#include "tools/matchbin_utils.h"
#include "tools/Random.h"
#include "config/ArgManager.h"
#include "data/DataFile.h"

#include "Config.h"

void LowSpecificityAnalysis(Config &cfg) {

  emp::HammingMetric<32> ham;
  emp::StreakMetric<32> streak;
  emp::Slide<emp::HammingMetric<32>> slide_ham;
  emp::Slide<emp::StreakMetric<32>> slide_streak;

  emp::Random rand(cfg.SEED());

  size_t s;
  size_t r;
  std::string metric;
  double match;

  emp::DataFile df(cfg.LSA_FILE());
  df.AddVar(s, "Sample");
  df.AddVar(r, "Replicate");
  df.AddVar(metric, "Metric");
  df.AddVar(match, "Match Distance");
  df.PrintHeaderKeys();

  for(s = 0; s < cfg.LSA_NSAMPLES(); ++s) {
    std::cout << "sample " << s << std::endl;

    emp::BitSet<32> samp(rand);

    for(r = 0; r < cfg.LSA_NREPS(); ++r) {

      emp::BitSet<32> rep(rand);

      metric = "Hamming Distance";
      match = ham(samp, rep) / ham.max_dist;
      df.Update();

      metric = "Streak Distance";
      match = streak(samp, rep) / streak.max_dist;
      df.Update();

      metric = "Sliding Hamming Distance";
      match = slide_ham(samp, rep) / slide_ham.max_dist;
      df.Update();

      metric = "Sliding Streak Distance";
      match = slide_streak(samp, rep) / slide_streak.max_dist;
      df.Update();

      // metric = "Bitstring Integer Distance";
      // match = intdiff(samp, rep) / intdiff.max_dist;
      // df.Update();

    }
  }

}
