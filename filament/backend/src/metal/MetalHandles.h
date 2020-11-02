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
#include "TextureReshaper.h"

#include <utils/Panic.h>

#include <atomic>
#include <condition_variable>
#include <memory>
#include <vector>

namespace filament {
namespace backend {
namespace metal {

class MetalSwapChain : public HwSwapChain {
public:

    // Instantiate a SwapChain from a CAMetalLayer
    MetalSwapChain(MetalContext& context, CAMetalLayer* nativeWindow, uint64_t flags);

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

    void setFrameFinishedCallback(FrameFinishedCallback callback, void* user);

    // For CAMetalLayer-backed SwapChains, presents the drawable or schedules a
    // FrameFinishedCallback.
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

    void scheduleFrameFinishedCallback();

    MetalContext& context;
    id<CAMetalDrawable> drawable = nil;
    id<MTLTexture> depthTexture = nil;
    id<MTLTexture> headlessDrawable = nil;
    NSUInteger headlessWidth;
    NSUInteger headlessHeight;
    CAMetalLayer* layer = nullptr;
    SwapChainType type;
    FrameFinishedCallback frameFinishedCallback = nullptr;
    void* frameFinishedUserData = nullptr;
};

struct MetalVertexBuffer : public HwVertexBuffer {
    MetalVertexBuffer(MetalContext& context, uint8_t bufferCount, uint8_t attributeCount,
            uint32_t vertexCount, AttributeArray const& attributes);
    ~MetalVertexBuffer();

    std::vector<MetalBuffer*> buffers;
};

struct MetalIndexBuffer : public HwIndexBuffer {
    MetalIndexBuffer(MetalContext& context, uint8_t elementSize, uint32_t indexCount);

    MetalBuffer buffer;
};

struct MetalUniformBuffer : public HwUniformBuffer {
    MetalUniformBuffer(MetalContext& context, size_t size);

    MetalBuffer buffer;
};

struct MetalRenderPrimitive : public HwRenderPrimitive {
    void setBuffers(MetalVertexBuffer* vertexBuffer, MetalIndexBuffer* indexBuffer,
            uint32_t enabledAttributes);
    // The pointers to MetalVertexBuffer and MetalIndexBuffer are "weak".
    // The MetalVertexBuffer and MetalIndexBuffer must outlive the MetalRenderPrimitive.

    MetalVertexBuffer* vertexBuffer = nullptr;
    MetalIndexBuffer* indexBuffer = nullptr;

    // This struct is used to create the pipeline description to describe vertex assembly.
    VertexDescription vertexDescription = {};

    std::vector<MetalBuffer*> buffers;
    std::vector<NSUInteger> offsets;
};

struct MetalProgram : public HwProgram {
    MetalProgram(id<MTLDevice> device, const Program& program) noexcept;

    id<MTLFunction> vertexFunction;
    id<MTLFunction> fragmentFunction;
    Program::SamplerGroupInfo samplerGroupInfo;
    bool isValid = false;
};

struct MetalTexture : public HwTexture {
    MetalTexture(MetalContext& context, SamplerType target, uint8_t levels, TextureFormat format,
            uint8_t samples, uint32_t width, uint32_t height, uint32_t depth, TextureUsage usage)
            noexcept;

    // Constructor for importing an id<MTLTexture> outside of Filament.
    MetalTexture(MetalContext& context, SamplerType target, uint8_t levels, TextureFormat format,
            uint8_t samples, uint32_t width, uint32_t height, uint32_t depth, TextureUsage usage,
            id<MTLTexture> texture) noexcept;

    ~MetalTexture();

    void load2DImage(uint32_t level, uint32_t xoffset, uint32_t yoffset, uint32_t width,
            uint32_t height, PixelBufferDescriptor& p) noexcept;
    void load3DImage(uint32_t level, uint32_t xoffset, uint32_t yoffset, uint32_t zoffset,
            uint32_t width, uint32_t height, uint32_t depth, PixelBufferDescriptor& p) noexcept;
    void loadCubeImage(const FaceOffsets& faceOffsets, int miplevel, PixelBufferDescriptor& p);
    void loadSlice(uint32_t level, uint32_t xoffset, uint32_t yoffset, uint32_t zoffset,
            uint32_t width, uint32_t height, uint32_t depth, uint32_t byteOffset, uint32_t slice,
            PixelBufferDescriptor& data, id<MTLBlitCommandEncoder> blitCommandEncoder,
            id<MTLCommandBuffer> blitCommandBuffer) noexcept;
    void updateLodRange(uint32_t level);

    MetalContext& context;
    MetalExternalImage externalImage;
    id<MTLTexture> texture = nil;
    uint8_t bytesPerElement; // The number of bytes per pixel, or block (for compressed texture formats).
    uint8_t blockWidth; // The number of horizontal pixels per block (only for compressed texture formats).
    uint8_t blockHeight; // The number of vertical pixels per block (only for compressed texture formats).
    TextureReshaper reshaper;
    MTLPixelFormat metalPixelFormat;
    uint32_t minLod = UINT_MAX;
    uint32_t maxLod = 0;
};

struct MetalSamplerGroup : public HwSamplerGroup {
    explicit MetalSamplerGroup(size_t size) : HwSamplerGroup(size) {}
};

class MetalRenderTarget : public HwRenderTarget {
public:

    struct Attachment {
        id<MTLTexture> texture = nil;
        uint8_t level = 0;
        uint16_t layer = 0;

        explicit operator bool() const {
            return texture != nil;
        }
    };

    MetalRenderTarget(MetalContext* context, uint32_t width, uint32_t height, uint8_t samples,
            Attachment colorAttachments[MRT::TARGET_COUNT], Attachment depthAttachment);
    explicit MetalRenderTarget(MetalContext* context)
            : HwRenderTarget(0, 0), context(context), defaultRenderTarget(true) {}

    void setUpRenderPassAttachments(MTLRenderPassDescriptor* descriptor, const RenderPassParams& params);

    bool isDefaultRenderTarget() const { return defaultRenderTarget; }
    uint8_t getSamples() const { return samples; }

    Attachment getColorAttachment(size_t index);
    Attachment getDepthAttachment();

private:

    static MTLLoadAction getLoadAction(const RenderPassParams& params, TargetBufferFlags buffer);
    static MTLStoreAction getStoreAction(const RenderPassParams& params, TargetBufferFlags buffer);
    static id<MTLTexture> createMultisampledTexture(id<MTLDevice> device, MTLPixelFormat format,
            uint32_t width, uint32_t height, uint8_t samples);

    MetalContext* context;
    bool defaultRenderTarget = false;
    uint8_t samples = 1;

    Attachment color[MRT::TARGET_COUNT] = {};
    Attachment depth = {};

    // "Sidecar" textures used to implement automatic MSAA resolve.
    id<MTLTexture> multisampledColor[MRT::TARGET_COUNT] = { 0 };
    id<MTLTexture> multisampledDepth = nil;
};

class MetalFence : public HwFence {
public:

    // MetalFence is special, as it gets constructed on the Filament thread. We must delay inserting
    // the fence into the command stream until encode() is called (on the driver thread).
    MetalFence(MetalContext& context);

    // Inserts this fence into the current command buffer. Must be called from the driver thread.
    void encode();

    FenceStatus wait(uint64_t timeoutNs);

    API_AVAILABLE(macos(10.14), ios(12.0))
    typedef void (^MetalFenceSignalBlock)(id<MTLSharedEvent>, uint64_t value);

    API_AVAILABLE(macos(10.14), ios(12.0))
    void onSignal(MetalFenceSignalBlock block);

private:

    MetalContext& context;

    struct State {
        FenceStatus status { FenceStatus::TIMEOUT_EXPIRED };
        std::condition_variable cv;
        std::mutex mutex;
    };
    std::shared_ptr<State> state { std::make_shared<State>() };

    // MTLSharedEvent is only available on macOS 10.14 and iOS 12.0 and above.
    // The availability annotation ensures we wrap all usages of event in an @availability check.
    API_AVAILABLE(macos(10.14), ios(12.0))
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

} // namespace metal
} // namespace backend
} // namespace filament

#endif //TNT_FILAMENT_DRIVER_METALHANDLES_H
