//  This file is part of Tag Olympics
//  Copyright (C) Matthew Andres Moreno, 2019.
//  Released under MIT license; see LICENSE

#include <iostream>

#include "config/command_line.h"
#include "tools/MatchBin.h"
#include "config/ArgManager.h"

#include "../Config.h"
#include "../LowMutationalWalk.h"

// This is the main function for the NATIVE version of Tag Olympics.

int main(int argc, char* argv[])
{

  // Read configs.
  Config cfg;
  cfg.Read(cfg.FILENAME());

  // TODO command line args

  std::cout << "==============================" << std::endl;
  std::cout << "|    How am I configured?    |" << std::endl;
  std::cout << "==============================" << std::endl;
  cfg.Write(std::cout);
  std::cout << "==============================\n"
           << std::endl;

  LowMutationalWalk(cfg);

}
