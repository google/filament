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

#include "dawn/tests/AdapterTestConfig.h"

#include <webgpu/webgpu_cpp.h>

#include <initializer_list>
#include <ostream>
#include <string>
#include <vector>

#include "dawn/common/Assert.h"

BackendTestConfig::BackendTestConfig(wgpu::BackendType backendType,
                                     std::initializer_list<const char*> forceEnabledWorkarounds,
                                     std::initializer_list<const char*> forceDisabledWorkarounds)
    : backendType(backendType),
      forceEnabledWorkarounds(forceEnabledWorkarounds),
      forceDisabledWorkarounds(forceDisabledWorkarounds) {}

BackendTestConfig D3D11Backend(std::initializer_list<const char*> forceEnabledWorkarounds,
                               std::initializer_list<const char*> forceDisabledWorkarounds) {
    return BackendTestConfig(wgpu::BackendType::D3D11, forceEnabledWorkarounds,
                             forceDisabledWorkarounds);
}

BackendTestConfig D3D12Backend(std::initializer_list<const char*> forceEnabledWorkarounds,
                               std::initializer_list<const char*> forceDisabledWorkarounds) {
    return BackendTestConfig(wgpu::BackendType::D3D12, forceEnabledWorkarounds,
                             forceDisabledWorkarounds);
}

BackendTestConfig MetalBackend(std::initializer_list<const char*> forceEnabledWorkarounds,
                               std::initializer_list<const char*> forceDisabledWorkarounds) {
    return BackendTestConfig(wgpu::BackendType::Metal, forceEnabledWorkarounds,
                             forceDisabledWorkarounds);
}

BackendTestConfig NullBackend(std::initializer_list<const char*> forceEnabledWorkarounds,
                              std::initializer_list<const char*> forceDisabledWorkarounds) {
    return BackendTestConfig(wgpu::BackendType::Null, forceEnabledWorkarounds,
                             forceDisabledWorkarounds);
}

BackendTestConfig WebGPUBackend(std::initializer_list<const char*> forceEnabledWorkarounds,
                                std::initializer_list<const char*> forceDisabledWorkarounds) {
    return BackendTestConfig(wgpu::BackendType::WebGPU, forceEnabledWorkarounds,
                             forceDisabledWorkarounds);
}

BackendTestConfig OpenGLBackend(std::initializer_list<const char*> forceEnabledWorkarounds,
                                std::initializer_list<const char*> forceDisabledWorkarounds) {
    return BackendTestConfig(wgpu::BackendType::OpenGL, forceEnabledWorkarounds,
                             forceDisabledWorkarounds);
}

BackendTestConfig OpenGLESBackend(std::initializer_list<const char*> forceEnabledWorkarounds,
                                  std::initializer_list<const char*> forceDisabledWorkarounds) {
    return BackendTestConfig(wgpu::BackendType::OpenGLES, forceEnabledWorkarounds,
                             forceDisabledWorkarounds);
}

BackendTestConfig VulkanBackend(std::initializer_list<const char*> forceEnabledWorkarounds,
                                std::initializer_list<const char*> forceDisabledWorkarounds) {
    return BackendTestConfig(wgpu::BackendType::Vulkan, forceEnabledWorkarounds,
                             forceDisabledWorkarounds);
}

TestAdapterProperties::TestAdapterProperties(const wgpu::AdapterInfo& info,
                                             bool selected,
                                             bool compatibilityMode)
    : vendorID(info.vendorID),
      vendorName(info.vendor),
      architecture(info.architecture),
      deviceID(info.deviceID),
      name(info.device),
      driverDescription(info.description),
      adapterType(info.adapterType),
      backendType(info.backendType),
      compatibilityMode(compatibilityMode),
      selected(selected) {}

std::string TestAdapterProperties::ParamName() const {
    switch (backendType) {
        case wgpu::BackendType::D3D11:
            return "D3D11";
        case wgpu::BackendType::D3D12:
            return "D3D12";
        case wgpu::BackendType::Metal:
            return "Metal";
        case wgpu::BackendType::Null:
            return "Null";
        case wgpu::BackendType::OpenGL:
            return "OpenGL";
        case wgpu::BackendType::OpenGLES:
            return "OpenGLES";
        case wgpu::BackendType::Vulkan:
            return "Vulkan";
        case wgpu::BackendType::WebGPU:
            return "WebGPU";
        case wgpu::BackendType::Undefined:
        default:
            DAWN_UNREACHABLE();
    }
}

std::string TestAdapterProperties::AdapterTypeName() const {
    switch (adapterType) {
        case wgpu::AdapterType::DiscreteGPU:
            return "Discrete GPU";
        case wgpu::AdapterType::IntegratedGPU:
            return "Integrated GPU";
        case wgpu::AdapterType::CPU:
            return "CPU";
        case wgpu::AdapterType::Unknown:
            return "Unknown";
        default:
            DAWN_UNREACHABLE();
    }
}

AdapterTestParam::AdapterTestParam(const BackendTestConfig& config,
                                   const TestAdapterProperties& adapterProperties)
    : adapterProperties(adapterProperties),
      forceEnabledWorkarounds(config.forceEnabledWorkarounds),
      forceDisabledWorkarounds(config.forceDisabledWorkarounds) {}

std::ostream& operator<<(std::ostream& os, const AdapterTestParam& param) {
    os << param.adapterProperties.ParamName() << " " << param.adapterProperties.name;

    // In a Windows Remote Desktop session there are two adapters named "Microsoft Basic Render
    // Driver" with different adapter types. We must differentiate them to avoid any tests using the
    // same name.
    if (param.adapterProperties.deviceID == 0x008C) {
        std::string adapterType = param.adapterProperties.AdapterTypeName();
        os << " " << adapterType;
    }

    for (const char* forceEnabledWorkaround : param.forceEnabledWorkarounds) {
        os << "; e:" << forceEnabledWorkaround;
    }
    for (const char* forceDisabledWorkaround : param.forceDisabledWorkarounds) {
        os << "; d:" << forceDisabledWorkaround;
    }
    return os;
}
