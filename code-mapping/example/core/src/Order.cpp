#include "shop/Order.h"

#include <numeric>

namespace shop {

void Order::add(LineItem item) { items_.push_back(std::move(item)); }

double Order::total() const {
    return std::accumulate(items_.begin(), items_.end(), 0.0,
                           [](double acc, const LineItem& li) { return acc + li.quantity * li.unitPrice; });
}

} // namespace shop
