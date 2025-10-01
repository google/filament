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
#ifndef TNT_UTILS_REFCOUNTEDINTERNPOOL_H
#define TNT_UTILS_REFCOUNTEDINTERNPOOL_H

#include <utils/Slice.h>
#include <utils/FixedCapacityVector.h>
#include <utils/Hash.h>
#include <utils/Panic.h>
#include <utils/debug.h>

#include <tsl/robin_map.h>

namespace utils {

/** A reference-counted intern pool of slices of T. */
template<typename T, typename Hash = std::hash<T>>
class RefCountedInternPool {
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
            "RefCountedInternPool is missing entry";

public:
    RefCountedInternPool() = default;
    RefCountedInternPool(RefCountedInternPool const& rhs) = delete;
    RefCountedInternPool& operator=(RefCountedInternPool const& rhs) = delete;
    RefCountedInternPool(RefCountedInternPool&& rhs) = default;
    RefCountedInternPool& operator=(RefCountedInternPool&& rhs) = default;

    /** Acquire an interned copy of value. */
    Slice<const T> acquire(Slice<const T> slice, size_t hash) noexcept {
        if (slice.empty()) {
            return { nullptr, nullptr };
        }
        auto it = mMap.find(slice, hash);
        if (it != mMap.end()) {
            it.value().referenceCount++;
            return it.key();
        }
        FixedCapacityVector<T> value(slice);
        // TODO: how to use above computed hash here?
        return mMap.insert({ value.as_slice(), Entry{
                .referenceCount = 1,
                .value = std::move(value),
            }}).first.key();
    }

    inline Slice<const T> acquire(Slice<const T> slice) noexcept {
        return acquire(slice, HashSlice{}(slice));
    }

    Slice<const T> acquire(FixedCapacityVector<T>&& value, size_t hash) noexcept {
        if (value.empty()) {
            return { nullptr, nullptr };
        }
        Slice<const T> slice = value.as_slice();
        auto it = mMap.find(slice, hash);
        if (it != mMap.end()) {
            it.value().referenceCount++;
            return it.key();
        }
        // TODO: how to use above computed hash here?
        return mMap.insert({ slice, Entry{
                .referenceCount = 1,
                .value = std::move(value),
            }}).first.key();
    }

    inline Slice<const T> acquire(FixedCapacityVector<T>&& value) noexcept {
        return acquire(std::move(value), HashSlice{}(value.as_slice()));
    }

    inline Slice<const T> acquire(FixedCapacityVector<T> const& value, size_t hash) noexcept {
        return acquire(value.as_slice(), hash);
    }

    inline Slice<const T> acquire(FixedCapacityVector<T> const& value) noexcept {
        Slice slice = value.as_slice();
        return acquire(slice, HashSlice{}(slice));
    }

    /** Release interned value. */
    void release(Slice<const T> slice, size_t hash) noexcept {
        if (slice.empty()) {
            return;
        }
        auto it = mMap.find(slice, hash);
        FILAMENT_CHECK_PRECONDITION(it != mMap.end()) << MISSING_ENTRY_ERROR_STRING;
        if (--it.value().referenceCount == 0) {
            // TODO: change to erase_fast
            mMap.erase(it);
        }
    }

    inline void release(Slice<const T> slice) noexcept {
        return release(slice, HashSlice{}(slice));
    }

    inline void release(FixedCapacityVector<T> const& value, size_t hash) noexcept {
        return release(value.as_slice(), hash);
    }

    inline void release(FixedCapacityVector<T> const& value) noexcept {
        Slice slice = value.as_slice();
        return release(slice, HashSlice{}(slice));
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
    Map mMap;
};

}

#endif  // TNT_UTILS_REFCOUNTEDINTERNPOOL_H
