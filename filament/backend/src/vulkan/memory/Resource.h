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

#ifndef TNT_FILAMENT_BACKEND_VULKAN_MEMORY_RESOURCE_H
#define TNT_FILAMENT_BACKEND_VULKAN_MEMORY_RESOURCE_H

#include <private/backend/HandleAllocator.h>
#include <utils/Mutex.h>

#include <atomic>
#include <cstdint>

namespace filament::backend::fvkmemory {

using CounterIndex = int32_t;
using HandleId = HandleBase::HandleId;

class ResourceManager;

template <typename D>
struct resource_ptr;

// Subclasses of VulkanResource must provide this enum in their construction.
enum class ResourceType : uint8_t {
    BUFFER_OBJECT = 0,
    INDEX_BUFFER = 1,
    PROGRAM = 2,
    RENDER_TARGET = 3,
    SWAP_CHAIN = 4,
    RENDER_PRIMITIVE = 5,
    TEXTURE = 6,
    TEXTURE_STATE = 7,
    TIMER_QUERY = 8,
    VERTEX_BUFFER = 9,
    VERTEX_BUFFER_INFO = 10,
    DESCRIPTOR_SET_LAYOUT = 11,
    DESCRIPTOR_SET = 12,
    FENCE = 13,
    VULKAN_BUFFER = 14,
    STAGE_SEGMENT = 15,
    STAGE_IMAGE = 16,
    UNDEFINED_TYPE = 17,    // Must be the last enum because we use it for iterating over the enums.
};

template<typename D>
ResourceType getTypeEnum() noexcept;

std::string getTypeStr(ResourceType type);

inline bool isThreadSafeType(ResourceType type) {
    return type == ResourceType::FENCE || type == ResourceType::TIMER_QUERY;
}

struct Resource {
    Resource()
        : resManager(nullptr),
          id(HandleBase::nullid),
          mCount(0),
          restype(ResourceType::UNDEFINED_TYPE),
          mHandleConsideredDestroyed(false) {}

    template<typename D>
    bool isType() const {
        return getTypeEnum<D>() == restype;
    }

private:
    inline void inc() noexcept {
        mCount++;
    }

    inline void dec() noexcept {
        assert_invariant(mCount > 0);
        if (--mCount == 0) {
            destroy(restype, id);
        }
    }

    // To be able to detect use-after-free, we need a bit to signify if the handle should be
    // consider destroyed (from Filament's perspective).
    inline void setHandleConsiderDestroyed() noexcept {
        mHandleConsideredDestroyed = true;
    }

    inline bool isHandleConsideredDestroyed() const {
        return mHandleConsideredDestroyed;
    }

    template <typename T>
    inline void init(HandleId id, ResourceManager* resManager) {
        this->id = id;
        this->resManager = resManager;
        this->restype = getTypeEnum<T>();
    }

    void destroy(ResourceType type, HandleId id);

    ResourceManager* resManager; // 8
    HandleId id;                 // 4
    uint32_t mCount      : 24;
    ResourceType restype : 7;
    bool mHandleConsideredDestroyed : 1;  // restype + mCount + mHandleConsideredDestroyed
                                          //   is 4 bytes.

    friend class ResourceManager;

    template <typename D>
    friend struct resource_ptr;
};

struct ThreadSafeResource {
    ThreadSafeResource()
        : resManager(nullptr),
          id(HandleBase::nullid),
          mCount(0),
          restype(ResourceType::UNDEFINED_TYPE),
          mHandleConsideredDestroyed(false) {}

private:
    inline void inc() noexcept {
        mCount.fetch_add(1, std::memory_order_relaxed);
    }

    inline void dec() noexcept {
        if (mCount.fetch_sub(1, std::memory_order_acq_rel) == 1) {
            destroy(restype, id);
        }
    }

    // To be able to detect use-after-free, we need a bit to signify if the handle should be
    // consider destroyed (from Filament's perspective).
    inline void setHandleConsiderDestroyed() noexcept {
        mHandleConsideredDestroyed = true;
    }

    inline bool isHandleConsideredDestroyed() const {
        return mHandleConsideredDestroyed;
    }

    template <typename T>
    inline void init(HandleId id, ResourceManager* resManager) {
        this->id = id;
        this->resManager = resManager;
        this->restype = getTypeEnum<T>();
    }

    void destroy(ResourceType type, HandleId id);

    ResourceManager* resManager;  // 8
    HandleId id;                  // 4
    std::atomic<uint32_t> mCount; // 4
    ResourceType restype : 7;
    bool mHandleConsideredDestroyed : 1;  // restype + mHandleConsideredDestroyed is 1 byte

    friend class ResourceManager;

    template <typename D>
    friend struct resource_ptr;
};

} // namespace filament::backend::fvkmemory

#endif // TNT_FILAMENT_BACKEND_VULKAN_MEMORY_RESOURCE_H
