#pragma once

#include "base/vector.h"
#include "meta/TypePack.h"
#include "tools/BitSet.h"
#include "tools/matchbin_utils.h"

struct Metrics {

  using collection_t = emp::vector<
    emp::Ptr<
      emp::BaseMetric<
        emp::BitSet<32>,
        emp::BitSet<32>
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
