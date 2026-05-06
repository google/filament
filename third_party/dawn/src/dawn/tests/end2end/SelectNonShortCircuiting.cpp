// Copyright 2026 The Dawn & Tint Authors
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

#include <array>
#include <string>
#include <vector>

#include "dawn/tests/DawnTest.h"
#include "dawn/utils/WGPUHelpers.h"

namespace dawn {
namespace {

class SelectNonShortCircuitingTest : public DawnTest {};

TEST_P(SelectNonShortCircuitingTest, SelectWithConstantCondition) {
    const char* shader = R"(
        struct StorageBuffer {
            a: i32,
        }

        @group(0) @binding(0) var<storage, read_write> s_output: StorageBuffer;

        var<private> g: i32 = 0i;

        fn func_1() -> bool {
            g = 1i;
            return false;
        }

        @compute @workgroup_size(1)
        fn main() {
            var var_1 = select(func_1(), false, true);
            s_output.a = g;
        }
    )";

    wgpu::ShaderModule csModule = utils::CreateShaderModule(device, shader);

    wgpu::ComputePipelineDescriptor pipelineDesc = {};
    pipelineDesc.compute.module = csModule;
    pipelineDesc.compute.entryPoint = "main";
    wgpu::ComputePipeline pipeline = device.CreateComputePipeline(&pipelineDesc);

    wgpu::Buffer outputBuffer = utils::CreateBufferFromData<int32_t>(
        device, wgpu::BufferUsage::Storage | wgpu::BufferUsage::CopySrc, {0});

    wgpu::BindGroup bindGroup =
        utils::MakeBindGroup(device, pipeline.GetBindGroupLayout(0), {{0, outputBuffer}});

    wgpu::CommandEncoder encoder = device.CreateCommandEncoder();
    wgpu::ComputePassEncoder pass = encoder.BeginComputePass();
    pass.SetPipeline(pipeline);
    pass.SetBindGroup(0, bindGroup);
    pass.DispatchWorkgroups(1);
    pass.End();
    wgpu::CommandBuffer commands = encoder.Finish();
    queue.Submit(1, &commands);

    // Expect that `g` is 1, because the call to `func_1` should have been executed.
    EXPECT_BUFFER_U32_EQ(1, outputBuffer, 0);
}

DAWN_INSTANTIATE_TEST(SelectNonShortCircuitingTest,
                      D3D11Backend(),
                      D3D12Backend(),
                      MetalBackend(),
                      OpenGLBackend(),
                      OpenGLESBackend(),
                      VulkanBackend());

}  // namespace
}  // namespace dawn
