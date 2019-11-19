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

#include "private/backend/CommandStream.h"
#include "CommandStreamDispatcher.h"
#include "metal/MetalDriver.h"

#include "MetalContext.h"
#include "MetalDefines.h"
#include "MetalDriverFactory.h"
#include "MetalEnums.h"
#include "MetalHandles.h"
#include "MetalState.h"

#include <CoreVideo/CVMetalTexture.h>
#include <CoreVideo/CVPixelBuffer.h>
#include <Metal/Metal.h>
#include <QuartzCore/QuartzCore.h>

#include <utils/Log.h>
#include <utils/Panic.h>

namespace filament {
namespace backend {

Driver* MetalDriverFactory::create(MetalPlatform* const platform) {
    return metal::MetalDriver::create(platform);
}

namespace metal {

UTILS_NOINLINE
Driver* MetalDriver::create(MetalPlatform* const platform) {
    assert(platform);
    return new MetalDriver(platform);
}

MetalDriver::MetalDriver(backend::MetalPlatform* platform) noexcept
        : DriverBase(new ConcreteDispatcher<MetalDriver>()),
        mPlatform(*platform),
        mContext(new MetalContext) {
    mContext->device = MTLCreateSystemDefaultDevice();
    mContext->commandQueue = [mContext->device newCommandQueue];
    mContext->commandQueue.label = @"Filament";
    mContext->pipelineStateCache.setDevice(mContext->device);
    mContext->depthStencilStateCache.setDevice(mContext->device);
    mContext->samplerStateCache.setDevice(mContext->device);
    mContext->bufferPool = new MetalBufferPool(*mContext);
    mContext->blitter = new MetalBlitter(*mContext);

    CVReturn success = CVMetalTextureCacheCreate(kCFAllocatorDefault, nullptr, mContext->device,
            nullptr, &mContext->textureCache);
    ASSERT_POSTCONDITION(success == kCVReturnSuccess, "Could not create Metal texture cache.");

#if METAL_FENCES_SUPPORTED
    dispatch_queue_t queue = dispatch_get_global_queue(QOS_CLASS_DEFAULT, 0);
    mContext->eventListener = [[MTLSharedEventListener alloc] initWithDispatchQueue:queue];
#endif
}

MetalDriver::~MetalDriver() noexcept {
    mContext->device = nil;
    mContext->emptyTexture = nil;
    CFRelease(mContext->textureCache);
    delete mContext->bufferPool;
    delete mContext->blitter;
    delete mContext;
}

#define METAL_DEBUG_COMMANDS 0
#if !defined(NDEBUG)
void MetalDriver::debugCommand(const char *methodName) {
#if METAL_DEBUG_COMMANDS
    utils::slog.d << methodName << utils::io::endl;
#endif
}
#endif

void MetalDriver::beginFrame(int64_t monotonic_clock_ns, uint32_t frameId,
        backend::FrameFinishedCallback callback, void* user) {
    id<MTLCommandBuffer> commandBuffer = acquireCommandBuffer(mContext);
    [commandBuffer addCompletedHandler:^(id <MTLCommandBuffer> buffer) {
        mContext->resourceTracker.clearResources((__bridge void*) buffer);
    }];

    // If a callback was specified, then the client is responsible for presenting the frame.
    mContext->frameFinishedCallback = callback;
    mContext->frameFinishedUserData = user;
}

void MetalDriver::execute(std::function<void(void)> fn) noexcept {
    @autoreleasepool {
        fn();
    }
}

void MetalDriver::setPresentationTime(int64_t monotonic_clock_ns) {

}

void MetalDriver::endFrame(uint32_t frameId) {
    // If we haven't commited the command buffer (if the frame was canceled), do it now. There may
    // be commands in it (like fence signaling) that need to execute.
    if (mContext->currentCommandBuffer) {
        assert(mContext->currentCommandBuffer.status != MTLCommandBufferStatusCommitted);
        [mContext->currentCommandBuffer commit];
        mContext->currentCommandBuffer = nil;
    }

    mContext->bufferPool->gc();

    // If we acquired a drawable for this frame, ensure that we release it here.
    mContext->currentDrawable = nil;
    mContext->headlessDrawable = nil;

    CVMetalTextureCacheFlush(mContext->textureCache, 0);
}

void MetalDriver::flush(int) {
    // TODO: implement flush, equivalent of glFlush() (needed for performance)
}

void MetalDriver::finish(int) {
    // Wait for all frames to finish by submitting and waiting on a dummy command buffer.
    id<MTLCommandBuffer> oneOffBuffer = [mContext->commandQueue commandBuffer];
    [oneOffBuffer commit];
    [oneOffBuffer waitUntilCompleted];
}

void MetalDriver::createVertexBufferR(Handle<HwVertexBuffer> vbh, uint8_t bufferCount,
        uint8_t attributeCount, uint32_t vertexCount, AttributeArray attributes,
        BufferUsage usage) {
    // TODO: Take BufferUsage into account when creating the buffer.
    construct_handle<MetalVertexBuffer>(mHandleMap, vbh, mContext->device, bufferCount,
            attributeCount, vertexCount, attributes);
}

void MetalDriver::createIndexBufferR(Handle<HwIndexBuffer> ibh, ElementType elementType,
        uint32_t indexCount, BufferUsage usage) {
    auto elementSize = (uint8_t) getElementTypeSize(elementType);
    construct_handle<MetalIndexBuffer>(mHandleMap, ibh, mContext->device, elementSize, indexCount);
}

void MetalDriver::createTextureR(Handle<HwTexture> th, SamplerType target, uint8_t levels,
        TextureFormat format, uint8_t samples, uint32_t width, uint32_t height,
        uint32_t depth, TextureUsage usage) {
    construct_handle<MetalTexture>(mHandleMap, th, *mContext, target, levels, format, samples,
            width, height, depth, usage);
}

void MetalDriver::createSamplerGroupR(Handle<HwSamplerGroup> sbh, size_t size) {
    construct_handle<MetalSamplerGroup>(mHandleMap, sbh, size);
}

void MetalDriver::createUniformBufferR(Handle<HwUniformBuffer> ubh, size_t size,
        BufferUsage usage) {
    construct_handle<MetalUniformBuffer>(mHandleMap, ubh, *mContext, size);
}

void MetalDriver::createRenderPrimitiveR(Handle<HwRenderPrimitive> rph, int dummy) {
    construct_handle<MetalRenderPrimitive>(mHandleMap, rph);
}

void MetalDriver::createProgramR(Handle<HwProgram> rph, Program&& program) {
    construct_handle<MetalProgram>(mHandleMap, rph, mContext->device, program);
}

void MetalDriver::createDefaultRenderTargetR(Handle<HwRenderTarget> rth, int dummy) {
    construct_handle<MetalRenderTarget>(mHandleMap, rth, mContext);
}

void MetalDriver::createRenderTargetR(Handle<HwRenderTarget> rth,
        TargetBufferFlags targetBufferFlags, uint32_t width, uint32_t height,
        uint8_t samples, TargetBufferInfo color,
        TargetBufferInfo depth, TargetBufferInfo stencil) {

    auto getColorTexture = [&]() -> id<MTLTexture> {
        if (color.handle) {
            auto colorTexture = handle_cast<MetalTexture>(mHandleMap, color.handle);
            ASSERT_PRECONDITION(colorTexture->texture,
                    "Color texture passed to render target has no texture allocation");
            return colorTexture->texture;
        } else if (any(targetBufferFlags & TargetBufferFlags::COLOR)) {
            ASSERT_POSTCONDITION(false, "The COLOR flag was specified, but no color texture provided.");
        }
        return nil;
    };

    auto getDepthTexture = [&]() -> id<MTLTexture> {
        if (depth.handle) {
            auto depthTexture = handle_cast<MetalTexture>(mHandleMap, depth.handle);
            ASSERT_PRECONDITION(depthTexture->texture,
                    "Depth texture passed to render target has no texture allocation.");
            return depthTexture->texture;
        } else if (any(targetBufferFlags & TargetBufferFlags::DEPTH)) {
            ASSERT_POSTCONDITION(false, "The DEPTH flag was specified, but no depth texture provided.");
        }
        return nil;
    };

    construct_handle<MetalRenderTarget>(mHandleMap, rth, mContext, width, height, samples,
            getColorTexture(), getDepthTexture(), color.level, depth.level);

    ASSERT_POSTCONDITION(
            !stencil.handle &&
            !(targetBufferFlags & TargetBufferFlags::STENCIL),
            "Stencil buffer not supported.");
}

void MetalDriver::createFenceR(Handle<HwFence> fh, int dummy) {
    ASSERT_PRECONDITION(mContext->currentCommandBuffer, "createFence must be called within a frame.");
    construct_handle<MetalFence>(mHandleMap, fh, *mContext);
}

void MetalDriver::createSwapChainR(Handle<HwSwapChain> sch, void* nativeWindow, uint64_t flags) {
    auto* metalLayer = (__bridge CAMetalLayer*) nativeWindow;
    construct_handle<MetalSwapChain>(mHandleMap, sch, mContext->device, metalLayer);
}

void MetalDriver::createSwapChainHeadlessR(Handle<HwSwapChain> sch,
        uint32_t width, uint32_t height, uint64_t flags) {
    construct_handle<MetalSwapChain>(mHandleMap, sch, width, height);
}

void MetalDriver::createStreamFromTextureIdR(Handle<HwStream>, intptr_t externalTextureId,
        uint32_t width, uint32_t height) {

}

Handle<HwVertexBuffer> MetalDriver::createVertexBufferS() noexcept {
    return alloc_handle<MetalVertexBuffer, HwVertexBuffer>();
}

Handle<HwIndexBuffer> MetalDriver::createIndexBufferS() noexcept {
    return alloc_handle<MetalIndexBuffer, HwIndexBuffer>();
}

Handle<HwTexture> MetalDriver::createTextureS() noexcept {
    return alloc_handle<MetalTexture, HwTexture>();
}

Handle<HwSamplerGroup> MetalDriver::createSamplerGroupS() noexcept {
    return alloc_handle<MetalSamplerGroup, HwSamplerGroup>();
}

Handle<HwUniformBuffer> MetalDriver::createUniformBufferS() noexcept {
    return alloc_handle<MetalUniformBuffer, HwUniformBuffer>();
}

Handle<HwRenderPrimitive> MetalDriver::createRenderPrimitiveS() noexcept {
    return alloc_handle<MetalRenderPrimitive, HwRenderPrimitive>();
}

Handle<HwProgram> MetalDriver::createProgramS() noexcept {
    return alloc_handle<MetalProgram, HwProgram>();
}

Handle<HwRenderTarget> MetalDriver::createDefaultRenderTargetS() noexcept {
    return alloc_handle<MetalRenderTarget, HwRenderTarget>();
}

Handle<HwRenderTarget> MetalDriver::createRenderTargetS() noexcept {
    return alloc_handle<MetalRenderTarget, HwRenderTarget>();
}

Handle<HwFence> MetalDriver::createFenceS() noexcept {
    return alloc_handle<MetalFence, HwFence>();
}

Handle<HwSwapChain> MetalDriver::createSwapChainS() noexcept {
    return alloc_handle<MetalSwapChain, HwSwapChain>();
}

Handle<HwSwapChain> MetalDriver::createSwapChainHeadlessS() noexcept {
    return alloc_handle<MetalSwapChain, HwSwapChain>();
}

Handle<HwStream> MetalDriver::createStreamFromTextureIdS() noexcept {
    return {};
}

void MetalDriver::destroyVertexBuffer(Handle<HwVertexBuffer> vbh) {
    if (vbh) {
        destruct_handle<MetalVertexBuffer>(mHandleMap, vbh);
    }
}

void MetalDriver::destroyIndexBuffer(Handle<HwIndexBuffer> ibh) {
    if (ibh) {
        destruct_handle<MetalIndexBuffer>(mHandleMap, ibh);
    }
}

void MetalDriver::destroyRenderPrimitive(Handle<HwRenderPrimitive> rph) {
    if (rph) {
        destruct_handle<MetalRenderPrimitive>(mHandleMap, rph);
    }
}

void MetalDriver::destroyProgram(Handle<HwProgram> ph) {
    if (ph) {
        destruct_handle<MetalProgram>(mHandleMap, ph);
    }
}

void MetalDriver::destroySamplerGroup(Handle<HwSamplerGroup> sbh) {
    if (!sbh) {
        return;
    }
    // Unbind this sampler group from our internal state.
    auto* metalSampler = handle_cast<MetalSamplerGroup>(mHandleMap, sbh);
    for (auto& samplerBinding : mContext->samplerBindings) {
        if (samplerBinding == metalSampler) {
            samplerBinding = {};
        }
    }
    destruct_handle<MetalSamplerGroup>(mHandleMap, sbh);
}

void MetalDriver::destroyUniformBuffer(Handle<HwUniformBuffer> ubh) {
    if (!ubh) {
        return;
    }
    destruct_handle<MetalUniformBuffer>(mHandleMap, ubh);
    for (auto& thisUniform : mContext->uniformState) {
        if (thisUniform.ubh == ubh) {
            thisUniform.bound = false;
        }
    }
}

void MetalDriver::destroyTexture(Handle<HwTexture> th) {
    if (!th) {
        return;
    }
    // Unbind this texture from any sampler groups that currently reference it.
    for (auto& samplerBinding : mContext->samplerBindings) {
        if (!samplerBinding) {
            continue;
        }
        const SamplerGroup::Sampler* samplers = samplerBinding->sb->getSamplers();
        for (size_t i = 0; i < samplerBinding->sb->getSize(); i++) {
            const SamplerGroup::Sampler* sampler = samplers + i;
            if (sampler->t == th) {
                samplerBinding->sb->setSampler(i, {{}, {}});
            }
        }
    }
    destruct_handle<MetalTexture>(mHandleMap, th);
}

void MetalDriver::destroyRenderTarget(Handle<HwRenderTarget> rth) {
    if (rth) {
        destruct_handle<MetalRenderTarget>(mHandleMap, rth);
    }
}

void MetalDriver::destroySwapChain(Handle<HwSwapChain> sch) {
    if (sch) {
        destruct_handle<MetalSwapChain>(mHandleMap, sch);
    }
}

void MetalDriver::destroyStream(Handle<HwStream> sh) {
    // no-op
}

void MetalDriver::terminate() {
    // Wait for all frames to finish by submitting and waiting on a dummy command buffer.
    // This must be done before calling bufferPool->reset() to ensure no buffers are in flight.
    id<MTLCommandBuffer> oneOffBuffer = [mContext->commandQueue commandBuffer];
    [oneOffBuffer commit];
    [oneOffBuffer waitUntilCompleted];

    mContext->bufferPool->reset();
    mContext->commandQueue = nil;

    MetalExternalImage::shutdown();
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

void MetalDriver::setAcquiredImage(Handle<HwStream> sh, void* image, backend::StreamCallback cb,
        void* userData) {
}

void MetalDriver::setStreamDimensions(Handle<HwStream> stream, uint32_t width,
        uint32_t height) {

}

int64_t MetalDriver::getStreamTimestamp(Handle<HwStream> stream) {
    return 0;
}

void MetalDriver::updateStreams(backend::DriverApi* driver) {

}

void MetalDriver::destroyFence(Handle<HwFence> fh) {
    if (fh) {
        destruct_handle<MetalFence>(mHandleMap, fh);
    }
}

FenceStatus MetalDriver::wait(Handle<HwFence> fh, uint64_t timeout) {
    auto* fence = handle_cast<MetalFence>(mHandleMap, fh);
    if (!fence) {
        return FenceStatus::ERROR;
    }
    return fence->wait(timeout);
}

bool MetalDriver::isTextureFormatSupported(TextureFormat format) {
    return getMetalFormat(format) != MTLPixelFormatInvalid ||
           TextureReshaper::canReshapeTextureFormat(format);
}

bool MetalDriver::isTextureFormatMipmappable(TextureFormat format) {
    // Derived from the Metal 3.0 Feature Set Tables.
    // In order for a format to be mipmappable, it must be color-renderable and filterable.
    auto isMipmappable = [](TextureFormat format) {
        switch (format) {
            // Mipmappable across all devices:
            case TextureFormat::R8:
            case TextureFormat::R8_SNORM:
            case TextureFormat::R16F:
            case TextureFormat::RG8:
            case TextureFormat::RG8_SNORM:
            case TextureFormat::RG16F:
            case TextureFormat::RGBA8:
            case TextureFormat::SRGB8_A8:
            case TextureFormat::RGBA8_SNORM:
            case TextureFormat::RGB10_A2:
            case TextureFormat::R11F_G11F_B10F:
            case TextureFormat::RGBA16F:
                return true;

#if !defined(IOS)
            // Mipmappable only on desktop:
            case TextureFormat::R32F:
            case TextureFormat::RG32F:
            case TextureFormat::RGBA32F:
                return true;
#endif

#if defined(IOS)
            // Mipmappable only on iOS:
            case TextureFormat::RGB9_E5:
                return true;
#endif

            default:
                return false;
        }
    };

    // Certain Filament formats aren't natively supported by Metal, but can be reshaped into
    // supported Formats.
    TextureReshaper reshaper(format);
    return isMipmappable(format) || isMipmappable(reshaper.getReshapedFormat());
}

bool MetalDriver::isRenderTargetFormatSupported(TextureFormat format) {
    MTLPixelFormat mtlFormat = getMetalFormat(format);
    // RGB9E5 isn't supported on Mac as a color render target.
    return mtlFormat != MTLPixelFormatInvalid && mtlFormat != MTLPixelFormatRGB9E5Float;
}

bool MetalDriver::isFrameTimeSupported() {
    // Frame time is calculated via hard fences.
#if METAL_FENCES_SUPPORTED
    return true;
#else
    return false;
#endif
}

// TODO: the implementations here for updateVertexBuffer and updateIndexBuffer assume static usage.
// Dynamically updated vertex / index buffers will require synchronization.

void MetalDriver::updateVertexBuffer(Handle<HwVertexBuffer> vbh, size_t index,
        BufferDescriptor&& data, uint32_t byteOffset) {
    assert(byteOffset == 0);    // TODO: handle byteOffset for vertex buffers
    auto* vb = handle_cast<MetalVertexBuffer>(mHandleMap, vbh);
    memcpy(vb->buffers[index].contents, data.buffer, data.size);
}

void MetalDriver::updateIndexBuffer(Handle<HwIndexBuffer> ibh, BufferDescriptor&& data,
        uint32_t byteOffset) {
    assert(byteOffset == 0);    // TODO: handle byteOffset for index buffers
    auto* ib = handle_cast<MetalIndexBuffer>(mHandleMap, ibh);
    memcpy(ib->buffer.contents, data.buffer, data.size);
}

void MetalDriver::update2DImage(Handle<HwTexture> th, uint32_t level, uint32_t xoffset,
        uint32_t yoffset, uint32_t width, uint32_t height, PixelBufferDescriptor&& data) {
    auto tex = handle_cast<MetalTexture>(mHandleMap, th);
    tex->load2DImage(level, xoffset, yoffset, width, height, data);
    scheduleDestroy(std::move(data));
}

void MetalDriver::updateCubeImage(Handle<HwTexture> th, uint32_t level,
        PixelBufferDescriptor&& data, FaceOffsets faceOffsets) {
    auto tex = handle_cast<MetalTexture>(mHandleMap, th);
    tex->loadCubeImage(data, faceOffsets, level);
    scheduleDestroy(std::move(data));
}

void MetalDriver::setupExternalImage(void* image) {
    // Take ownership of the passed in buffer. It will be released the next time
    // setExternalImage is called, or when the texture is destroyed.
    CVPixelBufferRef pixelBuffer = (CVPixelBufferRef) image;
    CVPixelBufferRetain(pixelBuffer);
}

void MetalDriver::cancelExternalImage(void* image) {
    CVPixelBufferRef pixelBuffer = (CVPixelBufferRef) image;
    CVPixelBufferRelease(pixelBuffer);
}

void MetalDriver::setExternalImage(Handle<HwTexture> th, void* image) {
    auto texture = handle_cast<MetalTexture>(mHandleMap, th);
    texture->externalImage.set((CVPixelBufferRef) image);
}

void MetalDriver::setExternalStream(Handle<HwTexture> th, Handle<HwStream> sh) {

}

void MetalDriver::generateMipmaps(Handle<HwTexture> th) {
    auto tex = handle_cast<MetalTexture>(mHandleMap, th);
    // Create a one-off command buffer to execute the blit command. Technically, we could re-use
    // this command buffer for later rendering commands, but we'll just commit it here for
    // simplicity.
    id <MTLCommandBuffer> commandBuffer = [mContext->commandQueue commandBuffer];
    id <MTLBlitCommandEncoder> blitEncoder = [commandBuffer blitCommandEncoder];
    [blitEncoder generateMipmapsForTexture:tex->texture];
    [blitEncoder endEncoding];
    [commandBuffer commit];
}

bool MetalDriver::canGenerateMipmaps() {
    return true;
}

void MetalDriver::loadUniformBuffer(Handle<HwUniformBuffer> ubh,
        BufferDescriptor&& data) {
    if (data.size <= 0) {
       return;
    }

    auto buffer = handle_cast<MetalUniformBuffer>(mHandleMap, ubh);

    buffer->copyIntoBuffer(data.buffer, data.size);
    scheduleDestroy(std::move(data));
}

void MetalDriver::updateSamplerGroup(Handle<HwSamplerGroup> sbh,
        SamplerGroup&& samplerGroup) {
    auto sb = handle_cast<MetalSamplerGroup>(mHandleMap, sbh);
    *sb->sb = samplerGroup;
}

void MetalDriver::beginRenderPass(Handle<HwRenderTarget> rth,
        const RenderPassParams& params) {
    auto renderTarget = handle_cast<MetalRenderTarget>(mHandleMap, rth);
    mContext->currentRenderTarget = renderTarget;
    mContext->currentRenderPassFlags = params.flags;

    MTLRenderPassDescriptor* descriptor = [MTLRenderPassDescriptor renderPassDescriptor];

    const auto discardFlags = params.flags.discardEnd;
    const bool discardColor = any(discardFlags & TargetBufferFlags::COLOR);
    const bool discardDepth = any(discardFlags & TargetBufferFlags::DEPTH);

    // Color

    const auto& colorAttachment = descriptor.colorAttachments[0];
    colorAttachment.texture = renderTarget->getColor();
    colorAttachment.resolveTexture = discardColor ? nil : renderTarget->getColorResolve();
    colorAttachment.level = renderTarget->getColorLevel();
    mContext->currentSurfacePixelFormat = colorAttachment.texture.pixelFormat;

    // Metal clears the entire attachment without respect to viewport or scissor.
    // TODO: Might need to clear the scissor area manually via a draw if we need that functionality.

    colorAttachment.loadAction = renderTarget->getLoadAction(params, TargetBufferFlags::COLOR);
    colorAttachment.storeAction = renderTarget->getStoreAction(params, TargetBufferFlags::COLOR);
    colorAttachment.clearColor = MTLClearColorMake(
            params.clearColor.r, params.clearColor.g, params.clearColor.b, params.clearColor.a);

    // Depth

    const auto& depthAttachment = descriptor.depthAttachment;
    depthAttachment.texture = renderTarget->getDepth();
    depthAttachment.resolveTexture = discardDepth ? nil : renderTarget->getDepthResolve();
    depthAttachment.loadAction = renderTarget->getLoadAction(params, TargetBufferFlags::DEPTH);
    depthAttachment.storeAction = renderTarget->getStoreAction(params, TargetBufferFlags::DEPTH);
    depthAttachment.clearDepth = params.clearDepth;
    depthAttachment.level = renderTarget->getDepthLevel();
    mContext->currentDepthPixelFormat = descriptor.depthAttachment.texture.pixelFormat;

    mContext->currentCommandEncoder =
            [mContext->currentCommandBuffer renderCommandEncoderWithDescriptor:descriptor];

    // Filament's default winding is counter clockwise.
    [mContext->currentCommandEncoder setFrontFacingWinding:MTLWindingCounterClockwise];

    // Flip the viewport, because Metal's screen space is vertically flipped that of Filament's.
    NSInteger renderTargetHeight =
            mContext->currentRenderTarget->isDefaultRenderTarget() ?
            mContext->currentSurface->surfaceHeight : mContext->currentRenderTarget->height;
    MTLViewport metalViewport {
            .originX = static_cast<double>(params.viewport.left),
            .originY = renderTargetHeight - static_cast<double>(params.viewport.bottom) -
                       static_cast<double>(params.viewport.height),
            .width = static_cast<double>(params.viewport.width),
            .height = static_cast<double>(params.viewport.height),
            .znear = 0.0,
            .zfar = 1.0
    };
    [mContext->currentCommandEncoder setViewport:metalViewport];

    // Metal requires a new command encoder for each render pass, and they cannot be reused.
    // We must bind certain states for each command encoder, so we dirty the states here to force a
    // rebinding at the first the draw call of this pass.
    mContext->pipelineState.invalidate();
    mContext->depthStencilState.invalidate();
    mContext->cullModeState.invalidate();
}

void MetalDriver::endRenderPass(int dummy) {
    [mContext->currentCommandEncoder endEncoding];

    // Command encoders are one time use. Set it to nil to release the encoder and ensure we don't
    // accidentally use it again.
    mContext->currentCommandEncoder = nil;
}

void MetalDriver::discardSubRenderTargetBuffers(Handle<HwRenderTarget> rth,
        TargetBufferFlags targetBufferFlags, uint32_t left, uint32_t bottom, uint32_t width,
        uint32_t height) {

}

void MetalDriver::setRenderPrimitiveBuffer(Handle<HwRenderPrimitive> rph,
        Handle<HwVertexBuffer> vbh, Handle<HwIndexBuffer> ibh, uint32_t enabledAttributes) {
    auto primitive = handle_cast<MetalRenderPrimitive>(mHandleMap, rph);
    auto vertexBuffer = handle_cast<MetalVertexBuffer>(mHandleMap, vbh);
    auto indexBuffer = handle_cast<MetalIndexBuffer>(mHandleMap, ibh);
    primitive->setBuffers(vertexBuffer, indexBuffer, enabledAttributes);
}

void MetalDriver::setRenderPrimitiveRange(Handle<HwRenderPrimitive> rph,
        PrimitiveType pt, uint32_t offset, uint32_t minIndex, uint32_t maxIndex,
        uint32_t count) {
    auto primitive = handle_cast<MetalRenderPrimitive>(mHandleMap, rph);
    primitive->type = pt;
    primitive->offset = offset * primitive->indexBuffer->elementSize;
    primitive->count = count;
    primitive->minIndex = minIndex;
    primitive->maxIndex = maxIndex > minIndex ? maxIndex : primitive->maxVertexCount - 1;
}

void MetalDriver::makeCurrent(Handle<HwSwapChain> schDraw, Handle<HwSwapChain> schRead) {
    ASSERT_PRECONDITION_NON_FATAL(schDraw == schRead,
                                  "Metal driver does not support distinct draw/read swap chains.");
    auto* swapChain = handle_cast<MetalSwapChain>(mHandleMap, schDraw);
    mContext->currentSurface = swapChain;
}

void MetalDriver::commit(Handle<HwSwapChain> sch) {
    if (mContext->currentDrawable != nil && !mContext->frameFinishedCallback) {
        [mContext->currentCommandBuffer presentDrawable:mContext->currentDrawable];
    }
    [mContext->currentCommandBuffer commit];
    mContext->currentCommandBuffer = nil;
    mContext->currentDrawable = nil;
    mContext->headlessDrawable = nil;
}

void MetalDriver::bindUniformBuffer(size_t index, Handle<HwUniformBuffer> ubh) {
    mContext->uniformState[index] = UniformBufferState {
        .bound = true,
        .ubh = ubh,
        .offset = 0
    };
}

void MetalDriver::bindUniformBufferRange(size_t index, Handle<HwUniformBuffer> ubh,
        size_t offset, size_t size) {
    mContext->uniformState[index] = UniformBufferState {
        .bound = true,
        .ubh = ubh,
        .offset = offset
    };
}

void MetalDriver::bindSamplers(size_t index, Handle<HwSamplerGroup> sbh) {
    auto sb = handle_cast<MetalSamplerGroup>(mHandleMap, sbh);
    mContext->samplerBindings[index] = sb;
}

void MetalDriver::insertEventMarker(const char* string, size_t len) {

}

void MetalDriver::pushGroupMarker(const char* string, size_t len) {

}

void MetalDriver::popGroupMarker(int dummy) {

}

void MetalDriver::startCapture(int) {
    [[MTLCaptureManager sharedCaptureManager] startCaptureWithDevice:mContext->device];
}

void MetalDriver::stopCapture(int) {
    [[MTLCaptureManager sharedCaptureManager] stopCapture];
}

void MetalDriver::readPixels(Handle<HwRenderTarget> src, uint32_t x, uint32_t y, uint32_t width,
        uint32_t height, PixelBufferDescriptor&& data) {
    ASSERT_PRECONDITION(mContext->currentCommandBuffer != nil &&
                        mContext->currentCommandEncoder == nil,
                        "readPixels must be called during a frame, but outside of a render pass.");

    auto srcTarget = handle_cast<MetalRenderTarget>(mHandleMap, src);
    id<MTLTexture> srcTexture = srcTarget->getColor();
    size_t miplevel = srcTarget->getColorLevel();

    auto chooseMetalPixelFormat = [] (PixelDataFormat format, PixelDataType type) {
        // TODO: Add support for UINT and INT
        if (format == PixelDataFormat::RGBA && type == PixelDataType::UBYTE) {
                return MTLPixelFormatRGBA8Unorm;
        }

        if (format == PixelDataFormat::RGBA && type == PixelDataType::FLOAT) {
                return MTLPixelFormatRGBA32Float;
        }

        return MTLPixelFormatInvalid;
    };

    const MTLPixelFormat format = chooseMetalPixelFormat(data.format, data.type);
    ASSERT_PRECONDITION(format != MTLPixelFormatInvalid,
            "The chosen combination of PixelDataFormat and PixelDataType is not supported for "
            "readPixels.");
    MTLTextureDescriptor* textureDescriptor =
            [MTLTextureDescriptor texture2DDescriptorWithPixelFormat:format
                                                               width:(srcTexture.width >> miplevel)
                                                              height:(srcTexture.height >> miplevel)
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

    mContext->blitter->blit(args);

#if !defined(IOS)
    // Managed textures on macOS require explicit synchronization between GPU / CPU.
    id <MTLBlitCommandEncoder> blitEncoder = [mContext->currentCommandBuffer blitCommandEncoder];
    [blitEncoder synchronizeResource:readPixelsTexture];
    [blitEncoder endEncoding];
#endif

    // TODO: right now, every Filament frame gets its own command buffer. We should adopt a more
    // granular approach to command buffer allocation, as this command buffer won't start executing
    // until the entire frame has been encoded.

    PixelBufferDescriptor* p = new PixelBufferDescriptor(std::move(data));
    [mContext->currentCommandBuffer addCompletedHandler:^(id<MTLCommandBuffer> cb) {
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

void MetalDriver::readStreamPixels(Handle<HwStream> sh, uint32_t x, uint32_t y, uint32_t width,
        uint32_t height, PixelBufferDescriptor&& data) {

}

void MetalDriver::blit(TargetBufferFlags buffers,
        Handle<HwRenderTarget> dst, backend::Viewport dstRect,
        Handle<HwRenderTarget> src, backend::Viewport srcRect,
        SamplerMagFilter filter) {
    ASSERT_PRECONDITION(mContext->currentCommandBuffer != nil &&
                        mContext->currentCommandEncoder == nil,
                        "Blitting must be done during a frame, but outside of a render pass.");

    auto srcTarget = handle_cast<MetalRenderTarget>(mHandleMap, src);
    auto dstTarget = handle_cast<MetalRenderTarget>(mHandleMap, dst);

    ASSERT_PRECONDITION(srcRect.left >= 0 && srcRect.bottom >= 0 &&
                        dstRect.left >= 0 && dstRect.bottom >= 0,
            "Source and destination rects must be positive.");

    id<MTLTexture> srcTexture = srcTarget->getBlitColorSource();
    id<MTLTexture> dstTexture = dstTarget->getColor();

    // Metal's texture coordinates have (0, 0) at the top-left of the texture, but Filament's
    // coordinates have (0, 0) at bottom-left.
    MTLRegion srcRegion = MTLRegionMake2D(
            (NSUInteger) srcRect.left,
            srcTexture.height - (NSUInteger) srcRect.bottom - srcRect.height,
            srcRect.width, srcRect.height);

    MTLRegion dstRegion = MTLRegionMake2D(
            (NSUInteger) dstRect.left,
            dstTexture.height - (NSUInteger) dstRect.bottom - dstRect.height,
            dstRect.width, dstRect.height);

    const uint8_t srcLevel = srcTarget->getColorLevel();
    const uint8_t dstLevel = dstTarget->getColorLevel();

    auto isBlitableTextureType = [](MTLTextureType t) {
        return t == MTLTextureType2D || t == MTLTextureType2DMultisample;
    };
    ASSERT_PRECONDITION(isBlitableTextureType(srcTexture.textureType) &&
                        isBlitableTextureType(dstTexture.textureType),
                       "Metal does not support blitting to/from non-2D textures.");

    MetalBlitter::BlitArgs args;
    args.filter = filter;
    args.source.level = srcLevel;
    args.source.region = srcRegion;
    args.destination.level = dstLevel;
    args.destination.region = dstRegion;

    if (any(buffers & TargetBufferFlags::COLOR)) {
        args.source.color = srcTexture;
        args.destination.color = dstTexture;
    }

    if (any(buffers & TargetBufferFlags::DEPTH)) {
        args.source.depth = srcTarget->getBlitDepthSource();
        args.destination.depth = dstTarget->getDepth();
    }

    mContext->blitter->blit(args);
}

void MetalDriver::draw(backend::PipelineState ps, Handle<HwRenderPrimitive> rph) {
    ASSERT_PRECONDITION(mContext->currentCommandEncoder != nullptr,
            "Attempted to draw without a valid command encoder.");
    auto primitive = handle_cast<MetalRenderPrimitive>(mHandleMap, rph);
    auto program = handle_cast<MetalProgram>(mHandleMap, ps.program);
    const auto& rs = ps.rasterState;

    // Pipeline state
    metal::PipelineState pipelineState {
        .vertexFunction = program->vertexFunction,
        .fragmentFunction = program->fragmentFunction,
        .vertexDescription = primitive->vertexDescription,
        .colorAttachmentPixelFormat = mContext->currentSurfacePixelFormat,
        .depthAttachmentPixelFormat = mContext->currentDepthPixelFormat,
        .sampleCount = mContext->currentRenderTarget->getSamples(),
        .blendState = BlendState {
            .blendingEnabled = rs.hasBlending(),
            .rgbBlendOperation = getMetalBlendOperation(rs.blendEquationRGB),
            .alphaBlendOperation = getMetalBlendOperation(rs.blendEquationAlpha),
            .sourceRGBBlendFactor = getMetalBlendFactor(rs.blendFunctionSrcRGB),
            .sourceAlphaBlendFactor = getMetalBlendFactor(rs.blendFunctionSrcAlpha),
            .destinationRGBBlendFactor = getMetalBlendFactor(rs.blendFunctionDstRGB),
            .destinationAlphaBlendFactor = getMetalBlendFactor(rs.blendFunctionDstAlpha)
        }
    };
    mContext->pipelineState.updateState(pipelineState);
    if (mContext->pipelineState.stateChanged()) {
        id<MTLRenderPipelineState> pipeline =
                mContext->pipelineStateCache.getOrCreateState(pipelineState);
        assert(pipeline != nil);
        [mContext->currentCommandEncoder setRenderPipelineState:pipeline];
    }

    // Cull mode
    MTLCullMode cullMode = getMetalCullMode(rs.culling);
    mContext->cullModeState.updateState(cullMode);
    if (mContext->cullModeState.stateChanged()) {
        [mContext->currentCommandEncoder setCullMode:cullMode];
    }

    // Set the depth-stencil state, if a state change is needed.
    DepthStencilState depthState {
        .compareFunction = getMetalCompareFunction(rs.depthFunc),
        .depthWriteEnabled = rs.depthWrite,
    };
    mContext->depthStencilState.updateState(depthState);
    if (mContext->depthStencilState.stateChanged()) {
        id<MTLDepthStencilState> state =
                mContext->depthStencilStateCache.getOrCreateState(depthState);
        assert(state != nil);
        [mContext->currentCommandEncoder setDepthStencilState:state];
    }

    if (ps.polygonOffset.constant != 0.0 || ps.polygonOffset.slope != 0.0) {
        [mContext->currentCommandEncoder setDepthBias:ps.polygonOffset.constant
                                         slopeScale:ps.polygonOffset.slope
                                              clamp:0.0];
    }

    // FIXME: implement take ps.scissor into account
    //  must be intersected with viewport (see OpenGLDriver.cpp for implementation details)

    // Bind uniform buffers.
    id<MTLBuffer> uniformsToBind[Program::UNIFORM_BINDING_COUNT] = { nil };
    NSUInteger offsets[Program::UNIFORM_BINDING_COUNT] = { 0 };

    enumerateBoundUniformBuffers([&uniformsToBind, &offsets](const UniformBufferState& state,
            MetalUniformBuffer* uniform, uint32_t index) {
        // getGpuBuffer() might return nil, which means there isn't a device allocation for this
        // uniform. In this case, we'll update the uniform below with our CPU-side buffer via
        // setVertexBytes and setFragmentBytes.
        id<MTLBuffer> gpuBuffer = uniform->getGpuBufferForDraw();
        if (gpuBuffer == nil) {
            return;
        }
        uniformsToBind[index] = gpuBuffer;
        offsets[index] = state.offset;
    });

    NSRange uniformRange = NSMakeRange(0, Program::UNIFORM_BINDING_COUNT);
    [mContext->currentCommandEncoder setVertexBuffers:uniformsToBind
                                              offsets:offsets
                                            withRange:uniformRange];
    [mContext->currentCommandEncoder setFragmentBuffers:uniformsToBind
                                                offsets:offsets
                                              withRange:uniformRange];

    enumerateBoundUniformBuffers([commandEncoder = mContext->currentCommandEncoder](
            const UniformBufferState& state, const MetalUniformBuffer* uniform, uint32_t index) {
        void* cpuBuffer = uniform->getCpuBuffer();
        if (!cpuBuffer) {
            return;
        }
        uint8_t* bytes = static_cast<uint8_t*>(cpuBuffer) + state.offset;
        [commandEncoder setVertexBytes:bytes
                                length:(uniform->getSize() - state.offset)
                               atIndex:index];
        [commandEncoder setFragmentBytes:bytes
                                  length:(uniform->getSize() - state.offset)
                                 atIndex:index];
    });

    // Enumerate all the sampler buffers for the program and check which textures and samplers need
    // to be bound.

    id<MTLTexture> texturesToBind[SAMPLER_BINDING_COUNT] = {};
    id<MTLSamplerState> samplersToBind[SAMPLER_BINDING_COUNT] = {};

    enumerateSamplerGroups(program, [this, &texturesToBind, &samplersToBind](
            const SamplerGroup::Sampler* sampler,
            uint8_t binding) {
        const auto metalTexture = handle_const_cast<MetalTexture>(mHandleMap, sampler->t);
        texturesToBind[binding] = metalTexture->texture;

        if (metalTexture->externalImage.isValid()) {
            texturesToBind[binding] = metalTexture->externalImage.getMetalTextureForDraw();
        }

        if (!texturesToBind[binding]) {
            utils::slog.w << "Warning: no texture bound at binding point " << (size_t) binding
                    << "." << utils::io::endl;
            texturesToBind[binding] = getOrCreateEmptyTexture(mContext);
        }

        id <MTLSamplerState> samplerState = mContext->samplerStateCache.getOrCreateState(sampler->s);
        samplersToBind[binding] = samplerState;
    });

    // Assign a default sampler to empty slots, in case Filament hasn't bound all samplers.
    // Metal requires all samplers referenced in shaders to be bound.
    for (auto& sampler : samplersToBind) {
        if (!sampler) {
            sampler = mContext->samplerStateCache.getOrCreateState({});
        }
    }

    // Similar to uniforms, we can't tell which stage will use the textures / samplers, so bind
    // to both the vertex and fragment stages.

    NSRange samplerRange = NSMakeRange(0, SAMPLER_BINDING_COUNT);
    [mContext->currentCommandEncoder setFragmentTextures:texturesToBind
                                             withRange:samplerRange];
    [mContext->currentCommandEncoder setVertexTextures:texturesToBind
                                           withRange:samplerRange];
    [mContext->currentCommandEncoder setFragmentSamplerStates:samplersToBind
                                                  withRange:samplerRange];
    [mContext->currentCommandEncoder setVertexSamplerStates:samplersToBind
                                                withRange:samplerRange];

    // Bind the vertex buffers.
    NSRange bufferRange = NSMakeRange(VERTEX_BUFFER_START, primitive->buffers.size());
    [mContext->currentCommandEncoder setVertexBuffers:primitive->buffers.data()
                                            offsets:primitive->offsets.data()
                                          withRange:bufferRange];

    // Bind the zero buffer, used for missing vertex attributes.
    static const char bytes[16] = { 0 };
    [mContext->currentCommandEncoder setVertexBytes:bytes
                                             length:16
                                            atIndex:(VERTEX_BUFFER_START + ZERO_VERTEX_BUFFER)];

    MetalIndexBuffer* indexBuffer = primitive->indexBuffer;

    [mContext->currentCommandEncoder drawIndexedPrimitives:getMetalPrimitiveType(primitive->type)
                                              indexCount:primitive->count
                                               indexType:getIndexType(indexBuffer->elementSize)
                                             indexBuffer:indexBuffer->buffer
                                       indexBufferOffset:primitive->offset];
}

void MetalDriver::enumerateSamplerGroups(
        const MetalProgram* program,
        const std::function<void(const SamplerGroup::Sampler*, size_t)>& f) {
    for (uint8_t samplerGroupIdx = 0; samplerGroupIdx < SAMPLER_GROUP_COUNT; samplerGroupIdx++) {
        const auto& samplerGroup = program->samplerGroupInfo[samplerGroupIdx];
        if (samplerGroup.empty()) {
            continue;
        }
        const auto* metalSamplerGroup = mContext->samplerBindings[samplerGroupIdx];
        if (!metalSamplerGroup) {
            utils::slog.w << "Program has non-empty samplerGroup (index " << samplerGroupIdx <<
                    ") but has not bound any samplers." << utils::io::endl;
            continue;
        }
        SamplerGroup* sb = metalSamplerGroup->sb.get();
        assert(sb->getSize() == samplerGroup.size());
        size_t samplerIdx = 0;
        for (const auto& sampler : samplerGroup) {
            size_t bindingPoint = sampler.binding;
            const SamplerGroup::Sampler* boundSampler = sb->getSamplers() + samplerIdx;
            samplerIdx++;

            if (!boundSampler->t) {
                continue;
            }

            f(boundSampler, bindingPoint);
        }
    }
}

void MetalDriver::enumerateBoundUniformBuffers(
        const std::function<void(const UniformBufferState&, MetalUniformBuffer*, uint32_t)>& f) {
    for (uint32_t i = 0; i < Program::UNIFORM_BINDING_COUNT; i++) {
        auto& thisUniform = mContext->uniformState[i];
        if (!thisUniform.bound) {
            continue;
        }
        auto* uniform = handle_cast<MetalUniformBuffer>(mHandleMap, thisUniform.ubh);
        f(thisUniform, uniform, i);
    }
}

} // namespace metal

// explicit instantiation of the Dispatcher
template class ConcreteDispatcher<metal::MetalDriver>;

} // namespace backend
} // namespace filament
