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

#include "dawn/common/WGPUDeviceCallbackInfos.h"

#include "dawn/common/Log.h"

namespace dawn {
namespace {
// Default callback infos depending on the build type.
#ifdef DAWN_ENABLE_ASSERTS
static constexpr WGPUDeviceLostCallbackInfo kDefaultDeviceLostCallbackInfo = {
    nullptr, WGPUCallbackMode_AllowSpontaneous,
    [](WGPUDevice const*, WGPUDeviceLostReason, WGPUStringView, void*, void*) {
        static std::once_flag flag;
        std::call_once(flag, []() {
            dawn::WarningLog() << "No Dawn device lost callback was set. This is probably not "
                                  "intended. If you really want to ignore device lost "
                                  "and suppress this message, set the callback explicitly.";
        });
    },
    nullptr, nullptr};
static constexpr WGPUUncapturedErrorCallbackInfo kDefaultUncapturedErrorCallbackInfo = {
    nullptr,
    [](WGPUDevice const*, WGPUErrorType, WGPUStringView, void*, void*) {
        static std::once_flag flag;
        std::call_once(flag, []() {
            dawn::WarningLog() << "No Dawn device uncaptured error callback was set. This is "
                                  "probably not intended. If you really want to ignore errors "
                                  "and suppress this message, set the callback explicitly.";
        });
    },
    nullptr, nullptr};
static constexpr WGPULoggingCallbackInfo kDefaultLoggingCallbackInfo = {
    nullptr,
    [](WGPULoggingType, WGPUStringView, void*, void*) {
        static std::once_flag flag;
        std::call_once(flag, []() {
            dawn::WarningLog() << "No Dawn device logging callback callback was set. This is "
                                  "probably not intended. If you really want to ignore logs "
                                  "and suppress this message, set the callback explicitly.";
        });
    },
    nullptr, nullptr};
#else
static constexpr WGPUDeviceLostCallbackInfo kDefaultDeviceLostCallbackInfo = {
    nullptr, WGPUCallbackMode_AllowSpontaneous, nullptr, nullptr, nullptr};
static constexpr WGPUUncapturedErrorCallbackInfo kDefaultUncapturedErrorCallbackInfo = {
    nullptr, nullptr, nullptr, nullptr};
static constexpr WGPULoggingCallbackInfo kDefaultLoggingCallbackInfo = {nullptr, nullptr, nullptr,
                                                                        nullptr};
#endif  // DAWN_ENABLE_ASSERTS

const WGPUUncapturedErrorCallbackInfo& GetUncapturedErrorCallbackInfoOrDefault(
    const WGPUDeviceDescriptor* descriptor) {
    if (descriptor != nullptr && descriptor->uncapturedErrorCallbackInfo.callback != nullptr) {
        return descriptor->uncapturedErrorCallbackInfo;
    }
    return kDefaultUncapturedErrorCallbackInfo;
}
}  // namespace

const WGPUDeviceLostCallbackInfo& GetDeviceLostCallbackInfoOrDefault(
    const WGPUDeviceDescriptor* descriptor) {
    if (descriptor != nullptr && descriptor->deviceLostCallbackInfo.callback != nullptr) {
        return descriptor->deviceLostCallbackInfo;
    }
    return kDefaultDeviceLostCallbackInfo;
}

WGPUDeviceCallbackInfos::CallbackInfos::CallbackInfos() = default;

WGPUDeviceCallbackInfos::CallbackInfos::CallbackInfos(const WGPUUncapturedErrorCallbackInfo& error,
                                                      const WGPULoggingCallbackInfo& logging) {
    if (error.callback != nullptr) {
        this->error = error;
    }
    if (logging.callback != nullptr) {
        this->logging = logging;
    }
}

WGPUDeviceCallbackInfos::WGPUDeviceCallbackInfos() = default;

WGPUDeviceCallbackInfos::WGPUDeviceCallbackInfos(const WGPUDeviceDescriptor* descriptor)
    : mCallbackInfos(GetUncapturedErrorCallbackInfoOrDefault(descriptor),
                     kDefaultLoggingCallbackInfo) {}

void WGPUDeviceCallbackInfos::CallErrorCallback(WGPUDevice const* device,
                                                WGPUErrorType type,
                                                WGPUStringView message) {
    std::optional<WGPUUncapturedErrorCallbackInfo> callbackInfo;
    mCallbackInfos.Use<NotifyType::None>([&](auto callbackInfos) {
        callbackInfo = callbackInfos->error;
        if (callbackInfo) {
            callbackInfos->semaphore += 1;
        }
    });

    // If we don't have a callback info, we can just return.
    if (!callbackInfo) {
        return;
    }

    // Call the callback without holding the lock to prevent any re-entrant issues.
    DAWN_ASSERT(callbackInfo->callback != nullptr);
    callbackInfo->callback(device, type, message, callbackInfo->userdata1, callbackInfo->userdata2);

    mCallbackInfos.Use([&](auto callbackInfos) {
        DAWN_ASSERT(callbackInfos->semaphore > 0);
        callbackInfos->semaphore -= 1;
    });
}

void WGPUDeviceCallbackInfos::CallLoggingCallback(WGPULoggingType type, WGPUStringView message) {
    std::optional<WGPULoggingCallbackInfo> callbackInfo;
    mCallbackInfos.Use<NotifyType::None>([&](auto callbackInfos) {
        callbackInfo = callbackInfos->logging;
        if (callbackInfo) {
            callbackInfos->semaphore += 1;
        }
    });

    // If we don't have a callback info, we can just return.
    if (!callbackInfo) {
        return;
    }

    // Call the callback without holding the lock to prevent any re-entrant issues.
    DAWN_ASSERT(callbackInfo->callback != nullptr);
    callbackInfo->callback(type, message, callbackInfo->userdata1, callbackInfo->userdata2);

    mCallbackInfos.Use([&](auto callbackInfos) {
        DAWN_ASSERT(callbackInfos->semaphore > 0);
        callbackInfos->semaphore -= 1;
    });
}

void WGPUDeviceCallbackInfos::SetLoggingCallbackInfo(const WGPULoggingCallbackInfo& callbackInfo) {
    mCallbackInfos.Use<NotifyType::None>(
        [&](auto callbackInfos) { callbackInfos->logging = callbackInfo; });
}

void WGPUDeviceCallbackInfos::Clear() {
    mCallbackInfos.Use<NotifyType::None>([](auto callbackInfos) {
        callbackInfos->error = std::nullopt;
        callbackInfos->logging = std::nullopt;

        // The uncaptured error and logging callbacks are spontaneous and must not be called
        // after we call the device lost's |mCallback| below. Although we have cleared those
        // callbacks, we need to wait for any remaining outstanding callbacks to finish before
        // continuing.
        callbackInfos.Wait([](auto& x) { return x.semaphore == 0; });
    });
}

}  // namespace dawn
