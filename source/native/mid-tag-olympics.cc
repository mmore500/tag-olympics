//  This file is part of Tag Olympics
//  Copyright (C) Matthew Andres Moreno, 2019.
//  Released under MIT license; see LICENSE

#include <iostream>

#include "config/command_line.h"
#include "tools/MatchBin.h"
#include "config/ArgManager.h"

#include "../Config.h"
#include "../MidFlexBiMatch.h"
#include "../MidFlexBiMatchNK.h"
#include "../MakeMetricKey.h"
#include "../Metrics.h"
#include "../MetricsMid.h"
// #include "../MetricsMidModularity.h"

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

  const MetricsMid m;
  // const MetricsMidModularity mod_m;
  const Metrics::collection_t & metrics = m.mets;
  // const Metrics::collection_t & mod_metrics = mod_m.mets;

  if (!res) {
    std::cout << "no run type provided" << std::endl;
  } else if (res->size() > 1) {
    std::cout << "multiple run types provided" << std::endl;
  } else if (res->at(0) == "MBM") {
    std::cout << "running mode: " << res->at(0) << std::endl;
    MidFlexBiMatch(metrics, cfg);
  } else if (res->at(0) == "MBM_NK") {
    std::cout << "running mode: " << res->at(0) << std::endl;
    MidFlexBiMatchNK(metrics, cfg);
  } else if (res->at(0) == "MBMM") {
    std::cout << "running mode: " << res->at(0) << std::endl;
    // MidFlexBiMatch(mod_metrics, cfg);
  } else if (res->at(0) == "MMK") {
    std::cout << "running mode: " << res->at(0) << std::endl;
    MakeMetricKey(metrics, cfg);
  } else {
    std::cout << "uknown running mode: " << res->at(0) << std::endl;
  }
}
