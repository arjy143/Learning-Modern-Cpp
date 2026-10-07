#include "shop/OrderService.h"

namespace shop {

OrderService::OrderService(std::unique_ptr<IStorage> storage, ILogger& logger)
    : storage_(std::move(storage)), logger_(logger) {}

void OrderService::place(Order order) {
    std::lock_guard lock(mutex_);
    revenue_ += order.total();
    storage_->save(order);
    recent_.put(order);
    logger_.info("order placed");
}

double OrderService::revenue() const {
    std::lock_guard lock(mutex_);
    return revenue_;
}

} // namespace shop
