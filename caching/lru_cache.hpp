#pragma once
#include <unordered_map>
#include <list>
#include <optional>
#include <iterator>

//basic LRU cache using only STL, and very few performance improvments
template <typename K, typename V>
class LRUCache
{

using Node = std::pair<K, V>;
using ListIterator = typename std::list<Node>::iterator;

private:
    size_t capacity_;
    std::unordered_map<K, ListIterator> lookup_table_;
    std::list<Node> recency_list_;

public:
    LRUCache(size_t capacity) : capacity_(capacity) 
    {}

    //no copy constructor or assignment
    LRUCache(const LRUCache&)=delete;
    LRUCache& operator==(const LRUCache&)=delete;

    //allow move constructor and assignment
    LRUCache(LRUCache&&)=default;
    LRUCache&operator=(LRUCache&&)=default;

    std::optional<V> get(const K& key)
    {
        //if the key exists, then get the corresponding iterator, move it up the recency list, and return the associated value.
        auto it = lookup_table_.find(key);
        if (it != lookup_table_.end())
        {
            auto& iterator = it->second;

            recency_list_.splice(recency_list_.end(), recency_list_, iterator);

            return iterator->second;
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

            //get old iterator, dereference it, overwrite the data, push it to the front.
            auto& iterator = it->second;

            iterator->second = std::move(value);

            recency_list_.splice(recency_list_.end(), recency_list_, iterator);
        }
        else
        {
            //if key does not exist, then add it, and evict old data if needed.
            if (recency_list_.size() >= capacity_)
            {
                auto iterator = recency_list_.begin();
                
                lookup_table_.erase(iterator->first);

                recency_list_.pop_front();
            }

            auto new_iterator = recency_list_.emplace(recency_list_.end(), std::move(key), std::move(value));

            lookup_table_.emplace(new_iterator->first, new_iterator); 
        }
    }

    //check existence without causing cache update
    bool contains(const K& key)
    {
        return lookup_table_.count(key) > 0;
    }



};