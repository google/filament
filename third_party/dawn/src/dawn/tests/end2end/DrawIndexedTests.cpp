// Copyright 2018 The Dawn & Tint Authors
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

#include "dawn/tests/DawnTest.h"

#include "dawn/utils/ComboRenderPipelineDescriptor.h"
#include "dawn/utils/WGPUHelpers.h"

namespace dawn {
namespace {

constexpr uint32_t kRTSize = 4;

class DrawIndexedTest : public DawnTest {
  protected:
    void SetUp() override {
        DawnTest::SetUp();

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
        indexBuffer = utils::CreateBufferFromData<uint32_t>(
            device, wgpu::BufferUsage::Index,
            {0, 1, 2, 0, 3, 1,
             // The indices below are added to test negatve baseVertex
             0 + 4, 1 + 4, 2 + 4, 0 + 4, 3 + 4, 1 + 4});
        zeroSizedIndexBuffer =
            utils::CreateBufferFromData<uint32_t>(device, wgpu::BufferUsage::Index, {});
    }

    utils::BasicRenderPass renderPass;
    wgpu::RenderPipeline pipeline;
    wgpu::Buffer vertexBuffer;
    wgpu::Buffer indexBuffer;
    wgpu::Buffer zeroSizedIndexBuffer;

    void Test(uint32_t indexCount,
              uint32_t instanceCount,
              uint32_t firstIndex,
              int32_t baseVertex,
              uint32_t firstInstance,
              uint64_t bufferOffset,
              utils::RGBA8 bottomLeftExpected,
              utils::RGBA8 topRightExpected) {
        // Regular draw with a reasonable index buffer
        TestImplementation(indexCount, instanceCount, firstIndex, baseVertex, firstInstance,
                           bufferOffset, indexBuffer, bottomLeftExpected, topRightExpected);
    }

    void TestZeroSizedIndexBufferDraw(uint32_t indexCount,
                                      uint32_t firstIndex,
                                      utils::RGBA8 bottomLeftExpected,
                                      utils::RGBA8 topRightExpected) {
        TestImplementation(indexCount, 1, firstIndex, 0, 0, 0, zeroSizedIndexBuffer,
                           bottomLeftExpected, topRightExpected);
    }

    void TestImplementation(uint32_t indexCount,
                            uint32_t instanceCount,
                            uint32_t firstIndex,
                            int32_t baseVertex,
                            uint32_t firstInstance,
                            uint64_t bufferOffset,
                            const wgpu::Buffer& curIndexBuffer,
                            utils::RGBA8 bottomLeftExpected,
                            utils::RGBA8 topRightExpected) {
        wgpu::CommandEncoder encoder = device.CreateCommandEncoder();
        {
            wgpu::RenderPassEncoder pass = encoder.BeginRenderPass(&renderPass.renderPassInfo);
            pass.SetPipeline(pipeline);
            pass.SetVertexBuffer(0, vertexBuffer);
            pass.SetIndexBuffer(curIndexBuffer, wgpu::IndexFormat::Uint32, bufferOffset);
            pass.DrawIndexed(indexCount, instanceCount, firstIndex, baseVertex, firstInstance);
            pass.End();
        }

        wgpu::CommandBuffer commands = encoder.Finish();
        queue.Submit(1, &commands);

        EXPECT_PIXEL_RGBA8_EQ(bottomLeftExpected, renderPass.color, 1, 3);
        EXPECT_PIXEL_RGBA8_EQ(topRightExpected, renderPass.color, 3, 1);
    }
};

// The most basic DrawIndexed triangle draw.
TEST_P(DrawIndexedTest, Uint32) {
    utils::RGBA8 filled(0, 255, 0, 255);
    utils::RGBA8 notFilled(0, 0, 0, 0);

    // Test a draw with no indices.
    Test(0, 0, 0, 0, 0, 0, notFilled, notFilled);
    // Test a draw with only the first 3 indices of the first quad (bottom left triangle)
    Test(3, 1, 0, 0, 0, 0, filled, notFilled);
    // Test a draw with only the last 3 indices of the first quad (top right triangle)
    Test(3, 1, 3, 0, 0, 0, notFilled, filled);
    // Test a draw with all 6 indices (both triangles).
    Test(6, 1, 0, 0, 0, 0, filled, filled);
}

// Test the parameter 'baseVertex' of DrawIndexed() works.
TEST_P(DrawIndexedTest, BaseVertex) {
    DAWN_TEST_UNSUPPORTED_IF(HasToggleEnabled("disable_base_vertex"));
    utils::RGBA8 filled(0, 255, 0, 255);
    utils::RGBA8 notFilled(0, 0, 0, 0);

    // Test a draw with only the first 3 indices of the second quad (top right triangle)
    Test(3, 1, 0, 4, 0, 0, notFilled, filled);
    // Test a draw with only the last 3 indices of the second quad (bottom left triangle)
    Test(3, 1, 3, 4, 0, 0, filled, notFilled);
}

// Test the parameter 'baseVertex' of DrawIndexed() works.
TEST_P(DrawIndexedTest, NegativeBaseVertex) {
    DAWN_TEST_UNSUPPORTED_IF(HasToggleEnabled("disable_base_vertex"));

    // TODO(crbug.com/343178421): ANGLE/SwiftShader and ANGLE/D3D11 fail with negative baseVertex.
    DAWN_SUPPRESS_TEST_IF(IsANGLESwiftShader());
    DAWN_SUPPRESS_TEST_IF(IsANGLED3D11());

    // Also failing on Qualcomm and ARM GLES.
    DAWN_SUPPRESS_TEST_IF(IsOpenGLES() && IsAndroid() && (IsQualcomm() || IsARM()));

    utils::RGBA8 filled(0, 255, 0, 255);
    utils::RGBA8 notFilled(0, 0, 0, 0);

    // Test negative baseVertex
    // Test a draw with only the first 3 indices of the first quad (bottom left triangle)
    Test(3, 1, 0, -4, 0, 6 * sizeof(uint32_t), filled, notFilled);
    // Test a draw with only the last 3 indices of the first quad (top right triangle)
    Test(3, 1, 3, -4, 0, 6 * sizeof(uint32_t), notFilled, filled);
}

DAWN_INSTANTIATE_TEST(DrawIndexedTest,
                      D3D11Backend(),
                      D3D12Backend(),
                      MetalBackend(),
                      OpenGLBackend(),
                      OpenGLESBackend(),
                      VulkanBackend());

}  // anonymous namespace
}  // namespace dawn
