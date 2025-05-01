/*
 * Copyright (C) 2018 The Android Open Source Project
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#ifndef TNT_FILAMENT_BACKEND_VULKANHANDLES_H
#define TNT_FILAMENT_BACKEND_VULKANHANDLES_H

// This needs to be at the top
#include "DriverBase.h"

#include "VulkanAsyncHandles.h"
#include "VulkanBuffer.h"
#include "VulkanFboCache.h"
#include "VulkanSwapChain.h"
#include "VulkanTexture.h"
#include "vulkan/memory/Resource.h"
#include "vulkan/utils/StaticVector.h"
#include "vulkan/utils/Definitions.h"

#include <backend/Program.h>

#include <utils/bitset.h>
#include <utils/FixedCapacityVector.h>
#include <utils/Mutex.h>
#include <utils/StructureOfArrays.h>

#include <array>

namespace filament::backend {

namespace {
// Counts the total number of descriptors for both vertex and fragment stages.
template<typename Bitmask>
inline uint8_t collapsedCount(Bitmask const& mask) {
    static_assert(sizeof(mask) <= 64);
    constexpr uint64_t VERTEX_MASK = (1ULL << fvkutils::getFragmentStageShift<Bitmask>()) - 1ULL;
    constexpr uint64_t FRAGMENT_MASK = (VERTEX_MASK << fvkutils::getFragmentStageShift<Bitmask>());
    uint64_t val = mask.getValue();
    val = ((val & VERTEX_MASK) >> fvkutils::getVertexStageShift<Bitmask>()) |
          ((val & FRAGMENT_MASK) >> fvkutils::getFragmentStageShift<Bitmask>());
    return (uint8_t) Bitmask(val).count();
}

} // anonymous namespace

struct VulkanBufferObject;

struct VulkanDescriptorSetLayout : public HwDescriptorSetLayout, fvkmemory::Resource {
    static constexpr uint8_t UNIQUE_DESCRIPTOR_SET_COUNT = 4;
    static constexpr uint8_t MAX_BINDINGS = 25;

    using DescriptorSetLayoutArray = std::array<VkDescriptorSetLayout,
            VulkanDescriptorSetLayout::UNIQUE_DESCRIPTOR_SET_COUNT>;

    // The bitmask representation of a set layout.
    struct Bitmask {
        fvkutils::UniformBufferBitmask ubo;         // 8 bytes
        fvkutils::UniformBufferBitmask dynamicUbo;  // 8 bytes
        fvkutils::SamplerBitmask sampler;           // 8 bytes
        fvkutils::InputAttachmentBitmask inputAttachment; // 8 bytes

        // This is a subset of the sampler field.
        fvkutils::SamplerBitmask externalSampler; // 8 bytes

        bool operator==(Bitmask const& right) const {
            return ubo == right.ubo && dynamicUbo == right.dynamicUbo && sampler == right.sampler &&
                   inputAttachment == right.inputAttachment &&
                   externalSampler == right.externalSampler;
        }

        static Bitmask fromLayoutDescription(DescriptorSetLayout const& layout);
    };
    static_assert(sizeof(Bitmask) == 40);

    // This is a convenience struct to quickly check layout compatibility in terms of descriptor set
    // pools.
    struct Count {
        uint32_t ubo = 0;
        uint32_t dynamicUbo = 0;
        uint32_t sampler = 0;
        uint32_t inputAttachment = 0;

        inline uint32_t total() const {
            return ubo + dynamicUbo + sampler + inputAttachment;
        }

        bool operator==(Count const& right) const noexcept {
            return ubo == right.ubo && dynamicUbo == right.dynamicUbo && sampler == right.sampler &&
                   inputAttachment == right.inputAttachment;
        }

        static inline Count fromLayoutBitmask(Bitmask const& mask) {
            return {
                .ubo = collapsedCount(mask.ubo),
                .dynamicUbo = collapsedCount(mask.dynamicUbo),
                .sampler = collapsedCount(mask.sampler),
                .inputAttachment = collapsedCount(mask.inputAttachment),
            };
        }

        Count operator*(uint16_t mult) const noexcept {
            // TODO: check for overflow.

            Count ret;
            ret.ubo = ubo * mult;
            ret.dynamicUbo = dynamicUbo * mult;
            ret.sampler = sampler * mult;
            ret.inputAttachment = inputAttachment * mult;
            return ret;
        }
    };

    VulkanDescriptorSetLayout(DescriptorSetLayout&& layout, VkDescriptorSetLayout vkLayout);

    // Note that we don't destroy the vklayout. This is done by the layout cache.
    ~VulkanDescriptorSetLayout() = default;

    VkDescriptorSetLayout getVkLayout() const noexcept { return mVkLayout; }

    VkDescriptorSetLayout getExternalSamplerVkLayout() const noexcept {
        return mExternalSamplerVkLayout;
    }

    void setExternalSamplerVkLayout(VkDescriptorSetLayout vklayout) noexcept {
        mExternalSamplerVkLayout = vklayout;
    }

    bool hasExternalSamplers() const noexcept { return bitmask.externalSampler.count() > 0; }

    Bitmask const bitmask;
    Count const count;

private:
    // This is the layout without any immutable samplers.
    VkDescriptorSetLayout const mVkLayout = VK_NULL_HANDLE;

    // This is the layout with immutable samplers, and can be updated.
    VkDescriptorSetLayout mExternalSamplerVkLayout = VK_NULL_HANDLE;
};

struct VulkanDescriptorSet : public HwDescriptorSet, fvkmemory::Resource {
public:
    // Because we need to recycle descriptor sets not used, we allow for a callback that the "Pool"
    // can use to repackage the vk handle.
    using OnRecycle = std::function<void(VulkanDescriptorSet*)>;

    VulkanDescriptorSet(
            fvkutils::UniformBufferBitmask const& dynamicUboMask,
            uint8_t uniqueDynamicUboCount,
            OnRecycle&& onRecycleFn, VkDescriptorSet vkSet)
        : dynamicUboMask(dynamicUboMask),
          uniqueDynamicUboCount(uniqueDynamicUboCount),
          mVkSet(vkSet),
          mOnRecycleFn(std::move(onRecycleFn)) {}

    // NOLINTNEXTLINE(bugprone-exception-escape)
    ~VulkanDescriptorSet() {
        if (mOnRecycleFn) {
            mOnRecycleFn(this);
        }
        if (mOnRecycleExternalSamplerFn) {
            mOnRecycleExternalSamplerFn(this);
        }
    }

    VkDescriptorSet getVkSet() const noexcept {
        return mVkSet;
    }

    VkDescriptorSet getExternalSamplerVkSet() const noexcept {
        return mExternalSamplerVkSet;
    }

    void setExternalSamplerVkSet(VkDescriptorSet vkset, OnRecycle onRecycle) {
        mExternalSamplerVkSet = vkset;
        if (mOnRecycleExternalSamplerFn) {
            mOnRecycleExternalSamplerFn(this);
        }
        mOnRecycleExternalSamplerFn = onRecycle;
    }

    void setOffsets(backend::DescriptorSetOffsetArray&& offsets) noexcept {
        mOffsets = std::move(offsets);
    }

    backend::DescriptorSetOffsetArray const* getOffsets() {
        return &mOffsets;
    }

    void acquire(fvkmemory::resource_ptr<VulkanTexture> texture);
    void acquire(fvkmemory::resource_ptr<VulkanBufferObject> buffer);

    fvkutils::UniformBufferBitmask const dynamicUboMask;
    uint8_t const uniqueDynamicUboCount;

private:
    VkDescriptorSet const mVkSet;
    VkDescriptorSet mExternalSamplerVkSet = VK_NULL_HANDLE;

    backend::DescriptorSetOffsetArray mOffsets;
    std::vector<fvkmemory::resource_ptr<fvkmemory::Resource>> mResources;
    OnRecycle mOnRecycleFn;
    OnRecycle mOnRecycleExternalSamplerFn;
};

using PushConstantNameArray = utils::FixedCapacityVector<char const*>;
using PushConstantNameByStage = std::array<PushConstantNameArray, Program::SHADER_TYPE_COUNT>;

struct PushConstantDescription {
    explicit PushConstantDescription(backend::Program const& program);

    VkPushConstantRange const* getVkRanges() const noexcept { return mRanges; }
    uint32_t getVkRangeCount() const noexcept { return mRangeCount; }
    void write(VkCommandBuffer cmdbuf, VkPipelineLayout layout, backend::ShaderStage stage,
            uint8_t index, backend::PushConstantVariant const& value);

private:
    static constexpr uint32_t ENTRY_SIZE = sizeof(uint32_t);

    utils::FixedCapacityVector<backend::ConstantType> mTypes[Program::SHADER_TYPE_COUNT];
    VkPushConstantRange mRanges[Program::SHADER_TYPE_COUNT];
    uint32_t mRangeCount;
};

struct VulkanProgram : public HwProgram, fvkmemory::Resource {
    using BindingList = fvkutils::StaticVector<uint16_t, MAX_SAMPLER_COUNT>;

    VulkanProgram(VkDevice device, Program const& builder) noexcept;
    ~VulkanProgram();

    inline VkShaderModule getVertexShader() const {
        return mInfo->shaders[0];
    }

    inline VkShaderModule getFragmentShader() const { return mInfo->shaders[1]; }

    inline uint32_t getPushConstantRangeCount() const {
        return mInfo->pushConstantDescription.getVkRangeCount();
    }

    inline VkPushConstantRange const* getPushConstantRanges() const {
        return mInfo->pushConstantDescription.getVkRanges();
    }

    inline void writePushConstant(VkCommandBuffer cmdbuf, VkPipelineLayout layout,
            backend::ShaderStage stage, uint8_t index, backend::PushConstantVariant const& value) {
        mInfo->pushConstantDescription.write(cmdbuf, layout, stage, index, value);
    }

    // TODO: handle compute shaders.
    // The expected order of shaders - from frontend to backend - is vertex, fragment, compute.
    static constexpr uint8_t const MAX_SHADER_MODULES = 2;

private:
    struct PipelineInfo {
        explicit PipelineInfo(backend::Program const& program) noexcept
            : pushConstantDescription(program)
            {}

        VkShaderModule shaders[MAX_SHADER_MODULES] = { VK_NULL_HANDLE };
        PushConstantDescription pushConstantDescription;
    };

    PipelineInfo* mInfo;
    VkDevice mDevice = VK_NULL_HANDLE;
};

// The render target bundles together a set of attachments, each of which can have one of the
// following ownership semantics:
//
// - The attachment's VkImage is shared and the owner is VulkanSwapChain (mOffscreen = false).
// - The attachment's VkImage is shared and the owner is VulkanTexture   (mOffscreen = true).
//
// We use private inheritance to shield clients from the width / height fields in HwRenderTarget,
// which are not representative when this is the default render target.
struct VulkanRenderTarget : private HwRenderTarget, fvkmemory::Resource {
    // Creates an offscreen render target.
    VulkanRenderTarget(VkDevice device, VkPhysicalDevice physicalDevice,
            VulkanContext const& context, fvkmemory::ResourceManager* resourceManager,
            VmaAllocator allocator, VulkanCommands* commands, uint32_t width, uint32_t height,
            uint8_t samples, VulkanAttachment color[MRT::MAX_SUPPORTED_RENDER_TARGET_COUNT],
            VulkanAttachment depthStencil[2], VulkanStagePool& stagePool, uint8_t layerCount);

    ~VulkanRenderTarget();

    // Creates a special "default" render target (i.e. associated with the swap chain)
    explicit VulkanRenderTarget();

    void transformClientRectToPlatform(VkRect2D* bounds) const;

    void transformViewportToPlatform(VkViewport* bounds) const;

    inline VkExtent2D getExtent() const {
        return {width, height};
    }

    inline VulkanAttachment& getColor0() const {
        assert_invariant(mInfo->colors[0]);
        return mInfo->attachments[0];
    }

    inline VulkanAttachment& getDepth() const {
        assert_invariant(hasDepth());
        if (mInfo->fbkey.samples == 1) {
            return mInfo->attachments[mInfo->depthIndex];
        }
        return mInfo->attachments[mInfo->msaaDepthIndex];
    }

    inline VulkanFboCache::RenderPassKey const& getRenderPassKey() const {
        return mInfo->rpkey;
    }

    inline VulkanFboCache::FboKey const& getFboKey() const {
        return mInfo->fbkey;
    }

    inline uint8_t getSamples() const {
        return mInfo->fbkey.samples;
    }

    uint8_t getColorTargetCount(VulkanRenderPass const& pass) const;

    inline bool hasDepth() const { return mInfo->depthIndex != Auxiliary::UNDEFINED_INDEX; }

    inline bool isSwapChain() const { return !mOffscreen; }
    inline bool isProtected() const { return mProtected; }

    void bindToSwapChain(fvkmemory::resource_ptr<VulkanSwapChain> swapchain);

    void emitBarriersBeginRenderPass(VulkanCommandBuffer& commands);

    void emitBarriersEndRenderPass(VulkanCommandBuffer& commands);

private:
    struct Auxiliary {
        static constexpr int8_t UNDEFINED_INDEX = -1;

        explicit Auxiliary() noexcept = default;

        VulkanFboCache::RenderPassKey rpkey = {};
        VulkanFboCache::FboKey fbkey = {};
        std::vector<VulkanAttachment> attachments;
        utils::bitset32 colors;
        int8_t depthIndex = UNDEFINED_INDEX;
        int8_t msaaDepthIndex = UNDEFINED_INDEX;
        int8_t msaaIndex = UNDEFINED_INDEX;
    };
    bool const mOffscreen;
    bool mProtected;

    std::unique_ptr<Auxiliary> mInfo;
};

struct VulkanBufferObject;

struct VulkanVertexBufferInfo : public HwVertexBufferInfo, fvkmemory::Resource {
    VulkanVertexBufferInfo(uint8_t bufferCount, uint8_t attributeCount,
            AttributeArray const& attributes);

    inline VkVertexInputAttributeDescription const* getAttribDescriptions() const {
        return mInfo.mSoa.data<PipelineInfo::ATTRIBUTE_DESCRIPTION>();
    }

    inline VkVertexInputBindingDescription const* getBufferDescriptions() const {
        return mInfo.mSoa.data<PipelineInfo::BUFFER_DESCRIPTION>();
    }

    inline int8_t const* getAttributeToBuffer() const {
        return mInfo.mSoa.data<PipelineInfo::ATTRIBUTE_TO_BUFFER_INDEX>();
    }

    inline VkDeviceSize const* getOffsets() const {
        return mInfo.mSoa.data<PipelineInfo::OFFSETS>();
    }

    size_t getAttributeCount() const noexcept {
        return mInfo.mSoa.size();
    }

private:
    struct PipelineInfo {
        PipelineInfo(size_t size) : mSoa(size /* capacity */) {
            mSoa.resize(size);
        }

        // These correspond to the index of the element in the SoA
        static constexpr uint8_t ATTRIBUTE_DESCRIPTION = 0;
        static constexpr uint8_t BUFFER_DESCRIPTION = 1;
        static constexpr uint8_t OFFSETS = 2;
        static constexpr uint8_t ATTRIBUTE_TO_BUFFER_INDEX = 3;

        utils::StructureOfArrays<
            VkVertexInputAttributeDescription,
            VkVertexInputBindingDescription,
            VkDeviceSize,
            int8_t
        > mSoa;
    };

    PipelineInfo mInfo;
};

struct VulkanVertexBuffer : public HwVertexBuffer, fvkmemory::Resource {
    VulkanVertexBuffer(VulkanContext& context, VulkanStagePool& stagePool, uint32_t vertexCount,
            fvkmemory::resource_ptr<VulkanVertexBufferInfo> vbi);
    void setBuffer(fvkmemory::resource_ptr<VulkanBufferObject> bufferObject, uint32_t index);

    inline VkBuffer const* getVkBuffers() const { return mBuffers.data(); }
    inline VkBuffer* getVkBuffers() { return mBuffers.data(); }
    fvkmemory::resource_ptr<VulkanVertexBufferInfo> vbi;

private:
    utils::FixedCapacityVector<VkBuffer> mBuffers;
    std::vector<fvkmemory::resource_ptr<VulkanBufferObject>> mResources;
};

struct VulkanIndexBuffer : public HwIndexBuffer, fvkmemory::Resource {
    VulkanIndexBuffer(VmaAllocator allocator, VulkanStagePool& stagePool, uint8_t elementSize,
            uint32_t indexCount)
        : HwIndexBuffer(elementSize, indexCount),
          buffer(allocator, stagePool, VK_BUFFER_USAGE_INDEX_BUFFER_BIT, elementSize * indexCount),
          indexType(elementSize == 2 ? VK_INDEX_TYPE_UINT16 : VK_INDEX_TYPE_UINT32) {}

    VulkanBuffer buffer;
    const VkIndexType indexType;
};

struct VulkanBufferObject : public HwBufferObject, fvkmemory::Resource {
    VulkanBufferObject(VmaAllocator allocator, VulkanStagePool& stagePool, uint32_t byteCount,
            BufferObjectBinding bindingType);

    VulkanBuffer buffer;
    const BufferObjectBinding bindingType;
};

struct VulkanRenderPrimitive : public HwRenderPrimitive, fvkmemory::Resource {
    VulkanRenderPrimitive(PrimitiveType pt, fvkmemory::resource_ptr<VulkanVertexBuffer> vb,
            fvkmemory::resource_ptr<VulkanIndexBuffer> ib);
    ~VulkanRenderPrimitive() = default;

    fvkmemory::resource_ptr<VulkanVertexBuffer> vertexBuffer;
    fvkmemory::resource_ptr<VulkanIndexBuffer> indexBuffer;
};

inline constexpr VkBufferUsageFlagBits getBufferObjectUsage(
        BufferObjectBinding bindingType) noexcept {
    switch(bindingType) {
        case BufferObjectBinding::VERTEX:
            return VK_BUFFER_USAGE_VERTEX_BUFFER_BIT;
        case BufferObjectBinding::UNIFORM:
            return VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT;
        case BufferObjectBinding::SHADER_STORAGE:
            return VK_BUFFER_USAGE_STORAGE_BUFFER_BIT;
        // when adding more buffer-types here, make sure to update VulkanBuffer::loadFromCpu()
        // if necessary.
    }
}

} // namespace filament::backend

#endif // TNT_FILAMENT_BACKEND_VULKANHANDLES_H
