#pragma once

#include <iostream>

#include "base/vector.h"
#include "tools/BitSet.h"
#include "tools/Random.h"
#include "tools/Binomial.h"

#include "Config.h"

template <size_t BS_WIDTH>
struct MidOrganism {

  emp::vector<emp::BitSet<BS_WIDTH>> bsets;
  const Config & cfg;

  MidOrganism(
    const Config & cfg_,
    emp::Random & rand
  ) : cfg(cfg_)
  {
    for(size_t i = 0; i < cfg.MO_LENGTH(); ++i) bsets.emplace_back(
      rand
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

    const static double MUT_BIT_REDRAW_PER_BIT = (
      cfg.MO_MUT_EXPECTED_REDRAWS() / cfg.MO_LENGTH()
    );

    static emp::Binomial bino(MUT_BIT_REDRAW_PER_BIT, cfg.BS_WIDTH());

    for (auto & bs : bsets) res += bs.Mutate(
      rand,
      bino.PickRandom(rand)
    );

    return res;

  }

  double Distance(const MidOrganism & other) const {
    double res = 0.0;
    for (size_t idx = 0; idx < bsets.size(); ++idx) {
      res += (bsets[idx]^other.bsets[idx]).CountOnes();
    }
    return res / (bsets.size() * bsets[0].GetSize());
  }

  void Print(std::ostream & os) {
    for (auto & bs : bsets) {
      os << bs << "|";
    }
  }

  const emp::BitSet<BS_WIDTH> & Get(size_t i) const { return bsets[i]; }

};
