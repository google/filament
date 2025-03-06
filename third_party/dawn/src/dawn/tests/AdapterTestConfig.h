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

#ifndef SRC_DAWN_TESTS_ADAPTERTESTCONFIG_H_
#define SRC_DAWN_TESTS_ADAPTERTESTCONFIG_H_

#include <stdint.h>
#include <webgpu/webgpu_cpp.h>

#include <initializer_list>
#include <ostream>
#include <string>
#include <vector>

struct BackendTestConfig {
    BackendTestConfig(wgpu::BackendType backendType,
                      std::initializer_list<const char*> forceEnabledWorkarounds = {},
                      std::initializer_list<const char*> forceDisabledWorkarounds = {});

    wgpu::BackendType backendType;

    std::vector<const char*> forceEnabledWorkarounds;
    std::vector<const char*> forceDisabledWorkarounds;
};

struct TestAdapterProperties {
    TestAdapterProperties(const wgpu::AdapterInfo& info, bool selected);
    uint32_t vendorID;
    std::string vendorName;
    std::string architecture;
    uint32_t deviceID;
    std::string name;
    std::string driverDescription;
    wgpu::AdapterType adapterType;
    wgpu::BackendType backendType;
    bool compatibilityMode;
    bool selected;

    std::string ParamName() const;
    std::string AdapterTypeName() const;
};

struct AdapterTestParam {
    AdapterTestParam(const BackendTestConfig& config,
                     const TestAdapterProperties& adapterProperties);

    TestAdapterProperties adapterProperties;
    std::vector<const char*> forceEnabledWorkarounds;
    std::vector<const char*> forceDisabledWorkarounds;
};

std::ostream& operator<<(std::ostream& os, const AdapterTestParam& param);

BackendTestConfig D3D11Backend(std::initializer_list<const char*> forceEnabledWorkarounds = {},
                               std::initializer_list<const char*> forceDisabledWorkarounds = {});

BackendTestConfig D3D12Backend(std::initializer_list<const char*> forceEnabledWorkarounds = {},
                               std::initializer_list<const char*> forceDisabledWorkarounds = {});

BackendTestConfig MetalBackend(std::initializer_list<const char*> forceEnabledWorkarounds = {},
                               std::initializer_list<const char*> forceDisabledWorkarounds = {});

BackendTestConfig NullBackend(std::initializer_list<const char*> forceEnabledWorkarounds = {},
                              std::initializer_list<const char*> forceDisabledWorkarounds = {});

BackendTestConfig OpenGLBackend(std::initializer_list<const char*> forceEnabledWorkarounds = {},
                                std::initializer_list<const char*> forceDisabledWorkarounds = {});

BackendTestConfig OpenGLESBackend(std::initializer_list<const char*> forceEnabledWorkarounds = {},
                                  std::initializer_list<const char*> forceDisabledWorkarounds = {});

BackendTestConfig VulkanBackend(std::initializer_list<const char*> forceEnabledWorkarounds = {},
                                std::initializer_list<const char*> forceDisabledWorkarounds = {});

#endif  // SRC_DAWN_TESTS_ADAPTERTESTCONFIG_H_
