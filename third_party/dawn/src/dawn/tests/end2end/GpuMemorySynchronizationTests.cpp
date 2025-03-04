// Copyright 2019 The Dawn & Tint Authors
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

#include <tuple>
#include <vector>

#include "dawn/common/Assert.h"
#include "dawn/common/Constants.h"
#include "dawn/common/Math.h"
#include "dawn/tests/DawnTest.h"
#include "dawn/utils/ComboRenderPipelineDescriptor.h"
#include "dawn/utils/WGPUHelpers.h"

namespace dawn {
namespace {

class GpuMemorySyncTests : public DawnTest {
  protected:
    wgpu::RequiredLimits GetRequiredLimits(const wgpu::SupportedLimits& supported) override {
        // Just copy all the limits, though all we really care about is
        // maxStorageBuffersInFragmentStage
        wgpu::RequiredLimits required = {};
        required.limits = supported.limits;
        return required;
    }

    void SetUp() override {
        DawnTest::SetUp();
        DAWN_SUPPRESS_TEST_IF(GetSupportedLimits().limits.maxStorageBuffersInFragmentStage < 1);
    }

    wgpu::Buffer CreateBuffer() {
        wgpu::BufferDescriptor srcDesc;
        srcDesc.size = 4;
        srcDesc.usage =
            wgpu::BufferUsage::CopySrc | wgpu::BufferUsage::CopyDst | wgpu::BufferUsage::Storage;
        wgpu::Buffer buffer = device.CreateBuffer(&srcDesc);

        int myData = 0;
        queue.WriteBuffer(buffer, 0, &myData, sizeof(myData));
        return buffer;
    }

    std::tuple<wgpu::ComputePipeline, wgpu::BindGroup> CreatePipelineAndBindGroupForCompute(
        const wgpu::Buffer& buffer) {
        wgpu::ShaderModule csModule = utils::CreateShaderModule(device, R"(
            struct Data {
                a : i32
            }
            @group(0) @binding(0) var<storage, read_write> data : Data;
            @compute @workgroup_size(1) fn main() {
                data.a = data.a + 1;
            })");

        wgpu::ComputePipelineDescriptor cpDesc;
        cpDesc.compute.module = csModule;
        wgpu::ComputePipeline pipeline = device.CreateComputePipeline(&cpDesc);

        wgpu::BindGroup bindGroup =
            utils::MakeBindGroup(device, pipeline.GetBindGroupLayout(0), {{0, buffer}});
        return std::make_tuple(pipeline, bindGroup);
    }

    std::tuple<wgpu::RenderPipeline, wgpu::BindGroup> CreatePipelineAndBindGroupForRender(
        const wgpu::Buffer& buffer,
        wgpu::TextureFormat colorFormat) {
        wgpu::ShaderModule vsModule = utils::CreateShaderModule(device, R"(
            @vertex fn main() -> @builtin(position) vec4f {
                return vec4f(0.0, 0.0, 0.0, 1.0);
            })");

        wgpu::ShaderModule fsModule = utils::CreateShaderModule(device, R"(
            struct Data {
                i : i32
            }
            @group(0) @binding(0) var<storage, read_write> data : Data;
            @fragment fn main() -> @location(0) vec4f {
                data.i = data.i + 1;
                return vec4f(f32(data.i) / 255.0, 0.0, 0.0, 1.0);
            })");

        utils::ComboRenderPipelineDescriptor rpDesc;
        rpDesc.vertex.module = vsModule;
        rpDesc.cFragment.module = fsModule;
        rpDesc.primitive.topology = wgpu::PrimitiveTopology::PointList;
        rpDesc.cTargets[0].format = colorFormat;

        wgpu::RenderPipeline pipeline = device.CreateRenderPipeline(&rpDesc);

        wgpu::BindGroup bindGroup =
            utils::MakeBindGroup(device, pipeline.GetBindGroupLayout(0), {{0, buffer}});
        return std::make_tuple(pipeline, bindGroup);
    }
};

// Clear storage buffer with zero. Then read data, add one, and write the result to storage buffer
// in compute pass. Iterate this read-add-write steps per compute pass a few time. The successive
// iteration reads the result in buffer from last iteration, which makes the iterations a data
// dependency chain. The test verifies that data in buffer among iterations in compute passes is
// correctly synchronized.
TEST_P(GpuMemorySyncTests, ComputePass) {
    DAWN_SUPPRESS_TEST_IF(GetSupportedLimits().limits.maxStorageBuffersInFragmentStage < 1);

    // Create pipeline, bind group, and buffer for compute pass.
    wgpu::Buffer buffer = CreateBuffer();
    auto [compute, bindGroup] = CreatePipelineAndBindGroupForCompute(buffer);
    wgpu::CommandEncoder encoder = device.CreateCommandEncoder();

    // Iterate the read-add-write operations in compute pass a few times.
    int iteration = 3;
    for (int i = 0; i < iteration; ++i) {
        wgpu::ComputePassEncoder pass = encoder.BeginComputePass();
        pass.SetPipeline(compute);
        pass.SetBindGroup(0, bindGroup);
        pass.DispatchWorkgroups(1);
        pass.End();
    }

    wgpu::CommandBuffer commands = encoder.Finish();
    queue.Submit(1, &commands);

    // Verify the result.
    EXPECT_BUFFER_U32_EQ(iteration, buffer, 0);
}

// Clear storage buffer with zero. Then read data, add one, and write the result to storage buffer
// in render pass. Iterate this read-add-write steps per render pass a few time. The successive
// iteration reads the result in buffer from last iteration, which makes the iterations a data
// dependency chain. In addition, color output by fragment shader depends on the data in storage
// buffer, so we can check color in render target to verify that data in buffer among iterations in
// render passes is correctly synchronized.
TEST_P(GpuMemorySyncTests, RenderPass) {
    // Create pipeline, bind group, and buffer for render pass.
    wgpu::Buffer buffer = CreateBuffer();
    utils::BasicRenderPass renderPass = utils::CreateBasicRenderPass(device, 1, 1);
    auto [render, bindGroup] = CreatePipelineAndBindGroupForRender(buffer, renderPass.colorFormat);
    wgpu::CommandEncoder encoder = device.CreateCommandEncoder();

    // Iterate the read-add-write operations in render pass a few times.
    int iteration = 3;
    for (int i = 0; i < iteration; ++i) {
        wgpu::RenderPassEncoder pass = encoder.BeginRenderPass(&renderPass.renderPassInfo);
        pass.SetPipeline(render);
        pass.SetBindGroup(0, bindGroup);
        pass.Draw(1);
        pass.End();
    }

    wgpu::CommandBuffer commands = encoder.Finish();
    queue.Submit(1, &commands);

    // Verify the result.
    EXPECT_PIXEL_RGBA8_EQ(utils::RGBA8(iteration, 0, 0, 255), renderPass.color, 0, 0);
}

// Write into a storage buffer in a render pass. Then read that data in a compute
// pass. And verify the data flow is correctly synchronized.
TEST_P(GpuMemorySyncTests, RenderPassToComputePass) {
    // TODO(crbug.com/388318201): assert failed in setSerial libANGLE/renderer/vulkan/vk_resource.h
    DAWN_TEST_UNSUPPORTED_IF(IsCompatibilityMode() &&
                             HasToggleEnabled("gl_force_es_31_and_no_extensions"));

    // Create pipeline, bind group, and buffer for render pass and compute pass.
    wgpu::Buffer buffer = CreateBuffer();
    utils::BasicRenderPass renderPass = utils::CreateBasicRenderPass(device, 1, 1);

    auto [render, bindGroup0] = CreatePipelineAndBindGroupForRender(buffer, renderPass.colorFormat);
    auto [compute, bindGroup1] = CreatePipelineAndBindGroupForCompute(buffer);

    wgpu::CommandEncoder encoder = device.CreateCommandEncoder();

    // Write data into a storage buffer in render pass.
    wgpu::RenderPassEncoder pass0 = encoder.BeginRenderPass(&renderPass.renderPassInfo);
    pass0.SetPipeline(render);
    pass0.SetBindGroup(0, bindGroup0);
    pass0.Draw(1);
    pass0.End();

    // Read that data in compute pass.
    wgpu::ComputePassEncoder pass1 = encoder.BeginComputePass();
    pass1.SetPipeline(compute);
    pass1.SetBindGroup(0, bindGroup1);
    pass1.DispatchWorkgroups(1);
    pass1.End();

    wgpu::CommandBuffer commands = encoder.Finish();
    queue.Submit(1, &commands);

    // Verify the result.
    EXPECT_BUFFER_U32_EQ(2, buffer, 0);
}

// Write into a storage buffer in a compute pass. Then read that data in a render
// pass. And verify the data flow is correctly synchronized.
TEST_P(GpuMemorySyncTests, ComputePassToRenderPass) {
    // Create pipeline, bind group, and buffer for compute pass and render pass.
    wgpu::Buffer buffer = CreateBuffer();
    auto [compute, bindGroup1] = CreatePipelineAndBindGroupForCompute(buffer);

    utils::BasicRenderPass renderPass = utils::CreateBasicRenderPass(device, 1, 1);
    auto [render, bindGroup0] = CreatePipelineAndBindGroupForRender(buffer, renderPass.colorFormat);

    wgpu::CommandEncoder encoder = device.CreateCommandEncoder();

    // Write data into a storage buffer in compute pass.
    wgpu::ComputePassEncoder pass0 = encoder.BeginComputePass();
    pass0.SetPipeline(compute);
    pass0.SetBindGroup(0, bindGroup1);
    pass0.DispatchWorkgroups(1);
    pass0.End();

    // Read that data in render pass.
    wgpu::RenderPassEncoder pass1 = encoder.BeginRenderPass(&renderPass.renderPassInfo);
    pass1.SetPipeline(render);
    pass1.SetBindGroup(0, bindGroup0);
    pass1.Draw(1);
    pass1.End();

    wgpu::CommandBuffer commands = encoder.Finish();
    queue.Submit(1, &commands);

    // Verify the result.
    EXPECT_PIXEL_RGBA8_EQ(utils::RGBA8(2, 0, 0, 255), renderPass.color, 0, 0);
}

DAWN_INSTANTIATE_TEST(
    GpuMemorySyncTests,
    D3D11Backend(),
    D3D12Backend(),
    MetalBackend(),
    OpenGLBackend(),
    OpenGLESBackend(),
    VulkanBackend(),
    VulkanBackend({"vulkan_split_command_buffer_on_compute_pass_after_render_pass"}));

class StorageToUniformSyncTests : public DawnTest {
  protected:
    void CreateBuffer() {
        wgpu::BufferDescriptor bufferDesc;
        bufferDesc.size = sizeof(float);
        bufferDesc.usage = wgpu::BufferUsage::Storage | wgpu::BufferUsage::Uniform;
        mBuffer = device.CreateBuffer(&bufferDesc);
    }

    std::tuple<wgpu::ComputePipeline, wgpu::BindGroup> CreatePipelineAndBindGroupForCompute() {
        wgpu::ShaderModule csModule = utils::CreateShaderModule(device, R"(
            struct Data {
                a : f32
            }
            @group(0) @binding(0) var<storage, read_write> data : Data;
            @compute @workgroup_size(1) fn main() {
                data.a = 1.0;
            })");

        wgpu::ComputePipelineDescriptor cpDesc;
        cpDesc.compute.module = csModule;
        wgpu::ComputePipeline pipeline = device.CreateComputePipeline(&cpDesc);

        wgpu::BindGroup bindGroup =
            utils::MakeBindGroup(device, pipeline.GetBindGroupLayout(0), {{0, mBuffer}});
        return std::make_tuple(pipeline, bindGroup);
    }

    std::tuple<wgpu::RenderPipeline, wgpu::BindGroup> CreatePipelineAndBindGroupForRender(
        wgpu::TextureFormat colorFormat) {
        wgpu::ShaderModule vsModule = utils::CreateShaderModule(device, R"(
            @vertex fn main() -> @builtin(position) vec4f {
                return vec4f(0.0, 0.0, 0.0, 1.0);
            })");

        wgpu::ShaderModule fsModule = utils::CreateShaderModule(device, R"(
            struct Contents {
                color : f32
            }
            @group(0) @binding(0) var<uniform> contents : Contents;

            @fragment fn main() -> @location(0) vec4f {
                return vec4f(contents.color, 0.0, 0.0, 1.0);
            })");

        utils::ComboRenderPipelineDescriptor rpDesc;
        rpDesc.vertex.module = vsModule;
        rpDesc.cFragment.module = fsModule;
        rpDesc.primitive.topology = wgpu::PrimitiveTopology::PointList;
        rpDesc.cTargets[0].format = colorFormat;

        wgpu::RenderPipeline pipeline = device.CreateRenderPipeline(&rpDesc);

        wgpu::BindGroup bindGroup =
            utils::MakeBindGroup(device, pipeline.GetBindGroupLayout(0), {{0, mBuffer}});
        return std::make_tuple(pipeline, bindGroup);
    }

    wgpu::Buffer mBuffer;
};

// Write into a storage buffer in compute pass in a command buffer. Then read that data in a render
// pass. The two passes use the same command buffer.
TEST_P(StorageToUniformSyncTests, ReadAfterWriteWithSameCommandBuffer) {
    // Create pipeline, bind group, and buffer for compute pass and render pass.
    CreateBuffer();
    utils::BasicRenderPass renderPass = utils::CreateBasicRenderPass(device, 1, 1);
    auto [compute, computeBindGroup] = CreatePipelineAndBindGroupForCompute();
    auto [render, renderBindGroup] = CreatePipelineAndBindGroupForRender(renderPass.colorFormat);

    // Write data into a storage buffer in compute pass.
    wgpu::CommandEncoder encoder0 = device.CreateCommandEncoder();
    wgpu::ComputePassEncoder pass0 = encoder0.BeginComputePass();
    pass0.SetPipeline(compute);
    pass0.SetBindGroup(0, computeBindGroup);
    pass0.DispatchWorkgroups(1);
    pass0.End();

    // Read that data in render pass.
    wgpu::RenderPassEncoder pass1 = encoder0.BeginRenderPass(&renderPass.renderPassInfo);
    pass1.SetPipeline(render);
    pass1.SetBindGroup(0, renderBindGroup);
    pass1.Draw(1);
    pass1.End();

    wgpu::CommandBuffer commands = encoder0.Finish();
    queue.Submit(1, &commands);

    // Verify the rendering result.
    EXPECT_PIXEL_RGBA8_EQ(utils::RGBA8::kRed, renderPass.color, 0, 0);
}

// Write into a storage buffer in compute pass in a command buffer. Then read that data in a render
// pass. The two passes use the different command buffers. The command buffers are submitted to the
// queue in one shot.
TEST_P(StorageToUniformSyncTests, ReadAfterWriteWithDifferentCommandBuffers) {
    // Create pipeline, bind group, and buffer for compute pass and render pass.
    CreateBuffer();
    utils::BasicRenderPass renderPass = utils::CreateBasicRenderPass(device, 1, 1);
    auto [compute, computeBindGroup] = CreatePipelineAndBindGroupForCompute();
    auto [render, renderBindGroup] = CreatePipelineAndBindGroupForRender(renderPass.colorFormat);

    // Write data into a storage buffer in compute pass.
    wgpu::CommandBuffer cb[2];
    wgpu::CommandEncoder encoder0 = device.CreateCommandEncoder();
    wgpu::ComputePassEncoder pass0 = encoder0.BeginComputePass();
    pass0.SetPipeline(compute);
    pass0.SetBindGroup(0, computeBindGroup);
    pass0.DispatchWorkgroups(1);
    pass0.End();
    cb[0] = encoder0.Finish();

    // Read that data in render pass.
    wgpu::CommandEncoder encoder1 = device.CreateCommandEncoder();
    wgpu::RenderPassEncoder pass1 = encoder1.BeginRenderPass(&renderPass.renderPassInfo);
    pass1.SetPipeline(render);
    pass1.SetBindGroup(0, renderBindGroup);
    pass1.Draw(1);
    pass1.End();

    cb[1] = encoder1.Finish();
    queue.Submit(2, cb);

    // Verify the rendering result.
    EXPECT_PIXEL_RGBA8_EQ(utils::RGBA8::kRed, renderPass.color, 0, 0);
}

// Write into a storage buffer in compute pass in a command buffer. Then read that data in a render
// pass. The two passes use the different command buffers. The command buffers are submitted to the
// queue separately.
TEST_P(StorageToUniformSyncTests, ReadAfterWriteWithDifferentQueueSubmits) {
    // Create pipeline, bind group, and buffer for compute pass and render pass.
    CreateBuffer();
    utils::BasicRenderPass renderPass = utils::CreateBasicRenderPass(device, 1, 1);
    auto [compute, computeBindGroup] = CreatePipelineAndBindGroupForCompute();
    auto [render, renderBindGroup] = CreatePipelineAndBindGroupForRender(renderPass.colorFormat);

    // Write data into a storage buffer in compute pass.
    wgpu::CommandBuffer cb[2];
    wgpu::CommandEncoder encoder0 = device.CreateCommandEncoder();
    wgpu::ComputePassEncoder pass0 = encoder0.BeginComputePass();
    pass0.SetPipeline(compute);
    pass0.SetBindGroup(0, computeBindGroup);
    pass0.DispatchWorkgroups(1);
    pass0.End();
    cb[0] = encoder0.Finish();
    queue.Submit(1, &cb[0]);

    // Read that data in render pass.
    wgpu::CommandEncoder encoder1 = device.CreateCommandEncoder();
    wgpu::RenderPassEncoder pass1 = encoder1.BeginRenderPass(&renderPass.renderPassInfo);
    pass1.SetPipeline(render);
    pass1.SetBindGroup(0, renderBindGroup);
    pass1.Draw(1);
    pass1.End();

    cb[1] = encoder1.Finish();
    queue.Submit(1, &cb[1]);

    // Verify the rendering result.
    EXPECT_PIXEL_RGBA8_EQ(utils::RGBA8::kRed, renderPass.color, 0, 0);
}

DAWN_INSTANTIATE_TEST(StorageToUniformSyncTests,
                      D3D11Backend(),
                      D3D12Backend(),
                      MetalBackend(),
                      OpenGLBackend(),
                      OpenGLESBackend(),
                      VulkanBackend());

constexpr int kRTSize = 8;
constexpr int kVertexBufferStride = 4 * sizeof(float);

class MultipleWriteThenMultipleReadTests : public DawnTest {
  protected:
    wgpu::RequiredLimits GetRequiredLimits(const wgpu::SupportedLimits& supported) override {
        // Just copy all the limits, though all we really care about is
        // maxStorageBuffersInFragmentStage
        wgpu::RequiredLimits required = {};
        required.limits = supported.limits;
        return required;
    }

    void SetUp() override {
        DawnTest::SetUp();
        DAWN_SUPPRESS_TEST_IF(GetSupportedLimits().limits.maxStorageBuffersInFragmentStage < 1);
    }

  protected:
    wgpu::Buffer CreateZeroedBuffer(uint64_t size, wgpu::BufferUsage usage) {
        wgpu::BufferDescriptor srcDesc;
        srcDesc.size = size;
        srcDesc.usage = usage;
        wgpu::Buffer buffer = device.CreateBuffer(&srcDesc);

        std::vector<uint8_t> zeros(size, 0);
        queue.WriteBuffer(buffer, 0, zeros.data(), size);

        return buffer;
    }
};

// Write into a few storage buffers in compute pass. Then read that data in a render pass. The
// readonly buffers in render pass include vertex buffer, index buffer, uniform buffer, and readonly
// storage buffer. Data to be read in all of these buffers in render pass depend on the write
// operation in compute pass.
TEST_P(MultipleWriteThenMultipleReadTests, SeparateBuffers) {
    // TODO(crbug.com/dawn/2295): diagnose this failure on Pixel 6 OpenGLES
    DAWN_SUPPRESS_TEST_IF(IsOpenGLES() && IsAndroid() && IsARM());

    // Create pipeline, bind group, and different buffers for compute pass.
    wgpu::ShaderModule csModule = utils::CreateShaderModule(device, R"(
        struct VBContents {
            pos : array<vec4f, 4>
        }
        @group(0) @binding(0) var<storage, read_write> vbContents : VBContents;

        struct IBContents {
            indices : array<vec4i, 2>
        }
        @group(0) @binding(1) var<storage, read_write> ibContents : IBContents;

        struct ColorContents {
            color : f32
        }
        @group(0) @binding(2) var<storage, read_write> uniformContents : ColorContents;
        @group(0) @binding(3) var<storage, read_write> storageContents : ColorContents;

        @compute @workgroup_size(1) fn main() {
            vbContents.pos[0] = vec4f(-1.0, 1.0, 0.0, 1.0);
            vbContents.pos[1] = vec4f(1.0, 1.0, 0.0, 1.0);
            vbContents.pos[2] = vec4f(1.0, -1.0, 0.0, 1.0);
            vbContents.pos[3] = vec4f(-1.0, -1.0, 0.0, 1.0);
            let placeholder : i32 = 0;
            ibContents.indices[0] = vec4i(0, 1, 2, 0);
            ibContents.indices[1] = vec4i(2, 3, placeholder, placeholder);
            uniformContents.color = 1.0;
            storageContents.color = 1.0;
        })");

    wgpu::ComputePipelineDescriptor cpDesc;
    cpDesc.compute.module = csModule;
    wgpu::ComputePipeline cp = device.CreateComputePipeline(&cpDesc);
    wgpu::Buffer vertexBuffer = CreateZeroedBuffer(
        kVertexBufferStride * 4,
        wgpu::BufferUsage::Vertex | wgpu::BufferUsage::Storage | wgpu::BufferUsage::CopyDst);
    wgpu::Buffer indexBuffer = CreateZeroedBuffer(
        sizeof(int) * 4 * 2,
        wgpu::BufferUsage::Index | wgpu::BufferUsage::Storage | wgpu::BufferUsage::CopyDst);
    wgpu::Buffer uniformBuffer =
        CreateZeroedBuffer(sizeof(float), wgpu::BufferUsage::Uniform | wgpu::BufferUsage::Storage |
                                              wgpu::BufferUsage::CopyDst);
    wgpu::Buffer storageBuffer =
        CreateZeroedBuffer(sizeof(float), wgpu::BufferUsage::Storage | wgpu::BufferUsage::CopyDst);

    wgpu::BindGroup bindGroup0 = utils::MakeBindGroup(
        device, cp.GetBindGroupLayout(0),
        {{0, vertexBuffer}, {1, indexBuffer}, {2, uniformBuffer}, {3, storageBuffer}});
    // Write data into storage buffers in compute pass.
    wgpu::CommandEncoder encoder = device.CreateCommandEncoder();
    wgpu::ComputePassEncoder pass0 = encoder.BeginComputePass();
    pass0.SetPipeline(cp);
    pass0.SetBindGroup(0, bindGroup0);
    pass0.DispatchWorkgroups(1);
    pass0.End();

    // Create pipeline, bind group, and reuse buffers in render pass.
    wgpu::ShaderModule vsModule = utils::CreateShaderModule(device, R"(
        @vertex
        fn main(@location(0) pos : vec4f) -> @builtin(position) vec4f {
            return pos;
        })");

    wgpu::ShaderModule fsModule = utils::CreateShaderModule(device, R"(
        struct Buf {
            color : f32
        }

        @group(0) @binding(0) var<uniform> uniformBuffer : Buf;
        @group(0) @binding(1) var<storage, read> storageBuffer : Buf;

        @fragment fn main() -> @location(0) vec4f {
            return vec4f(uniformBuffer.color, storageBuffer.color, 0.0, 1.0);
        })");

    utils::BasicRenderPass renderPass = utils::CreateBasicRenderPass(device, kRTSize, kRTSize);

    utils::ComboRenderPipelineDescriptor rpDesc;
    rpDesc.vertex.module = vsModule;
    rpDesc.cFragment.module = fsModule;
    rpDesc.primitive.topology = wgpu::PrimitiveTopology::TriangleList;
    rpDesc.vertex.bufferCount = 1;
    rpDesc.cBuffers[0].arrayStride = kVertexBufferStride;
    rpDesc.cBuffers[0].attributeCount = 1;
    rpDesc.cAttributes[0].format = wgpu::VertexFormat::Float32x4;
    rpDesc.cTargets[0].format = renderPass.colorFormat;

    wgpu::RenderPipeline rp = device.CreateRenderPipeline(&rpDesc);

    wgpu::BindGroup bindGroup1 = utils::MakeBindGroup(device, rp.GetBindGroupLayout(0),
                                                      {{0, uniformBuffer}, {1, storageBuffer}});

    // Read data in buffers in render pass.
    wgpu::RenderPassEncoder pass1 = encoder.BeginRenderPass(&renderPass.renderPassInfo);
    pass1.SetPipeline(rp);
    pass1.SetVertexBuffer(0, vertexBuffer);
    pass1.SetIndexBuffer(indexBuffer, wgpu::IndexFormat::Uint32, 0);
    pass1.SetBindGroup(0, bindGroup1);
    pass1.DrawIndexed(6);
    pass1.End();

    wgpu::CommandBuffer commandBuffer = encoder.Finish();
    queue.Submit(1, &commandBuffer);

    // Verify the rendering result.
    uint32_t min = 1, max = kRTSize - 3;
    EXPECT_PIXEL_RGBA8_EQ(utils::RGBA8::kYellow, renderPass.color, min, min);
    EXPECT_PIXEL_RGBA8_EQ(utils::RGBA8::kYellow, renderPass.color, max, min);
    EXPECT_PIXEL_RGBA8_EQ(utils::RGBA8::kYellow, renderPass.color, min, max);
    EXPECT_PIXEL_RGBA8_EQ(utils::RGBA8::kYellow, renderPass.color, max, max);
}

// Write into a storage buffer in compute pass. Then read that data in buffer in a render pass. The
// buffer is composed of vertices, indices, uniforms and readonly storage. Data to be read in the
// buffer in render pass depend on the write operation in compute pass.
TEST_P(MultipleWriteThenMultipleReadTests, OneBuffer) {
    // TODO(crbug.com/dawn/646): diagnose this failure on Pixel 6 OpenGLES
    DAWN_SUPPRESS_TEST_IF(IsOpenGLES() && IsAndroid() && IsARM());

    // Create pipeline, bind group, and a complex buffer for compute pass.
    wgpu::ShaderModule csModule = utils::CreateShaderModule(device, R"(
        struct Contents {
            @align(256) pos : array<vec4f, 4>,
            @align(256) indices : array<vec4i, 2>,
            @align(256) color0 : f32,
            @align(256) color1 : f32,
        }

        @group(0) @binding(0) var<storage, read_write> contents : Contents;

        @compute @workgroup_size(1) fn main() {
            contents.pos[0] = vec4f(-1.0, 1.0, 0.0, 1.0);
            contents.pos[1] = vec4f(1.0, 1.0, 0.0, 1.0);
            contents.pos[2] = vec4f(1.0, -1.0, 0.0, 1.0);
            contents.pos[3] = vec4f(-1.0, -1.0, 0.0, 1.0);
            let placeholder : i32 = 0;
            contents.indices[0] = vec4i(0, 1, 2, 0);
            contents.indices[1] = vec4i(2, 3, placeholder, placeholder);
            contents.color0 = 1.0;
            contents.color1 = 1.0;
        })");

    wgpu::ComputePipelineDescriptor cpDesc;
    cpDesc.compute.module = csModule;
    wgpu::ComputePipeline cp = device.CreateComputePipeline(&cpDesc);
    struct Data {
        float pos[4][4];
        char padding0[256 - sizeof(float) * 16];
        int indices[2][4];
        char padding1[256 - sizeof(int) * 8];
        float color0;
        char padding2[256 - sizeof(float)];
        float color1;
        char padding3[256 - sizeof(float)];
    };
    wgpu::Buffer buffer = CreateZeroedBuffer(
        sizeof(Data), wgpu::BufferUsage::Vertex | wgpu::BufferUsage::Index |
                          wgpu::BufferUsage::Uniform | wgpu::BufferUsage::Storage |
                          wgpu::BufferUsage::CopyDst);
    wgpu::BindGroup bindGroup0 =
        utils::MakeBindGroup(device, cp.GetBindGroupLayout(0), {{0, buffer}});

    // Write various data (vertices, indices, and uniforms) into the buffer in compute pass.
    wgpu::CommandEncoder encoder = device.CreateCommandEncoder();
    wgpu::ComputePassEncoder pass0 = encoder.BeginComputePass();
    pass0.SetPipeline(cp);
    pass0.SetBindGroup(0, bindGroup0);
    pass0.DispatchWorkgroups(1);
    pass0.End();

    // Create pipeline, bind group, and reuse the buffer in render pass.
    wgpu::ShaderModule vsModule = utils::CreateShaderModule(device, R"(
        @vertex
        fn main(@location(0) pos : vec4f) -> @builtin(position) vec4f {
            return pos;
        })");

    wgpu::ShaderModule fsModule = utils::CreateShaderModule(device, R"(
        struct Buf {
            color : f32
        }
        @group(0) @binding(0) var<uniform> uniformBuffer : Buf;
        @group(0) @binding(1) var<storage, read> storageBuffer : Buf;

        @fragment fn main() -> @location(0) vec4f {
            return vec4f(uniformBuffer.color, storageBuffer.color, 0.0, 1.0);
        })");

    utils::BasicRenderPass renderPass = utils::CreateBasicRenderPass(device, kRTSize, kRTSize);

    utils::ComboRenderPipelineDescriptor rpDesc;
    rpDesc.vertex.module = vsModule;
    rpDesc.cFragment.module = fsModule;
    rpDesc.primitive.topology = wgpu::PrimitiveTopology::TriangleList;
    rpDesc.vertex.bufferCount = 1;
    rpDesc.cBuffers[0].arrayStride = kVertexBufferStride;
    rpDesc.cBuffers[0].attributeCount = 1;
    rpDesc.cAttributes[0].format = wgpu::VertexFormat::Float32x4;
    rpDesc.cTargets[0].format = renderPass.colorFormat;

    wgpu::RenderPipeline rp = device.CreateRenderPipeline(&rpDesc);

    wgpu::BindGroup bindGroup1 =
        utils::MakeBindGroup(device, rp.GetBindGroupLayout(0),
                             {{0, buffer, offsetof(Data, color0), sizeof(float)},
                              {1, buffer, offsetof(Data, color1), sizeof(float)}});

    // Read various data in the buffer in render pass.
    wgpu::RenderPassEncoder pass1 = encoder.BeginRenderPass(&renderPass.renderPassInfo);
    pass1.SetPipeline(rp);
    pass1.SetVertexBuffer(0, buffer);
    pass1.SetIndexBuffer(buffer, wgpu::IndexFormat::Uint32, offsetof(Data, indices));
    pass1.SetBindGroup(0, bindGroup1);
    pass1.DrawIndexed(6);
    pass1.End();

    wgpu::CommandBuffer commandBuffer = encoder.Finish();
    queue.Submit(1, &commandBuffer);

    // Verify the rendering result.
    uint32_t min = 1, max = kRTSize - 3;
    EXPECT_PIXEL_RGBA8_EQ(utils::RGBA8::kYellow, renderPass.color, min, min);
    EXPECT_PIXEL_RGBA8_EQ(utils::RGBA8::kYellow, renderPass.color, max, min);
    EXPECT_PIXEL_RGBA8_EQ(utils::RGBA8::kYellow, renderPass.color, min, max);
    EXPECT_PIXEL_RGBA8_EQ(utils::RGBA8::kYellow, renderPass.color, max, max);
}

DAWN_INSTANTIATE_TEST(MultipleWriteThenMultipleReadTests,
                      D3D11Backend(),
                      D3D12Backend(),
                      MetalBackend(),
                      OpenGLBackend(),
                      OpenGLESBackend(),
                      VulkanBackend());

}  // anonymous namespace
}  // namespace dawn
