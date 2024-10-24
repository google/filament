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

#include <cstdint>

namespace filament::backend::fvkmemory {

using CounterIndex = int32_t;
using HandleId = HandleBase::HandleId;

struct Resource {
    CounterIndex counterIndex = -1;
};

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
    UNDEFINED_TYPE = 14,
};

template<typename D>
ResourceType getTypeEnum() noexcept;

std::string getTypeStr(ResourceType type);

} // namespace filament::backend::fvkmemory

#endif // TNT_FILAMENT_BACKEND_VULKAN_MEMORY_RESOURCE_H

