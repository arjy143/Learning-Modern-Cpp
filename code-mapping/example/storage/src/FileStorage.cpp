#include "shop/FileStorage.h"

#include <fstream>

namespace shop {

FileStorage::FileStorage(std::filesystem::path dir, std::shared_ptr<ILogger> logger)
    : dir_(std::move(dir)), logger_(std::move(logger)) {
    std::filesystem::create_directories(dir_);
}

std::filesystem::path FileStorage::pathFor(int id) const { return dir_ / (std::to_string(id) + ".order"); }

void FileStorage::save(const Order& order) {
    std::ofstream out(pathFor(order.id()));
    for (const LineItem& li : order.items())
        out << li.sku << ' ' << li.quantity << ' ' << li.unitPrice << '\n';
    logger_->info("saved order");
}

std::optional<Order> FileStorage::load(int id) {
    std::ifstream in(pathFor(id));
    if (!in) {
        logger_->error("order not found");
        return std::nullopt;
    }
    Order order(id);
    LineItem li;
    while (in >> li.sku >> li.quantity >> li.unitPrice)
        order.add(li);
    return order;
}

} // namespace shop
