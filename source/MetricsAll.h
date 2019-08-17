#pragma once

#include "base/vector.h"
#include "meta/TypePack.h"
#include "tools/BitSet.h"
#include "tools/matchbin_utils.h"

#include "Metrics.h"

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
    emp::HashMetric<32/Dim>,
    emp::SlideMod<emp::HammingMetric<32/Dim>>,
    emp::SlideMod<emp::StreakMetric<32/Dim>>,
    emp::SlideMod<emp::HashMetric<32/Dim>>,
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
  using min_pack_t = typename with_anti_pack_t::template wrap<
    CurryDimMod<emp::MinDimMod, Dim>::template t
  >;

  // make mean multidimensional variants
  using euc_pack_t = typename with_anti_pack_t::template wrap<
    CurryDimMod<emp::EuclideanDimMod, Dim>::template t
  >;

  // merge together all multimntional variants
  using min_mean_pack_t = typename mean_pack_t::template merge<
    min_pack_t
  >;

  // merge together all multimntional variants
  using nested_pack_t = typename min_mean_pack_t::template merge<
    euc_pack_t
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

    } else if constexpr (Dim > 1) {

      AddDim<Dim/2>::template append<typename AddDim<Dim/2>::pack_t>(c);

    }

  }

};

struct MetricsAll {

  using collection_t = Metrics::collection_t;

  collection_t mets;

  MetricsAll() {
    AddDim<4>::append<AddDim<4>::pack_t>(mets);
    std::cout << mets.size() << std::endl;
  }


};
