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
  emp::SymmetricWrapMetric<32> swm;
  emp::SymmetricNoWrapMetric<32> snwm;
  emp::AsymmetricWrapMetric<32> awm;
  emp::AsymmetricNoWrapMetric<32> anwm;

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

    for(r = 0; r < cfg.LMW_NREPS(); ++r) {
      std::cout << ".";
      std::cout.flush();
      emp::BitSet<32> bs = orig_bs;

      for(t = 0; t < cfg.LMW_NSTEPS(); ++t) {

        bs.Toggle(rand.GetUInt(32));

        metric = "Hamming Distance";
        match = ham(orig_bs, bs);
        df.Update();

        metric = "Streak Distance";
        match = streak(orig_bs, bs);
        df.Update();

        metric = "Symmetric Wrap Metric Distance";
        match = swm(orig_bs, bs);
        df.Update();

        metric = "Symmetric No Wrap Metric Distance";
        match = snwm(orig_bs, bs);
        df.Update();

        metric = "Asymmetic Wrap Metric Distance";
        match = awm(orig_bs, bs);
        df.Update();

        metric = "Asymmetic No Wrap Metric Distance";
        match = anwm(orig_bs, bs);
        df.Update();

      }
    }
    std::cout << std::endl;
  }

}
