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
#include <string>
#include <vector>

#include "dawn/tests/DawnTest.h"
#include "dawn/utils/WGPUHelpers.h"
#include "gtest/gtest.h"

namespace dawn {
namespace {

class RuntimeTimeoutComputeTest : public DawnTest {
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

// This test will take up to 5+ seconds to execute if we do not have module constant hoisting
// (transform) enabled on Apple silicon. See crbug.com/413690572
TEST_P(RuntimeTimeoutComputeTest, LargeConstantArrayInLoop) {
    DAWN_SKIP_TEST_IF_BASE(IsCPU() || IsWARP(), "timeout",
                           "skipping because software rendering is slow");
    // Test disable due to CI failures. See crbug.com/421869294
    DAWN_SKIP_TEST_IF_BASE(IsIntel(), "timeout", "device found to be slow on CI");

    std::string kShaderCode = R"(
@group(0)@binding(0) var < storage,read_write > A: array < i32 >;
@compute @workgroup_size(256)
fn main(@builtin(local_invocation_index) g: u32) {
  var offset = g;
  var b = i32(offset);
      for (var i = 0;  i < 20; i++) {
        var n = array(13, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
         1, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 13, 0, 0, 0,
         13, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 23, 0, 0, 0, 20, 0, 0, 0,
          0, 0, 0, 0, 0, 0, 0, 0, 13, 0, 13, 0, 13, 0, 13, 0, 0, 0, 0,
          0, 0, 0, 0, 0, 25, 0, 20, 0, 23, 0, 20, 0, 0, 0, 0, 0, 0, 0,
          0, 0, 13, 0, 0, 0, 13, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 23, 0,
          0, 0, 20, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 13, 0, 13, 0, 13, 0,
          1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 23, 0, 20, 0, 18, 0, 20, 0, 11, 18,
           15, 18, 23, 18, 15, 18, 10, 18, 13, 18, 22, 18, 13, 18, 10, 13,
           18, 22, 23, 22, 18, 13, 13, 0, 0, 0, 13, 0, 0, 0, 0, 0, 0, 0, 0,
            0, 0, 0, 1, 0, 0, 0, 1, 1, 0, 1, 0, 13, 0, 11, 0, 9, 0, 11, 0,
            13, 0, 11, 0, 9, 0, 8, 0, 13, 1, 13, 8, 13, 8, 13, 8, 13, 8, 16,
            11, 13, 8, 16, 11, 13, 8, 13, 1, 13, 18, 16, 8, 13, 18, 16, 8,
            13, 18, 16, 8, 13, 18, 16, 8, 13, 13, 11, 18, 13, 13, 11, 18, 13, 1)[i];
         b +=  n;
      }
  A[offset] = b;
}
    )";

    StartTestTimer(0.5f);
    wgpu::ComputePipeline pipeline = CreateComputePipeline(kShaderCode);
    uint32_t kDefaultVal = 0;
    uint32_t kDispatchSizeDim = 64;
    uint32_t kBufferNumElements = 256 * kDispatchSizeDim;
    wgpu::Buffer output = CreateBuffer(kBufferNumElements, kDefaultVal);
    wgpu::BindGroup bindGroup =
        utils::MakeBindGroup(device, pipeline.GetBindGroupLayout(0), {{0, output}});

    wgpu::CommandBuffer commands;
    {
        wgpu::CommandEncoder encoder = device.CreateCommandEncoder();
        wgpu::ComputePassEncoder pass = encoder.BeginComputePass();
        pass.SetPipeline(pipeline);
        pass.SetBindGroup(0, bindGroup);

        pass.DispatchWorkgroups(kDispatchSizeDim, kDispatchSizeDim, kDispatchSizeDim);
        pass.End();
        commands = encoder.Finish();
    }

    queue.Submit(1, &commands);
    std::vector<uint32_t> expected = {15, 16};
    EXPECT_BUFFER_U32_RANGE_EQ(expected.data(), output, 0, expected.size());
}

DAWN_INSTANTIATE_TEST(RuntimeTimeoutComputeTest, D3D12Backend(), MetalBackend(), VulkanBackend());

}  // anonymous namespace
}  // namespace dawn
