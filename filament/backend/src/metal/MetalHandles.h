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

#ifndef TNT_FILAMENT_DRIVER_METALHANDLES_H
#define TNT_FILAMENT_DRIVER_METALHANDLES_H

#include "metal/MetalDriver.h"

#include <CoreVideo/CVPixelBuffer.h>
#include <Metal/Metal.h>
#include <QuartzCore/QuartzCore.h> // for CAMetalLayer

#include "MetalBuffer.h"
#include "MetalContext.h"
#include "MetalEnums.h"
#include "MetalExternalImage.h"
#include "MetalState.h" // for MetalState::VertexDescription

#include <utils/bitset.h>
#include <utils/FixedCapacityVector.h>
#include <utils/Panic.h>

#include <atomic>
#include <condition_variable>
#include <memory>
#include <type_traits>

namespace filament {
namespace backend {

class MetalSwapChain : public HwSwapChain {
public:

    // Instantiate a SwapChain from a CAMetalLayer
    MetalSwapChain(MetalContext& context, CAMetalLayer* nativeWindow, uint64_t flags);

    // Instantiate a SwapChain from a CVPixelBuffer
    MetalSwapChain(MetalContext& context, CVPixelBufferRef pixelBuffer, uint64_t flags);

    // Instantiate a headless SwapChain.
    MetalSwapChain(MetalContext& context, int32_t width, int32_t height, uint64_t flags);

    ~MetalSwapChain();

    // Acquires a texture that can be used to render into this SwapChain.
    // The texture source depends on the type of SwapChain:
    //   - CAMetalLayer-backed: acquires the CAMetalDrawable and returns its texture.
    //   - Headless: lazily creates and returns a headless texture.
    id<MTLTexture> acquireDrawable();

    id<MTLTexture> acquireDepthTexture();

    void releaseDrawable();

    void setFrameScheduledCallback(FrameScheduledCallback callback, void* user);
    void setFrameCompletedCallback(FrameCompletedCallback callback, void* user);

    // For CAMetalLayer-backed SwapChains, presents the drawable or schedules a
    // FrameScheduledCallback.
    void present();

    NSUInteger getSurfaceWidth() const;
    NSUInteger getSurfaceHeight() const;

private:

    enum class SwapChainType {
        CAMETALLAYER,
        CVPIXELBUFFERREF,
        HEADLESS
    };
    bool isCaMetalLayer() const { return type == SwapChainType::CAMETALLAYER; }
    bool isHeadless() const { return type == SwapChainType::HEADLESS; }
    bool isPixelBuffer() const { return type == SwapChainType::CVPIXELBUFFERREF; }

    void scheduleFrameScheduledCallback();
    void scheduleFrameCompletedCallback();

    MetalContext& context;
    id<CAMetalDrawable> drawable = nil;
    id<MTLTexture> depthTexture = nil;
    id<MTLTexture> headlessDrawable = nil;
    NSUInteger headlessWidth;
    NSUInteger headlessHeight;
    CAMetalLayer* layer = nullptr;
    MetalExternalImage externalImage;
    SwapChainType type;

    // These two fields store a callback and user data to notify the client that a frame is ready
    // for presentation.
    // If frameScheduledCallback is nullptr, then the Metal backend automatically calls
    // presentDrawable when the frame is committed.
    // Otherwise, the Metal backend will not automatically present the frame. Instead, clients bear
    // the responsibility of presenting the frame by calling the PresentCallable object.
    FrameScheduledCallback frameScheduledCallback = nullptr;
    void* frameScheduledUserData = nullptr;

    FrameCompletedCallback frameCompletedCallback = nullptr;
    void* frameCompletedUserData = nullptr;
};

class MetalBufferObject : public HwBufferObject {
public:
    MetalBufferObject(MetalContext& context, BufferUsage usage, uint32_t byteCount);

    void updateBuffer(void* data, size_t size, uint32_t byteOffset);
    void updateBufferUnsynchronized(void* data, size_t size, uint32_t byteOffset);
    MetalBuffer* getBuffer() { return &buffer; }

    // Tracks which uniform buffers this buffer object is bound into.
    static_assert(Program::BINDING_COUNT <= 32);
    utils::bitset32 boundUniformBuffers;

private:
    MetalBuffer buffer;
};

struct MetalVertexBuffer : public HwVertexBuffer {
    MetalVertexBuffer(MetalContext& context, uint8_t bufferCount, uint8_t attributeCount,
            uint32_t vertexCount, AttributeArray const& attributes);

    utils::FixedCapacityVector<MetalBuffer*> buffers;
};

struct MetalIndexBuffer : public HwIndexBuffer {
    MetalIndexBuffer(MetalContext& context, BufferUsage usage, uint8_t elementSize,
            uint32_t indexCount);

    MetalBuffer buffer;
};

struct MetalRenderPrimitive : public HwRenderPrimitive {
    void setBuffers(MetalVertexBuffer* vertexBuffer, MetalIndexBuffer* indexBuffer);
    // The pointers to MetalVertexBuffer and MetalIndexBuffer are "weak".
    // The MetalVertexBuffer and MetalIndexBuffer must outlive the MetalRenderPrimitive.

    MetalVertexBuffer* vertexBuffer = nullptr;
    MetalIndexBuffer* indexBuffer = nullptr;

    // This struct is used to create the pipeline description to describe vertex assembly.
    VertexDescription vertexDescription = {};
};

struct MetalProgram : public HwProgram {
    MetalProgram(id<MTLDevice> device, const Program& program) noexcept;

    id<MTLFunction> vertexFunction;
    id<MTLFunction> fragmentFunction;

    struct SamplerBlockInfo {
        uint8_t samplerGroup = UINT8_MAX;
        uint8_t sampler = UINT8_MAX;
    };

    std::array<SamplerBlockInfo, MAX_VERTEX_SAMPLER_COUNT> vertexSamplerBlockInfo;
    std::array<SamplerBlockInfo, MAX_FRAGMENT_SAMPLER_COUNT> fragmentSamplerBlockInfo;

    bool isValid = false;
};

struct PixelBufferShape {
    uint32_t bytesPerPixel;
    uint32_t bytesPerRow;
    uint32_t bytesPerSlice;
    uint32_t totalBytes;

    // Offset into the buffer where the pixel data begins.
    uint32_t sourceOffset;

    static PixelBufferShape compute(const PixelBufferDescriptor& data, TextureFormat format,
            MTLSize size, uint32_t byteOffset);
};

struct MetalTexture : public HwTexture {
    MetalTexture(MetalContext& context, SamplerType target, uint8_t levels, TextureFormat format,
            uint8_t samples, uint32_t width, uint32_t height, uint32_t depth, TextureUsage usage,
            TextureSwizzle r, TextureSwizzle g, TextureSwizzle b, TextureSwizzle a)
            noexcept;

    // Constructor for importing an id<MTLTexture> outside of Filament.
    MetalTexture(MetalContext& context, SamplerType target, uint8_t levels, TextureFormat format,
            uint8_t samples, uint32_t width, uint32_t height, uint32_t depth, TextureUsage usage,
            id<MTLTexture> texture) noexcept;

    ~MetalTexture();

    void loadImage(uint32_t level, MTLRegion region, PixelBufferDescriptor& p) noexcept;
    void loadCubeImage(const FaceOffsets& faceOffsets, int miplevel, PixelBufferDescriptor& p);
    void loadSlice(uint32_t level, MTLRegion region, uint32_t byteOffset, uint32_t slice,
            PixelBufferDescriptor& data) noexcept;
    void loadWithCopyBuffer(uint32_t level, uint32_t slice, MTLRegion region, PixelBufferDescriptor& data,
            const PixelBufferShape& shape);
    void loadWithBlit(uint32_t level, uint32_t slice, MTLRegion region, PixelBufferDescriptor& data,
            const PixelBufferShape& shape);
    void updateLodRange(uint32_t level);

    static MTLPixelFormat decidePixelFormat(MetalContext* context, TextureFormat format);

    MetalContext& context;
    MetalExternalImage externalImage;
    id<MTLTexture> texture = nil;

    // A "sidecar" texture used to implement automatic MSAA resolve.
    // This is created by MetalRenderTarget and stored here so it can be used with multiple
    // render targets.
    id<MTLTexture> msaaSidecar = nil;

    // If non-nil, a swizzled texture view to use instead of "texture".
    // Filament swizzling only affects texture reads, so this should not be used when the texture is
    // bound as a render target attachment.
    id<MTLTexture> swizzledTextureView = nil;

    MTLPixelFormat devicePixelFormat;
    uint32_t minLod = UINT_MAX;
    uint32_t maxLod = 0;
};

struct MetalSamplerGroup : public HwSamplerGroup {
    explicit MetalSamplerGroup(size_t size) : HwSamplerGroup(size) {}
};

class MetalRenderTarget : public HwRenderTarget {
public:

    class Attachment {
    public:

        friend class MetalRenderTarget;

        Attachment() = default;
        Attachment(MetalTexture* metalTexture, uint8_t level = 0, uint16_t layer = 0) :
                level(level), layer(layer),
                texture(metalTexture->texture),
                metalTexture(metalTexture) { }

        id<MTLTexture> getTexture() const {
            return texture;
        }

        NSUInteger getSampleCount() const {
            return texture ? texture.sampleCount : 0u;
        }

        MTLPixelFormat getPixelFormat() const {
            return texture ? texture.pixelFormat : MTLPixelFormatInvalid;
        }

        MTLRegion getRegionFromClientRect(Viewport rect) {
            // Convert the Filament rect into Metal texture coordinates, taking into account Metal's
            // inverted texture space and the mip level. Note that the underlying Texture could be
            // larger than the RenderTarget. Metal's texture coordinates have (0, 0) at the top-left
            // of the texture, but Filament's coordinates have (0, 0) at bottom-left.
            const auto mipheight = texture.height >> level;
            return MTLRegionMake2D((NSUInteger)rect.left,
                    mipheight - (NSUInteger)rect.bottom - rect.height, rect.width, rect.height);
        }

        explicit operator bool() const {
            return texture != nil;
        }

        uint8_t level = 0;
        uint16_t layer = 0;

    private:

        id<MTLTexture> getMSAASidecarTexture() const {
            // This should only be called from render targets associated with a MetalTexture.
            assert_invariant(metalTexture);
            return metalTexture->msaaSidecar;
        }

        id<MTLTexture> texture = nil;
        MetalTexture* metalTexture = nullptr;
    };

    MetalRenderTarget(MetalContext* context, uint32_t width, uint32_t height, uint8_t samples,
            Attachment colorAttachments[MRT::MAX_SUPPORTED_RENDER_TARGET_COUNT], Attachment depthAttachment);
    explicit MetalRenderTarget(MetalContext* context)
            : HwRenderTarget(0, 0), context(context), defaultRenderTarget(true) {}

    void setUpRenderPassAttachments(MTLRenderPassDescriptor* descriptor, const RenderPassParams& params);

    bool isDefaultRenderTarget() const { return defaultRenderTarget; }
    uint8_t getSamples() const { return samples; }

    Attachment getDrawColorAttachment(size_t index);
    Attachment getReadColorAttachment(size_t index);
    Attachment getDepthAttachment();

private:

    static MTLLoadAction getLoadAction(const RenderPassParams& params, TargetBufferFlags buffer);
    static MTLStoreAction getStoreAction(const RenderPassParams& params, TargetBufferFlags buffer);
    static id<MTLTexture> createMultisampledTexture(id<MTLDevice> device, MTLPixelFormat format,
            uint32_t width, uint32_t height, uint8_t samples);

    MetalContext* context;
    bool defaultRenderTarget = false;
    uint8_t samples = 1;

    Attachment color[MRT::MAX_SUPPORTED_RENDER_TARGET_COUNT] = {};
    Attachment depth = {};
};

// MetalFence is used to implement both Fences and Syncs.
// There's no diamond problem, because HwBase (superclass of HwFence and HwSync) is empty.
static_assert(std::is_empty_v<HwBase>);
class MetalFence : public HwFence, public HwSync {
public:

    // MetalFence is special, as it gets constructed on the Filament thread. We must delay inserting
    // the fence into the command stream until encode() is called (on the driver thread).
    MetalFence(MetalContext& context);

    // Inserts this fence into the current command buffer. Must be called from the driver thread.
    void encode();

    FenceStatus wait(uint64_t timeoutNs);

    API_AVAILABLE(ios(12.0))
    typedef void (^MetalFenceSignalBlock)(id<MTLSharedEvent>, uint64_t value);

    API_AVAILABLE(ios(12.0))
    void onSignal(MetalFenceSignalBlock block);

private:

    MetalContext& context;

    struct State {
        FenceStatus status { FenceStatus::TIMEOUT_EXPIRED };
        std::condition_variable cv;
        std::mutex mutex;
    };
    std::shared_ptr<State> state { std::make_shared<State>() };

    // MTLSharedEvent is only available on iOS 12.0 and above.
    // The availability annotation ensures we wrap all usages of event in an @availability check.
    API_AVAILABLE(ios(12.0))
    id<MTLSharedEvent> event = nil;

    uint64_t value;
};

struct MetalTimerQuery : public HwTimerQuery {
    MetalTimerQuery() : status(std::make_shared<Status>()) {}

    struct Status {
        std::atomic<bool> available {false};
        uint64_t elapsed {0};   // only valid if available is true
    };

    std::shared_ptr<Status> status;
};

} // namespace backend
} // namespace filament

#endif //TNT_FILAMENT_DRIVER_METALHANDLES_H
