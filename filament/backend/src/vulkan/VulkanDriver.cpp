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

#include "VulkanDriver.h"

#include "CommandStreamDispatcher.h"
#include "DataReshaper.h"
#include "SystraceProfile.h"
#include "VulkanAsyncHandles.h"
#include "VulkanBuffer.h"
#include "VulkanCommands.h"
#include "VulkanDriverFactory.h"
#include "VulkanHandles.h"
#include "VulkanMemory.h"
#include "VulkanTexture.h"
#include "memory/ResourceManager.h"
#include "memory/ResourcePointer.h"

#include <backend/platforms/VulkanPlatform.h>

#include <utils/CString.h>
#include <utils/FixedCapacityVector.h>
#include <utils/Panic.h>

#ifndef NDEBUG
#include <set>  // For VulkanDriver::debugCommandBegin
#endif

using namespace bluevk;

using utils::FixedCapacityVector;

// Vulkan functions often immediately dereference pointers, so it's fine to pass in a pointer
// to a stack-allocated variable.
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wreturn-stack-address"
#pragma clang diagnostic ignored "-Wunused-parameter"

namespace filament::backend {

namespace {

VmaAllocator createAllocator(VkInstance instance, VkPhysicalDevice physicalDevice,
        VkDevice device) {
    VmaAllocator allocator;
    VmaVulkanFunctions const funcs {
#if VMA_DYNAMIC_VULKAN_FUNCTIONS
        .vkGetInstanceProcAddr = vkGetInstanceProcAddr,
        .vkGetDeviceProcAddr = vkGetDeviceProcAddr,
#else
        .vkGetPhysicalDeviceProperties = vkGetPhysicalDeviceProperties,
        .vkGetPhysicalDeviceMemoryProperties = vkGetPhysicalDeviceMemoryProperties,
        .vkAllocateMemory = vkAllocateMemory,
        .vkFreeMemory = vkFreeMemory,
        .vkMapMemory = vkMapMemory,
        .vkUnmapMemory = vkUnmapMemory,
        .vkFlushMappedMemoryRanges = vkFlushMappedMemoryRanges,
        .vkInvalidateMappedMemoryRanges = vkInvalidateMappedMemoryRanges,
        .vkBindBufferMemory = vkBindBufferMemory,
        .vkBindImageMemory = vkBindImageMemory,
        .vkGetBufferMemoryRequirements = vkGetBufferMemoryRequirements,
        .vkGetImageMemoryRequirements = vkGetImageMemoryRequirements,
        .vkCreateBuffer = vkCreateBuffer,
        .vkDestroyBuffer = vkDestroyBuffer,
        .vkCreateImage = vkCreateImage,
        .vkDestroyImage = vkDestroyImage,
        .vkCmdCopyBuffer = vkCmdCopyBuffer,
        .vkGetBufferMemoryRequirements2KHR = vkGetBufferMemoryRequirements2KHR,
        .vkGetImageMemoryRequirements2KHR = vkGetImageMemoryRequirements2KHR
#endif
    };
    VmaAllocatorCreateInfo const allocatorInfo {
        .physicalDevice = physicalDevice,
        .device = device,
        .pVulkanFunctions = &funcs,
        .instance = instance,
    };
    vmaCreateAllocator(&allocatorInfo, &allocator);
    return allocator;
}

#if FVK_ENABLED(FVK_DEBUG_VALIDATION)
VKAPI_ATTR VkBool32 VKAPI_CALL debugReportCallback(VkDebugReportFlagsEXT flags,
        VkDebugReportObjectTypeEXT objectType, uint64_t object, size_t location,
        int32_t messageCode, const char* pLayerPrefix, const char* pMessage, void* pUserData) {
    if (flags & VK_DEBUG_REPORT_ERROR_BIT_EXT) {
        FVK_LOGE << "VULKAN ERROR: (" << pLayerPrefix << ") " << pMessage << utils::io::endl;
    } else {
        // TODO: emit best practices warnings about aggressive pipeline barriers.
        if (strstr(pMessage, "ALL_GRAPHICS_BIT") || strstr(pMessage, "ALL_COMMANDS_BIT")) {
            return VK_FALSE;
        }
        FVK_LOGW << "VULKAN WARNING: (" << pLayerPrefix << ") " << pMessage << utils::io::endl;
    }
    FVK_LOGE << utils::io::endl;
    return VK_FALSE;
}
#endif // FVK_ENABLED(FVK_DEBUG_VALIDATION)

#if FVK_ENABLED(FVK_DEBUG_DEBUG_UTILS)
VKAPI_ATTR VkBool32 VKAPI_CALL debugUtilsCallback(VkDebugUtilsMessageSeverityFlagBitsEXT severity,
        VkDebugUtilsMessageTypeFlagsEXT types, const VkDebugUtilsMessengerCallbackDataEXT* cbdata,
        void* pUserData) {
    if (severity & VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT) {
        FVK_LOGE << "VULKAN ERROR: (" << cbdata->pMessageIdName << ") " << cbdata->pMessage
                 << utils::io::endl;
    } else {
        // TODO: emit best practices warnings about aggressive pipeline barriers.
        if (strstr(cbdata->pMessage, "ALL_GRAPHICS_BIT")
                || strstr(cbdata->pMessage, "ALL_COMMANDS_BIT")) {
            return VK_FALSE;
        }
        FVK_LOGW << "VULKAN WARNING: (" << cbdata->pMessageIdName << ") " << cbdata->pMessage
                 << utils::io::endl;
    }
    FVK_LOGE << utils::io::endl;
    return VK_FALSE;
}
#endif // FVK_ENABLED(FVK_DEBUG_DEBUG_UTILS)


}// anonymous namespace

#if FVK_ENABLED(FVK_DEBUG_DEBUG_UTILS)
using DebugUtils = VulkanDriver::DebugUtils;
DebugUtils* DebugUtils::mSingleton = nullptr;

DebugUtils::DebugUtils(VkInstance instance, VkDevice device, VulkanContext const* context)
    : mInstance(instance), mDevice(device), mEnabled(context->isDebugUtilsSupported()) {

#if FVK_ENABLED(FVK_DEBUG_VALIDATION)
    // Also initialize debug utils messenger here
    if (mEnabled) {
        VkDebugUtilsMessengerCreateInfoEXT const createInfo = {
                .sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT,
                .pNext = nullptr,
                .flags = 0,
                .messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT
                                   | VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT,
                .messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT
                               | VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT,
                .pfnUserCallback = debugUtilsCallback,
        };
        VkResult result = vkCreateDebugUtilsMessengerEXT(instance, &createInfo,
                VKALLOC, &mDebugMessenger);
        FILAMENT_CHECK_POSTCONDITION(result == VK_SUCCESS)
                << "Unable to create Vulkan debug messenger.";
    }
#endif // FVK_ENABLED(FVK_DEBUG_VALIDATION)
}

DebugUtils* DebugUtils::get() {
    assert_invariant(DebugUtils::mSingleton);
    return DebugUtils::mSingleton;
}

DebugUtils::~DebugUtils() {
    if (mDebugMessenger) {
        vkDestroyDebugUtilsMessengerEXT(mInstance, mDebugMessenger, VKALLOC);
    }
}

void DebugUtils::setName(VkObjectType type, uint64_t handle, char const* name) {
    auto impl = DebugUtils::get();
    if (!impl->mEnabled) {
        return;
    }
    VkDebugUtilsObjectNameInfoEXT const info = {
            .sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_OBJECT_NAME_INFO_EXT,
            .pNext = nullptr,
            .objectType = type,
            .objectHandle = handle,
            .pObjectName = name,
    };
    vkSetDebugUtilsObjectNameEXT(impl->mDevice, &info);
}
#endif // FVK_ENABLED(FVK_DEBUG_DEBUG_UTILS)

Dispatcher VulkanDriver::getDispatcher() const noexcept {
    return ConcreteDispatcher<VulkanDriver>::make();
}

VulkanDriver::VulkanDriver(VulkanPlatform* platform, VulkanContext const& context,
        Platform::DriverConfig const& driverConfig) noexcept
    : mPlatform(platform),
      mResourceManager(driverConfig.handleArenaSize, driverConfig.disableHandleUseAfterFreeCheck),
      mAllocator(createAllocator(mPlatform->getInstance(), mPlatform->getPhysicalDevice(),
              mPlatform->getDevice())),
      mContext(context),
      mCommands(mPlatform->getDevice(), mPlatform->getGraphicsQueue(),
              mPlatform->getGraphicsQueueFamilyIndex(), mPlatform->getProtectedGraphicsQueue(),
              mPlatform->getProtectedGraphicsQueueFamilyIndex(), &mContext),
      mPipelineLayoutCache(mPlatform->getDevice()),
      mPipelineCache(mPlatform->getDevice(), mAllocator),
      mStagePool(mAllocator, &mCommands),
      mFramebufferCache(mPlatform->getDevice()),
      mSamplerCache(mPlatform->getDevice()),
      mBlitter(mPlatform->getPhysicalDevice(), &mCommands),
      mReadPixels(mPlatform->getDevice()),
      mDescriptorSetManager(mPlatform->getDevice(), &mResourceManager),
      mIsSRGBSwapChainSupported(mPlatform->getCustomization().isSRGBSwapChainSupported),
      mStereoscopicType(driverConfig.stereoscopicType) {

#if FVK_ENABLED(FVK_DEBUG_DEBUG_UTILS)
    DebugUtils::mSingleton =
            new DebugUtils(mPlatform->getInstance(), mPlatform->getDevice(), &context);
#endif

#if FVK_ENABLED(FVK_DEBUG_VALIDATION)
    UTILS_UNUSED const PFN_vkCreateDebugReportCallbackEXT createDebugReportCallback
            = vkCreateDebugReportCallbackEXT;
    if (!context.isDebugUtilsSupported() && createDebugReportCallback) {
        VkDebugReportCallbackCreateInfoEXT const cbinfo = {
                .sType = VK_STRUCTURE_TYPE_DEBUG_REPORT_CALLBACK_CREATE_INFO_EXT,
                .flags = VK_DEBUG_REPORT_WARNING_BIT_EXT | VK_DEBUG_REPORT_ERROR_BIT_EXT,
                .pfnCallback = debugReportCallback,
        };
        VkResult result = createDebugReportCallback(mPlatform->getInstance(), &cbinfo, VKALLOC,
                &mDebugCallback);
        FILAMENT_CHECK_POSTCONDITION(result == VK_SUCCESS)
                << "Unable to create Vulkan debug callback.";
    }
#endif

    mTimestamps = std::make_unique<VulkanTimestamps>(mPlatform->getDevice());
}

VulkanDriver::~VulkanDriver() noexcept = default;

UTILS_NOINLINE
Driver* VulkanDriver::create(VulkanPlatform* platform, VulkanContext const& context,
         Platform::DriverConfig const& driverConfig) noexcept {
#if 0
    // this is useful for development, but too verbose even for debug builds
    // For reference on a 64-bits machine in Release mode:
    //    VulkanSamplerGroup            :  16       few
    //    HwStream                      :  24       few
    //    VulkanFence                   :  32       few
    //    VulkanProgram                 :  32       moderate
    //    VulkanIndexBuffer             :  64       moderate
    //    VulkanBufferObject            :  64       many
    // -- less than or equal 64 bytes
    //    VulkanRenderPrimitive         :  72       many
    //    VulkanVertexBufferInfo        :  96       moderate
    //    VulkanSwapChain               : 104       few
    //    VulkanTimerQuery              : 160       few
    // -- less than or equal 160 bytes
    //    VulkanVertexBuffer            : 192       moderate
    //    VulkanTexture                 : 224       moderate
    //    VulkanRenderTarget            : 312       few
    // -- less than or equal to 312 bytes

    FVK_LOGD
           << "\nVulkanSwapChain: " << sizeof(VulkanSwapChain)
           << "\nVulkanBufferObject: " << sizeof(VulkanBufferObject)
           << "\nVulkanVertexBuffer: " << sizeof(VulkanVertexBuffer)
           << "\nVulkanVertexBufferInfo: " << sizeof(VulkanVertexBufferInfo)
           << "\nVulkanIndexBuffer: " << sizeof(VulkanIndexBuffer)
           << "\nVulkanSamplerGroup: " << sizeof(VulkanSamplerGroup)
           << "\nVulkanRenderPrimitive: " << sizeof(VulkanRenderPrimitive)
           << "\nVulkanTexture: " << sizeof(VulkanTexture)
           << "\nVulkanTimerQuery: " << sizeof(VulkanTimerQuery)
           << "\nHwStream: " << sizeof(HwStream)
           << "\nVulkanRenderTarget: " << sizeof(VulkanRenderTarget)
           << "\nVulkanFence: " << sizeof(VulkanFence)
           << "\nVulkanProgram: " << sizeof(VulkanProgram)
           << utils::io::endl;
#endif

    assert_invariant(platform);
    size_t defaultSize = FVK_HANDLE_ARENA_SIZE_IN_MB * 1024U * 1024U;
    Platform::DriverConfig validConfig {driverConfig};
    validConfig.handleArenaSize = std::max(driverConfig.handleArenaSize, defaultSize);
    return new VulkanDriver(platform, context, validConfig);
}

ShaderModel VulkanDriver::getShaderModel() const noexcept {
#if defined(__ANDROID__) || defined(IOS)
    return ShaderModel::MOBILE;
#else
    return ShaderModel::DESKTOP;
#endif
}

void VulkanDriver::terminate() {
    // Flush and wait here to make sure all queued commands are executed and resources that are tied
    // to those commands are no longer referenced.
    finish(0);

    mCurrentSwapChain = {};
    mDefaultRenderTarget = {};
    mBoundPipeline = {};

    mTimestamps.reset();

    mBlitter.terminate();
    mReadPixels.terminate();

    // Allow the stage pool to clean up.
    mStagePool.gc();

    mCommands.terminate();

    mStagePool.terminate();
    mPipelineCache.terminate();
    mFramebufferCache.reset();
    mSamplerCache.terminate();
    mDescriptorSetManager.terminate();
    mPipelineLayoutCache.terminate();

    // Before terminating ResourceManager, we must make sure all of the resource_ptrs have been unset.
    mResourceManager.terminate();

#if FVK_ENABLED(FVK_DEBUG_RESOURCE_LEAK)
    mResourceManager.print();
#endif

    vmaDestroyAllocator(mAllocator);

    if (mDebugCallback) {
        vkDestroyDebugReportCallbackEXT(mPlatform->getInstance(), mDebugCallback, VKALLOC);
    }

#if FVK_ENABLED(FVK_DEBUG_DEBUG_UTILS)
    assert_invariant(DebugUtils::mSingleton);
    delete DebugUtils::mSingleton;
#endif

    mPlatform->terminate();
}

void VulkanDriver::tick(int) {
    mCommands.updateFences();
}

// Garbage collection should not occur too frequently, only about once per frame. Internally, the
// eviction time of various resources is often measured in terms of an approximate frame number
// rather than the wall clock, because we must wait 3 frames after a DriverAPI-level resource has
// been destroyed for safe destruction, due to outstanding command buffers and triple buffering.
void VulkanDriver::collectGarbage() {
    FVK_SYSTRACE_SCOPE();
    // Command buffers need to be submitted and completed before other resources can be gc'd.
    mCommands.gc();
    mDescriptorSetManager.clearHistory();
    mStagePool.gc();
    mFramebufferCache.gc();
    mPipelineCache.gc();

    mResourceManager.gc();

#if FVK_ENABLED(FVK_DEBUG_RESOURCE_LEAK)
    mResourceManager.print();
#endif
}
void VulkanDriver::beginFrame(int64_t monotonic_clock_ns,
        int64_t refreshIntervalNs, uint32_t frameId) {
    FVK_PROFILE_MARKER(PROFILE_NAME_BEGINFRAME);
    // Do nothing.
}

void VulkanDriver::setFrameScheduledCallback(Handle<HwSwapChain> sch, CallbackHandler* handler,
        FrameScheduledCallback&& callback, uint64_t flags) {
}

void VulkanDriver::setFrameCompletedCallback(Handle<HwSwapChain> sch,
        CallbackHandler* handler, utils::Invocable<void(void)>&& callback) {
}

void VulkanDriver::setPresentationTime(int64_t monotonic_clock_ns) {
}

void VulkanDriver::endFrame(uint32_t frameId) {
    FVK_PROFILE_MARKER(PROFILE_NAME_ENDFRAME);
    mCommands.flush();
    collectGarbage();
}

void VulkanDriver::updateDescriptorSetBuffer(
        backend::DescriptorSetHandle dsh,
        backend::descriptor_binding_t binding,
        backend::BufferObjectHandle boh,
        uint32_t offset,
        uint32_t size) {
    FVK_SYSTRACE_SCOPE();
    auto set = resource_ptr<VulkanDescriptorSet>::cast(&mResourceManager, dsh);
    auto buffer = resource_ptr<VulkanBufferObject>::cast(&mResourceManager, boh);
    mDescriptorSetManager.updateBuffer(set, binding, buffer, offset, size);
}

void VulkanDriver::updateDescriptorSetTexture(
        backend::DescriptorSetHandle dsh,
        backend::descriptor_binding_t binding,
        backend::TextureHandle th,
        SamplerParams params) {
    FVK_SYSTRACE_SCOPE();
    auto set = resource_ptr<VulkanDescriptorSet>::cast(&mResourceManager, dsh);
    auto texture = resource_ptr<VulkanTexture>::cast(&mResourceManager, th);

    VkSampler const vksampler = mSamplerCache.getSampler(params);
    mDescriptorSetManager.updateSampler(set, binding, texture, vksampler);
}

void VulkanDriver::flush(int) {
    FVK_SYSTRACE_SCOPE();
    mCommands.flush();
}

void VulkanDriver::finish(int dummy) {
    FVK_SYSTRACE_SCOPE();

    mCommands.flush();
    mCommands.wait();

    mReadPixels.runUntilComplete();
}

void VulkanDriver::createRenderPrimitiveR(Handle<HwRenderPrimitive> rph,
        Handle<HwVertexBuffer> vbh, Handle<HwIndexBuffer> ibh,
        PrimitiveType pt) {
    auto vb = resource_ptr<VulkanVertexBuffer>::cast(&mResourceManager, vbh);
    auto ib = resource_ptr<VulkanIndexBuffer>::cast(&mResourceManager, ibh);
    auto ptr = resource_ptr<VulkanRenderPrimitive>::make(&mResourceManager, rph, pt, vb, ib);
    ptr.inc();
}

void VulkanDriver::destroyRenderPrimitive(Handle<HwRenderPrimitive> rph) {
    if (!rph) {
        return;
    }
    auto ptr = resource_ptr<VulkanRenderPrimitive>::cast(&mResourceManager, rph);
    ptr.dec();
}

void VulkanDriver::createVertexBufferInfoR(Handle<HwVertexBufferInfo> vbih, uint8_t bufferCount,
        uint8_t attributeCount, AttributeArray attributes) {
    auto vbi = resource_ptr<VulkanVertexBufferInfo>::make(&mResourceManager, vbih, bufferCount,
            attributeCount, attributes);
    vbi.inc();
}

void VulkanDriver::destroyVertexBufferInfo(Handle<HwVertexBufferInfo> vbih) {
    if (!vbih) {
        return;
    }
    auto vbi = resource_ptr<VulkanVertexBufferInfo>::cast(&mResourceManager, vbih);
    vbi.dec();
}

void VulkanDriver::createVertexBufferR(Handle<HwVertexBuffer> vbh, uint32_t vertexCount,
        Handle<HwVertexBufferInfo> vbih) {
    auto vbi = resource_ptr<VulkanVertexBufferInfo>::cast(&mResourceManager, vbih);
    auto vb = resource_ptr<VulkanVertexBuffer>::make(&mResourceManager, vbh, mContext, mStagePool,
            vertexCount, vbi);
    vb.inc();
}

void VulkanDriver::destroyVertexBuffer(Handle<HwVertexBuffer> vbh) {
    if (!vbh) {
        return;
    }
    auto vb = resource_ptr<VulkanVertexBuffer>::cast(&mResourceManager, vbh);
    vb.dec();
}

void VulkanDriver::createIndexBufferR(Handle<HwIndexBuffer> ibh, ElementType elementType,
        uint32_t indexCount, BufferUsage usage) {
    auto elementSize = (uint8_t) getElementTypeSize(elementType);
    auto ib = resource_ptr<VulkanIndexBuffer>::make(&mResourceManager, ibh, mAllocator, mStagePool,
            elementSize, indexCount);
    ib.inc();
}

void VulkanDriver::destroyIndexBuffer(Handle<HwIndexBuffer> ibh) {
    if (!ibh) {
        return;
    }
    auto ib = resource_ptr<VulkanIndexBuffer>::cast(&mResourceManager, ibh);
    ib.dec();
}

void VulkanDriver::createBufferObjectR(Handle<HwBufferObject> boh, uint32_t byteCount,
        BufferObjectBinding bindingType, BufferUsage usage) {
    auto bo = resource_ptr<VulkanBufferObject>::make(&mResourceManager, boh, mAllocator, mStagePool,
            byteCount, bindingType);
    bo.inc();
}

void VulkanDriver::destroyBufferObject(Handle<HwBufferObject> boh) {
    if (!boh) {
        return;
    }
    auto bo = resource_ptr<VulkanBufferObject>::cast(&mResourceManager, boh);
    bo.dec();
}

void VulkanDriver::createTextureR(Handle<HwTexture> th, SamplerType target, uint8_t levels,
        TextureFormat format, uint8_t samples, uint32_t w, uint32_t h, uint32_t depth,
        TextureUsage usage) {
    FVK_SYSTRACE_SCOPE();
    auto texture = resource_ptr<VulkanTexture>::make(&mResourceManager, th, mPlatform->getDevice(),
            mPlatform->getPhysicalDevice(), mContext, mAllocator, &mResourceManager, &mCommands,
            target, levels, format, samples, w, h, depth, usage, mStagePool);

    // Do transition to default layout.
    VulkanCommandBuffer& commandsBuf = mCommands.get();
    auto const& primaryViewRange = texture->getPrimaryViewRange();
    auto const defaultLayout = texture->getDefaultLayout();
    texture->transitionLayout(&commandsBuf, primaryViewRange, defaultLayout);

    texture.inc();
}

void VulkanDriver::createTextureViewR(Handle<HwTexture> th, Handle<HwTexture> srch,
        uint8_t baseLevel, uint8_t levelCount) {
    auto src = resource_ptr<VulkanTexture>::cast(&mResourceManager, srch);
    auto texture = resource_ptr<VulkanTexture>::make(&mResourceManager, th, mPlatform->getDevice(),
            mPlatform->getPhysicalDevice(), mContext, mAllocator, &mCommands, src, baseLevel,
            levelCount);
    texture.inc();
}

void VulkanDriver::createTextureViewSwizzleR(Handle<HwTexture> th, Handle<HwTexture> srch,
        backend::TextureSwizzle r, backend::TextureSwizzle g, backend::TextureSwizzle b,
        backend::TextureSwizzle a) {
   TextureSwizzle const swizzleArray[] = {r, g, b, a};
   VkComponentMapping const swizzle = getSwizzleMap(swizzleArray);
   auto src = resource_ptr<VulkanTexture>::cast(&mResourceManager, srch);
   auto texture = resource_ptr<VulkanTexture>::make(&mResourceManager, th, mPlatform->getDevice(),
           mPlatform->getPhysicalDevice(), mContext, mAllocator, &mCommands, src, swizzle);
   texture.inc();
}

void VulkanDriver::createTextureExternalImageR(Handle<HwTexture> th, backend::TextureFormat format,
        uint32_t width, uint32_t height, backend::TextureUsage usage, void* image) {
}

void VulkanDriver::createTextureExternalImagePlaneR(Handle<HwTexture> th,
        backend::TextureFormat format, uint32_t width, uint32_t height, backend::TextureUsage usage,
        void* image, uint32_t plane) {
}

void VulkanDriver::importTextureR(Handle<HwTexture> th, intptr_t id,
        SamplerType target, uint8_t levels,
        TextureFormat format, uint8_t samples, uint32_t w, uint32_t h, uint32_t depth,
        TextureUsage usage) {
    // not supported in this backend
}

void VulkanDriver::destroyTexture(Handle<HwTexture> th) {
    if (!th) {
        return;
    }
    auto texture = resource_ptr<VulkanTexture>::cast(&mResourceManager, th);
    texture.dec();
}

void VulkanDriver::createProgramR(Handle<HwProgram> ph, Program&& program) {
    FVK_SYSTRACE_SCOPE();
    auto vprogram = resource_ptr<VulkanProgram>::make(&mResourceManager, ph, mPlatform->getDevice(),
            program);
    vprogram.inc();
}

void VulkanDriver::destroyProgram(Handle<HwProgram> ph) {
    if (!ph) {
        return;
    }
    auto vprogram = resource_ptr<VulkanProgram>::cast(&mResourceManager, ph);
    vprogram.dec();
}

void VulkanDriver::createDefaultRenderTargetR(Handle<HwRenderTarget> rth, int) {
    assert_invariant(!mDefaultRenderTarget);
    auto renderTarget = resource_ptr<VulkanRenderTarget>::make(&mResourceManager, rth);
    mDefaultRenderTarget = renderTarget;
}

void VulkanDriver::createRenderTargetR(Handle<HwRenderTarget> rth,
        TargetBufferFlags targets, uint32_t width, uint32_t height, uint8_t samples,
        uint8_t layerCount, MRT color, TargetBufferInfo depth, TargetBufferInfo stencil) {

    FVK_SYSTRACE_SCOPE();

    UTILS_UNUSED_IN_RELEASE math::vec2<uint32_t> tmin = {std::numeric_limits<uint32_t>::max()};
    UTILS_UNUSED_IN_RELEASE math::vec2<uint32_t> tmax = {0};
    UTILS_UNUSED_IN_RELEASE size_t attachmentCount = 0;

    VulkanAttachment colorTargets[MRT::MAX_SUPPORTED_RENDER_TARGET_COUNT] = {};
    for (int i = 0; i < MRT::MAX_SUPPORTED_RENDER_TARGET_COUNT; i++) {
        if (color[i].handle) {
            colorTargets[i] = {
                .texture = resource_ptr<VulkanTexture>::cast(&mResourceManager, color[i].handle),
                .level = color[i].level,
                .layerCount = layerCount,
                .layer = (uint8_t) color[i].layer,
            };
            UTILS_UNUSED_IN_RELEASE VkExtent2D extent = colorTargets[i].getExtent2D();
            tmin = { std::min(tmin.x, extent.width), std::min(tmin.y, extent.height) };
            tmax = { std::max(tmax.x, extent.width), std::max(tmax.y, extent.height) };
            attachmentCount++;
        }
    }

    VulkanAttachment depthStencil[2] = {};
    if (depth.handle) {
        depthStencil[0] = {
            .texture = resource_ptr<VulkanTexture>::cast(&mResourceManager, depth.handle),
            .level = depth.level,
            .layerCount = layerCount,
            .layer = (uint8_t) depth.layer,
        };
        UTILS_UNUSED_IN_RELEASE VkExtent2D extent = depthStencil[0].getExtent2D();
        tmin = { std::min(tmin.x, extent.width), std::min(tmin.y, extent.height) };
        tmax = { std::max(tmax.x, extent.width), std::max(tmax.y, extent.height) };
        attachmentCount++;
    }

    if (stencil.handle) {
        depthStencil[1] = {
            .texture = resource_ptr<VulkanTexture>::cast(&mResourceManager, stencil.handle),
            .level = stencil.level,
            .layerCount = layerCount,
            .layer = (uint8_t) stencil.layer,
        };
        UTILS_UNUSED_IN_RELEASE VkExtent2D extent = depthStencil[1].getExtent2D();
        tmin = { std::min(tmin.x, extent.width), std::min(tmin.y, extent.height) };
        tmax = { std::max(tmax.x, extent.width), std::max(tmax.y, extent.height) };
        attachmentCount++;
    }

    // All attachments must have the same dimensions, which must be greater than or equal to the
    // render target dimensions.
    assert_invariant(attachmentCount > 0);
    assert_invariant(tmin == tmax);
    assert_invariant(tmin.x >= width && tmin.y >= height);

    auto rt = resource_ptr<VulkanRenderTarget>::make(&mResourceManager, rth, mPlatform->getDevice(),
            mPlatform->getPhysicalDevice(), mContext, &mResourceManager, mAllocator, &mCommands,
            width, height, samples, colorTargets, depthStencil, mStagePool, layerCount);
    rt.inc();
}

void VulkanDriver::destroyRenderTarget(Handle<HwRenderTarget> rth) {
    if (!rth) {
        return;
    }

    auto rt = resource_ptr<VulkanRenderTarget>::cast(&mResourceManager, rth);
    if (UTILS_UNLIKELY(rt == mDefaultRenderTarget)) {
        mDefaultRenderTarget = {};
    } else {
        rt.dec();
    }
}

void VulkanDriver::createFenceR(Handle<HwFence> fh, int) {
    VulkanCommandBuffer* cmdbuf;
    if (mCurrentRenderPass.commandBuffer) {
        cmdbuf = mCurrentRenderPass.commandBuffer;
    } else {
        cmdbuf = &mCommands.get();
    }
    // Note at this point, the fence has already been constructed via createFenceS, so we just tag
    // it with appropriate VulkanCmdFence, which is associated with the current, recording command
    // buffer.
    auto fence = resource_ptr<VulkanFence>::cast(&mResourceManager, fh);
    fence->fence = cmdbuf->getFenceStatus();
}

void VulkanDriver::createSwapChainR(Handle<HwSwapChain> sch, void* nativeWindow, uint64_t flags) {
    if ((flags & backend::SWAP_CHAIN_CONFIG_SRGB_COLORSPACE) != 0 && !isSRGBSwapChainSupported()) {
        FVK_LOGW << "sRGB swapchain requested, but Platform does not support it"
                 << utils::io::endl;
        flags = flags | ~(backend::SWAP_CHAIN_CONFIG_SRGB_COLORSPACE);
    }
    if (flags & backend::SWAP_CHAIN_CONFIG_PROTECTED_CONTENT) {
        if (!isProtectedContentSupported()) {
            FVK_LOGW << "protected swapchain requested, but Platform does not support it"
                << utils::io::endl;
        }
    }
    auto swapChain = resource_ptr<VulkanSwapChain>::make(&mResourceManager, sch, mPlatform,
            mContext, &mResourceManager, mAllocator, &mCommands, mStagePool, nativeWindow, flags);
    swapChain.inc();
}

void VulkanDriver::createSwapChainHeadlessR(Handle<HwSwapChain> sch, uint32_t width,
        uint32_t height, uint64_t flags) {
    if ((flags & backend::SWAP_CHAIN_CONFIG_SRGB_COLORSPACE) != 0 && !isSRGBSwapChainSupported()) {
        FVK_LOGW << "sRGB swapchain requested, but Platform does not support it"
                      << utils::io::endl;
        flags = flags | ~(backend::SWAP_CHAIN_CONFIG_SRGB_COLORSPACE);
    }
    assert_invariant(width > 0 && height > 0 && "Vulkan requires non-zero swap chain dimensions.");
    auto swapChain = resource_ptr<VulkanSwapChain>::make(&mResourceManager, sch, mPlatform,
            mContext, &mResourceManager, mAllocator, &mCommands, mStagePool, nullptr, flags,
            VkExtent2D{width, height});
    swapChain.inc();
}

void VulkanDriver::createTimerQueryR(Handle<HwTimerQuery> tqh, int) {
    // nothing to do, timer query was constructed in createTimerQueryS
}

void VulkanDriver::createDescriptorSetLayoutR(Handle<HwDescriptorSetLayout> dslh,
        backend::DescriptorSetLayout&& info) {
    auto layout = resource_ptr<VulkanDescriptorSetLayout>::make(&mResourceManager, dslh, info);
    mDescriptorSetManager.initVkLayout(layout);
    layout.inc();
}

void VulkanDriver::createDescriptorSetR(Handle<HwDescriptorSet> dsh,
        Handle<HwDescriptorSetLayout> dslh) {
    FVK_SYSTRACE_SCOPE();
    fvkmemory::resource_ptr<VulkanDescriptorSetLayout> layout =
            fvkmemory::resource_ptr<VulkanDescriptorSetLayout>::cast(&mResourceManager, dslh);
    auto set = mDescriptorSetManager.createSet(dsh, layout);
    set.inc();
}

Handle<HwVertexBufferInfo> VulkanDriver::createVertexBufferInfoS() noexcept {
    return mResourceManager.allocHandle<VulkanVertexBufferInfo>();
}

Handle<HwVertexBuffer> VulkanDriver::createVertexBufferS() noexcept {
    return mResourceManager.allocHandle<VulkanVertexBuffer>();
}

Handle<HwIndexBuffer> VulkanDriver::createIndexBufferS() noexcept {
    return mResourceManager.allocHandle<VulkanIndexBuffer>();
}

Handle<HwBufferObject> VulkanDriver::createBufferObjectS() noexcept {
    return mResourceManager.allocHandle<VulkanBufferObject>();
}

Handle<HwTexture> VulkanDriver::createTextureS() noexcept {
    return mResourceManager.allocHandle<VulkanTexture>();
}

Handle<HwTexture> VulkanDriver::createTextureViewS() noexcept {
    return mResourceManager.allocHandle<VulkanTexture>();
}

Handle<HwTexture> VulkanDriver::createTextureViewSwizzleS() noexcept {
    return mResourceManager.allocHandle<VulkanTexture>();
}

Handle<HwTexture> VulkanDriver::createTextureExternalImageS() noexcept {
    return mResourceManager.allocHandle<VulkanTexture>();
}

Handle<HwTexture> VulkanDriver::createTextureExternalImagePlaneS() noexcept {
    return mResourceManager.allocHandle<VulkanTexture>();
}

Handle<HwTexture> VulkanDriver::importTextureS() noexcept {
    return mResourceManager.allocHandle<VulkanTexture>();
}

Handle<HwRenderPrimitive> VulkanDriver::createRenderPrimitiveS() noexcept {
    return mResourceManager.allocHandle<VulkanRenderPrimitive>();
}

Handle<HwProgram> VulkanDriver::createProgramS() noexcept {
    return mResourceManager.allocHandle<VulkanProgram>();
}

Handle<HwRenderTarget> VulkanDriver::createDefaultRenderTargetS() noexcept {
    return mResourceManager.allocHandle<VulkanRenderTarget>();
}

Handle<HwRenderTarget> VulkanDriver::createRenderTargetS() noexcept {
    return mResourceManager.allocHandle<VulkanRenderTarget>();
}

Handle<HwFence> VulkanDriver::createFenceS() noexcept {
    auto handle = mResourceManager.allocHandle<VulkanFence>();
    auto fence = resource_ptr<VulkanFence>::make(&mResourceManager, handle);
    fence.inc();
    return handle;
}

Handle<HwSwapChain> VulkanDriver::createSwapChainS() noexcept {
    return mResourceManager.allocHandle<VulkanSwapChain>();
}

Handle<HwSwapChain> VulkanDriver::createSwapChainHeadlessS() noexcept {
    return mResourceManager.allocHandle<VulkanSwapChain>();
}

Handle<HwTimerQuery> VulkanDriver::createTimerQueryS() noexcept {
    // The handle must be constructed here, as a synchronous call to getTimerQueryValue might happen
    // before createTimerQueryR is executed.
    auto query = resource_ptr<VulkanTimerQuery>::construct(&mResourceManager,
            mTimestamps->getNextQuery());
    query.inc();
    return Handle<VulkanTimerQuery>(query.id());
}

Handle<HwDescriptorSetLayout> VulkanDriver::createDescriptorSetLayoutS() noexcept {
    return mResourceManager.allocHandle<VulkanDescriptorSetLayout>();
}

Handle<HwDescriptorSet> VulkanDriver::createDescriptorSetS() noexcept {
    return mResourceManager.allocHandle<VulkanDescriptorSet>();
}

void VulkanDriver::destroySwapChain(Handle<HwSwapChain> sch) {
    if (!sch) {
        return;
    }
    auto swapChain = resource_ptr<VulkanSwapChain>::cast(&mResourceManager, sch);
    if (mCurrentSwapChain == swapChain) {
        mCurrentSwapChain = {};
    }
    swapChain.dec();
}

void VulkanDriver::destroyStream(Handle<HwStream> sh) {
}

void VulkanDriver::destroyTimerQuery(Handle<HwTimerQuery> tqh) {
    if (!tqh) {
        return;
    }
    auto vtq = resource_ptr<VulkanTimerQuery>::cast(&mResourceManager, tqh);
    vtq.dec();
}

void VulkanDriver::destroyDescriptorSetLayout(Handle<HwDescriptorSetLayout> dslh) {
    auto layout = resource_ptr<VulkanDescriptorSetLayout>::cast(&mResourceManager, dslh);
    layout.dec();
}

void VulkanDriver::destroyDescriptorSet(Handle<HwDescriptorSet> dsh) {
    auto set = resource_ptr<VulkanDescriptorSet>::cast(&mResourceManager, dsh);
    set.dec();
}

Handle<HwStream> VulkanDriver::createStreamNative(void* nativeStream) {
    return {};
}

Handle<HwStream> VulkanDriver::createStreamAcquired() {
    return {};
}

void VulkanDriver::setAcquiredImage(Handle<HwStream> sh, void* image,
        CallbackHandler* handler, StreamCallback cb, void* userData) {
}

void VulkanDriver::setStreamDimensions(Handle<HwStream> sh, uint32_t width, uint32_t height) {
}

int64_t VulkanDriver::getStreamTimestamp(Handle<HwStream> sh) {
    return 0;
}

void VulkanDriver::updateStreams(CommandStream* driver) {
}

void VulkanDriver::destroyFence(Handle<HwFence> fh) {
    auto fence = resource_ptr<VulkanFence>::cast(&mResourceManager, fh);
    fence.dec();
}

FenceStatus VulkanDriver::getFenceStatus(Handle<HwFence> fh) {
    auto fence = resource_ptr<VulkanFence>::cast(&mResourceManager, fh);

    auto& cmdfence = fence->fence;
    if (!cmdfence) {
        // If wait is called before a fence actually exists, we return timeout.  This matches the
        // current behavior in OpenGLDriver, but we should eventually reconsider a different error
        // code.
        return FenceStatus::TIMEOUT_EXPIRED;
    }

    // Internally we use the VK_INCOMPLETE status to mean "not yet submitted".
    // When this fence gets submitted, its status changes to VK_NOT_READY.
    if (cmdfence->getStatus() == VK_SUCCESS) {
        return FenceStatus::CONDITION_SATISFIED;
    }


    // Two other states are possible:
    //   - VK_INCOMPLETE: the corresponding buffer has not yet been submitted.
    //   - VK_NOT_READY: the buffer has been submitted but not yet signaled.
    //     In either case, we return TIMEOUT_EXPIRED to indicate the fence has not been signaled.
    return FenceStatus::TIMEOUT_EXPIRED;
}

// We create all textures using VK_IMAGE_TILING_OPTIMAL, so our definition of "supported" is that
// the GPU supports the given texture format with non-zero optimal tiling features.
bool VulkanDriver::isTextureFormatSupported(TextureFormat format) {
    VkFormat vkformat = getVkFormat(format);
    if (vkformat == VK_FORMAT_UNDEFINED) {
        return false;
    }
    VkFormatProperties info;
    vkGetPhysicalDeviceFormatProperties(mPlatform->getPhysicalDevice(), vkformat, &info);
    return info.optimalTilingFeatures != 0;
}

bool VulkanDriver::isTextureSwizzleSupported() {
    return true;
}

bool VulkanDriver::isTextureFormatMipmappable(TextureFormat format) {
    switch (format) {
        case TextureFormat::DEPTH16:
        case TextureFormat::DEPTH24:
        case TextureFormat::DEPTH32F:
        case TextureFormat::DEPTH24_STENCIL8:
        case TextureFormat::DEPTH32F_STENCIL8:
            return false;
        default:
            return isRenderTargetFormatSupported(format);
    }
}

bool VulkanDriver::isRenderTargetFormatSupported(TextureFormat format) {
    VkFormat vkformat = getVkFormat(format);
    if (vkformat == VK_FORMAT_UNDEFINED) {
        return false;
    }
    VkFormatProperties info;
    vkGetPhysicalDeviceFormatProperties(mPlatform->getPhysicalDevice(), vkformat, &info);
    return (info.optimalTilingFeatures & VK_FORMAT_FEATURE_COLOR_ATTACHMENT_BIT) != 0;
}

bool VulkanDriver::isFrameBufferFetchSupported() {
    // TODO: we must fix this before landing descriptor set change.  Otherwise, the scuba tests will fail.
    //return true;
    return false;
}

bool VulkanDriver::isFrameBufferFetchMultiSampleSupported() {
    return false;
}

bool VulkanDriver::isFrameTimeSupported() {
    return true;
}

bool VulkanDriver::isAutoDepthResolveSupported() {
    // TODO: this could be supported with vk 1.2 or VK_KHR_depth_stencil_resolve
    return false;
}

bool VulkanDriver::isSRGBSwapChainSupported() {
    return mIsSRGBSwapChainSupported;
}

bool VulkanDriver::isProtectedContentSupported() {
    return mContext.isProtectedMemorySupported();
}

bool VulkanDriver::isStereoSupported() {
    switch (mStereoscopicType) {
        case backend::StereoscopicType::INSTANCED:
            return mContext.isClipDistanceSupported();
        case backend::StereoscopicType::MULTIVIEW:
            return mContext.isMultiviewEnabled();
        case backend::StereoscopicType::NONE:
            return false;
    }
}

bool VulkanDriver::isParallelShaderCompileSupported() {
    return false;
}

bool VulkanDriver::isDepthStencilResolveSupported() {
    // TODO: apparently it could be supported in core 1.2 and/or with VK_KHR_depth_stencil_resolve
    return false;
}

bool VulkanDriver::isDepthStencilBlitSupported(TextureFormat format) {
    auto const& formats = mContext.getBlittableDepthStencilFormats();
    return std::find(formats.begin(), formats.end(), getVkFormat(format)) != formats.end();
}

bool VulkanDriver::isProtectedTexturesSupported() {
    return false;
}

bool VulkanDriver::isDepthClampSupported() {
    return mContext.isDepthClampSupported();
}

bool VulkanDriver::isWorkaroundNeeded(Workaround workaround) {
    switch (workaround) {
        case Workaround::SPLIT_EASU: {
            auto const vendorId = mContext.getPhysicalDeviceVendorId();
            // early exit condition is flattened in EASU code
            return vendorId == 0x5143; // Qualcomm
        }
        case Workaround::ALLOW_READ_ONLY_ANCILLARY_FEEDBACK_LOOP:
            // Supporting depth attachment as both sampler and attachment is only possible if we set
            // the depth attachment as read-only (e.g. during SSAO pass), however note that the
            // store-ops for attachments wrt VkRenderPass only has VK_ATTACHMENT_STORE_OP_DONT_CARE
            // and VK_ATTACHMENT_STORE_OP_STORE for versions below 1.3. Only at 1.3 and above do we
            // have a true read-only choice VK_ATTACHMENT_STORE_OP_NONE. That means for < 1.3, we
            // will trigger a validation sync error if we use the depth attachment also as a
            // sampler. See full error below:
            //
            // SYNC-HAZARD-WRITE-AFTER-READ(ERROR / SPEC): msgNum: 929810911 - Validation Error:
            // [ SYNC-HAZARD-WRITE-AFTER-READ ] Object 0: handle = 0x6160000c3680,
            // type = VK_OBJECT_TYPE_RENDER_PASS; | MessageID = 0x376bc9df | vkCmdEndRenderPass:
            // Hazard WRITE_AFTER_READ in subpass 0 for attachment 1 depth aspect during store with
            // storeOp VK_ATTACHMENT_STORE_OP_STORE. Access info (usage:
            // SYNC_LATE_FRAGMENT_TESTS_DEPTH_STENCIL_ATTACHMENT_WRITE, prior_usage:
            // SYNC_FRAGMENT_SHADER_SHADER_STORAGE_READ, read_barriers: VK_PIPELINE_STAGE_2_NONE,
            // command: vkCmdDrawIndexed, seq_no: 177, reset_no: 1)
            //
            // Therefore we apply the existing workaround of an extra blit until a better
            // resolution.
            return false;
        case Workaround::ADRENO_UNIFORM_ARRAY_CRASH:
            return false;
        case Workaround::DISABLE_BLIT_INTO_TEXTURE_ARRAY:
            return false;
        default:
            return false;
    }
    return false;
}

FeatureLevel VulkanDriver::getFeatureLevel() {
    VkPhysicalDeviceLimits const& limits = mContext.getPhysicalDeviceLimits();

    // If cubemap arrays are not supported, then this is an FL1 device.
    if (!mContext.isImageCubeArraySupported()) {
        return FeatureLevel::FEATURE_LEVEL_1;
    }

    // If the max sampler counts do not meet FL2 standards, then this is an FL1 device.
    const auto& fl2 = FEATURE_LEVEL_CAPS[+FeatureLevel::FEATURE_LEVEL_2];
    if (limits.maxPerStageDescriptorSamplers < fl2.MAX_VERTEX_SAMPLER_COUNT  ||
        limits.maxPerStageDescriptorSamplers < fl2.MAX_FRAGMENT_SAMPLER_COUNT) {
        return FeatureLevel::FEATURE_LEVEL_1;
    }

    // If the max sampler counts do not meet FL3 standards, then this is an FL2 device.
    const auto& fl3 = FEATURE_LEVEL_CAPS[+FeatureLevel::FEATURE_LEVEL_3];
    if (limits.maxPerStageDescriptorSamplers < fl3.MAX_VERTEX_SAMPLER_COUNT||
        limits.maxPerStageDescriptorSamplers < fl3.MAX_FRAGMENT_SAMPLER_COUNT) {
        return FeatureLevel::FEATURE_LEVEL_2;
    }

    return FeatureLevel::FEATURE_LEVEL_3;
}

math::float2 VulkanDriver::getClipSpaceParams() {
    // virtual and physical z-coordinate of clip-space is in [-w, 0]
    // Note: this is actually never used (see: main.vs), but it's a backend API, so we implement it
    // properly.
    return math::float2{ 1.0f, 0.0f };
}

uint8_t VulkanDriver::getMaxDrawBuffers() {
    return MRT::MIN_SUPPORTED_RENDER_TARGET_COUNT; // TODO: query real value
}

size_t VulkanDriver::getMaxUniformBufferSize() {
    // TODO: return the actual size instead of hardcoded value
    // TODO: devices that return less than 32768 should be rejected. This represents only 3%
    //       of android devices.
    return 32768;
}

void VulkanDriver::setVertexBufferObject(Handle<HwVertexBuffer> vbh, uint32_t index,
        Handle<HwBufferObject> boh) {
    auto vb = resource_ptr<VulkanVertexBuffer>::cast(&mResourceManager, vbh);
    auto bo = resource_ptr<VulkanBufferObject>::cast(&mResourceManager, boh);
    assert_invariant(bo->bindingType == BufferObjectBinding::VERTEX);
    vb->setBuffer(bo, index);
}

void VulkanDriver::updateIndexBuffer(Handle<HwIndexBuffer> ibh, BufferDescriptor&& p,
        uint32_t byteOffset) {
    VulkanCommandBuffer& commands = mCommands.get();
    auto ib = resource_ptr<VulkanIndexBuffer>::cast(&mResourceManager, ibh);
    commands.acquire(ib);
    ib->buffer.loadFromCpu(commands.buffer(), p.buffer, byteOffset, p.size);

    scheduleDestroy(std::move(p));
}

void VulkanDriver::updateBufferObject(Handle<HwBufferObject> boh, BufferDescriptor&& bd,
        uint32_t byteOffset) {
    VulkanCommandBuffer& commands = mCommands.get();

    auto bo = resource_ptr<VulkanBufferObject>::cast(&mResourceManager, boh);
    commands.acquire(bo);
    bo->buffer.loadFromCpu(commands.buffer(), bd.buffer, byteOffset, bd.size);

    scheduleDestroy(std::move(bd));
}

void VulkanDriver::updateBufferObjectUnsynchronized(Handle<HwBufferObject> boh,
        BufferDescriptor&& bd, uint32_t byteOffset) {
    VulkanCommandBuffer& commands = mCommands.get();
    auto bo = resource_ptr<VulkanBufferObject>::cast(&mResourceManager, boh);
    commands.acquire(bo);
    // TODO: implement unsynchronized version
    bo->buffer.loadFromCpu(commands.buffer(), bd.buffer, byteOffset, bd.size);
    scheduleDestroy(std::move(bd));
}

void VulkanDriver::resetBufferObject(Handle<HwBufferObject> boh) {
    // TODO: implement resetBufferObject(). This is equivalent to calling
    // destroyBufferObject() followed by createBufferObject() keeping the same handle.
    // It is actually okay to keep a no-op implementation, the intention here is to "orphan" the
    // buffer (and possibly return it to a pool) and allocate a new one (or get it from a pool),
    // so that no further synchronization with the GPU is needed.
    // This is only useful if updateBufferObjectUnsynchronized() is implemented unsynchronizedly.
}

void VulkanDriver::update3DImage(Handle<HwTexture> th, uint32_t level, uint32_t xoffset,
        uint32_t yoffset, uint32_t zoffset, uint32_t width, uint32_t height, uint32_t depth,
        PixelBufferDescriptor&& data) {
    auto texture = resource_ptr<VulkanTexture>::cast(&mResourceManager, th);
    texture->updateImage(data, width, height, depth, xoffset, yoffset, zoffset, level);
    scheduleDestroy(std::move(data));
}

void VulkanDriver::setupExternalImage(void* image) {
}

TimerQueryResult VulkanDriver::getTimerQueryValue(Handle<HwTimerQuery> tqh, uint64_t* elapsedTime) {
    auto vtq = resource_ptr<VulkanTimerQuery>::cast(&mResourceManager, tqh);
    if (!vtq->isCompleted()) {
        return TimerQueryResult::NOT_READY;
    }

    auto results = mTimestamps->getResult(vtq);
    uint64_t timestamp0 = results[0];
    uint64_t available0 = results[1];
    uint64_t timestamp1 = results[2];
    uint64_t available1 = results[3];

    if (available0 == 0 || available1 == 0) {
        return TimerQueryResult::NOT_READY;
    }

    FILAMENT_CHECK_POSTCONDITION(timestamp1 >= timestamp0)
            << "Timestamps are not monotonically increasing.";

    // NOTE: MoltenVK currently writes system time so the following delta will always be zero.
    // However there are plans for implementing this properly. See the following GitHub ticket.
    // https://github.com/KhronosGroup/MoltenVK/issues/773

    float const period = mContext.getPhysicalDeviceLimits().timestampPeriod;
    uint64_t delta = uint64_t(float(timestamp1 - timestamp0) * period);
    *elapsedTime = delta;
    return TimerQueryResult::AVAILABLE;
}

void VulkanDriver::setExternalImage(Handle<HwTexture> th, void* image) {
}

void VulkanDriver::setExternalImagePlane(Handle<HwTexture> th, void* image, uint32_t plane) {
}

void VulkanDriver::setExternalStream(Handle<HwTexture> th, Handle<HwStream> sh) {
}

void VulkanDriver::generateMipmaps(Handle<HwTexture> th) {
    auto t = resource_ptr<VulkanTexture>::cast(&mResourceManager, th);
    assert_invariant(t);

    int32_t layerCount = int32_t(t->depth);
    if (t->target == SamplerType::SAMPLER_CUBEMAP_ARRAY ||
        t->target == SamplerType::SAMPLER_CUBEMAP) {
        layerCount *= 6;
    }

    assert_invariant(layerCount < 1 << (sizeof(VulkanAttachment::layerCount) * 8));

    // FIXME: the loop below can perform many layout transitions and back. We should be
    //        able to optimize that.
    uint8_t level = 0;
    int32_t srcw = int32_t(t->width);
    int32_t srch = int32_t(t->height);
    do {
        int32_t const dstw = std::max(srcw >> 1, 1);
        int32_t const dsth = std::max(srch >> 1, 1);
        const VkOffset3D srcOffsets[2] = {{ 0, 0, 0 }, { srcw, srch, 1 }};
        const VkOffset3D dstOffsets[2] = {{ 0, 0, 0 }, { dstw, dsth, 1 }};

        // TODO: there should be a way to do this using layerCount in vkBlitImage
        // TODO: vkBlitImage should be able to handle 3D textures too
        for (uint8_t layer = 0; layer < layerCount; layer++) {
            VulkanAttachment dst {
                .level = uint8_t(level + 1),
                .layer = layer,
            };
            dst.texture = t;
            VulkanAttachment src {
                .level = uint8_t(level),
                .layer = layer,
            };
            src.texture = t;
            mBlitter.blit(VK_FILTER_LINEAR, dst, dstOffsets, src, srcOffsets);
        }

        level++;
        srcw = dstw;
        srch = dsth;
    } while ((srcw > 1 || srch > 1) && level < t->levels);
}

void VulkanDriver::compilePrograms(CompilerPriorityQueue priority,
        CallbackHandler* handler, CallbackHandler::Callback callback, void* user) {
    if (callback) {
        scheduleCallback(handler, user, callback);
    }
}

void VulkanDriver::beginRenderPass(Handle<HwRenderTarget> rth, const RenderPassParams& params) {
    FVK_SYSTRACE_SCOPE();

    auto rt = resource_ptr<VulkanRenderTarget>::cast(&mResourceManager, rth);
    VkExtent2D const extent = rt->getExtent();

    assert_invariant(rt == mDefaultRenderTarget || extent.width > 0 && extent.height > 0);

    VulkanCommandBuffer* commandBuffer = rt->isProtected() ?
           &mCommands.getProtected() : &mCommands.get();

    // Filament has the expectation that the contents of the swap chain are not preserved on the
    // first render pass. Note however that its contents are often preserved on subsequent render
    // passes, due to multiple views.
    TargetBufferFlags discardStart = params.flags.discardStart;
    if (rt->isSwapChain()) {
        fvkmemory::resource_ptr<VulkanSwapChain> sc = mCurrentSwapChain;
        assert_invariant(sc);
        if (sc->isFirstRenderPass()) {
            discardStart |= TargetBufferFlags::COLOR;
            sc->markFirstRenderPass();
        }
    }

#if FVK_ENABLED(FVK_DEBUG_TEXTURE)
    if (rt->hasDepth()) {
        auto depth = rt->getDepth();
        depth.texture->print();
    }
#endif

    // We need to determine whether the same depth texture is both sampled and set as an attachment.
    // If that's the case, we need to change the layout of the texture to DEPTH_SAMPLER, which is a
    // more general layout. Otherwise, we prefer the DEPTH_ATTACHMENT layout, which is optimal for
    // the non-sampling case.
    VkCommandBuffer const cmdbuffer = commandBuffer->buffer();

    // Scissor is reset with each render pass
    // This also takes care of VUID-vkCmdDrawIndexed-None-07832.
    VkRect2D const scissor{ .offset = { 0, 0 }, .extent = extent };
    vkCmdSetScissor(cmdbuffer, 0, 1, &scissor);

    VulkanLayout currentDepthLayout = VulkanLayout::UNDEFINED;
    TargetBufferFlags clearVal = params.flags.clear;
    TargetBufferFlags discardEndVal = params.flags.discardEnd;
    if (rt->hasDepth()) {
        if (params.readOnlyDepthStencil & RenderPassParams::READONLY_DEPTH) {
            discardEndVal &= ~TargetBufferFlags::DEPTH;
            clearVal &= ~TargetBufferFlags::DEPTH;
        }
        currentDepthLayout = VulkanLayout::DEPTH_ATTACHMENT;
    }


    // Create the VkRenderPass or fetch it from cache.

    VulkanFboCache::RenderPassKey rpkey = rt->getRenderPassKey();
    rpkey.clear = clearVal;
    rpkey.discardStart = discardStart;
    rpkey.discardEnd = discardEndVal;
    rpkey.initialDepthLayout = currentDepthLayout;
    rpkey.subpassMask = uint8_t(params.subpassMask);

    VkRenderPass renderPass = mFramebufferCache.getRenderPass(rpkey);
    mPipelineCache.bindRenderPass(renderPass, 0);

    // Create the VkFramebuffer or fetch it from cache.
    VulkanFboCache::FboKey fbkey = rt->getFboKey();
    fbkey.renderPass = renderPass;
    fbkey.layers = 1;

    rt->emitBarriersBeginRenderPass(*commandBuffer);

    VkFramebuffer vkfb = mFramebufferCache.getFramebuffer(fbkey);

// Assign a label to the framebuffer for debugging purposes.
#if FVK_ENABLED(FVK_DEBUG_GROUP_MARKERS | FVK_DEBUG_DEBUG_UTILS)
    auto const topMarker = mCommands.getTopGroupMarker();
    if (!topMarker.empty()) {
        DebugUtils::setName(VK_OBJECT_TYPE_FRAMEBUFFER, reinterpret_cast<uint64_t>(vkfb),
                topMarker.c_str());
    }
#endif

    // The current command buffer now has references to the render target and its attachments.
    commandBuffer->acquire(rt);

    // Populate the structures required for vkCmdBeginRenderPass.
    VkRenderPassBeginInfo renderPassInfo {
        .sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO,
        .renderPass = renderPass,
        .framebuffer = vkfb,

        // The renderArea field constrains the LoadOp, but scissoring does not.
        // Therefore, we do not set the scissor rect here, we only need it in draw().
        .renderArea = { .offset = {}, .extent = extent }
    };

    rt->transformClientRectToPlatform(&renderPassInfo.renderArea);

    VkClearValue clearValues[
            MRT::MAX_SUPPORTED_RENDER_TARGET_COUNT + MRT::MAX_SUPPORTED_RENDER_TARGET_COUNT +
            1] = {};
    if (clearVal != TargetBufferFlags::NONE) {
        // NOTE: clearValues must be populated in the same order as the attachments array in
        // VulkanFboCache::getFramebuffer. Values must be provided regardless of whether Vulkan is
        // actually clearing that particular target.
        for (int i = 0; i < MRT::MAX_SUPPORTED_RENDER_TARGET_COUNT; i++) {
            if (fbkey.color[i]) {
                VkClearValue &clearValue = clearValues[renderPassInfo.clearValueCount++];
                clearValue.color.float32[0] = params.clearColor.r;
                clearValue.color.float32[1] = params.clearColor.g;
                clearValue.color.float32[2] = params.clearColor.b;
                clearValue.color.float32[3] = params.clearColor.a;
            }
        }
        // Resolve attachments are not cleared but still have entries in the list, so skip over them.
        for (int i = 0; i < MRT::MAX_SUPPORTED_RENDER_TARGET_COUNT; i++) {
            if (rpkey.needsResolveMask & (1u << i)) {
                renderPassInfo.clearValueCount++;
            }
        }
        if (fbkey.depth) {
            VkClearValue &clearValue = clearValues[renderPassInfo.clearValueCount++];
            clearValue.depthStencil = {(float) params.clearDepth, 0};
        }
        renderPassInfo.pClearValues = &clearValues[0];
    }

    vkCmdBeginRenderPass(cmdbuffer, &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);

    VkViewport viewport = {
        .x = (float) params.viewport.left,
        .y = (float) params.viewport.bottom,
        .width = (float) params.viewport.width,
        .height = (float) params.viewport.height,
        .minDepth = params.depthRange.near,
        .maxDepth = params.depthRange.far
    };

    rt->transformViewportToPlatform(&viewport);
    vkCmdSetViewport(cmdbuffer, 0, 1, &viewport);

    mCurrentRenderPass = {
        .commandBuffer = commandBuffer,
        .renderTarget = rt,
        .renderPass = renderPassInfo.renderPass,
        .params = params,
        .currentSubpass = 0,
    };
}

void VulkanDriver::endRenderPass(int) {
    FVK_SYSTRACE_SCOPE();

    VkCommandBuffer cmdbuffer = mCurrentRenderPass.commandBuffer->buffer();
    vkCmdEndRenderPass(cmdbuffer);

    auto rt = mCurrentRenderPass.renderTarget;
    assert_invariant(rt);

    // Since we might soon be sampling from the render target that we just wrote to, we need a
    // pipeline barrier between framebuffer writes and shader reads.
    rt->emitBarriersEndRenderPass(*mCurrentRenderPass.commandBuffer);

    mRenderPassFboInfo = {};
    mCurrentRenderPass.renderTarget = {};
    mCurrentRenderPass.renderPass = VK_NULL_HANDLE;

    mCurrentRenderPass.commandBuffer = nullptr;
}

void VulkanDriver::nextSubpass(int) {
    FILAMENT_CHECK_PRECONDITION(mCurrentRenderPass.currentSubpass == 0)
            << "Only two subpasses are currently supported.";

    auto renderTarget = mCurrentRenderPass.renderTarget;
    assert_invariant(renderTarget);
    assert_invariant(mCurrentRenderPass.params.subpassMask);

    vkCmdNextSubpass(mCurrentRenderPass.commandBuffer->buffer(),
            VK_SUBPASS_CONTENTS_INLINE);

    mPipelineCache.bindRenderPass(mCurrentRenderPass.renderPass,
            ++mCurrentRenderPass.currentSubpass);

    if (mCurrentRenderPass.params.subpassMask & 0x1) {
        VulkanAttachment& subpassInput = renderTarget->getColor0();
        mDescriptorSetManager.updateInputAttachment({}, subpassInput);
    }
}

void VulkanDriver::makeCurrent(Handle<HwSwapChain> drawSch, Handle<HwSwapChain> readSch) {
    FVK_SYSTRACE_SCOPE();

    ASSERT_PRECONDITION_NON_FATAL(drawSch == readSch,
            "Vulkan driver does not support distinct draw/read swap chains.");

    resource_ptr<VulkanSwapChain> swapChain =
            resource_ptr<VulkanSwapChain>::cast(&mResourceManager, drawSch);
    mCurrentSwapChain = swapChain;

    bool resized = false;
    swapChain->acquire(resized);

    if (resized) {
        mFramebufferCache.reset();
    }

    if (UTILS_LIKELY(mDefaultRenderTarget)) {
        mDefaultRenderTarget->bindToSwapChain(swapChain);
    }
}

void VulkanDriver::commit(Handle<HwSwapChain> sch) {
    FVK_SYSTRACE_SCOPE();

    auto swapChain = resource_ptr<VulkanSwapChain>::cast(&mResourceManager, sch);

    // Present the backbuffer after the most recent command buffer submission has finished.
    swapChain->present();
}

void VulkanDriver::setPushConstant(backend::ShaderStage stage, uint8_t index,
        backend::PushConstantVariant value) {
    assert_invariant(mBoundPipeline.program && "Expect a program when writing to push constants");
    assert_invariant(mCurrentRenderPass.commandBuffer && "Should be called within a renderpass");
    mBoundPipeline.program->writePushConstant(mCurrentRenderPass.commandBuffer->buffer(),
            mBoundPipeline.pipelineLayout, stage, index, value);
}

void VulkanDriver::insertEventMarker(char const* string) {
#if FVK_ENABLED(FVK_DEBUG_GROUP_MARKERS)
    mCommands.insertEventMarker(string, strlen(string));
#endif
}

void VulkanDriver::pushGroupMarker(char const* string) {
    // Turns out all the markers are 0-terminated, so we can just pass it without len.
#if FVK_ENABLED(FVK_DEBUG_GROUP_MARKERS)
    mCommands.pushGroupMarker(string);
#endif
    FVK_SYSTRACE_CONTEXT();
    FVK_SYSTRACE_START(string);
}

void VulkanDriver::popGroupMarker(int) {
#if FVK_ENABLED(FVK_DEBUG_GROUP_MARKERS)
    mCommands.popGroupMarker();
#endif
    FVK_SYSTRACE_CONTEXT();
    FVK_SYSTRACE_END();
}

void VulkanDriver::startCapture(int) {}

void VulkanDriver::stopCapture(int) {}

void VulkanDriver::readPixels(Handle<HwRenderTarget> src, uint32_t x, uint32_t y, uint32_t width,
        uint32_t height, PixelBufferDescriptor&& pbd) {
    auto srcTarget = resource_ptr<VulkanRenderTarget>::cast(&mResourceManager, src);
    mCommands.flush();
    mReadPixels.run(
            srcTarget, x, y, width, height, mPlatform->getGraphicsQueueFamilyIndex(),
            std::move(pbd),
            [&context = mContext](uint32_t reqs, VkFlags flags) {
                return context.selectMemoryType(reqs, flags);
            },
            [this](PixelBufferDescriptor&& pbd) {
                scheduleDestroy(std::move(pbd));
            });
}

void VulkanDriver::readBufferSubData(backend::BufferObjectHandle boh,
        uint32_t offset, uint32_t size, backend::BufferDescriptor&& p) {
    // TODO: implement readBufferSubData
    scheduleDestroy(std::move(p));
}

void VulkanDriver::resolve(
        Handle<HwTexture> dst, uint8_t srcLevel, uint8_t srcLayer,
        Handle<HwTexture> src, uint8_t dstLevel, uint8_t dstLayer) {
    FVK_SYSTRACE_SCOPE();

    FILAMENT_CHECK_PRECONDITION(mCurrentRenderPass.renderPass == VK_NULL_HANDLE)
            << "resolve() cannot be invoked inside a render pass.";

    auto srcTexture = resource_ptr<VulkanTexture>::cast(&mResourceManager, src);
    auto dstTexture = resource_ptr<VulkanTexture>::cast(&mResourceManager, dst);

    assert_invariant(srcTexture);
    assert_invariant(dstTexture);

    FILAMENT_CHECK_PRECONDITION(
            dstTexture->width == srcTexture->width && dstTexture->height == srcTexture->height)
            << "invalid resolve: src and dst sizes don't match";

    FILAMENT_CHECK_PRECONDITION(srcTexture->samples > 1 && dstTexture->samples == 1)
            << "invalid resolve: src.samples=" << +srcTexture->samples
            << ", dst.samples=" << +dstTexture->samples;

    FILAMENT_CHECK_PRECONDITION(srcTexture->format == dstTexture->format)
            << "src and dst texture format don't match";

    FILAMENT_CHECK_PRECONDITION(!isDepthFormat(srcTexture->format))
            << "can't resolve depth formats";

    FILAMENT_CHECK_PRECONDITION(!isStencilFormat(srcTexture->format))
            << "can't resolve stencil formats";

    FILAMENT_CHECK_PRECONDITION(any(dstTexture->usage & TextureUsage::BLIT_DST))
            << "texture doesn't have BLIT_DST";

    FILAMENT_CHECK_PRECONDITION(any(srcTexture->usage & TextureUsage::BLIT_SRC))
            << "texture doesn't have BLIT_SRC";

    mBlitter.resolve(
            { .texture = dstTexture, .level = dstLevel, .layer = dstLayer },
            { .texture = srcTexture, .level = srcLevel, .layer = srcLayer });
}

void VulkanDriver::blit(
        Handle<HwTexture> dst, uint8_t srcLevel, uint8_t srcLayer, math::uint2 dstOrigin,
        Handle<HwTexture> src, uint8_t dstLevel, uint8_t dstLayer, math::uint2 srcOrigin,
        math::uint2 size) {
    FVK_SYSTRACE_SCOPE();

    FILAMENT_CHECK_PRECONDITION(mCurrentRenderPass.renderPass == VK_NULL_HANDLE)
            << "blit() cannot be invoked inside a render pass.";

    auto srcTexture = resource_ptr<VulkanTexture>::cast(&mResourceManager, src);
    auto dstTexture = resource_ptr<VulkanTexture>::cast(&mResourceManager, dst);

    FILAMENT_CHECK_PRECONDITION(any(dstTexture->usage & TextureUsage::BLIT_DST))
            << "texture doesn't have BLIT_DST";

    FILAMENT_CHECK_PRECONDITION(any(srcTexture->usage & TextureUsage::BLIT_SRC))
            << "texture doesn't have BLIT_SRC";

    FILAMENT_CHECK_PRECONDITION(srcTexture->format == dstTexture->format)
            << "src and dst texture format don't match";

    // The Y inversion below makes it so that Vk matches GL and Metal.

    auto const srcLeft   = int32_t(srcOrigin.x);
    auto const dstLeft   = int32_t(dstOrigin.x);
    auto const srcTop    = int32_t(srcTexture->height - (srcOrigin.y + size.y));
    auto const dstTop    = int32_t(dstTexture->height - (dstOrigin.y + size.y));
    auto const srcRight  = int32_t(srcOrigin.x + size.x);
    auto const dstRight  = int32_t(dstOrigin.x + size.x);
    auto const srcBottom = int32_t(srcTop + size.y);
    auto const dstBottom = int32_t(dstTop + size.y);
    VkOffset3D const srcOffsets[2] = { { srcLeft, srcTop, 0 }, { srcRight, srcBottom, 1 }};
    VkOffset3D const dstOffsets[2] = { { dstLeft, dstTop, 0 }, { dstRight, dstBottom, 1 }};

    // no scaling guaranteed
    mBlitter.blit(VK_FILTER_NEAREST,
            { .texture = dstTexture, .level = dstLevel, .layer = dstLayer }, dstOffsets,
            { .texture = srcTexture, .level = srcLevel, .layer = srcLayer }, srcOffsets);
}

void VulkanDriver::blitDEPRECATED(TargetBufferFlags buffers,
        Handle<HwRenderTarget> dst, Viewport dstRect,
        Handle<HwRenderTarget> src, Viewport srcRect,
        SamplerMagFilter filter) {
    FVK_SYSTRACE_SCOPE();

    // Note: blitDEPRECATED is only used for Renderer::copyFrame()

    FILAMENT_CHECK_PRECONDITION(mCurrentRenderPass.renderPass == VK_NULL_HANDLE)
            << "blitDEPRECATED() cannot be invoked inside a render pass.";

    FILAMENT_CHECK_PRECONDITION(buffers == TargetBufferFlags::COLOR0)
            << "blitDEPRECATED only supports COLOR0";

    FILAMENT_CHECK_PRECONDITION(
            srcRect.left >= 0 && srcRect.bottom >= 0 && dstRect.left >= 0 && dstRect.bottom >= 0)
            << "Source and destination rects must be positive.";

    auto dstTarget = resource_ptr<VulkanRenderTarget>::cast(&mResourceManager, dst);
    auto srcTarget = resource_ptr<VulkanRenderTarget>::cast(&mResourceManager, src);

    VkFilter const vkfilter = (filter == SamplerMagFilter::NEAREST) ?
            VK_FILTER_NEAREST : VK_FILTER_LINEAR;

    // The Y inversion below makes it so that Vk matches GL and Metal.

    VkExtent2D const srcExtent = srcTarget->getExtent();
    VkExtent2D const dstExtent = dstTarget->getExtent();

    auto const dstLeft   = int32_t(dstRect.left);
    auto const srcLeft   = int32_t(srcRect.left);
    auto const dstTop    = int32_t(dstExtent.height - (dstRect.bottom + dstRect.height));
    auto const srcTop    = int32_t(srcExtent.height - (srcRect.bottom + srcRect.height));
    auto const dstRight  = int32_t(dstRect.left + dstRect.width);
    auto const srcRight  = int32_t(srcRect.left + srcRect.width);
    auto const dstBottom = int32_t(dstTop + dstRect.height);
    auto const srcBottom = int32_t(srcTop + srcRect.height);
    VkOffset3D const srcOffsets[2] = { { srcLeft, srcTop, 0 }, { srcRight, srcBottom, 1 }};
    VkOffset3D const dstOffsets[2] = { { dstLeft, dstTop, 0 }, { dstRight, dstBottom, 1 }};

    mBlitter.blit(vkfilter,
            dstTarget->getColor0(), dstOffsets,
            srcTarget->getColor0(), srcOffsets);
}

void VulkanDriver::bindPipeline(PipelineState const& pipelineState) {
    FVK_SYSTRACE_SCOPE();
    auto commands = mCurrentRenderPass.commandBuffer;
    auto vbi = resource_ptr<VulkanVertexBufferInfo>::cast(&mResourceManager,
            pipelineState.vertexBufferInfo);

    Handle<HwProgram> programHandle = pipelineState.program;
    RasterState const& rasterState = pipelineState.rasterState;
    PolygonOffset const& depthOffset = pipelineState.polygonOffset;

    auto program = resource_ptr<VulkanProgram>::cast(&mResourceManager, programHandle);
    commands->acquire(program);

    // Update the VK raster state.
    auto rt = mCurrentRenderPass.renderTarget;

    VulkanPipelineCache::RasterState const vulkanRasterState{
        .cullMode = getCullMode(rasterState.culling),
        .frontFace = getFrontFace(rasterState.inverseFrontFaces),
        .depthBiasEnable = (depthOffset.constant || depthOffset.slope) ? true : false,
        .blendEnable = rasterState.hasBlending(),
        .depthWriteEnable = rasterState.depthWrite,
        .alphaToCoverageEnable = rasterState.alphaToCoverage,
        .srcColorBlendFactor = getBlendFactor(rasterState.blendFunctionSrcRGB),
        .dstColorBlendFactor = getBlendFactor(rasterState.blendFunctionDstRGB),
        .srcAlphaBlendFactor = getBlendFactor(rasterState.blendFunctionSrcAlpha),
        .dstAlphaBlendFactor = getBlendFactor(rasterState.blendFunctionDstAlpha),
        .colorWriteMask = (VkColorComponentFlags) (rasterState.colorWrite ? 0xf : 0x0),
        .rasterizationSamples = rt->getSamples(),
        .depthClamp = rasterState.depthClamp,
        .colorTargetCount = rt->getColorTargetCount(mCurrentRenderPass),
        .colorBlendOp = rasterState.blendEquationRGB,
        .alphaBlendOp =  rasterState.blendEquationAlpha,
        .depthCompareOp = rasterState.depthFunc,
        .depthBiasConstantFactor = depthOffset.constant,
        .depthBiasSlopeFactor = depthOffset.slope
    };

    // unfortunately in Vulkan the topology is per pipeline
    VkPrimitiveTopology const topology =
            VulkanPipelineCache::getPrimitiveTopology(pipelineState.primitiveType);

    // Declare fixed-size arrays that get passed to the pipeCache and to vkCmdBindVertexBuffers.
    VkVertexInputAttributeDescription const* attribDesc = vbi->getAttribDescriptions();
    VkVertexInputBindingDescription const* bufferDesc =  vbi->getBufferDescriptions();

    // Push state changes to the VulkanPipelineCache instance. This is fast and does not make VK calls.
    mPipelineCache.bindProgram(program);
    mPipelineCache.bindRasterState(vulkanRasterState);
    mPipelineCache.bindPrimitiveTopology(topology);
    mPipelineCache.bindVertexArray(attribDesc, bufferDesc, vbi->getAttributeCount());

    auto& setLayouts = pipelineState.pipelineLayout.setLayout;
    VulkanDescriptorSetLayout::DescriptorSetLayoutArray layoutList;
    uint8_t layoutCount = 0;
    std::transform(setLayouts.begin(), setLayouts.end(), layoutList.begin(),
            [&](Handle<HwDescriptorSetLayout> handle) -> VkDescriptorSetLayout {
                if (!handle) {
                    return VK_NULL_HANDLE;
                }
                auto layout =
                        resource_ptr<VulkanDescriptorSetLayout>::cast(&mResourceManager, handle);
                layoutCount++;
                return layout->getVkLayout();
            });
    auto pipelineLayout = mPipelineLayoutCache.getLayout(layoutList, program);

    constexpr uint8_t descriptorSetMaskTable[4] = {0x1, 0x3, 0x7, 0xF};

    mBoundPipeline = {
        .program = program,
        .pipelineLayout = pipelineLayout,
        .descriptorSetMask = DescriptorSetMask(descriptorSetMaskTable[layoutCount]),
    };

    mPipelineCache.bindLayout(pipelineLayout);
    mPipelineCache.bindPipeline(mCurrentRenderPass.commandBuffer);
}

void VulkanDriver::bindRenderPrimitive(Handle<HwRenderPrimitive> rph) {
    FVK_SYSTRACE_SCOPE();

    VulkanCommandBuffer* commands = mCurrentRenderPass.commandBuffer;
    VkCommandBuffer cmdbuffer = commands->buffer();
    auto prim = resource_ptr<VulkanRenderPrimitive>::cast(&mResourceManager, rph);
    commands->acquire(prim);

    // This *must* match the VulkanVertexBufferInfo that was bound in bindPipeline(). But we want
    // to allow to call this before bindPipeline(), so the validation can only happen in draw()
    auto vbi = prim->vertexBuffer->vbi;

    uint32_t const bufferCount = vbi->getAttributeCount();
    VkDeviceSize const* offsets = vbi->getOffsets();
    VkBuffer const* buffers = prim->vertexBuffer->getVkBuffers();

    // Next bind the vertex buffers and index buffer. One potential performance improvement is to
    // avoid rebinding these if they are already bound, but since we do not (yet) support subranges
    // it would be rare for a client to make consecutive draw calls with the same render primitive.
    vkCmdBindVertexBuffers(cmdbuffer, 0, bufferCount, buffers, offsets);
    vkCmdBindIndexBuffer(cmdbuffer, prim->indexBuffer->buffer.getGpuBuffer(), 0,
            prim->indexBuffer->indexType);
}

void VulkanDriver::bindDescriptorSet(
        backend::DescriptorSetHandle dsh,
        backend::descriptor_set_t setIndex,
        backend::DescriptorSetOffsetArray&& offsets) {
    if (dsh) {
        auto set = resource_ptr<VulkanDescriptorSet>::cast(&mResourceManager, dsh);
        mDescriptorSetManager.bind(setIndex, set, std::move(offsets));
    } else {
        mDescriptorSetManager.unbind(setIndex);
    }
}

void VulkanDriver::draw2(uint32_t indexOffset, uint32_t indexCount, uint32_t instanceCount) {
    FVK_SYSTRACE_SCOPE();
    VkCommandBuffer cmdbuffer = mCurrentRenderPass.commandBuffer->buffer();

    mDescriptorSetManager.commit(mCurrentRenderPass.commandBuffer,
            mBoundPipeline.pipelineLayout,
            mBoundPipeline.descriptorSetMask);

    // Finally, make the actual draw call. TODO: support subranges
    const uint32_t firstIndex = indexOffset;
    const int32_t vertexOffset = 0;
    const uint32_t firstInstId = 0;

    vkCmdDrawIndexed(cmdbuffer, indexCount, instanceCount, firstIndex, vertexOffset, firstInstId);
}

void VulkanDriver::draw(PipelineState state, Handle<HwRenderPrimitive> rph,
        uint32_t const indexOffset, uint32_t const indexCount, uint32_t const instanceCount) {
    auto rp = resource_ptr<VulkanRenderPrimitive>::cast(&mResourceManager, rph);
    state.primitiveType = rp->type;
    state.vertexBufferInfo = Handle<HwVertexBufferInfo>(rp->vertexBuffer->vbi.id());
    bindPipeline(state);
    bindRenderPrimitive(rph);
    draw2(indexOffset, indexCount, instanceCount);
}

void VulkanDriver::dispatchCompute(Handle<HwProgram> program, math::uint3 workGroupCount) {
    // FIXME: implement me
}

void VulkanDriver::scissor(Viewport scissorBox) {
    VkCommandBuffer cmdbuffer = mCurrentRenderPass.commandBuffer->buffer();

    // TODO: it's a common case that scissor() is called with (0, 0, maxint, maxint)
    //       we should maybe have a fast path for this and avoid vkCmdSetScissor() if possible

    // Set scissoring.
    // clamp left-bottom to 0,0 and avoid overflows
    constexpr int32_t maxvali  = std::numeric_limits<int32_t>::max();
    constexpr uint32_t maxvalu  = std::numeric_limits<int32_t>::max();
    int32_t l = scissorBox.left;
    int32_t b = scissorBox.bottom;
    uint32_t w = std::min(maxvalu, scissorBox.width);
    uint32_t h = std::min(maxvalu, scissorBox.height);
    int32_t r = (l > int32_t(maxvalu - w)) ? maxvali : l + int32_t(w);
    int32_t t = (b > int32_t(maxvalu - h)) ? maxvali : b + int32_t(h);
    l = std::max(0, l);
    b = std::max(0, b);
    assert_invariant(r >= l && t >= b);
    VkRect2D scissor{
            .offset = { l, b },
            .extent = { uint32_t(r - l), uint32_t(t - b) }
    };

    auto rt = mCurrentRenderPass.renderTarget;
    rt->transformClientRectToPlatform(&scissor);
    vkCmdSetScissor(cmdbuffer, 0, 1, &scissor);
}

void VulkanDriver::beginTimerQuery(Handle<HwTimerQuery> tqh) {
    auto vtq = resource_ptr<VulkanTimerQuery>::cast(&mResourceManager, tqh);
    mTimestamps->beginQuery(&(mCommands.get()), vtq);
}

void VulkanDriver::endTimerQuery(Handle<HwTimerQuery> tqh) {
    auto vtq = resource_ptr<VulkanTimerQuery>::cast(&mResourceManager, tqh);
     mTimestamps->endQuery(&(mCommands.get()), vtq);
}

void VulkanDriver::debugCommandBegin(CommandStream* cmds, bool synchronous, const char* methodName) noexcept {
    DriverBase::debugCommandBegin(cmds, synchronous, methodName);
#ifndef NDEBUG
    static const std::set<std::string_view> OUTSIDE_COMMANDS = {
        "loadUniformBuffer",
        "updateBufferObject",
        "updateIndexBuffer",
        "update3DImage",
    };
    static const std::string_view BEGIN_COMMAND = "beginRenderPass";
    static const std::string_view END_COMMAND = "endRenderPass";
    static bool inRenderPass = false; // for debug only
    const std::string_view command{ methodName };
    if (command == BEGIN_COMMAND) {
        assert_invariant(!inRenderPass);
        inRenderPass = true;
    } else if (command == END_COMMAND) {
        assert_invariant(inRenderPass);
        inRenderPass = false;
    } else if (inRenderPass && OUTSIDE_COMMANDS.find(command) != OUTSIDE_COMMANDS.end()) {
        FVK_LOGE << command.data() << " issued inside a render pass." << utils::io::endl;
    }
#endif
}

void VulkanDriver::resetState(int) {
}

void VulkanDriver::setDebugTag(HandleBase::HandleId handleId, utils::CString tag) {
    mResourceManager.associateHandle(handleId, std::move(tag));
}

// explicit instantiation of the Dispatcher
template class ConcreteDispatcher<VulkanDriver>;

} // namespace filament::backend

#pragma clang diagnostic pop
