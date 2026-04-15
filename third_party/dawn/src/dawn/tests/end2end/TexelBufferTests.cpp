// Copyright 2026 The Dawn & Tint Authors
//
// Redistribution and use in source and binary forms, with or without
// modification, are permitted provided that the following conditions are met:
//
// 1. Redistributions of source code must retain the above copyright notice, this
//    list of conditions and the following disclaimer.
//
// 2. Redistributions in binary form must reproduce the above copyright notice,
//    this list of conditions and the following disclaimer in the documentation
//    and/or other materials provided with the distribution.
//
// 3. Neither the name of the copyright holder nor the names of its
//    contributors may be used to endorse or promote products derived from
//    this software without specific prior written permission.
//
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
// AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
// IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
// DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
// FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
// DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
// SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
// CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
// OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
// OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

#include <array>

#include "dawn/tests/DawnTest.h"
#include "dawn/utils/WGPUHelpers.h"

namespace dawn {
namespace {

class TexelBufferTests : public DawnTest {
  protected:
    void SetUp() override {
        DawnTest::SetUp();
        DAWN_TEST_UNSUPPORTED_IF(
            !GetInstance().HasWGSLLanguageFeature(wgpu::WGSLLanguageFeatureName::TexelBuffers));
    }
};

// Verify that a read-only texel buffer binding can be read in a compute shader.
TEST_P(TexelBufferTests, ComputeReadOnly) {
    uint32_t initValue = 42u;
    wgpu::Buffer texelBuffer = utils::CreateBufferFromData(
        device,
        wgpu::BufferUsage::CopySrc | wgpu::BufferUsage::CopyDst | wgpu::BufferUsage::TexelBuffer,
        {initValue});

    wgpu::TexelBufferViewDescriptor viewDesc = {};
    viewDesc.format = wgpu::TextureFormat::R32Uint;
    wgpu::TexelBufferView view = texelBuffer.CreateTexelView(&viewDesc);

    wgpu::BufferDescriptor outDesc;
    outDesc.size = sizeof(uint32_t);
    outDesc.usage = wgpu::BufferUsage::CopySrc | wgpu::BufferUsage::Storage;
    wgpu::Buffer out = device.CreateBuffer(&outDesc);

    const char* shaderSrc = R"(
        requires texel_buffers;

        @group(0) @binding(0) var<storage, read_write> result : array<u32, 1>;
        @group(0) @binding(1) var t : texel_buffer<r32uint, read>;

        @compute @workgroup_size(1)
        fn main() {
            result[0] = textureLoad(t, 0u).x;
        }
    )";

    wgpu::ComputePipelineDescriptor csDesc;
    csDesc.compute.module = utils::CreateShaderModule(device, shaderSrc);
    wgpu::ComputePipeline pipeline = device.CreateComputePipeline(&csDesc);

    wgpu::BindGroup bindGroup =
        utils::MakeBindGroup(device, pipeline.GetBindGroupLayout(0), {{0, out}, {1, view}});

    wgpu::CommandEncoder encoder = device.CreateCommandEncoder();
    wgpu::ComputePassEncoder pass = encoder.BeginComputePass();
    pass.SetPipeline(pipeline);
    pass.SetBindGroup(0, bindGroup);
    pass.DispatchWorkgroups(1);
    pass.End();

    wgpu::CommandBuffer commands = encoder.Finish();
    queue.Submit(1, &commands);

    EXPECT_BUFFER_U32_EQ(initValue, out, 0);
}

// Verify that a read-write texel buffer binding can be written in a compute shader.
TEST_P(TexelBufferTests, ComputeReadWrite) {
    uint32_t zero = 0u;
    wgpu::Buffer texelBuffer =
        utils::CreateBufferFromData(device,
                                    wgpu::BufferUsage::CopySrc | wgpu::BufferUsage::CopyDst |
                                        wgpu::BufferUsage::TexelBuffer | wgpu::BufferUsage::Storage,
                                    {zero});

    wgpu::TexelBufferViewDescriptor viewDesc = {};
    viewDesc.format = wgpu::TextureFormat::R32Uint;
    wgpu::TexelBufferView view = texelBuffer.CreateTexelView(&viewDesc);

    const char* shaderSrc = R"(
        requires texel_buffers;

        @group(0) @binding(0) var t : texel_buffer<r32uint, read_write>;

        @compute @workgroup_size(1)
        fn main() {
            textureStore(t, 0u, vec4<u32>(99u, 0u, 0u, 0u));
        }
    )";

    wgpu::ComputePipelineDescriptor csDesc;
    csDesc.compute.module = utils::CreateShaderModule(device, shaderSrc);
    wgpu::ComputePipeline pipeline = device.CreateComputePipeline(&csDesc);

    wgpu::BindGroup bindGroup =
        utils::MakeBindGroup(device, pipeline.GetBindGroupLayout(0), {{0, view}});

    wgpu::CommandEncoder encoder = device.CreateCommandEncoder();
    wgpu::ComputePassEncoder pass = encoder.BeginComputePass();
    pass.SetPipeline(pipeline);
    pass.SetBindGroup(0, bindGroup);
    pass.DispatchWorkgroups(1);
    pass.End();

    wgpu::CommandBuffer commands = encoder.Finish();
    queue.Submit(1, &commands);

    EXPECT_BUFFER_U32_EQ(99u, texelBuffer, 0);
}

// Verify that texel buffers can supply vertex positions for a render pipeline.
TEST_P(TexelBufferTests, RenderVertexPositions) {
    // Positions for a full screen triangle stored as RGBA32 floats.
    wgpu::Buffer texelBuffer = utils::CreateBufferFromData(
        device,
        wgpu::BufferUsage::CopySrc | wgpu::BufferUsage::CopyDst | wgpu::BufferUsage::TexelBuffer,
        {
            -1.f, -1.f, 0.f, 0.f,  // Vertex 0
            3.f, -1.f, 0.f, 0.f,   // Vertex 1
            -1.f, 3.f, 0.f, 0.f    // Vertex 2
        });

    wgpu::TexelBufferViewDescriptor viewDesc = {};
    viewDesc.format = wgpu::TextureFormat::RGBA32Float;
    wgpu::TexelBufferView view = texelBuffer.CreateTexelView(&viewDesc);

    const char* shaderSrc = R"(
        requires texel_buffers;

        @group(0) @binding(0) var positions : texel_buffer<rgba32float, read>;

        struct VSOut {
            @builtin(position) pos : vec4<f32>
        };

        @vertex
        fn vs_main(@builtin(vertex_index) vi : u32) -> VSOut {
            let xy = textureLoad(positions, vi).xy;
            var out : VSOut;
            out.pos = vec4<f32>(xy, 0.0, 1.0);
            return out;
        }

        @fragment
        fn fs_main() -> @location(0) vec4<f32> {
            return vec4<f32>(1.0, 0.0, 0.0, 1.0);
        }
    )";

    utils::BasicRenderPass renderPass = utils::CreateBasicRenderPass(device, 1, 1);

    wgpu::RenderPipelineDescriptor desc;
    desc.vertex.module = utils::CreateShaderModule(device, shaderSrc);
    desc.vertex.entryPoint = "vs_main";

    wgpu::FragmentState fragment = {};
    fragment.module = desc.vertex.module;
    fragment.entryPoint = "fs_main";
    wgpu::ColorTargetState colorTarget = {};
    colorTarget.format = renderPass.colorFormat;
    fragment.targetCount = 1;
    fragment.targets = &colorTarget;
    desc.fragment = &fragment;
    desc.primitive.topology = wgpu::PrimitiveTopology::TriangleList;

    wgpu::RenderPipeline pipeline = device.CreateRenderPipeline(&desc);

    wgpu::BindGroup bindGroup =
        utils::MakeBindGroup(device, pipeline.GetBindGroupLayout(0), {{0, view}});

    wgpu::CommandEncoder encoder = device.CreateCommandEncoder();
    wgpu::RenderPassEncoder pass = encoder.BeginRenderPass(&renderPass.renderPassInfo);
    pass.SetPipeline(pipeline);
    pass.SetBindGroup(0, bindGroup);
    pass.Draw(3);
    pass.End();

    wgpu::CommandBuffer commands = encoder.Finish();
    queue.Submit(1, &commands);

    EXPECT_PIXEL_RGBA8_EQ(utils::RGBA8::kRed, renderPass.color, 0, 0);
}

// Verify that fragment shaders can read from texel buffers as color palettes.
TEST_P(TexelBufferTests, RenderFragmentColorPalette) {
    // Two colors stored as RGBA8 values representing a simple palette.
    wgpu::Buffer texelBuffer = utils::CreateBufferFromData(
        device,
        wgpu::BufferUsage::CopySrc | wgpu::BufferUsage::CopyDst | wgpu::BufferUsage::TexelBuffer,
        {utils::RGBA8::kRed, utils::RGBA8::kGreen});

    wgpu::TexelBufferViewDescriptor viewDesc = {};
    viewDesc.format = wgpu::TextureFormat::RGBA8Unorm;
    wgpu::TexelBufferView view = texelBuffer.CreateTexelView(&viewDesc);

    const char* shaderSrc = R"(
        requires texel_buffers;

        @group(0) @binding(0) var colors : texel_buffer<rgba8unorm, read>;

        @vertex
        fn vs_main(@builtin(vertex_index) vi : u32) -> @builtin(position) vec4<f32> {
            var pos = array<vec2<f32>, 3>(
                vec2<f32>(-1.0, -1.0),
                vec2<f32>(3.0, -1.0),
                vec2<f32>(-1.0, 3.0)
            );
            return vec4<f32>(pos[vi], 0.0, 1.0);
        }

        @fragment
        fn fs_main(@builtin(position) coord : vec4<f32>) -> @location(0) vec4<f32> {
            let idx = u32(floor(coord.x));
            return textureLoad(colors, idx);
        }
    )";

    utils::BasicRenderPass renderPass = utils::CreateBasicRenderPass(device, 2, 1);

    wgpu::RenderPipelineDescriptor desc;
    desc.vertex.module = utils::CreateShaderModule(device, shaderSrc);
    desc.vertex.entryPoint = "vs_main";

    wgpu::FragmentState fragment = {};
    fragment.module = desc.vertex.module;
    fragment.entryPoint = "fs_main";
    wgpu::ColorTargetState colorTarget = {};
    colorTarget.format = renderPass.colorFormat;
    fragment.targetCount = 1;
    fragment.targets = &colorTarget;
    desc.fragment = &fragment;
    desc.primitive.topology = wgpu::PrimitiveTopology::TriangleList;

    wgpu::RenderPipeline pipeline = device.CreateRenderPipeline(&desc);

    wgpu::BindGroup bindGroup =
        utils::MakeBindGroup(device, pipeline.GetBindGroupLayout(0), {{0, view}});

    wgpu::CommandEncoder encoder = device.CreateCommandEncoder();
    wgpu::RenderPassEncoder pass = encoder.BeginRenderPass(&renderPass.renderPassInfo);
    pass.SetPipeline(pipeline);
    pass.SetBindGroup(0, bindGroup);
    pass.Draw(3);
    pass.End();

    wgpu::CommandBuffer commands = encoder.Finish();
    queue.Submit(1, &commands);

    EXPECT_PIXEL_RGBA8_EQ(utils::RGBA8::kRed, renderPass.color, 0, 0);
    EXPECT_PIXEL_RGBA8_EQ(utils::RGBA8::kGreen, renderPass.color, 1, 0);
}

// Verify that multiple elements can be incremented in a read-write texel buffer.
TEST_P(TexelBufferTests, ComputeIncrementValues) {
    wgpu::Buffer texelBuffer =
        utils::CreateBufferFromData(device,
                                    wgpu::BufferUsage::CopySrc | wgpu::BufferUsage::CopyDst |
                                        wgpu::BufferUsage::TexelBuffer | wgpu::BufferUsage::Storage,
                                    {0u, 1u, 2u, 3u});

    wgpu::TexelBufferViewDescriptor viewDesc = {};
    viewDesc.format = wgpu::TextureFormat::R32Uint;
    wgpu::TexelBufferView view = texelBuffer.CreateTexelView(&viewDesc);

    const char* shaderSrc = R"(
        requires texel_buffers;

        @group(0) @binding(0) var t : texel_buffer<r32uint, read_write>;

        @compute @workgroup_size(4)
        fn main(@builtin(local_invocation_id) lid : vec3<u32>) {
            let i = lid.x;
            let value = textureLoad(t, i).x;
            textureStore(t, i, vec4<u32>(value + 1u, 0u, 0u, 0u));
        }
    )";

    wgpu::ComputePipelineDescriptor csDesc;
    csDesc.compute.module = utils::CreateShaderModule(device, shaderSrc);
    wgpu::ComputePipeline pipeline = device.CreateComputePipeline(&csDesc);

    wgpu::BindGroup bindGroup =
        utils::MakeBindGroup(device, pipeline.GetBindGroupLayout(0), {{0, view}});

    wgpu::CommandEncoder encoder = device.CreateCommandEncoder();
    wgpu::ComputePassEncoder pass = encoder.BeginComputePass();
    pass.SetPipeline(pipeline);
    pass.SetBindGroup(0, bindGroup);
    pass.DispatchWorkgroups(1);
    pass.End();

    wgpu::CommandBuffer commands = encoder.Finish();
    queue.Submit(1, &commands);

    std::array<uint32_t, 4> expected = {1u, 2u, 3u, 4u};
    EXPECT_BUFFER_U32_RANGE_EQ(expected.data(), texelBuffer, 0, expected.size());
}

// Use a texel buffer as a lookup table for indices stored in a storage buffer.
TEST_P(TexelBufferTests, ComputePaletteLookup) {
    // Palette of 4 colors stored in a texel buffer.
    wgpu::Buffer palette = utils::CreateBufferFromData(
        device, wgpu::BufferUsage::TexelBuffer | wgpu::BufferUsage::CopyDst, {0u, 1u, 2u, 3u});

    wgpu::TexelBufferViewDescriptor paletteViewDesc = {};
    paletteViewDesc.format = wgpu::TextureFormat::R32Uint;
    wgpu::TexelBufferView paletteView = palette.CreateTexelView(&paletteViewDesc);

    // Indices selecting colors in the palette.
    wgpu::Buffer indices =
        utils::CreateBufferFromData(device, wgpu::BufferUsage::Storage, {2u, 0u, 3u, 1u});

    wgpu::BufferDescriptor resultDesc = {};
    resultDesc.size = sizeof(uint32_t) * 4;
    resultDesc.usage = wgpu::BufferUsage::CopySrc | wgpu::BufferUsage::Storage;
    wgpu::Buffer result = device.CreateBuffer(&resultDesc);

    const char* shaderSrc = R"(
        requires texel_buffers;

        @group(0) @binding(0) var<storage, read> indices : array<u32, 4>;
        @group(0) @binding(1) var<storage, read_write> result : array<u32, 4>;
        @group(0) @binding(2) var palette : texel_buffer<r32uint, read>;

        @compute @workgroup_size(4)
        fn main(@builtin(local_invocation_id) lid : vec3<u32>) {
            let i = lid.x;
            let idx = indices[i];
            result[i] = textureLoad(palette, idx).x;
        }
    )";

    wgpu::ComputePipelineDescriptor csDesc;
    csDesc.compute.module = utils::CreateShaderModule(device, shaderSrc);
    wgpu::ComputePipeline pipeline = device.CreateComputePipeline(&csDesc);

    wgpu::BindGroup bindGroup = utils::MakeBindGroup(device, pipeline.GetBindGroupLayout(0),
                                                     {{0, indices}, {1, result}, {2, paletteView}});

    wgpu::CommandEncoder encoder = device.CreateCommandEncoder();
    wgpu::ComputePassEncoder pass = encoder.BeginComputePass();
    pass.SetPipeline(pipeline);
    pass.SetBindGroup(0, bindGroup);
    pass.DispatchWorkgroups(1);
    pass.End();

    wgpu::CommandBuffer commands = encoder.Finish();
    queue.Submit(1, &commands);

    std::array<uint32_t, 4> expected = {2u, 0u, 3u, 1u};
    EXPECT_BUFFER_U32_RANGE_EQ(expected.data(), result, 0, expected.size());
}

// Use a texel buffer as a source for colors in a vertex shader.
TEST_P(TexelBufferTests, RenderVertexShaderColorLookup) {
    // Palette of three colors encoded as RGBA8Unorm texels.
    wgpu::Buffer texelBuffer = utils::CreateBufferFromData(
        device,
        wgpu::BufferUsage::TexelBuffer | wgpu::BufferUsage::CopyDst | wgpu::BufferUsage::CopySrc,
        {utils::RGBA8::kRed, utils::RGBA8::kGreen, utils::RGBA8::kBlue});

    wgpu::TexelBufferViewDescriptor viewDesc = {};
    viewDesc.format = wgpu::TextureFormat::RGBA8Unorm;
    wgpu::TexelBufferView view = texelBuffer.CreateTexelView(&viewDesc);

    // Set up a 3x1 render target so each point maps to a single pixel.
    utils::BasicRenderPass renderPass = utils::CreateBasicRenderPass(device, 3, 1);

    const char* shaderSrc = R"(
        requires texel_buffers;

        @group(0) @binding(0) var palette : texel_buffer<rgba8unorm, read>;

        struct VSOut {
            @builtin(position) position : vec4<f32>,
            @location(0) color : vec4<f32>,
        };

        @vertex
        fn vs_main(@builtin(vertex_index) vertexIndex : u32) -> VSOut {
            var out : VSOut;
            let x = -1.0 + (f32(vertexIndex) + 0.5) * 2.0 / 3.0;
            out.position = vec4<f32>(x, 0.0, 0.0, 1.0);
            out.color = textureLoad(palette, vertexIndex);
            return out;
        }

        @fragment
        fn fs_main(@location(0) color : vec4<f32>) -> @location(0) vec4<f32> {
            return color;
        }
    )";

    wgpu::RenderPipelineDescriptor desc;
    desc.vertex.module = utils::CreateShaderModule(device, shaderSrc);
    desc.vertex.entryPoint = "vs_main";

    wgpu::FragmentState fragment = {};
    fragment.module = desc.vertex.module;
    fragment.entryPoint = "fs_main";
    wgpu::ColorTargetState colorTarget = {};
    colorTarget.format = renderPass.colorFormat;
    fragment.targetCount = 1;
    fragment.targets = &colorTarget;
    desc.fragment = &fragment;
    desc.primitive.topology = wgpu::PrimitiveTopology::PointList;

    wgpu::RenderPipeline pipeline = device.CreateRenderPipeline(&desc);

    wgpu::BindGroup bindGroup =
        utils::MakeBindGroup(device, pipeline.GetBindGroupLayout(0), {{0, view}});

    wgpu::CommandEncoder encoder = device.CreateCommandEncoder();
    wgpu::RenderPassEncoder pass = encoder.BeginRenderPass(&renderPass.renderPassInfo);
    pass.SetPipeline(pipeline);
    pass.SetBindGroup(0, bindGroup);
    pass.Draw(3);
    pass.End();

    wgpu::CommandBuffer commands = encoder.Finish();
    queue.Submit(1, &commands);

    EXPECT_PIXEL_RGBA8_EQ(utils::RGBA8::kRed, renderPass.color, 0, 0);
    EXPECT_PIXEL_RGBA8_EQ(utils::RGBA8::kGreen, renderPass.color, 1, 0);
    EXPECT_PIXEL_RGBA8_EQ(utils::RGBA8::kBlue, renderPass.color, 2, 0);
}

// Copy data from a texel buffer into a storage texture using a compute shader.
TEST_P(TexelBufferTests, CopyToStorageTexture) {
    // Four RGBA colors.
    wgpu::Buffer texelBuffer = utils::CreateBufferFromData(
        device,
        wgpu::BufferUsage::TexelBuffer | wgpu::BufferUsage::CopyDst | wgpu::BufferUsage::CopySrc,
        {utils::RGBA8::kRed, utils::RGBA8::kGreen, utils::RGBA8::kBlue, utils::RGBA8::kWhite});

    wgpu::TexelBufferViewDescriptor viewDesc = {};
    viewDesc.format = wgpu::TextureFormat::RGBA8Unorm;
    wgpu::TexelBufferView view = texelBuffer.CreateTexelView(&viewDesc);

    wgpu::TextureDescriptor texDesc;
    texDesc.size = {4, 1, 1};
    texDesc.format = wgpu::TextureFormat::RGBA8Unorm;
    texDesc.usage = wgpu::TextureUsage::StorageBinding | wgpu::TextureUsage::CopySrc;
    wgpu::Texture texture = device.CreateTexture(&texDesc);

    const char* shaderSrc = R"(
        requires texel_buffers;

        @group(0) @binding(0) var src : texel_buffer<rgba8unorm, read>;
        @group(0) @binding(1) var dst : texture_storage_2d<rgba8unorm, write>;

        @compute @workgroup_size(1)
        fn main(@builtin(global_invocation_id) gid : vec3<u32>) {
            let c = textureLoad(src, gid.x);
            textureStore(dst, vec2<u32>(gid.x, 0u), c);
        }
    )";

    wgpu::ComputePipelineDescriptor csDesc;
    csDesc.compute.module = utils::CreateShaderModule(device, shaderSrc);
    wgpu::ComputePipeline pipeline = device.CreateComputePipeline(&csDesc);

    wgpu::BindGroup bindGroup = utils::MakeBindGroup(device, pipeline.GetBindGroupLayout(0),
                                                     {{0, view}, {1, texture.CreateView()}});

    wgpu::CommandEncoder encoder = device.CreateCommandEncoder();
    wgpu::ComputePassEncoder pass = encoder.BeginComputePass();
    pass.SetPipeline(pipeline);
    pass.SetBindGroup(0, bindGroup);
    pass.DispatchWorkgroups(4);
    pass.End();

    wgpu::CommandBuffer commands = encoder.Finish();
    queue.Submit(1, &commands);

    EXPECT_PIXEL_RGBA8_EQ(utils::RGBA8::kRed, texture, 0, 0);
    EXPECT_PIXEL_RGBA8_EQ(utils::RGBA8::kGreen, texture, 1, 0);
    EXPECT_PIXEL_RGBA8_EQ(utils::RGBA8::kBlue, texture, 2, 0);
    EXPECT_PIXEL_RGBA8_EQ(utils::RGBA8::kWhite, texture, 3, 0);
}

// Use one texel buffer to index another and write results to a third texel buffer.
TEST_P(TexelBufferTests, ChainedLookup) {
    wgpu::Buffer indices = utils::CreateBufferFromData(
        device, wgpu::BufferUsage::TexelBuffer | wgpu::BufferUsage::CopyDst, {2u, 0u, 3u, 1u});

    wgpu::Buffer palette = utils::CreateBufferFromData(
        device, wgpu::BufferUsage::TexelBuffer | wgpu::BufferUsage::CopyDst, {0u, 1u, 2u, 3u});

    wgpu::Buffer result =
        utils::CreateBufferFromData(device,
                                    wgpu::BufferUsage::TexelBuffer | wgpu::BufferUsage::Storage |
                                        wgpu::BufferUsage::CopySrc | wgpu::BufferUsage::CopyDst,
                                    {0u, 0u, 0u, 0u});

    wgpu::TexelBufferViewDescriptor viewDesc = {};
    viewDesc.format = wgpu::TextureFormat::R32Uint;
    wgpu::TexelBufferView indicesView = indices.CreateTexelView(&viewDesc);
    wgpu::TexelBufferView paletteView = palette.CreateTexelView(&viewDesc);
    wgpu::TexelBufferView resultView = result.CreateTexelView(&viewDesc);

    const char* shaderSrc = R"(
        requires texel_buffers;

        @group(0) @binding(0) var indices : texel_buffer<r32uint, read>;
        @group(0) @binding(1) var palette : texel_buffer<r32uint, read>;
        @group(0) @binding(2) var output : texel_buffer<r32uint, read_write>;

        @compute @workgroup_size(4)
        fn main(@builtin(local_invocation_id) lid : vec3<u32>) {
            let i = lid.x;
            let idx = textureLoad(indices, i).x;
            let value = textureLoad(palette, idx).x;
            textureStore(output, i, vec4<u32>(value, 0u, 0u, 0u));
        }
    )";

    wgpu::ComputePipelineDescriptor csDesc;
    csDesc.compute.module = utils::CreateShaderModule(device, shaderSrc);
    wgpu::ComputePipeline pipeline = device.CreateComputePipeline(&csDesc);

    utils::BindingInitializationHelper indicesBinding(0, indicesView);
    utils::BindingInitializationHelper paletteBinding(1, paletteView);
    utils::BindingInitializationHelper resultBinding(2, resultView);

    wgpu::BindGroup bindGroup = utils::MakeBindGroup(
        device, pipeline.GetBindGroupLayout(0), {indicesBinding, paletteBinding, resultBinding});

    wgpu::CommandEncoder encoder = device.CreateCommandEncoder();
    wgpu::ComputePassEncoder pass = encoder.BeginComputePass();
    pass.SetPipeline(pipeline);
    pass.SetBindGroup(0, bindGroup);
    pass.DispatchWorkgroups(1);
    pass.End();

    wgpu::CommandBuffer commands = encoder.Finish();
    queue.Submit(1, &commands);

    std::array<uint32_t, 4> expected = {2u, 0u, 3u, 1u};
    EXPECT_BUFFER_U32_RANGE_EQ(expected.data(), result, 0, expected.size());
}

// Read from two texel buffers, add their values and write the results to a third
// texel buffer.
TEST_P(TexelBufferTests, AddTexelBuffers) {
    wgpu::Buffer a = utils::CreateBufferFromData(
        device, wgpu::BufferUsage::CopyDst | wgpu::BufferUsage::TexelBuffer, {1u, 2u, 3u, 4u});
    wgpu::Buffer b = utils::CreateBufferFromData(
        device, wgpu::BufferUsage::CopyDst | wgpu::BufferUsage::TexelBuffer, {10u, 20u, 30u, 40u});
    wgpu::Buffer c =
        utils::CreateBufferFromData(device,
                                    wgpu::BufferUsage::CopySrc | wgpu::BufferUsage::CopyDst |
                                        wgpu::BufferUsage::TexelBuffer | wgpu::BufferUsage::Storage,
                                    {0u, 0u, 0u, 0u});

    wgpu::TexelBufferViewDescriptor viewDesc = {};
    viewDesc.format = wgpu::TextureFormat::R32Uint;
    wgpu::TexelBufferView aView = a.CreateTexelView(&viewDesc);
    wgpu::TexelBufferView bView = b.CreateTexelView(&viewDesc);
    wgpu::TexelBufferView cView = c.CreateTexelView(&viewDesc);

    const char* shaderSrc = R"(
        requires texel_buffers;

        @group(0) @binding(0) var srcA : texel_buffer<r32uint, read>;
        @group(0) @binding(1) var srcB : texel_buffer<r32uint, read>;
        @group(0) @binding(2) var dst : texel_buffer<r32uint, read_write>;

        @compute @workgroup_size(4)
        fn main(@builtin(local_invocation_id) lid : vec3<u32>) {
            let i = lid.x;
            let av = textureLoad(srcA, i).x;
            let bv = textureLoad(srcB, i).x;
            textureStore(dst, i, vec4<u32>(av + bv, 0u, 0u, 0u));
        }
    )";

    wgpu::TexelBufferBindingLayout layoutA = {};
    layoutA.format = wgpu::TextureFormat::R32Uint;
    layoutA.access = wgpu::TexelBufferAccess::ReadOnly;
    wgpu::TexelBufferBindingLayout layoutB = layoutA;
    wgpu::TexelBufferBindingLayout layoutC = layoutA;
    layoutC.access = wgpu::TexelBufferAccess::ReadWrite;

    wgpu::BindGroupLayout bgl =
        utils::MakeBindGroupLayout(device, {{0, wgpu::ShaderStage::Compute, &layoutA},
                                            {1, wgpu::ShaderStage::Compute, &layoutB},
                                            {2, wgpu::ShaderStage::Compute, &layoutC}});
    wgpu::PipelineLayout pl = utils::MakePipelineLayout(device, {bgl});

    wgpu::ComputePipelineDescriptor csDesc;
    csDesc.layout = pl;
    csDesc.compute.module = utils::CreateShaderModule(device, shaderSrc);
    wgpu::ComputePipeline pipeline = device.CreateComputePipeline(&csDesc);

    wgpu::BindGroup bindGroup =
        utils::MakeBindGroup(device, bgl, {{0, aView}, {1, bView}, {2, cView}});

    wgpu::CommandEncoder encoder = device.CreateCommandEncoder();
    wgpu::ComputePassEncoder pass = encoder.BeginComputePass();
    pass.SetPipeline(pipeline);
    pass.SetBindGroup(0, bindGroup);
    pass.DispatchWorkgroups(1);
    pass.End();

    wgpu::CommandBuffer commands = encoder.Finish();
    queue.Submit(1, &commands);

    std::array<uint32_t, 4> expected = {11u, 22u, 33u, 44u};
    EXPECT_BUFFER_U32_RANGE_EQ(expected.data(), c, 0, expected.size());
}

// Copy a range of texels between two texel buffer views using a buffer copy.
TEST_P(TexelBufferTests, CopyBetweenViews) {
    wgpu::Buffer src =
        utils::CreateBufferFromData(device,
                                    wgpu::BufferUsage::CopySrc | wgpu::BufferUsage::CopyDst |
                                        wgpu::BufferUsage::TexelBuffer | wgpu::BufferUsage::Storage,
                                    {1u, 2u, 3u});

    // dst is large enough to hold 256 bytes of padding followed by 3 uint32 values.
    // The view uses offset=256 to satisfy kTexelBufferOffsetAlignment.
    constexpr uint64_t kDstViewOffset = 256;
    constexpr uint64_t kViewSize = 3 * sizeof(uint32_t);
    wgpu::BufferDescriptor dstBufDesc = {};
    dstBufDesc.size = kDstViewOffset + kViewSize;
    dstBufDesc.usage = wgpu::BufferUsage::CopySrc | wgpu::BufferUsage::CopyDst |
                       wgpu::BufferUsage::TexelBuffer | wgpu::BufferUsage::Storage;
    wgpu::Buffer dst = device.CreateBuffer(&dstBufDesc);

    wgpu::TexelBufferViewDescriptor srcDesc = {};
    srcDesc.format = wgpu::TextureFormat::R32Uint;
    srcDesc.offset = 0;
    srcDesc.size = kViewSize;
    wgpu::TexelBufferView srcView = src.CreateTexelView(&srcDesc);

    wgpu::TexelBufferViewDescriptor dstDesc = {};
    dstDesc.format = wgpu::TextureFormat::R32Uint;
    dstDesc.offset = kDstViewOffset;
    dstDesc.size = kViewSize;
    wgpu::TexelBufferView dstView = dst.CreateTexelView(&dstDesc);

    wgpu::CommandEncoder encoder = device.CreateCommandEncoder();
    encoder.CopyBufferToBuffer(src, srcDesc.offset, dst, dstDesc.offset, srcDesc.size);
    wgpu::CommandBuffer commands = encoder.Finish();
    queue.Submit(1, &commands);

    std::array<uint32_t, 3> expected = {1u, 2u, 3u};
    EXPECT_BUFFER_U32_RANGE_EQ(expected.data(), dst, dstDesc.offset, expected.size());
}

DAWN_INSTANTIATE_TEST(TexelBufferTests, VulkanBackend());

}  // anonymous namespace
}  // namespace dawn
