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

#include <vector>

#include "dawn/tests/DawnTest.h"

#include "dawn/utils/ComboRenderPipelineDescriptor.h"
#include "dawn/utils/WGPUHelpers.h"

namespace dawn {
namespace {

constexpr uint32_t kRTSize = 4;
constexpr utils::RGBA8 filled(0, 255, 0, 255);
constexpr utils::RGBA8 notFilled(0, 0, 0, 0);

class DrawIndirectTest : public DawnTest {
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
              utils::RGBA8 bottomLeftExpected,
              utils::RGBA8 topRightExpected) {
        wgpu::Buffer indirectBuffer =
            utils::CreateBufferFromData<uint32_t>(device, wgpu::BufferUsage::Indirect, bufferList);

        wgpu::CommandEncoder encoder = device.CreateCommandEncoder();
        {
            wgpu::RenderPassEncoder pass = encoder.BeginRenderPass(&renderPass.renderPassInfo);
            pass.SetPipeline(pipeline);
            pass.SetVertexBuffer(0, vertexBuffer);
            pass.DrawIndirect(indirectBuffer, indirectOffset);
            pass.End();
        }

        wgpu::CommandBuffer commands = encoder.Finish();
        queue.Submit(1, &commands);

        EXPECT_PIXEL_RGBA8_EQ(bottomLeftExpected, renderPass.color, 1, 3);
        EXPECT_PIXEL_RGBA8_EQ(topRightExpected, renderPass.color, 3, 1);
    }
};

// The basic triangle draw.
TEST_P(DrawIndirectTest, Uint32) {
    // Test a draw with no indices.
    Test({0, 0, 0, 0}, 0, notFilled, notFilled);

    // Test a draw with only the first 3 indices (bottom left triangle)
    Test({3, 1, 0, 0}, 0, filled, notFilled);

    // Test a draw with only the last 3 indices (top right triangle)
    Test({3, 1, 3, 0}, 0, notFilled, filled);

    // Test a draw with all 6 indices (both triangles).
    Test({6, 1, 0, 0}, 0, filled, filled);
}

TEST_P(DrawIndirectTest, IndirectOffset) {
    // Test an offset draw call, with indirect buffer containing 2 calls:
    // 1) only the first 3 indices (bottom left triangle)
    // 2) only the last 3 indices (top right triangle)

    // Test #1 (no offset)
    Test({3, 1, 0, 0, 3, 1, 3, 0}, 0, filled, notFilled);

    // Offset to draw #2
    Test({3, 1, 0, 0, 3, 1, 3, 0}, 4 * sizeof(uint32_t), notFilled, filled);
}

DAWN_INSTANTIATE_TEST(DrawIndirectTest,
                      D3D11Backend(),
                      D3D12Backend(),
                      MetalBackend(),
                      OpenGLBackend(),
                      OpenGLESBackend(),
                      VulkanBackend());

class DrawIndirectUsingFirstVertexTest : public DawnTest {
  protected:
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
              utils::RGBA8 bottomLeftExpected,
              utils::RGBA8 topRightExpected) {
        // Indirect buffer contains 2 draw params
        DAWN_ASSERT(bufferList.size() == 8);
        wgpu::Buffer indirectBuffer =
            utils::CreateBufferFromData<uint32_t>(device, wgpu::BufferUsage::Indirect, bufferList);

        wgpu::CommandEncoder encoder = device.CreateCommandEncoder();
        {
            wgpu::RenderPassEncoder pass = encoder.BeginRenderPass(&renderPass.renderPassInfo);
            pass.SetPipeline(pipeline);
            pass.SetVertexBuffer(0, vertexBuffer);
            pass.SetBindGroup(0, bindGroup);
            pass.DrawIndirect(indirectBuffer, 0);
            pass.DrawIndirect(indirectBuffer, 4 * sizeof(uint32_t));
            pass.End();
        }

        wgpu::CommandBuffer commands = encoder.Finish();
        queue.Submit(1, &commands);

        EXPECT_PIXEL_RGBA8_EQ(bottomLeftExpected, renderPass.color, 1, 3);
        EXPECT_PIXEL_RGBA8_EQ(topRightExpected, renderPass.color, 3, 1);
    }
};

TEST_P(DrawIndirectUsingFirstVertexTest, IndirectOffset) {
    // Won't fix for OpenGLES + ANGLE D3D11
    DAWN_SUPPRESS_TEST_IF(IsANGLED3D11());

    // Test an offset draw call, with indirect buffer containing 2 calls:
    // 1) only the first 3 indices (bottom left triangle)
    // 2) only the last 3 indices (top right triangle)

    // #2 draw has the correct offset applied by vertex index
    Test({3, 1, 0, 0, 3, 1, 3, 0}, notFilled, filled);
}

DAWN_INSTANTIATE_TEST(DrawIndirectUsingFirstVertexTest,
                      D3D11Backend(),
                      D3D12Backend(),
                      MetalBackend(),
                      OpenGLBackend(),
                      OpenGLESBackend(),
                      VulkanBackend());

class DrawIndirectUsingInstanceIndexTest : public DrawIndirectUsingFirstVertexTest {
  protected:
    std::vector<wgpu::FeatureName> GetRequiredFeatures() override {
        if (!SupportsFeatures({wgpu::FeatureName::IndirectFirstInstance})) {
            return {};
        }
        return {wgpu::FeatureName::IndirectFirstInstance};
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
        DAWN_TEST_UNSUPPORTED_IF(!device.HasFeature(wgpu::FeatureName::IndirectFirstInstance));
        GeneralSetup();
    }
};

TEST_P(DrawIndirectUsingInstanceIndexTest, IndirectOffset) {
    // Test an offset draw call, with indirect buffer containing 2 calls:
    // 1) only the first 3 indices (bottom left triangle)
    // 2) only the last 3 indices (top right triangle)

    // Test 1: #1 draw has the correct calibration referenced by instance index
    Test({3, 1, 0, 1, 3, 1, 3, 0}, filled, notFilled);

    // Test 2: #2 draw has the correct offset applied by instance index
    Test({3, 1, 0, 0, 3, 1, 3, 1}, notFilled, filled);
}

DAWN_INSTANTIATE_TEST(DrawIndirectUsingInstanceIndexTest,
                      D3D11Backend(),
                      D3D12Backend(),
                      MetalBackend(),
                      OpenGLBackend(),
                      OpenGLESBackend(),
                      VulkanBackend());

}  // anonymous namespace
}  // namespace dawn
