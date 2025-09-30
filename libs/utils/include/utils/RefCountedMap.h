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
#ifndef TNT_UTILS_REFCOUNTEDMAP_H
#define TNT_UTILS_REFCOUNTEDMAP_H

#include <utils/Panic.h>
#include <utils/compiler.h>
#include <utils/debug.h>

#include <tsl/robin_map.h>

#include <optional>
#include <type_traits>
#include <utility>

namespace utils {

namespace refcountedmap {

template <typename T, typename = void>
struct is_pointer_like_trait : std::false_type {};

template <typename T>
struct is_pointer_like_trait<T, std::void_t<decltype(*std::declval<T&>())>> : std::true_type {};

template<typename T>
inline constexpr bool IsPointer = is_pointer_like_trait<T>::value;

template<typename T, typename = void>
struct PointerTraits {
    using element_type = T;
};

template<typename T>
struct PointerTraits<T, std::enable_if_t<IsPointer<T>>> {
    using element_type = typename std::pointer_traits<T>::element_type;
};

} // namespace refcountedmap

/** A reference-counted map.
 *
 * Don't use RAII here, both because we sometimes want to deliberately leak memory, and because
 * we're managing GL resources that require more managed destruction.
 */
template<typename Key, typename T, typename Hash = std::hash<Key>>
class RefCountedMap {
    // Use references for the key if the size of the key type is greater than the size of a pointer.
    using KeyRef = std::conditional_t<(sizeof(Key) > sizeof(void*)), const Key&, Key>;
    using TValue = typename refcountedmap::PointerTraits<T>::element_type;

    struct Entry {
        uint32_t referenceCount;
        T value;
    };

    using Map = tsl::robin_map<Key, Entry, Hash>;

    static constexpr TValue& deref(T& a) {
        if constexpr (refcountedmap::IsPointer<T>) {
            return *a;
        } else {
            return a;
        }
    }

    static constexpr TValue const& deref(T const& a) {
        if constexpr (refcountedmap::IsPointer<T>) {
            return *a;
        } else {
            return a;
        }
    }

    static constexpr const char* UTILS_NONNULL MISSING_ENTRY_ERROR_STRING =
            "Cache is missing entry";

public:
    /** Acquire a new reference to value by key, initializing it with F if it doesn't exist.
     *
     * If T is a pointer type, F returns T, where nullptr indicates a failure to create the object.
     * Otherwise, F returns std::optional<T>, where nullopt indicates a failure to create the
     * object, and the returned pointer is valid only as long as the next call to acquire() or
     * release().
     */
    template<typename F>
    TValue* UTILS_NULLABLE acquire(KeyRef key, size_t hash, F factory) noexcept {
        auto it = mMap.find(key, hash);
        if (it != mMap.end()) {
            it.value().referenceCount++;
            return &deref(it.value().value);
        }
        if constexpr (refcountedmap::IsPointer<T>) {
            T r = factory();
            if (r) {
                // TODO: how to use above computed hash here?
                return &*mMap.insert({key, Entry{ 1, std::move(r) }}).first.value().value;
            }
            return nullptr;
        } else {
            std::optional<T> r = factory();
            if (r) {
                // TODO: how to use above computed hash here?
                return &mMap.insert({key, Entry{ 1, std::move(r.value()) }}).first.value().value;
            }
            return nullptr;
        }
    }

    template<typename F>
    inline TValue* UTILS_NULLABLE acquire(KeyRef key, F factory) noexcept {
        return acquire(key, Hash{}(key), std::move(factory));
    }

    /** Acquire a reference to value by key, panicking if it doesn't exist.
     *
     * This reference is valid only as long as the next call to acquire() or release().
     */
    TValue& acquire(KeyRef key, size_t hash) noexcept {
        auto it = mMap.find(key, hash);
        FILAMENT_CHECK_PRECONDITION(it != mMap.end()) << MISSING_ENTRY_ERROR_STRING;
        it.value().referenceCount++;
        return deref(it.value().value);
    }

    inline TValue& acquire(KeyRef key) noexcept {
        return acquire(key, Hash{}(key));
    }

    /** Release a reference to key, destroying it with F if reference count reaches zero.
     *
     * Panics if no entry found in map.
     */
    template<typename F>
    void release(KeyRef key, size_t hash, F releaser) noexcept {
        auto it = mMap.find(key, hash);
        FILAMENT_CHECK_PRECONDITION(it != mMap.end()) << MISSING_ENTRY_ERROR_STRING;
        if (--it.value().referenceCount == 0) {
            releaser(deref(it.value().value));
            // TODO: change to erase_fast
            mMap.erase(it);
        }
    }

    template<typename F>
    inline void release(KeyRef key, F releaser) noexcept {
        release(key, Hash{}(key), std::move(releaser));
    }

    /** Release a reference to key.
     *
     * Panics if no entry found in map.
     */
    void release(KeyRef key, size_t hash) noexcept {
        auto it = mMap.find(key, hash);
        FILAMENT_CHECK_PRECONDITION(it != mMap.end()) << MISSING_ENTRY_ERROR_STRING;
        if (--it.value().referenceCount == 0) {
            // TODO: change to erase_fast
            mMap.erase(it);
        }
    }

    inline void release(KeyRef key) noexcept {
        release(key, Hash{}(key));
    }

    /** Return reference to existing value by key.
     *
     * This reference is valid only as long as the next call to acquire() or release().
     *
     * Panics if no entry found in map.
     */
    TValue& get(KeyRef key, size_t hash) noexcept {
        auto it = mMap.find(key);
        FILAMENT_CHECK_PRECONDITION(it != mMap.end()) << MISSING_ENTRY_ERROR_STRING;
        return deref(it.value().value);
    }

    inline TValue& get(KeyRef key) noexcept { return get(key, Hash{}(key)); }

    TValue const& get(KeyRef key, size_t hash) const noexcept {
        auto it = mMap.find(key);
        FILAMENT_CHECK_PRECONDITION(it != mMap.end()) << MISSING_ENTRY_ERROR_STRING;
        return deref(it->second.value);
    }

    inline TValue const& get(KeyRef key) const noexcept { return get(key, Hash{}(key)); }

    /** Returns true if the map is empty. */
    inline bool empty() const noexcept { return mMap.empty(); }

private:
    Map mMap;
};

}

#endif  // TNT_UTILS_REFCOUNTEDMAP_H
