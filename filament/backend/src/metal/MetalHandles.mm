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

#include <details/Texture.h> // for FTexture::getFormatSize

#include <utils/Panic.h>
#include <utils/trap.h>

namespace filament {
namespace backend {
namespace metal {

static inline MTLTextureUsage getMetalTextureUsage(TextureUsage usage) {
    switch (usage) {
        case TextureUsage::DEFAULT:
            return MTLTextureUsageShaderRead | MTLTextureUsageShaderWrite;

        case TextureUsage::COLOR_ATTACHMENT:
            return MTLTextureUsageShaderRead | MTLTextureUsageRenderTarget;

        case TextureUsage::DEPTH_ATTACHMENT:
        case TextureUsage::STENCIL_ATTACHMENT:
            return MTLTextureUsageShaderRead | MTLTextureUsageRenderTarget;
    }
}

static inline MTLStorageMode getMetalStorageMode(TextureFormat format) {
    switch (format) {
        // Depth textures must have a private storage mode.
        case TextureFormat::DEPTH16:
        case TextureFormat::DEPTH24:
        case TextureFormat::DEPTH32F:
        case TextureFormat::DEPTH24_STENCIL8:
        case TextureFormat::DEPTH32F_STENCIL8:
            return MTLStorageModePrivate;

        default:
#if defined(IOS)
            return MTLStorageModeShared;
#else
            return MTLStorageModeManaged;
#endif
    }
}

MetalSwapChain::MetalSwapChain(id<MTLDevice> device, CAMetalLayer* nativeWindow)
        : layer(nativeWindow) {
    layer.device = device;

    // Create a depth buffer for the swap chain.
    CGSize size = layer.drawableSize;
    MTLTextureDescriptor* depthTextureDesc =
            [MTLTextureDescriptor texture2DDescriptorWithPixelFormat:MTLPixelFormatDepth32Float
                                                               width:(NSUInteger)(size.width)
                                                              height:(NSUInteger)(size.height)
                                                           mipmapped:NO];
    depthTextureDesc.usage = MTLTextureUsageRenderTarget;
    depthTextureDesc.resourceOptions = MTLResourceStorageModePrivate;
    depthTexture = [device newTextureWithDescriptor:depthTextureDesc];
    surfaceHeight = (NSUInteger)(size.height);
}

MetalSwapChain::~MetalSwapChain() {
    [depthTexture release];
}

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

MetalVertexBuffer::~MetalVertexBuffer() {
    for (auto buffer : buffers) {
        [buffer release];
    }
}

MetalIndexBuffer::MetalIndexBuffer(id<MTLDevice> device, uint8_t elementSize, uint32_t indexCount)
    : HwIndexBuffer(elementSize, indexCount) {
    buffer = [device newBufferWithLength:(elementSize * indexCount)
                                 options:MTLResourceStorageModeShared];
}

MetalIndexBuffer::~MetalIndexBuffer() {
    [buffer release];
}

MetalUniformBuffer::MetalUniformBuffer(MetalContext& context, size_t size) : HwUniformBuffer(),
        size(size), context(context) {
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
    ASSERT_PRECONDITION(size <= this->size, "Attempting to copy %d bytes into a uniform of size %d",
            size, this->size);

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

    bufferPoolEntry = context.bufferPool->acquireBuffer(size);
    memcpy(static_cast<uint8_t*>(bufferPoolEntry->buffer.contents), src, size);
}

id<MTLBuffer> MetalUniformBuffer::getGpuBufferForDraw() {
    if (!bufferPoolEntry) {
        return nil;
    }

    // This uniform is being used in a draw call, so we retain it so it's not released back into the
    // buffer pool until the frame has finished.
    auto uniformDeleter = [bufferPool = context.bufferPool](const void* resource){
        bufferPool->releaseBuffer((const MetalBufferPoolEntry*) resource);
    };
    id<MTLCommandBuffer> commandBuffer = context.currentCommandBuffer;
    if (context.resourceTracker.trackResource(commandBuffer, bufferPoolEntry, uniformDeleter)) {
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
                .stride = attribute.stride
        };

        bufferIndex++;
    };
}

MetalProgram::MetalProgram(id<MTLDevice> device, const Program& program) noexcept
    : HwProgram(program.getName()) {

    using MetalFunctionPtr = id<MTLFunction>*;

    static_assert(Program::SHADER_TYPE_COUNT == 2, "Only vertex and fragment shaders expected.");
    MetalFunctionPtr shaderFunctions[2] = { &vertexFunction, &fragmentFunction };

    const auto& sources = program.getShadersSource();
    for (size_t i = 0; i < Program::SHADER_TYPE_COUNT; i++) {
        const auto& source = sources[i];
        // It's okay for some shaders to be empty, they shouldn't be used in any draw calls.
        if (source.empty()) {
            continue;
        }
        NSString* objcSource = [NSString stringWithCString:(const char*)source.data()
                                                  encoding:NSUTF8StringEncoding];
        NSError* error = nil;
        id<MTLLibrary> library = [device newLibraryWithSource:objcSource
                                                      options:nil
                                                        error:&error];
        if (error) {
            auto description =
                    [error.localizedDescription cStringUsingEncoding:NSUTF8StringEncoding];
            utils::slog.w << description << utils::io::endl;
        }
        ASSERT_POSTCONDITION(library != nil, "Unable to compile Metal shading library.");

        *shaderFunctions[i] = [library newFunctionWithName:@"main0"];

        [library release];
    }

    samplerGroupInfo = program.getSamplerGroupInfo();
}

MetalProgram::~MetalProgram() {
    [vertexFunction release];
    [fragmentFunction release];
}

MetalTexture::MetalTexture(MetalContext& context, backend::SamplerType target, uint8_t levels,
        TextureFormat format, uint8_t samples, uint32_t width, uint32_t height, uint32_t depth,
        TextureUsage usage) noexcept
    : HwTexture(target, levels, samples, width, height, depth, format), context(context),
        externalImage(context), reshaper(format) {

    // Metal does not natively support 3 component textures. We'll emulate support by reshaping the
    // image data and using a 4 component texture.
    const TextureFormat reshapedFormat = reshaper.getReshapedFormat();
    const MTLPixelFormat pixelFormat = getMetalFormat(reshapedFormat);

    bytesPerPixel = static_cast<uint8_t>(details::FTexture::getFormatSize(reshapedFormat));

    ASSERT_POSTCONDITION(pixelFormat != MTLPixelFormatInvalid, "Pixel format not supported.");

    const BOOL mipmapped = levels > 1;

    MTLTextureDescriptor* descriptor;
    if (target == backend::SamplerType::SAMPLER_2D) {
        descriptor = [MTLTextureDescriptor texture2DDescriptorWithPixelFormat:pixelFormat
                                                                        width:width
                                                                       height:height
                                                                    mipmapped:mipmapped];
        descriptor.mipmapLevelCount = levels;
        descriptor.textureType = MTLTextureType2D;
        descriptor.usage = getMetalTextureUsage(usage);
        descriptor.storageMode = getMetalStorageMode(format);
        texture = [context.device newTextureWithDescriptor:descriptor];
    } else if (target == backend::SamplerType::SAMPLER_CUBEMAP) {
        ASSERT_POSTCONDITION(width == height, "Cubemap faces must be square.");
        descriptor = [MTLTextureDescriptor textureCubeDescriptorWithPixelFormat:pixelFormat
                                                                           size:width
                                                                      mipmapped:mipmapped];
        descriptor.mipmapLevelCount = levels;
        descriptor.usage = getMetalTextureUsage(usage);
        descriptor.storageMode = getMetalStorageMode(format);
        texture = [context.device newTextureWithDescriptor:descriptor];
    } else if (target == backend::SamplerType::SAMPLER_EXTERNAL) {
        // If we're using external textures (CVPixelBufferRefs), we don't need to make any texture
        // allocations.
        texture = nil;
    } else {
        ASSERT_POSTCONDITION(false, "Sampler type not supported.");
    }

}

MetalTexture::~MetalTexture() {
    [texture release];
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
    NSUInteger bytesPerRow = bytesPerPixel * width;
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
    NSUInteger faceWidth = width >> miplevel;
    NSUInteger bytesPerRow = bytesPerPixel * faceWidth;
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

MetalRenderTarget::MetalRenderTarget(MetalContext* context, uint32_t width, uint32_t height,
        uint8_t samples, TextureFormat format, id<MTLTexture> color, id<MTLTexture> depth)
        : HwRenderTarget(width, height), context(context), color(color), depth(depth),
        samples(samples) {
    [color retain];
    [depth retain];

    if (samples > 1) {
        multisampledColor =
                createMultisampledTexture(context->device, format, width, height, samples);

        if (depth != nil) {
            multisampledDepth = createMultisampledTexture(context->device, TextureFormat::DEPTH32F,
                    width, height, samples);
        }
    }
}

id<MTLTexture> MetalRenderTarget::getColor() {
    if (defaultRenderTarget) {
        return acquireDrawable(context).texture;
    }
    return isMultisampled() ? multisampledColor : color;
}

id<MTLTexture> MetalRenderTarget::getColorResolve() {
    return isMultisampled() ? color : nil;
}

id<MTLTexture> MetalRenderTarget::getDepthResolve() {
    return isMultisampled() ? depth : nil;
}

id<MTLTexture> MetalRenderTarget::getDepth() {
    if (defaultRenderTarget) {
        return context->currentSurface->depthTexture;
    }
    return isMultisampled() ? multisampledDepth : depth;
}

MetalRenderTarget::~MetalRenderTarget() {
    [color release];
    [depth release];
    [multisampledColor release];
    [multisampledDepth release];
}

id<MTLTexture> MetalRenderTarget::createMultisampledTexture(id<MTLDevice> device,
        TextureFormat format, uint32_t width, uint32_t height, uint8_t samples) {
    MTLPixelFormat metalFormat = getMetalFormat(format);
    ASSERT_POSTCONDITION(metalFormat != MTLPixelFormatInvalid, "Pixel format not supported.");

    MTLTextureDescriptor* descriptor =
            [MTLTextureDescriptor texture2DDescriptorWithPixelFormat:metalFormat
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

} // namespace metal
} // namespace backend
} // namespace filament
