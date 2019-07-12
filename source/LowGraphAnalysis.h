#include <iostream>
#include <limits>

#include "tools/MatchBin.h"
#include "tools/matchbin_utils.h"
#include "tools/Random.h"
#include "config/ArgManager.h"
#include "data/DataFile.h"

#include "Config.h"

void LowGraphAnalysis(Config &cfg) {

  emp::HammingMetric<32> ham;
  emp::StreakMetric<32> streak;
  emp::SymmetricWrapMetric<32> swm;
  emp::SymmetricNoWrapMetric<32> snwm;
  emp::AsymmetricWrapMetric<32> awm;
  emp::AsymmetricNoWrapMetric<32> anwm;

  emp::Random rand(cfg.SEED());

  size_t s;
  size_t from;
  size_t to;
  std::string metric;
  double match;

  emp::DataFile df(cfg.LGA_FILE());
  df.AddVar(s, "Sample");
  df.AddVar(from, "From");
  df.AddVar(to, "To");
  df.AddVar(metric, "Metric");
  df.AddVar(match, "Match Distance");
  df.PrintHeaderKeys();

  for(s = 0; s < cfg.LGA_NSAMPLES(); ++s) {
    std::cout << "sample " << s << std::endl;

    emp::vector<emp::BitSet<32>> bs;
    bs.reserve(cfg.LGA_NNODES());

    for(size_t n = 0; n < cfg.LGA_NNODES(); ++n) {
      bs.emplace_back(rand);
    }

    for(from = 0; from < cfg.LGA_NNODES(); ++from) {
      for(to = from; to < cfg.LGA_NNODES(); ++to) {

        metric = "Hamming Distance";
        match = ham(bs[from], bs[to]);
        df.Update();

        metric = "Streak Distance";
        match = streak(bs[from], bs[to]);
        df.Update();

        metric = "Symmetric Wrap Metric Distance";
        match = swm(bs[from], bs[to]);
        df.Update();

        metric = "Symmetric No Wrap Metric Distance";
        match = snwm(bs[from], bs[to]);
        df.Update();

        metric = "Asymmetic Wrap Metric Distance";
        match = awm(bs[from], bs[to]);
        df.Update();

        metric = "Asymmetic No Wrap Metric Distance";
        match = anwm(bs[from], bs[to]);
        df.Update();

      }
    }
  }

}
