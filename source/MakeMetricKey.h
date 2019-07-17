#pragma once

#include <iostream>
#include <limits>

#include "tools/MatchBin.h"
#include "tools/matchbin_utils.h"
#include "tools/Random.h"
#include "config/ArgManager.h"
#include "data/DataFile.h"

#include "Config.h"
#include "Metrics.h"

void MakeMetricKey(const Metrics &metrics, const Config &cfg) {

  std::string name;
  size_t dim;
  size_t width;
  bool slide_mod;
  bool anti_mod;
  std::string base;

  emp::DataFile df(cfg.MMK_FILE());
  df.AddVar(name, "Metric");
  df.AddVar(base, "Base Metric");
  df.AddVar(dim, "Dimension");
  df.AddVar(width, "Width");
  df.AddVar(slide_mod, "Sliding");
  df.AddVar(anti_mod, "Inverse");
  df.PrintHeaderKeys();

  for (const auto & mptr : metrics.mets) {
    const auto & metric = *mptr;

    name = metric.name() + " Distance";
    dim = metric.dim();
    width = metric.width();
    slide_mod = metric.name().find("Sliding") != std::string::npos;
    anti_mod = metric.name().find("Inverse") != std::string::npos;
    base = metric.base();

    df.Update();
  }

}
