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

void MidFlexMatch(const Metrics::collection_t &metrics, const Config &cfg) {

  for (const auto & mptr : metrics) {
  const auto & metric = *mptr;

  emp::Random rand(cfg.SEED());

  const std::string graph_source = [&cfg](){
    std::string graph_source;
    for(auto& p: std::filesystem::directory_iterator(".")) {
      if (
        const auto res = emp::keyname::unpack(p.path());
        res.count("title") && res.at("title") == "mid-graph"
        && res.count("ext") && res.at("ext") == ".csv"
        && res.count("seed") && res.at("seed") == emp::to_string(cfg.SEED())
        && res.count("node-count")
          && res.at("node-count") == emp::to_string(cfg.MO_LENGTH())
        && res.count("nodes-per-component")
        && res.count("base-degree")
        && res.count("extra-edges")
      ) {
        if (graph_source.size()) {
          std::cerr << "conflicting graph source files" << std::endl;
          std::cerr << graph_source << std::endl;
          std::cerr << p.path() << std::endl;
          std::cerr << "exiting..." << std::endl;
          std::exit(EXIT_FAILURE);
        }
        graph_source = p.path();
      }
    }

    if (!graph_source.size()) {
      std::cerr << "no graph source file found" << std::endl;
      std::cerr << "exiting..." << std::endl;
      std::exit(EXIT_FAILURE);
    }

    return graph_source;
  }();

  emp::File f(graph_source);

  f.RemoveComments('#');
  f.RemoveEmpty();

  emp_assert(f.size() == cfg.MO_LENGTH());

  emp::vector<emp::vector<double>> target(
    cfg.MO_LENGTH(),
    emp::vector<double>(cfg.MO_LENGTH(), 1.0)
  );

  const size_t nodes_per_component = [&graph_source](){
    size_t res;
    std::istringstream(
      emp::keyname::unpack(graph_source).at("nodes-per-component")
    ) >> res;
    return res;
  }();
  const size_t num_components = cfg.MO_LENGTH() / nodes_per_component;

  std::unordered_map<size_t, size_t> incoming_edge_counts;
  emp::vector<emp::vector<std::array<size_t, 2>>> edges(num_components);
  while (f.size()) {
    const auto res = f.ExtractRowAs<size_t>();
    for (size_t i = 1; i < res.size(); ++i) {
      target[res[0]][res[i]] = rand.GetDouble(cfg.MFM_TARGET_MAX());
      target[res[i]][res[0]] = target[res[i]][res[0]];
      edges[res[0]/nodes_per_component].push_back(
        std::array<size_t, 2>{res[0], res[i]}
      );

      incoming_edge_counts[res[0]]++;
      incoming_edge_counts[res[i]]++;

    }
  }

  emp::World<MidOrganism<Config::BS_WIDTH()>> grid_world(rand);

  grid_world.SetupFitnessFile(emp::keyname::pack({
    {"bitweight", emp::to_string(cfg.MO_BITWEIGHT())},
    {"metric-slug", emp::slugify(metric.name())},
    {"experiment", cfg.MFM_TITLE()},
    {"datafile", "fitness"},
    {"treatment", cfg.TREATMENT()},
    {"seed", emp::to_string(cfg.SEED())},
    {"node-count", emp::keyname::unpack(graph_source).at("node-count")},
    {"nodes-per-component", emp::keyname::unpack(graph_source).at("nodes-per-component")},
    {"base-degree", emp::keyname::unpack(graph_source).at("base-degree")},
    {"extra-edges", emp::keyname::unpack(graph_source).at("extra-edges")},
    {"fit-fun", cfg.MFM_RANKED() ? "ranked" : "scored"},
    // {"_emp_hash=", STRINGIFY(EMPIRICAL_HASH_)},
    // {"_source_hash=", STRINGIFY(DISHTINY_HASH_)},
    {"ext", ".csv"}
  }));

  grid_world.AddSystematics(
    emp::NewPtr<
      emp::Systematics<
        MidOrganism<Config::BS_WIDTH()>,
        MidOrganism<Config::BS_WIDTH()>
      >
    >(
      [](MidOrganism<Config::BS_WIDTH()> & o){ return o; },
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
      {"node-count", emp::keyname::unpack(graph_source).at("node-count")},
      {"nodes-per-component", emp::keyname::unpack(graph_source).at("nodes-per-component")},
      {"base-degree", emp::keyname::unpack(graph_source).at("base-degree")},
      {"extra-edges", emp::keyname::unpack(graph_source).at("extra-edges")},
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
      [&target, &cfg, &metric, &incoming_edge_counts, &rand]
      (MidOrganism<Config::BS_WIDTH()> & org){

        emp::MatchBin<
          size_t,
          WrapperMetric<Config::BS_WIDTH()>,
          emp::RankedSelector<>
        > mb(rand);
        mb.metric.metric = &metric;
        mb.SetCacheOn(false);
        const bool anti = metric.name().find("Inverse") != std::string::npos;

        for(size_t i = 0; i < cfg.MO_LENGTH(); ++i) {
          mb.Put(i, org.Get(i));
        }

        size_t worst = 0;
        double res = 0.0;

        for (size_t i = 0; i < cfg.MO_LENGTH(); ++i) {
          for (size_t j = 0; j < cfg.MO_LENGTH(); ++j) {
            // ignore self-loops
            if (i == j) continue;
            // non-edges cause fitness harm by inadvertently showing up
            // in another tag's match list
            if (target[i][j] == 1.0) continue;

            ++worst;

            if (const auto resp = mb.GetVals(
                    // +1 to allow for self-matching in non-anti metrics
                    mb.Match(org.Get(i), incoming_edge_counts[j] + !anti)
                  );
                !std::count(resp.begin(), resp.end(), j)
              ) res += metric(org.Get(i), org.Get(j));

          }
        }

        return 1.0 - res/worst;
      }
    );
  } else {
    grid_world.SetFitFun(
      [&target, &cfg, &metric](MidOrganism<Config::BS_WIDTH()> & org){

        double res = 0.0;
        double worst = 0.0;

        for (size_t i = 0; i < cfg.MO_LENGTH(); ++i) {
          for (size_t j = 0; j < cfg.MO_LENGTH(); ++j) {

            if (i == j) continue;

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
    [&cfg, &target, &edges, &rand, nodes_per_component, num_components]
    (size_t upd){
      for (size_t c = 0; c < num_components; ++c) {
        for (size_t s = 0; s < cfg.MFM_SWAPS_PER_GEN(); ++s) {

          const auto edge_ids = emp::Choose(rand, edges[c].size(), 2);

          auto & edge_a = edges[c][edge_ids[0]];
          auto & from_a = edge_a[0];
          auto & to_a = edge_a[1];

          auto & edge_b = edges[c][edge_ids[1]];
          auto & from_b = edge_b[0];
          auto & to_b = edge_b[1];

          std::swap(target[from_a][to_a], target[from_a][to_b]);
          std::swap(target[to_a][from_a], target[to_b][from_a]);

          std::swap(target[from_b][to_b], target[from_b][to_a]);
          std::swap(target[to_b][from_b], target[to_a][from_b]);

          std::swap(from_a, from_b);
        }
      }
    }
  );

  // POP_SIZE needs to be a perfect square.
  emp_assert(grid_world.GetSize() == cfg.MFM_POP_SIZE());

  for (size_t i = 0; i < cfg.MFM_POP_SIZE(); ++i) {
    MidOrganism<Config::BS_WIDTH()> org(cfg, rand);
    grid_world.InjectAt(org, i);
  }

  std::cout << "Metric " << metric.name() << std::endl;
  for (size_t g = 0; g < cfg.MFM_GENS(); ++g) {
    grid_world.Update();
    std::cout << ".";
    std::cout.flush();
  }

  std::cout << std::endl;

  [&cfg, &rand, &grid_world, &metric, &graph_source](){
    emp::DataFile df(emp::keyname::pack({
      {"bitweight", emp::to_string(cfg.MO_BITWEIGHT())},
      {"metric-slug", emp::slugify(metric.name())},
      {"experiment", cfg.MFM_TITLE()},
      {"datafile", "end-census"},
      {"treatment", cfg.TREATMENT()},
      {"seed", emp::to_string(cfg.SEED())},
      {"node-count", emp::keyname::unpack(graph_source).at("node-count")},
      {"nodes-per-component", emp::keyname::unpack(graph_source).at("nodes-per-component")},
      {"base-degree", emp::keyname::unpack(graph_source).at("base-degree")},
      {"extra-edges", emp::keyname::unpack(graph_source).at("extra-edges")},
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
      [&cfg, &rand, &grid_world, &metric, &pop_id, &pos](){
        const auto & target = grid_world.GetOrg(pop_id).Get(pos);
        double res = 0.0;
        for (size_t r = 0; r < cfg.LSA_NREPS(); ++r) {
          const decltype(target) sample{rand, cfg.MO_BITWEIGHT()};
          res += metric(target, sample);
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

    const MidOrganism<Config::BS_WIDTH()> & best = [&grid_world](){
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
      {"node-count", emp::keyname::unpack(graph_source).at("node-count")},
      {"nodes-per-component", emp::keyname::unpack(graph_source).at("nodes-per-component")},
      {"base-degree", emp::keyname::unpack(graph_source).at("base-degree")},
      {"extra-edges", emp::keyname::unpack(graph_source).at("extra-edges")},
      {"fit-fun", cfg.MFM_RANKED() ? "ranked" : "scored"},
      // {"_emp_hash=", STRINGIFY(EMPIRICAL_HASH_)},
      // {"_source_hash=", STRINGIFY(DISHTINY_HASH_)},
      {"ext", ".csv"}
    }));

    size_t rep;
    size_t step;
    double res;
    std::string measure;
    df.AddVar(rep, "Replicate");
    df.AddVar(step, "Mutational Step");
    df.AddVar(measure, "Cross-Component Activation Measure");
    df.AddVar(res, "Cross-Component Activation");

    df.PrintHeaderKeys();

    for (rep = 0; rep < cfg.MFM_COMPONENT_WALK_REPS(); ++rep) {
      MidOrganism<Config::BS_WIDTH()> walker = best;
      for (step = 0; step < cfg.MFM_COMPONENT_WALK_LENGTH(); ++step) {
        measure = "Ranked";
        res = [&](){
          emp::MatchBin<
            size_t,
            WrapperMetric<Config::BS_WIDTH()>,
            emp::RankedSelector<>
          > mb(rand);
          mb.metric.metric = &metric;
          mb.SetCacheOn(false);
          const bool anti = metric.name().find("Inverse") != std::string::npos;

          for(size_t i = 0; i < cfg.MO_LENGTH(); ++i) mb.Put(i, walker.Get(i));

          size_t res = 0;
          size_t worst = 0;

          for (size_t i = 0; i < cfg.MO_LENGTH(); ++i) {
            const size_t component = i / nodes_per_component;

            for (size_t j = 0; j < cfg.MO_LENGTH(); ++j) {
              // ignore self-loops
              if (i == j) continue;
              // non-edges cause fitness harm by inadvertently showing up
              // in another tag's match list
              if (target[i][j] == 1.0) continue;

              worst += incoming_edge_counts[j];
              const auto resp = mb.GetVals(
                // +1 to allow for self-matching in non-anti metrics
                mb.Match(walker.Get(i), incoming_edge_counts[j] + !anti)
              );
              res += std::count_if(
                resp.begin(),
                resp.end(),
                [component, nodes_per_component](size_t node){
                  return node / nodes_per_component != component;
                }
              );
            }
          }

          return static_cast<double>(res)/worst;
        }();
        df.Update();

        measure = "Scored";
        res = [&cfg, &metric, &walker, &target, nodes_per_component](){
          double res = 0.0;
          double worst = 0.0;
          for (size_t i = 0; i < cfg.MO_LENGTH(); ++i) {

            const size_t source_component = i / nodes_per_component;

            for (size_t j = 0; j < cfg.MO_LENGTH(); ++j) {

              const size_t dest_component = j / nodes_per_component;

              if (source_component == dest_component) continue;

              worst += cfg.MFM_NOMATCH_THRESH();
              res += std::max(
                0.0,
                cfg.MFM_NOMATCH_THRESH() - metric(walker.Get(i), walker.Get(j))
              );
            }
          }

          return res/worst;
        }();
        df.Update();

        grid_world.DoMutationsOrg(walker);

      }

    }
  }();

  }
}
