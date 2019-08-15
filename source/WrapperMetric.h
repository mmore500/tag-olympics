#pragma once

#include "tools/BitSet.h"
#include "tools/matchbin_utils.h"

template<size_t Width>
struct WrapperMetric : public emp::BaseMetric<
  emp::BitSet<Width>, emp::BitSet<Width>
> {

  emp::Ptr<const emp::BaseMetric<
    emp::BitSet<Width>, emp::BitSet<Width>
  >> metric;

  using query_t = emp::BitSet<Width>;
  using tag_t = emp::BitSet<Width>;

  size_t dim() const override { return metric->dim(); }

  size_t width() const override { return metric->width(); }

  std::string name() const override {
    return metric->name();
  }

  std::string base() const override { return metric->base(); }

  double operator()(const query_t& a, const tag_t& b) const override {
    return (*metric)(a,b);
  }

};
