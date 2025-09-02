// Copyright 2023 The Dawn & Tint Authors
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

#include <d3d11.h>
#include <d3d11_4.h>
#include <d3d12.h>
#include <dxgi1_4.h>
#include <webgpu/webgpu_cpp.h>
#include <wrl/client.h>

#include <memory>
#include <sstream>
#include <string>
#include <utility>
#include <vector>

#include "dawn/native/D3D11Backend.h"
#include "dawn/native/D3DBackend.h"
#include "dawn/native/DawnNative.h"
#include "dawn/tests/white_box/SharedTextureMemoryTests.h"

namespace dawn {
namespace {

using Microsoft::WRL::ComPtr;

enum class Mode {
    DXGISharedHandle,
    D3D11Texture2D,
};

struct BeginState : public SharedTextureMemoryTestBackend::BackendBeginState {
    wgpu::SharedTextureMemoryD3D11BeginState beginState{};
};

class Backend : public SharedTextureMemoryTestBackend {
  public:
    template <Mode kMode>
    static Backend* GetInstance() {
        static Backend b(kMode, /*useKeyedMutex=*/false, /*requiresEndAccessFence=*/true);
        return &b;
    }

    template <Mode kMode>
    static Backend* GetKeyedMutexInstance() {
        static Backend b(kMode, /*useKeyedMutex=*/true, /*requiresEndAccessFence=*/true);
        return &b;
    }

    template <Mode kMode>
    static Backend* GetInstanceWithoutEndAccessFence() {
        static Backend b(kMode, /*useKeyedMutex=*/false,
                         /*requiresEndAccessFence=*/false);
        return &b;
    }

    std::string Name() const override {
        std::ostringstream ss;
        switch (mMode) {
            case Mode::D3D11Texture2D: {
                ss << "D3D11Texture2D";
            } break;
            case Mode::DXGISharedHandle: {
                ss << "DXGISharedHandle";
            } break;
        }
        if (mUseKeyedMutex) {
            ss << " KeyedMutex";
        }

        if (!mRequiresEndAccessFence) {
            ss << " NoEndAccessFence";
        }

        return ss.str();
    }

    bool UseSameDevice() const override {
        return mMode == Mode::D3D11Texture2D || !mRequiresEndAccessFence;
    }
    bool SupportsConcurrentRead() const override { return true; }

    void SetUp(const wgpu::Device& device) override {
        // Without fence support, we can only use keyed mutex.
        DAWN_TEST_UNSUPPORTED_IF(!mUseKeyedMutex && !HasFenceSupport(device));
    }

    std::vector<wgpu::FeatureName> RequiredFeatures(const wgpu::Adapter& adapter) const override {
        std::vector<wgpu::FeatureName> features;

        if (adapter.HasFeature(wgpu::FeatureName::SharedFenceDXGISharedHandle)) {
            features.push_back(wgpu::FeatureName::SharedFenceDXGISharedHandle);
        }

        switch (mMode) {
            case Mode::D3D11Texture2D: {
                features.push_back(wgpu::FeatureName::SharedTextureMemoryD3D11Texture2D);
                break;
            }
            case Mode::DXGISharedHandle: {
                features.push_back(wgpu::FeatureName::SharedTextureMemoryDXGISharedHandle);
                break;
            }
        }

        features.push_back(wgpu::FeatureName::DawnMultiPlanarFormats);

        return features;
    }

    bool HasFenceSupport(const wgpu::Device& device) const {
        auto toggles = dawn::native::GetTogglesUsed(device.Get());
        return std::find_if(toggles.begin(), toggles.end(), [](const char* name) {
                   return strcmp("d3d11_disable_fence", name) == 0;
               }) == toggles.end();
    }

    bool UseKeyedMutex() const { return mUseKeyedMutex; }
    bool UseSharedHandle() const { return mMode == Mode::DXGISharedHandle; }

    ComPtr<ID3D11Device> MakeD3D11Device(const wgpu::Device& device) {
        switch (mMode) {
            case Mode::D3D11Texture2D: {
                return native::d3d11::GetD3D11Device(device.Get());
            }
            case Mode::DXGISharedHandle: {
                ComPtr<IDXGIAdapter> dxgiAdapter =
                    native::d3d::GetDXGIAdapter(device.GetAdapter().Get());

                ComPtr<ID3D11Device> d3d11Device;
                D3D_FEATURE_LEVEL d3dFeatureLevel;
                ComPtr<ID3D11DeviceContext> d3d11DeviceContext;
                HRESULT hr = ::D3D11CreateDevice(
                    dxgiAdapter.Get(), D3D_DRIVER_TYPE_UNKNOWN, nullptr, 0, nullptr, 0,
                    D3D11_SDK_VERSION, &d3d11Device, &d3dFeatureLevel, &d3d11DeviceContext);
                DAWN_ASSERT(hr == S_OK);

                return d3d11Device;
            }
        }
    }

    std::string LabelName(DXGI_FORMAT format, size_t size) const {
        std::stringstream ss;
        ss << "format 0x" << std::hex << format << " size " << size << "x" << size;
        return ss.str();
    }

    // Create one basic shared texture memory. It should support most operations.
    wgpu::SharedTextureMemory CreateSharedTextureMemory(const wgpu::Device& device,
                                                        int layerCount) override {
        return CreateSharedTextureMemory(device, layerCount, mUseKeyedMutex);
    }

    wgpu::SharedTextureMemory CreateSharedTextureMemory(const wgpu::Device& device,
                                                        int layerCount,
                                                        bool useKeyedMutex) {
        ComPtr<ID3D11Device> d3d11Device = MakeD3D11Device(device);

        // Create a DX11 texture with data then wrap it in a shared handle.
        D3D11_TEXTURE2D_DESC d3dDescriptor;
        d3dDescriptor.Width = 16;
        d3dDescriptor.Height = 16;
        d3dDescriptor.MipLevels = 1;
        d3dDescriptor.ArraySize = layerCount;
        d3dDescriptor.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
        d3dDescriptor.SampleDesc.Count = 1;
        d3dDescriptor.SampleDesc.Quality = 0;
        d3dDescriptor.Usage = D3D11_USAGE_DEFAULT;
        d3dDescriptor.BindFlags =
            D3D11_BIND_SHADER_RESOURCE | D3D11_BIND_UNORDERED_ACCESS | D3D11_BIND_RENDER_TARGET;
        d3dDescriptor.CPUAccessFlags = 0;
        d3dDescriptor.MiscFlags =
            D3D11_RESOURCE_MISC_SHARED_NTHANDLE |
            (useKeyedMutex ? D3D11_RESOURCE_MISC_SHARED_KEYEDMUTEX : D3D11_RESOURCE_MISC_SHARED);

        ComPtr<ID3D11Texture2D> d3d11Texture;
        HRESULT hr = d3d11Device->CreateTexture2D(&d3dDescriptor, nullptr, &d3d11Texture);

        switch (mMode) {
            case Mode::D3D11Texture2D: {
                native::d3d11::SharedTextureMemoryD3D11Texture2DDescriptor texture2DDesc;
                texture2DDesc.texture = std::move(d3d11Texture);

                wgpu::SharedTextureMemoryDescriptor desc;
                desc.nextInChain = &texture2DDesc;

                return device.ImportSharedTextureMemory(&desc);
            }
            case Mode::DXGISharedHandle: {
                ComPtr<IDXGIResource1> dxgiResource;
                hr = d3d11Texture.As(&dxgiResource);
                DAWN_ASSERT(hr == S_OK);

                HANDLE sharedHandle;
                hr = dxgiResource->CreateSharedHandle(
                    nullptr, DXGI_SHARED_RESOURCE_READ | DXGI_SHARED_RESOURCE_WRITE, nullptr,
                    &sharedHandle);
                DAWN_ASSERT(hr == S_OK);

                wgpu::SharedTextureMemoryDXGISharedHandleDescriptor sharedHandleDesc;
                sharedHandleDesc.handle = sharedHandle;

                std::string label = LabelName(d3dDescriptor.Format, d3dDescriptor.Width);
                wgpu::SharedTextureMemoryDescriptor desc;
                desc.nextInChain = &sharedHandleDesc;
                desc.label = label.c_str();

                auto memory = device.ImportSharedTextureMemory(&desc);
                ::CloseHandle(sharedHandle);

                return memory;
            }
        }
    }

    std::vector<std::vector<wgpu::SharedTextureMemory>> CreatePerDeviceSharedTextureMemories(
        const std::vector<wgpu::Device>& devices,
        int layerCount) override {
        std::vector<std::vector<wgpu::SharedTextureMemory>> memories;

        ComPtr<ID3D11Device> d3d11Device = MakeD3D11Device(devices[0]);

        bool supportsNV12Sharing = false;
        D3D11_FEATURE_DATA_D3D11_OPTIONS5 featureOptions5{};
        if (d3d11Device->CheckFeatureSupport(D3D11_FEATURE_D3D11_OPTIONS5, &featureOptions5,
                                             sizeof(featureOptions5)) == S_OK) {
            if (featureOptions5.SharedResourceTier >= D3D11_SHARED_RESOURCE_TIER_2) {
                UINT formatSupport;
                if (d3d11Device->CheckFormatSupport(DXGI_FORMAT_NV12, &formatSupport) == S_OK &&
                    (formatSupport & D3D11_FORMAT_SUPPORT_TEXTURE2D) != 0) {
                    supportsNV12Sharing = true;
                }
            }
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
                // Create a DX11 texture with data then wrap it in a shared handle.
                D3D11_TEXTURE2D_DESC d3dDescriptor;
                d3dDescriptor.Width = size;
                d3dDescriptor.Height = size;
                d3dDescriptor.MipLevels = 1;
                d3dDescriptor.ArraySize = layerCount;
                d3dDescriptor.Format = f.format;
                d3dDescriptor.SampleDesc.Count = 1;
                d3dDescriptor.SampleDesc.Quality = 0;
                d3dDescriptor.Usage = D3D11_USAGE_DEFAULT;
                d3dDescriptor.BindFlags = D3D11_BIND_SHADER_RESOURCE | D3D11_BIND_UNORDERED_ACCESS |
                                          D3D11_BIND_RENDER_TARGET;
                d3dDescriptor.CPUAccessFlags = 0;
                d3dDescriptor.MiscFlags = D3D11_RESOURCE_MISC_SHARED_NTHANDLE |
                                          (mUseKeyedMutex ? D3D11_RESOURCE_MISC_SHARED_KEYEDMUTEX
                                                          : D3D11_RESOURCE_MISC_SHARED);

                ComPtr<ID3D11Texture2D> d3d11Texture;
                HRESULT hr = d3d11Device->CreateTexture2D(&d3dDescriptor, nullptr, &d3d11Texture);

                std::vector<wgpu::SharedTextureMemory> perDeviceMemories;
                switch (mMode) {
                    case Mode::D3D11Texture2D: {
                        native::d3d11::SharedTextureMemoryD3D11Texture2DDescriptor texture2DDesc;
                        texture2DDesc.texture = d3d11Texture;

                        wgpu::SharedTextureMemoryDescriptor desc;
                        desc.nextInChain = &texture2DDesc;

                        for (auto& device : devices) {
                            if (f.requiredFeature != wgpu::FeatureName(0u) &&
                                !device.HasFeature(f.requiredFeature)) {
                                continue;
                            }

                            perDeviceMemories.push_back(device.ImportSharedTextureMemory(&desc));
                        }
                        break;
                    }
                    case Mode::DXGISharedHandle: {
                        ComPtr<IDXGIResource1> dxgiResource;
                        hr = d3d11Texture.As(&dxgiResource);
                        DAWN_ASSERT(hr == S_OK);

                        HANDLE sharedHandle;
                        hr = dxgiResource->CreateSharedHandle(
                            nullptr, DXGI_SHARED_RESOURCE_READ | DXGI_SHARED_RESOURCE_WRITE,
                            nullptr, &sharedHandle);
                        DAWN_ASSERT(hr == S_OK);

                        wgpu::SharedTextureMemoryDXGISharedHandleDescriptor sharedHandleDesc;
                        sharedHandleDesc.handle = sharedHandle;
                        sharedHandleDesc.useKeyedMutex = mUseKeyedMutex;

                        std::string label = LabelName(f.format, size);

                        wgpu::SharedTextureMemoryDescriptor desc;
                        desc.nextInChain = &sharedHandleDesc;
                        desc.label = label.c_str();

                        for (auto& device : devices) {
                            if (f.requiredFeature != wgpu::FeatureName(0u) &&
                                !device.HasFeature(f.requiredFeature)) {
                                continue;
                            }

                            perDeviceMemories.push_back(device.ImportSharedTextureMemory(&desc));
                        }

                        ::CloseHandle(sharedHandle);
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

    std::unique_ptr<BackendBeginState> ChainInitialBeginState(
        wgpu::SharedTextureMemoryBeginAccessDescriptor* beginDesc) override {
        auto state = std::make_unique<BeginState>();
        state->beginState.requiresEndAccessFence = mRequiresEndAccessFence;
        beginDesc->nextInChain = &state->beginState;
        return state;
    }

    std::unique_ptr<BackendBeginState> ChainBeginState(
        wgpu::SharedTextureMemoryBeginAccessDescriptor* beginDesc,
        const wgpu::SharedTextureMemoryEndAccessState& endState) override {
        return ChainInitialBeginState(beginDesc);
    }

  private:
    Backend(Mode mode, bool useKeyedMutex, bool requiresEndAccessFence)
        : mMode(mode),
          mUseKeyedMutex(useKeyedMutex),
          mRequiresEndAccessFence(requiresEndAccessFence) {}

    const Mode mMode;
    const bool mUseKeyedMutex;
    const bool mRequiresEndAccessFence;
};

// Test that it is an error to import a shared fence without enabling the feature.
TEST_P(SharedTextureMemoryNoFeatureTests, SharedFenceImportWithoutFeature) {
    auto backend = static_cast<Backend*>(GetParam().mBackend);
    DAWN_TEST_UNSUPPORTED_IF(!backend->HasFenceSupport(device));

    ComPtr<ID3D11Device> d3d11Device = backend->MakeD3D11Device(device);

    ComPtr<ID3D11Device5> d3d11Device5;
    HRESULT hr = d3d11Device.As(&d3d11Device5);

    ComPtr<ID3D11Fence> d3d11Fence;
    hr = d3d11Device5->CreateFence(0, D3D11_FENCE_FLAG_SHARED, IID_PPV_ARGS(&d3d11Fence));
    ASSERT_EQ(hr, S_OK);

    HANDLE fenceSharedHandle = nullptr;
    hr = d3d11Fence->CreateSharedHandle(nullptr, GENERIC_ALL, nullptr, &fenceSharedHandle);
    ASSERT_EQ(hr, S_OK);

    wgpu::SharedFenceDXGISharedHandleDescriptor sharedHandleDesc;
    sharedHandleDesc.handle = fenceSharedHandle;

    wgpu::SharedFenceDescriptor fenceDesc;
    fenceDesc.nextInChain = &sharedHandleDesc;

    ASSERT_DEVICE_ERROR_MSG(wgpu::SharedFence fence = device.ImportSharedFence(&fenceDesc),
                            testing::HasSubstr("is not enabled"));
    ::CloseHandle(fenceSharedHandle);
}

// Test that a shared handle can be imported, and then exported.
TEST_P(SharedTextureMemoryTests, SharedFenceSuccessfulImportExport) {
    auto backend = static_cast<Backend*>(GetParam().mBackend);
    DAWN_TEST_UNSUPPORTED_IF(!backend->HasFenceSupport(device));

    ComPtr<ID3D11Device> d3d11Device = backend->MakeD3D11Device(device);

    ComPtr<ID3D11Device5> d3d11Device5;
    HRESULT hr = d3d11Device.As(&d3d11Device5);

    ComPtr<ID3D11Fence> d3d11Fence;
    hr = d3d11Device5->CreateFence(0, D3D11_FENCE_FLAG_SHARED, IID_PPV_ARGS(&d3d11Fence));
    ASSERT_EQ(hr, S_OK);

    HANDLE fenceSharedHandle = nullptr;
    hr = d3d11Fence->CreateSharedHandle(nullptr, GENERIC_ALL, nullptr, &fenceSharedHandle);
    ASSERT_EQ(hr, S_OK);

    wgpu::SharedFenceDXGISharedHandleDescriptor sharedHandleDesc;
    sharedHandleDesc.handle = fenceSharedHandle;

    wgpu::SharedFenceDescriptor fenceDesc;
    fenceDesc.nextInChain = &sharedHandleDesc;

    wgpu::SharedFence fence = device.ImportSharedFence(&fenceDesc);
    ::CloseHandle(fenceSharedHandle);

    wgpu::SharedFenceDXGISharedHandleExportInfo sharedHandleInfo;
    wgpu::SharedFenceExportInfo exportInfo;
    exportInfo.nextInChain = &sharedHandleInfo;
    fence.ExportInfo(&exportInfo);

    // The exported handle should be the same as the imported one.
    EXPECT_EQ(exportInfo.type, wgpu::SharedFenceType::DXGISharedHandle);

    // Now, test that the fence handle is indeed the same one by making a new fence
    // with it. Changes it the first fence should be reflected in the second fence.

    // Open the exported handle.
    ComPtr<ID3D11Fence> d3d11Fence2;
    hr = d3d11Device5->OpenSharedFence(sharedHandleInfo.handle, IID_PPV_ARGS(&d3d11Fence2));
    ASSERT_EQ(hr, S_OK);

    // Check the fences point to the same object.
    uint64_t fenceValue = d3d11Fence->GetCompletedValue();
    EXPECT_EQ(fenceValue, d3d11Fence2->GetCompletedValue());

    ComPtr<ID3D11DeviceContext> d3d11DeviceContext;
    d3d11Device->GetImmediateContext(&d3d11DeviceContext);

    ComPtr<ID3D11DeviceContext4> d3d11DeviceContext4;
    hr = d3d11DeviceContext.As(&d3d11DeviceContext4);
    ASSERT_EQ(hr, S_OK);

    hr = d3d11DeviceContext4->Signal(d3d11Fence.Get(), fenceValue + 1);
    ASSERT_EQ(hr, S_OK);

    HANDLE ev = ::CreateEvent(NULL,                  // default security attributes
                              TRUE,                  // manual-reset event
                              FALSE,                 // initial state is nonsignaled
                              TEXT("FenceComplete")  // object name
    );

    // Wait for the fence.
    d3d11Fence->SetEventOnCompletion(fenceValue + 1, ev);
    ::WaitForSingleObject(ev, INFINITE);
    ::CloseHandle(ev);

    // Both fences should see the completed value.
    EXPECT_EQ(fenceValue + 1, d3d11Fence->GetCompletedValue());
    EXPECT_EQ(fenceValue + 1, d3d11Fence2->GetCompletedValue());
}

// Test that it is an error to import a shared fence with a null DXGISharedHandle
TEST_P(SharedTextureMemoryTests, SharedFenceImportDXGISharedHandleMissing) {
    auto backend = static_cast<Backend*>(GetParam().mBackend);
    DAWN_TEST_UNSUPPORTED_IF(!backend->HasFenceSupport(device));

    wgpu::SharedFenceDXGISharedHandleDescriptor sharedHandleDesc;
    sharedHandleDesc.handle = nullptr;

    wgpu::SharedFenceDescriptor fenceDesc;
    fenceDesc.nextInChain = &sharedHandleDesc;

    ASSERT_DEVICE_ERROR_MSG(wgpu::SharedFence fence = device.ImportSharedFence(&fenceDesc),
                            testing::HasSubstr("missing"));
}

// Test exporting info from a shared fence with no chained struct.
// It should be valid and the fence type is exported.
TEST_P(SharedTextureMemoryTests, SharedFenceExportInfoNoChainedStruct) {
    auto backend = static_cast<Backend*>(GetParam().mBackend);
    DAWN_TEST_UNSUPPORTED_IF(!backend->HasFenceSupport(device));

    ComPtr<ID3D11Device> d3d11Device = backend->MakeD3D11Device(device);

    ComPtr<ID3D11Device5> d3d11Device5;
    HRESULT hr = d3d11Device.As(&d3d11Device5);

    ComPtr<ID3D11Fence> d3d11Fence;
    hr = d3d11Device5->CreateFence(0, D3D11_FENCE_FLAG_SHARED, IID_PPV_ARGS(&d3d11Fence));
    ASSERT_EQ(hr, S_OK);

    HANDLE fenceSharedHandle = nullptr;
    hr = d3d11Fence->CreateSharedHandle(nullptr, GENERIC_ALL, nullptr, &fenceSharedHandle);
    ASSERT_EQ(hr, S_OK);

    wgpu::SharedFenceDXGISharedHandleDescriptor sharedHandleDesc;
    sharedHandleDesc.handle = fenceSharedHandle;

    wgpu::SharedFenceDescriptor fenceDesc;
    fenceDesc.nextInChain = &sharedHandleDesc;

    wgpu::SharedFence fence = device.ImportSharedFence(&fenceDesc);
    ::CloseHandle(fenceSharedHandle);

    // Test no chained struct.
    wgpu::SharedFenceExportInfo exportInfo;
    exportInfo.nextInChain = nullptr;

    fence.ExportInfo(&exportInfo);
    EXPECT_EQ(exportInfo.type, wgpu::SharedFenceType::DXGISharedHandle);
}

// Test exporting info from a shared fence with an invalid chained struct.
// It should not be valid, but the fence type should still be exported.
TEST_P(SharedTextureMemoryTests, SharedFenceExportInfoInvalidChainedStruct) {
    auto backend = static_cast<Backend*>(GetParam().mBackend);
    DAWN_TEST_UNSUPPORTED_IF(!backend->HasFenceSupport(device));

    ComPtr<ID3D11Device> d3d11Device = backend->MakeD3D11Device(device);

    ComPtr<ID3D11Device5> d3d11Device5;
    HRESULT hr = d3d11Device.As(&d3d11Device5);

    ComPtr<ID3D11Fence> d3d11Fence;
    hr = d3d11Device5->CreateFence(0, D3D11_FENCE_FLAG_SHARED, IID_PPV_ARGS(&d3d11Fence));
    ASSERT_EQ(hr, S_OK);

    HANDLE fenceSharedHandle = nullptr;
    hr = d3d11Fence->CreateSharedHandle(nullptr, GENERIC_ALL, nullptr, &fenceSharedHandle);
    ASSERT_EQ(hr, S_OK);

    wgpu::SharedFenceDXGISharedHandleDescriptor sharedHandleDesc;
    sharedHandleDesc.handle = fenceSharedHandle;

    wgpu::SharedFenceDescriptor fenceDesc;
    fenceDesc.nextInChain = &sharedHandleDesc;

    wgpu::SharedFence fence = device.ImportSharedFence(&fenceDesc);
    ::CloseHandle(fenceSharedHandle);

    // Test an invalid chained struct.
    wgpu::ChainedStructOut otherStruct;
    wgpu::SharedFenceExportInfo exportInfo;
    exportInfo.nextInChain = &otherStruct;

    ASSERT_DEVICE_ERROR(fence.ExportInfo(&exportInfo));
}

using SharedTextureMemoryWithFenceDisabledTests = SharedTextureMemoryTests;

// Test that when fences are disabled, if a shared handle is not created with a keyed mutex, it's an
// error.
TEST_P(SharedTextureMemoryWithFenceDisabledTests, ErrorSharedHandleWithoutKeyedMutex) {
    auto backend = static_cast<Backend*>(GetParam().mBackend);
    DAWN_TEST_UNSUPPORTED_IF(backend->HasFenceSupport(device));
    DAWN_TEST_UNSUPPORTED_IF(!backend->UseSharedHandle());

    wgpu::SharedTextureMemory memory;

    ASSERT_DEVICE_ERROR(memory = backend->CreateSharedTextureMemory(device, GetParam().mLayerCount,
                                                                    /*useKeyedMutex=*/false));
}

DAWN_INSTANTIATE_PREFIXED_TEST_P(D3D,
                                 SharedTextureMemoryNoFeatureTests,
                                 {D3D11Backend(), D3D11Backend({"d3d11_disable_fence"}),
                                  D3D12Backend()},
                                 {Backend::GetInstance<Mode::DXGISharedHandle>(),
                                  Backend::GetKeyedMutexInstance<Mode::DXGISharedHandle>()},
                                 {1});

DAWN_INSTANTIATE_PREFIXED_TEST_P(D3D,
                                 SharedTextureMemoryTests,
                                 {D3D11Backend(), D3D11Backend({"d3d11_delay_flush_to_gpu"}),
                                  D3D11Backend({"d3d11_disable_fence"}), D3D12Backend()},
                                 {Backend::GetInstance<Mode::DXGISharedHandle>(),
                                  Backend::GetKeyedMutexInstance<Mode::DXGISharedHandle>()},
                                 {1});

DAWN_INSTANTIATE_PREFIXED_TEST_P(
    D3D11,
    SharedTextureMemoryNoFeatureTests,
    {D3D11Backend(), D3D11Backend({"d3d11_disable_fence"})},
    {Backend::GetInstance<Mode::D3D11Texture2D>(),
     Backend::GetKeyedMutexInstance<Mode::D3D11Texture2D>(),
     Backend::GetInstanceWithoutEndAccessFence<Mode::D3D11Texture2D>()},
    {1, 2});

DAWN_INSTANTIATE_PREFIXED_TEST_P(
    D3D11,
    SharedTextureMemoryTests,
    {D3D11Backend(), D3D11Backend({"d3d11_delay_flush_to_gpu"}),
     D3D11Backend({"d3d11_disable_fence"})},
    {Backend::GetInstance<Mode::D3D11Texture2D>(),
     Backend::GetKeyedMutexInstance<Mode::D3D11Texture2D>(),
     Backend::GetInstanceWithoutEndAccessFence<Mode::D3D11Texture2D>()},
    {1, 2});

DAWN_INSTANTIATE_PREFIXED_TEST_P(D3D11,
                                 SharedTextureMemoryWithFenceDisabledTests,
                                 {D3D11Backend({"d3d11_disable_fence"}),
                                  D3D11Backend({"d3d11_disable_fence",
                                                "d3d11_delay_flush_to_gpu"})},
                                 {Backend::GetKeyedMutexInstance<Mode::DXGISharedHandle>()},
                                 {1});

GTEST_ALLOW_UNINSTANTIATED_PARAMETERIZED_TEST(SharedTextureMemoryWithFenceDisabledTests);

}  // anonymous namespace
}  // namespace dawn
