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
#include "VulkanMemory.h"

#include <backend/platforms/VulkanPlatform.h>

#include <utils/Panic.h>

using namespace bluevk;

namespace filament::backend {

static void flipVertically(VkRect2D* rect, uint32_t framebufferHeight) {
    rect->offset.y = framebufferHeight - rect->offset.y - rect->extent.height;
}

static void flipVertically(VkViewport* rect, uint32_t framebufferHeight) {
    rect->y = framebufferHeight - rect->y - rect->height;
}

static void clampToFramebuffer(VkRect2D* rect, uint32_t fbWidth, uint32_t fbHeight) {
    int32_t x = std::max(rect->offset.x, 0);
    int32_t y = std::max(rect->offset.y, 0);
    int32_t right = std::min(rect->offset.x + (int32_t) rect->extent.width, (int32_t) fbWidth);
    int32_t top = std::min(rect->offset.y + (int32_t) rect->extent.height, (int32_t) fbHeight);
    rect->offset.x = std::min(x, (int32_t) fbWidth);
    rect->offset.y = std::min(y, (int32_t) fbHeight);
    rect->extent.width = std::max(right - x, 0);
    rect->extent.height = std::max(top - y, 0);
}

VulkanProgram::VulkanProgram(VkDevice device, const Program& builder) noexcept
    : HwProgram(builder.getName()),
      VulkanResource(VulkanResourceType::PROGRAM),
      mDevice(device) {
    auto const& blobs = builder.getShadersSource();
    VkShaderModule* modules[2] = {&bundle.vertex, &bundle.fragment};
    // TODO: handle compute shaders.
    for (size_t i = 0; i < 2; i++) {
        const auto& blob = blobs[i];
        VkShaderModule* module = modules[i];
        VkShaderModuleCreateInfo moduleInfo = {};
        moduleInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
        moduleInfo.codeSize = blob.size();
        moduleInfo.pCode = (uint32_t*) blob.data();
        VkResult result = vkCreateShaderModule(mDevice, &moduleInfo, VKALLOC, module);
        ASSERT_POSTCONDITION(result == VK_SUCCESS, "Unable to create shader module.");
    }

    // populate the specialization constants requirements right now
    auto const& specializationConstants = builder.getSpecializationConstants();
    if (!specializationConstants.empty()) {
        // Allocate a single heap block to store all the specialization constants structures
        // our supported types are int32, float and bool, so we use 4 bytes per data. bool will
        // just use the first byte.
        char* pStorage = (char*)malloc(
                sizeof(VkSpecializationInfo) +
                specializationConstants.size() * sizeof(VkSpecializationMapEntry) +
                specializationConstants.size() * 4);

        VkSpecializationInfo* const pInfo = (VkSpecializationInfo*)pStorage;
        VkSpecializationMapEntry* const pEntries =
                (VkSpecializationMapEntry*)(pStorage + sizeof(VkSpecializationInfo));
        void* pData = pStorage + sizeof(VkSpecializationInfo) +
                      specializationConstants.size() * sizeof(VkSpecializationMapEntry);

        *pInfo = {
                .mapEntryCount = specializationConstants.size(),
                .pMapEntries = pEntries,
                .dataSize = specializationConstants.size() * 4,
                .pData = pData,
        };

        for (size_t i = 0; i < specializationConstants.size(); i++) {
            uint32_t const offset = uint32_t(i) * 4;
            pEntries[i] = {
                .constantID = specializationConstants[i].id,
                .offset = offset,
                // Note that bools are 4-bytes in Vulkan
                // https://registry.khronos.org/vulkan/specs/1.3-extensions/man/html/VkBool32.html
                .size = 4,
            };

            using SpecConstant = Program::SpecializationConstant::Type;
            char const* addr = (char*)pData + offset;
            SpecConstant const& arg = specializationConstants[i].value;
            if (std::holds_alternative<bool>(arg)) {
                *((VkBool32*)addr) = std::get<bool>(arg) ? VK_TRUE : VK_FALSE;
            } else if (std::holds_alternative<float>(arg)) {
                *((float*)addr) = std::get<float>(arg);
            } else {
                *((int32_t*)addr) = std::get<int32_t>(arg);
            }
        }
        bundle.specializationInfos = pInfo;
    }

    // Make a copy of the binding map
    samplerGroupInfo = builder.getSamplerGroupInfo();
    #if FVK_ENABLED(FVK_DEBUG_SHADER_MODULE)
        utils::slog.d << "Created VulkanProgram " << builder << ", shaders = (" << bundle.vertex
                      << ", " << bundle.fragment << ")" << utils::io::endl;
    #endif
}

VulkanProgram::VulkanProgram(VkDevice device, VkShaderModule vs, VkShaderModule fs) noexcept
    : VulkanResource(VulkanResourceType::PROGRAM),
      mDevice(device) {
    bundle.vertex = vs;
    bundle.fragment = fs;
}

VulkanProgram::~VulkanProgram() {
    vkDestroyShaderModule(mDevice, bundle.vertex, VKALLOC);
    vkDestroyShaderModule(mDevice, bundle.fragment, VKALLOC);
    free(bundle.specializationInfos);
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
        uint32_t width, uint32_t height, uint8_t samples,
        VulkanAttachment color[MRT::MAX_SUPPORTED_RENDER_TARGET_COUNT],
        VulkanAttachment depthStencil[2], VulkanStagePool& stagePool)
    : HwRenderTarget(width, height),
      VulkanResource(VulkanResourceType::RENDER_TARGET),
      mOffscreen(true),
      mSamples(samples) {
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
    const auto& limits = context.getPhysicalDeviceLimits();
    mSamples = samples = reduceSampleCount(samples, limits.framebufferDepthSampleCounts &
            limits.framebufferColorSampleCounts);

    // Create sidecar MSAA textures for color attachments if they don't already exist.
    for (int index = 0; index < MRT::MAX_SUPPORTED_RENDER_TARGET_COUNT; index++) {
        const VulkanAttachment& spec = color[index];
        VulkanTexture* texture = (VulkanTexture*) spec.texture;
        if (texture && texture->samples == 1) {
            auto msTexture = texture->getSidecar();
            if (UTILS_UNLIKELY(!msTexture)) {
                // TODO: This should be allocated with the ResourceAllocator.
                msTexture = new VulkanTexture(device, physicalDevice, context, allocator, commands,
                        texture->target, ((VulkanTexture const*) texture)->levels, texture->format,
                        samples, texture->width, texture->height, texture->depth, texture->usage,
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
                commands, depthTexture->target, msLevel, depthTexture->format, samples,
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

VulkanAttachment VulkanRenderTarget::getColor(int target) const {
    return mColor[target];
}

VulkanAttachment VulkanRenderTarget::getMsaaColor(int target) const {
    return mMsaaAttachments[target];
}

VulkanAttachment VulkanRenderTarget::getDepth() const {
    return mDepth;
}

VulkanAttachment VulkanRenderTarget::getMsaaDepth() const {
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

VulkanVertexBuffer::VulkanVertexBuffer(VulkanContext& context, VulkanStagePool& stagePool,
        VulkanResourceAllocator* allocator, uint8_t bufferCount, uint8_t attributeCount,
        uint32_t elementCount, AttributeArray const& attribs)
    : HwVertexBuffer(bufferCount, attributeCount, elementCount, attribs),
      VulkanResource(VulkanResourceType::VERTEX_BUFFER),
      buffers(bufferCount, nullptr),
      mResources(allocator) {}

void VulkanVertexBuffer::setBuffer(VulkanBufferObject* bufferObject, uint32_t index) {
    buffers[index] = &bufferObject->buffer;
    mResources.acquire(bufferObject);
}

VulkanBufferObject::VulkanBufferObject(VmaAllocator allocator, VulkanStagePool& stagePool,
        uint32_t byteCount, BufferObjectBinding bindingType, BufferUsage usage)
    : HwBufferObject(byteCount),
      VulkanResource(VulkanResourceType::BUFFER_OBJECT),
      buffer(allocator, stagePool, getBufferObjectUsage(bindingType), byteCount),
      bindingType(bindingType) {}

void VulkanRenderPrimitive::setPrimitiveType(PrimitiveType pt) {
    this->type = pt;
    switch (pt) {
        case PrimitiveType::POINTS:
            primitiveTopology = VK_PRIMITIVE_TOPOLOGY_POINT_LIST;
            break;
        case PrimitiveType::LINES:
            primitiveTopology = VK_PRIMITIVE_TOPOLOGY_LINE_LIST;
            break;
        case PrimitiveType::LINE_STRIP:
            primitiveTopology = VK_PRIMITIVE_TOPOLOGY_LINE_STRIP;
            break;
        case PrimitiveType::TRIANGLES:
            primitiveTopology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
            break;
        case PrimitiveType::TRIANGLE_STRIP:
            primitiveTopology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_STRIP;
            break;
    }
}

void VulkanRenderPrimitive::setBuffers(VulkanVertexBuffer* vertexBuffer,
        VulkanIndexBuffer* indexBuffer) {
    this->vertexBuffer = vertexBuffer;
    this->indexBuffer = indexBuffer;
    mResources.acquire(vertexBuffer);
    mResources.acquire(indexBuffer);
}

VulkanTimerQuery::VulkanTimerQuery(std::tuple<uint32_t, uint32_t> indices)
    : VulkanThreadSafeResource(VulkanResourceType::TIMER_QUERY),
      mStartingQueryIndex(std::get<0>(indices)),
      mStoppingQueryIndex(std::get<1>(indices)) {}

void VulkanTimerQuery::setFence(std::shared_ptr<VulkanCmdFence> fence) noexcept {
    std::unique_lock<utils::Mutex> lock(mFenceMutex);
    mFence = fence;
}

bool VulkanTimerQuery::isCompleted() noexcept {
    std::unique_lock<utils::Mutex> lock(mFenceMutex);
    // QueryValue is a synchronous call and might occur before beginTimerQuery has written anything
    // into the command buffer, which is an error according to the validation layer that ships in
    // the Android NDK.  Even when AVAILABILITY_BIT is set, validation seems to require that the
    // timestamp has at least been written into a processed command buffer.

    // This fence indicates that the corresponding buffer has been completed.
    if (!mFence) {
        return false;
    }
    VkResult status = mFence->status.load(std::memory_order_relaxed);
    if (status != VK_SUCCESS) {
        return false;
    }

    return true;
}

VulkanTimerQuery::~VulkanTimerQuery() = default;

} // namespace filament::backend
