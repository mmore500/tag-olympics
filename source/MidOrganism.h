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

  MidOrganism(const Config & cfg_, emp::Random & rand)
  : cfg(cfg_)
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

    size_t res = 0;

    for (auto & bs : bsets) res += bs.Mutate(
      rand,
      cfg.MO_MUT_BIT_REDRAW_PER_BIT(),
      cfg.MO_BITWEIGHT()
    );

    return res;

  }

  void Print(std::ostream & os) {
    for (auto & bs : bsets) {
      os << bs << "|";
    }
  }

  const emp::BitSet<BS_WIDTH> & Get(size_t i) const { return bsets[i]; }

};