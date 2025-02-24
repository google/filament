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

#include "dawn/native/opengl/BackendGL.h"

#include <memory>
#include <string>
#include <utility>

#include "dawn/native/ChainUtils.h"
#include "dawn/native/Instance.h"
#include "dawn/native/OpenGLBackend.h"
#include "dawn/native/opengl/DisplayEGL.h"
#include "dawn/native/opengl/PhysicalDeviceGL.h"

namespace dawn::native::opengl {

namespace {
#if DAWN_PLATFORM_IS(WINDOWS)
const char* kEGLLib = "libEGL.dll";
#elif DAWN_PLATFORM_IS(MACOS)
const char* kEGLLib = "libEGL.dylib";
#else
const char* kEGLLib = "libEGL.so";
#endif

}  // anonymous namespace

Backend::Backend(InstanceBase* instance, wgpu::BackendType backendType)
    : BackendConnection(instance, backendType) {}

std::vector<Ref<PhysicalDeviceBase>> Backend::DiscoverPhysicalDevices(
    const UnpackedPtr<RequestAdapterOptions>& options) {
    if (options->forceFallbackAdapter) {
        return {};
    }
    if (options->featureLevel != wgpu::FeatureLevel::Compatibility) {
        // Return an empty vector since GL physical devices can only support compatibility mode.
        return {};
    }

    bool forceES31AndMinExtensions = false;
    if (auto* togglesDesc = options.Get<DawnTogglesDescriptor>()) {
        TogglesState toggles =
            TogglesState::CreateFromTogglesDescriptor(togglesDesc, ToggleStage::Adapter);
        if (toggles.IsEnabled(Toggle::GLForceES31AndNoExtensions)) {
            forceES31AndMinExtensions = true;
        }
    }

    std::vector<Ref<PhysicalDeviceBase>> devices;

    // A helper function performing checks on the display we're trying to use, and adding the
    // physical device from it to the list returned by the discovery.
    auto AppendNewDeviceFrom = [&](ResultOrError<Ref<DisplayEGL>> maybeDisplay) -> MaybeError {
        Ref<DisplayEGL> display;
        DAWN_TRY_ASSIGN(display, std::move(maybeDisplay));

        if (!display->egl.HasExt(EGLExt::CreateContextRobustness)) {
            return DAWN_VALIDATION_ERROR("EGL_EXT_create_context_robustness is required.");
        }
        if (!display->egl.HasExt(EGLExt::FenceSync) && !display->egl.HasExt(EGLExt::ReusableSync)) {
            return DAWN_INTERNAL_ERROR(
                "EGL_KHR_fence_sync or EGL_KHR_reusable_sync must be supported");
        }

        Ref<PhysicalDevice> device;
        DAWN_TRY_ASSIGN(device, PhysicalDevice::Create(GetType(), std::move(display),
                                                       forceES31AndMinExtensions));
        devices.push_back(device);

        return {};
    };

    // A helper function used to give more context before printing the EGL loading error.
    auto SwallowDiscoveryError = [&](MaybeError maybeError) {
        if (maybeError.IsError()) {
            std::unique_ptr<ErrorData> error = maybeError.AcquireError();
            error->AppendContext("trying to discover a %s adapter.", GetType());
            GetInstance()->ConsumedErrorAndWarnOnce(std::move(error));
        }
    };

    if (auto* glGetProcOptions = options.Get<RequestAdapterOptionsGetGLProc>()) {
        SwallowDiscoveryError(AppendNewDeviceFrom(DisplayEGL::CreateFromProcAndDisplay(
            GetType(), glGetProcOptions->getProc, glGetProcOptions->display)));
    } else {
        SwallowDiscoveryError(
            AppendNewDeviceFrom(DisplayEGL::CreateFromDynamicLoading(GetType(), kEGLLib)));
    }

    return devices;
}

BackendConnection* Connect(InstanceBase* instance, wgpu::BackendType backendType) {
    return new Backend(instance, backendType);
}

}  // namespace dawn::native::opengl
