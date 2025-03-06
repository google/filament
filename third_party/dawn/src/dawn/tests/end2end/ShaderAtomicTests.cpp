// Copyright 2024 The Dawn & Tint Authors
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
#include <numeric>
#include <string>
#include <vector>

#include "dawn/common/GPUInfo.h"
#include "dawn/tests/DawnTest.h"
#include "dawn/utils/ComboRenderPipelineDescriptor.h"
#include "dawn/utils/WGPUHelpers.h"

// The motivation behind these tests is to investigate the failures in
// webgpu:shader,execution,expression,call,builtin,atomics,atomic* CTS for mobile gpus.

namespace dawn {
namespace {

enum class ShaderAtomicOp : uint8_t {
    AtomicAdd,
    AtomicCASFakeAdd,
};

std::ostream& operator<<(std::ostream& o, ShaderAtomicOp shader_op) {
    switch (shader_op) {
        case ShaderAtomicOp::AtomicAdd:
            o << "AtomicAdd";
            break;
        case ShaderAtomicOp::AtomicCASFakeAdd:
            o << "AtomicCASFakeAdd";
            break;
    }
    return o;
}

using WorkgroupSizeParameter = int;
using DispatchSizeParameter = int;
using ShaderAtomicUseArray = bool;
DAWN_TEST_PARAM_STRUCT(SubgroupsShaderTestsParams,
                       ShaderAtomicUseArray,
                       WorkgroupSizeParameter,
                       DispatchSizeParameter,
                       ShaderAtomicOp);

class ShaderAtomicTests : public DawnTestWithParams<SubgroupsShaderTestsParams> {
  public:
    using DawnTestWithParams<SubgroupsShaderTestsParams>::GetParam;
    using DawnTestWithParams<SubgroupsShaderTestsParams>::SupportsFeatures;
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

TEST_P(ShaderAtomicTests, WorkgroupAtomicArray) {
    // Suppression for Mali gpus.
    DAWN_SUPPRESS_TEST_IF(gpu_info::IsARM(GetParam().adapterProperties.vendorID));

    // Test code only supports up to 256 workgroup size.
    DAWN_ASSERT(GetParam().mWorkgroupSizeParameter <= 256);
    std::string kConditionalArray = GetParam().mShaderAtomicUseArray ? "[0]" : "";
    std::string kAtomicAddOperation =
        R"(  atomicAdd(&workgroup_buffer)" + kConditionalArray + R"(, 1);)";
    std::string kAtomicCASFakeAddOperation = R"(
        var curr = atomicLoad(&workgroup_buffer)" +
                                             kConditionalArray + R"();
        var next = curr + 1;
        var result = atomicCompareExchangeWeak(&workgroup_buffer)" +
                                             kConditionalArray + R"(, curr, next);
        while(!result.exchanged){
            curr = result.old_value;
            next = curr + 1;
            result = atomicCompareExchangeWeak(&workgroup_buffer)" +
                                             kConditionalArray + R"(, curr, next);
        }
    )";
    std::stringstream code;
    code << R"(
@binding(0) @group(0) var<storage, read_write> output : array<u32>;
// The bug requires an array to manifest.
)"
         << (GetParam().mShaderAtomicUseArray
                 ? R"(var<workgroup> workgroup_buffer : array<atomic<u32>, 16>; )"
                 : R"(var<workgroup> workgroup_buffer : atomic<u32>; )")
         <<
        R"(
@compute @workgroup_size( )"
         << GetParam().mWorkgroupSizeParameter
         << R"(  )
fn main(@builtin(local_invocation_index) local_invocation_index: u32,
        @builtin(workgroup_id) workgroup_id : vec3<u32>){
  if (local_invocation_index == 0) {
     atomicStore(&workgroup_buffer)" +
                kConditionalArray + R"(, 7);
  }

  workgroupBarrier();
  )"
         << (GetParam().mShaderAtomicOp == ShaderAtomicOp::AtomicAdd ? kAtomicAddOperation
                                                                     : kAtomicCASFakeAddOperation)
         <<

        R"(

  workgroupBarrier();

  if (local_invocation_index == 0) {
     output[workgroup_id.x] = atomicLoad(&workgroup_buffer)" +
            kConditionalArray + R"();
  }
}
)";

    wgpu::ComputePipeline pipeline = CreateComputePipeline(code.str());
    constexpr int kInitStartValueShader = 7;
    std::vector<uint32_t> expected(
        GetParam().mDispatchSizeParameter,
        static_cast<uint32_t>(GetParam().mWorkgroupSizeParameter + kInitStartValueShader));

    wgpu::Buffer output = CreateBuffer(GetParam().mDispatchSizeParameter, -1);
    wgpu::BindGroup bindGroup =
        utils::MakeBindGroup(device, pipeline.GetBindGroupLayout(0), {{0, output}});

    wgpu::CommandBuffer commands;
    {
        wgpu::CommandEncoder encoder = device.CreateCommandEncoder();
        wgpu::ComputePassEncoder pass = encoder.BeginComputePass();
        pass.SetPipeline(pipeline);
        pass.SetBindGroup(0, bindGroup);
        pass.DispatchWorkgroups(GetParam().mDispatchSizeParameter);
        pass.End();

        commands = encoder.Finish();
    }

    queue.Submit(1, &commands);

    EXPECT_BUFFER_U32_RANGE_EQ(expected.data(), output, 0, expected.size());
}

DAWN_INSTANTIATE_TEST_P(ShaderAtomicTests,
                        /*Supporting only modern graphics backends for now.*/
                        {D3D12Backend(), MetalBackend(), VulkanBackend()},
                        {true, false}, /*use shader array*/
                        {1,  2,  3,  4,  5,  6,   7,   8,   9,   13, 15,
                         16, 31, 32, 53, 64, 111, 128, 137, 173, 256}, /* workgroup size*/
                        {
                            1,
                        }, /*dispatch size */
                        {ShaderAtomicOp::AtomicAdd, ShaderAtomicOp::AtomicCASFakeAdd});

}  // anonymous namespace
}  // namespace dawn
