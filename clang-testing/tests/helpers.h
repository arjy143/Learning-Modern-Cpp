#pragma once
#include "logging.h"
#include <iostream>

inline void fromHeader(int Value) {
  std::cout << "from a header " << Value << std::endl;
}
