#pragma once
#include <string_view>

namespace shop {

class ILogger {
public:
    virtual ~ILogger() = default;
    virtual void info(std::string_view msg) = 0;
    virtual void error(std::string_view msg) = 0;
};

} // namespace shop
