// Copyright 2019 The Dawn & Tint Authors
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
#include <initializer_list>
#include <vector>

#include "dawn/common/Math.h"
#include "dawn/tests/DawnTest.h"
#include "dawn/utils/WGPUHelpers.h"

namespace dawn {
namespace {

constexpr static std::initializer_list<uint32_t> kSentinelData{0, 0, 0};

class ComputeDispatchTests : public DawnTest {
  protected:
    void SetUp() override {
        DawnTest::SetUp();

        // Write workgroup number into the output buffer if we saw the biggest dispatch
        // To make sure the dispatch was not called, write maximum u32 value for 0 dispatches
        wgpu::ShaderModule module = utils::CreateShaderModule(device, R"(
            @group(0) @binding(0) var<storage, read_write> output : vec3u;

            @compute @workgroup_size(1, 1, 1)
            fn main(@builtin(global_invocation_id) GlobalInvocationID : vec3u,
                    @builtin(num_workgroups) dispatch : vec3u) {
                if (dispatch.x == 0u || dispatch.y == 0u || dispatch.z == 0u) {
                    output = vec3u(0xFFFFFFFFu, 0xFFFFFFFFu, 0xFFFFFFFFu);
                    return;
                }

                if (all(GlobalInvocationID == dispatch - vec3u(1u, 1u, 1u))) {
                    output = dispatch;
                }
            })");

        wgpu::ComputePipelineDescriptor csDesc;
        csDesc.compute.module = module;
        pipeline = device.CreateComputePipeline(&csDesc);

        // Test the use of the compute pipelines without using @num_workgroups
        wgpu::ShaderModule moduleWithoutNumWorkgroups = utils::CreateShaderModule(device, R"(
            @group(0) @binding(0) var<uniform> input : vec3u;
            @group(0) @binding(1) var<storage, read_write> output : vec3u;

            @compute @workgroup_size(1, 1, 1)
            fn main(@builtin(global_invocation_id) GlobalInvocationID : vec3u) {
                let dispatch : vec3u = input;

                if (dispatch.x == 0u || dispatch.y == 0u || dispatch.z == 0u) {
                    output = vec3u(0xFFFFFFFFu, 0xFFFFFFFFu, 0xFFFFFFFFu);
                    return;
                }

                if (all(GlobalInvocationID == dispatch - vec3u(1u, 1u, 1u))) {
                    output = dispatch;
                }
            })");
        csDesc.compute.module = moduleWithoutNumWorkgroups;
        pipelineWithoutNumWorkgroups = device.CreateComputePipeline(&csDesc);
    }

    void DirectTest(uint32_t x, uint32_t y, uint32_t z) {
        // Set up dst storage buffer to contain dispatch x, y, z
        wgpu::Buffer dst = utils::CreateBufferFromData<uint32_t>(
            device,
            wgpu::BufferUsage::Storage | wgpu::BufferUsage::CopySrc | wgpu::BufferUsage::CopyDst,
            kSentinelData);

        // Set up bind group and issue dispatch
        wgpu::BindGroup bindGroup = utils::MakeBindGroup(device, pipeline.GetBindGroupLayout(0),
                                                         {
                                                             {0, dst, 0, 3 * sizeof(uint32_t)},
                                                         });

        wgpu::CommandBuffer commands;
        {
            wgpu::CommandEncoder encoder = device.CreateCommandEncoder();
            wgpu::ComputePassEncoder pass = encoder.BeginComputePass();
            pass.SetPipeline(pipeline);
            pass.SetBindGroup(0, bindGroup);
            pass.DispatchWorkgroups(x, y, z);
            pass.End();

            commands = encoder.Finish();
        }

        queue.Submit(1, &commands);

        std::vector<uint32_t> expected =
            x == 0 || y == 0 || z == 0 ? kSentinelData : std::initializer_list<uint32_t>{x, y, z};

        // Verify the dispatch got called if all group counts are not zero
        EXPECT_BUFFER_U32_RANGE_EQ(&expected[0], dst, 0, 3);
    }

    void IndirectTest(std::vector<uint32_t> indirectBufferData,
                      uint64_t indirectOffset,
                      bool useNumWorkgroups = true) {
        // Set up dst storage buffer to contain dispatch x, y, z
        wgpu::Buffer dst = utils::CreateBufferFromData<uint32_t>(
            device,
            wgpu::BufferUsage::Storage | wgpu::BufferUsage::CopySrc | wgpu::BufferUsage::CopyDst,
            kSentinelData);

        wgpu::Buffer indirectBuffer = utils::CreateBufferFromData(
            device, &indirectBufferData[0], indirectBufferData.size() * sizeof(uint32_t),
            wgpu::BufferUsage::Indirect | wgpu::BufferUsage::CopySrc);

        uint32_t indirectStart = indirectOffset / sizeof(uint32_t);

        // Set up bind group and issue dispatch
        wgpu::BindGroup bindGroup;
        wgpu::ComputePipeline computePipelineForTest;

        if (useNumWorkgroups) {
            computePipelineForTest = pipeline;
            bindGroup = utils::MakeBindGroup(device, pipeline.GetBindGroupLayout(0),
                                             {
                                                 {0, dst, 0, 3 * sizeof(uint32_t)},
                                             });
        } else {
            computePipelineForTest = pipelineWithoutNumWorkgroups;
            wgpu::Buffer expectedBuffer =
                utils::CreateBufferFromData(device, &indirectBufferData[indirectStart],
                                            3 * sizeof(uint32_t), wgpu::BufferUsage::Uniform);
            bindGroup =
                utils::MakeBindGroup(device, pipelineWithoutNumWorkgroups.GetBindGroupLayout(0),
                                     {
                                         {0, expectedBuffer, 0, 3 * sizeof(uint32_t)},
                                         {1, dst, 0, 3 * sizeof(uint32_t)},
                                     });
        }

        wgpu::CommandBuffer commands;
        {
            wgpu::CommandEncoder encoder = device.CreateCommandEncoder();
            wgpu::ComputePassEncoder pass = encoder.BeginComputePass();
            pass.SetPipeline(computePipelineForTest);
            pass.SetBindGroup(0, bindGroup);
            pass.DispatchWorkgroupsIndirect(indirectBuffer, indirectOffset);
            pass.End();

            commands = encoder.Finish();
        }

        queue.Submit(1, &commands);

        std::vector<uint32_t> expected;

        uint32_t maxComputeWorkgroupsPerDimension =
            GetSupportedLimits().maxComputeWorkgroupsPerDimension;
        if (indirectBufferData[indirectStart] == 0 || indirectBufferData[indirectStart + 1] == 0 ||
            indirectBufferData[indirectStart + 2] == 0 ||
            indirectBufferData[indirectStart] > maxComputeWorkgroupsPerDimension ||
            indirectBufferData[indirectStart + 1] > maxComputeWorkgroupsPerDimension ||
            indirectBufferData[indirectStart + 2] > maxComputeWorkgroupsPerDimension) {
            expected = kSentinelData;
        } else {
            expected.assign(indirectBufferData.begin() + indirectStart,
                            indirectBufferData.begin() + indirectStart + 3);
        }

        // Verify the indirect buffer is not modified
        EXPECT_BUFFER_U32_RANGE_EQ(&indirectBufferData[0], indirectBuffer, 0, 3);
        // Verify the dispatch got called with group counts in indirect buffer if all group counts
        // are not zero
        EXPECT_BUFFER_U32_RANGE_EQ(&expected[0], dst, 0, 3);
    }

  private:
    wgpu::ComputePipeline pipeline;
    wgpu::ComputePipeline pipelineWithoutNumWorkgroups;
};

// Test basic direct
TEST_P(ComputeDispatchTests, DirectBasic) {
    DirectTest(2, 3, 4);
}

// Test no-op direct
TEST_P(ComputeDispatchTests, DirectNoop) {
    // All dimensions are 0s
    DirectTest(0, 0, 0);

    // Only x dimension is 0
    DirectTest(0, 3, 4);

    // Only y dimension is 0
    DirectTest(2, 0, 4);

    // Only z dimension is 0
    DirectTest(2, 3, 0);
}

// Test basic indirect
TEST_P(ComputeDispatchTests, IndirectBasic) {
#if DAWN_PLATFORM_IS(32_BIT)
    // TODO(crbug.com/dawn/1196): Fails on Chromium's Quadro P400 bots
    DAWN_SUPPRESS_TEST_IF(IsD3D12() && IsNvidia());
#endif
    IndirectTest({2, 3, 4}, 0);
}

// Test basic indirect without using @num_workgroups
TEST_P(ComputeDispatchTests, IndirectBasicWithoutNumWorkgroups) {
    IndirectTest({2, 3, 4}, 0, false);
}

// Test no-op indirect
TEST_P(ComputeDispatchTests, IndirectNoop) {
    // All dimensions are 0s
    IndirectTest({0, 0, 0}, 0);

    // Only x dimension is 0
    IndirectTest({0, 3, 4}, 0);

    // Only y dimension is 0
    IndirectTest({2, 0, 4}, 0);

    // Only z dimension is 0
    IndirectTest({2, 3, 0}, 0);
}

// Test indirect with buffer offset
TEST_P(ComputeDispatchTests, IndirectOffset) {
#if DAWN_PLATFORM_IS(32_BIT)
    // TODO(crbug.com/dawn/1196): Fails on Chromium's Quadro P400 bots
    DAWN_SUPPRESS_TEST_IF(IsD3D12() && IsNvidia());
#endif
    IndirectTest({0, 0, 0, 2, 3, 4}, 3 * sizeof(uint32_t));
}

// Test indirect with buffer offset without using @num_workgroups
TEST_P(ComputeDispatchTests, IndirectOffsetWithoutNumWorkgroups) {
    IndirectTest({0, 0, 0, 2, 3, 4}, 3 * sizeof(uint32_t), false);
}

// Test indirect dispatches at max limit.
TEST_P(ComputeDispatchTests, MaxWorkgroups) {
#if DAWN_PLATFORM_IS(32_BIT)
    // TODO(crbug.com/dawn/1196): Fails on Chromium's Quadro P400 bots
    DAWN_SUPPRESS_TEST_IF(IsD3D12() && IsNvidia());
#endif
    // TODO(crbug.com/435074717): Flaky on WARP.
    DAWN_SUPPRESS_TEST_IF(IsWARP());

    uint32_t max = GetSupportedLimits().maxComputeWorkgroupsPerDimension;

    // Test that the maximum works in each dimension.
    // Note: Testing (max, max, max) is very slow.
    IndirectTest({max, 3, 4}, 0);
    IndirectTest({2, max, 4}, 0);
    IndirectTest({2, 3, max}, 0);
}

// Test indirect dispatches exceeding the max limit are noop-ed.
TEST_P(ComputeDispatchTests, ExceedsMaxWorkgroupsNoop) {
    DAWN_TEST_UNSUPPORTED_IF(HasToggleEnabled("skip_validation"));

    uint32_t max = GetSupportedLimits().maxComputeWorkgroupsPerDimension;

    // All dimensions are above the max
    IndirectTest({max + 1, max + 1, max + 1}, 0);

    // Only x dimension is above the max
    IndirectTest({max + 1, 3, 4}, 0);
    IndirectTest({2 * max, 3, 4}, 0);

    // Only y dimension is above the max
    IndirectTest({2, max + 1, 4}, 0);
    IndirectTest({2, 2 * max, 4}, 0);

    // Only z dimension is above the max
    IndirectTest({2, 3, max + 1}, 0);
    IndirectTest({2, 3, 2 * max}, 0);
}

// Test indirect dispatches exceeding the max limit with an offset are noop-ed.
TEST_P(ComputeDispatchTests, ExceedsMaxWorkgroupsWithOffsetNoop) {
    DAWN_SUPPRESS_TEST_IF(IsWARP());
    DAWN_TEST_UNSUPPORTED_IF(HasToggleEnabled("skip_validation"));

    uint32_t max = GetSupportedLimits().maxComputeWorkgroupsPerDimension;

    IndirectTest({1, 2, 3, max + 1, 4, 5}, 1 * sizeof(uint32_t));
    IndirectTest({1, 2, 3, max + 1, 4, 5}, 2 * sizeof(uint32_t));
    IndirectTest({1, 2, 3, max + 1, 4, 5}, 3 * sizeof(uint32_t));
}

DAWN_INSTANTIATE_TEST(ComputeDispatchTests,
                      D3D11Backend(),
                      D3D12Backend(),
                      MetalBackend(),
                      OpenGLBackend(),
                      OpenGLESBackend(),
                      VulkanBackend());

namespace {
using UseNumWorkgoups = bool;
DAWN_TEST_PARAM_STRUCT(Params, UseNumWorkgoups);
}  // namespace

class ComputeMultipleDispatchesTests : public DawnTestWithParams<Params> {
  protected:
    void SetUp() override {
        DawnTestWithParams<Params>::SetUp();

        bindGroupLayout = utils::MakeBindGroupLayout(
            device, {{0, wgpu::ShaderStage::Compute, wgpu::BufferBindingType::Uniform,
                      /* hasDynamicOffset = */ true},
                     {1, wgpu::ShaderStage::Compute, wgpu::BufferBindingType::Storage}});

        wgpu::PipelineLayoutDescriptor pipelineLayoutDescriptor;
        pipelineLayoutDescriptor.bindGroupLayoutCount = 1;
        pipelineLayoutDescriptor.bindGroupLayouts = &bindGroupLayout;
        wgpu::PipelineLayout pipelineLayout =
            device.CreatePipelineLayout(&pipelineLayoutDescriptor);

        // Write workgroup number into the output buffer if we saw the biggest dispatch
        // To make sure the dispatch was not called, write maximum u32 value for 0 dispatches
        wgpu::ShaderModule module = utils::CreateShaderModule(device, R"(
            @group(0) @binding(0) var<uniform> dispatchId : u32;
            @group(0) @binding(1) var<storage, read_write> output : array<vec3u>;

            @compute @workgroup_size(1, 1, 1)
            fn main(@builtin(global_invocation_id) GlobalInvocationID : vec3u,
                    @builtin(num_workgroups) dispatch : vec3u) {
                if (dispatch.x == 0u || dispatch.y == 0u || dispatch.z == 0u) {
                    output[dispatchId] = vec3u(0xFFFFFFFFu, 0xFFFFFFFFu, 0xFFFFFFFFu);
                    return;
                }

                if (all(GlobalInvocationID == dispatch - vec3u(1u, 1u, 1u))) {
                    output[dispatchId] = dispatch;
                }
            })");

        wgpu::ComputePipelineDescriptor csDesc;
        //
        csDesc.compute.module = module;
        csDesc.layout = pipelineLayout;
        pipeline = device.CreateComputePipeline(&csDesc);

        // Test the use of the compute pipelines without using @num_workgroups
        wgpu::ShaderModule moduleWithoutNumWorkgroups = utils::CreateShaderModule(device, R"(
            // input.xyz = num_workgroups.xyz, input.w = dispatch call id (i.e. offset in output buffer)
            @group(0) @binding(0) var<uniform> input : vec4u;
            @group(0) @binding(1) var<storage, read_write> output : array<vec3u>;

            @compute @workgroup_size(1, 1, 1)
            fn main(@builtin(global_invocation_id) GlobalInvocationID : vec3u) {
                let dispatch : vec3u = input.xyz;
                let dispatchId = input.w;

                if (dispatch.x == 0u || dispatch.y == 0u || dispatch.z == 0u) {
                    output[dispatchId] = vec3u(0xFFFFFFFFu, 0xFFFFFFFFu, 0xFFFFFFFFu);
                    return;
                }

                if (all(GlobalInvocationID == dispatch - vec3u(1u, 1u, 1u))) {
                    output[dispatchId] = dispatch;
                }
            })");
        csDesc.compute.module = moduleWithoutNumWorkgroups;
        csDesc.layout = pipelineLayout;
        pipelineWithoutNumWorkgroups = device.CreateComputePipeline(&csDesc);
    }

    void IndirectTest(std::vector<uint32_t> indirectBufferData,
                      std::vector<uint64_t> indirectOffsets) {
        bool useNumWorkgroups = GetParam().mUseNumWorkgoups;
        // Set up dst storage buffer to contain dispatch x, y, z
        wgpu::BufferDescriptor dstBufDescriptor;
        // array<vec3u> aligns to 16 bytes
        dstBufDescriptor.size = Align(indirectOffsets.size() * 4u * sizeof(uint32_t), 16u);
        dstBufDescriptor.usage =
            wgpu::BufferUsage::Storage | wgpu::BufferUsage::CopySrc | wgpu::BufferUsage::CopyDst;
        wgpu::Buffer dst = device.CreateBuffer(&dstBufDescriptor);

        wgpu::Buffer indirectBuffer = utils::CreateBufferFromData(
            device, &indirectBufferData[0], indirectBufferData.size() * sizeof(uint32_t),
            wgpu::BufferUsage::Indirect | wgpu::BufferUsage::CopySrc);

        // dynamic offset requires a 256 byte alignment. So we store dispatch index every 256 byte
        // instead of compactly
        constexpr uint32_t kDynamicOffsetReq = 256;
        constexpr uint32_t kIndexOffset = kDynamicOffsetReq / sizeof(uint32_t);

        std::vector<uint32_t> dynamicOffsets(indirectOffsets.size());
        for (size_t i = 0; i < indirectOffsets.size(); i++) {
            dynamicOffsets[i] = i * kDynamicOffsetReq;
        }

        // Set up bind group and issue dispatch
        wgpu::BindGroup bindGroup;
        wgpu::ComputePipeline computePipelineForTest;

        if (useNumWorkgroups) {
            computePipelineForTest = pipeline;

            std::vector<uint32_t> dispatchIds(indirectOffsets.size() * kIndexOffset);
            for (size_t i = 0; i < indirectOffsets.size(); i++) {
                size_t o = kIndexOffset * i;
                dispatchIds[o] = i;
            }

            wgpu::Buffer uniformBuffer = utils::CreateBufferFromData(
                device, dispatchIds.data(), dispatchIds.size() * sizeof(uint32_t),
                wgpu::BufferUsage::Uniform);
            bindGroup = utils::MakeBindGroup(device, bindGroupLayout,
                                             {
                                                 {0, uniformBuffer, 0, sizeof(uint32_t)},
                                                 {1, dst, 0, dstBufDescriptor.size},
                                             });
        } else {
            computePipelineForTest = pipelineWithoutNumWorkgroups;

            std::vector<uint32_t> inputs(indirectOffsets.size() * kIndexOffset);
            for (size_t i = 0; i < indirectOffsets.size(); i++) {
                uint32_t indirectStart = indirectOffsets[i] / sizeof(uint32_t);
                size_t o = kIndexOffset * i;
                // numWorkgroups
                inputs[o] = indirectBufferData[indirectStart];
                inputs[o + 1] = indirectBufferData[indirectStart + 1];
                inputs[o + 2] = indirectBufferData[indirectStart + 2];
                // dispatchId
                inputs[o + 3] = i;
            }

            wgpu::Buffer uniformBuffer =
                utils::CreateBufferFromData(device, inputs.data(), inputs.size() * sizeof(uint32_t),
                                            wgpu::BufferUsage::Uniform);
            bindGroup = utils::MakeBindGroup(device, bindGroupLayout,
                                             {
                                                 {0, uniformBuffer, 0, 4 * sizeof(uint32_t)},
                                                 {1, dst, 0, dstBufDescriptor.size},
                                             });
        }

        wgpu::CommandBuffer commands;
        {
            wgpu::CommandEncoder encoder = device.CreateCommandEncoder();
            wgpu::ComputePassEncoder pass = encoder.BeginComputePass();
            pass.SetPipeline(computePipelineForTest);
            for (size_t i = 0; i < indirectOffsets.size(); i++) {
                uint64_t indirectOffset = indirectOffsets[i];
                // Use dynamic binding offset to set dispatch Id (used as offset to output buffer)
                // for each dispatch call
                pass.SetBindGroup(0, bindGroup, 1, &dynamicOffsets[i]);
                pass.DispatchWorkgroupsIndirect(indirectBuffer, indirectOffset);
            }

            pass.End();

            commands = encoder.Finish();
        }

        queue.Submit(1, &commands);

        uint32_t maxComputeWorkgroupsPerDimension =
            GetSupportedLimits().maxComputeWorkgroupsPerDimension;

        std::vector<uint32_t> expected(4 * indirectOffsets.size(), 0);
        for (size_t i = 0; i < indirectOffsets.size(); i++) {
            uint32_t indirectStart = indirectOffsets[i] / sizeof(uint32_t);
            size_t o = 4 * i;

            if (indirectBufferData[indirectStart] == 0 ||
                indirectBufferData[indirectStart + 1] == 0 ||
                indirectBufferData[indirectStart + 2] == 0 ||
                indirectBufferData[indirectStart] > maxComputeWorkgroupsPerDimension ||
                indirectBufferData[indirectStart + 1] > maxComputeWorkgroupsPerDimension ||
                indirectBufferData[indirectStart + 2] > maxComputeWorkgroupsPerDimension) {
                std::copy(kSentinelData.begin(), kSentinelData.end(), expected.begin() + o);
            } else {
                expected[o] = indirectBufferData[indirectStart];
                expected[o + 1] = indirectBufferData[indirectStart + 1];
                expected[o + 2] = indirectBufferData[indirectStart + 2];
            }
        }

        // Verify the indirect buffer is not modified
        EXPECT_BUFFER_U32_RANGE_EQ(&indirectBufferData[0], indirectBuffer, 0,
                                   indirectBufferData.size());
        // Verify the dispatch got called with group counts in indirect buffer if all group counts
        // are not zero
        EXPECT_BUFFER_U32_RANGE_EQ(&expected[0], dst, 0, expected.size());
    }

  private:
    wgpu::ComputePipeline pipeline;
    wgpu::ComputePipeline pipelineWithoutNumWorkgroups;
    wgpu::BindGroupLayout bindGroupLayout;
};

// Test indirect dispatches with buffer offset
TEST_P(ComputeMultipleDispatchesTests, IndirectOffset) {
#if DAWN_PLATFORM_IS(32_BIT)
    // TODO(crbug.com/dawn/1196): Fails on Chromium's Quadro P400 bots
    DAWN_SUPPRESS_TEST_IF(IsD3D12() && IsNvidia());
#endif
    DAWN_SUPPRESS_TEST_IF(IsWARP());

    // Control case: One DispatchWorkgroupsIndirect call
    IndirectTest({0, 0, 0, 2, 3, 4}, {3 * sizeof(uint32_t)});

    // Two dispatches: first is no-op
    IndirectTest({0, 0, 0, 2, 3, 4}, {0, 3 * sizeof(uint32_t)});

    // Two dispatches
    IndirectTest({9, 8, 7, 2, 3, 4}, {0, 3 * sizeof(uint32_t)});

    // Indirect offsets not in order
    IndirectTest({9, 8, 7, 2, 3, 4}, {3 * sizeof(uint32_t), 0});

    // Multiple dispatches with duplicate indirect offsets
    IndirectTest({9, 8, 7, 0, 0, 0, 2, 3, 4, 0xa, 0xb, 0xc, 0xf, 0xe, 0xd},
                 {
                     6 * sizeof(uint32_t),
                     0,
                     3 * sizeof(uint32_t),
                     12 * sizeof(uint32_t),
                     9 * sizeof(uint32_t),
                     6 * sizeof(uint32_t),
                     6 * sizeof(uint32_t),
                 });
}

// Test indirect dispatches exceeding the max limit with an offset are noop-ed.
TEST_P(ComputeMultipleDispatchesTests, ExceedsMaxWorkgroupsWithOffsetNoop) {
    DAWN_TEST_UNSUPPORTED_IF(HasToggleEnabled("skip_validation"));
    DAWN_SUPPRESS_TEST_IF(IsWARP());

#if DAWN_PLATFORM_IS(32_BIT)
    // TODO(crbug.com/dawn/1196): Fails on Chromium's Quadro P400 bots
    DAWN_SUPPRESS_TEST_IF(IsD3D12() && IsNvidia());
#endif

    uint32_t max = GetSupportedLimits().maxComputeWorkgroupsPerDimension;

    // Two dispatches: first is no-op
    IndirectTest({max + 1, 1, 1, 2, 3, 4}, {0, 3 * sizeof(uint32_t)});

    // Two dispatches: second is no-op
    IndirectTest({2, 3, 4, max + 1, 1, 1}, {0, 3 * sizeof(uint32_t)});

    // Two dispatches: second is no-op
    IndirectTest({max + 1, 1, 1, 2, 3, 4}, {3 * sizeof(uint32_t), 0});
}

DAWN_INSTANTIATE_TEST_P(ComputeMultipleDispatchesTests,
                        {D3D11Backend(), D3D12Backend(), MetalBackend(), OpenGLBackend(),
                         OpenGLESBackend(), VulkanBackend()},
                        {true, false}  // useNumWorkgroups
);

}  // anonymous namespace
}  // namespace dawn
