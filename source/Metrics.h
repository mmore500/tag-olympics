#pragma once

#include "base/vector.h"
#include "meta/TypePack.h"
#include "tools/BitSet.h"
#include "tools/matchbin_utils.h"

struct Metrics {

  using metric_t = emp::BaseMetric<
    emp::BitSet<Config::BS_WIDTH()>,
    emp::BitSet<Config::BS_WIDTH()>
  >;

  using collection_t = emp::vector<
    emp::Ptr<
      metric_t
    >
  >;

};

template<
  template<typename, size_t> typename DimMod,
  size_t W
>
struct CurryDimMod {
  template<typename Metric>
  using t = DimMod<Metric, W>;
};
