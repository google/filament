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

#include "MetalContext.h"
#include "MetalEnums.h"
#include "MetalExternalImage.h"
#include "MetalState.h" // for MetalState::VertexDescription
#include "TextureReshaper.h"

#include <utils/Panic.h>

#include <vector>

namespace filament {
namespace backend {
namespace metal {

struct MetalSwapChain : public HwSwapChain {
    MetalSwapChain(id<MTLDevice> device, CAMetalLayer* nativeWindow);
    ~MetalSwapChain();

    CAMetalLayer* layer = nullptr;
    id<MTLTexture> depthTexture = nullptr;
    NSUInteger surfaceHeight = 0;
};

struct MetalVertexBuffer : public HwVertexBuffer {
    MetalVertexBuffer(id<MTLDevice> device, uint8_t bufferCount, uint8_t attributeCount,
            uint32_t vertexCount, AttributeArray const& attributes);
    ~MetalVertexBuffer();

    std::vector<id<MTLBuffer>> buffers;
};

struct MetalIndexBuffer : public HwIndexBuffer {
    MetalIndexBuffer(id<MTLDevice> device, uint8_t elementSize, uint32_t indexCount);
    ~MetalIndexBuffer();

    id<MTLBuffer> buffer;
};

class MetalUniformBuffer : public HwUniformBuffer {
public:
    MetalUniformBuffer(MetalContext& context, size_t size);
    ~MetalUniformBuffer();

    size_t getSize() const { return size; }

    /**
     * Update the uniform with data inside src. Potentially allocates a new buffer allocation to
     * hold the bytes which will be released when the current frame is finished.
     */
    void copyIntoBuffer(void* src, size_t size);

    /**
     * Denotes that this uniform is used for a draw call ensuring that its allocation remains valid
     * until the end of the current frame.
     *
     * @return The MTLBuffer representing the current state of the uniform to bind, or nil if there
     * is no device allocation.
     */
    id<MTLBuffer> getGpuBufferForDraw();

    /**
     * @return A pointer to the CPU buffer holding the uniform data or nullptr if there isn't one.
     */
    void* getCpuBuffer() const;

private:
    size_t size = 0;
    const MetalBufferPoolEntry* bufferPoolEntry = nullptr;
    void* cpuBuffer = nullptr;
    MetalContext& context;
};

struct MetalRenderPrimitive : public HwRenderPrimitive {
    void setBuffers(MetalVertexBuffer* vertexBuffer, MetalIndexBuffer* indexBuffer,
            uint32_t enabledAttributes);
    // The pointers to MetalVertexBuffer, MetalIndexBuffer, and id<MTLBuffer> are "weak".
    // The MetalVertexBuffer and MetalIndexBuffer must outlive the MetalRenderPrimitive.

    MetalVertexBuffer* vertexBuffer = nullptr;
    MetalIndexBuffer* indexBuffer = nullptr;

    // This struct is used to create the pipeline description to describe vertex assembly.
    VertexDescription vertexDescription = {};

    std::vector<id<MTLBuffer>> buffers;
    std::vector<NSUInteger> offsets;
};

struct MetalProgram : public HwProgram {
    MetalProgram(id<MTLDevice> device, const Program& program) noexcept;
    ~MetalProgram();

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
            uint32_t height, PixelBufferDescriptor& data) noexcept;
    void loadCubeImage(const PixelBufferDescriptor& data, const FaceOffsets& faceOffsets,
            int miplevel);

    MetalContext& context;
    MetalExternalImage externalImage;
    id<MTLTexture> texture = nil;
    uint8_t bytesPerPixel;
    TextureReshaper reshaper;
};

struct MetalSamplerGroup : public HwSamplerGroup {
    explicit MetalSamplerGroup(size_t size) : HwSamplerGroup(size) {}
};

class MetalRenderTarget : public HwRenderTarget {
public:
    MetalRenderTarget(MetalContext* context, uint32_t width, uint32_t height, uint8_t samples,
            TextureFormat format, id<MTLTexture> color, id<MTLTexture> depth);
    explicit MetalRenderTarget(MetalContext* context)
            : HwRenderTarget(0, 0), context(context), defaultRenderTarget(true) {}
    ~MetalRenderTarget();

    bool isDefaultRenderTarget() const { return defaultRenderTarget; }
    bool isMultisampled() const { return samples > 1; }
    uint8_t getSamples() const { return samples; }

    id<MTLTexture> getColor();
    id<MTLTexture> getColorResolve();
    id<MTLTexture> getDepth();
    id<MTLTexture> getDepthResolve();

private:
    static id<MTLTexture> createMultisampledTexture(id<MTLDevice> device, TextureFormat format,
            uint32_t width, uint32_t height, uint8_t samples);

    MetalContext* context;
    id<MTLTexture> color = nil;
    id<MTLTexture> depth = nil;
    bool defaultRenderTarget = false;
    uint8_t samples = 1;

    // These textures are only used if this render target is multisampled.
    id<MTLTexture> multisampledColor = nil;
    id<MTLTexture> multisampledDepth = nil;

};

} // namespace metal
} // namespace backend
} // namespace filament

#endif //TNT_FILAMENT_DRIVER_METALHANDLES_H
