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
#include "VulkanCommands.h"
#include "VulkanDriverFactory.h"
#include "VulkanHandles.h"
#include "VulkanMemory.h"
#include "VulkanPlatform.h"

#include <utils/CString.h>
#include <utils/FixedCapacityVector.h>
#include <utils/Panic.h>
#include <utils/trap.h>

#ifndef NDEBUG
#include <set>
#endif

using namespace bluevk;

using utils::FixedCapacityVector;

// Vulkan functions often immediately dereference pointers, so it's fine to pass in a pointer
// to a stack-allocated variable.
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wreturn-stack-address"
#pragma clang diagnostic ignored "-Wunused-parameter"

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
    utils::slog.e << utils::io::endl;
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
        // TODO: emit best practices warnings about aggressive pipeline barriers.
        if (strstr(cbdata->pMessage, "ALL_GRAPHICS_BIT") || strstr(cbdata->pMessage, "ALL_COMMANDS_BIT")) {
           return VK_FALSE;
        }
        utils::slog.w << "VULKAN WARNING: (" << cbdata->pMessageIdName << ") "
                << cbdata->pMessage << utils::io::endl;
    }
    utils::slog.e << utils::io::endl;
    return VK_FALSE;
}

}

#endif

namespace filament::backend {

Driver* VulkanDriverFactory::create(VulkanPlatform* const platform,
        const char* const* ppRequiredExtensions, uint32_t requiredExtensionCount) noexcept {
    return VulkanDriver::create(platform, ppRequiredExtensions, requiredExtensionCount);
}

Dispatcher VulkanDriver::getDispatcher() const noexcept {
    return ConcreteDispatcher<VulkanDriver>::make();
}

VulkanDriver::VulkanDriver(VulkanPlatform* platform,
        const char* const* ppRequiredExtensions, uint32_t requiredExtensionCount) noexcept :
        mHandleAllocator("Handles", FILAMENT_VULKAN_HANDLE_ARENA_SIZE_IN_MB * 1024U * 1024U),
        mContextManager(*platform),
        mStagePool(mContext),
        mFramebufferCache(mContext),
        mSamplerCache(mContext),
        mBlitter(mContext, mStagePool, mPipelineCache, mFramebufferCache, mSamplerCache) {
    mContext.rasterState = mPipelineCache.getDefaultRasterState();

    // Load Vulkan entry points.
    ASSERT_POSTCONDITION(bluevk::initialize(), "BlueVK is unable to load entry points.");

    // Determine if the VK_EXT_debug_utils instance extension is available.
    mContext.debugUtilsSupported = false;
    uint32_t availableExtsCount = 0;
    vkEnumerateInstanceExtensionProperties(nullptr, &availableExtsCount, nullptr);
    utils::FixedCapacityVector<VkExtensionProperties> availableExts(availableExtsCount);
    vkEnumerateInstanceExtensionProperties(nullptr, &availableExtsCount, availableExts.data());
    for  (const auto& extProps : availableExts) {
        if (!strcmp(extProps.extensionName, VK_EXT_DEBUG_UTILS_EXTENSION_NAME)) {
            mContext.debugUtilsSupported = true;
            break;
        }
    }

    VkInstanceCreateInfo instanceCreateInfo = {};

    bool validationFeaturesSupported = false;

#if VK_ENABLE_VALIDATION
    const utils::StaticString DESIRED_LAYERS[] = {
        "VK_LAYER_KHRONOS_validation",
#if FILAMENT_VULKAN_DUMP_API
        "VK_LAYER_LUNARG_api_dump",
#endif
#if defined(ENABLE_RENDERDOC)
        "VK_LAYER_RENDERDOC_Capture",
#endif
    };

    constexpr size_t kMaxEnabledLayersCount = sizeof(DESIRED_LAYERS) / sizeof(DESIRED_LAYERS[0]);

    uint32_t layerCount;
    vkEnumerateInstanceLayerProperties(&layerCount, nullptr);
    FixedCapacityVector<VkLayerProperties> availableLayers(layerCount);
    vkEnumerateInstanceLayerProperties(&layerCount, availableLayers.data());
    auto enabledLayers = FixedCapacityVector<const char*>::with_capacity(kMaxEnabledLayersCount);
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

        // Check if VK_EXT_validation_features is supported.
        uint32_t availableExtsCount = 0;
        vkEnumerateInstanceExtensionProperties("VK_LAYER_KHRONOS_validation", &availableExtsCount, nullptr);
        utils::FixedCapacityVector<VkExtensionProperties> availableExts(availableExtsCount);
        vkEnumerateInstanceExtensionProperties("VK_LAYER_KHRONOS_validation", &availableExtsCount, availableExts.data());
        for  (const auto& extProps : availableExts) {
            if (!strcmp(extProps.extensionName, VK_EXT_VALIDATION_FEATURES_EXTENSION_NAME)) {
                validationFeaturesSupported = true;
                break;
            }
        }

    } else {
#if defined(__ANDROID__)
        utils::slog.d << "Validation layers are not available; did you set jniLibs in your "
                << "gradle file?" << utils::io::endl;
#else
        utils::slog.d << "Validation layer not available; did you install the Vulkan SDK?\n"
                << "Please ensure that VK_LAYER_PATH is set correctly." << utils::io::endl;
#endif
    }
#endif // VK_ENABLE_VALIDATION

    // The Platform class can require 1 or 2 instance extensions, plus we'll request at most 5
    // instance extensions here in the common code. So that's a max of 7.
    static constexpr uint32_t MAX_INSTANCE_EXTENSION_COUNT = 7;
    const char* ppEnabledExtensions[MAX_INSTANCE_EXTENSION_COUNT];
    uint32_t enabledExtensionCount = 0;

    // Request all cross-platform extensions.
    ppEnabledExtensions[enabledExtensionCount++] = "VK_KHR_surface";
    ppEnabledExtensions[enabledExtensionCount++] = "VK_KHR_get_physical_device_properties2";
#if VK_ENABLE_VALIDATION
#if defined(__ANDROID__)
    ppEnabledExtensions[enabledExtensionCount++] = "VK_EXT_debug_report";
#endif
    if (validationFeaturesSupported) {
        ppEnabledExtensions[enabledExtensionCount++] = "VK_EXT_validation_features";
    }
#endif
    if (mContext.debugUtilsSupported) {
        ppEnabledExtensions[enabledExtensionCount++] = "VK_EXT_debug_utils";
    }

    // Request platform-specific extensions.
    for (uint32_t i = 0; i < requiredExtensionCount; ++i) {
        assert_invariant(enabledExtensionCount < MAX_INSTANCE_EXTENSION_COUNT);
        ppEnabledExtensions[enabledExtensionCount++] = ppRequiredExtensions[i];
    }

    // Create the Vulkan instance.
    VkApplicationInfo appInfo = {};
    appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
    appInfo.apiVersion = VK_MAKE_VERSION(VK_REQUIRED_VERSION_MAJOR, VK_REQUIRED_VERSION_MINOR, 0);
    instanceCreateInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
    instanceCreateInfo.pApplicationInfo = &appInfo;
    instanceCreateInfo.enabledExtensionCount = enabledExtensionCount;
    instanceCreateInfo.ppEnabledExtensionNames = ppEnabledExtensions;

    VkValidationFeaturesEXT features = {};
    VkValidationFeatureEnableEXT enables[] = {
        VK_VALIDATION_FEATURE_ENABLE_BEST_PRACTICES_EXT,
        // TODO: Enable synchronization validation.
        // VK_VALIDATION_FEATURE_ENABLE_SYNCHRONIZATION_VALIDATION_EXT,
    };
    if (validationFeaturesSupported) {
        features.sType = VK_STRUCTURE_TYPE_VALIDATION_FEATURES_EXT;
        features.enabledValidationFeatureCount = sizeof(enables) / sizeof(enables[0]);
        features.pEnabledValidationFeatures = enables;
        instanceCreateInfo.pNext = &features;
    }

    VkResult result = vkCreateInstance(&instanceCreateInfo, VKALLOC, &mContext.instance);
    ASSERT_POSTCONDITION(result == VK_SUCCESS, "Unable to create Vulkan instance.");
    bluevk::bindInstance(mContext.instance);
    UTILS_UNUSED const PFN_vkCreateDebugReportCallbackEXT createDebugReportCallback =
            vkCreateDebugReportCallbackEXT;

#if VK_ENABLE_VALIDATION
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
    mContext.selectPhysicalDevice();

    // Initialize device, graphicsQueue, and command buffer manager.
    mContext.createLogicalDevice();

    mContext.createEmptyTexture(mStagePool);

    mContext.commands->setObserver(&mPipelineCache);
    mPipelineCache.setDevice(mContext.device, mContext.allocator);
    mPipelineCache.setDummyTexture(mContext.emptyTexture->getPrimaryImageView());

    // Choose a depth format that meets our requirements. Take care not to include stencil formats
    // just yet, since that would require a corollary change to the "aspect" flags for the VkImage.
    const VkFormat formats[] = { VK_FORMAT_D32_SFLOAT, VK_FORMAT_X8_D24_UNORM_PACK32 };
    mContext.finalDepthFormat = mContext.findSupportedFormat(
        utils::Slice<VkFormat>(formats, formats + 2),
        VK_IMAGE_TILING_OPTIMAL, VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT);

    // For diagnostic purposes, print useful information about available depth formats.
    // Note that Vulkan is more constrained than OpenGL ES 3.1 in this area.
    if constexpr (VK_ENABLE_VALIDATION && FILAMENT_VULKAN_VERBOSE) {
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
    }
}

VulkanDriver::~VulkanDriver() noexcept = default;

UTILS_NOINLINE
Driver* VulkanDriver::create(VulkanPlatform* const platform,
        const char* const* ppEnabledExtensions, uint32_t enabledExtensionCount) noexcept {
    assert_invariant(platform);
    return new VulkanDriver(platform, ppEnabledExtensions, enabledExtensionCount);
}

ShaderModel VulkanDriver::getShaderModel() const noexcept {
#if defined(__ANDROID__) || defined(IOS)
    return ShaderModel::GL_ES_30;
#else
    return ShaderModel::GL_CORE_41;
#endif
}

void VulkanDriver::terminate() {
    if (!mContext.instance) {
        return;
    }

    delete mContext.commands;
    delete mContext.emptyTexture;

    mBlitter.shutdown();

    // Allow the stage pool and disposer to clean up.
    mStagePool.gc();
    mDisposer.reset();

    mStagePool.reset();
    mPipelineCache.destroyCache();
    mFramebufferCache.reset();
    mSamplerCache.reset();

    vmaDestroyPool(mContext.allocator, mContext.vmaPoolGPU);
    vmaDestroyPool(mContext.allocator, mContext.vmaPoolCPU);
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
    mContext.commands->updateFences();
}

// Garbage collection should not occur too frequently, only about once per frame. Internally, the
// eviction time of various resources is often measured in terms of an approximate frame number
// rather than the wall clock, because we must wait 3 frames after a DriverAPI-level resource has
// been destroyed for safe destruction, due to outstanding command buffers and triple buffering.
void VulkanDriver::collectGarbage() {
    mStagePool.gc();
    mFramebufferCache.gc();
    mDisposer.gc();
    mContext.commands->gc();
}

void VulkanDriver::beginFrame(int64_t monotonic_clock_ns, uint32_t frameId) {
    // Do nothing.
}

void VulkanDriver::setFrameScheduledCallback(Handle<HwSwapChain> sch,
        FrameScheduledCallback callback, void* user) {

}

void VulkanDriver::setFrameCompletedCallback(Handle<HwSwapChain> sch,
        FrameCompletedCallback callback, void* user) {

}

void VulkanDriver::setPresentationTime(int64_t monotonic_clock_ns) {
}

void VulkanDriver::endFrame(uint32_t frameId) {
    if (mContext.commands->flush()) {
        collectGarbage();
    }
}

void VulkanDriver::flush(int) {
    mContext.commands->flush();
}

void VulkanDriver::finish(int dummy) {
    mContext.commands->flush();
}

void VulkanDriver::createSamplerGroupR(Handle<HwSamplerGroup> sbh, uint32_t count) {
    construct<VulkanSamplerGroup>(sbh, count);
}

void VulkanDriver::createRenderPrimitiveR(Handle<HwRenderPrimitive> rph, int) {
    construct<VulkanRenderPrimitive>(rph);
}

void VulkanDriver::destroyRenderPrimitive(Handle<HwRenderPrimitive> rph) {
    if (rph) {
        destruct<VulkanRenderPrimitive>(rph);
    }
}

void VulkanDriver::createVertexBufferR(Handle<HwVertexBuffer> vbh, uint8_t bufferCount,
        uint8_t attributeCount, uint32_t elementCount, AttributeArray attributes) {
    auto vertexBuffer = construct<VulkanVertexBuffer>(vbh, mContext, mStagePool,
            bufferCount, attributeCount, elementCount, attributes);
    mDisposer.createDisposable(vertexBuffer, [this, vbh] () {
        destruct<VulkanVertexBuffer>(vbh);
    });
}

void VulkanDriver::destroyVertexBuffer(Handle<HwVertexBuffer> vbh) {
    if (vbh) {
        auto vertexBuffer = handle_cast<VulkanVertexBuffer*>(vbh);
        mDisposer.removeReference(vertexBuffer);
    }
}

void VulkanDriver::createIndexBufferR(Handle<HwIndexBuffer> ibh,
        ElementType elementType, uint32_t indexCount, BufferUsage usage) {
    auto elementSize = (uint8_t) getElementTypeSize(elementType);
    auto indexBuffer = construct<VulkanIndexBuffer>(ibh, mContext, mStagePool,
            elementSize, indexCount);
    mDisposer.createDisposable(indexBuffer, [this, ibh] () {
        destruct<VulkanIndexBuffer>(mContext, ibh);
    });
}

void VulkanDriver::destroyIndexBuffer(Handle<HwIndexBuffer> ibh) {
    if (ibh) {
        auto indexBuffer = handle_cast<VulkanIndexBuffer*>(ibh);
        mDisposer.removeReference(indexBuffer);
    }
}

void VulkanDriver::createBufferObjectR(Handle<HwBufferObject> boh,
        uint32_t byteCount, BufferObjectBinding bindingType, BufferUsage usage) {
    auto bufferObject = construct<VulkanBufferObject>(boh, mContext, mStagePool, byteCount,
            bindingType, usage);
    mDisposer.createDisposable(bufferObject, [this, boh] () {
       destruct<VulkanBufferObject>(mContext, boh);
    });
}

void VulkanDriver::destroyBufferObject(Handle<HwBufferObject> boh) {
    if (boh) {
       auto bufferObject = handle_cast<VulkanBufferObject*>(boh);
       if (bufferObject->bindingType == BufferObjectBinding::UNIFORM) {
           mPipelineCache.unbindUniformBuffer(bufferObject->buffer.getGpuBuffer());
           // Decrement the refcount of the uniform buffer, but schedule it for destruction a few
           // frames in the future. To be safe, we need to assume that the current command buffer is
           // still using it somewhere.
           mDisposer.acquire(bufferObject);
       }
       mDisposer.removeReference(bufferObject);
    }
}

void VulkanDriver::createTextureR(Handle<HwTexture> th, SamplerType target, uint8_t levels,
        TextureFormat format, uint8_t samples, uint32_t w, uint32_t h, uint32_t depth,
        TextureUsage usage) {
    auto vktexture = construct<VulkanTexture>(th, mContext, target, levels,
            format, samples, w, h, depth, usage, mStagePool);
    mDisposer.createDisposable(vktexture, [this, th] () {
        destruct<VulkanTexture>(th);
    });
}

void VulkanDriver::createTextureSwizzledR(Handle<HwTexture> th, SamplerType target, uint8_t levels,
        TextureFormat format, uint8_t samples, uint32_t w, uint32_t h, uint32_t depth,
        TextureUsage usage,
        TextureSwizzle r, TextureSwizzle g, TextureSwizzle b, TextureSwizzle a) {
    TextureSwizzle swizzleArray[] = {r, g, b, a};
    const VkComponentMapping swizzleMap = getSwizzleMap(swizzleArray);
    auto vktexture = construct<VulkanTexture>(th, mContext, target, levels,
            format, samples, w, h, depth, usage, mStagePool, swizzleMap);
    mDisposer.createDisposable(vktexture, [this, th] () {
        destruct<VulkanTexture>(th);
    });
}

void VulkanDriver::importTextureR(Handle<HwTexture> th, intptr_t id,
        SamplerType target, uint8_t levels,
        TextureFormat format, uint8_t samples, uint32_t w, uint32_t h, uint32_t depth,
        TextureUsage usage) {
    // not supported in this backend
}

void VulkanDriver::destroyTexture(Handle<HwTexture> th) {
    if (th) {
        auto texture = handle_cast<VulkanTexture*>(th);
        mPipelineCache.unbindImageView(texture->getPrimaryImageView());
        mDisposer.removeReference(texture);
    }
}

void VulkanDriver::createProgramR(Handle<HwProgram> ph, Program&& program) {
    auto vkprogram = construct<VulkanProgram>(ph, mContext, program);
    mDisposer.createDisposable(vkprogram, [this, ph] () {
        destruct<VulkanProgram>(ph);
    });
}

void VulkanDriver::destroyProgram(Handle<HwProgram> ph) {
    if (ph) {
        mDisposer.removeReference(handle_cast<VulkanProgram*>(ph));
    }
}

void VulkanDriver::createDefaultRenderTargetR(Handle<HwRenderTarget> rth, int) {
    assert_invariant(mContext.defaultRenderTarget == nullptr);
    VulkanRenderTarget* renderTarget = construct<VulkanRenderTarget>(rth);
    mContext.defaultRenderTarget = renderTarget;
    mDisposer.createDisposable(renderTarget, [this, rth] () {
        destruct<VulkanRenderTarget>(rth);
    });
}

void VulkanDriver::createRenderTargetR(Handle<HwRenderTarget> rth,
        TargetBufferFlags targets, uint32_t width, uint32_t height, uint8_t samples,
        MRT color, TargetBufferInfo depth, TargetBufferInfo stencil) {
    VulkanAttachment colorTargets[MRT::MAX_SUPPORTED_RENDER_TARGET_COUNT] = {};
    for (int i = 0; i < MRT::MAX_SUPPORTED_RENDER_TARGET_COUNT; i++) {
        if (color[i].handle) {
            colorTargets[i] = {
                .texture = handle_cast<VulkanTexture*>(color[i].handle),
                .level = color[i].level,
                .layer = color[i].layer,
            };
            UTILS_UNUSED_IN_RELEASE VkExtent2D extent = colorTargets[i].getExtent2D();
            assert_invariant(extent.width >= width && extent.height >= height);
        }
    }

    VulkanAttachment depthStencil[2] = {};
    if (depth.handle) {
        depthStencil[0] = {
            .texture = handle_cast<VulkanTexture*>(depth.handle),
            .level = depth.level,
            .layer = depth.layer,
        };
        UTILS_UNUSED_IN_RELEASE VkExtent2D extent = depthStencil[0].getExtent2D();
        assert_invariant(extent.width >= width && extent.height >= height);
    }

    if (stencil.handle) {
        depthStencil[1] = {
            .texture = handle_cast<VulkanTexture*>(stencil.handle),
            .level = stencil.level,
            .layer = stencil.layer,
        };
        UTILS_UNUSED_IN_RELEASE VkExtent2D extent = depthStencil[1].getExtent2D();
        assert_invariant(extent.width >= width && extent.height >= height);
    }

    auto renderTarget = construct<VulkanRenderTarget>(rth, mContext,
            width, height, samples, colorTargets, depthStencil, mStagePool);
    mDisposer.createDisposable(renderTarget, [this, rth] () {
        destruct<VulkanRenderTarget>(rth);
    });
}

void VulkanDriver::destroyRenderTarget(Handle<HwRenderTarget> rth) {
    if (rth) {
        VulkanRenderTarget* rt = handle_cast<VulkanRenderTarget*>(rth);
        if (UTILS_UNLIKELY(rt == mContext.defaultRenderTarget)) {
            mContext.defaultRenderTarget = nullptr;
        }
        mDisposer.removeReference(rt);
    }
}

void VulkanDriver::createFenceR(Handle<HwFence> fh, int) {
    VulkanCommandBuffer const& commandBuffer = mContext.commands->get();
    construct<VulkanFence>(fh, commandBuffer);
}

void VulkanDriver::createSyncR(Handle<HwSync> sh, int) {
    VulkanCommandBuffer const& commandBuffer = mContext.commands->get();
    construct<VulkanSync>(sh, commandBuffer);
}

void VulkanDriver::createSwapChainR(Handle<HwSwapChain> sch, void* nativeWindow, uint64_t flags) {
    const VkInstance instance = mContext.instance;
    auto vksurface = (VkSurfaceKHR) mContextManager.createVkSurfaceKHR(nativeWindow, instance,
            flags);
    construct<VulkanSwapChain>(sch, mContext, mStagePool, vksurface);
}

void VulkanDriver::createSwapChainHeadlessR(Handle<HwSwapChain> sch,
        uint32_t width, uint32_t height, uint64_t flags) {
    assert_invariant(width > 0 && height > 0 && "Vulkan requires non-zero swap chain dimensions.");
    construct<VulkanSwapChain>(sch, mContext, mStagePool, width, height);
}

void VulkanDriver::createTimerQueryR(Handle<HwTimerQuery> tqh, int) {
    // nothing to do, timer query was constructed in createTimerQueryS
}

Handle<HwVertexBuffer> VulkanDriver::createVertexBufferS() noexcept {
    return allocHandle<VulkanVertexBuffer>();
}

Handle<HwIndexBuffer> VulkanDriver::createIndexBufferS() noexcept {
    return allocHandle<VulkanIndexBuffer>();
}

Handle<HwBufferObject> VulkanDriver::createBufferObjectS() noexcept {
    return allocHandle<VulkanBufferObject>();
}

Handle<HwTexture> VulkanDriver::createTextureS() noexcept {
    return allocHandle<VulkanTexture>();
}

Handle<HwTexture> VulkanDriver::createTextureSwizzledS() noexcept {
    return allocHandle<VulkanTexture>();
}

Handle<HwTexture> VulkanDriver::importTextureS() noexcept {
    return allocHandle<VulkanTexture>();
}

Handle<HwSamplerGroup> VulkanDriver::createSamplerGroupS() noexcept {
    return allocHandle<VulkanSamplerGroup>();
}

Handle<HwRenderPrimitive> VulkanDriver::createRenderPrimitiveS() noexcept {
    return allocHandle<VulkanRenderPrimitive>();
}

Handle<HwProgram> VulkanDriver::createProgramS() noexcept {
    return allocHandle<VulkanProgram>();
}

Handle<HwRenderTarget> VulkanDriver::createDefaultRenderTargetS() noexcept {
    return allocHandle<VulkanRenderTarget>();
}

Handle<HwRenderTarget> VulkanDriver::createRenderTargetS() noexcept {
    return allocHandle<VulkanRenderTarget>();
}

Handle<HwFence> VulkanDriver::createFenceS() noexcept {
    return allocHandle<VulkanFence>();
}

Handle<HwSync> VulkanDriver::createSyncS() noexcept {
    Handle<HwSync> sh = allocHandle<VulkanSync>();
    construct<VulkanSync>(sh);
    return sh;
}

Handle<HwSwapChain> VulkanDriver::createSwapChainS() noexcept {
    return allocHandle<VulkanSwapChain>();
}

Handle<HwSwapChain> VulkanDriver::createSwapChainHeadlessS() noexcept {
    return allocHandle<VulkanSwapChain>();
}

Handle<HwTimerQuery> VulkanDriver::createTimerQueryS() noexcept {
    // The handle must be constructed here, as a synchronous call to getTimerQueryValue might happen
    // before createTimerQueryR is executed.
    Handle<HwTimerQuery> tqh = initHandle<VulkanTimerQuery>(mContext);
    auto query = handle_cast<VulkanTimerQuery*>(tqh);
    mDisposer.createDisposable(query, [this, tqh] () {
        destruct<VulkanTimerQuery>(tqh);
    });
    return tqh;
}

void VulkanDriver::destroySamplerGroup(Handle<HwSamplerGroup> sbh) {
    if (sbh) {
        // Unlike most of the other "Hw" handles, the sampler buffer is an abstract concept and does
        // not map to any Vulkan objects. To handle destruction, the only thing we need to do is
        // ensure that the next draw call doesn't try to access a zombie sampler buffer. Therefore,
        // simply replace all weak references with null.
        auto* hwsb = handle_cast<VulkanSamplerGroup*>(sbh);
        for (auto& binding : mSamplerBindings) {
            if (binding == hwsb) {
                binding = nullptr;
            }
        }
        destruct<VulkanSamplerGroup>(sbh);
    }
}

void VulkanDriver::destroySwapChain(Handle<HwSwapChain> sch) {
    if (sch) {
        VulkanSwapChain& swapChain = *handle_cast<VulkanSwapChain*>(sch);
        swapChain.destroy();

        vkDestroySurfaceKHR(mContext.instance, swapChain.surface, VKALLOC);
        if (mContext.currentSwapChain == &swapChain) {
            mContext.currentSwapChain = nullptr;
        }

        destruct<VulkanSwapChain>(sch);
    }
}

void VulkanDriver::destroyStream(Handle<HwStream> sh) {
}

void VulkanDriver::destroyTimerQuery(Handle<HwTimerQuery> tqh) {
    if (tqh) {
        mDisposer.removeReference(handle_cast<VulkanTimerQuery*>(tqh));
    }
}

void VulkanDriver::destroySync(Handle<HwSync> sh) {
    destruct<VulkanSync>(sh);
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
    destruct<VulkanFence>(fh);
}

FenceStatus VulkanDriver::wait(Handle<HwFence> fh, uint64_t timeout) {
    auto& cmdfence = handle_cast<VulkanFence*>(fh)->fence;

    // Internally we use the VK_INCOMPLETE status to mean "not yet submitted".
    // When this fence gets submitted, its status changes to VK_NOT_READY.
    std::unique_lock<utils::Mutex> lock(cmdfence->mutex);
    if (cmdfence->status.load() == VK_INCOMPLETE) {
        // This will obviously timeout if Filament creates a fence and immediately waits on it
        // without calling endFrame() or commit().
        cmdfence->condition.wait(lock);
    } else {
        lock.unlock();
    }
    VkResult result = vkWaitForFences(mContext.device, 1, &cmdfence->fence, VK_TRUE, timeout);
    return result == VK_SUCCESS ? FenceStatus::CONDITION_SATISFIED : FenceStatus::TIMEOUT_EXPIRED;
}

// We create all textures using VK_IMAGE_TILING_OPTIMAL, so our definition of "supported" is that
// the GPU supports the given texture format with non-zero optimal tiling features.
bool VulkanDriver::isTextureFormatSupported(TextureFormat format) {
    assert_invariant(mContext.physicalDevice);
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
    assert_invariant(mContext.physicalDevice);
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

bool VulkanDriver::isFrameBufferFetchMultiSampleSupported() {
    return false;
}

bool VulkanDriver::isFrameTimeSupported() {
    return true;
}

bool VulkanDriver::isAutoDepthResolveSupported() {
    return false;
}

bool VulkanDriver::isWorkaroundNeeded(Workaround workaround) {
    VkPhysicalDeviceProperties const& deviceProperties = mContext.physicalDeviceProperties;
    switch (workaround) {
        case Workaround::SPLIT_EASU:
            // early exit condition is flattened in EASU code
            return deviceProperties.vendorID == 0x5143; // Qualcomm
        case Workaround::ALLOW_READ_ONLY_ANCILLARY_FEEDBACK_LOOP:
            return true;
    }
    return false;
}

math::float2 VulkanDriver::getClipSpaceParams() {
    // virtual and physical z-coordinate of clip-space is in [-w, 0]
    // Note: this is actually never used (see: main.vs), but it's a backend API so we implement it
    // properly.
    return math::float2{ 1.0f, 0.0f };
}

uint8_t VulkanDriver::getMaxDrawBuffers() {
    return MRT::MIN_SUPPORTED_RENDER_TARGET_COUNT; // TODO: query real value
}

void VulkanDriver::setVertexBufferObject(Handle<HwVertexBuffer> vbh, uint32_t index,
        Handle<HwBufferObject> boh) {
    auto& vb = *handle_cast<VulkanVertexBuffer*>(vbh);
    auto& bo = *handle_cast<VulkanBufferObject*>(boh);
    assert_invariant(bo.bindingType == BufferObjectBinding::VERTEX);
    vb.buffers[index] = &bo.buffer;
}

void VulkanDriver::updateIndexBuffer(Handle<HwIndexBuffer> ibh, BufferDescriptor&& p,
        uint32_t byteOffset) {
    auto ib = handle_cast<VulkanIndexBuffer*>(ibh);
    ib->buffer.loadFromCpu(mContext, mStagePool, p.buffer, byteOffset, p.size);
    mDisposer.acquire(ib);
    scheduleDestroy(std::move(p));
}

void VulkanDriver::updateBufferObject(Handle<HwBufferObject> boh, BufferDescriptor&& bd,
        uint32_t byteOffset) {
    auto bo = handle_cast<VulkanBufferObject*>(boh);
    bo->buffer.loadFromCpu(mContext, mStagePool, bd.buffer, byteOffset, bd.size);
    mDisposer.acquire(bo);
    scheduleDestroy(std::move(bd));
}

void VulkanDriver::update2DImage(Handle<HwTexture> th,
        uint32_t level, uint32_t xoffset, uint32_t yoffset, uint32_t width, uint32_t height,
        PixelBufferDescriptor&& data) {
    handle_cast<VulkanTexture*>(th)->updateImage(data, width, height, 1,
            xoffset, yoffset, 0, level);
    scheduleDestroy(std::move(data));
}

void VulkanDriver::setMinMaxLevels(Handle<HwTexture> th, uint32_t minLevel, uint32_t maxLevel) {
    handle_cast<VulkanTexture*>(th)->setPrimaryRange(minLevel, maxLevel);
}

void VulkanDriver::update3DImage(
        Handle<HwTexture> th,
        uint32_t level, uint32_t xoffset, uint32_t yoffset, uint32_t zoffset,
        uint32_t width, uint32_t height, uint32_t depth,
        PixelBufferDescriptor&& data) {
    handle_cast<VulkanTexture*>(th)->updateImage(data, width, height, depth,
            xoffset, yoffset, zoffset, level);
    scheduleDestroy(std::move(data));
}

void VulkanDriver::updateCubeImage(Handle<HwTexture> th, uint32_t level,
        PixelBufferDescriptor&& data, FaceOffsets faceOffsets) {
    handle_cast<VulkanTexture*>(th)->updateCubeImage(data, faceOffsets, level);
    scheduleDestroy(std::move(data));
}

void VulkanDriver::setupExternalImage(void* image) {
}

void VulkanDriver::cancelExternalImage(void* image) {
}

bool VulkanDriver::getTimerQueryValue(Handle<HwTimerQuery> tqh, uint64_t* elapsedTime) {
    VulkanTimerQuery* vtq = handle_cast<VulkanTimerQuery*>(tqh);

    // This is a synchronous call and might occur before beginTimerQuery has written anything into
    // the command buffer, which is an error according to the validation layer that ships in the
    // Android NDK.  Even when AVAILABILITY_BIT is set, validation seems to require that the
    // timestamp has at least been written into a processed command buffer.
    VulkanCommandBuffer const* cmdbuf = vtq->cmdbuffer.load();
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

    float period = mContext.physicalDeviceProperties.limits.timestampPeriod;
    uint64_t delta = uint64_t(float(timestamp1 - timestamp0) * period);
    *elapsedTime = delta;
    return true;
}

SyncStatus VulkanDriver::getSyncStatus(Handle<HwSync> sh) {
    VulkanSync* sync = handle_cast<VulkanSync*>(sh);
    if (sync->fence == nullptr) {
        return SyncStatus::NOT_SIGNALED;
    }
    VkResult status = sync->fence->status.load(std::memory_order_relaxed);
    switch (status) {
        case VK_SUCCESS: return SyncStatus::SIGNALED;
        case VK_INCOMPLETE: return SyncStatus::NOT_SIGNALED;
        case VK_NOT_READY: return SyncStatus::NOT_SIGNALED;
        case VK_ERROR_DEVICE_LOST: return SyncStatus::ERROR;
        default:
            // NOTE: In theory, the fence status must be one of the above values.
            return SyncStatus::ERROR;
    }
}

void VulkanDriver::setExternalImage(Handle<HwTexture> th, void* image) {
}

void VulkanDriver::setExternalImagePlane(Handle<HwTexture> th, void* image, uint32_t plane) {
}

void VulkanDriver::setExternalStream(Handle<HwTexture> th, Handle<HwStream> sh) {
}

void VulkanDriver::generateMipmaps(Handle<HwTexture> th) { }

bool VulkanDriver::canGenerateMipmaps() {
    return false;
}

void VulkanDriver::updateSamplerGroup(Handle<HwSamplerGroup> sbh,
        SamplerGroup&& samplerGroup) {
    auto* sb = handle_cast<VulkanSamplerGroup*>(sbh);
    *sb->sb = samplerGroup;
}

void VulkanDriver::beginRenderPass(Handle<HwRenderTarget> rth, const RenderPassParams& params) {
    VulkanRenderTarget* const rt = handle_cast<VulkanRenderTarget*>(rth);

    const VkExtent2D extent = rt->getExtent();
    assert_invariant(extent.width > 0 && extent.height > 0);

    // Filament has the expectation that the contents of the swap chain are not preserved on the
    // first render pass. Note however that its contents are often preserved on subsequent render
    // passes, due to multiple views.
    TargetBufferFlags discardStart = params.flags.discardStart;
    if (rt->isSwapChain()) {
        VulkanSwapChain* const sc = mContext.currentSwapChain;
        assert_invariant(sc);
        if (sc->firstRenderPass) {
            discardStart |= TargetBufferFlags::COLOR;
            sc->firstRenderPass = false;
        }
    }

    const VkCommandBuffer cmdbuffer = mContext.commands->get().cmdbuffer;
    VulkanAttachment depth = rt->getSamples() == 1 ? rt->getDepth() : rt->getMsaaDepth();

    VulkanDepthLayout initialDepthLayout = fromVkImageLayout(depth.getLayout());
    VulkanDepthLayout renderPassDepthLayout =
            fromVkImageLayout(getDefaultImageLayout(TextureUsage::DEPTH_ATTACHMENT));
    VulkanDepthLayout finalDepthLayout = renderPassDepthLayout;

    // Sometimes we need to permit the shader to sample the depth attachment by transitioning the
    // layout of all its subresources to a read-only layout. This is especially crucial for SSAO.
    //
    // We cannot perform this transition using the render pass because the shaders in this render
    // pass might sample from multiple miplevels.
    //
    // We do not use GENERAL here due to the following validation message:
    //
    //   The Vulkan spec states: Image subresources used as attachments in the current render pass
    //   must not be accessed in any way other than as an attachment by this command, except for
    //   cases involving read-only access to depth/stencil attachments as described in the Render
    //   Pass chapter.
    //
    // https://vulkan.lunarg.com/doc/view/1.2.182.0/mac/1.2-extensions/vkspec.html#VUID-vkCmdDrawIndexed-None-04584)
    //
    if (params.readOnlyDepthStencil & RenderPassParams::READONLY_DEPTH) {
        VkImageSubresourceRange range = {
            .aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT,
            .baseMipLevel = 0,
            .levelCount = depth.texture->levels,
            .baseArrayLayer = 0,
            .layerCount = depth.texture->depth,
        };
        depth.texture->transitionLayout(cmdbuffer, range, VK_IMAGE_LAYOUT_DEPTH_STENCIL_READ_ONLY_OPTIMAL);
        initialDepthLayout = renderPassDepthLayout = finalDepthLayout = VulkanDepthLayout::READ_ONLY;
    }

    if (depth.texture) {
        depth.texture->trackLayout(depth.level, depth.layer, toVkImageLayout(renderPassDepthLayout));
    }

    // Create the VkRenderPass or fetch it from cache.
    VulkanFboCache::RenderPassKey rpkey = {
        .initialColorLayoutMask = 0,
        .initialDepthLayout = initialDepthLayout,
        .renderPassDepthLayout = renderPassDepthLayout,
        .finalDepthLayout = finalDepthLayout,
        .depthFormat = depth.getFormat(),
        .clear = params.flags.clear,
        .discardStart = discardStart,
        .discardEnd = params.flags.discardEnd,
        .samples = rt->getSamples(),
        .subpassMask = uint8_t(params.subpassMask),
    };
    for (int i = 0; i < MRT::MAX_SUPPORTED_RENDER_TARGET_COUNT; i++) {
        const VulkanAttachment& info = rt->getColor(i);
        if (info.texture) {
            rpkey.initialColorLayoutMask |= 1 << i;
            info.texture->trackLayout(info.level, info.layer,
                    getDefaultImageLayout(TextureUsage::COLOR_ATTACHMENT));
            rpkey.colorFormat[i] = info.getFormat();
            if (rpkey.samples > 1 && info.texture->samples == 1) {
                rpkey.needsResolveMask |= (1 << i);
            }
        } else {
            rpkey.colorFormat[i] = VK_FORMAT_UNDEFINED;
        }
    }

    VkRenderPass renderPass = mFramebufferCache.getRenderPass(rpkey);
    mPipelineCache.bindRenderPass(renderPass, 0);

    // Create the VkFramebuffer or fetch it from cache.
    VulkanFboCache::FboKey fbkey {
        .renderPass = renderPass,
        .width = (uint16_t) extent.width,
        .height = (uint16_t) extent.height,
        .layers = 1,
        .samples = rpkey.samples,
    };
    for (int i = 0; i < MRT::MAX_SUPPORTED_RENDER_TARGET_COUNT; i++) {
        if (!rt->getColor(i).texture) {
            fbkey.color[i] = VK_NULL_HANDLE;
            fbkey.resolve[i] = VK_NULL_HANDLE;
        } else if (fbkey.samples == 1) {
            fbkey.color[i] = rt->getColor(i).getImageView(VK_IMAGE_ASPECT_COLOR_BIT);
            fbkey.resolve[i] = VK_NULL_HANDLE;
            assert_invariant(fbkey.color[i]);
        } else {
            fbkey.color[i] = rt->getMsaaColor(i).getImageView(VK_IMAGE_ASPECT_COLOR_BIT);
            VulkanTexture* texture = rt->getColor(i).texture;
            if (texture->samples == 1) {
                fbkey.resolve[i] = rt->getColor(i).getImageView(VK_IMAGE_ASPECT_COLOR_BIT);
                assert_invariant(fbkey.resolve[i]);
            }
            assert_invariant(fbkey.color[i]);
        }
    }
    if (depth.texture) {
        fbkey.depth = depth.getImageView(VK_IMAGE_ASPECT_DEPTH_BIT);
        assert_invariant(fbkey.depth);

        // Vulkan 1.1 does not support multisampled depth resolve, so let's check here
        // and assert if this is requested. (c.f. isAutoDepthResolveSupported)
        // Reminder: Filament's backend API works like this:
        // - If the RT is SS then all attachments must be SS.
        // - If the RT is MS then all SS attachments are auto resolved if not discarded.
        assert_invariant(!(rt->getSamples() > 1 &&
                rt->getDepth().texture->samples == 1 &&
                !any(rpkey.discardEnd & TargetBufferFlags::DEPTH)));
    }
    VkFramebuffer vkfb = mFramebufferCache.getFramebuffer(fbkey);

    // Assign a label to the framebuffer for debugging purposes.
    if (UTILS_UNLIKELY(mContext.debugUtilsSupported) && !mContext.currentDebugMarker.empty()) {
        const VkDebugUtilsObjectNameInfoEXT info = {
            VK_STRUCTURE_TYPE_DEBUG_UTILS_OBJECT_NAME_INFO_EXT,
            nullptr,
            VK_OBJECT_TYPE_FRAMEBUFFER,
            reinterpret_cast<uint64_t>(vkfb),
            mContext.currentDebugMarker.c_str(),
        };
        vkSetDebugUtilsObjectNameEXT(mContext.device, &info);
    }

    // The current command buffer now owns a reference to the render target and its attachments.
    // Note that we must acquire parent textures, not sidecars.
    mDisposer.acquire(rt);
    mDisposer.acquire(rt->getDepth().texture);
    for (int i = 0; i < MRT::MAX_SUPPORTED_RENDER_TARGET_COUNT; i++) {
        mDisposer.acquire(rt->getColor(i).texture);
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

    VkClearValue clearValues[MRT::MAX_SUPPORTED_RENDER_TARGET_COUNT + MRT::MAX_SUPPORTED_RENDER_TARGET_COUNT + 1] = {};

    // NOTE: clearValues must be populated in the same order as the attachments array in
    // VulkanFboCache::getFramebuffer. Values must be provided regardless of whether Vulkan is
    // actually clearing that particular target.
    for (int i = 0; i < MRT::MAX_SUPPORTED_RENDER_TARGET_COUNT; i++) {
        if (fbkey.color[i]) {
            VkClearValue& clearValue = clearValues[renderPassInfo.clearValueCount++];
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
        VkClearValue& clearValue = clearValues[renderPassInfo.clearValueCount++];
        clearValue.depthStencil = {(float) params.clearDepth, 0};
    }
    renderPassInfo.pClearValues = &clearValues[0];

    vkCmdBeginRenderPass(cmdbuffer, &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);

    VkViewport viewport = mContext.viewport = {
        .x = (float) params.viewport.left,
        .y = (float) params.viewport.bottom,
        .width = (float) params.viewport.width,
        .height = (float) params.viewport.height,
        .minDepth = params.depthRange.near,
        .maxDepth = params.depthRange.far
    };

    rt->transformClientRectToPlatform(&viewport);
    vkCmdSetViewport(cmdbuffer, 0, 1, &viewport);

    mContext.currentRenderPass = {
        .renderTarget = rt,
        .renderPass = renderPassInfo.renderPass,
        .params = params,
        .currentSubpass = 0,
    };
}

void VulkanDriver::endRenderPass(int) {
    VkCommandBuffer cmdbuffer = mContext.commands->get().cmdbuffer;
    vkCmdEndRenderPass(cmdbuffer);

    VulkanRenderTarget* rt = mContext.currentRenderPass.renderTarget;
    assert_invariant(rt);

    // In some cases, depth needs to be transitioned from DEPTH_STENCIL_READ_ONLY_OPTIMAL back to
    // GENERAL. We did not do this using the render pass because we need to change multiple mips.
    if (mContext.currentRenderPass.params.readOnlyDepthStencil & RenderPassParams::READONLY_DEPTH) {
        const VulkanAttachment& depth = rt->getDepth();
        VkImageSubresourceRange range = {
            .aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT,
            .baseMipLevel = 0,
            .levelCount = depth.texture->levels,
            .baseArrayLayer = 0,
            .layerCount = depth.texture->depth,
        };
        depth.texture->transitionLayout(cmdbuffer, range,
                getDefaultImageLayout(TextureUsage::DEPTH_ATTACHMENT));
    }

    // Since we might soon be sampling from the render target that we just wrote to, we need a
    // pipeline barrier between framebuffer writes and shader reads. This is a memory barrier rather
    // than an image barrier. If we were to use image barriers here, we would potentially need to
    // issue several of them when considering MRT. This would be very complex to set up and would
    // require more state tracking, so we've chosen to use a memory barrier for simplicity and
    // correctness.

    // NOTE: ideally dstStageMask would merely be VERTEX_SHADER_BIT | FRAGMENT_SHADER_BIT, but this
    // seems to be insufficient on Mali devices. To work around this we are adding a more aggressive
    // TOP_OF_PIPE barrier.

    if (!rt->isSwapChain()) {
        VkMemoryBarrier barrier {
            .sType = VK_STRUCTURE_TYPE_MEMORY_BARRIER,
            .srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT,
            .dstAccessMask = VK_ACCESS_SHADER_READ_BIT,
        };
        VkPipelineStageFlags srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
        if (rt->hasDepth()) {
            barrier.srcAccessMask |= VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
            srcStageMask |= VK_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT;
        }
        vkCmdPipelineBarrier(cmdbuffer, srcStageMask,
                VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT | // <== For Mali
                VK_PIPELINE_STAGE_VERTEX_SHADER_BIT | VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT,
                0, 1, &barrier, 0, nullptr, 0, nullptr);
    }

    if (mContext.currentRenderPass.currentSubpass > 0) {
        for (uint32_t i = 0; i < VulkanPipelineCache::TARGET_BINDING_COUNT; i++) {
            mPipelineCache.bindInputAttachment(i, {});
        }
        mContext.currentRenderPass.currentSubpass = 0;
    }
    mContext.currentRenderPass.renderTarget = nullptr;
    mContext.currentRenderPass.renderPass = VK_NULL_HANDLE;
}

void VulkanDriver::nextSubpass(int) {
    ASSERT_PRECONDITION(mContext.currentRenderPass.currentSubpass == 0,
            "Only two subpasses are currently supported.");

    VulkanRenderTarget* renderTarget = mContext.currentRenderPass.renderTarget;
    assert_invariant(renderTarget);
    assert_invariant(mContext.currentRenderPass.params.subpassMask);

    vkCmdNextSubpass(mContext.commands->get().cmdbuffer, VK_SUBPASS_CONTENTS_INLINE);

    mPipelineCache.bindRenderPass(mContext.currentRenderPass.renderPass,
            ++mContext.currentRenderPass.currentSubpass);

    for (uint32_t i = 0; i < VulkanPipelineCache::TARGET_BINDING_COUNT; i++) {
        if ((1 << i) & mContext.currentRenderPass.params.subpassMask) {
            VulkanAttachment subpassInput = renderTarget->getColor(i);
            VkDescriptorImageInfo info = {
                .imageView = subpassInput.getImageView(VK_IMAGE_ASPECT_COLOR_BIT),
                .imageLayout = subpassInput.getLayout(),
            };
            mPipelineCache.bindInputAttachment(i, info);
        }
    }
}

void VulkanDriver::setRenderPrimitiveBuffer(Handle<HwRenderPrimitive> rph,
        Handle<HwVertexBuffer> vbh, Handle<HwIndexBuffer> ibh) {
    auto primitive = handle_cast<VulkanRenderPrimitive*>(rph);
    primitive->setBuffers(handle_cast<VulkanVertexBuffer*>(vbh),
            handle_cast<VulkanIndexBuffer*>(ibh));
}

void VulkanDriver::setRenderPrimitiveRange(Handle<HwRenderPrimitive> rph,
        PrimitiveType pt, uint32_t offset,
        uint32_t minIndex, uint32_t maxIndex, uint32_t count) {
    auto& primitive = *handle_cast<VulkanRenderPrimitive*>(rph);
    primitive.setPrimitiveType(pt);
    primitive.offset = offset * primitive.indexBuffer->elementSize;
    primitive.count = count;
    primitive.minIndex = minIndex;
    primitive.maxIndex = maxIndex > minIndex ? maxIndex : primitive.maxVertexCount - 1;
}

void VulkanDriver::makeCurrent(Handle<HwSwapChain> drawSch, Handle<HwSwapChain> readSch) {
    ASSERT_PRECONDITION_NON_FATAL(drawSch == readSch,
                                  "Vulkan driver does not support distinct draw/read swap chains.");
    VulkanSwapChain& swapChain = *handle_cast<VulkanSwapChain*>(drawSch);
    mContext.currentSwapChain = &swapChain;

    // Leave early if the swap chain image has already been acquired but not yet presented.
    if (swapChain.acquired) {
        if (UTILS_LIKELY(mContext.defaultRenderTarget)) {
            mContext.defaultRenderTarget->bindToSwapChain(swapChain);
        }
        return;
    }

    // Query the surface caps to see if it has been resized.  This handles not just resized windows,
    // but also screen rotation on Android and dragging between low DPI and high DPI monitors.
    if (swapChain.hasResized()) {
        refreshSwapChain();
    }

    // Call vkAcquireNextImageKHR and insert its signal semaphore into the command manager's
    // dependency chain.
    swapChain.acquire();

    if (UTILS_LIKELY(mContext.defaultRenderTarget)) {
        mContext.defaultRenderTarget->bindToSwapChain(swapChain);
    }
}

void VulkanDriver::commit(Handle<HwSwapChain> sch) {
    VulkanSwapChain& swapChain = *handle_cast<VulkanSwapChain*>(sch);

    // Before swapping, transition the current swap chain image to the PRESENT layout. This cannot
    // be done as part of the render pass because it does not know if it is last pass in the frame.
    swapChain.makePresentable();

    if (mContext.commands->flush()) {
        collectGarbage();
    }

    swapChain.firstRenderPass = true;

    if (swapChain.headlessQueue) {
        return;
    }

    swapChain.acquired = false;

    // Present the backbuffer after the most recent command buffer submission has finished.
    VkSemaphore renderingFinished = mContext.commands->acquireFinishedSignal();
    uint32_t currentSwapIndex = swapChain.getSwapIndex();
    VkPresentInfoKHR presentInfo {
        .sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR,
        .waitSemaphoreCount = 1,
        .pWaitSemaphores = &renderingFinished,
        .swapchainCount = 1,
        .pSwapchains = &swapChain.swapchain,
        .pImageIndices = &currentSwapIndex,
    };
    VkResult result = vkQueuePresentKHR(swapChain.presentQueue, &presentInfo);

    // On Android Q and above, a suboptimal surface is always reported after screen rotation:
    // https://android-developers.googleblog.com/2020/02/handling-device-orientation-efficiently.html
    if (result == VK_SUBOPTIMAL_KHR && !swapChain.suboptimal) {
        utils::slog.w << "Vulkan Driver: Suboptimal swap chain." << utils::io::endl;
        swapChain.suboptimal = true;
    }

    // The surface can be "out of date" when it has been resized, which is not an error.
    assert_invariant(result == VK_SUCCESS || result == VK_SUBOPTIMAL_KHR ||
            result == VK_ERROR_OUT_OF_DATE_KHR);
}

void VulkanDriver::bindUniformBuffer(uint32_t index, Handle<HwBufferObject> boh) {
    auto* bo = handle_cast<VulkanBufferObject*>(boh);
    const VkDeviceSize offset = 0;
    const VkDeviceSize size = VK_WHOLE_SIZE;
    mPipelineCache.bindUniformBuffer((uint32_t) index, bo->buffer.getGpuBuffer(), offset, size);
}

void VulkanDriver::bindUniformBufferRange(uint32_t index, Handle<HwBufferObject> boh,
        uint32_t offset, uint32_t size) {
    auto* bo = handle_cast<VulkanBufferObject*>(boh);
    mPipelineCache.bindUniformBuffer((uint32_t)index, bo->buffer.getGpuBuffer(), offset, size);
}

void VulkanDriver::bindSamplers(uint32_t index, Handle<HwSamplerGroup> sbh) {
    auto* hwsb = handle_cast<VulkanSamplerGroup*>(sbh);
    mSamplerBindings[index] = hwsb;
}

void VulkanDriver::insertEventMarker(char const* string, uint32_t len) {
    constexpr float MARKER_COLOR[] = { 0.0f, 1.0f, 0.0f, 1.0f };
    const VkCommandBuffer cmdbuffer = mContext.commands->get().cmdbuffer;
    if (mContext.debugUtilsSupported) {
        VkDebugUtilsLabelEXT labelInfo = {
            .sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_LABEL_EXT,
            .pLabelName = string,
            .color = {1, 1, 0, 1},
        };
        vkCmdInsertDebugUtilsLabelEXT(cmdbuffer, &labelInfo);
    } else if (mContext.debugMarkersSupported) {
        VkDebugMarkerMarkerInfoEXT markerInfo = {};
        markerInfo.sType = VK_STRUCTURE_TYPE_DEBUG_MARKER_MARKER_INFO_EXT;
        memcpy(markerInfo.color, &MARKER_COLOR[0], sizeof(MARKER_COLOR));
        markerInfo.pMarkerName = string;
        vkCmdDebugMarkerInsertEXT(cmdbuffer, &markerInfo);
    }
}

void VulkanDriver::pushGroupMarker(char const* string, uint32_t len) {
    // TODO: Add group marker color to the Driver API
    constexpr float MARKER_COLOR[] = { 0.0f, 1.0f, 0.0f, 1.0f };
    const VkCommandBuffer cmdbuffer = mContext.commands->get().cmdbuffer;
    if (mContext.debugUtilsSupported) {
        VkDebugUtilsLabelEXT labelInfo = {
            .sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_LABEL_EXT,
            .pLabelName = string,
            .color = {0, 1, 0, 1},
        };
        vkCmdBeginDebugUtilsLabelEXT(cmdbuffer, &labelInfo);
        mContext.currentDebugMarker = string;
    } else if (mContext.debugMarkersSupported) {
        VkDebugMarkerMarkerInfoEXT markerInfo = {};
        markerInfo.sType = VK_STRUCTURE_TYPE_DEBUG_MARKER_MARKER_INFO_EXT;
        memcpy(markerInfo.color, &MARKER_COLOR[0], sizeof(MARKER_COLOR));
        markerInfo.pMarkerName = string;
        vkCmdDebugMarkerBeginEXT(cmdbuffer, &markerInfo);
    }
}

void VulkanDriver::popGroupMarker(int) {
    const VkCommandBuffer cmdbuffer = mContext.commands->get().cmdbuffer;
    if (mContext.debugUtilsSupported) {
        vkCmdEndDebugUtilsLabelEXT(cmdbuffer);
        mContext.currentDebugMarker.clear();
    } else if (mContext.debugMarkersSupported) {
        vkCmdDebugMarkerEndEXT(cmdbuffer);
    }
}

void VulkanDriver::startCapture(int) {

}

void VulkanDriver::stopCapture(int) {

}

void VulkanDriver::readPixels(Handle<HwRenderTarget> src, uint32_t x, uint32_t y,
        uint32_t width, uint32_t height, PixelBufferDescriptor&& pbd) {
    const VkDevice device = mContext.device;
    VulkanRenderTarget* srcTarget = handle_cast<VulkanRenderTarget*>(src);
    VulkanTexture* srcTexture = srcTarget->getColor(0).texture;
    assert_invariant(srcTexture);
    const VkFormat srcFormat = srcTexture->getVkFormat();
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
        .memoryTypeIndex = mContext.selectMemoryType(memReqs.memoryTypeBits,
                VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT)
    };

    vkAllocateMemory(device, &allocInfo, nullptr, &stagingMemory);
    vkBindImageMemory(device, stagingImage, stagingMemory, 0);

    // TODO: don't flush/wait here, this should be asynchronous

    mContext.commands->flush();
    mContext.commands->wait();

    // Transition the staging image layout.

    const VkCommandBuffer cmdbuffer = mContext.commands->get().cmdbuffer;

    transitionImageLayout(cmdbuffer, {
        .image = stagingImage,
        .oldLayout = VK_IMAGE_LAYOUT_UNDEFINED,
        .newLayout = VK_IMAGE_LAYOUT_GENERAL,
        .subresources = {
            .aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
            .baseMipLevel = 0,
            .levelCount = 1,
            .baseArrayLayer = 0,
            .layerCount = 1,
        },
        .srcStage = VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT,
        .srcAccessMask = 0,
        .dstStage = VK_PIPELINE_STAGE_TRANSFER_BIT,
        .dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT,
    });

    const VulkanAttachment srcAttachment = srcTarget->getColor(0);

    VkImageCopy imageCopyRegion = {
        .srcSubresource = {
            .aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
            .mipLevel = srcAttachment.level,
            .baseArrayLayer = srcAttachment.layer,
            .layerCount = 1,
        },
        .srcOffset = {
            .x = (int32_t) x,
            .y = (int32_t) (srcTarget->getExtent().height - (height + y)),
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

    const VkImageSubresourceRange srcRange = {
        .aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
        .baseMipLevel = srcAttachment.level,
        .levelCount = 1,
        .baseArrayLayer = srcAttachment.layer,
        .layerCount = 1,
    };

    srcTexture->transitionLayout(cmdbuffer, srcRange, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL);

    // Perform the copy into the staging area. At this point we know that the src layout is
    // TRANSFER_SRC_OPTIMAL and the staging area is GENERAL.

    UTILS_UNUSED_IN_RELEASE VkExtent2D srcExtent = srcAttachment.getExtent2D();
    assert_invariant(imageCopyRegion.srcOffset.x + imageCopyRegion.extent.width <= srcExtent.width);
    assert_invariant(imageCopyRegion.srcOffset.y + imageCopyRegion.extent.height <= srcExtent.height);

    vkCmdCopyImage(cmdbuffer, srcAttachment.getImage(),
            VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL, stagingImage, VK_IMAGE_LAYOUT_GENERAL,
            1, &imageCopyRegion);

    // Restore the source image layout. Between driver API calls, color images are always kept in
    // UNDEFINED layout or in their "usage default" layout (see comment for getDefaultImageLayout).

    srcTexture->transitionLayout(cmdbuffer, srcRange,
            getDefaultImageLayout(TextureUsage::COLOR_ATTACHMENT));

    // TODO: don't flush/wait here -- we should do this asynchronously

    // Flush and wait.
    mContext.commands->flush();
    mContext.commands->wait();

    VkImageSubresource subResource { .aspectMask = VK_IMAGE_ASPECT_COLOR_BIT };
    VkSubresourceLayout subResourceLayout;
    vkGetImageSubresourceLayout(device, stagingImage, &subResource, &subResourceLayout);

    // Map image memory so we can start copying from it.

    const uint8_t* srcPixels;
    vkMapMemory(device, stagingMemory, 0, VK_WHOLE_SIZE, 0, (void**) &srcPixels);
    srcPixels += subResourceLayout.offset;

    // NOTE: the reasons for this are unclear, but issuing ReadPixels on a VkImage that has been
    // extracted from a swap chain does not need a Y flip, but explicitly created VkImages do. (The
    // former can be tested with "Export Screenshots" in gltf_viewer, the latter can be tested with
    // test_ReadPixels.cpp). We've seen this behavior with both SwiftShader and MoltenVK.
    const bool flipY = !srcTarget->isSwapChain();

    if (!DataReshaper::reshapeImage(&pbd, getComponentType(srcFormat), srcPixels,
            subResourceLayout.rowPitch, width, height, swizzle, flipY)) {
        utils::slog.e << "Unsupported PixelDataFormat or PixelDataType" << utils::io::endl;
    }

    vkUnmapMemory(device, stagingMemory);

    mDisposer.createDisposable((void*)stagingImage, [=] () {
        vkDestroyImage(device, stagingImage, nullptr);
        vkFreeMemory(device, stagingMemory, nullptr);
    });

    scheduleDestroy(std::move(pbd));
}

void VulkanDriver::blit(TargetBufferFlags buffers, Handle<HwRenderTarget> dst, Viewport dstRect,
        Handle<HwRenderTarget> src, Viewport srcRect, SamplerMagFilter filter) {
    assert_invariant(mContext.currentRenderPass.renderPass == VK_NULL_HANDLE);

    // blit operation only support COLOR0 color buffer
    assert_invariant(
            !(buffers & (TargetBufferFlags::COLOR_ALL & ~TargetBufferFlags::COLOR0)));

    if (UTILS_UNLIKELY(mContext.currentRenderPass.renderPass)) {
        utils::slog.e << "Blits cannot be invoked inside a render pass." << utils::io::endl;
        return;
    }

    VulkanRenderTarget* dstTarget = handle_cast<VulkanRenderTarget*>(dst);
    VulkanRenderTarget* srcTarget = handle_cast<VulkanRenderTarget*>(src);

    VkFilter vkfilter = filter == SamplerMagFilter::NEAREST ? VK_FILTER_NEAREST : VK_FILTER_LINEAR;

    const VkExtent2D srcExtent = srcTarget->getExtent();
    const int32_t srcLeft = std::min(srcRect.left, (int32_t) srcExtent.width);
    const int32_t srcBottom = std::min(srcRect.bottom, (int32_t) srcExtent.height);
    const int32_t srcRight = std::min(srcRect.left + srcRect.width, srcExtent.width);
    const int32_t srcTop = std::min(srcRect.bottom + srcRect.height, srcExtent.height);
    const VkOffset3D srcOffsets[2] = { { srcLeft, srcBottom, 0 }, { srcRight, srcTop, 1 }};

    const VkExtent2D dstExtent = dstTarget->getExtent();
    const int32_t dstLeft = std::min(dstRect.left, (int32_t) dstExtent.width);
    const int32_t dstBottom = std::min(dstRect.bottom, (int32_t) dstExtent.height);
    const int32_t dstRight = std::min(dstRect.left + dstRect.width, dstExtent.width);
    const int32_t dstTop = std::min(dstRect.bottom + dstRect.height, dstExtent.height);
    const VkOffset3D dstOffsets[2] = { { dstLeft, dstBottom, 0 }, { dstRight, dstTop, 1 }};

    if (any(buffers & TargetBufferFlags::DEPTH) && srcTarget->hasDepth() && dstTarget->hasDepth()) {
        mBlitter.blitDepth({dstTarget, dstOffsets, srcTarget, srcOffsets});
    }

    if (any(buffers & TargetBufferFlags::COLOR0)) {
        mBlitter.blitColor({ dstTarget, dstOffsets, srcTarget, srcOffsets, vkfilter, int(0) });
    }
}

void VulkanDriver::draw(PipelineState pipelineState, Handle<HwRenderPrimitive> rph,
        const uint32_t instanceCount) {
    VulkanCommandBuffer const* commands = &mContext.commands->get();
    VkCommandBuffer cmdbuffer = commands->cmdbuffer;
    const VulkanRenderPrimitive& prim = *handle_cast<VulkanRenderPrimitive*>(rph);

    Handle<HwProgram> programHandle = pipelineState.program;
    RasterState rasterState = pipelineState.rasterState;
    PolygonOffset depthOffset = pipelineState.polygonOffset;
    const Viewport& viewportScissor = pipelineState.scissor;

    auto* program = handle_cast<VulkanProgram*>(programHandle);
    mDisposer.acquire(program);
    mDisposer.acquire(prim.indexBuffer);
    mDisposer.acquire(prim.vertexBuffer);

    // If this is a debug build, validate the current shader.
#if !defined(NDEBUG)
    if (program->bundle.vertex == VK_NULL_HANDLE || program->bundle.fragment == VK_NULL_HANDLE) {
        utils::slog.e << "Binding missing shader: " << program->name.c_str() << utils::io::endl;
    }
#endif

    // Update the VK raster state.

    const VulkanRenderTarget* rt = mContext.currentRenderPass.renderTarget;

    auto& vkraster = mContext.rasterState;
    vkraster.cullMode = getCullMode(rasterState.culling);
    vkraster.frontFace = getFrontFace(rasterState.inverseFrontFaces);
    vkraster.depthBiasEnable = (depthOffset.constant || depthOffset.slope) ? true : false;
    vkraster.depthBiasConstantFactor = depthOffset.constant;
    vkraster.depthBiasSlopeFactor = depthOffset.slope;
    vkraster.blendEnable = rasterState.hasBlending();
    vkraster.srcColorBlendFactor = getBlendFactor(rasterState.blendFunctionSrcRGB);
    vkraster.dstColorBlendFactor = getBlendFactor(rasterState.blendFunctionDstRGB);
    vkraster.colorBlendOp = rasterState.blendEquationRGB;
    vkraster.srcAlphaBlendFactor = getBlendFactor(rasterState.blendFunctionSrcAlpha);
    vkraster.dstAlphaBlendFactor = getBlendFactor(rasterState.blendFunctionDstAlpha);
    vkraster.alphaBlendOp =  rasterState.blendEquationAlpha;
    vkraster.colorWriteMask = (VkColorComponentFlags) (rasterState.colorWrite ? 0xf : 0x0);
    vkraster.depthWriteEnable = rasterState.depthWrite;
    vkraster.depthCompareOp = rasterState.depthFunc;
    vkraster.rasterizationSamples = rt->getSamples();
    vkraster.alphaToCoverageEnable = rasterState.alphaToCoverage;
    vkraster.colorTargetCount = rt->getColorTargetCount(mContext.currentRenderPass);

    // Declare fixed-size arrays that get passed to the pipeCache and to vkCmdBindVertexBuffers.
    VulkanPipelineCache::VertexArray varray = {};
    VkBuffer buffers[MAX_VERTEX_ATTRIBUTE_COUNT] = {};
    VkDeviceSize offsets[MAX_VERTEX_ATTRIBUTE_COUNT] = {};

    // For each attribute, append to each of the above lists.
    const uint32_t bufferCount = prim.vertexBuffer->attributes.size();
    for (uint32_t attribIndex = 0; attribIndex < bufferCount; attribIndex++) {
        Attribute attrib = prim.vertexBuffer->attributes[attribIndex];

        const bool isInteger = attrib.flags & Attribute::FLAG_INTEGER_TARGET;
        const bool isNormalized = attrib.flags & Attribute::FLAG_NORMALIZED;

        VkFormat vkformat = getVkFormat(attrib.type, isNormalized, isInteger);

        // HACK: Re-use the positions buffer as a dummy buffer for disabled attributes. Filament's
        // vertex shaders declare all attributes as either vec4 or uvec4 (the latter for bone
        // indices), and positions are always at least 32 bits per element. Therefore we can assign
        // a dummy type of either R8G8B8A8_UINT or R8G8B8A8_SNORM, depending on whether the shader
        // expects to receive floats or ints.
        if (attrib.buffer == Attribute::BUFFER_UNUSED) {
            vkformat = isInteger ? VK_FORMAT_R8G8B8A8_UINT : VK_FORMAT_R8G8B8A8_SNORM;
            attrib = prim.vertexBuffer->attributes[0];
        }

        const VulkanBuffer* buffer = prim.vertexBuffer->buffers[attrib.buffer];

        // If the vertex buffer is missing a constituent buffer object, skip the draw call.
        // There is no need to emit an error message because this is not explicitly forbidden.
        if (buffer == nullptr) {
            return;
        }

        buffers[attribIndex] = buffer->getGpuBuffer();
        offsets[attribIndex] = attrib.offset;
        varray.attributes[attribIndex] = {
            .location = attribIndex, // matches the GLSL layout specifier
            .binding = attribIndex,  // matches the position within vkCmdBindVertexBuffers
            .format = vkformat,
        };
        varray.buffers[attribIndex] = {
            .binding = attribIndex,
            .stride = attrib.stride,
        };
    }

    // Push state changes to the VulkanPipelineCache instance. This is fast and does not make VK calls.
    mPipelineCache.bindProgram(*program);
    mPipelineCache.bindRasterState(mContext.rasterState);
    mPipelineCache.bindPrimitiveTopology(prim.primitiveTopology);
    mPipelineCache.bindVertexArray(varray);

    // Query the program for the mapping from (SamplerGroupBinding,Offset) to (SamplerBinding),
    // where "SamplerBinding" is the integer in the GLSL, and SamplerGroupBinding is the abstract
    // Filament concept used to form groups of samplers.

    VkDescriptorImageInfo iInfo[VulkanPipelineCache::SAMPLER_BINDING_COUNT] = {};

    for (uint8_t samplerGroupIdx = 0; samplerGroupIdx < Program::BINDING_COUNT; samplerGroupIdx++) {
        const auto& samplerGroup = program->samplerGroupInfo[samplerGroupIdx];
        const auto& samplers = samplerGroup.samplers;
        if (samplers.empty()) {
            continue;
        }
        VulkanSamplerGroup* vksb = mSamplerBindings[samplerGroupIdx];
        if (!vksb) {
            continue;
        }
        SamplerGroup* sb = vksb->sb.get();
        assert_invariant(sb->getSize() == samplers.size());
        size_t samplerIdx = 0;
        for (auto& sampler : samplers) {
            size_t bindingPoint = sampler.binding;
            const SamplerGroup::Sampler* boundSampler = sb->getSamplers() + samplerIdx;
            samplerIdx++;

            // Note that we always use a 2D texture for the fallback texture, which might not be
            // appropriate. The fallback improves robustness but does not guarantee 100% success.
            // It can be argued that clients are being malfeasant here anyway, since Vulkan does
            // not allow sampling from a non-bound texture.
            VulkanTexture* texture;
            if (UTILS_UNLIKELY(!boundSampler->t)) {
                if (!sampler.strict) {
                    continue;
                }
                utils::slog.w << "No texture bound to '" << sampler.name.c_str() << "'";
#ifndef NDEBUG
                utils::slog.w << " in material '" << program->name.c_str() << "'";
#endif
                utils::slog.w << " at binding point " << +bindingPoint << utils::io::endl;
                texture = mContext.emptyTexture;
            } else {
                texture = handle_cast<VulkanTexture*>(boundSampler->t);
                mDisposer.acquire(texture);
            }

            if (UTILS_UNLIKELY(texture->getPrimaryImageLayout() == VK_IMAGE_LAYOUT_UNDEFINED)) {
#ifndef NDEBUG
                utils::slog.w << "Uninitialized texture bound to '" << sampler.name.c_str() << "'";
                utils::slog.w << " in material '" << program->name.c_str() << "'";
                utils::slog.w << " at binding point " << +bindingPoint << utils::io::endl;
#endif
                texture = mContext.emptyTexture;
            }

            const SamplerParams& samplerParams = boundSampler->s;
            VkSampler vksampler = mSamplerCache.getSampler(samplerParams);

            iInfo[bindingPoint] = {
                .sampler = vksampler,
                .imageView = texture->getPrimaryImageView(),
                .imageLayout = texture->getPrimaryImageLayout()
            };
        }
    }

    mPipelineCache.bindSamplers(iInfo);

    // Bind new descriptor sets if they need to change.
    // If descriptor set allocation failed, skip the draw call and bail. No need to emit an error
    // message since the validation layers already do so.
    if (!mPipelineCache.bindDescriptors(cmdbuffer)) {
        return;
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
    mPipelineCache.bindScissor(cmdbuffer, scissor);

    // Bind a new pipeline if the pipeline state changed.
    // If allocation failed, skip the draw call and bail. We do not emit an error since the
    // validation layer will already do so.
    if (!mPipelineCache.bindPipeline(cmdbuffer)) {
        return;
    }

    // Next bind the vertex buffers and index buffer. One potential performance improvement is to
    // avoid rebinding these if they are already bound, but since we do not (yet) support subranges
    // it would be rare for a client to make consecutive draw calls with the same render primitive.
    vkCmdBindVertexBuffers(cmdbuffer, 0, bufferCount, buffers, offsets);
    vkCmdBindIndexBuffer(cmdbuffer, prim.indexBuffer->buffer.getGpuBuffer(), 0,
            prim.indexBuffer->indexType);

    // Finally, make the actual draw call. TODO: support subranges
    const uint32_t indexCount = prim.count;
    const uint32_t firstIndex = prim.offset / prim.indexBuffer->elementSize;
    const int32_t vertexOffset = 0;
    const uint32_t firstInstId = 1;
    vkCmdDrawIndexed(cmdbuffer, indexCount, instanceCount, firstIndex, vertexOffset, firstInstId);
}

void VulkanDriver::beginTimerQuery(Handle<HwTimerQuery> tqh) {
    VulkanCommandBuffer const* commands = &mContext.commands->get();
    VulkanTimerQuery* vtq = handle_cast<VulkanTimerQuery*>(tqh);
    const uint32_t index = vtq->startingQueryIndex;
    const VkPipelineStageFlagBits stage = VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT;

    vkCmdResetQueryPool(commands->cmdbuffer, mContext.timestamps.pool, index, 2);
    vkCmdWriteTimestamp(commands->cmdbuffer, stage, mContext.timestamps.pool, index);
    vtq->cmdbuffer.store(commands);
}

void VulkanDriver::endTimerQuery(Handle<HwTimerQuery> tqh) {
    VulkanCommandBuffer const* commands = &mContext.commands->get();
    VulkanTimerQuery* vtq = handle_cast<VulkanTimerQuery*>(tqh);
    const uint32_t index = vtq->stoppingQueryIndex;
    const VkPipelineStageFlagBits stage = VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT;
    vkCmdWriteTimestamp(commands->cmdbuffer, stage, mContext.timestamps.pool, index);
}

void VulkanDriver::refreshSwapChain() {
    VulkanSwapChain& swapChain = *mContext.currentSwapChain;

    assert_invariant(!swapChain.headlessQueue && "Resizing headless swap chains is not supported.");
    swapChain.destroy();
    swapChain.create(mStagePool);

    mFramebufferCache.reset();
}

void VulkanDriver::debugCommandBegin(CommandStream* cmds, bool synchronous, const char* methodName) noexcept {
    DriverBase::debugCommandBegin(cmds, synchronous, methodName);
#ifndef NDEBUG
    static const std::set<utils::StaticString> OUTSIDE_COMMANDS = {
        "loadUniformBuffer",
        "updateBufferObject",
        "updateIndexBuffer",
        "update2DImage",
        "updateCubeImage",
    };
    static const utils::StaticString BEGIN_COMMAND = "beginRenderPass";
    static const utils::StaticString END_COMMAND = "endRenderPass";
    static bool inRenderPass = false; // for debug only
    const utils::StaticString command = utils::StaticString::make(methodName, strlen(methodName));
    if (command == BEGIN_COMMAND) {
        assert_invariant(!inRenderPass);
        inRenderPass = true;
    } else if (command == END_COMMAND) {
        assert_invariant(inRenderPass);
        inRenderPass = false;
    } else if (inRenderPass && OUTSIDE_COMMANDS.find(command) != OUTSIDE_COMMANDS.end()) {
        utils::slog.e << command.c_str() << " issued inside a render pass." << utils::io::endl;
        utils::debug_trap();
    }
#endif
}

// explicit instantiation of the Dispatcher
template class ConcreteDispatcher<VulkanDriver>;

} // namespace filament::backend

#pragma clang diagnostic pop
