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

#include "VulkanBuffer.h"
#include "VulkanResources.h"
#include "VulkanSwapChain.h"
#include "VulkanTexture.h"
#include "VulkanUtility.h"

#include <private/backend/SamplerGroup.h>
#include <backend/Program.h>

#include <utils/FixedCapacityVector.h>
#include <utils/Mutex.h>
#include <utils/StructureOfArrays.h>

namespace filament::backend {

using namespace descset;

class VulkanTimestamps;
struct VulkanBufferObject;

struct VulkanDescriptorSetLayout : public VulkanResource, HwDescriptorSetLayout {
    static constexpr uint8_t UNIQUE_DESCRIPTOR_SET_COUNT = 4;
    static constexpr uint8_t MAX_BINDINGS = 25;

    using DescriptorSetLayoutArray = std::array<VkDescriptorSetLayout,
            VulkanDescriptorSetLayout::UNIQUE_DESCRIPTOR_SET_COUNT>;

    // The bitmask representation of a set layout.
    struct Bitmask {
        // TODO: better utiltize the space below and use bitset instead.
        UniformBufferBitmask ubo = 0;         // 8 bytes
        UniformBufferBitmask dynamicUbo = 0;  // 8 bytes
        SamplerBitmask sampler = 0;           // 8 bytes
        InputAttachmentBitmask inputAttachment = 0; // 8 bytes

        bool operator==(Bitmask const& right) const {
            return ubo == right.ubo && dynamicUbo == right.dynamicUbo && sampler == right.sampler &&
                   inputAttachment == right.inputAttachment;
        }
    };
    static_assert(sizeof(Bitmask) == 32);

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
                    .ubo = countBits(collapseStages(mask.ubo)),
                    .dynamicUbo = countBits(collapseStages(mask.dynamicUbo)),
                    .sampler = countBits(collapseStages(mask.sampler)),
                    .inputAttachment = countBits(collapseStages(mask.inputAttachment)),
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

    static_assert(sizeof(Bitmask) % 8 == 0);

    VulkanDescriptorSetLayout(VkDevice device, DescriptorSetLayout const& layout);

    ~VulkanDescriptorSetLayout();

    VkDevice const mDevice;
    VkDescriptorSetLayout const vklayout;
    Bitmask const bitmask;

    // This is a convenience struct so that we don't have to iterate through all the bits of the
    // bitmask (which correspondings to binding indices).
    struct _Bindings {
        utils::FixedCapacityVector<uint8_t> const ubo;
        utils::FixedCapacityVector<uint8_t> const dynamicUbo;
        utils::FixedCapacityVector<uint8_t> const sampler;
        utils::FixedCapacityVector<uint8_t> const inputAttachment;
    } bindings;

    Count const count;

private:

    template <typename MaskType>
    utils::FixedCapacityVector<uint8_t> bits(MaskType mask) {
        utils::FixedCapacityVector<uint8_t> ret =
                utils::FixedCapacityVector<uint8_t>::with_capacity(countBits(mask));
        for (uint8_t i = 0; i < sizeof(mask) * 4; ++i) {
            if (mask & (1 << i)) {
                ret.push_back(i);
            }
        }
        return ret;
    }

    _Bindings getBindings(Bitmask const& bitmask) {
        auto const uboCollapsed = collapseStages(bitmask.ubo);
        auto const dynamicUboCollapsed = collapseStages(bitmask.dynamicUbo);
        auto const samplerCollapsed = collapseStages(bitmask.sampler);
        auto const inputAttachmentCollapsed = collapseStages(bitmask.inputAttachment);
        return {
            bits(uboCollapsed),
            bits(dynamicUboCollapsed),
            bits(samplerCollapsed),
            bits(inputAttachmentCollapsed),
        };
    }
};

struct VulkanDescriptorSet : public VulkanResource, HwDescriptorSet {
public:
    // Because we need to recycle descriptor sets not used, we allow for a callback that the "Pool"
    // can use to repackage the vk handle.
    using OnRecycle = std::function<void()>;

    VulkanDescriptorSet(VulkanResourceAllocator* allocator, VkDescriptorSet rawSet,
            OnRecycle&& onRecycleFn)
        : VulkanResource(VulkanResourceType::DESCRIPTOR_SET),
          vkSet(rawSet),
          mResources(allocator),
          mOnRecycleFn(std::move(onRecycleFn)) {}

    ~VulkanDescriptorSet() {
        if (mOnRecycleFn) {
            mOnRecycleFn();
        }
    }

    void acquire(VulkanTexture* texture);

    void acquire(VulkanBufferObject* texture);

    bool hasTexture(VulkanTexture* texture) {
        return std::any_of(mTextures.begin(), mTextures.end(),
                [texture](auto t) { return t == texture; });
    }

    // TODO: maybe change to fixed size for performance.
    VkDescriptorSet const vkSet;

private:
    std::array<VulkanTexture*, 16> mTextures = { nullptr };
    uint8_t mTextureCount = 0;
    VulkanAcquireOnlyResourceManager mResources;
    OnRecycle mOnRecycleFn;
};

using PushConstantNameArray = utils::FixedCapacityVector<char const*>;
using PushConstantNameByStage = std::array<PushConstantNameArray, Program::SHADER_TYPE_COUNT>;

struct PushConstantDescription {

    explicit PushConstantDescription(backend::Program const& program) noexcept;

    VkPushConstantRange const* getVkRanges() const noexcept { return mRanges; }

    uint32_t getVkRangeCount() const noexcept { return mRangeCount; }

    void write(VulkanCommands* commands, VkPipelineLayout layout, backend::ShaderStage stage,
            uint8_t index, backend::PushConstantVariant const& value);

private:
    static constexpr uint32_t ENTRY_SIZE = sizeof(uint32_t);

    utils::FixedCapacityVector<backend::ConstantType> mTypes[Program::SHADER_TYPE_COUNT];
    VkPushConstantRange mRanges[Program::SHADER_TYPE_COUNT];
    uint32_t mRangeCount;
};

struct VulkanProgram : public HwProgram, VulkanResource {

    using BindingList = CappedArray<uint16_t, MAX_SAMPLER_COUNT>;

    VulkanProgram(VkDevice device, Program const& builder) noexcept;

    ~VulkanProgram();

    inline VkShaderModule getVertexShader() const {
        return mInfo->shaders[0];
    }

    inline VkShaderModule getFragmentShader() const { return mInfo->shaders[1]; }

    inline utils::FixedCapacityVector<uint16_t> const& getBindingToSamplerIndex() const {
        return mInfo->bindingToSamplerIndex;
    }

    // Get a list of the sampler binding indices so that we don't have to loop through all possible
    // samplers.
    inline BindingList const& getBindings() const { return mInfo->bindings; }

    inline uint32_t getPushConstantRangeCount() const {
        return mInfo->pushConstantDescription.getVkRangeCount();
    }

    inline VkPushConstantRange const* getPushConstantRanges() const {
        return mInfo->pushConstantDescription.getVkRanges();
    }

    inline void writePushConstant(VulkanCommands* commands, VkPipelineLayout layout,
            backend::ShaderStage stage, uint8_t index, backend::PushConstantVariant const& value) {
        mInfo->pushConstantDescription.write(commands, layout, stage, index, value);
    }

#if FVK_ENABLED_DEBUG_SAMPLER_NAME
    inline utils::FixedCapacityVector<std::string> const& getBindingToName() const {
        return mInfo->bindingToName;
    }
#endif

    // TODO: handle compute shaders.
    // The expected order of shaders - from frontend to backend - is vertex, fragment, compute.
    static constexpr uint8_t const MAX_SHADER_MODULES = 2;

private:
    struct PipelineInfo {
        explicit PipelineInfo(backend::Program const& program) noexcept
            : bindingToSamplerIndex(MAX_SAMPLER_COUNT, 0xffff),
              pushConstantDescription(program)
#if FVK_ENABLED_DEBUG_SAMPLER_NAME
            , bindingToName(MAX_SAMPLER_COUNT, "")
#endif
            {}

        BindingList bindings;

        // We store the samplerGroupIndex as the top 8-bit and the index within each group as the lower 8-bit.
        utils::FixedCapacityVector<uint16_t> bindingToSamplerIndex;
        VkShaderModule shaders[MAX_SHADER_MODULES] = { VK_NULL_HANDLE };

        PushConstantDescription pushConstantDescription;

#if FVK_ENABLED_DEBUG_SAMPLER_NAME
        // We store the sampler name mapped from binding index (only for debug purposes).
        utils::FixedCapacityVector<std::string> bindingToName;
#endif

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
struct VulkanRenderTarget : private HwRenderTarget, VulkanResource {
    // Creates an offscreen render target.
    VulkanRenderTarget(VkDevice device, VkPhysicalDevice physicalDevice,
            VulkanContext const& context, VmaAllocator allocator,
            VulkanCommands* commands,
            VulkanResourceAllocator* handleAllocator,
            uint32_t width, uint32_t height,
            uint8_t samples, VulkanAttachment color[MRT::MAX_SUPPORTED_RENDER_TARGET_COUNT],
            VulkanAttachment depthStencil[2], VulkanStagePool& stagePool, uint8_t layerCount);

    // Creates a special "default" render target (i.e. associated with the swap chain)
    explicit VulkanRenderTarget();

    void transformClientRectToPlatform(VkRect2D* bounds) const;
    void transformClientRectToPlatform(VkViewport* bounds) const;
    VkExtent2D getExtent() const;
    // We return references in the following methods to avoid a copy.
    VulkanAttachment& getColor(int target);
    VulkanAttachment& getMsaaColor(int target);
    VulkanAttachment& getDepth();
    VulkanAttachment& getMsaaDepth();
    uint8_t getColorTargetCount(const VulkanRenderPass& pass) const;
    uint8_t getSamples() const { return mSamples; }
    uint8_t getLayerCount() const { return mLayerCount; }
    bool hasDepth() const { return mDepth.texture; }
    bool isSwapChain() const { return !mOffscreen; }
    void bindToSwapChain(VulkanSwapChain& surf);

private:
    VulkanAttachment mColor[MRT::MAX_SUPPORTED_RENDER_TARGET_COUNT] = {};
    VulkanAttachment mDepth = {};
    VulkanAttachment mMsaaAttachments[MRT::MAX_SUPPORTED_RENDER_TARGET_COUNT] = {};
    VulkanAttachment mMsaaDepthAttachment = {};
    const bool mOffscreen : 1;
    uint8_t mSamples : 7;
    uint8_t mLayerCount = 1;
};

struct VulkanBufferObject;

struct VulkanVertexBufferInfo : public HwVertexBufferInfo, VulkanResource {
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

struct VulkanVertexBuffer : public HwVertexBuffer, VulkanResource {
    VulkanVertexBuffer(VulkanContext& context, VulkanStagePool& stagePool,
            VulkanResourceAllocator* allocator,
            uint32_t vertexCount, Handle<HwVertexBufferInfo> vbih);

    void setBuffer(VulkanResourceAllocator const& allocator,
            VulkanBufferObject* bufferObject, uint32_t index);

    inline VkBuffer const* getVkBuffers() const {
        return mBuffers.data();
    }

    inline VkBuffer* getVkBuffers() {
        return mBuffers.data();
    }

    Handle<HwVertexBufferInfo> vbih;
private:
    utils::FixedCapacityVector<VkBuffer> mBuffers;
    FixedSizeVulkanResourceManager<MAX_VERTEX_BUFFER_COUNT> mResources;
};

struct VulkanIndexBuffer : public HwIndexBuffer, VulkanResource {
    VulkanIndexBuffer(VmaAllocator allocator, VulkanStagePool& stagePool, uint8_t elementSize,
            uint32_t indexCount)
        : HwIndexBuffer(elementSize, indexCount),
          VulkanResource(VulkanResourceType::INDEX_BUFFER),
          buffer(allocator, stagePool, VK_BUFFER_USAGE_INDEX_BUFFER_BIT, elementSize * indexCount),
          indexType(elementSize == 2 ? VK_INDEX_TYPE_UINT16 : VK_INDEX_TYPE_UINT32) {}

    VulkanBuffer buffer;
    const VkIndexType indexType;
};

struct VulkanBufferObject : public HwBufferObject, VulkanResource {
    VulkanBufferObject(VmaAllocator allocator, VulkanStagePool& stagePool, uint32_t byteCount,
            BufferObjectBinding bindingType);

    VulkanBuffer buffer;
    const BufferObjectBinding bindingType;
};

struct VulkanSamplerGroup : public HwSamplerGroup, VulkanResource {
    // NOTE: we have to use out-of-line allocation here because the size of a Handle<> is limited
    std::unique_ptr<SamplerGroup> sb;// FIXME: this shouldn't depend on filament::SamplerGroup
    explicit VulkanSamplerGroup(size_t size) noexcept
        : VulkanResource(VulkanResourceType::SAMPLER_GROUP),
          sb(new SamplerGroup(size)) {}
};

struct VulkanRenderPrimitive : public HwRenderPrimitive, VulkanResource {
    VulkanRenderPrimitive(VulkanResourceAllocator* resourceAllocator,
            PrimitiveType pt, Handle<HwVertexBuffer> vbh, Handle<HwIndexBuffer> ibh);

    ~VulkanRenderPrimitive() {
        mResources.clear();
    }

    VulkanVertexBuffer* vertexBuffer = nullptr;
    VulkanIndexBuffer* indexBuffer = nullptr;

private:
    // Keep references to the vertex buffer and the index buffer.
    FixedSizeVulkanResourceManager<2> mResources;
};

struct VulkanFence : public HwFence, VulkanResource {
    VulkanFence()
        : VulkanResource(VulkanResourceType::FENCE) {}

    explicit VulkanFence(std::shared_ptr<VulkanCmdFence> fence)
        : VulkanResource(VulkanResourceType::FENCE),
          fence(fence) {}

    std::shared_ptr<VulkanCmdFence> fence;
};

struct VulkanTimerQuery : public HwTimerQuery, VulkanThreadSafeResource {
    explicit VulkanTimerQuery(std::tuple<uint32_t, uint32_t> indices);
    ~VulkanTimerQuery();

    void setFence(std::shared_ptr<VulkanCmdFence> fence) noexcept;

    bool isCompleted() noexcept;

    uint32_t getStartingQueryIndex() const {
        return mStartingQueryIndex;
    }

    uint32_t getStoppingQueryIndex() const {
        return mStoppingQueryIndex;
    }

private:
    uint32_t mStartingQueryIndex;
    uint32_t mStoppingQueryIndex;

    std::shared_ptr<VulkanCmdFence> mFence;
    utils::Mutex mFenceMutex;
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
