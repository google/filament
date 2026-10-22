/*
 * Copyright (C) 2023 The Android Open Source Project
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

#include "BlobCacheKey.h"

#include <cstring>
#include <memory>
#include <type_traits>
#include <variant>

namespace filament::backend {

struct BlobCacheKey::Key {
    // The key is hashed/compared as raw bytes by the platform blob cache, so its
    // object representation must be fully deterministic (no padding, no unused bytes).
    // This is why we can't store Program::SpecializationConstant (a std::variant) directly.
    struct Entry {
        uint32_t type;      // index of the active variant alternative
        uint32_t value;     // bool as 0/1, other alternatives as their raw 32 bits
    };
    uint64_t id;
    Entry constants[];
};

BlobCacheKey::BlobCacheKey() noexcept = default;

BlobCacheKey::BlobCacheKey(uint64_t id,
        BlobCacheKey::SpecializationConstants const& specConstants) {
    static_assert(std::has_unique_object_representations_v<Key::Entry>,
            "BlobCacheKey bytes are hashed/compared directly; no padding allowed");
    static_assert(sizeof(Key) == sizeof(uint64_t),
            "BlobCacheKey::Key must not have padding");

    mSize = sizeof(Key) + sizeof(Key::Entry) * specConstants.size();

    Key* const pKey = (Key *)malloc(mSize);
    memset(pKey, 0, mSize);
    mData.reset(pKey, ::free);

    mData->id = id;
    for (size_t i = 0; i < specConstants.size(); i++) {
        auto const& sc = specConstants[i];
        Key::Entry& entry = mData->constants[i];
        entry.type = uint32_t(sc.index());
        // Encode explicitly so the layout doesn't depend on endianness, and require every
        // non-bool alternative to be exactly 32 bits so new types must pick an encoding.
        std::visit([&entry](auto v) {
            using T = decltype(v);
            if constexpr (std::is_same_v<T, bool>) {
                entry.value = v ? 1u : 0u;
            } else {
                static_assert(sizeof(T) == sizeof(entry.value),
                        "new SpecializationConstant types need an explicit encoding");
                std::memcpy(&entry.value, &v, sizeof(v));
            }
        }, sc);
    }
}

BlobCacheKey::BlobCacheKey(BlobCacheKey&& rhs) noexcept
        : mData(std::move(rhs.mData)), mSize(rhs.mSize) {
    rhs.mSize = 0;
}

BlobCacheKey& BlobCacheKey::operator=(BlobCacheKey&& rhs) noexcept {
    if (this != &rhs) {
        using std::swap;
        swap(mData, rhs.mData);
        swap(mSize, rhs.mSize);
    }
    return *this;
}

} // namespace filament::backend
