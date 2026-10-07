#pragma once
#include <memory>
#include <mutex>
#include <vector>

#include "shop/ILogger.h"
#include "shop/IStorage.h"
#include "shop/Order.h"

namespace shop {

template <typename T>
class Cache {
public:
    void put(T value) { items_.push_back(std::move(value)); }
    std::size_t size() const { return items_.size(); }

private:
    std::vector<T> items_;
};

class OrderService {
public:
    OrderService(std::unique_ptr<IStorage> storage, ILogger& logger);
    void place(Order order);
    double revenue() const;

private:
    std::unique_ptr<IStorage> storage_;
    ILogger& logger_;
    Cache<Order> recent_;
    mutable std::mutex mutex_;
    double revenue_ = 0.0;
};

} // namespace shop
