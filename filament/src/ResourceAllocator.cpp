/*
 * Copyright (C) 2019 The Android Open Source Project
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

#include "ResourceAllocator.h"

#include <filament/Engine.h>

#include "details/Texture.h"

#include <backend/DriverApiForward.h>
#include <backend/Handle.h>
#include <backend/TargetBufferInfo.h>
#include <backend/DriverEnums.h>

#include "private/backend/DriverApi.h"

#include <utils/Logger.h>
#include <utils/algorithm.h>
#include <utils/bitset.h>
#include <utils/compiler.h>
#include <utils/debug.h>
#include <utils/ostream.h>

#include <array>
#include <algorithm>
#include <iterator>
#include <memory>
#include <optional>
#include <utility>

#include <stddef.h>
#include <stdint.h>

using namespace utils;

namespace filament {

using namespace backend;

// ------------------------------------------------------------------------------------------------

template<typename K, typename V, typename H>
UTILS_NOINLINE
ResourceAllocator::AssociativeContainer<K, V, H>::AssociativeContainer() {
    mContainer.reserve(128);
}

template<typename K, typename V, typename H>
UTILS_NOINLINE
ResourceAllocator::AssociativeContainer<K, V, H>::~AssociativeContainer() noexcept = default;

template<typename K, typename V, typename H>
UTILS_NOINLINE
typename ResourceAllocator::AssociativeContainer<K, V, H>::iterator
ResourceAllocator::AssociativeContainer<K, V, H>::erase(iterator it) {
    return mContainer.erase(it);
}

template<typename K, typename V, typename H>
typename ResourceAllocator::AssociativeContainer<K, V, H>::const_iterator
ResourceAllocator::AssociativeContainer<K, V, H>::find(key_type const& key) const {
    return const_cast<AssociativeContainer*>(this)->find(key);
}

template<typename K, typename V, typename H>
UTILS_NOINLINE
typename ResourceAllocator::AssociativeContainer<K, V, H>::iterator
ResourceAllocator::AssociativeContainer<K, V, H>::find(key_type const& key) {
    return std::find_if(mContainer.begin(), mContainer.end(), [&key](auto const& v) {
        return v.first == key;
    });
}

template<typename K, typename V, typename H>
template<typename... ARGS>
UTILS_NOINLINE
void ResourceAllocator::AssociativeContainer<K, V, H>::emplace(ARGS&& ... args) {
    mContainer.emplace_back(std::forward<ARGS>(args)...);
}

// ------------------------------------------------------------------------------------------------

ResourceAllocatorInterface::~ResourceAllocatorInterface() = default;

// ------------------------------------------------------------------------------------------------

ResourceAllocatorDisposerInterface::~ResourceAllocatorDisposerInterface() = default;

// ------------------------------------------------------------------------------------------------

size_t ResourceAllocator::TextureKey::getSize() const noexcept {
    size_t const pixelCount = width * height * depth;
    size_t size = pixelCount * FTexture::getFormatSize(format);
    size_t const s = std::max(uint8_t(1), samples);
    if (s > 1) {
        // if we have MSAA, we assume N times the storage
        size *= s;
    }
    if (levels > 1) {
        // if we have mip-maps we assume the full pyramid
        size += size / 3;
    }
    // TODO: this is not taking into account the potential sidecar MS buffer
    //  but we have no way to know about its existence at this point.
    return size;
}

ResourceAllocator::ResourceAllocator(Engine::Config const& config, DriverApi& driverApi) noexcept
        : mCacheMaxAge(config.resourceAllocatorCacheMaxAge),
          mBackend(driverApi),
          mDisposer(std::make_shared<ResourceAllocatorDisposer>(driverApi)) {
}

ResourceAllocator::ResourceAllocator(std::shared_ptr<ResourceAllocatorDisposer> disposer,
        Engine::Config const& config, DriverApi& driverApi) noexcept
        : mCacheMaxAge(config.resourceAllocatorCacheMaxAge),
          mBackend(driverApi),
          mDisposer(std::move(disposer)) {
}

ResourceAllocator::~ResourceAllocator() noexcept {
    assert_invariant(mTextureCache.empty());
}

void ResourceAllocator::terminate() noexcept {
    auto& textureCache = mTextureCache;
    for (auto it = textureCache.begin(); it != textureCache.end();) {
        mBackend.destroyTexture(it->second.handle);
        it = textureCache.erase(it);
    }
}

RenderTargetHandle ResourceAllocator::createRenderTarget(const char* name,
        TargetBufferFlags const targetBufferFlags, uint32_t const width, uint32_t const height,
        uint8_t const samples, uint8_t const layerCount, MRT const color, TargetBufferInfo const depth,
        TargetBufferInfo const stencil) noexcept {
    auto handle = mBackend.createRenderTarget(targetBufferFlags,
            width, height, samples ? samples : 1u, layerCount, color, depth, stencil);
    mBackend.setDebugTag(handle.getId(), CString{ name });
    return handle;
}

void ResourceAllocator::destroyRenderTarget(RenderTargetHandle const h) noexcept {
    mBackend.destroyRenderTarget(h);
}

TextureHandle ResourceAllocator::createTexture(const char* name,
        SamplerType const target, uint8_t const levels, TextureFormat const format, uint8_t samples,
        uint32_t const width, uint32_t const height, uint32_t const depth,
        std::array<TextureSwizzle, 4> const swizzle,
        TextureUsage const usage) noexcept {
    // The frame graph descriptor uses "0" to mean "auto" but the sample count that is passed to the
    // backend should always be 1 or greater.
    samples = samples ? samples : uint8_t(1);

    using TS = TextureSwizzle;
    constexpr const auto defaultSwizzle = std::array<TextureSwizzle, 4>{
        TS::CHANNEL_0, TS::CHANNEL_1, TS::CHANNEL_2, TS::CHANNEL_3};

    // do we have a suitable texture in the cache?
    TextureHandle handle;
    TextureKey const key{ name, target, levels, format, samples, width, height, depth, usage, swizzle };
    if constexpr (mEnabled) {
        auto& textureCache = mTextureCache;
        auto it = textureCache.find(key);
        if (UTILS_LIKELY(it != textureCache.end())) {
            // we do, move the entry to the in-use list, and remove from the cache
            handle = it->second.handle;
            mCacheSize -= it->second.size;
            textureCache.erase(it);
        } else {
            // we don't, allocate a new texture and populate the in-use list
            handle = mBackend.createTexture(
                    target, levels, format, samples, width, height, depth, usage);
            if (swizzle != defaultSwizzle) {
                TextureHandle swizzledHandle = mBackend.createTextureViewSwizzle(
                        handle, swizzle[0], swizzle[1], swizzle[2], swizzle[3]);
                mBackend.destroyTexture(handle);
                handle = swizzledHandle;
            }
        }
    } else {
        handle = mBackend.createTexture(
                target, levels, format, samples, width, height, depth, usage);
        if (swizzle != defaultSwizzle) {
            TextureHandle swizzledHandle = mBackend.createTextureViewSwizzle(
                    handle, swizzle[0], swizzle[1], swizzle[2], swizzle[3]);
            mBackend.destroyTexture(handle);
            handle = swizzledHandle;
        }
    }
    mDisposer->checkout(handle, key);
    mBackend.setDebugTag(handle.getId(), CString{ name });
    return handle;
}

void ResourceAllocator::destroyTexture(TextureHandle const h) noexcept {
    auto const key = mDisposer->checkin(h);
    if constexpr (mEnabled) {
        if (UTILS_LIKELY(key.has_value())) {
            uint32_t const size = key.value().getSize();
            mTextureCache.emplace(key.value(), TextureCachePayload{ h, mAge, size });
            mCacheSize += size;
            mCacheSizeHiWaterMark = std::max(mCacheSizeHiWaterMark, mCacheSize);
        }
    } else {
        mBackend.destroyTexture(h);
    }
}

ResourceAllocatorDisposerInterface& ResourceAllocator::getDisposer() noexcept {
    return *mDisposer;
}

void ResourceAllocator::gc(bool const skippedFrame) noexcept {
    // this is called regularly -- usually once per frame

    // increase our age at each (non-skipped) frame
    const size_t age = mAge;
    if (!skippedFrame) {
        mAge++;
    }

    // Purging strategy:
    //  - remove all entries older than MAX_AGE_SKIPPED_FRAME when skipping a frame
    //  - remove entries older than mCacheMaxAgeSoft
    //      - remove only MAX_EVICTION_COUNT entry per gc(),
    //  - look for the number of unique resource ages present in the cache (this basically gives
    //    us how many buckets of resources we have corresponding to previous frames.
    //      - remove all resources that have an age older than the MAX_UNIQUE_AGE_COUNT'th bucket

    auto& textureCache = mTextureCache;

    // when skipping a frame, the maximum age to keep in the cache
    constexpr size_t MAX_AGE_SKIPPED_FRAME = 1;

    // maximum entry count to evict per GC, under the mCacheMaxAgeSoft limit
    constexpr size_t MAX_EVICTION_COUNT = 1;

    // maximum number of unique ages in the cache
    constexpr size_t MAX_UNIQUE_AGE_COUNT = 3;

    bitset32 ages;
    uint32_t evictedCount = 0;
    for (auto it = textureCache.begin(); it != textureCache.end();) {
        size_t const ageDiff = age - it->second.age;
        if ((ageDiff >= MAX_AGE_SKIPPED_FRAME && skippedFrame) ||
            (ageDiff >= mCacheMaxAge && evictedCount < MAX_EVICTION_COUNT)) {
            evictedCount++;
            it = purge(it);
        } else {
            // build the set of ages present in the cache after eviction
            ages.set(std::min(size_t(31), ageDiff));
            ++it;
        }
    }

    // if we have MAX_UNIQUE_AGE_COUNT ages or more, we evict all the resources that
    // are older than the MAX_UNIQUE_AGE_COUNT'th age.
    if (!skippedFrame && ages.count() >= MAX_UNIQUE_AGE_COUNT) {
        uint32_t bits = ages.getValue();
        // remove from the set the ages we keep
        for (size_t i = 0; i < MAX_UNIQUE_AGE_COUNT - 1; i++) {
            bits &= ~(1 << ctz(bits));
        }
        size_t const maxAge = ctz(bits);
        for (auto it = textureCache.begin(); it != textureCache.end();) {
            const size_t ageDiff = age - it->second.age;
            if (ageDiff >= maxAge) {
                it = purge(it);
            } else {
                ++it;
            }
        }
    }
}

UTILS_NOINLINE
void ResourceAllocator::dump(bool const brief) const noexcept {
    constexpr float MiB = 1.0f / float(1u << 20u);
    DLOG(INFO) << "# entries=" << mTextureCache.size() << ", sz=" << (float) mCacheSize * MiB
               << " MiB"
               << ", max=" << (float) mCacheSizeHiWaterMark * MiB << " MiB";
    if (!brief) {
        for (auto const& it : mTextureCache) {
            auto w = it.first.width;
            auto h = it.first.height;
            auto f = FTexture::getFormatSize(it.first.format);
            DLOG(INFO) << it.first.name << ": w=" << w << ", h=" << h << ", f=" << f
                       << ", sz=" << (float) it.second.size * MiB;
        }
    }
}

ResourceAllocator::CacheContainer::iterator
ResourceAllocator::purge(
        CacheContainer::iterator const& pos) {
    // DLOG(INFO) << "purging " << pos->second.handle.getId() << ", age=" << pos->second.age;
    mBackend.destroyTexture(pos->second.handle);
    mCacheSize -= pos->second.size;
    return mTextureCache.erase(pos);
}

// ------------------------------------------------------------------------------------------------

ResourceAllocatorDisposer::ResourceAllocatorDisposer(DriverApi& driverApi) noexcept
        : mBackend(driverApi) {
}

ResourceAllocatorDisposer::~ResourceAllocatorDisposer() noexcept {
     assert_invariant(mInUseTextures.empty());
}

void ResourceAllocatorDisposer::terminate() noexcept {
    assert_invariant(mInUseTextures.empty());
}

void ResourceAllocatorDisposer::destroy(TextureHandle const handle) noexcept {
    if (handle) {
        auto r = checkin(handle);
        if (r.has_value()) {
            mBackend.destroyTexture(handle);
        }
    }
}

void ResourceAllocatorDisposer::checkout(TextureHandle handle,
        ResourceAllocator::TextureKey key) {
    mInUseTextures.emplace(handle, key);
}

std::optional<ResourceAllocator::TextureKey> ResourceAllocatorDisposer::checkin(
        TextureHandle handle) {
    // find the texture in the in-use list (it must be there!)
    auto it = mInUseTextures.find(handle);
    assert_invariant(it != mInUseTextures.end());
    if (it == mInUseTextures.end()) {
        return std::nullopt;
    }
    TextureKey const key = it->second;
    // remove it from the in-use list
    mInUseTextures.erase(it);
    return key;
}

} // namespace filament
