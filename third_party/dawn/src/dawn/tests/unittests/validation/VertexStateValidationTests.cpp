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

#include <limits>
#include <string>

#include "dawn/tests/unittests/validation/ValidationTest.h"
#include "dawn/utils/ComboRenderPipelineDescriptor.h"
#include "dawn/utils/WGPUHelpers.h"

namespace dawn {
namespace {

class VertexStateTest : public ValidationTest {
  protected:
    void CreatePipeline(bool success,
                        const utils::ComboVertexState& state,
                        const char* vertexSource) {
        wgpu::ShaderModule vsModule = utils::CreateShaderModule(device, vertexSource);
        wgpu::ShaderModule fsModule = utils::CreateShaderModule(device, R"(
            @fragment fn main() -> @location(0) vec4f {
                return vec4f(1.0, 0.0, 0.0, 1.0);
            }
        )");

        utils::ComboRenderPipelineDescriptor descriptor;
        descriptor.vertex.module = vsModule;
        descriptor.vertex.bufferCount = state.vertexBufferCount;
        descriptor.vertex.buffers = &state.cVertexBuffers[0];
        descriptor.cFragment.module = fsModule;
        descriptor.cTargets[0].format = wgpu::TextureFormat::RGBA8Unorm;

        if (!success) {
            ASSERT_DEVICE_ERROR(device.CreateRenderPipeline(&descriptor));
        } else {
            device.CreateRenderPipeline(&descriptor);
        }
    }

    const char* kPlaceholderVertexShader = R"(
        @vertex fn main() -> @builtin(position) vec4f {
            return vec4f(0.0, 0.0, 0.0, 0.0);
        }
    )";
};

// Check an empty vertex input is valid
TEST_F(VertexStateTest, EmptyIsOk) {
    utils::ComboVertexState state;
    CreatePipeline(true, state, kPlaceholderVertexShader);
}

// Check null buffer is valid
TEST_F(VertexStateTest, NullBufferIsOk) {
    utils::ComboVertexState state;
    // One null buffer (buffer[0]) is OK
    state.vertexBufferCount = 1;
    state.cVertexBuffers[0].arrayStride = 0;
    state.cVertexBuffers[0].attributeCount = 0;
    state.cVertexBuffers[0].attributes = nullptr;
    CreatePipeline(true, state, kPlaceholderVertexShader);

    // One null buffer (buffer[0]) followed by a buffer (buffer[1]) is OK
    state.vertexBufferCount = 2;
    state.cVertexBuffers[1].arrayStride = 0;
    state.cVertexBuffers[1].attributeCount = 1;
    state.cVertexBuffers[1].attributes = &state.cAttributes[0];
    state.cAttributes[0].shaderLocation = 0;
    CreatePipeline(true, state, kPlaceholderVertexShader);

    // Null buffer (buffer[2]) sitting between buffers (buffer[1] and buffer[3]) is OK
    state.vertexBufferCount = 4;
    state.cVertexBuffers[2].attributeCount = 0;
    state.cVertexBuffers[2].attributes = nullptr;
    state.cVertexBuffers[3].attributeCount = 1;
    state.cVertexBuffers[3].attributes = &state.cAttributes[1];
    state.cAttributes[1].shaderLocation = 1;
    CreatePipeline(true, state, kPlaceholderVertexShader);
}

// Check validation that pipeline vertex buffers are backed by attributes in the vertex input
TEST_F(VertexStateTest, PipelineCompatibility) {
    utils::ComboVertexState state;
    state.vertexBufferCount = 1;
    state.cVertexBuffers[0].arrayStride = 2 * sizeof(float);
    state.cVertexBuffers[0].attributeCount = 2;
    state.cAttributes[0].shaderLocation = 0;
    state.cAttributes[1].shaderLocation = 1;
    state.cAttributes[1].offset = sizeof(float);

    // Control case: pipeline with one input per attribute
    CreatePipeline(true, state, R"(
        @vertex fn main(
            @location(0) a : vec4f,
            @location(1) b : vec4f
        ) -> @builtin(position) vec4f {
            return vec4f(0.0, 0.0, 0.0, 0.0);
        }
    )");

    // Check it is valid for the pipeline to use a subset of the VertexState
    CreatePipeline(true, state, R"(
        @vertex fn main(
            @location(0) a : vec4f
        ) -> @builtin(position) vec4f {
            return vec4f(0.0, 0.0, 0.0, 0.0);
        }
    )");

    // Check for an error when the pipeline uses an attribute not in the vertex input
    CreatePipeline(false, state, R"(
        @vertex fn main(
            @location(2) a : vec4f
        ) -> @builtin(position) vec4f {
            return vec4f(0.0, 0.0, 0.0, 0.0);
        }
    )");
}

// Test that a arrayStride of 0 is valid
TEST_F(VertexStateTest, StrideZero) {
    // Works ok without attributes
    utils::ComboVertexState state;
    state.vertexBufferCount = 1;
    state.cVertexBuffers[0].arrayStride = 0;
    state.cVertexBuffers[0].attributeCount = 1;
    CreatePipeline(true, state, kPlaceholderVertexShader);

    // Works ok with attributes at a large-ish offset
    state.cAttributes[0].offset = 128;
    CreatePipeline(true, state, kPlaceholderVertexShader);
}

// Check validation that vertex attribute offset should be within vertex buffer arrayStride,
// if vertex buffer arrayStride is not zero.
TEST_F(VertexStateTest, SetOffsetOutOfBounds) {
    // Control case, setting correct arrayStride and offset
    utils::ComboVertexState state;
    state.vertexBufferCount = 1;
    state.cVertexBuffers[0].arrayStride = 2 * sizeof(float);
    state.cVertexBuffers[0].attributeCount = 2;
    state.cAttributes[0].shaderLocation = 0;
    state.cAttributes[1].shaderLocation = 1;
    state.cAttributes[1].offset = sizeof(float);
    CreatePipeline(true, state, kPlaceholderVertexShader);

    // Test vertex attribute offset exceed vertex buffer arrayStride range
    state.cVertexBuffers[0].arrayStride = sizeof(float);
    CreatePipeline(false, state, kPlaceholderVertexShader);

    // It's OK if arrayStride is zero
    state.cVertexBuffers[0].arrayStride = 0;
    CreatePipeline(true, state, kPlaceholderVertexShader);
}

// Check out of bounds condition on total number of vertex buffers
TEST_F(VertexStateTest, SetVertexBuffersNumLimit) {
    // Control case, setting max vertex buffer number
    utils::ComboVertexState state;
    state.vertexBufferCount = kMaxVertexBuffers;
    for (uint32_t i = 0; i < kMaxVertexBuffers; ++i) {
        state.cVertexBuffers[i].attributeCount = 1;
        state.cVertexBuffers[i].attributes = &state.cAttributes[i];
        state.cAttributes[i].shaderLocation = i;
    }
    CreatePipeline(true, state, kPlaceholderVertexShader);

    // Test vertex buffer number exceed the limit
    wgpu::VertexAttribute attributes = {};
    attributes.shaderLocation = 0;
    wgpu::VertexBufferLayout bufferLayout = {};
    bufferLayout.attributeCount = 1;
    bufferLayout.attributes = &attributes;

    state.vertexBufferCount = kMaxVertexBuffers + 1;
    state.cVertexBuffers.push_back(bufferLayout);
    CreatePipeline(false, state, kPlaceholderVertexShader);
}

// Check out of bounds condition on total number of vertex attributes
TEST_F(VertexStateTest, SetVertexAttributesNumLimit) {
    wgpu::SupportedLimits limits;
    device.GetLimits(&limits);
    uint32_t maxVertexAttributes = limits.limits.maxVertexAttributes;

    // Control case, setting max vertex attribute number
    utils::ComboVertexState state;
    state.vertexBufferCount = 2;
    state.cVertexBuffers[0].attributeCount = maxVertexAttributes;
    for (uint32_t i = 0; i < maxVertexAttributes; ++i) {
        state.cAttributes[i].shaderLocation = i;
    }
    CreatePipeline(true, state, kPlaceholderVertexShader);

    // Test vertex attribute number exceed the limit
    state.cVertexBuffers[1].attributeCount = 1;
    state.cVertexBuffers[1].attributes = &state.cAttributes[maxVertexAttributes - 1];
    CreatePipeline(false, state, kPlaceholderVertexShader);
}

// Check out of bounds condition on input arrayStride
TEST_F(VertexStateTest, SetInputStrideOutOfBounds) {
    // Control case, setting max input arrayStride
    utils::ComboVertexState state;
    state.vertexBufferCount = 1;
    state.cVertexBuffers[0].arrayStride = kMaxVertexBufferArrayStride;
    state.cVertexBuffers[0].attributeCount = 1;
    CreatePipeline(true, state, kPlaceholderVertexShader);

    // Test input arrayStride OOB
    state.cVertexBuffers[0].arrayStride = kMaxVertexBufferArrayStride + 1;
    CreatePipeline(false, state, kPlaceholderVertexShader);
}

// Check multiple of 4 bytes constraint on input arrayStride
TEST_F(VertexStateTest, SetInputStrideNotAligned) {
    // Control case, setting input arrayStride 4 bytes.
    utils::ComboVertexState state;
    state.vertexBufferCount = 1;
    state.cVertexBuffers[0].arrayStride = 4;
    state.cVertexBuffers[0].attributeCount = 1;
    CreatePipeline(true, state, kPlaceholderVertexShader);

    // Test input arrayStride not multiple of 4 bytes
    state.cVertexBuffers[0].arrayStride = 2;
    CreatePipeline(false, state, kPlaceholderVertexShader);
}

// Test that we cannot set an already set attribute
TEST_F(VertexStateTest, AlreadySetAttribute) {
    // Control case, setting attribute 0
    utils::ComboVertexState state;
    state.vertexBufferCount = 1;
    state.cVertexBuffers[0].attributeCount = 1;
    state.cAttributes[0].shaderLocation = 0;
    CreatePipeline(true, state, kPlaceholderVertexShader);

    // Oh no, attribute 0 is set twice
    state.cVertexBuffers[0].attributeCount = 2;
    state.cAttributes[0].shaderLocation = 0;
    state.cAttributes[1].shaderLocation = 0;
    CreatePipeline(false, state, kPlaceholderVertexShader);
}

// Test that a arrayStride of 0 is valid
TEST_F(VertexStateTest, SetSameShaderLocation) {
    // Control case, setting different shader locations in two attributes
    utils::ComboVertexState state;
    state.vertexBufferCount = 1;
    state.cVertexBuffers[0].attributeCount = 2;
    state.cAttributes[0].shaderLocation = 0;
    state.cAttributes[1].shaderLocation = 1;
    state.cAttributes[1].offset = sizeof(float);
    CreatePipeline(true, state, kPlaceholderVertexShader);

    // Test same shader location in two attributes in the same buffer
    state.cAttributes[1].shaderLocation = 0;
    CreatePipeline(false, state, kPlaceholderVertexShader);

    // Test same shader location in two attributes in different buffers
    state.vertexBufferCount = 2;
    state.cVertexBuffers[0].attributeCount = 1;
    state.cAttributes[0].shaderLocation = 0;
    state.cVertexBuffers[1].attributeCount = 1;
    state.cVertexBuffers[1].attributes = &state.cAttributes[1];
    state.cAttributes[1].shaderLocation = 0;
    CreatePipeline(false, state, kPlaceholderVertexShader);
}

// Check out of bounds condition on attribute shader location
TEST_F(VertexStateTest, SetAttributeLocationOutOfBounds) {
    wgpu::SupportedLimits limits;
    device.GetLimits(&limits);
    uint32_t maxVertexAttributes = limits.limits.maxVertexAttributes;

    // Control case, setting last attribute shader location
    utils::ComboVertexState state;
    state.vertexBufferCount = 1;
    state.cVertexBuffers[0].attributeCount = 1;
    state.cAttributes[0].shaderLocation = maxVertexAttributes - 1;
    CreatePipeline(true, state, kPlaceholderVertexShader);

    // Test attribute location OOB
    state.cAttributes[0].shaderLocation = maxVertexAttributes;
    CreatePipeline(false, state, kPlaceholderVertexShader);
}

// Check attribute offset out of bounds
TEST_F(VertexStateTest, SetAttributeOffsetOutOfBounds) {
    // Control case, setting max attribute offset for FloatR32 vertex format
    utils::ComboVertexState state;
    state.vertexBufferCount = 1;
    state.cVertexBuffers[0].attributeCount = 1;
    state.cAttributes[0].offset = kMaxVertexBufferArrayStride - sizeof(wgpu::VertexFormat::Float32);
    CreatePipeline(true, state, kPlaceholderVertexShader);

    // Test attribute offset out of bounds
    state.cAttributes[0].offset = kMaxVertexBufferArrayStride - 1;
    CreatePipeline(false, state, kPlaceholderVertexShader);
}

// Check the min(4, formatSize) alignment constraint for the offset.
TEST_F(VertexStateTest, SetOffsetNotAligned) {
    // Control case, setting the offset at the correct alignments.
    utils::ComboVertexState state;
    state.vertexBufferCount = 1;
    state.cVertexBuffers[0].attributeCount = 1;

    // Test that for small formats, the offset must be aligned to the format size.
    state.cAttributes[0].format = wgpu::VertexFormat::Float32;
    state.cAttributes[0].offset = 4;
    CreatePipeline(true, state, kPlaceholderVertexShader);
    state.cAttributes[0].offset = 2;
    CreatePipeline(false, state, kPlaceholderVertexShader);

    state.cAttributes[0].format = wgpu::VertexFormat::Snorm16x2;
    state.cAttributes[0].offset = 4;
    CreatePipeline(true, state, kPlaceholderVertexShader);
    state.cAttributes[0].offset = 2;
    CreatePipeline(false, state, kPlaceholderVertexShader);

    state.cAttributes[0].format = wgpu::VertexFormat::Unorm8x2;
    state.cAttributes[0].offset = 2;
    CreatePipeline(true, state, kPlaceholderVertexShader);
    state.cAttributes[0].offset = 1;
    CreatePipeline(false, state, kPlaceholderVertexShader);

    // Test that for large formts the offset only needs to be aligned to 4.
    state.cAttributes[0].format = wgpu::VertexFormat::Snorm16x4;
    state.cAttributes[0].offset = 4;
    CreatePipeline(true, state, kPlaceholderVertexShader);

    state.cAttributes[0].format = wgpu::VertexFormat::Uint32x3;
    state.cAttributes[0].offset = 4;
    CreatePipeline(true, state, kPlaceholderVertexShader);

    state.cAttributes[0].format = wgpu::VertexFormat::Sint32x4;
    state.cAttributes[0].offset = 4;
    CreatePipeline(true, state, kPlaceholderVertexShader);
}

// Check attribute offset overflow
TEST_F(VertexStateTest, SetAttributeOffsetOverflow) {
    utils::ComboVertexState state;
    state.vertexBufferCount = 1;
    state.cVertexBuffers[0].attributeCount = 1;
    state.cAttributes[0].offset = std::numeric_limits<uint32_t>::max();
    CreatePipeline(false, state, kPlaceholderVertexShader);
}

// Check for some potential underflow in the vertex input validation
TEST_F(VertexStateTest, VertexFormatLargerThanNonZeroStride) {
    utils::ComboVertexState state;
    state.vertexBufferCount = 1;
    state.cVertexBuffers[0].arrayStride = 4;
    state.cVertexBuffers[0].attributeCount = 1;
    state.cAttributes[0].format = wgpu::VertexFormat::Float32x4;
    CreatePipeline(false, state, kPlaceholderVertexShader);
}

// Check that the vertex format base type must match the shader's variable base type.
TEST_F(VertexStateTest, BaseTypeMatching) {
    auto DoTest = [&](wgpu::VertexFormat format, std::string shaderType, bool success) {
        utils::ComboVertexState state;
        state.vertexBufferCount = 1;
        state.cVertexBuffers[0].arrayStride = 16;
        state.cVertexBuffers[0].attributeCount = 1;
        state.cAttributes[0].format = format;

        std::string shader = "@vertex fn main(@location(0) attrib : " + shaderType +
                             R"() -> @builtin(position) vec4f {
            return vec4f(0.0, 0.0, 0.0, 0.0);
        })";

        CreatePipeline(success, state, shader.c_str());
    };

    // Test that a float format is compatible only with f32 base type.
    DoTest(wgpu::VertexFormat::Float32, "f32", true);
    DoTest(wgpu::VertexFormat::Float32, "i32", false);
    DoTest(wgpu::VertexFormat::Float32, "u32", false);

    // Test that an unorm format is compatible only with f32.
    DoTest(wgpu::VertexFormat::Unorm16x2, "f32", true);
    DoTest(wgpu::VertexFormat::Unorm16x2, "i32", false);
    DoTest(wgpu::VertexFormat::Unorm16x2, "u32", false);

    // Test that unorm10-10-10-2 format is compatible only with f32.
    DoTest(wgpu::VertexFormat::Unorm10_10_10_2, "f32", true);
    DoTest(wgpu::VertexFormat::Unorm10_10_10_2, "i32", false);
    DoTest(wgpu::VertexFormat::Unorm10_10_10_2, "u32", false);

    // Test that unorm8x4-unorm format is compatible only with f32.
    DoTest(wgpu::VertexFormat::Unorm8x4BGRA, "f32", true);
    DoTest(wgpu::VertexFormat::Unorm8x4BGRA, "i32", false);
    DoTest(wgpu::VertexFormat::Unorm8x4BGRA, "u32", false);

    // Test that an snorm format is compatible only with f32.
    DoTest(wgpu::VertexFormat::Snorm16x4, "f32", true);
    DoTest(wgpu::VertexFormat::Snorm16x4, "i32", false);
    DoTest(wgpu::VertexFormat::Snorm16x4, "u32", false);

    // Test that an uint format is compatible only with u32.
    DoTest(wgpu::VertexFormat::Uint32x3, "f32", false);
    DoTest(wgpu::VertexFormat::Uint32x3, "i32", false);
    DoTest(wgpu::VertexFormat::Uint32x3, "u32", true);

    // Test that an sint format is compatible only with u32.
    DoTest(wgpu::VertexFormat::Sint8x4, "f32", false);
    DoTest(wgpu::VertexFormat::Sint8x4, "i32", true);
    DoTest(wgpu::VertexFormat::Sint8x4, "u32", false);

    // Test that formats are compatible with any width of vectors.
    DoTest(wgpu::VertexFormat::Float32, "f32", true);
    DoTest(wgpu::VertexFormat::Float32, "vec2f", true);
    DoTest(wgpu::VertexFormat::Float32, "vec3f", true);
    DoTest(wgpu::VertexFormat::Float32, "vec4f", true);

    DoTest(wgpu::VertexFormat::Float32x4, "f32", true);
    DoTest(wgpu::VertexFormat::Float32x4, "vec2f", true);
    DoTest(wgpu::VertexFormat::Float32x4, "vec3f", true);
    DoTest(wgpu::VertexFormat::Float32x4, "vec4f", true);
}

// Check that we only check base type compatibility for vertex inputs the shader uses.
TEST_F(VertexStateTest, BaseTypeMatchingForInexistentInput) {
    auto DoTest = [&](wgpu::VertexFormat format) {
        utils::ComboVertexState state;
        state.vertexBufferCount = 1;
        state.cVertexBuffers[0].arrayStride = 16;
        state.cVertexBuffers[0].attributeCount = 1;
        state.cAttributes[0].format = format;

        std::string shader = R"(@vertex fn main() -> @builtin(position) vec4f {
            return vec4f(0.0, 0.0, 0.0, 0.0);
        })";

        CreatePipeline(true, state, shader.c_str());
    };

    DoTest(wgpu::VertexFormat::Float32);
    DoTest(wgpu::VertexFormat::Unorm16x2);
    DoTest(wgpu::VertexFormat::Snorm16x4);
    DoTest(wgpu::VertexFormat::Uint8x4);
    DoTest(wgpu::VertexFormat::Sint32x2);
}

}  // anonymous namespace
}  // namespace dawn
