// Copyright 2025 The Dawn & Tint Authors
//
// Redistribution and use in source and binary forms, with or without
// modification, are permitted provided that the following conditions are met:
//
// 1. Redistributions of source code must retain the above copyright notice, this
//    list of conditions and the following disclaimer.
//
// 2. Redistributions in binary form must reproduce the above copyright notice,
//    this list of conditions and the following disclaimer in the documentation
//    and/or other materials provided with the distribution.
//
// 3. Neither the name of the copyright holder nor the names of its
//    contributors may be used to endorse or promote products derived from
//    this software without specific prior written permission.
//
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
// AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
// IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
// DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
// FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
// DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
// SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
// CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
// OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
// OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

#ifndef SRC_DAWN_COMMON_LRUCACHE_H_
#define SRC_DAWN_COMMON_LRUCACHE_H_

#include <array>
#include <list>
#include <utility>

#include "absl/container/flat_hash_map.h"
#include "dawn/common/MutexProtected.h"
#include "dawn/common/Result.h"

namespace dawn {

// LRUCache is a Least-Recently-Used cache that retains the most recently queried values (up to
// mCapacity), and evicts older items from the cache as new values are created.
// Example usage:
//  class CachedValue {
//    public:
//      CachedValue(uint32_t a, bool b);
//  };
//  struct CachedValueKey {
//    uint32_t a;
//    bool b;
//  };
//
//  struct CachedValueCacheFuncs {
//    size_t operator()(const CachedValueKey& key) const {
//      return absl::HashOf(key.a, key.b);
//    }
//    bool operator()(const CachedValueKey& a, const CachedValueKey& b) const {
//      return std::tie(a.a, a.b) == std::tie(b.a, b.b);
//    }
//  };
//
//  class CacheUser {
//    public:
//      static const kCacheCapacity = 32;
//      CacheUser() : mCache(kCacheCapacity) {}
//
//      Ref<CachedValue> GetValue(const CachedValueKey& valueKey) {
//        return mCache.GetOrCreate(valueKey, [](const CachedValueKey& key) -> {
//          return Ref<CachedValue>(new CachedValue(key.a, key.b));
//        });
//      }
//
//    private:
//      using ValueCache = LRUCache<CachedValueKey, CachedValue, CachedValueCacheFuncs>;
//      // Or: `LRUCache<CachedValueKey, CachedValue>` if CachedValueKey already has an operator==
//      // and an absl hash specialization defined.
//      ValueCache mCache;
//  };

template <typename Key>
struct DefaultCacheFuncs {
    size_t operator()(const Key& key) const { return absl::DefaultHashContainerHash<Key>()(key); }
    bool operator()(const Key& lhs, const Key& rhs) const {
        return absl::DefaultHashContainerEq<Key>()(lhs, rhs);
    }
};

template <typename Key, typename Value, typename CacheFuncs = DefaultCacheFuncs<Key>>
class LRUCache {
  public:
    explicit LRUCache(size_t capacity) : mCapacity(capacity) {}
    virtual ~LRUCache() = default;

    template <typename CreateFn, typename ReturnType = std::invoke_result_t<CreateFn, const Key&>>
    ReturnType GetOrCreate(const Key& key, CreateFn createFn) {
        // A capacity of 0 means caching is disabled, so just return the result of the createFn.
        if (mCapacity == 0) {
            auto result = createFn(key);
            if (result.IsError()) [[unlikely]] {
                return result;
            }
            auto value = result.AcquireSuccess();
            // If caching is disabled treat every value as if it is immediately evicted from the
            // cache on creation.
            EvictedFromCache(value);
            return value;
        }

        return mCache.Use([&](auto cache) -> ReturnType {
            // Try to lookup |Key| to see if we have a cache hit.
            auto it = cache->map.find(key);
            if (it != cache->map.end()) {
                // Using iterators as a stable reference like this works because "Adding, removing,
                // and moving the elements within the list or across several lists does not
                // invalidate the iterators or references. An iterator is invalidated only when the
                // corresponding element is deleted."
                // (From https://en.cppreference.com/w/cpp/container/list)
                cache->list.splice(cache->list.begin(), cache->list, it->second);
                return it->second->second;
            }

            // Otherwise, we need to try to create the entry.
            auto result = createFn(key);
            if (result.IsError()) [[unlikely]] {
                return result;
            }
            auto value = result.AcquireSuccess();
            cache->list.emplace_front(key, value);
            cache->map.emplace(key, cache->list.begin());

            // If the LRU has exceeded it's capacity, remove the oldest entry.
            if (cache->list.size() > mCapacity) {
                auto back = cache->list.back();
                EvictedFromCache(back.second);
                cache->map.erase(back.first);
                cache->list.pop_back();
            }
            return value;
        });
    }

    void Clear() {
        mCache.Use([&](auto cache) {
            for (auto [_, value] : cache->list) {
                EvictedFromCache(value);
            }
            cache->map.clear();
            cache->list.clear();
        });
    }

    // Override if values need to be deleted or otherwise handled as they are evicted from the
    // cache. Useful for values that are not Ref<> values.
    virtual void EvictedFromCache(const Value& value) {}

  protected:
    const size_t mCapacity;

    using RecentList = std::list<std::pair<Key, Value>>;
    using Map = absl::flat_hash_map<Key, typename RecentList::iterator, CacheFuncs, CacheFuncs>;
    struct Cache {
        RecentList list;
        Map map;
    };
    MutexProtected<Cache> mCache;
};

}  // namespace dawn

#endif  // SRC_DAWN_COMMON_LRUCACHE_H_
