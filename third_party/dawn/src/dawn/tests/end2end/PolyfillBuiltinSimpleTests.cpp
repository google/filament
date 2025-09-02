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

#include "dawn/tests/DawnTest.h"
#include "dawn/utils/WGPUHelpers.h"

namespace dawn {
namespace {

class PolyfillBuiltinSimpleTests : public DawnTest {
  public:
    wgpu::Buffer CreateBuffer(const std::vector<uint32_t>& data,
                              wgpu::BufferUsage usage = wgpu::BufferUsage::Storage |
                                                        wgpu::BufferUsage::CopySrc) {
        uint64_t bufferSize = static_cast<uint64_t>(data.size() * sizeof(uint32_t));
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

DAWN_INSTANTIATE_TEST(PolyfillBuiltinSimpleTests,
                      D3D12Backend(),
                      D3D11Backend(),
                      MetalBackend(),
                      VulkanBackend(),
                      D3D12Backend({"scalarize_max_min_clamp"}),
                      MetalBackend({"scalarize_max_min_clamp"}),
                      VulkanBackend({"scalarize_max_min_clamp"}),
                      D3D11Backend({"scalarize_max_min_clamp"}),
                      OpenGLESBackend());

}  // anonymous namespace
}  // namespace dawn
