#pragma once
#include <optional>
#include <vector>
#include <cstdint>
#include <bit>
#include <cmath>
#include <functional>

//LRU cache using custom node pool and custom flat map

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
    size_t mask_;

    int head_;
    int tail_;
    int free_head_;

    static constexpr int NIL = -1;
    static constexpr int EMPTY_BUCKET = -1;
    static constexpr float MAX_LOAD_FACTOR = 0.7;

    //new more performant lookup table - each entry is a slot index into the node pool.
    std::vector<int> table_;

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

    //mixing algorithm, taking constants from splitmix64
    //the reason for this is when masking using std::hash<uint64_t>, the values cluster badly, leading to a lot of collisions.
    //this algorithm works by diffusing each input bit across many output bits per multiplication, leading to less collisions.
    static uint64_t mix(uint64_t h) 
    {
        h ^= h >> 30;
        h *= 0xBF58476D1CE4E5B9ULL;
        h ^= h >> 27;
        h *= 0x94D049BB133111EBULL;
        h ^= h >> 31;
        return h;
    }

    //this just tells you which array index to start probing at for a given key.
    size_t bucket(const K& key) const
    {
        uint64_t h = std::hash<K>{}(key);
        
        h = mix(h);

        return h & mask_;
    }
    
    //find the table index of the key
    int find(const K& key)
    {
        size_t i = bucket(key);

        while (table_[i] != EMPTY_BUCKET)
        {
            if (node_pool_[table_[i]].key == key)
            {
                return (int)i;
            }
            i = (i + 1) & mask_; 
        }
        return EMPTY_BUCKET;
    }
   
    void erase(const K& key)
    {
        //use the same logic as the find function to find the key
        size_t i = bucket(key);

        while (table_[i] != EMPTY_BUCKET && node_pool_[table_[i]].key != key)
        {
            i = (i + 1) & mask_; 
        }

        //key not present
        if (table_[i] == EMPTY_BUCKET)
        {
            return;
        }

        //key present. now we need to remove the item but also backwards shift every item in the chain afterwards, so that they are not lost
        size_t current = (i + 1) & mask_;

        while (table_[current] != EMPTY_BUCKET)
        {
            size_t home = bucket(node_pool_[table_[current]].key);

            if (((current - home) & mask_) >= ((current - i) & mask_))
            {
                table_[i] = table_[current];    
                i = current;
            }

            current = (current + 1) & mask_;
        }

        table_[i] = EMPTY_BUCKET;
    }

    void insert(const K& key, int slot)
    {
        size_t i = bucket(key);

        while (table_[i] != EMPTY_BUCKET)
        {
            i = (i + 1) & mask_;
        }

        table_[i] = slot;
    }

public:
    LRUCache2(size_t capacity) : capacity_(capacity) 
    {
        //set up node pool and link the nodes together
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

        //set up the table, calculate a size, and fill with -1
        auto needed = static_cast<size_t>(std::ceil(capacity_ / MAX_LOAD_FACTOR));
        auto table_size = std::bit_ceil(needed);

        mask_ = table_size - 1;

        table_.assign(table_size, EMPTY_BUCKET);
    }

    //no copy constructor or assignment
    LRUCache2(const LRUCache2&)=delete;
    LRUCache2& operator=(const LRUCache2&)=delete;

    //allow move constructor and assignment
    LRUCache2(LRUCache2&&)=default;
    LRUCache2&operator=(LRUCache2&&)=default;

    std::optional<V> get(const K& key)
    {
        //check if the key exists by hashing it and seeing if a value exists for it in the table
        

        auto i = find(key);
        if (i != EMPTY_BUCKET)
        {

            auto& node_slot = node_pool_[table_[i]];

            move_to_mru(table_[i]);

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
        auto i = find(key);
        if (i != EMPTY_BUCKET)
        {
            //get index of existing data and overwrite with new data.
            node_pool_[table_[i]].value = std::move(value);

            move_to_mru(table_[i]);
        }
        else
        {
            //if key does not exist, then add it, and evict old data if needed.
            if (free_head_ == NIL)
            {
                int victim = head_;

                erase(node_pool_[victim].key);

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

            insert(node_pool_[slot].key, slot);
        }
    }

    //check existence without causing cache update
    bool contains(const K& key)
    {
        return find(key) != EMPTY_BUCKET;
    }



};