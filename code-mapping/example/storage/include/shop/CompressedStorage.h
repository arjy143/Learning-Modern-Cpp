#pragma once
#include "shop/FileStorage.h"

namespace shop {

// Same as FileStorage but reports the compressed size of each order.
class CompressedStorage : public FileStorage {
public:
    using FileStorage::FileStorage;
    void save(const Order& order) override;
};

} // namespace shop
