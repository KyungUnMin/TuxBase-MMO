#include "EngineInfo.hpp"
#include <iostream>

int main() {
  std::cout << "TuxBase-MMO GameServer v" << TuxBase::EngineInfo::VERSION_MAJOR
            << "." << TuxBase::EngineInfo::VERSION_MINOR << "."
            << TuxBase::EngineInfo::VERSION_PATCH << std::endl;

  return 0;
}