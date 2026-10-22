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

#include <gbm.h>
#include <webgpu/webgpu_cpp.h>

#include <array>
#include <string>
#include <utility>
#include <vector>

#include "src/utils/compiler.h"

// This must be included instead of vulkan.h so that we can wrap it with vulkan_platform.h.
#include "src/dawn/common/DRMUtils.h"
#include "src/dawn/common/vulkan_platform.h"
#include "src/dawn/tests/white_box/SharedTextureMemoryTests.h"

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
                wgpu::FeatureName::DawnMultiPlanarFormats, wgpu::FeatureName::AdapterPropertiesDrm,
                FenceFeature};
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
        OwnedGbmBo bo(gbm_bo_create(mGbmDevice.get(), size, size, format, usage));
        EXPECT_NE(bo, nullptr) << "Failed to create GBM buffer object";

        wgpu::SharedTextureMemoryDmaBufDescriptor dmaBufDesc;
        dmaBufDesc.size = {size, size};
        dmaBufDesc.drmFormat = format;
        dmaBufDesc.drmModifier = gbm_bo_get_modifier(bo.get());

        std::array<wgpu::SharedTextureMemoryDmaBufPlane, GBM_MAX_PLANES> planes = {};
        const int gbmPlaneCount = gbm_bo_get_plane_count(bo.get());
        DAWN_ASSERT(gbmPlaneCount > 0);
        const size_t planeCount = static_cast<size_t>(gbmPlaneCount);
        DAWN_ASSERT(planeCount <= planes.size());
        dmaBufDesc.planeCount = planeCount;
        dmaBufDesc.planes = planes.data();

        std::array<SystemHandle, GBM_MAX_PLANES> planeFDs;
        for (size_t plane = 0; plane < planeCount; ++plane) {
            const int gbmPlane = static_cast<int>(plane);
            planeFDs[plane] = SystemHandle::Acquire(gbm_bo_get_fd(bo.get()));
            EXPECT_TRUE(planeFDs[plane].IsValid());
            planes[plane].fd = planeFDs[plane].Get();
            planes[plane].stride = gbm_bo_get_stride_for_plane(bo.get(), gbmPlane);
            planes[plane].offset = gbm_bo_get_offset(bo.get(), gbmPlane);
        }

        std::string label = MakeLabel(dmaBufDesc);
        wgpu::SharedTextureMemoryDescriptor desc;
        desc.label = label.c_str();
        desc.nextInChain = &dmaBufDesc;

        return createFn(desc);
    }

    // Create one basic shared texture memory. It should support most operations.
    wgpu::SharedTextureMemory CreateSharedTextureMemory(const wgpu::Device& device,
                                                        int layerCount) override {
        auto format = GBM_FORMAT_ABGR8888;
        auto usage = GBM_BO_USE_LINEAR;

        DAWN_ASSERT(gbm_device_is_format_supported(mGbmDevice.get(), format, usage));

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
                if (!gbm_device_is_format_supported(mGbmDevice.get(), format, usage)) {
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
        wgpu::AdapterPropertiesDrm drmProperties;
        wgpu::AdapterInfo adapterInfo;
        adapterInfo.nextInChain = &drmProperties;
        DAWN_TEST_UNSUPPORTED_IF(device.GetAdapter().GetInfo(&adapterInfo) !=
                                     wgpu::Status::Success ||
                                 !drmProperties.hasRender);
        SystemHandle renderNode =
            OpenDRMRenderNode(drmProperties.renderMajor, drmProperties.renderMinor);
        DAWN_TEST_UNSUPPORTED_IF(!renderNode.IsValid());

        OwnedGbmDevice gbmDevice(gbm_create_device(renderNode.Get()));
        DAWN_TEST_UNSUPPORTED_IF(gbmDevice == nullptr);

        // Make sure we can successfully create a basic buffer object.
        OwnedGbmBo bo(
            gbm_bo_create(gbmDevice.get(), 16, 16, GBM_FORMAT_XBGR8888, GBM_BO_USE_LINEAR));
        DAWN_TEST_UNSUPPORTED_IF(bo == nullptr);

        mGbmDevice = std::move(gbmDevice);
        mRenderNode = std::move(renderNode);
    }

    // Declare first so the render node outlives the GBM device.
    SystemHandle mRenderNode;
    OwnedGbmDevice mGbmDevice;
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
