#pragma once
#include <optional>
#include <string>

#include "shop/Order.h"

namespace shop {

class IStorage {
public:
    virtual ~IStorage() = default;
    virtual void save(const Order& order) = 0;
    virtual std::optional<Order> load(int id) = 0;
};

} // namespace shop
