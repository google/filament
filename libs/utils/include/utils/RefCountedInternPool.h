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

#include <utils/ConstSlice.h>
#include <utils/FixedCapacityVector.h>
#include <utils/Hash.h>
#include <utils/Panic.h>
#include <utils/debug.h>

#include <tsl/robin_map.h>

namespace utils {

/** A reference-counted intern pool, where keys are FixedCapacityVectors over T. */
template<typename T, typename Hash = std::hash<T>>
class RefCountedInternPool {
    struct HashSlice {
        size_t operator()(ConstSlice<T> const& value) const noexcept {
            size_t seed = 0;
            for (auto const& it : value) {
                hash::combine_fast(seed, Hash{}(it));
            }
            return seed;
        }
    };

    struct Entry {
        uint32_t referenceCount;
        FixedCapacityVector<T> value;
    };

    using Map = tsl::robin_map<ConstSlice<T>, Entry, HashSlice>;

    static constexpr const char* UTILS_NONNULL MISSING_ENTRY_ERROR_STRING =
            "RefCountedInternPool is missing entry";

public:
    RefCountedInternPool(RefCountedInternPool const& rhs) = delete;

    /** Acquire an interned copy of value. */
    ConstSlice<T> acquire(ConstSlice<T> const& value, size_t hash) noexcept {
        auto it = mMap.find(value, hash);
        if (it != mMap.end()) {
            it.value().referenceCount++;
            return it.key();
        }
        FixedCapacityVector<T> interned(value.size());
        std::copy(value.cbegin(), value.cend(), interned.begin());
        // TODO: how to use above computed hash here?
        return mMap.insert({ ConstSlice(interned.cbegin(), interned.cend()), Entry{
                .referenceCount = 1,
                .value = std::move(interned),
            }}).first.key();
    }

    inline ConstSlice<T> acquire(ConstSlice<T> const& value) noexcept {
        return acquire(value, HashSlice{}(value));
    }

    inline ConstSlice<T> acquire(FixedCapacityVector<T> const& value, size_t hash) noexcept {
        return acquire(ConstSlice(value.cbegin(), value.cend()), hash);
    }

    inline ConstSlice<T> acquire(FixedCapacityVector<T> const& value) noexcept {
        ConstSlice slice(value.cbegin(), value.cend());
        return acquire(slice, HashSlice{}(slice));
    }

    /** Release interned value. */
    void release(ConstSlice<T> const& value, size_t hash) noexcept {
        auto it = mMap.find(value, hash);
        FILAMENT_CHECK_PRECONDITION(it != mMap.end()) << MISSING_ENTRY_ERROR_STRING;
        if (--it.value().referenceCount == 0) {
            // TODO: change to erase_fast
            mMap.erase(it);
        }
    }

    inline void release(ConstSlice<T> const& value) noexcept {
        return release(value, HashSlice{}(value));
    }

    inline void release(FixedCapacityVector<T> const& value, size_t hash) noexcept {
        return release(ConstSlice(value.cbegin(), value.cend()), hash);
    }

    inline void release(FixedCapacityVector<T> const& value) noexcept {
        ConstSlice slice(value.cbegin(), value.cend());
        return release(slice, HashSlice{}(slice));
    }

    /** Returns true if the pool is empty. */
    inline bool empty() const noexcept { return mMap.empty(); }

    /** Returns hash of value. */
    size_t hash(ConstSlice<T> const& value) const noexcept {
        return HashSlice{}(value);
    }

    size_t hash(FixedCapacityVector<T> const& value) const noexcept {
        return HashSlice{}(ConstSlice(value.cbegin(), value.cend()));
    }

private:
    Map mMap;
};

}

#endif  // TNT_UTILS_REFCOUNTEDINTERNPOOL_H
