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

#ifndef TNT_FILAMENT_RENDERTARGETPOOL_H
#define TNT_FILAMENT_RENDERTARGETPOOL_H

#include "driver/DriverApiForward.h"
#include "driver/Driver.h"
#include "driver/Handle.h"

#include <filament/driver/DriverEnums.h>
#include <filament/driver/PixelBufferDescriptor.h>

#include <utils/Allocator.h>

#include <vector>

namespace filament {

namespace details {
class FEngine;
} // namespace details

class RenderTargetPool {
    // entries older than this are purged
    static constexpr uint32_t POOL_ENTRY_MAX_AGE = 60 * 60;     // ~1 min

    // e.g. layer sizes
    // 1440 x 2560 is ~ 29 MB for color buffer
    // 1280 x 720  is ~  7 MB for color buffer
    static constexpr size_t POOL_MAX_SIZE = 128 * 1024 * 1024;

    // 2 pages is way enough for the entry structures (should be about 400)
    static constexpr size_t POOL_ENTRY_ARENA_SIZE = 8192;

public:
    using TextureFormat = Driver::TextureFormat;

    void init(details::FEngine& engine) noexcept;

    void terminate(driver::DriverApi& driver) noexcept;

    struct Target {
        Handle<HwRenderTarget> target;
        Handle<HwTexture> texture;
        uint32_t w = 0;
        uint32_t h = 0;
        driver::TargetBufferFlags attachments = driver::TargetBufferFlags::NONE;
        TextureFormat format = TextureFormat::RGBA8;
        uint8_t samples = 1;
        uint8_t flags = 0;
        static constexpr uint8_t NO_TEXTURE = 0x1;
    };

    Target const* get(driver::TargetBufferFlags attachments,
            uint32_t width, uint32_t height, uint8_t samples, TextureFormat format,
            uint8_t flags = 0) const noexcept;

    void put(Target const* entry) noexcept;

    // remove older items in the cache. call this once per frame.
    void gc() noexcept;

private:
    struct Entry : public Target {
        Entry() = default;
        Entry(driver::TargetBufferFlags attachments,
                uint32_t w, uint32_t h, uint8_t samples, TextureFormat format,
                uint8_t flags) {
            this->attachments = attachments;
            this->w = w;
            this->h = h;
            this->samples = samples;
            this->format = format;
            this->flags = flags;
        }
        mutable uint32_t age = 0;
    };

    // we divide by two, so we have plenty of room
    static constexpr size_t POOL_MAX_ENTRY_COUNT = (POOL_ENTRY_ARENA_SIZE / sizeof(Entry)) / 2;

    static size_t getSize(Entry const* entry) noexcept;
    void destroyEntry(driver::DriverApi& driver, Entry const* entry) noexcept;
    std::vector<Entry const*>::iterator find(Entry const* entry) const noexcept;

    details::FEngine* mEngine = nullptr;
    mutable std::vector<Entry const*> mPool;
    mutable size_t mPoolSize = 0;
    // at 60 fps, 32 bit gives us 828 days without overflow
    uint32_t mDeepPurgeCountDown = POOL_ENTRY_MAX_AGE;
    uint32_t mCacheAge = POOL_ENTRY_MAX_AGE;

    using PoolAllocator = utils::Arena<utils::ObjectPoolAllocator<Entry>, utils::LockingPolicy::NoLock>;
    mutable PoolAllocator mEntryArena = { "PoolAllocator", POOL_ENTRY_ARENA_SIZE };

};

} // namespace filament

#endif // TNT_FILAMENT_RENDERTARGETPOOL_H
