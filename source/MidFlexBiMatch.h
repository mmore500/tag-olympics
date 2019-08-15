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
    uids.clear();
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

      uids[res[0]] = uids.size();

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
    size_t step;
    double res;
    std::string measure = cfg.MFM_RANKED() ? "ranked" : "scored";
    df.AddVar(rep, "Replicate");
    df.AddVar(step, "Mutational Step");
    df.AddVar(measure, "Cross-Component Activation Measure");
    df.AddVar(res, "Cross-Component Activation");

    df.PrintHeaderKeys();

    for (rep = 0; rep < cfg.MFM_COMPONENT_WALK_REPS(); ++rep) {
      MidOrganism<32> walker = best;
      for (step = 0; step < cfg.MFM_COMPONENT_WALK_LENGTH(); ++step) {

        res = grid_world.GetFitFun()(walker);

        df.Update();

        grid_world.DoMutationsOrg(walker);

      }

    }
  }();

  }
}
