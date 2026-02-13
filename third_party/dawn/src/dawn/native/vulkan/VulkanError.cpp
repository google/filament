// Copyright 2019 The Dawn & Tint Authors
//
// Redistribution and use in source and binary forms, with or without
// modification, are permitted provided that the following conditions are met:
//
// 1. Redistributions of source code must retain the above copyright notice, this
//    list of conditions and the following disclaimer.
//
// 2. Redistributions in binary form must reproduce the above copyright notice,
//    this list of conditions and the following disclaimer in the documentation
//    and/or other materials provided with the distribution.
//
// 3. Neither the name of the copyright holder nor the names of its
//    contributors may be used to endorse or promote products derived from
//    this software without specific prior written permission.
//
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
// AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
// IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
// DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
// FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
// DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
// SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
// CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
// OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
// OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

#include "dawn/native/vulkan/VulkanError.h"

#include <string>

namespace dawn::native::vulkan {

std::string VkResultAsString(::VkResult result) {
    // Convert to a int32_t to silence and MSVC warning that the fake errors don't appear in
    // the original VkResult enum.
    int32_t code = static_cast<int32_t>(result);

    switch (code) {
        case VK_SUCCESS:
            return "VK_SUCCESS";
        case VK_NOT_READY:
            return "VK_NOT_READY";
        case VK_TIMEOUT:
            return "VK_TIMEOUT";
        case VK_EVENT_SET:
            return "VK_EVENT_SET";
        case VK_EVENT_RESET:
            return "VK_EVENT_RESET";
        case VK_INCOMPLETE:
            return "VK_INCOMPLETE";
        case VK_ERROR_OUT_OF_HOST_MEMORY:
            return "VK_ERROR_OUT_OF_HOST_MEMORY";
        case VK_ERROR_OUT_OF_DEVICE_MEMORY:
            return "VK_ERROR_OUT_OF_DEVICE_MEMORY";
        case VK_ERROR_INITIALIZATION_FAILED:
            return "VK_ERROR_INITIALIZATION_FAILED";
        case VK_ERROR_DEVICE_LOST:
            return "VK_ERROR_DEVICE_LOST";
        case VK_ERROR_MEMORY_MAP_FAILED:
            return "VK_ERROR_MEMORY_MAP_FAILED";
        case VK_ERROR_LAYER_NOT_PRESENT:
            return "VK_ERROR_LAYER_NOT_PRESENT";
        case VK_ERROR_EXTENSION_NOT_PRESENT:
            return "VK_ERROR_EXTENSION_NOT_PRESENT";
        case VK_ERROR_FEATURE_NOT_PRESENT:
            return "VK_ERROR_FEATURE_NOT_PRESENT";
        case VK_ERROR_INCOMPATIBLE_DRIVER:
            return "VK_ERROR_INCOMPATIBLE_DRIVER";
        case VK_ERROR_TOO_MANY_OBJECTS:
            return "VK_ERROR_TOO_MANY_OBJECTS";
        case VK_ERROR_FORMAT_NOT_SUPPORTED:
            return "VK_ERROR_FORMAT_NOT_SUPPORTED";
        case VK_ERROR_FRAGMENTED_POOL:
            return "VK_ERROR_FRAGMENTED_POOL";
        case VK_ERROR_UNKNOWN:
            return "VK_ERROR_UNKNOWN";

        case VK_ERROR_SURFACE_LOST_KHR:
            return "VK_ERROR_SURFACE_LOST_KHR";
        case VK_ERROR_NATIVE_WINDOW_IN_USE_KHR:
            return "VK_ERROR_NATIVE_WINDOW_IN_USE_KHR";

        case VK_FAKE_DEVICE_OOM_FOR_TESTING:
            return "VK_FAKE_DEVICE_OOM_FOR_TESTING";
        case VK_FAKE_ERROR_FOR_TESTING:
            return "VK_FAKE_ERROR_FOR_TESTING";
        default: {
            std::stringstream s;
            s << "<Unknown VkResult: " << code << ">";
            return s.str();
        }
    }
}

MaybeError CheckVkSuccessImpl(VkResult result, const char* context) {
    if (result == VK_SUCCESS) [[likely]] {
        return {};
    }

    std::string message = std::string(context) + " failed with " + VkResultAsString(result);

    if (result == VK_ERROR_DEVICE_LOST) {
        return DAWN_DEVICE_LOST_ERROR(message);
    } else {
        return DAWN_INTERNAL_ERROR(message);
    }
}

MaybeError CheckVkOOMThenSuccessImpl(VkResult result, const char* context) {
    if (result == VK_SUCCESS) [[likely]] {
        return {};
    }

    std::string message = std::string(context) + " failed with " + VkResultAsString(result);

    if (result == VK_ERROR_OUT_OF_DEVICE_MEMORY || result == VK_ERROR_OUT_OF_HOST_MEMORY ||
        result == VK_FAKE_DEVICE_OOM_FOR_TESTING) {
        return DAWN_OUT_OF_MEMORY_ERROR(message);
    } else if (result == VK_ERROR_DEVICE_LOST) {
        return DAWN_DEVICE_LOST_ERROR(message);
    } else {
        return DAWN_INTERNAL_ERROR(message);
    }
}

}  // namespace dawn::native::vulkan
