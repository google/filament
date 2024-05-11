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

#include <memory>

namespace filament::backend {

struct BlobCacheKey::Key {
    uint64_t id;
    Program::SpecializationConstant constants[];
};

BlobCacheKey::BlobCacheKey() noexcept = default;

BlobCacheKey::BlobCacheKey(uint64_t id,
        BlobCacheKey::SpecializationConstants const& specConstants) {
    mSize = sizeof(Key) + sizeof(Key::constants[0]) * specConstants.size();

    Key* const pKey = (Key *)malloc(mSize);
    memset(pKey, 0, mSize);
    mData.reset(pKey, ::free);

    mData->id = id;
    for (size_t i = 0; i < specConstants.size(); i++) {
        mData->constants[i] = specConstants[i];
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
