// Copyright 2025 The Dawn & Tint Authors
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

#include <d3d12.h>
#include <dxgi1_4.h>
#include <webgpu/webgpu_cpp.h>
#include <wrl/client.h>

#include <memory>
#include <sstream>
#include <string>
#include <utility>
#include <vector>

#include "dawn/native/D3D12Backend.h"
#include "dawn/native/DawnNative.h"
#include "dawn/tests/white_box/SharedTextureMemoryTests.h"
#include "dawn/utils/SystemHandle.h"

namespace dawn {
namespace {

using Microsoft::WRL::ComPtr;

enum class Mode {
    // TODO(crbug.com/454827192): Add tests for shared handle to D3D12Resources.
    // DXGISharedHandle,
    D3D12Resource,
};

// Backend for testing SharedTextureMemory backed by D3D12 resources. This test
// setup is adapted from SharedTextureMemoryTests_win.cpp.
class BackendD3D12 : public SharedTextureMemoryTestBackend {
  public:
    template <Mode kMode>
    static BackendD3D12* GetInstance() {
        static BackendD3D12 b(kMode);
        return &b;
    }

    std::string Name() const override {
        std::ostringstream ss;
        switch (mMode) {
            case Mode::D3D12Resource: {
                ss << "D3D12Resource";
            } break;
        }

        return ss.str();
    }

    bool SupportsConcurrentRead() const override { return true; }

    ComPtr<ID3D12Device> MakeD3D12Device(const wgpu::Device& device) {
        ComPtr<ID3D12Device> d3d12Device = native::d3d12::GetD3D12Device(device.Get());
        return d3d12Device;
    }

    std::vector<wgpu::FeatureName> RequiredFeatures(const wgpu::Adapter& adapter) const override {
        std::vector<wgpu::FeatureName> features;
        if (adapter.HasFeature(wgpu::FeatureName::SharedFenceDXGISharedHandle)) {
            features.push_back(wgpu::FeatureName::SharedFenceDXGISharedHandle);
        }

        switch (mMode) {
            case Mode::D3D12Resource: {
                features.push_back(wgpu::FeatureName::SharedTextureMemoryD3D12Resource);
                break;
            }
        }

        return features;
    }

    bool UseSharedHandle() const { return false; }

    std::string LabelName(DXGI_FORMAT format, size_t size) const {
        std::stringstream ss;
        ss << "format 0x" << std::hex << format << " size " << size << "x" << size;
        return ss.str();
    }

    wgpu::SharedTextureMemory CreateSharedTextureMemory(const wgpu::Device& device,
                                                        int layerCount) override {
        ComPtr<ID3D12Device> d3d12Device = MakeD3D12Device(device);

        // Create a DX12 resource.
        D3D12_RESOURCE_DESC texDesc = {};
        texDesc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
        texDesc.Alignment = 0;
        texDesc.Width = 16;
        texDesc.Height = 16;
        texDesc.DepthOrArraySize = layerCount;
        texDesc.MipLevels = 1;
        texDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
        texDesc.SampleDesc.Count = 1;
        texDesc.SampleDesc.Quality = 0;
        texDesc.Layout = D3D12_TEXTURE_LAYOUT_UNKNOWN;
        texDesc.Flags = D3D12_RESOURCE_FLAG_ALLOW_RENDER_TARGET |
                        D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS |
                        D3D12_RESOURCE_FLAG_ALLOW_SIMULTANEOUS_ACCESS;

        D3D12_HEAP_PROPERTIES heapProps = {};
        heapProps.Type = D3D12_HEAP_TYPE_DEFAULT;
        heapProps.CPUPageProperty = D3D12_CPU_PAGE_PROPERTY_UNKNOWN;
        heapProps.MemoryPoolPreference = D3D12_MEMORY_POOL_UNKNOWN;
        heapProps.CreationNodeMask = 0;
        heapProps.VisibleNodeMask = 0;

        D3D12_CLEAR_VALUE clearValue = {};
        clearValue.Format = texDesc.Format;
        clearValue.Color[0] = 0.0f;
        clearValue.Color[1] = 0.0f;
        clearValue.Color[2] = 0.0f;
        clearValue.Color[3] = 0.0f;

        ComPtr<ID3D12Resource> d3d12Resource;
        HRESULT hr = d3d12Device->CreateCommittedResource(
            &heapProps, D3D12_HEAP_FLAG_NONE, &texDesc, D3D12_RESOURCE_STATE_COMMON, &clearValue,
            IID_PPV_ARGS(&d3d12Resource));
        DAWN_ASSERT(hr == S_OK);

        switch (mMode) {
            case Mode::D3D12Resource: {
                wgpu::SharedTextureMemoryDescriptor desc;
                native::d3d12::SharedTextureMemoryD3D12ResourceDescriptor bufferDesc;
                bufferDesc.resource = d3d12Resource.Get();
                desc.nextInChain = &bufferDesc;
                desc.label = "D3D12Resource Texture";
                return device.ImportSharedTextureMemory(&desc);
            }
        }
    }

    std::vector<std::vector<wgpu::SharedTextureMemory>> CreatePerDeviceSharedTextureMemories(
        const std::vector<wgpu::Device>& devices,
        int layerCount) override {
        std::vector<std::vector<wgpu::SharedTextureMemory>> memories;
        ComPtr<ID3D12Device> d3d12Device = MakeD3D12Device(devices[0]);

        // Check for D3D12 NV12 sharing support.
        bool supportsNV12Sharing = false;
        D3D12_FEATURE_DATA_D3D12_OPTIONS4 opts4 = {};
        d3d12Device->CheckFeatureSupport(D3D12_FEATURE_D3D12_OPTIONS4, &opts4, sizeof(opts4));
        if (opts4.SharedResourceCompatibilityTier >= D3D12_SHARED_RESOURCE_COMPATIBILITY_TIER_2) {
            supportsNV12Sharing = true;
        }

        struct D3DFormat {
            DXGI_FORMAT format;
            wgpu::FeatureName requiredFeature = wgpu::FeatureName(0u);
        };
        std::vector<D3DFormat> formats = {{
            {DXGI_FORMAT_R16G16B16A16_FLOAT},
            {DXGI_FORMAT_R16G16_FLOAT},
            {DXGI_FORMAT_R16_FLOAT},
            {DXGI_FORMAT_R8G8B8A8_UNORM},
            {DXGI_FORMAT_B8G8R8A8_UNORM},
            {DXGI_FORMAT_R10G10B10A2_UNORM},
            {DXGI_FORMAT_R16G16_UNORM, wgpu::FeatureName::Unorm16TextureFormats},
            {DXGI_FORMAT_R16_UNORM, wgpu::FeatureName::Unorm16TextureFormats},
            {DXGI_FORMAT_R8G8_UNORM},
            {DXGI_FORMAT_R8_UNORM},
        }};

        if (supportsNV12Sharing) {
            formats.push_back({DXGI_FORMAT_NV12});
        }

        for (auto f : formats) {
            for (uint32_t size : {4, 64}) {
                // Create a DX12 resource.
                D3D12_RESOURCE_DESC texDesc = {};
                texDesc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
                texDesc.Alignment = 0;
                texDesc.Width = size;
                texDesc.Height = size;
                texDesc.DepthOrArraySize = layerCount;
                texDesc.MipLevels = 1;
                texDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
                texDesc.SampleDesc.Count = 1;
                texDesc.SampleDesc.Quality = 0;
                texDesc.Layout = D3D12_TEXTURE_LAYOUT_UNKNOWN;
                texDesc.Flags = D3D12_RESOURCE_FLAG_ALLOW_RENDER_TARGET |
                                D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS |
                                D3D12_RESOURCE_FLAG_ALLOW_SIMULTANEOUS_ACCESS;

                D3D12_HEAP_PROPERTIES heapProps = {};
                heapProps.Type = D3D12_HEAP_TYPE_DEFAULT;
                heapProps.CPUPageProperty = D3D12_CPU_PAGE_PROPERTY_UNKNOWN;
                heapProps.MemoryPoolPreference = D3D12_MEMORY_POOL_UNKNOWN;
                heapProps.CreationNodeMask = 0;
                heapProps.VisibleNodeMask = 0;

                D3D12_CLEAR_VALUE clearValue = {};
                clearValue.Format = texDesc.Format;
                clearValue.Color[0] = 0.0f;
                clearValue.Color[1] = 0.0f;
                clearValue.Color[2] = 0.0f;
                clearValue.Color[3] = 0.0f;

                ComPtr<ID3D12Resource> d3d12Resource;
                HRESULT hr = d3d12Device->CreateCommittedResource(
                    &heapProps, D3D12_HEAP_FLAG_NONE, &texDesc, D3D12_RESOURCE_STATE_COMMON,
                    &clearValue, IID_PPV_ARGS(&d3d12Resource));
                DAWN_ASSERT(hr == S_OK);

                std::vector<wgpu::SharedTextureMemory> perDeviceMemories;
                switch (mMode) {
                    case Mode::D3D12Resource: {
                        wgpu::SharedTextureMemoryDescriptor desc;
                        native::d3d12::SharedTextureMemoryD3D12ResourceDescriptor bufferDesc;
                        bufferDesc.resource = d3d12Resource.Get();
                        desc.nextInChain = &bufferDesc;
                        std::string label = LabelName(f.format, size);
                        desc.label = label.c_str();

                        for (auto& deviceObj : devices) {
                            if (f.requiredFeature != wgpu::FeatureName(0u) &&
                                !deviceObj.HasFeature(f.requiredFeature)) {
                                continue;
                            }
                            perDeviceMemories.push_back(deviceObj.ImportSharedTextureMemory(&desc));
                        }
                        break;
                    }
                }
                if (!perDeviceMemories.empty()) {
                    memories.push_back(std::move(perDeviceMemories));
                }
            }
        }
        return memories;
    }

  private:
    explicit BackendD3D12(Mode mode) : mMode(mode) {}

    const Mode mMode;
};

// Test that it is an error to import a shared fence without enabling the feature (D3D12 fence
// path).
TEST_P(SharedTextureMemoryNoFeatureTests, SharedFenceImportWithoutFeatureD3D12) {
    DAWN_TEST_UNSUPPORTED_IF(!IsD3D12());
    auto backend = static_cast<BackendD3D12*>(GetParam().mBackend);
    ComPtr<ID3D12Device> d3d12Device = backend->MakeD3D12Device(device);

    // Create a shareable D3D12 fence.
    ComPtr<ID3D12Fence> d3d12Fence;
    HRESULT hr = d3d12Device->CreateFence(0, D3D12_FENCE_FLAG_SHARED, IID_PPV_ARGS(&d3d12Fence));
    ASSERT_EQ(hr, S_OK);

    // Create a shared handle for the fence.
    utils::SystemHandle fenceSharedHandle;
    hr = d3d12Device->CreateSharedHandle(d3d12Fence.Get(), nullptr, GENERIC_ALL, nullptr,
                                         fenceSharedHandle.GetMut());
    ASSERT_EQ(hr, S_OK);

    // Attempt to import without the feature enabled; should be a device error.
    wgpu::SharedFenceDXGISharedHandleDescriptor sharedHandleDesc;
    sharedHandleDesc.handle = fenceSharedHandle.Get();

    wgpu::SharedFenceDescriptor fenceDesc;
    fenceDesc.nextInChain = &sharedHandleDesc;

    ASSERT_DEVICE_ERROR_MSG(wgpu::SharedFence fence = device.ImportSharedFence(&fenceDesc),
                            testing::HasSubstr("is not enabled"));
}

// Test that a shared handle can be imported, and then exported, using a D3D12 fence.
TEST_P(SharedTextureMemoryTests, SharedFenceSuccessfulImportExportD3D12) {
    DAWN_TEST_UNSUPPORTED_IF(!IsD3D12());
    auto backend = static_cast<BackendD3D12*>(GetParam().mBackend);
    ComPtr<ID3D12Device> d3d12Device = backend->MakeD3D12Device(device);

    // Create a D3D12 fence that can be shared.
    ComPtr<ID3D12Fence> d3d12Fence;
    HRESULT hr = d3d12Device->CreateFence(0, D3D12_FENCE_FLAG_SHARED, IID_PPV_ARGS(&d3d12Fence));
    ASSERT_EQ(hr, S_OK);

    // Export a shared handle for the fence.
    utils::SystemHandle fenceSharedHandle;
    hr = d3d12Device->CreateSharedHandle(d3d12Fence.Get(), nullptr, GENERIC_ALL, nullptr,
                                         fenceSharedHandle.GetMut());
    ASSERT_EQ(hr, S_OK);

    // Import into Dawn as a shared fence.
    wgpu::SharedFenceDXGISharedHandleDescriptor sharedHandleDesc;
    sharedHandleDesc.handle = fenceSharedHandle.Get();

    wgpu::SharedFenceDescriptor fenceDesc;
    fenceDesc.nextInChain = &sharedHandleDesc;

    wgpu::SharedFence fence = device.ImportSharedFence(&fenceDesc);

    // Export info back out.
    wgpu::SharedFenceDXGISharedHandleExportInfo sharedHandleInfo;
    wgpu::SharedFenceExportInfo exportInfo;
    exportInfo.nextInChain = &sharedHandleInfo;
    fence.ExportInfo(&exportInfo);

    EXPECT_EQ(exportInfo.type, wgpu::SharedFenceType::DXGISharedHandle);

    // Open the exported handle again as a second D3D12 fence object.
    ComPtr<ID3D12Fence> d3d12Fence2;
    hr = d3d12Device->OpenSharedHandle(sharedHandleInfo.handle, IID_PPV_ARGS(&d3d12Fence2));
    ASSERT_EQ(hr, S_OK);

    // Verify both fence objects report the same initial completed value.
    uint64_t fenceValue = d3d12Fence->GetCompletedValue();
    EXPECT_EQ(fenceValue, d3d12Fence2->GetCompletedValue());

    // Create a command queue to signal the fence.
    D3D12_COMMAND_QUEUE_DESC queueDesc = {};
    queueDesc.Type = D3D12_COMMAND_LIST_TYPE_DIRECT;
    queueDesc.Priority = D3D12_COMMAND_QUEUE_PRIORITY_NORMAL;
    queueDesc.Flags = D3D12_COMMAND_QUEUE_FLAG_NONE;
    queueDesc.NodeMask = 0;

    ComPtr<ID3D12CommandQueue> queue;
    hr = d3d12Device->CreateCommandQueue(&queueDesc, IID_PPV_ARGS(&queue));
    ASSERT_EQ(hr, S_OK);

    // Signal the fence to a new value.
    hr = queue->Signal(d3d12Fence.Get(), fenceValue + 1);
    ASSERT_EQ(hr, S_OK);

    // Wait on the fence via event.
    utils::SystemHandle ev = utils::SystemHandle::Acquire(
        ::CreateEvent(nullptr, TRUE, FALSE, TEXT("FenceCompleteD3D12")));
    ASSERT_TRUE(ev.IsValid());
    hr = d3d12Fence->SetEventOnCompletion(fenceValue + 1, ev.Get());
    ASSERT_EQ(hr, S_OK);
    ::WaitForSingleObject(ev.Get(), INFINITE);

    // Both fence objects should now report the updated value.
    EXPECT_EQ(fenceValue + 1, d3d12Fence->GetCompletedValue());
    EXPECT_EQ(fenceValue + 1, d3d12Fence2->GetCompletedValue());
}

// Test exporting info from a shared fence created from a D3D12 fence with no chained struct.
// It should be valid and the fence type is exported.
TEST_P(SharedTextureMemoryTests, SharedFenceExportInfoNoChainedStructD3D12) {
    DAWN_TEST_UNSUPPORTED_IF(!IsD3D12());
    auto backend = static_cast<BackendD3D12*>(GetParam().mBackend);
    ComPtr<ID3D12Device> d3d12Device = backend->MakeD3D12Device(device);

    // Create a shareable D3D12 fence.
    ComPtr<ID3D12Fence> d3d12Fence;
    HRESULT hr = d3d12Device->CreateFence(0, D3D12_FENCE_FLAG_SHARED, IID_PPV_ARGS(&d3d12Fence));
    ASSERT_EQ(hr, S_OK);

    // Create a shared handle for the fence.
    utils::SystemHandle fenceSharedHandle;
    hr = d3d12Device->CreateSharedHandle(d3d12Fence.Get(), nullptr, GENERIC_ALL, nullptr,
                                         fenceSharedHandle.GetMut());
    ASSERT_EQ(hr, S_OK);

    // Import into Dawn.
    wgpu::SharedFenceDXGISharedHandleDescriptor sharedHandleDesc;
    sharedHandleDesc.handle = fenceSharedHandle.Get();

    wgpu::SharedFenceDescriptor fenceDesc;
    fenceDesc.nextInChain = &sharedHandleDesc;

    wgpu::SharedFence fence = device.ImportSharedFence(&fenceDesc);

    // Export info with no chained struct.
    wgpu::SharedFenceExportInfo exportInfo;
    exportInfo.nextInChain = nullptr;

    fence.ExportInfo(&exportInfo);
    EXPECT_EQ(exportInfo.type, wgpu::SharedFenceType::DXGISharedHandle);
}

// Test exporting info from a shared fence created from a D3D12 fence with an invalid chained
// struct. It should trigger a device error, but the fence type would still be known internally.
TEST_P(SharedTextureMemoryTests, SharedFenceExportInfoInvalidChainedStructD3D12) {
    DAWN_TEST_UNSUPPORTED_IF(!IsD3D12());
    auto backend = static_cast<BackendD3D12*>(GetParam().mBackend);
    ComPtr<ID3D12Device> d3d12Device = backend->MakeD3D12Device(device);

    // Create a shareable D3D12 fence.
    ComPtr<ID3D12Fence> d3d12Fence;
    HRESULT hr = d3d12Device->CreateFence(0, D3D12_FENCE_FLAG_SHARED, IID_PPV_ARGS(&d3d12Fence));
    ASSERT_EQ(hr, S_OK);

    // Create a shared handle for the fence.
    utils::SystemHandle fenceSharedHandle;
    hr = d3d12Device->CreateSharedHandle(d3d12Fence.Get(), nullptr, GENERIC_ALL, nullptr,
                                         fenceSharedHandle.GetMut());
    ASSERT_EQ(hr, S_OK);

    // Import into Dawn.
    wgpu::SharedFenceDXGISharedHandleDescriptor sharedHandleDesc;
    sharedHandleDesc.handle = fenceSharedHandle.Get();

    wgpu::SharedFenceDescriptor fenceDesc;
    fenceDesc.nextInChain = &sharedHandleDesc;

    wgpu::SharedFence fence = device.ImportSharedFence(&fenceDesc);

    // Provide an invalid chained struct to ExportInfo.
    wgpu::ChainedStructOut otherStruct;
    wgpu::SharedFenceExportInfo exportInfo;
    exportInfo.nextInChain = &otherStruct;

    ASSERT_DEVICE_ERROR(fence.ExportInfo(&exportInfo));
}

DAWN_INSTANTIATE_PREFIXED_TEST_P(D3D12,
                                 SharedTextureMemoryNoFeatureTests,
                                 {D3D12Backend()},
                                 {BackendD3D12::GetInstance<Mode::D3D12Resource>()},
                                 {1});

DAWN_INSTANTIATE_PREFIXED_TEST_P(D3D12,
                                 SharedTextureMemoryTests,
                                 {D3D12Backend()},
                                 {BackendD3D12::GetInstance<Mode::D3D12Resource>()},
                                 {1});

}  // anonymous namespace
}  // namespace dawn
