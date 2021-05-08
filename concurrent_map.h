#pragma once

#include <future>
#include <map>
#include <numeric>
#include <string>
#include <vector>
#include <mutex>
#include <algorithm>

using namespace std::string_literals;

template <typename Key, typename Value>
class ConcurrentMap
{
public:
    static_assert(std::is_integral_v<Key>, "ConcurrentMap supports only integer keys"s);

    struct Bucket
    {
        std::mutex m;
        std::map<Key, Value> submap;
    };

    struct Access
    {
        Access(Bucket& bucket, const Key& key)
            : lg(bucket.m), ref_to_value(bucket.submap[key])
        {
        }

        ~Access()
        {
        }

        std::lock_guard<std::mutex> lg;
        Value& ref_to_value;
    };

    explicit ConcurrentMap(size_t bucket_count)
        : bucket_count_(bucket_count), buckets(bucket_count_)
    {
    }

    Access operator[](const Key& key)
    {
        size_t index = static_cast<size_t>(key) % bucket_count_;
        return Access(buckets[index], key);
    }

    std::map<Key, Value> BuildOrdinaryMap()
    {
        std::map<Key, Value> result;
        for (auto& bucket : buckets)
        {
            std::lock_guard<std::mutex> lg(bucket.m);
            for (const auto& item : bucket.submap)
            {
                result.insert(item);
            }
        }
        return result;
    }

private:
    size_t bucket_count_;
    std::vector<Bucket> buckets;
};