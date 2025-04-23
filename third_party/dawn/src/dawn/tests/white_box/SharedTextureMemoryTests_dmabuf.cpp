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

#include <fcntl.h>
#include <gbm.h>
#include <unistd.h>
#include <webgpu/webgpu_cpp.h>

#include <memory>
#include <string>
#include <utility>
#include <vector>

// This must be included instead of vulkan.h so that we can wrap it with vulkan_platform.h.
#include "dawn/common/vulkan_platform.h"

#include "dawn/tests/white_box/SharedTextureMemoryTests.h"

namespace dawn {
namespace {

template <wgpu::FeatureName FenceFeature>
class Backend : public SharedTextureMemoryTestVulkanBackend {
  public:
    static SharedTextureMemoryTestBackend* GetInstance() {
        static Backend b;
        return &b;
    }

    std::string Name() const override {
        switch (FenceFeature) {
            case wgpu::FeatureName::SharedFenceVkSemaphoreOpaqueFD:
                return "dma buf, opaque fd";
            case wgpu::FeatureName::SharedFenceSyncFD:
                return "dma buf, sync fd";
            default:
                DAWN_UNREACHABLE();
        }
    }

    std::vector<wgpu::FeatureName> RequiredFeatures(const wgpu::Adapter&) const override {
        return {wgpu::FeatureName::SharedTextureMemoryDmaBuf,
                wgpu::FeatureName::DawnMultiPlanarFormats, FenceFeature};
    }

    static std::string MakeLabel(const wgpu::SharedTextureMemoryDmaBufDescriptor& desc) {
        // Internally, the GBM enums are defined as their fourcc values. Cast to that and use
        // it as the label. The fourcc value is a four-character name that can be
        // interpreted as a 32-bit integer enum ('ABGR', 'r011', etc.)
        return std::string(reinterpret_cast<const char*>(&desc.drmFormat), 4) + " " +
               "modifier:" + std::to_string(desc.drmModifier) + " " +
               std::to_string(desc.size.width) + "x" + std::to_string(desc.size.height);
    }

    template <typename CreateFn>
    auto CreateSharedTextureMemoryHelper(uint32_t size,
                                         uint32_t format,
                                         uint32_t usage,
                                         CreateFn createFn) {
        gbm_bo* bo = gbm_bo_create(mGbmDevice, size, size, format, usage);
        EXPECT_NE(bo, nullptr) << "Failed to create GBM buffer object";

        wgpu::SharedTextureMemoryDmaBufDescriptor dmaBufDesc;
        dmaBufDesc.size = {size, size};
        dmaBufDesc.drmFormat = format;
        dmaBufDesc.drmModifier = gbm_bo_get_modifier(bo);

        wgpu::SharedTextureMemoryDmaBufPlane planes[GBM_MAX_PLANES];
        dmaBufDesc.planeCount = gbm_bo_get_plane_count(bo);
        dmaBufDesc.planes = planes;
        DAWN_ASSERT(dmaBufDesc.planeCount <= GBM_MAX_PLANES);

        for (uint32_t plane = 0; plane < dmaBufDesc.planeCount; ++plane) {
            planes[plane].fd = gbm_bo_get_fd(bo);
            planes[plane].stride = gbm_bo_get_stride_for_plane(bo, plane);
            planes[plane].offset = gbm_bo_get_offset(bo, plane);
        }

        std::string label = MakeLabel(dmaBufDesc);
        wgpu::SharedTextureMemoryDescriptor desc;
        desc.label = label.c_str();
        desc.nextInChain = &dmaBufDesc;

        auto ret = createFn(desc);

        for (uint32_t plane = 0; plane < dmaBufDesc.planeCount; ++plane) {
            close(planes[plane].fd);
        }
        gbm_bo_destroy(bo);

        return ret;
    }

    // Create one basic shared texture memory. It should support most operations.
    wgpu::SharedTextureMemory CreateSharedTextureMemory(const wgpu::Device& device,
                                                        int layerCount) override {
        auto format = GBM_FORMAT_ABGR8888;
        auto usage = GBM_BO_USE_LINEAR;

        DAWN_ASSERT(gbm_device_is_format_supported(mGbmDevice, format, usage));

        return CreateSharedTextureMemoryHelper(
            16, format, usage, [&](const wgpu::SharedTextureMemoryDescriptor& desc) {
                return device.ImportSharedTextureMemory(&desc);
            });
    }

    std::vector<std::vector<wgpu::SharedTextureMemory>> CreatePerDeviceSharedTextureMemories(
        const std::vector<wgpu::Device>& devices,
        int layerCount) override {
        std::vector<std::vector<wgpu::SharedTextureMemory>> memories;
        for (uint32_t format : {
                 GBM_FORMAT_R8,
                 GBM_FORMAT_GR88,
                 GBM_FORMAT_ABGR8888,
                 GBM_FORMAT_ARGB8888,
                 GBM_FORMAT_XBGR8888,
                 GBM_FORMAT_XRGB8888,
                 GBM_FORMAT_ABGR2101010,
                 GBM_FORMAT_NV12,
             }) {
            for (gbm_bo_flags usage : {
                     gbm_bo_flags(0),
                     GBM_BO_USE_LINEAR,
                     GBM_BO_USE_RENDERING,
                     gbm_bo_flags(GBM_BO_USE_RENDERING | GBM_BO_USE_LINEAR),
                 }) {
                if (!gbm_device_is_format_supported(mGbmDevice, format, usage)) {
                    continue;
                }
                for (uint32_t size : {4, 64}) {
                    CreateSharedTextureMemoryHelper(
                        size, format, usage, [&](const wgpu::SharedTextureMemoryDescriptor& desc) {
                            std::vector<wgpu::SharedTextureMemory> perDeviceMemories;
                            for (auto& device : devices) {
                                perDeviceMemories.push_back(
                                    device.ImportSharedTextureMemory(&desc));
                            }
                            memories.push_back(std::move(perDeviceMemories));
                            return true;
                        });
                }
            }
        }
        return memories;
    }

  private:
    void SetUp(const wgpu::Device& device) override {
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

        for (uint32_t i = kRenderNodeStart; i < kRenderNodeEnd; i++) {
            std::string renderNode = kRenderNodeTemplate + std::to_string(i);
            mRenderNodeFd = open(renderNode.c_str(), O_RDWR);
            if (mRenderNodeFd >= 0) {
                break;
            }
        }

        // Failed to get file descriptor for render node and mGbmDevice is nullptr.
        DAWN_TEST_UNSUPPORTED_IF(mRenderNodeFd < 0);

        mGbmDevice = gbm_create_device(mRenderNodeFd);
        DAWN_TEST_UNSUPPORTED_IF(mGbmDevice == nullptr);

        // Make sure we can successfully create a basic buffer object.
        gbm_bo* bo = gbm_bo_create(mGbmDevice, 16, 16, GBM_FORMAT_XBGR8888, GBM_BO_USE_LINEAR);
        if (bo != nullptr) {
            gbm_bo_destroy(bo);
        }
        DAWN_TEST_UNSUPPORTED_IF(bo == nullptr);
    }

    void TearDown() override {
        if (mGbmDevice) {
            gbm_device_destroy(mGbmDevice);
            mGbmDevice = nullptr;
        }
        if (mRenderNodeFd >= 0) {
            close(mRenderNodeFd);
        }
    }

    int mRenderNodeFd = -1;
    gbm_device* mGbmDevice = nullptr;
};

DAWN_INSTANTIATE_PREFIXED_TEST_P(
    Vulkan,
    SharedTextureMemoryNoFeatureTests,
    {VulkanBackend()},
    {Backend<wgpu::FeatureName::SharedFenceVkSemaphoreOpaqueFD>::GetInstance(),
     Backend<wgpu::FeatureName::SharedFenceSyncFD>::GetInstance()},
    {1});

DAWN_INSTANTIATE_PREFIXED_TEST_P(
    Vulkan,
    SharedTextureMemoryTests,
    {VulkanBackend()},
    {Backend<wgpu::FeatureName::SharedFenceVkSemaphoreOpaqueFD>::GetInstance(),
     Backend<wgpu::FeatureName::SharedFenceSyncFD>::GetInstance()},
    {1});

}  // anonymous namespace
}  // namespace dawn
