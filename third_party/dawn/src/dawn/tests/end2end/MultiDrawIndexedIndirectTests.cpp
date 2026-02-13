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

#include <vector>

#include "dawn/tests/DawnTest.h"
#include "dawn/utils/ComboRenderBundleEncoderDescriptor.h"
#include "dawn/utils/ComboRenderPipelineDescriptor.h"
#include "dawn/utils/WGPUHelpers.h"

namespace dawn {
namespace {

constexpr uint32_t kRTSize = 4;

class MultiDrawIndexedIndirectTest : public DawnTest {
  protected:
    void GetRequiredLimits(const dawn::utils::ComboLimits& supported,
                           dawn::utils::ComboLimits& required) override {
        // Force larger limits, that might reach into the upper 32 bits of the 64bit limit values,
        // to help detect integer arithmetic bugs like overflows and truncations.
        supported.UnlinkedCopyTo(&required);
    }

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
            {// First quad: the first 3 vertices represent the bottom left triangle
             -1.0f, 1.0f, 0.0f, 1.0f, 1.0f, -1.0f, 0.0f, 1.0f, -1.0f, -1.0f, 0.0f, 1.0f, 1.0f, 1.0f,
             0.0f, 1.0f,

             // Second quad: the first 3 vertices represent the top right triangle
             -1.0f, 1.0f, 0.0f, 1.0f, 1.0f, -1.0f, 0.0f, 1.0f, 1.0f, 1.0f, 0.0f, 1.0f, -1.0f, -1.0f,
             0.0f, 1.0f});
    }

    utils::BasicRenderPass renderPass;
    wgpu::RenderPipeline pipeline;
    wgpu::Buffer vertexBuffer;

    wgpu::Buffer CreateIndirectBuffer(std::initializer_list<uint32_t> indirectParamList) {
        return utils::CreateBufferFromData<uint32_t>(
            device, wgpu::BufferUsage::Indirect | wgpu::BufferUsage::Storage, indirectParamList);
    }

    wgpu::Buffer CreateIndexBuffer(std::initializer_list<uint32_t> indexList) {
        return utils::CreateBufferFromData<uint32_t>(device, wgpu::BufferUsage::Index, indexList);
    }

    wgpu::CommandBuffer EncodeDrawCommands(std::initializer_list<uint32_t> bufferList,
                                           wgpu::Buffer indexBuffer,
                                           uint64_t indexOffset,
                                           uint64_t indirectOffset,
                                           uint32_t maxDrawCount,
                                           wgpu::Buffer drawCountBuffer = nullptr,
                                           uint64_t drawCountOffset = 0) {
        wgpu::Buffer indirectBuffer = CreateIndirectBuffer(bufferList);

        wgpu::CommandEncoder encoder = device.CreateCommandEncoder();
        {
            wgpu::RenderPassEncoder pass = encoder.BeginRenderPass(&renderPass.renderPassInfo);
            pass.SetPipeline(pipeline);
            pass.SetVertexBuffer(0, vertexBuffer);
            pass.SetIndexBuffer(indexBuffer, wgpu::IndexFormat::Uint32, indexOffset);
            pass.MultiDrawIndexedIndirect(indirectBuffer, indirectOffset, maxDrawCount,
                                          drawCountBuffer, drawCountOffset);
            pass.End();
        }

        return encoder.Finish();
    }

    void TestDraw(wgpu::CommandBuffer commands,
                  utils::RGBA8 bottomLeftExpected,
                  utils::RGBA8 topRightExpected) {
        queue.Submit(1, &commands);

        EXPECT_PIXEL_RGBA8_EQ(bottomLeftExpected, renderPass.color, 1, 3);
        EXPECT_PIXEL_RGBA8_EQ(topRightExpected, renderPass.color, 3, 1);
    }

    void Test(std::initializer_list<uint32_t> bufferList,
              uint64_t indexOffset,
              uint64_t indirectOffset,
              uint32_t maxDrawCount,
              utils::RGBA8 bottomLeftExpected,
              utils::RGBA8 topRightExpected) {
        wgpu::Buffer indexBuffer =
            CreateIndexBuffer({0, 1, 2, 0, 3, 1,
                               // The indices below are added to test negative baseVertex
                               0 + 4, 1 + 4, 2 + 4, 0 + 4, 3 + 4, 1 + 4});
        TestDraw(
            EncodeDrawCommands(bufferList, indexBuffer, indexOffset, indirectOffset, maxDrawCount),
            bottomLeftExpected, topRightExpected);
    }
};

// The most basic DrawIndexed triangle draw.
TEST_P(MultiDrawIndexedIndirectTest, Uint32) {
    utils::RGBA8 filled(0, 255, 0, 255);
    utils::RGBA8 notFilled(0, 0, 0, 0);

    // Test a draw with no indices.
    Test({0, 0, 0, 0, 0}, 0, 0, 1, notFilled, notFilled);

    // Test a draw with only the first 3 indices of the first quad (bottom left triangle)
    Test({3, 1, 0, 0, 0}, 0, 0, 1, filled, notFilled);

    // Test a draw with only the last 3 indices of the first quad (top right triangle)
    Test({3, 1, 3, 0, 0}, 0, 0, 1, notFilled, filled);

    // Test a draw with all 6 indices (both triangles).
    Test({6, 1, 0, 0, 0}, 0, 0, 1, filled, filled);
}

// Test the parameter 'baseVertex' of DrawIndexed() works.
TEST_P(MultiDrawIndexedIndirectTest, BaseVertex) {
    utils::RGBA8 filled(0, 255, 0, 255);
    utils::RGBA8 notFilled(0, 0, 0, 0);

    // Test a draw with only the first 3 indices of the second quad (top right triangle)
    Test({3, 1, 0, 4, 0}, 0, 0, 1, notFilled, filled);

    // Test a draw with only the last 3 indices of the second quad (bottom left triangle)
    Test({3, 1, 3, 4, 0}, 0, 0, 1, filled, notFilled);

    const int negFour = -4;
    uint32_t unsignedNegFour;
    std::memcpy(&unsignedNegFour, &negFour, sizeof(int));

    // Test negative baseVertex
    // Test a draw with only the first 3 indices of the first quad (bottom left triangle)
    Test({3, 1, 0, unsignedNegFour, 0}, 6 * sizeof(uint32_t), 0, 1, filled, notFilled);

    // Test a draw with only the last 3 indices of the first quad (top right triangle)
    Test({3, 1, 3, unsignedNegFour, 0}, 6 * sizeof(uint32_t), 0, 1, notFilled, filled);

    // Test a draw with only the last 3 indices of the first quad (top right triangle) and offset
    Test({0, 3, 1, 3, unsignedNegFour, 0}, 6 * sizeof(uint32_t), 1 * sizeof(uint32_t), 1, notFilled,
         filled);
}

TEST_P(MultiDrawIndexedIndirectTest, IndirectOffset) {
    utils::RGBA8 filled(0, 255, 0, 255);
    utils::RGBA8 notFilled(0, 0, 0, 0);

    // Test an offset draw call, with indirect buffer containing 2 calls:
    // 1) first 3 indices of the second quad (top right triangle)
    // 2) last 3 indices of the second quad

    // Test #1 (no offset)
    Test({3, 1, 0, 4, 0, 3, 1, 3, 4, 0}, 0, 0, 1, notFilled, filled);

    // Offset to draw #2
    Test({3, 1, 0, 4, 0, 3, 1, 3, 4, 0}, 0, 5 * sizeof(uint32_t), 1, filled, notFilled);
}

// The basic triangle draw with various drawCount.
TEST_P(MultiDrawIndexedIndirectTest, DrawCount) {
    // TODO(crbug.com/356461286): NVIDIA Drivers for Vulkan Linux are drawing more than specified.
    DAWN_SUPPRESS_TEST_IF(IsLinux() && IsNvidia() && IsVulkan());

    utils::RGBA8 filled(0, 255, 0, 255);
    utils::RGBA8 notFilled(0, 0, 0, 0);
    wgpu::Buffer indexBuffer = CreateIndexBuffer({0, 1, 2, 0, 3, 1});
    // Create a drawCount buffer with various values.
    wgpu::Buffer drawCountBuffer = CreateIndirectBuffer({0, 1, 2});
    // Test a draw with drawCount = 0, which should not draw anything.
    TestDraw(EncodeDrawCommands({3, 1, 0, 0, 0, 3, 1, 3, 0, 0}, indexBuffer, 0, 0, 2,
                                drawCountBuffer, 0),
             notFilled, notFilled);
    // Test a draw with drawCount = 1 where drawCount < maxDrawCount.
    TestDraw(EncodeDrawCommands({3, 1, 0, 0, 0, 3, 1, 3, 0, 0}, indexBuffer, 0, 0, 2,
                                drawCountBuffer, 4),
             filled, notFilled);
    // Test a draw with drawCount = 2 where drawCount = maxDrawCount.
    TestDraw(EncodeDrawCommands({3, 1, 0, 0, 0, 3, 1, 3, 0, 0}, indexBuffer, 0, 0, 2,
                                drawCountBuffer, 8),
             filled, filled);
    // Test a draw with drawCount = 2 where drawCount > maxDrawCount.
    TestDraw(EncodeDrawCommands({3, 1, 0, 0, 0, 3, 1, 3, 0, 0}, indexBuffer, 0, 0, 1,
                                drawCountBuffer, 8),
             filled, notFilled);
    // Test a draw without drawCount buffer. Should draw maxDrawCount times.
    TestDraw(EncodeDrawCommands({3, 1, 0, 0, 0, 3, 1, 3, 0, 0}, indexBuffer, 0, 0, 2), filled,
             filled);
}

TEST_P(MultiDrawIndexedIndirectTest, ValidateWithOffsets) {
    // It doesn't make sense to test invalid inputs when validation is disabled.
    DAWN_TEST_UNSUPPORTED_IF(HasToggleEnabled("skip_validation"));

    utils::RGBA8 filled(0, 255, 0, 255);
    utils::RGBA8 notFilled(0, 0, 0, 0);

    wgpu::Buffer indexBuffer = CreateIndexBuffer({0, 1, 2, 0, 3, 1, 0, 1, 2});

    // Test that validation properly accounts for index buffer offset.
    TestDraw(EncodeDrawCommands({3, 1, 0, 0, 0}, indexBuffer, 6 * sizeof(uint32_t), 0, 1), filled,
             notFilled);
    TestDraw(EncodeDrawCommands({4, 1, 0, 0, 0}, indexBuffer, 6 * sizeof(uint32_t), 0, 1),
             notFilled, notFilled);
    TestDraw(EncodeDrawCommands({3, 1, 4, 0, 0}, indexBuffer, 3 * sizeof(uint32_t), 0, 1),
             notFilled, notFilled);

    // Test that validation properly accounts for indirect buffer offset.
    TestDraw(EncodeDrawCommands({3, 1, 0, 0, 0, 1000, 1, 0, 0, 0}, indexBuffer, 0,
                                4 * sizeof(uint32_t), 1),
             notFilled, notFilled);
    TestDraw(EncodeDrawCommands({3, 1, 0, 0, 0, 1000, 1, 0, 0, 0}, indexBuffer, 0, 0, 1), filled,
             notFilled);
}

TEST_P(MultiDrawIndexedIndirectTest, ValidateMultiplePasses) {
    // It doesn't make sense to test invalid inputs when validation is disabled.
    DAWN_TEST_UNSUPPORTED_IF(HasToggleEnabled("skip_validation"));

    utils::RGBA8 filled(0, 255, 0, 255);
    utils::RGBA8 notFilled(0, 0, 0, 0);

    wgpu::Buffer indexBuffer = CreateIndexBuffer({0, 1, 2, 0, 3, 1, 0, 1, 2});

    // Test validation with multiple passes in a row. Namely this is exercising that scratch buffer
    // data for use with a previous pass's validation commands is not overwritten before it can be
    // used.
    TestDraw(EncodeDrawCommands({10, 1, 0, 0, 0, 3, 1, 9, 0, 0}, indexBuffer, 0, 0, 2), notFilled,
             notFilled);
    TestDraw(EncodeDrawCommands({3, 1, 6, 0, 0, 3, 1, 0, 0, 0}, indexBuffer, 0, 0, 2), filled,
             notFilled);
    TestDraw(EncodeDrawCommands({4, 1, 6, 0, 0, 3, 1, 3, 0, 0}, indexBuffer, 0, 0, 2), notFilled,
             filled);
    TestDraw(EncodeDrawCommands({6, 1, 6, 0, 0, 6, 1, 6, 0, 0}, indexBuffer, 0, 0, 2), notFilled,
             notFilled);
    TestDraw(EncodeDrawCommands({3, 1, 3, 0, 0}, indexBuffer, 0, 0, 1), notFilled, filled);
    TestDraw(EncodeDrawCommands({6, 1, 0, 0, 0}, indexBuffer, 0, 0, 1), filled, filled);
}

TEST_P(MultiDrawIndexedIndirectTest, ValidateEncodeMultipleThenSubmitInOrder) {
    // It doesn't make sense to test invalid inputs when validation is disabled.
    DAWN_TEST_UNSUPPORTED_IF(HasToggleEnabled("skip_validation"));

    utils::RGBA8 filled(0, 255, 0, 255);
    utils::RGBA8 notFilled(0, 0, 0, 0);

    wgpu::Buffer indexBuffer = CreateIndexBuffer({0, 1, 2, 0, 3, 1, 0, 1, 2});

    wgpu::CommandBuffer commands[6];
    commands[0] = EncodeDrawCommands({10, 1, 0, 0, 0, 3, 1, 9, 0, 0}, indexBuffer, 0, 0, 2);
    commands[1] = EncodeDrawCommands({3, 1, 6, 0, 0, 3, 1, 0, 0, 0}, indexBuffer, 0, 0, 2);
    commands[2] = EncodeDrawCommands({4, 1, 6, 0, 0, 3, 1, 3, 0, 0}, indexBuffer, 0, 0, 2);
    commands[3] = EncodeDrawCommands({6, 1, 6, 0, 0, 6, 1, 6, 0, 0}, indexBuffer, 0, 0, 2);
    commands[4] = EncodeDrawCommands({3, 1, 3, 0, 0}, indexBuffer, 0, 0, 1);
    commands[5] = EncodeDrawCommands({6, 1, 0, 0, 0}, indexBuffer, 0, 0, 1);

    TestDraw(commands[0], notFilled, notFilled);
    TestDraw(commands[1], filled, notFilled);
    TestDraw(commands[2], notFilled, filled);
    TestDraw(commands[3], notFilled, notFilled);
    TestDraw(commands[4], notFilled, filled);
    TestDraw(commands[5], filled, filled);
}

TEST_P(MultiDrawIndexedIndirectTest, ValidateEncodeMultipleThenSubmitOutOfOrder) {
    // It doesn't make sense to test invalid inputs when validation is disabled.
    DAWN_TEST_UNSUPPORTED_IF(HasToggleEnabled("skip_validation"));

    utils::RGBA8 filled(0, 255, 0, 255);
    utils::RGBA8 notFilled(0, 0, 0, 0);

    wgpu::Buffer indexBuffer = CreateIndexBuffer({0, 1, 2, 0, 3, 1, 0, 1, 2});

    wgpu::CommandBuffer commands[6];
    commands[0] = EncodeDrawCommands({10, 1, 0, 0, 0, 3, 1, 9, 0, 0}, indexBuffer, 0, 0, 2);
    commands[1] = EncodeDrawCommands({3, 1, 6, 0, 0, 3, 1, 0, 0, 0}, indexBuffer, 0, 0, 2);
    commands[2] = EncodeDrawCommands({4, 1, 6, 0, 0, 3, 1, 3, 0, 0}, indexBuffer, 0, 0, 2);
    commands[3] = EncodeDrawCommands({6, 1, 6, 0, 0, 6, 1, 6, 0, 0}, indexBuffer, 0, 0, 2);
    commands[4] = EncodeDrawCommands({3, 1, 3, 0, 0}, indexBuffer, 0, 0, 1);
    commands[5] = EncodeDrawCommands({6, 1, 0, 0, 0}, indexBuffer, 0, 0, 1);

    TestDraw(commands[0], notFilled, notFilled);
    TestDraw(commands[4], notFilled, filled);
    TestDraw(commands[2], notFilled, filled);
    TestDraw(commands[5], filled, filled);
    TestDraw(commands[1], filled, notFilled);
    TestDraw(commands[3], notFilled, notFilled);
}

TEST_P(MultiDrawIndexedIndirectTest, ValidateEncodeMultipleThenSubmitAtOnce) {
    // It doesn't make sense to test invalid inputs when validation is disabled.
    DAWN_TEST_UNSUPPORTED_IF(HasToggleEnabled("skip_validation"));

    utils::RGBA8 filled(0, 255, 0, 255);
    utils::RGBA8 notFilled(0, 0, 0, 0);

    wgpu::Buffer indexBuffer = CreateIndexBuffer({0, 1, 2, 0, 3, 1, 0, 1, 2});

    wgpu::CommandBuffer commands[5];
    commands[0] = EncodeDrawCommands({10, 1, 0, 0, 0, 3, 1, 9, 0, 0}, indexBuffer, 0, 0, 2);
    commands[1] = EncodeDrawCommands({3, 1, 6, 0, 0, 3, 1, 0, 0, 0}, indexBuffer, 0, 0, 2);
    commands[2] = EncodeDrawCommands({4, 1, 6, 0, 0, 3, 1, 3, 0, 0}, indexBuffer, 0, 0, 2);
    commands[3] = EncodeDrawCommands({6, 1, 6, 0, 0, 6, 1, 6, 0, 0}, indexBuffer, 0, 0, 2);
    commands[4] = EncodeDrawCommands({3, 1, 3, 0, 0}, indexBuffer, 0, 0, 1);

    queue.Submit(5, commands);
    EXPECT_PIXEL_RGBA8_EQ(notFilled, renderPass.color, 1, 3);
    EXPECT_PIXEL_RGBA8_EQ(filled, renderPass.color, 3, 1);
}

TEST_P(MultiDrawIndexedIndirectTest, ValidateMultiDrawMixed) {
    // It doesn't make sense to test invalid inputs when validation is disabled.
    DAWN_TEST_UNSUPPORTED_IF(HasToggleEnabled("skip_validation"));

    utils::RGBA8 filled(0, 255, 0, 255);
    utils::RGBA8 notFilled(0, 0, 0, 0);

    // Test that the multi draw does not affect the single draw and vice versa.

    wgpu::Buffer indexBuffer = CreateIndexBuffer({0, 1, 2, 0, 3, 1});

    wgpu::Buffer indirectBuffer =
        CreateIndirectBuffer({3, 1, 6, 0, 0, 0, 0, 0, 0, 0, 3, 1, 3, 0, 0});

    wgpu::Buffer drawCountBuffer = CreateIndirectBuffer({0, 1, 2});

    wgpu::CommandEncoder encoder = device.CreateCommandEncoder();
    wgpu::RenderPassEncoder pass = encoder.BeginRenderPass(&renderPass.renderPassInfo);
    pass.SetPipeline(pipeline);
    pass.SetVertexBuffer(0, vertexBuffer);
    pass.SetIndexBuffer(indexBuffer, wgpu::IndexFormat::Uint32, 0);
    pass.MultiDrawIndirect(indirectBuffer, 0, 2, nullptr);
    pass.MultiDrawIndexedIndirect(indirectBuffer, 20, 2, drawCountBuffer, 8);
    pass.End();
    wgpu::CommandBuffer commands = encoder.Finish();
    queue.Submit(1, &commands);

    EXPECT_PIXEL_RGBA8_EQ(notFilled, renderPass.color, 1, 3);
    EXPECT_PIXEL_RGBA8_EQ(filled, renderPass.color, 3, 1);
}

TEST_P(MultiDrawIndexedIndirectTest, ValidateMultiAndSingleDrawsInSingleRenderPass) {
    // It doesn't make sense to test invalid inputs when validation is disabled.
    DAWN_TEST_UNSUPPORTED_IF(HasToggleEnabled("skip_validation"));

    utils::RGBA8 filled(0, 255, 0, 255);

    // Test that the multi draw does not affect the single draw and vice versa.

    wgpu::Buffer indexBuffer = CreateIndexBuffer({0, 1, 2, 0, 3, 1});

    wgpu::Buffer indirectBuffer =
        CreateIndirectBuffer({3, 1, 0, 0, 0, 0, 0, 0, 0, 0, 3, 1, 3, 0, 0});

    wgpu::Buffer drawCountBuffer = CreateIndirectBuffer({0, 1, 2});

    wgpu::CommandEncoder encoder = device.CreateCommandEncoder();
    wgpu::RenderPassEncoder pass = encoder.BeginRenderPass(&renderPass.renderPassInfo);
    pass.SetPipeline(pipeline);
    pass.SetVertexBuffer(0, vertexBuffer);
    pass.SetIndexBuffer(indexBuffer, wgpu::IndexFormat::Uint32, 0);
    pass.DrawIndexedIndirect(indirectBuffer, 0);  // draw the first triangle
    pass.DrawIndirect(indirectBuffer, 16);        // no draw
    pass.MultiDrawIndexedIndirect(indirectBuffer, 20, 2, drawCountBuffer, 8);  // draw the second
    pass.MultiDrawIndirect(indirectBuffer, 28, 2, nullptr);                    // no draw
    pass.End();
    wgpu::CommandBuffer commands = encoder.Finish();
    queue.Submit(1, &commands);

    EXPECT_PIXEL_RGBA8_EQ(filled, renderPass.color, 1, 3);
    EXPECT_PIXEL_RGBA8_EQ(filled, renderPass.color, 3, 1);
}

DAWN_INSTANTIATE_TEST(MultiDrawIndexedIndirectTest,
                      VulkanBackend(),
                      D3D12Backend(),
                      MetalBackend());

class MultiDrawIndexedIndirectUsingFirstVertexTest : public DawnTest {
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
        vertexBuffer = dawn::utils::CreateBufferFromData<float>(
            device, wgpu::BufferUsage::Vertex,
            {// The bottom left triangle
             -1.0f - calibration, 1.0f, 0.0f, 1.0f, 1.0f - calibration, -1.0f, 0.0f, 1.0f,
             -1.0f - calibration, -1.0f, 0.0f, 1.0f,
             // The top right triangle
             -1.0f - calibration, 1.0f, 0.0f, 1.0f, 1.0f - calibration, -1.0f, 0.0f, 1.0f,
             1.0f - calibration, 1.0f, 0.0f, 1.0f});

        indexBuffer = dawn::utils::CreateBufferFromData<uint32_t>(device, wgpu::BufferUsage::Index,
                                                                  {0, 1, 2});

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
    wgpu::Buffer indexBuffer;
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
            pass.SetIndexBuffer(indexBuffer, wgpu::IndexFormat::Uint32, 0);
            pass.MultiDrawIndexedIndirect(indirectBuffer, 0, maxDrawCount, nullptr, 0);
            pass.End();
        }
        wgpu::CommandBuffer commands = encoder.Finish();
        queue.Submit(1, &commands);
        EXPECT_PIXEL_RGBA8_EQ(bottomLeftExpected, renderPass.color, 1, 3);
        EXPECT_PIXEL_RGBA8_EQ(topRightExpected, renderPass.color, 3, 1);
    }
};

TEST_P(MultiDrawIndexedIndirectUsingFirstVertexTest, IndirectOffset) {
    utils::RGBA8 filled(0, 255, 0, 255);
    utils::RGBA8 notFilled(0, 0, 0, 0);

    // Test an offset draw call, with indirect buffer containing 2 calls:
    // 1) only the first 3 indices (bottom left triangle)
    // 2) only the last 3 indices (top right triangle)
    // #2 draw has the correct offset applied by vertex index
    Test({3, 1, 0, 0, 0, 3, 1, 0, 3, 0}, 2, notFilled, filled);
}

DAWN_INSTANTIATE_TEST(MultiDrawIndexedIndirectUsingFirstVertexTest,
                      VulkanBackend(),
                      D3D12Backend(),
                      MetalBackend());

class MultiDrawIndexedIndirectUsingInstanceIndexTest
    : public MultiDrawIndexedIndirectUsingFirstVertexTest {
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

TEST_P(MultiDrawIndexedIndirectUsingInstanceIndexTest, IndirectOffset) {
    utils::RGBA8 filled(0, 255, 0, 255);
    utils::RGBA8 notFilled(0, 0, 0, 0);

    // Test an offset draw call, with indirect buffer containing 2 calls:
    // 1) only the first 3 indices (bottom left triangle)
    // 2) only the last 3 indices (top right triangle)

    // Test 1: #1 draw has the correct calibration referenced by instance index
    Test({3, 1, 0, 0, 1, 3, 1, 0, 3, 0}, 2, filled, notFilled);

    // Test 2: #2 draw has the correct offset applied by instance index
    Test({3, 1, 0, 0, 0, 3, 1, 0, 3, 1}, 2, notFilled, filled);
}

DAWN_INSTANTIATE_TEST(MultiDrawIndexedIndirectUsingInstanceIndexTest,
                      VulkanBackend(),
                      D3D12Backend(),
                      MetalBackend());

}  // anonymous namespace
}  // namespace dawn
