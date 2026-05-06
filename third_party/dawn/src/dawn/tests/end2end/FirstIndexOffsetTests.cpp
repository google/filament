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

struct FirstIndexOffset {
    DrawMode mode = DrawMode::NonIndexed;
    uint32_t firstVertex = 0;
    uint32_t firstInstance = 0;
};
class FirstIndexOffsetTests : public DawnTest {
  public:
    void TestVertexIndex(const std::vector<FirstIndexOffset>& offsets);
    void TestInstanceIndex(const std::vector<FirstIndexOffset>& offsets);
    void TestBothIndices(const std::vector<FirstIndexOffset>& offsets);

  protected:
    std::vector<wgpu::FeatureName> GetRequiredFeatures() override {
        if (!SupportsFeatures({wgpu::FeatureName::IndirectFirstInstance})) {
            return {};
        }
        return {wgpu::FeatureName::IndirectFirstInstance};
    }

  private:
    wgpu::Buffer CreateVertexBuffer(uint32_t firstVertexOffset) {
        std::vector<float> vertexData(firstVertexOffset * kComponentsPerVertex);
        vertexData.insert(vertexData.end(), {0, 0, 0, 1});
        vertexData.insert(vertexData.end(), {0, 0, 0, 1});
        return utils::CreateBufferFromData(device, vertexData.data(),
                                           vertexData.size() * sizeof(float),
                                           wgpu::BufferUsage::Vertex);
    }

    void TestImpl(CheckIndex checkIndex, const std::vector<FirstIndexOffset>& offsets);

    static constexpr uint32_t kComponentsPerVertex = 4;
};

void FirstIndexOffsetTests::TestVertexIndex(const std::vector<FirstIndexOffset>& offsets) {
    TestImpl(CheckIndex::Vertex, offsets);
}

void FirstIndexOffsetTests::TestInstanceIndex(const std::vector<FirstIndexOffset>& offsets) {
    TestImpl(CheckIndex::Instance, offsets);
}

void FirstIndexOffsetTests::TestBothIndices(const std::vector<FirstIndexOffset>& offsets) {
    using wgpu::operator|;
    TestImpl(CheckIndex::Vertex | CheckIndex::Instance, offsets);
}

// Conditionally tests if first/baseVertex and/or firstInstance have been correctly passed to the
// vertex shader. Since vertex shaders can't write to storage buffers, we pass vertex/instance
// indices to a fragment shader via u32 attributes. The fragment shader runs once and writes the
// values to a storage buffer. If vertex index is used, the vertex buffer is padded with 0s.
void FirstIndexOffsetTests::TestImpl(CheckIndex checkIndex,
                                     const std::vector<FirstIndexOffset>& offsets) {
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
        vertexOutputs << "  @location(1) @interpolate(flat, either) vertex_index : u32,\n";
        vertexBody << "  output.vertex_index = input.vertex_index;\n";

        fragmentInputs << "  @location(1) @interpolate(flat, either) vertex_index : u32,\n";
        fragmentBody << "  _ = atomicMin(&idx_vals.vertex_index, input.vertex_index);\n";
    }
    if ((checkIndex & CheckIndex::Instance) != 0) {
        vertexInputs << "  @builtin(instance_index) instance_index : u32,\n";
        vertexOutputs << "  @location(2) @interpolate(flat, either) instance_index : u32,\n";
        vertexBody << "  output.instance_index = input.instance_index;\n";

        fragmentInputs << "  @location(2) @interpolate(flat, either) instance_index : u32,\n";
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

    // Create reusable buffers.
    wgpu::Buffer indices =
        utils::CreateBufferFromData<uint32_t>(device, wgpu::BufferUsage::Index, {0});

    // Using arbitrary values for the initial vertex and instance indices.
    const uint32_t bufferInitialVertex =
        checkIndex & CheckIndex::Vertex ? std::numeric_limits<uint32_t>::max() : 0;
    const uint32_t bufferInitialInstance =
        checkIndex & CheckIndex::Instance ? std::numeric_limits<uint32_t>::max() : 0;
    wgpu::Buffer buffer =
        utils::CreateBufferFromData(device, wgpu::BufferUsage::CopySrc | wgpu::BufferUsage::Storage,
                                    {bufferInitialVertex, bufferInitialInstance});

    wgpu::BindGroup bindGroup =
        utils::MakeBindGroup(device, pipeline.GetBindGroupLayout(0), {{0, buffer}});

    wgpu::CommandEncoder encoder = device.CreateCommandEncoder();
    wgpu::RenderPassEncoder pass = encoder.BeginRenderPass(&renderPass.renderPassInfo);

    std::array<uint32_t, 2> expected = {};

    pass.SetPipeline(pipeline);
    pass.SetBindGroup(0, bindGroup);

    // Recording draws with different firstVertex and firstInstance values
    for (const auto& offset : offsets) {
        uint32_t firstVertex = offset.firstVertex;
        uint32_t firstInstance = offset.firstInstance;

        wgpu::Buffer vertices = CreateVertexBuffer(firstVertex);

        wgpu::Buffer indirectBuffer;
        switch (offset.mode) {
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

        pass.SetVertexBuffer(0, vertices);

        switch (offset.mode) {
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

        expected[0] = firstVertex;
        expected[1] = firstInstance;

        // Per the specification, if validation is enabled and indirect-first-instance is not
        // enabled, Draw[Indexed]Indirect with firstInstance > 0 will be a no-op. The buffer should
        // still have the values from the first draw.
        if (firstInstance > 0 && IsIndirectDraw(offset.mode) &&
            !device.HasFeature(wgpu::FeatureName::IndirectFirstInstance) &&
            !HasToggleEnabled("skip_validation")) {
            expected = {bufferInitialVertex, bufferInitialInstance};
        }
    }
    pass.End();
    wgpu::CommandBuffer commands = encoder.Finish();
    queue.Submit(1, &commands);

    EXPECT_BUFFER_U32_RANGE_EQ(expected.data(), buffer, 0, expected.size());
}

// Test that vertex_index starts at 7 when drawn using Draw()
TEST_P(FirstIndexOffsetTests, NonIndexedVertexOffset) {
    // Draw once: vertex_index starts at 9
    {
        TestVertexIndex({{DrawMode::NonIndexed, 9, 0}});
    }

    // Draw twice: vertex_index starts at 9 and 7
    {
        TestVertexIndex({{DrawMode::NonIndexed, 9, 0}, {DrawMode::NonIndexed, 7, 0}});
    }
}

// Test that instance_index when drawn using Draw()
TEST_P(FirstIndexOffsetTests, NonIndexedInstanceOffset) {
    // Draw once: instance_index starts at 13
    {
        TestInstanceIndex({{DrawMode::NonIndexed, 0, 13}});
    }

    // Draw twice: instance_index starts at 13 and 11
    {
        TestInstanceIndex({{DrawMode::NonIndexed, 0, 13}, {DrawMode::NonIndexed, 0, 11}});
    }
}

// Test that vertex_index and instance_index respectively when drawn using Draw()
TEST_P(FirstIndexOffsetTests, NonIndexedBothOffset) {
    // Draw once: vertex_index starts at 7 and instance_index starts at 13
    {
        TestBothIndices({{DrawMode::NonIndexed, 7, 13}});
    }
    // Draw twice: vertex_index starts at 7 , instance_index starts at 13 and 11
    {
        TestBothIndices({{DrawMode::NonIndexed, 7, 13}, {DrawMode::NonIndexed, 7, 11}});
    }
}

// Test vertex_index with DrawIndexed()
TEST_P(FirstIndexOffsetTests, IndexedVertex) {
    // Draw once: vertex_index starts at 9
    {
        TestVertexIndex({{DrawMode::Indexed, 9, 0}});
    }

    // Draw twice: vertex_index starts at 9 and 7
    {
        TestVertexIndex({{DrawMode::Indexed, 9, 0}, {DrawMode::Indexed, 7, 0}});
    }
}

// Test that instance_index when drawn using DrawIndexed()
TEST_P(FirstIndexOffsetTests, IndexedInstance) {
    // Draw once: instance_index starts at 13
    {
        TestInstanceIndex({{DrawMode::Indexed, 0, 13}});
    }

    // Draw twice: instance_index starts at 13 and 11
    {
        TestInstanceIndex({{DrawMode::Indexed, 0, 13}, {DrawMode::Indexed, 0, 11}});
    }
}

// Test that vertex_index and instance_index respectively when drawn using
// DrawIndexed()
TEST_P(FirstIndexOffsetTests, IndexedBothOffset) {
    // Draw once: vertex_index starts at 7 and instance_index starts at 13
    {
        TestBothIndices({{DrawMode::Indexed, 7, 13}});
    }
    // Draw twice: vertex_index starts at 7 , instance_index starts at 13 and 11
    {
        TestBothIndices({{DrawMode::Indexed, 7, 13}, {DrawMode::Indexed, 7, 11}});
    }
}

// Test that vertex index for non-indexed draws is unaffected by previous draws in the same pass.
TEST_P(FirstIndexOffsetTests, IndexedThenNonIndexedVertex) {
    // Draw indexed w/vertex index 42, then non-indexed with vertex index 0.
    {
        TestVertexIndex({{DrawMode::Indexed, 42, 0}, {DrawMode::NonIndexed, 0, 0}});
    }
}

// Test that vertex_index when drawn using DrawIndirect()
TEST_P(FirstIndexOffsetTests, NonIndexedIndirectVertexOffset) {
    // TODO(crbug.com/dawn/1429): Fails with the full validation turned on.
    DAWN_SUPPRESS_TEST_IF(IsD3D12() && IsFullBackendValidationEnabled());

    // TODO(crbug.com/459864802): Fails on Win/Snapdragon X Elite w/ D3D11.
    DAWN_SUPPRESS_TEST_IF(IsWindows() && IsQualcomm() && IsD3D11());

    // Draw once: vertex_index starts at 9
    {
        TestVertexIndex({{DrawMode::NonIndexedIndirect, 9, 0}});
    }

    // Draw twice: vertex_index starts at 9 and 7
    {
        TestVertexIndex(
            {{DrawMode::NonIndexedIndirect, 9, 0}, {DrawMode::NonIndexedIndirect, 7, 0}});
    }
}

// Test that instance_index when drawn using DrawIndirect()
TEST_P(FirstIndexOffsetTests, NonIndexedIndirectInstanceOffset) {
    // Draw once: instance_index starts at 13
    {
        TestInstanceIndex({{DrawMode::NonIndexedIndirect, 0, 13}});
    }

    // Draw twice: instance_index starts at 13 and 11
    {
        TestInstanceIndex(
            {{DrawMode::NonIndexedIndirect, 0, 13}, {DrawMode::NonIndexedIndirect, 0, 11}});
    }
}

// Test that vertex_index and instance_index respectively when drawn using
// DrawIndirect()
TEST_P(FirstIndexOffsetTests, NonIndexedIndirectBothOffset) {
    // TODO(crbug.com/459864802): Fails on Win/Snapdragon X Elite w/ D3D11.
    DAWN_SUPPRESS_TEST_IF(IsWindows() && IsQualcomm() && IsD3D11());

    // Draw once: vertex_index starts at 7 and instance_index starts at 13
    {
        TestBothIndices({{DrawMode::NonIndexedIndirect, 9, 0}});
    }
    // Draw twice: vertex_index starts at 7 , instance_index starts at 13 and 11
    {
        TestBothIndices(
            {{DrawMode::NonIndexedIndirect, 9, 0}, {DrawMode::NonIndexedIndirect, 7, 0}});
    }
}

// Test that vertex_index when drawn using DrawIndexedIndirect()
TEST_P(FirstIndexOffsetTests, IndexedIndirectVertex) {
    // TODO(crbug.com/dawn/1429): Fails with the full validation turned on.
    DAWN_SUPPRESS_TEST_IF(IsD3D12() && IsFullBackendValidationEnabled());

    // TODO(crbug.com/459864802): Fails on Win/Snapdragon X Elite w/ D3D11.
    DAWN_SUPPRESS_TEST_IF(IsWindows() && IsQualcomm() && IsD3D11());

    // Draw once: vertex_index starts at 9
    {
        TestVertexIndex({{DrawMode::IndexedIndirect, 9, 0}});
    }

    // Draw twice: vertex_index starts at 9 and 7
    {
        TestVertexIndex({{DrawMode::IndexedIndirect, 9, 0}, {DrawMode::IndexedIndirect, 7, 0}});
    }
}

// Test that instance_index when drawn using DrawIndexed()
TEST_P(FirstIndexOffsetTests, IndexedIndirectInstance) {
    // Draw once: instance_index starts at 13
    {
        TestInstanceIndex({{DrawMode::IndexedIndirect, 0, 13}});
    }

    // Draw twice: instance_index starts at 13 and 11
    {
        TestInstanceIndex({{DrawMode::IndexedIndirect, 0, 13}, {DrawMode::IndexedIndirect, 0, 11}});
    }
}

// Test that vertex_index and instance_index start at 7 and 11 respectively when drawn using
// DrawIndexed()
TEST_P(FirstIndexOffsetTests, IndexedIndirectBothOffset) {
    // Draw once: vertex_index starts at 7 and instance_index starts at 13
    {
        TestBothIndices({{DrawMode::IndexedIndirect, 7, 13}});
    }
    // Draw twice: vertex_index starts at 7 , instance_index starts at 13 and 11
    {
        TestBothIndices({{DrawMode::IndexedIndirect, 7, 13}, {DrawMode::IndexedIndirect, 7, 11}});
    }
}

DAWN_INSTANTIATE_TEST(FirstIndexOffsetTests,
                      D3D11Backend(),
                      D3D12Backend(),
                      MetalBackend(),
                      OpenGLBackend(),
                      OpenGLESBackend(),
                      VulkanBackend(),
                      WebGPUBackend());

}  // anonymous namespace
}  // namespace dawn
