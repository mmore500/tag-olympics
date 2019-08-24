#pragma once

#include <iostream>

#include "base/vector.h"
#include "tools/BitSet.h"
#include "tools/Random.h"

#include "Config.h"

template <size_t BS_WIDTH>
struct MidOrganism {

  emp::vector<emp::BitSet<BS_WIDTH>> bsets;
  const Config & cfg;

  std::function<double(size_t)> site_mut_p;

  MidOrganism(
    const Config & cfg_,
    std::function<double(size_t)> site_mut_p_,
    emp::Random & rand
  ) : cfg(cfg_)
  , site_mut_p(site_mut_p_)
  {
    for(size_t i = 0; i < cfg.MO_LENGTH(); ++i) bsets.emplace_back(
      rand,
      cfg.MO_BITWEIGHT()
    );
  }

  bool operator==(const MidOrganism & other) const {
    for (size_t i = 0; i < bsets.size(); ++i) {
      if (bsets[i] != other.bsets[i]) return false;
    }
    return true;
  }

  bool operator!=(const MidOrganism & other) const {
    return !operator==(other);
  }


  size_t DoMutations(emp::Random & rand) {

    if (rand.GetDouble() > cfg.MO_MUT_PROB()) return 0;

    size_t res = 0;

    for (auto & bs : bsets) res += bs.Mutate(
      rand,
      cfg.MO_MUT_BIT_REDRAW_PER_BIT(),
      cfg.MO_BITWEIGHT()
    );

    for (auto &bs : bsets) {
      for (size_t idx = 0; idx < bs.GetSize(); ++idx) {
        if (rand.GetDouble() < site_mut_p(idx)) {
          bs[idx] = rand.P(cfg.MO_BITWEIGHT());
        }
      }
    }

    return res;

  }

  void Print(std::ostream & os) {
    for (auto & bs : bsets) {
      os << bs << "|";
    }
  }

  const emp::BitSet<BS_WIDTH> & Get(size_t i) const { return bsets[i]; }

};
