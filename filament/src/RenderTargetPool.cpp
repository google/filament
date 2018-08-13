/*
 * Copyright (C) 2018 The Android Open Source Project
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

#include "RenderTargetPool.h"

#include "details/Engine.h"
#include "details/Texture.h"

#include <utils/Log.h>

namespace filament {

using namespace utils;
using namespace driver;
using namespace details;

void RenderTargetPool::init(FEngine& engine) noexcept {
    mEngine = &engine;
    mPool.reserve(16);
}

void RenderTargetPool::terminate(DriverApi& driver) noexcept {
    for (Entry const* entry : mPool) {
        destroyEntry(driver, entry);
    }
}

RenderTargetPool::Target const* RenderTargetPool::get(
        driver::TargetBufferFlags attachments,
        uint32_t w, uint32_t h, uint8_t samples, TextureFormat format,
        uint8_t flags) const noexcept {

    // samples can't be less than 1
    samples = std::max(uint8_t(1), samples);

    // round all allocations to 32x32 pixels, to avoid too many small resize
    uint32_t target_w = (w + 31u) & ~31u;
    uint32_t target_h = (h + 31u) & ~31u;
    Entry entry = { attachments, target_w, target_h, samples, format, flags };

    FEngine& engine = *mEngine;
    DriverApi& driver = engine.getDriverApi();
    
    // Since we order the cache by width then by height, we can get a cached
    // entry with a smaller height. Only reuse if both dimensions are higher.
    auto pos = find(&entry);
    Entry const* const it = pos != mPool.end() ? *pos : nullptr;
    if (UTILS_LIKELY(it &&
                     it->attachments == attachments &&
                     it->samples == samples &&
                     it->format == format &&
                     it->flags == flags &&
                     it->w >= target_w &&
                     it->h >= target_h)) {
        if (2 * it->w * it->h >= 3 * target_w * target_h) {
            // the surface we found is 1.5x larger than requested
            // there is a performance cost, especially on tilers, it's better to not allow
            // too much of a size difference
            goto not_found;
        }

        // update last usage age, remove the entry from the pool and return it
        (*pos)->age = mCacheAge;
        mPool.erase(pos);
        return it;
    }

    not_found:

    if (flags & RenderTargetPool::Target::NO_TEXTURE) {
        entry.target = driver.createRenderTarget(
                entry.attachments, target_w, target_h, samples, format, {}, {}, {});
    } else {
        entry.texture = driver.createTexture(Driver::SamplerType::SAMPLER_2D, 1,
                format, samples, target_w, target_h, 1, Driver::TextureUsage::COLOR_ATTACHMENT);

        entry.target = driver.createRenderTarget(
                entry.attachments, target_w, target_h, samples, format,
                { entry.texture }, {}, {});
    }

    // update last used age
    entry.age = mCacheAge;

    mPoolSize += getSize(&entry);

    // entry not found, create one
    return mEntryArena.make<Entry>(entry);
}

void RenderTargetPool::put(Target const* target) noexcept {
    // insert the entry back into the pool
    Entry const* entry = static_cast<Entry const*>(target);
    auto pos = find(entry);
    mPool.insert(pos, entry);
}

std::vector<RenderTargetPool::Entry const*>::iterator
RenderTargetPool::find(Entry const* entry) const noexcept {
    auto& cache = mPool;
    auto pos = std::lower_bound(cache.begin(), cache.end(), entry,
            [](Entry const* const& l, Entry const* const& r) {
                Entry const& lhs = *l;
                Entry const& rhs = *r;
                if (lhs.attachments == rhs.attachments) {
                    if (lhs.samples == rhs.samples) {
                        if (lhs.format == rhs.format) {
                            if (lhs.flags == rhs.flags) {
                                if (lhs.w == rhs.w) {
                                    return lhs.h < rhs.h;
                                }
                                return lhs.w < rhs.w;
                            }
                            return lhs.flags < rhs.flags;
                        }
                        return lhs.format < rhs.format;
                    }
                    return lhs.samples < rhs.samples;
                }
                return lhs.attachments < rhs.attachments;
            });
    return pos;
}

void RenderTargetPool::gc() noexcept {
//#ifndef NDEBUG
//    slog.d << "RenderTargetPool: " << mPoolSize/1024.0f << "KiB, count=" << mPool.size() << io::endl;
//#endif

    DriverApi& driver = mEngine->getDriverApi();
    auto& cache = mPool;
    size_t count = cache.size();
    while (count && (count > POOL_MAX_ENTRY_COUNT || mPoolSize > POOL_MAX_SIZE)) {

        // find the least recently used entry (linear search here)
        auto pos = std::min_element(cache.begin(), cache.end(),
                [](const Entry* rhs, const Entry* lhs) { return rhs->age < lhs->age; });

        // don't remove entries that were used in the last frame
        if ((*pos)->age == mCacheAge) {
            // no more entries old enough to remove
            break;
        }

        // free resources
        destroyEntry(driver, *pos);
        // lastly, remove entry from cache
        cache.erase(pos);
        count--;
    }

    if (UTILS_UNLIKELY(mDeepPurgeCountDown-- == 0)) {
        mDeepPurgeCountDown = POOL_ENTRY_MAX_AGE;
        uint32_t age = mCacheAge - POOL_ENTRY_MAX_AGE;
        // remove all entries that are older than CACHE_ENTRY_MAX_AGE
        auto last = std::remove_if(cache.begin(), cache.end(),
                [this, &driver, age](const Entry* entry) {
                    bool remove = entry->age <= age;
                    if (remove) {
                        destroyEntry(driver, entry);
                    }
                    return remove;
                });
        cache.erase(last, cache.end());
    }

    // all cache entries get older
    mCacheAge++;
}

void RenderTargetPool::destroyEntry(DriverApi& driver, Entry const* entry) noexcept {
    assert(entry);
    driver.destroyRenderTarget(entry->target);
    driver.destroyTexture(entry->texture);
    mPoolSize -= getSize(entry);
    mEntryArena.destroy(entry);
    assert(mPoolSize >= 0);
}

size_t RenderTargetPool::getSize(Entry const* entry) noexcept {
    size_t size = 0;
    if (entry->attachments & TargetBufferFlags::COLOR) {
        size += FTexture::getFormatSize(entry->format);
    }

    if (entry->attachments & TargetBufferFlags::DEPTH) {
        size += 3;
    }

    if (entry->attachments & TargetBufferFlags::STENCIL) {
        size += 1;
    }

    return size * entry->samples * entry->w * entry->h;
}


} // namespace filament
