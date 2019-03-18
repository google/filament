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

#include "driver/vulkan/VulkanDriver.h"

#include "driver/CommandStreamDispatcher.h"

#include "VulkanBuffer.h"
#include "VulkanHandles.h"

#include <utils/Panic.h>
#include <utils/CString.h>
#include <utils/trap.h>

#include <set>

// Vulkan functions often immediately dereference pointers, so it's fine to pass in a pointer
// to a stack-allocated variable.
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wreturn-stack-address"
#pragma clang diagnostic ignored "-Wunused-parameter"

namespace filament {
namespace driver {

VulkanDriver::VulkanDriver(VulkanPlatform* platform,
        const char* const* ppEnabledExtensions, uint32_t enabledExtensionCount) noexcept :
        DriverBase(new ConcreteDispatcher<VulkanDriver>()),
        mContextManager(*platform), mStagePool(mContext), mFramebufferCache(mContext),
        mSamplerCache(mContext) {
    mContext.rasterState = mBinder.getDefaultRasterState();

    // Load Vulkan entry points.
    ASSERT_POSTCONDITION(bluevk::initialize(), "BlueVK is unable to load entry points.");

    // In debug builds, attempt to enable the following layers if they are available.
    // Validation crashes on MoltenVK, so disable it by default on MacOS.
    VkInstanceCreateInfo instanceCreateInfo = {};
#if !defined(NDEBUG) && !defined(__APPLE__)
    static utils::StaticString DESIRED_LAYERS[] = {
    // NOTE: sometimes we see a message: "Cannot activate layer VK_LAYER_GOOGLE_unique_objects
    // prior to activating VK_LAYER_LUNARG_core_validation." despite the fact that it is clearly
    // last in the following list. Should we simply remove unique_objects from the list?
#if defined(ANDROID)
        "VK_LAYER_GOOGLE_threading",       "VK_LAYER_LUNARG_parameter_validation",
        "VK_LAYER_LUNARG_object_tracker",  "VK_LAYER_LUNARG_image",
        "VK_LAYER_LUNARG_core_validation", "VK_LAYER_LUNARG_swapchain",
        "VK_LAYER_GOOGLE_unique_objects"
#else
        "VK_LAYER_LUNARG_standard_validation",
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
    for (const VkLayerProperties& layer : availableLayers) {
        const utils::CString availableLayer(layer.layerName);
        for (const auto& desired : DESIRED_LAYERS) {
            if (availableLayer == desired) {
                enabledLayers.push_back(desired.c_str());
            }
        }
    }

    // To enable validation layers in Android, set the jniLibs property in the gradle file for
    // filament-android as follows. This copies the appropriate libraries from the NDK to the
    // device. This makes the aar much larger, so it should be avoided in release builds.
    //
    // sourceSets { main { jniLibs {
    //   srcDirs = ["${android.ndkDirectory}/sources/third_party/vulkan/src/build-android/jniLibs"]
    // } } }

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
#endif

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

    // In debug builds, set up a debug callback if the extension is available.
    // The debug callback seems to work properly only on 64-bit architectures.
#if !defined(NDEBUG) && (ULONG_MAX != UINT_MAX)
    if (createDebugReportCallback) {
        const PFN_vkDebugReportCallbackEXT cb = [] (VkDebugReportFlagsEXT flags,
                VkDebugReportObjectTypeEXT objectType, uint64_t object,
                size_t location, int32_t messageCode, const char* pLayerPrefix,
                const char* pMessage, void* pUserData) -> VkBool32 {
            if (flags & VK_DEBUG_REPORT_ERROR_BIT_EXT) {
                utils::slog.e << "VULKAN ERROR: (" << pLayerPrefix << ") "
                        << pMessage << utils::io::endl;
                // This is a good spot for a breakpoint although a permanent std::raise(SIGTRAP) is
                // a bit too aggressive since Mali drivers will emit ERROR reports for minor
                // violations that are not fatal.
            } else {
                utils::slog.w << "VULKAN WARNING: (" << pLayerPrefix << ") "
                        << pMessage << utils::io::endl;
            }
            return VK_FALSE;
        };
        VkDebugReportCallbackCreateInfoEXT cbinfo = {
            .sType = VK_STRUCTURE_TYPE_DEBUG_REPORT_CALLBACK_CREATE_INFO_EXT,
            .flags = VK_DEBUG_REPORT_ERROR_BIT_EXT | VK_DEBUG_REPORT_WARNING_BIT_EXT,
            .pfnCallback = cb
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
    waitForIdle(mContext);

    // Destroy the work command buffer and fence.
    WorkContext work = mContext.work;
    VkDevice device = mContext.device;
    vkFreeCommandBuffers(device, mContext.commandPool, 1, &work.cmdbuffer);
    vkDestroyFence(device, work.fence, VKALLOC);

    mBinder.destroyCache();
    mStagePool.reset();
    mFramebufferCache.reset();
    mSamplerCache.reset();
    vmaDestroyAllocator(mContext.allocator);
    vkDestroyCommandPool(mContext.device, mContext.commandPool, VKALLOC);
    vkDestroyDevice(mContext.device, VKALLOC);
    if (mDebugCallback) {
        vkDestroyDebugReportCallbackEXT(mContext.instance, mDebugCallback, VKALLOC);
    }
    vkDestroyInstance(mContext.instance, VKALLOC);
    mContext.device = nullptr;
    mContext.instance = nullptr;
}

void VulkanDriver::beginFrame(int64_t monotonic_clock_ns, uint32_t frameId) {
    // We allow multiple beginFrame / endFrame pairs before commit(), so gracefully return early
    // if the swap chain has already been acquired.
    if (mContext.cmdbuffer) {
        return;
    }

    acquireCommandBuffer(mContext);
    SwapContext& swapContext = getSwapContext(mContext);

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

    // Now that a command buffer is ready, perform any pending work that has been scheduled by
    // VulkanDriver, such as reclaiming memory. There are two sets of work queues: one in the
    // global context, and one in the per-cmdbuffer contexts. Crucially, we have begun the frame
    // but not the render pass; we cannot perform arbitrary work during the render pass.
    performPendingWork(swapContext.pendingWork, swapContext.cmdbuffer);
    performPendingWork(mContext.pendingWork, swapContext.cmdbuffer);

    // Free old unused objects.
    mStagePool.gc();
    mFramebufferCache.gc();
    mBinder.gc();
}

void VulkanDriver::setPresentationTime(int64_t monotonic_clock_ns) {
}

void VulkanDriver::endFrame(uint32_t frameId) {
    // Do nothing here; see commit().
}

void VulkanDriver::flush(int) {
    // Todo: equivalent of glFlush()
}

void VulkanDriver::createVertexBufferR(Driver::VertexBufferHandle vbh, uint8_t bufferCount,
        uint8_t attributeCount, uint32_t elementCount, Driver::AttributeArray attributes,
        Driver::BufferUsage usage) {
    construct_handle<VulkanVertexBuffer>(mHandleMap, vbh, mContext, mStagePool, bufferCount,
            attributeCount, elementCount, attributes);
}

void VulkanDriver::createIndexBufferR(Driver::IndexBufferHandle ibh, Driver::ElementType elementType,
        uint32_t indexCount, Driver::BufferUsage usage) {
    auto elementSize = (uint8_t) getElementTypeSize(elementType);
    construct_handle<VulkanIndexBuffer>(mHandleMap, ibh, mContext, mStagePool, elementSize,
            indexCount);
}

void VulkanDriver::createTextureR(Driver::TextureHandle th, SamplerType target, uint8_t levels,
        TextureFormat format, uint8_t samples, uint32_t w, uint32_t h, uint32_t depth,
        TextureUsage usage) {
    construct_handle<VulkanTexture>(mHandleMap, th, mContext, target, levels, format, samples,
            w, h, depth, usage, mStagePool);
}

void VulkanDriver::createSamplerGroupR(Driver::SamplerGroupHandle sbh, size_t count) {
    construct_handle<VulkanSamplerGroup>(mHandleMap, sbh, mContext, count);
}

void VulkanDriver::createUniformBufferR(Driver::UniformBufferHandle ubh, size_t size,
        Driver::BufferUsage usage) {
    construct_handle<VulkanUniformBuffer>(mHandleMap, ubh, mContext, mStagePool, size, usage);
}

void VulkanDriver::createRenderPrimitiveR(Driver::RenderPrimitiveHandle rph, int) {
    construct_handle<VulkanRenderPrimitive>(mHandleMap, rph, mContext);
}

void VulkanDriver::createProgramR(Driver::ProgramHandle ph, Program&& program) {
    construct_handle<VulkanProgram>(mHandleMap, ph, mContext, program);
}

void VulkanDriver::createDefaultRenderTargetR(Driver::RenderTargetHandle rth, int) {
    construct_handle<VulkanRenderTarget>(mHandleMap, rth, mContext);
}

void VulkanDriver::createRenderTargetR(Driver::RenderTargetHandle rth,
        Driver::TargetBufferFlags targets, uint32_t width, uint32_t height, uint8_t samples,
        TextureFormat format, Driver::TargetBufferInfo color, Driver::TargetBufferInfo depth,
        Driver::TargetBufferInfo stencil) {
    auto& renderTarget = *construct_handle<VulkanRenderTarget>(mHandleMap, rth, mContext,
            width, height, color.level);
    if (color.handle) {
        auto colorTexture = handle_cast<VulkanTexture>(mHandleMap, color.handle);
        renderTarget.setColorImage({
            .image = colorTexture->textureImage,
            .view = colorTexture->imageView,
            .format = colorTexture->vkformat
        });
    } else if (targets & TargetBufferFlags::COLOR) {
        renderTarget.createColorImage(getVkFormat(format));
    }
    if (depth.handle) {
        auto depthTexture = handle_cast<VulkanTexture>(mHandleMap, depth.handle);
        renderTarget.setDepthImage({
            .image = depthTexture->textureImage,
            .view = depthTexture->imageView,
            .format = depthTexture->vkformat
        });
    } else if (targets & TargetBufferFlags::DEPTH) {
        renderTarget.createDepthImage(mContext.depthFormat);
    }
}

void VulkanDriver::createFenceR(Driver::FenceHandle fh, int) {
}

void VulkanDriver::createSwapChainR(Driver::SwapChainHandle sch, void* nativeWindow,
        uint64_t flags) {
    auto* swapChain = construct_handle<VulkanSwapChain>(mHandleMap, sch);
    VulkanSurfaceContext& sc = swapChain->surfaceContext;
    sc.surface = (VkSurfaceKHR) mContextManager.createVkSurfaceKHR(nativeWindow,
            mContext.instance, &sc.clientSize.width, &sc.clientSize.height);
    getPresentationQueue(mContext, sc);
    getSurfaceCaps(mContext, sc);
    createSwapChain(mContext, sc);

    // TODO: move the following line into makeCurrent.
    mContext.currentSurface = &sc;
}

void VulkanDriver::createStreamFromTextureIdR(Driver::StreamHandle sh, intptr_t externalTextureId,
        uint32_t width, uint32_t height) {
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
    return {};
}

Handle<HwSwapChain> VulkanDriver::createSwapChainS() noexcept {
    return alloc_handle<VulkanSwapChain, HwSwapChain>();
}

Handle<HwStream> VulkanDriver::createStreamFromTextureIdS() noexcept {
    return {};
}

void VulkanDriver::destroyVertexBuffer(Driver::VertexBufferHandle vbh) {
    if (vbh) {
        waitForIdle(mContext);
        destruct_handle<VulkanVertexBuffer>(mHandleMap, vbh);
    }
}

void VulkanDriver::destroyIndexBuffer(Driver::IndexBufferHandle ibh) {
    if (ibh) {
        waitForIdle(mContext);
        destruct_handle<VulkanIndexBuffer>(mHandleMap, ibh);
    }
}

void VulkanDriver::destroyRenderPrimitive(Driver::RenderPrimitiveHandle rph) {
    if (rph) {
        waitForIdle(mContext);
        destruct_handle<VulkanRenderPrimitive>(mHandleMap, rph);
    }
}

void VulkanDriver::destroyProgram(Driver::ProgramHandle ph) {
    if (ph) {
        waitForIdle(mContext);
        destruct_handle<VulkanProgram>(mHandleMap, ph);
    }
}

void VulkanDriver::destroySamplerGroup(Driver::SamplerGroupHandle sbh) {
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

void VulkanDriver::destroyUniformBuffer(Driver::UniformBufferHandle ubh) {
    if (ubh) {
        auto* buffer = handle_cast<VulkanUniformBuffer>(mHandleMap, ubh);
        mBinder.unbindUniformBuffer(buffer->getGpuBuffer());
        waitForIdle(mContext);
        destruct_handle<VulkanUniformBuffer>(mHandleMap, ubh);
    }
}

void VulkanDriver::destroyTexture(Driver::TextureHandle th) {
    if (th) {
        auto* tex = handle_cast<VulkanTexture>(mHandleMap, th);
        mBinder.unbindImageView(tex->imageView);
        waitForIdle(mContext);
        destruct_handle<VulkanTexture>(mHandleMap, th);
    }
}

void VulkanDriver::destroyRenderTarget(Driver::RenderTargetHandle rth) {
    if (rth) {
        waitForIdle(mContext);
        destruct_handle<VulkanRenderTarget>(mHandleMap, rth);
    }
}

void VulkanDriver::destroySwapChain(Driver::SwapChainHandle sch) {
    if (sch) {
        waitForIdle(mContext);
        VulkanSurfaceContext& sc = handle_cast<VulkanSwapChain>(mHandleMap, sch)->surfaceContext;
        destroySurfaceContext(mContext, sc);
        destruct_handle<VulkanSwapChain>(mHandleMap, sch);
    }
}

void VulkanDriver::destroyStream(Driver::StreamHandle sh) {
}

Handle<HwStream> VulkanDriver::createStream(void* nativeStream) {
    return {};
}

void VulkanDriver::setStreamDimensions(Driver::StreamHandle sh, uint32_t width, uint32_t height) {
}

int64_t VulkanDriver::getStreamTimestamp(Driver::StreamHandle sh) {
    return 0;
}

void VulkanDriver::updateStreams(CommandStream* driver) {
}

void VulkanDriver::destroyFence(Driver::FenceHandle fh) {
}

Driver::FenceStatus VulkanDriver::wait(Driver::FenceHandle fh, uint64_t timeout) {
    return FenceStatus::ERROR;
}

// We create all textures using VK_IMAGE_TILING_OPTIMAL, so our definition of "supported" is that
// the GPU supports the given texture format with non-zero optimal tiling features.
bool VulkanDriver::isTextureFormatSupported(Driver::TextureFormat format) {
    assert(mContext.physicalDevice);
    VkFormat vkformat = getVkFormat(format);
    if (vkformat == VK_FORMAT_UNDEFINED) {
        return false;
    }
    VkFormatProperties info;
    vkGetPhysicalDeviceFormatProperties(mContext.physicalDevice, vkformat, &info);
    return info.optimalTilingFeatures != 0;
}

bool VulkanDriver::isRenderTargetFormatSupported(Driver::TextureFormat format) {
    assert(mContext.physicalDevice);
    VkFormat vkformat = getVkFormat(format);
    if (vkformat == VK_FORMAT_UNDEFINED) {
        return false;
    }
    VkFormatProperties info;
    vkGetPhysicalDeviceFormatProperties(mContext.physicalDevice, vkformat, &info);
    return (info.optimalTilingFeatures & VK_FORMAT_FEATURE_COLOR_ATTACHMENT_BIT) != 0;
}

bool VulkanDriver::isFrameTimeSupported() {
    return false;
}

void VulkanDriver::updateVertexBuffer(Driver::VertexBufferHandle vbh, size_t index,
        BufferDescriptor&& p, uint32_t byteOffset) {
    auto& vb = *handle_cast<VulkanVertexBuffer>(mHandleMap, vbh);
    vb.buffers[index]->loadFromCpu(p.buffer, byteOffset, p.size);
    scheduleDestroy(std::move(p));
}

void VulkanDriver::updateIndexBuffer(Driver::IndexBufferHandle ibh, BufferDescriptor&& p,
        uint32_t byteOffset) {
    auto& ib = *handle_cast<VulkanIndexBuffer>(mHandleMap, ibh);
    ib.buffer->loadFromCpu(p.buffer, byteOffset, p.size);
    scheduleDestroy(std::move(p));
}

void VulkanDriver::update2DImage(Driver::TextureHandle th,
        uint32_t level, uint32_t xoffset, uint32_t yoffset, uint32_t width, uint32_t height,
        PixelBufferDescriptor&& data) {
    assert(xoffset == 0 && yoffset == 0 && "Offsets not yet supported.");
    handle_cast<VulkanTexture>(mHandleMap, th)->update2DImage(data, width, height, level);
    scheduleDestroy(std::move(data));
}

void VulkanDriver::updateCubeImage(Driver::TextureHandle th, uint32_t level,
        PixelBufferDescriptor&& data, FaceOffsets faceOffsets) {
    handle_cast<VulkanTexture>(mHandleMap, th)->updateCubeImage(data, faceOffsets, level);
    scheduleDestroy(std::move(data));
}

void VulkanDriver::setExternalImage(Driver::TextureHandle th, void* image) {
}

void VulkanDriver::setExternalStream(Driver::TextureHandle th, Driver::StreamHandle sh) {
}

void VulkanDriver::generateMipmaps(Driver::TextureHandle th) { }

bool VulkanDriver::canGenerateMipmaps() {
    return false;
}

void VulkanDriver::updateUniformBuffer(Driver::UniformBufferHandle ubh, BufferDescriptor&& data) {
    if (data.size > 0) {
        auto* buffer = handle_cast<VulkanUniformBuffer>(mHandleMap, ubh);
        buffer->loadFromCpu(data.buffer, (uint32_t) data.size);
        scheduleDestroy(std::move(data));
    }
}

void VulkanDriver::updateSamplerGroup(Driver::SamplerGroupHandle sbh,
        SamplerGroup&& samplerGroup) {
    auto* sb = handle_cast<VulkanSamplerGroup>(mHandleMap, sbh);
    *sb->sb = samplerGroup;
}

void VulkanDriver::beginRenderPass(Driver::RenderTargetHandle rth,
        const Driver::RenderPassParams& params) {

    assert(mContext.cmdbuffer);
    assert(mContext.currentSurface);
    VulkanSurfaceContext& surface = *mContext.currentSurface;
    const SwapContext& swapContext = surface.swapContexts[surface.currentSwapIndex];
    mCurrentRenderTarget = handle_cast<VulkanRenderTarget>(mHandleMap, rth);
    VulkanRenderTarget* rt = mCurrentRenderTarget;
    const VkExtent2D extent = rt->getExtent();
    assert(extent.width > 0 && extent.height > 0);

    const auto color = rt->getColor();
    const auto depth = rt->getDepth();
    const bool hasColor = color.format != VK_FORMAT_UNDEFINED;
    const bool hasDepth = depth.format != VK_FORMAT_UNDEFINED;
    const bool depthOnly = hasDepth && !hasColor;

    VkImageLayout finalLayout;
    if (!rt->isOffscreen()) {
        finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;
    } else if (depthOnly) {
        finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_READ_ONLY_OPTIMAL;
    } else {
        finalLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
    }

    VkRenderPass renderPass = mFramebufferCache.getRenderPass({
        .finalLayout = finalLayout,
        .colorFormat = color.format,
        .depthFormat = depth.format,
        .flags.clear         = params.flags.clear,
        .flags.discardStart  = params.flags.discardStart,
        .flags.discardEnd    = params.flags.discardEnd,
        .flags.dependencies  = params.flags.dependencies
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
        .renderArea.offset.x = params.viewport.left,
        .renderArea.offset.y = params.viewport.bottom,
        .renderArea.extent.width = params.viewport.width,
        .renderArea.extent.height = params.viewport.height,
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

    vkCmdBeginRenderPass(swapContext.cmdbuffer, &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);
    if (!(params.flags.clear & RenderPassFlags::IGNORE_VIEWPORT)) {
        viewport(params.viewport.left, params.viewport.bottom,
                params.viewport.width, params.viewport.height);
    }

    mContext.currentRenderPass = renderPassInfo;
}

void VulkanDriver::endRenderPass(int) {
    assert(mContext.cmdbuffer);
    assert(mContext.currentSurface);
    assert(mCurrentRenderTarget);
    vkCmdEndRenderPass(mContext.cmdbuffer);
    mCurrentRenderTarget = VK_NULL_HANDLE;
    mContext.currentRenderPass.renderPass = VK_NULL_HANDLE;
}

void VulkanDriver::discardSubRenderTargetBuffers(Driver::RenderTargetHandle rth,
        Driver::TargetBufferFlags buffers,
        uint32_t left, uint32_t bottom, uint32_t width, uint32_t height) {
}

void VulkanDriver::resizeRenderTarget(Driver::RenderTargetHandle rth,
        uint32_t width, uint32_t height) {
}

void VulkanDriver::setRenderPrimitiveBuffer(Driver::RenderPrimitiveHandle rph,
        Driver::VertexBufferHandle vbh, Driver::IndexBufferHandle ibh,
        uint32_t enabledAttributes) {
    auto primitive = handle_cast<VulkanRenderPrimitive>(mHandleMap, rph);
    primitive->setBuffers(handle_cast<VulkanVertexBuffer>(mHandleMap, vbh),
            handle_cast<VulkanIndexBuffer>(mHandleMap, ibh), enabledAttributes);
}

void VulkanDriver::setRenderPrimitiveRange(Driver::RenderPrimitiveHandle rph,
        Driver::PrimitiveType pt, uint32_t offset,
        uint32_t minIndex, uint32_t maxIndex, uint32_t count) {
    auto& primitive = *handle_cast<VulkanRenderPrimitive>(mHandleMap, rph);
    primitive.setPrimitiveType(pt);
    primitive.offset = offset * primitive.indexBuffer->elementSize;
    primitive.count = count;
    primitive.minIndex = minIndex;
    primitive.maxIndex = maxIndex > minIndex ? maxIndex : primitive.maxVertexCount - 1;
}

void VulkanDriver::setViewportScissor(
        int32_t left, int32_t bottom, uint32_t width, uint32_t height) {
    assert(mContext.cmdbuffer && mCurrentRenderTarget);
    // Compute the intersection of the requested scissor rectangle with the current viewport.
    int32_t x = std::max(left, (int32_t) mContext.viewport.x);
    int32_t y = std::max(bottom, (int32_t) mContext.viewport.y);
    int32_t right = std::min(left + (int32_t) width,
            (int32_t) (mContext.viewport.x + mContext.viewport.width));
    int32_t top = std::min(bottom + (int32_t) height,
            (int32_t) (mContext.viewport.y + mContext.viewport.height));
    VkRect2D scissor {
        .extent = { (uint32_t) right - x, (uint32_t) top - y },
        .offset = { std::max(0, x), std::max(0, y) }
    };

    mCurrentRenderTarget->transformClientRectToPlatform(&scissor);
    vkCmdSetScissor(mContext.cmdbuffer, 0, 1, &scissor);
}

void VulkanDriver::makeCurrent(Driver::SwapChainHandle drawSch, Driver::SwapChainHandle readSch) {
    ASSERT_PRECONDITION_NON_FATAL(drawSch == readSch,
                                  "Vulkan driver does not support distinct draw/read swap chains.");
    VulkanSurfaceContext& sContext = handle_cast<VulkanSwapChain>(mHandleMap, drawSch)->surfaceContext;
    mContext.currentSurface = &sContext;
}

void VulkanDriver::commit(Driver::SwapChainHandle sch) {
    // Tell Vulkan we're done appending to the command buffer.
    ASSERT_POSTCONDITION(mContext.cmdbuffer,
            "Vulkan driver requires at least one frame before a commit.");

    // Finalize the command buffer and set the cmdbuffer pointer to null.
    VkResult result = vkEndCommandBuffer(mContext.cmdbuffer);
    ASSERT_POSTCONDITION(result == VK_SUCCESS, "vkEndCommandBuffer error.");
    mContext.cmdbuffer = nullptr;

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
            .pCommandBuffers = &swapContext.cmdbuffer,
            .signalSemaphoreCount = 1u,
            .pSignalSemaphores = &surfaceContext.renderingFinished,
    };
    result = vkQueueSubmit(mContext.graphicsQueue, 1, &submitInfo, swapContext.fence);
    ASSERT_POSTCONDITION(result == VK_SUCCESS, "vkQueueSubmit error.");
    swapContext.submitted = true;

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
    ASSERT_POSTCONDITION(result != VK_ERROR_OUT_OF_DATE_KHR && result != VK_SUBOPTIMAL_KHR,
            "Stale / resized swap chain not yet supported.");
    ASSERT_POSTCONDITION(result == VK_SUCCESS, "vkQueuePresentKHR error.");
}

void VulkanDriver::viewport(ssize_t left, ssize_t bottom, size_t width, size_t height) {
    assert(mContext.cmdbuffer && mCurrentRenderTarget);
    VkViewport viewport = mContext.viewport = {
        .x = (float) left,
        .y = (float) bottom,
        .height = (float) height,
        .width = (float) width,
        .minDepth = 0.0f,
        .maxDepth = 1.0f
    };
    VkRect2D scissor {
        .extent = { (uint32_t) width, (uint32_t) height },
        .offset = { std::max(0, (int32_t) left), std::max(0, (int32_t) bottom) }
    };

    mCurrentRenderTarget->transformClientRectToPlatform(&scissor);
    vkCmdSetScissor(mContext.cmdbuffer, 0, 1, &scissor);

    mCurrentRenderTarget->transformClientRectToPlatform(&viewport);
    vkCmdSetViewport(mContext.cmdbuffer, 0, 1, &viewport);
}

void VulkanDriver::bindUniformBuffer(size_t index, Driver::UniformBufferHandle ubh) {
    auto* buffer = handle_cast<VulkanUniformBuffer>(mHandleMap, ubh);
    // The driver API does not currently expose offset / range, but it will do so in the future.
    const VkDeviceSize offset = 0;
    const VkDeviceSize size = VK_WHOLE_SIZE;
    mBinder.bindUniformBuffer((uint32_t) index, buffer->getGpuBuffer(), offset, size);
}

void VulkanDriver::bindUniformBufferRange(size_t index, Driver::UniformBufferHandle ubh,
        size_t offset, size_t size) {
    auto* buffer = handle_cast<VulkanUniformBuffer>(mHandleMap, ubh);
    mBinder.bindUniformBuffer((uint32_t)index, buffer->getGpuBuffer(), offset, size);
}

void VulkanDriver::bindSamplers(size_t index, Driver::SamplerGroupHandle sbh) {
    auto* hwsb = handle_cast<VulkanSamplerGroup>(mHandleMap, sbh);
    mSamplerBindings[index] = hwsb;
}

void VulkanDriver::insertEventMarker(char const* string, size_t len) {
}

void VulkanDriver::pushGroupMarker(char const* string,  size_t len) {
    // TODO: Add group marker color to the Driver API
    constexpr float MARKER_COLOR[] = { 0.0f, 1.0f, 0.0f, 1.0f };
    ASSERT_POSTCONDITION(mContext.cmdbuffer,
            "Markers can only be inserted within a beginFrame / endFrame.");
    if (mContext.debugMarkersSupported) {
        VkDebugMarkerMarkerInfoEXT markerInfo = {};
        markerInfo.sType = VK_STRUCTURE_TYPE_DEBUG_MARKER_MARKER_INFO_EXT;
        memcpy(markerInfo.color, &MARKER_COLOR[0], sizeof(MARKER_COLOR));
        markerInfo.pMarkerName = string;
        vkCmdDebugMarkerBeginEXT(mContext.cmdbuffer, &markerInfo);
    }
}

void VulkanDriver::popGroupMarker(int) {
    ASSERT_POSTCONDITION(mContext.cmdbuffer,
            "Markers can only be inserted within a beginFrame / endFrame.");
    if (mContext.debugMarkersSupported) {
        vkCmdDebugMarkerEndEXT(mContext.cmdbuffer);
    }
}

void VulkanDriver::readPixels(Driver::RenderTargetHandle src,
        uint32_t x, uint32_t y, uint32_t width, uint32_t height,
        PixelBufferDescriptor&& p) {
    scheduleDestroy(std::move(p));
}

void VulkanDriver::readStreamPixels(Driver::StreamHandle sh, uint32_t x, uint32_t y, uint32_t width,
        uint32_t height, PixelBufferDescriptor&& p) {
    scheduleDestroy(std::move(p));
}

void VulkanDriver::blit(TargetBufferFlags buffers,
        Driver::RenderTargetHandle dst, driver::Viewport dstRect,
        Driver::RenderTargetHandle src, driver::Viewport srcRect) {
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
                VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, blitRegions, VK_FILTER_LINEAR);

        VulkanTexture::transitionImageLayout(cmdbuffer, dstImage, VK_IMAGE_LAYOUT_UNDEFINED,
                VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, dstLevel, 1);
    };

    if (!mContext.cmdbuffer) {
        mContext.pendingWork.emplace_back(vkblit);
    } else {
        vkblit(mContext.cmdbuffer);
    }
}

void VulkanDriver::draw(Driver::PipelineState pipelineState, Driver::RenderPrimitiveHandle rph) {
    VkCommandBuffer cmdbuffer = mContext.cmdbuffer;
    ASSERT_POSTCONDITION(cmdbuffer, "Draw calls can occur only within a beginFrame / endFrame.");
    const VulkanRenderPrimitive& prim = *handle_cast<VulkanRenderPrimitive>(mHandleMap, rph);

    Driver::ProgramHandle programHandle = pipelineState.program;
    Driver::RasterState rasterState = pipelineState.rasterState;
    Driver::PolygonOffset depthOffset = pipelineState.polygonOffset;

    // If this is a debug build, validate the current shader.
    auto* program = handle_cast<VulkanProgram>(mHandleMap, programHandle);
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

    // Query the program for the mapping from (SamplerBufferBinding,Offset) to (SamplerBinding),
    // where "SamplerBinding" is the integer in the GLSL, and SamplerBufferBinding is the abstract
    // Filament concept used to form groups of samplers.
    for (uint8_t bufferIdx = 0; bufferIdx < VulkanBinder::NUM_SAMPLER_BINDINGS; bufferIdx++) {
        VulkanSamplerGroup* vksb = mSamplerBindings[bufferIdx];
        if (!vksb) {
            continue;
        }
        SamplerGroup* sb = vksb->sb.get();
        for (uint8_t samplerIndex = 0; samplerIndex < sb->getSize(); samplerIndex++) {
            SamplerGroup::Sampler const* sampler = sb->getSamplers() + samplerIndex;
            if (!sampler->t) {
                continue;
            }

            // Obtain the global sampler binding index and pass this to VulkanBinder. Note that
            // "binding" is an offset that is global to the shader, whereas "samplerIndex" is an
            // offset into the virtual sampler buffer.

            if (samplerIndex < program->samplerBindings[bufferIdx].size()) {
                uint8_t binding = (uint8_t)program->samplerBindings[bufferIdx][samplerIndex].binding;
                const SamplerParams& samplerParams = sampler->s;
                VkSampler vksampler = mSamplerCache.getSampler(samplerParams);
                const auto* tex = handle_const_cast<VulkanTexture>(mHandleMap, sampler->t);
                mBinder.bindSampler(binding, {
                    .sampler = vksampler,
                    .imageView = tex->imageView,
                    .imageLayout = samplerParams.depthStencil ?
                            VK_IMAGE_LAYOUT_DEPTH_STENCIL_READ_ONLY_OPTIMAL :
                            VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL
                });
            }
        }
    }

    // Bind a new descriptor set if it needs to change.
    VkDescriptorSet descriptor;
    VkPipelineLayout pipelineLayout;
    if (mBinder.getOrCreateDescriptor(&descriptor, &pipelineLayout)) {
        vkCmdBindDescriptorSets(cmdbuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipelineLayout, 0, 1,
                &descriptor, 0, nullptr);
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

#ifndef NDEBUG
void VulkanDriver::debugCommand(const char* methodName) {
    static const std::set<utils::StaticString> OUTSIDE_COMMANDS = {
        "updateUniformBuffer",
        "updateVertexBuffer",
        "updateIndexBuffer",
        "update2DImage",
        "updateCubeImage",
    };
    static const utils::StaticString BEGIN_COMMAND = "beginRenderPass";
    static const utils::StaticString END_COMMAND = "endRenderPass";
    static bool inRenderPass = false;
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

} // namespace driver

// explicit instantiation of the Dispatcher
template class ConcreteDispatcher<driver::VulkanDriver>;

} // namespace filament

#pragma clang diagnostic pop
