/*
 * Copyright (C) 2025 The Android Open Source Project
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

#include "BackendTest.h"

#include "ImageExpectations.h"
#include "Lifetimes.h"
#include "Shader.h"
#include "SharedShaders.h"
#include "SharedShadersConstants.h"
#include "Skip.h"
#include "Workarounds.h"

#include <backend/BufferDescriptor.h>
#include <backend/DriverEnums.h>
#include <backend/Handle.h>
#include <backend/PipelineState.h>

#include <math/vec2.h>
#include <math/vec4.h>

#include <gtest/gtest.h>

#include <array>
#include <cstdint>
#include <cstddef>
#include <new>
#include <tuple>
#include <utility>

#include <stdlib.h>
#include <string.h>

namespace test {

using namespace filament;
using namespace filament::backend;

class MemoryMappedTest : public BackendTest {
protected:
    RenderTargetHandle makeRenderTarget(Cleanup& cleanup) {
        auto& api = getDriverApi();
        auto colorTexture = cleanup.add(
                api.createTexture(SamplerType::SAMPLER_2D, 1, TextureFormat::RGBA8, 1,
                        screenWidth(), screenHeight(), 1,
                        TextureUsage::COLOR_ATTACHMENT | TextureUsage::SAMPLEABLE TEXTURE_USAGE_READ_PIXELS));
        auto renderTarget = cleanup.add(api.createRenderTarget(TargetBufferFlags::COLOR0,
                screenWidth(), screenHeight(), 1, 1, { { colorTexture } }, {}, {}));
        return renderTarget;
    }

    std::tuple<VertexBufferInfoHandle, VertexBufferHandle, IndexBufferHandle>
    setupGeometryBuffer(Cleanup& cleanup,
            uint32_t const count, uint8_t const stride, uint32_t const baseVertex,
            BufferObjectHandle buffer) {
        auto& api = getDriverApi();
        AttributeArray attributes = {};
        attributes[0] = { .offset = 0, .stride = stride, .buffer = 0, .type = ElementType::FLOAT2 };
        auto vbih = cleanup.add(api.createVertexBufferInfo(1, 1, attributes));
        // The vertex buffer must be large enough to hold the "garbage" vertices created by the offset.
        auto vbh = cleanup.add(api.createVertexBuffer(count + baseVertex, vbih));
        api.setVertexBufferObject(vbh, 0, buffer);

        auto* const indices = new(std::nothrow)uint16_t[count];
        for (size_t i = 0; i < count; i++) {
            indices[i] = i + baseVertex;
        }

        auto ibh = cleanup.add(
                api.createIndexBuffer(ElementType::USHORT, count, BufferUsage::STATIC));

        api.updateIndexBuffer(ibh, {
                indices, count * sizeof(uint16_t),
                [](void* b, size_t, void*) {
                    delete [] static_cast<uint16_t*>(b);
                }, indices }, 0);

        return { vbih, vbh, ibh };
    }

    std::pair<Shader, DescriptorSetHandle> getSimpleShader(Cleanup& cleanup,
            math::float4 const& color) {
        auto& api = getDriverApi();
        Shader const shader = SharedShaders::makeShader(getDriverApi(), cleanup,
                ShaderRequest{
                    .mVertexType = VertexShaderType::Simple,
                    .mFragmentType = FragmentShaderType::SolidColored,
                    .mUniformType = ShaderUniformType::Simple,
                });

        auto descSet = shader.createDescriptorSet(api);
        UniformBindingConfig uboBindingConfig = {
            .descriptorSet = descSet,
        };
        auto const ubuffer = cleanup.add(api.createBufferObject(sizeof(SimpleMaterialParams),
                BufferObjectBinding::UNIFORM, BufferUsage::DYNAMIC_BIT));
        // This will bind the UBO to descSet and also bind descSet.  But we also need to manually
        // bind descset every begin/end frame.
        shader.bindUniform<SimpleMaterialParams>(api, ubuffer, uboBindingConfig);
        shader.uploadUniform(api, ubuffer, SimpleMaterialParams{ .color = color });
        return { shader, descSet };
    }

    void copyData(const void* data, size_t const size, size_t const offset,
            MemoryMappedBufferHandle const& mmb, int& callbacksExecuted) {
        auto& api = getDriverApi();
        void* p = malloc(size);
        memcpy(p, data, size);
        api.copyToMemoryMappedBuffer(mmb, offset, {
            p, size, [](void* buffer, size_t, void* user) {
                free(buffer);
                (*static_cast<int*>(user))++;
            }, &callbacksExecuted
        });
    }

    void render(uint32_t count,
            int64_t const frame,
            SwapChainHandle swapChain,
            RenderTargetHandle renderTarget,
            RenderPrimitiveHandle renderPrimitive,
            DescriptorSetHandle descSet,
            PipelineState const& state) {
        auto& api = getDriverApi();
        RenderPassParams params = getClearColorRenderPass({0,0,0,1});
        params.viewport = getFullViewport();

        api.beginFrame(frame, 0, 0);
        api.beginRenderPass(renderTarget, params);
        api.bindPipeline(state);
        // The binding index is always 0 for the simple shaders we're using.
        api.bindDescriptorSet(descSet, 0, {});
        api.bindRenderPrimitive(renderPrimitive);
        api.draw2(0, count, 1);
        api.endRenderPass();

        api.commit(swapChain);
        api.endFrame(0);
        flushAndWait();
    }

    void runOffsetRenderTest(size_t const mapOffset,
            size_t const copyOffset,
            const math::float4& color,
            const char* screenshotName) {

        SKIP_IF(Backend::WEBGPU, "HwMemoryMappedBuffer APIs not yet implemented");

        auto& api = getDriverApi();
        Cleanup cleanup(api);

        auto const swapChain = cleanup.add(createSwapChain());
        api.makeCurrent(swapChain, swapChain);

        auto const renderTarget = makeRenderTarget(cleanup);

        constexpr std::array<math::float2, 3> vertices = {{
            {-0.25f, -0.25f},
            { 0.25f, -0.25f},
            {-0.25f,  0.25f},
        }};
        constexpr size_t vertexDataSize = vertices.size() * sizeof(math::float2);
        constexpr size_t stride = sizeof(math::float2);

        // The total offset is the sum of the map and copy offsets.
        const size_t totalOffset = mapOffset + copyOffset;

        // Calculate how many vertices we need to skip to account for the total offset.
        const uint32_t baseVertex = totalOffset / stride;

        // Create a buffer large enough to hold the offset and the vertex data.
        auto const bufferObject = cleanup.add(api.createBufferObject(vertexDataSize + totalOffset,
                BufferObjectBinding::VERTEX, BufferUsage::SHARED_WRITE_BIT));

        int callbackExecuted = 0;
        // Map the buffer with a specific offset.
        MemoryMappedBufferHandle const memoryMappedBuffer =
                api.mapBuffer(bufferObject, mapOffset, vertexDataSize + copyOffset,
                MapBufferAccessFlags::WRITE_BIT, utils::ImmutableCString{ screenshotName });

        copyData(vertices.data(), vertexDataSize, copyOffset, memoryMappedBuffer, callbackExecuted);

        api.unmapBuffer(memoryMappedBuffer);

        flushAndWait();
        EXPECT_EQ(callbackExecuted, 1);

        auto [shader, descset] = getSimpleShader(cleanup, color);

        auto [vbih, vbh, ibh] = setupGeometryBuffer(cleanup, 3, stride, baseVertex, bufferObject);

        auto const renderPrimitive = cleanup.add(api.createRenderPrimitive(vbh, ibh, PrimitiveType::TRIANGLES));

        PipelineState state = getColorWritePipelineState();
        shader.addProgramToPipelineState(state);
        state.rasterState.culling = CullingMode::NONE;
        state.primitiveType = PrimitiveType::TRIANGLES;
        state.vertexBufferInfo = vbih;

        render(3, 0, swapChain, renderTarget, renderPrimitive, descset, state);

        EXPECT_IMAGE(renderTarget, ScreenshotParams(screenWidth(), screenHeight(), screenshotName, 0));
    }
};

TEST_F(MemoryMappedTest, MapCopyUnmap) {
    SKIP_IF(Backend::WEBGPU, "HwMemoryMappedBuffer APIs not yet implemented");

    auto& api = getDriverApi();
    Cleanup cleanup(api);

    // Create a buffer object.
    BufferObjectHandle const bufferObject = cleanup.add(api.createBufferObject(1024,
            BufferObjectBinding::VERTEX, BufferUsage::DYNAMIC_BIT | BufferUsage::SHARED_WRITE_BIT));

    // Map the buffer.
    MemoryMappedBufferHandle const memoryMappedBuffer = api.mapBuffer(bufferObject, 0, 1024,
            MapBufferAccessFlags::WRITE_BIT, "MemoryMappedBuffer");

    if (memoryMappedBuffer) {
        // Create some data to copy.
        constexpr size_t dataSize = 256;
        auto* const data = new uint8_t[dataSize];
        for (size_t i = 0; i < dataSize; ++i) {
            data[i] = uint8_t(i);
        }

        // Copy the data into the mapped buffer.
        BufferDescriptor bufferDescriptor(data, dataSize, [](void* buffer, size_t, void*) {
            delete[] static_cast<uint8_t*>(buffer);
        });
        api.copyToMemoryMappedBuffer(memoryMappedBuffer, 0, std::move(bufferDescriptor));

        // Unmap the buffer.
        api.unmapBuffer(memoryMappedBuffer);
    }
}

TEST_F(MemoryMappedTest, WriteAndRender) {
    runOffsetRenderTest(0, 0, {1, 0, 0, 1}, "WriteAndRender");
}

TEST_F(MemoryMappedTest, MapWithOffset) {
    runOffsetRenderTest(16, 0, {0, 1, 0, 1}, "MapWithOffset");
}

TEST_F(MemoryMappedTest, CopyWithOffset) {
    runOffsetRenderTest(0, 16, {0, 0, 1, 1}, "CopyWithOffset");
}

TEST_F(MemoryMappedTest, MapAndCopyWithOffsets) {
    runOffsetRenderTest(16, 32, {1, 0, 1, 1}, "MapAndCopyWithOffsets");
}

TEST_F(MemoryMappedTest, MultipleCopies) {
    SKIP_IF(Backend::WEBGPU, "HwMemoryMappedBuffer APIs not yet implemented");

    auto& api = getDriverApi();
    Cleanup cleanup(api);

    auto const swapChain = cleanup.add(createSwapChain());
    api.makeCurrent(swapChain, swapChain);

    auto const renderTarget = makeRenderTarget(cleanup);

    constexpr std::array<math::float2, 3> triangle1 = {{{-0.5f, 0.5f}, {-0.8f, 0.2f}, {-0.2f, 0.2f}}};
    constexpr std::array<math::float2, 3> triangle2 = {{{0.5f, 0.5f}, {0.2f, 0.2f}, {0.8f, 0.2f}}};
    constexpr std::array<math::float2, 3> triangle3 = {{{-0.5f, -0.5f}, {-0.8f, -0.8f}, {-0.2f, -0.8f}}};
    constexpr size_t singleTriangleSize = triangle1.size() * sizeof(math::float2);
    constexpr size_t totalDataSize = singleTriangleSize * 3;

    auto const bufferObject = cleanup.add(api.createBufferObject(totalDataSize,
            BufferObjectBinding::VERTEX, BufferUsage::SHARED_WRITE_BIT));

    int callbacksExecuted = 0;
    MemoryMappedBufferHandle const memoryMappedBuffer = api.mapBuffer(bufferObject, 0, totalDataSize,
            MapBufferAccessFlags::WRITE_BIT, "MultipleCopies");

    copyData(triangle1.data(), singleTriangleSize, 0,
            memoryMappedBuffer, callbacksExecuted);
    copyData(triangle2.data(), singleTriangleSize, singleTriangleSize,
            memoryMappedBuffer, callbacksExecuted);
    copyData(triangle3.data(), singleTriangleSize, singleTriangleSize * 2,
            memoryMappedBuffer, callbacksExecuted);

    api.unmapBuffer(memoryMappedBuffer);

    flushAndWait();
    EXPECT_EQ(callbacksExecuted, 3);

    auto [shader, descset] = getSimpleShader(cleanup, { 1, 1, 0, 1 });

    auto [vbih, vbh, ibh] = setupGeometryBuffer(cleanup, 9, sizeof(math::float2), 0, bufferObject);

    auto const renderPrimitive = cleanup.add(api.createRenderPrimitive(vbh, ibh, PrimitiveType::TRIANGLES));

    PipelineState state = getColorWritePipelineState();
    shader.addProgramToPipelineState(state);
    state.rasterState.culling = CullingMode::NONE;
    state.primitiveType = PrimitiveType::TRIANGLES;
    state.vertexBufferInfo = vbih;

    render(9, 0, swapChain, renderTarget, renderPrimitive, descset, state);

    EXPECT_IMAGE(renderTarget, ScreenshotParams(screenWidth(), screenHeight(), "MultipleCopies", 0));
}

TEST_F(MemoryMappedTest, UpdatePartial) {
    SKIP_IF(Backend::WEBGPU, "HwMemoryMappedBuffer APIs not yet implemented");

    auto& api = getDriverApi();
    Cleanup cleanup(api);

    auto swapChain = cleanup.add(createSwapChain());
    api.makeCurrent(swapChain, swapChain);

    auto renderTarget = makeRenderTarget(cleanup);

    constexpr std::array<math::float2, 3> triangle1 = {{{-0.5f, 0.5f}, {-0.8f, 0.2f}, {-0.2f, 0.2f}}};
    constexpr std::array<math::float2, 3> triangle2 = {{{0.5f, 0.5f}, {0.2f, 0.2f}, {0.8f, 0.2f}}};
    constexpr std::array<math::float2, 3> triangle3 = {{{-0.5f, -0.5f}, {-0.8f, -0.8f}, {-0.2f, -0.8f}}};
    constexpr size_t singleTriangleSize = triangle1.size() * sizeof(math::float2);
    constexpr size_t totalDataSize = singleTriangleSize * 3;

    auto bufferObject = cleanup.add(api.createBufferObject(totalDataSize,
            BufferObjectBinding::VERTEX, BufferUsage::SHARED_WRITE_BIT));

    int callbacksExecuted = 0;

    // Initial data upload
    {
        MemoryMappedBufferHandle const memoryMappedBuffer = api.mapBuffer(bufferObject, 0, totalDataSize,
                MapBufferAccessFlags::WRITE_BIT, "UpdatePartial_initial");

        copyData(triangle1.data(), singleTriangleSize, 0,
                memoryMappedBuffer, callbacksExecuted);
        copyData(triangle2.data(), singleTriangleSize, singleTriangleSize,
                memoryMappedBuffer, callbacksExecuted);
        copyData(triangle3.data(), singleTriangleSize, singleTriangleSize * 2,
                memoryMappedBuffer, callbacksExecuted);
        api.unmapBuffer(memoryMappedBuffer);

        flushAndWait();
        EXPECT_EQ(callbacksExecuted, 3);
    }


    auto [shader, descset] = getSimpleShader(cleanup, { 1, 1, 0, 1 });

    auto [vbih, vbh, ibh] = setupGeometryBuffer(cleanup, 9, sizeof(math::float2), 0, bufferObject);

    auto renderPrimitive = cleanup.add(api.createRenderPrimitive(vbh, ibh, PrimitiveType::TRIANGLES));

    PipelineState state = getColorWritePipelineState();
    shader.addProgramToPipelineState(state);
    state.rasterState.culling = CullingMode::NONE;
    state.primitiveType = PrimitiveType::TRIANGLES;
    state.vertexBufferInfo = vbih;

    render(9, 0, swapChain, renderTarget, renderPrimitive, descset, state);

    EXPECT_IMAGE(renderTarget, ScreenshotParams(screenWidth(), screenHeight(), "UpdatePartial_before", 0));

    // Now, update the middle triangle
    constexpr std::array<math::float2, 3> triangle2_updated = {{{0.5f, -0.5f}, {0.2f, -0.8f}, {0.8f, -0.8f}}};
    callbacksExecuted = 0;
    {
        MemoryMappedBufferHandle const memoryMappedBuffer =
                api.mapBuffer(bufferObject, singleTriangleSize, singleTriangleSize,
                MapBufferAccessFlags::WRITE_BIT, "UpdatePartial_update");

        copyData(triangle2_updated.data(), singleTriangleSize, 0, memoryMappedBuffer, callbacksExecuted);
        api.unmapBuffer(memoryMappedBuffer);

        flushAndWait();
        EXPECT_EQ(callbacksExecuted, 1);
    }

    // need this because render() above calls commit()
    api.makeCurrent(swapChain, swapChain);

    // Second render, after update
    render(9, 1, swapChain, renderTarget, renderPrimitive, descset, state);

    EXPECT_IMAGE(renderTarget, ScreenshotParams(screenWidth(), screenHeight(), "UpdatePartial_after", 0));
}

} // namespace test
