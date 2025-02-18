// Copyright 2020 The Dawn & Tint Authors
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
#include <gtest/gtest.h>
#include <unistd.h>

#include <memory>
#include <string>
#include <utility>
#include <vector>

#include "dawn/native/vulkan/DeviceVk.h"
#include "dawn/tests/white_box/VulkanImageWrappingTests_DmaBuf.h"
#include "partition_alloc/pointers/raw_ptr.h"

namespace dawn::native::vulkan {

namespace {

struct GbmDeviceDeleter {
    void operator()(gbm_device* ptr) { gbm_device_destroy(ptr); }
};
struct GbmBoDeleter {
    void operator()(gbm_bo* ptr) { gbm_bo_destroy(ptr); }
};

}  // namespace

class ExternalSemaphoreDmaBuf : public VulkanImageWrappingTestBackend::ExternalSemaphore {
  public:
    explicit ExternalSemaphoreDmaBuf(int handle) : mHandle(handle) {}
    ~ExternalSemaphoreDmaBuf() override {
        if (mHandle != -1) {
            close(mHandle);
        }
    }
    int AcquireHandle() {
        int handle = mHandle;
        mHandle = -1;
        return handle;
    }

  private:
    int mHandle = -1;
};

class ExternalTextureDmaBuf : public VulkanImageWrappingTestBackend::ExternalTexture {
  public:
    ExternalTextureDmaBuf(
        gbm_bo* bo,
        int fd,
        std::array<PlaneLayout, ExternalImageDescriptorDmaBuf::kMaxPlanes> planeLayouts,
        uint64_t drmModifier)
        : mGbmBo(bo), mFd(fd), planeLayouts(planeLayouts), drmModifier(drmModifier) {}

    ~ExternalTextureDmaBuf() override {
        if (mFd != -1) {
            close(mFd);
        }
    }

    int Dup() const { return dup(mFd); }

  private:
    std::unique_ptr<gbm_bo, GbmBoDeleter> mGbmBo;
    int mFd = -1;

  public:
    const std::array<PlaneLayout, ExternalImageDescriptorDmaBuf::kMaxPlanes> planeLayouts;
    const uint64_t drmModifier;
};

class VulkanImageWrappingTestBackendDmaBuf : public VulkanImageWrappingTestBackend {
  public:
    explicit VulkanImageWrappingTestBackendDmaBuf(const wgpu::Device& device) {
        mDeviceVk = native::vulkan::ToBackend(native::FromAPI(device.Get()));
        CreateGbmDevice();
    }

    bool SupportsTestParams(const TestParams& params) const override {
        // Even though this backend doesn't decide on creation whether the image should use
        // dedicated allocation, it still supports all options of NeedsDedicatedAllocation so we
        // test them.
        return mGbmDevice != nullptr &&
               (mDeviceVk->GetDeviceInfo().HasExt(DeviceExt::ExternalMemoryFD) &&
                mDeviceVk->GetDeviceInfo().HasExt(DeviceExt::ImageDrmFormatModifier)) &&
               (!params.useDedicatedAllocation ||
                mDeviceVk->GetDeviceInfo().HasExt(DeviceExt::DedicatedAllocation));
    }

    std::unique_ptr<ExternalTexture> CreateTexture(uint32_t width,
                                                   uint32_t height,
                                                   wgpu::TextureFormat format,
                                                   wgpu::TextureUsage usage) override {
        EXPECT_EQ(format, wgpu::TextureFormat::RGBA8Unorm);

        gbm_bo* bo = CreateGbmBo(width, height, true);

        std::array<PlaneLayout, ExternalImageDescriptorDmaBuf::kMaxPlanes> planeLayouts;
        for (int plane = 0; plane < gbm_bo_get_plane_count(bo); ++plane) {
            planeLayouts[plane].stride = gbm_bo_get_stride_for_plane(bo, plane);
            planeLayouts[plane].offset = gbm_bo_get_offset(bo, plane);
        }
        return std::make_unique<ExternalTextureDmaBuf>(bo, gbm_bo_get_fd(bo), planeLayouts,
                                                       gbm_bo_get_modifier(bo));
    }

    wgpu::Texture WrapImage(const wgpu::Device& device,
                            const ExternalTexture* texture,
                            const ExternalImageDescriptorVkForTesting& descriptor,
                            std::vector<std::unique_ptr<ExternalSemaphore>> semaphores) override {
        const ExternalTextureDmaBuf* textureDmaBuf =
            static_cast<const ExternalTextureDmaBuf*>(texture);
        std::vector<int> waitFDs;
        for (auto& semaphore : semaphores) {
            waitFDs.push_back(
                static_cast<ExternalSemaphoreDmaBuf*>(semaphore.get())->AcquireHandle());
        }

        ExternalImageDescriptorDmaBuf descriptorDmaBuf;
        *static_cast<ExternalImageDescriptorVk*>(&descriptorDmaBuf) = descriptor;

        descriptorDmaBuf.memoryFD = textureDmaBuf->Dup();
        descriptorDmaBuf.waitFDs = std::move(waitFDs);

        descriptorDmaBuf.planeLayouts = textureDmaBuf->planeLayouts;
        descriptorDmaBuf.drmModifier = textureDmaBuf->drmModifier;

        return wgpu::Texture::Acquire(
            native::vulkan::WrapVulkanImage(device.Get(), &descriptorDmaBuf));
    }

    bool ExportImage(const wgpu::Texture& texture,
                     ExternalImageExportInfoVkForTesting* exportInfo) override {
        ExternalImageExportInfoDmaBuf infoDmaBuf;
        bool success = ExportVulkanImage(texture.Get(), VK_IMAGE_LAYOUT_UNDEFINED, &infoDmaBuf);

        *static_cast<ExternalImageExportInfoVk*>(exportInfo) = infoDmaBuf;
        for (int fd : infoDmaBuf.semaphoreHandles) {
            EXPECT_NE(fd, -1);
            exportInfo->semaphores.push_back(std::make_unique<ExternalSemaphoreDmaBuf>(fd));
        }

        return success;
    }

    void CreateGbmDevice() {
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

        int renderNodeFd = -1;
        for (uint32_t i = kRenderNodeStart; i < kRenderNodeEnd; i++) {
            std::string renderNode = kRenderNodeTemplate + std::to_string(i);
            renderNodeFd = open(renderNode.c_str(), O_RDWR);
            if (renderNodeFd >= 0) {
                break;
            }
        }

        // Failed to get file descriptor for render node and mGbmDevice is nullptr.
        if (renderNodeFd < 0) {
            return;
        }

        // Might be failed to create GBM device and mGbmDevice is nullptr.
        mGbmDevice.reset(gbm_create_device(renderNodeFd));
    }

  private:
    gbm_bo* CreateGbmBo(uint32_t width, uint32_t height, bool linear) {
        uint32_t flags = GBM_BO_USE_RENDERING;
        if (linear) {
            flags |= GBM_BO_USE_LINEAR;
        }
        gbm_bo* gbmBo = gbm_bo_create(mGbmDevice.get(), width, height, GBM_FORMAT_XBGR8888, flags);
        EXPECT_NE(gbmBo, nullptr) << "Failed to create GBM buffer object";
        return gbmBo;
    }

    std::unique_ptr<gbm_device, GbmDeviceDeleter> mGbmDevice;
    raw_ptr<native::vulkan::Device> mDeviceVk;
};

std::unique_ptr<VulkanImageWrappingTestBackend> CreateDMABufBackend(const wgpu::Device& device) {
    return std::make_unique<VulkanImageWrappingTestBackendDmaBuf>(device);
}

}  // namespace dawn::native::vulkan
