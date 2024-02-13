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

#include <backend/DriverEnums.h>
#include <backend/Handle.h>

#include <private/backend/DriverApi.h>

#include <utils/Allocator.h>

#include <tsl/robin_map.h>

#include <functional>
#include <set>

#include <stddef.h>
#include <stdint.h>

namespace filament {

class FEngine;

class HwRenderPrimitiveFactory {
public:

    HwRenderPrimitiveFactory();
    ~HwRenderPrimitiveFactory() noexcept;

    HwRenderPrimitiveFactory(HwRenderPrimitiveFactory const& rhs) = delete;
    HwRenderPrimitiveFactory(HwRenderPrimitiveFactory&& rhs) noexcept = delete;
    HwRenderPrimitiveFactory& operator=(HwRenderPrimitiveFactory const& rhs) = delete;
    HwRenderPrimitiveFactory& operator=(HwRenderPrimitiveFactory&& rhs) noexcept = delete;

    void terminate(backend::DriverApi& driver) noexcept;

    backend::RenderPrimitiveHandle create(backend::DriverApi& driver,
            backend::VertexBufferHandle vbh,
            backend::IndexBufferHandle ibh,
            backend::PrimitiveType type) noexcept;

    void destroy(backend::DriverApi& driver,
            backend::RenderPrimitiveHandle rph) noexcept;

private:
    struct Key { // 20 bytes
        backend::VertexBufferHandle vbh;            // 4
        backend::IndexBufferHandle ibh;             // 4
        backend::PrimitiveType type;                // 4
    };

    struct Entry { // 28 bytes
        Key key;                                    // 20
        backend::RenderPrimitiveHandle handle;      //  4
        mutable uint32_t refs;                      //  4
    };


    // Size of the arena used for the "set" part of the bimap
    static constexpr size_t SET_ARENA_SIZE = 4 * 1024 * 1024;

    // Arena for the set<>, using a pool allocator inside a heap area.
    using Arena = utils::Arena<
            utils::PoolAllocator<64>,   // this seems to work with clang and msvc
            utils::LockingPolicy::NoLock,
#ifndef NDEBUG
            utils::TrackingPolicy::HighWatermark,
#else
            utils::TrackingPolicy::Untracked,
#endif
            utils::AreaPolicy::HeapArea>;

    using Set = std::set<
            Entry,
            std::less<>,
            utils::STLAllocator<Entry, Arena>>;

    using Map = tsl::robin_map<
            backend::RenderPrimitiveHandle::HandleId,
            Set::const_iterator>;

    // Arena where the set memory is allocated
    Arena mArena;

    // set of HwRenderPrimitive data
    Set mSet;

    // map of RenderPrimitiveHandle to Set Entry
    Map mMap;

    friend bool operator<(Key const& lhs, Key const& rhs) noexcept;
    friend bool operator<(Key const& lhs, Entry const& rhs) noexcept;
    friend bool operator<(Entry const& lhs, Key const& rhs) noexcept;
    friend bool operator<(Entry const& lhs, Entry const& rhs) noexcept;
};

} // namespace filament

#endif // TNT_FILAMENT_HWRENDERPRIMITIVEFACTORY_H
