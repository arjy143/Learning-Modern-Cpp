#include "shop/CompressedStorage.h"

#include <zlib.h>

#include <string>

namespace shop {

void CompressedStorage::save(const Order& order) {
    std::string raw = std::to_string(order.id()) + ":" + std::to_string(order.total());
    uLongf size = compressBound(static_cast<uLong>(raw.size()));
    std::string buf(size, '\0');
    compress(reinterpret_cast<Bytef*>(buf.data()), &size, reinterpret_cast<const Bytef*>(raw.data()),
             static_cast<uLong>(raw.size()));
    FileStorage::save(order);
}

} // namespace shop
