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
    for(size_t i = 0; i < cfg.MO_LENGTH(); ++i) {
      if (cfg.MFM_ZEROINIT()) {
        bsets.emplace_back();
      } else {
        bsets.emplace_back(rand);
      }
    }
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

    const static double MUT_BIT_TOGGLE_PER_BIT = (
      cfg.MO_MUT_EXPECTED_REDRAWS() / (cfg.MO_LENGTH() * cfg.BS_WIDTH())
    );

    static emp::Binomial bino(MUT_BIT_TOGGLE_PER_BIT, cfg.BS_WIDTH());

    if (!cfg.MO_MUT_NORMAL()) {
      for (auto & bs : bsets) res += bs.Mutate(
        rand,
        bino.PickRandom(rand)
      );
    } else {
      for (auto & bs : bsets) {
        int64_t diff_val = rand.GetRandNormal(
          0.0,
          bs.MaxDouble() * cfg.MO_MUT_NORMAL_SD()
        );
        emp::BitSet<BS_WIDTH> diff_bs{};
        diff_bs.SetUInt64(0, std::abs(diff_val));
        if (diff_val < 0) bs -= diff_bs;
        else bs += diff_bs;

        ++res;
      }
    }

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
