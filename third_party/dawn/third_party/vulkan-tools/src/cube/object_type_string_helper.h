/***************************************************************************
 *
 * Copyright (c) 2018 The Khronos Group Inc.
 * Copyright (c) 2018 Valve Corporation
 * Copyright (c) 2018 LunarG, Inc.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *
 * Author: Mark Lobodzinski <mark@lunarg.com>
 *
 ****************************************************************************/

#pragma once
#ifdef _MSC_VER
#pragma warning(disable : 4065)
#endif

#include <vulkan/vulkan.h>

static inline const char* string_VkObjectType(VkObjectType input_value) {
    switch ((VkObjectType)input_value) {
        case VK_OBJECT_TYPE_QUERY_POOL:
            return "VK_OBJECT_TYPE_QUERY_POOL";
        case VK_OBJECT_TYPE_SAMPLER_YCBCR_CONVERSION:
            return "VK_OBJECT_TYPE_SAMPLER_YCBCR_CONVERSION";
        case VK_OBJECT_TYPE_SEMAPHORE:
            return "VK_OBJECT_TYPE_SEMAPHORE";
        case VK_OBJECT_TYPE_SHADER_MODULE:
            return "VK_OBJECT_TYPE_SHADER_MODULE";
        case VK_OBJECT_TYPE_SWAPCHAIN_KHR:
            return "VK_OBJECT_TYPE_SWAPCHAIN_KHR";
        case VK_OBJECT_TYPE_SAMPLER:
            return "VK_OBJECT_TYPE_SAMPLER";
        case VK_OBJECT_TYPE_INDIRECT_COMMANDS_LAYOUT_NV:
            return "VK_OBJECT_TYPE_INDIRECT_COMMANDS_LAYOUT_NV";
        case VK_OBJECT_TYPE_DEBUG_REPORT_CALLBACK_EXT:
            return "VK_OBJECT_TYPE_DEBUG_REPORT_CALLBACK_EXT";
        case VK_OBJECT_TYPE_IMAGE:
            return "VK_OBJECT_TYPE_IMAGE";
        case VK_OBJECT_TYPE_UNKNOWN:
            return "VK_OBJECT_TYPE_UNKNOWN";
        case VK_OBJECT_TYPE_DESCRIPTOR_POOL:
            return "VK_OBJECT_TYPE_DESCRIPTOR_POOL";
        case VK_OBJECT_TYPE_COMMAND_BUFFER:
            return "VK_OBJECT_TYPE_COMMAND_BUFFER";
        case VK_OBJECT_TYPE_BUFFER:
            return "VK_OBJECT_TYPE_BUFFER";
        case VK_OBJECT_TYPE_SURFACE_KHR:
            return "VK_OBJECT_TYPE_SURFACE_KHR";
        case VK_OBJECT_TYPE_INSTANCE:
            return "VK_OBJECT_TYPE_INSTANCE";
        case VK_OBJECT_TYPE_VALIDATION_CACHE_EXT:
            return "VK_OBJECT_TYPE_VALIDATION_CACHE_EXT";
        case VK_OBJECT_TYPE_IMAGE_VIEW:
            return "VK_OBJECT_TYPE_IMAGE_VIEW";
        case VK_OBJECT_TYPE_DESCRIPTOR_SET:
            return "VK_OBJECT_TYPE_DESCRIPTOR_SET";
        case VK_OBJECT_TYPE_DESCRIPTOR_SET_LAYOUT:
            return "VK_OBJECT_TYPE_DESCRIPTOR_SET_LAYOUT";
        case VK_OBJECT_TYPE_COMMAND_POOL:
            return "VK_OBJECT_TYPE_COMMAND_POOL";
        case VK_OBJECT_TYPE_PHYSICAL_DEVICE:
            return "VK_OBJECT_TYPE_PHYSICAL_DEVICE";
        case VK_OBJECT_TYPE_DISPLAY_KHR:
            return "VK_OBJECT_TYPE_DISPLAY_KHR";
        case VK_OBJECT_TYPE_BUFFER_VIEW:
            return "VK_OBJECT_TYPE_BUFFER_VIEW";
        case VK_OBJECT_TYPE_DEBUG_UTILS_MESSENGER_EXT:
            return "VK_OBJECT_TYPE_DEBUG_UTILS_MESSENGER_EXT";
        case VK_OBJECT_TYPE_FRAMEBUFFER:
            return "VK_OBJECT_TYPE_FRAMEBUFFER";
        case VK_OBJECT_TYPE_DESCRIPTOR_UPDATE_TEMPLATE:
            return "VK_OBJECT_TYPE_DESCRIPTOR_UPDATE_TEMPLATE";
        case VK_OBJECT_TYPE_PIPELINE_CACHE:
            return "VK_OBJECT_TYPE_PIPELINE_CACHE";
        case VK_OBJECT_TYPE_PIPELINE_LAYOUT:
            return "VK_OBJECT_TYPE_PIPELINE_LAYOUT";
        case VK_OBJECT_TYPE_DEVICE_MEMORY:
            return "VK_OBJECT_TYPE_DEVICE_MEMORY";
        case VK_OBJECT_TYPE_FENCE:
            return "VK_OBJECT_TYPE_FENCE";
        case VK_OBJECT_TYPE_QUEUE:
            return "VK_OBJECT_TYPE_QUEUE";
        case VK_OBJECT_TYPE_DEVICE:
            return "VK_OBJECT_TYPE_DEVICE";
        case VK_OBJECT_TYPE_RENDER_PASS:
            return "VK_OBJECT_TYPE_RENDER_PASS";
        case VK_OBJECT_TYPE_DISPLAY_MODE_KHR:
            return "VK_OBJECT_TYPE_DISPLAY_MODE_KHR";
        case VK_OBJECT_TYPE_EVENT:
            return "VK_OBJECT_TYPE_EVENT";
        case VK_OBJECT_TYPE_PIPELINE:
            return "VK_OBJECT_TYPE_PIPELINE";
        default:
            return "Unhandled VkObjectType";
    }
}
