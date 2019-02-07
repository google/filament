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

namespace filament {
namespace driver {
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
            uint32_t vertexCount, Driver::AttributeArray const& attributes)
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

        id<MTLBuffer> buffer = [device newBufferWithLength:size
                                                   options:MTLResourceStorageModeShared];
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
};

MetalUniformBuffer::MetalUniformBuffer(id<MTLDevice> device, size_t size) : HwUniformBuffer(),
        size(size) {
    // If the buffer is less than 4K in size, we don't use an explicit buffer and instead use
    // immediate command encoder methods like setVertexBytes:length:atIndex:.
    if (size <= 4 * 1024) {   // 4K
        buffer = nil;
        cpuBuffer = malloc(size);
    } else {
        buffer = [device newBufferWithLength:size
                                     options:MTLResourceStorageModeShared];
        cpuBuffer = nullptr;
    }
}

MetalUniformBuffer::~MetalUniformBuffer() {
    if (buffer) {
        [buffer release];
    } else if (cpuBuffer) {
        free(cpuBuffer);
    }
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
                                         attribute.flags & Driver::Attribute::FLAG_NORMALIZED),
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

    static_assert(Program::NUM_SHADER_TYPES == 2, "Only vertex and fragment shaders expected.");
    MetalFunctionPtr shaderFunctions[2] = { &vertexFunction, &fragmentFunction };

    const auto& sources = program.getShadersSource();
    for (size_t i = 0; i < Program::NUM_SHADER_TYPES; i++) {
        const auto& source = sources[i];
        // It's okay for some shaders to be empty, they shouldn't be used in any draw calls.
        if (source.empty()) {
            continue;
        }
        NSString* objcSource = [NSString stringWithCString:source.c_str()
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

    samplerBindings = *program.getSamplerBindings();
}

MetalProgram::~MetalProgram() {
    [vertexFunction release];
    [fragmentFunction release];
}

MetalTexture::MetalTexture(id<MTLDevice> device, driver::SamplerType target, uint8_t levels,
        TextureFormat format, uint8_t samples, uint32_t width, uint32_t height, uint32_t depth,
        TextureUsage usage) noexcept
    : HwTexture(target, levels, samples, width, height, depth, format) {

    MTLPixelFormat pixelFormat = getMetalFormat(format);
    bytesPerPixel = static_cast<uint8_t>(details::FTexture::getFormatSize(format));

    ASSERT_POSTCONDITION(pixelFormat != MTLPixelFormatInvalid, "Pixel format not supported.");

    const BOOL mipmapped = levels > 1;

    MTLTextureDescriptor* descriptor;
    if (target == driver::SamplerType::SAMPLER_2D) {
        descriptor = [MTLTextureDescriptor texture2DDescriptorWithPixelFormat:pixelFormat
                                                                        width:width
                                                                       height:height
                                                                    mipmapped:mipmapped];
        descriptor.mipmapLevelCount = levels;
    } else if (target == driver::SamplerType::SAMPLER_CUBEMAP) {
        ASSERT_POSTCONDITION(width == height, "Cubemap faces must be square.");
        descriptor = [MTLTextureDescriptor textureCubeDescriptorWithPixelFormat:pixelFormat
                                                                           size:width
                                                                      mipmapped:mipmapped];
        descriptor.mipmapLevelCount = levels;
    } else {
        ASSERT_POSTCONDITION(false, "Sampler type not supported.");
    }

    descriptor.usage = getMetalTextureUsage(usage);
    descriptor.storageMode = getMetalStorageMode(format);

    texture = [device newTextureWithDescriptor:descriptor];
}

MetalTexture::~MetalTexture() {
    [texture release];
}

void MetalTexture::load2DImage(uint32_t level, uint32_t xoffset, uint32_t yoffset, uint32_t width,
        uint32_t height, Driver::PixelBufferDescriptor& data) noexcept {
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
                 withBytes:data.buffer
               bytesPerRow:bytesPerRow
             bytesPerImage:0];          // only needed for MTLTextureType3D
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

MetalRenderTarget::~MetalRenderTarget() {
    [color release];
    [depth release];
}

} // namespace metal
} // namespace driver
} // namespace filament
