// Copyright 2023 The Dawn & Tint Authors
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

#include <string>
#include <vector>

#include "dawn/tests/perf_tests/DawnPerfTest.h"
#include "dawn/utils/WGPUHelpers.h"

namespace dawn {
namespace {

constexpr unsigned int kNumIterations = 50;
constexpr uint32_t kWorkgroupArraySize = 2048u;

struct ZeroInitializeWorkgroupMemoryParams : AdapterTestParam {
    ZeroInitializeWorkgroupMemoryParams(const AdapterTestParam& param, uint32_t workgroupSize)
        : AdapterTestParam(param), workgroupSize(workgroupSize) {}
    uint32_t workgroupSize;
};

std::ostream& operator<<(std::ostream& ostream, const ZeroInitializeWorkgroupMemoryParams& param) {
    ostream << static_cast<const AdapterTestParam&>(param);
    ostream << "_workgroupSize_" << param.workgroupSize;
    return ostream;
}

class VulkanZeroInitializeWorkgroupMemoryExtensionTest
    : public DawnPerfTestWithParams<ZeroInitializeWorkgroupMemoryParams> {
  public:
    VulkanZeroInitializeWorkgroupMemoryExtensionTest()
        : DawnPerfTestWithParams<ZeroInitializeWorkgroupMemoryParams>(kNumIterations, 1) {}

    ~VulkanZeroInitializeWorkgroupMemoryExtensionTest() override = default;

    void SetUp() override;

  private:
    void Step() override;

    wgpu::BindGroup mBindGroup;
    wgpu::ComputePipeline mPipeline;
};

void VulkanZeroInitializeWorkgroupMemoryExtensionTest::SetUp() {
    DawnPerfTestWithParams<ZeroInitializeWorkgroupMemoryParams>::SetUp();

    std::ostringstream ostream;
    ostream << R"(
        @group(0) @binding(0) var<storage, read_write> dst : array<f32>;

        const kWorkgroupSize = )"
            << GetParam().workgroupSize << R"(;
        const kWorkgroupArraySize = )"
            << kWorkgroupArraySize << R"(;
        const kLoopLength = kWorkgroupArraySize / kWorkgroupSize;

        var<workgroup> mm_Asub : array<f32, kWorkgroupArraySize>;
        @compute @workgroup_size(kWorkgroupSize)
        fn main(@builtin(local_invocation_id) LocalInvocationID : vec3u) {
            for (var k = 0u; k < kLoopLength; k = k + 1u) {
                var index = kLoopLength * LocalInvocationID.x + k;
                dst[index] = mm_Asub[index];
            }
        })";
    wgpu::BindGroupLayout bindGroupLayout = utils::MakeBindGroupLayout(
        device, {{0, wgpu::ShaderStage::Compute, wgpu::BufferBindingType::Storage, true}});

    wgpu::ComputePipelineDescriptor csDesc;
    csDesc.layout = utils::MakePipelineLayout(device, {bindGroupLayout});
    csDesc.compute.module = utils::CreateShaderModule(device, ostream.str().c_str());
    mPipeline = device.CreateComputePipeline(&csDesc);

    std::array<float, kWorkgroupArraySize * kNumIterations> data;
    data.fill(1);
    wgpu::Buffer buffer = utils::CreateBufferFromData(
        device, data.data(), sizeof(data), wgpu::BufferUsage::CopyDst | wgpu::BufferUsage::Storage);

    mBindGroup = utils::MakeBindGroup(device, bindGroupLayout,
                                      {
                                          {0, buffer, 0, kWorkgroupArraySize},
                                      });
}

void VulkanZeroInitializeWorkgroupMemoryExtensionTest::Step() {
    wgpu::CommandBuffer commands;
    {
        wgpu::CommandEncoder encoder = device.CreateCommandEncoder();

        if (SupportsTimestampQuery()) {
            RecordBeginTimestamp(encoder);
        }

        wgpu::ComputePassEncoder pass = encoder.BeginComputePass();
        pass.SetPipeline(mPipeline);
        for (uint32_t i = 0; i < kNumIterations; ++i) {
            uint32_t dynamicBufferOffset = sizeof(float) * kWorkgroupArraySize * i;
            pass.SetBindGroup(0, mBindGroup, 1, &dynamicBufferOffset);
            pass.DispatchWorkgroups(1);
        }
        pass.End();

        if (SupportsTimestampQuery()) {
            RecordEndTimestampAndResolveQuerySet(encoder);
        }

        commands = encoder.Finish();
    }

    queue.Submit(1, &commands);

    if (SupportsTimestampQuery()) {
        ComputeGPUElapsedTime();
    }
}

TEST_P(VulkanZeroInitializeWorkgroupMemoryExtensionTest, Run) {
    RunTest();
}

DAWN_INSTANTIATE_TEST_P(
    VulkanZeroInitializeWorkgroupMemoryExtensionTest,
    {VulkanBackend(), VulkanBackend({}, {"use_vulkan_zero_initialize_workgroup_memory_extension"}),
     VulkanBackend({"disable_workgroup_init"}, {})},
    {64, 256});

}  // anonymous namespace
}  // namespace dawn
