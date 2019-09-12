#pragma once

#include <iostream>
#include <cmath>
#include <unordered_map>
#include <filesystem>
#include <cstdlib>
#include <sstream>
#include <list>

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

void MidFlexBiMatch(const Metrics::metric_t &metric,  const Config &cfg) {

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

  emp::World<MidOrganism<Config::BS_WIDTH()>> grid_world(rand);

  grid_world.SetupFitnessFile(emp::keyname::pack({
    {"metric-slug", emp::slugify(metric.name())},
    {"experiment", cfg.MFM_TITLE()},
    {"datafile", "fitness"},
    {"target-config", cfg.MFM_TARGET_CONFIG()},
    {"target-size", emp::to_string(lefts.size() + rights.size())},
    {"treatment", cfg.TREATMENT()},
    {"seed", emp::to_string(cfg.SEED())},
    {"fit-fun", cfg.MFM_RANKED() ? "ranked" : "scored"},
    // {"_emp_hash=", STRINGIFY(EMPIRICAL_HASH_)},
    // {"_source_hash=", STRINGIFY(DISHTINY_HASH_)},
    {"ext", ".csv"}
  }));

  auto sys = emp::NewPtr<
    emp::Systematics<
      MidOrganism<Config::BS_WIDTH()>,
      MidOrganism<Config::BS_WIDTH()>
    >
  >(
    [](MidOrganism<Config::BS_WIDTH()> & o){ return o; },
    true,
    true,
    false
  );

  // subrid transfers segfaults systematics
  if (!cfg.MFM_SUBGRID_TRANSFERS()) {
  grid_world.AddSystematics(
    sys,
    "systematics"
  );

  grid_world.SetupSystematicsFile(
    "systematics",
    emp::keyname::pack({
      {"metric-slug", emp::slugify(metric.name())},
      {"experiment", cfg.MFM_TITLE()},
      {"datafile", "systematics"},
      {"target-config", cfg.MFM_TARGET_CONFIG()},
      {"target-size", emp::to_string(lefts.size() + rights.size())},
      {"mut", emp::to_string(cfg.MO_MUT_EXPECTED_REDRAWS())},
      {"treatment", cfg.TREATMENT()},
      {"seed", emp::to_string(cfg.SEED())},
      {"fit-fun", cfg.MFM_RANKED() ? "ranked" : "scored"},
      // {"_emp_hash=", STRINGIFY(EMPIRICAL_HASH_)},
      // {"_source_hash=", STRINGIFY(DISHTINY_HASH_)},
      {"ext", ".csv"}
    })
  );
  }

  const size_t side = (size_t) std::sqrt(cfg.MFM_POP_SIZE());

  grid_world.SetPopStruct_Grid(side, side);
  if (cfg.MFM_RANKED()) {
    grid_world.SetFitFun(
      [&]
      (MidOrganism<Config::BS_WIDTH()> & org){

        emp::MatchBin<
          std::string,
          WrapperMetric<Config::BS_WIDTH()>,
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
      [&](MidOrganism<Config::BS_WIDTH()> & org){

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
  if (cfg.MFM_SUBGRID_DIM()) {
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
  }


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
          auto dest = grid_world.GetRandomOrgID();
          // if source == dest, systematics segfaults
          while (source == dest) dest = grid_world.GetRandomOrgID();
          grid_world.AddOrgAt(
            emp::NewPtr<MidOrganism<Config::BS_WIDTH()>>(grid_world.GetOrg(source)),
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
    MidOrganism<Config::BS_WIDTH()> org(
      cfg,
      rand
    );
    grid_world.InjectAt(org, i);
  }

  // [&](){
  //   emp::DataFile df(emp::keyname::pack({
  //     {"metric-slug", emp::slugify(metric.name())},
  //     {"experiment", cfg.MFM_TITLE()},
  //     {"datafile", "end-census"},
  //     {"treatment", cfg.TREATMENT()},
  //     {"mut", emp::to_string(cfg.MO_MUT_EXPECTED_REDRAWS())},
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

  // modularity setup

  emp::DataManager<
    double,
    emp::data::Stats, emp::data::Range,
    emp::data::Log, emp::data::Pull, emp::data::Info
  > mod_man;

  // number of mutational steps to first phenotypic change
  emp::DataNode<size_t, emp::data::Range> step;
  mod_man.New("step").AddPull([&step](){
    const double res = step.GetMean();
    step.Reset();
    return res;
  });
  mod_man.Get("step").SetName("Mutational Distance");

  // cross-component activation before first phenotypic chagne
  double cca_before;
  mod_man.New("cca_before").AddPull([&cca_before](){ return cca_before; });
  mod_man.Get("cca_before").SetName("Initial Cross-Component Activation");

  // cross-component activation before first phenotypic chagne
  double scaled_cca_before;
  mod_man.New("scaled_cca_before").AddPull([&scaled_cca_before](){
    return scaled_cca_before;
  });
  mod_man.Get("scaled_cca_before").SetName(
    "Initial Per-Possibility Cross-Component Activation"
  );

  // cross-component activation after first phenotypic chagne
  emp::DataNode<double, emp::data::Range> cca_after;
  mod_man.New("cca_after").AddPull([&cca_after](){
    const double res = cca_after.GetMean();
    cca_after.Reset();
    return res;
  });
  mod_man.Get("cca_after").SetName("Final Cross-Component Activation");

  emp::DataNode<double, emp::data::Range> scaled_cca_after;
  mod_man.New("scaled_cca_after").AddPull([&scaled_cca_after](){
    const double res = scaled_cca_after.GetMean();
    scaled_cca_after.Reset();
    return res;
  });
  mod_man.Get("scaled_cca_after").SetName(
    "Final Per-Possibility Cross-Component Activation"
  );

  // total number of phenotypic changes that occur at once
  emp::DataNode<size_t, emp::data::Range> phen_diff;
  mod_man.New("phen_diff").AddPull([&phen_diff](){
    const double res = phen_diff.GetMean();
    phen_diff.Reset();
    return res;
  });
  mod_man.Get("phen_diff").SetName("Phenotypic Changes");

  // number of phenotypic traits assessed
  // (a.k.a. number of phenotypic change opportunities)
  size_t phen_size;
  mod_man.New("phen_size").AddPull([&phen_size](){ return phen_size; });
  mod_man.Get("phen_size").SetName("Phenotypic Size");

  // what num of phenotypic change opportunities are cross-component
  size_t opp_cca;
  mod_man.New("opp_cca").AddPull([&opp_cca](){ return opp_cca; });
  mod_man.Get("opp_cca").SetName(
    "Opportunities for Cross-Component Phenotypic Change"
  );

  // what num of phenotypic change opportunities are same-component
  size_t opp_sca;
  mod_man.New("opp_sca").AddPull([&opp_sca](){ return opp_sca; });
  mod_man.Get("opp_sca").SetName(
    "Opportunities for Same-Component Phenotypic Change"
  );

  // what num of phenotypic changes are cross-component
  emp::DataNode<size_t, emp::data::Range> count_cca;
  mod_man.New("count_cca").AddPull([&count_cca](){
    const double res = count_cca.GetMean();
    count_cca.Reset();
    return res;
  });
  mod_man.Get("count_cca").SetName("Cross-Component Phenotypic Changes");

  // what num of phenotypic change are same-component
  emp::DataNode<size_t, emp::data::Range> count_sca;
  mod_man.New("count_sca").AddPull([&count_sca](){
    const double res = count_sca.GetMean();
    count_sca.Reset();
    return res;
  });
  mod_man.Get("count_sca").SetName("Same-Component Phenotypic Changes");

  // what proportion of phenotypic change are cross-component
  emp::DataNode<double, emp::data::Range> prop_cca;
  mod_man.New("prop_cca").AddPull([&prop_cca](){
    const double res = prop_cca.GetMean();
    prop_cca.Reset();
    return res;
  });
  mod_man.Get("prop_cca").SetName(
    "Proportion Cross-Component Phenotypic Change"
  );

  // what proportion of phenotypic change are same-component
  emp::DataNode<double, emp::data::Range> prop_sca;
  mod_man.New("prop_sca").AddPull([&prop_sca](){
    const double res = prop_sca.GetMean();
    prop_sca.Reset();
    return res;
  });
  mod_man.Get("prop_sca").SetName(
    "Proportion Same-Component Phenotypic Change"
  );

  // what proportion of phenotypic change are cross-component
  // scaled by possible scas vs ccas
  emp::DataNode<double, emp::data::Range> scaled_prop_cca;
  mod_man.New("scaled_prop_cca").AddPull([&scaled_prop_cca](){
    const double res = scaled_prop_cca.GetMean();
    scaled_prop_cca.Reset();
    return res;
  });
  mod_man.Get("scaled_prop_cca").SetName(
    "Per-Possibility Proportion Cross-Component Phenotypic Change"
  );

  // what proportion of phenotypic change are same-component
  // scaled by possible scas vs ccas
  emp::DataNode<double, emp::data::Range> scaled_prop_sca;
  mod_man.New("scaled_prop_sca").AddPull([&scaled_prop_sca](){
    const double res = scaled_prop_sca.GetMean();
    scaled_prop_sca.Reset();
    return res;
  });
  mod_man.Get("scaled_prop_sca").SetName(
    "Per-Possibility Proportion Same-Component Phenotypic Change"
  );

  std::string mod_measure;
  std::string mod_statistic;
  double mod_value;

  auto df = emp::DataFile(emp::keyname::pack({
    {"metric-slug", emp::slugify(metric.name())},
    {"experiment", cfg.MFM_TITLE()},
    {"datafile", "modularity-census"},
    {"target-config", cfg.MFM_TARGET_CONFIG()},
    {"target-size", emp::to_string(lefts.size() + rights.size())},
    {"mut", emp::to_string(cfg.MO_MUT_EXPECTED_REDRAWS())},
    {"treatment", cfg.TREATMENT()},
    {"seed", emp::to_string(cfg.SEED())},
    {"fit-fun", cfg.MFM_RANKED() ? "ranked" : "scored"},
    // {"_emp_hash=", STRINGIFY(EMPIRICAL_HASH_)},
    // {"_source_hash=", STRINGIFY(DISHTINY_HASH_)},
    {"ext", ".csv"}
  }));

  df.AddFun(
    std::function([&grid_world](){ return grid_world.GetUpdate(); }),
    "Update"
  );
  df.AddVar(mod_measure, "Measure");
  df.AddVar(mod_statistic, "Statistic");
  df.AddVar(mod_value, "Value");

  df.PrintHeaderKeys();

  grid_world.OnUpdate([&](const size_t update){

    if (
      !update
      || !cfg.MFM_COMPONENT_FREQ()
      || update % cfg.MFM_COMPONENT_FREQ()
    ) return;

    phen_size = 0;
    opp_cca = 0;
    opp_sca = 0;

    for (size_t pop_id = 0; pop_id < grid_world.GetSize(); ++pop_id) {

      if (!(pop_id % 20)) std::cout << "+";
      std::cout.flush();

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
        WrapperMetric<Config::BS_WIDTH()>,
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

      auto calc_cca = [&](const MidOrganism<Config::BS_WIDTH()> & org){

        emp::MatchBin<
          std::string,
          WrapperMetric<Config::BS_WIDTH()>,
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

      emp::MatchBin<
        std::string,
        WrapperMetric<Config::BS_WIDTH()>,
        emp::RankedSelector<>
      > mb_orig(rand);
      mb_orig.metric.metric = &metric;

      for (const auto & right : rights) {
        mb_orig.Put(right, grid_world.GetOrg(pop_id).Get(uids[right]));
      }

      cca_before = calc_cca(grid_world.GetOrg(pop_id));
      scaled_cca_before = scaled_cca_before / static_cast<double>(opp_cca);

      for (size_t rep = 0; rep < cfg.MFM_COMPONENT_WALK_REPS(); ++rep) {

        size_t phen_diff_tally = 0;
        size_t count_cca_tally = 0;
        size_t count_sca_tally = 0;
        size_t step_tally = 0;

        MidOrganism<Config::BS_WIDTH()> walker(grid_world.GetOrg(pop_id));

        for (step_tally = 1; !phen_diff_tally; ++step_tally) {

          // perform one mutational step
          const size_t target_bs = rand.GetUInt(walker.bsets.size());
          walker.bsets[target_bs].Mutate(rand, 1);

          // set up match bins
          emp::MatchBin<
            std::string,
            WrapperMetric<Config::BS_WIDTH()>,
            emp::RankedSelector<>
          > mb(rand);
          mb.metric.metric = &metric;
          mb.SetCacheOn(false);

          for (const auto & right : rights) {
            mb.Put(right, walker.Get(uids[right]));
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

      for (auto & [name, node] : mod_man.GetNodes()) node->PullData();

    } // pop_id

    for (auto & [name, node] : mod_man.GetNodes()) {

      mod_measure = node->GetName();

      mod_statistic = "Mean";
      mod_value = node->GetMean();
      df.Update();

      mod_statistic = "Median";
      mod_value = node->GetMedian();
      df.Update();

      mod_statistic = "Minimum";
      mod_value = node->GetMin();
      df.Update();

      mod_statistic = "Maximum";
      mod_value = node->GetMax();
      df.Update();

      mod_statistic = "Standard Deviation";
      mod_value = node->GetStandardDeviation();
      df.Update();

    }

    mod_man.ResetAll();

  });

  // end modularity setup

  // run the experiment
  for (size_t g = 0; g < cfg.MFM_GENS(); ++g) {
    grid_world.Update();
    std::cout << ".";
    std::cout.flush();
  }

  // neutrality census!
  if (!cfg.MFM_SUBGRID_TRANSFERS()) {

  std::string neut_measure;
  std::string neut_statistic;
  double neut_value;
  size_t step;

  auto df_neut = emp::DataFile(emp::keyname::pack({
    {"metric-slug", emp::slugify(metric.name())},
    {"experiment", cfg.MFM_TITLE()},
    {"datafile", "neutrality-census"},
    {"target-config", cfg.MFM_TARGET_CONFIG()},
    {"target-size", emp::to_string(lefts.size() + rights.size())},
    {"mut", emp::to_string(cfg.MO_MUT_EXPECTED_REDRAWS())},
    {"treatment", cfg.TREATMENT()},
    {"seed", emp::to_string(cfg.SEED())},
    {"fit-fun", cfg.MFM_RANKED() ? "ranked" : "scored"},
    // {"_emp_hash=", STRINGIFY(EMPIRICAL_HASH_)},
    // {"_source_hash=", STRINGIFY(DISHTINY_HASH_)},
    {"ext", ".csv"}
  }));

  df_neut.AddVar(step, "Step");
  df_neut.AddVar(neut_measure, "Measure");
  df_neut.AddVar(neut_statistic, "Statistic");
  df_neut.AddVar(neut_value, "Value");

  df_neut.PrintHeaderKeys();

  emp::DataManager<
    double,
    emp::data::Stats, emp::data::Range,
    emp::data::Log, emp::data::Pull, emp::data::Info
  > neut_man;

  // how many updates ago did this ancestor live?
  double update;
  neut_man.New("update").AddPull([&update](){
    return update;
  });
  neut_man.Get("update").SetName("Updates Elapsed");

  // genetic difference from ancestor to descendant
  double diff;
  neut_man.New("diff").AddPull([&diff](){
    return diff;
  });
  neut_man.Get("diff").SetName("Genetic Distance");

  std::list<
    std::pair<
      decltype(sys->GetTaxonAt(0)),
      decltype(grid_world.GetOrgPtr(0))
    >
  > taxas;
  for (size_t pop_id = 0; pop_id < grid_world.GetSize(); ++pop_id) {
    taxas.push_back({
      sys->GetTaxonAt(pop_id),
      grid_world.GetOrgPtr(pop_id)
    });
  }

  for (step = 0; taxas.size(); ++step) {

    for (auto it = std::begin(taxas); it != std::end(taxas); ++it) {
      update = grid_world.GetUpdate() - it->first->GetOriginationTime();
      diff = it->first->GetInfo().Distance(*(it->second));
      for (auto & [name, node] : neut_man.GetNodes()) node->PullData();
    }

    if (step < cfg.MFM_NEUT_INTERVAL() || step % cfg.MFM_NEUT_INTERVAL() == 0) {
    for (auto & [name, node] : neut_man.GetNodes()) {

      neut_measure = node->GetName();

      neut_statistic = "Mean";
      neut_value = node->GetMean();
      df_neut.Update();

      neut_statistic = "Median";
      neut_value = node->GetMedian();
      df_neut.Update();

      neut_statistic = "Minimum";
      neut_value = node->GetMin();
      df_neut.Update();

      neut_statistic = "Maximum";
      neut_value = node->GetMax();
      df_neut.Update();

      neut_statistic = "Count";
      neut_value = node->GetCount();
      df_neut.Update();

      neut_statistic = "Standard Deviation";
      neut_value = node->GetStandardDeviation();
      df_neut.Update();

    }
    }

    neut_man.ResetAll();

    for (auto it = std::begin(taxas); it != std::end(taxas); ) {
      it->first = it->first->GetParent();
      if (it->first) {
        ++it;
      } else {
        it = taxas.erase(it);
      }
    }

  }

  }

}

void MidFlexBiMatch(const Metrics::collection_t &metrics, const Config &cfg) {
  for (const auto & mptr : metrics) {
    const auto & metric = *mptr;
    std::cout << "Metric " << metric.name() << std::endl;
    MidFlexBiMatch(metric, cfg);
    std::cout << std::endl;
  }
}
