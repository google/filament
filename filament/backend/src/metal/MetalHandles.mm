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

#include <filament/SwapChain.h>

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

MetalSwapChain::MetalSwapChain(MetalContext& context, CAMetalLayer* nativeWindow, uint64_t flags)
        : context(context), layer(nativeWindow), type(SwapChainType::CAMETALLAYER) {

    if (!(flags & SwapChain::CONFIG_TRANSPARENT) && !nativeWindow.opaque) {
        utils::slog.w << "Warning: Filament SwapChain has no CONFIG_TRANSPARENT flag, "
                         "but the CAMetaLayer(" << (__bridge void*) nativeWindow << ")"
                         " has .opaque set to NO." << utils::io::endl;
    }
    if ((flags & SwapChain::CONFIG_TRANSPARENT) && nativeWindow.opaque) {
        utils::slog.w << "Warning: Filament SwapChain has the CONFIG_TRANSPARENT flag, "
                         "but the CAMetaLayer(" << (__bridge void*) nativeWindow << ")"
                         " has .opaque set to YES." << utils::io::endl;
    }

    // Needed so we can use the SwapChain as a blit source.
    if (flags & SwapChain::CONFIG_READABLE) {
        nativeWindow.framebufferOnly = NO;
    }

    layer.device = context.device;
}

MetalSwapChain::MetalSwapChain(MetalContext& context, int32_t width, int32_t height, uint64_t flags)
        : context(context), headlessWidth(width), headlessHeight(height),
        type(SwapChainType::HEADLESS) { }


NSUInteger MetalSwapChain::getSurfaceWidth() const {
    if (isHeadless()) {
        return headlessWidth;
    }
    return (NSUInteger) layer.drawableSize.width;
}

NSUInteger MetalSwapChain::getSurfaceHeight() const {
    if (isHeadless()) {
        return headlessHeight;
    }
    return (NSUInteger) layer.drawableSize.height;
}

id<MTLTexture> MetalSwapChain::acquireDrawable() {
    if (drawable) {
        return drawable.texture;
    }

    if (isHeadless()) {
        if (headlessDrawable) {
            return headlessDrawable;
        }
        // For headless surfaces we construct a "fake" drawable, which is simply a renderable
        // texture.
        MTLTextureDescriptor* textureDescriptor = [MTLTextureDescriptor new];
        textureDescriptor.pixelFormat = MTLPixelFormatBGRA8Unorm;
        textureDescriptor.width = headlessWidth;
        textureDescriptor.height = headlessHeight;
        // Specify MTLTextureUsageShaderRead so the headless surface can be blitted from.
        textureDescriptor.usage = MTLTextureUsageRenderTarget | MTLTextureUsageShaderRead;
#if defined(IOS)
        textureDescriptor.storageMode = MTLStorageModeShared;
#else
        textureDescriptor.storageMode = MTLStorageModeManaged;
#endif
        headlessDrawable = [context.device newTextureWithDescriptor:textureDescriptor];
        return headlessDrawable;
    }

    assert(isCaMetalLayer());
    drawable = [layer nextDrawable];

    ASSERT_POSTCONDITION(drawable != nil, "Could not obtain drawable.");
    return drawable.texture;
}

void MetalSwapChain::releaseDrawable() {
    drawable = nil;
}

id<MTLTexture> MetalSwapChain::acquireDepthTexture() {
    if (depthTexture) {
        // If the surface size has changed, we'll need to allocate a new depth texture.
        if (depthTexture.width != getSurfaceWidth() ||
            depthTexture.height != getSurfaceHeight()) {
            depthTexture = nil;
        } else {
            return depthTexture;
        }
    }

    const MTLPixelFormat depthFormat =
#if defined(IOS)
            MTLPixelFormatDepth32Float;
#else
    context.device.depth24Stencil8PixelFormatSupported ?
            MTLPixelFormatDepth24Unorm_Stencil8 : MTLPixelFormatDepth32Float;
#endif

    const NSUInteger width = getSurfaceWidth();
    const NSUInteger height = getSurfaceHeight();
    MTLTextureDescriptor* descriptor =
            [MTLTextureDescriptor texture2DDescriptorWithPixelFormat:depthFormat
                                                               width:width
                                                              height:height
                                                           mipmapped:NO];
    descriptor.usage = MTLTextureUsageRenderTarget;
    descriptor.resourceOptions = MTLResourceStorageModePrivate;

    depthTexture = [context.device newTextureWithDescriptor:descriptor];

    return depthTexture;
}

void MetalSwapChain::setFrameFinishedCallback(FrameFinishedCallback callback, void* user) {
    frameFinishedCallback = callback;
    frameFinishedUserData = user;
}

void MetalSwapChain::present() {
    if (drawable) {
        if (frameFinishedCallback) {
            scheduleFrameFinishedCallback();
        } else  {
            [getPendingCommandBuffer(&context) presentDrawable:drawable];
        }
    }
}

MetalSwapChain::~MetalSwapChain() {
}

void presentDrawable(bool presentFrame, void* user) {
    // CFBridgingRelease here is used to balance the CFBridgingRetain inside of acquireDrawable.
    id<CAMetalDrawable> drawable = (id<CAMetalDrawable>) CFBridgingRelease(user);
    if (presentFrame) {
        [drawable present];
    }
    // The drawable will be released here when the "drawable" variable goes out of scope.
}

void MetalSwapChain::scheduleFrameFinishedCallback() {
    if (!frameFinishedCallback) {
        return;
    }

    assert(drawable);
    backend::FrameFinishedCallback callback = frameFinishedCallback;
    // This block strongly captures drawable to keep it alive until the handler executes.
    // We cannot simply reference this->drawable inside the block because the block would then only
    // capture the _this_ pointer (MetalSwapChain*) instead of the drawable.
    id<CAMetalDrawable> d = drawable;
    void* userData = frameFinishedUserData;
    [getPendingCommandBuffer(&context) addScheduledHandler:^(id<MTLCommandBuffer> cb) {
        // CFBridgingRetain is used here to give the drawable a +1 retain count before
        // casting it to a void*.
        PresentCallable callable(presentDrawable, (void*) CFBridgingRetain(d));
        dispatch_async(dispatch_get_global_queue(DISPATCH_QUEUE_PRIORITY_DEFAULT, 0), ^{
            callback(callable, userData);
        });
    }];
}

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
    : HwProgram(program.getName()), vertexFunction(nil), fragmentFunction(nil), samplerGroupInfo(),
        isValid(false) {

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
            PANIC_LOG("Failed to compile Metal program.");
            return;
        }

        *shaderFunctions[i] = [library newFunctionWithName:@"main0"];
    }

    // All stages of the program have compiled successfuly, this is a valid program.
    isValid = true;

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

    const auto get2DTextureType = [](bool isArray, bool isMultisampled) {
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
            descriptor.textureType = get2DTextureType(textureArray, multisampled);
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
        case SamplerType::SAMPLER_3D:
            descriptor = [MTLTextureDescriptor new];
            descriptor.pixelFormat = metalPixelFormat;
            descriptor.textureType = MTLTextureType3D;
            descriptor.width = width;
            descriptor.height = height;
            descriptor.depth = depth;
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

MetalTexture::MetalTexture(MetalContext& context, SamplerType target, uint8_t levels, TextureFormat format,
        uint8_t samples, uint32_t width, uint32_t height, uint32_t depth, TextureUsage usage,
        id<MTLTexture> metalTexture) noexcept
    : HwTexture(target, levels, samples, width, height, depth, format, usage), context(context),
        externalImage(context), reshaper(format) {
    texture = metalTexture;
    minLod = 0;
    maxLod = levels - 1;
}

MetalTexture::~MetalTexture() {
    externalImage.set(nullptr);
}

void MetalTexture::load2DImage(uint32_t level, uint32_t xoffset, uint32_t yoffset, uint32_t width,
        uint32_t height, PixelBufferDescriptor& p) noexcept {
    PixelBufferDescriptor* data = &p;
    PixelBufferDescriptor reshapedData;
    if (UTILS_UNLIKELY(reshaper.needsReshaping())) {
        reshapedData = reshaper.reshape(p);
        data = &reshapedData;
    }

    id<MTLCommandBuffer> blitCommandBuffer = getPendingCommandBuffer(&context);
    id<MTLBlitCommandEncoder> blitCommandEncoder = [blitCommandBuffer blitCommandEncoder];

    loadSlice(level, xoffset, yoffset, 0, width, height, 1, 0, 0, *data, blitCommandEncoder,
            blitCommandBuffer);

    updateLodRange(level);

    [blitCommandEncoder endEncoding];
}

void MetalTexture::load3DImage(uint32_t level, uint32_t xoffset, uint32_t yoffset, uint32_t zoffset,
        uint32_t width, uint32_t height, uint32_t depth, PixelBufferDescriptor& p) noexcept {
    // TODO: support texture reshaping for 3D textures.

    id<MTLCommandBuffer> blitCommandBuffer = getPendingCommandBuffer(&context);
    id<MTLBlitCommandEncoder> blitCommandEncoder = [blitCommandBuffer blitCommandEncoder];

    loadSlice(level, xoffset, yoffset, zoffset, width, height, depth, 0, 0, p,
            blitCommandEncoder, blitCommandBuffer);

    updateLodRange(level);

    [blitCommandEncoder endEncoding];
}

void MetalTexture::loadCubeImage(const FaceOffsets& faceOffsets, int miplevel,
        PixelBufferDescriptor& p) {
    PixelBufferDescriptor* data = &p;
    PixelBufferDescriptor reshapedData;
    if (UTILS_UNLIKELY(reshaper.needsReshaping())) {
        reshapedData = reshaper.reshape(p);
        data = &reshapedData;
    }

    id<MTLCommandBuffer> blitCommandBuffer = getPendingCommandBuffer(&context);
    id<MTLBlitCommandEncoder> blitCommandEncoder = [blitCommandBuffer blitCommandEncoder];

    const NSUInteger faceWidth = width >> miplevel;

    for (NSUInteger slice = 0; slice < 6; slice++) {
        FaceOffsets::size_type faceOffset = faceOffsets.offsets[slice];
        loadSlice(miplevel, 0, 0, 0, faceWidth, faceWidth, 1, faceOffset, slice, *data,
                blitCommandEncoder, blitCommandBuffer);
    }

    updateLodRange((uint32_t) miplevel);

    [blitCommandEncoder endEncoding];
}

void MetalTexture::loadSlice(uint32_t level, uint32_t xoffset, uint32_t yoffset, uint32_t zoffset,
        uint32_t width, uint32_t height, uint32_t depth, uint32_t byteOffset, uint32_t slice,
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
    const size_t stagingBufferSize = bytesPerSlice * depth;
    NSUInteger deviceMaxBufferLength = 0;
    if (@available(macOS 10.14, iOS 12, *)) {
        deviceMaxBufferLength = context.device.maxBufferLength;
    }
    if (UTILS_LIKELY(stagingBufferSize <= deviceMaxBufferLength)) {
        auto entry = context.bufferPool->acquireBuffer(stagingBufferSize);
        memcpy(entry->buffer.contents,
                static_cast<uint8_t*>(data.buffer) + sourceOffset,
                stagingBufferSize);
        [blitCommandEncoder copyFromBuffer:entry->buffer
                              sourceOffset:0
                         sourceBytesPerRow:bytesPerRow
                       sourceBytesPerImage:bytesPerSlice
                                sourceSize:MTLSizeMake(width, height, depth)
                                 toTexture:texture
                          destinationSlice:slice
                          destinationLevel:level
                         destinationOrigin:MTLOriginMake(xoffset, yoffset, zoffset)];
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
        if (depth > 1) {
            descriptor.textureType = MTLTextureType3D;
            descriptor.depth = depth;
        }
#if defined(IOS)
        descriptor.storageMode = MTLStorageModeShared;
#else
        descriptor.storageMode = MTLStorageModeManaged;
#endif
        id<MTLTexture> stagingTexture = [context.device newTextureWithDescriptor:descriptor];
        [stagingTexture replaceRegion:MTLRegionMake3D(0, 0, 0, width, height, depth)
                          mipmapLevel:0
                                slice:0
                            withBytes:static_cast<uint8_t*>(data.buffer) + sourceOffset
                          bytesPerRow:bytesPerRow
                        bytesPerImage:bytesPerSlice];
        [blitCommandEncoder copyFromTexture:stagingTexture
                                sourceSlice:0
                                sourceLevel:0
                               sourceOrigin:MTLOriginMake(0, 0, 0)
                                 sourceSize:MTLSizeMake(width, height, depth)
                                  toTexture:texture
                           destinationSlice:slice
                           destinationLevel:level
                          destinationOrigin:MTLOriginMake(xoffset, yoffset, zoffset)];
    }
}

void MetalTexture::updateLodRange(uint32_t level) {
    minLod = std::min(minLod, level);
    maxLod = std::max(maxLod, level);
}

MetalRenderTarget::MetalRenderTarget(MetalContext* context, uint32_t width, uint32_t height,
        uint8_t samples, Attachment colorAttachments[MRT::TARGET_COUNT], Attachment depthAttachment) :
        HwRenderTarget(width, height), context(context), samples(samples) {
    // If we were given a single-sampled texture but the samples parameter is > 1, we create
    // multisampled sidecar textures and do a resolve automatically.
    const bool msaaResolve = samples > 1;

    for (size_t i = 0; i < MRT::TARGET_COUNT; i++) {
        if (!colorAttachments[i]) {
            continue;
        }
        this->color[i] = colorAttachments[i];

        const auto textureSampleCount = this->color[i].texture.sampleCount;

        ASSERT_PRECONDITION(textureSampleCount <= samples,
                "MetalRenderTarget was initialized with a MSAA COLOR%d texture, but sample count is %d.",
                i, samples);

        if (msaaResolve && textureSampleCount == 1) {
            multisampledColor[i] =
                    createMultisampledTexture(context->device, color[0].texture.pixelFormat,
                            width, height, samples);
        }
    }

    if (depthAttachment) {
        this->depth = depthAttachment;

        const auto textureSampleCount = this->depth.texture.sampleCount;

        ASSERT_PRECONDITION(textureSampleCount <= samples,
                "MetalRenderTarget was initialized with a MSAA DEPTH texture, but sample count is %d.",
                samples);

        if (msaaResolve && this->depth.texture.sampleCount == 1) {
            // TODO: we only need to resolve depth if the depth texture is not SAMPLEABLE.
            multisampledDepth = createMultisampledTexture(context->device, depth.texture.pixelFormat,
                    width, height, samples);
        }
    }
}

void MetalRenderTarget::setUpRenderPassAttachments(MTLRenderPassDescriptor* descriptor,
        const RenderPassParams& params) {

    const auto discardFlags = params.flags.discardEnd;

    for (size_t i = 0; i < MRT::TARGET_COUNT; i++) {
        Attachment attachment = getColorAttachment(i);
        if (!attachment) {
            continue;
        }

        descriptor.colorAttachments[i].texture = attachment.texture;
        descriptor.colorAttachments[i].level = attachment.level;
        descriptor.colorAttachments[i].slice = attachment.layer;
        descriptor.colorAttachments[i].loadAction = getLoadAction(params, getMRTColorFlag(i));
        descriptor.colorAttachments[i].storeAction = getStoreAction(params, getMRTColorFlag(i));
        descriptor.colorAttachments[i].clearColor = MTLClearColorMake(
                params.clearColor.r, params.clearColor.g, params.clearColor.b, params.clearColor.a);

        if (multisampledColor[i]) {
            // We're rendering into our temporary MSAA texture and doing an automatic resolve.
            // We should not be attempting to load anything into the MSAA texture.
            assert(descriptor.colorAttachments[i].loadAction != MTLLoadActionLoad);

            descriptor.colorAttachments[i].texture = multisampledColor[i];
            descriptor.colorAttachments[i].level = 0;
            descriptor.colorAttachments[i].slice = 0;
            const bool discard = any(discardFlags & getMRTColorFlag(i));
            if (!discard) {
                descriptor.colorAttachments[i].resolveTexture = attachment.texture;
                descriptor.colorAttachments[i].resolveLevel = attachment.level;
                descriptor.colorAttachments[i].resolveSlice = attachment.layer;
                descriptor.colorAttachments[i].storeAction = MTLStoreActionMultisampleResolve;
            }
        }
    }

    Attachment depthAttachment = getDepthAttachment();
    descriptor.depthAttachment.texture = depthAttachment.texture;
    descriptor.depthAttachment.level = depthAttachment.level;
    descriptor.depthAttachment.slice = depthAttachment.layer;
    descriptor.depthAttachment.loadAction = getLoadAction(params, TargetBufferFlags::DEPTH);
    descriptor.depthAttachment.storeAction = getStoreAction(params, TargetBufferFlags::DEPTH);
    descriptor.depthAttachment.clearDepth = params.clearDepth;

    if (multisampledDepth) {
        // We're rendering into our temporary MSAA texture and doing an automatic resolve.
        // We should not be attempting to load anything into the MSAA texture.
        assert(descriptor.depthAttachment.loadAction != MTLLoadActionLoad);

        descriptor.depthAttachment.texture = multisampledDepth;
        descriptor.depthAttachment.level = 0;
        descriptor.depthAttachment.slice = 0;
        const bool discard = any(discardFlags & TargetBufferFlags::DEPTH);
        if (!discard) {
            descriptor.depthAttachment.resolveTexture = depthAttachment.texture;
            descriptor.depthAttachment.resolveLevel = depthAttachment.level;
            descriptor.depthAttachment.resolveSlice = depthAttachment.layer;
            descriptor.depthAttachment.storeAction = MTLStoreActionMultisampleResolve;
        }
    }
}

MetalRenderTarget::Attachment MetalRenderTarget::getColorAttachment(size_t index) {
    assert(index < MRT::TARGET_COUNT);
    Attachment result = color[index];
    if (index == 0 && defaultRenderTarget) {
        result.texture = context->currentSurface->acquireDrawable();
    }
    return result;
}

MetalRenderTarget::Attachment MetalRenderTarget::getDepthAttachment() {
    Attachment result = depth;
    if (defaultRenderTarget) {
        result.texture = context->currentSurface->acquireDepthTexture();
    }
    return result;
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
        timeoutNs = std::min(timeoutNs, (uint64_t) std::chrono::nanoseconds::max().count());
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
