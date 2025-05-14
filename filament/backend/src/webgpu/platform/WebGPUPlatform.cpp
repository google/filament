/*
 * Copyright (C) 2025 The Android Open Source Project
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include <backend/platforms/WebGPUPlatform.h>

#include "webgpu/WebGPUConstants.h"
#include "webgpu/WebGPUDriver.h"

#include <backend/Platform.h>
#include <utils/Panic.h>
#include <utils/ostream.h>

#include <dawn/webgpu_cpp_print.h>
#include <webgpu/webgpu_cpp.h>

#include <cstdint>
#include <sstream>

/**
 * WebGPU Backend implementation common across platforms or operating systems (at least for now).
 * Some of these functions may likely be refactored to platform/OS-specific implementations
 * over time as needed. The caller of the WebGPUPlatform doesn't need to care which is the case.
 */

namespace filament::backend {

namespace {

//either returns a valid instance or panics
[[nodiscard]] wgpu::Instance createInstance() {
    wgpu::DawnTogglesDescriptor dawnTogglesDescriptor{};
#if defined(FILAMENT_WEBGPU_IMMEDIATE_ERROR_HANDLING)
#if FWGPU_ENABLED(FWGPU_PRINT_SYSTEM)
    FWGPU_LOGI << "setting on toggle enable_immediate_error_handling" << utils::io::endl;
#endif
    /**
     * Have the un-captured error callback invoked immediately when an error occurs, rather than
     * waiting for the next Tick. This enables using the stack trace in which the un-captured error
     * occurred when breaking into the un-captured error callback.
     * https://crbug.com/dawn/1789
     */
    static const char* toggleName = "enable_immediate_error_handling";
    dawnTogglesDescriptor.enabledToggleCount = 1;
    dawnTogglesDescriptor.enabledToggles = &toggleName;
#endif
    wgpu::InstanceDescriptor instanceDescriptor{
        .nextInChain = &dawnTogglesDescriptor,
        .capabilities = {
            .timedWaitAnyEnable = true// TODO consider using pure async instead
        }
    };
    wgpu::Instance instance = wgpu::CreateInstance(&instanceDescriptor);
    FILAMENT_CHECK_POSTCONDITION(instance != nullptr) << "Unable to create webgpu instance.";
    return instance;
}

}// namespace

wgpu::Adapter WebGPUPlatform::requestAdapter(wgpu::Surface const& surface) {
    // TODO consider power preference etc. (can be custom preferences passed to the platform or
    //                                      based on whether this is a Mobile or Desktop system,
    //                                      etc...)
    wgpu::RequestAdapterOptions adaptorOptions{ .compatibleSurface = surface };
    // note this just gets the first adapter
    wgpu::Adapter adapter = nullptr;
    wgpu::WaitStatus status = mInstance.WaitAny(
            mInstance.RequestAdapter(&adaptorOptions, wgpu::CallbackMode::WaitAnyOnly,
                    [&adapter](wgpu::RequestAdapterStatus const status,
                            wgpu::Adapter const& readyAdapter, wgpu::StringView const message) {
                        // TODO consider more robust error handling
                        FILAMENT_CHECK_POSTCONDITION(status == wgpu::RequestAdapterStatus::Success)
                                << "Unable to request a WebGPU adapter. Status "
                                << static_cast<uint32_t>(status)
                                << " with message: " << message.data;
                        adapter = readyAdapter;
                    }),
            UINT16_MAX);// TODO define reasonable timeout (or do this asynchronously)
    FILAMENT_CHECK_POSTCONDITION(status == wgpu::WaitStatus::Success)
            << "Non-successful wait status requesting a WebGPU adapter "
            << static_cast<uint32_t>(status);
    FILAMENT_CHECK_POSTCONDITION(adapter != nullptr)
            << "Failed to get a WebGPU adapter for the platform";
    // TODO consider validating adapter has required features and/or limits
    return adapter;
}

wgpu::Device WebGPUPlatform::requestDevice(wgpu::Adapter const& adapter) {
    // TODO consider passing limits
    constexpr std::array optionalFeatures = { wgpu::FeatureName::DepthClipControl,
        wgpu::FeatureName::Depth32FloatStencil8, wgpu::FeatureName::CoreFeaturesAndLimits };

    constexpr std::array requiredFeatures = { wgpu::FeatureName::TransientAttachments };

    wgpu::SupportedFeatures supportedFeatures;
    adapter.GetFeatures(&supportedFeatures);

    std::vector<wgpu::FeatureName> enabledFeatures;
    enabledFeatures.reserve(requiredFeatures.size() + optionalFeatures.size());

    std::set_intersection(supportedFeatures.features,
            supportedFeatures.features + supportedFeatures.featureCount, requiredFeatures.begin(),
            requiredFeatures.end(), std::back_inserter(enabledFeatures));

    if (enabledFeatures.size() != requiredFeatures.size()) {
        std::vector<wgpu::FeatureName> missingFeatures;
        std::set_difference(requiredFeatures.begin(), requiredFeatures.end(),
                supportedFeatures.features,
                supportedFeatures.features + supportedFeatures.featureCount,
                std::back_inserter(missingFeatures));

        std::stringstream missingFeaturesStream{};
        for (const auto& entry: missingFeatures) {
            missingFeaturesStream << std::to_string(static_cast<uint32_t>(entry)) << " ";
        }
        PANIC_POSTCONDITION("Some required features are not available %s/n",
                missingFeaturesStream.str().c_str());
    }

    std::set_intersection(supportedFeatures.features,
            supportedFeatures.features + supportedFeatures.featureCount, optionalFeatures.begin(),
            optionalFeatures.end(), std::back_inserter(enabledFeatures));

    wgpu::DeviceDescriptor deviceDescriptor{};
    deviceDescriptor.label = "graphics_device";
    deviceDescriptor.defaultQueue.label = "default_queue";
    deviceDescriptor.requiredFeatureCount = enabledFeatures.size();
    deviceDescriptor.requiredFeatures = enabledFeatures.data();
    deviceDescriptor.SetDeviceLostCallback(wgpu::CallbackMode::AllowSpontaneous,
            [](wgpu::Device const&, wgpu::DeviceLostReason const& reason,
                    wgpu::StringView message) {
                if (reason == wgpu::DeviceLostReason::Destroyed) {
                    FWGPU_LOGD << "WebGPU device lost due to being destroyed (expected)"
                               << utils::io::endl;
                    return;
                }
                // TODO try recreating the device instead of just panicking
                std::stringstream reasonStream{};
                reasonStream << reason;
                FILAMENT_CHECK_POSTCONDITION(reason != wgpu::DeviceLostReason::Destroyed)
                        << "WebGPU device lost: " << reasonStream.str() << " " << message.data;
            });
    deviceDescriptor.SetUncapturedErrorCallback(
            [](wgpu::Device const&, wgpu::ErrorType errorType, wgpu::StringView message) {
                std::stringstream typeStream{};
                typeStream << errorType;
                FWGPU_LOGE << "WebGPU device error: " << typeStream.str() << " " << message.data
                           << utils::io::endl;
            });
    wgpu::Device device = nullptr;
    wgpu::WaitStatus status = mInstance.WaitAny(
            adapter.RequestDevice(&deviceDescriptor, wgpu::CallbackMode::WaitAnyOnly,
                    [&device](wgpu::RequestDeviceStatus const status,
                            wgpu::Device const& readyDevice, wgpu::StringView const message) {
                        FILAMENT_CHECK_POSTCONDITION(status == wgpu::RequestDeviceStatus::Success)
                                << "Unable to request a WebGPU device. Status: "
                                << static_cast<uint32_t>(status)
                                << " with message: " << message.data;
                        device = readyDevice;
                    }),
            UINT64_MAX);// TODO define reasonable timeout (or do this asynchronously)
    FILAMENT_CHECK_POSTCONDITION(status == wgpu::WaitStatus::Success)
            << "Non-successful wait status requesting a WebGPU device "
            << static_cast<uint32_t>(status);
    FILAMENT_CHECK_POSTCONDITION(device != nullptr)
            << "Failed to get a WebGPU device for the platform.";
    return device;
}

Driver* WebGPUPlatform::createDriver(void* sharedContext,
        const Platform::DriverConfig& driverConfig) noexcept {
    if (sharedContext) {
        FWGPU_LOGW << "sharedContext is ignored/unused in the WebGPU backend. A non-null "
                      "sharedContext was provided, but it will be ignored."
                   << utils::io::endl;
    }
    return WebGPUDriver::create(*this, driverConfig);
}

WebGPUPlatform::WebGPUPlatform()
    : mInstance(createInstance()) {}

}// namespace filament::backend
