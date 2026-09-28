/*
 * Copyright (C) 2025 The Android Open Source Project
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
#ifndef TNT_UTILS_INTERNPOOL_H
#define TNT_UTILS_INTERNPOOL_H

#include <utils/debug.h>
#include <utils/FixedCapacityVector.h>
#include <utils/Hash.h>
#include <utils/Panic.h>
#include <utils/Slice.h>

#include <tsl/robin_map.h>

#include <limits>

namespace utils {

/** A reference-counted intern pool of slices of T. */
template<typename T, typename Hash = std::hash<T>>
class InternPool {
    struct HashSlice {
        inline size_t operator()(Slice<const T> const& slice) const noexcept {
            return slice.template hash<Hash>();
        }
    };

    struct Entry {
        uint32_t referenceCount;
        FixedCapacityVector<T> value;
    };

    using Map = tsl::robin_map<Slice<const T>, Entry, HashSlice>;

    static constexpr const char* UTILS_NONNULL MISSING_ENTRY_ERROR_STRING =
            "InternPool is missing entry";

public:
    /**
     * An RAII owning handle to an interned slice.
     *
     * Keeps the underlying pool entry alive until destroyed; any owner of an interned
     * Slice<const T> (e.g., as a map key) must retain a Ref to prevent dangling pointers.
     * Releasing on destruction makes it safe to store in containers without cleanup hooks
     * (e.g., LRU eviction or clear()).
     */
    class Ref {
    public:
        Ref() noexcept = default;

        // Copying is deliberately not implicit: an extra reference should be visible at the call
        // site. Use clone() to make one.
        Ref(Ref const& rhs) = delete;
        Ref& operator=(Ref const& rhs) = delete;

        Ref(Ref&& rhs) noexcept : mPool(rhs.mPool), mSlice(rhs.mSlice), mHash(rhs.mHash) {
            rhs.mPool = nullptr;
            rhs.mSlice.clear();
        }

        Ref& operator=(Ref&& rhs) noexcept {
            if (this != &rhs) {
                releaseSelf();
                mPool = rhs.mPool;
                mSlice = rhs.mSlice;
                mHash = rhs.mHash;
                rhs.mPool = nullptr;
                rhs.mSlice.clear();
            }
            return *this;
        }

        ~Ref() noexcept {
            releaseSelf();
        }

        /** Returns another owning reference to the same interned slice. */
        Ref clone() const noexcept {
            if (!mPool) {
                return {};
            }
            return Ref{ mPool, mPool->intern(mSlice, mHash), mHash };
        }

        /** The interned slice this reference keeps alive. Empty if there is none. */
        Slice<const T> get() const noexcept { return mSlice; }

        /** Returns true if this reference doesn't refer to anything. */
        bool empty() const noexcept { return mSlice.empty(); }

    private:
        friend class InternPool;

        Ref(InternPool* UTILS_NONNULL pool, Slice<const T> slice, size_t hash) noexcept
                : mPool(pool),
                  mSlice(slice),
                  mHash(hash) {}

        // Both callers are destruction paths, so this must not throw.
        void releaseSelf() noexcept {
            if (mPool) {
                bool const released = mPool->release(mSlice, mHash);
                ASSERT_DESTRUCTOR(released, "%s", MISSING_ENTRY_ERROR_STRING);
            }
        }

        InternPool* UTILS_NULLABLE mPool = nullptr;
        Slice<const T> mSlice;
        size_t mHash = 0;
    };

    InternPool() = default;
    InternPool(InternPool const& rhs) = delete;
    InternPool& operator=(InternPool const& rhs) = delete;
    InternPool(InternPool&& rhs) = delete;
    InternPool& operator=(InternPool&& rhs) = delete;

    ~InternPool() noexcept {
        assert_invariant(mMap.empty());
    }

    /** Acquire an owning reference to an interned copy of value. */
    Ref acquire(Slice<const T> slice, size_t hash) noexcept {
        return Ref{ this, intern(slice, hash), hash };
    }

    inline Ref acquire(Slice<const T> slice) noexcept {
        return acquire(slice, HashSlice{}(slice));
    }

    inline Ref acquire(FixedCapacityVector<T>&& value, size_t hash) noexcept {
        return Ref{ this, intern(std::move(value), hash), hash };
    }

    inline Ref acquire(FixedCapacityVector<T>&& value) noexcept {
        size_t hash = HashSlice{}(value.as_slice());
        return acquire(std::move(value), hash);
    }

    inline Ref acquire(FixedCapacityVector<T> const& value, size_t hash) noexcept {
        return acquire(value.as_slice(), hash);
    }

    inline Ref acquire(FixedCapacityVector<T> const& value) noexcept {
        Slice slice = value.as_slice();
        return acquire(slice, HashSlice{}(slice));
    }

    /** Returns true if the pool is empty. */
    inline bool empty() const noexcept { return mMap.empty(); }

    /** Returns hash of value. */
    static size_t hash(Slice<const T> slice) noexcept {
        return HashSlice{}(slice);
    }

    static size_t hash(FixedCapacityVector<T> const& value) noexcept {
        return HashSlice{}(value.as_slice());
    }

private:
    /** Intern a copy of value, adding a reference to it. Returns the pool's canonical slice. */
    Slice<const T> intern(Slice<const T> slice, size_t hash) noexcept {
        if (slice.empty()) {
            return { nullptr, nullptr };
        }
        auto it = mMap.find(slice, hash);
        if (it != mMap.end()) {
            assert_invariant(it.value().referenceCount <
                             std::numeric_limits<decltype(Entry::referenceCount)>::max());
            it.value().referenceCount++;
            return it.key();
        }
        FixedCapacityVector<T> value(slice);
        // TODO: how to use above computed hash here?
        return mMap.insert({ value.as_slice(), Entry{ 1, std::move(value) } }).first.key();
    }

    Slice<const T> intern(FixedCapacityVector<T>&& value, size_t hash) noexcept {
        if (value.empty()) {
            return { nullptr, nullptr };
        }
        Slice<const T> slice = value.as_slice();
        auto it = mMap.find(slice, hash);
        if (it != mMap.end()) {
            assert_invariant(it.value().referenceCount <
                             std::numeric_limits<decltype(Entry::referenceCount)>::max());
            it.value().referenceCount++;
            return it.key();
        }
        // TODO: how to use above computed hash here?
        return mMap.insert({ slice, Entry{ 1, std::move(value) } }).first.key();
    }

    /** Release interned value. Returns false if slice has no entry in the pool. */
    [[nodiscard]] bool release(Slice<const T> slice, size_t hash) noexcept {
        if (slice.empty()) {
            return true;
        }
        auto it = mMap.find(slice, hash);
        if (UTILS_UNLIKELY(it == mMap.end())) {
            return false;
        }
        if (--it.value().referenceCount == 0) {
            // TODO: change to erase_fast
            mMap.erase(it);
        }
        return true;
    }

    Map mMap;
};

} // namespace utils

#endif  // TNT_UTILS_INTERNPOOL_H
