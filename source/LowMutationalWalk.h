#include <iostream>
#include <limits>

#include "tools/MatchBin.h"
#include "tools/matchbin_utils.h"
#include "tools/Random.h"
#include "config/ArgManager.h"
#include "data/DataFile.h"

#include "Config.h"

void LowMutationalWalk(Config &cfg) {

  emp::HammingMetric<32> ham;
  emp::StreakMetric<32> streak;
  emp::AbsIntDiffMetric<32> intdiff;
  emp::AbsDiffMetric absdiff;
  emp::NextUpMetric<> nextup; // std::numeric_limits<size_t>::max()

  emp::Random rand(cfg.SEED());

  size_t s;
  size_t r;
  size_t t;
  std::string metric;
  double match;

  emp::DataFile df(cfg.LMW_FILE());
  df.AddVar(s, "Sample");
  df.AddVar(r, "Replicate");
  df.AddVar(t, "Step");
  df.AddVar(metric, "Metric");
  df.AddVar(match, "Match Distance");
  df.PrintHeaderKeys();

  for(s = 0; s < cfg.LMW_NSAMPLES(); ++s) {
    std::cout << "sample " << s << std::endl;

    const emp::BitSet<32> orig_bs(rand);
    const size_t orig_st = rand.GetUInt();
    const int orig_it = rand.GetUInt();

    for(r = 0; r < cfg.LMW_NREPS(); ++r) {
      std::cout << ".";
      std::cout.flush();
      emp::BitSet<32> bs = orig_bs;
      size_t st = orig_st;
      int it = orig_it;

      for(t = 0; t < cfg.LMW_NSTEPS(); ++t) {

        bs.Toggle(rand.GetUInt(32));
        st += rand.P(0.5) ? -1 : 1;
        it += rand.P(0.5) ? -1 : 1;

        metric = "Hamming Distance";
        match = ham(orig_bs, bs) / ham.max_dist;
        df.Update();

        metric = "Streak Distance";
        match = streak(orig_bs, bs) / streak.max_dist;
        df.Update();

        metric = "Bitstring Integer Distance";
        match = intdiff(orig_bs, bs) / intdiff.max_dist;
        df.Update();

        metric = "Bidirectional Integer Distance";
        match = absdiff(orig_it, it) / absdiff.max_dist;
        df.Update();

        metric = "Unidirectional Integer Distance";
        match = nextup(orig_st, st) / nextup.max_dist;
        df.Update();

      }
    }
    std::cout << std::endl;
  }

}
