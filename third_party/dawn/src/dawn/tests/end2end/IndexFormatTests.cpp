// Copyright 2017 The Dawn & Tint Authors
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

#include "dawn/common/Assert.h"
#include "dawn/utils/ComboRenderPipelineDescriptor.h"
#include "dawn/utils/WGPUHelpers.h"

namespace dawn {
namespace {

constexpr uint32_t kRTSize = 400;

class IndexFormatTest : public DawnTest {
  protected:
    void SetUp() override {
        DawnTest::SetUp();

        renderPass = utils::CreateBasicRenderPass(device, kRTSize, kRTSize);
    }

    utils::BasicRenderPass renderPass;

    wgpu::RenderPipeline MakeTestPipeline(
        wgpu::IndexFormat format,
        wgpu::PrimitiveTopology primitiveTopology = wgpu::PrimitiveTopology::TriangleStrip) {
        wgpu::ShaderModule vsModule = utils::CreateShaderModule(device, R"(
            struct VertexIn {
                @location(0) pos : vec4f,
                @builtin(vertex_index) idx : u32,
            }

            @vertex fn main(input : VertexIn) -> @builtin(position) vec4f {
                // 0xFFFFFFFE is a designated invalid index used by some tests.
                if (input.idx == 0xFFFFFFFEu) {
                    return vec4f(0.0, 0.0, 0.0, 1.0);
                }
                return input.pos;
            })");

        wgpu::ShaderModule fsModule = utils::CreateShaderModule(device, R"(
            @fragment fn main() -> @location(0) vec4f {
                return vec4f(0.0, 1.0, 0.0, 1.0);
            })");

        utils::ComboRenderPipelineDescriptor descriptor;
        descriptor.vertex.module = vsModule;
        descriptor.cFragment.module = fsModule;
        descriptor.primitive.topology = primitiveTopology;
        descriptor.primitive.stripIndexFormat = format;
        descriptor.vertex.bufferCount = 1;
        descriptor.cBuffers[0].arrayStride = 4 * sizeof(float);
        descriptor.cBuffers[0].attributeCount = 1;
        descriptor.cAttributes[0].format = wgpu::VertexFormat::Float32x4;
        descriptor.cTargets[0].format = renderPass.colorFormat;

        return device.CreateRenderPipeline(&descriptor);
    }
};

// Test that the Uint32 index format is correctly interpreted
TEST_P(IndexFormatTest, Uint32) {
    wgpu::RenderPipeline pipeline = MakeTestPipeline(wgpu::IndexFormat::Uint32);

    wgpu::Buffer vertexBuffer = utils::CreateBufferFromData<float>(
        device, wgpu::BufferUsage::Vertex,
        {-1.0f, -1.0f, 0.0f, 1.0f,  // Note Vertices[0] = Vertices[1]
         -1.0f, -1.0f, 0.0f, 1.0f, 1.0f, -1.0f, 0.0f, 1.0f, -1.0f, 1.0f, 0.0f, 1.0f});
    // If this is interpreted as Uint16, then it would be 0, 1, 0, ... and would draw nothing.
    wgpu::Buffer indexBuffer =
        utils::CreateBufferFromData<uint32_t>(device, wgpu::BufferUsage::Index, {1, 2, 3});

    wgpu::CommandEncoder encoder = device.CreateCommandEncoder();
    {
        wgpu::RenderPassEncoder pass = encoder.BeginRenderPass(&renderPass.renderPassInfo);
        pass.SetPipeline(pipeline);
        pass.SetVertexBuffer(0, vertexBuffer);
        pass.SetIndexBuffer(indexBuffer, wgpu::IndexFormat::Uint32);
        pass.DrawIndexed(3);
        pass.End();
    }

    wgpu::CommandBuffer commands = encoder.Finish();
    queue.Submit(1, &commands);

    EXPECT_PIXEL_RGBA8_EQ(utils::RGBA8::kGreen, renderPass.color, 100, 300);
}

// Test that the Uint16 index format is correctly interpreted
TEST_P(IndexFormatTest, Uint16) {
    wgpu::RenderPipeline pipeline = MakeTestPipeline(wgpu::IndexFormat::Uint16);

    wgpu::Buffer vertexBuffer = utils::CreateBufferFromData<float>(
        device, wgpu::BufferUsage::Vertex,
        {-1.0f, -1.0f, 0.0f, 1.0f, 1.0f, -1.0f, 0.0f, 1.0f, -1.0f, 1.0f, 0.0f, 1.0f});
    // If this is interpreted as uint32, it will have index 1 and 2 be both 0 and render nothing
    wgpu::Buffer indexBuffer =
        utils::CreateBufferFromData<uint16_t>(device, wgpu::BufferUsage::Index, {1, 2, 0, 0, 0, 0});

    wgpu::CommandEncoder encoder = device.CreateCommandEncoder();
    {
        wgpu::RenderPassEncoder pass = encoder.BeginRenderPass(&renderPass.renderPassInfo);
        pass.SetPipeline(pipeline);
        pass.SetVertexBuffer(0, vertexBuffer);
        pass.SetIndexBuffer(indexBuffer, wgpu::IndexFormat::Uint16);
        pass.DrawIndexed(3);
        pass.End();
    }

    wgpu::CommandBuffer commands = encoder.Finish();
    queue.Submit(1, &commands);

    EXPECT_PIXEL_RGBA8_EQ(utils::RGBA8::kGreen, renderPass.color, 100, 300);
}

// Test that the index format used is the format of the last set pipeline. This is to
// prevent a case in D3D12 where the index format would be captured from the last
// pipeline on SetIndexBuffer.
TEST_P(IndexFormatTest, ChangePipelineAfterSetIndexBuffer) {
    wgpu::RenderPipeline pipeline32 = MakeTestPipeline(wgpu::IndexFormat::Uint32);
    wgpu::RenderPipeline pipeline16 = MakeTestPipeline(wgpu::IndexFormat::Uint16);

    wgpu::Buffer vertexBuffer = utils::CreateBufferFromData<float>(
        device, wgpu::BufferUsage::Vertex,
        {-1.0f, -1.0f, 0.0f, 1.0f,  // Note Vertices[0] = Vertices[1]
         -1.0f, -1.0f, 0.0f, 1.0f, 1.0f, -1.0f, 0.0f, 1.0f, -1.0f, 1.0f, 0.0f, 1.0f});
    // If this is interpreted as Uint16, then it would be 0, 1, 0, ... and would draw nothing.
    wgpu::Buffer indexBuffer =
        utils::CreateBufferFromData<uint32_t>(device, wgpu::BufferUsage::Index, {1, 2, 3});

    wgpu::CommandEncoder encoder = device.CreateCommandEncoder();
    {
        wgpu::RenderPassEncoder pass = encoder.BeginRenderPass(&renderPass.renderPassInfo);
        pass.SetPipeline(pipeline16);
        pass.SetVertexBuffer(0, vertexBuffer);
        pass.SetIndexBuffer(indexBuffer, wgpu::IndexFormat::Uint32);
        pass.SetPipeline(pipeline32);
        pass.DrawIndexed(3);
        pass.End();
    }

    wgpu::CommandBuffer commands = encoder.Finish();
    queue.Submit(1, &commands);

    EXPECT_PIXEL_RGBA8_EQ(utils::RGBA8::kGreen, renderPass.color, 100, 300);
}

// Test that setting the index buffer before the pipeline works, this is important
// for backends where the index format is passed inside the call to SetIndexBuffer
// because it needs to be done lazily (to query the format from the last pipeline).
TEST_P(IndexFormatTest, SetIndexBufferBeforeSetPipeline) {
    wgpu::RenderPipeline pipeline = MakeTestPipeline(wgpu::IndexFormat::Uint32);

    wgpu::Buffer vertexBuffer = utils::CreateBufferFromData<float>(
        device, wgpu::BufferUsage::Vertex,
        {-1.0f, -1.0f, 0.0f, 1.0f, 1.0f, -1.0f, 0.0f, 1.0f, -1.0f, 1.0f, 0.0f, 1.0f});
    wgpu::Buffer indexBuffer =
        utils::CreateBufferFromData<uint32_t>(device, wgpu::BufferUsage::Index, {0, 1, 2});

    wgpu::CommandEncoder encoder = device.CreateCommandEncoder();
    {
        wgpu::RenderPassEncoder pass = encoder.BeginRenderPass(&renderPass.renderPassInfo);
        pass.SetIndexBuffer(indexBuffer, wgpu::IndexFormat::Uint32);
        pass.SetPipeline(pipeline);
        pass.SetVertexBuffer(0, vertexBuffer);
        pass.DrawIndexed(3);
        pass.End();
    }

    wgpu::CommandBuffer commands = encoder.Finish();
    queue.Submit(1, &commands);

    EXPECT_PIXEL_RGBA8_EQ(utils::RGBA8(0, 255, 0, 255), renderPass.color, 100, 300);
}

// Test that index buffers of multiple formats can be used with a pipeline that
// doesn't use strip primitive topology.
TEST_P(IndexFormatTest, SetIndexBufferDifferentFormats) {
    wgpu::RenderPipeline pipeline =
        MakeTestPipeline(wgpu::IndexFormat::Undefined, wgpu::PrimitiveTopology::TriangleList);

    wgpu::Buffer vertexBuffer = utils::CreateBufferFromData<float>(
        device, wgpu::BufferUsage::Vertex,
        {-1.0f, -1.0f, 0.0f, 1.0f, 1.0f, -1.0f, 0.0f, 1.0f, -1.0f, 1.0f, 0.0f, 1.0f});
    wgpu::Buffer indexBuffer32 =
        utils::CreateBufferFromData<uint32_t>(device, wgpu::BufferUsage::Index, {0, 1, 2});
    wgpu::Buffer indexBuffer16 =
        utils::CreateBufferFromData<uint16_t>(device, wgpu::BufferUsage::Index, {0, 1, 2, 0});

    wgpu::CommandEncoder encoder = device.CreateCommandEncoder();
    {
        wgpu::RenderPassEncoder pass = encoder.BeginRenderPass(&renderPass.renderPassInfo);
        pass.SetIndexBuffer(indexBuffer32, wgpu::IndexFormat::Uint32);
        pass.SetPipeline(pipeline);
        pass.SetVertexBuffer(0, vertexBuffer);
        pass.DrawIndexed(3);
        pass.End();
    }

    wgpu::CommandBuffer commands = encoder.Finish();
    queue.Submit(1, &commands);

    EXPECT_PIXEL_RGBA8_EQ(utils::RGBA8(0, 255, 0, 255), renderPass.color, 100, 300);

    encoder = device.CreateCommandEncoder();
    {
        wgpu::RenderPassEncoder pass = encoder.BeginRenderPass(&renderPass.renderPassInfo);
        pass.SetIndexBuffer(indexBuffer16, wgpu::IndexFormat::Uint16);
        pass.SetPipeline(pipeline);
        pass.SetVertexBuffer(0, vertexBuffer);
        pass.DrawIndexed(3);
        pass.End();
    }

    commands = encoder.Finish();
    queue.Submit(1, &commands);

    EXPECT_PIXEL_RGBA8_EQ(utils::RGBA8(0, 255, 0, 255), renderPass.color, 100, 300);
}

// Tests for primitive restart use vertices like in the drawing and draw the following
// indices: 0 1 2 PRIM_RESTART 3 4 5. Then A and B should be written but not C.
//      |--------------|
//      |      0---1   |
//      |       \ B|   |
//      |         \|   |
//      |  3   C   2   |
//      |  |\          |
//      |  |A \        |
//      |  4---5       |
//      |--------------|

class TriangleStripPrimitiveRestartTests : public IndexFormatTest {
  protected:
    wgpu::Buffer mVertexBuffer;

    void SetUp() override {
        IndexFormatTest::SetUp();
        mVertexBuffer = utils::CreateBufferFromData<float>(device, wgpu::BufferUsage::Vertex,
                                                           {
                                                               0.0f,  1.0f,  0.0f, 1.0f,  // 0
                                                               1.0f,  1.0f,  0.0f, 1.0f,  // 1
                                                               1.0f,  0.0f,  0.0f, 1.0f,  // 2
                                                               -1.0f, 0.0f,  0.0f, 1.0f,  // 3
                                                               -1.0f, -1.0f, 0.0f, 1.0f,  // 4
                                                               0.0f,  -1.0f, 0.0f, 1.0f,  // 5
                                                           });
    }
};

// Test use of primitive restart with an Uint32 index format
TEST_P(TriangleStripPrimitiveRestartTests, Uint32PrimitiveRestart) {
    wgpu::RenderPipeline pipeline = MakeTestPipeline(wgpu::IndexFormat::Uint32);

    wgpu::Buffer indexBuffer =
        utils::CreateBufferFromData<uint32_t>(device, wgpu::BufferUsage::Index,
                                              {
                                                  0,
                                                  1,
                                                  2,
                                                  0xFFFFFFFFu,
                                                  3,
                                                  4,
                                                  5,
                                              });

    wgpu::CommandEncoder encoder = device.CreateCommandEncoder();
    {
        wgpu::RenderPassEncoder pass = encoder.BeginRenderPass(&renderPass.renderPassInfo);
        pass.SetPipeline(pipeline);
        pass.SetVertexBuffer(0, mVertexBuffer);
        pass.SetIndexBuffer(indexBuffer, wgpu::IndexFormat::Uint32);
        pass.DrawIndexed(7);
        pass.End();
    }

    wgpu::CommandBuffer commands = encoder.Finish();
    queue.Submit(1, &commands);

    EXPECT_PIXEL_RGBA8_EQ(utils::RGBA8::kGreen, renderPass.color, 50, 350);  // A
    EXPECT_PIXEL_RGBA8_EQ(utils::RGBA8::kGreen, renderPass.color, 350, 50);  // B
    EXPECT_PIXEL_RGBA8_EQ(utils::RGBA8::kZero, renderPass.color, 198, 200);  // C
}

// Same as the above test, but uses an OOB index to emulate primitive restart being disabled,
// causing point C to be written to.
TEST_P(TriangleStripPrimitiveRestartTests, Uint32WithoutPrimitiveRestart) {
    wgpu::RenderPipeline pipeline = MakeTestPipeline(wgpu::IndexFormat::Uint32);
    wgpu::Buffer indexBuffer =
        utils::CreateBufferFromData<uint32_t>(device, wgpu::BufferUsage::Index,
                                              {
                                                  0,
                                                  1,
                                                  2,
                                                  // Not a valid index.
                                                  0xFFFFFFFEu,
                                                  3,
                                                  4,
                                                  5,
                                              });

    wgpu::CommandEncoder encoder = device.CreateCommandEncoder();
    {
        wgpu::RenderPassEncoder pass = encoder.BeginRenderPass(&renderPass.renderPassInfo);
        pass.SetPipeline(pipeline);
        pass.SetVertexBuffer(0, mVertexBuffer);
        pass.SetIndexBuffer(indexBuffer, wgpu::IndexFormat::Uint32);
        pass.DrawIndexed(7);
        pass.End();
    }

    wgpu::CommandBuffer commands = encoder.Finish();
    queue.Submit(1, &commands);

    EXPECT_PIXEL_RGBA8_EQ(utils::RGBA8::kGreen, renderPass.color, 50, 350);   // A
    EXPECT_PIXEL_RGBA8_EQ(utils::RGBA8::kGreen, renderPass.color, 350, 50);   // B
    EXPECT_PIXEL_RGBA8_EQ(utils::RGBA8::kGreen, renderPass.color, 198, 200);  // C
}

// Test use of primitive restart with an Uint16 index format
TEST_P(TriangleStripPrimitiveRestartTests, Uint16PrimitiveRestart) {
    wgpu::RenderPipeline pipeline = MakeTestPipeline(wgpu::IndexFormat::Uint16);

    wgpu::Buffer indexBuffer =
        utils::CreateBufferFromData<uint16_t>(device, wgpu::BufferUsage::Index,
                                              {
                                                  0,
                                                  1,
                                                  2,
                                                  0xFFFFu,
                                                  3,
                                                  4,
                                                  5,
                                                  // This value is for padding.
                                                  0xFFFFu,
                                              });

    wgpu::CommandEncoder encoder = device.CreateCommandEncoder();
    {
        wgpu::RenderPassEncoder pass = encoder.BeginRenderPass(&renderPass.renderPassInfo);
        pass.SetPipeline(pipeline);
        pass.SetVertexBuffer(0, mVertexBuffer);
        pass.SetIndexBuffer(indexBuffer, wgpu::IndexFormat::Uint16);
        pass.DrawIndexed(7);
        pass.End();
    }

    wgpu::CommandBuffer commands = encoder.Finish();
    queue.Submit(1, &commands);

    EXPECT_PIXEL_RGBA8_EQ(utils::RGBA8::kGreen, renderPass.color, 50, 350);  // A
    EXPECT_PIXEL_RGBA8_EQ(utils::RGBA8::kGreen, renderPass.color, 350, 50);  // B
    EXPECT_PIXEL_RGBA8_EQ(utils::RGBA8::kZero, renderPass.color, 198, 200);  // C
}

// Tests for primitive restart use vertices like in the drawing and draw the following
// indices: 0 1 PRIM_RESTART 2 3. Then 1 and 2 should be written but not A.
//      |--------------|
//      |      3      0|
//      |      |      ||
//      |      |      ||
//      |      2  A   1|
//      |              |
//      |              |
//      |              |
//      |--------------|

class LineStripPrimitiveRestartTests : public IndexFormatTest {
  protected:
  protected:
    wgpu::Buffer mVertexBuffer;

    void SetUp() override {
        IndexFormatTest::SetUp();
        mVertexBuffer = utils::CreateBufferFromData<float>(device, wgpu::BufferUsage::Vertex,
                                                           {
                                                               1.0f, 1.0f, 0.0f, 1.0f,  // 0
                                                               1.0f, 0.0f, 0.0f, 1.0f,  // 1
                                                               0.0f, 0.0f, 0.0f, 1.0f,  // 2
                                                               0.0f, 1.0f, 0.0f, 1.0f   // 3
                                                           });
    }
};

TEST_P(LineStripPrimitiveRestartTests, Uint32PrimitiveRestart) {
    wgpu::RenderPipeline pipeline =
        MakeTestPipeline(wgpu::IndexFormat::Uint32, wgpu::PrimitiveTopology::LineStrip);

    wgpu::Buffer indexBuffer = utils::CreateBufferFromData<uint32_t>(
        device, wgpu::BufferUsage::Index, {0, 1, 0xFFFFFFFFu, 2, 3});

    wgpu::CommandEncoder encoder = device.CreateCommandEncoder();
    {
        wgpu::RenderPassEncoder pass = encoder.BeginRenderPass(&renderPass.renderPassInfo);
        pass.SetPipeline(pipeline);
        pass.SetVertexBuffer(0, mVertexBuffer);
        pass.SetIndexBuffer(indexBuffer, wgpu::IndexFormat::Uint32);
        pass.DrawIndexed(5);
        pass.End();
    }

    wgpu::CommandBuffer commands = encoder.Finish();
    queue.Submit(1, &commands);

    EXPECT_PIXEL_RGBA8_EQ(utils::RGBA8::kGreen, renderPass.color, 399, 199);  // 1
    EXPECT_PIXEL_RGBA8_EQ(utils::RGBA8::kGreen, renderPass.color, 199, 199);  // 2
    EXPECT_PIXEL_RGBA8_EQ(utils::RGBA8::kZero, renderPass.color, 300, 199);   // A
}

// Same as the above test, but uses an OOB index to emulate primitive restart being disabled,
// causing point A to be written to.
TEST_P(LineStripPrimitiveRestartTests, Uint32WithoutPrimitiveRestart) {
    // TODO(crbug.com/dawn/2295): diagnose this failure on Pixel 6 OpenGLES
    DAWN_SUPPRESS_TEST_IF(IsOpenGLES() && IsAndroid() && IsARM());

    wgpu::RenderPipeline pipeline =
        MakeTestPipeline(wgpu::IndexFormat::Uint32, wgpu::PrimitiveTopology::LineStrip);

    wgpu::Buffer indexBuffer =
        utils::CreateBufferFromData<uint32_t>(device, wgpu::BufferUsage::Index,
                                              {0, 1,  // Not a valid index
                                               0xFFFFFFFEu, 2, 3});

    wgpu::CommandEncoder encoder = device.CreateCommandEncoder();
    {
        wgpu::RenderPassEncoder pass = encoder.BeginRenderPass(&renderPass.renderPassInfo);
        pass.SetPipeline(pipeline);
        pass.SetVertexBuffer(0, mVertexBuffer);
        pass.SetIndexBuffer(indexBuffer, wgpu::IndexFormat::Uint32);
        pass.DrawIndexed(5);
        pass.End();
    }

    wgpu::CommandBuffer commands = encoder.Finish();
    queue.Submit(1, &commands);

    EXPECT_PIXEL_RGBA8_EQ(utils::RGBA8::kGreen, renderPass.color, 399, 199);  // 1
    EXPECT_PIXEL_RGBA8_EQ(utils::RGBA8::kGreen, renderPass.color, 199, 199);  // 2
    EXPECT_PIXEL_RGBA8_EQ(utils::RGBA8::kGreen, renderPass.color, 300, 199);  // A
}

TEST_P(LineStripPrimitiveRestartTests, Uint16PrimitiveRestart) {
    wgpu::RenderPipeline pipeline =
        MakeTestPipeline(wgpu::IndexFormat::Uint16, wgpu::PrimitiveTopology::LineStrip);

    wgpu::Buffer indexBuffer =
        utils::CreateBufferFromData<uint16_t>(device, wgpu::BufferUsage::Index,
                                              {0, 1, 0xFFFFu, 2, 3,  // This value is for padding.
                                               0xFFFFu});

    wgpu::CommandEncoder encoder = device.CreateCommandEncoder();
    {
        wgpu::RenderPassEncoder pass = encoder.BeginRenderPass(&renderPass.renderPassInfo);
        pass.SetPipeline(pipeline);
        pass.SetVertexBuffer(0, mVertexBuffer);
        pass.SetIndexBuffer(indexBuffer, wgpu::IndexFormat::Uint16);
        pass.DrawIndexed(5);
        pass.End();
    }

    wgpu::CommandBuffer commands = encoder.Finish();
    queue.Submit(1, &commands);

    EXPECT_PIXEL_RGBA8_EQ(utils::RGBA8::kGreen, renderPass.color, 399, 199);  // 1
    EXPECT_PIXEL_RGBA8_EQ(utils::RGBA8::kGreen, renderPass.color, 199, 199);  // 2
    EXPECT_PIXEL_RGBA8_EQ(utils::RGBA8::kZero, renderPass.color, 300, 199);   // A
}

DAWN_INSTANTIATE_TEST(IndexFormatTest,
                      D3D11Backend(),
                      D3D12Backend(),
                      MetalBackend(),
                      OpenGLBackend(),
                      OpenGLESBackend(),
                      VulkanBackend());
DAWN_INSTANTIATE_TEST(TriangleStripPrimitiveRestartTests,
                      D3D11Backend(),
                      D3D12Backend(),
                      MetalBackend(),
                      OpenGLBackend(),
                      OpenGLESBackend(),
                      VulkanBackend());
DAWN_INSTANTIATE_TEST(LineStripPrimitiveRestartTests,
                      D3D11Backend(),
                      D3D12Backend(),
                      MetalBackend(),
                      OpenGLBackend(),
                      OpenGLESBackend(),
                      VulkanBackend());

}  // anonymous namespace
}  // namespace dawn
