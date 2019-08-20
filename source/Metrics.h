#pragma once

#include "base/vector.h"
#include "meta/TypePack.h"
#include "tools/BitSet.h"
#include "tools/matchbin_utils.h"

struct Metrics {

  using collection_t = emp::vector<
    emp::Ptr<
      emp::BaseMetric<
        emp::BitSet<Config::BS_WIDTH()>,
        emp::BitSet<Config::BS_WIDTH()>
      >
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
