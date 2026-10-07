#include "shop/ConsoleLogger.h"

#include <iostream>

namespace shop {

void ConsoleLogger::info(std::string_view msg) { std::cout << "[info] " << msg << '\n'; }
void ConsoleLogger::error(std::string_view msg) { std::cerr << "[error] " << msg << '\n'; }

} // namespace shop
