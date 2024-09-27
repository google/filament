/*
 * Copyright (C) 2022 The Android Open Source Project
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

#ifndef TNT_FILAMENT_HWRENDERPRIMITIVEFACTORY_H
#define TNT_FILAMENT_HWRENDERPRIMITIVEFACTORY_H

#include "Bimap.h"

#include <backend/DriverApiForward.h>
#include <backend/DriverEnums.h>
#include <backend/Handle.h>

#include <utils/Allocator.h>

#include <functional>

#include <stddef.h>
#include <stdint.h>

namespace filament {

class FEngine;

class HwRenderPrimitiveFactory {
public:
    using Handle = backend::RenderPrimitiveHandle;

    HwRenderPrimitiveFactory();
    ~HwRenderPrimitiveFactory() noexcept;

    HwRenderPrimitiveFactory(HwRenderPrimitiveFactory const& rhs) = delete;
    HwRenderPrimitiveFactory(HwRenderPrimitiveFactory&& rhs) noexcept = delete;
    HwRenderPrimitiveFactory& operator=(HwRenderPrimitiveFactory const& rhs) = delete;
    HwRenderPrimitiveFactory& operator=(HwRenderPrimitiveFactory&& rhs) noexcept = delete;

    void terminate(backend::DriverApi& driver) noexcept;

    struct Parameters { // 12 bytes
        backend::VertexBufferHandle vbh;            // 4
        backend::IndexBufferHandle ibh;             // 4
        backend::PrimitiveType type;                // 4
        size_t hash() const noexcept;
    };

    friend bool operator==(Parameters const& lhs, Parameters const& rhs) noexcept;

    Handle create(backend::DriverApi& driver,
            backend::VertexBufferHandle vbh,
            backend::IndexBufferHandle ibh,
            backend::PrimitiveType type) noexcept;

    void destroy(backend::DriverApi& driver, Handle handle) noexcept;

private:
    struct Key { // 16 bytes
        // The key should not be copyable, unfortunately due to how the Bimap works we have
        // to copy-construct it once.
        Key(Key const&) = default;
        Key& operator=(Key const&) = delete;
        Key& operator=(Key&&) noexcept = delete;
        explicit Key(Parameters const& params) : params(params), refs(1) { }
        Parameters params;
        mutable uint32_t refs;  // 4 bytes
        bool operator==(Key const& rhs) const noexcept {
            return params == rhs.params;
        }
    };

    struct KeyHasher {
        size_t operator()(Key const& p) const noexcept {
            return p.params.hash();
        }
    };

    struct Value { // 4 bytes
        Handle handle;
    };

    struct ValueHasher {
        size_t operator()(Value const v) const noexcept {
            return std::hash<Handle::HandleId>()(v.handle.getId());
        }
    };

    friend bool operator==(Value const lhs, Value const rhs) noexcept {
        return lhs.handle == rhs.handle;
    }

    // Size of the arena used for the "set" part of the bimap
    // about ~65K entry before fall back to heap
    static constexpr size_t SET_ARENA_SIZE = 1 * 1024 * 1024;

    // Arena for the set<>, using a pool allocator inside a heap area.
    using PoolAllocatorArena = utils::Arena<
            utils::PoolAllocatorWithFallback<sizeof(Key)>,
            utils::LockingPolicy::NoLock,
            utils::TrackingPolicy::Untracked,
            utils::AreaPolicy::HeapArea>;


    // Arena where the set memory is allocated
    PoolAllocatorArena mArena;

    // The special Bimap
    Bimap<Key, Value, KeyHasher, ValueHasher,
            utils::STLAllocator<Key, PoolAllocatorArena>> mBimap;
};

} // namespace filament

#endif // TNT_FILAMENT_HWRENDERPRIMITIVEFACTORY_H
