#pragma once

#include <iostream>
#include <cmath>

#include "base/Ptr.h"

#include "Evolve/World.h"
#include "tools/BitSet.h"
#include "tools/Random.h"
#include "tools/matchbin_utils.h"
#include "tools/File.h"

#include "Config.h"
#include "MidOrganism.h"

void MidFlexMatch(const Config &cfg) {

  emp::StreakMetric<32> metric;

  emp::Random rand(cfg.SEED());

  emp::File f(
    std::string()
    + "title="
    + "graph"
    + "+"
    + "seed="
    + emp::to_string(cfg.SEED())
    // + "+"
    // + "_emp_hash="
    // + STRINGIFY(EMPIRICAL_HASH_)
    // + "+"
    // + "_source_hash="
    // + STRINGIFY(DISHTINY_HASH_)
    + "+"
    + "ext="
    + ".csv"
  );

  f.RemoveComments('#');

  emp_assert(f.size() == cfg.MO_LENGTH());

  emp::vector<emp::vector<double>> target(
    cfg.MO_LENGTH(),
    emp::vector<double>(cfg.MO_LENGTH(), 1.0)
  );
  for (size_t i = 0; i < target.size(); ++i) {
    // mark self loops and duplicate non-edges to be ignored
    target[i][i] = -1.0;
    for (size_t j = i; j < target.size(); ++j) target[i][j] = -1.0;
  }

  size_t edge_count = 0;
  while (f.size()) {
    auto res = f.ExtractRowAs<size_t>();
    for (size_t i = 1; i < res.size(); ++i) {
      target[res[0]][i] = rand.GetDouble(cfg.MFM_TARGET_MAX());
      target[i][res[0]] = -1.0;

      // randomly choose edge direction
      if (rand.P(0.5)) std::swap(target[res[0]][i], target[i][res[0]]);

      ++edge_count;
    }
  }

  emp::World<MidOrganism<32>> grid_world(rand);

  grid_world.SetupFitnessFile(
    std::string()
    + "bitweight="
    + emp::to_string(cfg.MO_BITWEIGHT())
    + "+"
    + "title="
    + cfg.MFM_TITLE()
    + "-fitness"
    + "+"
    + "seed="
    + emp::to_string(cfg.SEED())
    // + "+"
    // + "_emp_hash="
    // + STRINGIFY(EMPIRICAL_HASH_)
    // + "+"
    // + "_source_hash="
    // + STRINGIFY(DISHTINY_HASH_)
    + "+"
    + "ext="
    + ".csv"
  );

  grid_world.AddSystematics(
    emp::NewPtr<
      emp::Systematics<
        MidOrganism<32>,
        MidOrganism<32>
      >
    >(
      [](MidOrganism<32> & o){ return o; },
      true,
      true,
      false
    ),
    "systematics"
  );

  grid_world.SetupSystematicsFile(
    "systematics",
    std::string()
    + "bitweight="
    + emp::to_string(cfg.MO_BITWEIGHT())
    + "+"
    + "title="
    + cfg.MFM_TITLE()
    + "-systematics"
    + "+"
    + "seed="
    + emp::to_string(cfg.SEED())
    // + "+"
    // + "_emp_hash="
    // + STRINGIFY(EMPIRICAL_HASH_)
    // + "+"
    // + "_source_hash="
    // + STRINGIFY(DISHTINY_HASH_)
    + "+"
    + "ext="
    + ".csv"
  );

  const size_t side = (size_t) std::sqrt(cfg.MFM_POP_SIZE());

  grid_world.SetPopStruct_Grid(side, side);
  grid_world.SetFitFun([&target, &cfg, &metric](MidOrganism<32> & org){

    double res = 0.0;
    double worst = 0.0;

    for (size_t i = 0; i < cfg.MO_LENGTH(); ++i) {
      for (size_t j = 0; j < cfg.MO_LENGTH(); ++j) {

        if (target[i][j] == -1.0) continue;

        worst += 1.0 - (
          target[i][j] == 1.0
          ? cfg.MFM_NOMATCH_THRESH()
          : cfg.MFM_MATCH_THRESH()
        );

        res += (target[i][j] == 1.0)
        ? std::max(0.0, std::abs(
          target[i][j] - metric(org.Get(i), org.Get(j))
        ) - cfg.MFM_NOMATCH_THRESH())
        : std::max(0.0, std::abs(
          target[i][j] - metric(org.Get(i), org.Get(j))
        ) - cfg.MFM_MATCH_THRESH());
      }
    }

    return 1.0 - res/worst;

  });
  grid_world.SetAutoMutate();
  grid_world.OnUpdate([&grid_world, &cfg](size_t upd){
    emp::TournamentSelect(
      grid_world,
      cfg.MFM_TOURNEY_SIZE(),
      cfg.MFM_TOURNEY_REPS()
    );
  });

  // POP_SIZE needs to be a perfect square.
  emp_assert(grid_world.GetSize() == cfg.MFM_POP_SIZE());

  for (size_t i = 0; i < cfg.MFM_POP_SIZE(); ++i) {
    MidOrganism<32> org(cfg, rand);
    grid_world.InjectAt(org, i);
  }

  for (size_t g = 0; g < cfg.MFM_GENS(); ++g) {
    grid_world.Update();
    std::cout << ".";
    std::cout.flush();
  }

  std::cout << std::endl;
  grid_world.PrintGrid();

}
