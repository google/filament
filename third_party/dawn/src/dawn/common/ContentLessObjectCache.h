// Copyright 2023 The Dawn & Tint Authors
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

#ifndef SRC_DAWN_COMMON_CONTENTLESSOBJECTCACHE_H_
#define SRC_DAWN_COMMON_CONTENTLESSOBJECTCACHE_H_

#include <mutex>
#include <type_traits>
#include <utility>

#include "absl/container/flat_hash_set.h"
#include "absl/container/inlined_vector.h"
#include "dawn/common/ContentLessObjectCacheable.h"
#include "dawn/common/Ref.h"
#include "dawn/common/RefCounted.h"
#include "dawn/common/WeakRef.h"
#include "partition_alloc/pointers/raw_ptr.h"

namespace dawn {

template <typename RefCountedT>
class ContentLessObjectCache;

namespace detail {

// Tagged-type to force special path for EqualityFunc when dealing with Erase. When erasing, we only
// care about pointer equality, not value equality. This is also particularly important because
// trying to promote on the Erase path can cause failures as the object's last ref could've been
// dropped already.
template <typename RefCountedT>
struct ForErase {
    explicit ForErase(RefCountedT* value) : value(value) {}
    raw_ptr<RefCountedT> value;
};

// All cached WeakRefs must have an immutable hash value determined at insertion. This ensures that
// even if the last ref of the cached value is dropped, we still get the same hash in the set for
// erasing.
template <typename RefCountedT>
struct WeakRefAndHash {
    WeakRef<RefCountedT> weakRef;
    size_t hash;

    explicit WeakRefAndHash(RefCountedT* obj)
        : weakRef(GetWeakRef(obj)), hash(typename RefCountedT::HashFunc()(obj)) {}
};

template <typename RefCountedT>
struct ContentLessObjectCacheKeyFuncs {
    using BaseHashFunc = typename RefCountedT::HashFunc;
    using BaseEqualityFunc = typename RefCountedT::EqualityFunc;

    struct HashFunc {
        using is_transparent = void;

        size_t operator()(const RefCountedT* ptr) const { return BaseHashFunc()(ptr); }
        size_t operator()(const WeakRefAndHash<RefCountedT>& obj) const { return obj.hash; }
        size_t operator()(const ForErase<RefCountedT>& obj) const {
            return BaseHashFunc()(obj.value);
        }
    };

    struct EqualityFunc {
        using is_transparent = void;

        explicit EqualityFunc(ContentLessObjectCache<RefCountedT>* cache) : mCache(cache) {}

        bool operator()(const WeakRefAndHash<RefCountedT>& a,
                        const WeakRefAndHash<RefCountedT>& b) const {
            Ref<RefCountedT> aRef = a.weakRef.Promote();
            Ref<RefCountedT> bRef = b.weakRef.Promote();

            bool equal = (aRef && bRef && BaseEqualityFunc()(aRef.Get(), bRef.Get()));
            if (aRef) {
                mCache->TrackTemporaryRef(std::move(aRef));
            }
            if (bRef) {
                mCache->TrackTemporaryRef(std::move(bRef));
            }
            return equal;
        }

        bool operator()(const WeakRefAndHash<RefCountedT>& a,
                        const ForErase<RefCountedT>& b) const {
            // An object is being erased. In this scenario, UnsafeGet is OK because either:
            //   (1) a == b, in which case that means we are destroying the last copy and must be
            //       valid because cached objects must uncache themselves before being completely
            //       destroyed.
            //   (2) a != b, in which case the lock on the cache guarantees that the element in the
            //       cache has not been erased yet and hence cannot have been destroyed.
            return a.weakRef.UnsafeGet() == b.value;
        }

        bool operator()(const WeakRefAndHash<RefCountedT>& a, const RefCountedT* b) const {
            Ref<RefCountedT> aRef = a.weakRef.Promote();
            bool equal = aRef && BaseEqualityFunc()(aRef.Get(), b);
            if (aRef) {
                mCache->TrackTemporaryRef(std::move(aRef));
            }
            return equal;
        }

        raw_ptr<ContentLessObjectCache<RefCountedT>> mCache = nullptr;
    };
};

}  // namespace detail

template <typename RefCountedT>
class ContentLessObjectCache {
    static_assert(std::is_base_of_v<detail::ContentLessObjectCacheableBase, RefCountedT>,
                  "Type must be cacheable to use with ContentLessObjectCache.");
    static_assert(std::is_base_of_v<RefCounted, RefCountedT>,
                  "Type must be refcounted to use with ContentLessObjectCache.");

    using CacheKeyFuncs = detail::ContentLessObjectCacheKeyFuncs<RefCountedT>;

  public:
    ContentLessObjectCache()
        : mCache(/*capacity=*/0,
                 typename CacheKeyFuncs::HashFunc(),
                 typename CacheKeyFuncs::EqualityFunc(this)) {}

    // The dtor asserts that the cache is empty to aid in finding pointer leaks that can be
    // possible if the RefCountedT doesn't correctly implement the DeleteThis function to Uncache.
    ~ContentLessObjectCache() { DAWN_ASSERT(Empty()); }

    // Inserts the object into the cache returning a pair where the first is a Ref to the
    // inserted or existing object, and the second is a bool that is true if we inserted
    // `object` and false otherwise.
    std::pair<Ref<RefCountedT>, bool> Insert(RefCountedT* obj) {
        return WithLockAndCleanup([&]() -> std::pair<Ref<RefCountedT>, bool> {
            auto [it, inserted] = mCache.emplace(obj);
            if (inserted) {
                obj->mCache = this;
                return {obj, inserted};
            } else {
                // Try to promote the found WeakRef to a Ref. If promotion fails, remove the old Key
                // and insert this one.
                Ref<RefCountedT> ref = it->weakRef.Promote();
                if (ref != nullptr) {
                    return {std::move(ref), false};
                } else {
                    mCache.erase(it);
                    auto result = mCache.emplace(obj);
                    DAWN_ASSERT(result.second);
                    obj->mCache = this;
                    return {obj, true};
                }
            }
        });
    }

    // Returns a valid Ref<T> if we can Promote the underlying WeakRef. Returns nullptr otherwise.
    Ref<RefCountedT> Find(RefCountedT* blueprint) {
        return WithLockAndCleanup([&]() -> Ref<RefCountedT> {
            auto it = mCache.find(blueprint);
            if (it != mCache.end()) {
                return it->weakRef.Promote();
            }
            return nullptr;
        });
    }

    // Erases the object from the cache if it exists and are pointer equal. Otherwise does not
    // modify the cache. Since Erase never Promotes any WeakRefs, it does not need to be wrapped by
    // a WithLockAndCleanup, and a simple lock is enough.
    void Erase(RefCountedT* obj) {
        size_t count;
        {
            std::lock_guard<std::mutex> lock(mMutex);
            count = mCache.erase(detail::ForErase<RefCountedT>(obj));
        }
        if (count == 0) {
            return;
        }
        obj->mCache = nullptr;
    }

    // Returns true iff the cache is empty.
    bool Empty() {
        std::lock_guard<std::mutex> lock(mMutex);
        return mCache.empty();
    }

  private:
    friend struct CacheKeyFuncs::EqualityFunc;

    void TrackTemporaryRef(Ref<RefCountedT> ref) { mTemporaryRefs->push_back(std::move(ref)); }
    template <typename F>
    auto WithLockAndCleanup(F func) {
        using RetType = decltype(func());
        RetType result;

        // Creates and owns a temporary InlinedVector that we point to internally to track Refs.
        absl::InlinedVector<Ref<RefCountedT>, 4> temps;
        {
            std::lock_guard<std::mutex> lock(mMutex);
            mTemporaryRefs = &temps;
            result = func();
            mTemporaryRefs = nullptr;
        }
        return result;
    }

    std::mutex mMutex;
    absl::flat_hash_set<detail::WeakRefAndHash<RefCountedT>,
                        typename CacheKeyFuncs::HashFunc,
                        typename CacheKeyFuncs::EqualityFunc>
        mCache;

    // The cache has a pointer to a InlinedVector of temporary Refs that are by-products of Promotes
    // inside the EqualityFunc. These Refs need to outlive the EqualityFunc calls because otherwise,
    // they could be the last living Ref of the object resulting in a re-entrant Erase call that
    // deadlocks on the mutex.
    // Absl should make fewer than 1 equality checks per set operation, so a InlinedVector of length
    // 4 should be sufficient for most cases. See dawn:1993 for more details.
    raw_ptr<absl::InlinedVector<Ref<RefCountedT>, 4>> mTemporaryRefs = nullptr;
};

}  // namespace dawn

#endif  // SRC_DAWN_COMMON_CONTENTLESSOBJECTCACHE_H_
