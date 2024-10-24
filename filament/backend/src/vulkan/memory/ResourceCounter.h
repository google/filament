/*
 * Copyright (C) 2024 The Android Open Source Project
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#ifndef TNT_FILAMENT_BACKEND_VULKAN_MEMORY_RESOURCECOUNTER_H
#define TNT_FILAMENT_BACKEND_VULKAN_MEMORY_RESOURCECOUNTER_H

#include "vulkan/memory/Resource.h"

#include <utils/Mutex.h>
#include <utils/Panic.h>

#include <functional>

namespace filament::backend::fvkmemory {

namespace {
constexpr bool IS_THREAD_SAFE = true;
constexpr bool IS_NOT_THREAD_SAFE = false;
} // anonymous namespace

using DestructionFunc = std::function<void(ResourceType, HandleId)>;

template<bool THREAD_SAFE>
struct alignas(8) CounterImpl {
public:
    inline void inc() noexcept {
        if constexpr (THREAD_SAFE) {
            std::unique_lock<utils::Mutex> lock(*mMutex);
            if UTILS_LIKELY (!mDestroyed) {
                mCount++;
            }
        } else {
            mCount++;
        }
    }

    inline void dec() noexcept {
        if constexpr (THREAD_SAFE) {
            if (mDestroyed) {
                return;
            }
            bool shouldDestroy = false;
            {
                std::unique_lock<utils::Mutex> lock(*mMutex);
                shouldDestroy = --mCount == 0;
            }
            if (shouldDestroy) {
                destroyLater(type, id);
                mDestroyed = true;
            }
        } else {
            assert_invariant(mCount > 0);
            if (--mCount == 0) {
                destroyNow(type, id);
                mDestroyed = true;
            }
        }
    }

    CounterImpl(ResourceType type, HandleId id)
        : id(id),
          type(type),
          mDestroyed(false),
          mCount(0) {
        if constexpr (THREAD_SAFE) {
            mMutex = std::make_unique<utils::Mutex>();
        }
    }

    CounterImpl()
        : id(HandleBase::nullid),
          type(ResourceType::UNDEFINED_TYPE),
          mDestroyed(false),
          mCount(0) {}

    CounterImpl(CounterImpl const& rhs)
        : id(rhs.id),
          type(rhs.type),
          mDestroyed(false),
          mCount(rhs.mCount) {
        if (rhs.mMutex) {
            mMutex = std::make_unique<utils::Mutex>();
        }
    }

    ~CounterImpl() = default;

    HandleId const id;        // 8
    ResourceType const type;  // 1

private:
    void destroyNow(ResourceType type, HandleId id);
    void destroyLater(ResourceType type, HandleId id);

    bool mDestroyed = false;  // 1
    uint16_t padding = 0;     // 2
    uint32_t mCount;          // 4

    // To keep down the size of this class, we only allocate utils::Mutex if necessary.
    std::unique_ptr<utils::Mutex> mMutex; // 8
};

static_assert(sizeof(CounterImpl<true>) == 24);

// This encapsulates a std::vector of counters and a list of free counters. The use of a vector (as
// oppose to dynamically allocating/deallocating counters) is meant to ensure memory locality.
template<bool THREAD_SAFE>
struct CounterPoolImpl {
private:
    using Counter = CounterImpl<THREAD_SAFE>;

public:

    static constexpr CounterIndex BAD_INDEX = -1;

    CounterPoolImpl(size_t arenaSize)
        : mArenaSize(arenaSize) {}

    Counter* get(CounterIndex ind) {
        assert_invariant(ind >=0 && ind < (CounterIndex) mCounters.size());
        return &mCounters[ind];
    }

    CounterIndex create(ResourceType type, HandleId id) {
        bool needToExpand = false;
        if constexpr (THREAD_SAFE) {
            std::unique_lock<utils::Mutex> lock(mFreeListMutex);
            needToExpand = mFreeList.empty();
        } else {
            needToExpand = mFreeList.empty();
        }

        if (needToExpand) {
            expand();
        }

        CounterIndex freeInd = BAD_INDEX;
        if constexpr (THREAD_SAFE) {
            std::unique_lock<utils::Mutex> lock(mFreeListMutex);
            freeInd = mFreeList.back();
            mFreeList.pop_back();
        } else {
            freeInd = mFreeList.back();
            mFreeList.pop_back();
        }
        createCounter(freeInd, type, id);
        return freeInd;
    }

    void free(CounterIndex ind) {
        if constexpr (THREAD_SAFE) {
            {
                std::unique_lock<utils::Mutex> lock(mCountersMutex);
                Counter* ret = &mCounters[ind];
                ret->~Counter();
            }
            {
                std::unique_lock<utils::Mutex> lock(mFreeListMutex);
                mFreeList.push_back(ind);
            }
        } else {
            Counter* ret = &mCounters[ind];
            ret->~Counter();
            mFreeList.push_back(ind);
        }
    }

private:
    // This actually allocates counter from a pool.  This is thread-safe.
    void createCounter(CounterIndex freeInd, ResourceType type, HandleId id);

    // Expand the pool if needed.
    void expand() {
        auto const expandImpl = [&]() -> std::pair<size_t, size_t> {
            size_t const oldSize = mCounters.size();
            mCounters.resize(!oldSize ? (size_t) (mArenaSize / 2) : (size_t) oldSize * 1.5);
            return {oldSize, mCounters.size()};
        };
        auto const addFree = [&](size_t from, size_t to) {
            for (size_t i = from; i < to; ++i) {
                mFreeList.push_back(i);
            }
        };

        if constexpr (THREAD_SAFE) {
            size_t countersCount = 0;
            size_t oldSize = 0;
            {
                std::unique_lock<utils::Mutex> lock(mCountersMutex);
                auto [olds, counters] = expandImpl();
                oldSize = olds;
                countersCount = counters;
            }
            {
                std::unique_lock<utils::Mutex> lock(mFreeListMutex);
                addFree(oldSize, countersCount);
            }
        } else {
            auto [olds, counters] = expandImpl();
            addFree(olds, counters);
        }
    }

    size_t const mArenaSize;
    utils::Mutex mCountersMutex;
    std::vector<CounterImpl<THREAD_SAFE>> mCounters;
    utils::Mutex mFreeListMutex;
    std::vector<int32_t> mFreeList;
};

// This wraps a counter, either thread-safe or not.
struct RefCounter {
private:
    using TSCounterPtr = CounterImpl<IS_THREAD_SAFE>*;
    using CounterPtr = CounterImpl<IS_NOT_THREAD_SAFE>*;

public:
    RefCounter() = default;

    RefCounter(TSCounterPtr counter)
        : mTsCounter{counter} {}

    RefCounter(CounterPtr counter)
        : mCounter{counter} {}

    RefCounter(RefCounter&& rhs) = default;
    RefCounter& operator=(RefCounter&& rhs) = default;

    RefCounter(RefCounter const& rhs) = default;
    RefCounter& operator=(RefCounter const& rhs) = default;

    inline void inc() {
        assert_invariant(bool(*this));
        if (mTsCounter) {
            mTsCounter->inc();
            return;
        }
        mCounter->inc();
    }
    inline void dec() {
        assert_invariant(bool(*this));
        if (mTsCounter) {
            mTsCounter->dec();
            return;
        }
        mCounter->dec();
    }
    inline HandleId id() const {
        assert_invariant(bool(*this));
        if (mTsCounter) {
            return mTsCounter->id;
        }
        return mCounter->id;
    }

    inline ResourceType type() const {
        assert_invariant(bool(*this));
        if (mTsCounter) {
            return mTsCounter->type;
        }
        return mCounter->type;
    }

    inline explicit operator bool() const {
        return mTsCounter || mCounter;
    }

    inline bool operator==(RefCounter const& rhs) const {
        return mTsCounter == rhs.mTsCounter && mCounter == rhs.mCounter;
    }

    inline bool operator!=(RefCounter const& rhs) const {
        return !(*this == rhs);
    }

private:
    TSCounterPtr mTsCounter = nullptr;
    CounterPtr mCounter = nullptr;
};

static_assert(sizeof(RefCounter) == 16);

// This wraps the thread-safe and non-thread-safe counter pools.
struct RefCounterPool {
public:
    RefCounterPool(size_t arenaSize)
        : mThreadSafePool(arenaSize),
          mPool(arenaSize) {}

    template<bool THREAD_SAFE>
    RefCounter get(CounterIndex ind) {
        if constexpr (THREAD_SAFE) {
            return {mThreadSafePool.get(ind)};
        } else {
            return {mPool.get(ind)};
        }
    }

    template<bool THREAD_SAFE>
    CounterIndex create(ResourceType type, HandleId id) {
        if constexpr (THREAD_SAFE) {
            return mThreadSafePool.create(type, id);
        } else {
            return mPool.create(type, id);
        }
    }

    template<bool THREAD_SAFE>
    void free(CounterIndex counterIndex) {
        if constexpr (THREAD_SAFE) {
            mThreadSafePool.free(counterIndex);
        } else {
            mPool.free(counterIndex);
        }
    }

private:
    CounterPoolImpl<IS_THREAD_SAFE> mThreadSafePool;
    CounterPoolImpl<IS_NOT_THREAD_SAFE> mPool;
};

} // namespace filament::backend::fvkmemory

#endif // TNT_FILAMENT_BACKEND_VULKAN_MEMORY_RESOURCECOUNTER_H
