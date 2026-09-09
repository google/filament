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

#include <memory>
#include <utility>
#include <variant>
#include <vector>

#include "VideoViewsTests.h"
#include "src/dawn/common/CoreFoundationRef.h"
#include "src/dawn/common/IOSurfaceUtils.h"
#include "src/dawn/utils/TextureUtils.h"
#include "src/utils/assert.h"
#include "src/utils/compiler.h"

namespace dawn {
namespace {

class PlatformTextureIOSurface : public VideoViewsTestBackend::PlatformTexture {
  public:
    PlatformTextureIOSurface(wgpu::Texture&& texture,
                             wgpu::SharedTextureMemory&& sharedTextureMemory)
        : PlatformTexture(std::move(texture)),
          mSharedTextureMemory(std::move(sharedTextureMemory)) {}
    ~PlatformTextureIOSurface() override {}

    bool CanWrapAsWGPUTexture() override { return true; }

  public:
    wgpu::SharedTextureMemory mSharedTextureMemory;
};

class VideoViewsTestBackendIOSurface : public VideoViewsTestBackend {
  public:
    bool Initialize(const wgpu::Device& device) override {
        mWGPUDevice = device;
        return true;
    }

  private:
    std::unique_ptr<VideoViewsTestBackend::PlatformTexture> CreateVideoTextureForTest(
        wgpu::TextureFormat format,
        wgpu::TextureUsage usage,
        bool isCheckerboard,
        bool initialized) override {
        IOSurfaceRef surface =
            CreateMultiPlanarIOSurface(format, VideoViewsTestsBase::kYUVAImageDataWidthInTexels,
                                       VideoViewsTestsBase::kYUVAImageDataHeightInTexels);

        if (initialized) {
            const size_t numPlanes = utils::GetMultiPlaneTextureNumPlanes(format);

            IOSurfaceLock(surface, 0, nullptr);

            for (size_t plane = 0; plane < numPlanes; ++plane) {
                void* pointer = IOSurfaceGetBaseAddressOfPlane(surface, plane);
                if (utils::GetMultiPlaneTextureBitDepth(format) == 16) {
                    std::vector<uint16_t> data =
                        VideoViewsTestsBase::GetTestTextureDataWithPlaneIndex<uint16_t>(
                            format, plane, IOSurfaceGetBytesPerRowOfPlane(surface, plane),
                            IOSurfaceGetHeightOfPlane(surface, plane), isCheckerboard,
                            /*hasAlpha=*/false);
                    DAWN_UNSAFE_TODO(memcpy(pointer, data.data(), data.size() * 2));
                } else {
                    std::vector<uint8_t> data =
                        VideoViewsTestsBase::GetTestTextureDataWithPlaneIndex<uint8_t>(
                            format, plane, IOSurfaceGetBytesPerRowOfPlane(surface, plane),
                            IOSurfaceGetHeightOfPlane(surface, plane), isCheckerboard,
                            /*hasAlpha=*/format == wgpu::TextureFormat::R8BG8A8Triplanar420Unorm);
                    DAWN_UNSAFE_TODO(memcpy(pointer, data.data(), data.size()));
                }
            }
            IOSurfaceUnlock(surface, 0, nullptr);
        }

        wgpu::TextureDescriptor textureDesc;
        textureDesc.format = format;
        textureDesc.dimension = wgpu::TextureDimension::e2D;
        textureDesc.usage = usage;
        textureDesc.size = {VideoViewsTestsBase::kYUVAImageDataWidthInTexels,
                            VideoViewsTestsBase::kYUVAImageDataHeightInTexels, 1};

        wgpu::DawnTextureInternalUsageDescriptor internalDesc;
        internalDesc.internalUsage = wgpu::TextureUsage::CopySrc;
        textureDesc.nextInChain = &internalDesc;

        wgpu::SharedTextureMemoryIOSurfaceDescriptor ioSurfaceDesc;
        ioSurfaceDesc.ioSurface = surface;
        wgpu::SharedTextureMemoryDescriptor desc;
        desc.nextInChain = &ioSurfaceDesc;

        auto sharedTextureMemory = mWGPUDevice.ImportSharedTextureMemory(&desc);

        auto texture = sharedTextureMemory.CreateTexture(&textureDesc);

        // Invoke BeginAccess() on the texture to ensure that it can be used by
        // the test. We will end the access when the texture is destroyed
        // (below).
        wgpu::SharedTextureMemoryBeginAccessDescriptor beginAccessDesc;
        beginAccessDesc.initialized = initialized;
        beginAccessDesc.fenceCount = 0;
        beginAccessDesc.signaledValueCount = 0;
        wgpu::Status status = sharedTextureMemory.BeginAccess(texture, &beginAccessDesc);

        return status == wgpu::Status::Success
                   ? std::make_unique<PlatformTextureIOSurface>(std::move(texture),
                                                                std::move(sharedTextureMemory))
                   : nullptr;
    }

    void DestroyVideoTextureForTest(
        std::unique_ptr<VideoViewsTestBackend::PlatformTexture>&& platformTexture) override {}

    wgpu::Device mWGPUDevice = nullptr;
};

}  // anonymous namespace

// static
std::vector<BackendTestConfig> VideoViewsTestBackend::Backends() {
    return {MetalBackend()};
}

// static
std::vector<Format> VideoViewsTestBackend::Formats() {
    return {wgpu::TextureFormat::R8BG8Biplanar420Unorm,
            wgpu::TextureFormat::R8BG8Biplanar422Unorm,
            wgpu::TextureFormat::R8BG8Biplanar444Unorm,
            wgpu::TextureFormat::R10X6BG10X6Biplanar420Unorm,
            wgpu::TextureFormat::R10X6BG10X6Biplanar422Unorm,
            wgpu::TextureFormat::R10X6BG10X6Biplanar444Unorm,
            wgpu::TextureFormat::R8BG8A8Triplanar420Unorm};
}

// static
std::unique_ptr<VideoViewsTestBackend> VideoViewsTestBackend::Create() {
    return std::make_unique<VideoViewsTestBackendIOSurface>();
}

}  // namespace dawn
