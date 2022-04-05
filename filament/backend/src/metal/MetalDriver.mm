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

#include "private/backend/MetalPlatform.h"

#include <CoreVideo/CVMetalTexture.h>
#include <CoreVideo/CVPixelBuffer.h>
#include <Metal/Metal.h>
#include <QuartzCore/QuartzCore.h>

#include <utils/Log.h>
#include <utils/Panic.h>

namespace filament {
namespace backend {

Driver* MetalDriverFactory::create(MetalPlatform* const platform) {
    return MetalDriver::create(platform);
}

UTILS_NOINLINE
Driver* MetalDriver::create(MetalPlatform* const platform) {
    assert_invariant(platform);
    return new MetalDriver(platform);
}

Dispatcher MetalDriver::getDispatcher() const noexcept {
    return ConcreteDispatcher<MetalDriver>::make();
}

MetalDriver::MetalDriver(MetalPlatform* platform) noexcept
        : mPlatform(*platform),
          mContext(new MetalContext),
          mHandleAllocator("Handles", FILAMENT_METAL_HANDLE_ARENA_SIZE_IN_MB * 1024U * 1024U) {
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

    mContext->commandQueue = mPlatform.createCommandQueue(mContext->device);
    mContext->pipelineStateCache.setDevice(mContext->device);
    mContext->depthStencilStateCache.setDevice(mContext->device);
    mContext->samplerStateCache.setDevice(mContext->device);
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
    construct_handle<MetalBufferObject>(boh, *mContext, usage, byteCount);
}

void MetalDriver::createTextureR(Handle<HwTexture> th, SamplerType target, uint8_t levels,
        TextureFormat format, uint8_t samples, uint32_t width, uint32_t height,
        uint32_t depth, TextureUsage usage) {
    // Clamp sample count to what the device supports.
    auto& sc = mContext->sampleCountLookup;
    samples = sc[std::min(MAX_SAMPLE_COUNT, samples)];

    construct_handle<MetalTexture>(th, *mContext, target, levels, format, samples,
            width, height, depth, usage, TextureSwizzle::CHANNEL_0, TextureSwizzle::CHANNEL_1,
            TextureSwizzle::CHANNEL_2, TextureSwizzle::CHANNEL_3);
}

void MetalDriver::createTextureSwizzledR(Handle<HwTexture> th, SamplerType target, uint8_t levels,
        TextureFormat format, uint8_t samples, uint32_t width, uint32_t height,
        uint32_t depth, TextureUsage usage,
        TextureSwizzle r, TextureSwizzle g, TextureSwizzle b, TextureSwizzle a) {
    // Clamp sample count to what the device supports.
    auto& sc = mContext->sampleCountLookup;
    samples = sc[std::min(MAX_SAMPLE_COUNT, samples)];

    construct_handle<MetalTexture>(th, *mContext, target, levels, format, samples,
            width, height, depth, usage, r, g, b, a);
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
    MTLPixelFormat filamentMetalFormat = getMetalFormat(mContext, format);
    ASSERT_PRECONDITION(metalTexture.pixelFormat == filamentMetalFormat,
            "Imported id<MTLTexture> format (%d) != Filament texture format (%d)",
            metalTexture.pixelFormat, filamentMetalFormat);
    MTLTextureType filamentMetalType = getMetalType(target);
    ASSERT_PRECONDITION(metalTexture.textureType == filamentMetalType,
            "Imported id<MTLTexture> type (%d) != Filament texture type (%d)",
            metalTexture.textureType, filamentMetalType);
    construct_handle<MetalTexture>(th, *mContext, target, levels, format, samples,
        width, height, depth, usage, metalTexture);
}

void MetalDriver::createSamplerGroupR(Handle<HwSamplerGroup> sbh, uint32_t size) {
    mContext->samplerGroups.insert(construct_handle<MetalSamplerGroup>(sbh, size));
}

void MetalDriver::createRenderPrimitiveR(Handle<HwRenderPrimitive> rph, int dummy) {
    construct_handle<MetalRenderPrimitive>(rph);
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
    // Clamp sample count to what the device supports.
    auto& sc = mContext->sampleCountLookup;
    samples = sc[std::min(MAX_SAMPLE_COUNT, samples)];

    MetalRenderTarget::Attachment colorAttachments[MRT::MAX_SUPPORTED_RENDER_TARGET_COUNT] = {{}};
    for (size_t i = 0; i < MRT::MAX_SUPPORTED_RENDER_TARGET_COUNT; i++) {
        const auto& buffer = color[i];
        if (!buffer.handle) {
            ASSERT_POSTCONDITION(none(targetBufferFlags & getTargetBufferFlagsAt(i)),
                    "The COLOR%u flag was specified, but no color texture provided.", i);
            continue;
        }

        auto colorTexture = handle_cast<MetalTexture>(buffer.handle);
        ASSERT_PRECONDITION(colorTexture->texture,
                "Color texture passed to render target has no texture allocation");
        colorTexture->updateLodRange(buffer.level);
        colorAttachments[i] = { colorTexture, color[i].level, color[i].layer };
    }

    MetalRenderTarget::Attachment depthAttachment = {};
    if (depth.handle) {
        auto depthTexture = handle_cast<MetalTexture>(depth.handle);
        ASSERT_PRECONDITION(depthTexture->texture,
                "Depth texture passed to render target has no texture allocation.");
        depthTexture->updateLodRange(depth.level);
        depthAttachment = { depthTexture, depth.level, depth.layer };
    }
    ASSERT_POSTCONDITION(!depth.handle || any(targetBufferFlags & TargetBufferFlags::DEPTH),
            "The DEPTH flag was specified, but no depth texture provided.");

    construct_handle<MetalRenderTarget>(rth, mContext, width, height, samples,
            colorAttachments, depthAttachment);

    ASSERT_POSTCONDITION(
            !stencil.handle &&
            !(targetBufferFlags & TargetBufferFlags::STENCIL),
            "Stencil buffer not supported.");
}

void MetalDriver::createFenceR(Handle<HwFence> fh, int dummy) {
    auto* fence = handle_cast<MetalFence>(fh);
    fence->encode();
}

void MetalDriver::createSyncR(Handle<HwSync> sh, int) {
    auto* fence = handle_cast<MetalFence>(sh);
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

Handle<HwSync> MetalDriver::createSyncS() noexcept {
    // The handle must be constructed here, as a synchronous call to getSyncStatus might happen
    // before createSyncR is executed.
    return alloc_and_construct_handle<MetalFence, HwSync>(*mContext);
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
    // Unbind this buffer object from any uniform slots it's still bound to.
    bo->boundUniformBuffers.forEachSetBit([this](size_t index) {
        mContext->uniformState[index].buffer = nullptr;
        mContext->uniformState[index].bound = false;
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

    // Unbind this texture from any sampler groups that currently reference it.
    for (auto* metalSamplerGroup : mContext->samplerGroups) {
        const SamplerGroup::Sampler* samplers = metalSamplerGroup->sb->getSamplers();
        for (size_t i = 0; i < metalSamplerGroup->sb->getSize(); i++) {
            const SamplerGroup::Sampler* sampler = samplers + i;
            if (sampler->t == th) {
                metalSamplerGroup->sb->setSampler(i, {{}, {}});
            }
        }
    }

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

void MetalDriver::destroySync(Handle<HwSync> sh) {
    if (sh) {
        destruct_handle<MetalFence>(sh);
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
    return ShaderModel::GL_ES_30;
#else
    return ShaderModel::GL_CORE_41;
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

FenceStatus MetalDriver::wait(Handle<HwFence> fh, uint64_t timeout) {
    auto* fence = handle_cast<MetalFence>(fh);
    if (!fence) {
        return FenceStatus::ERROR;
    }
    return fence->wait(timeout);
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
    return true;
}

bool MetalDriver::isWorkaroundNeeded(Workaround workaround) {
    switch (workaround) {
        case Workaround::SPLIT_EASU:
            return false;
        case Workaround::ALLOW_READ_ONLY_ANCILLARY_FEEDBACK_LOOP:
            return true;
    }
    return false;
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

void MetalDriver::updateIndexBuffer(Handle<HwIndexBuffer> ibh, BufferDescriptor&& data,
        uint32_t byteOffset) {
    auto* ib = handle_cast<MetalIndexBuffer>(ibh);
    ib->buffer.copyIntoBuffer(data.buffer, data.size, byteOffset);
    scheduleDestroy(std::move(data));
}

void MetalDriver::updateBufferObject(Handle<HwBufferObject> boh, BufferDescriptor&& data,
        uint32_t byteOffset) {
    auto* bo = handle_cast<MetalBufferObject>(boh);
    bo->updateBuffer(data.buffer, data.size, byteOffset);
    scheduleDestroy(std::move(data));
}

void MetalDriver::setVertexBufferObject(Handle<HwVertexBuffer> vbh, uint32_t index,
        Handle<HwBufferObject> boh) {
    auto* vertexBuffer = handle_cast<MetalVertexBuffer>(vbh);
    auto* bufferObject = handle_cast<MetalBufferObject>(boh);
    assert_invariant(index < vertexBuffer->buffers.size());
    vertexBuffer->buffers[index] = bufferObject->getBuffer();
}

void MetalDriver::update2DImage(Handle<HwTexture> th, uint32_t level, uint32_t xoffset,
        uint32_t yoffset, uint32_t width, uint32_t height, PixelBufferDescriptor&& data) {
    ASSERT_PRECONDITION(!isInRenderPass(mContext),
            "update2DImage must be called outside of a render pass.");
    auto tex = handle_cast<MetalTexture>(th);
    tex->loadImage(level, MTLRegionMake2D(xoffset, yoffset, width, height), data);
    scheduleDestroy(std::move(data));
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

void MetalDriver::updateCubeImage(Handle<HwTexture> th, uint32_t level,
        PixelBufferDescriptor&& data, FaceOffsets faceOffsets) {
    ASSERT_PRECONDITION(!isInRenderPass(mContext),
            "updateCubeImage must be called outside of a render pass.");
    auto tex = handle_cast<MetalTexture>(th);
    tex->loadCubeImage(faceOffsets, level, data);
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

void MetalDriver::cancelExternalImage(void* image) {
    CVPixelBufferRef pixelBuffer = (CVPixelBufferRef) image;
    CVPixelBufferRelease(pixelBuffer);
}

void MetalDriver::setExternalImage(Handle<HwTexture> th, void* image) {
    auto texture = handle_cast<MetalTexture>(th);
    texture->externalImage.set((CVPixelBufferRef) image);
}

void MetalDriver::setExternalImagePlane(Handle<HwTexture> th, void* image, uint32_t plane) {
    auto texture = handle_cast<MetalTexture>(th);
    texture->externalImage.set((CVPixelBufferRef) image, plane);
}

void MetalDriver::setExternalStream(Handle<HwTexture> th, Handle<HwStream> sh) {
}

bool MetalDriver::getTimerQueryValue(Handle<HwTimerQuery> tqh, uint64_t* elapsedTime) {
    auto* tq = handle_cast<MetalTimerQuery>(tqh);
    return mContext->timerQueryImpl->getQueryResult(tq, elapsedTime);
}

SyncStatus MetalDriver::getSyncStatus(Handle<HwSync> sh) {
    auto* fence = handle_cast<MetalFence>(sh);
    FenceStatus status = fence->wait(0);
    if (status == FenceStatus::TIMEOUT_EXPIRED) {
        return SyncStatus::NOT_SIGNALED;
    } else if (status == FenceStatus::CONDITION_SATISFIED) {
        return SyncStatus::SIGNALED;
    }
    return SyncStatus::ERROR;
}

void MetalDriver::generateMipmaps(Handle<HwTexture> th) {
    ASSERT_PRECONDITION(!isInRenderPass(mContext),
                        "generateMipmaps must be called outside of a render pass.");
    auto tex = handle_cast<MetalTexture>(th);
    id <MTLBlitCommandEncoder> blitEncoder = [getPendingCommandBuffer(mContext) blitCommandEncoder];
    [blitEncoder generateMipmapsForTexture:tex->texture];
    [blitEncoder endEncoding];
    tex->minLod = 0;
    tex->maxLod = tex->texture.mipmapLevelCount - 1;
}

bool MetalDriver::canGenerateMipmaps() {
    return true;
}

void MetalDriver::updateSamplerGroup(Handle<HwSamplerGroup> sbh,
        SamplerGroup&& samplerGroup) {
    auto sb = handle_cast<MetalSamplerGroup>(sbh);
    *sb->sb = samplerGroup;
}

void MetalDriver::beginRenderPass(Handle<HwRenderTarget> rth,
        const RenderPassParams& params) {
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

    // Flip the viewport, because Metal's screen space is vertically flipped that of Filament's.
    NSInteger renderTargetHeight =
            mContext->currentRenderTarget->isDefaultRenderTarget() ?
            mContext->currentReadSwapChain->getSurfaceHeight() : mContext->currentRenderTarget->height;
    MTLViewport metalViewport {
            .originX = static_cast<double>(params.viewport.left),
            .originY = renderTargetHeight - static_cast<double>(params.viewport.bottom) -
                       static_cast<double>(params.viewport.height),
            .width = static_cast<double>(params.viewport.width),
            .height = static_cast<double>(params.viewport.height),
            .znear = static_cast<double>(params.depthRange.near),
            .zfar = static_cast<double>(params.depthRange.far)
    };
    [mContext->currentRenderPassEncoder setViewport:metalViewport];

    // Metal requires a new command encoder for each render pass, and they cannot be reused.
    // We must bind certain states for each command encoder, so we dirty the states here to force a
    // rebinding at the first the draw call of this pass.
    mContext->pipelineState.invalidate();
    mContext->depthStencilState.invalidate();
    mContext->cullModeState.invalidate();
    mContext->windingState.invalidate();
}

void MetalDriver::nextSubpass(int dummy) {}

void MetalDriver::endRenderPass(int dummy) {
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
    auto* bo = handle_cast<MetalBufferObject>(boh);
    auto* currentBo = mContext->uniformState[index].buffer;
    if (currentBo) {
        currentBo->boundUniformBuffers.unset(index);
    }
    bo->boundUniformBuffers.set(index);
    mContext->uniformState[index] = UniformBufferState{
            .buffer = bo,
            .offset = 0,
            .bound = true
    };
}

void MetalDriver::bindUniformBufferRange(uint32_t index, Handle<HwBufferObject> boh,
        uint32_t offset, uint32_t size) {
    auto* bo = handle_cast<MetalBufferObject>(boh);
    auto* currentBo = mContext->uniformState[index].buffer;
    if (currentBo) {
        currentBo->boundUniformBuffers.unset(index);
    }
    bo->boundUniformBuffers.set(index);
    mContext->uniformState[index] = UniformBufferState{
            .buffer = bo,
            .offset = offset,
            .bound = true
    };
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
    if (@available(iOS 13, *)) {
        MTLCaptureDescriptor* descriptor = [MTLCaptureDescriptor new];
        descriptor.captureObject = mContext->device;
        descriptor.destination = MTLCaptureDestinationGPUTraceDocument;
        descriptor.outputURL = [[NSURL alloc] initFileURLWithPath:@"filament.gputrace"];
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

    mContext->blitter->blit(getPendingCommandBuffer(mContext), args);

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
    }];
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

    // Metal's texture coordinates have (0, 0) at the top-left of the texture, but Filament's
    // coordinates have (0, 0) at bottom-left.
    const NSInteger srcHeight =
            srcTarget->isDefaultRenderTarget() ?
            mContext->currentReadSwapChain->getSurfaceHeight() : srcTarget->height;
    MTLRegion srcRegion = MTLRegionMake2D(
            (NSUInteger) srcRect.left,
            srcHeight - (NSUInteger) srcRect.bottom - srcRect.height,
            srcRect.width, srcRect.height);

    const NSInteger dstHeight =
            dstTarget->isDefaultRenderTarget() ?
            mContext->currentDrawSwapChain->getSurfaceHeight() : dstTarget->height;
    MTLRegion dstRegion = MTLRegionMake2D(
            (NSUInteger) dstRect.left,
            dstHeight - (NSUInteger) dstRect.bottom - dstRect.height,
            dstRect.width, dstRect.height);

    auto isBlitableTextureType = [](MTLTextureType t) {
        return t == MTLTextureType2D || t == MTLTextureType2DMultisample ||
               t == MTLTextureType2DArray;
    };

    MetalBlitter::BlitArgs args;
    args.filter = filter;
    args.source.region = srcRegion;
    args.destination.region = dstRegion;

    if (any(buffers & TargetBufferFlags::COLOR_ALL)) {
        // We always blit from/to the COLOR0 attachment.
        MetalRenderTarget::Attachment srcColorAttachment = srcTarget->getReadColorAttachment(0);
        MetalRenderTarget::Attachment dstColorAttachment = dstTarget->getDrawColorAttachment(0);

        if (srcColorAttachment && dstColorAttachment) {
            ASSERT_PRECONDITION(isBlitableTextureType(srcColorAttachment.getTexture().textureType) &&
                                isBlitableTextureType(dstColorAttachment.getTexture().textureType),
                               "Metal does not support blitting to/from non-2D textures.");

            args.source.color = srcColorAttachment.getTexture();
            args.destination.color = dstColorAttachment.getTexture();
            args.source.level = srcColorAttachment.level;
            args.destination.level = dstColorAttachment.level;
            args.source.slice = srcColorAttachment.layer;
            args.destination.slice = dstColorAttachment.layer;
        }
    }

    if (any(buffers & TargetBufferFlags::DEPTH)) {
        MetalRenderTarget::Attachment srcDepthAttachment = srcTarget->getDepthAttachment();
        MetalRenderTarget::Attachment dstDepthAttachment = dstTarget->getDepthAttachment();

        if (srcDepthAttachment && dstDepthAttachment) {
            ASSERT_PRECONDITION(isBlitableTextureType(srcDepthAttachment.getTexture().textureType) &&
                                isBlitableTextureType(dstDepthAttachment.getTexture().textureType),
                               "Metal does not support blitting to/from non-2D textures.");

            args.source.depth = srcDepthAttachment.getTexture();
            args.destination.depth = dstDepthAttachment.getTexture();

            if (args.blitColor()) {
                // If blitting color, we've already set the source and destination levels and slices.
                // Check that they match the requested depth levels/slices.
                ASSERT_PRECONDITION(args.source.level == srcDepthAttachment.level,
                                   "Color and depth source LOD must match. (%d != %d)",
                                   args.source.level, srcDepthAttachment.level);
                ASSERT_PRECONDITION(args.destination.level == dstDepthAttachment.level,
                                   "Color and depth destination LOD must match. (%d != %d)",
                                   args.destination.level, dstDepthAttachment.level);
                ASSERT_PRECONDITION(args.source.slice == srcDepthAttachment.layer,
                        "Color and depth source layer must match. (%d != %d)",
                        args.source.slice, srcDepthAttachment.layer);
                ASSERT_PRECONDITION(args.destination.slice == dstDepthAttachment.layer,
                        "Color and depth destination layer must match. (%d != %d)",
                        args.destination.slice, dstDepthAttachment.layer);
            }

            args.source.level = srcDepthAttachment.level;
            args.destination.level = dstDepthAttachment.level;
            args.source.slice = srcDepthAttachment.layer;
            args.destination.slice = dstDepthAttachment.layer;
        }
    }

    mContext->blitter->blit(getPendingCommandBuffer(mContext), args);
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
        depthState.compareFunction = getMetalCompareFunction(rs.depthFunc);
        depthState.depthWriteEnabled = rs.depthWrite;
    }
    mContext->depthStencilState.updateState(depthState);
    if (mContext->depthStencilState.stateChanged()) {
        id<MTLDepthStencilState> state =
                mContext->depthStencilStateCache.getOrCreateState(depthState);
        assert_invariant(state != nil);
        [mContext->currentRenderPassEncoder setDepthStencilState:state];
    }

    if (ps.polygonOffset.constant != 0.0 || ps.polygonOffset.slope != 0.0) {
        [mContext->currentRenderPassEncoder setDepthBias:ps.polygonOffset.constant
                                              slopeScale:ps.polygonOffset.slope
                                                   clamp:0.0];
    }

    // FIXME: implement take ps.scissor into account
    //  must be intersected with viewport (see OpenGLDriver.cpp for implementation details)

    // Bind uniform buffers.
    MetalBuffer* uniformsToBind[Program::BINDING_COUNT] = { nil };
    NSUInteger offsets[Program::BINDING_COUNT] = { 0 };

    enumerateBoundUniformBuffers([&uniformsToBind, &offsets](const UniformBufferState& state,
            MetalBuffer* buffer, uint32_t index) {
        uniformsToBind[index] = buffer;
        offsets[index] = state.offset;
    });
    MetalBuffer::bindBuffers(getPendingCommandBuffer(mContext), mContext->currentRenderPassEncoder,
            0, MetalBuffer::Stage::VERTEX | MetalBuffer::Stage::FRAGMENT, uniformsToBind, offsets,
            Program::BINDING_COUNT);

    // Enumerate all the sampler buffers for the program and check which textures and samplers need
    // to be bound.

    auto getTextureToBind = [this](const SamplerGroup::Sampler* sampler) {
        const auto metalTexture = handle_const_cast<MetalTexture>(sampler->t);
        id<MTLTexture> textureToBind = metalTexture->swizzledTextureView ? metalTexture->swizzledTextureView
                                                                         : metalTexture->texture;
        if (metalTexture->externalImage.isValid()) {
            textureToBind = metalTexture->externalImage.getMetalTextureForDraw();
        }
        return textureToBind;
    };

    auto getSamplerToBind = [this](const SamplerGroup::Sampler* sampler) {
        const auto metalTexture = handle_const_cast<MetalTexture>(sampler->t);
        SamplerState s {
            .samplerParams = sampler->s,
            .minLod = metalTexture->minLod,
            .maxLod = metalTexture->maxLod
        };
        return mContext->samplerStateCache.getOrCreateState(s);
    };

    id<MTLTexture> texturesToBindVertex[MAX_VERTEX_SAMPLER_COUNT] = {};
    id<MTLSamplerState> samplersToBindVertex[MAX_VERTEX_SAMPLER_COUNT] = {};

    enumerateSamplerGroups(program, ShaderType::VERTEX,
            [this, &getTextureToBind, &getSamplerToBind, &texturesToBindVertex, &samplersToBindVertex](
                    const SamplerGroup::Sampler* sampler, uint8_t binding) {
        // We currently only support a max of MAX_VERTEX_SAMPLER_COUNT samplers. Ignore any additional
        // samplers that may be bound.
        if (binding >= MAX_VERTEX_SAMPLER_COUNT) {
            return;
        }

        auto& textureToBind = texturesToBindVertex[binding];
        textureToBind = getTextureToBind(sampler);
        if (!textureToBind) {
            utils::slog.w << "Warning: no texture bound at binding point " << (size_t) binding
                    << " at the vertex shader." << utils::io::endl;
            textureToBind = getOrCreateEmptyTexture(mContext);
        }

        auto& samplerToBind = samplersToBindVertex[binding];
        samplerToBind = getSamplerToBind(sampler);
    });

    // Assign a default sampler to empty slots, in case Filament hasn't bound all samplers.
    // Metal requires all samplers referenced in shaders to be bound.
    for (auto& sampler : samplersToBindVertex) {
        if (!sampler) {
            sampler = mContext->samplerStateCache.getOrCreateState({});
        }
    }

    NSRange vertexSamplerRange = NSMakeRange(0, MAX_VERTEX_SAMPLER_COUNT);
    [mContext->currentRenderPassEncoder setVertexTextures:texturesToBindVertex
                                                withRange:vertexSamplerRange];
    [mContext->currentRenderPassEncoder setVertexSamplerStates:samplersToBindVertex
                                                     withRange:vertexSamplerRange];

    id<MTLTexture> texturesToBindFragment[MAX_FRAGMENT_SAMPLER_COUNT] = {};
    id<MTLSamplerState> samplersToBindFragment[MAX_FRAGMENT_SAMPLER_COUNT] = {};

    enumerateSamplerGroups(program, ShaderType::FRAGMENT,
            [this, &getTextureToBind, &getSamplerToBind, &texturesToBindFragment, &samplersToBindFragment](
                    const SamplerGroup::Sampler* sampler, uint8_t binding) {
        // We currently only support a max of MAX_FRAGMENT_SAMPLER_COUNT samplers. Ignore any additional
        // samplers that may be bound.
        if (binding >= MAX_FRAGMENT_SAMPLER_COUNT) {
            return;
        }

        auto& textureToBind = texturesToBindFragment[binding];
        textureToBind = getTextureToBind(sampler);
        if (!textureToBind) {
            utils::slog.w << "Warning: no texture bound at binding point " << (size_t) binding
                          << " at the fragment shader." << utils::io::endl;
            textureToBind = getOrCreateEmptyTexture(mContext);
        }

        auto& samplerToBind = samplersToBindFragment[binding];
        samplerToBind = getSamplerToBind(sampler);
    });

    // Assign a default sampler to empty slots, in case Filament hasn't bound all samplers.
    // Metal requires all samplers referenced in shaders to be bound.
    for (auto& sampler : samplersToBindFragment) {
        if (!sampler) {
            sampler = mContext->samplerStateCache.getOrCreateState({});
        }
    }

    NSRange fragmentSamplerRange = NSMakeRange(0, MAX_FRAGMENT_SAMPLER_COUNT);
    [mContext->currentRenderPassEncoder setFragmentTextures:texturesToBindFragment
                                                  withRange:fragmentSamplerRange];
    [mContext->currentRenderPassEncoder setFragmentSamplerStates:samplersToBindFragment
                                                       withRange:fragmentSamplerRange];

    // Bind the vertex buffers.

    MetalBuffer* buffers[MAX_VERTEX_BUFFER_COUNT];
    size_t vertexBufferOffsets[MAX_VERTEX_BUFFER_COUNT];
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
            VERTEX_BUFFER_START, MetalBuffer::Stage::VERTEX, buffers,
            vertexBufferOffsets, bufferCount);

    // Bind the zero buffer, used for missing vertex attributes.
    static const char bytes[16] = { 0 };
    [mContext->currentRenderPassEncoder setVertexBytes:bytes
                                                length:16
                                               atIndex:(VERTEX_BUFFER_START + ZERO_VERTEX_BUFFER)];

    MetalIndexBuffer* indexBuffer = primitive->indexBuffer;

    id<MTLCommandBuffer> cmdBuffer = getPendingCommandBuffer(mContext);
    id<MTLBuffer> metalIndexBuffer = indexBuffer->buffer.getGpuBufferForDraw(cmdBuffer);
    size_t offset = indexBuffer->buffer.getGpuBufferStreamOffset();
    [mContext->currentRenderPassEncoder drawIndexedPrimitives:getMetalPrimitiveType(primitive->type)
                                                   indexCount:primitive->count
                                                    indexType:getIndexType(indexBuffer->elementSize)
                                                  indexBuffer:metalIndexBuffer
                                            indexBufferOffset:primitive->offset + offset
                                                instanceCount:instanceCount];
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

void MetalDriver::enumerateSamplerGroups(
        const MetalProgram* program, ShaderType shaderType,
        const std::function<void(const SamplerGroup::Sampler*, size_t)>& f) {
    auto& samplerBlockInfo = (shaderType == ShaderType::VERTEX) ?
            program->vertexSamplerBlockInfo : program->fragmentSamplerBlockInfo;
    auto maxSamplerCount = (shaderType == ShaderType::VERTEX) ?
            MAX_VERTEX_SAMPLER_COUNT : MAX_FRAGMENT_SAMPLER_COUNT;
    for (size_t bindingIdx = 0; bindingIdx != maxSamplerCount; ++bindingIdx) {
        auto& blockInfo = samplerBlockInfo[bindingIdx];
        if (blockInfo.samplerGroup == UINT8_MAX) {
            continue;
        }

        const auto* metalSamplerGroup = mContext->samplerBindings[blockInfo.samplerGroup];
        if (!metalSamplerGroup) {
            // Do not emit warning here. For example this can arise when skinning is enabled
            // and the morphing texture is unused.
            continue;
        }

        SamplerGroup* sb = metalSamplerGroup->sb.get();
        const SamplerGroup::Sampler* boundSampler = sb->getSamplers() + blockInfo.sampler;

        if (!boundSampler->t) {
            continue;
        }

        f(boundSampler, bindingIdx);
    }
}

void MetalDriver::enumerateBoundUniformBuffers(
        const std::function<void(const UniformBufferState&, MetalBuffer*, uint32_t)>& f) {
    for (uint32_t i = 0; i < Program::BINDING_COUNT; i++) {
        auto& thisUniform = mContext->uniformState[i];
        if (!thisUniform.bound) {
            continue;
        }
        f(thisUniform, thisUniform.buffer->getBuffer(), i);
    }
}

// explicit instantiation of the Dispatcher
template class ConcreteDispatcher<MetalDriver>;

} // namespace backend
} // namespace filament
