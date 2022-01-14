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

#include "MetalBlitter.h"
#include "MetalEnums.h"
#include "MetalUtils.h"

#include <filament/SwapChain.h>

#include "private/backend/BackendUtils.h"

#include <utils/Panic.h>
#include <utils/trap.h>
#include <utils/debug.h>

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
        : context(context), layer(nativeWindow), externalImage(context),
        type(SwapChainType::CAMETALLAYER) {

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
        : context(context), headlessWidth(width), headlessHeight(height), externalImage(context),
        type(SwapChainType::HEADLESS) { }

MetalSwapChain::MetalSwapChain(MetalContext& context, CVPixelBufferRef pixelBuffer, uint64_t flags)
        : context(context), externalImage(context), type(SwapChainType::CVPIXELBUFFERREF) {
    assert_invariant(flags & backend::SWAP_CHAIN_CONFIG_APPLE_CVPIXELBUFFER);
    MetalExternalImage::assertWritableImage(pixelBuffer);
    externalImage.set(pixelBuffer);
    assert_invariant(externalImage.isValid());
}

MetalSwapChain::~MetalSwapChain() {
    externalImage.set(nullptr);
}

NSUInteger MetalSwapChain::getSurfaceWidth() const {
    if (isHeadless()) {
        return headlessWidth;
    }
    if (isPixelBuffer()) {
        return externalImage.getWidth();
    }
    return (NSUInteger) layer.drawableSize.width;
}

NSUInteger MetalSwapChain::getSurfaceHeight() const {
    if (isHeadless()) {
        return headlessHeight;
    }
    if (isPixelBuffer()) {
        return externalImage.getHeight();
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

    if (isPixelBuffer()) {
        return externalImage.getMetalTextureForDraw();
    }

    assert_invariant(isCaMetalLayer());
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

void MetalSwapChain::setFrameScheduledCallback(FrameScheduledCallback callback, void* user) {
    frameScheduledCallback = callback;
    frameScheduledUserData = user;
}

void MetalSwapChain::setFrameCompletedCallback(FrameCompletedCallback callback, void* user) {
    frameCompletedCallback = callback;
    frameCompletedUserData = user;
}

void MetalSwapChain::present() {
    if (frameCompletedCallback) {
        scheduleFrameCompletedCallback();
    }
    if (drawable) {
        if (frameScheduledCallback) {
            scheduleFrameScheduledCallback();
        } else  {
            [getPendingCommandBuffer(&context) presentDrawable:drawable];
        }
    }
}

void presentDrawable(bool presentFrame, void* user) {
    // CFBridgingRelease here is used to balance the CFBridgingRetain inside of acquireDrawable.
    id<CAMetalDrawable> drawable = (id<CAMetalDrawable>) CFBridgingRelease(user);
    if (presentFrame) {
        [drawable present];
    }
    // The drawable will be released here when the "drawable" variable goes out of scope.
}

void MetalSwapChain::scheduleFrameScheduledCallback() {
    if (!frameScheduledCallback) {
        return;
    }

    assert_invariant(drawable);
    backend::FrameScheduledCallback callback = frameScheduledCallback;
    // This block strongly captures drawable to keep it alive until the handler executes.
    // We cannot simply reference this->drawable inside the block because the block would then only
    // capture the _this_ pointer (MetalSwapChain*) instead of the drawable.
    id<CAMetalDrawable> d = drawable;
    void* userData = frameScheduledUserData;
    [getPendingCommandBuffer(&context) addScheduledHandler:^(id<MTLCommandBuffer> cb) {
        // CFBridgingRetain is used here to give the drawable a +1 retain count before
        // casting it to a void*.
        PresentCallable callable(presentDrawable, (void*) CFBridgingRetain(d));
        dispatch_async(dispatch_get_global_queue(DISPATCH_QUEUE_PRIORITY_DEFAULT, 0), ^{
            callback(callable, userData);
        });
    }];
}

void MetalSwapChain::scheduleFrameCompletedCallback() {
    if (!frameCompletedCallback) {
        return;
    }

    backend::FrameCompletedCallback callback = frameCompletedCallback;
    void* userData = frameCompletedUserData;
    [getPendingCommandBuffer(&context) addCompletedHandler:^(id<MTLCommandBuffer> cb) {
        struct CallbackData {
            void* userData;
            backend::FrameCompletedCallback callback;
        };
        CallbackData* data = new CallbackData();
        data->userData = userData;
        data->callback = callback;

        // Instantiate a BufferDescriptor with a callback for the sole purpose of passing it to
        // scheduleDestroy. This forces the BufferDescriptor callback (and thus the
        // FrameCompletedCallback) to be called on the user thread.
        BufferDescriptor b(nullptr, 0u, [](void* buffer, size_t size, void* user) {
            CallbackData* data = (CallbackData*) user;
            data->callback(data->userData);
            free(data);
        }, data);
        context.driver->scheduleDestroy(std::move(b));
    }];
}

MetalBufferObject::MetalBufferObject(MetalContext& context, BufferUsage usage, uint32_t byteCount)
        : HwBufferObject(byteCount), buffer(context, usage, byteCount) {}

void MetalBufferObject::updateBuffer(void* data, size_t size, uint32_t byteOffset) {
    buffer.copyIntoBuffer(data, size, byteOffset);
}

MetalVertexBuffer::MetalVertexBuffer(MetalContext& context, uint8_t bufferCount,
            uint8_t attributeCount, uint32_t vertexCount, AttributeArray const& attributes)
    : HwVertexBuffer(bufferCount, attributeCount, vertexCount, attributes), buffers(bufferCount, nullptr) {}

MetalIndexBuffer::MetalIndexBuffer(MetalContext& context, BufferUsage usage, uint8_t elementSize,
        uint32_t indexCount) : HwIndexBuffer(elementSize, indexCount),
        buffer(context, usage, elementSize * indexCount, true) { }

void MetalRenderPrimitive::setBuffers(MetalVertexBuffer* vertexBuffer, MetalIndexBuffer*
        indexBuffer) {
    this->vertexBuffer = vertexBuffer;
    this->indexBuffer = indexBuffer;

    const size_t attributeCount = vertexBuffer->attributes.size();

    vertexDescription = {};

    // Each attribute gets its own vertex buffer.

    uint32_t bufferIndex = 0;
    for (uint32_t attributeIndex = 0; attributeIndex < attributeCount; attributeIndex++) {
        const auto& attribute = vertexBuffer->attributes[attributeIndex];
        if (attribute.buffer == Attribute::BUFFER_UNUSED) {
            const uint8_t flags = attribute.flags;
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
        // When options is nil, Metal uses the most recent language version available.
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

MetalTexture::MetalTexture(MetalContext& context, SamplerType target, uint8_t levels,
        TextureFormat format, uint8_t samples, uint32_t width, uint32_t height, uint32_t depth,
        TextureUsage usage, TextureSwizzle r, TextureSwizzle g, TextureSwizzle b,
        TextureSwizzle a) noexcept
    : HwTexture(target, levels, samples, width, height, depth, format, usage), context(context),
        externalImage(context, r, g, b, a) {

    devicePixelFormat = decidePixelFormat(&context, format);
    ASSERT_POSTCONDITION(devicePixelFormat != MTLPixelFormatInvalid, "Texture format not supported.");

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
            descriptor.pixelFormat = devicePixelFormat;
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
            descriptor = [MTLTextureDescriptor textureCubeDescriptorWithPixelFormat:devicePixelFormat
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
            descriptor.pixelFormat = devicePixelFormat;
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

    // If swizzling is set, set up a swizzled texture view that we'll use when sampling this texture.
    const bool isDefaultSwizzle =
            r == TextureSwizzle::CHANNEL_0 &&
            g == TextureSwizzle::CHANNEL_1 &&
            b == TextureSwizzle::CHANNEL_2 &&
            a == TextureSwizzle::CHANNEL_3;
    // If texture is nil, then it must be a SAMPLER_EXTERNAL texture.
    // Swizzling for external textures is handled inside MetalExternalImage.
    if (!isDefaultSwizzle && texture && context.supportsTextureSwizzling) {
        // Even though we've already checked context.supportsTextureSwizzling, we still need to
        // guard these calls with @availability, otherwise the API usage will generate compiler
        // warnings.
        if (@available(iOS 13, *)) {
            swizzledTextureView =
                    createTextureViewWithSwizzle(texture, getSwizzleChannels(r, g, b, a));
        }
    }
}

MetalTexture::MetalTexture(MetalContext& context, SamplerType target, uint8_t levels, TextureFormat format,
        uint8_t samples, uint32_t width, uint32_t height, uint32_t depth, TextureUsage usage,
        id<MTLTexture> metalTexture) noexcept
    : HwTexture(target, levels, samples, width, height, depth, format, usage), context(context),
        externalImage(context) {
    texture = metalTexture;
    minLod = 0;
    maxLod = levels - 1;
}

MetalTexture::~MetalTexture() {
    externalImage.set(nullptr);
}


MTLPixelFormat MetalTexture::decidePixelFormat(MetalContext* context, TextureFormat format) {
    const MTLPixelFormat metalFormat = getMetalFormat(context, format);

    // If getMetalFormat can't find an exact match for the format, it returns MTLPixelFormatInvalid.
    if (metalFormat == MTLPixelFormatInvalid) {
        // These MTLPixelFormats are always supported.
        if (format == TextureFormat::DEPTH24_STENCIL8) return MTLPixelFormatDepth32Float_Stencil8;
        if (format == TextureFormat::DEPTH16) return MTLPixelFormatDepth32Float;
        if (format == TextureFormat::DEPTH24) {
            // DEPTH24 isn't supported at all by Metal. First try DEPTH24_STENCIL8. If that fails,
            // we'll fallback to DEPTH32F.
            MTLPixelFormat fallback = getMetalFormat(context, TextureFormat::DEPTH24_STENCIL8);
            if (fallback != MTLPixelFormatInvalid) return fallback;
            return MTLPixelFormatDepth32Float;
        }
    }

    // Metal does not natively support 3 component textures. We'll emulate support by using a 4
    // component texture and reshaping the pixel data during upload.
    switch (format) {
        case TextureFormat::RGB8: return MTLPixelFormatRGBA8Unorm;
        case TextureFormat::SRGB8: return MTLPixelFormatRGBA8Unorm_sRGB;
        case TextureFormat::RGB8_SNORM: return MTLPixelFormatRGBA8Snorm;
        case TextureFormat::RGB32F: return MTLPixelFormatRGBA32Float;
        case TextureFormat::RGB16F: return MTLPixelFormatRGBA16Float;
        case TextureFormat::RGB8UI: return MTLPixelFormatRGBA8Uint;
        case TextureFormat::RGB8I: return MTLPixelFormatRGBA8Sint;
        case TextureFormat::RGB16I: return MTLPixelFormatRGBA16Sint;
        case TextureFormat::RGB16UI: return MTLPixelFormatRGBA16Uint;
        case TextureFormat::RGB32UI: return MTLPixelFormatRGBA32Uint;
        case TextureFormat::RGB32I: return MTLPixelFormatRGBA32Sint;

        default: break;
    }

    return metalFormat;
}

PixelBufferShape PixelBufferShape::compute(const PixelBufferDescriptor& data,
        TextureFormat format, MTLSize size, uint32_t byteOffset) {
    PixelBufferShape result;

    auto width = size.width;
    auto height = size.height;
    auto depth = size.depth;

    const size_t stride = data.stride ? data.stride : width;
    result.bytesPerRow = PixelBufferDescriptor::computeDataSize(data.format, data.type, stride, 1,
            data.alignment);
    result.bytesPerPixel = PixelBufferDescriptor::computeDataSize(data.format, data.type, 1, 1, 1);
    result.bytesPerSlice = result.bytesPerRow * height;    // a slice is a 2D image, or face of a cubemap

    result.sourceOffset = (data.left * result.bytesPerPixel) + (data.top * result.bytesPerRow) + byteOffset;

    if (data.type == PixelDataType::COMPRESSED) {
        size_t blockWidth = getBlockWidth(format);   // number of horizontal pixels per block
        size_t blockHeight = getBlockHeight(format); // number of vertical pixels per block
        size_t bytesPerBlock = getFormatSize(format);
        assert_invariant(blockWidth > 0);
        assert_invariant(blockHeight > 0);
        assert_invariant(bytesPerBlock > 0);

        // From https://developer.apple.com/documentation/metal/mtltexture/1515464-replaceregion:
        // For an ordinary or packed pixel format, the stride, in bytes, between rows of source
        // data. For a compressed pixel format, the stride is the number of bytes from the
        // beginning of one row of blocks to the beginning of the next.
        const NSUInteger blocksPerRow = std::ceil(width / (float) blockWidth);
        const NSUInteger blocksPerCol = std::ceil(height / (float) blockHeight);

        result.bytesPerRow = bytesPerBlock * blocksPerRow;
        result.bytesPerSlice = result.bytesPerRow * blocksPerCol;
    }

    result.totalBytes = result.bytesPerSlice * depth;

    return result;
}

void MetalTexture::loadImage(uint32_t level, MTLRegion region, PixelBufferDescriptor& p) noexcept {
    PixelBufferDescriptor* data = &p;
    PixelBufferDescriptor reshapedData;
    if(reshape(p, reshapedData)) {
        data = &reshapedData;
    }

    switch (target) {
        case SamplerType::SAMPLER_2D:
        case SamplerType::SAMPLER_3D: {
            loadSlice(level, region, 0, 0, *data);
            break;
        }

        case SamplerType::SAMPLER_2D_ARRAY: {
            // Metal uses 'slice' (not z offset) to index into individual layers of a texture array.
            const uint32_t slice = region.origin.z;
            const uint32_t sliceCount = region.size.depth;
            region.origin.z = 0;
            region.size.depth = 1;

            const PixelBufferShape shape = PixelBufferShape::compute(*data, format, region.size, 0);

            uint32_t byteOffset = 0;
            for (uint32_t s = slice; s < slice + sliceCount; s++) {
                loadSlice(level, region, byteOffset, s, *data);
                byteOffset += shape.bytesPerSlice;
            }

            break;
        }

        case SamplerType::SAMPLER_CUBEMAP:
        case SamplerType::SAMPLER_EXTERNAL: {
            assert_invariant(false);
        }
    }

    updateLodRange(level);
}

void MetalTexture::loadCubeImage(const FaceOffsets& faceOffsets, int miplevel,
        PixelBufferDescriptor& p) {
    PixelBufferDescriptor* data = &p;
    PixelBufferDescriptor reshapedData;
    if(reshape(p, reshapedData)) {
        data = &reshapedData;
    }

    const NSUInteger faceWidth = width >> miplevel;

    for (NSUInteger slice = 0; slice < 6; slice++) {
        FaceOffsets::size_type faceOffset = faceOffsets.offsets[slice];
        loadSlice(miplevel, MTLRegionMake2D(0, 0, faceWidth, faceWidth), faceOffset, slice, *data);
    }

    updateLodRange((uint32_t) miplevel);
}

void MetalTexture::loadSlice(uint32_t level, MTLRegion region, uint32_t byteOffset, uint32_t slice,
        PixelBufferDescriptor& data) noexcept {
    const PixelBufferShape shape = PixelBufferShape::compute(data, format, region.size, byteOffset);

    ASSERT_PRECONDITION(data.size >= shape.totalBytes,
            "Expected buffer size of at least %d but "
            "received PixelBufferDescriptor with size %d.", shape.totalBytes, data.size);

    // Earlier versions of iOS don't have the maxBufferLength query, but 256 MB is a safe bet.
    NSUInteger deviceMaxBufferLength = 256 * 1024 * 1024;   // 256 MB
    if (@available(iOS 12, *)) {
        deviceMaxBufferLength = context.device.maxBufferLength;
    }

    // To upload the texture data, we either:
    // - allocate a staging buffer and perform a buffer copy
    // - allocate a staging texture and perform a texture blit
    // The buffer copy is preferred, but it cannot perform format conversions or handle large uploads.
    // The texture blit strategy does not have those limitations.

    MTLPixelFormat stagingPixelFormat = getMetalFormat(data.format, data.type);
    const bool conversionNecessary =
            stagingPixelFormat != getMetalFormatLinear(devicePixelFormat) &&
            data.type != PixelDataType::COMPRESSED;     // compressed formats should never need conversion

    const size_t stagingBufferSize = shape.totalBytes;
    const bool largeUpload = stagingBufferSize > deviceMaxBufferLength;

    if (conversionNecessary || largeUpload) {
        loadWithBlit(level, slice, region, data, shape);
    } else {
        loadWithCopyBuffer(level, slice, region, data, shape);
    }
}

void MetalTexture::loadWithCopyBuffer(uint32_t level, uint32_t slice, MTLRegion region,
        PixelBufferDescriptor& data, const PixelBufferShape& shape) {
    const size_t stagingBufferSize = shape.totalBytes;
    auto entry = context.bufferPool->acquireBuffer(stagingBufferSize);
    memcpy(entry->buffer.contents,
            static_cast<uint8_t*>(data.buffer) + shape.sourceOffset,
            stagingBufferSize);
    id<MTLCommandBuffer> blitCommandBuffer = getPendingCommandBuffer(&context);
    id<MTLBlitCommandEncoder> blitCommandEncoder = [blitCommandBuffer blitCommandEncoder];
    [blitCommandEncoder copyFromBuffer:entry->buffer
                          sourceOffset:0
                     sourceBytesPerRow:shape.bytesPerRow
                   sourceBytesPerImage:shape.bytesPerSlice
                            sourceSize:region.size
                             toTexture:texture
                      destinationSlice:slice
                      destinationLevel:level
                     destinationOrigin:region.origin];
    // We must ensure we only capture a pointer to bufferPool, not "this", as this texture could
    // be deallocated before the completion handler runs. The MetalBufferPool is guaranteed to
    // outlive the completion handler.
    MetalBufferPool* bufferPool = this->context.bufferPool;
    [blitCommandBuffer addCompletedHandler:^(id<MTLCommandBuffer> cb) {
        bufferPool->releaseBuffer(entry);
    }];
    [blitCommandEncoder endEncoding];
}

void MetalTexture::loadWithBlit(uint32_t level, uint32_t slice, MTLRegion region,
        PixelBufferDescriptor& data, const PixelBufferShape& shape) {
    MTLPixelFormat stagingPixelFormat = getMetalFormat(data.format, data.type);
    MTLTextureDescriptor* descriptor = [MTLTextureDescriptor new];
    descriptor.textureType = region.size.depth == 1 ? MTLTextureType2D : MTLTextureType3D;
    descriptor.pixelFormat = stagingPixelFormat;
    descriptor.width = region.size.width;
    descriptor.height = region.size.height;
    descriptor.depth = region.size.depth;

#if defined(IOS)
    descriptor.storageMode = MTLStorageModeShared;
#else
    descriptor.storageMode = MTLStorageModeManaged;
#endif

    id<MTLTexture> stagingTexture = [context.device newTextureWithDescriptor:descriptor];
    MTLRegion sourceRegion = MTLRegionMake3D(0, 0, 0,
            region.size.width, region.size.height, region.size.depth);
    [stagingTexture replaceRegion:sourceRegion
                      mipmapLevel:0
                            slice:0
                        withBytes:static_cast<uint8_t*>(data.buffer) + shape.sourceOffset
                      bytesPerRow:shape.bytesPerRow
                    bytesPerImage:shape.bytesPerSlice];

    // If we're blitting into an sRGB format, we need to create a linear view of the texture.
    // Otherwise, the blit will perform an unwanted sRGB conversion.
    id<MTLTexture> destinationTexture = texture;
    MTLPixelFormat linearFormat = getMetalFormatLinear(devicePixelFormat);
    if (linearFormat != devicePixelFormat) {
        NSUInteger slices = texture.arrayLength;
        if (texture.textureType == MTLTextureTypeCube ||
            texture.textureType == MTLTextureTypeCubeArray) {
            slices *= 6;
        }
        NSUInteger mips = texture.mipmapLevelCount;
        destinationTexture = [texture newTextureViewWithPixelFormat:linearFormat
                                                        textureType:texture.textureType
                                                             levels:NSMakeRange(0, mips)
                                                             slices:NSMakeRange(0, slices)];
    }

    MetalBlitter::BlitArgs args;
    args.filter = SamplerMagFilter::NEAREST;
    args.source.level = 0;
    args.source.slice = 0;
    args.source.region = sourceRegion;
    args.destination.level = level;
    args.destination.slice = slice;
    args.destination.region = region;
    args.source.color = stagingTexture;
    args.destination.color = destinationTexture;
    context.blitter->blit(getPendingCommandBuffer(&context), args);
}

void MetalTexture::updateLodRange(uint32_t level) {
    minLod = std::min(minLod, level);
    maxLod = std::max(maxLod, level);
}

MetalRenderTarget::MetalRenderTarget(MetalContext* context, uint32_t width, uint32_t height,
        uint8_t samples, Attachment colorAttachments[MRT::MAX_SUPPORTED_RENDER_TARGET_COUNT], Attachment depthAttachment) :
        HwRenderTarget(width, height), context(context), samples(samples) {
    for (size_t i = 0; i < MRT::MAX_SUPPORTED_RENDER_TARGET_COUNT; i++) {
        if (!colorAttachments[i]) {
            continue;
        }
        color[i] = colorAttachments[i];

        ASSERT_PRECONDITION(color[i].getSampleCount() <= samples,
                "MetalRenderTarget was initialized with a MSAA COLOR%d texture, but sample count is %d.",
                i, samples);

        // If we were given a single-sampled texture but the samples parameter is > 1, we create
        // a multisampled sidecar texture and do a resolve automatically.
        if (samples > 1 && color[i].getSampleCount() == 1) {
            auto& sidecar = color[i].metalTexture->msaaSidecar;
            if (!sidecar) {
                sidecar = createMultisampledTexture(context->device, color[i].getPixelFormat(),
                        color[i].metalTexture->width, color[i].metalTexture->height, samples);
            }
        }
    }

    if (depthAttachment) {
        depth = depthAttachment;

        ASSERT_PRECONDITION(depth.getSampleCount() <= samples,
                "MetalRenderTarget was initialized with a MSAA DEPTH texture, but sample count is %d.",
                samples);

        // If we were given a single-sampled texture but the samples parameter is > 1, we create
        // a multisampled sidecar texture and do a resolve automatically.
        if (samples > 1 && depth.getSampleCount() == 1) {
            auto& sidecar = depth.metalTexture->msaaSidecar;
            if (!sidecar) {
                sidecar = createMultisampledTexture(context->device, depth.getPixelFormat(),
                        depth.metalTexture->width, depth.metalTexture->height, samples);
            }
        }
    }
}

void MetalRenderTarget::setUpRenderPassAttachments(MTLRenderPassDescriptor* descriptor,
        const RenderPassParams& params) {

    const auto discardFlags = params.flags.discardEnd;

    for (size_t i = 0; i < MRT::MAX_SUPPORTED_RENDER_TARGET_COUNT; i++) {
        Attachment attachment = getDrawColorAttachment(i);
        if (!attachment) {
            continue;
        }

        descriptor.colorAttachments[i].texture = attachment.getTexture();
        descriptor.colorAttachments[i].level = attachment.level;
        descriptor.colorAttachments[i].slice = attachment.layer;
        descriptor.colorAttachments[i].loadAction = getLoadAction(params, getTargetBufferFlagsAt(i));
        descriptor.colorAttachments[i].storeAction = getStoreAction(params,
                getTargetBufferFlagsAt(i));
        descriptor.colorAttachments[i].clearColor = MTLClearColorMake(
                params.clearColor.r, params.clearColor.g, params.clearColor.b, params.clearColor.a);

        const bool automaticResolve = samples > 1 && attachment.getSampleCount() == 1;
        if (automaticResolve) {
            // We're rendering into our temporary MSAA texture and doing an automatic resolve.
            // We should not be attempting to load anything into the MSAA texture.
            assert_invariant(descriptor.colorAttachments[i].loadAction != MTLLoadActionLoad);
            assert_invariant(!defaultRenderTarget);

            id<MTLTexture> sidecar = attachment.getMSAASidecarTexture();
            assert_invariant(sidecar);

            descriptor.colorAttachments[i].texture = sidecar;
            descriptor.colorAttachments[i].level = 0;
            descriptor.colorAttachments[i].slice = 0;
            const bool discard = any(discardFlags & getTargetBufferFlagsAt(i));
            if (!discard) {
                descriptor.colorAttachments[i].resolveTexture = attachment.texture;
                descriptor.colorAttachments[i].resolveLevel = attachment.level;
                descriptor.colorAttachments[i].resolveSlice = attachment.layer;
                descriptor.colorAttachments[i].storeAction = MTLStoreActionMultisampleResolve;
            }
        }
    }

    Attachment depthAttachment = getDepthAttachment();
    descriptor.depthAttachment.texture = depthAttachment.getTexture();
    descriptor.depthAttachment.level = depthAttachment.level;
    descriptor.depthAttachment.slice = depthAttachment.layer;
    descriptor.depthAttachment.loadAction = getLoadAction(params, TargetBufferFlags::DEPTH);
    descriptor.depthAttachment.storeAction = getStoreAction(params, TargetBufferFlags::DEPTH);
    descriptor.depthAttachment.clearDepth = params.clearDepth;

    const bool automaticResolve = samples > 1 && depthAttachment.getSampleCount() == 1;
    if (automaticResolve) {
        // We're rendering into our temporary MSAA texture and doing an automatic resolve.
        // We should not be attempting to load anything into the MSAA texture.
        assert_invariant(descriptor.depthAttachment.loadAction != MTLLoadActionLoad);
        assert_invariant(!defaultRenderTarget);

        id<MTLTexture> sidecar = depthAttachment.getMSAASidecarTexture();
        assert_invariant(sidecar);

        descriptor.depthAttachment.texture = sidecar;
        descriptor.depthAttachment.level = 0;
        descriptor.depthAttachment.slice = 0;
        const bool discard = any(discardFlags & TargetBufferFlags::DEPTH);
        if (!discard) {
            descriptor.depthAttachment.resolveTexture = depthAttachment.getTexture();
            descriptor.depthAttachment.resolveLevel = depthAttachment.level;
            descriptor.depthAttachment.resolveSlice = depthAttachment.layer;
            descriptor.depthAttachment.storeAction = MTLStoreActionMultisampleResolve;
        }
    }
}

MetalRenderTarget::Attachment MetalRenderTarget::getDrawColorAttachment(size_t index) {
    assert_invariant(index < MRT::MAX_SUPPORTED_RENDER_TARGET_COUNT);
    Attachment result = color[index];
    if (index == 0 && defaultRenderTarget) {
        assert_invariant(context->currentDrawSwapChain);
        result.texture = context->currentDrawSwapChain->acquireDrawable();
    }
    return result;
}

MetalRenderTarget::Attachment MetalRenderTarget::getReadColorAttachment(size_t index) {
    assert_invariant(index < MRT::MAX_SUPPORTED_RENDER_TARGET_COUNT);
    Attachment result = color[index];
    if (index == 0 && defaultRenderTarget) {
        assert_invariant(context->currentReadSwapChain);
        result.texture = context->currentReadSwapChain->acquireDrawable();
    }
    return result;
}

MetalRenderTarget::Attachment MetalRenderTarget::getDepthAttachment() {
    Attachment result = depth;
    if (defaultRenderTarget) {
        result.texture = context->currentDrawSwapChain->acquireDepthTexture();
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
    descriptor.resourceOptions = MTLResourceStorageModePrivate;

    return [device newTextureWithDescriptor:descriptor];
}

MetalFence::MetalFence(MetalContext& context) : context(context), value(context.signalId++) { }

void MetalFence::encode() {
    if (@available(iOS 12, *)) {
        event = [context.device newSharedEvent];
        [getPendingCommandBuffer(&context) encodeSignalEvent:event value:value];

        // Using a weak_ptr here because the Fence could be deleted before the block executes.
        std::weak_ptr<State> weakState = state;
        [event notifyListener:context.eventListener atValue:value block:^(id <MTLSharedEvent> o,
                uint64_t value) {
            if (auto s = weakState.lock()) {
                std::lock_guard<std::mutex> guard(s->mutex);
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
    if (@available(iOS 12, *)) {
        using ns = std::chrono::nanoseconds;
        std::unique_lock<std::mutex> guard(state->mutex);
        while (state->status == FenceStatus::TIMEOUT_EXPIRED) {
            if (timeoutNs == FENCE_WAIT_FOR_EVER) {
                state->cv.wait(guard);
            } else if (timeoutNs == 0 ||
                    state->cv.wait_for(guard, ns(timeoutNs)) == std::cv_status::timeout) {
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
