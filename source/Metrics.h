#pragma once

#include "base/vector.h"
#include "meta/TypePack.h"
#include "tools/BitSet.h"
#include "tools/matchbin_utils.h"

template<
  template<typename, size_t> typename DimMod,
  size_t W
>
struct CurryDimMod {
  template<typename Metric>
  using t = DimMod<Metric, W>;
};

template <size_t Dim>
struct AddDim {

  using collection_t = emp::vector<
    emp::Ptr<
      emp::BaseMetric<
        emp::BitSet<32>,
        emp::BitSet<32>
      >
    >
  >;

  // base metrics
  using base_pack_t = typename emp::TypePack<
    emp::HammingMetric<32/Dim>,
    emp::StreakMetric<32/Dim>,
    emp::SlideMod<emp::HammingMetric<32/Dim>>,
    emp::SlideMod<emp::StreakMetric<32/Dim>>,
    emp::AsymmetricWrapMetric<32/Dim>,
    emp::AsymmetricNoWrapMetric<32/Dim>,
    emp::SymmetricWrapMetric<32/Dim>,
    emp::SymmetricNoWrapMetric<32/Dim>
  >;

  // add anti versions
  using with_anti_pack_t = typename base_pack_t::template merge<
    typename base_pack_t::template wrap<emp::AntiMod>
  >;

  // make mean multidimensional variants
  using mean_pack_t = typename with_anti_pack_t::template wrap<
    CurryDimMod<emp::MeanDimMod, Dim>::template t
  >;

  // make mean multidimensional variants
  using max_pack_t = typename with_anti_pack_t::template wrap<
    CurryDimMod<emp::MaxDimMod, Dim>::template t
  >;

  // merge together all multimntional variants
  using nested_pack_t = typename mean_pack_t::template merge<
    max_pack_t
  >;

  // flatten all multidimensional variatns
  // should yield 32 metrics
  using pack_t = typename nested_pack_t::template wrap<
    emp::FlatMod
  >;

  template<typename Pack>
  static void append(collection_t & c) {

    if constexpr (!Pack::IsEmpty()) {
      c.push_back(emp::NewPtr<typename Pack::first_t>());

      append<typename Pack::pop>(c);

    }

    if constexpr (Pack::IsEmpty() && Dim > 1) {

      AddDim<Dim/2>::template append<typename AddDim<Dim/2>::pack_t>(c);

    }

  }

};

struct Metrics {

  using collection_t = emp::vector<
    emp::Ptr<
      emp::BaseMetric<
        emp::BitSet<32>,
        emp::BitSet<32>
      >
    >
  >;

  collection_t mets;

  Metrics() {
    AddDim<32>::append<AddDim<32>::pack_t>(mets);
    std::cout << mets.size() << std::endl;
  }


};
