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

#include <algorithm>
#include <cstdint>
#include <random>
#include <sstream>
#include <string>
#include <vector>

#include "dawn/tests/DawnTest.h"
#include "dawn/tests/perf_tests/DawnPerfTest.h"
#include "dawn/tests/perf_tests/DawnPerfTestPlatform.h"
#include "dawn/utils/WGPUHelpers.h"

// The purpose of this test to determine if dispatches in webgpu run concurrently.
// We know that this almost certainly does not happen for metal as we are not using the 'concurrent'
// flag on the command decoder. See crbug.com/425987598.

namespace dawn {
namespace {

constexpr uint32_t kBufferSize = 1024;

std::string GetShader() {
    std::stringstream ss;
    ss << R"( const kBufferSize :u32 = )" << kBufferSize << R"(;)";
    ss << R"(
        @group(0) @binding(0) var<storage, read_write> inout_ : array<u32, kBufferSize>;
        var<workgroup> wg_data: array<u32, kBufferSize>;
        @compute @workgroup_size(1)
        fn main() {
            var accum = inout_[0];
            for(var i = 0u; i < kBufferSize;i++){
                wg_data[i] = inout_[i];
            }
            for(var i = 0u; i < 1000000;i++){
                accum = (accum ^ wg_data[(i + accum) % kBufferSize])  + 123u;
            }
            inout_[0] = accum;
        }
        )";
    return ss.str();
}

constexpr unsigned int kNumIterations = 5;

enum class ConcurrentExecutionType : uint8_t {
    RunSingle,
    RunTwoConcurrent,
};

std::ostream& operator<<(std::ostream& ostream, const ConcurrentExecutionType& usageType) {
    switch (usageType) {
        case ConcurrentExecutionType::RunSingle:
            ostream << "RunSingle";
            break;
        case ConcurrentExecutionType::RunTwoConcurrent:
            ostream << "RunTwoConcurrent";
            break;
    }
    return ostream;
}

DAWN_TEST_PARAM_STRUCT(ConcurrentExecutionTestParams, ConcurrentExecutionType);

class ConcurrentExecutionTest : public DawnPerfTestWithParams<ConcurrentExecutionTestParams> {
  public:
    ConcurrentExecutionTest() : DawnPerfTestWithParams(kNumIterations, 1) {}
    ~ConcurrentExecutionTest() override = default;

    void SetUp() override;

  protected:
    std::vector<wgpu::FeatureName> GetRequiredFeatures() override {
        auto requirements =
            DawnPerfTestWithParams<ConcurrentExecutionTestParams>::GetRequiredFeatures();
        return requirements;
    }

  private:
    void Step() override;

    wgpu::BindGroup mBindGroupA;
    wgpu::BindGroup mBindGroupB;
    wgpu::ComputePipeline mPipeline;
};

void ConcurrentExecutionTest::SetUp() {
    DawnPerfTestWithParams<ConcurrentExecutionTestParams>::SetUp();

    uint64_t byteDstSize = sizeof(uint32_t) * kBufferSize;
    wgpu::BufferDescriptor desc = {};
    desc.usage = wgpu::BufferUsage::Storage;
    desc.size = byteDstSize;

    std::vector<uint32_t> data_rnd(byteDstSize);
    std::random_device rnd_device;
    std::mt19937 twister{rnd_device()};
    std::uniform_int_distribution<uint32_t> distrb(0, -1);

    std::generate(data_rnd.begin(), data_rnd.end(), [&]() { return distrb(twister); });

    wgpu::Buffer dst_a = utils::CreateBufferFromData(device, data_rnd.data(), byteDstSize,
                                                     wgpu::BufferUsage::Storage);
    wgpu::Buffer dst_b = utils::CreateBufferFromData(device, data_rnd.data(), byteDstSize,
                                                     wgpu::BufferUsage::Storage);

    wgpu::ShaderModule module = utils::CreateShaderModule(device, GetShader().c_str());

    wgpu::ComputePipelineDescriptor csDesc;
    csDesc.compute.module = module;
    mPipeline = device.CreateComputePipeline(&csDesc);

    mBindGroupA = utils::MakeBindGroup(device, mPipeline.GetBindGroupLayout(0),
                                       {
                                           {0, dst_a, 0, byteDstSize},
                                       });
    mBindGroupB = utils::MakeBindGroup(device, mPipeline.GetBindGroupLayout(0),
                                       {
                                           {0, dst_b, 0, byteDstSize},
                                       });
}

void ConcurrentExecutionTest::Step() {
    bool useTimestamps = SupportsTimestampQuery();

    wgpu::CommandBuffer commands;
    {
        wgpu::CommandEncoder encoder = device.CreateCommandEncoder();
        wgpu::ComputePassDescriptor computePassDesc;
        wgpu::PassTimestampWrites timestampWrites;
        if (useTimestamps) {
            timestampWrites = GetPassTimestampWrites();
            computePassDesc.timestampWrites = &timestampWrites;
        }
        wgpu::ComputePassEncoder pass = encoder.BeginComputePass(&computePassDesc);
        pass.SetPipeline(mPipeline);
        pass.SetBindGroup(0, mBindGroupA);
        pass.DispatchWorkgroups(1);

        if (GetParam().mConcurrentExecutionType == ConcurrentExecutionType::RunTwoConcurrent) {
            pass.SetBindGroup(0, mBindGroupB);
            pass.DispatchWorkgroups(1);
        }

        pass.End();
        if (useTimestamps) {
            ResolveTimestamps(encoder);
        }

        commands = encoder.Finish();
    }

    queue.Submit(1, &commands);

    if (useTimestamps) {
        ComputeGPUElapsedTime();
    }
}

TEST_P(ConcurrentExecutionTest, Run) {
    RunTest();
}

DAWN_INSTANTIATE_TEST_P(ConcurrentExecutionTest,
                        {D3D12Backend(), MetalBackend(), OpenGLBackend(), VulkanBackend()},
                        {ConcurrentExecutionType::RunSingle,
                         ConcurrentExecutionType::RunTwoConcurrent});

}  // anonymous namespace
}  // namespace dawn
