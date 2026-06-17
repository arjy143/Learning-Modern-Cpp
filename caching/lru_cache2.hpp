#pragma once
#include <unordered_map>
#include <list>
#include <optional>
#include <iterator>
#include <vector>

//LRU cache using custom node pool

//manual linked list, using int instead of pointers to refer to the previous and next nodes. This is because the ints are indices into the node pool.
template <typename K, typename V>
struct NodeSlot
{
    K key;
    V value;
    int prev;
    int next;
};

template <typename K, typename V>
class LRUCache2
{

using Node = std::pair<K, V>;

private:
    size_t capacity_;
    int head_;
    int tail_;
    int free_head_;

    static constexpr int NIL = -1;

    //lookup table maps key to index in node pool
    std::unordered_map<K, int> lookup_table_;

    std::vector<NodeSlot<K, V>> node_pool_;


    void move_to_mru(int index)
    {
        if (index == tail_)
        {
            return;
        }

        int prev = node_pool_[index].prev;
        int next = node_pool_[index].next;

        if (prev == NIL)
        {
            head_ = next;
        }
        else
        {
            node_pool_[prev].next = next;
        }

        node_pool_[next].prev = prev;

        link_at_mru(index);
    }

    void link_at_mru(int slot)
    {
        node_pool_[slot].prev = tail_;
        node_pool_[slot].next = NIL;

        if (tail_ == NIL)
        {
            head_ = slot;
        }
        else
        {
            node_pool_[tail_].next = slot;
        }

        tail_ = slot;
    }

public:
    LRUCache2(size_t capacity) : capacity_(capacity) 
    {
        if (capacity_ == 0)
        {
            free_head_ = NIL;
            return;
        }

        node_pool_.resize(capacity_);

        head_ = tail_ = NIL;

        for (int i = 0; i < (int)capacity_; ++i)
        {
            node_pool_[i].next = i + 1;
        }

        node_pool_[capacity_ - 1].next = NIL;
        
        free_head_ = 0;
    }

    //no copy constructor or assignment
    LRUCache2(const LRUCache2&)=delete;
    LRUCache2& operator=(const LRUCache2&)=delete;

    //allow move constructor and assignment
    LRUCache2(LRUCache2&&)=default;
    LRUCache2&operator=(LRUCache2&&)=default;

    std::optional<V> get(const K& key)
    {
        //if the key exists, then get the corresponding iterator, move it up the recency list, and return the associated value.
        auto it = lookup_table_.find(key);
        if (it != lookup_table_.end())
        {
            auto& index = it->second;

            auto& node_slot = node_pool_[index];

            move_to_mru(index);

            return node_slot.value;
        }

        return std::nullopt;
    }

    void put(K key, V value)
    {
        if (capacity_ == 0)
        {
            return;
        }

        //if key exists already, then overwrite it
        auto it = lookup_table_.find(key);
        if (it != lookup_table_.end())
        {
            //get index of existing data and overwrite with new data.
            auto& index = it->second;

            node_pool_[index].value = std::move(value);

            move_to_mru(index);
        }
        else
        {
            //if key does not exist, then add it, and evict old data if needed.
            if (free_head_ == NIL)
            {
                int victim = head_;

                lookup_table_.erase(node_pool_[victim].key);

                head_ = node_pool_[victim].next;
                
                if (head_ != NIL)
                {
                    node_pool_[head_].prev = NIL;
                }
                else
                {
                    tail_ = NIL;
                }

                node_pool_[victim].next = free_head_;

                free_head_ = victim;
            }

            int slot = free_head_;

            free_head_ = node_pool_[slot].next;

            node_pool_[slot].key = std::move(key);
            node_pool_[slot].value = std::move(value);

            link_at_mru(slot);

            lookup_table_.emplace(node_pool_[slot].key, slot);
        }
    }

    //check existence without causing cache update
    bool contains(const K& key)
    {
        return lookup_table_.count(key) > 0;
    }



};