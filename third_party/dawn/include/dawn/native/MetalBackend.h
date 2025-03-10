// Copyright 2018 The Dawn & Tint Authors
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

#ifndef INCLUDE_DAWN_NATIVE_METALBACKEND_H_
#define INCLUDE_DAWN_NATIVE_METALBACKEND_H_

#include <webgpu/webgpu.h>

#include "dawn/native/dawn_native_export.h"

#if defined(__OBJC__)
#import <Metal/Metal.h>
#endif

namespace dawn::native::metal {

// When making Metal interop with other APIs, we need to be careful that QueueSubmit doesn't
// mean that the operations will be visible to other APIs/Metal devices right away. macOS
// does have a global queue of graphics operations, but the command buffers are inserted there
// when they are "scheduled". Submitting other operations before the command buffer is
// scheduled could lead to races in who gets scheduled first and incorrect rendering.
DAWN_NATIVE_EXPORT void WaitForCommandsToBeScheduled(WGPUDevice device);

#if defined(__OBJC__)
// Return the MTLDevice corresponding to the WGPUDevice.
DAWN_NATIVE_EXPORT id<MTLDevice> GetMTLDevice(WGPUDevice device);
#endif

}  // namespace dawn::native::metal

#endif  // INCLUDE_DAWN_NATIVE_METALBACKEND_H_
