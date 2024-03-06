/*
 * Copyright (C) 2024 The Android Open Source Project
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

#ifndef TNT_FILAMENT_HWVERTEXBUFFERINFOFACTORY_H
#define TNT_FILAMENT_HWVERTEXBUFFERINFOFACTORY_H

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

class HwVertexBufferInfoFactory {
public:
    using Handle = backend::VertexBufferInfoHandle;

    HwVertexBufferInfoFactory();
    ~HwVertexBufferInfoFactory() noexcept;

    HwVertexBufferInfoFactory(HwVertexBufferInfoFactory const& rhs) = delete;
    HwVertexBufferInfoFactory(HwVertexBufferInfoFactory&& rhs) noexcept = delete;
    HwVertexBufferInfoFactory& operator=(HwVertexBufferInfoFactory const& rhs) = delete;
    HwVertexBufferInfoFactory& operator=(HwVertexBufferInfoFactory&& rhs) noexcept = delete;

    void terminate(backend::DriverApi& driver) noexcept;

    struct Parameters { // 136 bytes
        uint8_t bufferCount;
        uint8_t attributeCount;
        uint8_t padding[2] = {};
        backend::AttributeArray attributes;
        size_t hash() const noexcept;
    };

    friend bool operator==(Parameters const& lhs, Parameters const& rhs) noexcept;

    Handle create(backend::DriverApi& driver,
            uint8_t bufferCount,
            uint8_t attributeCount,
            backend::AttributeArray attributes) noexcept;

    void destroy(backend::DriverApi& driver, Handle handle) noexcept;

private:
    struct Key { // 140 bytes
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

    struct Value {
        Handle handle;
    };

    struct ValueHasher {
        size_t operator()(Value v) const noexcept {
            std::hash<Handle::HandleId> const hasher;
            return hasher(v.handle.getId());
        }
    };

    friend bool operator==(Value const& lhs, Value const& rhs) noexcept {
        return lhs.handle == rhs.handle;
    }

    // Size of the arena used for the "set" part of the bimap
    static constexpr size_t SET_ARENA_SIZE = 4 * 1024 * 1024;

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

#endif // TNT_FILAMENT_HWVERTEXBUFFERINFOFACTORY_H
