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

#include <vector>

#include "dawn/common/Assert.h"
#include "dawn/common/Math.h"
#include "dawn/tests/DawnTest.h"
#include "dawn/utils/ComboRenderPipelineDescriptor.h"
#include "dawn/utils/WGPUHelpers.h"
#include "partition_alloc/pointers/raw_ptr.h"

namespace dawn {
namespace {

using wgpu::VertexFormat;
using wgpu::VertexStepMode;

// Input state tests all work the same way: the test will render triangles in a grid up to 4x4. Each
// triangle is position in the grid such that X will correspond to the "triangle number" and the Y
// to the instance number. Each test will set up an input state and buffers, and the vertex shader
// will check that the vertex attributes corresponds to predetermined values. On success it outputs
// green, otherwise red.
//
// The predetermined values are "K * gl_VertexID + componentIndex" for vertex-indexed buffers, and
// "K * gl_InstanceID + componentIndex" for instance-indexed buffers.

constexpr static unsigned int kRTSize = 400;
constexpr static unsigned int kRTCellOffset = 50;
constexpr static unsigned int kRTCellSize = 100;

class VertexStateTest : public DawnTest {
  protected:
    void SetUp() override {
        DawnTest::SetUp();

        renderPass = utils::CreateBasicRenderPass(device, kRTSize, kRTSize);
    }

    bool ShouldComponentBeDefault(VertexFormat format, int component) {
        EXPECT_TRUE(component >= 0 && component < 4);
        switch (format) {
            case VertexFormat::Float32x4:
            case VertexFormat::Unorm8x4:
                return component >= 4;
            case VertexFormat::Float32x3:
                return component >= 3;
            case VertexFormat::Float32x2:
            case VertexFormat::Unorm8x2:
                return component >= 2;
            case VertexFormat::Float32:
                return component >= 1;
            default:
                DAWN_UNREACHABLE();
        }
    }

    struct ShaderTestSpec {
        uint32_t location;
        VertexFormat format;
        VertexStepMode step;
    };
    wgpu::RenderPipeline MakeTestPipeline(const utils::ComboVertexState& vertexState,
                                          int multiplier,
                                          const std::vector<ShaderTestSpec>& testSpec) {
        std::ostringstream vs;
        vs << "struct VertexIn {\n";

        // TODO(cwallez@chromium.org): this only handles float attributes, we should extend it to
        // other types Adds line of the form
        //    @location(1) input1 : vec4f;
        for (const auto& input : testSpec) {
            vs << "@location(" << input.location << ") input" << input.location << " : vec4f,\n";
        }

        vs << R"(
                @builtin(vertex_index) VertexIndex : u32,
                @builtin(instance_index) InstanceIndex : u32,
            }

            struct VertexOut {
                @location(0) color : vec4f,
                @builtin(position) position : vec4f,
            }

            @vertex fn main(input : VertexIn) -> VertexOut {
                var output : VertexOut;
        )";

        // Hard code the triangle in the shader so that we don't have to add a vertex input for it.
        // Also this places the triangle in the grid based on its VertexID and InstanceID
        vs << "    var pos = array(\n"
              "         vec2f(0.5, 1.0), vec2f(0.0, 0.0), vec2f(1.0, 0.0));\n";
        vs << "    var offset : vec2f = vec2f(f32(input.VertexIndex / 3u), "
              "f32(input.InstanceIndex));\n";
        vs << "    var worldPos = pos[input.VertexIndex % 3u] + offset;\n";
        vs << "    var position = vec4f(0.5 * worldPos - vec2f(1.0, 1.0), 0.0, "
              "1.0);\n";
        vs << "    output.position = vec4f(position.x, -position.y, position.z, position.w);\n";

        // Perform the checks by successively ANDing a boolean
        vs << "    var success = true;\n";
        for (const auto& input : testSpec) {
            for (int component = 0; component < 4; ++component) {
                vs << "    success = success && (input.input" << input.location << "[" << component
                   << "] == ";
                if (ShouldComponentBeDefault(input.format, component)) {
                    vs << (component == 3 ? "1.0" : "0.0");
                } else {
                    if (input.step == VertexStepMode::Vertex) {
                        vs << "f32(" << multiplier << "u * input.VertexIndex) + " << component
                           << ".0";
                    } else {
                        vs << "f32(" << multiplier << "u * input.InstanceIndex) + " << component
                           << ".0";
                    }
                }
                vs << ");\n";
            }
        }

        // Choose the color
        vs << R"(
            if (success) {
                output.color = vec4f(0.0, 1.0, 0.0, 1.0);
            } else {
                output.color = vec4f(1.0, 0.0, 0.0, 1.0);
            }
            return output;
        })";

        wgpu::ShaderModule vsModule = utils::CreateShaderModule(device, vs.str().c_str());
        wgpu::ShaderModule fsModule = utils::CreateShaderModule(device, R"(
            @fragment
            fn main(@location(0) color : vec4f) -> @location(0) vec4f {
                return color;
            }
        )");

        utils::ComboRenderPipelineDescriptor descriptor;
        descriptor.vertex.module = vsModule;
        descriptor.cFragment.module = fsModule;
        descriptor.vertex.bufferCount = vertexState.vertexBufferCount;
        descriptor.vertex.buffers = &vertexState.cVertexBuffers[0];
        descriptor.cTargets[0].format = renderPass.colorFormat;

        return device.CreateRenderPipeline(&descriptor);
    }

    struct VertexAttributeSpec {
        uint32_t location;
        uint64_t offset;
        VertexFormat format;
    };
    struct VertexBufferSpec {
        uint64_t arrayStride;
        VertexStepMode step;
        std::vector<VertexAttributeSpec> attributes;
    };

    void MakeVertexState(const std::vector<VertexBufferSpec>& buffers,
                         utils::ComboVertexState* vertexState) {
        uint32_t vertexBufferCount = 0;
        uint32_t totalNumAttributes = 0;
        for (const VertexBufferSpec& buffer : buffers) {
            vertexState->cVertexBuffers[vertexBufferCount].arrayStride = buffer.arrayStride;
            vertexState->cVertexBuffers[vertexBufferCount].stepMode = buffer.step;

            vertexState->cVertexBuffers[vertexBufferCount].attributes =
                &vertexState->cAttributes[totalNumAttributes];

            for (const VertexAttributeSpec& attribute : buffer.attributes) {
                vertexState->cAttributes[totalNumAttributes].shaderLocation = attribute.location;
                vertexState->cAttributes[totalNumAttributes].offset = attribute.offset;
                vertexState->cAttributes[totalNumAttributes].format = attribute.format;
                totalNumAttributes++;
            }
            vertexState->cVertexBuffers[vertexBufferCount].attributeCount =
                buffer.attributes.size();

            vertexBufferCount++;
        }

        vertexState->vertexBufferCount = vertexBufferCount;
    }

    template <typename T>
    wgpu::Buffer MakeVertexBuffer(std::vector<T> data) {
        return utils::CreateBufferFromData(device, data.data(),
                                           static_cast<uint32_t>(data.size() * sizeof(T)),
                                           wgpu::BufferUsage::Vertex);
    }

    struct DrawVertexBuffer {
        uint32_t location;
        raw_ptr<wgpu::Buffer> buffer;
    };
    void DoTestDraw(const wgpu::RenderPipeline& pipeline,
                    unsigned int triangles,
                    unsigned int instances,
                    std::vector<DrawVertexBuffer> vertexBuffers) {
        EXPECT_LE(triangles, 4u);
        EXPECT_LE(instances, 4u);

        wgpu::CommandEncoder encoder = device.CreateCommandEncoder();

        wgpu::RenderPassEncoder pass = encoder.BeginRenderPass(&renderPass.renderPassInfo);
        pass.SetPipeline(pipeline);

        for (const DrawVertexBuffer& buffer : vertexBuffers) {
            pass.SetVertexBuffer(buffer.location, *buffer.buffer);
        }

        pass.Draw(triangles * 3, instances);
        pass.End();

        wgpu::CommandBuffer commands = encoder.Finish();
        queue.Submit(1, &commands);

        CheckResult(triangles, instances);
    }

    void CheckResult(unsigned int triangles, unsigned int instances) {
        // Check that the center of each triangle is pure green, so that if a single vertex shader
        // instance fails, linear interpolation makes the pixel check fail.
        for (unsigned int triangle = 0; triangle < 4; triangle++) {
            for (unsigned int instance = 0; instance < 4; instance++) {
                unsigned int x = kRTCellOffset + kRTCellSize * triangle;
                unsigned int y = kRTCellOffset + kRTCellSize * instance;
                if (triangle < triangles && instance < instances) {
                    EXPECT_PIXEL_RGBA8_EQ(utils::RGBA8::kGreen, renderPass.color, x, y);
                } else {
                    EXPECT_PIXEL_RGBA8_EQ(utils::RGBA8::kZero, renderPass.color, x, y);
                }
            }
        }
    }

    utils::BasicRenderPass renderPass;
};

// Test compilation and usage of the fixture :)
TEST_P(VertexStateTest, Basic) {
    utils::ComboVertexState vertexState;
    MakeVertexState(
        {{4 * sizeof(float), VertexStepMode::Vertex, {{0, 0, VertexFormat::Float32x4}}}},
        &vertexState);
    wgpu::RenderPipeline pipeline =
        MakeTestPipeline(vertexState, 1, {{0, VertexFormat::Float32x4, VertexStepMode::Vertex}});

    // clang-format off
    wgpu::Buffer buffer0 = MakeVertexBuffer<float>({
        0, 1, 2, 3,
        1, 2, 3, 4,
        2, 3, 4, 5
    });
    // clang-format on
    DoTestDraw(pipeline, 1, 1, {DrawVertexBuffer{0, &buffer0}});
}

// Test a stride of 0 works
TEST_P(VertexStateTest, ZeroStride) {
    // This test was failing only on AMD but the OpenGL backend doesn't gather PCI info yet.
    DAWN_SUPPRESS_TEST_IF(IsLinux() && IsOpenGL());

    utils::ComboVertexState vertexState;
    MakeVertexState({{0, VertexStepMode::Vertex, {{0, 0, VertexFormat::Float32x4}}}}, &vertexState);
    wgpu::RenderPipeline pipeline =
        MakeTestPipeline(vertexState, 0, {{0, VertexFormat::Float32x4, VertexStepMode::Vertex}});

    wgpu::Buffer buffer0 = MakeVertexBuffer<float>({
        0,
        1,
        2,
        3,
    });
    DoTestDraw(pipeline, 1, 1, {DrawVertexBuffer{0, &buffer0}});
}

// Test attributes defaults to (0, 0, 0, 1) if the input state doesn't have all components
TEST_P(VertexStateTest, AttributeExpanding) {
    // This test was failing only on AMD but the OpenGL backend doesn't gather PCI info yet.
    DAWN_SUPPRESS_TEST_IF(IsLinux() && IsOpenGL());

    // R32F case
    {
        utils::ComboVertexState vertexState;
        MakeVertexState({{0, VertexStepMode::Vertex, {{0, 0, VertexFormat::Float32}}}},
                        &vertexState);
        wgpu::RenderPipeline pipeline =
            MakeTestPipeline(vertexState, 0, {{0, VertexFormat::Float32, VertexStepMode::Vertex}});

        wgpu::Buffer buffer0 = MakeVertexBuffer<float>({0, 1, 2, 3});
        DoTestDraw(pipeline, 1, 1, {DrawVertexBuffer{0, &buffer0}});
    }
    // RG32F case
    {
        utils::ComboVertexState vertexState;
        MakeVertexState({{0, VertexStepMode::Vertex, {{0, 0, VertexFormat::Float32x2}}}},
                        &vertexState);
        wgpu::RenderPipeline pipeline = MakeTestPipeline(
            vertexState, 0, {{0, VertexFormat::Float32x2, VertexStepMode::Vertex}});

        wgpu::Buffer buffer0 = MakeVertexBuffer<float>({0, 1, 2, 3});
        DoTestDraw(pipeline, 1, 1, {DrawVertexBuffer{0, &buffer0}});
    }
    // RGB32F case
    {
        utils::ComboVertexState vertexState;
        MakeVertexState({{0, VertexStepMode::Vertex, {{0, 0, VertexFormat::Float32x3}}}},
                        &vertexState);
        wgpu::RenderPipeline pipeline = MakeTestPipeline(
            vertexState, 0, {{0, VertexFormat::Float32x3, VertexStepMode::Vertex}});

        wgpu::Buffer buffer0 = MakeVertexBuffer<float>({0, 1, 2, 3});
        DoTestDraw(pipeline, 1, 1, {DrawVertexBuffer{0, &buffer0}});
    }
}

// Test a stride larger than the attributes
TEST_P(VertexStateTest, StrideLargerThanAttributes) {
    // This test was failing only on AMD but the OpenGL backend doesn't gather PCI info yet.
    DAWN_SUPPRESS_TEST_IF(IsLinux() && IsOpenGL());

    utils::ComboVertexState vertexState;
    MakeVertexState(
        {{8 * sizeof(float), VertexStepMode::Vertex, {{0, 0, VertexFormat::Float32x4}}}},
        &vertexState);
    wgpu::RenderPipeline pipeline =
        MakeTestPipeline(vertexState, 1, {{0, VertexFormat::Float32x4, VertexStepMode::Vertex}});

    // clang-format off
    wgpu::Buffer buffer0 = MakeVertexBuffer<float>({
        0, 1, 2, 3, 0, 0, 0, 0,
        1, 2, 3, 4, 0, 0, 0, 0,
        2, 3, 4, 5, 0, 0, 0, 0,
    });
    // clang-format on
    DoTestDraw(pipeline, 1, 1, {DrawVertexBuffer{0, &buffer0}});
}

// Test two attributes at an offset, vertex version
TEST_P(VertexStateTest, TwoAttributesAtAnOffsetVertex) {
    utils::ComboVertexState vertexState;
    MakeVertexState(
        {{8 * sizeof(float),
          VertexStepMode::Vertex,
          {{0, 0, VertexFormat::Float32x4}, {1, 4 * sizeof(float), VertexFormat::Float32x4}}}},
        &vertexState);
    wgpu::RenderPipeline pipeline =
        MakeTestPipeline(vertexState, 1, {{0, VertexFormat::Float32x4, VertexStepMode::Vertex}});

    // clang-format off
    wgpu::Buffer buffer0 = MakeVertexBuffer<float>({
        0, 1, 2, 3, 0, 1, 2, 3,
        1, 2, 3, 4, 1, 2, 3, 4,
        2, 3, 4, 5, 2, 3, 4, 5,
    });
    // clang-format on
    DoTestDraw(pipeline, 1, 1, {DrawVertexBuffer{0, &buffer0}});
}

// Test two attributes at an offset, instance version
TEST_P(VertexStateTest, TwoAttributesAtAnOffsetInstance) {
    utils::ComboVertexState vertexState;
    MakeVertexState(
        {{8 * sizeof(float),
          VertexStepMode::Instance,
          {{0, 0, VertexFormat::Float32x4}, {1, 4 * sizeof(float), VertexFormat::Float32x4}}}},
        &vertexState);
    wgpu::RenderPipeline pipeline =
        MakeTestPipeline(vertexState, 1, {{0, VertexFormat::Float32x4, VertexStepMode::Instance}});

    // clang-format off
    wgpu::Buffer buffer0 = MakeVertexBuffer<float>({
        0, 1, 2, 3, 0, 1, 2, 3,
        1, 2, 3, 4, 1, 2, 3, 4,
        2, 3, 4, 5, 2, 3, 4, 5,
    });
    // clang-format on
    DoTestDraw(pipeline, 1, 1, {DrawVertexBuffer{0, &buffer0}});
}

// Test a pure-instance input state
TEST_P(VertexStateTest, PureInstance) {
    utils::ComboVertexState vertexState;
    MakeVertexState(
        {{4 * sizeof(float), VertexStepMode::Instance, {{0, 0, VertexFormat::Float32x4}}}},
        &vertexState);
    wgpu::RenderPipeline pipeline =
        MakeTestPipeline(vertexState, 1, {{0, VertexFormat::Float32x4, VertexStepMode::Instance}});

    // clang-format off
    wgpu::Buffer buffer0 = MakeVertexBuffer<float>({
        0, 1, 2, 3,
        1, 2, 3, 4,
        2, 3, 4, 5,
        3, 4, 5, 6,
    });
    // clang-format on
    DoTestDraw(pipeline, 1, 4, {DrawVertexBuffer{0, &buffer0}});
}

// Test with mixed everything, vertex vs. instance, different stride and offsets
// different attribute types
TEST_P(VertexStateTest, MixedEverything) {
    utils::ComboVertexState vertexState;
    MakeVertexState(
        {{12 * sizeof(float),
          VertexStepMode::Vertex,
          {{0, 0, VertexFormat::Float32}, {1, 6 * sizeof(float), VertexFormat::Float32x2}}},
         {10 * sizeof(float),
          VertexStepMode::Instance,
          {{2, 0, VertexFormat::Float32x3}, {3, 5 * sizeof(float), VertexFormat::Float32x4}}}},
        &vertexState);
    wgpu::RenderPipeline pipeline =
        MakeTestPipeline(vertexState, 1,
                         {{0, VertexFormat::Float32, VertexStepMode::Vertex},
                          {1, VertexFormat::Float32x2, VertexStepMode::Vertex},
                          {2, VertexFormat::Float32x3, VertexStepMode::Instance},
                          {3, VertexFormat::Float32x4, VertexStepMode::Instance}});

    // clang-format off
    wgpu::Buffer buffer0 = MakeVertexBuffer<float>({
        0, 1, 2, 3, 0, 0, 0, 1, 2, 3, 0, 0,
        1, 2, 3, 4, 0, 0, 1, 2, 3, 4, 0, 0,
        2, 3, 4, 5, 0, 0, 2, 3, 4, 5, 0, 0,
        3, 4, 5, 6, 0, 0, 3, 4, 5, 6, 0, 0,
    });
    wgpu::Buffer buffer1 = MakeVertexBuffer<float>({
        0, 1, 2, 3, 0, 0, 1, 2, 3, 0,
        1, 2, 3, 4, 0, 1, 2, 3, 4, 0,
        2, 3, 4, 5, 0, 2, 3, 4, 5, 0,
        3, 4, 5, 6, 0, 3, 4, 5, 6, 0,
    });
    // clang-format on
    DoTestDraw(pipeline, 1, 1, {{0, &buffer0}, {1, &buffer1}});
}

// Test input state is unaffected by unused vertex slot
TEST_P(VertexStateTest, UnusedVertexSlot) {
    // Instance input state, using slot 1
    utils::ComboVertexState instanceVertexState;
    MakeVertexState(
        {{0, VertexStepMode::Vertex, {}},
         {4 * sizeof(float), VertexStepMode::Instance, {{0, 0, VertexFormat::Float32x4}}}},
        &instanceVertexState);
    wgpu::RenderPipeline instancePipeline = MakeTestPipeline(
        instanceVertexState, 1, {{0, VertexFormat::Float32x4, VertexStepMode::Instance}});

    // clang-format off
    wgpu::Buffer buffer = MakeVertexBuffer<float>({
        0, 1, 2, 3,
        1, 2, 3, 4,
        2, 3, 4, 5,
        3, 4, 5, 6,
    });
    // clang-format on

    wgpu::CommandEncoder encoder = device.CreateCommandEncoder();

    wgpu::RenderPassEncoder pass = encoder.BeginRenderPass(&renderPass.renderPassInfo);

    pass.SetVertexBuffer(0, buffer);
    pass.SetVertexBuffer(1, buffer);

    pass.SetPipeline(instancePipeline);
    pass.Draw(3, 4);

    pass.End();

    wgpu::CommandBuffer commands = encoder.Finish();
    queue.Submit(1, &commands);

    CheckResult(1, 4);
}

// Test setting a different pipeline with a different input state.
// This was a problem with the D3D12 backend where SetVertexBuffer
// was getting the input from the last set pipeline, not the current.
// SetVertexBuffer should be reapplied when the input state changes.
TEST_P(VertexStateTest, MultiplePipelinesMixedVertexState) {
    // Basic input state, using slot 0
    utils::ComboVertexState vertexVertexState;
    MakeVertexState(
        {{4 * sizeof(float), VertexStepMode::Vertex, {{0, 0, VertexFormat::Float32x4}}}},
        &vertexVertexState);
    wgpu::RenderPipeline vertexPipeline = MakeTestPipeline(
        vertexVertexState, 1, {{0, VertexFormat::Float32x4, VertexStepMode::Vertex}});

    // Instance input state, using slot 1
    utils::ComboVertexState instanceVertexState;
    MakeVertexState(
        {{0, VertexStepMode::Instance, {}},
         {4 * sizeof(float), VertexStepMode::Instance, {{0, 0, VertexFormat::Float32x4}}}},
        &instanceVertexState);
    wgpu::RenderPipeline instancePipeline = MakeTestPipeline(
        instanceVertexState, 1, {{0, VertexFormat::Float32x4, VertexStepMode::Instance}});

    // clang-format off
    wgpu::Buffer buffer = MakeVertexBuffer<float>({
        0, 1, 2, 3,
        1, 2, 3, 4,
        2, 3, 4, 5,
        3, 4, 5, 6,
    });
    // clang-format on

    wgpu::CommandEncoder encoder = device.CreateCommandEncoder();

    wgpu::RenderPassEncoder pass = encoder.BeginRenderPass(&renderPass.renderPassInfo);

    pass.SetVertexBuffer(0, buffer);
    pass.SetVertexBuffer(1, buffer);

    pass.SetPipeline(vertexPipeline);
    pass.Draw(3);

    pass.SetPipeline(instancePipeline);
    pass.Draw(3, 4);

    pass.End();

    wgpu::CommandBuffer commands = encoder.Finish();
    queue.Submit(1, &commands);

    CheckResult(1, 4);
}

// Checks that using the last vertex buffer doesn't overflow the vertex buffer table in Metal.
TEST_P(VertexStateTest, LastAllowedVertexBuffer) {
    constexpr uint32_t kBufferIndex = kMaxVertexBuffers - 1;

    utils::ComboVertexState vertexState;
    // All the other vertex buffers default to no attributes
    vertexState.vertexBufferCount = kMaxVertexBuffers;
    vertexState.cVertexBuffers[kBufferIndex].arrayStride = 4 * sizeof(float);
    // (Off-topic) spot-test for defaulting of .stepMode.
    vertexState.cVertexBuffers[kBufferIndex].stepMode = VertexStepMode::Undefined;
    vertexState.cVertexBuffers[kBufferIndex].attributeCount = 1;
    vertexState.cVertexBuffers[kBufferIndex].attributes = &vertexState.cAttributes[0];
    vertexState.cAttributes[0].shaderLocation = 0;
    vertexState.cAttributes[0].offset = 0;
    vertexState.cAttributes[0].format = VertexFormat::Float32x4;

    for (uint32_t i = 0; i < kBufferIndex; i++) {
        vertexState.cVertexBuffers[i].stepMode = VertexStepMode::Undefined;
    }

    wgpu::RenderPipeline pipeline =
        MakeTestPipeline(vertexState, 1, {{0, VertexFormat::Float32x4, VertexStepMode::Vertex}});

    wgpu::Buffer buffer0 = MakeVertexBuffer<float>({0, 1, 2, 3, 1, 2, 3, 4, 2, 3, 4, 5});
    DoTestDraw(pipeline, 1, 1, {DrawVertexBuffer{kMaxVertexBuffers - 1, &buffer0}});
}

// Test that overlapping vertex attributes are permitted and load data correctly
TEST_P(VertexStateTest, OverlappingVertexAttributes) {
    utils::BasicRenderPass renderPass = utils::CreateBasicRenderPass(device, 3, 3);

    utils::ComboVertexState vertexState;
    MakeVertexState({{16,
                      VertexStepMode::Vertex,
                      {
                          // "****" represents the bytes we'll actually read in the shader.
                          {0, 0 /* offset */, VertexFormat::Float32x4},  // |****|----|----|----|
                          {1, 4 /* offset */, VertexFormat::Uint32x2},   //      |****|****|
                          {2, 8 /* offset */, VertexFormat::Float16x4},  //           |-----****|
                          {3, 0 /* offset */, VertexFormat::Float32},    // |****|
                      }}},
                    &vertexState);

    struct Data {
        float fvalue;
        uint32_t uints[2];
        uint16_t halfs[2];
    };
    static_assert(sizeof(Data) == 16);
    Data data{1.f, {2u, 3u}, {Float32ToFloat16(4.f), Float32ToFloat16(5.f)}};

    wgpu::Buffer vertexBuffer =
        utils::CreateBufferFromData(device, &data, sizeof(data), wgpu::BufferUsage::Vertex);

    utils::ComboRenderPipelineDescriptor pipelineDesc;
    pipelineDesc.vertex.module = utils::CreateShaderModule(device, R"(
        struct VertexIn {
            @location(0) attr0 : vec4f,
            @location(1) attr1 : vec2u,
            @location(2) attr2 : vec4f,
            @location(3) attr3 : f32,
        }

        struct VertexOut {
            @location(0) color : vec4f,
            @builtin(position) position : vec4f,
        }

        @vertex fn main(input : VertexIn) -> VertexOut {
            var output : VertexOut;
            output.position = vec4f(0.0, 0.0, 0.0, 1.0);

            var success : bool = (
                input.attr0.x == 1.0 &&
                input.attr1.x == 2u &&
                input.attr1.y == 3u &&
                input.attr2.z == 4.0 &&
                input.attr2.w == 5.0 &&
                input.attr3 == 1.0
            );
            if (success) {
                output.color = vec4f(0.0, 1.0, 0.0, 1.0);
            } else {
                output.color = vec4f(1.0, 0.0, 0.0, 1.0);
            }
            return output;
        })");
    pipelineDesc.cFragment.module = utils::CreateShaderModule(device, R"(
        @fragment
        fn main(@location(0) color : vec4f) -> @location(0) vec4f {
            return color;
        })");
    pipelineDesc.vertex.bufferCount = vertexState.vertexBufferCount;
    pipelineDesc.vertex.buffers = &vertexState.cVertexBuffers[0];
    pipelineDesc.cTargets[0].format = renderPass.colorFormat;
    pipelineDesc.primitive.topology = wgpu::PrimitiveTopology::PointList;

    wgpu::RenderPipeline pipeline = device.CreateRenderPipeline(&pipelineDesc);

    wgpu::CommandEncoder encoder = device.CreateCommandEncoder();
    wgpu::RenderPassEncoder pass = encoder.BeginRenderPass(&renderPass.renderPassInfo);
    pass.SetPipeline(pipeline);
    pass.SetVertexBuffer(0, vertexBuffer);
    pass.Draw(1);
    pass.End();
    wgpu::CommandBuffer commands = encoder.Finish();
    queue.Submit(1, &commands);

    EXPECT_PIXEL_RGBA8_EQ(utils::RGBA8::kGreen, renderPass.color, 1, 1);
}

DAWN_INSTANTIATE_TEST(VertexStateTest,
                      D3D11Backend(),
                      D3D12Backend(),
                      MetalBackend(),
                      OpenGLBackend(),
                      OpenGLESBackend(),
                      VulkanBackend());

class OptionalVertexStateTest : public DawnTest {};

// Test that vertex input is not required in render pipeline descriptor.
TEST_P(OptionalVertexStateTest, Basic) {
    utils::BasicRenderPass renderPass = utils::CreateBasicRenderPass(device, 3, 3);

    wgpu::ShaderModule vsModule = utils::CreateShaderModule(device, R"(
        @vertex fn main() -> @builtin(position) vec4f {
            return vec4f(0.0, 0.0, 0.0, 1.0);
        })");

    wgpu::ShaderModule fsModule = utils::CreateShaderModule(device, R"(
        @fragment fn main() -> @location(0) vec4f {
            return vec4f(0.0, 1.0, 0.0, 1.0);
        })");

    utils::ComboRenderPipelineDescriptor descriptor;
    descriptor.vertex.module = vsModule;
    descriptor.cFragment.module = fsModule;
    descriptor.primitive.topology = wgpu::PrimitiveTopology::PointList;
    descriptor.vertex.bufferCount = 0;
    descriptor.vertex.buffers = nullptr;

    wgpu::RenderPipeline pipeline = device.CreateRenderPipeline(&descriptor);

    wgpu::CommandEncoder encoder = device.CreateCommandEncoder();
    {
        wgpu::RenderPassEncoder pass = encoder.BeginRenderPass(&renderPass.renderPassInfo);
        pass.SetPipeline(pipeline);
        pass.Draw(1);
        pass.End();
    }

    wgpu::CommandBuffer commands = encoder.Finish();
    queue.Submit(1, &commands);

    EXPECT_PIXEL_RGBA8_EQ(utils::RGBA8::kGreen, renderPass.color, 1, 1);
}

DAWN_INSTANTIATE_TEST(OptionalVertexStateTest,
                      D3D11Backend(),
                      D3D12Backend(),
                      MetalBackend(),
                      OpenGLBackend(),
                      OpenGLESBackend(),
                      VulkanBackend());

}  // anonymous namespace
}  // namespace dawn
