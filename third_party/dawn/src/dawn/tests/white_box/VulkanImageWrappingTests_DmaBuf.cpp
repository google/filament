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

#include "src/dawn/tests/white_box/VulkanImageWrappingTests_DmaBuf.h"

#include <gbm.h>
#include <gtest/gtest.h>
#include <unistd.h>

#include <memory>
#include <utility>
#include <vector>

#include "partition_alloc/pointers/raw_ptr.h"
#include "src/dawn/common/DRMUtils.h"
#include "src/dawn/native/vulkan/DeviceVk.h"

namespace dawn::native::vulkan {

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
        OwnedGbmBo bo,
        int fd,
        std::array<PlaneLayout, ExternalImageDescriptorDmaBuf::kMaxPlanes> planeLayouts,
        uint64_t drmModifier)
        : mGbmBo(std::move(bo)), mFd(fd), planeLayouts(planeLayouts), drmModifier(drmModifier) {}

    ~ExternalTextureDmaBuf() override {
        if (mFd != -1) {
            close(mFd);
        }
    }

    int Dup() const { return dup(mFd); }

  private:
    OwnedGbmBo mGbmBo;
    int mFd = -1;

  public:
    const std::array<PlaneLayout, ExternalImageDescriptorDmaBuf::kMaxPlanes> planeLayouts;
    const uint64_t drmModifier;
};

class VulkanImageWrappingTestBackendDmaBuf : public VulkanImageWrappingTestBackend {
  public:
    explicit VulkanImageWrappingTestBackendDmaBuf(const wgpu::Device& device) {
        mDeviceVk = native::vulkan::ToBackend(native::FromAPI(device.Get()));
        wgpu::AdapterPropertiesDrm drmProperties;
        wgpu::AdapterInfo adapterInfo;
        adapterInfo.nextInChain = &drmProperties;
        if (device.GetAdapter().GetInfo(&adapterInfo) == wgpu::Status::Success &&
            drmProperties.hasRender) {
            mRenderNode = OpenDRMRenderNode(drmProperties.renderMajor, drmProperties.renderMinor);
            if (mRenderNode.IsValid()) {
                mGbmDevice.reset(gbm_create_device(mRenderNode.Get()));
            }
        }
    }

    bool SupportsTestParams(const TestParams& params) const override {
        // Even though this backend doesn't decide on creation whether the image should use
        // dedicated allocation, it still supports all options of NeedsDedicatedAllocation so we
        // test them.
        return mGbmDevice != nullptr &&
               (mDeviceVk->GetDeviceInfo().HasExt(DeviceExt::ExternalMemoryFD) &&
                mDeviceVk->GetDeviceInfo().HasExt(DeviceExt::ImageDrmFormatModifier));
    }

    std::unique_ptr<ExternalTexture> CreateTexture(uint32_t width,
                                                   uint32_t height,
                                                   wgpu::TextureFormat format,
                                                   wgpu::TextureUsage usage) override {
        EXPECT_EQ(format, wgpu::TextureFormat::RGBA8Unorm);

        OwnedGbmBo bo = CreateGbmBo(width, height, true);

        std::array<PlaneLayout, ExternalImageDescriptorDmaBuf::kMaxPlanes> planeLayouts = {};
        for (int plane = 0; plane < gbm_bo_get_plane_count(bo.get()); ++plane) {
            planeLayouts[plane].stride = gbm_bo_get_stride_for_plane(bo.get(), plane);
            planeLayouts[plane].offset = gbm_bo_get_offset(bo.get(), plane);
        }
        int fd = gbm_bo_get_fd(bo.get());
        uint64_t drmModifier = gbm_bo_get_modifier(bo.get());
        return std::make_unique<ExternalTextureDmaBuf>(std::move(bo), fd, planeLayouts,
                                                       drmModifier);
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

  private:
    OwnedGbmBo CreateGbmBo(uint32_t width, uint32_t height, bool linear) {
        uint32_t flags = GBM_BO_USE_RENDERING;
        if (linear) {
            flags |= GBM_BO_USE_LINEAR;
        }
        OwnedGbmBo gbmBo(
            gbm_bo_create(mGbmDevice.get(), width, height, GBM_FORMAT_XBGR8888, flags));
        EXPECT_NE(gbmBo, nullptr) << "Failed to create GBM buffer object";
        return gbmBo;
    }

    // Declare first so the render node outlives the GBM device.
    SystemHandle mRenderNode;
    OwnedGbmDevice mGbmDevice;
    raw_ptr<native::vulkan::Device> mDeviceVk;
};

std::unique_ptr<VulkanImageWrappingTestBackend> CreateDMABufBackend(const wgpu::Device& device) {
    return std::make_unique<VulkanImageWrappingTestBackendDmaBuf>(device);
}

}  // namespace dawn::native::vulkan
