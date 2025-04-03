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

#include <numeric>
#include <string>
#include <vector>

#include "dawn/tests/DawnTest.h"
#include "dawn/utils/ComboRenderPipelineDescriptor.h"
#include "dawn/utils/WGPUHelpers.h"

namespace dawn {
namespace {

class ShaderTests : public DawnTest {
  protected:
    wgpu::Limits GetRequiredLimits(const wgpu::Limits& supported) override {
        // Just copy all the limits, though all we really care about is
        // maxStorageBuffersInFragmentStage
        // maxStorageTexturesInFragmentStage
        // maxStorageBuffersInVertexStage
        // maxStorageTexturesInVertexStage
        return supported;
    }

  public:
    wgpu::Buffer CreateBuffer(const std::vector<uint32_t>& data,
                              wgpu::BufferUsage usage = wgpu::BufferUsage::Storage |
                                                        wgpu::BufferUsage::CopySrc) {
        uint64_t bufferSize = static_cast<uint64_t>(data.size() * sizeof(uint32_t));
        return utils::CreateBufferFromData(device, data.data(), bufferSize, usage);
    }

    wgpu::Buffer CreateBuffer(const uint32_t count,
                              wgpu::BufferUsage usage = wgpu::BufferUsage::Storage |
                                                        wgpu::BufferUsage::CopySrc) {
        return CreateBuffer(std::vector<uint32_t>(count, 0), usage);
    }

    wgpu::ComputePipeline CreateComputePipeline(
        const std::string& shader,
        const char* entryPoint = nullptr,
        const std::vector<wgpu::ConstantEntry>* constants = nullptr) {
        wgpu::ComputePipelineDescriptor csDesc;
        csDesc.compute.module = utils::CreateShaderModule(device, shader.c_str());
        csDesc.compute.entryPoint = entryPoint;
        if (constants) {
            csDesc.compute.constants = constants->data();
            csDesc.compute.constantCount = constants->size();
        }
        return device.CreateComputePipeline(&csDesc);
    }
};

// Test that log2 is being properly calculated, base on crbug.com/1046622
TEST_P(ShaderTests, ComputeLog2) {
    uint32_t const kSteps = 19;
    std::vector<uint32_t> expected{0, 1, 1, 2, 2, 3, 3, 4, 4, 5, 5, 6, 6, 7, 7, 8, 8, 9, 32};
    wgpu::Buffer buffer = CreateBuffer(kSteps);

    std::string shader = R"(
struct Buf {
    data : array<u32, 19>
}

@group(0) @binding(0) var<storage, read_write> buf : Buf;

@compute @workgroup_size(1) fn main() {
    let factor : f32 = 1.0001;

    buf.data[0] = u32(log2(1.0 * factor));
    buf.data[1] = u32(log2(2.0 * factor));
    buf.data[2] = u32(log2(3.0 * factor));
    buf.data[3] = u32(log2(4.0 * factor));
    buf.data[4] = u32(log2(7.0 * factor));
    buf.data[5] = u32(log2(8.0 * factor));
    buf.data[6] = u32(log2(15.0 * factor));
    buf.data[7] = u32(log2(16.0 * factor));
    buf.data[8] = u32(log2(31.0 * factor));
    buf.data[9] = u32(log2(32.0 * factor));
    buf.data[10] = u32(log2(63.0 * factor));
    buf.data[11] = u32(log2(64.0 * factor));
    buf.data[12] = u32(log2(127.0 * factor));
    buf.data[13] = u32(log2(128.0 * factor));
    buf.data[14] = u32(log2(255.0 * factor));
    buf.data[15] = u32(log2(256.0 * factor));
    buf.data[16] = u32(log2(511.0 * factor));
    buf.data[17] = u32(log2(512.0 * factor));
    buf.data[18] = u32(log2(4294967295.0 * factor));
})";

    wgpu::ComputePipeline pipeline = CreateComputePipeline(shader, "main");

    wgpu::BindGroup bindGroup =
        utils::MakeBindGroup(device, pipeline.GetBindGroupLayout(0), {{0, buffer}});

    wgpu::CommandBuffer commands;
    {
        wgpu::CommandEncoder encoder = device.CreateCommandEncoder();
        wgpu::ComputePassEncoder pass = encoder.BeginComputePass();
        pass.SetPipeline(pipeline);
        pass.SetBindGroup(0, bindGroup);
        pass.DispatchWorkgroups(1);
        pass.End();

        commands = encoder.Finish();
    }

    queue.Submit(1, &commands);

    EXPECT_BUFFER_U32_RANGE_EQ(expected.data(), buffer, 0, kSteps);
}

TEST_P(ShaderTests, BadWGSL) {
    DAWN_TEST_UNSUPPORTED_IF(HasToggleEnabled("skip_validation"));

    std::string shader = R"(
I am an invalid shader and should never pass validation!
})";
    ASSERT_DEVICE_ERROR(utils::CreateShaderModule(device, shader.c_str()));
}

// Tests that shaders using non-struct function parameters and return values for shader stage I/O
// can compile and link successfully.
TEST_P(ShaderTests, WGSLParamIO) {
    std::string vertexShader = R"(
@vertex
fn main(@builtin(vertex_index) VertexIndex : u32) -> @builtin(position) vec4f {
    var pos = array(
        vec2f(-1.0,  1.0),
        vec2f( 1.0,  1.0),
        vec2f( 0.0, -1.0));
    return vec4f(pos[VertexIndex], 0.0, 1.0);
})";
    wgpu::ShaderModule vsModule = utils::CreateShaderModule(device, vertexShader.c_str());

    std::string fragmentShader = R"(
@fragment
fn main(@builtin(position) fragCoord : vec4f) -> @location(0) vec4f {
    return vec4f(fragCoord.xy, 0.0, 1.0);
})";
    wgpu::ShaderModule fsModule = utils::CreateShaderModule(device, fragmentShader.c_str());

    utils::ComboRenderPipelineDescriptor rpDesc;
    rpDesc.vertex.module = vsModule;
    rpDesc.cFragment.module = fsModule;
    wgpu::RenderPipeline pipeline = device.CreateRenderPipeline(&rpDesc);
}

// Tests that a vertex shader using struct function parameters and return values for shader stage
// I/O can compile and link successfully against a fragement shader using compatible non-struct I/O.
TEST_P(ShaderTests, WGSLMixedStructParamIO) {
    std::string vertexShader = R"(
struct VertexIn {
    @location(0) position : vec3f,
    @location(1) color : vec4f,
}

struct VertexOut {
    @location(0) color : vec4f,
    @builtin(position) position : vec4f,
}

@vertex
fn main(input : VertexIn) -> VertexOut {
    var output : VertexOut;
    output.position = vec4f(input.position, 1.0);
    output.color = input.color;
    return output;
})";
    wgpu::ShaderModule vsModule = utils::CreateShaderModule(device, vertexShader.c_str());

    std::string fragmentShader = R"(
@fragment
fn main(@location(0) color : vec4f) -> @location(0) vec4f {
    return color;
})";
    wgpu::ShaderModule fsModule = utils::CreateShaderModule(device, fragmentShader.c_str());

    utils::ComboRenderPipelineDescriptor rpDesc;
    rpDesc.vertex.module = vsModule;
    rpDesc.cFragment.module = fsModule;
    rpDesc.vertex.bufferCount = 1;
    rpDesc.cBuffers[0].attributeCount = 2;
    rpDesc.cBuffers[0].arrayStride = 28;
    rpDesc.cAttributes[0].shaderLocation = 0;
    rpDesc.cAttributes[0].format = wgpu::VertexFormat::Float32x3;
    rpDesc.cAttributes[1].shaderLocation = 1;
    rpDesc.cAttributes[1].format = wgpu::VertexFormat::Float32x4;
    wgpu::RenderPipeline pipeline = device.CreateRenderPipeline(&rpDesc);
}

// Tests that shaders using struct function parameters and return values for shader stage I/O
// can compile and link successfully.
TEST_P(ShaderTests, WGSLStructIO) {
    std::string vertexShader = R"(
struct VertexIn {
    @location(0) position : vec3f,
    @location(1) color : vec4f,
}

struct VertexOut {
    @location(0) color : vec4f,
    @builtin(position) position : vec4f,
}

@vertex
fn main(input : VertexIn) -> VertexOut {
    var output : VertexOut;
    output.position = vec4f(input.position, 1.0);
    output.color = input.color;
    return output;
})";
    wgpu::ShaderModule vsModule = utils::CreateShaderModule(device, vertexShader.c_str());

    std::string fragmentShader = R"(
struct FragmentIn {
    @location(0) color : vec4f,
    @builtin(position) fragCoord : vec4f,
}

@fragment
fn main(input : FragmentIn) -> @location(0) vec4f {
    return input.color * input.fragCoord;
})";
    wgpu::ShaderModule fsModule = utils::CreateShaderModule(device, fragmentShader.c_str());

    utils::ComboRenderPipelineDescriptor rpDesc;
    rpDesc.vertex.module = vsModule;
    rpDesc.cFragment.module = fsModule;
    rpDesc.vertex.bufferCount = 1;
    rpDesc.cBuffers[0].attributeCount = 2;
    rpDesc.cBuffers[0].arrayStride = 28;
    rpDesc.cAttributes[0].shaderLocation = 0;
    rpDesc.cAttributes[0].format = wgpu::VertexFormat::Float32x3;
    rpDesc.cAttributes[1].shaderLocation = 1;
    rpDesc.cAttributes[1].format = wgpu::VertexFormat::Float32x4;
    wgpu::RenderPipeline pipeline = device.CreateRenderPipeline(&rpDesc);
}

// Tests that shaders I/O structs that us compatible locations but are not sorted by hand can link.
TEST_P(ShaderTests, WGSLUnsortedStructIO) {
    std::string vertexShader = R"(
struct VertexIn {
    @location(0) position : vec3f,
    @location(1) color : vec4f,
}

struct VertexOut {
    @builtin(position) position : vec4f,
    @location(0) color : vec4f,
}

@vertex
fn main(input : VertexIn) -> VertexOut {
    var output : VertexOut;
    output.position = vec4f(input.position, 1.0);
    output.color = input.color;
    return output;
})";
    wgpu::ShaderModule vsModule = utils::CreateShaderModule(device, vertexShader.c_str());

    std::string fragmentShader = R"(
struct FragmentIn {
    @location(0) color : vec4f,
    @builtin(position) fragCoord : vec4f,
}

@fragment
fn main(input : FragmentIn) -> @location(0) vec4f {
    return input.color * input.fragCoord;
})";
    wgpu::ShaderModule fsModule = utils::CreateShaderModule(device, fragmentShader.c_str());

    utils::ComboRenderPipelineDescriptor rpDesc;
    rpDesc.vertex.module = vsModule;
    rpDesc.cFragment.module = fsModule;
    rpDesc.vertex.bufferCount = 1;
    rpDesc.cBuffers[0].attributeCount = 2;
    rpDesc.cBuffers[0].arrayStride = 28;
    rpDesc.cAttributes[0].shaderLocation = 0;
    rpDesc.cAttributes[0].format = wgpu::VertexFormat::Float32x3;
    rpDesc.cAttributes[1].shaderLocation = 1;
    rpDesc.cAttributes[1].format = wgpu::VertexFormat::Float32x4;
    wgpu::RenderPipeline pipeline = device.CreateRenderPipeline(&rpDesc);
}

// Tests that shaders I/O structs can be shared between vertex and fragment shaders.
TEST_P(ShaderTests, WGSLSharedStructIO) {
    std::string shader = R"(
struct VertexIn {
    @location(0) position : vec3f,
    @location(1) color : vec4f,
}

struct VertexOut {
    @location(0) color : vec4f,
    @builtin(position) position : vec4f,
}

@vertex
fn vertexMain(input : VertexIn) -> VertexOut {
    var output : VertexOut;
    output.position = vec4f(input.position, 1.0);
    output.color = input.color;
    return output;
}

@fragment
fn fragmentMain(input : VertexOut) -> @location(0) vec4f {
    return input.color;
})";
    wgpu::ShaderModule shaderModule = utils::CreateShaderModule(device, shader.c_str());

    utils::ComboRenderPipelineDescriptor rpDesc;
    rpDesc.vertex.module = shaderModule;
    rpDesc.cFragment.module = shaderModule;
    rpDesc.vertex.bufferCount = 1;
    rpDesc.cBuffers[0].attributeCount = 2;
    rpDesc.cBuffers[0].arrayStride = 28;
    rpDesc.cAttributes[0].shaderLocation = 0;
    rpDesc.cAttributes[0].format = wgpu::VertexFormat::Float32x3;
    rpDesc.cAttributes[1].shaderLocation = 1;
    rpDesc.cAttributes[1].format = wgpu::VertexFormat::Float32x4;
    wgpu::RenderPipeline pipeline = device.CreateRenderPipeline(&rpDesc);
}

// Tests that sparse input output locations should work properly.
// This test is not in dawn_unittests/RenderPipelineValidationTests because we want to test the
// compilation of the pipeline in D3D12 backend.
TEST_P(ShaderTests, WGSLInterstageVariablesSparse) {
    std::string shader = R"(
struct ShaderIO {
    @builtin(position) position : vec4f,
    @location(1) attribute1 : vec4f,
    @location(3) attribute3 : vec4f,
}

@vertex
fn vertexMain() -> ShaderIO {
    var output : ShaderIO;
    output.position = vec4f(0.0, 0.0, 0.0, 1.0);
    output.attribute1 = vec4f(0.0, 0.0, 0.0, 1.0);
    output.attribute3 = vec4f(0.0, 0.0, 0.0, 1.0);
    return output;
}

@fragment
fn fragmentMain(input : ShaderIO) -> @location(0) vec4f {
    return input.attribute1;
})";
    wgpu::ShaderModule shaderModule = utils::CreateShaderModule(device, shader.c_str());

    utils::ComboRenderPipelineDescriptor rpDesc;
    rpDesc.vertex.module = shaderModule;
    rpDesc.cFragment.module = shaderModule;
    wgpu::RenderPipeline pipeline = device.CreateRenderPipeline(&rpDesc);
}

// Tests that interstage built-in inputs and outputs usage mismatch don't mess up with input-output
// locations.
// This test is not in dawn_unittests/RenderPipelineValidationTests because we want to test the
// compilation of the pipeline in D3D12 backend.
TEST_P(ShaderTests, WGSLInterstageVariablesBuiltinsMismatched) {
    std::string shader = R"(
struct VertexOut {
    @builtin(position) position : vec4f,
    @location(1) attribute1 : f32,
    @location(3) attribute3 : vec4f,
}

struct FragmentIn {
    @location(3) attribute3 : vec4f,
    @builtin(front_facing) front_facing : bool,
    @location(1) attribute1 : f32,
    @builtin(position) position : vec4f,
}

@vertex
fn vertexMain() -> VertexOut {
    var output : VertexOut;
    output.position = vec4f(0.0, 0.0, 0.0, 1.0);
    output.attribute1 = 1.0;
    output.attribute3 = vec4f(0.0, 0.0, 0.0, 1.0);
    return output;
}

@fragment
fn fragmentMain(input : FragmentIn) -> @location(0) vec4f {
    _ = input.front_facing;
    _ = input.position.x;
    return input.attribute3;
})";
    wgpu::ShaderModule shaderModule = utils::CreateShaderModule(device, shader.c_str());

    utils::ComboRenderPipelineDescriptor rpDesc;
    rpDesc.vertex.module = shaderModule;
    rpDesc.cFragment.module = shaderModule;
    wgpu::RenderPipeline pipeline = device.CreateRenderPipeline(&rpDesc);
}

// Tests that interstage inputs could be a prefix subset of the outputs.
// This test is not in dawn_unittests/RenderPipelineValidationTests because we want to test the
// compilation of the pipeline in D3D12 backend.
TEST_P(ShaderTests, WGSLInterstageVariablesPrefixSubset) {
    std::string shader = R"(
struct VertexOut {
    @builtin(position) position : vec4f,
    @location(1) attribute1 : f32,
    @location(3) attribute3 : vec4f,
}

struct FragmentIn {
    @location(1) attribute1 : f32,
    @builtin(position) position : vec4f,
}

@vertex
fn vertexMain() -> VertexOut {
    var output : VertexOut;
    output.position = vec4f(0.0, 0.0, 0.0, 1.0);
    output.attribute1 = 1.0;
    output.attribute3 = vec4f(0.0, 0.0, 0.0, 1.0);
    return output;
}

@fragment
fn fragmentMain(input : FragmentIn) -> @location(0) vec4f {
    _ = input.position.x;
    return vec4f(input.attribute1, 0.0, 0.0, 1.0);
})";
    wgpu::ShaderModule shaderModule = utils::CreateShaderModule(device, shader.c_str());

    utils::ComboRenderPipelineDescriptor rpDesc;
    rpDesc.vertex.module = shaderModule;
    rpDesc.cFragment.module = shaderModule;
    wgpu::RenderPipeline pipeline = device.CreateRenderPipeline(&rpDesc);
}

// Tests that interstage inputs could be a sparse non-prefix subset of the outputs.
// This test is not in dawn_unittests/RenderPipelineValidationTests because we want to test the
// compilation of the pipeline in D3D12 backend.
TEST_P(ShaderTests, WGSLInterstageVariablesSparseSubset) {
    std::string shader = R"(
struct VertexOut {
    @builtin(position) position : vec4f,
    @location(1) attribute1 : f32,
    @location(3) attribute3 : vec4f,
}

struct FragmentIn {
    @location(3) attribute3 : vec4f,
    @builtin(position) position : vec4f,
}

@vertex
fn vertexMain() -> VertexOut {
    var output : VertexOut;
    output.position = vec4f(0.0, 0.0, 0.0, 1.0);
    output.attribute1 = 1.0;
    output.attribute3 = vec4f(0.0, 0.0, 0.0, 1.0);
    return output;
}

@fragment
fn fragmentMain(input : FragmentIn) -> @location(0) vec4f {
    _ = input.position.x;
    return input.attribute3;
})";
    wgpu::ShaderModule shaderModule = utils::CreateShaderModule(device, shader.c_str());

    utils::ComboRenderPipelineDescriptor rpDesc;
    rpDesc.vertex.module = shaderModule;
    rpDesc.cFragment.module = shaderModule;
    wgpu::RenderPipeline pipeline = device.CreateRenderPipeline(&rpDesc);
}

// Tests that interstage inputs could be a sparse non-prefix subset of the outputs, and that
// fragment inputs are unused. This test is not in dawn_unittests/RenderPipelineValidationTests
// because we want to test the compilation of the pipeline in D3D12 backend.
TEST_P(ShaderTests, WGSLInterstageVariablesSparseSubsetUnused) {
    std::string shader = R"(
struct VertexOut {
    @builtin(position) position : vec4f,
    @location(1) attribute1 : f32,
    @location(3) attribute3 : vec4f,
}

struct FragmentIn {
    @location(3) attribute3 : vec4f,
    @builtin(position) position : vec4f,
}

@vertex
fn vertexMain() -> VertexOut {
    var output : VertexOut;
    output.position = vec4f(0.0, 0.0, 0.0, 1.0);
    output.attribute1 = 1.0;
    output.attribute3 = vec4f(0.0, 0.0, 0.0, 1.0);
    return output;
}

@fragment
fn fragmentMain(input : FragmentIn) -> @location(0) vec4f {
    return vec4f(0.0, 0.0, 0.0, 1.0);
})";
    wgpu::ShaderModule shaderModule = utils::CreateShaderModule(device, shader.c_str());

    utils::ComboRenderPipelineDescriptor rpDesc;
    rpDesc.vertex.module = shaderModule;
    rpDesc.cFragment.module = shaderModule;
    wgpu::RenderPipeline pipeline = device.CreateRenderPipeline(&rpDesc);
}

// Tests that interstage inputs could be empty when outputs are not.
// This test is not in dawn_unittests/RenderPipelineValidationTests because we want to test the
// compilation of the pipeline in D3D12 backend.
TEST_P(ShaderTests, WGSLInterstageVariablesEmptySubset) {
    std::string shader = R"(
struct VertexOut {
    @builtin(position) position : vec4f,
    @location(1) attribute1 : f32,
    @location(3) attribute3 : vec4f,
}

@vertex
fn vertexMain() -> VertexOut {
    var output : VertexOut;
    output.position = vec4f(0.0, 0.0, 0.0, 1.0);
    output.attribute1 = 1.0;
    output.attribute3 = vec4f(0.0, 0.0, 0.0, 1.0);
    return output;
}

@fragment
fn fragmentMain() -> @location(0) vec4f {
    return vec4f(0.0, 0.0, 0.0, 1.0);
})";
    wgpu::ShaderModule shaderModule = utils::CreateShaderModule(device, shader.c_str());

    utils::ComboRenderPipelineDescriptor rpDesc;
    rpDesc.vertex.module = shaderModule;
    rpDesc.cFragment.module = shaderModule;
    wgpu::RenderPipeline pipeline = device.CreateRenderPipeline(&rpDesc);
}

// Regression test for crbug.com/dawn/1733. Even when user defined attribute input is empty,
// Builtin input for the next stage could still cause register mismatch issue on D3D12 HLSL
// compiler. So the TruncateInterstageVariables transform should still be run. This test is not in
// dawn_unittests/RenderPipelineValidationTests because we want to test the compilation of the
// pipeline in D3D12 backend.
TEST_P(ShaderTests, WGSLInterstageVariablesEmptyUserAttributeSubset) {
    std::string shader = R"(
struct VertexOut {
    @builtin(position) position : vec4f,
    @location(1) attribute1 : f32,
    @location(3) attribute3 : vec4f,
}

@vertex
fn vertexMain() -> VertexOut {
    var output : VertexOut;
    output.position = vec4f(0.0, 0.0, 0.0, 1.0);
    output.attribute1 = 1.0;
    output.attribute3 = vec4f(0.0, 0.0, 0.0, 1.0);
    return output;
}

@fragment
fn fragmentMain(@builtin(position) position : vec4<f32>) -> @location(0) vec4f {
    return vec4f(0.0, 0.0, 0.0, 1.0);
})";
    wgpu::ShaderModule shaderModule = utils::CreateShaderModule(device, shader.c_str());

    utils::ComboRenderPipelineDescriptor rpDesc;
    rpDesc.vertex.module = shaderModule;
    rpDesc.cFragment.module = shaderModule;
    wgpu::RenderPipeline pipeline = device.CreateRenderPipeline(&rpDesc);
}

// This is a regression test for an issue caused by the FirstIndexOffset transfrom being done before
// the BindingRemapper, causing an intermediate AST to be invalid (and fail the overall
// compilation).
TEST_P(ShaderTests, FirstIndexOffsetRegisterConflictInHLSLTransforms) {
    const char* shader = R"(
// Dumped WGSL:

struct Inputs {
  @location(1) attrib1 : u32,
  // The extra register added to handle base_vertex for vertex_index conflicts with [1]
  @builtin(vertex_index) vertexIndex: u32,
}

// [1] a binding point that conflicts with the regitster
struct S1 { data : array<vec4u, 20> }
@group(0) @binding(1) var<uniform> providedData1 : S1;

@vertex fn vsMain(input : Inputs) -> @builtin(position) vec4f {
  _ = providedData1.data[input.vertexIndex][0];
  return vec4f();
}

@fragment fn fsMain() -> @location(0) vec4f {
  return vec4f();
}
    )";
    auto module = utils::CreateShaderModule(device, shader);

    utils::ComboRenderPipelineDescriptor rpDesc;
    rpDesc.vertex.module = module;
    rpDesc.cFragment.module = module;
    rpDesc.vertex.bufferCount = 1;
    rpDesc.cBuffers[0].attributeCount = 1;
    rpDesc.cBuffers[0].arrayStride = 16;
    rpDesc.cAttributes[0].shaderLocation = 1;
    rpDesc.cAttributes[0].format = wgpu::VertexFormat::Uint8x2;
    device.CreateRenderPipeline(&rpDesc);
}

// Test that WGSL built-in variable @sample_index can be used in fragment shaders.
TEST_P(ShaderTests, SampleIndex) {
    // TODO(crbug.com/dawn/673): Work around or enforce via validation that sample variables are not
    // supported on some platforms.
    DAWN_TEST_UNSUPPORTED_IF(HasToggleEnabled("disable_sample_variables"));

    // Compat mode does not support sample_index
    DAWN_TEST_UNSUPPORTED_IF(IsCompatibilityMode());

    wgpu::ShaderModule vsModule = utils::CreateShaderModule(device, R"(
@vertex
fn main(@location(0) pos : vec4f) -> @builtin(position) vec4f {
    return pos;
})");

    wgpu::ShaderModule fsModule = utils::CreateShaderModule(device, R"(
@fragment fn main(@builtin(sample_index) sampleIndex : u32)
    -> @location(0) vec4f {
    return vec4f(f32(sampleIndex), 1.0, 0.0, 1.0);
})");

    utils::ComboRenderPipelineDescriptor descriptor;
    descriptor.vertex.module = vsModule;
    descriptor.cFragment.module = fsModule;
    descriptor.primitive.topology = wgpu::PrimitiveTopology::TriangleList;
    descriptor.vertex.bufferCount = 1;
    descriptor.cBuffers[0].arrayStride = 4 * sizeof(float);
    descriptor.cBuffers[0].attributeCount = 1;
    descriptor.cAttributes[0].format = wgpu::VertexFormat::Float32x4;
    descriptor.cTargets[0].format = wgpu::TextureFormat::RGBA8Unorm;

    device.CreateRenderPipeline(&descriptor);
}

// Test overridable constants without numeric identifiers
TEST_P(ShaderTests, OverridableConstants) {
    uint32_t const kCount = 11;
    std::vector<uint32_t> expected(kCount);
    std::iota(expected.begin(), expected.end(), 0);
    wgpu::Buffer buffer = CreateBuffer(kCount);

    std::string shader = R"(
override c0: bool;              // type: bool
override c1: bool = false;      // default override
override c2: f32;               // type: float32
override c3: f32 = 0.0;         // default override
override c4: f32 = 4.0;         // default
override c5: i32;               // type: int32
override c6: i32 = 0;           // default override
override c7: i32 = 7;           // default
override c8: u32;               // type: uint32
override c9: u32 = 0u;          // default override
override c10: u32 = 10u;        // default

struct Buf {
    data : array<u32, 11>
}

@group(0) @binding(0) var<storage, read_write> buf : Buf;

@compute @workgroup_size(1) fn main() {
    buf.data[0] = u32(c0);
    buf.data[1] = u32(c1);
    buf.data[2] = u32(c2);
    buf.data[3] = u32(c3);
    buf.data[4] = u32(c4);
    buf.data[5] = u32(c5);
    buf.data[6] = u32(c6);
    buf.data[7] = u32(c7);
    buf.data[8] = u32(c8);
    buf.data[9] = u32(c9);
    buf.data[10] = u32(c10);
})";

    std::vector<wgpu::ConstantEntry> constants;
    constants.push_back({nullptr, "c0", 0});
    constants.push_back({nullptr, "c1", 1});
    constants.push_back({nullptr, "c2", 2});
    constants.push_back({nullptr, "c3", 3});
    // c4 is not assigned, testing default value
    constants.push_back({nullptr, "c5", 5});
    constants.push_back({nullptr, "c6", 6});
    // c7 is not assigned, testing default value
    constants.push_back({nullptr, "c8", 8});
    constants.push_back({nullptr, "c9", 9});
    // c10 is not assigned, testing default value

    wgpu::ComputePipeline pipeline = CreateComputePipeline(shader, "main", &constants);

    wgpu::BindGroup bindGroup =
        utils::MakeBindGroup(device, pipeline.GetBindGroupLayout(0), {{0, buffer}});

    wgpu::CommandBuffer commands;
    {
        wgpu::CommandEncoder encoder = device.CreateCommandEncoder();
        wgpu::ComputePassEncoder pass = encoder.BeginComputePass();
        pass.SetPipeline(pipeline);
        pass.SetBindGroup(0, bindGroup);
        pass.DispatchWorkgroups(1);
        pass.End();

        commands = encoder.Finish();
    }

    queue.Submit(1, &commands);

    EXPECT_BUFFER_U32_RANGE_EQ(expected.data(), buffer, 0, kCount);
}

// Test one shader shared by two pipelines with different constants overridden
TEST_P(ShaderTests, OverridableConstantsSharedShader) {
    std::vector<uint32_t> expected1{1};
    wgpu::Buffer buffer1 = CreateBuffer(expected1.size());
    std::vector<uint32_t> expected2{2};
    wgpu::Buffer buffer2 = CreateBuffer(expected2.size());

    std::string shader = R"(
override a: u32;

struct Buf {
    data : array<u32, 1>
}

@group(0) @binding(0) var<storage, read_write> buf : Buf;

@compute @workgroup_size(1) fn main() {
    buf.data[0] = a;
})";

    std::vector<wgpu::ConstantEntry> constants1;
    constants1.push_back({nullptr, "a", 1});
    std::vector<wgpu::ConstantEntry> constants2;
    constants2.push_back({nullptr, "a", 2});

    wgpu::ComputePipeline pipeline1 = CreateComputePipeline(shader, "main", &constants1);
    wgpu::ComputePipeline pipeline2 = CreateComputePipeline(shader, "main", &constants2);

    wgpu::BindGroup bindGroup1 =
        utils::MakeBindGroup(device, pipeline1.GetBindGroupLayout(0), {{0, buffer1}});
    wgpu::BindGroup bindGroup2 =
        utils::MakeBindGroup(device, pipeline2.GetBindGroupLayout(0), {{0, buffer2}});

    wgpu::CommandBuffer commands;
    {
        wgpu::CommandEncoder encoder = device.CreateCommandEncoder();
        wgpu::ComputePassEncoder pass = encoder.BeginComputePass();
        pass.SetPipeline(pipeline1);
        pass.SetBindGroup(0, bindGroup1);
        pass.DispatchWorkgroups(1);
        pass.SetPipeline(pipeline2);
        pass.SetBindGroup(0, bindGroup2);
        pass.DispatchWorkgroups(1);
        pass.End();

        commands = encoder.Finish();
    }

    queue.Submit(1, &commands);

    EXPECT_BUFFER_U32_RANGE_EQ(expected1.data(), buffer1, 0, expected1.size());
    EXPECT_BUFFER_U32_RANGE_EQ(expected2.data(), buffer2, 0, expected2.size());
}

// Test overridable constants work with workgroup size
TEST_P(ShaderTests, OverridableConstantsWorkgroupSize) {
    std::string shader = R"(
override x: u32;

struct Buf {
    data : array<u32, 1>
}

@group(0) @binding(0) var<storage, read_write> buf : Buf;

@compute @workgroup_size(x) fn main(
    @builtin(local_invocation_id) local_invocation_id : vec3u
) {
    if (local_invocation_id.x >= x - 1) {
        buf.data[0] = local_invocation_id.x + 1;
    }
})";

    const uint32_t workgroup_size_x_1 = 16u;
    const uint32_t workgroup_size_x_2 = 64u;

    std::vector<uint32_t> expected1{workgroup_size_x_1};
    wgpu::Buffer buffer1 = CreateBuffer(expected1.size());
    std::vector<uint32_t> expected2{workgroup_size_x_2};
    wgpu::Buffer buffer2 = CreateBuffer(expected2.size());

    std::vector<wgpu::ConstantEntry> constants1;
    constants1.push_back({nullptr, "x", static_cast<double>(workgroup_size_x_1)});
    std::vector<wgpu::ConstantEntry> constants2;
    constants2.push_back({nullptr, "x", static_cast<double>(workgroup_size_x_2)});

    wgpu::ComputePipeline pipeline1 = CreateComputePipeline(shader, "main", &constants1);
    wgpu::ComputePipeline pipeline2 = CreateComputePipeline(shader, "main", &constants2);

    wgpu::BindGroup bindGroup1 =
        utils::MakeBindGroup(device, pipeline1.GetBindGroupLayout(0), {{0, buffer1}});
    wgpu::BindGroup bindGroup2 =
        utils::MakeBindGroup(device, pipeline2.GetBindGroupLayout(0), {{0, buffer2}});

    wgpu::CommandBuffer commands;
    {
        wgpu::CommandEncoder encoder = device.CreateCommandEncoder();
        wgpu::ComputePassEncoder pass = encoder.BeginComputePass();
        pass.SetPipeline(pipeline1);
        pass.SetBindGroup(0, bindGroup1);
        pass.DispatchWorkgroups(1);
        pass.SetPipeline(pipeline2);
        pass.SetBindGroup(0, bindGroup2);
        pass.DispatchWorkgroups(1);
        pass.End();

        commands = encoder.Finish();
    }

    queue.Submit(1, &commands);

    EXPECT_BUFFER_U32_RANGE_EQ(expected1.data(), buffer1, 0, expected1.size());
    EXPECT_BUFFER_U32_RANGE_EQ(expected2.data(), buffer2, 0, expected2.size());
}

// Test overridable constants with numeric identifiers
TEST_P(ShaderTests, OverridableConstantsNumericIdentifiers) {
    uint32_t const kCount = 4;
    std::vector<uint32_t> expected{1u, 2u, 3u, 0u};
    wgpu::Buffer buffer = CreateBuffer(kCount);

    std::string shader = R"(
@id(1001) override c1: u32;            // some big numeric id
@id(1) override c2: u32 = 0u;          // id == 1 might collide with some generated constant id
@id(1003) override c3: u32 = 3u;       // default
@id(1004) override c4: u32;            // default unspecified

struct Buf {
    data : array<u32, 4>
}

@group(0) @binding(0) var<storage, read_write> buf : Buf;

@compute @workgroup_size(1) fn main() {
    buf.data[0] = c1;
    buf.data[1] = c2;
    buf.data[2] = c3;
    buf.data[3] = c4;
})";

    std::vector<wgpu::ConstantEntry> constants;
    constants.push_back({nullptr, "1001", 1});
    constants.push_back({nullptr, "1", 2});
    // c3 is not assigned, testing default value
    constants.push_back({nullptr, "1004", 0});

    wgpu::ComputePipeline pipeline = CreateComputePipeline(shader, "main", &constants);

    wgpu::BindGroup bindGroup =
        utils::MakeBindGroup(device, pipeline.GetBindGroupLayout(0), {{0, buffer}});

    wgpu::CommandBuffer commands;
    {
        wgpu::CommandEncoder encoder = device.CreateCommandEncoder();
        wgpu::ComputePassEncoder pass = encoder.BeginComputePass();
        pass.SetPipeline(pipeline);
        pass.SetBindGroup(0, bindGroup);
        pass.DispatchWorkgroups(1);
        pass.End();

        commands = encoder.Finish();
    }

    queue.Submit(1, &commands);

    EXPECT_BUFFER_U32_RANGE_EQ(expected.data(), buffer, 0, kCount);
}

// Test overridable constants precision
// D3D12 HLSL shader uses defines so we want float number to have enough precision
TEST_P(ShaderTests, OverridableConstantsPrecision) {
    uint32_t const kCount = 2;
    float const kValue1 = 3.14159;
    float const kValue2 = 3.141592653589793238;
    std::vector<float> expected{kValue1, kValue2};
    wgpu::Buffer buffer = CreateBuffer(kCount);

    std::string shader = R"(
@id(1001) override c1: f32;
@id(1002) override c2: f32;

struct Buf {
    data : array<f32, 2>
}

@group(0) @binding(0) var<storage, read_write> buf : Buf;

@compute @workgroup_size(1) fn main() {
    buf.data[0] = c1;
    buf.data[1] = c2;
})";

    std::vector<wgpu::ConstantEntry> constants;
    constants.push_back({nullptr, "1001", kValue1});
    constants.push_back({nullptr, "1002", kValue2});
    wgpu::ComputePipeline pipeline = CreateComputePipeline(shader, "main", &constants);

    wgpu::BindGroup bindGroup =
        utils::MakeBindGroup(device, pipeline.GetBindGroupLayout(0), {{0, buffer}});

    wgpu::CommandBuffer commands;
    {
        wgpu::CommandEncoder encoder = device.CreateCommandEncoder();
        wgpu::ComputePassEncoder pass = encoder.BeginComputePass();
        pass.SetPipeline(pipeline);
        pass.SetBindGroup(0, bindGroup);
        pass.DispatchWorkgroups(1);
        pass.End();

        commands = encoder.Finish();
    }

    queue.Submit(1, &commands);

    EXPECT_BUFFER_FLOAT_RANGE_EQ(expected.data(), buffer, 0, kCount);
}

// Test overridable constants for different entry points
TEST_P(ShaderTests, OverridableConstantsMultipleEntryPoints) {
    uint32_t const kCount = 1;
    std::vector<uint32_t> expected1{1u};
    std::vector<uint32_t> expected2{2u};
    std::vector<uint32_t> expected3{3u};

    wgpu::Buffer buffer1 = CreateBuffer(kCount);
    wgpu::Buffer buffer2 = CreateBuffer(kCount);
    wgpu::Buffer buffer3 = CreateBuffer(kCount);

    std::string shader = R"(
@id(1001) override c1: u32;
@id(1002) override c2: u32;
@id(1003) override c3: u32;

struct Buf {
    data : array<u32, 1>
}

@group(0) @binding(0) var<storage, read_write> buf : Buf;

@compute @workgroup_size(1) fn main1() {
    buf.data[0] = c1;
}

@compute @workgroup_size(1) fn main2() {
    buf.data[0] = c2;
}

@compute @workgroup_size(c3) fn main3() {
    buf.data[0] = 3u;
}
)";

    std::vector<wgpu::ConstantEntry> constants1;
    constants1.push_back({nullptr, "1001", 1});
    std::vector<wgpu::ConstantEntry> constants2;
    constants2.push_back({nullptr, "1002", 2});
    std::vector<wgpu::ConstantEntry> constants3;
    constants3.push_back({nullptr, "1003", 1});

    wgpu::ShaderModule shaderModule = utils::CreateShaderModule(device, shader.c_str());

    wgpu::ComputePipelineDescriptor csDesc1;
    csDesc1.compute.module = shaderModule;
    csDesc1.compute.entryPoint = "main1";
    csDesc1.compute.constants = constants1.data();
    csDesc1.compute.constantCount = constants1.size();
    wgpu::ComputePipeline pipeline1 = device.CreateComputePipeline(&csDesc1);

    wgpu::ComputePipelineDescriptor csDesc2;
    csDesc2.compute.module = shaderModule;
    csDesc2.compute.entryPoint = "main2";
    csDesc2.compute.constants = constants2.data();
    csDesc2.compute.constantCount = constants2.size();
    wgpu::ComputePipeline pipeline2 = device.CreateComputePipeline(&csDesc2);

    wgpu::ComputePipelineDescriptor csDesc3;
    csDesc3.compute.module = shaderModule;
    csDesc3.compute.entryPoint = "main3";
    csDesc3.compute.constants = constants3.data();
    csDesc3.compute.constantCount = constants3.size();
    wgpu::ComputePipeline pipeline3 = device.CreateComputePipeline(&csDesc3);

    wgpu::BindGroup bindGroup1 =
        utils::MakeBindGroup(device, pipeline1.GetBindGroupLayout(0), {{0, buffer1}});
    wgpu::BindGroup bindGroup2 =
        utils::MakeBindGroup(device, pipeline2.GetBindGroupLayout(0), {{0, buffer2}});
    wgpu::BindGroup bindGroup3 =
        utils::MakeBindGroup(device, pipeline3.GetBindGroupLayout(0), {{0, buffer3}});

    wgpu::CommandBuffer commands;
    {
        wgpu::CommandEncoder encoder = device.CreateCommandEncoder();
        wgpu::ComputePassEncoder pass = encoder.BeginComputePass();
        pass.SetPipeline(pipeline1);
        pass.SetBindGroup(0, bindGroup1);
        pass.DispatchWorkgroups(1);

        pass.SetPipeline(pipeline2);
        pass.SetBindGroup(0, bindGroup2);
        pass.DispatchWorkgroups(1);

        pass.SetPipeline(pipeline3);
        pass.SetBindGroup(0, bindGroup3);
        pass.DispatchWorkgroups(1);

        pass.End();

        commands = encoder.Finish();
    }

    queue.Submit(1, &commands);

    EXPECT_BUFFER_U32_RANGE_EQ(expected1.data(), buffer1, 0, kCount);
    EXPECT_BUFFER_U32_RANGE_EQ(expected2.data(), buffer2, 0, kCount);
    EXPECT_BUFFER_U32_RANGE_EQ(expected3.data(), buffer3, 0, kCount);
}

// Test overridable constants with render pipeline
// Draw a triangle covering the render target, with vertex position and color values from
// overridable constants
TEST_P(ShaderTests, OverridableConstantsRenderPipeline) {
    wgpu::ShaderModule vsModule = utils::CreateShaderModule(device, R"(
@id(1111) override xright: f32;
@id(2222) override ytop: f32;
@vertex
fn main(@builtin(vertex_index) VertexIndex : u32)
     -> @builtin(position) vec4f {
  var pos = array(
      vec2f(-1.0, ytop),
      vec2f(-1.0, -ytop),
      vec2f(xright, 0.0));

  return vec4f(pos[VertexIndex], 0.0, 1.0);
})");

    wgpu::ShaderModule fsModule = utils::CreateShaderModule(device, R"(
@id(1000) override intensity: f32 = 0.0;
@fragment fn main()
    -> @location(0) vec4f {
    return vec4f(intensity, intensity, intensity, 1.0);
})");

    utils::BasicRenderPass renderPass = utils::CreateBasicRenderPass(device, 1, 1);

    utils::ComboRenderPipelineDescriptor descriptor;
    descriptor.vertex.module = vsModule;
    descriptor.cFragment.module = fsModule;
    descriptor.primitive.topology = wgpu::PrimitiveTopology::TriangleList;
    descriptor.cTargets[0].format = renderPass.colorFormat;

    std::vector<wgpu::ConstantEntry> vertexConstants;
    vertexConstants.push_back({nullptr, "1111", 3.0});  // x right
    vertexConstants.push_back({nullptr, "2222", 3.0});  // y top
    descriptor.vertex.constants = vertexConstants.data();
    descriptor.vertex.constantCount = vertexConstants.size();
    std::vector<wgpu::ConstantEntry> fragmentConstants;
    fragmentConstants.push_back({nullptr, "1000", 1.0});  // color intensity
    descriptor.cFragment.constants = fragmentConstants.data();
    descriptor.cFragment.constantCount = fragmentConstants.size();

    wgpu::RenderPipeline pipeline = device.CreateRenderPipeline(&descriptor);

    wgpu::CommandEncoder encoder = device.CreateCommandEncoder();
    wgpu::RenderPassEncoder pass = encoder.BeginRenderPass(&renderPass.renderPassInfo);
    pass.SetPipeline(pipeline);
    pass.Draw(3);
    pass.End();
    wgpu::CommandBuffer commands = encoder.Finish();
    queue.Submit(1, &commands);

    EXPECT_PIXEL_RGBA8_EQ(utils::RGBA8(255, 255, 255, 255), renderPass.color, 0, 0);
}

// This is a regression test for crbug.com/dawn:1363 where the BindingRemapper transform was run
// before the SingleEntryPoint transform, causing one of the other entry points to have conflicting
// bindings.
TEST_P(ShaderTests, ConflictingBindingsDueToTransformOrder) {
    wgpu::ShaderModule module = utils::CreateShaderModule(device, R"(
        @group(0) @binding(0) var<uniform> b0 : u32;
        @group(0) @binding(1) var<uniform> b1 : u32;

        @vertex fn vertex() -> @builtin(position) vec4f {
            _ = b0;
            return vec4f(0.0);
        }

        @fragment fn fragment() -> @location(0) vec4f {
            _ = b0;
            _ = b1;
            return vec4f(0.0);
        }
    )");

    utils::ComboRenderPipelineDescriptor desc;
    desc.vertex.module = module;
    desc.cFragment.module = module;

    device.CreateRenderPipeline(&desc);
}

// Check that chromium_disable_uniformity_analysis can be used. It is normally disallowed as unsafe
// but DawnTests allow all unsafe APIs by default.
TEST_P(ShaderTests, CheckUsageOf_chromium_disable_uniformity_analysis) {
    wgpu::ShaderModule module = utils::CreateShaderModule(device, R"(
        enable chromium_disable_uniformity_analysis;

        @compute @workgroup_size(8) fn uniformity_error(
            @builtin(local_invocation_id) local_invocation_id : vec3u
        ) {
            if (local_invocation_id.x == 0u) {
                workgroupBarrier();
            }
        }
    )");
    ASSERT_DEVICE_ERROR(utils::CreateShaderModule(device, R"(
        @compute @workgroup_size(8) fn uniformity_error(
            @builtin(local_invocation_id) local_invocation_id : vec3u
        ) {
            if (local_invocation_id.x == 0u) {
                workgroupBarrier();
            }
        }
    )"));
}

// Test that it is not possible to override the builtins in a way that breaks the robustness
// transform.
TEST_P(ShaderTests, ShaderOverridingRobustnessBuiltins) {
    // Make the test compute pipeline.
    wgpu::ComputePipelineDescriptor cDesc;
    cDesc.compute.module = utils::CreateShaderModule(device, R"(
        // A fake min() function that always returns 0.
        fn min(a : u32, b : u32) -> u32 {
            return 0;
        }

        @group(0) @binding(0) var<storage, read_write> result : u32;
        @compute @workgroup_size(1) fn little_bobby_tables() {
            // Prevent the SingleEntryPoint transform from removing our min().
            let forceUseOfMin = min(0, 1);

            let values = array(1u, 2u);
            let index = 1u;
            // Robustness adds transforms values[index] into values[min(index, 1u)].
            //  - If our min() is called, the this will be values[0] which is 1.
            //  - If the correct min() is called, the this will be values[1] which is 2.
            result = values[index];
        }
    )");
    wgpu::ComputePipeline pipeline = device.CreateComputePipeline(&cDesc);

    // Test 4-byte buffer that will receive the result.
    wgpu::BufferDescriptor bufDesc;
    bufDesc.size = 4;
    bufDesc.usage = wgpu::BufferUsage::Storage | wgpu::BufferUsage::CopySrc;
    wgpu::Buffer buf = device.CreateBuffer(&bufDesc);

    wgpu::BindGroup bg = utils::MakeBindGroup(device, pipeline.GetBindGroupLayout(0), {{0, buf}});

    // Run the compute pipeline.
    wgpu::CommandEncoder encoder = device.CreateCommandEncoder();
    wgpu::ComputePassEncoder pass = encoder.BeginComputePass();
    pass.SetPipeline(pipeline);
    pass.SetBindGroup(0, bg);
    pass.DispatchWorkgroups(1);
    pass.End();

    wgpu::CommandBuffer commands = encoder.Finish();
    queue.Submit(1, &commands);

    // See the comment in the shader for why we expect a 2 here.
    EXPECT_BUFFER_U32_EQ(2, buf, 0);
}

// Test that when fragment input is a subset of the vertex output, the render pipeline should be
// valid.
TEST_P(ShaderTests, FragmentInputIsSubsetOfVertexOutput) {
    wgpu::ShaderModule vsModule = utils::CreateShaderModule(device, R"(
struct ShaderIO {
    @location(1) var1: f32,
    @location(3) @interpolate(flat, either) var3: u32,
    @location(5) @interpolate(flat, either) var5: i32,
    @location(7) var7: f32,
    @location(9) @interpolate(flat, either) var9: u32,
    @builtin(position) pos: vec4f,
}

@vertex fn main(@builtin(vertex_index) VertexIndex : u32)
     -> ShaderIO {
  var pos = array(
      vec2f(-1.0, 3.0),
      vec2f(-1.0, -3.0),
      vec2f(3.0, 0.0));

  var shaderIO: ShaderIO;
  shaderIO.var1 = 0.0;
  shaderIO.var3 = 1u;
  shaderIO.var5 = -9;
  shaderIO.var7 = 1.0;
  shaderIO.var9 = 0u;
  shaderIO.pos = vec4f(pos[VertexIndex], 0.0, 1.0);

  return shaderIO;
})");

    wgpu::ShaderModule fsModule = utils::CreateShaderModule(device, R"(
struct ShaderIO {
    @location(3) @interpolate(flat, either) var3: u32,
    @location(7) var7: f32,
}

@fragment fn main(io: ShaderIO)
    -> @location(0) vec4f {
    return vec4f(f32(io.var3), io.var7, 1.0, 1.0);
})");

    utils::BasicRenderPass renderPass = utils::CreateBasicRenderPass(device, 1, 1);

    utils::ComboRenderPipelineDescriptor descriptor;
    descriptor.vertex.module = vsModule;
    descriptor.cFragment.module = fsModule;
    descriptor.primitive.topology = wgpu::PrimitiveTopology::TriangleList;
    descriptor.cTargets[0].format = renderPass.colorFormat;

    wgpu::RenderPipeline pipeline = device.CreateRenderPipeline(&descriptor);

    wgpu::CommandEncoder encoder = device.CreateCommandEncoder();
    wgpu::RenderPassEncoder pass = encoder.BeginRenderPass(&renderPass.renderPassInfo);
    pass.SetPipeline(pipeline);
    pass.Draw(3);
    pass.End();
    wgpu::CommandBuffer commands = encoder.Finish();
    queue.Submit(1, &commands);

    EXPECT_PIXEL_RGBA8_EQ(utils::RGBA8(255, 255, 255, 255), renderPass.color, 0, 0);
}

// Test that when fragment input is a subset of the vertex output and the order of them is
// different, the render pipeline should be valid.
TEST_P(ShaderTests, FragmentInputIsSubsetOfVertexOutputWithDifferentOrder) {
    wgpu::ShaderModule vsModule = utils::CreateShaderModule(device, R"(
struct ShaderIO {
    @location(5) @align(16) var5: f32,
    @location(1) var1: f32,
    @location(2) var2: f32,
    @location(3) @align(8) var3: f32,
    @location(4) var4: vec4f,
    @builtin(position) pos: vec4f,
}

@vertex fn main(@builtin(vertex_index) VertexIndex : u32)
     -> ShaderIO {
  var pos = array(
      vec2f(-1.0, 3.0),
      vec2f(-1.0, -3.0),
      vec2f(3.0, 0.0));

  var shaderIO: ShaderIO;
  shaderIO.var1 = 0.0;
  shaderIO.var2 = 0.0;
  shaderIO.var3 = 1.0;
  shaderIO.var4 = vec4f(0.4, 0.4, 0.4, 0.4);
  shaderIO.var5 = 1.0;
  shaderIO.pos = vec4f(pos[VertexIndex], 0.0, 1.0);

  return shaderIO;
})");

    wgpu::ShaderModule fsModule = utils::CreateShaderModule(device, R"(
struct ShaderIO {
    @location(4) var4: vec4f,
    @location(1) var1: f32,
    @location(5) @align(16) var5: f32,
}

@fragment fn main(io: ShaderIO)
    -> @location(0) vec4f {
    return vec4f(io.var1, io.var5, io.var4.x, 1.0);
})");

    utils::BasicRenderPass renderPass = utils::CreateBasicRenderPass(device, 1, 1);

    utils::ComboRenderPipelineDescriptor descriptor;
    descriptor.vertex.module = vsModule;
    descriptor.cFragment.module = fsModule;
    descriptor.primitive.topology = wgpu::PrimitiveTopology::TriangleList;
    descriptor.cTargets[0].format = renderPass.colorFormat;

    wgpu::RenderPipeline pipeline = device.CreateRenderPipeline(&descriptor);

    wgpu::CommandEncoder encoder = device.CreateCommandEncoder();
    wgpu::RenderPassEncoder pass = encoder.BeginRenderPass(&renderPass.renderPassInfo);
    pass.SetPipeline(pipeline);
    pass.Draw(3);
    pass.End();
    wgpu::CommandBuffer commands = encoder.Finish();
    queue.Submit(1, &commands);

    EXPECT_PIXEL_RGBA8_EQ(utils::RGBA8(0, 255, 102, 255), renderPass.color, 0, 0);
}

// Test that when fragment input is a subset of the vertex output and that when the builtin
// interstage variables may mess up with the order, the render pipeline should be valid.
TEST_P(ShaderTests, FragmentInputIsSubsetOfVertexOutputBuiltinOrder) {
    wgpu::ShaderModule vsModule = utils::CreateShaderModule(device, R"(
struct ShaderIO {
    @location(1) var1: f32,
    @builtin(position) pos: vec4f,
    @location(8) var8: vec3f,
    @location(7) var7: f32,
}

@vertex fn main(@builtin(vertex_index) VertexIndex : u32)
     -> ShaderIO {
  var pos = array(
      vec2f(-1.0, 3.0),
      vec2f(-1.0, -3.0),
      vec2f(3.0, 0.0));

  var shaderIO: ShaderIO;
  shaderIO.var1 = 0.0;
  shaderIO.var7 = 1.0;
  shaderIO.var8 = vec3f(1.0, 0.4, 0.0);
  shaderIO.pos = vec4f(pos[VertexIndex], 0.0, 1.0);

  return shaderIO;
})");

    wgpu::ShaderModule fsModule = utils::CreateShaderModule(device, R"(
struct ShaderIO {
    @builtin(position) pos: vec4f,
    @location(7) var7: f32,
}

@fragment fn main(io: ShaderIO)
    -> @location(0) vec4f {
    return vec4f(0.0, io.var7, 0.4, 1.0);
})");

    utils::BasicRenderPass renderPass = utils::CreateBasicRenderPass(device, 1, 1);

    utils::ComboRenderPipelineDescriptor descriptor;
    descriptor.vertex.module = vsModule;
    descriptor.cFragment.module = fsModule;
    descriptor.primitive.topology = wgpu::PrimitiveTopology::TriangleList;
    descriptor.cTargets[0].format = renderPass.colorFormat;

    wgpu::RenderPipeline pipeline = device.CreateRenderPipeline(&descriptor);

    wgpu::CommandEncoder encoder = device.CreateCommandEncoder();
    wgpu::RenderPassEncoder pass = encoder.BeginRenderPass(&renderPass.renderPassInfo);
    pass.SetPipeline(pipeline);
    pass.Draw(3);
    pass.End();
    wgpu::CommandBuffer commands = encoder.Finish();
    queue.Submit(1, &commands);

    EXPECT_PIXEL_RGBA8_EQ(utils::RGBA8(0, 255, 102, 255), renderPass.color, 0, 0);
}

// Test that the derivative_uniformity diagnostic filter is handled correctly through the full
// shader compilation flow.
TEST_P(ShaderTests, DerivativeUniformityDiagnosticFilter) {
    wgpu::ShaderModule vsModule = utils::CreateShaderModule(device, R"(
struct VertexOut {
  @builtin(position) pos : vec4f,
  @location(0) value : f32,
}

@vertex
fn main(@builtin(vertex_index) VertexIndex : u32) -> VertexOut {
  const pos = array(
      vec2( 1.0, -1.0),
      vec2(-1.0, -1.0),
      vec2( 0.0,  1.0),
  );
  return VertexOut(vec4(pos[VertexIndex], 0.0, 1.0), 0.5);
})");

    wgpu::ShaderModule fsModule = utils::CreateShaderModule(device, R"(
diagnostic(off, derivative_uniformity);

@fragment
fn main(@location(0) value : f32) -> @location(0) vec4f {
  if (value > 0) {
    let intensity = 1.0 - dpdx(1.0);
    return vec4(intensity, intensity, intensity, 1.0);
  }
  return vec4(1.0);
})");

    utils::BasicRenderPass renderPass = utils::CreateBasicRenderPass(device, 1, 1);

    utils::ComboRenderPipelineDescriptor descriptor;
    descriptor.vertex.module = vsModule;
    descriptor.cFragment.module = fsModule;
    descriptor.primitive.topology = wgpu::PrimitiveTopology::TriangleList;
    descriptor.cTargets[0].format = renderPass.colorFormat;
    wgpu::RenderPipeline pipeline = device.CreateRenderPipeline(&descriptor);

    wgpu::CommandEncoder encoder = device.CreateCommandEncoder();
    wgpu::RenderPassEncoder pass = encoder.BeginRenderPass(&renderPass.renderPassInfo);
    pass.SetPipeline(pipeline);
    pass.Draw(3);
    pass.End();
    wgpu::CommandBuffer commands = encoder.Finish();
    queue.Submit(1, &commands);

    EXPECT_PIXEL_RGBA8_EQ(utils::RGBA8(255, 255, 255, 255), renderPass.color, 0, 0);
}

// Test that identifiers containing double underscores are renamed in the GLSL backend.
TEST_P(ShaderTests, DoubleUnderscore) {
    wgpu::ShaderModule vsModule = utils::CreateShaderModule(device, R"(
@vertex
fn main(@builtin(vertex_index) VertexIndex : u32) -> @builtin(position) vec4f {
  const pos = array(
      vec2( 1.0, -1.0),
      vec2(-1.0, -1.0),
      vec2( 0.0,  1.0),
  );
  return vec4(pos[VertexIndex], 0.0, 1.0);
})");

    wgpu::ShaderModule fsModule = utils::CreateShaderModule(device, R"(
diagnostic(off, derivative_uniformity);

@fragment
fn main() -> @location(0) vec4f {
  let re__sult = vec4f(1.0);
  return re__sult;
})");

    utils::BasicRenderPass renderPass = utils::CreateBasicRenderPass(device, 1, 1);

    utils::ComboRenderPipelineDescriptor descriptor;
    descriptor.vertex.module = vsModule;
    descriptor.cFragment.module = fsModule;
    descriptor.primitive.topology = wgpu::PrimitiveTopology::TriangleList;
    descriptor.cTargets[0].format = renderPass.colorFormat;
    wgpu::RenderPipeline pipeline = device.CreateRenderPipeline(&descriptor);

    wgpu::CommandEncoder encoder = device.CreateCommandEncoder();
    wgpu::RenderPassEncoder pass = encoder.BeginRenderPass(&renderPass.renderPassInfo);
    pass.SetPipeline(pipeline);
    pass.Draw(3);
    pass.End();
    wgpu::CommandBuffer commands = encoder.Finish();
    queue.Submit(1, &commands);

    EXPECT_PIXEL_RGBA8_EQ(utils::RGBA8(255, 255, 255, 255), renderPass.color, 0, 0);
}

// Test that matrices can be passed by value, which can cause issues on Qualcomm devices.
// See crbug.com/tint/2045#c5
TEST_P(ShaderTests, PassMatrixByValue) {
    std::vector<float> inputs{0.1, 0.2, 0.3, 0.0, 0.4, 0.5, 0.6, 0.0, 0.7, 0.8, 0.9, 0.0};
    std::vector<float> expected{1.1, 1.2, 1.3, 0.0, 1.4, 1.5, 1.6, 0.0, 1.7, 1.8, 1.9, 0.0};
    uint64_t bufferSize = static_cast<uint64_t>(inputs.size() * sizeof(float));
    wgpu::Buffer buffer = utils::CreateBufferFromData(
        device, inputs.data(), bufferSize, wgpu::BufferUsage::Storage | wgpu::BufferUsage::CopySrc);

    std::string shader = R"(
@group(0) @binding(0)
var<storage, read_write> buffer : mat3x3f;

fn foo(rhs : mat3x3f) {
  buffer = buffer + rhs;
}

@compute @workgroup_size(1)
fn main() {
  let rhs = mat3x3f(1, 1, 1, 1, 1, 1, 1, 1, 1);
  foo(rhs);
}
)";

    wgpu::ComputePipeline pipeline = CreateComputePipeline(shader, "main");

    wgpu::BindGroup bindGroup =
        utils::MakeBindGroup(device, pipeline.GetBindGroupLayout(0), {{0, buffer}});

    wgpu::CommandBuffer commands;
    {
        wgpu::CommandEncoder encoder = device.CreateCommandEncoder();
        wgpu::ComputePassEncoder pass = encoder.BeginComputePass();
        pass.SetPipeline(pipeline);
        pass.SetBindGroup(0, bindGroup);
        pass.DispatchWorkgroups(1);
        pass.End();

        commands = encoder.Finish();
    }

    queue.Submit(1, &commands);

    EXPECT_BUFFER_FLOAT_RANGE_EQ(expected.data(), buffer, 0, expected.size());
}

// Test that matrices in structs can be passed by value, which can cause issues on Qualcomm devices.
// See crbug.com/tint/2045#c5
TEST_P(ShaderTests, PassMatrixInStructByValue) {
    std::vector<float> inputs{0.1, 0.2, 0.3, 0.0, 0.4, 0.5, 0.6, 0.0, 0.7, 0.8, 0.9, 0.0};
    std::vector<float> expected{1.1, 1.2, 1.3, 0.0, 1.4, 1.5, 1.6, 0.0, 1.7, 1.8, 1.9, 0.0};
    uint64_t bufferSize = static_cast<uint64_t>(inputs.size() * sizeof(float));
    wgpu::Buffer buffer = utils::CreateBufferFromData(
        device, inputs.data(), bufferSize, wgpu::BufferUsage::Storage | wgpu::BufferUsage::CopySrc);

    std::string shader = R"(
struct S {
    m : mat3x3f,
}

@group(0) @binding(0)
var<storage, read_write> buffer : S;

fn foo(rhs : S) {
  buffer = S(buffer.m + rhs.m);
}

@compute @workgroup_size(1)
fn main() {
  let rhs = S(mat3x3(1, 1, 1, 1, 1, 1, 1, 1, 1));
  foo(rhs);
}
)";

    wgpu::ComputePipeline pipeline = CreateComputePipeline(shader, "main");

    wgpu::BindGroup bindGroup =
        utils::MakeBindGroup(device, pipeline.GetBindGroupLayout(0), {{0, buffer}});

    wgpu::CommandBuffer commands;
    {
        wgpu::CommandEncoder encoder = device.CreateCommandEncoder();
        wgpu::ComputePassEncoder pass = encoder.BeginComputePass();
        pass.SetPipeline(pipeline);
        pass.SetBindGroup(0, bindGroup);
        pass.DispatchWorkgroups(1);
        pass.End();

        commands = encoder.Finish();
    }

    queue.Submit(1, &commands);

    EXPECT_BUFFER_FLOAT_RANGE_EQ(expected.data(), buffer, 0, expected.size());
}

// Test that robustness works correctly on uniform buffers that contain mat4x3 types, which can
// cause issues on Qualcomm devices. See crbug.com/tint/2074.
TEST_P(ShaderTests, Robustness_Uniform_Mat4x3) {
    // Note: Using non-zero values would make the test more robust, but this involves small changes
    // to the shader which stop the miscompile in the original bug from happening.
    std::vector<float> inputs{0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0,
                              0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0};
    std::vector<uint32_t> constantData{0};
    std::vector<uint32_t> outputs{0xDEADBEEFu};
    uint64_t bufferSize = static_cast<uint64_t>(inputs.size() * sizeof(float));
    wgpu::Buffer buffer =
        utils::CreateBufferFromData(device, inputs.data(), bufferSize, wgpu::BufferUsage::Uniform);
    wgpu::Buffer constants =
        utils::CreateBufferFromData(device, constantData.data(), 4, wgpu::BufferUsage::Uniform);
    wgpu::Buffer output = utils::CreateBufferFromData(
        device, outputs.data(), 4, wgpu::BufferUsage::Storage | wgpu::BufferUsage::CopySrc);

    // Note: This shader was lifted from WebGPU CTS, and triggers a miscompile for the second case.
    // The miscompile disappears when too much of the unrelated code is deleted or changed, so the
    // shader is left in it's original form.
    std::string shader = R"(
    struct Constants {
      zero: u32
    };
    @group(0) @binding(2) var<uniform> constants: Constants;

    struct Result {
      value: u32
    };
    @group(0) @binding(1) var<storage, read_write> result: Result;

    struct TestData {
      data: mat4x3<f32>,
    };
    @group(0) @binding(0) var<uniform> s: TestData;

    fn runTest() -> u32 {
      {
        let index = (0u);
        if (any(s.data[index] != vec3<f32>())) { return 0x1001u; }
      }
      {
        let index = (4u - 1u);
        if (any(s.data[index] != vec3<f32>())) { return 0x1002u; }
      }
      {
        let index = (4u);
        if (any(s.data[index] != vec3<f32>())) { return 0x1003u; }
      }
      {
        let index = (1000000u);
        if (any(s.data[index] != vec3<f32>())) { return 0x1004u; }
      }
      {
        let index = (4294967295u);
        if (any(s.data[index] != vec3<f32>())) { return 0x1005u; }
      }
      {
        let index = (2147483647u);
        if (any(s.data[index] != vec3<f32>())) { return 0x1006u; }
      }
      {
        let index = (0u) + 0u;
        if (any(s.data[index] != vec3<f32>())) { return 0x1007u; }
      }
      {
        let index = (4u - 1u) + 0u;
        if (any(s.data[index] != vec3<f32>())) { return 0x1008u; }
      }
      {
        let index = (4u) + 0u;
        if (any(s.data[index] != vec3<f32>())) { return 0x1009u; }
      }
      {
        let index = (1000000u) + 0u;
        if (any(s.data[index] != vec3<f32>())) { return 0x100au; }
      }
      {
        let index = (4294967295u) + 0u;
        if (any(s.data[index] != vec3<f32>())) { return 0x100bu; }
      }
      {
        let index = (2147483647u) + 0u;
        if (any(s.data[index] != vec3<f32>())) { return 0x100cu; }
      }
      {
        let index = (0u) + u32(constants.zero);
        if (any(s.data[index] != vec3<f32>())) { return 0x100du; }
      }
      {
        let index = (4u - 1u) + u32(constants.zero);
        if (any(s.data[index] != vec3<f32>())) { return 0x100eu; }
      }
      {
        let index = (4u) + u32(constants.zero);
        if (any(s.data[index] != vec3<f32>())) { return 0x100fu; }
      }
      {
        let index = (1000000u) + u32(constants.zero);
        if (any(s.data[index] != vec3<f32>())) { return 0x1010u; }
      }
      {
        let index = (4294967295u) + u32(constants.zero);
        if (any(s.data[index] != vec3<f32>())) { return 0x1011u; }
      }
      {
        let index = (2147483647u) + u32(constants.zero);
        if (any(s.data[index] != vec3<f32>())) { return 0x1012u; }
      }
      {
        let index = (0);
        if (any(s.data[index] != vec3<f32>())) { return 0x1013u; }
      }
      {
        let index = (4 - 1);
        if (any(s.data[index] != vec3<f32>())) { return 0x1014u; }
      }
      {
        let index = (-1);
        if (any(s.data[index] != vec3<f32>())) { return 0x1015u; }
      }
      {
        let index = (4);
        if (any(s.data[index] != vec3<f32>())) { return 0x1016u; }
      }
      {
        let index = (-1000000);
        if (any(s.data[index] != vec3<f32>())) { return 0x1017u; }
      }
      {
        let index = (1000000);
        if (any(s.data[index] != vec3<f32>())) { return 0x1018u; }
      }
      {
        let index = (-2147483648);
        if (any(s.data[index] != vec3<f32>())) { return 0x1019u; }
      }
      {
        let index = (2147483647);
        if (any(s.data[index] != vec3<f32>())) { return 0x101au; }
      }
      {
        let index = (0) + 0;
        if (any(s.data[index] != vec3<f32>())) { return 0x101bu; }
      }
      {
        let index = (4 - 1) + 0;
        if (any(s.data[index] != vec3<f32>())) { return 0x101cu; }
      }
      {
        let index = (-1) + 0;
        if (any(s.data[index] != vec3<f32>())) { return 0x101du; }
      }
      {
        let index = (4) + 0;
        if (any(s.data[index] != vec3<f32>())) { return 0x101eu; }
      }
      {
        let index = (-1000000) + 0;
        if (any(s.data[index] != vec3<f32>())) { return 0x101fu; }
      }
      {
        let index = (1000000) + 0;
        if (any(s.data[index] != vec3<f32>())) { return 0x1020u; }
      }
      {
        let index = (-2147483648) + 0;
        if (any(s.data[index] != vec3<f32>())) { return 0x1021u; }
      }
      {
        let index = (2147483647) + 0;
        if (any(s.data[index] != vec3<f32>())) { return 0x1022u; }
      }
      {
        let index = (0) + i32(constants.zero);
        if (any(s.data[index] != vec3<f32>())) { return 0x1023u; }
      }
      {
        let index = (4 - 1) + i32(constants.zero);
        if (any(s.data[index] != vec3<f32>())) { return 0x1024u; }
      }
      {
        let index = (-1) + i32(constants.zero);
        if (any(s.data[index] != vec3<f32>())) { return 0x1025u; }
      }
      {
        let index = (4) + i32(constants.zero);
        if (any(s.data[index] != vec3<f32>())) { return 0x1026u; }
      }
      {
        let index = (-1000000) + i32(constants.zero);
        if (any(s.data[index] != vec3<f32>())) { return 0x1027u; }
      }
      {
        let index = (1000000) + i32(constants.zero);
        if (any(s.data[index] != vec3<f32>())) { return 0x1028u; }
      }
      {
        let index = (-2147483648) + i32(constants.zero);
        if (any(s.data[index] != vec3<f32>())) { return 0x1029u; }
      }
      {
        let index = (2147483647) + i32(constants.zero);
        if (any(s.data[index] != vec3<f32>())) { return 0x102au; }
      }
      return 0u;
    }

    @compute @workgroup_size(1)
    fn main() {
      result.value = runTest();
    }
)";

    wgpu::ComputePipeline pipeline = CreateComputePipeline(shader, "main");

    wgpu::BindGroup bindGroup = utils::MakeBindGroup(device, pipeline.GetBindGroupLayout(0),
                                                     {{0, buffer}, {1, output}, {2, constants}});

    wgpu::CommandBuffer commands;
    {
        wgpu::CommandEncoder encoder = device.CreateCommandEncoder();
        wgpu::ComputePassEncoder pass = encoder.BeginComputePass();
        pass.SetPipeline(pipeline);
        pass.SetBindGroup(0, bindGroup);
        pass.DispatchWorkgroups(1);
        pass.End();

        commands = encoder.Finish();
    }

    queue.Submit(1, &commands);

    outputs[0] = 0u;
    EXPECT_BUFFER_U32_RANGE_EQ(outputs.data(), output, 0, outputs.size());
}

// SSBOs declared with the same name in multiple shader stages must contain the same members in
// GLSL. If not renamed properly, names of binding at (2, 2) in the vertex stage and (0, 3) in the
// fragment stage can possibly collide.
TEST_P(ShaderTests, StorageAcrossStages) {
    DAWN_SUPPRESS_TEST_IF(GetSupportedLimits().maxStorageBuffersInFragmentStage < 4);
    DAWN_SUPPRESS_TEST_IF(GetSupportedLimits().maxStorageBuffersInVertexStage < 3);

    wgpu::ShaderModule module = utils::CreateShaderModule(device, R"(
        @group(2) @binding(2) var<storage> u0_2: f32;
        @group(1) @binding(1) var<storage> u0_1: f32;
        @group(0) @binding(0) var<storage> u0_0: f32;

        @group(0) @binding(3) var<storage> u1_3: f32;
        @group(2) @binding(2) var<storage> u1_2: f32;
        @group(1) @binding(1) var<storage> u1_1: f32;
        @group(0) @binding(0) var<storage> u1_0: f32;

        @vertex fn vertex() -> @builtin(position) vec4f {
          _ = u0_0;
          _ = u0_1;
          _ = u0_2;
          return vec4f(0);
        }

        @fragment fn fragment() -> @location(0) vec4f {
          _ = u1_0;
          _ = u1_1;
          _ = u1_2;
          _ = u1_3;
          return vec4f(0);
        }
    )");

    utils::ComboRenderPipelineDescriptor desc;
    desc.vertex.module = module;
    desc.cFragment.module = module;

    device.CreateRenderPipeline(&desc);
}

// SSBOs declared with the same name in multiple shader stages must contain the same members in
// GLSL. If not renamed properly, names of binding at (2, 2) in the vertex stage and (0, 3) in the
// fragment stage can possibly collide.
TEST_P(ShaderTests, StorageAcrossStagesStruct) {
    DAWN_SUPPRESS_TEST_IF(GetSupportedLimits().maxStorageBuffersInFragmentStage < 2);
    DAWN_SUPPRESS_TEST_IF(GetSupportedLimits().maxStorageBuffersInVertexStage < 1);

    wgpu::ShaderModule module = utils::CreateShaderModule(device, R"(
        struct block {
            inner: f32
        }
        @group(2) @binding(2) var<storage> u0_2: block;

        @group(0) @binding(3) var<storage> u1_3: f32;
        @group(2) @binding(2) var<storage> u1_2: f32;

        @vertex fn vertex() -> @builtin(position) vec4f {
          _ = u0_2.inner;
          return vec4f(0);
        }

        @fragment fn fragment() -> @location(0) vec4f {
          _ = u1_2;
          _ = u1_3;
          return vec4f(0);
        }
    )");

    utils::ComboRenderPipelineDescriptor desc;
    desc.vertex.module = module;
    desc.cFragment.module = module;

    device.CreateRenderPipeline(&desc);
}

// SSBOs declared with the same name in multiple shader stages must contain the same members in
// GLSL. If not renamed properly, names of binding at (2, 2) in the vertex stage and (0, 3) in the
// fragment stage can possibly collide.
TEST_P(ShaderTests, StorageAcrossStagesSeparateModules) {
    DAWN_SUPPRESS_TEST_IF(GetSupportedLimits().maxStorageBuffersInFragmentStage < 4);
    DAWN_SUPPRESS_TEST_IF(GetSupportedLimits().maxStorageBuffersInVertexStage < 3);

    wgpu::ShaderModule vsModule = utils::CreateShaderModule(device, R"(
        @group(2) @binding(2) var<storage> u0_2: f32;
        @group(1) @binding(1) var<storage> u0_1: f32;
        @group(0) @binding(0) var<storage> u0_0: f32;

        @vertex fn vertex() -> @builtin(position) vec4f {
          _ = u0_0;
          _ = u0_1;
          _ = u0_2;
          return vec4f(0);
        }
    )");
    wgpu::ShaderModule fsModule = utils::CreateShaderModule(device, R"(
        @group(0) @binding(3) var<storage> u1_3: f32;
        @group(2) @binding(2) var<storage> u1_2: f32;
        @group(1) @binding(1) var<storage> u1_1: f32;
        @group(0) @binding(0) var<storage> u1_0: f32;

        @fragment fn fragment() -> @location(0) vec4f {
          _ = u1_0;
          _ = u1_1;
          _ = u1_2;
          _ = u1_3;
          return vec4f(0);
        }
    )");

    utils::ComboRenderPipelineDescriptor desc;
    desc.vertex.module = vsModule;
    desc.cFragment.module = fsModule;

    device.CreateRenderPipeline(&desc);
}

// Deliberately mismatch an SSBO block name at differrent stages.
TEST_P(ShaderTests, StorageAcrossStagesSeparateModuleMismatch) {
    DAWN_SUPPRESS_TEST_IF(GetSupportedLimits().maxStorageBuffersInFragmentStage < 1);
    DAWN_SUPPRESS_TEST_IF(GetSupportedLimits().maxStorageBuffersInVertexStage < 2);

    wgpu::ShaderModule module = utils::CreateShaderModule(device, R"(
        @group(0) @binding(0) var<storage> tint_symbol_ubo_0: f32;
        @group(0) @binding(1) var<storage> tint_symbol_ubo_1: u32;

        @vertex fn vertex() -> @builtin(position) vec4f {
          _ = tint_symbol_ubo_0;
          _ = tint_symbol_ubo_1;
            return vec4f(tint_symbol_ubo_0) + vec4f(f32(tint_symbol_ubo_1));
        }

        @fragment fn fragment() -> @location(0) vec4f {
          _ = tint_symbol_ubo_1;
            return vec4f(f32(tint_symbol_ubo_1));
        }
    )");

    utils::ComboRenderPipelineDescriptor desc;
    desc.vertex.module = module;
    desc.cFragment.module = module;

    device.CreateRenderPipeline(&desc);
}

// Having different block contents at the same binding point used in different stages is allowed.
TEST_P(ShaderTests, StorageAcrossStagesSameBindingPointCollide) {
    DAWN_SUPPRESS_TEST_IF(GetSupportedLimits().maxStorageBuffersInFragmentStage < 1);
    DAWN_SUPPRESS_TEST_IF(GetSupportedLimits().maxStorageBuffersInVertexStage < 1);

    wgpu::ShaderModule module = utils::CreateShaderModule(device, R"(
        struct X { x : vec4f }
        struct Y { y : vec4i }

        @group(0) @binding(0) var<storage> v : X;
        @group(0) @binding(0) var<storage> f : Y;

        @vertex fn vertex() -> @builtin(position) vec4f {
            _ = v;
            return vec4f();
        }

        @fragment fn fragment() -> @location(0) vec4f {
            _ = f;
            return vec4f();
        }
    )");

    utils::ComboRenderPipelineDescriptor desc;
    desc.vertex.module = module;
    desc.cFragment.module = module;

    device.CreateRenderPipeline(&desc);
}

// Having different block contents at the same binding point used in different stages is allowed,
// with or without struct wrapper.
TEST_P(ShaderTests, StorageAcrossStagesSameBindingPointCollideMixedStructDef) {
    DAWN_SUPPRESS_TEST_IF(GetSupportedLimits().maxStorageBuffersInFragmentStage < 1);
    DAWN_SUPPRESS_TEST_IF(GetSupportedLimits().maxStorageBuffersInVertexStage < 1);

    wgpu::ShaderModule module = utils::CreateShaderModule(device, R"(
        struct X { x : vec4f }

        @group(0) @binding(0) var<storage> v : X;
        @group(0) @binding(0) var<storage> f : vec3u;

        @vertex fn vertex() -> @builtin(position) vec4f {
            _ = v;
            return vec4f();
        }

        @fragment fn fragment() -> @location(0) vec4f {
            _ = f;
            return vec4f();
        }
    )");

    utils::ComboRenderPipelineDescriptor desc;
    desc.vertex.module = module;
    desc.cFragment.module = module;

    device.CreateRenderPipeline(&desc);
}

// UBOs declared with the same name in multiple shader stages must contain the same members in GLSL.
// If not renamed properly, names of binding at (2, 2) in the vertex stage and (0, 3) in the
// fragment stage can possibly collide.
TEST_P(ShaderTests, UniformAcrossStages) {
    wgpu::ShaderModule module = utils::CreateShaderModule(device, R"(
        @group(2) @binding(2) var<uniform> u0_2: f32;

        @group(0) @binding(3) var<uniform> u1_3: f32;
        @group(2) @binding(2) var<uniform> u1_2: f32;

        @vertex fn vertex() -> @builtin(position) vec4f {
          _ = u0_2;
          return vec4f(0);
        }

        @fragment fn fragment() -> @location(0) vec4f {
          _ = u1_2;
          _ = u1_3;
          return vec4f(0);
        }
    )");

    utils::ComboRenderPipelineDescriptor desc;
    desc.vertex.module = module;
    desc.cFragment.module = module;

    device.CreateRenderPipeline(&desc);
}

// UBOs declared with the same name in multiple shader stages must contain the same members in GLSL.
// If not renamed properly, names of binding at (2, 2) in the vertex stage and (0, 3) in the
// fragment stage can possibly collide.
TEST_P(ShaderTests, UniformAcrossStagesStruct) {
    wgpu::ShaderModule module = utils::CreateShaderModule(device, R"(
        struct block {
            inner: f32
        }
        @group(2) @binding(2) var<uniform> u0_2: block;

        @group(0) @binding(3) var<uniform> u1_3: f32;
        @group(2) @binding(2) var<uniform> u1_2: f32;

        @vertex fn vertex() -> @builtin(position) vec4f {
          _ = u0_2.inner;
          return vec4f(0);
        }

        @fragment fn fragment() -> @location(0) vec4f {
          _ = u1_2;
          _ = u1_3;
          return vec4f(0);
        }
    )");

    utils::ComboRenderPipelineDescriptor desc;
    desc.vertex.module = module;
    desc.cFragment.module = module;

    device.CreateRenderPipeline(&desc);
}

// UBOs declared with the same name in multiple shader stages must contain the same members in GLSL.
// If not renamed properly, names of binding at (2, 2) in the vertex stage and (0, 3) in the
// fragment stage can possibly collide.
TEST_P(ShaderTests, UniformAcrossStagesSeparateModule) {
    wgpu::ShaderModule vsModule = utils::CreateShaderModule(device, R"(
        @group(2) @binding(2) var<uniform> u0_2: f32;

        @vertex fn vertex() -> @builtin(position) vec4f {
          _ = u0_2;
          return vec4f(0);
        }
    )");
    wgpu::ShaderModule fsModule = utils::CreateShaderModule(device, R"(
        @group(0) @binding(3) var<uniform> u1_3: f32;
        @group(2) @binding(2) var<uniform> u1_2: f32;

        @fragment fn fragment() -> @location(0) vec4f {
          _ = u1_2;
          _ = u1_3;
          return vec4f(0);
        }
    )");

    utils::ComboRenderPipelineDescriptor desc;
    desc.vertex.module = vsModule;
    desc.cFragment.module = fsModule;

    device.CreateRenderPipeline(&desc);
}

// Deliberately mismatch a UBO block name at differrent stages.
TEST_P(ShaderTests, UniformAcrossStagesSeparateModuleMismatch) {
    wgpu::ShaderModule module = utils::CreateShaderModule(device, R"(
        @group(0) @binding(0) var<uniform> tint_symbol_ubo_0: f32;
        @group(0) @binding(1) var<uniform> tint_symbol_ubo_1: u32;

        @vertex fn vertex() -> @builtin(position) vec4f {
          _ = tint_symbol_ubo_0;
          _ = tint_symbol_ubo_1;
          return vec4f(tint_symbol_ubo_0) + vec4f(f32(tint_symbol_ubo_1));
        }

        @fragment fn fragment() -> @location(0) vec4f {
          _ = tint_symbol_ubo_1;
          return vec4f(f32(tint_symbol_ubo_1));
        }
    )");

    utils::ComboRenderPipelineDescriptor desc;
    desc.vertex.module = module;
    desc.cFragment.module = module;

    device.CreateRenderPipeline(&desc);
}

// Test that padding is correctly applied to a UBO used in both vert and
// frag stages. Insert an additional UBO in the frag stage before the reused UBO.
TEST_P(ShaderTests, UniformAcrossStagesSeparateModuleMismatchWithCustomSize) {
    wgpu::ShaderModule module = utils::CreateShaderModule(device, R"(
        struct A {
          f : f32,
        };
        struct B {
          u : u32,
        }
        @group(0) @binding(0) var<uniform> tint_symbol_ubo_0: A;
        @group(0) @binding(1) var<uniform> tint_symbol_ubo_1: B;

        @vertex fn vertex() -> @builtin(position) vec4f {
          _ = tint_symbol_ubo_0;
          _ = tint_symbol_ubo_1;
          return vec4f(tint_symbol_ubo_0.f) + vec4f(f32(tint_symbol_ubo_1.u));
        }

        @fragment fn fragment() -> @location(0) vec4f {
          _ = tint_symbol_ubo_1;
          return vec4f(f32(tint_symbol_ubo_1.u));
        }
    )");

    utils::ComboRenderPipelineDescriptor desc;
    desc.vertex.module = module;
    desc.cFragment.module = module;

    device.CreateRenderPipeline(&desc);
}

// Test that accessing instance_index in the vert shader and assigning to frag_depth in the frag
// shader works.
TEST_P(ShaderTests, FragDepthAndInstanceIndex) {
    wgpu::ShaderModule module = utils::CreateShaderModule(device, R"(
        @group(0) @binding(0) var<uniform> a : f32;

        @fragment fn fragment() -> @builtin(frag_depth) f32 {
          return a;
        }

        @vertex fn vertex(@builtin(instance_index) instance : u32) -> @builtin(position) vec4f {
          return vec4f(f32(instance));
        }
    )");

    utils::ComboRenderPipelineDescriptor desc;
    desc.vertex.module = module;
    desc.cFragment.module = module;
    desc.cFragment.targetCount = 0;
    wgpu::DepthStencilState* dsState = desc.EnableDepthStencil();
    dsState->depthWriteEnabled = wgpu::OptionalBool::True;
    dsState->depthCompare = wgpu::CompareFunction::Always;

    device.CreateRenderPipeline(&desc);
}

// Having different block contents at the same binding point used in different stages is allowed.
TEST_P(ShaderTests, UniformAcrossStagesSameBindingPointCollide) {
    wgpu::ShaderModule module = utils::CreateShaderModule(device, R"(
        struct X { x : vec4f }
        struct Y { y : vec4i }

        @group(0) @binding(0) var<uniform> v : X;
        @group(0) @binding(0) var<uniform> f : Y;

        @vertex fn vertex() -> @builtin(position) vec4f {
            _ = v;
            return vec4f();
        }

        @fragment fn fragment() -> @location(0) vec4f {
            _ = f;
            return vec4f();
        }
    )");

    utils::ComboRenderPipelineDescriptor desc;
    desc.vertex.module = module;
    desc.cFragment.module = module;

    device.CreateRenderPipeline(&desc);
}

// Having different block contents at the same binding point used in different stages is allowed,
// with or without struct wrapper.
TEST_P(ShaderTests, UniformAcrossStagesSameBindingPointCollideMixedStructDef) {
    wgpu::ShaderModule module = utils::CreateShaderModule(device, R"(
        struct X { x : vec4f }

        @group(0) @binding(0) var<uniform> v : X;
        @group(0) @binding(0) var<uniform> f : vec3u;

        @vertex fn vertex() -> @builtin(position) vec4f {
            _ = v;
            return vec4f();
        }

        @fragment fn fragment() -> @location(0) vec4f {
            _ = f;
            return vec4f();
        }
    )");

    utils::ComboRenderPipelineDescriptor desc;
    desc.vertex.module = module;
    desc.cFragment.module = module;

    device.CreateRenderPipeline(&desc);
}

// Test that the `w` component of fragment builtin position behaves correctly.
TEST_P(ShaderTests, FragmentPositionW) {
    // TODO(crbug.com/dawn/2295): diagnose this failure on Pixel 4 OpenGLES
    DAWN_SUPPRESS_TEST_IF(IsOpenGLES() && IsAndroid() && IsQualcomm());

    wgpu::ShaderModule vsModule = utils::CreateShaderModule(device, R"(
@vertex fn main(@builtin(vertex_index) vertex_index : u32) -> @builtin(position) vec4f {
  let pos = array(
    vec4f(-1.0, -1.0, 0.0, 2.0),
    vec4f(-0.5,  0.0, 0.0, 2.0),
    vec4f( 0.0, -1.0, 0.0, 2.0),

    vec4f( 0.0, -1.0, 0.0, 4.0),
    vec4f( 0.5,  0.0, 0.0, 4.0),
    vec4f( 1.0, -1.0, 0.0, 4.0),

    vec4f(-0.5,  0.0, 0.0, 8.0),
    vec4f( 0.0,  1.0, 0.0, 8.0),
    vec4f( 0.5,  0.0, 0.0, 8.0),
  )[vertex_index];

  return vec4(pos.xy * pos.w, 0.0, pos.w);
})");

    wgpu::ShaderModule fsModule = utils::CreateShaderModule(device, R"(
@fragment fn main(@builtin(position) position : vec4f) -> @location(0) vec4f {
    return vec4f(position.w, 0.0, 1.0, 1.0);
})");

    utils::BasicRenderPass renderPass = utils::CreateBasicRenderPass(device, 64, 64);

    utils::ComboRenderPipelineDescriptor descriptor;
    descriptor.vertex.module = vsModule;
    descriptor.cFragment.module = fsModule;
    descriptor.primitive.topology = wgpu::PrimitiveTopology::TriangleList;
    descriptor.cTargets[0].format = renderPass.colorFormat;

    wgpu::RenderPipeline pipeline = device.CreateRenderPipeline(&descriptor);

    wgpu::CommandEncoder encoder = device.CreateCommandEncoder();
    wgpu::RenderPassEncoder pass = encoder.BeginRenderPass(&renderPass.renderPassInfo);
    pass.SetPipeline(pipeline);
    pass.Draw(9);
    pass.End();
    wgpu::CommandBuffer commands = encoder.Finish();
    queue.Submit(1, &commands);

    EXPECT_PIXEL_RGBA8_BETWEEN(utils::RGBA8(126, 0, 255, 255), utils::RGBA8(129, 0, 255, 255),
                               renderPass.color, 16, 48);
    EXPECT_PIXEL_RGBA8_BETWEEN(utils::RGBA8(62, 0, 255, 255), utils::RGBA8(65, 0, 255, 255),
                               renderPass.color, 48, 48);
    EXPECT_PIXEL_RGBA8_BETWEEN(utils::RGBA8(30, 0, 255, 255), utils::RGBA8(33, 0, 255, 255),
                               renderPass.color, 32, 16);
}

// Regression test for crbug.com/dawn/2340. GLSL requires the main enty point to be named "main".
// We need to make sure when the entry point is "main" or other GLSL reserved keyword,
// the renaming is always properly handled for the GL backend no matter what
// "disable_symbol_renaming" is.
TEST_P(ShaderTests, EntryPointShaderKeywordsComputePipeline) {
    {
        // Entry point is "main".
        std::string shader = R"(
@compute @workgroup_size(1) fn main() {
    _ = 1;
})";

        wgpu::ComputePipeline pipeline = CreateComputePipeline(shader, "main");
    }
    {
        // Entry point is a GLSL reserved keyword other than "main".
        std::string shader = R"(
@compute @workgroup_size(1) fn mat2() {
    _ = 1;
})";

        wgpu::ComputePipeline pipeline = CreateComputePipeline(shader, "mat2");
    }
    {
        // Entry point is not a GLSL reserved keyword.
        std::string shader = R"(
@compute @workgroup_size(1) fn foo1234() {
    _ = 1;
})";

        wgpu::ComputePipeline pipeline = CreateComputePipeline(shader, "foo1234");
    }
}

// Regression test for crbug.com/dawn/2340. GLSL requires the main enty point to be named "main".
// We need to make sure when the entry point is "main" or other GLSL reserved keyword,
// the renaming is always properly handled for the GL backend no matter what
// "disable_symbol_renaming" is.
TEST_P(ShaderTests, EntryPointShaderKeywordsRenderPipeline) {
    std::string shader = R"(
// Entry point is "main".
@vertex
fn main() -> @builtin(position) vec4f {
    return vec4f(0.0, 0.0, 0.0, 1.0);
}
// Entry point is a GLSL reserved keyword other than "main".
@fragment
fn mat2() -> @location(0) vec4f {
    return vec4f(0.0, 0.0, 0.0, 1.0);
}
// Entry point is not a GLSL reserved keyword.
@fragment
fn foo1234() -> @location(0) vec4f {
    return vec4f(1.0, 1.0, 1.0, 1.0);
}
)";
    wgpu::ShaderModule shaderModule = utils::CreateShaderModule(device, shader.c_str());
    utils::ComboRenderPipelineDescriptor rpDesc;
    rpDesc.vertex.module = shaderModule;
    rpDesc.cFragment.module = shaderModule;
    {
        rpDesc.cFragment.entryPoint = "mat2";
        wgpu::RenderPipeline pipeline = device.CreateRenderPipeline(&rpDesc);
    }
    {
        rpDesc.cFragment.entryPoint = "foo1234";
        wgpu::RenderPipeline pipeline = device.CreateRenderPipeline(&rpDesc);
    }
}

TEST_P(ShaderTests, PrivateVarInitWithStruct) {
    wgpu::ComputePipeline pipeline = CreateComputePipeline(R"(
@binding(0) @group(0) var<storage, read_write> output : i32;

struct S {
  i : i32,
}

var<private> P = S(42);

@compute @workgroup_size(1)
fn main() {
  output = P.i;
}
)");

    wgpu::Buffer output = CreateBuffer(1);

    wgpu::BindGroup bindGroup =
        utils::MakeBindGroup(device, pipeline.GetBindGroupLayout(0), {{0, output}});

    wgpu::CommandBuffer commands;
    {
        wgpu::CommandEncoder encoder = device.CreateCommandEncoder();
        wgpu::ComputePassEncoder pass = encoder.BeginComputePass();
        pass.SetPipeline(pipeline);
        pass.SetBindGroup(0, bindGroup);
        pass.DispatchWorkgroups(1);
        pass.End();

        commands = encoder.Finish();
    }

    queue.Submit(1, &commands);

    EXPECT_BUFFER_U32_EQ(42, output, 0);
}

TEST_P(ShaderTests, UnrestrictedPointerParameters) {
    // TODO(crbug.com/dawn/2350): Investigate, fix.
    DAWN_TEST_UNSUPPORTED_IF(IsD3D11());

    wgpu::ComputePipeline pipeline = CreateComputePipeline(R"(
@binding(0) @group(0) var<uniform> input : array<vec4i, 4>;
@binding(1) @group(0) var<storage, read_write> output : array<vec4i, 4>;

fn sum(f : ptr<function, i32>,
       w : ptr<workgroup, atomic<i32>>,
       p : ptr<private, i32>,
       u : ptr<uniform, vec4i>) -> vec4i {

  return vec4(*f + atomicLoad(w) + *p) + *u;
}

struct S {
  i : i32,
}

var<private> P0 = S(0);
var<private> P1 = S(10);
var<private> P2 = 20;
var<private> P3 = 30;

struct T {
  i : atomic<i32>,
}

var<workgroup> W0 : T;
var<workgroup> W1 : atomic<i32>;
var<workgroup> W2 : T;
var<workgroup> W3 : atomic<i32>;

@compute @workgroup_size(1)
fn main() {
  atomicStore(&W0.i, 0);
  atomicStore(&W1,   100);
  atomicStore(&W2.i, 200);
  atomicStore(&W3,   300);

  var F = array(0, 1000, 2000, 3000);

  output[0] = sum(&F[2], &W3,   &P1.i, &input[0]); // vec4(2310) + vec4(1, 2, 3, 4)
  output[1] = sum(&F[1], &W2.i, &P0.i, &input[1]); // vec4(1200) + vec4(4, 3, 2, 1)
  output[2] = sum(&F[3], &W0.i, &P3,   &input[2]); // vec4(3030) + vec4(2, 4, 1, 3)
  output[3] = sum(&F[2], &W1,   &P2,   &input[3]); // vec4(2120) + vec4(4, 1, 2, 3)
}
)");

    wgpu::Buffer input = CreateBuffer(
        std::vector<uint32_t>{
            1, 2, 3, 4,  // [0]
            4, 3, 2, 1,  // [1]
            2, 4, 1, 3,  // [2]
            4, 1, 2, 3,  // [3]
        },
        wgpu::BufferUsage::Uniform);

    std::vector<uint32_t> expected{
        2311, 2312, 2313, 2314,  // [0]
        1204, 1203, 1202, 1201,  // [1]
        3032, 3034, 3031, 3033,  // [2]
        2124, 2121, 2122, 2123,  // [3]
    };

    wgpu::Buffer output = CreateBuffer(expected.size());

    wgpu::BindGroup bindGroup =
        utils::MakeBindGroup(device, pipeline.GetBindGroupLayout(0), {{0, input}, {1, output}});

    wgpu::CommandBuffer commands;
    {
        wgpu::CommandEncoder encoder = device.CreateCommandEncoder();
        wgpu::ComputePassEncoder pass = encoder.BeginComputePass();
        pass.SetPipeline(pipeline);
        pass.SetBindGroup(0, bindGroup);
        pass.DispatchWorkgroups(1);
        pass.End();

        commands = encoder.Finish();
    }

    queue.Submit(1, &commands);

    EXPECT_BUFFER_U32_RANGE_EQ(expected.data(), output, 0, expected.size());
}

// A regression test for chromium:341282611. Test that Vulkan shader module cache should take the
// primitive type into account because for `PointList` we should generate `PointSize` in the SPIRV
// of the vertex shader.
TEST_P(ShaderTests, SameShaderModuleToRenderPointAndNonPoint) {
    std::string shader = R"(
@vertex
fn vs_main() -> @builtin(position) vec4f {
    return vec4f(0.0, 0.0, 0.0, 1.0);
}
@fragment
fn fs_main() -> @location(0) vec4f {
    return vec4f(0.0, 0.0, 0.0, 1.0);
}
)";
    wgpu::PipelineLayoutDescriptor layoutDesc = {};
    layoutDesc.bindGroupLayoutCount = 0;
    wgpu::PipelineLayout layout = device.CreatePipelineLayout(&layoutDesc);

    wgpu::ShaderModule shaderModule = utils::CreateShaderModule(device, shader.c_str());
    utils::ComboRenderPipelineDescriptor rpDesc;
    rpDesc.vertex.module = shaderModule;
    rpDesc.vertex.entryPoint = "vs_main";
    rpDesc.layout = layout;
    rpDesc.cFragment.module = shaderModule;
    rpDesc.cFragment.entryPoint = "fs_main";

    utils::BasicRenderPass renderPass = utils::CreateBasicRenderPass(device, 64, 64);
    rpDesc.cTargets[0].format = renderPass.colorFormat;

    wgpu::CommandEncoder encoder = device.CreateCommandEncoder();
    wgpu::RenderPassEncoder pass = encoder.BeginRenderPass(&renderPass.renderPassInfo);
    {
        rpDesc.primitive.topology = wgpu::PrimitiveTopology::TriangleList;
        wgpu::RenderPipeline pipeline = device.CreateRenderPipeline(&rpDesc);
        pass.SetPipeline(pipeline);
        pass.Draw(3);
    }
    {
        rpDesc.primitive.topology = wgpu::PrimitiveTopology::PointList;
        wgpu::RenderPipeline pipeline = device.CreateRenderPipeline(&rpDesc);
        pass.SetPipeline(pipeline);
        pass.Draw(1);
    }
    pass.End();
    wgpu::CommandBuffer commandBuffer = encoder.Finish();
    queue.Submit(1, &commandBuffer);
}

// Regression test for crbug.com/dawn/380433758.
TEST_P(ShaderTests, DuplicateTexture) {
    wgpu::ShaderModule module = utils::CreateShaderModule(device, R"(
fn sample(t: texture_2d<f32>, s: sampler) -> vec4f {
  return textureSampleLevel(t, s, vec2f(0), 0);
}

fn useCombos0() -> vec4f {
  return sample(tex0_0, smp0_0);
}

fn useCombos1() -> vec4f {
  return sample(tex1_15, smp0_0);
}

@group(0) @binding(0) var tex0_0: texture_2d<f32>;
@group(0) @binding(1) var tex1_15: texture_2d<f32>;
@group(0) @binding(2) var smp0_0: sampler;

@vertex fn vs() -> @builtin(position) vec4f {
  return useCombos0();
}

@fragment fn fs() -> @location(0) vec4f {
  return vec4f(useCombos1());
}
)");

    utils::ComboRenderPipelineDescriptor desc;
    desc.vertex.module = module;
    desc.cFragment.module = module;

    device.CreateRenderPipeline(&desc);
}

// Regression test for crbug.com/dawn/388870480.
TEST_P(ShaderTests, CollisionHandle) {
    DAWN_TEST_UNSUPPORTED_IF(GetSupportedLimits().maxStorageTexturesInVertexStage < 2);
    DAWN_TEST_UNSUPPORTED_IF(GetSupportedLimits().maxStorageTexturesInFragmentStage < 1);

    // TODO(crbug.com/394915257): ANGLE D3D11 bug
    DAWN_SUPPRESS_TEST_IF(IsANGLED3D11());

    wgpu::ShaderModule module = utils::CreateShaderModule(device, R"(
@group(0) @binding(1) var u0_1: texture_storage_2d<r32float, read>;
@group(0) @binding(0) var u0_0: texture_storage_2d<r32float, read>;

@group(0) @binding(0) var u1_0: texture_storage_2d<r32float, read>;

@vertex fn vs() -> @builtin(position) vec4f {
    _ = textureLoad(u0_0, vec2u(0));
    _ = textureLoad(u0_1, vec2u(0));
    return vec4f(0);
}

@fragment fn fs() -> @location(0) vec4f {
    _ = textureLoad(u1_0, vec2u(0));
    return vec4f(0);
}
)");

    utils::ComboRenderPipelineDescriptor desc;
    desc.vertex.module = module;
    desc.cFragment.module = module;

    device.CreateRenderPipeline(&desc);
}

// Regression test for crbug.com/dawn/388870480.
// Trigger potential collision when disable_symbol_renaming is on
TEST_P(ShaderTests, CollisionHandle_DifferentModules) {
    DAWN_TEST_UNSUPPORTED_IF(GetSupportedLimits().maxStorageTexturesInVertexStage < 2);
    DAWN_TEST_UNSUPPORTED_IF(GetSupportedLimits().maxStorageTexturesInFragmentStage < 1);

    // TODO(crbug.com/394915257): ANGLE D3D11 bug
    DAWN_SUPPRESS_TEST_IF(IsANGLED3D11());

    wgpu::ShaderModule vmodule = utils::CreateShaderModule(device, R"(
@group(0) @binding(1) var v1: texture_storage_2d<r32float, read>;
@group(0) @binding(0) var v0: texture_storage_2d<r32float, read>;

@vertex fn vs() -> @builtin(position) vec4f {
    _ = textureLoad(v0, vec2u(0));
    _ = textureLoad(v1, vec2u(0));
    return vec4f(0);
}
)");

    wgpu::ShaderModule fmodule = utils::CreateShaderModule(device, R"(
@group(0) @binding(1) var v1: texture_storage_2d<r32float, read>;

@fragment fn fs() -> @location(0) vec4f {
    _ = textureLoad(v1, vec2u(0));
    return vec4f(0);
}
)");

    utils::ComboRenderPipelineDescriptor desc;
    desc.vertex.module = vmodule;
    desc.cFragment.module = fmodule;

    device.CreateRenderPipeline(&desc);
}

DAWN_INSTANTIATE_TEST(ShaderTests,
                      D3D11Backend(),
                      D3D12Backend(),
                      D3D12Backend({"use_dxc"}),
                      MetalBackend(),
                      OpenGLBackend(),
                      OpenGLESBackend(),
                      OpenGLBackend({"disable_symbol_renaming"}),
                      OpenGLESBackend({"disable_symbol_renaming"}),
                      VulkanBackend());

}  // anonymous namespace
}  // namespace dawn
