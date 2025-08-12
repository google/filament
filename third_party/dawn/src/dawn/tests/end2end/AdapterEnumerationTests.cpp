// Copyright 2021 The Dawn & Tint Authors
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

#include <webgpu/webgpu_cpp.h>

#include <memory>
#include <utility>

#include "dawn/common/GPUInfo.h"
#include "dawn/common/StringViewUtils.h"
#include "dawn/dawn_proc.h"
#include "dawn/native/DawnNative.h"

#if defined(DAWN_ENABLE_BACKEND_VULKAN)
// This must be above VulkanBackend.h otherwise vulkan.h will be included before we can wrap it with
// vulkan_platform.h.
#include "dawn/common/vulkan_platform.h"
#include "dawn/native/VulkanBackend.h"
#endif  // defined(DAWN_ENABLE_BACKEND_VULKAN)

#if defined(DAWN_ENABLE_BACKEND_D3D11)
#include "dawn/native/D3D11Backend.h"
#endif  // defined(DAWN_ENABLE_BACKEND_D3D11)

#if defined(DAWN_ENABLE_BACKEND_D3D12)
#include "dawn/native/D3D12Backend.h"
#endif  // defined(DAWN_ENABLE_BACKEND_D3D12)

#if defined(DAWN_ENABLE_BACKEND_OPENGL)
#include "dawn/native/OpenGLBackend.h"
#endif  // defined(DAWN_ENABLE_BACKEND_OPENGL)

#include <gtest/gtest.h>

namespace dawn {
namespace {

class AdapterEnumerationTests : public ::testing::Test {
    void SetUp() override { dawnProcSetProcs(&dawn::native::GetProcs()); }
};

// Test only enumerating the fallback adapters
TEST_F(AdapterEnumerationTests, OnlyFallback) {
    native::Instance instance;

    wgpu::RequestAdapterOptions adapterOptions = {};
    adapterOptions.forceFallbackAdapter = true;

    const auto& adapters = instance.EnumerateAdapters(&adapterOptions);
    for (const auto& nativeAdapter : adapters) {
        wgpu::Adapter adapter = wgpu::Adapter(nativeAdapter.Get());
        wgpu::AdapterInfo info;
        adapter.GetInfo(&info);

        EXPECT_TRUE(info.backendType == wgpu::BackendType::Vulkan ||
                    info.backendType == wgpu::BackendType::WebGPU);
        EXPECT_EQ(info.adapterType, wgpu::AdapterType::CPU);
        EXPECT_TRUE(gpu_info::IsGoogleSwiftshader(info.vendorID, info.deviceID));
    }
}

// Test enumerating only Vulkan physical devices
TEST_F(AdapterEnumerationTests, OnlyVulkan) {
    native::Instance instance;

    wgpu::RequestAdapterOptions adapterOptions = {};
    adapterOptions.backendType = wgpu::BackendType::Vulkan;

    const auto& adapters = instance.EnumerateAdapters(&adapterOptions);
    for (const auto& nativeAdapter : adapters) {
        wgpu::Adapter adapter = wgpu::Adapter(nativeAdapter.Get());
        wgpu::AdapterInfo info;
        adapter.GetInfo(&info);

        EXPECT_EQ(info.backendType, wgpu::BackendType::Vulkan);
    }
}

// Test enumerating only D3D11 physical devices
TEST_F(AdapterEnumerationTests, OnlyD3D11) {
    native::Instance instance;

    wgpu::RequestAdapterOptions adapterOptions = {};
    adapterOptions.backendType = wgpu::BackendType::D3D11;

    const auto& adapters = instance.EnumerateAdapters(&adapterOptions);
    for (const auto& nativeAdapter : adapters) {
        wgpu::Adapter adapter = wgpu::Adapter(nativeAdapter.Get());
        wgpu::AdapterInfo info;
        adapter.GetInfo(&info);

        EXPECT_EQ(info.backendType, wgpu::BackendType::D3D11);
    }
}

#if defined(DAWN_ENABLE_BACKEND_D3D11)
// Test enumerating a D3D11 physical device from a prexisting DXGI adapter
TEST_F(AdapterEnumerationTests, MatchingDXGIAdapterD3D11) {
    using Microsoft::WRL::ComPtr;

    ComPtr<IDXGIFactory4> dxgiFactory;
    HRESULT hr = ::CreateDXGIFactory2(0, IID_PPV_ARGS(&dxgiFactory));
    ASSERT_EQ(hr, S_OK);

    for (uint32_t adapterIndex = 0;; ++adapterIndex) {
        ComPtr<IDXGIAdapter1> dxgiAdapter = nullptr;
        if (dxgiFactory->EnumAdapters1(adapterIndex, &dxgiAdapter) == DXGI_ERROR_NOT_FOUND) {
            break;  // No more adapters to enumerate.
        }

        native::Instance instance;

        DXGI_ADAPTER_DESC adapterDesc;
        dxgiAdapter->GetDesc(&adapterDesc);

        native::d3d::RequestAdapterOptionsLUID luidOptions = {};
        luidOptions.adapterLUID = adapterDesc.AdapterLuid;

        wgpu::RequestAdapterOptions adapterOptions = {};
        adapterOptions.backendType = wgpu::BackendType::D3D11;
        adapterOptions.nextInChain = &luidOptions;

        const auto& nativeAdapters = instance.EnumerateAdapters(&adapterOptions);
        if (nativeAdapters.empty()) {
            // Initialize of the backend may fail.
            continue;
        }
        ASSERT_EQ(nativeAdapters.size(), 1u);

        wgpu::Adapter adapter = wgpu::Adapter(nativeAdapters[0].Get());
        wgpu::AdapterInfo info;
        adapter.GetInfo(&info);
        EXPECT_EQ(info.backendType, wgpu::BackendType::D3D11);

        // Test that enumeration again yields the same adapter device.
        const auto& nativeAdaptersAgain = instance.EnumerateAdapters(&adapterOptions);
        ASSERT_EQ(nativeAdaptersAgain.size(), 1u);

        wgpu::Adapter adapterAgain = wgpu::Adapter(nativeAdaptersAgain[0].Get());
        wgpu::AdapterInfo infoAgain;
        adapterAgain.GetInfo(&infoAgain);

        EXPECT_EQ(info.vendor, infoAgain.vendor);
        EXPECT_EQ(info.architecture, infoAgain.architecture);
        EXPECT_EQ(info.device, infoAgain.device);
        EXPECT_EQ(info.description, infoAgain.description);
        EXPECT_EQ(info.backendType, infoAgain.backendType);
        EXPECT_EQ(info.adapterType, infoAgain.adapterType);
        EXPECT_EQ(info.vendorID, infoAgain.vendorID);
        EXPECT_EQ(info.deviceID, infoAgain.deviceID);
    }
}
#endif  // defined(DAWN_ENABLE_BACKEND_D3D11)

// Test enumerating only D3D12 physical devices
TEST_F(AdapterEnumerationTests, OnlyD3D12) {
    native::Instance instance;

    wgpu::RequestAdapterOptions adapterOptions = {};
    adapterOptions.backendType = wgpu::BackendType::D3D12;

    const auto& adapters = instance.EnumerateAdapters(&adapterOptions);
    for (const auto& nativeAdapter : adapters) {
        wgpu::Adapter adapter = wgpu::Adapter(nativeAdapter.Get());
        wgpu::AdapterInfo info;
        adapter.GetInfo(&info);

        EXPECT_EQ(info.backendType, wgpu::BackendType::D3D12);
    }
}

#if defined(DAWN_ENABLE_BACKEND_D3D12)
// Test enumerating a D3D12 physical device from a prexisting DXGI adapter
TEST_F(AdapterEnumerationTests, MatchingDXGIAdapterD3D12) {
    using Microsoft::WRL::ComPtr;

    ComPtr<IDXGIFactory4> dxgiFactory;
    HRESULT hr = ::CreateDXGIFactory2(0, IID_PPV_ARGS(&dxgiFactory));
    ASSERT_EQ(hr, S_OK);

    for (uint32_t adapterIndex = 0;; ++adapterIndex) {
        ComPtr<IDXGIAdapter1> dxgiAdapter = nullptr;
        if (dxgiFactory->EnumAdapters1(adapterIndex, &dxgiAdapter) == DXGI_ERROR_NOT_FOUND) {
            break;  // No more adapters to enumerate.
        }

        native::Instance instance;

        DXGI_ADAPTER_DESC adapterDesc;
        dxgiAdapter->GetDesc(&adapterDesc);

        native::d3d::RequestAdapterOptionsLUID luidOptions = {};
        luidOptions.adapterLUID = adapterDesc.AdapterLuid;

        wgpu::RequestAdapterOptions adapterOptions = {};
        adapterOptions.backendType = wgpu::BackendType::D3D12;
        adapterOptions.nextInChain = &luidOptions;

        const auto& nativeAdapters = instance.EnumerateAdapters(&adapterOptions);
        if (nativeAdapters.empty()) {
            // Initialize of the backend may fail.
            continue;
        }
        ASSERT_EQ(nativeAdapters.size(), 1u);

        wgpu::Adapter adapter = wgpu::Adapter(nativeAdapters[0].Get());
        wgpu::AdapterInfo info;
        adapter.GetInfo(&info);
        EXPECT_EQ(info.backendType, wgpu::BackendType::D3D12);

        // Test that enumeration again yields the same adapter device.
        const auto& nativeAdaptersAgain = instance.EnumerateAdapters(&adapterOptions);
        ASSERT_EQ(nativeAdaptersAgain.size(), 1u);

        wgpu::Adapter adaptersAgain = wgpu::Adapter(nativeAdaptersAgain[0].Get());
        wgpu::AdapterInfo infoAgain;
        adaptersAgain.GetInfo(&infoAgain);

        EXPECT_EQ(info.vendor, infoAgain.vendor);
        EXPECT_EQ(info.architecture, infoAgain.architecture);
        EXPECT_EQ(info.device, infoAgain.device);
        EXPECT_EQ(info.description, infoAgain.description);
        EXPECT_EQ(info.backendType, infoAgain.backendType);
        EXPECT_EQ(info.adapterType, infoAgain.adapterType);
        EXPECT_EQ(info.vendorID, infoAgain.vendorID);
        EXPECT_EQ(info.deviceID, infoAgain.deviceID);
    }
}
#endif  // defined(DAWN_ENABLE_BACKEND_D3D12)

// Test enumerating only Metal physical devices
TEST_F(AdapterEnumerationTests, OnlyMetal) {
    native::Instance instance;

    wgpu::RequestAdapterOptions adapterOptions = {};
    adapterOptions.backendType = wgpu::BackendType::Metal;

    const auto& adapters = instance.EnumerateAdapters(&adapterOptions);
    for (const auto& nativeAdapter : adapters) {
        wgpu::Adapter adapter = wgpu::Adapter(nativeAdapter.Get());
        wgpu::AdapterInfo info;
        adapter.GetInfo(&info);

        EXPECT_EQ(info.backendType, wgpu::BackendType::Metal);
    }
}

// Test enumerating the Metal backend, then the Vulkan backend
// does not duplicate physical devices.
TEST_F(AdapterEnumerationTests, OneBackendThenTheOther) {
    wgpu::RequestAdapterOptions adapterOptions = {};
    adapterOptions.backendType = wgpu::BackendType::Metal;

    native::Instance instance;

    // Enumerate metal adapters. We should only see metal adapters.
    uint32_t metalAdapterCount = 0;
    {
        const auto& adapters = instance.EnumerateAdapters(&adapterOptions);
        metalAdapterCount = adapters.size();
        for (const auto& nativeAdapter : adapters) {
            wgpu::Adapter adapter = wgpu::Adapter(nativeAdapter.Get());
            wgpu::AdapterInfo info;
            adapter.GetInfo(&info);

            ASSERT_EQ(info.backendType, wgpu::BackendType::Metal);
        }
    }
    // Enumerate vulkan adapters. We should only see vulkan adapters.
    {
        adapterOptions.backendType = wgpu::BackendType::Vulkan;

        const auto& adapters = instance.EnumerateAdapters(&adapterOptions);
        for (const auto& nativeAdapter : adapters) {
            wgpu::Adapter adapter = wgpu::Adapter(nativeAdapter.Get());
            wgpu::AdapterInfo info;
            adapter.GetInfo(&info);

            ASSERT_EQ(info.backendType, wgpu::BackendType::Vulkan);
        }
    }

    // Enumerate metal adapters. We should see the same number of metal adapters.
    {
        adapterOptions.backendType = wgpu::BackendType::Metal;

        const auto& adapters = instance.EnumerateAdapters(&adapterOptions);
        uint32_t metalAdapterCount2 = adapters.size();
        for (const auto& nativeAdapter : adapters) {
            wgpu::Adapter adapter = wgpu::Adapter(nativeAdapter.Get());
            wgpu::AdapterInfo info;
            adapter.GetInfo(&info);

            ASSERT_EQ(info.backendType, wgpu::BackendType::Metal);
        }
        EXPECT_EQ(metalAdapterCount, metalAdapterCount2);
    }
}

#if defined(DAWN_ENABLE_BACKEND_WEBGPU)
// Test enumerating the WebGPU backend with the RequestAdapterWebGPUBackendOptions.
TEST_F(AdapterEnumerationTests, WebGPUBackend) {
    native::Instance instance;

    wgpu::RequestAdapterOptions adapterOptions = {};

    wgpu::RequestAdapterWebGPUBackendOptions webgpuBackendOptions = {};
    adapterOptions.nextInChain = &webgpuBackendOptions;

    // Test selecting without specifying the implementation backend type.
    {
        adapterOptions.backendType = wgpu::BackendType::Undefined;
        const auto& adapters = instance.EnumerateAdapters(&adapterOptions);
        EXPECT_TRUE(adapters.size() > 0);
        for (const auto& nativeAdapter : adapters) {
            wgpu::Adapter adapter = wgpu::Adapter(nativeAdapter.Get());
            wgpu::AdapterInfo info;
            adapter.GetInfo(&info);

            EXPECT_EQ(info.backendType, wgpu::BackendType::WebGPU);
        }
    }

    // Test selecting a specific implementation backend type.
    {
        adapterOptions.backendType = wgpu::BackendType::Vulkan;
        const auto& adapters = instance.EnumerateAdapters(&adapterOptions);
        for (const auto& nativeAdapter : adapters) {
            wgpu::Adapter adapter = wgpu::Adapter(nativeAdapter.Get());
            wgpu::AdapterInfo info;
            adapter.GetInfo(&info);

            EXPECT_EQ(info.backendType, wgpu::BackendType::WebGPU);
            EXPECT_NE(std::string_view(info.device.data, info.device.length).find("Vulkan"),
                      std::string_view::npos);
        }
    }

    // Test selecting WebGPUBackend on WebGPUBackend gives nothing.
    {
        adapterOptions.backendType = wgpu::BackendType::WebGPU;
        const auto& adapters = instance.EnumerateAdapters(&adapterOptions);
        EXPECT_TRUE(adapters.empty());
    }

    // Test selecting WebGPUBackend without RequestAdapterWebGPUBackendOptions gives nothing.
    {
        adapterOptions.backendType = wgpu::BackendType::WebGPU;
        adapterOptions.nextInChain = nullptr;
        const auto& adapters = instance.EnumerateAdapters(&adapterOptions);
        EXPECT_TRUE(adapters.empty());
    }
}
#endif  // defined(DAWN_ENABLE_BACKEND_WEBGPU)

}  // anonymous namespace
}  // namespace dawn
