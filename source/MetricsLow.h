#pragma once

#include "base/vector.h"
#include "meta/TypePack.h"
#include "tools/BitSet.h"
#include "tools/matchbin_utils.h"

#include "Metrics.h"

using collection_t = emp::vector<
  emp::Ptr<
    emp::BaseMetric<
      emp::BitSet<Config::BS_WIDTH()>,
      emp::BitSet<Config::BS_WIDTH()>
    >
  >
>;

struct MetricsLow {

  using collection_t = Metrics::collection_t;

  collection_t mets{
    new emp::UnifMod<emp::HammingMetric<Config::BS_WIDTH()>>,
    new emp::UnifMod<emp::HammingStreakMetric<Config::BS_WIDTH()>>,
    // new emp::UnifMod<emp::CodonMetric<Config::BS_WIDTH()>>,
    new emp::UnifMod<emp::ApproxDualStreakMetric<Config::BS_WIDTH()>>,
    new emp::UnifMod<emp::ApproxSingleStreakMetric<Config::BS_WIDTH()>>,
    new emp::UnifMod<emp::CryptoHashMetric<Config::BS_WIDTH()>>,
    // new emp::UnifMod<emp::SymmetricWrapMetric<Config::BS_WIDTH()>>,
    // new emp::UnifMod<emp::AsymmetricWrapMetric<Config::BS_WIDTH()>>
    // new emp::HammingMetric<Config::BS_WIDTH()>,
    // new emp::StreakMetric<Config::BS_WIDTH()>,
    // new emp::HashMetric<Config::BS_WIDTH()>,
    // new emp::SymmetricWrapMetric<Config::BS_WIDTH()>,
    // new emp::AsymmetricWrapMetric<Config::BS_WIDTH()>
  };

  MetricsLow() {
    std::cout << mets.size() << std::endl;
  }


};
