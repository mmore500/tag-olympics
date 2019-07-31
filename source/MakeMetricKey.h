#pragma once

#include <iostream>
#include <limits>

#include "tools/MatchBin.h"
#include "tools/matchbin_utils.h"
#include "tools/Random.h"
#include "config/ArgManager.h"
#include "data/DataFile.h"
#include "tools/string_utils.h"
#include "tools/keyname_utils.h"

#include "Config.h"
#include "Metrics.h"

void MakeMetricKey(
  const Metrics::collection_t &metrics,
  const Config &cfg
) {

  std::string name;
  std::string slug;
  size_t dim;
  size_t width;
  bool slide_mod;
  bool anti_mod;
  std::string base;
  std::string dim_type;

  emp::DataFile df(emp::keyname::pack({
    {"title", cfg.MMK_TITLE()},
    // {"_emp_hash=", STRINGIFY(EMPIRICAL_HASH_)},
    // {"_source_hash=", STRINGIFY(DISHTINY_HASH_)},
    {"ext", ".csv"}
  }));
  df.AddVar(name, "Metric");
  df.AddVar(slug, "Slug");
  df.AddVar(base, "Base Metric");
  df.AddVar(dim, "Dimension");
  df.AddVar(width, "Width");
  df.AddVar(slide_mod, "Sliding");
  df.AddVar(anti_mod, "Inverse");
  df.AddVar(dim_type, "Dimension Type");
  df.PrintHeaderKeys();

  for (const auto & mptr : metrics) {
    const auto & metric = *mptr;

    name = metric.name() + " Distance";
    slug = emp::slugify(metric.name());
    dim = metric.dim();
    width = metric.width();
    slide_mod = metric.name().find("Sliding") != std::string::npos;
    anti_mod = metric.name().find("Inverse") != std::string::npos;
    base = metric.base();

    if (metric.name().find("Minimum") != std::string::npos) {
      dim_type = "Minimum";
    } else if (metric.name().find("Mean") != std::string::npos) {
      dim_type = "Mean";
    } else if (metric.name().find("Euclidean") != std::string::npos) {
      dim_type = "Euclidean";
    } else {
      dim_type = "None";
    }

    df.Update();
  }

}
