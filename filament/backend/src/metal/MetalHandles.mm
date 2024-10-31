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
#include "MetalBufferPool.h"

#include <filament/SwapChain.h>

#include <backend/DriverEnums.h>

#include "private/backend/BackendUtils.h"

#include <utils/compiler.h>
#include <utils/Panic.h>
#include <utils/trap.h>
#include <utils/debug.h>

#include <math/scalar.h>

#include <math.h>

namespace filament {
namespace backend {

static inline MTLTextureUsage getMetalTextureUsage(TextureUsage usage) {
    NSUInteger u = MTLTextureUsageUnknown;

    if (any(usage & TextureUsage::SAMPLEABLE)) {
        u |= MTLTextureUsageShaderRead;
    }
    if (any(usage & TextureUsage::UPLOADABLE)) {
        // This is only needed because of the slowpath is MetalBlitter
        u |= MTLTextureUsageRenderTarget;
    }
    if (any(usage & TextureUsage::COLOR_ATTACHMENT)) {
        u |= MTLTextureUsageRenderTarget;
    }
    if (any(usage & TextureUsage::DEPTH_ATTACHMENT)) {
        u |= MTLTextureUsageRenderTarget;
    }
    if (any(usage & TextureUsage::STENCIL_ATTACHMENT)) {
        u |= MTLTextureUsageRenderTarget;
    }
    if (any(usage & TextureUsage::BLIT_DST)) {
        // This is only needed because of the slowpath is MetalBlitter
        u |= MTLTextureUsageRenderTarget;
    }
    if (any(usage & TextureUsage::BLIT_SRC)) {
        u |= MTLTextureUsageShaderRead;
    }

    return MTLTextureUsage(u);
}

MetalSwapChain::MetalSwapChain(MetalContext& context, CAMetalLayer* nativeWindow, uint64_t flags)
    : context(context),
      depthStencilFormat(decideDepthStencilFormat(flags)),
      layer(nativeWindow),
      layerDrawableMutex(std::make_shared<std::mutex>()),
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
    // Also needed for rendering directly into the SwapChain during refraction.
    nativeWindow.framebufferOnly = NO;

    layer.device = context.device;
}

MetalSwapChain::MetalSwapChain(MetalContext& context, int32_t width, int32_t height, uint64_t flags)
    : context(context),
      depthStencilFormat(decideDepthStencilFormat(flags)),
      headlessWidth(width),
      headlessHeight(height),
      type(SwapChainType::HEADLESS) {}

MetalSwapChain::MetalSwapChain(MetalContext& context, CVPixelBufferRef pixelBuffer, uint64_t flags)
    : context(context),
      depthStencilFormat(decideDepthStencilFormat(flags)),
      externalImage(MetalExternalImage::createFromImage(context, pixelBuffer)),
      type(SwapChainType::CVPIXELBUFFERREF) {
    assert_invariant(flags & SWAP_CHAIN_CONFIG_APPLE_CVPIXELBUFFER);
    MetalExternalImage::assertWritableImage(pixelBuffer);
    assert_invariant(externalImage.isValid());
}

MTLPixelFormat MetalSwapChain::decideDepthStencilFormat(uint64_t flags) {
    // These formats are supported on all devices, both iOS and macOS.
    return flags & SwapChain::CONFIG_HAS_STENCIL_BUFFER ? MTLPixelFormatDepth32Float_Stencil8
                                                        : MTLPixelFormatDepth32Float;
}

MetalSwapChain::~MetalSwapChain() {
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
        return externalImage.getMtlTexture();
    }

    assert_invariant(isCaMetalLayer());

    // CAMetalLayer's drawable pool is not thread safe. Use a mutex when
    // calling -nextDrawable, or when releasing the last known reference
    // to any CAMetalDrawable returned from a previous -nextDrawable.
    {
        std::lock_guard<std::mutex> lock(*layerDrawableMutex);
        drawable = [layer nextDrawable];
    }

    FILAMENT_CHECK_POSTCONDITION(drawable != nil) << "Could not obtain drawable.";
    return drawable.texture;
}

void MetalSwapChain::releaseDrawable() {
    if (drawable) {
        std::lock_guard<std::mutex> lock(*layerDrawableMutex);
        drawable = nil;
    }
}

id<MTLTexture> MetalSwapChain::acquireDepthTexture() {
    ensureDepthStencilTexture();
    assert_invariant(depthStencilTexture);
    return depthStencilTexture;
}

id<MTLTexture> MetalSwapChain::acquireStencilTexture() {
    if (!isMetalFormatStencil(depthStencilFormat)) {
        return nil;
    }
    ensureDepthStencilTexture();
    assert_invariant(depthStencilTexture);
    return depthStencilTexture;
}

void MetalSwapChain::ensureDepthStencilTexture() {
    NSUInteger width = getSurfaceWidth();
    NSUInteger height = getSurfaceHeight();
    if (UTILS_LIKELY(depthStencilTexture)) {
        // If the surface size has changed, we'll need to allocate a new depth/stencil texture.
        if (UTILS_UNLIKELY(
                    depthStencilTexture.width != width || depthStencilTexture.height != height)) {
            depthStencilTexture = nil;
        } else {
            return;
        }
    }
    MTLTextureDescriptor* descriptor =
            [MTLTextureDescriptor texture2DDescriptorWithPixelFormat:depthStencilFormat
                                                               width:width
                                                              height:height
                                                           mipmapped:NO];
    descriptor.usage = MTLTextureUsageRenderTarget;
    descriptor.resourceOptions = MTLResourceStorageModePrivate;
    depthStencilTexture = [context.device newTextureWithDescriptor:descriptor];
}

void MetalSwapChain::setFrameScheduledCallback(
        CallbackHandler* handler, FrameScheduledCallback&& callback, uint64_t flags) {
    frameScheduled.handler = handler;
    frameScheduled.callback = std::make_shared<FrameScheduledCallback>(std::move(callback));
    frameScheduled.flags = flags;
}

void MetalSwapChain::setFrameCompletedCallback(
        CallbackHandler* handler, utils::Invocable<void(void)>&& callback) {
    frameCompleted.handler = handler;
    frameCompleted.callback = std::make_shared<utils::Invocable<void(void)>>(std::move(callback));
}

void MetalSwapChain::present() {
    if (frameCompleted.callback) {
        scheduleFrameCompletedCallback();
    }
    if (drawable) {
        if (frameScheduled.callback) {
            scheduleFrameScheduledCallback();
        } else  {
            [getPendingCommandBuffer(&context) presentDrawable:drawable];
        }
    }
}

class PresentDrawableData {
public:
    PresentDrawableData() = delete;
    PresentDrawableData(const PresentDrawableData&) = delete;
    PresentDrawableData& operator=(const PresentDrawableData&) = delete;

    static PresentDrawableData* create(id<CAMetalDrawable> drawable,
            std::shared_ptr<std::mutex> drawableMutex, MetalDriver* driver, uint64_t flags) {
        assert_invariant(drawableMutex);
        assert_invariant(driver);
        return new PresentDrawableData(drawable, drawableMutex, driver, flags);
    }

    static void maybePresentAndDestroyAsync(PresentDrawableData* that, bool shouldPresent) {
        if (shouldPresent) {
           [that->mDrawable present];
        }

        if (that->mFlags & SwapChain::CALLBACK_DEFAULT_USE_METAL_COMPLETION_HANDLER) {
            cleanupAndDestroy(that);
        } else {
            // mDrawable is acquired on the driver thread. Typically, we would release this object
            // on the same thread, but after receiving consistent crash reports from within
            // [CAMetalDrawable dealloc], we suspect this object requires releasing on the main
            // thread.
            dispatch_async(dispatch_get_main_queue(), ^{
                cleanupAndDestroy(that);
            });
        }
    }

private:
    PresentDrawableData(id<CAMetalDrawable> drawable, std::shared_ptr<std::mutex> drawableMutex,
            MetalDriver* driver, uint64_t flags)
        : mDrawable(drawable), mDrawableMutex(drawableMutex), mDriver(driver), mFlags(flags) {}

    static void cleanupAndDestroy(PresentDrawableData *that) {
        if (that->mDrawable) {
            std::lock_guard<std::mutex> lock(*(that->mDrawableMutex));
            that->mDrawable = nil;
        }
        that->mDrawableMutex.reset();
        that->mDriver = nullptr;
        delete that;
    }

    id<CAMetalDrawable> mDrawable;
    std::shared_ptr<std::mutex> mDrawableMutex;
    MetalDriver* mDriver = nullptr;
    uint64_t mFlags = 0;
};

void presentDrawable(bool presentFrame, void* user) {
    auto* presentDrawableData = static_cast<PresentDrawableData*>(user);
    PresentDrawableData::maybePresentAndDestroyAsync(presentDrawableData, presentFrame);
}

void MetalSwapChain::scheduleFrameScheduledCallback() {
    if (!frameScheduled.callback) {
        return;
    }

    assert_invariant(drawable);

    struct Callback {
        Callback(std::shared_ptr<FrameScheduledCallback> callback, id<CAMetalDrawable> drawable,
                 std::shared_ptr<std::mutex> drawableMutex, MetalDriver* driver, uint64_t flags)
            : f(callback), data(PresentDrawableData::create(drawable, drawableMutex, driver, flags)) {}
        std::shared_ptr<FrameScheduledCallback> f;
        // PresentDrawableData* is destroyed by maybePresentAndDestroyAsync() later.
        std::unique_ptr<PresentDrawableData> data;
        static void func(void* user) {
            auto* const c = reinterpret_cast<Callback*>(user);
            PresentDrawableData* presentDrawableData = c->data.release();
            PresentCallable presentCallable(presentDrawable, presentDrawableData);
            c->f->operator()(presentCallable);
            delete c;
        }
    };

    // This callback pointer will be captured by the block. Even if the scheduled handler is never
    // called, the unique_ptr will still ensure we don't leak memory.
    uint64_t const flags = frameScheduled.flags;
    __block auto callback = std::make_unique<Callback>(
            frameScheduled.callback, drawable, layerDrawableMutex, context.driver, flags);

    backend::CallbackHandler* handler = frameScheduled.handler;
    MetalDriver* driver = context.driver;
    [getPendingCommandBuffer(&context) addScheduledHandler:^(id<MTLCommandBuffer> cb) {
        Callback* user = callback.release();
        if (flags & SwapChain::CALLBACK_DEFAULT_USE_METAL_COMPLETION_HANDLER) {
            Callback::func(user);
        } else {
            driver->scheduleCallback(handler, user, &Callback::func);
        }
    }];
}

void MetalSwapChain::scheduleFrameCompletedCallback() {
    if (!frameCompleted.callback) {
        return;
    }

    struct Callback {
        Callback(std::shared_ptr<utils::Invocable<void(void)>> callback) : f(callback) {}
        std::shared_ptr<utils::Invocable<void(void)>> f;
        static void func(void* user) {
            auto* const c = reinterpret_cast<Callback*>(user);
            c->f->operator()();
            delete c;
        }
    };

    // This callback pointer will be captured by the block. Even if the completed handler is never
    // called, the unique_ptr will still ensure we don't leak memory.
    __block auto callback = std::make_unique<Callback>(frameCompleted.callback);

    CallbackHandler* handler = frameCompleted.handler;
    MetalDriver* driver = context.driver;
    [getPendingCommandBuffer(&context) addCompletedHandler:^(id<MTLCommandBuffer> cb) {
        Callback* user = callback.release();
        driver->scheduleCallback(handler, user, &Callback::func);
    }];
}

MetalBufferObject::MetalBufferObject(MetalContext& context, BufferObjectBinding bindingType,
        BufferUsage usage, uint32_t byteCount)
        : HwBufferObject(byteCount), buffer(context, bindingType, usage, byteCount) {}

void MetalBufferObject::updateBuffer(
        void* data, size_t size, uint32_t byteOffset, TagResolver&& getHandleTag) {
    buffer.copyIntoBuffer(data, size, byteOffset, std::move(getHandleTag));
}

void MetalBufferObject::updateBufferUnsynchronized(
        void* data, size_t size, uint32_t byteOffset, TagResolver&& getHandleTag) {
    buffer.copyIntoBufferUnsynchronized(data, size, byteOffset, std::move(getHandleTag));
}

MetalVertexBufferInfo::MetalVertexBufferInfo(MetalContext& context, uint8_t bufferCount,
        uint8_t attributeCount, AttributeArray const& attributes)
        : HwVertexBufferInfo(bufferCount, attributeCount),
          bufferMapping(utils::FixedCapacityVector<Entry>::with_capacity(MAX_VERTEX_BUFFER_COUNT)) {

    const size_t maxAttributeCount = attributes.size();

    auto& mapping = bufferMapping;
    mapping.clear();
    vertexDescription = {};

    // Set the layout for the zero buffer, which unused attributes are mapped to.
    vertexDescription.layouts[ZERO_VERTEX_BUFFER_LOGICAL_INDEX] = {
            .step = MTLVertexStepFunctionConstant, .stride = 16
    };

    // Here we map each source buffer to a Metal buffer argument.
    // Each attribute has a source buffer, offset, and stride.
    // Two source buffers with the same index and stride can share the same Metal buffer argument
    // index.
    //
    // The source buffer is the buffer index that the Filament client sets.
    //                                       * source buffer
    // .attribute(VertexAttribute::POSITION, 0, VertexBuffer::AttributeType::FLOAT2, 0, 12)
    // .attribute(VertexAttribute::UV,       0, VertexBuffer::AttributeType::HALF2,  8, 12)
    // .attribute(VertexAttribute::COLOR,    1, VertexBuffer::AttributeType::UBYTE4, 0,  4)

    auto allocateOrGetBufferArgumentIndex =
            [&mapping, currentBufferArgumentIndex = USER_VERTEX_BUFFER_BINDING_START, this](
                    auto sourceBuffer, auto sourceBufferStride) mutable -> uint8_t {
        auto match = [&](const auto& e) {
            return e.sourceBufferIndex == sourceBuffer && e.stride == sourceBufferStride;
        };
        if (auto it = std::find_if(mapping.begin(), mapping.end(), match); it != mapping.end()) {
            return it->bufferArgumentIndex;
        } else {
            auto bufferArgumentIndex = currentBufferArgumentIndex++;
            mapping.emplace_back(sourceBuffer, sourceBufferStride, bufferArgumentIndex);
            vertexDescription.layouts[bufferArgumentIndex] = {
                    .step = MTLVertexStepFunctionPerVertex, .stride = sourceBufferStride
            };
            return bufferArgumentIndex;
        }
    };

    for (uint32_t attributeIndex = 0; attributeIndex < maxAttributeCount; attributeIndex++) {
        const auto& attribute = attributes[attributeIndex];

        // If the attribute is unused, bind it to the zero buffer. It's a Metal error for a shader
        // to read from missing vertex attributes.
        if (attribute.buffer == Attribute::BUFFER_UNUSED) {
            const MTLVertexFormat format = (attribute.flags & Attribute::FLAG_INTEGER_TARGET)
                    ? MTLVertexFormatUInt4
                    : MTLVertexFormatFloat4;
            vertexDescription.attributes[attributeIndex] = {
                    .format = format, .buffer = ZERO_VERTEX_BUFFER_LOGICAL_INDEX, .offset = 0
            };
            continue;
        }

        // Map the source buffer and stride of this attribute to a Metal buffer argument.
        auto bufferArgumentIndex =
                allocateOrGetBufferArgumentIndex(attribute.buffer, attribute.stride);

        vertexDescription.attributes[attributeIndex] = {
                .format = getMetalFormat(
                        attribute.type, attribute.flags & Attribute::FLAG_NORMALIZED),
                .buffer = uint32_t(bufferArgumentIndex),
                .offset = attribute.offset
        };
    }
}

MetalVertexBuffer::MetalVertexBuffer(MetalContext& context,
        uint32_t vertexCount, uint32_t bufferCount, Handle<HwVertexBufferInfo> vbih)
    : HwVertexBuffer(vertexCount), vbih(vbih), buffers(bufferCount, nullptr) {
}

MetalIndexBuffer::MetalIndexBuffer(MetalContext& context, BufferUsage usage, uint8_t elementSize,
        uint32_t indexCount) : HwIndexBuffer(elementSize, indexCount),
        buffer(context, BufferObjectBinding::VERTEX, usage, elementSize * indexCount, true) { }

MetalRenderPrimitive::MetalRenderPrimitive() {
}

void MetalRenderPrimitive::setBuffers(MetalVertexBufferInfo const* const vbi,
        MetalVertexBuffer* vertexBuffer, MetalIndexBuffer* indexBuffer) {
    this->vertexBuffer = vertexBuffer;
    this->indexBuffer = indexBuffer;
}

MetalProgram::MetalProgram(MetalContext& context, Program&& program) noexcept
    : HwProgram(program.getName()), mContext(context) {
    mToken = context.shaderCompiler->createProgram(program.getName(), std::move(program));
    assert_invariant(mToken);
}

const MetalShaderCompiler::MetalFunctionBundle& MetalProgram::getFunctions() {
    initialize();
    return mFunctionBundle;
}

const MetalShaderCompiler::MetalFunctionBundle& MetalProgram::getFunctionsIfPresent() const {
    return mFunctionBundle;
}

void MetalProgram::initialize() {
    if (!mToken) {
        return;
    }
    mFunctionBundle = mContext.shaderCompiler->getProgram(mToken);
    assert_invariant(!mToken);
}

MetalTexture::MetalTexture(MetalContext& context, SamplerType target, uint8_t levels,
        TextureFormat format, uint8_t samples, uint32_t width, uint32_t height, uint32_t depth,
        TextureUsage usage) noexcept
    : HwTexture(target, levels, samples, width, height, depth, format, usage), context(context) {
    assert_invariant(target != SamplerType::SAMPLER_EXTERNAL);

    devicePixelFormat = decidePixelFormat(&context, format);
    FILAMENT_CHECK_POSTCONDITION(devicePixelFormat != MTLPixelFormatInvalid)
            << "Texture format not supported.";

    const BOOL mipmapped = levels > 1;
    const BOOL multisampled = samples > 1;

#if defined(IOS)
    const BOOL textureArray = target == SamplerType::SAMPLER_2D_ARRAY;
    FILAMENT_CHECK_PRECONDITION(!textureArray || !multisampled)
            << "iOS does not support multisampled texture arrays.";
#endif

    const auto get2DTextureType = [](SamplerType target, bool isMultisampled) {
        if (!isMultisampled) {
            return getMetalType(target);
        }
        switch (target) {
            case SamplerType::SAMPLER_2D:
                return MTLTextureType2DMultisample;
#if !defined(IOS)
            case SamplerType::SAMPLER_2D_ARRAY:
                return MTLTextureType2DMultisampleArray;
#endif
            default:
                // should not get here
                return getMetalType(target);
        }
    };

    MTLTextureDescriptor* descriptor;
    switch (target) {
        case SamplerType::SAMPLER_2D:
        case SamplerType::SAMPLER_2D_ARRAY:
            descriptor = [MTLTextureDescriptor new];
            descriptor.pixelFormat = devicePixelFormat;
            descriptor.textureType = get2DTextureType(target, multisampled);
            descriptor.width = width;
            descriptor.height = height;
            descriptor.arrayLength = depth;
            descriptor.mipmapLevelCount = levels;
            descriptor.sampleCount = multisampled ? samples : 1;
            descriptor.usage = getMetalTextureUsage(usage);
            descriptor.storageMode = MTLStorageModePrivate;
            texture = [context.device newTextureWithDescriptor:descriptor];
            break;
        case SamplerType::SAMPLER_CUBEMAP:
        case SamplerType::SAMPLER_CUBEMAP_ARRAY:
            FILAMENT_CHECK_POSTCONDITION(!multisampled)
                    << "Multisampled cubemap faces not supported.";
            FILAMENT_CHECK_POSTCONDITION(width == height) << "Cubemap faces must be square.";
            descriptor = [MTLTextureDescriptor textureCubeDescriptorWithPixelFormat:devicePixelFormat
                                                                               size:width
                                                                          mipmapped:mipmapped];
            descriptor.arrayLength = depth;
            descriptor.mipmapLevelCount = levels;
            descriptor.usage = getMetalTextureUsage(usage);
            descriptor.storageMode = MTLStorageModePrivate;
            texture = [context.device newTextureWithDescriptor:descriptor];
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
            break;
        case SamplerType::SAMPLER_EXTERNAL:
            // If we're using external textures (CVPixelBufferRefs), we don't need to make any
            // texture allocations.
            texture = nil;
            break;
    }

    FILAMENT_CHECK_POSTCONDITION(target == SamplerType::SAMPLER_EXTERNAL || texture != nil)
            << "Could not create Metal texture (SamplerType = " << int(target)
            << ", levels = " << int(levels) << ", MTLPixelFormat = " << int(devicePixelFormat)
            << ", width = " << width << ", height = " << height << ", depth = " << depth
            << "). Out of memory?";
}

MetalTexture::MetalTexture(MetalContext& context, MetalTexture const* src, uint8_t baseLevel,
        uint8_t levelCount) noexcept
    : HwTexture(src->target, src->levels, src->samples, src->width, src->height, src->depth,
              src->format, src->usage),
      context(context),
      devicePixelFormat(src->devicePixelFormat),
      externalImage(src->externalImage) {
    texture = createTextureViewWithLodRange(
            src->getMtlTextureForRead(), baseLevel, baseLevel + levelCount - 1);
}

MetalTexture::MetalTexture(MetalContext& context, MetalTexture const* src, TextureSwizzle r,
        TextureSwizzle g, TextureSwizzle b, TextureSwizzle a) noexcept
    : HwTexture(src->target, src->levels, src->samples, src->width, src->height, src->depth,
              src->format, src->usage),
      context(context),
      devicePixelFormat(src->devicePixelFormat),
      externalImage(src->externalImage) {
    texture = src->getMtlTextureForRead();
    if (context.supportsTextureSwizzling) {
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
    : HwTexture(target, levels, samples, width, height, depth, format, usage), context(context) {
    texture = metalTexture;
}

MetalTexture::MetalTexture(MetalContext& context, TextureFormat format, uint32_t width,
        uint32_t height, TextureUsage usage, CVPixelBufferRef image) noexcept
    : HwTexture(SamplerType::SAMPLER_EXTERNAL, 1, 1, width, height, 1, format, usage),
      context(context),
      externalImage(std::make_shared<MetalExternalImage>(
              MetalExternalImage::createFromImage(context, image))) {
    texture = externalImage->getMtlTexture();
}

MetalTexture::MetalTexture(MetalContext& context, TextureFormat format, uint32_t width,
        uint32_t height, TextureUsage usage, CVPixelBufferRef image, uint32_t plane) noexcept
    : HwTexture(SamplerType::SAMPLER_EXTERNAL, 1, 1, width, height, 1, format, usage),
      context(context),
      externalImage(std::make_shared<MetalExternalImage>(
              MetalExternalImage::createFromImagePlane(context, image, plane))) {
    texture = externalImage->getMtlTexture();
}

id<MTLTexture> MetalTexture::getMtlTextureForRead() const noexcept {
    return swizzledTextureView ? swizzledTextureView : texture;
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

        case SamplerType::SAMPLER_CUBEMAP:
        case SamplerType::SAMPLER_CUBEMAP_ARRAY:
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

        case SamplerType::SAMPLER_EXTERNAL: {
            assert_invariant(false);
        }
    }
}

void MetalTexture::generateMipmaps() noexcept {
    id <MTLBlitCommandEncoder> blitEncoder = [getPendingCommandBuffer(&context) blitCommandEncoder];
    [blitEncoder generateMipmapsForTexture:texture];
    [blitEncoder endEncoding];
}

void MetalTexture::loadSlice(uint32_t level, MTLRegion region, uint32_t byteOffset, uint32_t slice,
        PixelBufferDescriptor const& data) noexcept {
    const PixelBufferShape shape = PixelBufferShape::compute(data, format, region.size, byteOffset);

    FILAMENT_CHECK_PRECONDITION(data.size >= shape.totalBytes)
            << "Expected buffer size of at least " << shape.totalBytes
            << " but received PixelBufferDescriptor with size " << data.size << ".";

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
        PixelBufferDescriptor const& data, const PixelBufferShape& shape) {
    const size_t stagingBufferSize = shape.totalBytes;
    auto entry = context.bufferPool->acquireBuffer(stagingBufferSize);
    memcpy(entry->buffer.get().contents,
            static_cast<uint8_t*>(data.buffer) + shape.sourceOffset,
            stagingBufferSize);
    id<MTLCommandBuffer> blitCommandBuffer = getPendingCommandBuffer(&context);
    id<MTLBlitCommandEncoder> blitCommandEncoder = [blitCommandBuffer blitCommandEncoder];
    blitCommandEncoder.label = @"Texture upload buffer blit";
    [blitCommandEncoder copyFromBuffer:entry->buffer.get()
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
        PixelBufferDescriptor const& data, const PixelBufferShape& shape) {
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
    // FIXME? Why is this not just `MTLRegion sourceRegion = region;` ?
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

    MetalBlitter::BlitArgs args{};
    args.filter = SamplerMagFilter::NEAREST;
    args.source.level = 0;
    args.source.slice = 0;
    args.source.region = sourceRegion;
    args.source.texture = stagingTexture;
    args.destination.level = level;
    args.destination.slice = slice;
    args.destination.region = region;
    args.destination.texture = destinationTexture;
    context.blitter->blit(getPendingCommandBuffer(&context), args, "Texture upload blit");
}

MetalRenderTarget::MetalRenderTarget(MetalContext* context, uint32_t width, uint32_t height,
        uint8_t samples, Attachment colorAttachments[MRT::MAX_SUPPORTED_RENDER_TARGET_COUNT],
        Attachment depthAttachment, Attachment stencilAttachment) :
        HwRenderTarget(width, height), context(context), samples(samples) {
    math::uint2 tmin = {std::numeric_limits<uint32_t>::max()};
    UTILS_UNUSED_IN_RELEASE math::uint2 tmax = {0};
    UTILS_UNUSED_IN_RELEASE size_t attachmentCount = 0;

    for (size_t i = 0; i < MRT::MAX_SUPPORTED_RENDER_TARGET_COUNT; i++) {
        if (!colorAttachments[i]) {
            continue;
        }
        color[i] = colorAttachments[i];

        FILAMENT_CHECK_PRECONDITION(color[i].getSampleCount() <= samples)
                << "MetalRenderTarget was initialized with a MSAA COLOR" << i
                << " texture, but sample count is " << samples << ".";

        auto t = color[i].metalTexture;
        const auto twidth = std::max(1u, t->width >> color[i].level);
        const auto theight = std::max(1u, t->height >> color[i].level);
        tmin = { std::min(tmin.x, twidth), std::min(tmin.y, theight) };
        tmax = { std::max(tmax.x, twidth), std::max(tmax.y, theight) };
        attachmentCount++;

        // If we were given a single-sampled texture but the samples parameter is > 1, we create
        // a multisampled sidecar texture and do a resolve automatically.
        if (samples > 1 && color[i].getSampleCount() == 1) {
            auto& sidecar = color[i].metalTexture->msaaSidecar;
            if (!sidecar) {
                sidecar = createMultisampledTexture(color[i].getPixelFormat(),
                        color[i].metalTexture->width, color[i].metalTexture->height, samples);
            }
        }
    }

    if (depthAttachment) {
        depth = depthAttachment;

        FILAMENT_CHECK_PRECONDITION(depth.getSampleCount() <= samples)
                << "MetalRenderTarget was initialized with a MSAA DEPTH texture, but sample count "
                   "is "
                << samples << ".";

        auto t = depth.metalTexture;
        const auto twidth = std::max(1u, t->width >> depth.level);
        const auto theight = std::max(1u, t->height >> depth.level);
        tmin = { math::min(tmin.x, twidth), math::min(tmin.y, theight) };
        tmax = { math::max(tmax.x, twidth), math::max(tmax.y, theight) };
        attachmentCount++;

        // If we were given a single-sampled texture but the samples parameter is > 1, we create
        // a multisampled sidecar texture and do a resolve automatically.
        if (samples > 1 && depth.getSampleCount() == 1) {
            auto& sidecar = depth.metalTexture->msaaSidecar;
            if (!sidecar) {
                sidecar = createMultisampledTexture(depth.getPixelFormat(),
                        depth.metalTexture->width, depth.metalTexture->height, samples);
            }
        }
    }

    if (stencilAttachment) {
        stencil = stencilAttachment;

        FILAMENT_CHECK_PRECONDITION(stencil.getSampleCount() <= samples)
                << "MetalRenderTarget was initialized with a MSAA STENCIL texture, but sample "
                   "count is "
                << samples << ".";

        auto t = stencil.metalTexture;
        const auto twidth = std::max(1u, t->width >> stencil.level);
        const auto theight = std::max(1u, t->height >> stencil.level);
        tmin = { math::min(tmin.x, twidth), math::min(tmin.y, theight) };
        tmax = { math::max(tmax.x, twidth), math::max(tmax.y, theight) };
        attachmentCount++;

        // If we were given a single-sampled texture but the samples parameter is > 1, we create
        // a multisampled sidecar texture and do a resolve automatically.
        if (samples > 1 && stencil.getSampleCount() == 1) {
            auto& sidecar = stencil.metalTexture->msaaSidecar;
            if (!sidecar) {
                sidecar = createMultisampledTexture(stencil.getPixelFormat(),
                        stencil.metalTexture->width, stencil.metalTexture->height, samples);
            }
        }
    }

    // Verify that all attachments have the same non-zero dimensions.
    assert_invariant(attachmentCount > 0);
    assert_invariant(tmin == tmax);
    assert_invariant(tmin.x > 0 && tmin.y > 0);

    // The render target dimensions must be less than or equal to the attachment size.
    assert_invariant(width <= tmin.x && height <= tmin.y);

    // Store the width/height of all attachments. We'll use this when converting from Filament's
    // coordinate system to Metal's.
    attachmentSize = tmin;
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
    if (depthAttachment) {
        descriptor.depthAttachment.texture = depthAttachment.getTexture();
        descriptor.depthAttachment.level = depthAttachment.level;
        descriptor.depthAttachment.slice = depthAttachment.layer;
        descriptor.depthAttachment.loadAction = getLoadAction(params, TargetBufferFlags::DEPTH);
        descriptor.depthAttachment.storeAction = getStoreAction(params, TargetBufferFlags::DEPTH);
        descriptor.depthAttachment.clearDepth = params.clearDepth;
    }

    const bool depthAutomaticResolve = samples > 1 && depthAttachment.getSampleCount() == 1;
    if (depthAutomaticResolve) {
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
            assert_invariant(context->supportsAutoDepthResolve);
            descriptor.depthAttachment.resolveTexture = depthAttachment.getTexture();
            descriptor.depthAttachment.resolveLevel = depthAttachment.level;
            descriptor.depthAttachment.resolveSlice = depthAttachment.layer;
            descriptor.depthAttachment.storeAction = MTLStoreActionMultisampleResolve;
        }
    }

    Attachment stencilAttachment = getStencilAttachment();
    if (stencilAttachment) {
        descriptor.stencilAttachment.texture = stencilAttachment.getTexture();
        descriptor.stencilAttachment.level = stencilAttachment.level;
        descriptor.stencilAttachment.slice = stencilAttachment.layer;
        descriptor.stencilAttachment.loadAction = getLoadAction(params, TargetBufferFlags::STENCIL);
        descriptor.stencilAttachment.storeAction = getStoreAction(params, TargetBufferFlags::STENCIL);
        descriptor.stencilAttachment.clearStencil = params.clearStencil;
    }

    const bool stencilAutomaticResolve = samples > 1 && stencilAttachment.getSampleCount() == 1;
    if (stencilAutomaticResolve) {
        // We're rendering into our temporary MSAA texture and doing an automatic resolve.
        // We should not be attempting to load anything into the MSAA texture.
        assert_invariant(descriptor.stencilAttachment.loadAction != MTLLoadActionLoad);
        assert_invariant(!defaultRenderTarget);

        id<MTLTexture> sidecar = stencilAttachment.getMSAASidecarTexture();
        assert_invariant(sidecar);

        descriptor.stencilAttachment.texture = sidecar;
        descriptor.stencilAttachment.level = 0;
        descriptor.stencilAttachment.slice = 0;
        const bool discard = any(discardFlags & TargetBufferFlags::STENCIL);
        if (!discard) {
            assert_invariant(context->supportsAutoDepthResolve);
            descriptor.stencilAttachment.resolveTexture = stencilAttachment.getTexture();
            descriptor.stencilAttachment.resolveLevel = stencilAttachment.level;
            descriptor.stencilAttachment.resolveSlice = stencilAttachment.layer;
            descriptor.stencilAttachment.storeAction = MTLStoreActionMultisampleResolve;
            if (@available(iOS 12.0, *)) {
                descriptor.stencilAttachment.stencilResolveFilter = MTLMultisampleStencilResolveFilterSample0;
            }
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

MetalRenderTarget::Attachment MetalRenderTarget::getStencilAttachment() {
    Attachment result = stencil;
    if (defaultRenderTarget) {
        result.texture = context->currentDrawSwapChain->acquireStencilTexture();
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

id<MTLTexture> MetalRenderTarget::createMultisampledTexture(MTLPixelFormat format,
        uint32_t width, uint32_t height, uint8_t samples) const {
    MTLTextureDescriptor* descriptor =
            [MTLTextureDescriptor texture2DDescriptorWithPixelFormat:format
                                                               width:width
                                                              height:height
                                                            mipmapped:NO];
    descriptor.textureType = MTLTextureType2DMultisample;
    descriptor.sampleCount = samples;
    descriptor.usage = MTLTextureUsageRenderTarget;
    descriptor.resourceOptions = MTLResourceStorageModePrivate;

    if (context->supportsMemorylessRenderTargets) {
        if (@available(macOS 11.0, *)) {
            descriptor.resourceOptions = MTLResourceStorageModeMemoryless;
        }
    }

    return [context->device newTextureWithDescriptor:descriptor];
}

math::uint2 MetalRenderTarget::getAttachmentSize() noexcept {
    if (defaultRenderTarget) {
        // The default render target always has a color attachment.
        auto colorAttachment = getDrawColorAttachment(0);
        assert_invariant(colorAttachment);
        id<MTLTexture> t = colorAttachment.getTexture();
        return {t.width, t.height};
    }
    assert_invariant(attachmentSize.x > 0 && attachmentSize.y > 0);
    return attachmentSize;
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

MetalDescriptorSetLayout::MetalDescriptorSetLayout(DescriptorSetLayout&& l) noexcept
    : mLayout(std::move(l)) {
    size_t dynamicBindings = 0;
    for (const auto& binding : mLayout.bindings) {
        if (any(binding.flags & DescriptorFlags::DYNAMIC_OFFSET)) {
            dynamicBindings++;
        }
    }
    mDynamicOffsetCount = dynamicBindings;
}

id<MTLArgumentEncoder> MetalDescriptorSetLayout::getArgumentEncoder(id<MTLDevice> device, ShaderStage stage,
        utils::FixedCapacityVector<MTLTextureType> const& textureTypes) {
    auto const index = static_cast<size_t>(stage);
    assert_invariant(index < mCachedArgumentEncoder.size());
    if (mCachedArgumentEncoder[index] &&
            std::equal(
                    textureTypes.begin(), textureTypes.end(), mCachedTextureTypes[index].begin())) {
        return mCachedArgumentEncoder[index];
    }
    mCachedArgumentEncoder[index] = getArgumentEncoderSlow(device, stage, textureTypes);
    mCachedTextureTypes[index] = textureTypes;
    return mCachedArgumentEncoder[index];
}

id<MTLArgumentEncoder> MetalDescriptorSetLayout::getArgumentEncoderSlow(id<MTLDevice> device,
        ShaderStage stage, utils::FixedCapacityVector<MTLTextureType> const& textureTypes) {
    auto const& bindings = getBindings();
    NSMutableArray<MTLArgumentDescriptor*>* arguments = [NSMutableArray new];
    // Important! The bindings must be sorted by binding number. This has already been done inside
    // createDescriptorSetLayout.
    size_t textureIndex = 0;
    for (auto const& binding : bindings) {
        if (!hasShaderType(binding.stageFlags, stage)) {
            continue;
        }
        switch (binding.type) {
            case DescriptorType::UNIFORM_BUFFER:
            case DescriptorType::SHADER_STORAGE_BUFFER: {
                MTLArgumentDescriptor* bufferArgument = [MTLArgumentDescriptor argumentDescriptor];
                bufferArgument.index = binding.binding * 2;
                bufferArgument.dataType = MTLDataTypePointer;
                bufferArgument.access = MTLArgumentAccessReadOnly;
                [arguments addObject:bufferArgument];
                break;
            }
            case DescriptorType::SAMPLER: {
                MTLArgumentDescriptor* textureArgument = [MTLArgumentDescriptor argumentDescriptor];
                textureArgument.index = binding.binding * 2;
                textureArgument.dataType = MTLDataTypeTexture;
                MTLTextureType textureType = MTLTextureType2D;
                if (textureIndex < textureTypes.size()) {
                    textureType = textureTypes[textureIndex++];
                }
                textureArgument.textureType = textureType;
                textureArgument.access = MTLArgumentAccessReadOnly;
                [arguments addObject:textureArgument];

                MTLArgumentDescriptor* samplerArgument = [MTLArgumentDescriptor argumentDescriptor];
                samplerArgument.index = binding.binding * 2 + 1;
                samplerArgument.dataType = MTLDataTypeSampler;
                textureArgument.access = MTLArgumentAccessReadOnly;
                [arguments addObject:samplerArgument];
                break;
            }
            case DescriptorType::INPUT_ATTACHMENT:
                // TODO: support INPUT_ATTACHMENT
                assert_invariant(false);
                break;
        }
    }
    return [device newArgumentEncoderWithArguments:arguments];
}

MetalDescriptorSet::MetalDescriptorSet(MetalDescriptorSetLayout* layout) noexcept
    : layout(layout) {}

void MetalDescriptorSet::finalize(MetalDriver* driver) {
    [driver->mContext->currentRenderPassEncoder useResource:driver->mContext->emptyBuffer
                                                      usage:MTLResourceUsageRead];
    [driver->mContext->currentRenderPassEncoder
            useResource:getOrCreateEmptyTexture(driver->mContext)
                  usage:MTLResourceUsageRead];

    if (@available(iOS 13.0, *)) {
        [driver->mContext->currentRenderPassEncoder useResources:vertexResources.data()
                                                           count:vertexResources.size()
                                                           usage:MTLResourceUsageRead
                                                          stages:MTLRenderStageVertex];
        [driver->mContext->currentRenderPassEncoder useResources:fragmentResources.data()
                                                           count:fragmentResources.size()
                                                           usage:MTLResourceUsageRead
                                                          stages:MTLRenderStageFragment];
    } else {
        [driver->mContext->currentRenderPassEncoder useResources:vertexResources.data()
                                                           count:vertexResources.size()
                                                           usage:MTLResourceUsageRead];
        [driver->mContext->currentRenderPassEncoder useResources:fragmentResources.data()
                                                           count:fragmentResources.size()
                                                           usage:MTLResourceUsageRead];
    }
}

id<MTLBuffer> MetalDescriptorSet::finalizeAndGetBuffer(MetalDriver* driver, ShaderStage stage) {
    auto const index = static_cast<size_t>(stage);
    assert_invariant(index < cachedBuffer.size());
    auto& buffer = cachedBuffer[index];

    if (buffer) {
        return buffer.get();
    }

    // Map all the texture bindings to their respective texture types.
    auto const& bindings = layout->getBindings();
    auto textureTypes = utils::FixedCapacityVector<MTLTextureType>::with_capacity(bindings.size());
    for (auto const& binding : bindings) {
        if (!hasShaderType(binding.stageFlags, stage)) {
            continue;
        }
        MTLTextureType textureType = MTLTextureType2D;
        if (auto found = textures.find(binding.binding); found != textures.end()) {
            auto const& textureBinding = textures[binding.binding];
            textureType = textureBinding.texture.textureType;
        }
        textureTypes.push_back(textureType);
    }

    MetalContext const& context = *driver->mContext;

    id<MTLArgumentEncoder> encoder =
            layout->getArgumentEncoder(context.device, stage, textureTypes);

    {
        ScopedAllocationTimer timer("descriptor_set");
        buffer = { [context.device newBufferWithLength:encoder.encodedLength
                                               options:MTLResourceStorageModeShared],
            TrackedMetalBuffer::Type::DESCRIPTOR_SET };
    }
    [encoder setArgumentBuffer:buffer.get() offset:0];

    for (auto const& binding : bindings) {
        if (!hasShaderType(binding.stageFlags, stage)) {
            continue;
        }
        switch (binding.type) {
            case DescriptorType::UNIFORM_BUFFER:
            case DescriptorType::SHADER_STORAGE_BUFFER: {
                auto found = buffers.find(binding.binding);
                if (found == buffers.end()) {
                    [encoder setBuffer:driver->mContext->emptyBuffer
                                offset:0
                               atIndex:binding.binding * 2];
                    continue;
                }

                auto const& bufferBinding = buffers[binding.binding];
                [encoder setBuffer:bufferBinding.buffer
                            offset:bufferBinding.offset
                           atIndex:binding.binding * 2];
                break;
            }
            case DescriptorType::SAMPLER: {
                auto found = textures.find(binding.binding);
                if (found == textures.end()) {
                    [encoder setTexture:driver->mContext->emptyTexture atIndex:binding.binding * 2];
                    id<MTLSamplerState> sampler =
                            driver->mContext->samplerStateCache.getOrCreateState({});
                    [encoder setSamplerState:sampler atIndex:binding.binding * 2 + 1];
                    continue;
                }

                auto const& textureBinding = textures[binding.binding];
                [encoder setTexture:textureBinding.texture atIndex:binding.binding * 2];
                SamplerState samplerState { .samplerParams = textureBinding.sampler };
                id<MTLSamplerState> sampler =
                        driver->mContext->samplerStateCache.getOrCreateState(samplerState);
                [encoder setSamplerState:sampler atIndex:binding.binding * 2 + 1];
                break;
            }
            case DescriptorType::INPUT_ATTACHMENT:
                assert_invariant(false);
                break;
        }
    }

    return buffer.get();
}

} // namespace backend
} // namespace filament
