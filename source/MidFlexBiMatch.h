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
  grid_world.SetAutoMutate();
  grid_world.OnUpdate([&grid_world, &cfg](size_t upd){
    emp::LocalTournamentSelect(
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

  [&](){
    emp::DataFile df(emp::keyname::pack({
      {"bitweight", emp::to_string(cfg.MO_BITWEIGHT())},
      {"metric-slug", emp::slugify(metric.name())},
      {"experiment", cfg.MFM_TITLE()},
      {"datafile", "end-census"},
      {"treatment", cfg.TREATMENT()},
      {"seed", emp::to_string(cfg.SEED())},
      {"fit-fun", cfg.MFM_RANKED() ? "ranked" : "scored"},
      // {"_emp_hash=", STRINGIFY(EMPIRICAL_HASH_)},
      // {"_source_hash=", STRINGIFY(DISHTINY_HASH_)},
      {"ext", ".csv"}
    }));

    size_t pop_id;
    size_t pos;
    df.AddVar(pop_id, "Population ID");
    df.AddVar(pos, "Genome Position");
    df.AddFun<double>(
      [&](){
        const auto & compare_to = grid_world.GetOrg(pop_id).Get(pos);
        double res = 0.0;
        for (size_t r = 0; r < cfg.LSA_NREPS(); ++r) {
          const decltype(compare_to) sample{rand, cfg.MO_BITWEIGHT()};
          res += metric(compare_to, sample);
        }
        return res / cfg.LSA_NREPS();
      },
      "Specificity"
    );

    df.PrintHeaderKeys();

    for (pop_id = 0; pop_id < grid_world.size(); ++pop_id) {
      for (pos = 0; pos < cfg.MO_LENGTH(); ++pos) df.Update();
    }
  }();

  [&](){

    const MidOrganism<32> & best = [&grid_world](){
      double best_fit = grid_world.CalcFitnessID(0);
      size_t best_id = 0;

      // Search for a higher fit org in the tournament.
      for (size_t i = 1; i < grid_world.size(); i++) {
        const double cur_fit = grid_world.CalcFitnessID(i);
        if (cur_fit > best_fit) {
          best_fit = cur_fit;
          best_id = i;
        }
      }

      return grid_world.GetOrg(best_id);
    }();

    emp::DataFile df(emp::keyname::pack({
      {"bitweight", emp::to_string(cfg.MO_BITWEIGHT())},
      {"metric-slug", emp::slugify(metric.name())},
      {"experiment", cfg.MFM_TITLE()},
      {"datafile", "cross-component-activation"},
      {"treatment", cfg.TREATMENT()},
      {"seed", emp::to_string(cfg.SEED())},
      {"fit-fun", cfg.MFM_RANKED() ? "ranked" : "scored"},
      // {"_emp_hash=", STRINGIFY(EMPIRICAL_HASH_)},
      // {"_source_hash=", STRINGIFY(DISHTINY_HASH_)},
      {"ext", ".csv"}
    }));

    size_t rep;
    size_t step; // number of mutational steps to first phenotypic change

    double cca_before; // cross-component activation
                       // before first phenotypic chagne
    double cca_after; // cross-component activation
                      // after first phenotypic chagne

    size_t phen_diff = 0; // total number of phenotypic changes
    size_t phen_size; // number of phenotypic traits assessed

    size_t count_cca = 0; // what num of phenotypic changes are cross-component
    size_t count_sca = 0; // what num of phenotypic change are same-component
    double prop_cca; // what num of phenotypic change are same-component
    double prop_sca; // what num of phenotypic change are same-component

    size_t opp_cca = 0; // what num of phenotypic change opportunities
                        // are cross-component
    size_t opp_sca = 0; // what num of phenotypic change opportunities
                        // are same-component
    double scaled_prop_cca; // scaled by possible scas vs ccas
    double scaled_prop_sca; // scaled by possible scas vs ccas

    std::string measure = cfg.MFM_RANKED() ? "ranked" : "scored";

    df.AddVar(rep, "Replicate");
    df.AddVar(step, "Mutational Distance");
    df.AddVar(cca_before, "Initial Cross-Component Activation");
    df.AddVar(cca_after, "Final Cross-Component Activation");
    df.AddVar(phen_diff, "Phenotypic Changes");
    df.AddVar(phen_size, "Phenotypic Size");
    df.AddVar(count_cca, "Cross-Component Phenotypic Changes");
    df.AddVar(count_sca, "Same-Component Phenotypic Changes");
    df.AddVar(prop_cca, "Proportion Cross-Component Phenotypic Change");
    df.AddVar(prop_sca, "Proportion Same-Component Phenotypic Change");
    df.AddVar(opp_cca, "Opportunities for Cross-Component Phenotypic Change");
    df.AddVar(opp_sca, "Opportunities for Same-Component Phenotypic Change");
    df.AddVar(scaled_prop_cca, "Per-Possibility Proportion Cross-Component Phenotypic Change");
    df.AddVar(scaled_prop_sca, "Per-Possibility Proportion Same-Component Phenotypic Change");

    df.PrintHeaderKeys();

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

    for (const auto & right : rights) mb.Put(right, best.Get(uids[right]));

    for (const auto & left : lefts) {

      const auto resp = mb.GetVals(
          mb.Match(best.Get(uids[left]), outgoing_edge_counts[left])
        );

      for (const auto & right : resp) {
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

    auto calc_cca = cfg.MFM_RANKED()
    ? std::function([&](const MidOrganism<32> & org){

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
    }) : std::function([&](const MidOrganism<32> & org){

      emp::DataNode<double, emp::data::Range> cca;
      emp::DataNode<double, emp::data::Range> sca;

      for (const auto & left : lefts) {
        for (const auto & right : rights) {

          if (
            emp::keyname::unpack(left)["module"]
            !=
            emp::keyname::unpack(right)["module"]
          ) {
            cca.Add(metric(org.Get(uids[left]), org.Get(uids[right])));
          } else {
            sca.Add(metric(org.Get(uids[left]), org.Get(uids[right])));
          }
        }
      }

      return cca.GetMean() - sca.GetMean();

    });

    cca_before = calc_cca(best);

    for (rep = 0; rep < cfg.MFM_COMPONENT_WALK_REPS(); ++rep) {

      phen_diff = 0; // total number of phenotypic changes
      count_cca = 0; // what num of phenotypic changes are cross-component
      count_sca = 0; // what num of phenotypic change are same-component

      MidOrganism<32> walker(best);

      for (step = 1; !phen_diff; ++step) {

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

        if (cfg.MFM_RANKED()) {

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
            mb_orig.Put(right, best.Get(uids[right]));
          }

          phen_size = 0;

          for (const auto & left : lefts) {

            const auto resp = mb.GetVals(
                mb.Match(walker.Get(uids[left]), outgoing_edge_counts[left])
              );
            const auto resp_orig = mb_orig.GetVals(
                mb_orig.Match(best.Get(uids[left]), outgoing_edge_counts[left])
              );
            const std::unordered_set<std::string> set_orig(
              std::begin(resp_orig), std::end(resp_orig)
            );

            for (const auto & right : resp) {
              ++phen_size;
              if (!set_orig.count(right)) {
                ++phen_diff;
                if (
                  emp::keyname::unpack(left)["module"]
                  !=
                  emp::keyname::unpack(right)["module"]
                ) {
                  ++count_cca;
                } else {
                  ++count_sca;
                }
              }
            }

          }

        } else {
          emp_assert(false, "unimplemented");
        }

      }

      cca_after = calc_cca(walker);
      prop_cca = (
        static_cast<double>(count_cca) / static_cast<double>(phen_diff)
      );
      prop_sca = (
        static_cast<double>(count_sca) / static_cast<double>(phen_diff)
      );
      scaled_prop_cca = (
        prop_cca * (opp_cca + opp_sca) / opp_cca
      );
      scaled_prop_sca = (
        prop_sca * (opp_cca + opp_sca) / opp_sca
      );
      df.Update();

    }
  }();

  }
}
