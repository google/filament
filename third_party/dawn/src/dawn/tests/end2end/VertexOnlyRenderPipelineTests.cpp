// Copyright 2021 The Dawn & Tint Authors
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

constexpr wgpu::TextureFormat kDepthStencilFormat = wgpu::TextureFormat::Depth24PlusStencil8;
constexpr wgpu::TextureFormat kColorFormat = wgpu::TextureFormat::RGBA8Unorm;
constexpr uint32_t kRTWidth = 4;
constexpr uint32_t kRTHeight = 1;

class VertexOnlyRenderPipelineTest : public DawnTest {
  protected:
    void SetUp() override {
        DawnTest::SetUp();

        vertexBuffer =
            utils::CreateBufferFromData<float>(device, wgpu::BufferUsage::Vertex,
                                               {// The middle back line
                                                -0.5f, 0.0f, 0.0f, 1.0f, 0.5f, 0.0f, 0.0f, 1.0f,

                                                // The right front line
                                                0.0f, 0.0f, 1.0f, 1.0f, 1.0f, 0.0f, 1.0f, 1.0f,

                                                // The whole in-between line
                                                -1.0f, 0.0f, 0.5f, 1.0f, 1.0f, 0.0f, 0.5f, 1.0f});

        // Create a color texture as render target
        {
            wgpu::TextureDescriptor descriptor;
            descriptor.dimension = wgpu::TextureDimension::e2D;
            descriptor.size = {kRTWidth, kRTHeight};
            descriptor.format = kColorFormat;
            descriptor.usage = wgpu::TextureUsage::RenderAttachment | wgpu::TextureUsage::CopySrc;
            renderTargetColor = device.CreateTexture(&descriptor);
        }

        // Create a DepthStencilView for vertex-only pipeline to write and full pipeline to read
        {
            wgpu::TextureDescriptor descriptor;
            descriptor.dimension = wgpu::TextureDimension::e2D;
            descriptor.size = {kRTWidth, kRTHeight};
            descriptor.format = kDepthStencilFormat;
            descriptor.usage = wgpu::TextureUsage::RenderAttachment | wgpu::TextureUsage::CopySrc;
            depthStencilTexture = device.CreateTexture(&descriptor);
            depthStencilView = depthStencilTexture.CreateView();
        }

        // The vertex-only render pass to modify the depth and stencil
        renderPassDescNoColor = utils::ComboRenderPassDescriptor({}, depthStencilView);
        renderPassDescNoColor.cDepthStencilAttachmentInfo.depthLoadOp = wgpu::LoadOp::Load;
        renderPassDescNoColor.cDepthStencilAttachmentInfo.stencilLoadOp = wgpu::LoadOp::Load;

        // The complete render pass to read the depth and stencil and draw to color attachment
        renderPassDescWithColor =
            utils::ComboRenderPassDescriptor({renderTargetColor.CreateView()}, depthStencilView);
        renderPassDescWithColor.cDepthStencilAttachmentInfo.depthLoadOp = wgpu::LoadOp::Load;
        renderPassDescWithColor.cDepthStencilAttachmentInfo.stencilLoadOp = wgpu::LoadOp::Load;

        // Create a vertex-only render pipeline that only modify the depth in DepthStencilView, and
        // ignore the stencil component
        depthPipelineNoFragment =
            CreateRenderPipeline(wgpu::CompareFunction::Always, wgpu::StencilOperation::Keep,
                                 wgpu::CompareFunction::Always, wgpu::OptionalBool::True, false);
        depthPipelineWithFragment =
            CreateRenderPipeline(wgpu::CompareFunction::Always, wgpu::StencilOperation::Keep,
                                 wgpu::CompareFunction::Always, wgpu::OptionalBool::True, true);

        // Create a vertex-only render pipeline that only modify the stencil in DepthStencilView,
        // and ignore the depth component
        stencilPipelineNoFragment =
            CreateRenderPipeline(wgpu::CompareFunction::Always, wgpu::StencilOperation::Replace,
                                 wgpu::CompareFunction::Always, wgpu::OptionalBool::False, false);
        stencilPipelineWithFragment =
            CreateRenderPipeline(wgpu::CompareFunction::Always, wgpu::StencilOperation::Replace,
                                 wgpu::CompareFunction::Always, wgpu::OptionalBool::False, true);

        // Create a complete render pipeline that do both depth and stencil tests, and draw to color
        // attachment
        fullPipeline = CreateRenderPipeline(
            wgpu::CompareFunction::Equal, wgpu::StencilOperation::Keep,
            wgpu::CompareFunction::GreaterEqual, wgpu::OptionalBool::False, true);
    }

    wgpu::RenderPipeline CreateRenderPipeline(
        wgpu::CompareFunction stencilCompare = wgpu::CompareFunction::Always,
        wgpu::StencilOperation stencilPassOp = wgpu::StencilOperation::Keep,
        wgpu::CompareFunction depthCompare = wgpu::CompareFunction::Always,
        wgpu::OptionalBool writeDepth = wgpu::OptionalBool::False,
        bool useFragment = true) {
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
        descriptor.primitive.topology = wgpu::PrimitiveTopology::LineList;

        descriptor.vertex.module = vsModule;
        descriptor.vertex.bufferCount = 1;
        descriptor.cBuffers[0].arrayStride = 4 * sizeof(float);
        descriptor.cBuffers[0].attributeCount = 1;
        descriptor.cAttributes[0].format = wgpu::VertexFormat::Float32x4;

        descriptor.cFragment.module = fsModule;
        descriptor.cTargets[0].format = kColorFormat;

        wgpu::DepthStencilState* depthStencil = descriptor.EnableDepthStencil(kDepthStencilFormat);

        depthStencil->stencilFront.compare = stencilCompare;
        depthStencil->stencilBack.compare = stencilCompare;
        depthStencil->stencilFront.passOp = stencilPassOp;
        depthStencil->stencilBack.passOp = stencilPassOp;
        depthStencil->depthWriteEnabled = writeDepth;
        depthStencil->depthCompare = depthCompare;

        if (!useFragment) {
            descriptor.fragment = nullptr;
        }

        return device.CreateRenderPipeline(&descriptor);
    }

    void ClearAttachment(const wgpu::CommandEncoder& encoder) {
        utils::ComboRenderPassDescriptor clearPass =
            utils::ComboRenderPassDescriptor({renderTargetColor.CreateView()}, depthStencilView);
        clearPass.cDepthStencilAttachmentInfo.depthLoadOp = wgpu::LoadOp::Clear;
        clearPass.cDepthStencilAttachmentInfo.depthClearValue = 0.0f;
        clearPass.cDepthStencilAttachmentInfo.stencilLoadOp = wgpu::LoadOp::Clear;
        clearPass.cDepthStencilAttachmentInfo.stencilClearValue = 0x0;
        for (auto& t : clearPass.cColorAttachments) {
            t.loadOp = wgpu::LoadOp::Clear;
            t.clearValue = {0.0, 0.0, 0.0, 0.0};
        }

        auto pass = encoder.BeginRenderPass(&clearPass);
        pass.End();
    }

    // Render resource
    wgpu::Buffer vertexBuffer;
    // Render target
    wgpu::Texture depthStencilTexture;
    wgpu::TextureView depthStencilView;
    wgpu::Texture renderTargetColor;
    // Render result
    const utils::RGBA8 filled = utils::RGBA8(0, 255, 0, 255);
    const utils::RGBA8 notFilled = utils::RGBA8(0, 0, 0, 0);
    // Render pass
    utils::ComboRenderPassDescriptor renderPassDescNoColor{};
    utils::ComboRenderPassDescriptor renderPassDescWithColor{};
    // Render pipeline
    wgpu::RenderPipeline stencilPipelineNoFragment;
    wgpu::RenderPipeline stencilPipelineWithFragment;
    wgpu::RenderPipeline depthPipelineNoFragment;
    wgpu::RenderPipeline depthPipelineWithFragment;
    wgpu::RenderPipeline fullPipeline;
};

// Test that a vertex-only render pipeline modify the stencil attachment as same as a complete
// render pipeline do.
TEST_P(VertexOnlyRenderPipelineTest, Stencil) {
    auto doStencilTest = [&](const wgpu::RenderPassDescriptor* renderPass,
                             const wgpu::RenderPipeline& pipeline,
                             const utils::RGBA8& colorExpect) -> void {
        wgpu::CommandEncoder encoder = device.CreateCommandEncoder();

        ClearAttachment(encoder);

        {
            wgpu::RenderPassEncoder pass = encoder.BeginRenderPass(renderPass);
            pass.SetPipeline(pipeline);
            // Set the stencil reference to a arbitrary value
            pass.SetStencilReference(0x42);
            pass.SetVertexBuffer(0, vertexBuffer);
            // Draw the whole line
            pass.Draw(2, 1, 4, 0);
            pass.End();
        }

        wgpu::CommandBuffer commands = encoder.Finish();
        queue.Submit(1, &commands);

        EXPECT_PIXEL_RGBA8_EQ(colorExpect, renderTargetColor, 0, 0);
        EXPECT_PIXEL_RGBA8_EQ(colorExpect, renderTargetColor, 1, 0);
        EXPECT_PIXEL_RGBA8_EQ(colorExpect, renderTargetColor, 2, 0);
        EXPECT_PIXEL_RGBA8_EQ(colorExpect, renderTargetColor, 3, 0);

        // Test that the stencil is set to the chosen value
        ExpectAttachmentStencilTestData(depthStencilTexture, kDepthStencilFormat, 4, 1, 0, 0, 0x42);
    };

    doStencilTest(&renderPassDescWithColor, stencilPipelineWithFragment, filled);
    doStencilTest(&renderPassDescNoColor, stencilPipelineNoFragment, notFilled);
}

// Test that a vertex-only render pipeline modify the depth attachment as same as a complete render
// pipeline do.
TEST_P(VertexOnlyRenderPipelineTest, Depth) {
    // TODO(crbug.com/dawn/2295): diagnose this failure on Pixel 4 OpenGLES
    DAWN_SUPPRESS_TEST_IF(IsOpenGLES() && IsAndroid() && IsQualcomm());

    auto doStencilTest = [&](const wgpu::RenderPassDescriptor* renderPass,
                             const wgpu::RenderPipeline& pipeline,
                             const utils::RGBA8& colorExpect) -> void {
        wgpu::CommandEncoder encoder = device.CreateCommandEncoder();

        ClearAttachment(encoder);

        {
            wgpu::RenderPassEncoder pass = encoder.BeginRenderPass(renderPass);
            pass.SetPipeline(pipeline);
            pass.SetStencilReference(0x0);
            pass.SetVertexBuffer(0, vertexBuffer);
            // Draw the whole line
            pass.Draw(2, 1, 4, 0);
            pass.End();
        }

        wgpu::CommandBuffer commands = encoder.Finish();
        queue.Submit(1, &commands);

        EXPECT_PIXEL_RGBA8_EQ(colorExpect, renderTargetColor, 0, 0);
        EXPECT_PIXEL_RGBA8_EQ(colorExpect, renderTargetColor, 1, 0);
        EXPECT_PIXEL_RGBA8_EQ(colorExpect, renderTargetColor, 2, 0);
        EXPECT_PIXEL_RGBA8_EQ(colorExpect, renderTargetColor, 3, 0);

        // Test that the stencil is set to the chosen value
        uint8_t expectedStencil = 0;
        ExpectAttachmentDepthStencilTestData(depthStencilTexture, kDepthStencilFormat, 4, 1, 0, 0,
                                             {0.5, 0.5, 0.5, 0.5}, &expectedStencil);
    };

    doStencilTest(&renderPassDescWithColor, depthPipelineWithFragment, filled);
    doStencilTest(&renderPassDescNoColor, depthPipelineNoFragment, notFilled);
}

// Test that vertex-only render pipelines and complete render pipelines cooperate correctly in a
// single encoder, each in a render pass
// In this test we first draw with a vertex-only pipeline to set up stencil in a region, than draw
// with another vertex-only pipeline to modify depth in another region, and finally draw with a
// complete pipeline with depth and stencil tests enabled. We check the color result of the final
// draw, and make sure that it correctly use the stencil and depth result set in previous
// vertex-only pipelines.
TEST_P(VertexOnlyRenderPipelineTest, MultiplePass) {
    wgpu::CommandEncoder encoder = device.CreateCommandEncoder();

    ClearAttachment(encoder);

    // Use the stencil pipeline to set the stencil on the middle
    {
        wgpu::RenderPassEncoder pass = encoder.BeginRenderPass(&renderPassDescNoColor);
        pass.SetStencilReference(0x1);
        pass.SetPipeline(stencilPipelineNoFragment);
        pass.SetVertexBuffer(0, vertexBuffer);
        // Draw the middle line
        pass.Draw(2, 1, 0, 0);
        pass.End();
    }

    // Use the depth pipeline to set the depth on the right
    {
        wgpu::RenderPassEncoder pass = encoder.BeginRenderPass(&renderPassDescNoColor);
        pass.SetStencilReference(0x0);
        pass.SetPipeline(depthPipelineNoFragment);
        pass.SetVertexBuffer(0, vertexBuffer);
        // Draw the right line
        pass.Draw(2, 1, 2, 0);
        pass.End();
    }

    // Use the complete pipeline to draw with depth and stencil tests
    {
        wgpu::RenderPassEncoder pass = encoder.BeginRenderPass(&renderPassDescWithColor);
        pass.SetStencilReference(0x1);
        pass.SetPipeline(fullPipeline);
        pass.SetVertexBuffer(0, vertexBuffer);
        // Draw the full line with depth and stencil tests
        pass.Draw(2, 1, 4, 0);
        pass.End();
    }

    wgpu::CommandBuffer commands = encoder.Finish();
    queue.Submit(1, &commands);

    // Only the middle left pixel should pass both stencil and depth tests
    EXPECT_PIXEL_RGBA8_EQ(notFilled, renderTargetColor, 0, 0);
    EXPECT_PIXEL_RGBA8_EQ(filled, renderTargetColor, 1, 0);
    EXPECT_PIXEL_RGBA8_EQ(notFilled, renderTargetColor, 2, 0);
    EXPECT_PIXEL_RGBA8_EQ(notFilled, renderTargetColor, 3, 0);
}

DAWN_INSTANTIATE_TEST(VertexOnlyRenderPipelineTest,
                      D3D11Backend(),
                      D3D11Backend({"use_placeholder_fragment_in_vertex_only_pipeline"}),
                      D3D12Backend(),
                      D3D12Backend({"use_placeholder_fragment_in_vertex_only_pipeline"}),
                      MetalBackend(),
                      MetalBackend({"use_placeholder_fragment_in_vertex_only_pipeline"}),
                      OpenGLBackend(),
                      OpenGLBackend({"use_placeholder_fragment_in_vertex_only_pipeline"}),
                      OpenGLESBackend(),
                      OpenGLESBackend({"use_placeholder_fragment_in_vertex_only_pipeline"}),
                      VulkanBackend(),
                      VulkanBackend({"use_placeholder_fragment_in_vertex_only_pipeline"}));

}  // anonymous namespace
}  // namespace dawn
