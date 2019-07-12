#include <iostream>
#include <limits>

#include "tools/MatchBin.h"
#include "tools/matchbin_utils.h"
#include "tools/Random.h"
#include "config/ArgManager.h"
#include "data/DataFile.h"

#include "Config.h"

void LowTripletAnalysis(Config &cfg) {

  emp::HammingMetric<32> ham;
  emp::StreakMetric<32> streak;
  emp::SymmetricWrapMetric<32> swm;
  emp::SymmetricNoWrapMetric<32> snwm;
  emp::AsymmetricWrapMetric<32> awm;
  emp::AsymmetricNoWrapMetric<32> anwm;

  emp::Random rand(cfg.SEED());

  size_t s;
  std::string metric;
  double detdiff;

  emp::DataFile df(cfg.LTA_FILE());
  df.AddVar(s, "Sample");
  df.AddVar(metric, "Metric");
  df.AddVar(detdiff, "Detour Difference");
  df.PrintHeaderKeys();

  for(s = 0; s < cfg.LTA_NSAMPLES(); ++s) {
    if (s % 10000 == 0)  std::cout << "sample " << s << std::endl;

    emp::BitSet<32> bs_x(rand);
    emp::BitSet<32> bs_y(rand);
    emp::BitSet<32> bs_z(rand);

    metric = "Hamming Distance";
    detdiff = (ham(bs_x, bs_y) + ham(bs_y, bs_z) - ham(bs_x, bs_z));
    df.Update();

    metric = "Streak Distance";
    detdiff = (streak(bs_x, bs_y) + streak(bs_y, bs_z) - streak(bs_x, bs_z));
    df.Update();

    metric = "Symmetric Wrap Metric Distance";
    detdiff = (swm(bs_x, bs_y) + swm(bs_y, bs_z) - swm(bs_x, bs_z));
    df.Update();

    metric = "Symmetric No Wrap Metric Distance";
    detdiff = (snwm(bs_x, bs_y) + snwm(bs_y, bs_z) - snwm(bs_x, bs_z));
    df.Update();

    metric = "Asymmetic Wrap Metric Distance";
    detdiff = (awm(bs_x, bs_y) + awm(bs_y, bs_z) - awm(bs_x, bs_z));
    df.Update();

    metric = "Asymmetic No Wrap Metric Distance";
    detdiff = (anwm(bs_x, bs_y) + anwm(bs_y, bs_z) - anwm(bs_x, bs_z));
    df.Update();

  }

}
