#pragma once
#include <filesystem>
#include <memory>

#include "shop/ILogger.h"
#include "shop/IStorage.h"

namespace shop {

class FileStorage : public IStorage {
public:
    FileStorage(std::filesystem::path dir, std::shared_ptr<ILogger> logger);
    void save(const Order& order) override;
    std::optional<Order> load(int id) override;

protected:
    std::filesystem::path pathFor(int id) const;

private:
    std::filesystem::path dir_;
    std::shared_ptr<ILogger> logger_;
};

} // namespace shop
