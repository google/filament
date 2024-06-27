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

#include "private/backend/SamplerGroup.h"

#include <backend/DriverEnums.h>

#include <utils/bitset.h>
#include <utils/CString.h>
#include <utils/FixedCapacityVector.h>
#include <utils/Panic.h>

#include <math/vec2.h>

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
    id<MTLTexture> acquireStencilTexture();

    void releaseDrawable();

    void setFrameScheduledCallback(CallbackHandler* handler, FrameScheduledCallback&& callback);
    void setFrameCompletedCallback(
            CallbackHandler* handler, utils::Invocable<void(void)>&& callback);

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

    static MTLPixelFormat decideDepthStencilFormat(uint64_t flags);
    void ensureDepthStencilTexture();

    MetalContext& context;
    id<CAMetalDrawable> drawable = nil;
    id<MTLTexture> depthStencilTexture = nil;
    id<MTLTexture> headlessDrawable = nil;
    MTLPixelFormat depthStencilFormat = MTLPixelFormatInvalid;
    NSUInteger headlessWidth = 0;
    NSUInteger headlessHeight = 0;
    CAMetalLayer* layer = nullptr;
    std::shared_ptr<std::mutex> layerDrawableMutex;
    MetalExternalImage externalImage;
    SwapChainType type;

    // These fields store a callback to notify the client that a frame is ready for presentation. If
    // !frameScheduled.callback, then the Metal backend automatically calls presentDrawable when the
    // frame is committed. Otherwise, the Metal backend will not automatically present the frame.
    // Instead, clients bear the responsibility of presenting the frame by calling the
    // PresentCallable object.
    struct {
        CallbackHandler* handler = nullptr;
        std::shared_ptr<FrameScheduledCallback> callback = nullptr;
    } frameScheduled;

    struct {
        CallbackHandler* handler = nullptr;
        std::shared_ptr<utils::Invocable<void(void)>> callback = nullptr;
    } frameCompleted;
};

class MetalBufferObject : public HwBufferObject {
public:
    MetalBufferObject(MetalContext& context, BufferObjectBinding bindingType, BufferUsage usage,
         uint32_t byteCount);

    void updateBuffer(void* data, size_t size, uint32_t byteOffset);
    void updateBufferUnsynchronized(void* data, size_t size, uint32_t byteOffset);
    MetalBuffer* getBuffer() { return &buffer; }

    // Tracks which uniform/ssbo buffers this buffer object is bound into.
    static_assert(Program::UNIFORM_BINDING_COUNT <= 32);
    static_assert(MAX_SSBO_COUNT <= 32);
    utils::bitset32 boundUniformBuffers;
    utils::bitset32 boundSsbos;

private:
    MetalBuffer buffer;
};

struct MetalVertexBufferInfo : public HwVertexBufferInfo {
    MetalVertexBufferInfo(MetalContext& context,
            uint8_t bufferCount, uint8_t attributeCount, AttributeArray const& attributes);

    // This struct is used to create the pipeline description to describe vertex assembly.
    VertexDescription vertexDescription = {};

    struct Entry {
        uint8_t sourceBufferIndex = 0;
        uint8_t stride = 0;
        // maps to ->
        uint8_t bufferArgumentIndex = 0;

        Entry(uint8_t sourceBufferIndex, uint8_t stride, uint8_t bufferArgumentIndex)
                : sourceBufferIndex(sourceBufferIndex),
                  stride(stride),
                  bufferArgumentIndex(bufferArgumentIndex) {}
    };
    utils::FixedCapacityVector<Entry> bufferMapping;
};

struct MetalVertexBuffer : public HwVertexBuffer {
    MetalVertexBuffer(MetalContext& context,
            uint32_t vertexCount, uint32_t bufferCount, Handle<HwVertexBufferInfo> vbih);

    Handle<HwVertexBufferInfo> vbih;
    utils::FixedCapacityVector<MetalBuffer*> buffers;
};

struct MetalIndexBuffer : public HwIndexBuffer {
    MetalIndexBuffer(MetalContext& context, BufferUsage usage, uint8_t elementSize,
            uint32_t indexCount);

    MetalBuffer buffer;
};

struct MetalRenderPrimitive : public HwRenderPrimitive {
    MetalRenderPrimitive();
    void setBuffers(MetalVertexBufferInfo const* const vbi,
            MetalVertexBuffer* vertexBuffer, MetalIndexBuffer* indexBuffer);
    // The pointers to MetalVertexBuffer and MetalIndexBuffer are "weak".
    // The MetalVertexBuffer and MetalIndexBuffer must outlive the MetalRenderPrimitive.

    MetalVertexBuffer* vertexBuffer = nullptr;
    MetalIndexBuffer* indexBuffer = nullptr;
};

class MetalProgram : public HwProgram {
public:
    MetalProgram(MetalContext& context, Program&& program) noexcept;

    const MetalShaderCompiler::MetalFunctionBundle& getFunctions();
//    const Program::SamplerGroupInfo& getSamplerGroupInfo() { return samplerGroupInfo; }

private:
    void initialize();

//    Program::SamplerGroupInfo samplerGroupInfo;
    MetalContext& mContext;
    MetalShaderCompiler::MetalFunctionBundle mFunctionBundle;
    MetalShaderCompiler::program_token_t mToken;
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

class MetalTexture : public HwTexture {
public:
    MetalTexture(MetalContext& context, SamplerType target, uint8_t levels, TextureFormat format,
            uint8_t samples, uint32_t width, uint32_t height, uint32_t depth, TextureUsage usage,
            TextureSwizzle r, TextureSwizzle g, TextureSwizzle b, TextureSwizzle a)
            noexcept;

    // Constructor for importing an id<MTLTexture> outside of Filament.
    MetalTexture(MetalContext& context, SamplerType target, uint8_t levels, TextureFormat format,
            uint8_t samples, uint32_t width, uint32_t height, uint32_t depth, TextureUsage usage,
            id<MTLTexture> texture) noexcept;

    ~MetalTexture();

    // Returns an id<MTLTexture> suitable for reading in a shader, taking into account swizzle and
    // LOD clamping.
    id<MTLTexture> getMtlTextureForRead() noexcept;

    // Returns the id<MTLTexture> for attaching to a render pass.
    id<MTLTexture> getMtlTextureForWrite() noexcept {
        return texture;
    }

    void loadImage(uint32_t level, MTLRegion region, PixelBufferDescriptor& p) noexcept;
    void generateMipmaps() noexcept;

    // A texture starts out with none of its mip levels (also referred to as LODs) available for
    // reading. 4 actions update the range of LODs available:
    // - calling loadImage
    // - calling generateMipmaps
    // - using the texture as a render target attachment
    // - calling setMinMaxLevels
    // A texture's available mips are consistent throughout a render pass.
    void setLodRange(uint16_t minLevel, uint16_t maxLevel);
    void extendLodRangeTo(uint16_t level);

    static MTLPixelFormat decidePixelFormat(MetalContext* context, TextureFormat format);

    MetalContext& context;
    MetalExternalImage externalImage;

    // A "sidecar" texture used to implement automatic MSAA resolve.
    // This is created by MetalRenderTarget and stored here so it can be used with multiple
    // render targets.
    id<MTLTexture> msaaSidecar = nil;

    MTLPixelFormat devicePixelFormat;

    // Frees memory associated with this texture and marks it as "terminated".
    // Used to track "use after free" scenario.
    void terminate() noexcept;
    bool isTerminated() const noexcept { return terminated; }
    inline void checkUseAfterFree(const char* samplerGroupDebugName, size_t textureIndex) const {
        if (UTILS_LIKELY(!isTerminated())) {
            return;
        }
        NSString* reason =
                [NSString stringWithFormat:
                                  @"Filament Metal texture use after free, sampler group = "
                                  @"%s, texture index = %zu",
                          samplerGroupDebugName, textureIndex];
        NSException* useAfterFreeException =
                [NSException exceptionWithName:@"MetalTextureUseAfterFree"
                                        reason:reason
                                      userInfo:nil];
        [useAfterFreeException raise];
    }

private:
    void loadSlice(uint32_t level, MTLRegion region, uint32_t byteOffset, uint32_t slice,
            PixelBufferDescriptor const& data) noexcept;
    void loadWithCopyBuffer(uint32_t level, uint32_t slice, MTLRegion region, PixelBufferDescriptor const& data,
            const PixelBufferShape& shape);
    void loadWithBlit(uint32_t level, uint32_t slice, MTLRegion region, PixelBufferDescriptor const& data,
            const PixelBufferShape& shape);

    id<MTLTexture> texture = nil;

    // If non-nil, a swizzled texture view to use instead of "texture".
    // Filament swizzling only affects texture reads, so this should not be used when the texture is
    // bound as a render target attachment.
    id<MTLTexture> swizzledTextureView = nil;
    id<MTLTexture> lodTextureView = nil;

    uint16_t minLod = std::numeric_limits<uint16_t>::max();
    uint16_t maxLod = 0;

    bool terminated = false;
};

class MetalSamplerGroup : public HwSamplerGroup {
public:
    explicit MetalSamplerGroup(size_t size, utils::FixedSizeString<32> name) noexcept
        : size(size),
          debugName(name),
          textureHandles(size, Handle<HwTexture>()),
          textures(size, nil),
          samplers(size, nil) {}

    inline void setTextureHandle(size_t index, Handle<HwTexture> th) {
        assert_invariant(!finalized);
        textureHandles[index] = th;
    }

    // This method is only used for debugging, to ensure all texture handles are alive.
    const auto& getTextureHandles() const {
        return textureHandles;
    }

    // Encode a MTLTexture into this SamplerGroup at the given index.
    inline void setFinalizedTexture(size_t index, id<MTLTexture> t) {
        assert_invariant(!finalized);
        textures[index] = t;
    }

    // Encode a MTLSamplerState into this SamplerGroup at the given index.
    inline void setFinalizedSampler(size_t index, id<MTLSamplerState> s) {
        assert_invariant(!finalized);
        samplers[index] = s;
    }

    // A SamplerGroup is "finalized" when all of its textures have been set and is ready for use in
    // a draw call.
    // Once a SamplerGroup is finalized, it must be reset or mutated to be written into again.
    void finalize();
    bool isFinalized() const noexcept { return finalized; }

    // Both of these methods "unfinalize" a SamplerGroup, allowing it to be updated via calls to
    // setFinalizedTexture or setFinalizedSampler. The difference is that when reset is called, all
    // the samplers/textures must be rebound. The MTLArgumentEncoder must be specified, in case
    // the texture types have changed.
    // Mutate re-encodes the current set of samplers/textures into the new argument
    // buffer.
    void reset(id<MTLCommandBuffer> cmdBuffer, id<MTLArgumentEncoder> e, id<MTLDevice> device);
    void mutate(id<MTLCommandBuffer> cmdBuffer);

    id<MTLBuffer> getArgumentBuffer() const {
        assert_invariant(finalized);
        return argBuffer->getCurrentAllocation().first;
    }

    NSUInteger getArgumentBufferOffset() const {
        return argBuffer->getCurrentAllocation().second;
    }

    inline std::pair<Handle<HwTexture>, id<MTLTexture>> getFinalizedTexture(size_t index) {
        return {textureHandles[index], textures[index]};
    }

    // Calls the Metal useResource:usage:stages: method for all the textures in this SamplerGroup.
    void useResources(id<MTLRenderCommandEncoder> renderPassEncoder);

    size_t size;
    utils::FixedSizeString<32> debugName;

public:

    // These vectors are kept in sync with one another.
    utils::FixedCapacityVector<Handle<HwTexture>> textureHandles;
    utils::FixedCapacityVector<id<MTLTexture>> textures;
    utils::FixedCapacityVector<id<MTLSamplerState>> samplers;

    id<MTLArgumentEncoder> encoder;

    std::unique_ptr<MetalRingBuffer> argBuffer = nullptr;

    bool finalized = false;
};

class MetalRenderTarget : public HwRenderTarget {
public:

    class Attachment {
    public:

        friend class MetalRenderTarget;

        Attachment() = default;
        Attachment(MetalTexture* metalTexture, uint8_t level = 0, uint16_t layer = 0) :
                level(level), layer(layer),
                texture(metalTexture->getMtlTextureForWrite()),
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
            Attachment colorAttachments[MRT::MAX_SUPPORTED_RENDER_TARGET_COUNT],
            Attachment depthAttachment, Attachment stencilAttachment);
    explicit MetalRenderTarget(MetalContext* context)
            : HwRenderTarget(0, 0), context(context), defaultRenderTarget(true) {}

    void setUpRenderPassAttachments(MTLRenderPassDescriptor* descriptor, const RenderPassParams& params);

    MTLViewport getViewportFromClientViewport(
            Viewport rect, float depthRangeNear, float depthRangeFar) {
        const int32_t height = int32_t(getAttachmentSize().y);
        assert_invariant(height > 0);

        // Metal's viewport coordinates have (0, 0) at the top-left, but Filament's have (0, 0) at
        // bottom-left.
        return {double(rect.left),
                double(height - rect.bottom - int32_t(rect.height)),
                double(rect.width), double(rect.height),
                double(depthRangeNear), double(depthRangeFar)};
    }

    MTLRegion getRegionFromClientRect(Viewport rect) {
        const uint32_t height = getAttachmentSize().y;
        assert_invariant(height > 0);

        // Convert the Filament rect into Metal texture coordinates, taking into account Metal's
        // inverted texture space. Note that the underlying Texture could be larger than the
        // RenderTarget. Metal's texture coordinates have (0, 0) at the top-left of the texture, but
        // Filament's coordinates have (0, 0) at bottom-left.
        return MTLRegionMake2D((NSUInteger)rect.left,
                std::max(height - (int64_t) rect.bottom - rect.height, (int64_t) 0),
                rect.width, rect.height);
    }

    math::uint2 getAttachmentSize() noexcept;

    bool isDefaultRenderTarget() const { return defaultRenderTarget; }
    uint8_t getSamples() const { return samples; }

    Attachment getDrawColorAttachment(size_t index);
    Attachment getReadColorAttachment(size_t index);
    Attachment getDepthAttachment();
    Attachment getStencilAttachment();

private:

    static MTLLoadAction getLoadAction(const RenderPassParams& params, TargetBufferFlags buffer);
    static MTLStoreAction getStoreAction(const RenderPassParams& params, TargetBufferFlags buffer);
    id<MTLTexture> createMultisampledTexture(MTLPixelFormat format, uint32_t width, uint32_t height, uint8_t samples) const;

    MetalContext* context;
    bool defaultRenderTarget = false;
    uint8_t samples = 1;

    Attachment color[MRT::MAX_SUPPORTED_RENDER_TARGET_COUNT] = {};
    Attachment depth = {};
    Attachment stencil = {};
    math::uint2 attachmentSize = {};
};

// MetalFence is used to implement both Fences and Syncs.
class MetalFence : public HwFence {
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

class MetalDescriptorSetLayout : public HwDescriptorSetLayout {
public:
    MetalDescriptorSetLayout(DescriptorSetLayout&& layout) noexcept;

    const auto& getBindings() const noexcept { return mLayout.bindings; }

    size_t getDynamicOffsetCount() const noexcept { return mDynamicOffsetCount; }

    id<MTLArgumentEncoder> getArgumentEncoderForTextureTypes(
            id<MTLDevice> device, utils::FixedCapacityVector<MTLTextureType> const& textureTypes);

private:
    id<MTLArgumentEncoder> getArgumentEncoderForTextureTypesSlow(
            id<MTLDevice> device, utils::FixedCapacityVector<MTLTextureType> const& textureTypes);

    DescriptorSetLayout mLayout;
    size_t mDynamicOffsetCount = 0;
    id<MTLArgumentEncoder> mCachedArgumentEncoder = nil;
    utils::FixedCapacityVector<MTLTextureType> mCachedTextureTypes;
};

struct MetalDescriptorSet : public HwDescriptorSet {
    MetalDescriptorSet(MetalDescriptorSetLayout* layout) noexcept;
    MetalDescriptorSetLayout* layout;

    void finalize(MetalDriver* driver) {
        [driver->mContext->currentRenderPassEncoder useResource:driver->mContext->emptyBuffer
                                                          usage:MTLResourceUsageRead];
        [driver->mContext->currentRenderPassEncoder useResource:getOrCreateEmptyTexture(driver->mContext)
                                                          usage:MTLResourceUsageRead];
        auto const& bindings = this->layout->getBindings();
        for (auto const& binding : bindings) {
            switch (binding.type) {
                case DescriptorType::UNIFORM_BUFFER:
                case DescriptorType::SHADER_STORAGE_BUFFER: {
                    auto found = buffers.find(binding.binding);
                    if (found == buffers.end()) {
                        continue;
                    }

                    auto const& bufferBinding = buffers[binding.binding];
                    auto* metalBuffer = driver->handle_cast<MetalBufferObject>(bufferBinding.buffer)
                                                ->getBuffer()
                                                ->getGpuBufferForDraw();
                    [driver->mContext->currentRenderPassEncoder useResource:metalBuffer
                                                                      usage:MTLResourceUsageRead];
                    break;
                }
                case DescriptorType::SAMPLER: {
                    auto found = textures.find(binding.binding);
                    if (found == textures.end()) {
                        continue;
                    }

                    auto const& textureBinding = textures[binding.binding];
                    auto* texture = driver->handle_cast<MetalTexture>(textureBinding.texture)
                                            ->getMtlTextureForRead();
                    [driver->mContext->currentRenderPassEncoder useResource:texture
                                                                      usage:MTLResourceUsageRead];
                    break;
                }
                case DescriptorType::INPUT_ATTACHMENT:
                    break;
            }
        }
    }

    id<MTLBuffer> finalizeAndGetBuffer(MetalDriver* driver) {
        if (buffer) {
            return buffer;
        }

        // Map all the texture bindings to their respective texture types.
        auto const& bindings = layout->getBindings();
        auto textureTypes = utils::FixedCapacityVector<MTLTextureType>::with_capacity(bindings.size());
        for (auto const& binding : bindings) {
            MTLTextureType textureType = MTLTextureType2D;
            if (auto found = textures.find(binding.binding); found != textures.end()) {
                auto const& textureBinding = textures[binding.binding];
                auto* texture = driver->handle_cast<MetalTexture>(textureBinding.texture)
                                    ->getMtlTextureForRead();
                textureType = texture.textureType;
            }
            textureTypes.push_back(textureType);
        }

        MetalContext const& context = *driver->mContext;

        id<MTLArgumentEncoder> encoder =
                layout->getArgumentEncoderForTextureTypes(context.device, textureTypes);

        buffer = [context.device newBufferWithLength:encoder.encodedLength
                                             options:MTLResourceStorageModeShared];
        [encoder setArgumentBuffer:buffer offset:0];

        for (auto const& binding : bindings) {
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
                    auto* metalBuffer = driver->handle_cast<MetalBufferObject>(bufferBinding.buffer)
                                                ->getBuffer()
                                                ->getGpuBufferForDraw();
                    [encoder setBuffer:metalBuffer
                                offset:bufferBinding.offset
                               atIndex:binding.binding * 2];
                    break;
                }
                case DescriptorType::SAMPLER: {
                    auto found = textures.find(binding.binding);
                    if (found == textures.end()) {
                        [encoder setTexture:driver->mContext->emptyTexture
                                    atIndex:binding.binding * 2];
                        id<MTLSamplerState> sampler =
                                driver->mContext->samplerStateCache.getOrCreateState({});
                        [encoder setSamplerState:sampler
                                         atIndex:binding.binding * 2 + 1];
                        continue;
                    }

                    auto const& textureBinding = textures[binding.binding];
                    auto* texture = driver->handle_cast<MetalTexture>(textureBinding.texture)
                                            ->getMtlTextureForRead();
                    [encoder setTexture:texture atIndex:binding.binding * 2];
                    SamplerState samplerState {
                            .samplerParams = textureBinding.sampler
                    };
                    id<MTLSamplerState> sampler = driver->mContext->samplerStateCache.getOrCreateState(samplerState);
                    [encoder setSamplerState:sampler
                                     atIndex:binding.binding * 2 + 1];
                    break;
                }
                case DescriptorType::INPUT_ATTACHMENT:
                    assert_invariant(false);
                    break;
            }
        }

        return buffer;
    }

    struct BufferBinding {
        BufferObjectHandle buffer;
        uint32_t offset;
        uint32_t size;
    };
    struct TextureBinding {
        TextureHandle texture;
        SamplerParams sampler;
    };
    tsl::robin_map<descriptor_binding_t, BufferBinding> buffers;
    tsl::robin_map<descriptor_binding_t, TextureBinding> textures;

    id<MTLBuffer> buffer = nil;
};

} // namespace backend
} // namespace filament

#endif //TNT_FILAMENT_DRIVER_METALHANDLES_H
