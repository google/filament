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

#include <d3d11.h>
#include <d3d11_4.h>
#include <d3d12.h>
#include <dxgi1_4.h>
#include <wrl/client.h>

#include <memory>
#include <utility>
#include <variant>
#include <vector>

#include "VideoViewsTests.h"
#include "dawn/common/Assert.h"
#include "dawn/native/D3DBackend.h"
#include "dawn/utils/TextureUtils.h"

namespace dawn {
namespace {

using Microsoft::WRL::ComPtr;

class PlatformTextureWin : public VideoViewsTestBackend::PlatformTexture {
  public:
    explicit PlatformTextureWin(wgpu::Texture&& texture) : PlatformTexture(std::move(texture)) {}
    ~PlatformTextureWin() override = default;

    bool CanWrapAsWGPUTexture() override { return true; }
};

class VideoViewsTestBackendWin : public VideoViewsTestBackend {
  public:
    ~VideoViewsTestBackendWin() override = default;

    void OnSetUp(const wgpu::Device& device) override {
        mWGPUDevice = device;

        // Create the D3D11 device/contexts that will be used in subsequent tests
        ComPtr<IDXGIAdapter> dxgiAdapter =
            native::d3d::GetDXGIAdapter(wgpuDeviceGetAdapter(device.Get()));

        ComPtr<ID3D11Device> d3d11Device;
        D3D_FEATURE_LEVEL d3dFeatureLevel;
        ComPtr<ID3D11DeviceContext> d3d11DeviceContext;
        HRESULT hr;
        hr = ::D3D11CreateDevice(dxgiAdapter.Get(), D3D_DRIVER_TYPE_UNKNOWN, nullptr, 0, nullptr, 0,
                                 D3D11_SDK_VERSION, &d3d11Device, &d3dFeatureLevel,
                                 &d3d11DeviceContext);
        ASSERT_EQ(hr, S_OK);

        // Runtime of the created texture (D3D11 device) and OpenSharedHandle runtime (Dawn's
        // D3D12 device) must agree on resource sharing capability. For NV12 formats, D3D11
        // requires at-least D3D11_SHARED_RESOURCE_TIER_2 support.
        // https://docs.microsoft.com/en-us/windows/win32/api/d3d11/ne-d3d11-d3d11_shared_resource_tier
        D3D11_FEATURE_DATA_D3D11_OPTIONS5 featureOptions5{};
        hr = d3d11Device->CheckFeatureSupport(D3D11_FEATURE_D3D11_OPTIONS5, &featureOptions5,
                                              sizeof(featureOptions5));
        ASSERT_EQ(hr, S_OK);

        ASSERT_GE(featureOptions5.SharedResourceTier, D3D11_SHARED_RESOURCE_TIER_2);

        // Not all D3D11 devices support NV12 textures.
        UINT formatSupport;
        hr = d3d11Device->CheckFormatSupport(DXGI_FORMAT_NV12, &formatSupport);
        ASSERT_EQ(hr, S_OK);

        ASSERT_TRUE(formatSupport & D3D11_FORMAT_SUPPORT_TEXTURE2D);

        mD3d11Device = std::move(d3d11Device);
    }

  protected:
    static DXGI_FORMAT GetDXGITextureFormat(wgpu::TextureFormat format) {
        switch (format) {
            case wgpu::TextureFormat::R8BG8Biplanar420Unorm:
                return DXGI_FORMAT_NV12;
            case wgpu::TextureFormat::R10X6BG10X6Biplanar420Unorm:
                return DXGI_FORMAT_P010;
            default:
                DAWN_UNREACHABLE();
        }
    }

    static UINT GetD3D11TextureBindFlags(wgpu::TextureUsage usage) {
        UINT bindFlags = 0;
        if (usage & wgpu::TextureUsage::TextureBinding) {
            bindFlags |= D3D11_BIND_SHADER_RESOURCE;
        }
        if (usage & wgpu::TextureUsage::StorageBinding) {
            bindFlags |= D3D11_BIND_UNORDERED_ACCESS | D3D11_BIND_SHADER_RESOURCE;
        }
        if (usage & wgpu::TextureUsage::RenderAttachment) {
            bindFlags |= D3D11_BIND_RENDER_TARGET;
        }
        return bindFlags;
    }

    std::unique_ptr<VideoViewsTestBackend::PlatformTexture> CreateVideoTextureForTest(
        wgpu::TextureFormat format,
        wgpu::TextureUsage usage,
        bool isCheckerboard,
        bool initialized) override {
        wgpu::TextureDescriptor textureDesc;
        textureDesc.format = format;
        textureDesc.dimension = wgpu::TextureDimension::e2D;
        textureDesc.usage = usage;
        textureDesc.size = {VideoViewsTestsBase::kYUVAImageDataWidthInTexels,
                            VideoViewsTestsBase::kYUVAImageDataHeightInTexels, 1};

        // Create a DX11 texture with data then wrap it in a shared handle.
        D3D11_TEXTURE2D_DESC d3dDescriptor;
        d3dDescriptor.Width = VideoViewsTestsBase::kYUVAImageDataWidthInTexels;
        d3dDescriptor.Height = VideoViewsTestsBase::kYUVAImageDataHeightInTexels;
        d3dDescriptor.MipLevels = 1;
        d3dDescriptor.ArraySize = 1;
        d3dDescriptor.Format = GetDXGITextureFormat(format);
        d3dDescriptor.SampleDesc.Count = 1;
        d3dDescriptor.SampleDesc.Quality = 0;
        d3dDescriptor.Usage = D3D11_USAGE_DEFAULT;
        d3dDescriptor.BindFlags = GetD3D11TextureBindFlags(usage);
        d3dDescriptor.CPUAccessFlags = 0;
        d3dDescriptor.MiscFlags = D3D11_RESOURCE_MISC_SHARED_NTHANDLE | D3D11_RESOURCE_MISC_SHARED;

        D3D11_SUBRESOURCE_DATA subres;
        subres.SysMemPitch = VideoViewsTestsBase::kYUVAImageDataWidthInTexels;

        std::variant<std::vector<uint8_t>, std::vector<uint16_t>> initialData;
        if (utils::GetMultiPlaneTextureBitDepth(format) == 16) {
            initialData = VideoViewsTestsBase::GetTestTextureData<uint16_t>(format, isCheckerboard,
                                                                            /*hasAlpha=*/false);
            subres.pSysMem = std::get<1>(initialData).data();
            subres.SysMemPitch *= 2;
        } else {
            initialData = VideoViewsTestsBase::GetTestTextureData<uint8_t>(format, isCheckerboard,
                                                                           /*hasAlpha=*/false);
            subres.pSysMem = std::get<0>(initialData).data();
        }

        ComPtr<ID3D11Texture2D> d3d11Texture;
        HRESULT hr = mD3d11Device->CreateTexture2D(
            &d3dDescriptor, (initialized ? &subres : nullptr), &d3d11Texture);
        DAWN_ASSERT(hr == S_OK);

        ComPtr<IDXGIResource1> dxgiResource;
        hr = d3d11Texture.As(&dxgiResource);
        DAWN_ASSERT(hr == S_OK);

        HANDLE sharedHandle;
        hr = dxgiResource->CreateSharedHandle(
            nullptr, DXGI_SHARED_RESOURCE_READ | DXGI_SHARED_RESOURCE_WRITE, nullptr,
            &sharedHandle);
        DAWN_ASSERT(hr == S_OK);

        HANDLE fenceSharedHandle = nullptr;
        ComPtr<ID3D11Fence> d3d11Fence;

        ComPtr<ID3D11Device5> d3d11Device5;
        hr = mD3d11Device.As(&d3d11Device5);
        DAWN_ASSERT(hr == S_OK);

        hr = d3d11Device5->CreateFence(0, D3D11_FENCE_FLAG_SHARED, IID_PPV_ARGS(&d3d11Fence));
        DAWN_ASSERT(hr == S_OK);

        hr = d3d11Fence->CreateSharedHandle(nullptr, GENERIC_ALL, nullptr, &fenceSharedHandle);
        DAWN_ASSERT(hr == S_OK);

        ComPtr<ID3D11DeviceContext> d3d11DeviceContext;
        mD3d11Device->GetImmediateContext(&d3d11DeviceContext);

        ComPtr<ID3D11DeviceContext4> d3d11DeviceContext4;
        hr = d3d11DeviceContext.As(&d3d11DeviceContext4);
        DAWN_ASSERT(hr == S_OK);
        // D3D11 texture should be initialized upon CreateTexture2D, but we need to make Dawn/D3D12
        // wait on the initializtaion. The fence starts with 0 signaled, but that won't capture the
        // initialization above, so signal explicitly with 1 and make Dawn wait on it.
        hr = d3d11DeviceContext4->Signal(d3d11Fence.Get(), 1);
        DAWN_ASSERT(hr == S_OK);

        // Open the DX11 texture in Dawn from the shared handle and return it as a WebGPU texture.
        wgpu::SharedTextureMemoryDXGISharedHandleDescriptor sharedHandleDesc{};
        sharedHandleDesc.handle = sharedHandle;

        wgpu::SharedTextureMemoryDescriptor desc;
        desc.nextInChain = &sharedHandleDesc;

        auto sharedTextureMemory = mWGPUDevice.ImportSharedTextureMemory(&desc);
        // Handle is no longer needed once resources are created.
        ::CloseHandle(sharedHandle);

        wgpu::SharedFenceDXGISharedHandleDescriptor dxgiFenceDesc{};
        dxgiFenceDesc.handle = fenceSharedHandle;
        wgpu::SharedFenceDescriptor fenceDesc{};
        fenceDesc.nextInChain = &dxgiFenceDesc;
        auto wgpuFence = mWGPUDevice.ImportSharedFence(&fenceDesc);
        uint64_t signaled_value = 1;

        wgpu::SharedTextureMemoryBeginAccessDescriptor beginDesc{};
        beginDesc.initialized = initialized;
        beginDesc.concurrentRead = false;
        beginDesc.fenceCount = 1;
        beginDesc.fences = &wgpuFence;
        beginDesc.signaledValues = &signaled_value;

        auto wgpuTexture = sharedTextureMemory.CreateTexture(&textureDesc);
        bool success = sharedTextureMemory.BeginAccess(wgpuTexture, &beginDesc);
        // Fence handle is no longer needed after begin access.
        ::CloseHandle(fenceSharedHandle);

        return success ? std::make_unique<PlatformTextureWin>(std::move(wgpuTexture)) : nullptr;
    }

    void DestroyVideoTextureForTest(
        std::unique_ptr<VideoViewsTestBackend::PlatformTexture>&& platformTexture) override {}

    wgpu::Device mWGPUDevice = nullptr;
    ComPtr<ID3D11Device> mD3d11Device;
};

}  // anonymous namespace

// static
std::vector<BackendTestConfig> VideoViewsTestBackend::Backends() {
    return {D3D11Backend(), D3D12Backend()};
}

// static
std::vector<Format> VideoViewsTestBackend::Formats() {
    return {wgpu::TextureFormat::R8BG8Biplanar420Unorm,
            wgpu::TextureFormat::R10X6BG10X6Biplanar420Unorm};
}

// static
std::unique_ptr<VideoViewsTestBackend> VideoViewsTestBackend::Create() {
    return std::make_unique<VideoViewsTestBackendWin>();
}

}  // namespace dawn
