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
  emp::AbsIntDiffMetric<32> intdiff;
  emp::AbsDiffMetric absdiff;
  emp::NextUpMetric<> nextup; // std::numeric_limits<size_t>::max()

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
    emp::vector<size_t> st;
    st.reserve(cfg.LGA_NNODES());
    emp::vector<int> it;
    it.reserve(cfg.LGA_NNODES());

    for(size_t n = 0; n < cfg.LGA_NNODES(); ++n) {
      bs.emplace_back(rand);
      st.push_back(rand.GetUInt());
      it.push_back(rand.GetUInt());
    }

    for(from = 0; from < cfg.LGA_NNODES(); ++from) {
      for(to = from; to < cfg.LGA_NNODES(); ++to) {

        metric = "Hamming Distance";
        match = ham(bs[from], bs[to]) / ham.max_dist;
        df.Update();

        metric = "Streak Distance";
        match = streak(bs[from], bs[to]) / streak.max_dist;
        df.Update();

        metric = "Bitstring Integer Distance";
        match = intdiff(bs[from], bs[to]) / intdiff.max_dist;
        df.Update();

        metric = "Bidirectional Integer Distance";
        match = absdiff(it[from], it[to]) / absdiff.max_dist;
        df.Update();

        metric = "Unidirectional Integer Distance";
        match = nextup(st[from], st[to]) / nextup.max_dist;
        df.Update();

      }
    }
  }

}
