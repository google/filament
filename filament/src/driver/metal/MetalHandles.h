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

#include "driver/metal/MetalDriver.h"

#include <Metal/Metal.h>
#include <QuartzCore/QuartzCore.h> // for CAMetalLayer

#include "MetalState.h" // for MetalState::VertexDescription
#include "MetalEnums.h"

#include <utils/Panic.h>

#include <vector>

namespace filament {
namespace driver {
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
            uint32_t vertexCount, Driver::AttributeArray const& attributes);
    ~MetalVertexBuffer();

    std::vector<id<MTLBuffer>> buffers;
};

struct MetalIndexBuffer : public HwIndexBuffer {
    MetalIndexBuffer(id<MTLDevice> device, uint8_t elementSize, uint32_t indexCount);
    ~MetalIndexBuffer();

    id<MTLBuffer> buffer;
};

struct MetalUniformBuffer : public HwUniformBuffer {
    // TODO: We aren't implementing triple-buffering for uniform buffers yet- i.e., a single
    // Filament uniform buffer maps to a single MetalUniformBuffer. This could cause access
    // conflicts between CPU / GPU.

    MetalUniformBuffer(id<MTLDevice> device, size_t size);
    ~MetalUniformBuffer();

    void copyIntoBuffer(void* src, size_t size);

    size_t size = 0;

    id <MTLBuffer> buffer;
    void* cpuBuffer;
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
    SamplerBindingMap samplerBindings;
};

struct MetalTexture : public HwTexture {
    MetalTexture(id<MTLDevice> device, driver::SamplerType target, uint8_t levels,
            TextureFormat format, uint8_t samples, uint32_t width, uint32_t height, uint32_t depth,
            TextureUsage usage) noexcept;
    ~MetalTexture();

    void load2DImage(uint32_t level, uint32_t xoffset, uint32_t yoffset, uint32_t width,
            uint32_t height, Driver::PixelBufferDescriptor& data) noexcept;
    void loadCubeImage(const PixelBufferDescriptor& data, const FaceOffsets& faceOffsets,
            int miplevel);

    id<MTLTexture> texture;
    uint8_t bytesPerPixel;
};

struct MetalSamplerBuffer : public HwSamplerBuffer {
    explicit MetalSamplerBuffer(size_t size) : HwSamplerBuffer(size) {}
};

struct MetalRenderTarget : public HwRenderTarget {
    MetalRenderTarget(uint32_t width, uint32_t height) : HwRenderTarget(width, height) {}
    MetalRenderTarget() : HwRenderTarget(0, 0), isDefaultRenderTarget(true) {}
    ~MetalRenderTarget();

    bool isDefaultRenderTarget = false;
    id<MTLTexture> color = nil;
    id<MTLTexture> depth = nil;
};

} // namespace metal
} // namespace driver
} // namespace filament

#endif //TNT_FILAMENT_DRIVER_METALHANDLES_H
