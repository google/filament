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

#ifndef TNT_FILAMENT_FG_RESOURCEALLOCATOR_H
#define TNT_FILAMENT_FG_RESOURCEALLOCATOR_H

#include <backend/DriverEnums.h>
#include <backend/Handle.h>
#include <backend/TargetBufferInfo.h>

#include "private/backend/DriverApiForward.h"

#include <utils/Hash.h>

#include <unordered_map>

#include <stdint.h>
#include <tsl/robin_map.h>

namespace filament {
namespace fg {

class ResourceAllocator {
public:
    explicit ResourceAllocator(backend::DriverApi& driverApi) noexcept;
    ~ResourceAllocator() noexcept;

    void terminate() noexcept;

    backend::RenderTargetHandle createRenderTarget(
            backend::TargetBufferFlags targetBufferFlags,
            uint32_t width,
            uint32_t height,
            uint8_t samples,
            backend::TargetBufferInfo color,
            backend::TargetBufferInfo depth,
            backend::TargetBufferInfo stencil) noexcept;

    void destroyRenderTarget(backend::RenderTargetHandle h) noexcept;

    backend::TextureHandle createTexture(const char* name, backend::SamplerType target,
            uint8_t levels,
            backend::TextureFormat format, uint8_t samples, uint32_t width, uint32_t height,
            uint32_t depth, backend::TextureUsage usage) noexcept;

    void destroyTexture(backend::TextureHandle h) noexcept;

    void gc() noexcept;

private:
    // TODO: these should be settings of the engine
    static constexpr size_t CACHE_CAPACITY = 64u << 20u;   // 64 MiB
    static constexpr size_t CACHE_MAX_AGE  = 30u;          // 64 MiB

    struct TextureKey {
        const char* name; // doesn't participate in the hash
        backend::SamplerType target;
        uint8_t levels;
        backend::TextureFormat format;
        uint8_t samples;
        uint32_t width;
        uint32_t height;
        uint32_t depth;
        backend::TextureUsage usage;

        size_t getSize() const noexcept;

        bool operator==(const TextureKey& other) const noexcept {
            return target == other.target &&
                   levels == other.levels &&
                   format == other.format &&
                   samples == other.samples &&
                   width == other.width &&
                   height == other.height &&
                   depth == other.depth &&
                   usage == other.usage;
        }

        friend size_t hash_value(TextureKey const& k) {
            size_t seed = 0;
            utils::hash::hash_combine(seed, k.target);
            utils::hash::hash_combine(seed, k.levels);
            utils::hash::hash_combine(seed, k.format);
            utils::hash::hash_combine(seed, k.samples);
            utils::hash::hash_combine(seed, k.width);
            utils::hash::hash_combine(seed, k.height);
            utils::hash::hash_combine(seed, k.depth);
            utils::hash::hash_combine(seed, k.usage);
            return seed;
        }
    };

    struct TextureCachePayload {
        backend::TextureHandle handle;
        size_t age = 0;
        uint32_t size = 0;
    };

    template<typename T>
    struct Hasher {
        std::size_t operator()(T const& s) const noexcept {
            return hash_value(s);
        }
    };

    template<typename T>
    struct Hasher<backend::Handle<T>> {
        std::size_t operator()(backend::Handle<T> const& s) const noexcept {
            std::hash<typename backend::Handle<T>::HandleId> hash{};
            return hash(s.getId());
        }
    };

    void dump() const noexcept;

    backend::DriverApi& mBackend;
    std::unordered_multimap<TextureKey, TextureCachePayload, Hasher<TextureKey>> mTextureCache;
    std::unordered_multimap<backend::TextureHandle, TextureKey, Hasher<backend::TextureHandle>> mInUseTextures;
    size_t mAge = 0;
    uint32_t mCacheSize = 0;
    const bool mEnabled = true;
};

}// namespace fg
} // namespace filament


#endif //TNT_FILAMENT_FG_RESOURCEALLOCATOR_H
