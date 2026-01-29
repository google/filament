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

#ifndef TNT_FILAMENT_RESOURCEALLOCATOR_H
#define TNT_FILAMENT_RESOURCEALLOCATOR_H

#include <filament/Engine.h>

#include <backend/DriverEnums.h>
#include <backend/Handle.h>
#include <backend/TargetBufferInfo.h>

#include "backend/DriverApiForward.h"

#include <utils/StaticString.h>
#include <utils/Hash.h>

#include <array>
#include <memory>
#include <optional>
#include <utility>
#include <vector>

#include <cstddef>
#include <stddef.h>
#include <stdint.h>

namespace filament {

class TextureCacheDisposer;

// The only reason we use an interface here is for unit-tests, so we can mock this allocator.
// This is not too time-critical, so that's okay.

class TextureCacheDisposerInterface {
public:
    virtual void destroy(backend::TextureHandle handle) noexcept = 0;
protected:
    virtual ~TextureCacheDisposerInterface();
};

class TextureCacheInterface {
public:
    virtual backend::RenderTargetHandle createRenderTarget(utils::StaticString name,
            backend::TargetBufferFlags targetBufferFlags,
            uint32_t width,
            uint32_t height,
            uint8_t samples,
            uint8_t layerCount,
            backend::MRT color,
            backend::TargetBufferInfo depth,
            backend::TargetBufferInfo stencil) noexcept = 0;

    virtual void destroyRenderTarget(backend::RenderTargetHandle h) noexcept = 0;

    virtual backend::TextureHandle createTexture(utils::StaticString name, backend::SamplerType target,
            uint8_t levels,backend::TextureFormat format, uint8_t samples,
            uint32_t width, uint32_t height, uint32_t depth,
            std::array<backend::TextureSwizzle, 4> swizzle,
            backend::TextureUsage usage) noexcept = 0;

    virtual void destroyTexture(backend::TextureHandle h) noexcept = 0;

    virtual TextureCacheDisposerInterface& getDisposer() noexcept = 0;

protected:
    virtual ~TextureCacheInterface();
};

class TextureCache final : public TextureCacheInterface {
public:
    explicit TextureCache(std::shared_ptr<TextureCacheDisposer> disposer,
            Engine::Config const& config, backend::DriverApi& driverApi) noexcept;

    explicit TextureCache(
            Engine::Config const& config, backend::DriverApi& driverApi) noexcept;

    ~TextureCache() noexcept override;

    void terminate() noexcept;

    backend::RenderTargetHandle createRenderTarget(utils::StaticString name,
            backend::TargetBufferFlags targetBufferFlags,
            uint32_t width,
            uint32_t height,
            uint8_t samples,
            uint8_t layerCount,
            backend::MRT color,
            backend::TargetBufferInfo depth,
            backend::TargetBufferInfo stencil) noexcept override;

    void destroyRenderTarget(backend::RenderTargetHandle h) noexcept override;

    backend::TextureHandle createTexture(utils::StaticString name, backend::SamplerType target,
            uint8_t levels, backend::TextureFormat format, uint8_t samples,
            uint32_t width, uint32_t height, uint32_t depth,
            std::array<backend::TextureSwizzle, 4> swizzle,
            backend::TextureUsage usage) noexcept override;

    void destroyTexture(backend::TextureHandle h) noexcept override;

    TextureCacheDisposerInterface& getDisposer() noexcept override;

    void gc(bool skippedFrame = false) noexcept;

private:
    size_t const mCacheMaxAge;

    struct TextureKey {
        utils::StaticString name; // doesn't participate in the hash
        backend::SamplerType target;
        uint8_t levels;
        backend::TextureFormat format;
        uint8_t samples;
        uint32_t width;
        uint32_t height;
        uint32_t depth;
        backend::TextureUsage usage;
        std::array<backend::TextureSwizzle, 4> swizzle;

        size_t getSize() const noexcept;

        bool operator==(const TextureKey& other) const noexcept {
            return target == other.target &&
                   levels == other.levels &&
                   format == other.format &&
                   samples == other.samples &&
                   width == other.width &&
                   height == other.height &&
                   depth == other.depth &&
                   usage == other.usage &&
                   swizzle == other.swizzle;
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
            utils::hash::combine_fast(seed, k.swizzle[0]);
            utils::hash::combine_fast(seed, k.swizzle[1]);
            utils::hash::combine_fast(seed, k.swizzle[2]);
            utils::hash::combine_fast(seed, k.swizzle[3]);
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
            return s.getId();
        }
    };

    inline void dump(bool brief = false) const noexcept;

    template<typename Key, typename Value, typename Hasher = Hasher<Key>>
    class AssociativeContainer {
        // We use a std::vector instead of a std::multimap because we don't expect many items
        // in the cache and std::multimap generates tons of code. std::multimap starts getting
        // significantly better around 1000 items.
        using Container = std::vector<std::pair<Key, Value>>;
        Container mContainer;

    public:
        AssociativeContainer();
        ~AssociativeContainer() noexcept;
        using iterator = typename Container::iterator;
        using const_iterator = typename Container::const_iterator;
        using key_type = typename Container::value_type::first_type;
        using value_type = typename Container::value_type::second_type;

        size_t size() const { return mContainer.size(); }
        bool empty() const { return size() == 0; }
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

    using CacheContainer = AssociativeContainer<TextureKey, TextureCachePayload>;

    CacheContainer::iterator
    purge(CacheContainer::iterator const& pos);

    backend::DriverApi& mBackend;
    std::shared_ptr<TextureCacheDisposer> mDisposer;
    CacheContainer mTextureCache;
    size_t mAge = 0;
    uint32_t mCacheSize = 0;
    uint32_t mCacheSizeHiWaterMark = 0;
    static constexpr bool mEnabled = true;

    friend class TextureCacheDisposer;
};

class TextureCacheDisposer final : public TextureCacheDisposerInterface {
    using TextureKey = TextureCache::TextureKey;
public:
    explicit TextureCacheDisposer(backend::DriverApi& driverApi) noexcept;
    ~TextureCacheDisposer() noexcept override;
    void terminate() noexcept;
    void destroy(backend::TextureHandle handle) noexcept override;

private:
    friend class TextureCache;
    void checkout(backend::TextureHandle handle, TextureKey key);
    std::optional<TextureKey> checkin(backend::TextureHandle handle);

    using InUseContainer = TextureCache::AssociativeContainer<backend::TextureHandle, TextureKey>;
    backend::DriverApi& mBackend;
    InUseContainer mInUseTextures;
};

} // namespace filament


#endif //TNT_FILAMENT_RESOURCEALLOCATOR_H
