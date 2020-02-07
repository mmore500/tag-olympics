//  This file is part of Tag Olympics
//  Copyright (C) Matthew Andres Moreno, 2019.
//  Released under MIT license; see LICENSE

#include <iostream>

#include "config/command_line.h"
#include "tools/MatchBin.h"
#include "config/ArgManager.h"

#include "../Config.h"
#include "../LowMutationalWalk.h"
#include "../LowMutationalStep.h"
#include "../LowDimensionality.h"
#include "../LowMakeDigraph.h"
#include "../LowMakeGraph.h"
#include "../LowTripletAnalysis.h"
#include "../LowSpecificityAnalysis.h"
#include "../LowScoreDistribution.h"
#include "../MakeMetricKey.h"
#include "../Metrics.h"
#include "../MetricsLow.h"

// This is the main function for the NATIVE version of Tag Olympics.

int main(int argc, char* argv[])
{

  // Read configs.
  Config cfg;
  cfg.Read(cfg.FILENAME());

  emp::ArgManager am(
    argc,
    argv,
    emp::ArgManager::make_builtin_specs(&cfg)
  );

  if (!am.ProcessBuiltin(&cfg)) return 1;

  std::cout << "==============================" << std::endl;
  std::cout << "|    How am I configured?    |" << std::endl;
  std::cout << "==============================" << std::endl;
  cfg.Write(std::cout);
  std::cout << "==============================\n"
           << std::endl;

  const auto res = am.UseArg("_positional");

  const MetricsLow m;
  const Metrics::collection_t & metrics = m.mets;

  if (!res) {
    std::cout << "no run type provided" << std::endl;
  } else if (res->size() > 1) {
    std::cout << "multiple run types provided" << std::endl;
  } else if (res->at(0) == "LMW") {
    std::cout << "running mode: " << res->at(0) << std::endl;
    LowMutationalWalk(metrics, cfg);
  } else if (res->at(0) == "LMS") {
    std::cout << "running mode: " << res->at(0) << std::endl;
    LowMutationalStep(metrics, cfg);
  } else if (res->at(0) == "LMD") {
    std::cout << "running mode: " << res->at(0) << std::endl;
    LowMakeDigraph(metrics, cfg);
  } else if (res->at(0) == "LMG") {
    std::cout << "running mode: " << res->at(0) << std::endl;
    LowMakeGraph(metrics, cfg);
  } else if (res->at(0) == "LTA") {
    std::cout << "running mode: " << res->at(0) << std::endl;
    LowTripletAnalysis(metrics, cfg);
  } else if (res->at(0) == "LSA") {
    std::cout << "running mode: " << res->at(0) << std::endl;
    LowSpecificityAnalysis(metrics, cfg);
  } else if (res->at(0) == "LSD") {
    std::cout << "running mode: " << res->at(0) << std::endl;
    LowScoreDistribution(metrics, cfg);
  } else if (res->at(0) == "LD") {
    std::cout << "running mode: " << res->at(0) << std::endl;
    LowDimensionality(metrics, cfg);
  } else if (res->at(0) == "MMK") {
    std::cout << "running mode: " << res->at(0) << std::endl;
    MakeMetricKey(metrics, cfg);
  } else {
    std::cout << "uknown running mode: " << res->at(0) << std::endl;
  }
}
