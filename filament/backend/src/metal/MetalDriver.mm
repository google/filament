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
#include "MetalBufferPool.h"
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
#include <utils/sstream.h>

#include <algorithm>

#ifndef DEBUG_LOG_DESCRIPTOR_SETS
#define DEBUG_LOG_DESCRIPTOR_SETS 0
#endif

#if DEBUG_LOG_DESCRIPTOR_SETS == 1
#define DEBUG_LOG(...) printf(__VA_ARGS__)
#else
#define DEBUG_LOG(...)
#endif

namespace filament {
namespace backend {

Driver* MetalDriverFactory::create(MetalPlatform* const platform, const Platform::DriverConfig& driverConfig) {
#if 0
    // this is useful for development, but too verbose even for debug builds
    // For reference on a 64-bits machine in Release mode:
    //    MetalTimerQuery              :  16       few
    //    HwStream                     :  24       few
    //    MetalRenderPrimitive         :  24       many
    //    MetalVertexBuffer            :  32       moderate
    // -- less than or equal 32 bytes
    //    MetalIndexBuffer             :  40       moderate
    //    MetalFence                   :  48       few
    //    MetalBufferObject            :  48       many
    // -- less than or equal 48 bytes
    //    MetalSamplerGroup            : 112       few
    //    MetalProgram                 : 152       moderate
    //    MetalTexture                 : 152       moderate
    //    MetalSwapChain               : 184       few
    //    MetalRenderTarget            : 272       few
    //    MetalVertexBufferInfo        : 552       moderate
    // -- less than or equal to 552 bytes

    utils::slog.d
           << "\nMetalSwapChain: " << sizeof(MetalSwapChain)
           << "\nMetalBufferObject: " << sizeof(MetalBufferObject)
           << "\nMetalVertexBuffer: " << sizeof(MetalVertexBuffer)
           << "\nMetalVertexBufferInfo: " << sizeof(MetalVertexBufferInfo)
           << "\nMetalIndexBuffer: " << sizeof(MetalIndexBuffer)
           << "\nMetalSamplerGroup: " << sizeof(MetalSamplerGroup)
           << "\nMetalRenderPrimitive: " << sizeof(MetalRenderPrimitive)
           << "\nMetalTexture: " << sizeof(MetalTexture)
           << "\nMetalTimerQuery: " << sizeof(MetalTimerQuery)
           << "\nHwStream: " << sizeof(HwStream)
           << "\nMetalRenderTarget: " << sizeof(MetalRenderTarget)
           << "\nMetalFence: " << sizeof(MetalFence)
           << "\nMetalProgram: " << sizeof(MetalProgram)
           << utils::io::endl;
#endif
    return MetalDriver::create(platform, driverConfig);
}

UTILS_NOINLINE
Driver* MetalDriver::create(MetalPlatform* const platform, const Platform::DriverConfig& driverConfig) {
    assert_invariant(platform);
    size_t defaultSize = FILAMENT_METAL_HANDLE_ARENA_SIZE_IN_MB * 1024U * 1024U;
    Platform::DriverConfig validConfig {driverConfig};
    validConfig.handleArenaSize = std::max(driverConfig.handleArenaSize, defaultSize);
    return new MetalDriver(platform, validConfig);
}

Dispatcher MetalDriver::getDispatcher() const noexcept {
    return ConcreteDispatcher<MetalDriver>::make();
}

MetalDriver::MetalDriver(MetalPlatform* platform, const Platform::DriverConfig& driverConfig) noexcept
        : mPlatform(*platform),
          mContext(new MetalContext(driverConfig.textureUseAfterFreePoolSize)),
          mHandleAllocator("Handles",
                  driverConfig.handleArenaSize,
                  driverConfig.disableHandleUseAfterFreeCheck),
          mStereoscopicType(driverConfig.stereoscopicType) {
    mContext->driver = this;

    TrackedMetalBuffer::setPlatform(platform);
    ScopedAllocationTimer::setPlatform(platform);

    mContext->device = mPlatform.createDevice();
    assert_invariant(mContext->device);

    mContext->emptyBuffer = [mContext->device newBufferWithLength:4 * 1024
                                                          options:MTLResourceStorageModePrivate];

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
    mContext->bumpAllocator =
            new MetalBumpAllocator(mContext->device, driverConfig.metalUploadBufferSizeBytes);
    mContext->blitter = new MetalBlitter(*mContext);

    if (@available(iOS 12, *)) {
        mContext->timerQueryImpl = new MetalTimerQueryFence(*mContext);
    } else {
        mContext->timerQueryImpl = new TimerQueryNoop();
    }

    CVReturn success = CVMetalTextureCacheCreate(kCFAllocatorDefault, nullptr, mContext->device,
            nullptr, &mContext->textureCache);
    FILAMENT_CHECK_POSTCONDITION(success == kCVReturnSuccess)
            << "Could not create Metal texture cache.";

    if (@available(iOS 12, *)) {
        dispatch_queue_t queue = dispatch_get_global_queue(QOS_CLASS_DEFAULT, 0);
        mContext->eventListener = [[MTLSharedEventListener alloc] initWithDispatchQueue:queue];
    }

    const MetalShaderCompiler::Mode compilerMode = driverConfig.disableParallelShaderCompile
            ? MetalShaderCompiler::Mode::SYNCHRONOUS
            : MetalShaderCompiler::Mode::ASYNCHRONOUS;
    mContext->shaderCompiler = new MetalShaderCompiler(mContext->device, *this, compilerMode);
    mContext->shaderCompiler->init();

#if defined(FILAMENT_METAL_PROFILING)
    mContext->log = os_log_create("com.google.filament", "Metal");
    mContext->signpostId = os_signpost_id_generate(mContext->log);
    assert_invariant(mContext->signpostId != OS_SIGNPOST_ID_NULL);
#endif
}

MetalDriver::~MetalDriver() noexcept {
    TrackedMetalBuffer::setPlatform(nullptr);
    ScopedAllocationTimer::setPlatform(nullptr);
    mContext->device = nil;
    mContext->emptyTexture = nil;
    CFRelease(mContext->textureCache);
    delete mContext->bufferPool;
    delete mContext->bumpAllocator;
    delete mContext->blitter;
    delete mContext->timerQueryImpl;
    delete mContext->shaderCompiler;
    delete mContext;
}

void MetalDriver::tick(int) {
    executeTickOps();
}

void MetalDriver::beginFrame(int64_t monotonic_clock_ns,
        int64_t refreshIntervalNs, uint32_t frameId) {
    DEBUG_LOG(
            "[DS] beginFrame(monotonic_clock_ns = %lld, refreshIntervalNs = %lld, frameId = %d)\n",
            monotonic_clock_ns, refreshIntervalNs, frameId);
#if defined(FILAMENT_METAL_PROFILING)
    os_signpost_interval_begin(mContext->log, mContext->signpostId, "Frame encoding", "%{public}d", frameId);
#endif
    if (mPlatform.hasDebugUpdateStatFunc()) {
        mPlatform.debugUpdateStat("filament.metal.alive_buffers", TrackedMetalBuffer::getAliveBuffers());
        mPlatform.debugUpdateStat("filament.metal.alive_buffers.generic",
                TrackedMetalBuffer::getAliveBuffers(TrackedMetalBuffer::Type::GENERIC));
        mPlatform.debugUpdateStat("filament.metal.alive_buffers.ring",
                TrackedMetalBuffer::getAliveBuffers(TrackedMetalBuffer::Type::RING));
        mPlatform.debugUpdateStat("filament.metal.alive_buffers.staging",
                TrackedMetalBuffer::getAliveBuffers(TrackedMetalBuffer::Type::STAGING));
    }
}

void MetalDriver::setFrameScheduledCallback(
        Handle<HwSwapChain> sch, CallbackHandler* handler, FrameScheduledCallback&& callback) {
    auto* swapChain = handle_cast<MetalSwapChain>(sch);
    swapChain->setFrameScheduledCallback(handler, std::move(callback));
}

void MetalDriver::setFrameCompletedCallback(
        Handle<HwSwapChain> sch, CallbackHandler* handler, utils::Invocable<void(void)>&& callback) {
    auto* swapChain = handle_cast<MetalSwapChain>(sch);
    swapChain->setFrameCompletedCallback(handler, std::move(callback));
}

void MetalDriver::execute(std::function<void(void)> const& fn) noexcept {
    @autoreleasepool {
        fn();
    }
}

void MetalDriver::setPresentationTime(int64_t monotonic_clock_ns) {
}

void MetalDriver::endFrame(uint32_t frameId) {
    DEBUG_LOG("[DS] endFrame(frameId = %d)\n", frameId);
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

void MetalDriver::updateDescriptorSetBuffer(
        backend::DescriptorSetHandle dsh,
        backend::descriptor_binding_t binding,
        backend::BufferObjectHandle boh,
        uint32_t offset,
        uint32_t size) {
    ASSERT_PRECONDITION(!isInRenderPass(mContext),
            "updateDescriptorSetBuffer must be called outside of a render pass.");
    DEBUG_LOG(
            "[DS] updateDescriptorSetBuffer(dsh = %d, binding = %d, boh = %d, offset = %d, size = "
            "%d)\n",
            dsh.getId(), binding, boh.getId(), offset, size);

    auto* descriptorSet = handle_cast<MetalDescriptorSet>(dsh);
    descriptorSet->buffers[binding] = { boh, offset, size };
}

void MetalDriver::updateDescriptorSetTexture(
        backend::DescriptorSetHandle dsh,
        backend::descriptor_binding_t binding,
        backend::TextureHandle th,
        SamplerParams params) {
    ASSERT_PRECONDITION(!isInRenderPass(mContext),
            "updateDescriptorSetTexture must be called outside of a render pass.");
    DEBUG_LOG("[DS] updateDescriptorSetTexture(dsh = %d, binding = %d, th = %d, params = {...})\n",
            dsh.getId(), binding, th.getId());

    auto* descriptorSet = handle_cast<MetalDescriptorSet>(dsh);
    descriptorSet->textures[binding] = MetalDescriptorSet::TextureBinding{ th, params };
}

void MetalDriver::flush(int) {
    FILAMENT_CHECK_PRECONDITION(!isInRenderPass(mContext))
            << "flush must be called outside of a render pass.";
    submitPendingCommands(mContext);
}

void MetalDriver::finish(int) {
    FILAMENT_CHECK_PRECONDITION(!isInRenderPass(mContext))
            << "finish must be called outside of a render pass.";
    // Wait for all frames to finish by submitting and waiting on a dummy command buffer.
    submitPendingCommands(mContext);
    id<MTLCommandBuffer> oneOffBuffer = [mContext->commandQueue commandBuffer];
    [oneOffBuffer commit];
    [oneOffBuffer waitUntilCompleted];
}

void MetalDriver::createVertexBufferInfoR(Handle<HwVertexBufferInfo> vbih, uint8_t bufferCount,
        uint8_t attributeCount, AttributeArray attributes) {
    construct_handle<MetalVertexBufferInfo>(vbih, *mContext,
            bufferCount, attributeCount, attributes);
}

void MetalDriver::createVertexBufferR(Handle<HwVertexBuffer> vbh,
        uint32_t vertexCount, Handle<HwVertexBufferInfo> vbih) {
    MetalVertexBufferInfo const* const vbi = handle_cast<const MetalVertexBufferInfo>(vbih);
    construct_handle<MetalVertexBuffer>(vbh, *mContext, vertexCount, vbi->bufferCount, vbih);
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

    DEBUG_LOG(
            "createTextureR(th = %d, target = %s, levels = %d, format = ?, samples = %d, width = "
            "%d, height = %d, depth = %d, usage = %s)\n",
            th.getId(), stringify(target), levels, samples, width, height, depth, stringify(usage));
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

    DEBUG_LOG(
            "createTextureSwizzledR(th = %d, target = %s, levels = %d, format = ?, samples = %d, "
            "width = %d, height = %d, depth = %d, usage = %s, r = ?, g = ?, b = ?, a = ?)\n",
            th.getId(), stringify(target), levels, samples, width, height, depth, stringify(usage));
}

void MetalDriver::createTextureViewR(Handle<HwTexture> th,
        Handle<HwTexture> srch, uint8_t baseLevel, uint8_t levelCount) {
    MetalTexture const* src = handle_cast<MetalTexture>(srch);
    mContext->textures.insert(construct_handle<MetalTexture>(th, *mContext,
            src, baseLevel, levelCount));
}

void MetalDriver::importTextureR(Handle<HwTexture> th, intptr_t i,
        SamplerType target, uint8_t levels,
        TextureFormat format, uint8_t samples, uint32_t width, uint32_t height,
        uint32_t depth, TextureUsage usage) {
    id<MTLTexture> metalTexture = (id<MTLTexture>) CFBridgingRelease((void*) i);
    FILAMENT_CHECK_PRECONDITION(metalTexture.width == width)
            << "Imported id<MTLTexture> width (" << metalTexture.width
            << ") != Filament texture width (" << width << ")";
    FILAMENT_CHECK_PRECONDITION(metalTexture.height == height)
            << "Imported id<MTLTexture> height (" << metalTexture.height
            << ") != Filament texture height (" << height << ")";
    FILAMENT_CHECK_PRECONDITION(metalTexture.mipmapLevelCount == levels)
            << "Imported id<MTLTexture> levels (" << metalTexture.mipmapLevelCount
            << ") != Filament texture levels (" << levels << ")";
    MTLTextureType filamentMetalType = getMetalType(target);
    FILAMENT_CHECK_PRECONDITION(metalTexture.textureType == filamentMetalType)
            << "Imported id<MTLTexture> type (" << metalTexture.textureType
            << ") != Filament texture type (" << filamentMetalType << ")";
    mContext->textures.insert(construct_handle<MetalTexture>(th, *mContext,
        target, levels, format, samples, width, height, depth, usage, metalTexture));
}

void MetalDriver::createSamplerGroupR(
        Handle<HwSamplerGroup> sbh, uint32_t size, utils::FixedSizeString<32> debugName) {
    mContext->samplerGroups.insert(construct_handle<MetalSamplerGroup>(sbh, size, debugName));
}

void MetalDriver::createRenderPrimitiveR(Handle<HwRenderPrimitive> rph,
        Handle<HwVertexBuffer> vbh, Handle<HwIndexBuffer> ibh,
        PrimitiveType pt) {
    construct_handle<MetalRenderPrimitive>(rph);
    MetalDriver::setRenderPrimitiveBuffer(rph, pt, vbh, ibh);
}

void MetalDriver::createProgramR(Handle<HwProgram> rph, Program&& program) {
    construct_handle<MetalProgram>(rph, *mContext, std::move(program));
}

void MetalDriver::createDefaultRenderTargetR(Handle<HwRenderTarget> rth, int dummy) {
    construct_handle<MetalRenderTarget>(rth, mContext);
}

void MetalDriver::createRenderTargetR(Handle<HwRenderTarget> rth,
        TargetBufferFlags targetBufferFlags, uint32_t width, uint32_t height,
        uint8_t samples, uint8_t layerCount, MRT color,
        TargetBufferInfo depth, TargetBufferInfo stencil) {
    FILAMENT_CHECK_PRECONDITION(!isInRenderPass(mContext))
            << "createRenderTarget must be called outside of a render pass.";
    // Clamp sample count to what the device supports.
    auto& sc = mContext->sampleCountLookup;
    samples = sc[std::min(MAX_SAMPLE_COUNT, samples)];

    MetalRenderTarget::Attachment colorAttachments[MRT::MAX_SUPPORTED_RENDER_TARGET_COUNT] = {{}};
    for (size_t i = 0; i < MRT::MAX_SUPPORTED_RENDER_TARGET_COUNT; i++) {
        if (none(targetBufferFlags & getTargetBufferFlagsAt(i))) {
            continue;
        }
        const auto& buffer = color[i];
        FILAMENT_CHECK_PRECONDITION(buffer.handle)
                << "The COLOR" << i << " flag was specified, but invalid color handle provided.";
        auto colorTexture = handle_cast<MetalTexture>(buffer.handle);
        FILAMENT_CHECK_PRECONDITION(colorTexture->getMtlTextureForWrite())
                << "Color texture passed to render target has no texture allocation";
        colorAttachments[i] = { colorTexture, color[i].level, color[i].layer };
    }

    MetalRenderTarget::Attachment depthAttachment = {};
    if (any(targetBufferFlags & TargetBufferFlags::DEPTH)) {
        FILAMENT_CHECK_PRECONDITION(depth.handle)
                << "The DEPTH flag was specified, but invalid depth handle provided.";
        auto depthTexture = handle_cast<MetalTexture>(depth.handle);
        FILAMENT_CHECK_PRECONDITION(depthTexture->getMtlTextureForWrite())
                << "Depth texture passed to render target has no texture allocation.";
        depthAttachment = { depthTexture, depth.level, depth.layer };
    }

    MetalRenderTarget::Attachment stencilAttachment = {};
    if (any(targetBufferFlags & TargetBufferFlags::STENCIL)) {
        FILAMENT_CHECK_PRECONDITION(stencil.handle)
                << "The STENCIL flag was specified, but invalid stencil handle provided.";
        auto stencilTexture = handle_cast<MetalTexture>(stencil.handle);
        FILAMENT_CHECK_PRECONDITION(stencilTexture->getMtlTextureForWrite())
                << "Stencil texture passed to render target has no texture allocation.";
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

const char* prettyDescriptorType(DescriptorType type) {
    switch (type) {
        case DescriptorType::UNIFORM_BUFFER: return "UNIFORM_BUFFER";
        case DescriptorType::SHADER_STORAGE_BUFFER: return "SHADER_STORAGE_BUFFER";
        case DescriptorType::SAMPLER: return "SAMPLER";
        case DescriptorType::INPUT_ATTACHMENT: return "INPUT_ATTACHMENT";
    }
}

const char* prettyShaderStageFlags(ShaderStageFlags flags) {
    std::vector<const char*> stages;
    if (any(flags & ShaderStageFlags::VERTEX)) {
        stages.push_back("VERTEX");
    }
    if (any(flags & ShaderStageFlags::FRAGMENT)) {
        stages.push_back("FRAGMENT");
    }
    if (any(flags & ShaderStageFlags::COMPUTE)) {
        stages.push_back("COMPUTE");
    }
    if (stages.empty()) {
        return "NONE";
    }
    static char buffer[64];
    buffer[0] = '\0';
    for (size_t i = 0; i < stages.size(); i++) {
        if (i > 0) {
            strcat(buffer, " | ");
        }
        strcat(buffer, stages[i]);
    }
    return buffer;
}

const char* prettyDescriptorFlags(DescriptorFlags flags) {
    if (flags == DescriptorFlags::DYNAMIC_OFFSET) {
        return "DYNAMIC_OFFSET";
    }
    return "NONE";
}

void MetalDriver::createDescriptorSetLayoutR(
        Handle<HwDescriptorSetLayout> dslh, DescriptorSetLayout&& info) {
    DEBUG_LOG("[DS] createDescriptorSetLayoutR(dslh = %d, info = {\n", dslh.getId());
    for (size_t i = 0; i < info.bindings.size(); i++) {
        DEBUG_LOG("    {binding = %d, type = %s, count = %d, stage = %s, flags = %s}",
                info.bindings[i].binding, prettyDescriptorType(info.bindings[i].type),
                info.bindings[i].count, prettyShaderStageFlags(info.bindings[i].stageFlags),
                prettyDescriptorFlags(info.bindings[i].flags));
        DEBUG_LOG(",\n");
    }
    DEBUG_LOG("})\n");
    std::sort(info.bindings.begin(), info.bindings.end(),
            [](const auto& a, const auto& b) { return a.binding < b.binding; });
    construct_handle<MetalDescriptorSetLayout>(dslh, std::move(info));
}

void MetalDriver::createDescriptorSetR(
        Handle<HwDescriptorSet> dsh, Handle<HwDescriptorSetLayout> dslh) {
    DEBUG_LOG("[DS] createDescriptorSetR(dsh = %d, dslh = %d)\n", dsh.getId(), dslh.getId());
    MetalDescriptorSetLayout* layout = handle_cast<MetalDescriptorSetLayout>(dslh);
    construct_handle<MetalDescriptorSet>(dsh, layout);
}

Handle<HwVertexBufferInfo> MetalDriver::createVertexBufferInfoS() noexcept {
    return alloc_handle<MetalVertexBufferInfo>();
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

Handle<HwTexture> MetalDriver::createTextureViewS() noexcept {
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

Handle<HwDescriptorSetLayout> MetalDriver::createDescriptorSetLayoutS() noexcept {
    return alloc_handle<MetalDescriptorSetLayout>();
}

Handle<HwDescriptorSet> MetalDriver::createDescriptorSetS() noexcept {
    return alloc_handle<MetalDescriptorSet>();
}

void MetalDriver::destroyVertexBufferInfo(Handle<HwVertexBufferInfo> vbih) {
    if (vbih) {
        destruct_handle<MetalVertexBufferInfo>(vbih);
    }
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

    auto* metalTexture = handle_cast<MetalTexture>(th);
    mContext->textures.erase(metalTexture);

    // Free memory from the texture and mark it as freed.
    metalTexture->terminate();

    // Add this texture handle to our texturesToDestroy queue to be destroyed later.
    if (auto handleToFree = mContext->texturesToDestroy.push(th)) {
        // If texturesToDestroy is full, then .push evicts the oldest texture handle in the
        // queue (or simply th, if use-after-free detection is disabled).
        destruct_handle<MetalTexture>(handleToFree.value());
    }
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

void MetalDriver::destroyDescriptorSetLayout(Handle<HwDescriptorSetLayout> dslh) {
    DEBUG_LOG("[DS] destroyDescriptorSetLayout(dslh = %d)\n", dslh.getId());
    if (dslh) {
        destruct_handle<MetalDescriptorSetLayout>(dslh);
    }
}

void MetalDriver::destroyDescriptorSet(Handle<HwDescriptorSet> dsh) {
    DEBUG_LOG("[DS] destroyDescriptorSet(dsh = %d)\n", dsh.getId());
    if (dsh) {
        destruct_handle<MetalDescriptorSet>(dsh);
    }
}

void MetalDriver::terminate() {
    // Terminate any outstanding MetalTextures.
    while (!mContext->texturesToDestroy.empty()) {
        Handle<HwTexture> toDestroy = mContext->texturesToDestroy.pop();
        destruct_handle<MetalTexture>(toDestroy);
    }

    // finish() will flush the pending command buffer and will ensure all GPU work has finished.
    // This must be done before calling bufferPool->reset() to ensure no buffers are in flight.
    finish();

    executeTickOps();

    mContext->bufferPool->reset();
    mContext->commandQueue = nil;

    MetalExternalImage::shutdown(*mContext);
    mContext->blitter->shutdown();
    mContext->shaderCompiler->terminate();
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

bool MetalDriver::isProtectedContentSupported() {
    // the SWAP_CHAIN_CONFIG_PROTECTED_CONTENT flag is not supported
    return false;
}

bool MetalDriver::isStereoSupported() {
    switch (mStereoscopicType) {
        case backend::StereoscopicType::INSTANCED:
            return true;
        case backend::StereoscopicType::MULTIVIEW:
            // TODO: implement multiview feature in Metal.
            return false;
        case backend::StereoscopicType::NONE:
            return false;
    }
}

bool MetalDriver::isParallelShaderCompileSupported() {
    return mContext->shaderCompiler->isParallelShaderCompileSupported();
}

bool MetalDriver::isDepthStencilResolveSupported() {
    return false;
}

bool MetalDriver::isDepthStencilBlitSupported(TextureFormat format) {
    return true;
}

bool MetalDriver::isProtectedTexturesSupported() {
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
    FILAMENT_CHECK_PRECONDITION(!isInRenderPass(mContext))
            << "updateBufferObject must be called outside of a render pass.";
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

void MetalDriver::update3DImage(Handle<HwTexture> th, uint32_t level,
        uint32_t xoffset, uint32_t yoffset, uint32_t zoffset,
        uint32_t width, uint32_t height, uint32_t depth,
        PixelBufferDescriptor&& data) {
    FILAMENT_CHECK_PRECONDITION(!isInRenderPass(mContext))
            << "update3DImage must be called outside of a render pass.";
    auto tex = handle_cast<MetalTexture>(th);
    tex->loadImage(level, MTLRegionMake3D(xoffset, yoffset, zoffset, width, height, depth), data);
    scheduleDestroy(std::move(data));

    DEBUG_LOG(
            "update3DImage(th = %d, level = %d, xoffset = %d, yoffset = %d, zoffset = %d, width = "
            "%d, height = %d, depth = %d, data = ?)\n",
            th.getId(), level, xoffset, yoffset, zoffset, width, height, depth);
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
    FILAMENT_CHECK_PRECONDITION(!isInRenderPass(mContext))
            << "setExternalImage must be called outside of a render pass.";
    auto texture = handle_cast<MetalTexture>(th);
    texture->externalImage.set((CVPixelBufferRef) image);
}

void MetalDriver::setExternalImagePlane(Handle<HwTexture> th, void* image, uint32_t plane) {
    FILAMENT_CHECK_PRECONDITION(!isInRenderPass(mContext))
            << "setExternalImagePlane must be called outside of a render pass.";
    auto texture = handle_cast<MetalTexture>(th);
    texture->externalImage.set((CVPixelBufferRef) image, plane);
}

void MetalDriver::setExternalStream(Handle<HwTexture> th, Handle<HwStream> sh) {
}

TimerQueryResult MetalDriver::getTimerQueryValue(Handle<HwTimerQuery> tqh, uint64_t* elapsedTime) {
    auto* tq = handle_cast<MetalTimerQuery>(tqh);
    return mContext->timerQueryImpl->getQueryResult(tq, elapsedTime) ?
           TimerQueryResult::AVAILABLE : TimerQueryResult::NOT_READY;
}

void MetalDriver::generateMipmaps(Handle<HwTexture> th) {
    FILAMENT_CHECK_PRECONDITION(!isInRenderPass(mContext))
            << "generateMipmaps must be called outside of a render pass.";
    auto tex = handle_cast<MetalTexture>(th);
    tex->generateMipmaps();

    DEBUG_LOG("generateMipmaps(th = %d)\n", th.getId());
}

void MetalDriver::updateSamplerGroup(Handle<HwSamplerGroup> sbh, BufferDescriptor&& data) {
    FILAMENT_CHECK_PRECONDITION(!isInRenderPass(mContext))
            << "updateSamplerGroup must be called outside of a render pass.";

    auto sb = handle_cast<MetalSamplerGroup>(sbh);
    assert_invariant(sb->size == data.size / sizeof(SamplerDescriptor));
    auto const* const samplers = (SamplerDescriptor const*) data.buffer;

    // Verify that all the textures in the sampler group are still alive.
    // These bugs lead to memory corruption and can be difficult to track down.
    for (size_t s = 0; s < data.size / sizeof(SamplerDescriptor); s++) {
        if (!samplers[s].t) {
            continue;
        }
        // The difference between this check and the one below is that in release, we do this for
        // only a set number of recently freed textures, while the debug check is exhaustive.
        auto* metalTexture = handle_cast<MetalTexture>(samplers[s].t);
        metalTexture->checkUseAfterFree(sb->debugName.c_str(), s);
#ifndef NDEBUG
        auto iter = mContext->textures.find(handle_cast<MetalTexture>(samplers[s].t));
        if (iter == mContext->textures.end()) {
            utils::slog.e << "updateSamplerGroup: texture #"
                          << (int) s << " is dead, texture handle = "
                          << samplers[s].t << utils::io::endl;
        }
        assert_invariant(iter != mContext->textures.end());
#endif
    }

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
            encoderCache.getOrCreateState(ArgumentEncoderState(0, std::move(textureTypes)));
    sb->reset(getPendingCommandBuffer(mContext), encoder, mContext->device);

    // In a perfect world, all the MTLTexture bindings would be known at updateSamplerGroup time.
    // However, there are two special cases preventing this:
    // 1. External images
    // 2. LOD-clamped textures
    //
    // Both of these cases prevent us from knowing the final id<MTLTexture> that will be bound into
    // the argument buffer representing the sampler group. So, we wait until draw call time to bind
    // textures (done in finalizeSamplerGroup).
    // The good news is that once a render pass has started, the texture bindings won't change.
    // A SamplerGroup is "finalized" when all of its textures have been set and is ready for use in
    // a draw call.
    // finalizeSamplerGroup has one additional responsibility: to call useResources for all the
    // textures, which is required by Metal.
    for (size_t s = 0; s < data.size / sizeof(SamplerDescriptor); s++) {
        if (!samplers[s].t) {
            // Assign a default sampler to empty slots.
            // Metal requires all samplers referenced in shaders to be bound.
            // An empty texture will be assigned inside finalizeSamplerGroup.
            id<MTLSamplerState> sampler = mContext->samplerStateCache.getOrCreateState({});
            sb->setFinalizedSampler(s, sampler);
            continue;
        }

        // Bind the sampler state. We always know the full sampler state at updateSamplerGroup time.
        SamplerState samplerState {
                .samplerParams = samplers[s].s,
        };
        id<MTLSamplerState> sampler = mContext->samplerStateCache.getOrCreateState(samplerState);
        sb->setFinalizedSampler(s, sampler);

        sb->setTextureHandle(s, samplers[s].t);
    }

    scheduleDestroy(std::move(data));
}

void MetalDriver::compilePrograms(CompilerPriorityQueue priority,
        CallbackHandler* handler, CallbackHandler::Callback callback, void* user) {
    if (callback) {
        mContext->shaderCompiler->notifyWhenAllProgramsAreReady(handler, callback, user);
    }
}

void MetalDriver::beginRenderPass(Handle<HwRenderTarget> rth,
        const RenderPassParams& params) {
    DEBUG_LOG("[DS] beginRenderPass(rth = %d, params = {...})\n", rth.getId());

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
    mContext->finalizedDescriptorSets.clear();
    mContext->descriptorBindings.invalidate();
    mContext->dynamicOffsets.setDirty(true);

    for (auto& pc : mContext->currentPushConstants) {
        pc.clear();
    }
}

void MetalDriver::nextSubpass(int dummy) {}

void MetalDriver::endRenderPass(int dummy) {
    DEBUG_LOG("[DS] endRenderPass()\n");
#if defined(FILAMENT_METAL_PROFILING)
    os_signpost_interval_end(mContext->log, OS_SIGNPOST_ID_EXCLUSIVE, "Render pass");
#endif

    [mContext->currentRenderPassEncoder endEncoding];

    // Command encoders are one time use. Set it to nil to release the encoder and ensure we don't
    // accidentally use it again.
    mContext->currentRenderPassEncoder = nil;
}

void MetalDriver::setRenderPrimitiveBuffer(Handle<HwRenderPrimitive> rph, PrimitiveType pt,
        Handle<HwVertexBuffer> vbh, Handle<HwIndexBuffer> ibh) {
    auto primitive = handle_cast<MetalRenderPrimitive>(rph);
    auto vertexBuffer = handle_cast<MetalVertexBuffer>(vbh);
    auto indexBuffer = handle_cast<MetalIndexBuffer>(ibh);
    MetalVertexBufferInfo const* const vbi = handle_cast<MetalVertexBufferInfo>(vertexBuffer->vbih);
    primitive->setBuffers(vbi, vertexBuffer, indexBuffer);
    primitive->type = pt;
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

void MetalDriver::bindSamplers(uint32_t index, Handle<HwSamplerGroup> sbh) {
    auto sb = handle_cast<MetalSamplerGroup>(sbh);
    mContext->samplerBindings[index] = sb;
}

void MetalDriver::setPushConstant(backend::ShaderStage stage, uint8_t index,
        backend::PushConstantVariant value) {
    FILAMENT_CHECK_PRECONDITION(isInRenderPass(mContext))
            << "setPushConstant must be called inside a render pass.";
    assert_invariant(static_cast<size_t>(stage) < mContext->currentPushConstants.size());
    MetalPushConstantBuffer& pushConstants =
            mContext->currentPushConstants[static_cast<size_t>(stage)];
    pushConstants.setPushConstant(value, index);
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
    FILAMENT_CHECK_PRECONDITION(!isInRenderPass(mContext))
            << "readPixels must be called outside of a render pass.";

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
    FILAMENT_CHECK_PRECONDITION(format != MTLPixelFormatInvalid)
            << "The chosen combination of PixelDataFormat (" << (int)data.format
            << ") and PixelDataType (" << (int)data.type
            << ") is not supported for "
               "readPixels.";

    const bool formatConversionNecessary = srcTexture.pixelFormat != format;

    // TODO: MetalBlitter does not currently support format conversions to integer types.
    // The format and type must match the source pixel format exactly.
    FILAMENT_CHECK_PRECONDITION(!formatConversionNecessary || !isMetalFormatInteger(format))
            << "readPixels does not support integer format conversions from MTLPixelFormat ("
            << (int)srcTexture.pixelFormat << ") to (" << (int)format << ").";

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

    MetalBlitter::BlitArgs args{};
    args.filter = SamplerMagFilter::NEAREST;
    args.source.level = miplevel;
    args.source.region = MTLRegionMake2D(0, 0, srcTexture.width >> miplevel, srcTexture.height >> miplevel);
    args.source.texture = srcTexture;
    args.destination.level = 0;
    args.destination.region = MTLRegionMake2D(0, 0, readPixelsTexture.width, readPixelsTexture.height);
    args.destination.texture = readPixelsTexture;

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

void MetalDriver::resolve(
        Handle<HwTexture> dst, uint8_t srcLevel, uint8_t srcLayer,
        Handle<HwTexture> src, uint8_t dstLevel, uint8_t dstLayer) {
    auto* const srcTexture = handle_cast<MetalTexture>(src);
    auto* const dstTexture = handle_cast<MetalTexture>(dst);
    assert_invariant(srcTexture);
    assert_invariant(dstTexture);

    FILAMENT_CHECK_PRECONDITION(mContext->currentRenderPassEncoder == nil)
            << "resolve() cannot be invoked inside a render pass.";

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

    // FIXME: on metal the blit() call below always take the slow path (using a shader)

    blit(   dst, dstLevel, dstLayer, {},
            src, srcLevel, srcLayer, {},
            { dstTexture->width, dstTexture->height });
}

void MetalDriver::blit(
        Handle<HwTexture> dst, uint8_t srcLevel, uint8_t srcLayer, math::uint2 dstOrigin,
        Handle<HwTexture> src, uint8_t dstLevel, uint8_t dstLayer, math::uint2 srcOrigin,
        math::uint2 size) {


    auto isBlitableTextureType = [](MTLTextureType t) -> bool {
        return t == MTLTextureType2D || t == MTLTextureType2DMultisample ||
               t == MTLTextureType2DArray;
    };

    auto* const srcTexture = handle_cast<MetalTexture>(src);
    auto* const dstTexture = handle_cast<MetalTexture>(dst);
    assert_invariant(srcTexture);
    assert_invariant(dstTexture);

    FILAMENT_CHECK_PRECONDITION(mContext->currentRenderPassEncoder == nil)
            << "blit() cannot be invoked inside a render pass.";

    FILAMENT_CHECK_PRECONDITION(any(dstTexture->usage & TextureUsage::BLIT_DST))
            << "texture doesn't have BLIT_DST";

    FILAMENT_CHECK_PRECONDITION(any(srcTexture->usage & TextureUsage::BLIT_SRC))
            << "texture doesn't have BLIT_SRC";

    FILAMENT_CHECK_PRECONDITION(srcTexture->format == dstTexture->format)
            << "src and dst texture format don't match";

    FILAMENT_CHECK_PRECONDITION(
            isBlitableTextureType(srcTexture->getMtlTextureForRead().textureType) &&
            isBlitableTextureType(dstTexture->getMtlTextureForWrite().textureType))
            << "Metal does not support blitting to/from non-2D textures.";

    MetalBlitter::BlitArgs args{};
    args.filter = SamplerMagFilter::NEAREST;
    args.source.region = MTLRegionMake2D(
            (NSUInteger)srcOrigin.x,
            std::max(srcTexture->height - (int64_t)srcOrigin.y - size.y, (int64_t)0),
            size.x,  size.y);
    args.destination.region = MTLRegionMake2D(
            (NSUInteger)dstOrigin.x,
            std::max(dstTexture->height - (int64_t)dstOrigin.y - size.y, (int64_t)0),
            size.x,  size.y);

    // FIXME: we shouldn't need to know the type here. This is an artifact of using the old API.

    args.source.texture = srcTexture->getMtlTextureForRead();
    args.destination.texture = dstTexture->getMtlTextureForWrite();
    args.source.level = srcLevel;
    args.source.slice = srcLayer;
    args.destination.level = dstLevel;
    args.destination.slice = dstLayer;

    // TODO: The blit() call below always take the fast path.

    mContext->blitter->blit(getPendingCommandBuffer(mContext), args, "blit/resolve");

    DEBUG_LOG(
            "blit(dst = %d, srcLevel = %d, srcLayer = %d, dstOrigin = (%d, %d), src = %d, dstLevel "
            "= %d, dstLayer = %d, srcOrigin = (%d, %d), size = (%d, %d))\n",
            dst.getId(), srcLevel, srcLayer, dstOrigin.x, dstOrigin.y, src.getId(), dstLevel,
            dstLayer, srcOrigin.x, srcOrigin.y, size.x, size.y);
}

void MetalDriver::blitDEPRECATED(TargetBufferFlags buffers,
        Handle<HwRenderTarget> dst, Viewport dstRect,
        Handle<HwRenderTarget> src, Viewport srcRect,
        SamplerMagFilter filter) {

    // Note: blitDEPRECATED is only used by Renderer::copyFrame
    // It is called between beginFrame and endFrame, but should never be called in the middle of
    // a render pass.

    FILAMENT_CHECK_PRECONDITION(mContext->currentRenderPassEncoder == nil)
            << "blitDEPRECATED() cannot be invoked inside a render pass.";

    auto srcTarget = handle_cast<MetalRenderTarget>(src);
    auto dstTarget = handle_cast<MetalRenderTarget>(dst);

    FILAMENT_CHECK_PRECONDITION(buffers == TargetBufferFlags::COLOR0)
            << "blitDEPRECATED only supports COLOR0";

    FILAMENT_CHECK_PRECONDITION(
            srcRect.left >= 0 && srcRect.bottom >= 0 && dstRect.left >= 0 && dstRect.bottom >= 0)
            << "Source and destination rects must be positive.";

    auto isBlitableTextureType = [](MTLTextureType t) {
        return t == MTLTextureType2D || t == MTLTextureType2DMultisample ||
               t == MTLTextureType2DArray;
    };

    // We always blit from/to the COLOR0 attachment.
    MetalRenderTarget::Attachment const srcColorAttachment = srcTarget->getReadColorAttachment(0);
    MetalRenderTarget::Attachment const dstColorAttachment = dstTarget->getDrawColorAttachment(0);

    if (srcColorAttachment && dstColorAttachment) {
        FILAMENT_CHECK_PRECONDITION(
                isBlitableTextureType(srcColorAttachment.getTexture().textureType) &&
                isBlitableTextureType(dstColorAttachment.getTexture().textureType))
                << "Metal does not support blitting to/from non-2D textures.";

        MetalBlitter::BlitArgs args{};
        args.filter = filter;
        args.source.region = srcTarget->getRegionFromClientRect(srcRect);
        args.source.texture = srcColorAttachment.getTexture();
        args.source.level = srcColorAttachment.level;
        args.source.slice = srcColorAttachment.layer;

        args.destination.region = dstTarget->getRegionFromClientRect(dstRect);
        args.destination.texture = dstColorAttachment.getTexture();
        args.destination.level = dstColorAttachment.level;
        args.destination.slice = dstColorAttachment.layer;

        mContext->blitter->blit(getPendingCommandBuffer(mContext), args, "blitDEPRECATED");
    }
}

void MetalDriver::finalizeSamplerGroup(MetalSamplerGroup* samplerGroup) {
    // All the id<MTLSamplerState> objects have already been bound to the argument buffer.
    // Here we bind all the textures.

    id<MTLCommandBuffer> cmdBuffer = getPendingCommandBuffer(mContext);

    // Verify that all the textures in the sampler group are still alive.
    // These bugs lead to memory corruption and can be difficult to track down.
    const auto& handles = samplerGroup->getTextureHandles();
    for (size_t s = 0; s < handles.size(); s++) {
        if (!handles[s]) {
            continue;
        }
        // The difference between this check and the one below is that in release, we do this for
        // only a set number of recently freed textures, while the debug check is exhaustive.
        auto* metalTexture = handle_cast<MetalTexture>(handles[s]);
        metalTexture->checkUseAfterFree(samplerGroup->debugName.c_str(), s);
#ifndef NDEBUG
        auto iter = mContext->textures.find(metalTexture);
        if (iter == mContext->textures.end()) {
            utils::slog.e << "finalizeSamplerGroup: texture #"
                          << (int) s << " is dead, texture handle = "
                          << handles[s] << utils::io::endl;
        }
        assert_invariant(iter != mContext->textures.end());
#endif
    }

    utils::FixedCapacityVector<id<MTLTexture>> newTextures(samplerGroup->size, nil);
    for (size_t binding = 0; binding < samplerGroup->size; binding++) {
        auto [th, _] = samplerGroup->getFinalizedTexture(binding);

        if (!th) {
            // Bind an empty texture.
            newTextures[binding] = getOrCreateEmptyTexture(mContext);
            continue;
        }

        assert_invariant(th);
        auto* texture = handle_cast<MetalTexture>(th);

        // External images
        if (texture->target == SamplerType::SAMPLER_EXTERNAL) {
            if (texture->externalImage.isValid()) {
                id<MTLTexture> mtlTexture = texture->externalImage.getMetalTextureForDraw();
                assert_invariant(mtlTexture);
                newTextures[binding] = mtlTexture;
            } else {
                // Bind an empty texture.
                newTextures[binding] = getOrCreateEmptyTexture(mContext);
            }
            continue;
        }

        newTextures[binding] = texture->getMtlTextureForRead();
    }

    if (!std::equal(newTextures.begin(), newTextures.end(), samplerGroup->textures.begin())) {
        // One or more of the id<MTLTexture>s has changed.
        // First, determine if this SamplerGroup needs mutation.
        // We can't just simply mutate the SamplerGroup, since it could currently be in use by the
        // GPU from a prior render pass.
        // If the SamplerGroup does need mutation, then there's two cases:
        // 1. The SamplerGroup has not been finalized yet (which means it has not yet been used in a
        //    draw call). We're free to mutate it.
        // 2. The SamplerGroup is finalized. We must call mutate(), which will create a new argument
        //    buffer that we can then mutate freely.

        if (samplerGroup->isFinalized()) {
            samplerGroup->mutate(cmdBuffer);
        }

        for (size_t binding = 0; binding < samplerGroup->size; binding++) {
            samplerGroup->setFinalizedTexture(binding, newTextures[binding]);
        }

        samplerGroup->finalize();
    }

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

void MetalDriver::bindPipeline(PipelineState const& ps) {
    FILAMENT_CHECK_PRECONDITION(mContext->currentRenderPassEncoder != nullptr)
            << "bindPipeline() without a valid command encoder.";

    MetalVertexBufferInfo const* const vbi =
            handle_cast<MetalVertexBufferInfo>(ps.vertexBufferInfo);

    auto program = handle_cast<MetalProgram>(ps.program);
    const auto& rs = ps.rasterState;

    // This might block until the shader compilation has finished.
    auto functions = program->getFunctions();

    // If the material debugger is enabled, avoid fatal (or cascading) errors and that can occur
    // during the draw call when the program is invalid. The shader compile error has already been
    // dumped to the console at this point, so it's fine to simply return early.
    if (FILAMENT_ENABLE_MATDBG && UTILS_UNLIKELY(!functions)) {
        return;
    }

    functions.validate();

    auto [fragment, vertex] = functions.getRasterFunctions();

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
    MetalPipelineState const pipelineState {
        .vertexFunction = vertex,
        .fragmentFunction = fragment,
        .vertexDescription = vbi->vertexDescription,
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
            .alphaBlendOperation = getMetalBlendOperation(rs.blendEquationAlpha),
            .rgbBlendOperation = getMetalBlendOperation(rs.blendEquationRGB),
            .destinationAlphaBlendFactor = getMetalBlendFactor(rs.blendFunctionDstAlpha),
            .destinationRGBBlendFactor = getMetalBlendFactor(rs.blendFunctionDstRGB),
            .sourceAlphaBlendFactor = getMetalBlendFactor(rs.blendFunctionSrcAlpha),
            .sourceRGBBlendFactor = getMetalBlendFactor(rs.blendFunctionSrcRGB),
            .blendingEnabled = rs.hasBlending(),
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

    // Bind sampler groups (argument buffers).
    for (size_t s = 0; s < Program::SAMPLER_BINDING_COUNT; s++) {
        MetalSamplerGroup* const samplerGroup = mContext->samplerBindings[s];
        if (!samplerGroup) {
            continue;
        }
//        const auto& stageFlags = program->getSamplerGroupInfo()[s].stageFlags;
//        if (stageFlags == ShaderStageFlags::NONE) {
//            continue;
//        }

        auto iter = mContext->finalizedSamplerGroups.find(samplerGroup);
        if (iter == mContext->finalizedSamplerGroups.end()) {
            finalizeSamplerGroup(samplerGroup);
            mContext->finalizedSamplerGroups.insert(samplerGroup);
        }

        assert_invariant(samplerGroup->getArgumentBuffer());

//        if (uint8_t(stageFlags) & uint8_t(ShaderStageFlags::VERTEX)) {
//            [mContext->currentRenderPassEncoder setVertexBuffer:samplerGroup->getArgumentBuffer()
//                                                         offset:samplerGroup->getArgumentBufferOffset()
//                                                        atIndex:(SAMPLER_GROUP_BINDING_START + s)];
//        }
//        if (uint8_t(stageFlags) & uint8_t(ShaderStageFlags::FRAGMENT)) {
//            [mContext->currentRenderPassEncoder setFragmentBuffer:samplerGroup->getArgumentBuffer()
//                                                           offset:samplerGroup->getArgumentBufferOffset()
//                                                          atIndex:(SAMPLER_GROUP_BINDING_START + s)];
//        }
    }
}

void MetalDriver::bindRenderPrimitive(Handle<HwRenderPrimitive> rph) {
    FILAMENT_CHECK_PRECONDITION(mContext->currentRenderPassEncoder != nullptr)
            << "bindRenderPrimitive() without a valid command encoder.";

    // Bind the user vertex buffers.
    MetalBuffer* vertexBuffers[MAX_VERTEX_BUFFER_COUNT] = {};
    size_t vertexBufferOffsets[MAX_VERTEX_BUFFER_COUNT] = {};
    size_t maxBufferIndex = 0;

    MetalRenderPrimitive const* const primitive = handle_cast<MetalRenderPrimitive>(rph);
    MetalVertexBufferInfo const* const vbi =
            handle_cast<MetalVertexBufferInfo>(primitive->vertexBuffer->vbih);

    mContext->currentRenderPrimitive = rph;

    auto vb = primitive->vertexBuffer;
    for (auto m : vbi->bufferMapping) {
        assert_invariant(
                m.bufferArgumentIndex >= USER_VERTEX_BUFFER_BINDING_START &&
                m.bufferArgumentIndex < USER_VERTEX_BUFFER_BINDING_START + MAX_VERTEX_BUFFER_COUNT);
        size_t const vertexBufferIndex = m.bufferArgumentIndex - USER_VERTEX_BUFFER_BINDING_START;
        vertexBuffers[vertexBufferIndex] = vb->buffers[m.sourceBufferIndex];
        maxBufferIndex = std::max(maxBufferIndex, vertexBufferIndex);
    }

    const auto bufferCount = maxBufferIndex + 1;
    MetalBuffer::bindBuffers(getPendingCommandBuffer(mContext), mContext->currentRenderPassEncoder,
            USER_VERTEX_BUFFER_BINDING_START, MetalBuffer::Stage::VERTEX, vertexBuffers,
            vertexBufferOffsets, bufferCount);

    // Bind the zero buffer, used for missing vertex attributes.
    static const char bytes[16] = { 0 };
    [mContext->currentRenderPassEncoder setVertexBytes:bytes
                                                length:16
                                               atIndex:ZERO_VERTEX_BUFFER_BINDING];
}

void MetalDriver::bindDescriptorSet(
        backend::DescriptorSetHandle dsh,
        backend::descriptor_set_t set,
        backend::DescriptorSetOffsetArray&& offsets) {
    auto descriptorSet = handle_cast<MetalDescriptorSet>(dsh);
    const size_t dynamicBindings = descriptorSet->layout->getDynamicOffsetCount();
    utils::FixedCapacityVector<size_t> offsetsVector(dynamicBindings, 0);
    DEBUG_LOG("[DS] bindDescriptorSet(dsh = %d, set = %d, offsets = [", dsh.getId(), set);
    for (size_t i = 0; i < dynamicBindings; i++) {
        DEBUG_LOG("%d", offsets[i]);
        if (i < dynamicBindings - 1) {
            DEBUG_LOG(", ");
        }

        offsetsVector[i] = offsets[i];
    }
    DEBUG_LOG("])\n");

    // Bind the descriptor set.
    mContext->currentDescriptorSets[set] = descriptorSet;
    mContext->descriptorBindings.setBuffer(descriptorSet->finalizeAndGetBuffer(this), 0, set);
    mContext->dynamicOffsets.setOffsets(set, offsets.data(), dynamicBindings);
}

void MetalDriver::draw2(uint32_t indexOffset, uint32_t indexCount, uint32_t instanceCount) {
    FILAMENT_CHECK_PRECONDITION(mContext->currentRenderPassEncoder != nullptr)
            << "draw() without a valid command encoder.";
    DEBUG_LOG("[DS] draw2(...)\n");

    // Finalize any descriptor sets that have been bound.
    for (size_t i = 0; i < MAX_DESCRIPTOR_SET_COUNT; i++) {
        auto* descriptorSet = mContext->currentDescriptorSets[i];
        if (!descriptorSet) {
            continue;
        }
        auto found = mContext->finalizedDescriptorSets.find(descriptorSet);
        if (found == mContext->finalizedDescriptorSets.end()) {
            descriptorSet->finalize(this);
            mContext->finalizedDescriptorSets.insert(descriptorSet);
        }
    }

    // Bind descriptor sets.
    mContext->descriptorBindings.bindBuffers(mContext->currentRenderPassEncoder, 21);

    // Bind the offset data.
    if (mContext->dynamicOffsets.isDirty()) {
        const auto [size, data] = mContext->dynamicOffsets.getOffsets();
        [mContext->currentRenderPassEncoder setFragmentBytes:data
                                                      length:size * sizeof(uint32_t)
                                                     atIndex:25];
        [mContext->currentRenderPassEncoder setVertexBytes:data
                                                    length:size * sizeof(uint32_t)
                                                   atIndex:25];
        mContext->dynamicOffsets.setDirty(false);
    }

    // Update push constants.
    for (size_t i = 0; i < Program::SHADER_TYPE_COUNT; i++) {
        auto& pushConstants = mContext->currentPushConstants[i];
        if (UTILS_UNLIKELY(pushConstants.isDirty())) {
            pushConstants.setBytes(mContext->currentRenderPassEncoder, static_cast<ShaderStage>(i));
        }
    }

    auto primitive = handle_cast<MetalRenderPrimitive>(mContext->currentRenderPrimitive);

    MetalIndexBuffer* indexBuffer = primitive->indexBuffer;

    id<MTLBuffer> metalIndexBuffer = indexBuffer->buffer.getGpuBufferForDraw();
    [mContext->currentRenderPassEncoder drawIndexedPrimitives:getMetalPrimitiveType(primitive->type)
                                                   indexCount:indexCount
                                                    indexType:getIndexType(indexBuffer->elementSize)
                                                  indexBuffer:metalIndexBuffer
                                            indexBufferOffset:indexOffset * primitive->indexBuffer->elementSize
                                                instanceCount:instanceCount];
}

void MetalDriver::draw(PipelineState ps, Handle<HwRenderPrimitive> rph,
        uint32_t const indexOffset, uint32_t const indexCount, uint32_t const instanceCount) {
    MetalRenderPrimitive const* const rp = handle_cast<MetalRenderPrimitive>(rph);
    ps.primitiveType = rp->type;
    ps.vertexBufferInfo = rp->vertexBuffer->vbih;
    bindPipeline(ps);
    bindRenderPrimitive(rph);
    draw2(indexOffset, indexCount, instanceCount);
}

void MetalDriver::dispatchCompute(Handle<HwProgram> program, math::uint3 workGroupCount) {
    FILAMENT_CHECK_PRECONDITION(!isInRenderPass(mContext))
            << "dispatchCompute must be called outside of a render pass.";

    auto mtlProgram = handle_cast<MetalProgram>(program);

    // This might block until the shader compilation has finished.
    auto functions = mtlProgram->getFunctions();

    // If the material debugger is enabled, avoid fatal (or cascading) errors and that can occur
    // during the draw call when the program is invalid. The shader compile error has already been
    // dumped to the console at this point, so it's fine to simply return early.
    if (FILAMENT_ENABLE_MATDBG && UTILS_UNLIKELY(!functions)) {
        return;
    }

    auto compute = functions.getComputeFunction();

    assert_invariant(bool(functions) && compute);

    id<MTLComputeCommandEncoder> computeEncoder =
            [getPendingCommandBuffer(mContext) computeCommandEncoder];

    NSError* error = nil;
    id<MTLComputePipelineState> computePipelineState =
            [mContext->device newComputePipelineStateWithFunction:compute
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

void MetalDriver::scissor(Viewport scissorBox) {
    // Set scissor-rectangle.
    // In order to do this, we compute the intersection between:
    //  1. the scissor rectangle
    //  2. the render target attachment dimensions (important, as the scissor can't be set larger)
    // fmax/min are used below to guard against NaN and because the MTLViewport/MTLRegion
    // coordinates are doubles.
    MTLRegion scissor = mContext->currentRenderTarget->getRegionFromClientRect(scissorBox);
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
}

void MetalDriver::beginTimerQuery(Handle<HwTimerQuery> tqh) {
    FILAMENT_CHECK_PRECONDITION(!isInRenderPass(mContext))
            << "beginTimerQuery must be called outside of a render pass.";
    auto* tq = handle_cast<MetalTimerQuery>(tqh);
    mContext->timerQueryImpl->beginTimeElapsedQuery(tq);
}

void MetalDriver::endTimerQuery(Handle<HwTimerQuery> tqh) {
    FILAMENT_CHECK_PRECONDITION(!isInRenderPass(mContext))
            << "endTimerQuery must be called outside of a render pass.";
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

void MetalDriver::runAtNextTick(const std::function<void()>& fn) noexcept {
    std::lock_guard<std::mutex> const lock(mTickOpsLock);
    mTickOps.push_back(fn);
}

void MetalDriver::executeTickOps() noexcept {
    std::vector<std::function<void()>> ops;
    mTickOpsLock.lock();
    std::swap(ops, mTickOps);
    mTickOpsLock.unlock();
    for (const auto& f : ops) {
        f();
    }
}

// explicit instantiation of the Dispatcher
template class ConcreteDispatcher<MetalDriver>;

} // namespace backend
} // namespace filament
