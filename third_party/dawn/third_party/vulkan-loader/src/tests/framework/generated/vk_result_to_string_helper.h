#pragma once
// *** THIS FILE IS GENERATED - DO NOT EDIT ***
// See vk_result_to_string_generator.py for modifications

/*
 * Copyright (c) 2025 The Khronos Group Inc.
 * Copyright (c) 2025 Valve Corporation
 * Copyright (c) 2025 LunarG, Inc.
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

 */

#include <ostream>
#include <string>

#include <vulkan/vulkan.h>

inline std::ostream& operator<<(std::ostream& os, const VkResult& result) {
    switch (result) {
        case (VK_SUCCESS):
            return os << "VK_SUCCESS";
        case (VK_NOT_READY):
            return os << "VK_NOT_READY";
        case (VK_TIMEOUT):
            return os << "VK_TIMEOUT";
        case (VK_EVENT_SET):
            return os << "VK_EVENT_SET";
        case (VK_EVENT_RESET):
            return os << "VK_EVENT_RESET";
        case (VK_INCOMPLETE):
            return os << "VK_INCOMPLETE";
        case (VK_ERROR_OUT_OF_HOST_MEMORY):
            return os << "VK_ERROR_OUT_OF_HOST_MEMORY";
        case (VK_ERROR_OUT_OF_DEVICE_MEMORY):
            return os << "VK_ERROR_OUT_OF_DEVICE_MEMORY";
        case (VK_ERROR_INITIALIZATION_FAILED):
            return os << "VK_ERROR_INITIALIZATION_FAILED";
        case (VK_ERROR_DEVICE_LOST):
            return os << "VK_ERROR_DEVICE_LOST";
        case (VK_ERROR_MEMORY_MAP_FAILED):
            return os << "VK_ERROR_MEMORY_MAP_FAILED";
        case (VK_ERROR_LAYER_NOT_PRESENT):
            return os << "VK_ERROR_LAYER_NOT_PRESENT";
        case (VK_ERROR_EXTENSION_NOT_PRESENT):
            return os << "VK_ERROR_EXTENSION_NOT_PRESENT";
        case (VK_ERROR_FEATURE_NOT_PRESENT):
            return os << "VK_ERROR_FEATURE_NOT_PRESENT";
        case (VK_ERROR_INCOMPATIBLE_DRIVER):
            return os << "VK_ERROR_INCOMPATIBLE_DRIVER";
        case (VK_ERROR_TOO_MANY_OBJECTS):
            return os << "VK_ERROR_TOO_MANY_OBJECTS";
        case (VK_ERROR_FORMAT_NOT_SUPPORTED):
            return os << "VK_ERROR_FORMAT_NOT_SUPPORTED";
        case (VK_ERROR_FRAGMENTED_POOL):
            return os << "VK_ERROR_FRAGMENTED_POOL";
        case (VK_ERROR_UNKNOWN):
            return os << "VK_ERROR_UNKNOWN";
        case (VK_ERROR_VALIDATION_FAILED):
            return os << "VK_ERROR_VALIDATION_FAILED";
        case (VK_ERROR_OUT_OF_POOL_MEMORY):
            return os << "VK_ERROR_OUT_OF_POOL_MEMORY";
        case (VK_ERROR_INVALID_EXTERNAL_HANDLE):
            return os << "VK_ERROR_INVALID_EXTERNAL_HANDLE";
        case (VK_ERROR_INVALID_OPAQUE_CAPTURE_ADDRESS):
            return os << "VK_ERROR_INVALID_OPAQUE_CAPTURE_ADDRESS";
        case (VK_ERROR_FRAGMENTATION):
            return os << "VK_ERROR_FRAGMENTATION";
        case (VK_PIPELINE_COMPILE_REQUIRED):
            return os << "VK_PIPELINE_COMPILE_REQUIRED";
        case (VK_ERROR_NOT_PERMITTED):
            return os << "VK_ERROR_NOT_PERMITTED";
        case (VK_ERROR_SURFACE_LOST_KHR):
            return os << "VK_ERROR_SURFACE_LOST_KHR";
        case (VK_ERROR_NATIVE_WINDOW_IN_USE_KHR):
            return os << "VK_ERROR_NATIVE_WINDOW_IN_USE_KHR";
        case (VK_SUBOPTIMAL_KHR):
            return os << "VK_SUBOPTIMAL_KHR";
        case (VK_ERROR_OUT_OF_DATE_KHR):
            return os << "VK_ERROR_OUT_OF_DATE_KHR";
        case (VK_ERROR_INCOMPATIBLE_DISPLAY_KHR):
            return os << "VK_ERROR_INCOMPATIBLE_DISPLAY_KHR";
        case (VK_ERROR_INVALID_SHADER_NV):
            return os << "VK_ERROR_INVALID_SHADER_NV";
        case (VK_ERROR_IMAGE_USAGE_NOT_SUPPORTED_KHR):
            return os << "VK_ERROR_IMAGE_USAGE_NOT_SUPPORTED_KHR";
        case (VK_ERROR_VIDEO_PICTURE_LAYOUT_NOT_SUPPORTED_KHR):
            return os << "VK_ERROR_VIDEO_PICTURE_LAYOUT_NOT_SUPPORTED_KHR";
        case (VK_ERROR_VIDEO_PROFILE_OPERATION_NOT_SUPPORTED_KHR):
            return os << "VK_ERROR_VIDEO_PROFILE_OPERATION_NOT_SUPPORTED_KHR";
        case (VK_ERROR_VIDEO_PROFILE_FORMAT_NOT_SUPPORTED_KHR):
            return os << "VK_ERROR_VIDEO_PROFILE_FORMAT_NOT_SUPPORTED_KHR";
        case (VK_ERROR_VIDEO_PROFILE_CODEC_NOT_SUPPORTED_KHR):
            return os << "VK_ERROR_VIDEO_PROFILE_CODEC_NOT_SUPPORTED_KHR";
        case (VK_ERROR_VIDEO_STD_VERSION_NOT_SUPPORTED_KHR):
            return os << "VK_ERROR_VIDEO_STD_VERSION_NOT_SUPPORTED_KHR";
        case (VK_ERROR_INVALID_DRM_FORMAT_MODIFIER_PLANE_LAYOUT_EXT):
            return os << "VK_ERROR_INVALID_DRM_FORMAT_MODIFIER_PLANE_LAYOUT_EXT";
        case (VK_ERROR_PRESENT_TIMING_QUEUE_FULL_EXT):
            return os << "VK_ERROR_PRESENT_TIMING_QUEUE_FULL_EXT";
        case (VK_ERROR_FULL_SCREEN_EXCLUSIVE_MODE_LOST_EXT):
            return os << "VK_ERROR_FULL_SCREEN_EXCLUSIVE_MODE_LOST_EXT";
        case (VK_THREAD_IDLE_KHR):
            return os << "VK_THREAD_IDLE_KHR";
        case (VK_THREAD_DONE_KHR):
            return os << "VK_THREAD_DONE_KHR";
        case (VK_OPERATION_DEFERRED_KHR):
            return os << "VK_OPERATION_DEFERRED_KHR";
        case (VK_OPERATION_NOT_DEFERRED_KHR):
            return os << "VK_OPERATION_NOT_DEFERRED_KHR";
        case (VK_ERROR_INVALID_VIDEO_STD_PARAMETERS_KHR):
            return os << "VK_ERROR_INVALID_VIDEO_STD_PARAMETERS_KHR";
        case (VK_ERROR_COMPRESSION_EXHAUSTED_EXT):
            return os << "VK_ERROR_COMPRESSION_EXHAUSTED_EXT";
        case (VK_INCOMPATIBLE_SHADER_BINARY_EXT):
            return os << "VK_INCOMPATIBLE_SHADER_BINARY_EXT";
        case (VK_PIPELINE_BINARY_MISSING_KHR):
            return os << "VK_PIPELINE_BINARY_MISSING_KHR";
        case (VK_ERROR_NOT_ENOUGH_SPACE_KHR):
            return os << "VK_ERROR_NOT_ENOUGH_SPACE_KHR";
        default:
            return os << static_cast<int32_t>(result);
    }
}
