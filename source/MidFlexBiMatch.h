#pragma once

#include <iostream>
#include <cmath>
#include <unordered_map>
#include <filesystem>
#include <cstdlib>
#include <sstream>

#include "base/Ptr.h"

#include "Evolve/World.h"
#include "tools/BitSet.h"
#include "tools/Random.h"
#include "tools/MatchBin.h"
#include "tools/matchbin_utils.h"
#include "tools/File.h"
#include "tools/keyname_utils.h"
#include "tools/random_utils.h"

#include "Config.h"
#include "Metrics.h"
#include "MidOrganism.h"
#include "WrapperMetric.h"

void MidFlexBiMatch(const Metrics::collection_t &metrics, const Config &cfg) {

  for (const auto & mptr : metrics) {
  const auto & metric = *mptr;

  emp::Random rand(cfg.SEED());

  emp::vector<std::string> graph_instances;

  for(auto& p: std::filesystem::directory_iterator(".")) {
    if (
      const auto res = emp::keyname::unpack(p.path());
      res.count("title") && res.at("title") == "mid-bigraph"
      && res.count("ext") && res.at("ext") == ".csv"
      && res.count("seed") && res.at("seed") == emp::to_string(cfg.SEED())
    ) {
      graph_instances.push_back(p.path());
    }
  }

  if (!graph_instances.size()) {
    std::cerr << "no graph source file found" << std::endl;
    std::cerr << "exiting..." << std::endl;
    std::exit(EXIT_FAILURE);
  }

  std::unordered_map<std::string, size_t> uids;
  std::unordered_map<std::string, double> target;
  std::unordered_set<std::string> lefts;
  std::unordered_set<std::string> rights;
  std::unordered_map<std::string, size_t> outgoing_edge_counts;
  size_t cur_instance = 0;

  auto incr_instance = [&](){
    cur_instance = (cur_instance + 1) % graph_instances.size();
    target.clear();
    lefts.clear();
    rights.clear();
    outgoing_edge_counts.clear();


    emp::File f(graph_instances[cur_instance]);

    f.RemoveComments('#');
    f.RemoveEmpty();

    emp_assert(f.size() == cfg.MO_LENGTH());

    while (f.size()) {
      const auto res = f.ExtractRowAs<std::string>();

      if (!uids.count(res[0])) uids[res[0]] = uids.size();

      if (emp::keyname::unpack(res[0])["side"] == "left") {
        lefts.insert(res[0]);
      } else {
        rights.insert(res[0]);
      }

      for (size_t i = 1; i < res.size(); ++i) {


        emp_assert(
          emp::keyname::unpack(res[0])["module"]
          ==
          emp::keyname::unpack(res[i])["module"]
        );

        emp_assert(
          emp::keyname::unpack(res[0])["side"]
          !=
          emp::keyname::unpack(res[i])["side"]
        );

        if (emp::keyname::unpack(res[0])["side"] == "left") {
          ++ outgoing_edge_counts[res[0]];
          target[res[0]+res[i]] = rand.GetDouble(cfg.MFM_TARGET_MAX());
        } else if (emp::keyname::unpack(res[i])["side"] == "left") {
          ++ outgoing_edge_counts[res[i]];
          target[res[i]+res[0]] = rand.GetDouble(cfg.MFM_TARGET_MAX());
        } else {
          emp_assert(false);
        }


      }
    }
  };

  incr_instance();

  emp::World<MidOrganism<32>> grid_world(rand);

  grid_world.SetupFitnessFile(emp::keyname::pack({
    {"bitweight", emp::to_string(cfg.MO_BITWEIGHT())},
    {"metric-slug", emp::slugify(metric.name())},
    {"experiment", cfg.MFM_TITLE()},
    {"datafile", "fitness"},
    {"treatment", cfg.TREATMENT()},
    {"seed", emp::to_string(cfg.SEED())},
    {"fit-fun", cfg.MFM_RANKED() ? "ranked" : "scored"},
    // {"_emp_hash=", STRINGIFY(EMPIRICAL_HASH_)},
    // {"_source_hash=", STRINGIFY(DISHTINY_HASH_)},
    {"ext", ".csv"}
  }));

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
    emp::keyname::pack({
      {"bitweight", emp::to_string(cfg.MO_BITWEIGHT())},
      {"metric-slug", emp::slugify(metric.name())},
      {"experiment", cfg.MFM_TITLE()},
      {"datafile", "systematics"},
      {"treatment", cfg.TREATMENT()},
      {"seed", emp::to_string(cfg.SEED())},
      {"fit-fun", cfg.MFM_RANKED() ? "ranked" : "scored"},
      // {"_emp_hash=", STRINGIFY(EMPIRICAL_HASH_)},
      // {"_source_hash=", STRINGIFY(DISHTINY_HASH_)},
      {"ext", ".csv"}
    })
  );

  const size_t side = (size_t) std::sqrt(cfg.MFM_POP_SIZE());

  grid_world.SetPopStruct_Grid(side, side);
  if (cfg.MFM_RANKED()) {
    grid_world.SetFitFun(
      [&]
      (MidOrganism<32> & org){

        emp::MatchBin<
          std::string,
          WrapperMetric<32>,
          emp::RankedSelector<>
        > mb(rand);
        mb.metric.metric = &metric;
        mb.SetCacheOn(false);

        for (const auto & right : rights) {
          mb.Put(right, org.Get(uids[right]));
        }

        size_t worst = 0;
        double res = 0.0;

        for (const auto & left : lefts) {

          worst += outgoing_edge_counts[left];

          const auto resp = mb.GetVals(
              mb.Match(org.Get(uids[left]), outgoing_edge_counts[left])
            );

          for (const auto & right : resp) {
            if (! target.count(left+right) ) res += 1.0;
          }

        }

        return 1.0 - res/worst;
      }
    );
  } else {
    grid_world.SetFitFun(
      [&](MidOrganism<32> & org){

        double res = 0.0;
        double worst = 0.0;

        for (const auto & left : lefts) {
          for (const auto & right : rights) {

            worst += 1.0 - (
              ! target.count(left+right)
              ? cfg.MFM_NOMATCH_THRESH()
              : cfg.MFM_MATCH_THRESH()
            );

            res += ! target.count(left+right)
            ? std::max(0.0, std::abs(
              1.0 - metric(org.Get(uids[left]), org.Get(uids[right]))
            ) - cfg.MFM_NOMATCH_THRESH())
            : std::max(0.0, std::abs(
              target[left+right]
              - metric(org.Get(uids[left]), org.Get(uids[right]))
            ) - cfg.MFM_MATCH_THRESH());

          }
        }

        return 1.0 - res/worst;
      }
    );
  }

  // wrap up into subgrids
  grid_world.SetGetNeighborFun([&](emp::WorldPosition pos){

    const size_t size_x = grid_world.GetWidth();
    const size_t size_y = grid_world.GetHeight();
    const size_t id = pos.GetIndex();

    const size_t orig_x = id%size_x;
    const size_t orig_y = id/size_x;

    // fancy footwork to exclude self (4) from consideration
    const int offset = (rand.GetInt(8) * 5) % 9;
    int rand_x = (int) orig_x + offset%3 - 1;
    int rand_y = (int) orig_y + offset/3 - 1;

    rand_x = (
      orig_x - emp::Mod(orig_x, static_cast<int>(cfg.MFM_SUBGRID_DIM()))
    ) + emp::Mod(rand_x, static_cast<int>(cfg.MFM_SUBGRID_DIM()));

    rand_y = (
      orig_y - emp::Mod(orig_y, static_cast<int>(cfg.MFM_SUBGRID_DIM()))
    ) + emp::Mod(rand_y, static_cast<int>(cfg.MFM_SUBGRID_DIM()));

    const auto neighbor_id = (
      emp::Mod(rand_x, (int) size_x)
      + emp::Mod(rand_y, (int) size_y) * (int)size_x
    );

    emp_assert((int)pos.GetIndex() != neighbor_id);

    return pos.SetIndex(neighbor_id);

  });


  grid_world.SetAutoMutate();
  grid_world.OnUpdate([&grid_world, &cfg](size_t upd){
    emp::TournamentSelect(
      grid_world,
      cfg.MFM_TOURNEY_SIZE(),
      cfg.MFM_TOURNEY_REPS()
    );
  });

  // swap edges
  // note: incoming edge counts won't change
  grid_world.OnUpdate(
    [&](size_t upd){
      if (cfg.MFM_GENS_PER_TARGET() && upd % cfg.MFM_GENS_PER_TARGET() == 0) {
        incr_instance();
        for (size_t t = 0; t < cfg.MFM_SUBGRID_TRANSFERS(); ++t) {
          const auto source = grid_world.GetRandomOrgID();
          grid_world.AddOrgAt(
            emp::NewPtr<MidOrganism<32>>(grid_world.GetOrg(source)),
            grid_world.GetRandomOrgID(),
            source
          );
        }
      }
    }
  );

  // POP_SIZE needs to be a perfect square.
  emp_assert(grid_world.GetSize() == cfg.MFM_POP_SIZE());

  for (size_t i = 0; i < cfg.MFM_POP_SIZE(); ++i) {
    MidOrganism<32> org(cfg, rand);
    grid_world.InjectAt(org, i);
  }

  std::cout << "Metric " << metric.name() << std::endl;
  for (size_t g = 0; g < cfg.MFM_GENS(); ++g) {
    grid_world.Update();
    std::cout << ".";
    std::cout.flush();
  }

  std::cout << std::endl;

  // [&](){
  //   emp::DataFile df(emp::keyname::pack({
  //     {"bitweight", emp::to_string(cfg.MO_BITWEIGHT())},
  //     {"metric-slug", emp::slugify(metric.name())},
  //     {"experiment", cfg.MFM_TITLE()},
  //     {"datafile", "end-census"},
  //     {"treatment", cfg.TREATMENT()},
  //     {"seed", emp::to_string(cfg.SEED())},
  //     {"fit-fun", cfg.MFM_RANKED() ? "ranked" : "scored"},
  //     // {"_emp_hash=", STRINGIFY(EMPIRICAL_HASH_)},
  //     // {"_source_hash=", STRINGIFY(DISHTINY_HASH_)},
  //     {"ext", ".csv"}
  //   }));
  //
  //   size_t pop_id;
  //   size_t pos;
  //   df.AddVar(pop_id, "Population ID");
  //   df.AddVar(pos, "Genome Position");
  //   df.AddFun<double>(
  //     [&](){
  //       const auto & compare_to = grid_world.GetOrg(pop_id).Get(pos);
  //       double res = 0.0;
  //       for (size_t r = 0; r < cfg.LSA_NREPS(); ++r) {
  //         const decltype(compare_to) sample{rand, cfg.MO_BITWEIGHT()};
  //         res += metric(compare_to, sample);
  //       }
  //       return res / cfg.LSA_NREPS();
  //     },
  //     "Specificity"
  //   );
  //
  //   df.PrintHeaderKeys();
  //
  //   for (pop_id = 0; pop_id < grid_world.size(); ++pop_id) {
  //     for (pos = 0; pos < cfg.MO_LENGTH(); ++pos) df.Update();
  //   }
  // }();


  // evaluate modularity
  [&](){

    emp::DataFile df(emp::keyname::pack({
      {"bitweight", emp::to_string(cfg.MO_BITWEIGHT())},
      {"metric-slug", emp::slugify(metric.name())},
      {"experiment", cfg.MFM_TITLE()},
      {"datafile", "modularity-census"},
      {"treatment", cfg.TREATMENT()},
      {"seed", emp::to_string(cfg.SEED())},
      {"fit-fun", cfg.MFM_RANKED() ? "ranked" : "scored"},
      // {"_emp_hash=", STRINGIFY(EMPIRICAL_HASH_)},
      // {"_source_hash=", STRINGIFY(DISHTINY_HASH_)},
      {"ext", ".csv"}
    }));

    size_t pop_id;
    df.AddVar(pop_id, "Individual");

    // number of mutational steps to first phenotypic change
    emp::DataNode<size_t, emp::data::Range> step;
    df.AddMean(step, "Mutational Distance", "TODO", true);

    // cross-component activation before first phenotypic chagne
    double cca_before;
    df.AddVar(cca_before, "Initial Cross-Component Activation");

    // cross-component activation before first phenotypic chagne
    double scaled_cca_before;
    df.AddVar(scaled_cca_before, "Initial Per-Possibility Cross-Component Activation");

    // cross-component activation after first phenotypic chagne
    emp::DataNode<double, emp::data::Range> cca_after;
    df.AddMean(cca_after, "Final Cross-Component Activation", "TODO", true);

    emp::DataNode<double, emp::data::Range> scaled_cca_after;
    df.AddMean(scaled_cca_after, "Final Per-Possibility Cross-Component Activation", "TODO", true);

    // total number of phenotypic changes that occur at once
    emp::DataNode<size_t, emp::data::Range> phen_diff;
    df.AddMean(phen_diff, "Phenotypic Changes", "TODO", true);

    // number of phenotypic traits assessed
    // (a.k.a. number of phenotypic change opportunities)
    size_t phen_size = 0;
    df.AddVar(phen_size, "Phenotypic Size");

    // what num of phenotypic change opportunities are cross-component
    size_t opp_cca = 0;
    df.AddVar(opp_cca, "Opportunities for Cross-Component Phenotypic Change");

    // what num of phenotypic change opportunities are same-component
    size_t opp_sca = 0;
    df.AddVar(opp_sca, "Opportunities for Same-Component Phenotypic Change");

    // what num of phenotypic changes are cross-component
    emp::DataNode<size_t, emp::data::Range> count_cca;
    df.AddMean(count_cca, "Cross-Component Phenotypic Changes", "TODO", true);

    // what num of phenotypic change are same-component
    emp::DataNode<size_t, emp::data::Range> count_sca;
    df.AddMean(count_sca, "Same-Component Phenotypic Changes", "TODO", true);

    // what proportion of phenotypic change are cross-component
    emp::DataNode<double, emp::data::Range> prop_cca;
    df.AddMean(prop_cca, "Proportion Cross-Component Phenotypic Change", "TODO", true);

    // what proportion of phenotypic change are same-component
    emp::DataNode<double, emp::data::Range> prop_sca;
    df.AddMean(prop_sca, "Proportion Same-Component Phenotypic Change", "TODO", true);

    // what proportion of phenotypic change are cross-component
    // scaled by possible scas vs ccas
    emp::DataNode<double, emp::data::Range> scaled_prop_cca;
    df.AddMean(scaled_prop_cca, "Per-Possibility Proportion Cross-Component Phenotypic Change");

    // what proportion of phenotypic change are same-component
    // scaled by possible scas vs ccas
    emp::DataNode<double, emp::data::Range> scaled_prop_sca;
    df.AddMean(scaled_prop_sca, "Per-Possibility Proportion Same-Component Phenotypic Change");

    emp_assert(cfg.MFM_RANKED() == "ranked");

    df.PrintHeaderKeys();

    for (pop_id = 0; pop_id < grid_world.GetSize(); ++pop_id) {

      std::unordered_map<std::string, size_t> module_left_counts;
      std::unordered_map<std::string, size_t> module_right_counts;

      for (const auto & left : lefts) {
        module_left_counts[emp::keyname::unpack(left)["module"]]++;
      }

      for (const auto & right : rights) {
        module_right_counts[emp::keyname::unpack(right)["module"]]++;
      }

      for (const auto & [module, val] : module_left_counts) {
        opp_cca += val * (rights.size() - module_right_counts[module]);
        opp_sca += val * module_right_counts[module];
      }

      emp::MatchBin<
        std::string,
        WrapperMetric<32>,
        emp::RankedSelector<>
      > mb(rand);
      mb.metric.metric = &metric;
      mb.SetCacheOn(false);

      for (const auto & right : rights) mb.Put(right, grid_world.GetOrg(pop_id).Get(uids[right]));

      for (const auto & left : lefts) {

        const auto resp = mb.GetVals(
            mb.Match(grid_world.GetOrg(pop_id).Get(uids[left]), outgoing_edge_counts[left])
          );

        for (const auto & right : resp) {
          phen_size++;
          if (
            emp::keyname::unpack(left)["module"]
            !=
            emp::keyname::unpack(right)["module"]
          ) {
            opp_cca--;
          } else {
            opp_sca--;
          }
        }

      }

      auto calc_cca = [&](const MidOrganism<32> & org){

        emp::MatchBin<
          std::string,
          WrapperMetric<32>,
          emp::RankedSelector<>
        > mb(rand);
        mb.metric.metric = &metric;
        mb.SetCacheOn(false);

        for (const auto & right : rights) mb.Put(right, org.Get(uids[right]));

        size_t cca = 0;
        size_t sca = 0;

        for (const auto & left : lefts) {

          const auto resp = mb.GetVals(
              mb.Match(org.Get(uids[left]), outgoing_edge_counts[left])
            );

          for (const auto & right : resp) {
            if (
              emp::keyname::unpack(left)["module"]
              !=
              emp::keyname::unpack(right)["module"]
            ) {
              ++cca;
            } else {
              ++sca;
            }
          }

        }

        return static_cast<double>(cca) / static_cast<double>(cca + sca);
      };

      cca_before = calc_cca(grid_world.GetOrg(pop_id));
      scaled_cca_before = scaled_cca_before / static_cast<double>(opp_cca);

      for (size_t rep = 0; rep < cfg.MFM_COMPONENT_WALK_REPS(); ++rep) {

        size_t phen_diff_tally = 0;
        size_t count_cca_tally = 0;
        size_t count_sca_tally = 0;
        size_t step_tally = 0;

        MidOrganism<32> walker(grid_world.GetOrg(pop_id));

        for (step_tally = 1; !phen_diff_tally; ++step_tally) {

          // perform one mutational step
          for (bool flip = false; !flip; ) {
            const size_t target_bs = rand.GetUInt(walker.bsets.size());
            const size_t target_bit = rand.GetUInt(
              walker.bsets[target_bs].size()
            );
            flip = (
              rand.P(cfg.MO_BITWEIGHT())
              !=
              walker.bsets[target_bs].Get(target_bit)
            );
            if (flip) { walker.bsets[target_bs].Toggle(target_bit); }
          }

          // set up match bins
          emp::MatchBin<
            std::string,
            WrapperMetric<32>,
            emp::RankedSelector<>
          > mb(rand);
          mb.metric.metric = &metric;
          mb.SetCacheOn(false);

          emp::MatchBin<
            std::string,
            WrapperMetric<32>,
            emp::RankedSelector<>
          > mb_orig(rand);
          mb_orig.metric.metric = &metric;
          mb_orig.SetCacheOn(false);

          for (const auto & right : rights) {
            mb.Put(right, walker.Get(uids[right]));
            mb_orig.Put(right, grid_world.GetOrg(pop_id).Get(uids[right]));
          }

          // compare phenotypes, count changes
          for (const auto & left : lefts) {

            const auto resp = mb.GetVals(
                mb.Match(walker.Get(uids[left]), outgoing_edge_counts[left])
              );
            const auto resp_orig = mb_orig.GetVals(
                mb_orig.Match(grid_world.GetOrg(pop_id).Get(uids[left]), outgoing_edge_counts[left])
              );
            const std::unordered_set<std::string> set_orig(
              std::begin(resp_orig), std::end(resp_orig)
            );

            for (const auto & right : resp) {
              if (!set_orig.count(right)) {
                ++phen_diff_tally;
                if (
                  emp::keyname::unpack(left)["module"]
                  !=
                  emp::keyname::unpack(right)["module"]
                ) {
                  ++count_cca_tally;
                } else {
                  ++count_sca_tally;
                }
              }
            }

          }

        } // mutational walk

        phen_diff.Add(phen_diff_tally);
        count_cca.Add(count_cca_tally);
        count_sca.Add(count_sca_tally);
        step.Add(step_tally);

        const double cca_after_res = calc_cca(walker);
        cca_after.Add(cca_after_res);
        scaled_cca_after.Add(
          cca_after_res / static_cast<double>(opp_cca)
        );

        const double prop_cca_res = (
          static_cast<double>(count_cca_tally)
          / static_cast<double>(phen_diff_tally)
        );
        const double prop_sca_res = (
          static_cast<double>(count_sca_tally)
          / static_cast<double>(phen_diff_tally)
        );

        prop_cca.Add(prop_cca_res);
        prop_sca.Add(prop_sca_res);

        scaled_prop_cca.Add(
          prop_cca_res * (opp_cca + opp_sca) / opp_cca
        );
        scaled_prop_sca.Add(
          prop_sca_res * (opp_cca + opp_sca) / opp_sca
        );

      } // replicate mutational walk

      df.Update();

    } // pop_id
  }();

  }
}
