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

#include "dawn/native/metal/BackendMTL.h"

#include "dawn/common/NSRef.h"
#include "dawn/common/SystemUtils.h"
#include "dawn/native/ChainUtils.h"
#include "dawn/native/Instance.h"
#include "dawn/native/MetalBackend.h"
#include "dawn/native/metal/PhysicalDeviceMTL.h"

#include <string>
#include <vector>

namespace dawn::native::metal {

namespace {

bool CheckMetalValidationEnabled(InstanceBase* instance) {
    if (instance->IsBackendValidationEnabled()) {
        return true;
    }

    // Sometime validation layer can be enabled eternally via xcode or command line.
    if (GetEnvironmentVar("METAL_DEVICE_WRAPPER_TYPE").first == "1" ||
        GetEnvironmentVar("MTL_DEBUG_LAYER").first == "1") {
        return true;
    }

    return false;
}

}  // anonymous namespace

Backend::Backend(InstanceBase* instance) : BackendConnection(instance, wgpu::BackendType::Metal) {
    if (GetInstance()->IsBackendValidationEnabled()) {
        setenv("METAL_DEVICE_WRAPPER_TYPE", "1", 1);
    }
}

Backend::~Backend() = default;

std::vector<Ref<PhysicalDeviceBase>> Backend::DiscoverPhysicalDevices(
    const UnpackedPtr<RequestAdapterOptions>& options) {
    if (options->forceFallbackAdapter) {
        return {};
    }
    if (!mPhysicalDevices.empty()) {
        // Devices already discovered.
        return std::vector<Ref<PhysicalDeviceBase>>{mPhysicalDevices};
    }

    bool metalValidationEnabled = CheckMetalValidationEnabled(GetInstance());
    @autoreleasepool {
#if DAWN_PLATFORM_IS(MACOS)
        for (id<MTLDevice> device in MTLCopyAllDevices()) {
            Ref<PhysicalDevice> physicalDevice = AcquireRef(
                new PhysicalDevice(GetInstance(), AcquireNSPRef(device), metalValidationEnabled));
            if (!GetInstance()->ConsumedErrorAndWarnOnce(physicalDevice->Initialize())) {
                mPhysicalDevices.push_back(std::move(physicalDevice));
            }
        }
#endif

        // iOS only has a single device so MTLCopyAllDevices doesn't exist there.
#if DAWN_PLATFORM_IS(IOS)
        Ref<PhysicalDevice> physicalDevice = AcquireRef(new PhysicalDevice(
            GetInstance(), AcquireNSPRef(MTLCreateSystemDefaultDevice()), metalValidationEnabled));
        if (!GetInstance()->ConsumedErrorAndWarnOnce(physicalDevice->Initialize())) {
            mPhysicalDevices.push_back(std::move(physicalDevice));
        }
#endif
    }
    return std::vector<Ref<PhysicalDeviceBase>>{mPhysicalDevices};
}

BackendConnection* Connect(InstanceBase* instance) {
    return new Backend(instance);
}

}  // namespace dawn::native::metal
