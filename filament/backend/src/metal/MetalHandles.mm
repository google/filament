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

#include "MetalHandles.h"

#include "MetalEnums.h"

#include "private/backend/BackendUtils.h"

#include <utils/Panic.h>
#include <utils/trap.h>

#include <math.h>

namespace filament {
namespace backend {
namespace metal {

static inline MTLTextureUsage getMetalTextureUsage(TextureUsage usage) {
    NSUInteger u = 0;
    if (any(usage & TextureUsage::COLOR_ATTACHMENT)) {
        u |= MTLTextureUsageRenderTarget;
    }
    if (any(usage & TextureUsage::DEPTH_ATTACHMENT)) {
        u |= MTLTextureUsageRenderTarget;
    }
    if (any(usage & TextureUsage::STENCIL_ATTACHMENT)) {
        u |= MTLTextureUsageRenderTarget;
    }

    // All textures can be blitted from, so they must have the UsageShaderRead flag.
    u |= MTLTextureUsageShaderRead;

    return MTLTextureUsage(u);
}

MetalSwapChain::MetalSwapChain(id<MTLDevice> device, CAMetalLayer* nativeWindow)
        : layer(nativeWindow) {
    layer.device = device;
}

MetalSwapChain::MetalSwapChain(int32_t width, int32_t height) : headlessWidth(width),
        headlessHeight(height) { }

MetalVertexBuffer::MetalVertexBuffer(MetalContext& context, uint8_t bufferCount,
            uint8_t attributeCount, uint32_t vertexCount, AttributeArray const& attributes)
    : HwVertexBuffer(bufferCount, attributeCount, vertexCount, attributes) {
    buffers.reserve(bufferCount);

    for (uint8_t bufferIndex = 0; bufferIndex < bufferCount; ++bufferIndex) {
        // Calculate buffer size.
        uint32_t size = 0;
        for (auto const& item : attributes) {
            if (item.buffer == bufferIndex) {
                uint32_t end = item.offset + vertexCount * item.stride;
                size = std::max(size, end);
            }
        }

        MetalBuffer* buffer = nullptr;
        if (size > 0) {
            buffer = new MetalBuffer(context, size);
        }
        buffers.push_back(buffer);
    }
}

MetalVertexBuffer::~MetalVertexBuffer() {
    for (auto* b : buffers) {
        delete b;
    }
    buffers.clear();
}

MetalIndexBuffer::MetalIndexBuffer(MetalContext& context, uint8_t elementSize, uint32_t indexCount)
    : HwIndexBuffer(elementSize, indexCount), buffer(context, elementSize * indexCount, true) { }

MetalUniformBuffer::MetalUniformBuffer(MetalContext& context, size_t size) : HwUniformBuffer(),
        buffer(context, size) { }

void MetalRenderPrimitive::setBuffers(MetalVertexBuffer* vertexBuffer, MetalIndexBuffer*
        indexBuffer, uint32_t enabledAttributes) {
    this->vertexBuffer = vertexBuffer;
    this->indexBuffer = indexBuffer;

    const size_t attributeCount = vertexBuffer->attributes.size();

    buffers.clear();
    buffers.reserve(attributeCount);
    offsets.clear();
    offsets.reserve(attributeCount);

    // Each attribute gets its own vertex buffer.

    uint32_t bufferIndex = 0;
    for (uint32_t attributeIndex = 0; attributeIndex < attributeCount; attributeIndex++) {
        if (!(enabledAttributes & (1U << attributeIndex))) {
            const uint8_t flags = vertexBuffer->attributes[attributeIndex].flags;
            const MTLVertexFormat format = (flags & Attribute::FLAG_INTEGER_TARGET) ?
                    MTLVertexFormatUInt4 : MTLVertexFormatFloat4;

            // If the attribute is not enabled, bind it to the zero buffer. It's a Metal error for a
            // shader to read from missing vertex attributes.
            vertexDescription.attributes[attributeIndex] = {
                    .format = format,
                    .buffer = ZERO_VERTEX_BUFFER,
                    .offset = 0
            };
            vertexDescription.layouts[ZERO_VERTEX_BUFFER] = {
                    .step = MTLVertexStepFunctionConstant,
                    .stride = 16
            };
            continue;
        }
        const auto& attribute = vertexBuffer->attributes[attributeIndex];

        buffers.push_back(vertexBuffer->buffers[attribute.buffer]);
        offsets.push_back(attribute.offset);

        vertexDescription.attributes[attributeIndex] = {
                .format = getMetalFormat(attribute.type,
                                         attribute.flags & Attribute::FLAG_NORMALIZED),
                .buffer = bufferIndex,
                .offset = 0
        };
        vertexDescription.layouts[bufferIndex] = {
                .step = MTLVertexStepFunctionPerVertex,
                .stride = attribute.stride
        };

        bufferIndex++;
    };
}

MetalProgram::MetalProgram(id<MTLDevice> device, const Program& program) noexcept
    : HwProgram(program.getName()) {

    using MetalFunctionPtr = __strong id<MTLFunction>*;

    static_assert(Program::SHADER_TYPE_COUNT == 2, "Only vertex and fragment shaders expected.");
    MetalFunctionPtr shaderFunctions[2] = { &vertexFunction, &fragmentFunction };

    const auto& sources = program.getShadersSource();
    for (size_t i = 0; i < Program::SHADER_TYPE_COUNT; i++) {
        const auto& source = sources[i];
        // It's okay for some shaders to be empty, they shouldn't be used in any draw calls.
        if (source.empty()) {
            continue;
        }

        NSString* objcSource = [[NSString alloc] initWithBytes:source.data()
                                                        length:source.size()
                                                      encoding:NSUTF8StringEncoding];
        NSError* error = nil;
        MTLCompileOptions* options = [MTLCompileOptions new];
        options.languageVersion = MTLLanguageVersion1_1;
        id<MTLLibrary> library = [device newLibraryWithSource:objcSource
                                                      options:nil
                                                        error:&error];
        if (library == nil) {
            if (error) {
                auto description =
                        [error.localizedDescription cStringUsingEncoding:NSUTF8StringEncoding];
                utils::slog.w << description << utils::io::endl;
            }
            ASSERT_POSTCONDITION(false, "Unable to compile Metal shading library.");
        }

        *shaderFunctions[i] = [library newFunctionWithName:@"main0"];
    }

    samplerGroupInfo = program.getSamplerGroupInfo();
}

static MTLPixelFormat decidePixelFormat(id<MTLDevice> device, TextureFormat format) {
    const MTLPixelFormat metalFormat = getMetalFormat(format);
#if !defined(IOS)
    // Some devices do not support the Depth24_Stencil8 format, so we'll fallback to Depth32.
    if (metalFormat == MTLPixelFormatDepth24Unorm_Stencil8 &&
        !device.depth24Stencil8PixelFormatSupported) {
        return MTLPixelFormatDepth32Float;
    }
#endif
    return metalFormat;
}

MetalTexture::MetalTexture(MetalContext& context, SamplerType target, uint8_t levels,
        TextureFormat format, uint8_t samples, uint32_t width, uint32_t height, uint32_t depth,
        TextureUsage usage) noexcept
    : HwTexture(target, levels, samples, width, height, depth, format, usage), context(context),
        externalImage(context), reshaper(format) {

    // Metal does not natively support 3 component textures. We'll emulate support by reshaping the
    // image data and using a 4 component texture.
    const TextureFormat reshapedFormat = reshaper.getReshapedFormat();
    metalPixelFormat = decidePixelFormat(context.device, reshapedFormat);

    bytesPerElement = static_cast<uint8_t>(getFormatSize(reshapedFormat));
    assert(bytesPerElement > 0);
    blockWidth = static_cast<uint8_t>(getBlockWidth(reshapedFormat));
    blockHeight = static_cast<uint8_t>(getBlockHeight(reshapedFormat));

    ASSERT_POSTCONDITION(metalPixelFormat != MTLPixelFormatInvalid, "Pixel format not supported.");

    const BOOL mipmapped = levels > 1;
    const BOOL multisampled = samples > 1;
    const BOOL textureArray = target == SamplerType::SAMPLER_2D_ARRAY;

#if defined(IOS)
    ASSERT_PRECONDITION(!textureArray || !multisampled,
            "iOS does not support multisampled texture arrays.");
#endif

    const auto getTextureType = [](bool isArray, bool isMultisampled) {
        uint8_t value = 0;
        if (isMultisampled) {
            value |= 0b10u;
        }
        if (isArray) {
            value |= 0b01u;
        }
        switch (value) {
            default:
            case 0b00:
                return MTLTextureType2D;
            case 0b01:
                return MTLTextureType2DArray;
            case 0b10:
                return MTLTextureType2DMultisample;
            case 0b11:
#if !defined(IOS)
                return MTLTextureType2DMultisampleArray;
#else
                // should not get here
                return MTLTextureType2DArray;
#endif
        }
    };

    MTLTextureDescriptor* descriptor;
    switch (target) {
        case SamplerType::SAMPLER_2D:
        case SamplerType::SAMPLER_2D_ARRAY:
            descriptor = [MTLTextureDescriptor new];
            descriptor.pixelFormat = metalPixelFormat;
            descriptor.textureType = getTextureType(textureArray, multisampled);
            descriptor.width = width;
            descriptor.height = height;
            descriptor.arrayLength = depth;
            descriptor.mipmapLevelCount = levels;
            descriptor.sampleCount = multisampled ? samples : 1;
            descriptor.usage = getMetalTextureUsage(usage);
            descriptor.storageMode = MTLStorageModePrivate;
            texture = [context.device newTextureWithDescriptor:descriptor];
            ASSERT_POSTCONDITION(texture != nil, "Could not create Metal texture. Out of memory?");
            break;
        case SamplerType::SAMPLER_CUBEMAP:
            ASSERT_POSTCONDITION(!multisampled, "Multisampled cubemap faces not supported.");
            ASSERT_POSTCONDITION(width == height, "Cubemap faces must be square.");
            descriptor = [MTLTextureDescriptor textureCubeDescriptorWithPixelFormat:metalPixelFormat
                                                                               size:width
                                                                          mipmapped:mipmapped];
            descriptor.mipmapLevelCount = levels;
            descriptor.usage = getMetalTextureUsage(usage);
            descriptor.storageMode = MTLStorageModePrivate;
            texture = [context.device newTextureWithDescriptor:descriptor];
            ASSERT_POSTCONDITION(texture != nil, "Could not create Metal texture. Out of memory?");
            break;
        case SamplerType::SAMPLER_EXTERNAL:
            // If we're using external textures (CVPixelBufferRefs), we don't need to make any
            // texture allocations.
            texture = nil;
            break;
    }
}

MetalTexture::~MetalTexture() {
    externalImage.set(nullptr);
}

void MetalTexture::load2DImage(uint32_t level, uint32_t xoffset, uint32_t yoffset, uint32_t width,
        uint32_t height, PixelBufferDescriptor&& p) noexcept {
    PixelBufferDescriptor data = reshaper.reshape(std::move(p));

    id<MTLCommandBuffer> blitCommandBuffer = getPendingCommandBuffer(&context);
    id<MTLBlitCommandEncoder> blitCommandEncoder = [blitCommandBuffer blitCommandEncoder];

    loadSlice(level, xoffset, yoffset, width, height, 0, 0, data, blitCommandEncoder,
            blitCommandBuffer);

    [blitCommandEncoder endEncoding];
}

void MetalTexture::loadCubeImage(const FaceOffsets& faceOffsets, int miplevel,
        PixelBufferDescriptor&& p) {
    PixelBufferDescriptor data = reshaper.reshape(std::move(p));

    id<MTLCommandBuffer> blitCommandBuffer = getPendingCommandBuffer(&context);
    id<MTLBlitCommandEncoder> blitCommandEncoder = [blitCommandBuffer blitCommandEncoder];

    const NSUInteger faceWidth = width >> miplevel;

    for (NSUInteger slice = 0; slice < 6; slice++) {
        FaceOffsets::size_type faceOffset = faceOffsets.offsets[slice];
        loadSlice(miplevel, 0, 0, faceWidth, faceWidth, faceOffset, slice, data, blitCommandEncoder,
                blitCommandBuffer);
    }

    [blitCommandEncoder endEncoding];
}

void MetalTexture::loadSlice(uint32_t level, uint32_t xoffset, uint32_t yoffset, uint32_t width,
        uint32_t height, uint32_t byteOffset, uint32_t slice,
        PixelBufferDescriptor& data, id<MTLBlitCommandEncoder> blitCommandEncoder,
        id<MTLCommandBuffer> blitCommandBuffer) noexcept {
    const size_t stride = data.stride ? data.stride : width;
    size_t bytesPerRow = PixelBufferDescriptor::computeDataSize(data.format, data.type, stride, 1,
            data.alignment);
    size_t bytesPerPixel = PixelBufferDescriptor::computeDataSize(data.format, data.type, 1, 1, 1);
    size_t bytesPerSlice = bytesPerRow * height;    // a slice is a 2D image, or face of a cubemap

    const size_t sourceOffset = (data.left * bytesPerPixel) + (data.top * bytesPerRow) + byteOffset;

    if (data.type == PixelDataType::COMPRESSED) {
        assert(blockWidth > 0);
        assert(blockHeight > 0);
        // From https://developer.apple.com/documentation/metal/mtltexture/1515464-replaceregion:
        // For an ordinary or packed pixel format, the stride, in bytes, between rows of source
        // data. For a compressed pixel format, the stride is the number of bytes from the
        // beginning of one row of blocks to the beginning of the next.
        const NSUInteger blocksPerRow = std::ceil(width / (float) blockWidth);
        const NSUInteger blocksPerCol = std::ceil(height / (float) blockHeight);
        bytesPerRow = bytesPerElement * blocksPerRow;
        bytesPerSlice = bytesPerRow * blocksPerCol;
    }

    ASSERT_PRECONDITION(data.size >= bytesPerSlice, "Expected buffer size of at least %d but "
            "received PixelBufferDescriptor with size %d.", bytesPerSlice, data.size);

    // Depending on the size of the required buffer, we either allocate a staging buffer or a
    // staging texture. Then the texture data is blited to the "real" texture.
    const size_t stagingBufferSize = bytesPerSlice;
    if (UTILS_LIKELY(stagingBufferSize <= context.device.maxBufferLength)) {
        auto entry = context.bufferPool->acquireBuffer(stagingBufferSize);
        memcpy(entry->buffer.contents,
                static_cast<uint8_t*>(data.buffer) + sourceOffset,
                stagingBufferSize);
        [blitCommandEncoder copyFromBuffer:entry->buffer
                              sourceOffset:0
                         sourceBytesPerRow:bytesPerRow
                       sourceBytesPerImage:0
                                sourceSize:MTLSizeMake(width, height, 1)
                                 toTexture:texture
                          destinationSlice:slice
                          destinationLevel:level
                         destinationOrigin:MTLOriginMake(xoffset, yoffset, 0)];
        // We must ensure we only capture a pointer to bufferPool, not "this", as this texture could
        // be deallocated before the completion handler runs. The MetalBufferPool is guaranteed to
        // outlive the completion handler.
        MetalBufferPool* bufferPool = this->context.bufferPool;
        [blitCommandBuffer addCompletedHandler:^(id<MTLCommandBuffer> cb) {
            bufferPool->releaseBuffer(entry);
        }];
    } else {
       // The texture is too large to fit into a single buffer, create a staging texture instead.
       MTLTextureDescriptor* descriptor =
               [MTLTextureDescriptor texture2DDescriptorWithPixelFormat:metalPixelFormat
                                                                  width:width
                                                                 height:height
                                                              mipmapped:NO];
#if defined(IOS)
       descriptor.storageMode = MTLStorageModeShared;
#else
       descriptor.storageMode = MTLStorageModeManaged;
#endif
       id<MTLTexture> stagingTexture = [context.device newTextureWithDescriptor:descriptor];
       [stagingTexture replaceRegion:MTLRegionMake2D(0, 0, width, height)
                         mipmapLevel:0
                               slice:0
                           withBytes:static_cast<uint8_t*>(data.buffer) + sourceOffset
                         bytesPerRow:bytesPerRow
                       bytesPerImage:0];
       [blitCommandEncoder copyFromTexture:stagingTexture
                               sourceSlice:0
                               sourceLevel:0
                              sourceOrigin:MTLOriginMake(0, 0, 0)
                                sourceSize:MTLSizeMake(width, height, 1)
                                 toTexture:texture
                          destinationSlice:slice
                          destinationLevel:level
                         destinationOrigin:MTLOriginMake(xoffset, yoffset, 0)];
    }
}

MetalRenderTarget::MetalRenderTarget(MetalContext* context, uint32_t width, uint32_t height,
        uint8_t samples, id<MTLTexture> color, TargetInfo colorInfo, id<MTLTexture> depth,
        TargetInfo depthInfo) : HwRenderTarget(width, height), context(context), samples(samples),
        colorInfo(colorInfo), depthInfo(depthInfo) {
    ASSERT_PRECONDITION(color || depth, "Must provide either a color or depth texture.");

    if (color) {
        if (color.textureType == MTLTextureType2DMultisample) {
            this->multisampledColor = color;
        } else {
            this->color = color;
        }
    }

    if (depth) {
        if (depth.textureType == MTLTextureType2DMultisample) {
            this->multisampledDepth = depth;
        } else {
            this->depth = depth;
        }
    }

    ASSERT_PRECONDITION(samples > 1 || (!multisampledDepth && !multisampledColor),
            "MetalRenderTarget was initialized with a MSAA texture, but sample count is %d.", samples);

    // Handle special cases. If we were given a single-sampled texture but the samples parameter
    // is > 1, we create multisampled textures and do a resolve automatically.
    if (samples > 1) {
        if (color && !multisampledColor) {
            multisampledColor =
                    createMultisampledTexture(context->device, color.pixelFormat, width, height,
                            samples);
        }

        if (depth && !multisampledDepth) {
            // TODO: we only need to resolve depth if the depth texture is not SAMPLEABLE.
            multisampledDepth = createMultisampledTexture(context->device, depth.pixelFormat, width,
                    height, samples);
        }
    }
}

id<MTLTexture> MetalRenderTarget::getColor() {
    if (defaultRenderTarget) {
        return acquireDrawable(context);
    }
    if (multisampledColor) {
        return multisampledColor;
    }
    return color;
}

id<MTLTexture> MetalRenderTarget::getDepth() {
    if (defaultRenderTarget) {
        return acquireDepthTexture(context);
    }
    if (multisampledDepth) {
        return multisampledDepth;
    }
    return depth;
}

id<MTLTexture> MetalRenderTarget::getColorResolve() {
    const bool shouldResolveColor = (multisampledColor && color);
    return shouldResolveColor ? color : nil;
}

id<MTLTexture> MetalRenderTarget::getDepthResolve() {
    const bool shouldResolveDepth = (multisampledDepth && depth);
    return shouldResolveDepth ? depth : nil;
}

id<MTLTexture> MetalRenderTarget::getBlitColorSource() {
    if (defaultRenderTarget) {
        return acquireDrawable(context);
    }
    if (color) {
        return color;
    }
    return multisampledColor;
}

id<MTLTexture> MetalRenderTarget::getBlitDepthSource() {
    if (defaultRenderTarget) {
        return acquireDepthTexture(context);
    }
    if (depth) {
        return depth;
    }
    return multisampledDepth;
}

MTLLoadAction MetalRenderTarget::getLoadAction(const RenderPassParams& params,
        TargetBufferFlags buffer) {
    const auto clearFlags = (TargetBufferFlags) params.flags.clear;
    const auto discardStartFlags = params.flags.discardStart;
    if (any(clearFlags & buffer)) {
        return MTLLoadActionClear;
    } else if (any(discardStartFlags & buffer)) {
        return MTLLoadActionDontCare;
    }
    return MTLLoadActionLoad;
}

MTLStoreAction MetalRenderTarget::getStoreAction(const RenderPassParams& params,
        TargetBufferFlags buffer) {
    const auto discardEndFlags = params.flags.discardEnd;
    if (any(discardEndFlags & buffer)) {
        return MTLStoreActionDontCare;
    }
    if (any(buffer & TargetBufferFlags::COLOR)) {
        const bool shouldResolveColor = (multisampledColor && color);
        return shouldResolveColor ? MTLStoreActionMultisampleResolve : MTLStoreActionStore;
    }
    if (any(buffer & TargetBufferFlags::DEPTH)) {
        const bool shouldResolveDepth = (multisampledDepth && depth);
        return shouldResolveDepth ? MTLStoreActionMultisampleResolve : MTLStoreActionStore;
    }
    // Shouldn't get here.
    return MTLStoreActionStore;
}

id<MTLTexture> MetalRenderTarget::createMultisampledTexture(id<MTLDevice> device,
        MTLPixelFormat format, uint32_t width, uint32_t height, uint8_t samples) {
    MTLTextureDescriptor* descriptor =
            [MTLTextureDescriptor texture2DDescriptorWithPixelFormat:format
                                                               width:width
                                                              height:height
                                                            mipmapped:NO];
    descriptor.textureType = MTLTextureType2DMultisample;
    descriptor.sampleCount = samples;
    descriptor.usage = MTLTextureUsageRenderTarget;
#if defined(IOS)
    descriptor.resourceOptions = MTLResourceStorageModeMemoryless;
#else
    descriptor.resourceOptions = MTLResourceStorageModePrivate;
#endif

    return [device newTextureWithDescriptor:descriptor];
}

MetalFence::MetalFence(MetalContext& context) : context(context), value(context.signalId++) { }

void MetalFence::encode() {
    if (@available(macOS 10.14, iOS 12, *)) {
        event = [context.device newSharedEvent];
        [getPendingCommandBuffer(&context) encodeSignalEvent:event value:value];

        // Using a weak_ptr here because the Fence could be deleted before the block executes.
        std::weak_ptr<State> weakState = state;
        [event notifyListener:context.eventListener atValue:value block:^(id <MTLSharedEvent> o,
                uint64_t value) {
            if (auto s = weakState.lock()) {
                std::unique_lock<std::mutex> guard(s->mutex);
                s->status = FenceStatus::CONDITION_SATISFIED;
                s->cv.notify_all();
            }
        }];
    }
}

void MetalFence::onSignal(MetalFenceSignalBlock block) {
    [event notifyListener:context.eventListener atValue:value block:block];
}

FenceStatus MetalFence::wait(uint64_t timeoutNs) {
    if (@available(macOS 10.14, iOS 12, *)) {
        std::unique_lock<std::mutex> guard(state->mutex);
        while (state->status == FenceStatus::TIMEOUT_EXPIRED) {
            if (timeoutNs == 0 ||
                state->cv.wait_for(guard, std::chrono::nanoseconds(timeoutNs)) ==
                        std::cv_status::timeout) {
                return FenceStatus::TIMEOUT_EXPIRED;
            }
        }
        return FenceStatus::CONDITION_SATISFIED;
    }
    return FenceStatus::ERROR;
}

} // namespace metal
} // namespace backend
} // namespace filament
