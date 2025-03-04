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

#ifndef SRC_DAWN_NATIVE_VULKAN_VULKANERROR_H_
#define SRC_DAWN_NATIVE_VULKAN_VULKANERROR_H_

#include <string>

#include "dawn/native/ErrorInjector.h"
#include "dawn/native/vulkan/VulkanFunctions.h"

constexpr VkResult VK_FAKE_ERROR_FOR_TESTING = VK_RESULT_MAX_ENUM;
constexpr VkResult VK_FAKE_DEVICE_OOM_FOR_TESTING = static_cast<VkResult>(VK_RESULT_MAX_ENUM - 1);

namespace dawn::native::vulkan {

// Returns a string version of the result.
std::string VkResultAsString(::VkResult result);

MaybeError CheckVkSuccessImpl(VkResult result, const char* context);
MaybeError CheckVkOOMThenSuccessImpl(VkResult result, const char* context);

// Returns a success only if result if VK_SUCCESS, an error with the context and stringified
// result value instead. Can be used like this:
//
//   DAWN_TRY(CheckVkSuccess(vkDoSomething, "doing something"));
#define CheckVkSuccess(resultIn, contextIn)                            \
    ::dawn::native::vulkan::CheckVkSuccessImpl(                        \
        ::dawn::native::vulkan::VkResult::WrapUnsafe(                  \
            INJECT_ERROR_OR_RUN(resultIn, VK_FAKE_ERROR_FOR_TESTING)), \
        contextIn)

#define CheckVkOOMThenSuccess(resultIn, contextIn)                                 \
    ::dawn::native::vulkan::CheckVkOOMThenSuccessImpl(                             \
        ::dawn::native::vulkan::VkResult::WrapUnsafe(INJECT_ERROR_OR_RUN(          \
            resultIn, VK_FAKE_DEVICE_OOM_FOR_TESTING, VK_FAKE_ERROR_FOR_TESTING)), \
        contextIn)

}  // namespace dawn::native::vulkan

#endif  // SRC_DAWN_NATIVE_VULKAN_VULKANERROR_H_
