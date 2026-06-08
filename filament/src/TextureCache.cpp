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

#include "TextureCache.h"

#include "details/Texture.h"

#include <filament/Engine.h>

#include <private/backend/DriverApi.h>

#include <backend/DriverApiForward.h>
#include <backend/DriverEnums.h>
#include <backend/Handle.h>
#include <backend/TargetBufferInfo.h>

#include <utils/algorithm.h>
#include <utils/bitset.h>
#include <utils/compiler.h>
#include <utils/debug.h>
#include <utils/ImmutableCString.h>
#include <utils/Logger.h>
#include <utils/ostream.h>
#include <utils/StaticString.h>

#include <algorithm>
#include <array>
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
TextureCache::AssociativeContainer<K, V, H>::AssociativeContainer() {
    mContainer.reserve(128);
}

template<typename K, typename V, typename H>
UTILS_NOINLINE
TextureCache::AssociativeContainer<K, V, H>::~AssociativeContainer() noexcept = default;

template<typename K, typename V, typename H>
UTILS_NOINLINE
typename TextureCache::AssociativeContainer<K, V, H>::iterator
TextureCache::AssociativeContainer<K, V, H>::erase(iterator it) {
    return mContainer.erase(it);
}

template<typename K, typename V, typename H>
typename TextureCache::AssociativeContainer<K, V, H>::const_iterator
TextureCache::AssociativeContainer<K, V, H>::find(key_type const& key) const {
    return const_cast<AssociativeContainer*>(this)->find(key);
}

template<typename K, typename V, typename H>
UTILS_NOINLINE
typename TextureCache::AssociativeContainer<K, V, H>::iterator
TextureCache::AssociativeContainer<K, V, H>::find(key_type const& key) {
    return std::find_if(mContainer.begin(), mContainer.end(), [&key](auto const& v) {
        return v.first == key;
    });
}

template<typename K, typename V, typename H>
template<typename... ARGS>
UTILS_NOINLINE
void TextureCache::AssociativeContainer<K, V, H>::emplace(ARGS&& ... args) {
    mContainer.emplace_back(std::forward<ARGS>(args)...);
}

// ------------------------------------------------------------------------------------------------

TextureCacheInterface::~TextureCacheInterface() = default;

// ------------------------------------------------------------------------------------------------

TextureCacheDisposerInterface::~TextureCacheDisposerInterface() = default;

// ------------------------------------------------------------------------------------------------

size_t TextureCache::TextureKey::getSize() const noexcept {
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

TextureCache::TextureCache(Engine::Config const& config, DriverApi& driverApi) noexcept
        : mCacheMaxAge(config.resourceAllocatorCacheMaxAge),
          mBackend(driverApi),
          mDisposer(std::make_shared<TextureCacheDisposer>(driverApi)) {
#if FILAMENT_TEXTURE_CACHE_DEBUG
    mDebugger = std::make_unique<Debugger>();
#endif
}

TextureCache::TextureCache(std::shared_ptr<TextureCacheDisposer> disposer,
        Engine::Config const& config, DriverApi& driverApi) noexcept
        : mCacheMaxAge(config.resourceAllocatorCacheMaxAge),
          mBackend(driverApi),
          mDisposer(std::move(disposer)) {
#if FILAMENT_TEXTURE_CACHE_DEBUG
    mDebugger = std::make_unique<Debugger>();
#endif
}

TextureCache::~TextureCache() noexcept {
    mDisposer->removeTextureCache(this);
    assert_invariant(mTextureCache.empty());
}

void TextureCache::terminate() noexcept {
    auto& textureCache = mTextureCache;
    for (auto it = textureCache.begin(); it != textureCache.end();) {
        mBackend.destroyTexture(it->second.handle);
        it = textureCache.erase(it);
    }
}

RenderTargetHandle TextureCache::createRenderTarget(StaticString const name,
        TargetBufferFlags const targetBufferFlags, uint32_t const width, uint32_t const height,
        uint8_t const samples, uint8_t const layerCount, MRT const color, TargetBufferInfo const depth,
        TargetBufferInfo const stencil) noexcept {
    auto handle = mBackend.createRenderTarget(targetBufferFlags,
            width, height, samples ? samples : 1u, layerCount, color, depth, stencil, name);
    return handle;
}

void TextureCache::destroyRenderTarget(RenderTargetHandle const h) noexcept {
    mBackend.destroyRenderTarget(h);
}

TextureHandle TextureCache::createTexture(StaticString const name,
        SamplerType const target, uint8_t const levels, TextureFormat const format, uint8_t samples,
        uint32_t const width, uint32_t const height, uint32_t const depth,
        std::array<TextureSwizzle, 4> const swizzle,
        TextureUsage const usage) noexcept {
    // The frame graph descriptor uses "0" to mean "auto" but the sample count that is passed to the
    // backend should always be 1 or greater.
    samples = samples ? samples : uint8_t(1);

    using TS = TextureSwizzle;
    constexpr auto defaultSwizzle = std::array{
        TS::CHANNEL_0, TS::CHANNEL_1, TS::CHANNEL_2, TS::CHANNEL_3};

    // do we have a suitable texture in the cache?
    TextureHandle handle;
    TextureKey const key{ name, target, levels, format, samples, width, height, depth, usage, swizzle };
    if constexpr (mEnabled) {
        auto& textureCache = mTextureCache;
        auto const it = textureCache.find(key);
        if (UTILS_LIKELY(it != textureCache.end())) {
            // we do, move the entry to the in-use list, and remove from the cache
            handle = it->second.handle;
            mCacheSize -= it->second.size;
            textureCache.erase(it);
        } else {
#if FILAMENT_TEXTURE_CACHE_DEBUG
            StaticString evictedName;
            EvictionReason reason;
            if (mDebugger && mDebugger->checkRecentEviction(key, evictedName, reason)) {
                const char* reasonStr = "";
                switch (reason) {
                    case EvictionReason::SKIPPED_FRAME:
                        reasonStr = "it was evicted when a frame was skipped";
                        break;
                    case EvictionReason::AGED_OUT:
                        reasonStr = "it aged out (unused for too many frames)";
                        break;
                    case EvictionReason::UNIQUE_AGE_LIMIT:
                        reasonStr = "the cache exceeded the unique age groups limit (capacity constraint)";
                        break;
                }
                LOG(INFO) << "TextureCache: texture '" << name.c_str()
                          << "' (w=" << width << ", h=" << height << ", format=" << (int)format
                          << ") created because compatible texture '" << evictedName.c_str()
                          << "' was recently evicted from cache because " << reasonStr << ".";
            }
#endif
            // we don't, allocate a new texture and populate the in-use list
            handle = mBackend.createTexture(
                    target, levels, format, samples, width, height, depth, usage, name);
            if (swizzle != defaultSwizzle) {
                TextureHandle const swizzledHandle = mBackend.createTextureViewSwizzle(
                        handle, swizzle[0], swizzle[1], swizzle[2], swizzle[3], name);
                mBackend.destroyTexture(handle);
                handle = swizzledHandle;
            }
        }
    } else {
        handle = mBackend.createTexture(
                target, levels, format, samples, width, height, depth, usage, name);
        if (swizzle != defaultSwizzle) {
            TextureHandle swizzledHandle = mBackend.createTextureViewSwizzle(
                    handle, swizzle[0], swizzle[1], swizzle[2], swizzle[3], name);
            mBackend.destroyTexture(handle);
            handle = swizzledHandle;
        }
    }
    mDisposer->checkout(this, handle, key);
    return handle;
}

void TextureCache::destroyTexture(TextureHandle const h) noexcept {
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

void TextureCache::gc(bool const skippedFrame) noexcept {
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
            auto const reason = (ageDiff >= MAX_AGE_SKIPPED_FRAME && skippedFrame)
                    ? EvictionReason::SKIPPED_FRAME
                    : EvictionReason::AGED_OUT;
            it = purge(it, reason);
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
                it = purge(it, EvictionReason::UNIQUE_AGE_LIMIT);
            } else {
                ++it;
            }
        }
    }

#if FILAMENT_TEXTURE_CACHE_DEBUG
    if (mDebugger) {
        mDebugger->ageRecentEvictions(mAge);
    }
#endif
}

UTILS_NOINLINE
void TextureCache::dump(bool const brief) const noexcept {
    constexpr float MiB = 1.0f / float(1u << 20u);
    DLOG(INFO) << "# entries=" << mTextureCache.size() << ", sz=" << float(mCacheSize) * MiB
               << " MiB"
               << ", max=" << float(mCacheSizeHiWaterMark) * MiB << " MiB";
    if (!brief) {
        for (auto const& it : mTextureCache) {
            auto w = it.first.width;
            auto h = it.first.height;
            auto f = FTexture::getFormatSize(it.first.format);
            DLOG(INFO) << it.first.name.c_str() << ": w=" << w << ", h=" << h << ", f=" << f
                       << ", sz=" << float(it.second.size) * MiB;
        }
    }
}

TextureCache::CacheContainer::iterator
TextureCache::purge(
        CacheContainer::iterator const& pos, EvictionReason reason) {
    // DLOG(INFO) << "purging " << pos->second.handle.getId() << ", age=" << pos->second.age;
    mBackend.destroyTexture(pos->second.handle);
    mCacheSize -= pos->second.size;
#if FILAMENT_TEXTURE_CACHE_DEBUG
    if (mDebugger) {
        mDebugger->recordEviction(pos->first, reason, mAge);
    }
#endif
    return mTextureCache.erase(pos);
}

// ------------------------------------------------------------------------------------------------

TextureCacheDisposer::TextureCacheDisposer(DriverApi& driverApi) noexcept
        : mBackend(driverApi) {
}

TextureCacheDisposer::~TextureCacheDisposer() noexcept {
     assert_invariant(mInUseTextures.empty());
}

void TextureCacheDisposer::terminate() noexcept {
    assert_invariant(mInUseTextures.empty());
}

void TextureCacheDisposer::destroy(TextureHandle const handle) noexcept {
    if (handle) {
        auto it = mInUseTextures.find(handle);
        if (it != mInUseTextures.end()) {
            if (it->second.cache) {
                it->second.cache->destroyTexture(handle);
            } else {
                mBackend.destroyTexture(handle);
                mInUseTextures.erase(it);
            }
        } else {
            mBackend.destroyTexture(handle);
        }
    }
}

void TextureCacheDisposer::checkout(TextureCacheInterface* cache, TextureHandle handle, TextureCache::TextureKey key) {
    mInUseTextures.emplace(handle, InUseRecord{key, cache});
}

std::optional<TextureCache::TextureKey> TextureCacheDisposer::checkin(TextureHandle handle) {
    // find the texture in the in-use list (it must be there!)
    auto const it = mInUseTextures.find(handle);
    assert_invariant(it != mInUseTextures.end());
    if (it == mInUseTextures.end()) {
        return std::nullopt;
    }
    TextureKey const key = it->second.key;
    // remove it from the in-use list
    mInUseTextures.erase(it);
    return key;
}

void TextureCacheDisposer::removeTextureCache(TextureCacheInterface* cache) noexcept {
    for (auto& entry : mInUseTextures) {
        if (entry.second.cache == cache) {
            entry.second.cache = nullptr;
        }
    }
}

void TextureCache::Debugger::recordEviction(TextureKey const& key,
        EvictionReason const reason, size_t const age) noexcept {
    auto const it = std::find_if(mRecentEvictions.begin(), mRecentEvictions.end(), [&key](auto const& entry) {
        return entry.key == key;
    });
    if (it != mRecentEvictions.end()) {
        it->evictionAge = age;
        it->reason = reason;
        return;
    }
    mRecentEvictions.push_back({key, age, reason});
    if (mRecentEvictions.size() > mConfig.historyLimit) {
        mRecentEvictions.erase(mRecentEvictions.begin());
    }
}

bool TextureCache::Debugger::checkRecentEviction(TextureKey const& key,
        StaticString& evictedName, EvictionReason& outReason) noexcept {
    auto const it = std::ranges::find_if(mRecentEvictions, [&key](auto const& entry) {
        return entry.key == key;
    });
    if (it != mRecentEvictions.end()) {
        evictedName = it->key.name;
        outReason = it->reason;
        return true;
    }

    // Check for a partial match (same dimensions and format) to help debug parameter mismatch
    for (auto const& entry : mRecentEvictions) {
        if (entry.key.width == key.width &&
            entry.key.height == key.height &&
            entry.key.format == key.format) {
            LOG(INFO) << "TextureCache partial match: requested '" << key.name.c_str()
                      << "' matches evicted '" << entry.key.name.c_str()
                      << "' in size/format, but differs on: "
                      << (entry.key.target != key.target ? "target " : "")
                      << (entry.key.levels != key.levels ? "levels " : "")
                      << (entry.key.samples != key.samples ? "samples " : "")
                      << (entry.key.usage != key.usage ? "usage " : "")
                      << (entry.key.swizzle != key.swizzle ? "swizzle " : "");
        }
    }
    return false;
}

void TextureCache::Debugger::ageRecentEvictions(size_t const age) noexcept {
    for (auto it = mRecentEvictions.begin(); it != mRecentEvictions.end();) {
        if (age - it->evictionAge > mConfig.recentEvictionThreshold) {
            it = mRecentEvictions.erase(it);
        } else {
            ++it;
        }
    }
}

} // namespace filament
