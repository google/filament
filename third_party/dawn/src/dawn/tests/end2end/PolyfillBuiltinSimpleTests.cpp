// Copyright 2025 The Dawn & Tint Authors
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

#include <cstdint>
#include <limits>
#include <string>
#include <vector>

#include "src/dawn/tests/DawnTest.h"
#include "src/dawn/utils/WGPUHelpers.h"

namespace dawn {
namespace {

class PolyfillBuiltinSimpleTests : public DawnTest {
  public:
    std::vector<wgpu::FeatureName> GetRequiredFeatures() override {
        std::vector<wgpu::FeatureName> features;
        if (SupportsFeatures({wgpu::FeatureName::ShaderF16})) {
            features.push_back(wgpu::FeatureName::ShaderF16);
        }
        return features;
    }

    wgpu::Buffer CreateBuffer(const std::vector<uint32_t>& data,
                              wgpu::BufferUsage usage = wgpu::BufferUsage::Storage |
                                                        wgpu::BufferUsage::CopySrc) {
        uint64_t bufferSize = static_cast<uint64_t>(data.size() * sizeof(uint32_t));
        return utils::CreateBufferFromData(device, data.data(), bufferSize, usage);
    }

    wgpu::Buffer CreateBuffer(const std::vector<float>& data,
                              wgpu::BufferUsage usage = wgpu::BufferUsage::Storage |
                                                        wgpu::BufferUsage::CopySrc) {
        uint64_t bufferSize = static_cast<uint64_t>(data.size() * sizeof(float));
        return utils::CreateBufferFromData(device, data.data(), bufferSize, usage);
    }

    wgpu::Buffer CreateBuffer(const uint32_t count,
                              const uint32_t default_val = 0,
                              wgpu::BufferUsage usage = wgpu::BufferUsage::Storage |
                                                        wgpu::BufferUsage::CopySrc) {
        return CreateBuffer(std::vector<uint32_t>(count, default_val), usage);
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

TEST_P(PolyfillBuiltinSimpleTests, ScalarizeClampBuiltinNanComponent) {
    // Some devices (Adreno) do not handle nan's correctly for the clamp function
    // This test will fail on those devices without the builtin polyfill/scalarize
    //  applied. See: crbug.com/407109052
    std::string kShaderCode = R"(
    @group(0) @binding(0) var<storage, read_write> in_out : array<u32, 2>;
    @compute @workgroup_size(1)
    fn main() {
        var zero = f32(in_out[0]);
        var x = vec2(0.0/zero, 1.0);
        var q = clamp(x, vec2(0.0), vec2(1.0));
        in_out[1] = u32(q.y);
    }
    )";

    wgpu::ComputePipeline pipeline = CreateComputePipeline(kShaderCode);
    uint32_t kDefaultVal = 0;
    wgpu::Buffer output = CreateBuffer(2, kDefaultVal);
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
    std::vector<uint32_t> expected = {0, 1};
    EXPECT_BUFFER_U32_RANGE_EQ(expected.data(), output, 0, expected.size());
}

TEST_P(PolyfillBuiltinSimpleTests, ScalarizeClampBuiltin) {
    // Basic correctness test for scalariztion of clamp.
    std::string kShaderCode = R"(
    @group(0) @binding(0) var<storage, read_write> in_out : array<u32, 2>;
    @compute @workgroup_size(1)
    fn main() {
        var x = vec2(5.0, -2.0);
        var q = clamp(x, vec2(0.0), vec2(1.0));
        in_out[0] = u32(q.x);
        in_out[1] = u32(q.y);
    }
    )";

    wgpu::ComputePipeline pipeline = CreateComputePipeline(kShaderCode);
    uint32_t kDefaultVal = 0;
    wgpu::Buffer output = CreateBuffer(2, kDefaultVal);
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
    std::vector<uint32_t> expected = {1, 0};
    EXPECT_BUFFER_U32_RANGE_EQ(expected.data(), output, 0, expected.size());
}

TEST_P(PolyfillBuiltinSimpleTests, ScalarizeMinMaxBuiltin) {
    // Basic correctness test for scalariztion of min and max.
    std::string kShaderCode = R"(
    @group(0) @binding(0) var<storage, read_write> in_out : array<u32, 2>;
    @compute @workgroup_size(1)
    fn main() {
        var x = vec2(5.0, -2.0);
        var q = min(vec2(3.0), max(x, vec2(2.0)));
        in_out[0] = u32(q.x);
        in_out[1] = u32(q.y);
    }
    )";

    wgpu::ComputePipeline pipeline = CreateComputePipeline(kShaderCode);
    uint32_t kDefaultVal = 0;
    wgpu::Buffer output = CreateBuffer(2, kDefaultVal);
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
    std::vector<uint32_t> expected = {3, 2};
    EXPECT_BUFFER_U32_RANGE_EQ(expected.data(), output, 0, expected.size());
}

TEST_P(PolyfillBuiltinSimpleTests, AbsWithBranch) {
    // Some backend compilers assume that return value of 'abs' is always positive. This is
    // not true for one specific value of i32 (0x8000'0000).
    // Operations on the value returned can prove that the compiler is assuming this value is
    // positive. See crbug.com/426999765
    std::string kShaderCode = R"(
    struct Data { values: array<i32> };
    @group(0) @binding(0) var<storage, read> input_data: Data;
    @group(0) @binding(1) var<storage, read_write> output_data: Data;

    @compute @workgroup_size(4)
    fn main(@builtin(global_invocation_id) global_id: vec3<u32>) {
        var result = input_data.values[global_id.x];
        // Translates to SAbs ext instruction (spriv)
        result = abs(result);
        // Will translate to SMax ext instruction (spriv) and reproduce the bug.
        // result = max(result, 3488);
        // Another way to test the compiler is to use a conditional.
        // The compiler incorrectly assumes 'result' is positive.
        if(result < 0){
            // This branch will (correctly) be taken iff original value was min i32.
            result = 1543;
        }
        // try 2
        output_data.values[global_id.x]  = result;
    }
    )";

    wgpu::ComputePipeline pipeline = CreateComputePipeline(kShaderCode);
    uint32_t kDefaultVal = 0;
    std::vector<uint32_t> init_input = {uint32_t(std::numeric_limits<int32_t>::lowest()),
                                        uint32_t(-15), 17, 123};

    wgpu::Buffer input = CreateBuffer(init_input);
    wgpu::Buffer output = CreateBuffer(4, kDefaultVal);
    wgpu::BindGroup bindGroup =
        utils::MakeBindGroup(device, pipeline.GetBindGroupLayout(0), {{0, input}, {1, output}});

    wgpu::CommandBuffer commands;
    {
        wgpu::CommandEncoder encoder = device.CreateCommandEncoder();
        wgpu::ComputePassEncoder pass = encoder.BeginComputePass();
        pass.SetPipeline(pipeline);
        pass.SetBindGroup(0, bindGroup);
        pass.DispatchWorkgroups(64);
        pass.End();
        commands = encoder.Finish();
    }

    queue.Submit(1, &commands);
    std::vector<uint32_t> expected = {1543, 15, 17, 123};

    EXPECT_BUFFER_U32_RANGE_EQ(expected.data(), output, 0, expected.size());
}

TEST_P(PolyfillBuiltinSimpleTests, CaseSwitchToIf) {
    // TODO(crbug.com/459848839): Fails on Win/Snapdragon X Elite.
    DAWN_SUPPRESS_TEST_IF(IsWindows() && IsQualcomm() && IsD3D11());
    DAWN_SUPPRESS_TEST_IF(IsWindows() && IsQualcomm() && IsD3D12() && !IsDXC());

    std::string kShaderCode = R"(
    struct Data { values: array<i32> };
    @group(0) @binding(0) var<storage, read> input_data: Data;
    @group(0) @binding(1) var<storage, read_write> output_data: Data;

    @compute @workgroup_size(4)
    fn main(@builtin(global_invocation_id) global_id: vec3<u32>) {
        var input_ = input_data.values[global_id.x];
        var ret = 0i;
        switch( input_ ) {
            case 1: {
                ret = 3;
            }
            case 2:{
                ret = 7;
            }
            case -2147483648:{
                ret = 71;
            }
            case 123, 87:{
                ret = 11;
            }
            case -1:{
                ret = 33;
            }
            default {
                ret = 82;
            }
        }
        output_data.values[global_id.x]  = ret;
    }
    )";

    wgpu::ComputePipeline pipeline = CreateComputePipeline(kShaderCode);
    uint32_t kDefaultVal = 0;
    std::vector<uint32_t> init_input = {uint32_t(std::numeric_limits<int32_t>::lowest()),
                                        uint32_t(-15), 17, 123};

    wgpu::Buffer input = CreateBuffer(init_input);
    wgpu::Buffer output = CreateBuffer(4, kDefaultVal);
    wgpu::BindGroup bindGroup =
        utils::MakeBindGroup(device, pipeline.GetBindGroupLayout(0), {{0, input}, {1, output}});

    wgpu::CommandBuffer commands;
    {
        wgpu::CommandEncoder encoder = device.CreateCommandEncoder();
        wgpu::ComputePassEncoder pass = encoder.BeginComputePass();
        pass.SetPipeline(pipeline);
        pass.SetBindGroup(0, bindGroup);
        pass.DispatchWorkgroups(64);
        pass.End();
        commands = encoder.Finish();
    }

    queue.Submit(1, &commands);
    std::vector<uint32_t> expected = {71, 82, 82, 11};

    EXPECT_BUFFER_U32_RANGE_EQ(expected.data(), output, 0, expected.size());
}

TEST_P(PolyfillBuiltinSimpleTests, CaseSwitchToIfComplex) {
    // TODO(crbug.com/459848839): Fails on Win/Snapdragon X Elite.
    DAWN_SUPPRESS_TEST_IF(IsWindows() && IsQualcomm() && IsD3D11());
    DAWN_SUPPRESS_TEST_IF(IsWindows() && IsQualcomm() && IsD3D12() && !IsDXC());

    std::string kShaderCode = R"(
    @group(0) @binding(0) var<storage, read> input_data: array<i32>;
    @group(0) @binding(1) var<storage, read_write> output_data: array<i32>;

    @compute @workgroup_size(4)
    fn main(@builtin(global_invocation_id) global_id: vec3<u32>) {
        var input_ = input_data[global_id.x];
        var ret = 0i;
        switch( input_ ) {
            case 1: {
                ret = 3;
            }
            case -2:{
                switch(input_){
                    case 1: {
                        ret = 3;
                    }
                    case -2:{
                        ret = 4;
                    }
                    default{
                        ret = 99;
                    }
                }
                break;
                ret = 7;
            }
            case -2147483648:{
                if(input_ == 17){
                    ret = 71;
                    break;
                }
                ret = 13;
            }
            case 3, 5:{
                if(input_ == 3){
                    break;
                }
                ret = 11;
            }
            default {
                ret = 82;
            }
        }
        output_data[global_id.x]  = ret;
    }
    )";

    wgpu::ComputePipeline pipeline = CreateComputePipeline(kShaderCode);
    uint32_t kDefaultVal = 0;
    std::vector<uint32_t> init_input = {uint32_t(std::numeric_limits<int32_t>::lowest()),
                                        uint32_t(-2), 3, 5};
    std::vector<uint32_t> expected = {13, 4, 0, 11};
    wgpu::Buffer input = CreateBuffer(init_input);
    wgpu::Buffer output = CreateBuffer(4, kDefaultVal);
    wgpu::BindGroup bindGroup =
        utils::MakeBindGroup(device, pipeline.GetBindGroupLayout(0), {{0, input}, {1, output}});

    wgpu::CommandBuffer commands;
    {
        wgpu::CommandEncoder encoder = device.CreateCommandEncoder();
        wgpu::ComputePassEncoder pass = encoder.BeginComputePass();
        pass.SetPipeline(pipeline);
        pass.SetBindGroup(0, bindGroup);
        pass.DispatchWorkgroups(64);
        pass.End();
        commands = encoder.Finish();
    }

    queue.Submit(1, &commands);

    EXPECT_BUFFER_U32_RANGE_EQ(expected.data(), output, 0, expected.size());
}

// Some versions of AMD Mesa (prior to 25.3) have a front-end optimizer bug where unary negation
// and abs operations on floating point values (f32 and f16) are incorrectly optimized or
// handled, leading to incorrect results.
// See crbug.com/448294721 and crbug.com/500099471.
TEST_P(PolyfillBuiltinSimpleTests, PolyfillFloatUnary) {
    bool hasF16 = device.HasFeature(wgpu::FeatureName::ShaderF16);

    std::string shader = R"(
        @group(0) @binding(0) var<storage, read> in_f32 : array<f32, 4>;
        @group(0) @binding(1) var<storage, read_write> out_f32 : array<f32, 8>;

        @compute @workgroup_size(1)
        fn main() {
            out_f32[0] = abs(in_f32[0]);
            out_f32[1] = -in_f32[1];
            out_f32[2] = length(in_f32[2]);
            out_f32[3] = distance(in_f32[3], 2.0);
    )";

    if (hasF16) {
        shader = "enable f16;\n" + shader;
        shader += R"(
            out_f32[4] = f32(abs(f16(in_f32[0])));
            out_f32[5] = f32(-f16(in_f32[1]));
            out_f32[6] = f32(length(f16(in_f32[2])));
            out_f32[7] = f32(distance(f16(in_f32[3]), 2.0h));
        )";
    }

    shader += R"(
        }
    )";

    wgpu::ComputePipeline pipeline = CreateComputePipeline(shader);

    std::vector<float> input_data = {-1.5f, 2.0f, -2.5f, 1.0f};
    wgpu::Buffer input = CreateBuffer(input_data, wgpu::BufferUsage::Storage);
    wgpu::Buffer output = CreateBuffer(8, 0);
    wgpu::BindGroup bindGroup = utils::MakeBindGroup(device, pipeline.GetBindGroupLayout(0),
                                                     {
                                                         {0, input},
                                                         {1, output},
                                                     });

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

    std::vector<float> expected = {1.5f, -2.0f, 2.5f, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f};
    if (hasF16) {
        expected[4] = 1.5f;
        expected[5] = -2.0f;
        expected[6] = 2.5f;
        expected[7] = 1.0f;
    }

    EXPECT_BUFFER_FLOAT_RANGE_EQ(expected.data(), output, 0, expected.size());
}

// Test dynamic indexing into a boolean vector. This is a minimal reproducer for a suspected driver
// bug on Intel Mac (crbug.com/540789158).
TEST_P(PolyfillBuiltinSimpleTests, BoolVectorDynamicIndexStore) {
    // Initial v is <true, true, true>.
    std::string shader = R"(
        struct Input {
            index : i32,
            value : u32,
        }

        @group(0) @binding(0) var<storage, read> input : Input;
        @group(0) @binding(1) var<storage, read_write> output : array<u32, 3>;

        @compute @workgroup_size(1) fn main() {
            var v = vec3<bool>(true, true, true);
            v[input.index] = (input.value == 0u);

            // Store result to output buffer as uints.
            output[0] = select(0u, 1u, v.x);
            output[1] = select(0u, 1u, v.y);
            output[2] = select(0u, 1u, v.z);
        }
    )";

    wgpu::ComputePipeline pipeline = CreateComputePipeline(shader.c_str(), "main");

    // Write false (indicated by non-zero value) to index 2, so v should be <true, true, false>.
    std::vector<uint32_t> inputData = {2, 1u};
    wgpu::Buffer inputBuffer = CreateBuffer(inputData, wgpu::BufferUsage::Storage);

    wgpu::Buffer outputBuffer = CreateBuffer(3);

    wgpu::BindGroup bindGroup = utils::MakeBindGroup(device, pipeline.GetBindGroupLayout(0),
                                                     {{0, inputBuffer}, {1, outputBuffer}});

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

    // Expected final v is <true, true, false> which is stored as <1, 1, 0>.
    std::vector<uint32_t> expected = {1u, 1u, 0u};
    EXPECT_BUFFER_U32_RANGE_EQ(expected.data(), outputBuffer, 0, 3);
}

DAWN_INSTANTIATE_TEST(PolyfillBuiltinSimpleTests,
                      D3D12Backend(),
                      D3D11Backend(),
                      MetalBackend(),
                      VulkanBackend(),
                      WebGPUBackend(),
                      D3D12Backend({"scalarize_max_min_clamp"}),
                      MetalBackend({"scalarize_max_min_clamp"}),
                      MetalBackend({"metal_polyfill_bool_vec_dynamic_store"}),
                      VulkanBackend({"scalarize_max_min_clamp"}),
                      VulkanBackend({"vulkan_polyfill_switch_with_if"}),
                      VulkanBackend({"spirv_polyfill_float_negation", "spirv_polyfill_float_abs"}),
                      D3D11Backend({"scalarize_max_min_clamp"}),
                      OpenGLESBackend());

}  // anonymous namespace
}  // namespace dawn
