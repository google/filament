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

#include "VulkanHandles.h"

#include "VulkanConstants.h"

// TODO: remove this by moving DebugUtils out of VulkanDriver
#include "VulkanDriver.h"

#include "VulkanMemory.h"
#include "VulkanResourceAllocator.h"
#include "VulkanUtility.h"
#include "spirv/VulkanSpirvUtils.h"

#include <backend/platforms/VulkanPlatform.h>

#include <utils/Panic.h>    // ASSERT_POSTCONDITION

using namespace bluevk;

namespace filament::backend {

namespace {

void flipVertically(VkRect2D* rect, uint32_t framebufferHeight) {
    rect->offset.y = framebufferHeight - rect->offset.y - rect->extent.height;
}

void flipVertically(VkViewport* rect, uint32_t framebufferHeight) {
    rect->y = framebufferHeight - rect->y - rect->height;
}

void clampToFramebuffer(VkRect2D* rect, uint32_t fbWidth, uint32_t fbHeight) {
    int32_t x = std::max(rect->offset.x, 0);
    int32_t y = std::max(rect->offset.y, 0);
    int32_t right = std::min(rect->offset.x + (int32_t) rect->extent.width, (int32_t) fbWidth);
    int32_t top = std::min(rect->offset.y + (int32_t) rect->extent.height, (int32_t) fbHeight);
    rect->offset.x = std::min(x, (int32_t) fbWidth);
    rect->offset.y = std::min(y, (int32_t) fbHeight);
    rect->extent.width = std::max(right - x, 0);
    rect->extent.height = std::max(top - y, 0);
}

template<typename Bitmask>
inline void fromStageFlags(backend::ShaderStageFlags stage, descriptor_binding_t binding,
        Bitmask& mask) {
    if ((bool) (stage & ShaderStageFlags::VERTEX)) {
        mask.set(binding + getVertexStageShift<Bitmask>());
    }
    if ((bool) (stage & ShaderStageFlags::FRAGMENT)) {
        mask.set(binding + getFragmentStageShift<Bitmask>());
    }
}

inline VkShaderStageFlags getVkStage(backend::ShaderStage stage) {
    switch(stage) {
        case backend::ShaderStage::VERTEX:
            return VK_SHADER_STAGE_VERTEX_BIT;
        case backend::ShaderStage::FRAGMENT:
            return VK_SHADER_STAGE_FRAGMENT_BIT;
        case backend::ShaderStage::COMPUTE:
            PANIC_POSTCONDITION("Unsupported stage");
    }
}

using BitmaskGroup = VulkanDescriptorSetLayout::Bitmask;
BitmaskGroup fromBackendLayout(DescriptorSetLayout const& layout) {
    BitmaskGroup mask;
    for (auto const& binding: layout.bindings) {
        switch (binding.type) {
            case DescriptorType::UNIFORM_BUFFER: {
                if ((binding.flags & DescriptorFlags::DYNAMIC_OFFSET) != DescriptorFlags::NONE) {
                    fromStageFlags(binding.stageFlags, binding.binding, mask.dynamicUbo);
                } else {
                    fromStageFlags(binding.stageFlags, binding.binding, mask.ubo);
                }
                break;
            }
            case DescriptorType::SAMPLER: {
                fromStageFlags(binding.stageFlags, binding.binding, mask.sampler);
                break;
            }
            case DescriptorType::INPUT_ATTACHMENT: {
                fromStageFlags(binding.stageFlags, binding.binding, mask.inputAttachment);
                break;
            }
            case DescriptorType::SHADER_STORAGE_BUFFER:
                PANIC_POSTCONDITION("Shader storage is not supported");
                break;
        }
    }
    return mask;
}

} // anonymous namespace

VulkanDescriptorSetLayout::VulkanDescriptorSetLayout(DescriptorSetLayout const& layout)
    : VulkanResource(VulkanResourceType::DESCRIPTOR_SET_LAYOUT),
      bitmask(fromBackendLayout(layout)),
      count(Count::fromLayoutBitmask(bitmask)) {}

void VulkanDescriptorSet::acquire(VulkanTexture* texture) {
    mResources.acquire(texture);
}

void VulkanDescriptorSet::acquire(VulkanBufferObject* bufferObject) {
    mResources.acquire(bufferObject);
}

PushConstantDescription::PushConstantDescription(backend::Program const& program) noexcept {
    mRangeCount = 0;
    for (auto stage : { ShaderStage::VERTEX, ShaderStage::FRAGMENT, ShaderStage::COMPUTE }) {
        auto const& constants = program.getPushConstants(stage);
        if (constants.empty()) {
            continue;
        }

        // We store the type of the constant for type-checking when writing.
        auto& types = mTypes[(uint8_t) stage];
        types.reserve(constants.size());
        std::for_each(constants.cbegin(), constants.cend(), [&types] (Program::PushConstant t) {
            types.push_back(t.type);
        });

        mRanges[mRangeCount++] = {
            .stageFlags = getVkStage(stage),
            .offset = 0,
            .size = (uint32_t) constants.size() * ENTRY_SIZE,
        };
    }
}

void PushConstantDescription::write(VulkanCommands* commands, VkPipelineLayout layout,
        backend::ShaderStage stage, uint8_t index, backend::PushConstantVariant const& value) {
    VulkanCommandBuffer* cmdbuf = &(commands->get());
    uint32_t binaryValue = 0;
    UTILS_UNUSED_IN_RELEASE auto const& types = mTypes[(uint8_t) stage];
    if (std::holds_alternative<bool>(value)) {
        assert_invariant(types[index] == ConstantType::BOOL);
        bool const bval = std::get<bool>(value);
        binaryValue = static_cast<uint32_t const>(bval ? VK_TRUE : VK_FALSE);
    } else if (std::holds_alternative<float>(value)) {
        assert_invariant(types[index] == ConstantType::FLOAT);
        float const fval = std::get<float>(value);
        binaryValue = *reinterpret_cast<uint32_t const*>(&fval);
    } else {
        assert_invariant(types[index] == ConstantType::INT);
        int const ival = std::get<int>(value);
        binaryValue = *reinterpret_cast<uint32_t const*>(&ival);
    }
    vkCmdPushConstants(cmdbuf->buffer(), layout, getVkStage(stage), index * ENTRY_SIZE, ENTRY_SIZE,
            &binaryValue);
}

VulkanProgram::VulkanProgram(VkDevice device, Program const& builder) noexcept
    : HwProgram(builder.getName()),
      VulkanResource(VulkanResourceType::PROGRAM),
      mInfo(new(std::nothrow) PipelineInfo(builder)),
      mDevice(device) {

    Program::ShaderSource const& blobs = builder.getShadersSource();
    auto& modules = mInfo->shaders;
    auto const& specializationConstants = builder.getSpecializationConstants();
    std::vector<uint32_t> shader;

    static_assert(static_cast<ShaderStage>(0) == ShaderStage::VERTEX &&
            static_cast<ShaderStage>(1) == ShaderStage::FRAGMENT &&
            MAX_SHADER_MODULES == 2);

    for (size_t i = 0; i < MAX_SHADER_MODULES; i++) {
        Program::ShaderBlob const& blob = blobs[i];

        uint32_t* data = (uint32_t*) blob.data();
        size_t dataSize = blob.size();

        if (!specializationConstants.empty()) {
            workaroundSpecConstant(blob, specializationConstants, shader);
            data = (uint32_t*) shader.data();
            dataSize = shader.size() * 4;
        }

        VkShaderModule& module = modules[i];
        VkShaderModuleCreateInfo moduleInfo = {
            .sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO,
            .codeSize = dataSize,
            .pCode = data,
        };
        VkResult result = vkCreateShaderModule(mDevice, &moduleInfo, VKALLOC, &module);
        FILAMENT_CHECK_POSTCONDITION(result == VK_SUCCESS) << "Unable to create shader module.";

#if FVK_ENABLED(FVK_DEBUG_DEBUG_UTILS)
        std::string name{ builder.getName().c_str(), builder.getName().size() };
        switch (static_cast<ShaderStage>(i)) {
            case ShaderStage::VERTEX:
                name += "_vs";
                break;
            case ShaderStage::FRAGMENT:
                name += "_fs";
                break;
            default:
                PANIC_POSTCONDITION("Unexpected stage");
                break;
        }
        VulkanDriver::DebugUtils::setName(VK_OBJECT_TYPE_SHADER_MODULE,
                reinterpret_cast<uint64_t>(module), name.c_str());
#endif
    }

#if FVK_ENABLED(FVK_DEBUG_SHADER_MODULE)
    FVK_LOGD << "Created VulkanProgram " << builder << ", shaders = (" << modules[0]
             << ", " << modules[1] << ")" << utils::io::endl;
#endif
}

VulkanProgram::~VulkanProgram() {
    for (auto shader: mInfo->shaders) {
        vkDestroyShaderModule(mDevice, shader, VKALLOC);
    }
    delete mInfo;
}

// Creates a special "default" render target (i.e. associated with the swap chain)
VulkanRenderTarget::VulkanRenderTarget() :
    HwRenderTarget(0, 0),
    VulkanResource(VulkanResourceType::RENDER_TARGET),
    mOffscreen(false), mSamples(1) {}

void VulkanRenderTarget::bindToSwapChain(VulkanSwapChain& swapChain) {
    assert_invariant(!mOffscreen);
    VkExtent2D const extent = swapChain.getExtent();
    mColor[0] = { .texture = swapChain.getCurrentColor() };
    mDepth = { .texture = swapChain.getDepth() };
    width = extent.width;
    height = extent.height;
}

VulkanRenderTarget::VulkanRenderTarget(VkDevice device, VkPhysicalDevice physicalDevice,
        VulkanContext const& context, VmaAllocator allocator, VulkanCommands* commands,
        VulkanResourceAllocator* handleAllocator,
        uint32_t width, uint32_t height, uint8_t samples,
        VulkanAttachment color[MRT::MAX_SUPPORTED_RENDER_TARGET_COUNT],
        VulkanAttachment depthStencil[2], VulkanStagePool& stagePool, uint8_t layerCount)
    : HwRenderTarget(width, height),
      VulkanResource(VulkanResourceType::RENDER_TARGET),
      mOffscreen(true),
      mSamples(samples),
      mLayerCount(layerCount) {
    for (int index = 0; index < MRT::MAX_SUPPORTED_RENDER_TARGET_COUNT; index++) {
        mColor[index] = color[index];
    }
    mDepth = depthStencil[0];
    VulkanTexture* depthTexture = (VulkanTexture*) mDepth.texture;

    if (samples == 1) {
        return;
    }

    // Constrain the sample count according to both kinds of sample count masks obtained from
    // VkPhysicalDeviceProperties. This is consistent with the VulkanTexture constructor.
    auto const& limits = context.getPhysicalDeviceLimits();
    mSamples = samples = reduceSampleCount(samples, limits.framebufferDepthSampleCounts &
            limits.framebufferColorSampleCounts);

    // Create sidecar MSAA textures for color attachments if they don't already exist.
    for (int index = 0; index < MRT::MAX_SUPPORTED_RENDER_TARGET_COUNT; index++) {
        VulkanAttachment const& spec = color[index];
        VulkanTexture* texture = (VulkanTexture*) spec.texture;
        if (texture && texture->samples == 1) {
            auto msTexture = texture->getSidecar();
            if (UTILS_UNLIKELY(!msTexture)) {
                // Clear all usage flags that are not related to attachments, so that we can
                // use the transient usage flag.
                const TextureUsage usage = texture->usage & TextureUsage::ALL_ATTACHMENTS;
                assert_invariant(static_cast<uint16_t>(usage) != 0U);

                // TODO: This should be allocated with the ResourceAllocator.
                msTexture = new VulkanTexture(device, physicalDevice, context, allocator, commands,
                        handleAllocator,
                        texture->target, ((VulkanTexture const*) texture)->levels, texture->format,
                        samples, texture->width, texture->height, texture->depth, usage,
                        stagePool, true /* heap allocated */);
                texture->setSidecar(msTexture);
            }
            mMsaaAttachments[index] = {.texture = msTexture};
        }
        if (texture && texture->samples > 1) {
            mMsaaAttachments[index] = mColor[index];
        }
    }

    if (!depthTexture) {
        return;
    }

    // There is no need for sidecar depth if the depth texture is already MSAA.
    if (depthTexture->samples > 1) {
        mMsaaDepthAttachment = mDepth;
        return;
    }

    // MSAA depth texture must have the mipmap count of 1
    uint8_t const msLevel = 1;

    // Create sidecar MSAA texture for the depth attachment if it does not already exist.
    VulkanTexture* msTexture = depthTexture->getSidecar();
    if (UTILS_UNLIKELY(!msTexture)) {
        msTexture = new VulkanTexture(device, physicalDevice, context, allocator,
                commands, handleAllocator,
                depthTexture->target, msLevel, depthTexture->format, samples,
                depthTexture->width, depthTexture->height, depthTexture->depth, depthTexture->usage,
                stagePool, true /* heap allocated */);
        depthTexture->setSidecar(msTexture);
    }

    mMsaaDepthAttachment = {
        .texture = msTexture,
        .level = msLevel,
        .layer = mDepth.layer,
    };
}

void VulkanRenderTarget::transformClientRectToPlatform(VkRect2D* bounds) const {
    const auto& extent = getExtent();
    flipVertically(bounds, extent.height);
    clampToFramebuffer(bounds, extent.width, extent.height);
}

void VulkanRenderTarget::transformClientRectToPlatform(VkViewport* bounds) const {
    flipVertically(bounds, getExtent().height);
}

VkExtent2D VulkanRenderTarget::getExtent() const {
    return {width, height};
}

VulkanAttachment& VulkanRenderTarget::getColor(int target) {
    return mColor[target];
}

VulkanAttachment& VulkanRenderTarget::getMsaaColor(int target) {
    return mMsaaAttachments[target];
}

VulkanAttachment& VulkanRenderTarget::getDepth() {
    return mDepth;
}

VulkanAttachment& VulkanRenderTarget::getMsaaDepth() {
    return mMsaaDepthAttachment;
}

uint8_t VulkanRenderTarget::getColorTargetCount(const VulkanRenderPass& pass) const {
    if (!mOffscreen) {
        return 1;
    }
    uint8_t count = 0;
    for (uint8_t i = 0; i < MRT::MAX_SUPPORTED_RENDER_TARGET_COUNT; i++) {
        if (!mColor[i].texture) {
            continue;
        }
        // NOTE: This must be consistent with VkRenderPass construction (see VulkanFboCache).
        if (!(pass.params.subpassMask & (1 << i)) || pass.currentSubpass == 1) {
            count++;
        }
    }
    return count;
}

VulkanVertexBufferInfo::VulkanVertexBufferInfo(
        uint8_t bufferCount, uint8_t attributeCount, AttributeArray const& attributes)
    : HwVertexBufferInfo(bufferCount, attributeCount),
      VulkanResource(VulkanResourceType::VERTEX_BUFFER_INFO),
      mInfo(attributes.size()) {

    auto attribDesc = mInfo.mSoa.data<PipelineInfo::ATTRIBUTE_DESCRIPTION>();
    auto bufferDesc = mInfo.mSoa.data<PipelineInfo::BUFFER_DESCRIPTION>();
    auto offsets = mInfo.mSoa.data<PipelineInfo::OFFSETS>();
    auto attribToBufferIndex = mInfo.mSoa.data<PipelineInfo::ATTRIBUTE_TO_BUFFER_INDEX>();
    std::fill(mInfo.mSoa.begin<PipelineInfo::ATTRIBUTE_TO_BUFFER_INDEX>(),
            mInfo.mSoa.end<PipelineInfo::ATTRIBUTE_TO_BUFFER_INDEX>(), -1);

    for (uint32_t attribIndex = 0; attribIndex < attributes.size(); attribIndex++) {
        Attribute attrib = attributes[attribIndex];
        bool const isInteger = attrib.flags & Attribute::FLAG_INTEGER_TARGET;
        bool const isNormalized = attrib.flags & Attribute::FLAG_NORMALIZED;
        VkFormat vkformat = getVkFormat(attrib.type, isNormalized, isInteger);

        // HACK: Re-use the positions buffer as a dummy buffer for disabled attributes. Filament's
        // vertex shaders declare all attributes as either vec4 or uvec4 (the latter for bone
        // indices), and positions are always at least 32 bits per element. Therefore we can assign
        // a dummy type of either R8G8B8A8_UINT or R8G8B8A8_SNORM, depending on whether the shader
        // expects to receive floats or ints.
        if (attrib.buffer == Attribute::BUFFER_UNUSED) {
            vkformat = isInteger ? VK_FORMAT_R8G8B8A8_UINT : VK_FORMAT_R8G8B8A8_SNORM;
            attrib = attributes[0];
        }
        offsets[attribIndex] = attrib.offset;
        attribDesc[attribIndex] = {
            .location = attribIndex,// matches the GLSL layout specifier
            .binding = attribIndex, // matches the position within vkCmdBindVertexBuffers
            .format = vkformat,
        };
        bufferDesc[attribIndex] = {
            .binding = attribIndex,
            .stride = attrib.stride,
        };
        attribToBufferIndex[attribIndex] = attrib.buffer;
    }
}

VulkanVertexBuffer::VulkanVertexBuffer(VulkanContext& context, VulkanStagePool& stagePool,
        VulkanResourceAllocator* allocator,
        uint32_t vertexCount, Handle<HwVertexBufferInfo> vbih)
    : HwVertexBuffer(vertexCount),
      VulkanResource(VulkanResourceType::VERTEX_BUFFER),
      vbih(vbih),
      mBuffers(MAX_VERTEX_BUFFER_COUNT), // TODO: can we do better here?
      mResources(allocator) {
}

void VulkanVertexBuffer::setBuffer(VulkanResourceAllocator const& allocator,
        VulkanBufferObject* bufferObject, uint32_t index) {
    VulkanVertexBufferInfo const* const vbi =
            const_cast<VulkanResourceAllocator&>(allocator).handle_cast<VulkanVertexBufferInfo*>(vbih);
    size_t const count = vbi->getAttributeCount();
    VkBuffer* const vkbuffers = getVkBuffers();
    int8_t const* const attribToBuffer = vbi->getAttributeToBuffer();
    for (uint8_t attribIndex = 0; attribIndex < count; attribIndex++) {
        if (attribToBuffer[attribIndex] == static_cast<int8_t>(index)) {
            vkbuffers[attribIndex] = bufferObject->buffer.getGpuBuffer();
        }
    }
    mResources.acquire(bufferObject);
}

VulkanBufferObject::VulkanBufferObject(VmaAllocator allocator, VulkanStagePool& stagePool,
        uint32_t byteCount, BufferObjectBinding bindingType)
    : HwBufferObject(byteCount),
      VulkanResource(VulkanResourceType::BUFFER_OBJECT),
      buffer(allocator, stagePool, getBufferObjectUsage(bindingType), byteCount),
      bindingType(bindingType) {}

VulkanRenderPrimitive::VulkanRenderPrimitive(VulkanResourceAllocator* resourceAllocator,
        PrimitiveType pt, Handle<HwVertexBuffer> vbh, Handle<HwIndexBuffer> ibh)
        : VulkanResource(VulkanResourceType::RENDER_PRIMITIVE),
          mResources(resourceAllocator) {
    type = pt;
    vertexBuffer = resourceAllocator->handle_cast<VulkanVertexBuffer*>(vbh);
    indexBuffer = resourceAllocator->handle_cast<VulkanIndexBuffer*>(ibh);
    mResources.acquire(vertexBuffer);
    mResources.acquire(indexBuffer);
}

} // namespace filament::backend
