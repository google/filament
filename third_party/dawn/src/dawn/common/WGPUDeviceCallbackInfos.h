// Copyright 2026 The Dawn & Tint Authors
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

#ifndef SRC_DAWN_COMMON_WGPUDEVICECALLBACKINFOS_H_
#define SRC_DAWN_COMMON_WGPUDEVICECALLBACKINFOS_H_

#include <webgpu/webgpu.h>

#include <optional>

#include "dawn/common/MutexProtected.h"

namespace dawn {

const WGPUDeviceLostCallbackInfo& GetDeviceLostCallbackInfoOrDefault(
    const WGPUDeviceDescriptor* descriptor);

// Device level unconditionally spontaneous callbacks need to be synchronized so this class provides
// a common wrapper for those callback infos so that the implementation can be shared across native
// and wire client.
class WGPUDeviceCallbackInfos {
  public:
    WGPUDeviceCallbackInfos();
    explicit WGPUDeviceCallbackInfos(const WGPUDeviceDescriptor* descriptor);

    // APIs to call the callbacks.
    void CallErrorCallback(WGPUDevice const* device, WGPUErrorType type, WGPUStringView message);
    void CallLoggingCallback(WGPULoggingType type, WGPUStringView message);

    // The logging callback currently needs to support a setter.
    void SetLoggingCallbackInfo(const WGPULoggingCallbackInfo& callbackInfo);

    // Used when the device is lost and we want to clear out the callbacks. This helper waits until
    // there are no other places using the callbacks before returning. This is important since this
    // is generally used when completing the device lost event which users may use to clean up the
    // uncaptured error and logging callbacks.
    void Clear();

  private:
    struct CallbackInfos {
        CallbackInfos();
        CallbackInfos(const WGPUUncapturedErrorCallbackInfo& error,
                      const WGPULoggingCallbackInfo& logging);

        // The callback infos are optional because once the device is lost, they are set to
        // std::nullopt and no longer do anything.
        std::optional<WGPUUncapturedErrorCallbackInfo> error = std::nullopt;
        std::optional<WGPULoggingCallbackInfo> logging = std::nullopt;

        // Counter that tracks how many places are currently using callback infos. This is used to
        // ensure that before we call the device lost callback (which may deallocate the uncaptured
        // error and logging callbacks), we have ensured that there are no outstanding references to
        // those callbacks.
        uint32_t semaphore = 0;
    };
    MutexCondVarProtected<CallbackInfos> mCallbackInfos;
};

}  // namespace dawn

#endif  // SRC_DAWN_COMMON_WGPUDEVICECALLBACKINFOS_H_
