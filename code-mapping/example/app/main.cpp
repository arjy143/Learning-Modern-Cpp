#include <memory>
#include <thread>

#include "shop/CompressedStorage.h"
#include "shop/ConsoleLogger.h"
#include "shop/OrderService.h"

int main() {
    auto logger = std::make_shared<shop::ConsoleLogger>();
    auto storage = std::make_unique<shop::CompressedStorage>("orders", logger);
    shop::OrderService service(std::move(storage), *logger);

    std::thread worker([&] {
        shop::Order order(1);
        order.add({"apple", 3, 0.5});
        service.place(order);
    });
    worker.join();
    logger->info("done");
}
