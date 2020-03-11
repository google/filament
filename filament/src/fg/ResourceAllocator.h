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

#include <vector>

#include <stdint.h>

namespace filament {
namespace fg {

// The only reason we use an interface here is for unit-tests, so we can mock this allocator.
// This is not too time-critical, so that's okay.
class ResourceAllocatorInterface {
public:
    virtual backend::RenderTargetHandle createRenderTarget(const char* name,
            backend::TargetBufferFlags targetBufferFlags,
            uint32_t width,
            uint32_t height,
            uint8_t samples,
            backend::MRT color,
            backend::TargetBufferInfo depth,
            backend::TargetBufferInfo stencil) noexcept = 0;

    virtual void destroyRenderTarget(backend::RenderTargetHandle h) noexcept = 0;

    virtual backend::TextureHandle createTexture(const char* name, backend::SamplerType target,
            uint8_t levels,
            backend::TextureFormat format, uint8_t samples, uint32_t width, uint32_t height,
            uint32_t depth, backend::TextureUsage usage) noexcept = 0;

    virtual void destroyTexture(backend::TextureHandle h) noexcept = 0;

protected:
    virtual ~ResourceAllocatorInterface();
};


class ResourceAllocator final : public ResourceAllocatorInterface {
public:
    explicit ResourceAllocator(backend::DriverApi& driverApi) noexcept;
    ~ResourceAllocator() noexcept override;

    void terminate() noexcept;

    backend::RenderTargetHandle createRenderTarget(const char* name,
            backend::TargetBufferFlags targetBufferFlags,
            uint32_t width,
            uint32_t height,
            uint8_t samples,
            backend::MRT color,
            backend::TargetBufferInfo depth,
            backend::TargetBufferInfo stencil) noexcept override;

    void destroyRenderTarget(backend::RenderTargetHandle h) noexcept override;

    backend::TextureHandle createTexture(const char* name, backend::SamplerType target,
            uint8_t levels,
            backend::TextureFormat format, uint8_t samples, uint32_t width, uint32_t height,
            uint32_t depth, backend::TextureUsage usage) noexcept override;

    void destroyTexture(backend::TextureHandle h) noexcept override;

    void gc() noexcept;

private:
    // TODO: these should be settings of the engine
    static constexpr size_t CACHE_CAPACITY = 64u << 20u;   // 64 MiB
    static constexpr size_t CACHE_MAX_AGE  = 30u;

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
            utils::hash::combine_fast(seed, k.target);
            utils::hash::combine_fast(seed, k.levels);
            utils::hash::combine_fast(seed, k.format);
            utils::hash::combine_fast(seed, k.samples);
            utils::hash::combine_fast(seed, k.width);
            utils::hash::combine_fast(seed, k.height);
            utils::hash::combine_fast(seed, k.depth);
            utils::hash::combine_fast(seed, k.usage);
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

    inline void dump() const noexcept;

    template<typename Key, typename Value, typename Hasher = Hasher<Key>>
    class AssociativeContainer {
        // We use a std::vector instead of a std::multimap because we don't expect many items
        // in the cache and std::multimap generates tons of code. Even with more items, we
        // could improve this trivially by using a sorted std::vector.
        using Container = std::vector<std::pair<Key, Value>>;
        Container mContainer;
    public:
        using iterator = typename Container::iterator;
        using const_iterator = typename Container::const_iterator;
        using key_type = typename Container::value_type::first_type;
        using value_type = typename Container::value_type::second_type;

        size_t size() const { return mContainer.size(); }
        iterator begin() { return mContainer.begin(); }
        const_iterator begin() const { return mContainer.begin(); }
        iterator end() { return mContainer.end(); }
        const_iterator end() const  { return mContainer.end(); }
        iterator erase(iterator it);
        const_iterator find(key_type const& key) const;
        iterator find(key_type const& key);
        template<typename ... ARGS>
        void emplace(ARGS&&... args);
    };

    backend::DriverApi& mBackend;
    AssociativeContainer<TextureKey, TextureCachePayload> mTextureCache;
    AssociativeContainer<backend::TextureHandle, TextureKey> mInUseTextures;
    size_t mAge = 0;
    uint32_t mCacheSize = 0;
    const bool mEnabled = true;
};

}// namespace fg
} // namespace filament


#endif //TNT_FILAMENT_FG_RESOURCEALLOCATOR_H
