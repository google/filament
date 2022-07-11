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

#include "private/backend/DriverApi.h"

#include "details/Texture.h"

#include <utils/FixedCapacityVector.h>
#include <utils/Log.h>
#include <utils/debug.h>

#include <iterator>

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
ResourceAllocator::AssociativeContainer<K, V, H>::~AssociativeContainer() noexcept {
}

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

size_t ResourceAllocator::TextureKey::getSize() const noexcept {
    size_t pixelCount = width * height * depth;
    size_t size = pixelCount * FTexture::getFormatSize(format);
    size_t s = std::max(uint8_t(1), samples);
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

ResourceAllocator::ResourceAllocator(DriverApi& driverApi) noexcept
        : mBackend(driverApi) {
}

ResourceAllocator::~ResourceAllocator() noexcept {
    assert_invariant(!mTextureCache.size());
    assert_invariant(!mInUseTextures.size());
}

void ResourceAllocator::terminate() noexcept {
    assert_invariant(!mInUseTextures.size());
    auto& textureCache = mTextureCache;
    for (auto it = textureCache.begin(); it != textureCache.end();) {
        mBackend.destroyTexture(it->second.handle);
        it = textureCache.erase(it);
    }
}

RenderTargetHandle ResourceAllocator::createRenderTarget(const char* name,
        TargetBufferFlags targetBufferFlags, uint32_t width, uint32_t height,
        uint8_t samples, MRT color, TargetBufferInfo depth,
        TargetBufferInfo stencil) noexcept {
    return mBackend.createRenderTarget(targetBufferFlags,
            width, height, samples ? samples : 1u, color, depth, stencil);
}

void ResourceAllocator::destroyRenderTarget(RenderTargetHandle h) noexcept {
    mBackend.destroyRenderTarget(h);
}

backend::TextureHandle ResourceAllocator::createTexture(const char* name,
        backend::SamplerType target, uint8_t levels, backend::TextureFormat format, uint8_t samples,
        uint32_t width, uint32_t height, uint32_t depth,
        std::array<backend::TextureSwizzle, 4> swizzle,
        backend::TextureUsage usage) noexcept {
    // The frame graph descriptor uses "0" to mean "auto" but the sample count that is passed to the
    // backend should always be 1 or greater.
    samples = samples ? samples : uint8_t(1);

    using TS = backend::TextureSwizzle;
    constexpr const auto defaultSwizzle = std::array<backend::TextureSwizzle, 4>{
        TS::CHANNEL_0, TS::CHANNEL_1, TS::CHANNEL_2, TS::CHANNEL_3};

    // do we have a suitable texture in the cache?
    TextureHandle handle;
    if constexpr (mEnabled) {
        auto& textureCache = mTextureCache;
        const TextureKey key{ name, target, levels, format, samples, width, height, depth, usage, swizzle };
        auto it = textureCache.find(key);
        if (UTILS_LIKELY(it != textureCache.end())) {
            // we do, move the entry to the in-use list, and remove from the cache
            handle = it->second.handle;
            mCacheSize -= it->second.size;
            textureCache.erase(it);
        } else {
            // we don't, allocate a new texture and populate the in-use list
            if (swizzle == defaultSwizzle) {
                handle = mBackend.createTexture(
                        target, levels, format, samples, width, height, depth, usage);
            } else {
                handle = mBackend.createTextureSwizzled(
                        target, levels, format, samples, width, height, depth, usage,
                        swizzle[0], swizzle[1], swizzle[2], swizzle[3]);
            }
        }
        mInUseTextures.emplace(handle, key);
    } else {
        if (swizzle == defaultSwizzle) {
            handle = mBackend.createTexture(
                    target, levels, format, samples, width, height, depth, usage);
        } else {
            handle = mBackend.createTextureSwizzled(
                    target, levels, format, samples, width, height, depth, usage,
                    swizzle[0], swizzle[1], swizzle[2], swizzle[3]);
        }
    }
    return handle;
}

void ResourceAllocator::destroyTexture(TextureHandle h) noexcept {
    if constexpr (mEnabled) {
        // find the texture in the in-use list (it must be there!)
        auto it = mInUseTextures.find(h);
        assert_invariant(it != mInUseTextures.end());

        // move it to the cache
        const TextureKey key = it->second;
        uint32_t size = key.getSize();

        mTextureCache.emplace(key, TextureCachePayload{ h, mAge, size });
        mCacheSize += size;

        // remove it from the in-use list
        mInUseTextures.erase(it);
    } else {
        mBackend.destroyTexture(h);
    }
}

void ResourceAllocator::gc() noexcept {
    // this is called regularly -- usually once per frame of each Renderer

    // increase our age
    const size_t age = mAge++;

    // Purging strategy:
    //  - remove entries that are older than a certain age
    //      - remove only one entry per gc(),
    //      - unless we're at capacity
    // - remove LRU entries until we're below capacity

    auto& textureCache = mTextureCache;
    for (auto it = textureCache.begin(); it != textureCache.end();) {
        const size_t ageDiff = age - it->second.age;
        if (ageDiff >= CACHE_MAX_AGE) {
            it = purge(it);
            if (mCacheSize < CACHE_CAPACITY) {
                // if we're not at capacity, only purge a single entry per gc, trying to
                // avoid a burst of work.
                break;
            }
        } else {
            ++it;
        }
    }

    if (UTILS_UNLIKELY(mCacheSize >= CACHE_CAPACITY)) {
        // make a copy of our CacheContainer to a vector
        using Vector = FixedCapacityVector<std::pair<TextureKey, TextureCachePayload>>;
        auto cache = Vector::with_capacity(textureCache.size());
        std::copy(textureCache.begin(), textureCache.end(), std::back_insert_iterator<Vector>(cache));

        // sort by least recently used
        std::sort(cache.begin(), cache.end(), [](auto const& lhs, auto const& rhs) {
            return lhs.second.age < rhs.second.age;
        });

        // now remove entries until we're at capacity
        auto curr = cache.begin();
        while (mCacheSize >= CACHE_CAPACITY) {
            // by construction this entry must exist
            purge(textureCache.find(curr->first));
            ++curr;
        }

        // Since we're sorted already, reset the oldestAge of the whole system
        size_t oldestAge = cache.front().second.age;
        for (auto& it : textureCache) {
            it.second.age -= oldestAge;
        }
        mAge -= oldestAge;
    }
    //if (mAge % 60 == 0) dump();
}

UTILS_NOINLINE
void ResourceAllocator::dump(bool brief) const noexcept {
    slog.d << "# entries=" << mTextureCache.size() << ", sz=" << mCacheSize / float(1u << 20u)
           << " MiB" << io::endl;
    if (!brief) {
        for (auto const& it : mTextureCache) {
            auto w = it.first.width;
            auto h = it.first.height;
            auto f = FTexture::getFormatSize(it.first.format);
            slog.d << it.first.name << ": w=" << w << ", h=" << h << ", f=" << f << ", sz="
                   << it.second.size / float(1u << 20u) << io::endl;
        }
    }
}

ResourceAllocator::CacheContainer::iterator ResourceAllocator::purge(
        ResourceAllocator::CacheContainer::iterator const& pos) {
    //slog.d << "purging " << pos->second.handle.getId() << ", age=" << pos->second.age << io::endl;
    mBackend.destroyTexture(pos->second.handle);
    mCacheSize -= pos->second.size;
    return mTextureCache.erase(pos);
}

} // namespace filament
