#pragma once
#include "shop/ILogger.h"

namespace shop {

class ConsoleLogger final : public ILogger {
public:
    void info(std::string_view msg) override;
    void error(std::string_view msg) override;
};

} // namespace shop
