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

static inline MTLStorageMode getMetalStorageMode(TextureUsage usage) {
    if (any(usage & TextureUsage::UPLOADABLE)) {
#if defined(IOS)
        return MTLStorageModeShared;
#else
        return MTLStorageModeManaged;
#endif
    }
    return MTLStorageModePrivate;
}

MetalSwapChain::MetalSwapChain(id<MTLDevice> device, CAMetalLayer* nativeWindow)
        : layer(nativeWindow) {
    layer.device = device;
    CGSize size = layer.drawableSize;
    surfaceHeight = (NSUInteger) (size.height);
    surfaceWidth = (NSUInteger) (size.width);
}

MetalSwapChain::MetalSwapChain(int32_t width, int32_t height) : surfaceWidth(width),
        surfaceHeight(height) { }

MetalVertexBuffer::MetalVertexBuffer(id<MTLDevice> device, uint8_t bufferCount, uint8_t attributeCount,
            uint32_t vertexCount, AttributeArray const& attributes)
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

        id<MTLBuffer> buffer = nil;
        if (size > 0) {
            buffer = [device newBufferWithLength:size
                                         options:MTLResourceStorageModeShared];
        }
        buffers.push_back(buffer);
    }
}

MetalIndexBuffer::MetalIndexBuffer(id<MTLDevice> device, uint8_t elementSize, uint32_t indexCount)
    : HwIndexBuffer(elementSize, indexCount) {
    buffer = [device newBufferWithLength:(elementSize * indexCount)
                                 options:MTLResourceStorageModeShared];
}

MetalUniformBuffer::MetalUniformBuffer(MetalContext& context, size_t size) : HwUniformBuffer(),
        uniformSize(size), context(context) {
    ASSERT_PRECONDITION(size > 0, "Cannot create Metal uniform with size %d.", size);
    // If the buffer is less than 4K in size, we don't use an explicit buffer and instead use
    // immediate command encoder methods like setVertexBytes:length:atIndex:.
    if (size <= 4 * 1024) {   // 4K
        bufferPoolEntry = nullptr;
        cpuBuffer = malloc(size);
    }
}

MetalUniformBuffer::~MetalUniformBuffer() {
    if (cpuBuffer) {
        free(cpuBuffer);
    }
    // This uniform buffer is being destroyed. If we have a buffer, release it as it is no longer
    // needed.
    if (bufferPoolEntry) {
        context.bufferPool->releaseBuffer(bufferPoolEntry);
    }
}

void MetalUniformBuffer::copyIntoBuffer(void* src, size_t size) {
    if (size <= 0) {
        return;
    }
    ASSERT_PRECONDITION(size <= this->uniformSize, "Attempting to copy %d bytes into a uniform of size %d",
            size, this->uniformSize);

    // Either copy into the Metal buffer or into our cpu buffer.
    if (cpuBuffer) {
        memcpy(cpuBuffer, src, size);
        return;
    }

    // We're about to acquire a new buffer to hold the new contents of the uniform. If we previously
    // had obtained a buffer we release it, decrementing its reference count, as this uniform no
    // longer needs it.
    if (bufferPoolEntry) {
        context.bufferPool->releaseBuffer(bufferPoolEntry);
    }

    bufferPoolEntry = context.bufferPool->acquireBuffer(this->uniformSize);
    memcpy(static_cast<uint8_t*>(bufferPoolEntry->buffer.contents), src, size);
}

id<MTLBuffer> MetalUniformBuffer::getGpuBufferForDraw() {
    if (!bufferPoolEntry) {
        // If there's a CPU buffer, then we return nil here, as the CPU-side buffer will be bound
        // separately.
        if (cpuBuffer) {
            return nil;
        }

        // If there isn't a CPU buffer, it means no data has been loaded into this uniform yet. To
        // avoid an error, we'll allocate an empty buffer.
        bufferPoolEntry = context.bufferPool->acquireBuffer(this->uniformSize);
    }

    // This uniform is being used in a draw call, so we retain it so it's not released back into the
    // buffer pool until the frame has finished.
    auto uniformDeleter = [bufferPool = context.bufferPool](const void* resource){
        bufferPool->releaseBuffer((const MetalBufferPoolEntry*) resource);
    };
    id<MTLCommandBuffer> commandBuffer = context.currentCommandBuffer;
    if (context.resourceTracker.trackResource((__bridge void*) commandBuffer, bufferPoolEntry,
            uniformDeleter)) {
        // We only want to retain the buffer once per command buffer- trackResource will return
        // true if this is the first time tracking this uniform for this command buffer.
        context.bufferPool->retainBuffer(bufferPoolEntry);
    }

    return bufferPoolEntry->buffer;
}

void* MetalUniformBuffer::getCpuBuffer() const {
    return cpuBuffer;
}

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

MetalTexture::MetalTexture(MetalContext& context, backend::SamplerType target, uint8_t levels,
        TextureFormat format, uint8_t samples, uint32_t width, uint32_t height, uint32_t depth,
        TextureUsage usage) noexcept
    : HwTexture(target, levels, samples, width, height, depth, format, usage), context(context),
        externalImage(context), reshaper(format) {

    // Metal does not natively support 3 component textures. We'll emulate support by reshaping the
    // image data and using a 4 component texture.
    const TextureFormat reshapedFormat = reshaper.getReshapedFormat();
    const MTLPixelFormat pixelFormat = decidePixelFormat(context.device, reshapedFormat);

    bytesPerElement = static_cast<uint8_t>(getFormatSize(reshapedFormat));
    assert(bytesPerElement > 0);
    blockWidth = static_cast<uint8_t>(getBlockWidth(reshapedFormat));

    ASSERT_POSTCONDITION(pixelFormat != MTLPixelFormatInvalid, "Pixel format not supported.");

    const BOOL mipmapped = levels > 1;
    const BOOL multisampled = samples > 1;

    MTLTextureDescriptor* descriptor;
    if (target == backend::SamplerType::SAMPLER_2D) {
        descriptor = [MTLTextureDescriptor texture2DDescriptorWithPixelFormat:pixelFormat
                                                                        width:width
                                                                       height:height
                                                                    mipmapped:mipmapped];
        descriptor.mipmapLevelCount = levels;
        descriptor.textureType = multisampled ? MTLTextureType2DMultisample : MTLTextureType2D;
        descriptor.sampleCount = multisampled ? samples : 1;
        descriptor.usage = getMetalTextureUsage(usage);
        descriptor.storageMode = getMetalStorageMode(usage);
        texture = [context.device newTextureWithDescriptor:descriptor];
        ASSERT_POSTCONDITION(texture != nil, "Could not create Metal texture. Out of memory?");
    } else if (target == backend::SamplerType::SAMPLER_CUBEMAP) {
        ASSERT_POSTCONDITION(!multisampled, "Multisampled cubemap faces not supported.");
        ASSERT_POSTCONDITION(width == height, "Cubemap faces must be square.");
        descriptor = [MTLTextureDescriptor textureCubeDescriptorWithPixelFormat:pixelFormat
                                                                           size:width
                                                                      mipmapped:mipmapped];
        descriptor.mipmapLevelCount = levels;
        descriptor.usage = getMetalTextureUsage(usage);
        descriptor.storageMode = getMetalStorageMode(usage);
        texture = [context.device newTextureWithDescriptor:descriptor];
        ASSERT_POSTCONDITION(texture != nil, "Could not create Metal texture. Out of memory?");
    } else if (target == backend::SamplerType::SAMPLER_EXTERNAL) {
        // If we're using external textures (CVPixelBufferRefs), we don't need to make any texture
        // allocations.
        texture = nil;
    } else {
        ASSERT_POSTCONDITION(false, "Sampler type not supported.");
    }
}

MetalTexture::~MetalTexture() {
    externalImage.set(nullptr);
}

void MetalTexture::load2DImage(uint32_t level, uint32_t xoffset, uint32_t yoffset, uint32_t width,
        uint32_t height, PixelBufferDescriptor& data) noexcept {
    void* buffer = reshaper.reshape(data.buffer, data.size);

    MTLRegion region {
        .origin = {
            .x = xoffset,
            .y = yoffset,
            .z =  0
        },
        .size = {
            .height = height,
            .width = width,
            .depth = 1
        }
    };
    const NSUInteger bytesPerRow = getBytesPerRow(data.type, width);
    [texture replaceRegion:region
               mipmapLevel:level
                     slice:0
                 withBytes:buffer
               bytesPerRow:bytesPerRow
             bytesPerImage:0];          // only needed for MTLTextureType3D

    reshaper.freeBuffer(buffer);
}

void MetalTexture::loadCubeImage(const PixelBufferDescriptor& data, const FaceOffsets& faceOffsets,
        int miplevel) {
    const NSUInteger faceWidth = width >> miplevel;
    const NSUInteger bytesPerRow = getBytesPerRow(data.type, faceWidth);

    MTLRegion region = MTLRegionMake2D(0, 0, faceWidth, faceWidth);
    for (NSUInteger slice = 0; slice < 6; slice++) {
        FaceOffsets::size_type faceOffset = faceOffsets.offsets[slice];
        [texture replaceRegion:region
                   mipmapLevel:static_cast<NSUInteger>(miplevel)
                         slice:slice
                     withBytes:static_cast<uint8_t*>(data.buffer) + faceOffset
                   bytesPerRow:bytesPerRow
                 bytesPerImage:0];
    }
}

NSUInteger MetalTexture::getBytesPerRow(PixelDataType type, NSUInteger width) const noexcept {
    // From https://developer.apple.com/documentation/metal/mtltexture/1515464-replaceregion:
    // For an ordinary or packed pixel format, the stride, in bytes, between rows of source data.
    // For a compressed pixel format, the stride is the number of bytes from the beginning of one
    // row of blocks to the beginning of the next.
    if (type == PixelDataType::COMPRESSED) {
        assert(blockWidth > 0);
        const NSUInteger blocksPerRow = std::ceil(width / (float) blockWidth);
        return bytesPerElement * blocksPerRow;
    } else {
        return bytesPerElement * width;
    }
}

MetalRenderTarget::MetalRenderTarget(MetalContext* context, uint32_t width, uint32_t height,
        uint8_t samples, id<MTLTexture> color, id<MTLTexture> depth, uint8_t colorLevel,
        uint8_t depthLevel) : HwRenderTarget(width, height), context(context), samples(samples),
        colorLevel(colorLevel), depthLevel(depthLevel) {
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
    if (color) {
        return color;
    }
    return multisampledColor;
}

id<MTLTexture> MetalRenderTarget::getBlitDepthSource() {
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


MetalFence::MetalFence(MetalContext& context) {
#if METAL_FENCES_SUPPORTED
    cv = std::make_shared<std::condition_variable>();
    event = [context.device newSharedEvent];
    value = context.signalId++;
    [context.currentCommandBuffer encodeSignalEvent:event value:value];

    // Using a weak_ptr here because the Fence could be deleted before the block executes.
    std::weak_ptr<std::condition_variable> weakCv = cv;
    [event notifyListener:context.eventListener atValue:value block:^(id <MTLSharedEvent> o,
            uint64_t value) {
        if (auto cv = weakCv.lock()) {
            cv->notify_all();
        }
    }];
#endif
}

FenceStatus MetalFence::wait(uint64_t timeoutNs) {
#if METAL_FENCES_SUPPORTED
    std::unique_lock<std::mutex> guard(mutex);
    while (event.signaledValue != value) {
        if (timeoutNs == 0 ||
            cv->wait_for(guard, std::chrono::nanoseconds(timeoutNs)) == std::cv_status::timeout) {
            return FenceStatus::TIMEOUT_EXPIRED;
        }
    }
    return FenceStatus::CONDITION_SATISFIED;
#else
    return FenceStatus::ERROR;
#endif
}

} // namespace metal
} // namespace backend
} // namespace filament
