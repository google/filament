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

#include <algorithm>
#include <cstdint>
#include <random>
#include <sstream>
#include <string>
#include <vector>

#include "dawn/tests/perf_tests/DawnPerfTest.h"
#include "dawn/utils/WGPUHelpers.h"

// The purpose of this test is to estimate the overhead of atomics on workgroup memory.
// There are cases where simple memory load stores could be used instead of atomicLoad/Store and
// here we try to determine if there is any performance difference.

namespace dawn {
namespace {

constexpr uint32_t kWorkgroupSize = 256u;

std::string GenWorkgroupNoAtomics() {
    std::stringstream ss;
    ss << "const kWorkgroupSize = " << kWorkgroupSize << "u; // 8;\n";
    ss << R"(
        @group(0) @binding(0) var<storage, read_write> outVal : array<u32>;
            var<workgroup> wg: array<u32, kWorkgroupSize>;
                @compute @workgroup_size(kWorkgroupSize)
        fn main(@builtin(local_invocation_id) local_id : vec3u,
                @builtin(global_invocation_id) global_id  : vec3u) {
            var accum = outVal[global_id.x];
            wg[local_id.x] = accum + global_id.x;
            workgroupBarrier();
            for(var i = 0u; i < kWorkgroupSize;i++){
                accum = wg[(i + accum) % kWorkgroupSize];
            }
            workgroupBarrier();
            outVal[global_id.x] = accum;
        }
        )";
    return ss.str();
}

std::string GenWorkgroupWithAtomics() {
    std::stringstream ss;
    ss << "const kWorkgroupSize = " << kWorkgroupSize << "u; // 8;\n";
    ss << R"(
        @group(0) @binding(0) var<storage, read_write> outVal : array<u32>;
            var<workgroup> wg: array<atomic<u32>, kWorkgroupSize>;
        @compute @workgroup_size(kWorkgroupSize)
        fn main(@builtin(local_invocation_id) local_id : vec3u,
                @builtin(global_invocation_id) global_id  : vec3u) {
            var accum = outVal[global_id.x];
            atomicStore(&wg[local_id.x], accum + global_id.x);
            workgroupBarrier();
            for(var i = 0u; i < kWorkgroupSize;i++){
                accum = atomicLoad(&wg[(i + accum) % kWorkgroupSize]);
            }
            workgroupBarrier();
            outVal[global_id.x] = accum;
        }
        )";
    return ss.str();
}

constexpr unsigned int kNumIterations = 100;

enum class WorkgroupUsageType : uint8_t {
    WorkgroupTypeAtomic,
    WorkgroupTypeNonAtomic,
};

std::ostream& operator<<(std::ostream& ostream, const WorkgroupUsageType& usageType) {
    switch (usageType) {
        case WorkgroupUsageType::WorkgroupTypeAtomic:
            ostream << "WorkgroupTypeAtomic";
            break;
        case WorkgroupUsageType::WorkgroupTypeNonAtomic:
            ostream << "WorkgroupTypeNonAtomic";
            break;
    }
    return ostream;
}

DAWN_TEST_PARAM_STRUCT(WorkgroupAtomicParams, WorkgroupUsageType);

// Test the execution time of matrix multiplication (A [dimAOuter, dimInner] * B [dimInner,
// dimBOuter]) on the GPU and see the difference between robustness on and off.
class WorkgroupAtomicPerf : public DawnPerfTestWithParams<WorkgroupAtomicParams> {
  public:
    WorkgroupAtomicPerf() : DawnPerfTestWithParams(kNumIterations, 1) {}
    ~WorkgroupAtomicPerf() override = default;

    void SetUp() override;

  protected:
    std::vector<wgpu::FeatureName> GetRequiredFeatures() override {
        auto requirements = DawnPerfTestWithParams<WorkgroupAtomicParams>::GetRequiredFeatures();
        return requirements;
    }

  private:
    void Step() override;

    // Returns the shader body.
    std::string GetShaderBody();
    // Returns the shader source.
    std::string GetShader();

    wgpu::BindGroup mBindGroup;
    wgpu::ComputePipeline mPipeline;
};

uint32_t kNumDispatch = 1024;

void WorkgroupAtomicPerf::SetUp() {
    DawnPerfTestWithParams<WorkgroupAtomicParams>::SetUp();

    uint64_t byteDstSize = sizeof(uint32_t) * kNumDispatch * kWorkgroupSize;
    wgpu::BufferDescriptor desc = {};
    desc.usage = wgpu::BufferUsage::Storage;
    desc.size = byteDstSize;

    std::vector<uint32_t> dataA(byteDstSize);
    std::random_device rnd_device;
    std::mt19937 twister{rnd_device()};
    std::uniform_int_distribution<uint32_t> distrb(0, -1);

    std::generate(dataA.begin(), dataA.end(), [&]() { return distrb(twister); });

    wgpu::Buffer dst =
        utils::CreateBufferFromData(device, dataA.data(), byteDstSize, wgpu::BufferUsage::Storage);
    wgpu::ShaderModule module = utils::CreateShaderModule(device, GetShader().c_str());

    wgpu::ComputePipelineDescriptor csDesc;
    csDesc.compute.module = module;
    mPipeline = device.CreateComputePipeline(&csDesc);

    mBindGroup = utils::MakeBindGroup(device, mPipeline.GetBindGroupLayout(0),
                                      {
                                          {0, dst, 0, byteDstSize},
                                      });
}

std::string WorkgroupAtomicPerf::GetShader() {
    switch (GetParam().mWorkgroupUsageType) {
        case WorkgroupUsageType::WorkgroupTypeAtomic:
            return GenWorkgroupWithAtomics();
        case WorkgroupUsageType::WorkgroupTypeNonAtomic:
            return GenWorkgroupNoAtomics();
    }
    DAWN_UNREACHABLE();
}

void WorkgroupAtomicPerf::Step() {
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
        pass.SetBindGroup(0, mBindGroup);
        for (unsigned int i = 0; i < kNumIterations; ++i) {
            pass.DispatchWorkgroups(kNumDispatch);
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

TEST_P(WorkgroupAtomicPerf, Run) {
    RunTest();
}

DAWN_INSTANTIATE_TEST_P(WorkgroupAtomicPerf,
                        {D3D12Backend(), MetalBackend(), OpenGLBackend(), VulkanBackend()},
                        {WorkgroupUsageType::WorkgroupTypeAtomic,
                         WorkgroupUsageType::WorkgroupTypeNonAtomic});

}  // anonymous namespace
}  // namespace dawn
