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

#include "VulkanBuffer.h"
#include "VulkanDriverFactory.h"
#include "VulkanHandles.h"
#include "VulkanPlatform.h"

#include <utils/Panic.h>
#include <utils/CString.h>
#include <utils/trap.h>

#include <set>

// Vulkan functions often immediately dereference pointers, so it's fine to pass in a pointer
// to a stack-allocated variable.
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wreturn-stack-address"
#pragma clang diagnostic ignored "-Wunused-parameter"

// In debug builds, we enable validation layers and set up a debug callback if the extension is
// available. Caution: the debug callback causes a null pointer dereference with optimized builds.
//
// To enable validation layers in Android, also be sure to set the jniLibs property in the gradle
// file for filament-android as follows. This copies the appropriate libraries from the NDK to the
// device. This makes the aar much larger, so it should be avoided in release builds.
//
// sourceSets { main { jniLibs {
//   srcDirs = ["${android.ndkDirectory}/sources/third_party/vulkan/src/build-android/jniLibs"]
// } } }
//
#if !defined(NDEBUG)
#define ENABLE_VALIDATION 1
#else
#define ENABLE_VALIDATION 0
#endif

static constexpr int SWAP_CHAIN_MAX_ATTEMPTS = 16;

#if ENABLE_VALIDATION

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
#if ENABLE_VALIDATION
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
#endif // ENABLE_VALIDATION

    // Create the Vulkan instance.
    VkApplicationInfo appInfo = {};
    appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
    appInfo.apiVersion = VK_API_VERSION_1_0;
    instanceCreateInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
    instanceCreateInfo.pApplicationInfo = &appInfo;
    instanceCreateInfo.enabledExtensionCount = enabledExtensionCount;
    instanceCreateInfo.ppEnabledExtensionNames = ppEnabledExtensions;
    VkResult result = vkCreateInstance(&instanceCreateInfo, VKALLOC, &mContext.instance);
    ASSERT_POSTCONDITION(result == VK_SUCCESS, "Unable to create Vulkan instance.");
    bluevk::bindInstance(mContext.instance);
    UTILS_UNUSED const PFN_vkCreateDebugReportCallbackEXT createDebugReportCallback =
            vkCreateDebugReportCallbackEXT;

#if ENABLE_VALIDATION
    if (createDebugReportCallback) {
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
    createVirtualDevice(mContext);
    mBinder.setDevice(mContext.device);

    // Choose a depth format that meets our requirements. Take care not to include stencil formats
    // just yet, since that would require a corollary change to the "aspect" flags for the VkImage.
    mContext.depthFormat = findSupportedFormat(mContext,
        { VK_FORMAT_D32_SFLOAT, VK_FORMAT_X8_D24_UNORM_PACK32 },
        VK_IMAGE_TILING_OPTIMAL, VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT);
}

VulkanDriver::~VulkanDriver() noexcept = default;

UTILS_NOINLINE
Driver* VulkanDriver::create(VulkanPlatform* const platform,
        const char* const* ppEnabledExtensions, uint32_t enabledExtensionCount) noexcept {
    assert(platform);
    auto* const driver = new VulkanDriver(platform, ppEnabledExtensions,
            enabledExtensionCount);
    return driver;
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
    vkDestroyInstance(mContext.instance, VKALLOC);
    mContext.device = nullptr;
    mContext.instance = nullptr;
}

void VulkanDriver::tick(int) {
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

    // It might take several attempts to acquire a swap chain that is not marked as "out of date".
    int attempts = 0;
    while (!acquireSwapCommandBuffer(mContext)) {
        refreshSwapChain();
        if (attempts++ > SWAP_CHAIN_MAX_ATTEMPTS) {
            PANIC_POSTCONDITION("Unable to acquire optimal image from swap chain.");
        }
    }

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
            mStagePool, size, usage);
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
    auto renderPrimitive = construct_handle<VulkanRenderPrimitive>(mHandleMap, rph, mContext);
    mDisposer.createDisposable(renderPrimitive, [this, rph] () {
        destruct_handle<VulkanRenderPrimitive>(mHandleMap, rph);
    });
}

void VulkanDriver::destroyRenderPrimitive(Handle<HwRenderPrimitive> rph) {
    if (rph) {
        auto renderPrimitive = handle_cast<VulkanRenderPrimitive>(mHandleMap, rph);
        mDisposer.removeReference(renderPrimitive);
    }
}

void VulkanDriver::createVertexBufferR(Handle<HwVertexBuffer> vbh, uint8_t bufferCount,
        uint8_t attributeCount, uint32_t elementCount, AttributeArray attributes,
        BufferUsage usage) {
    auto vertexBuffer = construct_handle<VulkanVertexBuffer>(mHandleMap, vbh, mContext, mStagePool,
            bufferCount, attributeCount, elementCount, attributes);
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
            elementSize, indexCount);
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
    auto colorTexture = color[0].handle ? handle_cast<VulkanTexture>(mHandleMap, color[0].handle) : nullptr;
    auto depthTexture = depth.handle ? handle_cast<VulkanTexture>(mHandleMap, depth.handle) : nullptr;
    auto renderTarget = construct_handle<VulkanRenderTarget>(mHandleMap, rth, mContext,
            width, height, color[0], colorTexture, depth, depthTexture);
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
    // TODO: implement sync objects
}

void VulkanDriver::createSwapChainR(Handle<HwSwapChain> sch, void* nativeWindow, uint64_t flags) {
    auto* swapChain = construct_handle<VulkanSwapChain>(mHandleMap, sch);
    VulkanSurfaceContext& sc = swapChain->surfaceContext;
    sc.surface = (VkSurfaceKHR) mContextManager.createVkSurfaceKHR(nativeWindow, mContext.instance);
    sc.nativeWindow = nativeWindow;
    mContextManager.getClientExtent(nativeWindow, &sc.clientSize.width, &sc.clientSize.height);
    getPresentationQueue(mContext, sc);
    createSwapChain(mContext, sc);

    // TODO: move the following line into makeCurrent.
    mContext.currentSurface = &sc;
}

void VulkanDriver::createSwapChainHeadlessR(Handle<HwSwapChain> sch,
        uint32_t width, uint32_t height, uint64_t flags) {
    //auto* swapChain = construct_handle<VulkanSwapChain>(mHandleMap, sch);
    // TODO: implement headless swapchain
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
    // TODO: implement Sync ojbects
    return {};
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
    // TODO: implement Sync objects
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
        vkformat = mContext.depthFormat;
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
        vkformat = mContext.depthFormat;
    }
    if (vkformat == VK_FORMAT_UNDEFINED) {
        return false;
    }
    VkFormatProperties info;
    vkGetPhysicalDeviceFormatProperties(mContext.physicalDevice, vkformat, &info);
    return (info.optimalTilingFeatures & VK_FORMAT_FEATURE_COLOR_ATTACHMENT_BIT) != 0;
}

bool VulkanDriver::isFrameTimeSupported() {
    return true;
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

    uint64_t results[2] = {};
    size_t dataSize = sizeof(results);
    VkDeviceSize stride = sizeof(uint64_t);

    VkResult result = vkGetQueryPoolResults(mContext.device, mContext.timestamps.pool,
            vtq->startingQueryIndex, 2, dataSize, (void*) results, stride,
            VK_QUERY_RESULT_64_BIT);

    if (result == VK_NOT_READY) {
        return false;
    }

    ASSERT_POSTCONDITION(result == VK_SUCCESS, "vkGetQueryPoolResults error.");
    ASSERT_POSTCONDITION(results[1] >= results[0], "Timestamps are not monotonically increasing.");

    // NOTE: MoltenVK currently writes system time so the following delta will always be zero.
    // However there are plans for implementing this properly. See the following GitHub ticket.
    // https://github.com/KhronosGroup/MoltenVK/issues/773

    uint64_t delta = results[1] - results[0];
    *elapsedTime = delta;
    return true;
}

SyncStatus VulkanDriver::getSyncStatus(Handle<HwSync> sh) {
    // TODO: implement Sync objects
    return SyncStatus::SIGNALED;
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

    const VulkanAttachment color = rt->getColor();
    const VulkanAttachment depth = rt->getDepth();
    const bool hasColor = color.format != VK_FORMAT_UNDEFINED;
    const bool hasDepth = depth.format != VK_FORMAT_UNDEFINED;

    mDisposer.acquire(rt, mContext.currentCommands->resources);
    mDisposer.acquire(color.offscreen, mContext.currentCommands->resources);
    mDisposer.acquire(depth.offscreen, mContext.currentCommands->resources);

    VkImageLayout finalColorLayout;
    VkImageLayout finalDepthLayout;

    if (rt->isOffscreen()) {
        finalColorLayout = VK_IMAGE_LAYOUT_GENERAL;
        finalDepthLayout = VK_IMAGE_LAYOUT_GENERAL;
    } else {
        finalColorLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;
        finalDepthLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;
    }

    VkRenderPass renderPass = mFramebufferCache.getRenderPass({
        .finalColorLayout = finalColorLayout,
        .finalDepthLayout = finalDepthLayout,
        .colorFormat = color.format,
        .depthFormat = depth.format,
        .flags = {
            .clear = params.flags.clear,
            .discardStart = params.flags.discardStart,
            .discardEnd = params.flags.discardEnd
        }
    });
    mBinder.bindRenderPass(renderPass);

    VulkanFboCache::FboKey fbo { .renderPass = renderPass };
    int numAttachments = 0;
    if (hasColor) {
        fbo.attachments[numAttachments++] = color.view;
    }
    if (hasDepth) {
        fbo.attachments[numAttachments++] = depth.view;
    }

    VkRenderPassBeginInfo renderPassInfo {
        .sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO,
        .renderPass = renderPass,
        .framebuffer = mFramebufferCache.getFramebuffer(fbo, extent.width, extent.height),
        .renderArea = {
            .offset = {params.viewport.left, params.viewport.bottom},
            .extent = {params.viewport.width, params.viewport.height}
        }
    };

    rt->transformClientRectToPlatform(&renderPassInfo.renderArea);

    VkClearValue clearValues[2] = {};
    if (hasColor) {
        VkClearValue& clearValue = clearValues[renderPassInfo.clearValueCount++];
        clearValue.color.float32[0] = params.clearColor.r;
        clearValue.color.float32[1] = params.clearColor.g;
        clearValue.color.float32[2] = params.clearColor.b;
        clearValue.color.float32[3] = params.clearColor.a;
    }
    if (hasDepth) {
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
            .minDepth = 0.0f,
            .maxDepth = 1.0f
    };
    VkRect2D scissor {
            .offset = { std::max(0, (int32_t) viewport.x), std::max(0, (int32_t) viewport.y) },
            .extent = { (uint32_t) viewport.width, (uint32_t) viewport.height }
    };

    mCurrentRenderTarget->transformClientRectToPlatform(&scissor);
    vkCmdSetScissor(swapContext.commands.cmdbuffer, 0, 1, &scissor);

    mCurrentRenderTarget->transformClientRectToPlatform(&viewport);
    vkCmdSetViewport(swapContext.commands.cmdbuffer, 0, 1, &viewport);

    mContext.currentRenderPass = renderPassInfo;
}

void VulkanDriver::endRenderPass(int) {
    assert(mContext.currentCommands);
    assert(mContext.currentSurface);
    assert(mCurrentRenderTarget);
    vkCmdEndRenderPass(mContext.currentCommands->cmdbuffer);
    mCurrentRenderTarget = VK_NULL_HANDLE;
    mContext.currentRenderPass.renderPass = VK_NULL_HANDLE;
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

    auto& cmdfence = swapContext.commands.fence;
    std::unique_lock<utils::Mutex> lock(cmdfence->mutex);
    result = vkQueueSubmit(mContext.graphicsQueue, 1, &submitInfo, cmdfence->fence);
    cmdfence->submitted = true;
    lock.unlock();
    ASSERT_POSTCONDITION(result == VK_SUCCESS, "vkQueueSubmit error.");
    cmdfence->condition.notify_all();

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
    if (result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR) {
        refreshSwapChain();
        return;
    }
    ASSERT_POSTCONDITION(result == VK_SUCCESS, "vkQueuePresentKHR error.");
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
}

void VulkanDriver::pushGroupMarker(char const* string,  size_t len) {
    // TODO: Add group marker color to the Driver API
    constexpr float MARKER_COLOR[] = { 0.0f, 1.0f, 0.0f, 1.0f };
    ASSERT_POSTCONDITION(mContext.currentCommands,
            "Markers can only be inserted within a beginFrame / endFrame.");
    if (mContext.debugMarkersSupported) {
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
    if (mContext.debugMarkersSupported) {
        vkCmdDebugMarkerEndEXT(mContext.currentCommands->cmdbuffer);
    }
}

void VulkanDriver::startCapture(int) {

}

void VulkanDriver::stopCapture(int) {

}

void VulkanDriver::readPixels(Handle<HwRenderTarget> src,
        uint32_t x, uint32_t y, uint32_t width, uint32_t height,
        PixelBufferDescriptor&& p) {
    scheduleDestroy(std::move(p));
}

void VulkanDriver::readStreamPixels(Handle<HwStream> sh, uint32_t x, uint32_t y, uint32_t width,
        uint32_t height, PixelBufferDescriptor&& p) {
    scheduleDestroy(std::move(p));
}

void VulkanDriver::blit(TargetBufferFlags buffers,
        Handle<HwRenderTarget> dst, backend::Viewport dstRect,
        Handle<HwRenderTarget> src, backend::Viewport srcRect,
        SamplerMagFilter filter) {
    auto dstTarget = handle_cast<VulkanRenderTarget>(mHandleMap, dst);
    auto srcTarget = handle_cast<VulkanRenderTarget>(mHandleMap, src);

    // In debug builds, verify that the two render targets have blittable formats.
#ifndef NDEBUG
    const VkPhysicalDevice gpu = mContext.physicalDevice;
    VkFormatProperties info;
    vkGetPhysicalDeviceFormatProperties(gpu, srcTarget->getColor().format, &info);
    if (!ASSERT_POSTCONDITION_NON_FATAL(info.optimalTilingFeatures & VK_FORMAT_FEATURE_BLIT_SRC_BIT,
            "Source format is not blittable")) {
        return;
    }
    vkGetPhysicalDeviceFormatProperties(gpu, dstTarget->getColor().format, &info);
    if (!ASSERT_POSTCONDITION_NON_FATAL(info.optimalTilingFeatures & VK_FORMAT_FEATURE_BLIT_DST_BIT,
            "Destination format is not blittable")) {
        return;
    }
    if (any(buffers & TargetBufferFlags::DEPTH)) {
        utils::slog.w << "Depth blits are not yet supported." << utils::io::endl;
    }
#endif

    const int32_t srcRight = srcRect.left + srcRect.width;
    const int32_t srcTop = srcRect.bottom + srcRect.height;
    const uint32_t srcLevel = srcTarget->getColorLevel();

    const int32_t dstRight = dstRect.left + dstRect.width;
    const int32_t dstTop = dstRect.bottom + dstRect.height;
    const uint32_t dstLevel = dstTarget->getColorLevel();

    const VkImageBlit blitRegions[1] = {{
        .srcSubresource = { VK_IMAGE_ASPECT_COLOR_BIT, srcLevel, 0, 1 },
        .srcOffsets = { { srcRect.left, srcRect.bottom, 0 }, { srcRight, srcTop, 1 }},
        .dstSubresource = { VK_IMAGE_ASPECT_COLOR_BIT, dstLevel, 0, 1 },
        .dstOffsets = { { dstRect.left, dstRect.bottom, 0 }, { dstRight, dstTop, 1 }}
    }};

    auto vkblit = [=](VkCommandBuffer cmdbuffer) {
        VkImage srcImage = srcTarget->getColor().image;
        VulkanTexture::transitionImageLayout(cmdbuffer, srcImage, VK_IMAGE_LAYOUT_UNDEFINED,
                VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL, srcLevel, 1);

        VkImage dstImage = dstTarget->getColor().image;
        VulkanTexture::transitionImageLayout(cmdbuffer, dstImage, VK_IMAGE_LAYOUT_UNDEFINED,
                VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, dstLevel, 1);

        // TODO: Issue vkCmdResolveImage for MSAA targets.

        vkCmdBlitImage(cmdbuffer, srcImage, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL, dstImage,
                VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, blitRegions,
                filter == SamplerMagFilter::NEAREST ? VK_FILTER_NEAREST : VK_FILTER_LINEAR);

        VulkanTexture::transitionImageLayout(cmdbuffer, srcImage, VK_IMAGE_LAYOUT_UNDEFINED,
                getTextureLayout(srcTarget->getColor().offscreen->usage), srcLevel, 1);

        VulkanTexture::transitionImageLayout(cmdbuffer, dstImage, VK_IMAGE_LAYOUT_UNDEFINED,
                getTextureLayout(dstTarget->getColor().offscreen->usage), dstLevel, 1);
    };

    if (!mContext.currentCommands) {
        // NOTE: We do not call flushWorkCommandBuffer here. The pipeline barriers pushed by
        // "transitionImageLayout" is hopefully sufficient for proper synchronization.
        vkblit(acquireWorkCommandBuffer(mContext));
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

    // If this is a debug build, validate the current shader.
#if !defined(NDEBUG)
    if (program->bundle.vertex == VK_NULL_HANDLE || program->bundle.fragment == VK_NULL_HANDLE) {
        utils::slog.e << "Binding missing shader: " << program->name.c_str() << utils::io::endl;
    }
#endif

    // Update the VK raster state.
    mContext.rasterState.depthStencil = {
        .sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO,
        .depthTestEnable = VK_TRUE,
        .depthWriteEnable = (VkBool32) rasterState.depthWrite,
        .depthCompareOp = getCompareOp(rasterState.depthFunc),
        .depthBoundsTestEnable = VK_FALSE,
        .stencilTestEnable = VK_FALSE,
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

    auto& vkraster = mContext.rasterState.rasterization;
    vkraster.cullMode = getCullMode(rasterState.culling);
    vkraster.frontFace = getFrontFace(rasterState.inverseFrontFaces);
    vkraster.depthBiasEnable = (depthOffset.constant || depthOffset.slope) ? VK_TRUE : VK_FALSE;
    vkraster.depthBiasConstantFactor = depthOffset.constant;
    vkraster.depthBiasSlopeFactor = depthOffset.slope;

    // Remove the fragment shader from depth-only passes to avoid a validation warning.
    VulkanBinder::ProgramBundle shaderHandles = program->bundle;
    VulkanRenderTarget* rt = mCurrentRenderTarget;
    const auto color = rt->getColor();
    const auto depth = rt->getDepth();
    const bool hasColor = color.format != VK_FORMAT_UNDEFINED;
    const bool hasDepth = depth.format != VK_FORMAT_UNDEFINED;
    const bool depthOnly = hasDepth && !hasColor;
    if (depthOnly) {
        shaderHandles.fragment = VK_NULL_HANDLE;
    }

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
    VkDescriptorSet descriptors[2];
    VkPipelineLayout pipelineLayout;
    if (mBinder.getOrCreateDescriptors(descriptors, &pipelineLayout)) {
        vkCmdBindDescriptorSets(cmdbuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipelineLayout, 0, 2,
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
    getSurfaceCaps(mContext, surface);

    backend::destroySwapChain(mContext, surface, mDisposer);
    createSwapChain(mContext, surface);

    void* nativeWindow = surface.nativeWindow;
    VkExtent2D size;
    mContextManager.getClientExtent(nativeWindow, &size.width, &size.height);
    surface.clientSize = size;

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
