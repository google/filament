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

#ifndef TNT_FILAMENT_BACKEND_PRIVATE_BLOBCACHEKEY_H
#define TNT_FILAMENT_BACKEND_PRIVATE_BLOBCACHEKEY_H

#include <backend/Program.h>

namespace filament::backend {

class BlobCacheKey {
public:
    using SpecializationConstants = utils::FixedCapacityVector<Program::SpecializationConstant>;

    BlobCacheKey() noexcept;
    BlobCacheKey(uint64_t id, SpecializationConstants const& specConstants);

    BlobCacheKey(BlobCacheKey const& rhs) = default;
    BlobCacheKey& operator=(BlobCacheKey const& rhs) = default;

    BlobCacheKey(BlobCacheKey&& rhs) noexcept;
    BlobCacheKey& operator=(BlobCacheKey&& rhs) noexcept;

    void const* data() const noexcept {
        return mData.get();
    }

    size_t size() const noexcept {
        return mSize;
    }

    explicit operator bool() const noexcept { return mSize > 0; }

    void swap(BlobCacheKey& other) noexcept {
        using std::swap;
        swap(other.mSize, mSize);
        swap(other.mData, mData);
    }

private:
    struct Key;

    friend void swap(BlobCacheKey& lhs, BlobCacheKey& rhs) noexcept {
        lhs.swap(rhs);
    }

    // we use a shared_ptr to play nice with std::function<>, it works because the buffer is const
    std::shared_ptr<Key> mData{};
    size_t mSize{};
};

} // namespace filament::backend

#endif // TNT_FILAMENT_BACKEND_PRIVATE_BLOBCACHEKEY_H
