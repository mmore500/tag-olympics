#pragma once

#include "base/vector.h"
#include "meta/TypePack.h"
#include "tools/BitSet.h"
#include "tools/matchbin_utils.h"

#include "Metrics.h"

template <size_t Dim>
struct AddAntiDim {

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
    emp::AsymmetricWrapMetric<32/Dim>,
    emp::AsymmetricNoWrapMetric<32/Dim>,
    emp::SymmetricWrapMetric<32/Dim>,
    emp::SymmetricNoWrapMetric<32/Dim>
  >;

  // make anti versions
  using anti_pack_t = typename base_pack_t::template wrap<emp::AntiMod>;

  // make mean multidimensional variants
  using mean_pack_t = typename anti_pack_t::template wrap<
    CurryDimMod<emp::MeanDimMod, Dim>::template t
  >;

  // make min multidimensional variants
  using min_pack_t = typename anti_pack_t::template wrap<
    CurryDimMod<emp::MinDimMod, Dim>::template t
  >;

  // merge together all multimntional variants
  using nested_pack_t = typename mean_pack_t::template merge<
    min_pack_t
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

    } else if constexpr (Dim < 4) {

      AddAntiDim<Dim*4>::template append<typename AddAntiDim<Dim*4>::pack_t>(c);

    }

  }

};

struct MetricsFewerAnti {

  using collection_t = Metrics::collection_t;

  collection_t mets;

  MetricsFewerAnti() {
    AddAntiDim<1>::append<AddAntiDim<1>::pack_t>(mets);
    std::cout << "anti " <<mets.size() << std::endl;
  }


};
