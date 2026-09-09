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

#include <cstring>
#include <memory>
#include <utility>
#include <vector>

// gbm.h transitively include X11 headers which have problematic #define for common names (like
// "Always") preemptively include xlib_with_undefs.h that will do the necessary #undefs before gbm.h
// tries to include X11.h.
#include <gbm.h>

#include "src/dawn/common/xlib_with_undefs.h"
// Comment to prevent reordering.

#include "dawn/native/VulkanBackend.h"
#include "src/dawn/common/DRMUtils.h"
#include "src/dawn/tests/end2end/VideoViewsTests.h"
#include "src/utils/assert.h"

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
    PlatformTextureGbm(wgpu::Texture&& texture, OwnedGbmBo gbmBo)
        : PlatformTexture(std::move(texture)), mGbmBo(std::move(gbmBo)) {}
    ~PlatformTextureGbm() override = default;

    // TODO(chromium:1258986): Add DISJOINT vkImage support for multi-plannar formats.
    bool CanWrapAsWGPUTexture() override {
        DAWN_ASSERT(mGbmBo != nullptr);
        // Checks if all plane handles of a multi-planar gbm_bo are same.
        gbm_bo_handle plane0Handle = gbm_bo_get_handle_for_plane(mGbmBo.get(), 0);
        for (int plane = 1; plane < gbm_bo_get_plane_count(mGbmBo.get()); ++plane) {
            if (gbm_bo_get_handle_for_plane(mGbmBo.get(), plane).u32 != plane0Handle.u32) {
                return false;
            }
        }
        return true;
    }

  private:
    OwnedGbmBo mGbmBo;
};

class VideoViewsTestBackendGbm : public VideoViewsTestBackend {
  public:
    bool Initialize(const wgpu::Device& device) override {
        mWGPUDevice = device.Get();
        wgpu::AdapterPropertiesDrm drmProperties;
        wgpu::AdapterInfo adapterInfo;
        adapterInfo.nextInChain = &drmProperties;
        if (device.GetAdapter().GetInfo(&adapterInfo) != wgpu::Status::Success ||
            !drmProperties.hasRender) {
            return false;
        }

        SystemHandle renderNode =
            OpenDRMRenderNode(drmProperties.renderMajor, drmProperties.renderMinor);
        if (!renderNode.IsValid()) {
            return false;
        }

        OwnedGbmDevice gbmDevice(gbm_create_device(renderNode.Get()));
        if (gbmDevice == nullptr) {
            return false;
        }

        mGbmDevice = std::move(gbmDevice);
        mRenderNode = std::move(renderNode);
        return true;
    }

  private:
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
        OwnedGbmBo gbmBo(gbm_bo_create(
            mGbmDevice.get(), VideoViewsTestsBase::kYUVAImageDataWidthInTexels,
            VideoViewsTestsBase::kYUVAImageDataHeightInTexels, GetGbmBoFormat(format), flags));
        if (gbmBo == nullptr) {
            ADD_FAILURE() << "Failed to create GBM buffer object";
            return nullptr;
        }

        if (initialized) {
            void* mapHandle = nullptr;
            uint32_t strideBytes = 0;
            void* addr =
                gbm_bo_map(gbmBo.get(), 0, 0, VideoViewsTestsBase::kYUVAImageDataWidthInTexels,
                           VideoViewsTestsBase::kYUVAImageDataHeightInTexels, GBM_BO_TRANSFER_WRITE,
                           &strideBytes, &mapHandle);
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

            gbm_bo_unmap(gbmBo.get(), mapHandle);
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

        SystemHandle memoryFD = SystemHandle::Acquire(gbm_bo_get_fd(gbmBo.get()));
        descriptor.memoryFD = memoryFD.Get();
        for (int plane = 0; plane < gbm_bo_get_plane_count(gbmBo.get()); ++plane) {
            descriptor.planeLayouts[plane].stride = gbm_bo_get_stride_for_plane(gbmBo.get(), plane);
            descriptor.planeLayouts[plane].offset = gbm_bo_get_offset(gbmBo.get(), plane);
        }
        descriptor.drmModifier = gbm_bo_get_modifier(gbmBo.get());
        descriptor.waitFDs = {};

        auto texture = std::make_unique<PlatformTextureGbm>(
            native::vulkan::WrapVulkanImage(mWGPUDevice, &descriptor), std::move(gbmBo));
        if (texture->wgpuTexture) {
            memoryFD.Detach();
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
    }

    WGPUDevice mWGPUDevice = nullptr;
    // Declare first so the render node outlives the GBM device.
    SystemHandle mRenderNode;
    OwnedGbmDevice mGbmDevice;
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
