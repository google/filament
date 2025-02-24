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

#include <condition_variable>
#include <functional>
#include <mutex>
#include <string>
#include <thread>
#include <vector>

#include "dawn/common/ContentLessObjectCache.h"
#include "dawn/utils/BinarySemaphore.h"
#include "gtest/gtest.h"

namespace dawn {
namespace {

using utils::BinarySemaphore;

class CacheableT : public RefCounted, public ContentLessObjectCacheable<CacheableT> {
  public:
    explicit CacheableT(size_t value) : mHash(value), mValue(value) {}
    CacheableT(size_t hash, size_t value) : mHash(hash), mValue(value) {}

    ~CacheableT() override { mDeleteFn(this); }

    struct HashFunc {
        size_t operator()(const CacheableT* x) const {
            x->mHashFn(x);
            return x->mHash;
        }
    };

    struct EqualityFunc {
        bool operator()(const CacheableT* l, const CacheableT* r) const {
            l->mEqualFn(l);
            r->mEqualFn(r);
            return l->mValue == r->mValue;
        }
    };

    void SetHashFn(std::function<void(const CacheableT*)> fn) { mHashFn = fn; }
    void SetEqualFn(std::function<void(const CacheableT*)> fn) { mEqualFn = fn; }
    void SetDeleteFn(std::function<void(CacheableT*)> fn) { mDeleteFn = fn; }

  private:
    size_t mHash;
    size_t mValue;

    // Injectable functions to allow for isolated thread testing.
    std::function<void(const CacheableT*)> mHashFn = [](const CacheableT*) -> void {};
    std::function<void(const CacheableT*)> mEqualFn = [](const CacheableT*) -> void {};
    std::function<void(CacheableT*)> mDeleteFn = [](CacheableT*) -> void {};
};

// Empty cache returns true on Empty().
TEST(ContentLessObjectCacheTest, Empty) {
    ContentLessObjectCache<CacheableT> cache;
    EXPECT_TRUE(cache.Empty());
}

// Non-empty cache returns false on Empty().
TEST(ContentLessObjectCacheTest, NonEmpty) {
    ContentLessObjectCache<CacheableT> cache;
    Ref<CacheableT> object = AcquireRef(new CacheableT(1));
    object->SetDeleteFn([&](CacheableT* x) { cache.Erase(x); });
    EXPECT_TRUE(cache.Insert(object.Get()).second);
    EXPECT_FALSE(cache.Empty());
}

// Object inserted into the cache are findable.
TEST(ContentLessObjectCacheTest, Insert) {
    ContentLessObjectCache<CacheableT> cache;
    Ref<CacheableT> object = AcquireRef(new CacheableT(1));
    object->SetDeleteFn([&](CacheableT* x) { cache.Erase(x); });
    EXPECT_TRUE(cache.Insert(object.Get()).second);

    CacheableT blueprint(1);
    Ref<CacheableT> cached = cache.Find(&blueprint);
    EXPECT_TRUE(object.Get() == cached.Get());
}

// Duplicate insert calls on different equivalent objects only inserts the first.
TEST(ContentLessObjectCacheTest, InsertDuplicate) {
    ContentLessObjectCache<CacheableT> cache;
    Ref<CacheableT> object1 = AcquireRef(new CacheableT(1));
    object1->SetDeleteFn([&](CacheableT* x) { cache.Erase(x); });
    EXPECT_TRUE(cache.Insert(object1.Get()).second);

    Ref<CacheableT> object2 = AcquireRef(new CacheableT(1));
    EXPECT_FALSE(cache.Insert(object2.Get()).second);

    CacheableT blueprint(1);
    Ref<CacheableT> cached = cache.Find(&blueprint);
    EXPECT_TRUE(object1.Get() == cached.Get());
}

// Duplicate insert calls on different objects with the same hash inserts both objects.
TEST(ContentLessObjectCacheTest, InsertHashDuplicate) {
    ContentLessObjectCache<CacheableT> cache;
    Ref<CacheableT> object1 = AcquireRef(new CacheableT(1, 1));
    object1->SetDeleteFn([&](CacheableT* x) { cache.Erase(x); });
    EXPECT_TRUE(cache.Insert(object1.Get()).second);

    Ref<CacheableT> object2 = AcquireRef(new CacheableT(1, 2));
    object2->SetDeleteFn([&](CacheableT* x) { cache.Erase(x); });
    EXPECT_TRUE(cache.Insert(object2.Get()).second);

    CacheableT blueprint1(1, 1);
    Ref<CacheableT> cached1 = cache.Find(&blueprint1);
    EXPECT_TRUE(object1.Get() == cached1.Get());
    CacheableT blueprint2(1, 2);
    Ref<CacheableT> cached2 = cache.Find(&blueprint2);
    EXPECT_TRUE(object2.Get() == cached2.Get());
}

// Erasing the only entry leaves the cache empty.
TEST(ContentLessObjectCacheTest, Erase) {
    ContentLessObjectCache<CacheableT> cache;
    Ref<CacheableT> object = AcquireRef(new CacheableT(1));
    EXPECT_TRUE(cache.Insert(object.Get()).second);
    EXPECT_FALSE(cache.Empty());

    cache.Erase(object.Get());
    EXPECT_TRUE(cache.Empty());
}

// Erasing an equivalent but not pointer equivalent entry is a no-op.
TEST(ContentLessObjectCacheTest, EraseDuplicate) {
    ContentLessObjectCache<CacheableT> cache;
    Ref<CacheableT> object1 = AcquireRef(new CacheableT(1));
    object1->SetDeleteFn([&](CacheableT* x) { cache.Erase(x); });
    EXPECT_TRUE(cache.Insert(object1.Get()).second);
    EXPECT_FALSE(cache.Empty());

    Ref<CacheableT> object2 = AcquireRef(new CacheableT(1));
    cache.Erase(object2.Get());
    EXPECT_FALSE(cache.Empty());
}

// Inserting and finding elements should respect the results from the insert call.
TEST(ContentLessObjectCacheTest, InsertingAndFinding) {
    constexpr size_t kNumObjects = 100;
    constexpr size_t kNumThreads = 8;
    ContentLessObjectCache<CacheableT> cache;
    std::vector<Ref<CacheableT>> objects(kNumObjects);

    auto f = [&] {
        for (size_t i = 0; i < kNumObjects; i++) {
            Ref<CacheableT> object = AcquireRef(new CacheableT(i));
            object->SetDeleteFn([&](CacheableT* x) { cache.Erase(x); });
            if (cache.Insert(object.Get()).second) {
                // This shouldn't race because exactly 1 thread should successfully insert.
                objects[i] = object;
            }
        }
        for (size_t i = 0; i < kNumObjects; i++) {
            CacheableT blueprint(i);
            Ref<CacheableT> cached = cache.Find(&blueprint);
            EXPECT_NE(cached.Get(), nullptr);
            EXPECT_EQ(cached.Get(), objects[i].Get());
        }
    };

    std::vector<std::thread> threads;
    for (size_t t = 0; t < kNumThreads; t++) {
        threads.emplace_back(f);
    }
    for (size_t t = 0; t < kNumThreads; t++) {
        threads[t].join();
    }
}

// Finding an element that is in the process of deletion should return nullptr.
TEST(ContentLessObjectCacheTest, FindDeleting) {
    BinarySemaphore semA, semB;

    ContentLessObjectCache<CacheableT> cache;
    Ref<CacheableT> object = AcquireRef(new CacheableT(1));
    object->SetDeleteFn([&](CacheableT* x) {
        semA.Release();
        semB.Acquire();
        cache.Erase(x);
    });
    EXPECT_TRUE(cache.Insert(object.Get()).second);

    // Thread A will release the last reference of the original object.
    auto threadA = [&] { object = nullptr; };
    // Thread B will try to Find the entry before it is completely destroyed.
    auto threadB = [&] {
        semA.Acquire();
        CacheableT blueprint(1);
        EXPECT_TRUE(cache.Find(&blueprint) == nullptr);
        semB.Release();
    };

    std::thread tA(threadA);
    std::thread tB(threadB);
    tA.join();
    tB.join();
}

// Finding an equivalent element when the cached version is in the process of deletion and the
// last ref of the object is actually the one acquired inside the find operation does not deadlock
// and returns the cached value. This is a regression test for dawn:1993.
TEST(ContentLessObjectCacheTest, FindDeletingLastRef) {
    BinarySemaphore semA, semB;

    ContentLessObjectCache<CacheableT> cache;
    Ref<CacheableT> object = AcquireRef(new CacheableT(1));
    object->SetDeleteFn([&](CacheableT* x) { cache.Erase(x); });
    EXPECT_TRUE(cache.Insert(object.Get()).second);
    CacheableT* objectPtr = object.Get();

    std::atomic_flag eqOnceFlag;
    object->SetEqualFn([&](const CacheableT* x) {
        if (!eqOnceFlag.test_and_set()) {
            semA.Release();
            semB.Acquire();
        }
    });

    // Thread A will release the last reference of the original object after the object has been
    // promoted internally for equality check.
    auto threadA = [&] {
        semA.Acquire();
        object = nullptr;
        semB.Release();
    };
    // Thread B will try to Find the entry before the original object is destroyed.
    auto threadB = [&] {
        CacheableT blueprint(1);
        EXPECT_TRUE(cache.Find(&blueprint) == objectPtr);
    };

    std::thread tA(threadA);
    std::thread tB(threadB);
    tA.join();
    tB.join();
}

// Finding a non-equivalent but hash equivalent element when the cached version is in the process
// of deletion and the last ref of the object is actually the one acquired inside the find operation
// does not deadlock and returns nullptr. This is a regression test for dawn:1993.
TEST(ContentLessObjectCacheTest, FindDeletingLastRefHashEquivalent) {
    BinarySemaphore semA, semB;

    ContentLessObjectCache<CacheableT> cache;
    Ref<CacheableT> object = AcquireRef(new CacheableT(1, 1));
    object->SetDeleteFn([&](CacheableT* x) { cache.Erase(x); });
    EXPECT_TRUE(cache.Insert(object.Get()).second);

    std::atomic_flag eqOnceFlag;
    object->SetEqualFn([&](const CacheableT* x) {
        if (!eqOnceFlag.test_and_set()) {
            semA.Release();
            semB.Acquire();
        }
    });

    // Thread A will release the last reference of the original object after the object has been
    // promoted internally for equality check.
    auto threadA = [&] {
        semA.Acquire();
        object = nullptr;
        semB.Release();
    };
    // Thread B will try to Find a hash-equivalent entry before the original object is destroyed.
    auto threadB = [&] {
        CacheableT blueprint(1, 2);
        EXPECT_TRUE(cache.Find(&blueprint) == nullptr);
    };

    std::thread tA(threadA);
    std::thread tB(threadB);
    tA.join();
    tB.join();
}

// Inserting an element that has an entry which is in process of deletion should insert the new
// object.
TEST(ContentLessObjectCacheTest, InsertDeleting) {
    BinarySemaphore semA, semB;

    ContentLessObjectCache<CacheableT> cache;
    Ref<CacheableT> object1 = AcquireRef(new CacheableT(1));
    object1->SetDeleteFn([&](CacheableT* x) {
        semA.Release();
        semB.Acquire();
        cache.Erase(x);
    });
    EXPECT_TRUE(cache.Insert(object1.Get()).second);

    Ref<CacheableT> object2 = AcquireRef(new CacheableT(1));
    object2->SetDeleteFn([&](CacheableT* x) { cache.Erase(x); });

    // Thread A will release the last reference of the original object.
    auto threadA = [&] { object1 = nullptr; };
    // Thread B will try to Insert an equivalent entry before the original is completely
    // destroyed.
    auto threadB = [&] {
        semA.Acquire();
        EXPECT_TRUE(cache.Insert(object2.Get()).second);
        semB.Release();
    };

    std::thread tA(threadA);
    std::thread tB(threadB);
    tA.join();
    tB.join();

    CacheableT blueprint(1);
    Ref<CacheableT> cached = cache.Find(&blueprint);
    EXPECT_TRUE(object2.Get() == cached.Get());
}

// Inserting an equivalent element when the cached version is in the process of deletion and the
// last ref of the object is actually the one acquired inside the find operation does not deadlock
// and no insert happens. This is a regression test for dawn:1993.
TEST(ContentLessObjectCacheTest, InsertDeletingLastRef) {
    BinarySemaphore semA, semB;

    ContentLessObjectCache<CacheableT> cache;
    Ref<CacheableT> object1 = AcquireRef(new CacheableT(1));
    object1->SetDeleteFn([&](CacheableT* x) { cache.Erase(x); });
    EXPECT_TRUE(cache.Insert(object1.Get()).second);
    CacheableT* object1Ptr = object1.Get();

    std::atomic_flag eqOnceFlag;
    object1->SetEqualFn([&](const CacheableT* x) {
        if (!eqOnceFlag.test_and_set()) {
            semA.Release();
            semB.Acquire();
        }
    });

    // Thread A will release the last reference of the original object after the object has been
    // promoted internally for equality check.
    auto threadA = [&] {
        semA.Acquire();
        object1 = nullptr;
        semB.Release();
    };
    // Thread B will try to Insert an equivalent entry before the original object is destroyed.
    auto threadB = [&] {
        Ref<CacheableT> object2 = AcquireRef(new CacheableT(1));
        auto result = cache.Insert(object2.Get());
        EXPECT_FALSE(result.second);
        EXPECT_TRUE(result.first == object1Ptr);
    };

    std::thread tA(threadA);
    std::thread tB(threadB);
    tA.join();
    tB.join();
}

// Inserting a non-equivalent but hash equivalent element when the cached version is in the process
// of deletion and the last ref of the object is actually the one acquired inside the find operation
// does not deadlock and the insert occurs successfully. This is a regression test for dawn:1993.
TEST(ContentLessObjectCacheTest, InsertDeletingLastRefHashEquivalent) {
    BinarySemaphore semA, semB;

    ContentLessObjectCache<CacheableT> cache;
    Ref<CacheableT> object1 = AcquireRef(new CacheableT(1, 1));
    object1->SetDeleteFn([&](CacheableT* x) { cache.Erase(x); });
    EXPECT_TRUE(cache.Insert(object1.Get()).second);

    std::atomic_flag eqOnceFlag;
    object1->SetEqualFn([&](const CacheableT* x) {
        if (!eqOnceFlag.test_and_set()) {
            semA.Release();
            semB.Acquire();
        }
    });

    // Thread A will release the last reference of the original object after the object has been
    // promoted internally for equality check.
    auto threadA = [&] {
        semA.Acquire();
        object1 = nullptr;
        semB.Release();
    };
    // Thread B will try to Insert a hash equivalent entry before the original object is destroyed.
    auto threadB = [&] {
        Ref<CacheableT> object2 = AcquireRef(new CacheableT(1, 2));
        object2->SetDeleteFn([&](CacheableT* x) { cache.Erase(x); });
        auto result = cache.Insert(object2.Get());
        EXPECT_TRUE(result.second);
        EXPECT_TRUE(result.first == object2.Get());
    };

    std::thread tA(threadA);
    std::thread tB(threadB);
    tA.join();
    tB.join();
}

}  // anonymous namespace
}  // namespace dawn
