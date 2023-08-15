/*
 * Copyright (C) 2019 The Android Open Source Project
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

#include "backend/PresentCallable.h"
#include "private/backend/CommandStream.h"
#include "CommandStreamDispatcher.h"
#include "metal/MetalDriver.h"

#include "MetalBlitter.h"
#include "MetalContext.h"
#include "MetalDriverFactory.h"
#include "MetalEnums.h"
#include "MetalHandles.h"
#include "MetalState.h"
#include "MetalTimerQuery.h"

#include "MetalPlatform.h"

#include <CoreVideo/CVMetalTexture.h>
#include <CoreVideo/CVPixelBuffer.h>
#include <Metal/Metal.h>
#include <QuartzCore/QuartzCore.h>

#include <utils/Log.h>
#include <utils/Panic.h>

#include <algorithm>

namespace filament {
namespace backend {

Driver* MetalDriverFactory::create(MetalPlatform* const platform, const Platform::DriverConfig& driverConfig) {
    return MetalDriver::create(platform, driverConfig);
}

UTILS_NOINLINE
Driver* MetalDriver::create(MetalPlatform* const platform, const Platform::DriverConfig& driverConfig) {
    assert_invariant(platform);
    size_t defaultSize = FILAMENT_METAL_HANDLE_ARENA_SIZE_IN_MB * 1024U * 1024U;
    Platform::DriverConfig validConfig { .handleArenaSize = std::max(driverConfig.handleArenaSize, defaultSize) };
    return new MetalDriver(platform, validConfig);
}

Dispatcher MetalDriver::getDispatcher() const noexcept {
    return ConcreteDispatcher<MetalDriver>::make();
}

MetalDriver::MetalDriver(MetalPlatform* platform, const Platform::DriverConfig& driverConfig) noexcept
        : mPlatform(*platform),
          mContext(new MetalContext),
          mHandleAllocator("Handles", driverConfig.handleArenaSize) {
    mContext->driver = this;

    mContext->device = mPlatform.createDevice();
    assert_invariant(mContext->device);

    initializeSupportedGpuFamilies(mContext);

    utils::slog.v << "Supported GPU families: " << utils::io::endl;
    if (mContext->highestSupportedGpuFamily.common > 0) {
        utils::slog.v << "  MTLGPUFamilyCommon" << (int) mContext->highestSupportedGpuFamily.common << utils::io::endl;
    }
    if (mContext->highestSupportedGpuFamily.apple > 0) {
        utils::slog.v << "  MTLGPUFamilyApple" << (int) mContext->highestSupportedGpuFamily.apple << utils::io::endl;
    }
    if (mContext->highestSupportedGpuFamily.mac > 0) {
        utils::slog.v << "  MTLGPUFamilyMac" << (int) mContext->highestSupportedGpuFamily.mac << utils::io::endl;
    }
    utils::slog.v << "Features:" << utils::io::endl;
    utils::slog.v << "  readWriteTextureSupport: " <<
            (bool) mContext->device.readWriteTextureSupport << utils::io::endl;

    // In order to support texture swizzling, the GPU needs to support it and the system be running
    // iOS 13+.
    mContext->supportsTextureSwizzling = false;
    if (@available(iOS 13, *)) {
        mContext->supportsTextureSwizzling =
            mContext->highestSupportedGpuFamily.apple >= 1 ||   // all Apple GPUs
            mContext->highestSupportedGpuFamily.mac   >= 2;     // newer macOS GPUs
    }

    // In order to support resolve store action on depth attachment, the GPU needs to support it.
    // Note that support for depth resolve implies support for stencil resolve using .sample0 resolve filter.
    // (Other resolve filters are supported starting .apple5 and .mac2 families).
    mContext->supportsAutoDepthResolve =
        mContext->highestSupportedGpuFamily.apple >= 3 ||
        mContext->highestSupportedGpuFamily.mac   >= 2;

    // In order to support memoryless render targets, an Apple GPU is needed.
    // Available starting macOS 11.0, the first version to run on Apple GPUs.
    // On iOS, it's available on all OS versions.
    mContext->supportsMemorylessRenderTargets = mContext->highestSupportedGpuFamily.apple >= 1;

    mContext->maxColorRenderTargets = 4;
    if (mContext->highestSupportedGpuFamily.apple >= 2 ||
        mContext->highestSupportedGpuFamily.mac >= 1) {
        mContext->maxColorRenderTargets = 8;
    }

    // Round requested sample counts down to the nearest device-supported sample count.
    auto& sc = mContext->sampleCountLookup;
    sc[0] = 1;
    for (uint32_t s = 1; s < sc.size(); s++) {
        sc[s] = [mContext->device supportsTextureSampleCount:s] ? s : sc[s - 1];
    }

    mContext->bugs.a8xStaticTextureTargetError =
            [mContext->device.name containsString:@"Apple A8X GPU"];

    mContext->commandQueue = mPlatform.createCommandQueue(mContext->device);
    mContext->pipelineStateCache.setDevice(mContext->device);
    mContext->depthStencilStateCache.setDevice(mContext->device);
    mContext->samplerStateCache.setDevice(mContext->device);
    mContext->argumentEncoderCache.setDevice(mContext->device);
    mContext->bufferPool = new MetalBufferPool(*mContext);
    mContext->blitter = new MetalBlitter(*mContext);

    if (@available(iOS 12, *)) {
        mContext->timerQueryImpl = new MetalTimerQueryFence(*mContext);
    } else {
        mContext->timerQueryImpl = new TimerQueryNoop();
    }

    CVReturn success = CVMetalTextureCacheCreate(kCFAllocatorDefault, nullptr, mContext->device,
            nullptr, &mContext->textureCache);
    ASSERT_POSTCONDITION(success == kCVReturnSuccess, "Could not create Metal texture cache.");

    if (@available(iOS 12, *)) {
        dispatch_queue_t queue = dispatch_get_global_queue(QOS_CLASS_DEFAULT, 0);
        mContext->eventListener = [[MTLSharedEventListener alloc] initWithDispatchQueue:queue];
    }

#if defined(FILAMENT_METAL_PROFILING)
    mContext->log = os_log_create("com.google.filament", "Metal");
    mContext->signpostId = os_signpost_id_generate(mContext->log);
    assert_invariant(mContext->signpostId != OS_SIGNPOST_ID_NULL);
#endif
}

MetalDriver::~MetalDriver() noexcept {
    mContext->device = nil;
    mContext->emptyTexture = nil;
    CFRelease(mContext->textureCache);
    delete mContext->bufferPool;
    delete mContext->blitter;
    delete mContext->timerQueryImpl;
    delete mContext;
}

void MetalDriver::tick(int) {
}

void MetalDriver::beginFrame(int64_t monotonic_clock_ns, uint32_t frameId) {
#if defined(FILAMENT_METAL_PROFILING)
    os_signpost_interval_begin(mContext->log, mContext->signpostId, "Frame encoding", "%{public}d", frameId);
#endif
}

void MetalDriver::setFrameScheduledCallback(Handle<HwSwapChain> sch,
        FrameScheduledCallback callback, void* user) {
    auto* swapChain = handle_cast<MetalSwapChain>(sch);
    swapChain->setFrameScheduledCallback(callback, user);
}

void MetalDriver::setFrameCompletedCallback(Handle<HwSwapChain> sch,
        FrameCompletedCallback callback, void* user) {
    auto* swapChain = handle_cast<MetalSwapChain>(sch);
    swapChain->setFrameCompletedCallback(callback, user);
}

void MetalDriver::execute(std::function<void(void)> const& fn) noexcept {
    @autoreleasepool {
        fn();
    }
}

void MetalDriver::setPresentationTime(int64_t monotonic_clock_ns) {
}

void MetalDriver::endFrame(uint32_t frameId) {
    // If we haven't committed the command buffer (if the frame was canceled), do it now. There may
    // be commands in it (like fence signaling) that need to execute.
    submitPendingCommands(mContext);

    mContext->bufferPool->gc();

    // If we acquired a drawable for this frame, ensure that we release it here.
    if (mContext->currentDrawSwapChain) {
        mContext->currentDrawSwapChain->releaseDrawable();
    }

    CVMetalTextureCacheFlush(mContext->textureCache, 0);

    assert_invariant(mContext->groupMarkers.empty());

    // If we exceeded memoryless limits, turn it off for the rest of the lifetime of the driver.
    if (mContext->supportsMemorylessRenderTargets && mContext->memorylessLimitsReached) {
        for (MetalTexture* texture : mContext->textures) {
            // Release memoryless MTLTexture-s, which are currently only the MSAA sidecars.
            // Creation of new render targets is going to trigger the re-allocation of sidecars,
            // with private storage mode from now on. Here, at the end of the frame,
            // all render targets that have textures with sidecars are assumed to be destroyed.
            texture->msaaSidecar = nil;
        }
        mContext->supportsMemorylessRenderTargets = false;
    }

#if defined(FILAMENT_METAL_PROFILING)
    os_signpost_interval_end(mContext->log, mContext->signpostId, "Frame encoding");
#endif
}

void MetalDriver::flush(int) {
    ASSERT_PRECONDITION(!isInRenderPass(mContext),
                        "flush must be called outside of a render pass.");
    submitPendingCommands(mContext);
}

void MetalDriver::finish(int) {
    ASSERT_PRECONDITION(!isInRenderPass(mContext),
            "finish must be called outside of a render pass.");
    // Wait for all frames to finish by submitting and waiting on a dummy command buffer.
    submitPendingCommands(mContext);
    id<MTLCommandBuffer> oneOffBuffer = [mContext->commandQueue commandBuffer];
    [oneOffBuffer commit];
    [oneOffBuffer waitUntilCompleted];
}

void MetalDriver::createVertexBufferR(Handle<HwVertexBuffer> vbh, uint8_t bufferCount,
        uint8_t attributeCount, uint32_t vertexCount, AttributeArray attributes) {
    construct_handle<MetalVertexBuffer>(vbh, *mContext, bufferCount,
            attributeCount, vertexCount, attributes);
}

void MetalDriver::createIndexBufferR(Handle<HwIndexBuffer> ibh, ElementType elementType,
        uint32_t indexCount, BufferUsage usage) {
    auto elementSize = (uint8_t) getElementTypeSize(elementType);
    construct_handle<MetalIndexBuffer>(ibh, *mContext, usage, elementSize, indexCount);
}

void MetalDriver::createBufferObjectR(Handle<HwBufferObject> boh, uint32_t byteCount,
        BufferObjectBinding bindingType, BufferUsage usage) {
    construct_handle<MetalBufferObject>(boh, *mContext, bindingType, usage, byteCount);
}

void MetalDriver::createTextureR(Handle<HwTexture> th, SamplerType target, uint8_t levels,
        TextureFormat format, uint8_t samples, uint32_t width, uint32_t height,
        uint32_t depth, TextureUsage usage) {
    // Clamp sample count to what the device supports.
    auto& sc = mContext->sampleCountLookup;
    samples = sc[std::min(MAX_SAMPLE_COUNT, samples)];

    mContext->textures.insert(construct_handle<MetalTexture>(th, *mContext,
            target, levels, format, samples, width, height, depth, usage,
            TextureSwizzle::CHANNEL_0, TextureSwizzle::CHANNEL_1,
            TextureSwizzle::CHANNEL_2, TextureSwizzle::CHANNEL_3));
}

void MetalDriver::createTextureSwizzledR(Handle<HwTexture> th, SamplerType target, uint8_t levels,
        TextureFormat format, uint8_t samples, uint32_t width, uint32_t height,
        uint32_t depth, TextureUsage usage,
        TextureSwizzle r, TextureSwizzle g, TextureSwizzle b, TextureSwizzle a) {
    // Clamp sample count to what the device supports.
    auto& sc = mContext->sampleCountLookup;
    samples = sc[std::min(MAX_SAMPLE_COUNT, samples)];

    mContext->textures.insert(construct_handle<MetalTexture>(th, *mContext,
            target, levels, format, samples, width, height, depth, usage, r, g, b, a));
}

void MetalDriver::importTextureR(Handle<HwTexture> th, intptr_t i,
        SamplerType target, uint8_t levels,
        TextureFormat format, uint8_t samples, uint32_t width, uint32_t height,
        uint32_t depth, TextureUsage usage) {
    id<MTLTexture> metalTexture = (id<MTLTexture>) CFBridgingRelease((void*) i);
    ASSERT_PRECONDITION(metalTexture.width == width,
            "Imported id<MTLTexture> width (%d) != Filament texture width (%d)",
            metalTexture.width, width);
    ASSERT_PRECONDITION(metalTexture.height == height,
            "Imported id<MTLTexture> height (%d) != Filament texture height (%d)",
            metalTexture.height, height);
    ASSERT_PRECONDITION(metalTexture.mipmapLevelCount == levels,
            "Imported id<MTLTexture> levels (%d) != Filament texture levels (%d)",
            metalTexture.mipmapLevelCount, levels);
    MTLTextureType filamentMetalType = getMetalType(target);
    ASSERT_PRECONDITION(metalTexture.textureType == filamentMetalType,
            "Imported id<MTLTexture> type (%d) != Filament texture type (%d)",
            metalTexture.textureType, filamentMetalType);
    mContext->textures.insert(construct_handle<MetalTexture>(th, *mContext,
        target, levels, format, samples, width, height, depth, usage, metalTexture));
}

void MetalDriver::createSamplerGroupR(Handle<HwSamplerGroup> sbh, uint32_t size) {
    mContext->samplerGroups.insert(construct_handle<MetalSamplerGroup>(sbh, size));
}

void MetalDriver::createRenderPrimitiveR(Handle<HwRenderPrimitive> rph,
        Handle<HwVertexBuffer> vbh, Handle<HwIndexBuffer> ibh,
        PrimitiveType pt, uint32_t offset,
        uint32_t minIndex, uint32_t maxIndex, uint32_t count) {
    construct_handle<MetalRenderPrimitive>(rph);
    MetalDriver::setRenderPrimitiveBuffer(rph, vbh, ibh);
    MetalDriver::setRenderPrimitiveRange(rph, pt, offset, minIndex, maxIndex, count);
}

void MetalDriver::createProgramR(Handle<HwProgram> rph, Program&& program) {
    construct_handle<MetalProgram>(rph, mContext->device, program);
}

void MetalDriver::createDefaultRenderTargetR(Handle<HwRenderTarget> rth, int dummy) {
    construct_handle<MetalRenderTarget>(rth, mContext);
}

void MetalDriver::createRenderTargetR(Handle<HwRenderTarget> rth,
        TargetBufferFlags targetBufferFlags, uint32_t width, uint32_t height,
        uint8_t samples, MRT color,
        TargetBufferInfo depth, TargetBufferInfo stencil) {
    ASSERT_PRECONDITION(!isInRenderPass(mContext),
            "createRenderTarget must be called outside of a render pass.");
    // Clamp sample count to what the device supports.
    auto& sc = mContext->sampleCountLookup;
    samples = sc[std::min(MAX_SAMPLE_COUNT, samples)];

    MetalRenderTarget::Attachment colorAttachments[MRT::MAX_SUPPORTED_RENDER_TARGET_COUNT] = {{}};
    for (size_t i = 0; i < MRT::MAX_SUPPORTED_RENDER_TARGET_COUNT; i++) {
        if (none(targetBufferFlags & getTargetBufferFlagsAt(i))) {
            continue;
        }
        const auto& buffer = color[i];
        ASSERT_PRECONDITION(buffer.handle,
                "The COLOR%u flag was specified, but invalid color handle provided.", i);
        auto colorTexture = handle_cast<MetalTexture>(buffer.handle);
        ASSERT_PRECONDITION(colorTexture->getMtlTextureForWrite(),
                "Color texture passed to render target has no texture allocation");
        colorTexture->updateLodRange(buffer.level);
        colorAttachments[i] = { colorTexture, color[i].level, color[i].layer };
    }

    MetalRenderTarget::Attachment depthAttachment = {};
    if (any(targetBufferFlags & TargetBufferFlags::DEPTH)) {
        ASSERT_PRECONDITION(depth.handle,
                "The DEPTH flag was specified, but invalid depth handle provided.");
        auto depthTexture = handle_cast<MetalTexture>(depth.handle);
        ASSERT_PRECONDITION(depthTexture->getMtlTextureForWrite(),
                "Depth texture passed to render target has no texture allocation.");
        depthTexture->updateLodRange(depth.level);
        depthAttachment = { depthTexture, depth.level, depth.layer };
    }

    MetalRenderTarget::Attachment stencilAttachment = {};
    if (any(targetBufferFlags & TargetBufferFlags::STENCIL)) {
        ASSERT_PRECONDITION(stencil.handle,
                "The STENCIL flag was specified, but invalid stencil handle provided.");
        auto stencilTexture = handle_cast<MetalTexture>(stencil.handle);
        ASSERT_PRECONDITION(stencilTexture->getMtlTextureForWrite(),
                "Stencil texture passed to render target has no texture allocation.");
        stencilTexture->updateLodRange(stencil.level);
        stencilAttachment = { stencilTexture, stencil.level, stencil.layer };
    }

    construct_handle<MetalRenderTarget>(rth, mContext, width, height, samples,
            colorAttachments, depthAttachment, stencilAttachment);
}

void MetalDriver::createFenceR(Handle<HwFence> fh, int dummy) {
    auto* fence = handle_cast<MetalFence>(fh);
    fence->encode();
}

void MetalDriver::createSwapChainR(Handle<HwSwapChain> sch, void* nativeWindow, uint64_t flags) {
    if (UTILS_UNLIKELY(flags & SWAP_CHAIN_CONFIG_APPLE_CVPIXELBUFFER)) {
        CVPixelBufferRef pixelBuffer = (CVPixelBufferRef) nativeWindow;
        construct_handle<MetalSwapChain>(sch, *mContext, pixelBuffer, flags);
    } else {
        auto* metalLayer = (__bridge CAMetalLayer*) nativeWindow;
        construct_handle<MetalSwapChain>(sch, *mContext, metalLayer, flags);
    }
}

void MetalDriver::createSwapChainHeadlessR(Handle<HwSwapChain> sch,
        uint32_t width, uint32_t height, uint64_t flags) {
    construct_handle<MetalSwapChain>(sch, *mContext, width, height, flags);
}

void MetalDriver::createTimerQueryR(Handle<HwTimerQuery> tqh, int) {
    // nothing to do, timer query was constructed in createTimerQueryS
}

Handle<HwVertexBuffer> MetalDriver::createVertexBufferS() noexcept {
    return alloc_handle<MetalVertexBuffer>();
}

Handle<HwIndexBuffer> MetalDriver::createIndexBufferS() noexcept {
    return alloc_handle<MetalIndexBuffer>();
}

Handle<HwBufferObject> MetalDriver::createBufferObjectS() noexcept {
    return alloc_handle<MetalBufferObject>();
}

Handle<HwTexture> MetalDriver::createTextureS() noexcept {
    return alloc_handle<MetalTexture>();
}

Handle<HwTexture> MetalDriver::createTextureSwizzledS() noexcept {
    return alloc_handle<MetalTexture>();
}

Handle<HwTexture> MetalDriver::importTextureS() noexcept {
    return alloc_handle<MetalTexture>();
}

Handle<HwSamplerGroup> MetalDriver::createSamplerGroupS() noexcept {
    return alloc_handle<MetalSamplerGroup>();
}

Handle<HwRenderPrimitive> MetalDriver::createRenderPrimitiveS() noexcept {
    return alloc_handle<MetalRenderPrimitive>();
}

Handle<HwProgram> MetalDriver::createProgramS() noexcept {
    return alloc_handle<MetalProgram>();
}

Handle<HwRenderTarget> MetalDriver::createDefaultRenderTargetS() noexcept {
    return alloc_handle<MetalRenderTarget>();
}

Handle<HwRenderTarget> MetalDriver::createRenderTargetS() noexcept {
    return alloc_handle<MetalRenderTarget>();
}

Handle<HwFence> MetalDriver::createFenceS() noexcept {
    // The handle must be constructed here, as a synchronous call to wait might happen before
    // createFenceR is executed.
    return alloc_and_construct_handle<MetalFence, HwFence>(*mContext);
}

Handle<HwSwapChain> MetalDriver::createSwapChainS() noexcept {
    return alloc_handle<MetalSwapChain>();
}

Handle<HwSwapChain> MetalDriver::createSwapChainHeadlessS() noexcept {
    return alloc_handle<MetalSwapChain>();
}

Handle<HwTimerQuery> MetalDriver::createTimerQueryS() noexcept {
    // The handle must be constructed here, as a synchronous call to getTimerQueryValue might happen
    // before createTimerQueryR is executed.
    return alloc_and_construct_handle<MetalTimerQuery, HwTimerQuery>();
}

void MetalDriver::destroyVertexBuffer(Handle<HwVertexBuffer> vbh) {
    if (vbh) {
        destruct_handle<MetalVertexBuffer>(vbh);
    }
}

void MetalDriver::destroyIndexBuffer(Handle<HwIndexBuffer> ibh) {
    if (ibh) {
        destruct_handle<MetalIndexBuffer>(ibh);
    }
}

void MetalDriver::destroyBufferObject(Handle<HwBufferObject> boh) {
    if (UTILS_UNLIKELY(!boh)) {
        return;
    }
    auto* bo = handle_cast<MetalBufferObject>(boh);
    // Unbind this buffer object from any uniform / SSBO slots it's still bound to.
    bo->boundUniformBuffers.forEachSetBit([this](size_t index) {
        mContext->uniformState[index] = BufferState {
                .buffer = nullptr,
                .offset = 0,
                .bound = false
        };
    });
    bo->boundSsbos.forEachSetBit([this](size_t index) {
        mContext->ssboState[index] = BufferState {
                .buffer = nullptr,
                .offset = 0,
                .bound = false
        };
    });
    destruct_handle<MetalBufferObject>(boh);
}

void MetalDriver::destroyRenderPrimitive(Handle<HwRenderPrimitive> rph) {
    if (rph) {
        destruct_handle<MetalRenderPrimitive>(rph);
    }
}

void MetalDriver::destroyProgram(Handle<HwProgram> ph) {
    if (ph) {
        destruct_handle<MetalProgram>(ph);
    }
}

void MetalDriver::destroySamplerGroup(Handle<HwSamplerGroup> sbh) {
    if (!sbh) {
        return;
    }
    // Unbind this sampler group from our internal state.
    auto* metalSampler = handle_cast<MetalSamplerGroup>(sbh);
    for (auto& samplerBinding : mContext->samplerBindings) {
        if (samplerBinding == metalSampler) {
            samplerBinding = {};
        }
    }
    mContext->samplerGroups.erase(metalSampler);
    destruct_handle<MetalSamplerGroup>(sbh);
}

void MetalDriver::destroyTexture(Handle<HwTexture> th) {
    if (!th) {
        return;
    }

    mContext->textures.erase(handle_cast<MetalTexture>(th));
    destruct_handle<MetalTexture>(th);
}

void MetalDriver::destroyRenderTarget(Handle<HwRenderTarget> rth) {
    if (rth) {
        destruct_handle<MetalRenderTarget>(rth);
    }
}

void MetalDriver::destroySwapChain(Handle<HwSwapChain> sch) {
    if (sch) {
        destruct_handle<MetalSwapChain>(sch);
    }
}

void MetalDriver::destroyStream(Handle<HwStream> sh) {
    // no-op
}

void MetalDriver::destroyTimerQuery(Handle<HwTimerQuery> tqh) {
    if (tqh) {
        destruct_handle<MetalTimerQuery>(tqh);
    }
}

void MetalDriver::terminate() {
    // finish() will flush the pending command buffer and will ensure all GPU work has finished.
    // This must be done before calling bufferPool->reset() to ensure no buffers are in flight.
    finish();

    mContext->bufferPool->reset();
    mContext->commandQueue = nil;

    MetalExternalImage::shutdown(*mContext);
    mContext->blitter->shutdown();
}

ShaderModel MetalDriver::getShaderModel() const noexcept {
#if defined(IOS)
    return ShaderModel::MOBILE;
#else
    return ShaderModel::DESKTOP;
#endif
}

Handle<HwStream> MetalDriver::createStreamNative(void* stream) {
    return {};
}

Handle<HwStream> MetalDriver::createStreamAcquired() {
    return {};
}

void MetalDriver::setAcquiredImage(Handle<HwStream> sh, void* image,
        CallbackHandler* handler, StreamCallback cb, void* userData) {
}

void MetalDriver::setStreamDimensions(Handle<HwStream> stream, uint32_t width,
        uint32_t height) {

}

int64_t MetalDriver::getStreamTimestamp(Handle<HwStream> stream) {
    return 0;
}

void MetalDriver::updateStreams(DriverApi* driver) {

}

void MetalDriver::destroyFence(Handle<HwFence> fh) {
    if (fh) {
        destruct_handle<MetalFence>(fh);
    }
}

FenceStatus MetalDriver::getFenceStatus(Handle<HwFence> fh) {
    auto* fence = handle_cast<MetalFence>(fh);
    if (!fence) {
        return FenceStatus::ERROR;
    }
    return fence->wait(0);
}

bool MetalDriver::isTextureFormatSupported(TextureFormat format) {
    return MetalTexture::decidePixelFormat(mContext, format) != MTLPixelFormatInvalid;
}

bool MetalDriver::isTextureSwizzleSupported() {
    return mContext->supportsTextureSwizzling;
}

bool MetalDriver::isTextureFormatMipmappable(TextureFormat format) {
    // Derived from the Metal 3.0 Feature Set Tables.
    // In order for a format to be mipmappable, it must be color-renderable and filterable.
    MTLPixelFormat metalFormat = MetalTexture::decidePixelFormat(mContext, format);
    switch (metalFormat) {
        // Mipmappable across all devices:
        case MTLPixelFormatR8Unorm:
        case MTLPixelFormatR8Snorm:
        case MTLPixelFormatR16Float:
        case MTLPixelFormatRG8Unorm:
        case MTLPixelFormatRG8Snorm:
        case MTLPixelFormatRG16Float:
        case MTLPixelFormatRGBA8Unorm:
        case MTLPixelFormatRGBA8Unorm_sRGB:
        case MTLPixelFormatRGBA8Snorm:
        case MTLPixelFormatRGB10A2Unorm:
        case MTLPixelFormatRG11B10Float:
        case MTLPixelFormatRGBA16Float:
            return true;

#if !defined(IOS)
        // Mipmappable only on desktop:
        case MTLPixelFormatR32Float:
        case MTLPixelFormatRG32Float:
        case MTLPixelFormatRGBA32Float:
            return true;
#endif

#if defined(IOS)
        // Mipmappable only on iOS:
        case MTLPixelFormatRGB9E5Float:
            return true;
#endif

        default:
            return false;
    }
}

bool MetalDriver::isRenderTargetFormatSupported(TextureFormat format) {
    MTLPixelFormat mtlFormat = getMetalFormat(mContext, format);
    // RGB9E5 isn't supported on Mac as a color render target.
    return mtlFormat != MTLPixelFormatInvalid && mtlFormat != MTLPixelFormatRGB9E5Float;
}

bool MetalDriver::isFrameBufferFetchSupported() {
    // FrameBuffer fetch is achievable via "programmable blending" in Metal, and only supported on
    // Apple GPUs with readWriteTextureSupport.
    return mContext->highestSupportedGpuFamily.apple >= 1 &&
            mContext->device.readWriteTextureSupport;
}

bool MetalDriver::isFrameBufferFetchMultiSampleSupported() {
    return isFrameBufferFetchSupported();
}

bool MetalDriver::isFrameTimeSupported() {
    // Frame time is calculated via hard fences, which are only available on iOS 12 and above.
    if (@available(iOS 12, *)) {
        return true;
    }
    return false;
}

bool MetalDriver::isAutoDepthResolveSupported() {
    return mContext->supportsAutoDepthResolve;
}

bool MetalDriver::isSRGBSwapChainSupported() {
    // the SWAP_CHAIN_CONFIG_SRGB_COLORSPACE flag is not supported
    return false;
}

bool MetalDriver::isWorkaroundNeeded(Workaround workaround) {
    switch (workaround) {
        case Workaround::SPLIT_EASU:
            return false;
        case Workaround::ALLOW_READ_ONLY_ANCILLARY_FEEDBACK_LOOP:
            return true;
        case Workaround::ADRENO_UNIFORM_ARRAY_CRASH:
            return false;
        case Workaround::A8X_STATIC_TEXTURE_TARGET_ERROR:
            return mContext->bugs.a8xStaticTextureTargetError;
        case Workaround::DISABLE_BLIT_INTO_TEXTURE_ARRAY:
            return false;
        default:
            return false;
    }
    return false;
}

FeatureLevel MetalDriver::getFeatureLevel() {
    // FEATURE_LEVEL_3 requires >= 31 textures, which all Metal devices support. However, older
    // Metal devices only support 16 unique samplers. We could get around this in the future by
    // virtualizing samplers.
    if (mContext->highestSupportedGpuFamily.apple >= 6 ||
            mContext->highestSupportedGpuFamily.mac >= 2) {
        return FeatureLevel::FEATURE_LEVEL_3;
    }
    return FeatureLevel::FEATURE_LEVEL_2;
}

math::float2 MetalDriver::getClipSpaceParams() {
    // virtual and physical z-coordinate of clip-space is in [-w, 0]
    // Note: this is actually never used (see: main.vs), but it's a backend API so we implement it
    // properly.
    return math::float2{ 1.0f, 0.0f };
}

uint8_t MetalDriver::getMaxDrawBuffers() {
    return std::min(mContext->maxColorRenderTargets, MRT::MAX_SUPPORTED_RENDER_TARGET_COUNT);
}

size_t MetalDriver::getMaxUniformBufferSize() {
    return 256 * 1024 * 1024;   // TODO: return the actual size instead of hardcoding the minspec
}

void MetalDriver::updateIndexBuffer(Handle<HwIndexBuffer> ibh, BufferDescriptor&& data,
        uint32_t byteOffset) {
    auto* ib = handle_cast<MetalIndexBuffer>(ibh);
    ib->buffer.copyIntoBuffer(data.buffer, data.size, byteOffset);
    scheduleDestroy(std::move(data));
}

void MetalDriver::updateBufferObject(Handle<HwBufferObject> boh, BufferDescriptor&& data,
        uint32_t byteOffset) {
    ASSERT_PRECONDITION(!isInRenderPass(mContext),
            "updateBufferObject must be called outside of a render pass.");
    auto* bo = handle_cast<MetalBufferObject>(boh);
    bo->updateBuffer(data.buffer, data.size, byteOffset);
    scheduleDestroy(std::move(data));
}

void MetalDriver::updateBufferObjectUnsynchronized(Handle<HwBufferObject> boh,
        BufferDescriptor&& data, uint32_t byteOffset) {
    auto* bo = handle_cast<MetalBufferObject>(boh);
    bo->updateBufferUnsynchronized(data.buffer, data.size, byteOffset);
    scheduleDestroy(std::move(data));
}

void MetalDriver::resetBufferObject(Handle<HwBufferObject> boh) {
    // TODO: implement resetBufferObject(). This is equivalent to calling
    // destroyBufferObject() followed by createBufferObject() keeping the same handle.
    // It is actually okay to keep a no-op implementation, the intention here is to "orphan" the
    // buffer (and possibly return it to a pool) and allocate a new one (or get it from a pool),
    // so that no further synchronization with the GPU is needed.
    // This is only useful if updateBufferObjectUnsynchronized() is implemented unsynchronizedly.
}

void MetalDriver::setVertexBufferObject(Handle<HwVertexBuffer> vbh, uint32_t index,
        Handle<HwBufferObject> boh) {
    auto* vertexBuffer = handle_cast<MetalVertexBuffer>(vbh);
    auto* bufferObject = handle_cast<MetalBufferObject>(boh);
    assert_invariant(index < vertexBuffer->buffers.size());
    vertexBuffer->buffers[index] = bufferObject->getBuffer();
}

void MetalDriver::setMinMaxLevels(Handle<HwTexture> th, uint32_t minLevel, uint32_t maxLevel) {
}

void MetalDriver::update3DImage(Handle<HwTexture> th, uint32_t level,
        uint32_t xoffset, uint32_t yoffset, uint32_t zoffset,
        uint32_t width, uint32_t height, uint32_t depth,
        PixelBufferDescriptor&& data) {
    ASSERT_PRECONDITION(!isInRenderPass(mContext),
            "update3DImage must be called outside of a render pass.");
    auto tex = handle_cast<MetalTexture>(th);
    tex->loadImage(level, MTLRegionMake3D(xoffset, yoffset, zoffset, width, height, depth), data);
    scheduleDestroy(std::move(data));
}

void MetalDriver::setupExternalImage(void* image) {
    // This is called when passing in a CVPixelBuffer as either an external image or swap chain.
    // Here we take ownership of the passed in buffer. It will be released the next time
    // setExternalImage is called, when the texture is destroyed, or when the swap chain is
    // destroyed.
    CVPixelBufferRef pixelBuffer = (CVPixelBufferRef) image;
    CVPixelBufferRetain(pixelBuffer);
}

void MetalDriver::setExternalImage(Handle<HwTexture> th, void* image) {
    ASSERT_PRECONDITION(!isInRenderPass(mContext),
            "setExternalImage must be called outside of a render pass.");
    auto texture = handle_cast<MetalTexture>(th);
    texture->externalImage.set((CVPixelBufferRef) image);
}

void MetalDriver::setExternalImagePlane(Handle<HwTexture> th, void* image, uint32_t plane) {
    ASSERT_PRECONDITION(!isInRenderPass(mContext),
            "setExternalImagePlane must be called outside of a render pass.");
    auto texture = handle_cast<MetalTexture>(th);
    texture->externalImage.set((CVPixelBufferRef) image, plane);
}

void MetalDriver::setExternalStream(Handle<HwTexture> th, Handle<HwStream> sh) {
}

bool MetalDriver::getTimerQueryValue(Handle<HwTimerQuery> tqh, uint64_t* elapsedTime) {
    auto* tq = handle_cast<MetalTimerQuery>(tqh);
    return mContext->timerQueryImpl->getQueryResult(tq, elapsedTime);
}

void MetalDriver::generateMipmaps(Handle<HwTexture> th) {
    ASSERT_PRECONDITION(!isInRenderPass(mContext),
                        "generateMipmaps must be called outside of a render pass.");
    auto tex = handle_cast<MetalTexture>(th);
    tex->generateMipmaps();
}

bool MetalDriver::canGenerateMipmaps() {
    return true;
}

void MetalDriver::updateSamplerGroup(Handle<HwSamplerGroup> sbh, BufferDescriptor&& data) {
    ASSERT_PRECONDITION(!isInRenderPass(mContext),
            "updateSamplerGroup must be called outside of a render pass.");

    auto sb = handle_cast<MetalSamplerGroup>(sbh);
    assert_invariant(sb->size == data.size / sizeof(SamplerDescriptor));
    auto const* const samplers = (SamplerDescriptor const*) data.buffer;

#ifndef NDEBUG
    // In debug builds, verify that all the textures in the sampler group are still alive.
    // These bugs lead to memory corruption and can be difficult to track down.
    for (size_t s = 0; s < data.size / sizeof(SamplerDescriptor); s++) {
        if (!samplers[s].t) {
            continue;
        }
        auto iter = mContext->textures.find(handle_cast<MetalTexture>(samplers[s].t));
        if (iter == mContext->textures.end()) {
            utils::slog.e << "updateSamplerGroup: texture #"
                          << (int) s << " is dead, texture handle = "
                          << samplers[s].t << utils::io::endl;
        }
        assert_invariant(iter != mContext->textures.end());
    }
#endif

    // Create a MTLArgumentEncoder for these textures.
    // Ideally, we would create this encoder at createSamplerGroup time, but we need to know the
    // texture type of each texture. It's also not guaranteed that the types won't change between
    // calls to updateSamplerGroup.
    utils::FixedCapacityVector<MTLTextureType> textureTypes(sb->size);
    std::transform(samplers, samplers + data.size / sizeof(SamplerDescriptor), textureTypes.begin(),
            [this](const SamplerDescriptor& sampler) {
        if (!sampler.t) {
            // Use Type2D for non-bound textures.
            return MTLTextureType2D;
        }
        auto* t = handle_cast<MetalTexture>(sampler.t);
        if (t->target == SamplerType::SAMPLER_EXTERNAL) {
            // Use Type2D for external image textures.
            return MTLTextureType2D;
        }
        id<MTLTexture> mtlTexture = t->getMtlTextureForRead();
        assert_invariant(mtlTexture);
        return mtlTexture.textureType;
    });
    auto& encoderCache = mContext->argumentEncoderCache;
    id<MTLArgumentEncoder> encoder =
            encoderCache.getOrCreateState(ArgumentEncoderState(std::move(textureTypes)));
    sb->reset(getPendingCommandBuffer(mContext), encoder, mContext->device);

    // In a perfect world, all the MTLTexture bindings would be known at updateSamplerGroup time.
    // However, there are two special cases preventing this:
    // 1. External images
    // 2. LOD-clamped textures
    //
    // Both of these cases prevent us from knowing the final id<MTLTexture> that will be bound into
    // the argument buffer representing the sampler group. So, we bind what we can now and wait
    // until draw call time to bind any special cases (done in finalizeSamplerGroup).
    // The good news is that once a render pass has started, the texture bindings won't change.
    // A SamplerGroup is "finalized" when all of its textures have been set and is ready for use in
    // a draw call.
    // Even if we do know all the final textures at this point, we still wait until draw call time
    // to call finalizeSamplerGroup, which has one additional responsibility: to call useResources
    // for all the textures, which is required by Metal.
    for (size_t s = 0; s < data.size / sizeof(SamplerDescriptor); s++) {
        if (!samplers[s].t) {
            // Assign a default texture / sampler to empty slots.
            // Metal requires all samplers referenced in shaders to be bound.
            id<MTLTexture> empty = getOrCreateEmptyTexture(mContext);
            sb->setFinalizedTexture(s, empty);

            id<MTLSamplerState> sampler = mContext->samplerStateCache.getOrCreateState({});
            sb->setFinalizedSampler(s, sampler);

            continue;
        }

        // First, bind the sampler state. We always know the full sampler state at
        // updateSamplerGroup time.
        SamplerState samplerState {
                .samplerParams = samplers[s].s,
        };
        id<MTLSamplerState> sampler = mContext->samplerStateCache.getOrCreateState(samplerState);
        sb->setFinalizedSampler(s, sampler);

        sb->setTextureHandle(s, samplers[s].t);

        auto* t = handle_cast<MetalTexture>(samplers[s].t);
        assert_invariant(t);

        // If this texture is an external texture, we defer binding the texture until draw call time
        // (in finalizeSamplerGroup).
        if (t->target == SamplerType::SAMPLER_EXTERNAL) {
            continue;
        }

        if (!t->allLodsValid()) {
            // The texture doesn't have all of its LODs loaded, and this could change by the time we
            // issue a draw call with this sampler group. So, we defer binding the texture until
            // draw call time (in finalizeSamplerGroup).
            continue;
        }

        // If we get here, we know we have a valid MTLTexture that's guaranteed not to change.
        id<MTLTexture> mtlTexture = t->getMtlTextureForRead();
        assert_invariant(mtlTexture);
        sb->setFinalizedTexture(s, mtlTexture);
    }

    scheduleDestroy(std::move(data));
}

void MetalDriver::compilePrograms(CompilerPriorityQueue priority,
        CallbackHandler* handler, CallbackHandler::Callback callback, void* user) {
    if (callback) {
        scheduleCallback(handler, user, callback);
    }
}

void MetalDriver::beginRenderPass(Handle<HwRenderTarget> rth,
        const RenderPassParams& params) {

#if defined(FILAMENT_METAL_PROFILING)
    const char* renderPassName = "Unknown";
    if (!mContext->groupMarkers.empty()) {
        renderPassName = mContext->groupMarkers.top();
    }
    os_signpost_interval_begin(mContext->log, OS_SIGNPOST_ID_EXCLUSIVE, "Render pass", "%s",
            renderPassName);
#endif

    auto renderTarget = handle_cast<MetalRenderTarget>(rth);
    mContext->currentRenderTarget = renderTarget;
    mContext->currentRenderPassFlags = params.flags;

    MTLRenderPassDescriptor* descriptor = [MTLRenderPassDescriptor renderPassDescriptor];
    renderTarget->setUpRenderPassAttachments(descriptor, params);

    mContext->currentRenderPassEncoder =
            [getPendingCommandBuffer(mContext) renderCommandEncoderWithDescriptor:descriptor];
    if (!mContext->groupMarkers.empty()) {
        mContext->currentRenderPassEncoder.label =
                [NSString stringWithCString:mContext->groupMarkers.top()
                                   encoding:NSUTF8StringEncoding];
    }

    MTLViewport metalViewport =
            renderTarget->getViewportFromClientViewport(params.viewport,
                    params.depthRange.near,
                    params.depthRange.far);
    [mContext->currentRenderPassEncoder setViewport:metalViewport];
    mContext->currentViewport = metalViewport;

    // Metal requires a new command encoder for each render pass, and they cannot be reused.
    // We must bind certain states for each command encoder, so we dirty the states here to force a
    // rebinding at the first the draw call of this pass.
    mContext->pipelineState.invalidate();
    mContext->depthStencilState.invalidate();
    mContext->cullModeState.invalidate();
    mContext->windingState.invalidate();
    mContext->currentPolygonOffset = {0.0f, 0.0f};

    mContext->finalizedSamplerGroups.clear();
}

void MetalDriver::nextSubpass(int dummy) {}

void MetalDriver::endRenderPass(int dummy) {
#if defined(FILAMENT_METAL_PROFILING)
    os_signpost_interval_end(mContext->log, OS_SIGNPOST_ID_EXCLUSIVE, "Render pass");
#endif

    [mContext->currentRenderPassEncoder endEncoding];

    // Command encoders are one time use. Set it to nil to release the encoder and ensure we don't
    // accidentally use it again.
    mContext->currentRenderPassEncoder = nil;
}

void MetalDriver::setRenderPrimitiveBuffer(Handle<HwRenderPrimitive> rph,
        Handle<HwVertexBuffer> vbh, Handle<HwIndexBuffer> ibh) {
    auto primitive = handle_cast<MetalRenderPrimitive>(rph);
    auto vertexBuffer = handle_cast<MetalVertexBuffer>(vbh);
    auto indexBuffer = handle_cast<MetalIndexBuffer>(ibh);
    primitive->setBuffers(vertexBuffer, indexBuffer);
}

void MetalDriver::setRenderPrimitiveRange(Handle<HwRenderPrimitive> rph,
        PrimitiveType pt, uint32_t offset, uint32_t minIndex, uint32_t maxIndex,
        uint32_t count) {
    auto primitive = handle_cast<MetalRenderPrimitive>(rph);
    primitive->type = pt;
    primitive->offset = offset * primitive->indexBuffer->elementSize;
    primitive->count = count;
    primitive->minIndex = minIndex;
    primitive->maxIndex = maxIndex > minIndex ? maxIndex : primitive->maxVertexCount - 1;
}

void MetalDriver::makeCurrent(Handle<HwSwapChain> schDraw, Handle<HwSwapChain> schRead) {
    ASSERT_PRECONDITION_NON_FATAL(schDraw, "A draw SwapChain must be set.");
    auto* drawSwapChain = handle_cast<MetalSwapChain>(schDraw);
    mContext->currentDrawSwapChain = drawSwapChain;

    if (schRead) {
        auto* readSwapChain = handle_cast<MetalSwapChain>(schRead);
        mContext->currentReadSwapChain = readSwapChain;
    }
}

void MetalDriver::commit(Handle<HwSwapChain> sch) {
    auto* swapChain = handle_cast<MetalSwapChain>(sch);
    swapChain->present();
    submitPendingCommands(mContext);
    swapChain->releaseDrawable();
}

void MetalDriver::bindUniformBuffer(uint32_t index, Handle<HwBufferObject> boh) {
    assert_invariant(index < Program::UNIFORM_BINDING_COUNT);
    auto* bo = handle_cast<MetalBufferObject>(boh);
    auto* currentBo = mContext->uniformState[index].buffer;
    if (currentBo) {
        currentBo->boundUniformBuffers.unset(index);
    }
    bo->boundUniformBuffers.set(index);
    mContext->uniformState[index] = BufferState{
            .buffer = bo,
            .offset = 0,
            .bound = true
    };
}

void MetalDriver::bindBufferRange(BufferObjectBinding bindingType, uint32_t index,
        Handle<HwBufferObject> boh,  uint32_t offset, uint32_t size) {

    assert_invariant(bindingType == BufferObjectBinding::SHADER_STORAGE ||
                     bindingType == BufferObjectBinding::UNIFORM);

    auto* bo = handle_cast<MetalBufferObject>(boh);

    switch (bindingType) {
        default:
        case BufferObjectBinding::UNIFORM: {
            assert_invariant(index < Program::UNIFORM_BINDING_COUNT);
            auto* currentBo = mContext->uniformState[index].buffer;
            if (currentBo) {
                currentBo->boundUniformBuffers.unset(index);
            }
            bo->boundUniformBuffers.set(index);
            mContext->uniformState[index] = BufferState {
                    .buffer = bo,
                    .offset = offset,
                    .bound = true
            };

            break;
        }

        case BufferObjectBinding::SHADER_STORAGE: {
            assert_invariant(index < MAX_SSBO_COUNT);
            auto* currentBo = mContext->ssboState[index].buffer;
            if (currentBo) {
                currentBo->boundSsbos.unset(index);
            }
            bo->boundSsbos.set(index);
            mContext->ssboState[index] = BufferState {
                    .buffer = bo,
                    .offset = offset,
                    .bound = true
            };

            break;
        }
    }
}

void MetalDriver::unbindBuffer(BufferObjectBinding bindingType, uint32_t index) {

    assert_invariant(bindingType == BufferObjectBinding::SHADER_STORAGE ||
                     bindingType == BufferObjectBinding::UNIFORM);

    switch (bindingType) {
        default:
        case BufferObjectBinding::UNIFORM: {
            assert_invariant(index < Program::UNIFORM_BINDING_COUNT);
            auto* currentBo = mContext->uniformState[index].buffer;
            if (currentBo) {
                currentBo->boundUniformBuffers.unset(index);
            }
            mContext->uniformState[index] = BufferState {
                    .buffer = nullptr,
                    .offset = 0,
                    .bound = false
            };

            break;
        }

        case BufferObjectBinding::SHADER_STORAGE: {
            assert_invariant(index < MAX_SSBO_COUNT);
            auto* currentBo = mContext->ssboState[index].buffer;
            if (currentBo) {
                currentBo->boundSsbos.unset(index);
            }
            mContext->ssboState[index] = BufferState {
                    .buffer = nullptr,
                    .offset = 0,
                    .bound = false
            };

            break;
        }
    }
}

void MetalDriver::bindSamplers(uint32_t index, Handle<HwSamplerGroup> sbh) {
    auto sb = handle_cast<MetalSamplerGroup>(sbh);
    mContext->samplerBindings[index] = sb;
}

void MetalDriver::insertEventMarker(const char* string, uint32_t len) {

}

void MetalDriver::pushGroupMarker(const char* string, uint32_t len) {
    mContext->groupMarkers.push(string);
}

void MetalDriver::popGroupMarker(int) {
    assert_invariant(!mContext->groupMarkers.empty());
    mContext->groupMarkers.pop();
}

void MetalDriver::startCapture(int) {
    // Submit any pending command buffers. Metal will only capture command buffers created and
    // submitted during the capture period.
    submitPendingCommands(mContext);
    if (@available(iOS 13, *)) {
        MTLCaptureDescriptor* descriptor = [MTLCaptureDescriptor new];
        descriptor.captureObject = mContext->device;
#if defined(IOS)
        descriptor.destination = MTLCaptureDestinationDeveloperTools;
#else
        descriptor.destination = MTLCaptureDestinationGPUTraceDocument;
        descriptor.outputURL = [[NSURL alloc] initFileURLWithPath:@"filament.gputrace"];
#endif
        NSError* error = nil;
        [[MTLCaptureManager sharedCaptureManager] startCaptureWithDescriptor:descriptor
                                                                           error:&error];
        if (error) {
            NSLog(@"%@", [error localizedDescription]);
        }
    } else {
        // This compile-time check is used to silence deprecation warnings when compiling for the
        // iOS simulator, which only supports Metal on iOS 13.0+.
#if (TARGET_OS_IOS && __IPHONE_OS_VERSION_MIN_REQUIRED < __IPHONE_13_0)
        [[MTLCaptureManager sharedCaptureManager] startCaptureWithDevice:mContext->device];
#endif
    }
}

void MetalDriver::stopCapture(int) {
    // Submit any pending command buffers. Metal will only capture command buffers created and
    // submitted during the capture period.
    submitPendingCommands(mContext);
    [[MTLCaptureManager sharedCaptureManager] stopCapture];
}

void MetalDriver::readPixels(Handle<HwRenderTarget> src, uint32_t x, uint32_t y, uint32_t width,
        uint32_t height, PixelBufferDescriptor&& data) {
    ASSERT_PRECONDITION(!isInRenderPass(mContext),
                        "readPixels must be called outside of a render pass.");

    auto srcTarget = handle_cast<MetalRenderTarget>(src);
    // We always readPixels from the COLOR0 attachment.
    MetalRenderTarget::Attachment color = srcTarget->getDrawColorAttachment(0);
    id<MTLTexture> srcTexture = color.getTexture();
    size_t miplevel = color.level;

    // Clamp height and width to actual texture's height and width
    MTLSize srcTextureSize = MTLSizeMake(srcTexture.width >> miplevel, srcTexture.height >> miplevel, 1);
    height = std::min(static_cast<uint32_t>(srcTextureSize.height), height);
    width = std::min(static_cast<uint32_t>(srcTextureSize.width), width);

    const MTLPixelFormat format = getMetalFormat(data.format, data.type);
    ASSERT_PRECONDITION(format != MTLPixelFormatInvalid,
            "The chosen combination of PixelDataFormat (%d) and PixelDataType (%d) is not supported for "
            "readPixels.", (int) data.format, (int) data.type);

    const bool formatConversionNecessary = srcTexture.pixelFormat != format;

    // TODO: MetalBlitter does not currently support format conversions to integer types.
    // The format and type must match the source pixel format exactly.
    ASSERT_PRECONDITION(!formatConversionNecessary || !isMetalFormatInteger(format),
            "readPixels does not support integer format conversions from MTLPixelFormat (%d) to (%d).",
            (int) srcTexture.pixelFormat, (int) format);

    MTLTextureDescriptor* textureDescriptor =
            [MTLTextureDescriptor texture2DDescriptorWithPixelFormat:format
                                                               width:srcTextureSize.width
                                                              height:srcTextureSize.height
                                                           mipmapped:NO];
#if defined(IOS)
    textureDescriptor.storageMode = MTLStorageModeShared;
#else
    textureDescriptor.storageMode = MTLStorageModeManaged;
#endif
    textureDescriptor.usage = MTLTextureUsageShaderWrite | MTLTextureUsageRenderTarget;
    id<MTLTexture> readPixelsTexture = [mContext->device newTextureWithDescriptor:textureDescriptor];

    MetalBlitter::BlitArgs args;
    args.filter = SamplerMagFilter::NEAREST;
    args.source.level = miplevel;
    args.source.region = MTLRegionMake2D(0, 0, srcTexture.width >> miplevel, srcTexture.height >> miplevel);
    args.destination.level = 0;
    args.destination.region = MTLRegionMake2D(0, 0, readPixelsTexture.width, readPixelsTexture.height);
    args.source.color = srcTexture;
    args.destination.color = readPixelsTexture;

    mContext->blitter->blit(getPendingCommandBuffer(mContext), args, "readPixels blit");

#if !defined(IOS)
    // Managed textures on macOS require explicit synchronization between GPU / CPU.
    id <MTLBlitCommandEncoder> blitEncoder = [getPendingCommandBuffer(mContext) blitCommandEncoder];
    [blitEncoder synchronizeResource:readPixelsTexture];
    [blitEncoder endEncoding];
#endif

    PixelBufferDescriptor* p = new PixelBufferDescriptor(std::move(data));
    [getPendingCommandBuffer(mContext) addCompletedHandler:^(id<MTLCommandBuffer> cb) {
        size_t stride = p->stride ? p->stride : width;
        size_t bpp = PixelBufferDescriptor::computeDataSize(p->format, p->type, 1, 1, 1);
        size_t bpr = PixelBufferDescriptor::computeDataSize(p->format, p->type, stride, 1, p->alignment);
        // Metal's texture coordinates have (0, 0) at the top-left of the texture, but readPixels
        // assumes (0, 0) at bottom-left.
        MTLRegion srcRegion = MTLRegionMake2D(x, readPixelsTexture.height - y - height, width, height);
        const uint8_t* bufferStart = (const uint8_t*) p->buffer + (p->left * bpp) +
                                                                  (p->top * bpr);
        [readPixelsTexture getBytes:(void*) bufferStart
                        bytesPerRow:bpr
                         fromRegion:srcRegion
                        mipmapLevel:0];
        scheduleDestroy(std::move(*p));
        delete p;
    }];
}

void MetalDriver::readBufferSubData(backend::BufferObjectHandle boh,
        uint32_t offset, uint32_t size, backend::BufferDescriptor&& p) {
    // TODO: implement readBufferSubData
    scheduleDestroy(std::move(p));
}

void MetalDriver::blit(TargetBufferFlags buffers,
        Handle<HwRenderTarget> dst, Viewport dstRect,
        Handle<HwRenderTarget> src, Viewport srcRect,
        SamplerMagFilter filter) {
    // If we're the in middle of a render pass, finish it.
    // This condition should only occur during copyFrame. It's okay to end the render pass because
    // we don't issue any other rendering commands.
    if (mContext->currentRenderPassEncoder) {
        [mContext->currentRenderPassEncoder endEncoding];
        mContext->currentRenderPassEncoder = nil;
    }

    auto srcTarget = handle_cast<MetalRenderTarget>(src);
    auto dstTarget = handle_cast<MetalRenderTarget>(dst);

    ASSERT_PRECONDITION(
            !(buffers & (TargetBufferFlags::COLOR_ALL & ~TargetBufferFlags::COLOR0)),
            "Blitting only supports COLOR0");

    ASSERT_PRECONDITION(srcRect.left >= 0 && srcRect.bottom >= 0 &&
                        dstRect.left >= 0 && dstRect.bottom >= 0,
            "Source and destination rects must be positive.");

    auto isBlitableTextureType = [](MTLTextureType t) {
        return t == MTLTextureType2D || t == MTLTextureType2DMultisample ||
               t == MTLTextureType2DArray;
    };

    // MetalBlitter supports blitting color and depth simultaneously, but for simplicitly we'll blit
    // them separately. In practice, Filament only ever blits a single buffer at a time anyway.

    if (any(buffers & TargetBufferFlags::COLOR_ALL)) {
        // We always blit from/to the COLOR0 attachment.
        MetalRenderTarget::Attachment srcColorAttachment = srcTarget->getReadColorAttachment(0);
        MetalRenderTarget::Attachment dstColorAttachment = dstTarget->getDrawColorAttachment(0);

        if (srcColorAttachment && dstColorAttachment) {
            ASSERT_PRECONDITION(isBlitableTextureType(srcColorAttachment.getTexture().textureType) &&
                                isBlitableTextureType(dstColorAttachment.getTexture().textureType),
                               "Metal does not support blitting to/from non-2D textures.");

            MetalBlitter::BlitArgs args;
            args.filter = filter;
            args.source.region = srcTarget->getRegionFromClientRect(srcRect);
            args.destination.region = dstTarget->getRegionFromClientRect(dstRect);
            args.source.color = srcColorAttachment.getTexture();
            args.destination.color = dstColorAttachment.getTexture();
            args.source.level = srcColorAttachment.level;
            args.destination.level = dstColorAttachment.level;
            args.source.slice = srcColorAttachment.layer;
            args.destination.slice = dstColorAttachment.layer;
            mContext->blitter->blit(getPendingCommandBuffer(mContext), args, "Color blit");
        }
    }

    if (any(buffers & TargetBufferFlags::DEPTH)) {
        MetalRenderTarget::Attachment srcDepthAttachment = srcTarget->getDepthAttachment();
        MetalRenderTarget::Attachment dstDepthAttachment = dstTarget->getDepthAttachment();

        if (srcDepthAttachment && dstDepthAttachment) {
            ASSERT_PRECONDITION(isBlitableTextureType(srcDepthAttachment.getTexture().textureType) &&
                                isBlitableTextureType(dstDepthAttachment.getTexture().textureType),
                               "Metal does not support blitting to/from non-2D textures.");

            MetalBlitter::BlitArgs args;
            args.filter = filter;
            args.source.region = srcTarget->getRegionFromClientRect(srcRect);
            args.destination.region = dstTarget->getRegionFromClientRect(dstRect);
            args.source.depth = srcDepthAttachment.getTexture();
            args.destination.depth = dstDepthAttachment.getTexture();
            args.source.level = srcDepthAttachment.level;
            args.destination.level = dstDepthAttachment.level;
            args.source.slice = srcDepthAttachment.layer;
            args.destination.slice = dstDepthAttachment.layer;
            mContext->blitter->blit(getPendingCommandBuffer(mContext), args, "Depth blit");
        }
    }
}

void MetalDriver::finalizeSamplerGroup(MetalSamplerGroup* samplerGroup) {
    // All of the id<MTLSamplerState> objects have already been bound to the argument buffer.
    // Here we bind any textures that were unable to be bound in updateSamplerGroup.

    id<MTLCommandBuffer> cmdBuffer = getPendingCommandBuffer(mContext);

#ifndef NDEBUG
    // In debug builds, verify that all the textures in the sampler group are still alive.
    // These bugs lead to memory corruption and can be difficult to track down.
    const auto& handles = samplerGroup->getTextureHandles();
    for (size_t s = 0; s < handles.size(); s++) {
        if (!handles[s]) {
            continue;
        }
        auto iter = mContext->textures.find(handle_cast<MetalTexture>(handles[s]));
        if (iter == mContext->textures.end()) {
            utils::slog.e << "finalizeSamplerGroup: texture #"
                          << (int) s << " is dead, texture handle = "
                          << handles[s] << utils::io::endl;
        }
        assert_invariant(iter != mContext->textures.end());
    }
#endif

    for (size_t binding = 0; binding < samplerGroup->size; binding++) {
        auto [th, t] = samplerGroup->getFinalizedTexture(binding);

        // This may be an external texture, in which case we can't cache the id<MTLTexture>, we
        // need to refetch it in case the external image has changed.
        bool isExternalImage = false;
        if (th) {
            auto* texture = handle_cast<MetalTexture>(th);
            isExternalImage = texture->target == SamplerType::SAMPLER_EXTERNAL;
        }

        // If t is non-nil, then we've already finalized this texture.
        if (t && !isExternalImage) {
            continue;
        }

        // It's possible that some texture handles are null, but we should have already handled
        // these inside updateSamplerGroup by binding an "empty" texture.
        assert_invariant(th);
        auto* texture = handle_cast<MetalTexture>(th);

        // Determine if this SamplerGroup needs mutation.
        // We can't just simply mutate the SamplerGroup, since it could currently be in use by the
        // GPU from a prior render pass.
        // If the SamplerGroup does need mutation, then there's two cases:
        // 1. The SamplerGroup has not been finalized yet (which means it has not yet been used in a
        //    draw call). We're free to mutate it.
        // 2. The SamplerGroup is finalized. We must call mutate(), which will create a new argument
        //    buffer that we can then mutate freely.
        // TODO: don't just always call mutate, check to see if the texture is actually different.

        if (samplerGroup->isFinalized()) {
            samplerGroup->mutate(cmdBuffer);
        }

        // External images
        if (texture->target == SamplerType::SAMPLER_EXTERNAL) {
            if (texture->externalImage.isValid()) {
                id<MTLTexture> mtlTexture = texture->externalImage.getMetalTextureForDraw();
                assert_invariant(mtlTexture);
                samplerGroup->setFinalizedTexture(binding, mtlTexture);
            } else {
                // Bind an empty texture.
                samplerGroup->setFinalizedTexture(binding, getOrCreateEmptyTexture(mContext));
            }
            continue;
        }

        samplerGroup->setFinalizedTexture(binding, texture->getMtlTextureForRead());
    }

    samplerGroup->finalize();

    // At this point, all the id<MTLTextures> should be set to valid textures. Some of them will be
    // the "empty" texture. Per Apple documentation, the useResource method must be called once per
    // render pass.
    samplerGroup->useResources(mContext->currentRenderPassEncoder);

    // useResources won't retain references to the textures, so we need to do so manually.
    for (id<MTLTexture> texture : samplerGroup->textures) {
        const void* retainedTexture = CFBridgingRetain(texture);
        [cmdBuffer addCompletedHandler:^(id<MTLCommandBuffer> cb) {
            CFBridgingRelease(retainedTexture);
        }];
    }
}

void MetalDriver::draw(PipelineState ps, Handle<HwRenderPrimitive> rph, uint32_t instanceCount) {
    ASSERT_PRECONDITION(mContext->currentRenderPassEncoder != nullptr,
            "Attempted to draw without a valid command encoder.");
    auto primitive = handle_cast<MetalRenderPrimitive>(rph);
    auto program = handle_cast<MetalProgram>(ps.program);
    const auto& rs = ps.rasterState;

    // If the material debugger is enabled, avoid fatal (or cascading) errors and that can occur
    // during the draw call when the program is invalid. The shader compile error has already been
    // dumped to the console at this point, so it's fine to simply return early.
    if (FILAMENT_ENABLE_MATDBG && UTILS_UNLIKELY(!program->isValid)) {
        return;
    }

    ASSERT_PRECONDITION(program->isValid, "Attempting to draw with an invalid Metal program.");

    // Pipeline state
    MTLPixelFormat colorPixelFormat[MRT::MAX_SUPPORTED_RENDER_TARGET_COUNT] = { MTLPixelFormatInvalid };
    for (size_t i = 0; i < MRT::MAX_SUPPORTED_RENDER_TARGET_COUNT; i++) {
        const auto& attachment = mContext->currentRenderTarget->getDrawColorAttachment(i);
        if (!attachment) {
            continue;
        }
        colorPixelFormat[i] = attachment.getPixelFormat();
    }
    MTLPixelFormat depthPixelFormat = MTLPixelFormatInvalid;
    const auto& depthAttachment = mContext->currentRenderTarget->getDepthAttachment();
    if (depthAttachment) {
        depthPixelFormat = depthAttachment.getPixelFormat();
    }
    MTLPixelFormat stencilPixelFormat = MTLPixelFormatInvalid;
    const auto& stencilAttachment = mContext->currentRenderTarget->getStencilAttachment();
    if (stencilAttachment) {
        stencilPixelFormat = stencilAttachment.getPixelFormat();
        assert_invariant(isMetalFormatStencil(stencilPixelFormat));
    }
    MetalPipelineState pipelineState {
        .vertexFunction = program->vertexFunction,
        .fragmentFunction = program->fragmentFunction,
        .vertexDescription = primitive->vertexDescription,
        .colorAttachmentPixelFormat = {
            colorPixelFormat[0],
            colorPixelFormat[1],
            colorPixelFormat[2],
            colorPixelFormat[3],
            colorPixelFormat[4],
            colorPixelFormat[5],
            colorPixelFormat[6],
            colorPixelFormat[7]
        },
        .depthAttachmentPixelFormat = depthPixelFormat,
        .stencilAttachmentPixelFormat = stencilPixelFormat,
        .sampleCount = mContext->currentRenderTarget->getSamples(),
        .blendState = BlendState {
            .blendingEnabled = rs.hasBlending(),
            .rgbBlendOperation = getMetalBlendOperation(rs.blendEquationRGB),
            .alphaBlendOperation = getMetalBlendOperation(rs.blendEquationAlpha),
            .sourceRGBBlendFactor = getMetalBlendFactor(rs.blendFunctionSrcRGB),
            .sourceAlphaBlendFactor = getMetalBlendFactor(rs.blendFunctionSrcAlpha),
            .destinationRGBBlendFactor = getMetalBlendFactor(rs.blendFunctionDstRGB),
            .destinationAlphaBlendFactor = getMetalBlendFactor(rs.blendFunctionDstAlpha)
        },
        .colorWrite = rs.colorWrite
    };
    mContext->pipelineState.updateState(pipelineState);
    if (mContext->pipelineState.stateChanged()) {
        id<MTLRenderPipelineState> pipeline =
                mContext->pipelineStateCache.getOrCreateState(pipelineState);
        assert_invariant(pipeline != nil);
        [mContext->currentRenderPassEncoder setRenderPipelineState:pipeline];
    }

    // Cull mode
    MTLCullMode cullMode = getMetalCullMode(rs.culling);
    mContext->cullModeState.updateState(cullMode);
    if (mContext->cullModeState.stateChanged()) {
        [mContext->currentRenderPassEncoder setCullMode:cullMode];
    }

    // Front face winding
    MTLWinding winding = rs.inverseFrontFaces ? MTLWindingClockwise : MTLWindingCounterClockwise;
    mContext->windingState.updateState(winding);
    if (mContext->windingState.stateChanged()) {
        [mContext->currentRenderPassEncoder setFrontFacingWinding:winding];
    }

    // Set the depth-stencil state, if a state change is needed.
    DepthStencilState depthState;
    if (depthAttachment) {
        depthState.depthCompare = getMetalCompareFunction(rs.depthFunc);
        depthState.depthWriteEnabled = rs.depthWrite;
    }
    if (stencilAttachment) {
        const auto& ss = ps.stencilState;

        auto& front = depthState.front;
        front.stencilCompare = getMetalCompareFunction(ss.front.stencilFunc);
        front.stencilOperationStencilFail = getMetalStencilOperation(ss.front.stencilOpStencilFail);
        front.stencilOperationDepthFail = getMetalStencilOperation(ss.front.stencilOpDepthFail);
        front.stencilOperationDepthStencilPass =
                getMetalStencilOperation(ss.front.stencilOpDepthStencilPass);
        front.readMask = ss.front.readMask;
        front.writeMask = ss.front.writeMask;

        auto& back = depthState.back;
        back.stencilCompare = getMetalCompareFunction(ss.back.stencilFunc);
        back.stencilOperationStencilFail = getMetalStencilOperation(ss.back.stencilOpStencilFail);
        back.stencilOperationDepthFail = getMetalStencilOperation(ss.back.stencilOpDepthFail);
        back.stencilOperationDepthStencilPass =
                getMetalStencilOperation(ss.back.stencilOpDepthStencilPass);
        back.readMask = ss.back.readMask;
        back.writeMask = ss.back.writeMask;

        depthState.stencilWriteEnabled = ss.stencilWrite;
        [mContext->currentRenderPassEncoder setStencilFrontReferenceValue:ss.front.ref
                                                       backReferenceValue:ss.back.ref];
    }
    mContext->depthStencilState.updateState(depthState);
    if (mContext->depthStencilState.stateChanged()) {
        id<MTLDepthStencilState> state =
                mContext->depthStencilStateCache.getOrCreateState(depthState);
        assert_invariant(state != nil);
        [mContext->currentRenderPassEncoder setDepthStencilState:state];
    }

    if (ps.polygonOffset.constant != mContext->currentPolygonOffset.constant ||
        ps.polygonOffset.slope != mContext->currentPolygonOffset.slope) {
        [mContext->currentRenderPassEncoder setDepthBias:ps.polygonOffset.constant
                                              slopeScale:ps.polygonOffset.slope
                                                   clamp:0.0];
        mContext->currentPolygonOffset = ps.polygonOffset;
    }

    // Set scissor-rectangle.
    // In order to do this, we compute the intersection between:
    //  1. the scissor rectangle
    //  2. the render target attachment dimensions (important, as the scissor can't be set larger)
    // fmax/min are used below to guard against NaN and because the MTLViewport/MTLRegion
    // coordinates are doubles.
    MTLRegion scissor = mContext->currentRenderTarget->getRegionFromClientRect(ps.scissor);
    const float sleft = scissor.origin.x, sright = scissor.origin.x + scissor.size.width;
    const float stop = scissor.origin.y, sbottom = scissor.origin.y + scissor.size.height;

    // Attachment extent
    const auto attachmentSize = mContext->currentRenderTarget->getAttachmentSize();
    const float aleft = 0.0f, atop = 0.0f;
    const float aright = static_cast<float>(attachmentSize.x);
    const float abottom = static_cast<float>(attachmentSize.y);

    const auto left   = std::fmax(sleft, aleft);
    const auto right  = std::fmin(sright, aright);
    const auto top    = std::fmax(stop, atop);
    const auto bottom = std::fmin(sbottom, abottom);

    MTLScissorRect scissorRect = {
        .x      = static_cast<NSUInteger>(left),
        .y      = static_cast<NSUInteger>(top),
        .width  = static_cast<NSUInteger>(right  - left),
        .height = static_cast<NSUInteger>(bottom - top)
    };

    [mContext->currentRenderPassEncoder setScissorRect:scissorRect];

    // Bind uniform buffers.
    MetalBuffer* uniformsToBind[Program::UNIFORM_BINDING_COUNT] = { nil };
    NSUInteger offsets[Program::UNIFORM_BINDING_COUNT] = { 0 };

    enumerateBoundBuffers(BufferObjectBinding::UNIFORM,
            [&uniformsToBind, &offsets](const BufferState& state, MetalBuffer* buffer,
                    uint32_t index) {
        uniformsToBind[index] = buffer;
        offsets[index] = state.offset;
    });
    MetalBuffer::bindBuffers(getPendingCommandBuffer(mContext), mContext->currentRenderPassEncoder,
            UNIFORM_BUFFER_BINDING_START, MetalBuffer::Stage::VERTEX | MetalBuffer::Stage::FRAGMENT,
            uniformsToBind, offsets, Program::UNIFORM_BINDING_COUNT);

    // Bind sampler groups (argument buffers).
    for (size_t s = 0; s < Program::SAMPLER_BINDING_COUNT; s++) {
        MetalSamplerGroup* const samplerGroup = mContext->samplerBindings[s];
        if (!samplerGroup) {
            continue;
        }
        const auto& stageFlags = program->samplerGroupInfo[s].stageFlags;
        if (stageFlags == ShaderStageFlags::NONE) {
            continue;
        }

        auto iter = mContext->finalizedSamplerGroups.find(samplerGroup);
        if (iter == mContext->finalizedSamplerGroups.end()) {
            finalizeSamplerGroup(samplerGroup);
            mContext->finalizedSamplerGroups.insert(samplerGroup);
        }

        assert_invariant(samplerGroup->getArgumentBuffer());

        if (uint8_t(stageFlags) & uint8_t(ShaderStageFlags::VERTEX)) {
            [mContext->currentRenderPassEncoder setVertexBuffer:samplerGroup->getArgumentBuffer()
                                                         offset:samplerGroup->getArgumentBufferOffset()
                                                        atIndex:(SAMPLER_GROUP_BINDING_START + s)];
        }
        if (uint8_t(stageFlags) & uint8_t(ShaderStageFlags::FRAGMENT)) {
            [mContext->currentRenderPassEncoder setFragmentBuffer:samplerGroup->getArgumentBuffer()
                                                           offset:samplerGroup->getArgumentBufferOffset()
                                                          atIndex:(SAMPLER_GROUP_BINDING_START + s)];
        }
    }

    // Bind the user vertex buffers.

    MetalBuffer* buffers[MAX_VERTEX_BUFFER_COUNT] = {};
    size_t vertexBufferOffsets[MAX_VERTEX_BUFFER_COUNT] = {};
    size_t bufferIndex = 0;

    auto vb = primitive->vertexBuffer;
    for (uint32_t attributeIndex = 0; attributeIndex < vb->attributes.size(); attributeIndex++) {
        const auto& attribute = vb->attributes[attributeIndex];
        if (attribute.buffer == Attribute::BUFFER_UNUSED) {
            continue;
        }

        assert_invariant(vb->buffers[attribute.buffer]);
        buffers[bufferIndex] = vb->buffers[attribute.buffer];
        vertexBufferOffsets[bufferIndex] = attribute.offset;
        bufferIndex++;
    }

    const auto bufferCount = bufferIndex;
    MetalBuffer::bindBuffers(getPendingCommandBuffer(mContext), mContext->currentRenderPassEncoder,
            USER_VERTEX_BUFFER_BINDING_START, MetalBuffer::Stage::VERTEX, buffers,
            vertexBufferOffsets, bufferCount);

    // Bind the zero buffer, used for missing vertex attributes.
    static const char bytes[16] = { 0 };
    [mContext->currentRenderPassEncoder setVertexBytes:bytes
                                                length:16
                                               atIndex:ZERO_VERTEX_BUFFER_BINDING];

    MetalIndexBuffer* indexBuffer = primitive->indexBuffer;

    id<MTLCommandBuffer> cmdBuffer = getPendingCommandBuffer(mContext);
    id<MTLBuffer> metalIndexBuffer = indexBuffer->buffer.getGpuBufferForDraw(cmdBuffer);
    [mContext->currentRenderPassEncoder drawIndexedPrimitives:getMetalPrimitiveType(primitive->type)
                                                   indexCount:primitive->count
                                                    indexType:getIndexType(indexBuffer->elementSize)
                                                  indexBuffer:metalIndexBuffer
                                            indexBufferOffset:primitive->offset
                                                instanceCount:instanceCount];
}

void MetalDriver::dispatchCompute(Handle<HwProgram> program, math::uint3 workGroupCount) {
    ASSERT_PRECONDITION(!isInRenderPass(mContext),
            "dispatchCompute must be called outside of a render pass.");

    auto mtlProgram = handle_cast<MetalProgram>(program);

    // If the material debugger is enabled, avoid fatal (or cascading) errors and that can occur
    // during the draw call when the program is invalid. The shader compile error has already been
    // dumped to the console at this point, so it's fine to simply return early.
    if (FILAMENT_ENABLE_MATDBG && UTILS_UNLIKELY(!mtlProgram->isValid)) {
        return;
    }

    assert_invariant(mtlProgram->isValid && mtlProgram->computeFunction);

    id<MTLComputeCommandEncoder> computeEncoder =
            [getPendingCommandBuffer(mContext) computeCommandEncoder];

    NSError* error = nil;
    id<MTLComputePipelineState> computePipelineState =
            [mContext->device newComputePipelineStateWithFunction:mtlProgram->computeFunction
                                                            error:&error];
    if (error) {
        auto description = [error.localizedDescription cStringUsingEncoding:NSUTF8StringEncoding];
        utils::slog.e << description << utils::io::endl;
    }
    assert_invariant(!error);

    // Bind uniform buffers.
    MetalBuffer* uniformsToBind[Program::UNIFORM_BINDING_COUNT] = { nil };
    NSUInteger uniformOffsets[Program::UNIFORM_BINDING_COUNT] = { 0 };
    enumerateBoundBuffers(BufferObjectBinding::UNIFORM,
            [&uniformsToBind, &uniformOffsets](const BufferState& state, MetalBuffer* buffer,
                    uint32_t index) {
        uniformsToBind[index] = buffer;
        uniformOffsets[index] = state.offset;
    });
    MetalBuffer::bindBuffers(getPendingCommandBuffer(mContext), computeEncoder,
            UNIFORM_BUFFER_BINDING_START, MetalBuffer::Stage::COMPUTE, uniformsToBind,
            uniformOffsets, Program::UNIFORM_BINDING_COUNT);

    // Bind SSBOs.
    MetalBuffer* ssbosToBind[MAX_SSBO_COUNT] = { nil };
    NSUInteger ssboOffsets[MAX_SSBO_COUNT] = { 0 };
    enumerateBoundBuffers(BufferObjectBinding::SHADER_STORAGE,
            [&ssbosToBind, &ssboOffsets](const BufferState& state, MetalBuffer* buffer,
                    uint32_t index) {
        ssbosToBind[index] = buffer;
        ssboOffsets[index] = state.offset;
    });
    MetalBuffer::bindBuffers(getPendingCommandBuffer(mContext), computeEncoder, SSBO_BINDING_START,
            MetalBuffer::Stage::COMPUTE, ssbosToBind, ssboOffsets, MAX_SSBO_COUNT);

    [computeEncoder setComputePipelineState:computePipelineState];

    MTLSize threadgroupsPerGrid = MTLSizeMake(workGroupCount.x, workGroupCount.y, workGroupCount.z);
    // FIXME: the threadgroup size should be specified in the Program
    MTLSize threadsPerThreadgroup = MTLSizeMake(16u, 1u, 1u);
    [computeEncoder dispatchThreadgroups:threadgroupsPerGrid
                   threadsPerThreadgroup:threadsPerThreadgroup];

    [computeEncoder endEncoding];
}

void MetalDriver::beginTimerQuery(Handle<HwTimerQuery> tqh) {
    ASSERT_PRECONDITION(!isInRenderPass(mContext),
            "beginTimerQuery must be called outside of a render pass.");
    auto* tq = handle_cast<MetalTimerQuery>(tqh);
    mContext->timerQueryImpl->beginTimeElapsedQuery(tq);
}

void MetalDriver::endTimerQuery(Handle<HwTimerQuery> tqh) {
    ASSERT_PRECONDITION(!isInRenderPass(mContext),
            "endTimerQuery must be called outside of a render pass.");
    auto* tq = handle_cast<MetalTimerQuery>(tqh);
    mContext->timerQueryImpl->endTimeElapsedQuery(tq);
}

void MetalDriver::enumerateBoundBuffers(BufferObjectBinding bindingType,
        const std::function<void(const BufferState&, MetalBuffer*, uint32_t)>& f) {
    assert_invariant(bindingType == BufferObjectBinding::UNIFORM ||
            bindingType == BufferObjectBinding::SHADER_STORAGE);

    auto enumerate = [&](auto arrayType){
        for (auto i = 0u; i < arrayType.size(); i++) {
            const auto& thisBuffer = arrayType[i];
            if (!thisBuffer.bound) {
                continue;
            }
            f(thisBuffer, thisBuffer.buffer->getBuffer(), i);
        }
    };

    switch (bindingType) {
        default:
        case (BufferObjectBinding::UNIFORM):
            enumerate(mContext->uniformState);
            break;
        case (BufferObjectBinding::SHADER_STORAGE):
            enumerate(mContext->ssboState);
            break;
    }
}

void MetalDriver::resetState(int) {
}

// explicit instantiation of the Dispatcher
template class ConcreteDispatcher<MetalDriver>;

} // namespace backend
} // namespace filament
