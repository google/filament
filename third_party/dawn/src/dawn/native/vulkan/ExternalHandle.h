// Copyright 2022 The Dawn & Tint Authors
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

#ifndef SRC_DAWN_NATIVE_VULKAN_EXTERNALHANDLE_H_
#define SRC_DAWN_NATIVE_VULKAN_EXTERNALHANDLE_H_

#include "dawn/common/vulkan_platform.h"

namespace dawn::native::vulkan {

// ExternalSemaphoreHandle
#if DAWN_PLATFORM_IS(WINDOWS)
using ExternalSemaphoreHandle = HANDLE;
const ExternalSemaphoreHandle kNullExternalSemaphoreHandle = nullptr;
#elif DAWN_PLATFORM_IS(FUCHSIA)
using ExternalSemaphoreHandle = zx_handle_t;
const ExternalSemaphoreHandle kNullExternalSemaphoreHandle = ZX_HANDLE_INVALID;
#elif DAWN_PLATFORM_IS(POSIX)
using ExternalSemaphoreHandle = int;
const ExternalSemaphoreHandle kNullExternalSemaphoreHandle = -1;
#else
#error "Platform not supported."
#endif

// ExternalMemoryHandle
#if DAWN_PLATFORM_IS(ANDROID)
using ExternalMemoryHandle = struct AHardwareBuffer*;
#elif DAWN_PLATFORM_IS(LINUX)
using ExternalMemoryHandle = int;
#elif DAWN_PLATFORM_IS(FUCHSIA)
// Really a Zircon vmo handle.
using ExternalMemoryHandle = zx_handle_t;
#else
// Generic types so that the rest of the Vulkan backend compiles.
using ExternalMemoryHandle = void*;
#endif

}  // namespace dawn::native::vulkan

#endif  // SRC_DAWN_NATIVE_VULKAN_EXTERNALHANDLE_H_
