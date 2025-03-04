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

#include <limits>
#include <sstream>
#include <string>
#include <vector>

#include "dawn/tests/DawnTest.h"
#include "dawn/utils/ComboRenderPipelineDescriptor.h"
#include "dawn/utils/WGPUHelpers.h"

namespace dawn {
enum class CheckIndex : uint32_t {
    Vertex = 0x0000001,
    Instance = 0x0000002,
};
}  // namespace dawn

template <>
struct wgpu::IsWGPUBitmask<dawn::CheckIndex> {
    static constexpr bool enable = true;
};

namespace dawn {
namespace {

constexpr uint32_t kRTSize = 1;

enum class DrawMode {
    NonIndexed,
    Indexed,
    NonIndexedIndirect,
    IndexedIndirect,
};

bool IsIndirectDraw(DrawMode mode) {
    return mode == DrawMode::NonIndexedIndirect || mode == DrawMode::IndexedIndirect;
}

}  // anonymous namespace

namespace {

class FirstIndexOffsetTests : public DawnTest {
  public:
    void TestVertexIndex(DrawMode mode, uint32_t firstVertex);
    void TestInstanceIndex(DrawMode mode, uint32_t firstInstance);
    void TestBothIndices(DrawMode mode, uint32_t firstVertex, uint32_t firstInstance);

  protected:
    std::vector<wgpu::FeatureName> GetRequiredFeatures() override {
        if (!SupportsFeatures({wgpu::FeatureName::IndirectFirstInstance})) {
            return {};
        }
        return {wgpu::FeatureName::IndirectFirstInstance};
    }

  private:
    void TestImpl(DrawMode mode,
                  CheckIndex checkIndex,
                  uint32_t vertexIndex,
                  uint32_t instanceIndex);
};

void FirstIndexOffsetTests::TestVertexIndex(DrawMode mode, uint32_t firstVertex) {
    TestImpl(mode, CheckIndex::Vertex, firstVertex, 0);
}

void FirstIndexOffsetTests::TestInstanceIndex(DrawMode mode, uint32_t firstInstance) {
    TestImpl(mode, CheckIndex::Instance, 0, firstInstance);
}

void FirstIndexOffsetTests::TestBothIndices(DrawMode mode,
                                            uint32_t firstVertex,
                                            uint32_t firstInstance) {
    using wgpu::operator|;
    TestImpl(mode, CheckIndex::Vertex | CheckIndex::Instance, firstVertex, firstInstance);
}

// Conditionally tests if first/baseVertex and/or firstInstance have been correctly passed to the
// vertex shader. Since vertex shaders can't write to storage buffers, we pass vertex/instance
// indices to a fragment shader via u32 attributes. The fragment shader runs once and writes the
// values to a storage buffer. If vertex index is used, the vertex buffer is padded with 0s.
void FirstIndexOffsetTests::TestImpl(DrawMode mode,
                                     CheckIndex checkIndex,
                                     uint32_t firstVertex,
                                     uint32_t firstInstance) {
    // Compatibility mode does not support @interpolate(flat, first).
    // It only supports @interpolate(flat, either).
    DAWN_TEST_UNSUPPORTED_IF(IsCompatibilityMode());

    using wgpu::operator&;

    std::stringstream vertexInputs;
    std::stringstream vertexOutputs;
    std::stringstream vertexBody;
    std::stringstream fragmentInputs;
    std::stringstream fragmentBody;

    vertexInputs << "  @location(0) position : vec4f,\n";
    vertexOutputs << "  @builtin(position) position : vec4f,\n";

    if ((checkIndex & CheckIndex::Vertex) != 0) {
        vertexInputs << "  @builtin(vertex_index) vertex_index : u32,\n";
        vertexOutputs << "  @location(1) @interpolate(flat) vertex_index : u32,\n";
        vertexBody << "  output.vertex_index = input.vertex_index;\n";

        fragmentInputs << "  @location(1) @interpolate(flat) vertex_index : u32,\n";
        fragmentBody << "  _ = atomicMin(&idx_vals.vertex_index, input.vertex_index);\n";
    }
    if ((checkIndex & CheckIndex::Instance) != 0) {
        vertexInputs << "  @builtin(instance_index) instance_index : u32,\n";
        vertexOutputs << "  @location(2) @interpolate(flat) instance_index : u32,\n";
        vertexBody << "  output.instance_index = input.instance_index;\n";

        fragmentInputs << "  @location(2) @interpolate(flat) instance_index : u32,\n";
        fragmentBody << "  _ = atomicMin(&idx_vals.instance_index, input.instance_index);\n";
    }

    std::string vertexShader = R"(
struct VertexInputs {
)" + vertexInputs.str() + R"(
}
struct VertexOutputs {
)" + vertexOutputs.str() + R"(
}
@vertex fn main(input : VertexInputs) -> VertexOutputs {
  var output : VertexOutputs;
)" + vertexBody.str() + R"(
  output.position = input.position;
  return output;
})";

    std::string fragmentShader = R"(
struct IndexVals {
  vertex_index : atomic<u32>,
  instance_index : atomic<u32>,
}
@group(0) @binding(0) var<storage, read_write> idx_vals : IndexVals;

struct FragInputs {
)" + fragmentInputs.str() + R"(
}
@fragment fn main(input : FragInputs) {
)" + fragmentBody.str() + R"(
})";

    utils::BasicRenderPass renderPass = utils::CreateBasicRenderPass(device, kRTSize, kRTSize);

    constexpr uint32_t kComponentsPerVertex = 4;

    utils::ComboRenderPipelineDescriptor pipelineDesc;
    pipelineDesc.vertex.module = utils::CreateShaderModule(device, vertexShader.c_str());
    pipelineDesc.cFragment.module = utils::CreateShaderModule(device, fragmentShader.c_str());
    pipelineDesc.primitive.topology = wgpu::PrimitiveTopology::PointList;
    pipelineDesc.vertex.bufferCount = 1;
    pipelineDesc.cBuffers[0].arrayStride = kComponentsPerVertex * sizeof(float);
    pipelineDesc.cBuffers[0].attributeCount = 1;
    pipelineDesc.cAttributes[0].format = wgpu::VertexFormat::Float32x4;
    pipelineDesc.cTargets[0].format = renderPass.colorFormat;
    pipelineDesc.cTargets[0].writeMask = wgpu::ColorWriteMask::None;

    wgpu::RenderPipeline pipeline = device.CreateRenderPipeline(&pipelineDesc);

    std::vector<float> vertexData(firstVertex * kComponentsPerVertex);
    vertexData.insert(vertexData.end(), {0, 0, 0, 1});
    vertexData.insert(vertexData.end(), {0, 0, 0, 1});
    wgpu::Buffer vertices = utils::CreateBufferFromData(
        device, vertexData.data(), vertexData.size() * sizeof(float), wgpu::BufferUsage::Vertex);
    wgpu::Buffer indices =
        utils::CreateBufferFromData<uint32_t>(device, wgpu::BufferUsage::Index, {0});

    const uint32_t bufferInitialVertex =
        checkIndex & CheckIndex::Vertex ? std::numeric_limits<uint32_t>::max() : 0;
    const uint32_t bufferInitialInstance =
        checkIndex & CheckIndex::Instance ? std::numeric_limits<uint32_t>::max() : 0;
    wgpu::Buffer buffer =
        utils::CreateBufferFromData(device, wgpu::BufferUsage::CopySrc | wgpu::BufferUsage::Storage,
                                    {bufferInitialVertex, bufferInitialInstance});

    wgpu::Buffer indirectBuffer;
    switch (mode) {
        case DrawMode::NonIndexed:
        case DrawMode::Indexed:
            break;
        case DrawMode::NonIndexedIndirect:
            indirectBuffer = utils::CreateBufferFromData<uint32_t>(
                device, wgpu::BufferUsage::Indirect, {1, 1, firstVertex, firstInstance});
            break;
        case DrawMode::IndexedIndirect:
            indirectBuffer = utils::CreateBufferFromData<uint32_t>(
                device, wgpu::BufferUsage::Indirect, {1, 1, 0, firstVertex, firstInstance});
            break;
        default:
            FAIL();
    }

    wgpu::BindGroup bindGroup =
        utils::MakeBindGroup(device, pipeline.GetBindGroupLayout(0), {{0, buffer}});

    wgpu::CommandEncoder encoder = device.CreateCommandEncoder();
    wgpu::RenderPassEncoder pass = encoder.BeginRenderPass(&renderPass.renderPassInfo);
    pass.SetPipeline(pipeline);
    pass.SetVertexBuffer(0, vertices);
    pass.SetBindGroup(0, bindGroup);
    // Do a first draw to make sure the offset values are correctly updated on the next draw.
    // We should only see the values from the second draw.
    std::array<uint32_t, 2> firstDrawValues = {firstVertex + 1, firstInstance + 1};
    pass.Draw(1, 1, firstDrawValues[0], firstDrawValues[1]);
    switch (mode) {
        case DrawMode::NonIndexed:
            pass.Draw(1, 1, firstVertex, firstInstance);
            break;
        case DrawMode::Indexed:
            pass.SetIndexBuffer(indices, wgpu::IndexFormat::Uint32);
            pass.DrawIndexed(1, 1, 0, firstVertex, firstInstance);
            break;
        case DrawMode::NonIndexedIndirect:
            pass.DrawIndirect(indirectBuffer, 0);
            break;
        case DrawMode::IndexedIndirect:
            pass.SetIndexBuffer(indices, wgpu::IndexFormat::Uint32);
            pass.DrawIndexedIndirect(indirectBuffer, 0);
            break;
        default:
            FAIL();
    }
    pass.End();
    wgpu::CommandBuffer commands = encoder.Finish();
    queue.Submit(1, &commands);

    std::array<uint32_t, 2> expected = {firstVertex, firstInstance};

    // Per the specification, if validation is enabled and indirect-first-instance is not enabled,
    // Draw[Indexed]Indirect with firstInstance > 0 will be a no-op. The buffer should still have
    // the values from the first draw.
    if (firstInstance > 0 && IsIndirectDraw(mode) &&
        !device.HasFeature(wgpu::FeatureName::IndirectFirstInstance) &&
        !HasToggleEnabled("skip_validation")) {
        expected = {checkIndex & CheckIndex::Vertex ? firstDrawValues[0] : 0, firstDrawValues[1]};
    }

    EXPECT_BUFFER_U32_RANGE_EQ(expected.data(), buffer, 0, expected.size());
}

// Test that vertex_index starts at 7 when drawn using Draw()
TEST_P(FirstIndexOffsetTests, NonIndexedVertexOffset) {
    TestVertexIndex(DrawMode::NonIndexed, 7);
}

// Test that instance_index starts at 11 when drawn using Draw()
TEST_P(FirstIndexOffsetTests, NonIndexedInstanceOffset) {
    TestInstanceIndex(DrawMode::NonIndexed, 11);
}

// Test that vertex_index and instance_index start at 7 and 11 respectively when drawn using Draw()
TEST_P(FirstIndexOffsetTests, NonIndexedBothOffset) {
    TestBothIndices(DrawMode::NonIndexed, 7, 11);
}

// Test that vertex_index starts at 7 when drawn using DrawIndexed()
TEST_P(FirstIndexOffsetTests, IndexedVertex) {
    TestVertexIndex(DrawMode::Indexed, 7);
}

// Test that instance_index starts at 11 when drawn using DrawIndexed()
TEST_P(FirstIndexOffsetTests, IndexedInstance) {
    TestInstanceIndex(DrawMode::Indexed, 11);
}

// Test that vertex_index and instance_index start at 7 and 11 respectively when drawn using
// DrawIndexed()
TEST_P(FirstIndexOffsetTests, IndexedBothOffset) {
    TestBothIndices(DrawMode::Indexed, 7, 11);
}

// Test that vertex_index starts at 7 when drawn using DrawIndirect()
TEST_P(FirstIndexOffsetTests, NonIndexedIndirectVertexOffset) {
    // TODO(crbug.com/347223100): failing on ANGLE/D3D11
    DAWN_SUPPRESS_TEST_IF(IsOpenGLES() && IsANGLED3D11());

    // TODO(crbug.com/dawn/1429): Fails with the full validation turned on.
    DAWN_SUPPRESS_TEST_IF(IsD3D12() && IsFullBackendValidationEnabled());
    TestVertexIndex(DrawMode::NonIndexedIndirect, 7);
}

// Test that instance_index starts at 11 when drawn using DrawIndirect()
TEST_P(FirstIndexOffsetTests, NonIndexedIndirectInstanceOffset) {
    // TODO(crbug.com/347223100): failing on ANGLE/D3D11
    DAWN_SUPPRESS_TEST_IF(IsOpenGLES() && IsANGLED3D11());

    TestInstanceIndex(DrawMode::NonIndexedIndirect, 11);
}

// Test that vertex_index and instance_index start at 7 and 11 respectively when drawn using
// DrawIndirect()
TEST_P(FirstIndexOffsetTests, NonIndexedIndirectBothOffset) {
    // TODO(crbug.com/347223100): failing on ANGLE/D3D11
    DAWN_SUPPRESS_TEST_IF(IsOpenGLES() && IsANGLED3D11());

    TestBothIndices(DrawMode::NonIndexedIndirect, 7, 11);
}

// Test that vertex_index starts at 7 when drawn using DrawIndexedIndirect()
TEST_P(FirstIndexOffsetTests, IndexedIndirectVertex) {
    // TODO(crbug.com/347223100): failing on ANGLE/D3D11
    DAWN_SUPPRESS_TEST_IF(IsOpenGLES() && IsANGLED3D11());

    // TODO(crbug.com/dawn/1429): Fails with the full validation turned on.
    DAWN_SUPPRESS_TEST_IF(IsD3D12() && IsFullBackendValidationEnabled());
    TestVertexIndex(DrawMode::IndexedIndirect, 7);
}

// Test that instance_index starts at 11 when drawn using DrawIndexed()
TEST_P(FirstIndexOffsetTests, IndexedIndirectInstance) {
    // TODO(crbug.com/347223100): failing on ANGLE/D3D11
    DAWN_SUPPRESS_TEST_IF(IsOpenGLES() && IsANGLED3D11());

    TestInstanceIndex(DrawMode::IndexedIndirect, 11);
}

// Test that vertex_index and instance_index start at 7 and 11 respectively when drawn using
// DrawIndexed()
TEST_P(FirstIndexOffsetTests, IndexedIndirectBothOffset) {
    // TODO(crbug.com/347223100): failing on ANGLE/D3D11
    DAWN_SUPPRESS_TEST_IF(IsOpenGLES() && IsANGLED3D11());

    TestBothIndices(DrawMode::IndexedIndirect, 7, 11);
}

DAWN_INSTANTIATE_TEST(FirstIndexOffsetTests,
                      D3D11Backend(),
                      D3D12Backend(),
                      MetalBackend(),
                      OpenGLBackend(),
                      OpenGLESBackend(),
                      VulkanBackend());

}  // anonymous namespace
}  // namespace dawn
