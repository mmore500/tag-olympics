#include <iostream>
#include <limits>

#include "tools/MatchBin.h"
#include "tools/Random.h"
#include "config/ArgManager.h"
#include "data/DataFile.h"

#include "Config.h"

void LowTripletAnalysis(Config &cfg) {

  emp::HammingMetric<32> ham;
  emp::StreakMetric<32> streak;
  emp::AbsIntDiffMetric<32> intdiff;
  emp::AbsDiffMetric absdiff;
  emp::NextUpMetric<> nextup; // std::numeric_limits<size_t>::max()

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
    size_t st_x = rand.GetUInt();
    size_t st_y = rand.GetUInt();
    size_t st_z = rand.GetUInt();
    int it_x = rand.GetUInt();
    int it_y = rand.GetUInt();
    int it_z = rand.GetUInt();

    metric = "Hamming Distance";
    detdiff = (ham(bs_x, bs_y) + ham(bs_y, bs_z) - ham(bs_x, bs_z))/ham.max_dist;
    df.Update();

    metric = "Streak Distance";
    detdiff = (streak(bs_x, bs_y) + streak(bs_y, bs_z) - streak(bs_x, bs_z))/streak.max_dist;
    df.Update();

    metric = "Bitstring Integer Distance";
    detdiff = (intdiff(bs_x, bs_y) + intdiff(bs_y, bs_z) - intdiff(bs_x, bs_z))/intdiff.max_dist;
    df.Update();

    metric = "Bidirectional Integer Distance";
    detdiff = (absdiff(it_x, it_y) + absdiff(it_y, it_z) - absdiff(it_x, it_z))/absdiff.max_dist;
    df.Update();

    metric = "Unidirectional Integer Distance";
    detdiff = (nextup(st_x, st_y) + nextup(st_y, st_z) - nextup(st_x, st_z))/nextup.max_dist;
    df.Update();

  }

}
