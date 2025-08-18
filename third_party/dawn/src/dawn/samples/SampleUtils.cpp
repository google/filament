// Copyright 2017 The Dawn & Tint Authors
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

#include "dawn/samples/SampleUtils.h"

#include <algorithm>
#include <cstring>
#include <iostream>
#include <memory>
#include <string>
#include <string_view>
#include <utility>
#include <vector>

#include "dawn/common/Assert.h"
#include "dawn/common/Log.h"
#include "dawn/common/Platform.h"
#include "dawn/common/SystemUtils.h"
#include "dawn/utils/CommandLineParser.h"
#include "dawn/utils/SystemUtils.h"
#include "dawn/utils/WGPUHelpers.h"
#include "dawn/webgpu_cpp_print.h"

#ifndef __EMSCRIPTEN__
#include "GLFW/glfw3.h"
#include "dawn/dawn_proc.h"  // nogncheck
#include "dawn/native/DawnNative.h"
#include "webgpu/webgpu_glfw.h"
#else
#include <emscripten/emscripten.h>
#endif  // __EMSCRIPTEN__

// Parsed options.
static wgpu::BackendType backendType = wgpu::BackendType::Undefined;
static wgpu::AdapterType adapterType = wgpu::AdapterType::Unknown;
static std::vector<std::string> enableToggles;
static std::vector<std::string> disableToggles;

bool InitSample(int argc, const char** argv) {
    dawn::utils::CommandLineParser opts;
    auto& helpOpt = opts.AddHelp();
    auto& enableTogglesOpt = opts.AddStringList("enable-toggles", "Toggles to enable in Dawn")
                                 .ShortName('e')
                                 .Parameter("comma separated list");
    auto& disableTogglesOpt = opts.AddStringList("disable-toggles", "Toggles to disable in Dawn")
                                  .ShortName('d')
                                  .Parameter("comma separated list");
    auto& backendOpt =
        opts.AddEnum<wgpu::BackendType>({{"d3d11", wgpu::BackendType::D3D11},
                                         {"d3d12", wgpu::BackendType::D3D12},
                                         {"metal", wgpu::BackendType::Metal},
                                         {"null", wgpu::BackendType::Null},
                                         {"opengl", wgpu::BackendType::OpenGL},
                                         {"opengles", wgpu::BackendType::OpenGLES},
                                         {"vulkan", wgpu::BackendType::Vulkan}},
                                        "backend", "The backend to get an adapter from")
            .ShortName('b')
            .Default(wgpu::BackendType::Undefined);
    auto& adapterTypeOpt = opts.AddEnum<wgpu::AdapterType>(
                                   {
                                       {"discrete", wgpu::AdapterType::DiscreteGPU},
                                       {"integrated", wgpu::AdapterType::IntegratedGPU},
                                       {"cpu", wgpu::AdapterType::CPU},
                                   },
                                   "adapter-type", "The type of adapter to request")
                               .ShortName('a')
                               .Default(wgpu::AdapterType::Unknown);

    auto result = opts.Parse(argc, argv);
    if (!result.success) {
        std::cerr << result.errorMessage << "\n";
        return false;
    }

    if (helpOpt.GetValue()) {
        std::cout << "Usage: " << argv[0] << " <options>\n\noptions\n";
        opts.PrintHelp(std::cout);
        return false;
    }

    backendType = backendOpt.GetValue();
    adapterType = adapterTypeOpt.GetValue();
    enableToggles = enableTogglesOpt.GetOwnedValue();
    disableToggles = disableTogglesOpt.GetOwnedValue();
    return true;
}

// Global state
static SampleBase* sample = nullptr;

SampleBase::SampleBase() {
    sample = this;
}

SampleBase::SampleBase(uint32_t w, uint32_t h) : width(w), height(h) {
    sample = this;
}

int SampleBase::Run(unsigned int delay) {
    dawn::ScopedEnvironmentVar angleDefaultPlatform;
    if (dawn::GetEnvironmentVar("ANGLE_DEFAULT_PLATFORM").first.empty()) {
        angleDefaultPlatform.Set("ANGLE_DEFAULT_PLATFORM", "swiftshader");
    }

    // Create the toggles descriptor if not using emscripten.
    wgpu::ChainedStruct* togglesChain = nullptr;
#ifndef __EMSCRIPTEN__
    std::vector<const char*> enableToggleNames;
    std::vector<const char*> disabledToggleNames;
    for (const std::string& toggle : enableToggles) {
        enableToggleNames.push_back(toggle.c_str());
    }
    for (const std::string& toggle : disableToggles) {
        disabledToggleNames.push_back(toggle.c_str());
    }

    wgpu::DawnTogglesDescriptor toggles = {};
    toggles.enabledToggles = enableToggleNames.data();
    toggles.enabledToggleCount = enableToggleNames.size();
    toggles.disabledToggles = disabledToggleNames.data();
    toggles.disabledToggleCount = disabledToggleNames.size();

    togglesChain = &toggles;
#endif  // __EMSCRIPTEN__

    // Setup base adapter options with toggles.
    wgpu::RequestAdapterOptions adapterOptions = {};
    adapterOptions.nextInChain = togglesChain;
    adapterOptions.backendType = backendType;
    if (backendType != wgpu::BackendType::Undefined) {
        adapterOptions.featureLevel = dawn::utils::BackendRequiresCompat(backendType)
                                          ? wgpu::FeatureLevel::Compatibility
                                          : wgpu::FeatureLevel::Core;
    }

    switch (adapterType) {
        case wgpu::AdapterType::CPU:
            adapterOptions.forceFallbackAdapter = true;
            break;
        case wgpu::AdapterType::DiscreteGPU:
            adapterOptions.powerPreference = wgpu::PowerPreference::HighPerformance;
            break;
        case wgpu::AdapterType::IntegratedGPU:
            adapterOptions.powerPreference = wgpu::PowerPreference::LowPower;
            break;
        case wgpu::AdapterType::Unknown:
            break;
    }

#ifndef __EMSCRIPTEN__
    dawnProcSetProcs(&dawn::native::GetProcs());
#endif  // __EMSCRIPTEN__

    // Create the instance with the toggles
    wgpu::InstanceDescriptor instanceDescriptor = {};
    instanceDescriptor.nextInChain = togglesChain;
    static constexpr auto kTimedWaitAny = wgpu::InstanceFeatureName::TimedWaitAny;
    instanceDescriptor.requiredFeatureCount = 1;
    instanceDescriptor.requiredFeatures = &kTimedWaitAny;
    sample->instance = wgpu::CreateInstance(&instanceDescriptor);

    // Synchronously create the adapter
    sample->instance.WaitAny(
        sample->instance.RequestAdapter(
            &adapterOptions, wgpu::CallbackMode::WaitAnyOnly,
            [](wgpu::RequestAdapterStatus status, wgpu::Adapter adapter, wgpu::StringView message) {
                if (status != wgpu::RequestAdapterStatus::Success) {
                    dawn::ErrorLog() << "Failed to get an adapter: " << message;
                    return;
                }
                sample->adapter = std::move(adapter);
            }),
        UINT64_MAX);
    if (sample->adapter == nullptr) {
        return 1;
    }
    wgpu::AdapterInfo info;
    sample->adapter.GetInfo(&info);
    dawn::InfoLog() << "Adaptor info:";
    dawn::InfoLog() << "  vendor: \"" << info.vendor << "\"";
    dawn::InfoLog() << "  architecture: \"" << info.architecture << "\"";
    dawn::InfoLog() << "  device: \"" << info.device << "\"";
    dawn::InfoLog() << "  subgroupSizes: { min: " << info.subgroupMinSize
                    << " max: " << info.subgroupMaxSize << " }";

    // Create device descriptor with callbacks and toggles
    wgpu::DeviceDescriptor deviceDesc = {};
    deviceDesc.nextInChain = togglesChain;
    deviceDesc.SetDeviceLostCallback(
        wgpu::CallbackMode::AllowSpontaneous,
        [](const wgpu::Device&, wgpu::DeviceLostReason reason, wgpu::StringView message) {
            const char* reasonName = "";
            switch (reason) {
                case wgpu::DeviceLostReason::Unknown:
                    reasonName = "Unknown";
                    break;
                case wgpu::DeviceLostReason::Destroyed:
                    reasonName = "Destroyed";
                    break;
                case wgpu::DeviceLostReason::CallbackCancelled:
                    reasonName = "CallbackCancelled";
                    break;
                case wgpu::DeviceLostReason::FailedCreation:
                    reasonName = "FailedCreation";
                    break;
                default:
                    DAWN_UNREACHABLE();
            }
            dawn::ErrorLog() << "Device lost because of " << reasonName << ": " << message;
        });
    deviceDesc.SetUncapturedErrorCallback(
        [](const wgpu::Device&, wgpu::ErrorType type, wgpu::StringView message) {
            const char* errorTypeName = "";
            switch (type) {
                case wgpu::ErrorType::Validation:
                    errorTypeName = "Validation";
                    break;
                case wgpu::ErrorType::OutOfMemory:
                    errorTypeName = "Out of memory";
                    break;
                case wgpu::ErrorType::Internal:
                    errorTypeName = "Internal";
                    break;
                case wgpu::ErrorType::Unknown:
                    errorTypeName = "Unknown";
                    break;
                default:
                    DAWN_UNREACHABLE();
            }
            dawn::ErrorLog() << errorTypeName << " error: " << message;
        });

    // Synchronously create the device
    sample->instance.WaitAny(
        sample->adapter.RequestDevice(
            &deviceDesc, wgpu::CallbackMode::WaitAnyOnly,
            [](wgpu::RequestDeviceStatus status, wgpu::Device device, wgpu::StringView message) {
                if (status != wgpu::RequestDeviceStatus::Success) {
                    dawn::ErrorLog() << "Failed to get an device: " << message;
                    return;
                }
                sample->device = std::move(device);
                sample->queue = sample->device.GetQueue();
            }),
        UINT64_MAX);
    if (sample->device == nullptr) {
        return 1;
    }

#ifndef __EMSCRIPTEN__
    if (!sample->Setup()) {
        dawn::ErrorLog() << "Failed to perform sample setup";
        return 1;
    }

    while (!glfwWindowShouldClose(sample->window)) {
        sample->FrameImpl();
        wgpu::Status presentStatus = sample->surface.Present();
        DAWN_ASSERT(presentStatus == wgpu::Status::Success);
        glfwPollEvents();
        if (delay) {
            dawn::utils::USleep(delay);
        }
    }
#else
    if (sample->Setup()) {
        emscripten_set_main_loop([]() { sample->FrameImpl(); }, 0, false);
    } else {
        dawn::ErrorLog() << "Failed to setup sample";
    }
#endif  // __EMSCRIPTEN__

    return 0;
}

bool SampleBase::Setup() {
#ifndef __EMSCRIPTEN__
    glfwSetErrorCallback([](int code, const char* message) {
        dawn::ErrorLog() << "GLFW error: " << code << " - " << message;
    });

    if (!glfwInit()) {
        return false;
    }

    // Create the test window with no client API.
    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
    window = glfwCreateWindow(width, height, "Dawn window", nullptr, nullptr);
    if (!window) {
        return false;
    }

    // Create the surface.
    surface = wgpu::glfw::CreateSurfaceForWindow(instance, window);
#else
    // Create the surface.
    wgpu::EmscriptenSurfaceSourceCanvasHTMLSelector canvasDesc{};
    canvasDesc.selector = "#canvas";

    wgpu::SurfaceDescriptor surfaceDesc = {};
    surfaceDesc.nextInChain = &canvasDesc;
    surface = instance.CreateSurface(&surfaceDesc);
#endif

    // Configure the surface.
    wgpu::SurfaceCapabilities capabilities;
    surface.GetCapabilities(adapter, &capabilities);
    wgpu::SurfaceConfiguration config = {};
    config.device = device;
    config.format = capabilities.formats[0];
    config.width = width;
    config.height = height;
    DAWN_ASSERT(capabilities.presentModeCount > 0);
    config.presentMode = capabilities.presentModes[0];
    surface.Configure(&config);
    this->preferredSurfaceTextureFormat = capabilities.formats[0];

    return SetupImpl();
}
