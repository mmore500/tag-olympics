#pragma once

#include "base/vector.h"
#include "meta/TypePack.h"
#include "tools/BitSet.h"
#include "tools/matchbin_utils.h"

#include "Config.h"
#include "Metrics.h"


struct MetricsMid {

  using collection_t = Metrics::collection_t;

  collection_t mets{
    // new emp::UnifMod<emp::HammingMetric<Config::BS_WIDTH()>>,
    // new emp::UnifMod<emp::StreakMetric<Config::BS_WIDTH()>>,
    // new emp::UnifMod<emp::CryptoHashMetric<Config::BS_WIDTH()>>,
    new emp::UnifMod<emp::SymmetricWrapMetric<Config::BS_WIDTH()>>,
    new emp::UnifMod<emp::AsymmetricWrapMetric<Config::BS_WIDTH()>>
  };

  MetricsMid() {
    std::cout << mets.size() << std::endl;
  }


};
