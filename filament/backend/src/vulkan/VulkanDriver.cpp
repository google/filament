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

#include "vulkan/VulkanDriver.h"

#include "CommandStreamDispatcher.h"
#include "DataReshaper.h"
#include "VulkanBuffer.h"
#include "VulkanDriverFactory.h"
#include "VulkanHandles.h"
#include "VulkanPlatform.h"

#include <utils/Panic.h>
#include <utils/CString.h>
#include <utils/trap.h>

#ifndef NDEBUG
#include <set>
#endif

// Vulkan functions often immediately dereference pointers, so it's fine to pass in a pointer
// to a stack-allocated variable.
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wreturn-stack-address"
#pragma clang diagnostic ignored "-Wunused-parameter"

static constexpr int SWAP_CHAIN_MAX_ATTEMPTS = 16;

#if VK_ENABLE_VALIDATION

namespace {

VKAPI_ATTR VkBool32 VKAPI_CALL debugReportCallback(VkDebugReportFlagsEXT flags,
        VkDebugReportObjectTypeEXT objectType, uint64_t object, size_t location,
        int32_t messageCode, const char* pLayerPrefix, const char* pMessage, void* pUserData) {
    if (flags & VK_DEBUG_REPORT_ERROR_BIT_EXT) {
        utils::slog.e << "VULKAN ERROR: (" << pLayerPrefix << ") " << pMessage << utils::io::endl;
        utils::debug_trap();
    } else {
        utils::slog.w << "VULKAN WARNING: (" << pLayerPrefix << ") "
                << pMessage << utils::io::endl;
    }
    // Return TRUE here if an abort is desired.
    return VK_FALSE;
}

VKAPI_ATTR VkBool32 VKAPI_CALL debugUtilsCallback(VkDebugUtilsMessageSeverityFlagBitsEXT severity,
        VkDebugUtilsMessageTypeFlagsEXT types, const VkDebugUtilsMessengerCallbackDataEXT* cbdata,
        void* pUserData) {
    if (severity & VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT) {
        utils::slog.e << "VULKAN ERROR: (" << cbdata->pMessageIdName << ") "
                << cbdata->pMessage << utils::io::endl;
        utils::debug_trap();
    } else {
        utils::slog.w << "VULKAN WARNING: (" << cbdata->pMessageIdName << ") "
                << cbdata->pMessage << utils::io::endl;
    }
    // Return TRUE here if an abort is desired.
    return VK_FALSE;
}

}

#endif

namespace filament {
namespace backend {

Driver* VulkanDriverFactory::create(VulkanPlatform* const platform,
        const char* const* ppEnabledExtensions, uint32_t enabledExtensionCount) noexcept {
    return VulkanDriver::create(platform, ppEnabledExtensions, enabledExtensionCount);
}

VulkanDriver::VulkanDriver(VulkanPlatform* platform,
        const char* const* ppEnabledExtensions, uint32_t enabledExtensionCount) noexcept :
        DriverBase(new ConcreteDispatcher<VulkanDriver>()),
        mContextManager(*platform), mStagePool(mContext, mDisposer), mFramebufferCache(mContext),
        mSamplerCache(mContext) {
    mContext.rasterState = mBinder.getDefaultRasterState();

    // Load Vulkan entry points.
    ASSERT_POSTCONDITION(bluevk::initialize(), "BlueVK is unable to load entry points.");

    VkInstanceCreateInfo instanceCreateInfo = {};
#if VK_ENABLE_VALIDATION
    const utils::StaticString DESIRED_LAYERS[] = {
#if defined(ANDROID)
        // TODO: use VK_LAYER_KHRONOS_validation instead of these layers after it becomes available
        "VK_LAYER_GOOGLE_threading",
        "VK_LAYER_LUNARG_parameter_validation",
        "VK_LAYER_LUNARG_object_tracker",
        "VK_LAYER_LUNARG_core_validation",
        "VK_LAYER_GOOGLE_unique_objects"
#else
        "VK_LAYER_KHRONOS_validation",
#endif
#if defined(ENABLE_RENDERDOC)
        "VK_LAYER_RENDERDOC_Capture",
#endif
    };

    uint32_t layerCount;
    vkEnumerateInstanceLayerProperties(&layerCount, nullptr);
    std::vector<VkLayerProperties> availableLayers(layerCount);
    vkEnumerateInstanceLayerProperties(&layerCount, availableLayers.data());
    std::vector<const char*> enabledLayers;
    for (const auto& desired : DESIRED_LAYERS) {
        for (const VkLayerProperties& layer : availableLayers) {
            const utils::CString availableLayer(layer.layerName);
            if (availableLayer == desired) {
                enabledLayers.push_back(desired.c_str());
            }
        }
    }

    if (!enabledLayers.empty()) {
        instanceCreateInfo.enabledLayerCount = (uint32_t) enabledLayers.size();
        instanceCreateInfo.ppEnabledLayerNames = enabledLayers.data();
    } else {
#if defined(ANDROID)
        utils::slog.d << "Validation layers are not available; did you set jniLibs in your "
                << "gradle file?" << utils::io::endl;
#else
        utils::slog.d << "Validation layer not available; did you install the Vulkan SDK?\n"
                << "Please ensure that VK_LAYER_PATH is set correctly." << utils::io::endl;
#endif
    }
#endif // VK_ENABLE_VALIDATION

    // Create the Vulkan instance.
    VkApplicationInfo appInfo = {};
    appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
    appInfo.apiVersion = VK_MAKE_VERSION(VK_REQUIRED_VERSION_MAJOR, VK_REQUIRED_VERSION_MINOR, 0);
    instanceCreateInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
    instanceCreateInfo.pApplicationInfo = &appInfo;
    instanceCreateInfo.enabledExtensionCount = enabledExtensionCount;
    instanceCreateInfo.ppEnabledExtensionNames = ppEnabledExtensions;
    VkResult result = vkCreateInstance(&instanceCreateInfo, VKALLOC, &mContext.instance);
    ASSERT_POSTCONDITION(result == VK_SUCCESS, "Unable to create Vulkan instance.");
    bluevk::bindInstance(mContext.instance);
    UTILS_UNUSED const PFN_vkCreateDebugReportCallbackEXT createDebugReportCallback =
            vkCreateDebugReportCallbackEXT;

#if VK_ENABLE_VALIDATION

    // We require the VK_EXT_debug_utils instance extension on all non-Android platforms when
    // validation is enabled.
    #ifndef ANDROID
    mContext.debugUtilsSupported = true;
    #endif

    if (mContext.debugUtilsSupported) {
        VkDebugUtilsMessengerCreateInfoEXT createInfo = {
            .sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT,
            .pNext = nullptr,
            .flags = 0,
            .messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT,
            .messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT,
            .pfnUserCallback = debugUtilsCallback
        };
        result = vkCreateDebugUtilsMessengerEXT(mContext.instance, &createInfo, VKALLOC, &mDebugMessenger);
        ASSERT_POSTCONDITION(result == VK_SUCCESS, "Unable to create Vulkan debug messenger.");
    } else if (createDebugReportCallback) {
        const VkDebugReportCallbackCreateInfoEXT cbinfo = {
            VK_STRUCTURE_TYPE_DEBUG_REPORT_CALLBACK_CREATE_INFO_EXT,
            nullptr,
            VK_DEBUG_REPORT_WARNING_BIT_EXT | VK_DEBUG_REPORT_ERROR_BIT_EXT,
            debugReportCallback,
            nullptr
        };
        result = createDebugReportCallback(mContext.instance, &cbinfo, VKALLOC, &mDebugCallback);
        ASSERT_POSTCONDITION(result == VK_SUCCESS, "Unable to create Vulkan debug callback.");
    }
#endif

    // Initialize the following fields: physicalDevice, physicalDeviceProperties,
    // physicalDeviceFeatures, graphicsQueueFamilyIndex.
    selectPhysicalDevice(mContext);

    // Initialize device and graphicsQueue.
    createLogicalDevice(mContext);
    mBinder.setDevice(mContext.device);

    // Choose a depth format that meets our requirements. Take care not to include stencil formats
    // just yet, since that would require a corollary change to the "aspect" flags for the VkImage.
    mContext.finalDepthFormat = findSupportedFormat(mContext,
        { VK_FORMAT_D32_SFLOAT, VK_FORMAT_X8_D24_UNORM_PACK32 },
        VK_IMAGE_TILING_OPTIMAL, VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT);

    // For diagnostic purposes, print useful information about available depth formats.
    // Note that Vulkan is more constrained than OpenGL ES 3.1 in this area.
#if VK_ENABLE_VALIDATION
    const VkFormatFeatureFlags required = VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT |
            VK_FORMAT_FEATURE_SAMPLED_IMAGE_BIT;
    utils::slog.i << "Sampleable depth formats: ";
    for (VkFormat format = (VkFormat) 1;;) {
        VkFormatProperties props;
        vkGetPhysicalDeviceFormatProperties(mContext.physicalDevice, format, &props);
        if ((props.optimalTilingFeatures & required) == required) {
            utils::slog.i << format << " ";
        }
        if (format == VK_FORMAT_ASTC_12x12_SRGB_BLOCK) {
            utils::slog.i << utils::io::endl;
            break;
        }
        format = (VkFormat) (1 + (int) format);
    }
#endif
}

VulkanDriver::~VulkanDriver() noexcept = default;

UTILS_NOINLINE
Driver* VulkanDriver::create(VulkanPlatform* const platform,
        const char* const* ppEnabledExtensions, uint32_t enabledExtensionCount) noexcept {
    assert(platform);
    return new VulkanDriver(platform, ppEnabledExtensions, enabledExtensionCount);
}

ShaderModel VulkanDriver::getShaderModel() const noexcept {
#if defined(ANDROID) || defined(IOS)
    return ShaderModel::GL_ES_30;
#else
    return ShaderModel::GL_CORE_41;
#endif
}

void VulkanDriver::terminate() {
    if (!mContext.instance) {
        return;
    }

    // Flush the work command buffer.
    acquireWorkCommandBuffer(mContext);
    mDisposer.release(mContext.work.resources);

    // Allow the stage pool and disposer to clean up.
    mStagePool.gc();
    mDisposer.reset();

    // Destroy the work command buffer and fence.
    VulkanCommandBuffer& work = mContext.work;
    VkDevice device = mContext.device;
    vkFreeCommandBuffers(device, mContext.commandPool, 1, &work.cmdbuffer);
    work.fence.reset();

    mStagePool.reset();
    mBinder.destroyCache();
    mFramebufferCache.reset();
    mSamplerCache.reset();

    vmaDestroyAllocator(mContext.allocator);
    vkDestroyQueryPool(mContext.device, mContext.timestamps.pool, VKALLOC);
    vkDestroyCommandPool(mContext.device, mContext.commandPool, VKALLOC);
    vkDestroyDevice(mContext.device, VKALLOC);
    if (mDebugCallback) {
        vkDestroyDebugReportCallbackEXT(mContext.instance, mDebugCallback, VKALLOC);
    }
    if (mDebugMessenger) {
        vkDestroyDebugUtilsMessengerEXT(mContext.instance, mDebugMessenger, VKALLOC);
    }
    vkDestroyInstance(mContext.instance, VKALLOC);
    mContext.device = nullptr;
    mContext.instance = nullptr;
}

void VulkanDriver::tick(int) {
    if (!mContext.currentSurface) {
        return;
    }
    for (SwapContext& sc : mContext.currentSurface->swapContexts) {
        VulkanCmdFence* fence = sc.commands.fence.get();
        if (fence) {
            VkResult status = vkGetFenceStatus(mContext.device, fence->fence);
            fence->status.store(status, std::memory_order_relaxed);
        }
    }
}

void VulkanDriver::beginFrame(int64_t monotonic_clock_ns, uint32_t frameId,
        backend::FrameFinishedCallback, void*) {
    // We allow multiple beginFrame / endFrame pairs before commit(), so gracefully return early
    // if the swap chain has already been acquired.
    if (mContext.currentCommands) {
        return;
    }

    // After each command buffer acquisition, we know that the previous submission of the acquired
    // command buffer has finished, so we can decrement the refcount for each of its referenced
    // resources.

    acquireWorkCommandBuffer(mContext);
    mDisposer.release(mContext.work.resources);

    // With MoltenVK, it might take several attempts to acquire a swap chain that is not marked as
    // "out of date" after a resize event.
    int attempts = 0;
    while (!acquireSwapCommandBuffer(mContext)) {
        refreshSwapChain();
        if (attempts++ > SWAP_CHAIN_MAX_ATTEMPTS) {
            PANIC_POSTCONDITION("Unable to acquire image from swap chain.");
        }
    }

    #ifdef ANDROID
    // Polling VkSurfaceCapabilitiesKHR is the most reliable way to detect a rotation change on
    // Android. Checking for VK_SUBOPTIMAL_KHR is not sufficient on pre-Android 10 devices. Even
    // on Android 10, we cannot rely on SUBOPTIMAL because we always use IDENTITY for the
    // preTransform field in VkSwapchainCreateInfoKHR (see other comment in createSwapChain).
    //
    // NOTE: we support apps that have "orientation|screenSize" enabled in android:configChanges.
    //
    // NOTE: we poll the currentExtent rather than currentTransform. The transform seems to change
    // before the extent (on a Pixel 4 anyway), which causes us to create a badly sized VkSwapChain.
    const VulkanSurfaceContext* surface = mContext.currentSurface;
    VkSurfaceCapabilitiesKHR caps;
    vkGetPhysicalDeviceSurfaceCapabilitiesKHR(mContext.physicalDevice, surface->surface, &caps);
    const VkExtent2D previous = surface->surfaceCapabilities.currentExtent;
    const VkExtent2D current = caps.currentExtent;
    if (current.width != previous.width || current.height != previous.height) {
        refreshSwapChain();
        acquireSwapCommandBuffer(mContext);
    }
    #endif

    mDisposer.release(mContext.currentCommands->resources);

    // vkCmdBindPipeline and vkCmdBindDescriptorSets establish bindings to a specific command
    // buffer; they are not global to the device. Since VulkanBinder doesn't have context about the
    // current command buffer, we need to reset its bindings after swapping over to a new command
    // buffer. Note that the following reset causes us to issue a few more vkBind* calls than
    // strictly necessary, but only in the first draw call of the frame. Alteneratively we could
    // enhance VulkanBinder by adding a mapping from command buffers to bindings, but this would
    // introduce complexity that doesn't seem worthwhile. Yet another design would be to instance a
    // separate VulkanBinder for each element in the swap chain, which would also have the benefit
    // of allowing us to safely mutate descriptor sets. For now we're avoiding that strategy in the
    // interest of maintaining a small memory footprint.
    mBinder.resetBindings();

    // Free old unused objects.
    mStagePool.gc();
    mFramebufferCache.gc();
    mBinder.gc();
    mDisposer.gc();
}

void VulkanDriver::setPresentationTime(int64_t monotonic_clock_ns) {
}

void VulkanDriver::endFrame(uint32_t frameId) {
    // Do nothing here; see commit().
}

void VulkanDriver::flush(int) {
    // Todo: equivalent of glFlush()
}

void VulkanDriver::finish(int) {
    // Todo: equivalent of glFinish()
}

void VulkanDriver::createSamplerGroupR(Handle<HwSamplerGroup> sbh, size_t count) {
    construct_handle<VulkanSamplerGroup>(mHandleMap, sbh, mContext, count);
}

void VulkanDriver::createUniformBufferR(Handle<HwUniformBuffer> ubh, size_t size,
        BufferUsage usage) {
    auto uniformBuffer = construct_handle<VulkanUniformBuffer>(mHandleMap, ubh, mContext,
            mStagePool, mDisposer, size, usage);
    mDisposer.createDisposable(uniformBuffer, [this, ubh] () {
        destruct_handle<VulkanUniformBuffer>(mHandleMap, ubh);
    });
}

void VulkanDriver::destroyUniformBuffer(Handle<HwUniformBuffer> ubh) {
    if (ubh) {
        auto buffer = handle_cast<VulkanUniformBuffer>(mHandleMap, ubh);
        mBinder.unbindUniformBuffer(buffer->getGpuBuffer());
        mDisposer.removeReference(buffer);
    }
}

void VulkanDriver::createRenderPrimitiveR(Handle<HwRenderPrimitive> rph, int) {
    construct_handle<VulkanRenderPrimitive>(mHandleMap, rph, mContext);
}

void VulkanDriver::destroyRenderPrimitive(Handle<HwRenderPrimitive> rph) {
    if (rph) {
        destruct_handle<VulkanRenderPrimitive>(mHandleMap, rph);
    }
}

void VulkanDriver::createVertexBufferR(Handle<HwVertexBuffer> vbh, uint8_t bufferCount,
        uint8_t attributeCount, uint32_t elementCount, AttributeArray attributes,
        BufferUsage usage) {
    auto vertexBuffer = construct_handle<VulkanVertexBuffer>(mHandleMap, vbh, mContext, mStagePool,
            mDisposer, bufferCount, attributeCount, elementCount, attributes);
    mDisposer.createDisposable(vertexBuffer, [this, vbh] () {
        destruct_handle<VulkanVertexBuffer>(mHandleMap, vbh);
    });
}

void VulkanDriver::destroyVertexBuffer(Handle<HwVertexBuffer> vbh) {
    if (vbh) {
        auto vertexBuffer = handle_cast<VulkanVertexBuffer>(mHandleMap, vbh);
        mDisposer.removeReference(vertexBuffer);
    }
}

void VulkanDriver::createIndexBufferR(Handle<HwIndexBuffer> ibh,
        ElementType elementType, uint32_t indexCount, BufferUsage usage) {
    auto elementSize = (uint8_t) getElementTypeSize(elementType);
    auto indexBuffer = construct_handle<VulkanIndexBuffer>(mHandleMap, ibh, mContext, mStagePool,
            mDisposer, elementSize, indexCount);
    mDisposer.createDisposable(indexBuffer, [this, ibh] () {
        destruct_handle<VulkanIndexBuffer>(mHandleMap, ibh);
    });
}

void VulkanDriver::destroyIndexBuffer(Handle<HwIndexBuffer> ibh) {
    if (ibh) {
        auto indexBuffer = handle_cast<VulkanIndexBuffer>(mHandleMap, ibh);
        mDisposer.removeReference(indexBuffer);
    }
}

void VulkanDriver::createTextureR(Handle<HwTexture> th, SamplerType target, uint8_t levels,
        TextureFormat format, uint8_t samples, uint32_t w, uint32_t h, uint32_t depth,
        TextureUsage usage) {
    auto vktexture = construct_handle<VulkanTexture>(mHandleMap, th, mContext, target, levels,
            format, samples, w, h, depth, usage, mStagePool);
    mDisposer.createDisposable(vktexture, [this, th] () {
        destruct_handle<VulkanTexture>(mHandleMap, th);
    });
}

void VulkanDriver::createTextureSwizzledR(Handle<HwTexture> th, SamplerType target, uint8_t levels,
        TextureFormat format, uint8_t samples, uint32_t w, uint32_t h, uint32_t depth,
        TextureUsage usage,
        TextureSwizzle r, TextureSwizzle g, TextureSwizzle b, TextureSwizzle a) {
    auto vktexture = construct_handle<VulkanTexture>(mHandleMap, th, mContext, target, levels,
            format, samples, w, h, depth, usage, mStagePool);
    mDisposer.createDisposable(vktexture, [this, th] () {
        destruct_handle<VulkanTexture>(mHandleMap, th);
    });
    // TODO: implement texture swizzling
}

void VulkanDriver::importTextureR(Handle<HwTexture> th, intptr_t id,
        SamplerType target, uint8_t levels,
        TextureFormat format, uint8_t samples, uint32_t w, uint32_t h, uint32_t depth,
        TextureUsage usage) {
    // not support in this backend
}

void VulkanDriver::destroyTexture(Handle<HwTexture> th) {
    if (th) {
        auto texture = handle_cast<VulkanTexture>(mHandleMap, th);
        mBinder.unbindImageView(texture->imageView);
        mDisposer.removeReference(texture);
    }
}

void VulkanDriver::createProgramR(Handle<HwProgram> ph, Program&& program) {
    auto vkprogram = construct_handle<VulkanProgram>(mHandleMap, ph, mContext, program);
    mDisposer.createDisposable(vkprogram, [this, ph] () {
        destruct_handle<VulkanProgram>(mHandleMap, ph);
    });
}

void VulkanDriver::destroyProgram(Handle<HwProgram> ph) {
    if (ph) {
        mDisposer.removeReference(handle_cast<VulkanProgram>(mHandleMap, ph));
    }
}

void VulkanDriver::createDefaultRenderTargetR(Handle<HwRenderTarget> rth, int) {
    auto renderTarget = construct_handle<VulkanRenderTarget>(mHandleMap, rth, mContext);
    mDisposer.createDisposable(renderTarget, [this, rth] () {
        destruct_handle<VulkanRenderTarget>(mHandleMap, rth);
    });
}

void VulkanDriver::createRenderTargetR(Handle<HwRenderTarget> rth,
        TargetBufferFlags targets, uint32_t width, uint32_t height, uint8_t samples,
        backend::MRT color, TargetBufferInfo depth, TargetBufferInfo stencil) {
    VulkanAttachment colorTargets[MRT::TARGET_COUNT] = {};
    for (int i = 0; i < MRT::TARGET_COUNT; i++) {
        if (color[i].handle) {
            colorTargets[i].texture = handle_cast<VulkanTexture>(mHandleMap, color[i].handle);
        }
        colorTargets[i].level = color[i].level;
        colorTargets[i].layer = color[i].layer;
    }

    VulkanAttachment depthStencil[2] = {};
    TextureHandle handle = depth.handle;
    depthStencil[0].texture = handle ? handle_cast<VulkanTexture>(mHandleMap, handle) : nullptr;
    depthStencil[0].level = depth.level;
    depthStencil[0].layer = depth.layer;

    handle = stencil.handle;
    depthStencil[1].texture = handle ? handle_cast<VulkanTexture>(mHandleMap, handle) : nullptr;
    depthStencil[1].level = stencil.level;
    depthStencil[1].layer = stencil.layer;

    auto renderTarget = construct_handle<VulkanRenderTarget>(mHandleMap, rth, mContext,
            width, height, samples, colorTargets, depthStencil, mStagePool);
    mDisposer.createDisposable(renderTarget, [this, rth] () {
        destruct_handle<VulkanRenderTarget>(mHandleMap, rth);
    });
}

void VulkanDriver::destroyRenderTarget(Handle<HwRenderTarget> rth) {
    if (rth) {
        mDisposer.removeReference(handle_cast<VulkanRenderTarget>(mHandleMap, rth));
    }
}

void VulkanDriver::createFenceR(Handle<HwFence> fh, int) {
    // We prefer the fence to be created inside a frame, otherwise there's no command buffer.
    assert(mContext.currentCommands != nullptr && "Fences should be created within a frame.");

     // As a fallback in release builds, trigger the fence based on the work command buffer.
    if (mContext.currentCommands == nullptr) {
        construct_handle<VulkanFence>(mHandleMap, fh, mContext.work);
        return;
    }

     construct_handle<VulkanFence>(mHandleMap, fh, *mContext.currentCommands);
}

void VulkanDriver::createSyncR(Handle<HwSync> sh, int) {
    ASSERT_PRECONDITION(mContext.currentCommands, "Syncs must be created within a frame.");
    construct_handle<VulkanSync>(mHandleMap, sh, *mContext.currentCommands);
}

void VulkanDriver::createSwapChainR(Handle<HwSwapChain> sch, void* nativeWindow, uint64_t flags) {
    const VkInstance instance = mContext.instance;
    auto vksurface = (VkSurfaceKHR) mContextManager.createVkSurfaceKHR(nativeWindow, instance);
    auto* swapChain = construct_handle<VulkanSwapChain>(mHandleMap, sch, mContext, vksurface);

    // TODO: move the following line into makeCurrent.
    mContext.currentSurface = &swapChain->surfaceContext;
}

void VulkanDriver::createSwapChainHeadlessR(Handle<HwSwapChain> sch,
        uint32_t width, uint32_t height, uint64_t flags) {
    assert(width > 0 && height > 0 && "Vulkan requires non-zero swap chain dimensions.");
    auto* swapChain = construct_handle<VulkanSwapChain>(mHandleMap, sch, mContext, width, height);
    mContext.currentSurface = &swapChain->surfaceContext;
}

void VulkanDriver::createStreamFromTextureIdR(Handle<HwStream> sh, intptr_t externalTextureId,
        uint32_t width, uint32_t height) {
}

void VulkanDriver::createTimerQueryR(Handle<HwTimerQuery> tqh, int) {
    // nothing to do, timer query was constructed in createTimerQueryS
}

Handle<HwVertexBuffer> VulkanDriver::createVertexBufferS() noexcept {
    return alloc_handle<VulkanVertexBuffer, HwVertexBuffer>();
}

Handle<HwIndexBuffer> VulkanDriver::createIndexBufferS() noexcept {
    return alloc_handle<VulkanIndexBuffer, HwIndexBuffer>();
}

Handle<HwTexture> VulkanDriver::createTextureS() noexcept {
    return alloc_handle<VulkanTexture, HwTexture>();
}

Handle<HwTexture> VulkanDriver::createTextureSwizzledS() noexcept {
    return alloc_handle<VulkanTexture, HwTexture>();
}

Handle<HwTexture> VulkanDriver::importTextureS() noexcept {
    return alloc_handle<VulkanTexture, HwTexture>();
}

Handle<HwSamplerGroup> VulkanDriver::createSamplerGroupS() noexcept {
    return alloc_handle<VulkanSamplerGroup, HwSamplerGroup>();
}

Handle<HwUniformBuffer> VulkanDriver::createUniformBufferS() noexcept {
    return alloc_handle<VulkanUniformBuffer, HwUniformBuffer>();
}

Handle<HwRenderPrimitive> VulkanDriver::createRenderPrimitiveS() noexcept {
    return alloc_handle<VulkanRenderPrimitive, HwRenderPrimitive>();
}

Handle<HwProgram> VulkanDriver::createProgramS() noexcept {
    return alloc_handle<VulkanProgram, HwProgram>();
}

Handle<HwRenderTarget> VulkanDriver::createDefaultRenderTargetS() noexcept {
    return alloc_handle<VulkanRenderTarget, HwRenderTarget>();
}

Handle<HwRenderTarget> VulkanDriver::createRenderTargetS() noexcept {
    return alloc_handle<VulkanRenderTarget, HwRenderTarget>();
}

Handle<HwFence> VulkanDriver::createFenceS() noexcept {
    return alloc_handle<VulkanFence, HwFence>();
}

Handle<HwSync> VulkanDriver::createSyncS() noexcept {
    return alloc_handle<VulkanSync, HwSync>();
}

Handle<HwSwapChain> VulkanDriver::createSwapChainS() noexcept {
    return alloc_handle<VulkanSwapChain, HwSwapChain>();
}

Handle<HwSwapChain> VulkanDriver::createSwapChainHeadlessS() noexcept {
    return alloc_handle<VulkanSwapChain, HwSwapChain>();
}

Handle<HwStream> VulkanDriver::createStreamFromTextureIdS() noexcept {
    return {};
}

Handle<HwTimerQuery> VulkanDriver::createTimerQueryS() noexcept {
    // The handle must be constructed here, as a synchronous call to getTimerQueryValue might happen
    // before createTimerQueryR is executed.
    Handle<HwTimerQuery> tqh = alloc_handle<VulkanTimerQuery, HwTimerQuery>();
    auto query = construct_handle<VulkanTimerQuery>(mHandleMap, tqh, mContext);
    mDisposer.createDisposable(query, [this, tqh] () {
        destruct_handle<VulkanTimerQuery>(mHandleMap, tqh);
    });
    return tqh;
}

void VulkanDriver::destroySamplerGroup(Handle<HwSamplerGroup> sbh) {
    if (sbh) {
        // Unlike most of the other "Hw" handles, the sampler buffer is an abstract concept and does
        // not map to any Vulkan objects. To handle destruction, the only thing we need to do is
        // ensure that the next draw call doesn't try to access a zombie sampler buffer. Therefore,
        // simply replace all weak references with null.
        auto* hwsb = handle_cast<VulkanSamplerGroup>(mHandleMap, sbh);
        for (auto& binding : mSamplerBindings) {
            if (binding == hwsb) {
                binding = nullptr;
            }
        }
        destruct_handle<VulkanSamplerGroup>(mHandleMap, sbh);
    }
}

void VulkanDriver::destroySwapChain(Handle<HwSwapChain> sch) {
    if (sch) {
        VulkanSurfaceContext& surfaceContext = handle_cast<VulkanSwapChain>(mHandleMap, sch)->surfaceContext;
        backend::destroySwapChain(mContext, surfaceContext, mDisposer);

        vkDestroySurfaceKHR(mContext.instance, surfaceContext.surface, VKALLOC);
        if (mContext.currentSurface == &surfaceContext) {
            mContext.currentSurface = nullptr;
        }

        destruct_handle<VulkanSwapChain>(mHandleMap, sch);
    }
}

void VulkanDriver::destroyStream(Handle<HwStream> sh) {
}

void VulkanDriver::destroyTimerQuery(Handle<HwTimerQuery> tqh) {
    if (tqh) {
        mDisposer.removeReference(handle_cast<VulkanTimerQuery>(mHandleMap, tqh));
    }
}

void VulkanDriver::destroySync(Handle<HwSync> sh) {
    destruct_handle<VulkanSync>(mHandleMap, sh);
}


Handle<HwStream> VulkanDriver::createStreamNative(void* nativeStream) {
    return {};
}

Handle<HwStream> VulkanDriver::createStreamAcquired() {
    return {};
}

void VulkanDriver::setAcquiredImage(Handle<HwStream> sh, void* image, backend::StreamCallback cb,
        void* userData) {
}

void VulkanDriver::setStreamDimensions(Handle<HwStream> sh, uint32_t width, uint32_t height) {
}

int64_t VulkanDriver::getStreamTimestamp(Handle<HwStream> sh) {
    return 0;
}

void VulkanDriver::updateStreams(CommandStream* driver) {
}

void VulkanDriver::destroyFence(Handle<HwFence> fh) {
    destruct_handle<VulkanFence>(mHandleMap, fh);
}

FenceStatus VulkanDriver::wait(Handle<HwFence> fh, uint64_t timeout) {
    auto& cmdfence = handle_cast<VulkanFence>(mHandleMap, fh)->fence;

    // The condition variable is used only to guarantee that we're calling vkWaitForFences *after*
    // calling vkQueueSubmit.
    std::unique_lock<utils::Mutex> lock(cmdfence->mutex);
    if (!cmdfence->submitted) {
        cmdfence->condition.wait(lock);
        assert(cmdfence->submitted);
    } else {
        lock.unlock();
    }
    if (cmdfence->swapChainDestroyed) {
        return FenceStatus::ERROR;
    }
    VkResult result = vkWaitForFences(mContext.device, 1, &cmdfence->fence, VK_FALSE, timeout);
    return result == VK_SUCCESS ? FenceStatus::CONDITION_SATISFIED : FenceStatus::TIMEOUT_EXPIRED;
}

// We create all textures using VK_IMAGE_TILING_OPTIMAL, so our definition of "supported" is that
// the GPU supports the given texture format with non-zero optimal tiling features.
bool VulkanDriver::isTextureFormatSupported(TextureFormat format) {
    assert(mContext.physicalDevice);
    VkFormat vkformat = getVkFormat(format);
    // We automatically use an alternative format when the client requests DEPTH24.
    if (format == TextureFormat::DEPTH24) {
        vkformat = mContext.finalDepthFormat;
    }
    if (vkformat == VK_FORMAT_UNDEFINED) {
        return false;
    }
    VkFormatProperties info;
    vkGetPhysicalDeviceFormatProperties(mContext.physicalDevice, vkformat, &info);
    return info.optimalTilingFeatures != 0;
}

bool VulkanDriver::isTextureFormatMipmappable(backend::TextureFormat format) {
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
    assert(mContext.physicalDevice);
    VkFormat vkformat = getVkFormat(format);
    // We automatically use an alternative format when the client requests DEPTH24.
    if (format == TextureFormat::DEPTH24) {
        vkformat = mContext.finalDepthFormat;
    }
    if (vkformat == VK_FORMAT_UNDEFINED) {
        return false;
    }
    VkFormatProperties info;
    vkGetPhysicalDeviceFormatProperties(mContext.physicalDevice, vkformat, &info);
    return (info.optimalTilingFeatures & VK_FORMAT_FEATURE_COLOR_ATTACHMENT_BIT) != 0;
}

bool VulkanDriver::isFrameBufferFetchSupported() {
    return true;
}

bool VulkanDriver::isFrameTimeSupported() {
    return true;
}

math::float2 VulkanDriver::getClipSpaceParams() {
    // z-coordinate of clip-space is in [0,w]
    return math::float2{ -0.5f, 0.5f };
}

void VulkanDriver::updateVertexBuffer(Handle<HwVertexBuffer> vbh, size_t index,
        BufferDescriptor&& p, uint32_t byteOffset) {
    auto& vb = *handle_cast<VulkanVertexBuffer>(mHandleMap, vbh);
    vb.buffers[index]->loadFromCpu(p.buffer, byteOffset, p.size);
    scheduleDestroy(std::move(p));
}

void VulkanDriver::updateIndexBuffer(Handle<HwIndexBuffer> ibh, BufferDescriptor&& p,
        uint32_t byteOffset) {
    auto& ib = *handle_cast<VulkanIndexBuffer>(mHandleMap, ibh);
    ib.buffer->loadFromCpu(p.buffer, byteOffset, p.size);
    scheduleDestroy(std::move(p));
}

void VulkanDriver::update2DImage(Handle<HwTexture> th,
        uint32_t level, uint32_t xoffset, uint32_t yoffset, uint32_t width, uint32_t height,
        PixelBufferDescriptor&& data) {
    assert(xoffset == 0 && yoffset == 0 && "Offsets not yet supported.");
    handle_cast<VulkanTexture>(mHandleMap, th)->update2DImage(data, width, height, level);
    scheduleDestroy(std::move(data));
}

void VulkanDriver::update3DImage(
        Handle<HwTexture> th,
        uint32_t level, uint32_t xoffset, uint32_t yoffset, uint32_t zoffset,
        uint32_t width, uint32_t height, uint32_t depth,
        PixelBufferDescriptor&& data) {
    assert(xoffset == 0 && yoffset == 0 && zoffset == 0 && "Offsets not yet supported.");
    handle_cast<VulkanTexture>(mHandleMap, th)->update3DImage(data, width, height, depth, level);
    scheduleDestroy(std::move(data));
}

void VulkanDriver::updateCubeImage(Handle<HwTexture> th, uint32_t level,
        PixelBufferDescriptor&& data, FaceOffsets faceOffsets) {
    handle_cast<VulkanTexture>(mHandleMap, th)->updateCubeImage(data, faceOffsets, level);
    scheduleDestroy(std::move(data));
}

void VulkanDriver::setupExternalImage(void* image) {
}

void VulkanDriver::cancelExternalImage(void* image) {
}

bool VulkanDriver::getTimerQueryValue(Handle<HwTimerQuery> tqh, uint64_t* elapsedTime) {
    VulkanTimerQuery* vtq = handle_cast<VulkanTimerQuery>(mHandleMap, tqh);

    // This is a synchronous call and might occur before beginTimerQuery has written anything into
    // the command buffer, which is an error according to the validation layer that ships in the
    // Android NDK.  Even when AVAILABILITY_BIT is set, validation seems to require that the
    // timestamp has at least been written into a processed command buffer.
    VulkanCommandBuffer* cmdbuf = vtq->cmdbuffer.load();
    if (!cmdbuf || !cmdbuf->fence) {
        return false;
    }
    VkResult status = cmdbuf->fence->status.load(std::memory_order_relaxed);
    if (status != VK_SUCCESS) {
        return false;
    }

    uint64_t results[4] = {};
    size_t dataSize = sizeof(results);
    VkDeviceSize stride = sizeof(uint64_t) * 2;

    VkResult result = vkGetQueryPoolResults(mContext.device, mContext.timestamps.pool,
            vtq->startingQueryIndex, 2, dataSize, (void*) results, stride,
            VK_QUERY_RESULT_64_BIT | VK_QUERY_RESULT_WITH_AVAILABILITY_BIT);

    uint64_t timestamp0 = results[0];
    uint64_t available0 = results[1];
    uint64_t timestamp1 = results[2];
    uint64_t available1 = results[3];

    if (result == VK_NOT_READY || available0 == 0 || available1 == 0) {
        return false;
    }

    ASSERT_POSTCONDITION(result == VK_SUCCESS, "vkGetQueryPoolResults error.");
    ASSERT_POSTCONDITION(timestamp1 >= timestamp0, "Timestamps are not monotonically increasing.");

    // NOTE: MoltenVK currently writes system time so the following delta will always be zero.
    // However there are plans for implementing this properly. See the following GitHub ticket.
    // https://github.com/KhronosGroup/MoltenVK/issues/773

    uint64_t delta = timestamp1 - timestamp0;
    *elapsedTime = delta;
    return true;
}

SyncStatus VulkanDriver::getSyncStatus(Handle<HwSync> sh) {
    VulkanSync* sync = handle_cast<VulkanSync>(mHandleMap, sh);
    if (sync->fence == nullptr) {
        return SyncStatus::NOT_SIGNALED;
    }
    if (sync->fence->swapChainDestroyed) {
        return SyncStatus::ERROR;
    }
    VkResult status = sync->fence->status.load(std::memory_order_relaxed);
    switch (status) {
        case VK_SUCCESS: return SyncStatus::SIGNALED;
        case VK_NOT_READY: return SyncStatus::NOT_SIGNALED;
        default: return SyncStatus::ERROR;
    }
}

void VulkanDriver::setExternalImage(Handle<HwTexture> th, void* image) {
}

void VulkanDriver::setExternalImagePlane(Handle<HwTexture> th, void* image, size_t plane) {
}

void VulkanDriver::setExternalStream(Handle<HwTexture> th, Handle<HwStream> sh) {
}

void VulkanDriver::generateMipmaps(Handle<HwTexture> th) { }

bool VulkanDriver::canGenerateMipmaps() {
    return false;
}

void VulkanDriver::loadUniformBuffer(Handle<HwUniformBuffer> ubh, BufferDescriptor&& data) {
    if (data.size > 0) {
        auto* buffer = handle_cast<VulkanUniformBuffer>(mHandleMap, ubh);
        buffer->loadFromCpu(data.buffer, (uint32_t) data.size);
        scheduleDestroy(std::move(data));
    }
}

void VulkanDriver::updateSamplerGroup(Handle<HwSamplerGroup> sbh,
        SamplerGroup&& samplerGroup) {
    auto* sb = handle_cast<VulkanSamplerGroup>(mHandleMap, sbh);
    *sb->sb = samplerGroup;
}

void VulkanDriver::beginRenderPass(Handle<HwRenderTarget> rth, const RenderPassParams& params) {
    assert(mContext.currentCommands);
    assert(mContext.currentSurface);
    VulkanSurfaceContext& surface = *mContext.currentSurface;
    mCurrentRenderTarget = handle_cast<VulkanRenderTarget>(mHandleMap, rth);
    VulkanRenderTarget* rt = mCurrentRenderTarget;

    const VkExtent2D extent = rt->getExtent();
    assert(extent.width > 0 && extent.height > 0);

    const VulkanAttachment depth = rt->getDepth();

    // Filament has the expectation that the contents of the swap chain are not preserved on the
    // first render pass. Note however that its contents are often preserved on subsequent render
    // passes, due to multiple views.
    TargetBufferFlags discardStart = params.flags.discardStart;
    if (rt->invalidate()) {
        discardStart |= TargetBufferFlags::COLOR;
    }

    // Create the VkRenderPass or fetch it from cache.
    VulkanFboCache::RenderPassKey rpkey = {
        .depthLayout = depth.layout,
        .depthFormat = depth.format,
        .clear = params.flags.clear,
        .discardStart = discardStart,
        .discardEnd = params.flags.discardEnd,
        .samples = rt->getSamples(),
        .subpassMask = uint8_t(params.subpassMask)
    };
    for (int i = 0; i < MRT::TARGET_COUNT; i++) {
        rpkey.colorLayout[i] = rt->getColor(i).layout;
        rpkey.colorFormat[i] = rt->getColor(i).format;
        VulkanTexture* texture = rt->getColor(i).texture;
        if (rpkey.samples > 1 && texture && texture->samples == 1) {
            rpkey.needsResolveMask |= (1 << i);
        }
    }

    VkRenderPass renderPass = mFramebufferCache.getRenderPass(rpkey);
    mBinder.bindRenderPass(renderPass, 0);

    // Create the VkFramebuffer or fetch it from cache.
    VulkanFboCache::FboKey fbkey {
        .renderPass = renderPass,
        .width = (uint16_t) extent.width,
        .height = (uint16_t) extent.height,
        .layers = 1,
        .samples = rpkey.samples
    };
    for (int i = 0; i < MRT::TARGET_COUNT; i++) {
        if (rt->getColor(i).format == VK_FORMAT_UNDEFINED) {
            fbkey.color[i] = VK_NULL_HANDLE;
            fbkey.resolve[i] = VK_NULL_HANDLE;
        } else if (fbkey.samples == 1) {
            fbkey.color[i] = rt->getColor(i).view;
            fbkey.resolve[i] = VK_NULL_HANDLE;
            assert(fbkey.color[i]);
        } else {
            fbkey.color[i] = rt->getMsaaColor(i).view;
            VulkanTexture* texture = rt->getColor(i).texture;
            if (texture && texture->samples == 1) {
                fbkey.resolve[i] = rt->getColor(i).view;
                assert(fbkey.resolve[i]);
            }
            assert(fbkey.color[i]);
        }
    }
    if (depth.format != VK_FORMAT_UNDEFINED) {
        fbkey.depth = rpkey.samples == 1 ? depth.view : rt->getMsaaDepth().view;
        assert(fbkey.depth);
    }
    VkFramebuffer vkfb = mFramebufferCache.getFramebuffer(fbkey);

    // The current command buffer now owns a reference to the render target and its attachments.
    mDisposer.acquire(rt, mContext.currentCommands->resources);
    mDisposer.acquire(depth.texture, mContext.currentCommands->resources);
    for (int i = 0; i < MRT::TARGET_COUNT; i++) {
        mDisposer.acquire(rt->getColor(i).texture, mContext.currentCommands->resources);
    }

    // Populate the structures required for vkCmdBeginRenderPass.
    VkRenderPassBeginInfo renderPassInfo {
        .sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO,
        .renderPass = renderPass,
        .framebuffer = vkfb,

        // The renderArea field constrains the LoadOp, but scissoring does not.
        // Therefore we do not set the scissor rect here, we only need it in draw().
        .renderArea = { .offset = {}, .extent = extent }
    };

    rt->transformClientRectToPlatform(&renderPassInfo.renderArea);

    VkClearValue clearValues[MRT::TARGET_COUNT + MRT::TARGET_COUNT + 1] = {};

    // NOTE: clearValues must be populated in the same order as the attachments array in
    // VulkanFboCache::getFramebuffer. Values must be provided regardless of whether Vulkan is
    // actually clearing that particular target.
    for (int i = 0; i < MRT::TARGET_COUNT; i++) {
        if (fbkey.color[i]) {
            VkClearValue& clearValue = clearValues[renderPassInfo.clearValueCount++];
            clearValue.color.float32[0] = params.clearColor.r;
            clearValue.color.float32[1] = params.clearColor.g;
            clearValue.color.float32[2] = params.clearColor.b;
            clearValue.color.float32[3] = params.clearColor.a;
        }
    }
    // Resolve attachments are not cleared but still have entries in the list, so skip over them.
    for (int i = 0; i < MRT::TARGET_COUNT; i++) {
        if (rpkey.needsResolveMask & (1u << i)) {
            renderPassInfo.clearValueCount++;
        }
    }
    if (fbkey.depth) {
        VkClearValue& clearValue = clearValues[renderPassInfo.clearValueCount++];
        clearValue.depthStencil = {(float) params.clearDepth, 0};
    }
    renderPassInfo.pClearValues = &clearValues[0];

    const SwapContext& swapContext = surface.swapContexts[surface.currentSwapIndex];

    vkCmdBeginRenderPass(swapContext.commands.cmdbuffer, &renderPassInfo,
            VK_SUBPASS_CONTENTS_INLINE);

    VkViewport viewport = mContext.viewport = {
        .x = (float) params.viewport.left,
        .y = (float) params.viewport.bottom,
        .width = (float) params.viewport.width,
        .height = (float) params.viewport.height,
        .minDepth = params.depthRange.near,
        .maxDepth = params.depthRange.far
    };

    mCurrentRenderTarget->transformClientRectToPlatform(&viewport);
    vkCmdSetViewport(swapContext.commands.cmdbuffer, 0, 1, &viewport);

    mContext.currentRenderPass = {
        .renderPass = renderPassInfo.renderPass,
        .subpassMask = params.subpassMask,
        .currentSubpass = 0
    };
}

void VulkanDriver::endRenderPass(int) {
    assert(mContext.currentCommands);
    assert(mContext.currentSurface);
    assert(mCurrentRenderTarget);
    vkCmdEndRenderPass(mContext.currentCommands->cmdbuffer);
    mCurrentRenderTarget = VK_NULL_HANDLE;
    if (mContext.currentRenderPass.currentSubpass > 0) {
        for (uint32_t i = 0; i < VulkanBinder::TARGET_BINDING_COUNT; i++) {
            mBinder.bindInputAttachment(i, {});
        }
        mContext.currentRenderPass.currentSubpass = 0;
    }
    mContext.currentRenderPass.renderPass = VK_NULL_HANDLE;
}

void VulkanDriver::nextSubpass(int) {
    ASSERT_PRECONDITION(mContext.currentRenderPass.currentSubpass == 0,
            "Only two subpasses are currently supported.");

    assert(mContext.currentCommands);
    assert(mContext.currentSurface);
    assert(mCurrentRenderTarget);
    assert(mContext.currentRenderPass.subpassMask);

    vkCmdNextSubpass(mContext.currentCommands->cmdbuffer, VK_SUBPASS_CONTENTS_INLINE);

    mBinder.bindRenderPass(mContext.currentRenderPass.renderPass,
            ++mContext.currentRenderPass.currentSubpass);

    for (uint32_t i = 0; i < VulkanBinder::TARGET_BINDING_COUNT; i++) {
        if ((1 << i) & mContext.currentRenderPass.subpassMask) {
            VulkanAttachment subpassInput = mCurrentRenderTarget->getColor(i);
            VkDescriptorImageInfo info = {
                .imageView = subpassInput.view,
                .imageLayout = subpassInput.layout,
            };
            mBinder.bindInputAttachment(i, info);
        }
    }
}

void VulkanDriver::setRenderPrimitiveBuffer(Handle<HwRenderPrimitive> rph,
        Handle<HwVertexBuffer> vbh, Handle<HwIndexBuffer> ibh,
        uint32_t enabledAttributes) {
    auto primitive = handle_cast<VulkanRenderPrimitive>(mHandleMap, rph);
    primitive->setBuffers(handle_cast<VulkanVertexBuffer>(mHandleMap, vbh),
            handle_cast<VulkanIndexBuffer>(mHandleMap, ibh), enabledAttributes);
}

void VulkanDriver::setRenderPrimitiveRange(Handle<HwRenderPrimitive> rph,
        PrimitiveType pt, uint32_t offset,
        uint32_t minIndex, uint32_t maxIndex, uint32_t count) {
    auto& primitive = *handle_cast<VulkanRenderPrimitive>(mHandleMap, rph);
    primitive.setPrimitiveType(pt);
    primitive.offset = offset * primitive.indexBuffer->elementSize;
    primitive.count = count;
    primitive.minIndex = minIndex;
    primitive.maxIndex = maxIndex > minIndex ? maxIndex : primitive.maxVertexCount - 1;
}

void VulkanDriver::makeCurrent(Handle<HwSwapChain> drawSch, Handle<HwSwapChain> readSch) {
    ASSERT_PRECONDITION_NON_FATAL(drawSch == readSch,
                                  "Vulkan driver does not support distinct draw/read swap chains.");
    VulkanSurfaceContext& sContext = handle_cast<VulkanSwapChain>(mHandleMap, drawSch)->surfaceContext;
    mContext.currentSurface = &sContext;
}

void VulkanDriver::commit(Handle<HwSwapChain> sch) {
    // Tell Vulkan we're done appending to the command buffer.
    ASSERT_POSTCONDITION(mContext.currentCommands,
            "Vulkan driver requires at least one frame before a commit.");

    // Before swapping, transition the current swap chain image to the PRESENT layout. This cannot
    // be done as part of the render pass because it does not know if it is last pass in the frame.
    makeSwapChainPresentable(mContext);

    // Finalize the command buffer and set the cmdbuffer pointer to null.
    VkResult result = vkEndCommandBuffer(mContext.currentCommands->cmdbuffer);
    ASSERT_POSTCONDITION(result == VK_SUCCESS, "vkEndCommandBuffer error.");
    mContext.currentCommands = nullptr;

    // Submit the command buffer.
    VkPipelineStageFlags waitDestStageMask = VK_PIPELINE_STAGE_TRANSFER_BIT;
    VulkanSurfaceContext& surfaceContext = *mContext.currentSurface;
    SwapContext& swapContext = getSwapContext(mContext);
    VkSubmitInfo submitInfo {
            .sType = VK_STRUCTURE_TYPE_SUBMIT_INFO,
            .waitSemaphoreCount = 1u,
            .pWaitSemaphores = &surfaceContext.imageAvailable,
            .pWaitDstStageMask = &waitDestStageMask,
            .commandBufferCount = 1,
            .pCommandBuffers = &swapContext.commands.cmdbuffer,
            .signalSemaphoreCount = 1u,
            .pSignalSemaphores = &surfaceContext.renderingFinished,
    };
    if (surfaceContext.headlessQueue) {
        submitInfo.waitSemaphoreCount = 0;
        submitInfo.pWaitSemaphores = nullptr;
        submitInfo.signalSemaphoreCount = 0;
        submitInfo.pSignalSemaphores = nullptr;
    }

    auto& cmdfence = swapContext.commands.fence;
    std::unique_lock<utils::Mutex> lock(cmdfence->mutex);
    result = vkQueueSubmit(mContext.graphicsQueue, 1, &submitInfo, cmdfence->fence);
    cmdfence->submitted = true;
    lock.unlock();
    ASSERT_POSTCONDITION(result == VK_SUCCESS, "vkQueueSubmit error.");
    swapContext.invalid = true;
    cmdfence->condition.notify_all();

    if (surfaceContext.headlessQueue) {
        return;
    }

    // Present the backbuffer.
    VulkanSurfaceContext& surface = handle_cast<VulkanSwapChain>(mHandleMap, sch)->surfaceContext;
    VkPresentInfoKHR presentInfo {
        .sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR,
        .waitSemaphoreCount = 1,
        .pWaitSemaphores = &surface.renderingFinished,
        .swapchainCount = 1,
        .pSwapchains = &surface.swapchain,
        .pImageIndices = &surface.currentSwapIndex,
    };
    result = vkQueuePresentKHR(surface.presentQueue, &presentInfo);

    // On Android Q and above, a suboptimal surface is always reported after screen rotation:
    // https://android-developers.googleblog.com/2020/02/handling-device-orientation-efficiently.html
    if (result == VK_SUBOPTIMAL_KHR && !surface.suboptimal) {
        utils::slog.w << "Vulkan Driver: Suboptimal swap chain." << utils::io::endl;
        surface.suboptimal = true;
    }

    // The surface can be "out of date" when it has been resized, which is not an error.
    assert(result == VK_SUCCESS || result == VK_SUBOPTIMAL_KHR ||
            result == VK_ERROR_OUT_OF_DATE_KHR);
}

void VulkanDriver::bindUniformBuffer(size_t index, Handle<HwUniformBuffer> ubh) {
    auto* buffer = handle_cast<VulkanUniformBuffer>(mHandleMap, ubh);
    // The driver API does not currently expose offset / range, but it will do so in the future.
    const VkDeviceSize offset = 0;
    const VkDeviceSize size = VK_WHOLE_SIZE;
    mBinder.bindUniformBuffer((uint32_t) index, buffer->getGpuBuffer(), offset, size);
}

void VulkanDriver::bindUniformBufferRange(size_t index, Handle<HwUniformBuffer> ubh,
        size_t offset, size_t size) {
    auto* buffer = handle_cast<VulkanUniformBuffer>(mHandleMap, ubh);
    mBinder.bindUniformBuffer((uint32_t)index, buffer->getGpuBuffer(), offset, size);
}

void VulkanDriver::bindSamplers(size_t index, Handle<HwSamplerGroup> sbh) {
    auto* hwsb = handle_cast<VulkanSamplerGroup>(mHandleMap, sbh);
    mSamplerBindings[index] = hwsb;
}

void VulkanDriver::insertEventMarker(char const* string, size_t len) {
    constexpr float MARKER_COLOR[] = { 0.0f, 1.0f, 0.0f, 1.0f };
    ASSERT_POSTCONDITION(mContext.currentCommands,
            "Markers can only be inserted within a beginFrame / endFrame.");
    if (mContext.debugUtilsSupported) {
        VkDebugUtilsLabelEXT labelInfo = {
            .sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_LABEL_EXT,
            .pLabelName = string,
            .color = {1, 1, 0, 1},
        };
        vkCmdInsertDebugUtilsLabelEXT(mContext.currentCommands->cmdbuffer, &labelInfo);
    } else if (mContext.debugMarkersSupported) {
        VkDebugMarkerMarkerInfoEXT markerInfo = {};
        markerInfo.sType = VK_STRUCTURE_TYPE_DEBUG_MARKER_MARKER_INFO_EXT;
        memcpy(markerInfo.color, &MARKER_COLOR[0], sizeof(MARKER_COLOR));
        markerInfo.pMarkerName = string;
        vkCmdDebugMarkerInsertEXT(mContext.currentCommands->cmdbuffer, &markerInfo);
    }
}

void VulkanDriver::pushGroupMarker(char const* string, size_t len) {
    // TODO: Add group marker color to the Driver API
    constexpr float MARKER_COLOR[] = { 0.0f, 1.0f, 0.0f, 1.0f };
    ASSERT_POSTCONDITION(mContext.currentCommands,
            "Markers can only be inserted within a beginFrame / endFrame.");
    if (mContext.debugUtilsSupported) {
        VkDebugUtilsLabelEXT labelInfo = {
            .sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_LABEL_EXT,
            .pLabelName = string,
            .color = {0, 1, 0, 1},
        };
        vkCmdBeginDebugUtilsLabelEXT(mContext.currentCommands->cmdbuffer, &labelInfo);
    } else if (mContext.debugMarkersSupported) {
        VkDebugMarkerMarkerInfoEXT markerInfo = {};
        markerInfo.sType = VK_STRUCTURE_TYPE_DEBUG_MARKER_MARKER_INFO_EXT;
        memcpy(markerInfo.color, &MARKER_COLOR[0], sizeof(MARKER_COLOR));
        markerInfo.pMarkerName = string;
        vkCmdDebugMarkerBeginEXT(mContext.currentCommands->cmdbuffer, &markerInfo);
    }
}

void VulkanDriver::popGroupMarker(int) {
    ASSERT_POSTCONDITION(mContext.currentCommands,
            "Markers can only be inserted within a beginFrame / endFrame.");
    if (mContext.debugUtilsSupported) {
        vkCmdEndDebugUtilsLabelEXT(mContext.currentCommands->cmdbuffer);
    } else if (mContext.debugMarkersSupported) {
        vkCmdDebugMarkerEndEXT(mContext.currentCommands->cmdbuffer);
    }
}

void VulkanDriver::startCapture(int) {

}

void VulkanDriver::stopCapture(int) {

}

void VulkanDriver::readPixels(Handle<HwRenderTarget> src, uint32_t x, uint32_t y,
        uint32_t width, uint32_t height, PixelBufferDescriptor&& pbd) {
    const VkDevice device = mContext.device;
    const VulkanRenderTarget* srcTarget = handle_cast<VulkanRenderTarget>(mHandleMap, src);
    const VulkanTexture* srcTexture = srcTarget->getColor(0).texture;
    const VkFormat swapChainFormat = mContext.currentSurface->surfaceFormat.format;
    const VkFormat srcFormat = srcTexture ? srcTexture->vkformat : swapChainFormat;
    const bool swizzle = srcFormat == VK_FORMAT_B8G8R8A8_UNORM;

    // Create a host visible, linearly tiled image as a staging area.

    VkImageCreateInfo imageInfo {
        .sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO,
        .imageType = VK_IMAGE_TYPE_2D,
        .format = srcFormat,
        .extent = { width, height, 1 },
        .mipLevels = 1,
        .arrayLayers = 1,
        .samples = VK_SAMPLE_COUNT_1_BIT,
        .tiling = VK_IMAGE_TILING_LINEAR,
        .usage = VK_IMAGE_USAGE_TRANSFER_DST_BIT,
        .initialLayout = VK_IMAGE_LAYOUT_UNDEFINED,
    };

    VkImage stagingImage;
    vkCreateImage(device, &imageInfo, VKALLOC, &stagingImage);

    VkMemoryRequirements memReqs;
    VkDeviceMemory stagingMemory;
    vkGetImageMemoryRequirements(device, stagingImage, &memReqs);
    VkMemoryAllocateInfo allocInfo = {
        .sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO,
        .allocationSize = memReqs.size,
        .memoryTypeIndex = selectMemoryType(mContext, memReqs.memoryTypeBits,
                VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT)
    };

    vkAllocateMemory(device, &allocInfo, nullptr, &stagingMemory);
    vkBindImageMemory(device, stagingImage, stagingMemory, 0);

    // TODO: replace waitForIdle with an image barrier coupled with acquireWorkCommandBuffer.
    waitForIdle(mContext);

    // Transition the staging image layout.

    VulkanTexture::transitionImageLayout(mContext.work.cmdbuffer, stagingImage,
            VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 0, 1, 1,
            VK_IMAGE_ASPECT_COLOR_BIT);

    const uint8_t srcMipLevel = srcTarget->getColor(0).level;

    VkImageCopy imageCopyRegion = {
        .srcSubresource = {
            .aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
            .mipLevel = srcMipLevel,
            .layerCount = 1,
        },
        .srcOffset = {
            .x = (int32_t) x,
            .y = (int32_t) y,
        },
        .dstSubresource = {
            .aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
            .layerCount = 1,
        },
        .extent = {
            .width = width,
            .height = height,
            .depth = 1,
        },
    };

    // Transition the source image layout (which might be the swap chain)

    VkImage srcImage = srcTarget->getColor(0).image;
    VulkanTexture::transitionImageLayout(mContext.work.cmdbuffer, srcImage,
            VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL, srcMipLevel, 1, 1,
            VK_IMAGE_ASPECT_COLOR_BIT);

    // Perform the blit.

    vkCmdCopyImage(mContext.work.cmdbuffer, srcTarget->getColor(0).image,
            VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL, stagingImage, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
            1, &imageCopyRegion);

    // Restore the source image layout.

    if (srcTexture || mContext.currentSurface->presentQueue) {
        const VkImageLayout present = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;
        VulkanTexture::transitionImageLayout(mContext.work.cmdbuffer, srcImage,
                VK_IMAGE_LAYOUT_UNDEFINED, srcTexture ? getTextureLayout(srcTexture->usage) : present,
                srcMipLevel, 1, 1, VK_IMAGE_ASPECT_COLOR_BIT);
    } else {
        VulkanTexture::transitionImageLayout(mContext.work.cmdbuffer, srcImage,
                VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_GENERAL,
                srcMipLevel, 1, 1, VK_IMAGE_ASPECT_COLOR_BIT);
    }

    // Transition the staging image layout to GENERAL.

    VkImageMemoryBarrier barrier = {
        .sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER,
        .srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT,
        .dstAccessMask = VK_ACCESS_MEMORY_READ_BIT,
        .oldLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
        .newLayout = VK_IMAGE_LAYOUT_GENERAL,
        .srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
        .dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
        .image = stagingImage,
        .subresourceRange = {
            .aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
            .baseMipLevel = 0,
            .levelCount = 1,
            .baseArrayLayer = 0,
            .layerCount = 1,
        }
    };

    vkCmdPipelineBarrier(mContext.work.cmdbuffer, VK_PIPELINE_STAGE_TRANSFER_BIT,
            VK_PIPELINE_STAGE_TRANSFER_BIT, 0, 0, nullptr, 0, nullptr, 1, &barrier);

    flushWorkCommandBuffer(mContext);

    // Create a closure-friendly pointer that holds the rvalue reference.

    PixelBufferDescriptor* closure = new PixelBufferDescriptor();
    *closure = std::move(pbd);

    // Create a disposable to defer execution of the following code until after
    // the work command buffer has completed.

    mDisposer.createDisposable((VulkanDisposer::Key) stagingImage, [=] () {

        VkImageSubresource subResource { .aspectMask = VK_IMAGE_ASPECT_COLOR_BIT };
        VkSubresourceLayout subResourceLayout;
        vkGetImageSubresourceLayout(device, stagingImage, &subResource, &subResourceLayout);

        // Map image memory so we can start copying from it.

        const uint8_t* srcPixels;
        vkMapMemory(device, stagingMemory, 0, VK_WHOLE_SIZE, 0, (void**) &srcPixels);
        srcPixels += subResourceLayout.offset;

        // TODO: investigate why this Y-flip exists.
        constexpr bool flipY = true;
        if (!DataReshaper::reshapeImage(closure, getComponentType(srcFormat), srcPixels,
                subResourceLayout.rowPitch, width, height, swizzle, flipY)) {
            utils::slog.e << "Unsupported PixelDataFormat or PixelDataType" << utils::io::endl;
        }

        vkUnmapMemory(device, stagingMemory);
        vkFreeMemory(device, stagingMemory, nullptr);
        vkDestroyImage(device, stagingImage, nullptr);

        scheduleDestroy(std::move(*closure));
        delete closure;
    });

    // Next we reduce the ref count of the image to zero, which schedules the above callback to be
    // executed on the next beginFrame(), after the work command buffer is completed.
    mDisposer.removeReference((VulkanDisposer::Key) stagingImage);
}

void VulkanDriver::readStreamPixels(Handle<HwStream> sh, uint32_t x, uint32_t y, uint32_t width,
        uint32_t height, PixelBufferDescriptor&& p) {
    scheduleDestroy(std::move(p));
}

void VulkanDriver::blit(TargetBufferFlags buffers, Handle<HwRenderTarget> dst, Viewport dstRect,
        Handle<HwRenderTarget> src, Viewport srcRect, SamplerMagFilter filter) {
    VulkanRenderTarget* dstTarget = handle_cast<VulkanRenderTarget>(mHandleMap, dst);
    VulkanRenderTarget* srcTarget = handle_cast<VulkanRenderTarget>(mHandleMap, src);
    const int targetIndex = 0; // TODO: support MRT in blit

    // In debug builds, verify that the two render targets have blittable formats.
#ifndef NDEBUG
    const VkPhysicalDevice gpu = mContext.physicalDevice;
    VkFormatProperties info;
    vkGetPhysicalDeviceFormatProperties(gpu, srcTarget->getColor(targetIndex).format, &info);
    if (!ASSERT_POSTCONDITION_NON_FATAL(info.optimalTilingFeatures & VK_FORMAT_FEATURE_BLIT_SRC_BIT,
            "Source format is not blittable")) {
        return;
    }
    vkGetPhysicalDeviceFormatProperties(gpu, dstTarget->getColor(targetIndex).format, &info);
    if (!ASSERT_POSTCONDITION_NON_FATAL(info.optimalTilingFeatures & VK_FORMAT_FEATURE_BLIT_DST_BIT,
            "Destination format is not blittable")) {
        return;
    }
    if (any(buffers & TargetBufferFlags::DEPTH)) {
        utils::slog.w << "Depth blits are not yet supported." << utils::io::endl;
    }
#endif

    const VkExtent2D srcExtent = srcTarget->getExtent();
    const VkExtent2D dstExtent = dstTarget->getExtent();

    const int32_t srcLeft = std::min(srcRect.left, (int32_t) srcExtent.width);
    const int32_t srcBottom = std::min(srcRect.bottom, (int32_t) srcExtent.height);
    const int32_t srcRight = std::min(srcRect.left + srcRect.width, srcExtent.width);
    const int32_t srcTop = std::min(srcRect.bottom + srcRect.height, srcExtent.height);
    const uint32_t srcLevel = srcTarget->getColor(targetIndex).level;
    const uint32_t srcLayer = srcTarget->getColor(targetIndex).layer;

    const int32_t dstLeft = std::min(dstRect.left, (int32_t) dstExtent.width);
    const int32_t dstBottom = std::min(dstRect.bottom, (int32_t) dstExtent.height);
    const int32_t dstRight = std::min(dstRect.left + dstRect.width, dstExtent.width);
    const int32_t dstTop = std::min(dstRect.bottom + dstRect.height, dstExtent.height);
    const uint32_t dstLevel = dstTarget->getColor(targetIndex).level;
    const uint32_t dstLayer = dstTarget->getColor(targetIndex).layer;

    const VkImageAspectFlags aspect = VK_IMAGE_ASPECT_COLOR_BIT;

    const VkImageBlit blitRegions[1] = {{
        .srcSubresource = { aspect, srcLevel, srcLayer, 1 },
        .srcOffsets = { { srcLeft, srcBottom, 0 }, { srcRight, srcTop, 1 }},
        .dstSubresource = { aspect, dstLevel, dstLayer, 1 },
        .dstOffsets = { { dstLeft, dstBottom, 0 }, { dstRight, dstTop, 1 }}
    }};

    const VkImageResolve resolveRegions[1] = {{
        .srcSubresource = { aspect, srcLevel, srcLayer, 1 },
        .srcOffset = { srcLeft, srcBottom, 0 },
        .dstSubresource = { aspect, dstLevel, dstLayer, 1 },
        .dstOffset = { dstLeft, dstBottom, 0 },
        .extent = { srcExtent.width, srcExtent.height, 1 }
    }};

    const VulkanTexture* srcTexture = srcTarget->getColor(targetIndex).texture;
    const VulkanTexture* dstTexture = dstTarget->getColor(targetIndex).texture;

    auto vkblit = [=](VkCommandBuffer cmdbuffer) {
        VkImage srcImage = srcTarget->getColor(targetIndex).image;
        VulkanTexture::transitionImageLayout(cmdbuffer, srcImage, VK_IMAGE_LAYOUT_UNDEFINED,
                VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL, srcLevel, 1, 1, aspect);

        VkImage dstImage = dstTarget->getColor(targetIndex).image;
        VulkanTexture::transitionImageLayout(cmdbuffer, dstImage, VK_IMAGE_LAYOUT_UNDEFINED,
                VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, dstLevel, 1, 1, aspect);

        if (srcTexture && srcTexture->samples > 1 && dstTexture && dstTexture->samples == 1) {
            vkCmdResolveImage(cmdbuffer, srcImage, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL, dstImage,
                    VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, resolveRegions);
        } else {
            vkCmdBlitImage(cmdbuffer, srcImage, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL, dstImage,
                    VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, blitRegions,
                    filter == SamplerMagFilter::NEAREST ? VK_FILTER_NEAREST : VK_FILTER_LINEAR);
        }

        if (srcTexture) {
            VulkanTexture::transitionImageLayout(cmdbuffer, srcImage, VK_IMAGE_LAYOUT_UNDEFINED,
                    getTextureLayout(srcTexture->usage), srcLevel, 1, 1, aspect);
        } else if  (!mContext.currentSurface->headlessQueue) {
            VulkanTexture::transitionImageLayout(cmdbuffer, srcImage, VK_IMAGE_LAYOUT_UNDEFINED,
                    VK_IMAGE_LAYOUT_PRESENT_SRC_KHR, srcLevel, 1, 1, aspect);
        }

        // Determine the desired texture layout for the destination while ensuring that the default
        // render target is supported, which has no associated texture.
        const VkImageLayout desiredLayout = dstTexture ? getTextureLayout(dstTexture->usage) :
                getSwapContext(mContext).attachment.layout;

        VulkanTexture::transitionImageLayout(cmdbuffer, dstImage, VK_IMAGE_LAYOUT_UNDEFINED,
                desiredLayout, dstLevel, 1, 1, aspect);
    };

    if (!mContext.currentCommands) {
        vkblit(acquireWorkCommandBuffer(mContext));
        flushWorkCommandBuffer(mContext);
    } else {
        vkblit(mContext.currentCommands->cmdbuffer);
    }
}

void VulkanDriver::draw(PipelineState pipelineState, Handle<HwRenderPrimitive> rph) {
    VulkanCommandBuffer* commands = mContext.currentCommands;
    ASSERT_POSTCONDITION(commands, "Draw calls can occur only within a beginFrame / endFrame.");
    VkCommandBuffer cmdbuffer = commands->cmdbuffer;
    const VulkanRenderPrimitive& prim = *handle_cast<VulkanRenderPrimitive>(mHandleMap, rph);

    Handle<HwProgram> programHandle = pipelineState.program;
    RasterState rasterState = pipelineState.rasterState;
    PolygonOffset depthOffset = pipelineState.polygonOffset;
    const Viewport& viewportScissor = pipelineState.scissor;

    auto* program = handle_cast<VulkanProgram>(mHandleMap, programHandle);
    mDisposer.acquire(program, commands->resources);
    mDisposer.acquire(prim.indexBuffer, commands->resources);
    mDisposer.acquire(prim.vertexBuffer, commands->resources);

    // If this is a debug build, validate the current shader.
#if !defined(NDEBUG)
    if (program->bundle.vertex == VK_NULL_HANDLE || program->bundle.fragment == VK_NULL_HANDLE) {
        utils::slog.e << "Binding missing shader: " << program->name.c_str() << utils::io::endl;
    }
#endif

    // Update the VK raster state.

    const VulkanRenderTarget* rt = mCurrentRenderTarget;

    mContext.rasterState.depthStencil = {
        .sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO,
        .depthTestEnable = VK_TRUE,
        .depthWriteEnable = (VkBool32) rasterState.depthWrite,
        .depthCompareOp = getCompareOp(rasterState.depthFunc),
        .depthBoundsTestEnable = VK_FALSE,
        .stencilTestEnable = VK_FALSE,
    };

    mContext.rasterState.multisampling = {
        .sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO,
        .rasterizationSamples = (VkSampleCountFlagBits) rt->getSamples(),
        .alphaToCoverageEnable = rasterState.alphaToCoverage,
    };

    mContext.rasterState.blending = {
        .blendEnable = (VkBool32) rasterState.hasBlending(),
        .srcColorBlendFactor = getBlendFactor(rasterState.blendFunctionSrcRGB),
        .dstColorBlendFactor = getBlendFactor(rasterState.blendFunctionDstRGB),
        .colorBlendOp = (VkBlendOp) rasterState.blendEquationRGB,
        .srcAlphaBlendFactor = getBlendFactor(rasterState.blendFunctionSrcAlpha),
        .dstAlphaBlendFactor = getBlendFactor(rasterState.blendFunctionDstAlpha),
        .alphaBlendOp =  (VkBlendOp) rasterState.blendEquationAlpha,
        .colorWriteMask = (VkColorComponentFlags) (rasterState.colorWrite ? 0xf : 0x0),
    };

    VkPipelineRasterizationStateCreateInfo& vkraster = mContext.rasterState.rasterization;
    vkraster.cullMode = getCullMode(rasterState.culling);
    vkraster.frontFace = getFrontFace(rasterState.inverseFrontFaces);
    vkraster.depthBiasEnable = (depthOffset.constant || depthOffset.slope) ? VK_TRUE : VK_FALSE;
    vkraster.depthBiasConstantFactor = depthOffset.constant;
    vkraster.depthBiasSlopeFactor = depthOffset.slope;

    mContext.rasterState.getColorTargetCount = rt->getColorTargetCount();

    VulkanBinder::ProgramBundle shaderHandles = program->bundle;

    // Push state changes to the VulkanBinder instance. This is fast and does not make VK calls.
    mBinder.bindProgramBundle(shaderHandles);
    mBinder.bindRasterState(mContext.rasterState);
    mBinder.bindPrimitiveTopology(prim.primitiveTopology);
    mBinder.bindVertexArray(prim.varray);

    // Query the program for the mapping from (SamplerGroupBinding,Offset) to (SamplerBinding),
    // where "SamplerBinding" is the integer in the GLSL, and SamplerGroupBinding is the abstract
    // Filament concept used to form groups of samplers.

    for (uint8_t samplerGroupIdx = 0; samplerGroupIdx < Program::SAMPLER_BINDING_COUNT; samplerGroupIdx++) {
        const auto& samplerGroup = program->samplerGroupInfo[samplerGroupIdx];
        if (samplerGroup.empty()) {
            continue;
        }
        VulkanSamplerGroup* vksb = mSamplerBindings[samplerGroupIdx];
        if (!vksb) {
            continue;
        }
        SamplerGroup* sb = vksb->sb.get();
        assert(sb->getSize() == samplerGroup.size());
        size_t samplerIdx = 0;
        for (const auto& sampler : samplerGroup) {
            size_t bindingPoint = sampler.binding;
            const SamplerGroup::Sampler* boundSampler = sb->getSamplers() + samplerIdx;
            samplerIdx++;

            if (!boundSampler->t) {
                continue;
            }

            const SamplerParams& samplerParams = boundSampler->s;
            VkSampler vksampler = mSamplerCache.getSampler(samplerParams);
            const auto* texture = handle_const_cast<VulkanTexture>(mHandleMap, boundSampler->t);
            mDisposer.acquire(texture, commands->resources);

            mBinder.bindSampler(bindingPoint, {
                .sampler = vksampler,
                .imageView = texture->imageView,
                .imageLayout = getTextureLayout(texture->usage)
            });
        }
    }

    // Set scissoring.
    // Compute the intersection of the requested scissor rectangle with the current viewport.
    const int32_t x = std::max(viewportScissor.left, (int32_t)mContext.viewport.x);
    const int32_t y = std::max(viewportScissor.bottom, (int32_t)mContext.viewport.y);
    const int32_t right = std::min(viewportScissor.left + (int32_t)viewportScissor.width,
            (int32_t)(mContext.viewport.x + mContext.viewport.width));
    const int32_t top = std::min(viewportScissor.bottom + (int32_t)viewportScissor.height,
            (int32_t)(mContext.viewport.y + mContext.viewport.height));
    VkRect2D scissor{
            .offset = { std::max(0, x), std::max(0, y) },
            .extent = { (uint32_t)right - x, (uint32_t)top - y }
    };
    rt->transformClientRectToPlatform(&scissor);
    vkCmdSetScissor(cmdbuffer, 0, 1, &scissor);

    // Bind new descriptor sets if they need to change.
    VkDescriptorSet descriptors[3];
    VkPipelineLayout pipelineLayout;
    if (mBinder.getOrCreateDescriptors(descriptors, &pipelineLayout)) {
        vkCmdBindDescriptorSets(cmdbuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipelineLayout, 0, 3,
                descriptors, 0, nullptr);
    }

    // Bind the pipeline if it changed. This can happen, for example, if the raster state changed.
    // Creating a new pipeline is slow, so we should consider using pipeline cache objects.
    VkPipeline pipeline;
    if (mBinder.getOrCreatePipeline(&pipeline)) {
        vkCmdBindPipeline(cmdbuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline);
    }

    // Next bind the vertex buffers and index buffer. One potential performance improvement is to
    // avoid rebinding these if they are already bound, but since we do not (yet) support subranges
    // it would be rare for a client to make consecutive draw calls with the same render primitive.
    vkCmdBindVertexBuffers(cmdbuffer, 0, (uint32_t) prim.buffers.size(),
            prim.buffers.data(), prim.offsets.data());
    vkCmdBindIndexBuffer(cmdbuffer, prim.indexBuffer->buffer->getGpuBuffer(), 0,
            prim.indexBuffer->indexType);

    // Finally, make the actual draw call. TODO: support subranges
    const uint32_t indexCount = prim.count;
    const uint32_t instanceCount = 1;
    const uint32_t firstIndex = prim.offset / prim.indexBuffer->elementSize;
    const int32_t vertexOffset = 0;
    const uint32_t firstInstId = 1;
    vkCmdDrawIndexed(cmdbuffer, indexCount, instanceCount, firstIndex, vertexOffset, firstInstId);
}

void VulkanDriver::beginTimerQuery(Handle<HwTimerQuery> tqh) {
    VulkanCommandBuffer* commands = mContext.currentCommands;
    ASSERT_POSTCONDITION(commands, "Timer queries can occur only within a beginFrame / endFrame.");

    VulkanTimerQuery* vtq = handle_cast<VulkanTimerQuery>(mHandleMap, tqh);
    const uint32_t index = vtq->startingQueryIndex;
    const VkPipelineStageFlagBits stage = VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT;

    vkCmdResetQueryPool(commands->cmdbuffer, mContext.timestamps.pool, index, 2);
    vkCmdWriteTimestamp(commands->cmdbuffer, stage, mContext.timestamps.pool, index);
    vtq->cmdbuffer.store(commands);
}

void VulkanDriver::endTimerQuery(Handle<HwTimerQuery> tqh) {
    VulkanCommandBuffer* commands = mContext.currentCommands;
    ASSERT_POSTCONDITION(commands, "Timer queries can occur only within a beginFrame / endFrame.");

    VulkanTimerQuery* vtq = handle_cast<VulkanTimerQuery>(mHandleMap, tqh);
    const uint32_t index = vtq->stoppingQueryIndex;
    const VkPipelineStageFlagBits stage = VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT;
    vkCmdWriteTimestamp(commands->cmdbuffer, stage, mContext.timestamps.pool, index);
}

void VulkanDriver::refreshSwapChain() {
    VulkanSurfaceContext& surface = *mContext.currentSurface;

    assert(!surface.headlessQueue && "Resizing headless swap chains is not supported.");
    backend::destroySwapChain(mContext, surface, mDisposer);
    createSwapChain(mContext, surface);

    mFramebufferCache.reset();
}

#ifndef NDEBUG
void VulkanDriver::debugCommand(const char* methodName) {
    static const std::set<utils::StaticString> OUTSIDE_COMMANDS = {
        "loadUniformBuffer",
        "updateVertexBuffer",
        "updateIndexBuffer",
        "update2DImage",
        "updateCubeImage",
    };
    static const utils::StaticString BEGIN_COMMAND = "beginRenderPass";
    static const utils::StaticString END_COMMAND = "endRenderPass";
    static bool inRenderPass = false; // for debug only
    const utils::StaticString command = utils::StaticString::make(methodName, strlen(methodName));
    if (command == BEGIN_COMMAND) {
        assert(!inRenderPass);
        inRenderPass = true;
    } else if (command == END_COMMAND) {
        assert(inRenderPass);
        inRenderPass = false;
    } else if (inRenderPass && OUTSIDE_COMMANDS.find(command) != OUTSIDE_COMMANDS.end()) {
        utils::slog.e << command.c_str() << " issued inside a render pass." << utils::io::endl;
        utils::debug_trap();
    }
}
#endif

// explicit instantiation of the Dispatcher
template class ConcreteDispatcher<VulkanDriver>;

} // namespace backend
} // namespace filament

#pragma clang diagnostic pop
