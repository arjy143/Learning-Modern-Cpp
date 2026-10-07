#pragma once
#include <format>
#include <iostream>

// A minimal logger with the same shape as the real one, for the tests.
namespace logging {
template <class... Args>
void log(std::format_string<Args...> Format, Args &&...Values) {
  std::cout << std::format(Format, std::forward<Args>(Values)...) << '\n';
}
template <class... Args>
void error(std::format_string<Args...> Format, Args &&...Values) {
  std::cout << "error: " << std::format(Format, std::forward<Args>(Values)...)
            << '\n';
}
// A plain (non-template) helper that uses std::cout. The tool must leave it
// alone, or it would turn into a call to the logger from inside the logger.
inline void raw(const char *Text) { std::cout << Text << '\n'; }
} // namespace logging
