// Copyright 2024 The Dawn & Tint Authors
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

#include <iostream>
#include <vector>

#include "dawn/tests/DawnTest.h"

#include "dawn/utils/ComboRenderPipelineDescriptor.h"
#include "dawn/utils/WGPUHelpers.h"

namespace dawn {
namespace {

constexpr uint32_t kRTSize = 4;
constexpr utils::RGBA8 filled(0, 255, 0, 255);
constexpr utils::RGBA8 notFilled(0, 0, 0, 0);

class MultiDrawIndirectTest : public DawnTest {
  protected:
    std::vector<wgpu::FeatureName> GetRequiredFeatures() override {
        if (!SupportsFeatures({wgpu::FeatureName::MultiDrawIndirect})) {
            return {};
        }
        return {wgpu::FeatureName::MultiDrawIndirect};
    }

    void SetUp() override {
        DawnTest::SetUp();
        DAWN_TEST_UNSUPPORTED_IF(!device.HasFeature(wgpu::FeatureName::MultiDrawIndirect));

        renderPass = utils::CreateBasicRenderPass(device, kRTSize, kRTSize);

        wgpu::ShaderModule vsModule = utils::CreateShaderModule(device, R"(
            @vertex
            fn main(@location(0) pos : vec4f) -> @builtin(position) vec4f {
                return pos;
            })");

        wgpu::ShaderModule fsModule = utils::CreateShaderModule(device, R"(
            @fragment fn main() -> @location(0) vec4f {
                return vec4f(0.0, 1.0, 0.0, 1.0);
            })");

        utils::ComboRenderPipelineDescriptor descriptor;
        descriptor.vertex.module = vsModule;
        descriptor.cFragment.module = fsModule;
        descriptor.primitive.topology = wgpu::PrimitiveTopology::TriangleStrip;
        descriptor.primitive.stripIndexFormat = wgpu::IndexFormat::Uint32;
        descriptor.vertex.bufferCount = 1;
        descriptor.cBuffers[0].arrayStride = 4 * sizeof(float);
        descriptor.cBuffers[0].attributeCount = 1;
        descriptor.cAttributes[0].format = wgpu::VertexFormat::Float32x4;
        descriptor.cTargets[0].format = renderPass.colorFormat;

        pipeline = device.CreateRenderPipeline(&descriptor);

        vertexBuffer = utils::CreateBufferFromData<float>(
            device, wgpu::BufferUsage::Vertex,
            {// The bottom left triangle
             -1.0f, 1.0f, 0.0f, 1.0f, 1.0f, -1.0f, 0.0f, 1.0f, -1.0f, -1.0f, 0.0f, 1.0f,

             // The top right triangle
             -1.0f, 1.0f, 0.0f, 1.0f, 1.0f, -1.0f, 0.0f, 1.0f, 1.0f, 1.0f, 0.0f, 1.0f});
    }

    utils::BasicRenderPass renderPass;
    wgpu::RenderPipeline pipeline;
    wgpu::Buffer vertexBuffer;

    void Test(std::initializer_list<uint32_t> bufferList,
              uint64_t indirectOffset,
              uint32_t maxDrawCount,
              utils::RGBA8 bottomLeftExpected,
              utils::RGBA8 topRightExpected,
              wgpu::Buffer drawCountBuffer = nullptr,
              uint64_t drawCountOffset = 0) {
        wgpu::Buffer indirectBuffer =
            utils::CreateBufferFromData<uint32_t>(device, wgpu::BufferUsage::Indirect, bufferList);

        wgpu::CommandEncoder encoder = device.CreateCommandEncoder();
        {
            wgpu::RenderPassEncoder pass = encoder.BeginRenderPass(&renderPass.renderPassInfo);
            pass.SetPipeline(pipeline);
            pass.SetVertexBuffer(0, vertexBuffer);
            pass.MultiDrawIndirect(indirectBuffer, indirectOffset, maxDrawCount, drawCountBuffer,
                                   drawCountOffset);
            pass.End();
        }

        wgpu::CommandBuffer commands = encoder.Finish();
        queue.Submit(1, &commands);

        EXPECT_PIXEL_RGBA8_EQ(bottomLeftExpected, renderPass.color, 1, 3);
        EXPECT_PIXEL_RGBA8_EQ(topRightExpected, renderPass.color, 3, 1);
    }
};

// The basic triangle draw.
TEST_P(MultiDrawIndirectTest, Uint32) {
    // Test a draw with no indices.
    Test({0, 0, 0, 0}, 0, 1, notFilled, notFilled);

    // Test a draw with only the first 3 indices (bottom left triangle)
    Test({3, 1, 0, 0}, 0, 1, filled, notFilled);

    // Test a draw with only the last 3 indices (top right triangle)
    Test({3, 1, 3, 0}, 0, 1, notFilled, filled);

    // Test a draw with all 6 indices (both triangles)
    Test({6, 1, 0, 0}, 0, 1, filled, filled);

    // Test a draw with 2 draw commands (both triangles)
    Test({3, 1, 0, 0, 3, 1, 3, 0}, 0, 2, filled, filled);
}

// The basic triangle draw with various drawCount.
TEST_P(MultiDrawIndirectTest, DrawCount) {
    // TODO(crbug.com/356461286): NVIDIA Drivers for Vulkan Linux are drawing more than
    // maxDrawCount.
    DAWN_SUPPRESS_TEST_IF(IsLinux() && IsNvidia() && IsVulkan());

    wgpu::Buffer drawBuffer =
        utils::CreateBufferFromData<uint32_t>(device, wgpu::BufferUsage::Indirect, {0, 1, 2});
    // Test a draw with drawCount = 0.
    Test({3, 1, 0, 0, 3, 1, 3, 0}, 0, 2, notFilled, notFilled, drawBuffer, 0);
    // Test a draw with drawCount < maxDrawCount.
    Test({3, 1, 0, 0, 3, 1, 3, 0}, 0, 2, filled, notFilled, drawBuffer, 4);
    // Test a draw with drawCount > maxDrawCount.
    Test({3, 1, 0, 0, 3, 1, 3, 0}, 0, 1, filled, notFilled, drawBuffer, 8);
    // Test a draw with drawCount = maxDrawCount.
    Test({3, 1, 0, 0, 3, 1, 3, 0}, 0, 2, filled, filled, drawBuffer, 8);
    // Test a draw without drawCount buffer. It should be treated as drawCount = maxDrawCount.
    Test({3, 1, 0, 0, 3, 1, 3, 0}, 0, 2, filled, filled);
}

// Test with both indirect draw and multi draw.
TEST_P(MultiDrawIndirectTest, IndirectOffset) {
    // Test #1 (no offset)
    Test({3, 1, 0, 0, 3, 1, 3, 0}, 0, 1, filled, notFilled);

    // Offset to draw #2
    Test({3, 1, 0, 0, 3, 1, 3, 0}, kDrawIndirectSize, 1, notFilled, filled);
}

DAWN_INSTANTIATE_TEST(MultiDrawIndirectTest, VulkanBackend(), D3D12Backend(), MetalBackend());

class MultiDrawIndirectUsingFirstVertexTest : public DawnTest {
  protected:
    std::vector<wgpu::FeatureName> GetRequiredFeatures() override {
        if (!SupportsFeatures({wgpu::FeatureName::MultiDrawIndirect})) {
            return {};
        }
        return {wgpu::FeatureName::MultiDrawIndirect};
    }
    virtual void SetupShaderModule() {
        vsModule = utils::CreateShaderModule(device, R"(
            struct VertexInput {
                @builtin(vertex_index) id : u32,
                @location(0) pos: vec4f,
            };
            @group(0) @binding(0) var<uniform> offset: array<vec4f, 2>;
            @vertex
            fn main(input: VertexInput) -> @builtin(position) vec4f {
                return input.pos + offset[input.id / 3u];
            })");
        fsModule = utils::CreateShaderModule(device, R"(
            @fragment fn main() -> @location(0) vec4f {
                return vec4f(0.0, 1.0, 0.0, 1.0);
            })");
    }
    void GeneralSetup() {
        renderPass = utils::CreateBasicRenderPass(device, kRTSize, kRTSize);
        SetupShaderModule();
        utils::ComboRenderPipelineDescriptor descriptor;
        descriptor.vertex.module = vsModule;
        descriptor.cFragment.module = fsModule;
        descriptor.primitive.topology = wgpu::PrimitiveTopology::TriangleStrip;
        descriptor.primitive.stripIndexFormat = wgpu::IndexFormat::Uint32;
        descriptor.vertex.bufferCount = 1;
        descriptor.cBuffers[0].arrayStride = 4 * sizeof(float);
        descriptor.cBuffers[0].attributeCount = 1;
        descriptor.cAttributes[0].format = wgpu::VertexFormat::Float32x4;
        descriptor.cTargets[0].format = renderPass.colorFormat;

        pipeline = device.CreateRenderPipeline(&descriptor);

        // Offset to the vertices, that needs correcting by the calibration offset from uniform
        // buffer referenced by instance index to get filled triangle on screen.
        constexpr float calibration = 99.0f;
        vertexBuffer = utils::CreateBufferFromData<float>(
            device, wgpu::BufferUsage::Vertex,
            {// The bottom left triangle
             -1.0f - calibration, 1.0f, 0.0f, 1.0f, 1.0f - calibration, -1.0f, 0.0f, 1.0f,
             -1.0f - calibration, -1.0f, 0.0f, 1.0f,
             // The top right triangle
             -1.0f - calibration, 1.0f, 0.0f, 1.0f, 1.0f - calibration, -1.0f, 0.0f, 1.0f,
             1.0f - calibration, 1.0f, 0.0f, 1.0f});
        // Providing calibration vec4f offset values
        wgpu::Buffer uniformBuffer =
            utils::CreateBufferFromData<float>(device, wgpu::BufferUsage::Uniform,
                                               {
                                                   // Bad calibration at [0]
                                                   0.0,
                                                   0.0,
                                                   0.0,
                                                   0.0,
                                                   // Good calibration at [1]
                                                   calibration,
                                                   0.0,
                                                   0.0,
                                                   0.0,
                                               });
        bindGroup =
            utils::MakeBindGroup(device, pipeline.GetBindGroupLayout(0), {{0, uniformBuffer}});
    }
    void SetUp() override {
        DawnTest::SetUp();
        DAWN_TEST_UNSUPPORTED_IF(!device.HasFeature(wgpu::FeatureName::MultiDrawIndirect));
        GeneralSetup();
    }
    utils::BasicRenderPass renderPass;
    wgpu::RenderPipeline pipeline;
    wgpu::Buffer vertexBuffer;
    wgpu::BindGroup bindGroup;
    wgpu::ShaderModule vsModule;
    wgpu::ShaderModule fsModule;
    // Test two DrawIndirect calls with different indirect offsets within one pass.
    void Test(std::initializer_list<uint32_t> bufferList,
              uint32_t maxDrawCount,
              utils::RGBA8 bottomLeftExpected,
              utils::RGBA8 topRightExpected) {
        wgpu::Buffer indirectBuffer =
            utils::CreateBufferFromData<uint32_t>(device, wgpu::BufferUsage::Indirect, bufferList);
        wgpu::CommandEncoder encoder = device.CreateCommandEncoder();
        {
            wgpu::RenderPassEncoder pass = encoder.BeginRenderPass(&renderPass.renderPassInfo);
            pass.SetPipeline(pipeline);
            pass.SetVertexBuffer(0, vertexBuffer);
            pass.SetBindGroup(0, bindGroup);
            pass.MultiDrawIndirect(indirectBuffer, 0, maxDrawCount, nullptr, 0);
            pass.End();
        }
        wgpu::CommandBuffer commands = encoder.Finish();
        queue.Submit(1, &commands);
        EXPECT_PIXEL_RGBA8_EQ(bottomLeftExpected, renderPass.color, 1, 3);
        EXPECT_PIXEL_RGBA8_EQ(topRightExpected, renderPass.color, 3, 1);
    }
};

TEST_P(MultiDrawIndirectUsingFirstVertexTest, IndirectOffset) {
    // Test an offset draw call, with indirect buffer containing 2 calls:
    // 1) only the first 3 indices (bottom left triangle)
    // 2) only the last 3 indices (top right triangle)
    // #2 draw has the correct offset applied by vertex index
    Test({3, 1, 0, 0, 3, 1, 3, 0}, 2, notFilled, filled);
}

DAWN_INSTANTIATE_TEST(MultiDrawIndirectUsingFirstVertexTest,
                      VulkanBackend(),
                      D3D12Backend(),
                      MetalBackend());

class MultiDrawIndirectUsingInstanceIndexTest : public MultiDrawIndirectUsingFirstVertexTest {
  protected:
    std::vector<wgpu::FeatureName> GetRequiredFeatures() override {
        if (!SupportsFeatures({wgpu::FeatureName::MultiDrawIndirect})) {
            return {};
        }
        return {wgpu::FeatureName::MultiDrawIndirect};
    }

    void SetupShaderModule() override {
        vsModule = utils::CreateShaderModule(device, R"(
            struct VertexInput {
                @builtin(instance_index) id : u32,
                @location(0) pos: vec4f,
            };

            @group(0) @binding(0) var<uniform> offset: array<vec4f, 2>;

            @vertex
            fn main(input: VertexInput) -> @builtin(position) vec4f {
                return input.pos + offset[input.id];
            })");

        fsModule = utils::CreateShaderModule(device, R"(
            @fragment fn main() -> @location(0) vec4f {
                return vec4f(0.0, 1.0, 0.0, 1.0);
            })");
    }

    void SetUp() override {
        DawnTest::SetUp();
        DAWN_TEST_UNSUPPORTED_IF(!device.HasFeature(wgpu::FeatureName::MultiDrawIndirect));
        GeneralSetup();
    }
};

TEST_P(MultiDrawIndirectUsingInstanceIndexTest, IndirectOffset) {
    // Test an offset draw call, with indirect buffer containing 2 calls:
    // 1) only the first 3 indices (bottom left triangle)
    // 2) only the last 3 indices (top right triangle)

    // Test 1: #1 draw has the correct calibration referenced by instance index
    Test({3, 1, 0, 1, 3, 1, 3, 0}, 2, filled, notFilled);

    // Test 2: #2 draw has the correct offset applied by instance index
    Test({3, 1, 0, 0, 3, 1, 3, 1}, 2, notFilled, filled);
}

DAWN_INSTANTIATE_TEST(MultiDrawIndirectUsingInstanceIndexTest,
                      VulkanBackend(),
                      D3D12Backend(),
                      MetalBackend());

}  // namespace
}  // namespace dawn
