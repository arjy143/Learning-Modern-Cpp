#pragma once
#include <string>
#include <vector>

namespace shop {

struct LineItem {
    std::string sku;
    int quantity = 0;
    double unitPrice = 0.0;
};

class Order {
public:
    explicit Order(int id) : id_(id) {}
    void add(LineItem item);
    double total() const;
    int id() const { return id_; }
    const std::vector<LineItem>& items() const { return items_; }

private:
    int id_;
    std::vector<LineItem> items_;
};

} // namespace shop
