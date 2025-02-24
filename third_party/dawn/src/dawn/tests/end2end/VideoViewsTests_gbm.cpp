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

#include <fcntl.h>
#include <gbm.h>

#include <memory>
#include <string>
#include <utility>
#include <vector>

#include "VideoViewsTests.h"
#include "dawn/common/Assert.h"
#include "dawn/native/VulkanBackend.h"

namespace dawn {
namespace {

// "linux-chromeos-rel"'s gbm.h is too old to compile, missing this change at least:
// https://chromium-review.googlesource.com/c/chromiumos/platform/minigbm/+/1963001/10/gbm.h#244
#ifndef MINIGBM
#define GBM_BO_USE_TEXTURING (1 << 5)
#define GBM_BO_USE_SW_WRITE_RARELY (1 << 12)
#define GBM_BO_USE_HW_VIDEO_DECODER (1 << 13)
#endif

#ifndef DRM_FORMAT_MOD_LINEAR
#define DRM_FORMAT_MOD_LINEAR 0
#endif

class PlatformTextureGbm : public VideoViewsTestBackend::PlatformTexture {
  public:
    PlatformTextureGbm(wgpu::Texture&& texture, gbm_bo* gbmBo)
        : PlatformTexture(std::move(texture)), mGbmBo(gbmBo) {}
    ~PlatformTextureGbm() override = default;

    // TODO(chromium:1258986): Add DISJOINT vkImage support for multi-plannar formats.
    bool CanWrapAsWGPUTexture() override {
        DAWN_ASSERT(mGbmBo != nullptr);
        // Checks if all plane handles of a multi-planar gbm_bo are same.
        gbm_bo_handle plane0Handle = gbm_bo_get_handle_for_plane(mGbmBo, 0);
        for (int plane = 1; plane < gbm_bo_get_plane_count(mGbmBo); ++plane) {
            if (gbm_bo_get_handle_for_plane(mGbmBo, plane).u32 != plane0Handle.u32) {
                return false;
            }
        }
        return true;
    }

    gbm_bo* GetGbmBo() { return mGbmBo; }

  private:
    gbm_bo* mGbmBo = nullptr;
};

class VideoViewsTestBackendGbm : public VideoViewsTestBackend {
  public:
    void OnSetUp(const wgpu::Device& device) override {
        mWGPUDevice = device.Get();
        mGbmDevice = CreateGbmDevice();
    }

    void OnTearDown() override {
        if (mGbmDevice != nullptr) {
            gbm_device_destroy(mGbmDevice);
        }
        if (mRenderNodeFd >= 0) {
            close(mRenderNodeFd);
        }
    }

  private:
    gbm_device* CreateGbmDevice() {
        // Render nodes [1] are the primary interface for communicating with the GPU on
        // devices that support DRM. The actual filename of the render node is
        // implementation-specific, so we must scan through all possible filenames to find
        // one that we can use [2].
        //
        // [1] https://dri.freedesktop.org/docs/drm/gpu/drm-uapi.html#render-nodes
        // [2]
        // https://cs.chromium.org/chromium/src/ui/ozone/platform/wayland/gpu/drm_render_node_path_finder.cc
        const uint32_t kRenderNodeStart = 128;
        const uint32_t kRenderNodeEnd = kRenderNodeStart + 16;
        const std::string kRenderNodeTemplate = "/dev/dri/renderD";

        mRenderNodeFd = -1;
        for (uint32_t i = kRenderNodeStart; i < kRenderNodeEnd; i++) {
            std::string renderNode = kRenderNodeTemplate + std::to_string(i);
            mRenderNodeFd = open(renderNode.c_str(), O_RDWR);
            if (mRenderNodeFd >= 0) {
                break;
            }
        }
        DAWN_ASSERT(mRenderNodeFd > 0);

        gbm_device* gbmDevice = gbm_create_device(mRenderNodeFd);
        DAWN_ASSERT(gbmDevice != nullptr);
        return gbmDevice;
    }

    static uint32_t GetGbmBoFormat(wgpu::TextureFormat format) {
        switch (format) {
            case wgpu::TextureFormat::R8BG8Biplanar420Unorm:
                return GBM_FORMAT_NV12;
            default:
                DAWN_UNREACHABLE();
        }
    }

    WGPUTextureFormat ToWGPUTextureFormat(wgpu::TextureFormat format) {
        switch (format) {
            case wgpu::TextureFormat::R8BG8Biplanar420Unorm:
                return WGPUTextureFormat_R8BG8Biplanar420Unorm;
            default:
                DAWN_UNREACHABLE();
        }
    }

    WGPUTextureUsage ToWGPUTextureUsage(wgpu::TextureUsage usage) {
        switch (usage) {
            case wgpu::TextureUsage::TextureBinding:
                return WGPUTextureUsage_TextureBinding;
            default:
                DAWN_UNREACHABLE();
        }
    }

    std::unique_ptr<VideoViewsTestBackend::PlatformTexture> CreateVideoTextureForTest(
        wgpu::TextureFormat format,
        wgpu::TextureUsage usage,
        bool isCheckerboard,
        bool initialized) override {
        // The flags Chromium is using for the VAAPI decoder.
        uint32_t flags = GBM_BO_USE_SCANOUT | GBM_BO_USE_TEXTURING | GBM_BO_USE_HW_VIDEO_DECODER;
        if (initialized) {
            // The flag specifically used for tests, which need to initialize the GBM buffer with
            // the expected raw video data via CPU, and then sample and draw the buffer via GPU.
            // With the flag added, the buffer's drm modifier will be DRM_FORMAT_MOD_LINEAR instead
            // of I915_FORMAT_MOD_Y_TILED.
            flags |= GBM_BO_USE_SW_WRITE_RARELY;
        }
        gbm_bo* gbmBo = gbm_bo_create(mGbmDevice, VideoViewsTestsBase::kYUVAImageDataWidthInTexels,
                                      VideoViewsTestsBase::kYUVAImageDataHeightInTexels,
                                      GetGbmBoFormat(format), flags);
        if (gbmBo == nullptr) {
            return nullptr;
        }

        if (initialized) {
            void* mapHandle = nullptr;
            uint32_t strideBytes = 0;
            void* addr = gbm_bo_map(gbmBo, 0, 0, VideoViewsTestsBase::kYUVAImageDataWidthInTexels,
                                    VideoViewsTestsBase::kYUVAImageDataHeightInTexels,
                                    GBM_BO_TRANSFER_WRITE, &strideBytes, &mapHandle);
            EXPECT_NE(addr, nullptr);
            std::vector<uint8_t> initialData =
                VideoViewsTestsBase::GetTestTextureData<uint8_t>(format, isCheckerboard,
                                                                 /*hasAlpha=*/false);
            uint8_t* srcBegin = initialData.data();
            uint8_t* srcEnd = srcBegin + initialData.size();
            uint8_t* dstBegin = static_cast<uint8_t*>(addr);
            for (; srcBegin < srcEnd; srcBegin += VideoViewsTestsBase::kYUVAImageDataWidthInTexels,
                                      dstBegin += strideBytes) {
                std::memcpy(dstBegin, srcBegin, VideoViewsTestsBase::kYUVAImageDataWidthInTexels);
            }

            gbm_bo_unmap(gbmBo, mapHandle);
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

        native::vulkan::ExternalImageDescriptorDmaBuf descriptor = {};
        descriptor.cTextureDescriptor =
            reinterpret_cast<const WGPUTextureDescriptor*>(&textureDesc);
        descriptor.isInitialized = initialized;

        descriptor.memoryFD = gbm_bo_get_fd(gbmBo);
        for (int plane = 0; plane < gbm_bo_get_plane_count(gbmBo); ++plane) {
            descriptor.planeLayouts[plane].stride = gbm_bo_get_stride_for_plane(gbmBo, plane);
            descriptor.planeLayouts[plane].offset = gbm_bo_get_offset(gbmBo, plane);
        }
        descriptor.drmModifier = gbm_bo_get_modifier(gbmBo);
        descriptor.waitFDs = {};

        auto texture = std::make_unique<PlatformTextureGbm>(
            native::vulkan::WrapVulkanImage(mWGPUDevice, &descriptor), gbmBo);
        // The ownership of FD is only transferred in case of a success import. Otherwise cleanup
        // is still needed to avoid FD leak.
        if (!texture->wgpuTexture && descriptor.memoryFD >= 0) {
            close(descriptor.memoryFD);
        }
        return texture;
    }

    void DestroyVideoTextureForTest(
        std::unique_ptr<VideoViewsTestBackend::PlatformTexture>&& platformTexture) override {
        // Exports the signal and ignores it.
        native::vulkan::ExternalImageExportInfoDmaBuf exportInfo;
        native::vulkan::ExportVulkanImage(platformTexture->wgpuTexture.Get(),
                                          VK_IMAGE_LAYOUT_UNDEFINED, &exportInfo);
        for (int fd : exportInfo.semaphoreHandles) {
            ASSERT_NE(fd, -1);
            close(fd);
        }
        gbm_bo* gbmBo = static_cast<PlatformTextureGbm*>(platformTexture.get())->GetGbmBo();
        ASSERT_NE(gbmBo, nullptr);
        gbm_bo_destroy(gbmBo);
    }

    WGPUDevice mWGPUDevice = nullptr;
    gbm_device* mGbmDevice = nullptr;
    int mRenderNodeFd = -1;
};

}  // anonymous namespace

// static
std::vector<BackendTestConfig> VideoViewsTestBackend::Backends() {
    return {VulkanBackend()};
}

// static
std::vector<Format> VideoViewsTestBackend::Formats() {
    // TODO(dawn:551): Support sharing P010 video surfaces.
    return {wgpu::TextureFormat::R8BG8Biplanar420Unorm};
}

// static
std::unique_ptr<VideoViewsTestBackend> VideoViewsTestBackend::Create() {
    return std::make_unique<VideoViewsTestBackendGbm>();
}

}  // namespace dawn
