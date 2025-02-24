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

#include <benchmark/benchmark.h>
#include <dawn/webgpu_cpp.h>
#include <array>
#include <vector>

#include "dawn/common/Log.h"
#include "dawn/tests/benchmarks/NullDeviceSetup.h"
#include "dawn/utils/ComboRenderPipelineDescriptor.h"
#include "dawn/utils/WGPUHelpers.h"

namespace dawn {
namespace {

// Benchmarks for creation and recreation of objects in Dawn.
class ObjectCreation : public NullDeviceBenchmarkFixture {
  protected:
    ObjectCreation() {
        // Currently, object creation still needs to be implicitly synchronized even though the
        // frontend cache is thread-safe. Once other parts of Dawn are thread-safe, i.e. memory
        // management, these tests should work without synchronization.
        requiredFeatures.push_back(wgpu::FeatureName::ImplicitDeviceSynchronization);
    }

  private:
    wgpu::DeviceDescriptor GetDeviceDescriptor() const override {
        wgpu::DeviceDescriptor deviceDesc = {};
        deviceDesc.requiredFeatures = requiredFeatures.data();
        deviceDesc.requiredFeatureCount = requiredFeatures.size();
        return deviceDesc;
    }

    std::vector<wgpu::FeatureName> requiredFeatures;
};

BENCHMARK_DEFINE_F(ObjectCreation, SameBindGroupLayout)
(benchmark::State& state) {
    std::vector<wgpu::BindGroupLayoutEntry> entries(state.range(0));
    for (uint32_t i = 0; i < entries.size(); ++i) {
        entries[i].binding = i;
        entries[i].visibility = wgpu::ShaderStage::Vertex | wgpu::ShaderStage::Fragment;
        entries[i].buffer.type = wgpu::BufferBindingType::Uniform;
    }

    wgpu::BindGroupLayoutDescriptor bglDesc = {};
    bglDesc.entryCount = entries.size();
    bglDesc.entries = entries.data();

    std::vector<wgpu::BindGroupLayout> bgls;
    bgls.reserve(100000);
    bgls.push_back(device.CreateBindGroupLayout(&bglDesc));
    for (auto _ : state) {
        bgls.push_back(device.CreateBindGroupLayout(&bglDesc));
    }
}
BENCHMARK_REGISTER_F(ObjectCreation, SameBindGroupLayout)
    ->Arg(1)
    ->Arg(12)
    ->Threads(1)
    ->Threads(4)
    ->Threads(16);

BENCHMARK_DEFINE_F(ObjectCreation, UniqueBindGroupLayout)
(benchmark::State& state) {
    std::vector<wgpu::BindGroupLayoutEntry> entries(state.range(0));
    for (uint32_t i = 0; i < entries.size(); ++i) {
        entries[i].binding = i;
        entries[i].visibility = wgpu::ShaderStage::Vertex | wgpu::ShaderStage::Fragment;
        entries[i].buffer.type = wgpu::BufferBindingType::Uniform;
        entries[i].buffer.minBindingSize = 4u;
    }

    wgpu::BindGroupLayoutDescriptor bglDesc = {};
    bglDesc.entryCount = entries.size();
    bglDesc.entries = entries.data();

    // Depending on the thread index, we increment a subset of the binding sizes to ensure we create
    // a new unique bind group descriptor. For now, this is just the thread_index if it's smaller
    // than Arg, otherwise its the last index AND the modulo index.
    std::vector<size_t> entryIndices;
    if (state.thread_index() < state.range(0)) {
        entryIndices.push_back(state.thread_index());
    } else {
        entryIndices.push_back(state.thread_index() % state.range(0));
        entryIndices.push_back(state.range(0) - 1);
    }

    std::vector<wgpu::BindGroupLayout> bgls;
    bgls.reserve(100000);
    for (auto _ : state) {
        for (size_t index : entryIndices) {
            entries[index].buffer.minBindingSize += 4;
        }
        bgls.push_back(device.CreateBindGroupLayout(&bglDesc));
    }
}
BENCHMARK_REGISTER_F(ObjectCreation, UniqueBindGroupLayout)
    ->Arg(12)
    ->Threads(1)
    ->Threads(4)
    ->Threads(16);

BENCHMARK_DEFINE_F(ObjectCreation, SameSampler)
(benchmark::State& state) {
    std::vector<wgpu::Sampler> samplers;
    samplers.reserve(400000);
    samplers.push_back(device.CreateSampler());
    for (auto _ : state) {
        samplers.push_back(device.CreateSampler());
    }
}
BENCHMARK_REGISTER_F(ObjectCreation, SameSampler)->Threads(1)->Threads(4)->Threads(16);

BENCHMARK_DEFINE_F(ObjectCreation, UniqueSampler)
(benchmark::State& state) {
    static constexpr float kLodStep = 1.0 / 400000.0;
    float kLodOffset = kLodStep * state.thread_index() / state.threads();

    wgpu::SamplerDescriptor samplerDesc = {};
    samplerDesc.lodMaxClamp = kLodOffset;

    std::vector<wgpu::Sampler> samplers;
    samplers.reserve(400000);
    for (auto _ : state) {
        samplerDesc.lodMaxClamp += kLodStep;
        samplers.push_back(device.CreateSampler(&samplerDesc));
    }
}
BENCHMARK_REGISTER_F(ObjectCreation, UniqueSampler)->Threads(1)->Threads(4)->Threads(16);

BENCHMARK_DEFINE_F(ObjectCreation, SameComputePipeline)
(benchmark::State& state) {
    wgpu::ComputePipelineDescriptor computeDesc = {};
    computeDesc.compute.module = utils::CreateShaderModule(device, R"(
        @compute @workgroup_size(1) fn main() { _ = 0u; }
    )");
    computeDesc.layout = utils::MakePipelineLayout(device, {});

    std::vector<wgpu::ComputePipeline> computePipelines;
    computePipelines.reserve(50000);
    computePipelines.push_back(device.CreateComputePipeline(&computeDesc));
    for (auto _ : state) {
        computePipelines.push_back(device.CreateComputePipeline(&computeDesc));
    }
}
BENCHMARK_REGISTER_F(ObjectCreation, SameComputePipeline)->Threads(1)->Threads(4)->Threads(16);

BENCHMARK_DEFINE_F(ObjectCreation, UniqueComputePipeline)
(benchmark::State& state) {
    wgpu::ConstantEntry constant = {};
    constant.key = "x";
    constant.value = state.thread_index();

    wgpu::ComputePipelineDescriptor computeDesc = {};
    computeDesc.compute.module = utils::CreateShaderModule(device, R"(
        override x: u32 = 0u;
        @compute @workgroup_size(1) fn main() { _ = x; }
    )");
    computeDesc.compute.constantCount = 1;
    computeDesc.compute.constants = &constant;
    computeDesc.layout = utils::MakePipelineLayout(device, {});

    std::vector<wgpu::ComputePipeline> computePipelines;
    computePipelines.reserve(40000);
    for (auto _ : state) {
        constant.value += state.threads();
        computePipelines.push_back(device.CreateComputePipeline(&computeDesc));
    }
}
BENCHMARK_REGISTER_F(ObjectCreation, UniqueComputePipeline)->Threads(1)->Threads(4)->Threads(16);

BENCHMARK_DEFINE_F(ObjectCreation, SameRenderPipeline)
(benchmark::State& state) {
    utils::ComboRenderPipelineDescriptor renderDesc;
    renderDesc.layout = utils::MakePipelineLayout(device, {});
    renderDesc.vertex.module = utils::CreateShaderModule(device, R"(
        @vertex fn main() -> @builtin(position) vec4f {
            return vec4f(0.0, 0.0, 0.0, 1.0);
        })");
    renderDesc.cFragment.module = utils::CreateShaderModule(device, R"(
        @fragment fn main() -> @location(0) vec4f {
            return vec4f(0.0, 1.0, 0.0, 1.0);
        })");

    std::vector<wgpu::RenderPipeline> renderPipelines;
    renderPipelines.reserve(40000);
    renderPipelines.push_back(device.CreateRenderPipeline(&renderDesc));
    for (auto _ : state) {
        renderPipelines.push_back(device.CreateRenderPipeline(&renderDesc));
    }
}
BENCHMARK_REGISTER_F(ObjectCreation, SameRenderPipeline)->Threads(1)->Threads(4)->Threads(16);

BENCHMARK_DEFINE_F(ObjectCreation, UniqueRenderPipeline)
(benchmark::State& state) {
    wgpu::ConstantEntry constant = {};
    constant.key = "x";
    constant.value = state.thread_index();

    utils::ComboRenderPipelineDescriptor renderDesc;
    renderDesc.layout = utils::MakePipelineLayout(device, {});
    renderDesc.vertex.module = utils::CreateShaderModule(device, R"(
        override x: f32 = 0.0;
        @vertex fn main() -> @builtin(position) vec4f {
            return vec4f(0.0, 0.0, 0.0, 1.0 / x);
        })");
    renderDesc.vertex.constantCount = 1;
    renderDesc.vertex.constants = &constant;
    renderDesc.cFragment.module = utils::CreateShaderModule(device, R"(
        override x: f32 = 0.0;
        @fragment fn main() -> @location(0) vec4f {
            return vec4f(0.0, 1.0, 0.0, 1.0 / x);
        })");
    renderDesc.cFragment.constantCount = 1;
    renderDesc.cFragment.constants = &constant;

    std::vector<wgpu::RenderPipeline> renderPipelines;
    renderPipelines.reserve(40000);
    for (auto _ : state) {
        constant.value += state.threads();
        renderPipelines.push_back(device.CreateRenderPipeline(&renderDesc));
    }
}
BENCHMARK_REGISTER_F(ObjectCreation, UniqueRenderPipeline)->Threads(1)->Threads(4)->Threads(16);

}  // namespace
}  // namespace dawn
