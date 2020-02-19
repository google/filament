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

#include <condition_variable>
#include <memory>
#include <vector>

namespace filament {
namespace backend {
namespace metal {

struct MetalSwapChain : public HwSwapChain {
    MetalSwapChain(id<MTLDevice> device, CAMetalLayer* nativeWindow);

    // Instantiate a headless SwapChain.
    MetalSwapChain(int32_t width, int32_t height);

    bool isHeadless() { return layer == nullptr; }

    CAMetalLayer* layer = nullptr;
    NSUInteger surfaceWidth = 0;
    NSUInteger surfaceHeight = 0;
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
};

struct MetalTexture : public HwTexture {
    MetalTexture(MetalContext& context, backend::SamplerType target, uint8_t levels,
            TextureFormat format, uint8_t samples, uint32_t width, uint32_t height, uint32_t depth,
            TextureUsage usage) noexcept;
    ~MetalTexture();

    void load2DImage(uint32_t level, uint32_t xoffset, uint32_t yoffset, uint32_t width,
            uint32_t height, PixelBufferDescriptor&& p) noexcept;
    void loadCubeImage(const FaceOffsets& faceOffsets, int miplevel, PixelBufferDescriptor&& p);
    void loadSlice(uint32_t level, uint32_t xoffset, uint32_t yoffset, uint32_t width,
            uint32_t height, uint32_t byteOffset, uint32_t slice,
            PixelBufferDescriptor& data, id<MTLBlitCommandEncoder> blitCommandEncoder,
            id<MTLCommandBuffer> blitCommandBuffer) noexcept;

    MetalContext& context;
    MetalExternalImage externalImage;
    id<MTLTexture> texture = nil;
    uint8_t bytesPerElement; // The number of bytes per pixel, or block (for compressed texture formats).
    uint8_t blockWidth; // The number of horizontal pixels per block (only for compressed texture formats).
    uint8_t blockHeight; // The number of vertical pixels per block (only for compressed texture formats).
    TextureReshaper reshaper;
    MTLPixelFormat metalPixelFormat;
};

struct MetalSamplerGroup : public HwSamplerGroup {
    explicit MetalSamplerGroup(size_t size) : HwSamplerGroup(size) {}
};

class MetalRenderTarget : public HwRenderTarget {
public:
    MetalRenderTarget(MetalContext* context, uint32_t width, uint32_t height, uint8_t samples,
            id<MTLTexture> color, id<MTLTexture> depth, uint8_t colorLevel, uint8_t depthLevel);
    explicit MetalRenderTarget(MetalContext* context)
            : HwRenderTarget(0, 0), context(context), defaultRenderTarget(true) {}

    bool isDefaultRenderTarget() const { return defaultRenderTarget; }
    uint8_t getSamples() const { return samples; }
    MTLLoadAction getLoadAction(const RenderPassParams& params, TargetBufferFlags buffer);
    MTLStoreAction getStoreAction(const RenderPassParams& params, TargetBufferFlags buffer);

    id<MTLTexture> getColor();
    id<MTLTexture> getColorResolve();
    id<MTLTexture> getDepth();
    id<MTLTexture> getDepthResolve();
    id<MTLTexture> getBlitColorSource();
    id<MTLTexture> getBlitDepthSource();
    uint8_t getColorLevel() { return colorLevel; }
    uint8_t getDepthLevel() { return depthLevel; }

private:
    static id<MTLTexture> createMultisampledTexture(id<MTLDevice> device, MTLPixelFormat format,
            uint32_t width, uint32_t height, uint8_t samples);

    MetalContext* context;
    bool defaultRenderTarget = false;
    uint8_t samples = 1;
    uint8_t colorLevel = 0;
    uint8_t depthLevel = 0;

    id<MTLTexture> color = nil;
    id<MTLTexture> depth = nil;
    id<MTLTexture> multisampledColor = nil;
    id<MTLTexture> multisampledDepth = nil;
};

class MetalFence : public HwFence {
public:

    MetalFence(MetalContext& context);

    FenceStatus wait(uint64_t timeoutNs);

private:

    std::shared_ptr<std::condition_variable> cv;
    std::mutex mutex;

    // MTLSharedEvent is only available on macOS 10.14 and iOS 12.0 and above.
    // The availability annotation ensures we wrap all usages of event in an @availability check.
    API_AVAILABLE(macos(10.14), ios(12.0))
    id<MTLSharedEvent> event;

    uint64_t value;
};

} // namespace metal
} // namespace backend
} // namespace filament

#endif //TNT_FILAMENT_DRIVER_METALHANDLES_H
