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
#include <numeric>
#include <string>
#include <vector>

#include "dawn/common/Math.h"
#include "dawn/tests/DawnTest.h"
#include "dawn/utils/ComboRenderPipelineDescriptor.h"
#include "dawn/utils/WGPUHelpers.h"

namespace dawn {
namespace {

constexpr uint32_t kRTSize = 400;
constexpr uint32_t kBindingSize = 8;

class DynamicBufferOffsetTests : public DawnTest {
  protected:
    wgpu::Limits GetRequiredLimits(const wgpu::Limits& supported) override {
        // TODO(crbug.com/383593270): Enable all the limits.
        wgpu::Limits required = {};
        required.maxStorageBuffersInFragmentStage = supported.maxStorageBuffersInFragmentStage;
        required.maxStorageBuffersPerShaderStage = supported.maxStorageBuffersPerShaderStage;
        return required;
    }

    void SetUp() override {
        DawnTest::SetUp();

        DAWN_SUPPRESS_TEST_IF(GetSupportedLimits().maxStorageBuffersInFragmentStage < 1);

        mMinUniformBufferOffsetAlignment = GetSupportedLimits().minUniformBufferOffsetAlignment;

        // Mix up dynamic and non dynamic resources in one bind group and using not continuous
        // binding number to cover more cases.
        std::vector<uint32_t> uniformData(mMinUniformBufferOffsetAlignment / sizeof(uint32_t) + 2);
        uniformData[0] = 1;
        uniformData[1] = 2;

        mUniformBuffers[0] = utils::CreateBufferFromData(device, uniformData.data(),
                                                         sizeof(uint32_t) * uniformData.size(),
                                                         wgpu::BufferUsage::Uniform);

        uniformData[uniformData.size() - 2] = 5;
        uniformData[uniformData.size() - 1] = 6;

        // Dynamic uniform buffer
        mUniformBuffers[1] = utils::CreateBufferFromData(device, uniformData.data(),
                                                         sizeof(uint32_t) * uniformData.size(),
                                                         wgpu::BufferUsage::Uniform);

        wgpu::BufferDescriptor storageBufferDescriptor;
        storageBufferDescriptor.size = sizeof(uint32_t) * uniformData.size();
        storageBufferDescriptor.usage =
            wgpu::BufferUsage::Storage | wgpu::BufferUsage::CopyDst | wgpu::BufferUsage::CopySrc;

        mStorageBuffers[0] = device.CreateBuffer(&storageBufferDescriptor);

        // Dynamic storage buffer
        mStorageBuffers[1] = device.CreateBuffer(&storageBufferDescriptor);

        // Default bind group layout
        mBindGroupLayouts[0] = utils::MakeBindGroupLayout(
            device, {{0, wgpu::ShaderStage::Compute | wgpu::ShaderStage::Fragment,
                      wgpu::BufferBindingType::Uniform},
                     {1, wgpu::ShaderStage::Compute | wgpu::ShaderStage::Fragment,
                      wgpu::BufferBindingType::Storage},
                     {3, wgpu::ShaderStage::Compute | wgpu::ShaderStage::Fragment,
                      wgpu::BufferBindingType::Uniform, true},
                     {4, wgpu::ShaderStage::Compute | wgpu::ShaderStage::Fragment,
                      wgpu::BufferBindingType::Storage, true}});

        // Default bind group
        mBindGroups[0] = utils::MakeBindGroup(device, mBindGroupLayouts[0],
                                              {{0, mUniformBuffers[0], 0, kBindingSize},
                                               {1, mStorageBuffers[0], 0, kBindingSize},
                                               {3, mUniformBuffers[1], 0, kBindingSize},
                                               {4, mStorageBuffers[1], 0, kBindingSize}});

        // Extra uniform buffer for inheriting test
        mUniformBuffers[2] = utils::CreateBufferFromData(device, uniformData.data(),
                                                         sizeof(uint32_t) * uniformData.size(),
                                                         wgpu::BufferUsage::Uniform);

        // Bind group layout for inheriting test
        mBindGroupLayouts[1] = utils::MakeBindGroupLayout(
            device, {{0, wgpu::ShaderStage::Compute | wgpu::ShaderStage::Fragment,
                      wgpu::BufferBindingType::Uniform}});

        // Bind group for inheriting test
        mBindGroups[1] = utils::MakeBindGroup(device, mBindGroupLayouts[1],
                                              {{0, mUniformBuffers[2], 0, kBindingSize}});
    }
    // Create objects to use as resources inside test bind groups.

    uint32_t mMinUniformBufferOffsetAlignment;
    wgpu::BindGroup mBindGroups[2];
    wgpu::BindGroupLayout mBindGroupLayouts[2];
    wgpu::Buffer mUniformBuffers[3];
    wgpu::Buffer mStorageBuffers[2];
    wgpu::Texture mColorAttachment;

    wgpu::RenderPipeline CreateRenderPipeline(bool isInheritedPipeline = false) {
        wgpu::ShaderModule vsModule = utils::CreateShaderModule(device, R"(
            @vertex
            fn main(@builtin(vertex_index) VertexIndex : u32) -> @builtin(position) vec4f {
                var pos = array(
                    vec2f(-1.0, 0.0),
                    vec2f(-1.0, 1.0),
                    vec2f( 0.0, 1.0));
                return vec4f(pos[VertexIndex], 0.0, 1.0);
            })");

        // Construct fragment shader source
        std::ostringstream fs;
        std::string multipleNumber = isInheritedPipeline ? "2" : "1";
        fs << R"(
            struct Buf {
                value : vec2u
            }

            @group(0) @binding(0) var<uniform> uBufferNotDynamic : Buf;
            @group(0) @binding(1) var<storage, read_write> sBufferNotDynamic : Buf;
            @group(0) @binding(3) var<uniform> uBuffer : Buf;
            @group(0) @binding(4) var<storage, read_write> sBuffer : Buf;
        )";

        if (isInheritedPipeline) {
            fs << R"(
                @group(1) @binding(0) var<uniform> paddingBlock : Buf;
            )";
        }

        fs << "const multipleNumber : u32 = " << multipleNumber << "u;\n";
        fs << R"(
            @fragment fn main() -> @location(0) vec4f {
                sBufferNotDynamic.value = uBufferNotDynamic.value.xy;
                sBuffer.value = vec2u(multipleNumber, multipleNumber) * (uBuffer.value.xy + uBufferNotDynamic.value.xy);
                return vec4f(f32(uBuffer.value.x) / 255.0, f32(uBuffer.value.y) / 255.0,
                                      1.0, 1.0);
            }
        )";

        wgpu::ShaderModule fsModule = utils::CreateShaderModule(device, fs.str().c_str());

        utils::ComboRenderPipelineDescriptor pipelineDescriptor;
        pipelineDescriptor.vertex.module = vsModule;
        pipelineDescriptor.cFragment.module = fsModule;
        pipelineDescriptor.cTargets[0].format = wgpu::TextureFormat::RGBA8Unorm;

        wgpu::PipelineLayoutDescriptor pipelineLayoutDescriptor;
        if (isInheritedPipeline) {
            pipelineLayoutDescriptor.bindGroupLayoutCount = 2;
        } else {
            pipelineLayoutDescriptor.bindGroupLayoutCount = 1;
        }
        pipelineLayoutDescriptor.bindGroupLayouts = mBindGroupLayouts;
        pipelineDescriptor.layout = device.CreatePipelineLayout(&pipelineLayoutDescriptor);

        return device.CreateRenderPipeline(&pipelineDescriptor);
    }

    wgpu::ComputePipeline CreateComputePipeline(bool isInheritedPipeline = false) {
        // Construct compute shader source
        std::ostringstream cs;
        std::string multipleNumber = isInheritedPipeline ? "2" : "1";
        cs << R"(
            struct Buf {
                value : vec2u
            }

            @group(0) @binding(0) var<uniform> uBufferNotDynamic : Buf;
            @group(0) @binding(1) var<storage, read_write> sBufferNotDynamic : Buf;
            @group(0) @binding(3) var<uniform> uBuffer : Buf;
            @group(0) @binding(4) var<storage, read_write> sBuffer : Buf;
        )";

        if (isInheritedPipeline) {
            cs << R"(
                @group(1) @binding(0) var<uniform> paddingBlock : Buf;
            )";
        }

        cs << "const multipleNumber : u32 = " << multipleNumber << "u;\n";
        cs << R"(
            @compute @workgroup_size(1) fn main() {
                sBufferNotDynamic.value = uBufferNotDynamic.value.xy;
                sBuffer.value = vec2u(multipleNumber, multipleNumber) * (uBuffer.value.xy + uBufferNotDynamic.value.xy);
            }
        )";

        wgpu::ShaderModule csModule = utils::CreateShaderModule(device, cs.str().c_str());

        wgpu::ComputePipelineDescriptor csDesc;
        csDesc.compute.module = csModule;

        wgpu::PipelineLayoutDescriptor pipelineLayoutDescriptor;
        if (isInheritedPipeline) {
            pipelineLayoutDescriptor.bindGroupLayoutCount = 2;
        } else {
            pipelineLayoutDescriptor.bindGroupLayoutCount = 1;
        }
        pipelineLayoutDescriptor.bindGroupLayouts = mBindGroupLayouts;
        csDesc.layout = device.CreatePipelineLayout(&pipelineLayoutDescriptor);

        return device.CreateComputePipeline(&csDesc);
    }
};

// Dynamic offsets are all zero and no effect to result.
TEST_P(DynamicBufferOffsetTests, BasicRenderPipeline) {
    wgpu::RenderPipeline pipeline = CreateRenderPipeline();
    utils::BasicRenderPass renderPass = utils::CreateBasicRenderPass(device, kRTSize, kRTSize);

    wgpu::CommandEncoder commandEncoder = device.CreateCommandEncoder();
    std::array<uint32_t, 2> offsets = {0, 0};
    wgpu::RenderPassEncoder renderPassEncoder =
        commandEncoder.BeginRenderPass(&renderPass.renderPassInfo);
    renderPassEncoder.SetPipeline(pipeline);
    renderPassEncoder.SetBindGroup(0, mBindGroups[0], offsets.size(), offsets.data());
    renderPassEncoder.Draw(3);
    renderPassEncoder.End();
    wgpu::CommandBuffer commands = commandEncoder.Finish();
    queue.Submit(1, &commands);

    std::vector<uint32_t> expectedData = {2, 4};
    EXPECT_PIXEL_RGBA8_EQ(utils::RGBA8(1, 2, 255, 255), renderPass.color, 0, 0);
    EXPECT_BUFFER_U32_RANGE_EQ(expectedData.data(), mStorageBuffers[1], 0, expectedData.size());
}

// Have non-zero dynamic offsets.
TEST_P(DynamicBufferOffsetTests, SetDynamicOffsetsRenderPipeline) {
    wgpu::RenderPipeline pipeline = CreateRenderPipeline();
    utils::BasicRenderPass renderPass = utils::CreateBasicRenderPass(device, kRTSize, kRTSize);

    wgpu::CommandEncoder commandEncoder = device.CreateCommandEncoder();
    std::array<uint32_t, 2> offsets = {mMinUniformBufferOffsetAlignment,
                                       mMinUniformBufferOffsetAlignment};
    wgpu::RenderPassEncoder renderPassEncoder =
        commandEncoder.BeginRenderPass(&renderPass.renderPassInfo);
    renderPassEncoder.SetPipeline(pipeline);
    renderPassEncoder.SetBindGroup(0, mBindGroups[0], offsets.size(), offsets.data());
    renderPassEncoder.Draw(3);
    renderPassEncoder.End();
    wgpu::CommandBuffer commands = commandEncoder.Finish();
    queue.Submit(1, &commands);

    std::vector<uint32_t> expectedData = {6, 8};
    EXPECT_PIXEL_RGBA8_EQ(utils::RGBA8(5, 6, 255, 255), renderPass.color, 0, 0);
    EXPECT_BUFFER_U32_RANGE_EQ(expectedData.data(), mStorageBuffers[1],
                               mMinUniformBufferOffsetAlignment, expectedData.size());
}

// Dynamic offsets are all zero and no effect to result.
TEST_P(DynamicBufferOffsetTests, BasicComputePipeline) {
    wgpu::ComputePipeline pipeline = CreateComputePipeline();

    std::array<uint32_t, 2> offsets = {0, 0};

    wgpu::CommandEncoder commandEncoder = device.CreateCommandEncoder();
    wgpu::ComputePassEncoder computePassEncoder = commandEncoder.BeginComputePass();
    computePassEncoder.SetPipeline(pipeline);
    computePassEncoder.SetBindGroup(0, mBindGroups[0], offsets.size(), offsets.data());
    computePassEncoder.DispatchWorkgroups(1);
    computePassEncoder.End();
    wgpu::CommandBuffer commands = commandEncoder.Finish();
    queue.Submit(1, &commands);

    std::vector<uint32_t> expectedData = {2, 4};
    EXPECT_BUFFER_U32_RANGE_EQ(expectedData.data(), mStorageBuffers[1], 0, expectedData.size());
}

// Have non-zero dynamic offsets.
TEST_P(DynamicBufferOffsetTests, SetDynamicOffsetsComputePipeline) {
    wgpu::ComputePipeline pipeline = CreateComputePipeline();

    std::array<uint32_t, 2> offsets = {mMinUniformBufferOffsetAlignment,
                                       mMinUniformBufferOffsetAlignment};

    wgpu::CommandEncoder commandEncoder = device.CreateCommandEncoder();
    wgpu::ComputePassEncoder computePassEncoder = commandEncoder.BeginComputePass();
    computePassEncoder.SetPipeline(pipeline);
    computePassEncoder.SetBindGroup(0, mBindGroups[0], offsets.size(), offsets.data());
    computePassEncoder.DispatchWorkgroups(1);
    computePassEncoder.End();
    wgpu::CommandBuffer commands = commandEncoder.Finish();
    queue.Submit(1, &commands);

    std::vector<uint32_t> expectedData = {6, 8};
    EXPECT_BUFFER_U32_RANGE_EQ(expectedData.data(), mStorageBuffers[1],
                               mMinUniformBufferOffsetAlignment, expectedData.size());
}

// Test basic inherit on render pipeline
TEST_P(DynamicBufferOffsetTests, BasicInheritRenderPipeline) {
    wgpu::ShaderModule vsModule = utils::CreateShaderModule(device, R"(
            @vertex
            fn main(@builtin(vertex_index) VertexIndex : u32) -> @builtin(position) vec4f {
                var pos = array(
                    vec2f(-1.0, 0.0),
                    vec2f(-1.0, 1.0),
                    vec2f( 0.0, 1.0));
                return vec4f(pos[VertexIndex], 0.0, 1.0);
            })");

    // Construct fragment shader source
    std::ostringstream fs;
    fs << R"(
            struct Buf {
                value : vec2u
            }

            @group(0) @binding(0) var<uniform> uBufferNotDynamic : Buf;
            @group(0) @binding(1) var<storage, read_write> sBufferNotDynamic : Buf;
            @group(1) @binding(3) var<uniform> uBuffer : Buf;
            @group(1) @binding(4) var<storage, read_write> sBuffer : Buf;

            @fragment fn main() -> @location(0) vec4f {
                sBufferNotDynamic.value = uBufferNotDynamic.value.xy;
                sBuffer.value = uBuffer.value.xy + uBufferNotDynamic.value.xy;
                return vec4f(f32(uBuffer.value.x) / 255.0, f32(uBuffer.value.y) / 255.0,
                                      1.0, 1.0);
            }
        )";

    wgpu::ShaderModule fsModule = utils::CreateShaderModule(device, fs.str().c_str());

    utils::ComboRenderPipelineDescriptor pipelineDescriptor;
    pipelineDescriptor.vertex.module = vsModule;
    pipelineDescriptor.cFragment.module = fsModule;
    pipelineDescriptor.cTargets[0].format = wgpu::TextureFormat::RGBA8Unorm;

    wgpu::BindGroupLayout bindGroupLayouts[2];
    bindGroupLayouts[0] = utils::MakeBindGroupLayout(
        device, {{0, wgpu::ShaderStage::Compute | wgpu::ShaderStage::Fragment,
                  wgpu::BufferBindingType::Uniform},
                 {1, wgpu::ShaderStage::Compute | wgpu::ShaderStage::Fragment,
                  wgpu::BufferBindingType::Storage}});
    bindGroupLayouts[1] = utils::MakeBindGroupLayout(
        device, {{3, wgpu::ShaderStage::Compute | wgpu::ShaderStage::Fragment,
                  wgpu::BufferBindingType::Uniform, true},
                 {4, wgpu::ShaderStage::Compute | wgpu::ShaderStage::Fragment,
                  wgpu::BufferBindingType::Storage, true}});

    wgpu::BindGroup bindGroups[2];
    bindGroups[0] = utils::MakeBindGroup(device, bindGroupLayouts[0],
                                         {
                                             {0, mUniformBuffers[0], 0, kBindingSize},
                                             {1, mStorageBuffers[0], 0, kBindingSize},
                                         });
    bindGroups[1] = utils::MakeBindGroup(
        device, bindGroupLayouts[1],
        {{3, mUniformBuffers[1], 0, kBindingSize}, {4, mStorageBuffers[1], 0, kBindingSize}});

    wgpu::PipelineLayoutDescriptor pipelineLayoutDescriptor;
    pipelineLayoutDescriptor.bindGroupLayoutCount = 2;
    pipelineLayoutDescriptor.bindGroupLayouts = bindGroupLayouts;
    pipelineDescriptor.layout = device.CreatePipelineLayout(&pipelineLayoutDescriptor);

    wgpu::RenderPipeline pipeline0 = device.CreateRenderPipeline(&pipelineDescriptor);
    wgpu::RenderPipeline pipeline1 = device.CreateRenderPipeline(&pipelineDescriptor);

    utils::BasicRenderPass renderPass = utils::CreateBasicRenderPass(device, kRTSize, kRTSize);

    wgpu::CommandEncoder commandEncoder = device.CreateCommandEncoder();
    std::array<uint32_t, 2> offsets0 = {0, 0};
    std::array<uint32_t, 2> offsets1 = {mMinUniformBufferOffsetAlignment,
                                        mMinUniformBufferOffsetAlignment};
    wgpu::RenderPassEncoder renderPassEncoder =
        commandEncoder.BeginRenderPass(&renderPass.renderPassInfo);
    renderPassEncoder.SetPipeline(pipeline0);
    renderPassEncoder.SetBindGroup(0, bindGroups[0]);
    renderPassEncoder.SetBindGroup(1, bindGroups[1], offsets0.size(), offsets0.data());
    renderPassEncoder.Draw(3);
    renderPassEncoder.SetPipeline(pipeline1);
    // bind group 0 should be inherited and still available.
    renderPassEncoder.SetBindGroup(1, bindGroups[1], offsets1.size(), offsets1.data());
    renderPassEncoder.Draw(3);
    renderPassEncoder.End();
    wgpu::CommandBuffer commands = commandEncoder.Finish();
    queue.Submit(1, &commands);

    std::vector<uint32_t> expectedData = {6, 8};
    EXPECT_PIXEL_RGBA8_EQ(utils::RGBA8(5, 6, 255, 255), renderPass.color, 0, 0);
    EXPECT_BUFFER_U32_RANGE_EQ(expectedData.data(), mStorageBuffers[1],
                               mMinUniformBufferOffsetAlignment, expectedData.size());
}

// Test inherit dynamic offsets on render pipeline
TEST_P(DynamicBufferOffsetTests, InheritDynamicOffsetsRenderPipeline) {
    // TODO(crbug.com/1497726): Remove when test is no longer flaky on M2
    // devices.
    DAWN_SUPPRESS_TEST_IF(IsApple());
    // TODO(crbug.com/40287156): Remove when test is no longer flaky on Pixel 6
    DAWN_SUPPRESS_TEST_IF(IsVulkan() && IsAndroid() && IsARM());

    // Using default pipeline and setting dynamic offsets
    wgpu::RenderPipeline pipeline = CreateRenderPipeline();
    wgpu::RenderPipeline testPipeline = CreateRenderPipeline(true);

    utils::BasicRenderPass renderPass = utils::CreateBasicRenderPass(device, kRTSize, kRTSize);

    wgpu::CommandEncoder commandEncoder = device.CreateCommandEncoder();
    std::array<uint32_t, 2> offsets = {mMinUniformBufferOffsetAlignment,
                                       mMinUniformBufferOffsetAlignment};
    wgpu::RenderPassEncoder renderPassEncoder =
        commandEncoder.BeginRenderPass(&renderPass.renderPassInfo);
    renderPassEncoder.SetPipeline(pipeline);
    renderPassEncoder.SetBindGroup(0, mBindGroups[0], offsets.size(), offsets.data());
    renderPassEncoder.Draw(3);
    renderPassEncoder.SetPipeline(testPipeline);
    renderPassEncoder.SetBindGroup(1, mBindGroups[1]);
    renderPassEncoder.Draw(3);
    renderPassEncoder.End();
    wgpu::CommandBuffer commands = commandEncoder.Finish();
    queue.Submit(1, &commands);

    std::vector<uint32_t> expectedData = {12, 16};
    EXPECT_PIXEL_RGBA8_EQ(utils::RGBA8(5, 6, 255, 255), renderPass.color, 0, 0);
    EXPECT_BUFFER_U32_RANGE_EQ(expectedData.data(), mStorageBuffers[1],
                               mMinUniformBufferOffsetAlignment, expectedData.size());
}

// Test inherit dynamic offsets on compute pipeline
TEST_P(DynamicBufferOffsetTests, InheritDynamicOffsetsComputePipeline) {
    wgpu::ComputePipeline pipeline = CreateComputePipeline();
    wgpu::ComputePipeline testPipeline = CreateComputePipeline(true);

    std::array<uint32_t, 2> offsets = {mMinUniformBufferOffsetAlignment,
                                       mMinUniformBufferOffsetAlignment};

    wgpu::CommandEncoder commandEncoder = device.CreateCommandEncoder();
    wgpu::ComputePassEncoder computePassEncoder = commandEncoder.BeginComputePass();
    computePassEncoder.SetPipeline(pipeline);
    computePassEncoder.SetBindGroup(0, mBindGroups[0], offsets.size(), offsets.data());
    computePassEncoder.DispatchWorkgroups(1);
    computePassEncoder.SetPipeline(testPipeline);
    computePassEncoder.SetBindGroup(1, mBindGroups[1]);
    computePassEncoder.DispatchWorkgroups(1);
    computePassEncoder.End();
    wgpu::CommandBuffer commands = commandEncoder.Finish();
    queue.Submit(1, &commands);

    std::vector<uint32_t> expectedData = {12, 16};
    EXPECT_BUFFER_U32_RANGE_EQ(expectedData.data(), mStorageBuffers[1],
                               mMinUniformBufferOffsetAlignment, expectedData.size());
}

// Setting multiple dynamic offsets for the same bindgroup in one render pass.
TEST_P(DynamicBufferOffsetTests, UpdateDynamicOffsetsMultipleTimesRenderPipeline) {
    // Using default pipeline and setting dynamic offsets
    wgpu::RenderPipeline pipeline = CreateRenderPipeline();

    utils::BasicRenderPass renderPass = utils::CreateBasicRenderPass(device, kRTSize, kRTSize);

    wgpu::CommandEncoder commandEncoder = device.CreateCommandEncoder();
    std::array<uint32_t, 2> offsets = {mMinUniformBufferOffsetAlignment,
                                       mMinUniformBufferOffsetAlignment};
    std::array<uint32_t, 2> testOffsets = {0, 0};

    wgpu::RenderPassEncoder renderPassEncoder =
        commandEncoder.BeginRenderPass(&renderPass.renderPassInfo);
    renderPassEncoder.SetPipeline(pipeline);
    renderPassEncoder.SetBindGroup(0, mBindGroups[0], offsets.size(), offsets.data());
    renderPassEncoder.Draw(3);
    renderPassEncoder.SetBindGroup(0, mBindGroups[0], testOffsets.size(), testOffsets.data());
    renderPassEncoder.Draw(3);
    renderPassEncoder.End();
    wgpu::CommandBuffer commands = commandEncoder.Finish();
    queue.Submit(1, &commands);

    std::vector<uint32_t> expectedData = {2, 4};
    EXPECT_PIXEL_RGBA8_EQ(utils::RGBA8(1, 2, 255, 255), renderPass.color, 0, 0);
    EXPECT_BUFFER_U32_RANGE_EQ(expectedData.data(), mStorageBuffers[1], 0, expectedData.size());
}

// Setting multiple dynamic offsets for the same bindgroup in one compute pass.
TEST_P(DynamicBufferOffsetTests, UpdateDynamicOffsetsMultipleTimesComputePipeline) {
    wgpu::ComputePipeline pipeline = CreateComputePipeline();

    std::array<uint32_t, 2> offsets = {mMinUniformBufferOffsetAlignment,
                                       mMinUniformBufferOffsetAlignment};
    std::array<uint32_t, 2> testOffsets = {0, 0};

    wgpu::CommandEncoder commandEncoder = device.CreateCommandEncoder();
    wgpu::ComputePassEncoder computePassEncoder = commandEncoder.BeginComputePass();
    computePassEncoder.SetPipeline(pipeline);
    computePassEncoder.SetBindGroup(0, mBindGroups[0], offsets.size(), offsets.data());
    computePassEncoder.DispatchWorkgroups(1);
    computePassEncoder.SetBindGroup(0, mBindGroups[0], testOffsets.size(), testOffsets.data());
    computePassEncoder.DispatchWorkgroups(1);
    computePassEncoder.End();
    wgpu::CommandBuffer commands = commandEncoder.Finish();
    queue.Submit(1, &commands);

    std::vector<uint32_t> expectedData = {2, 4};
    EXPECT_BUFFER_U32_RANGE_EQ(expectedData.data(), mStorageBuffers[1], 0, expectedData.size());
}

namespace {
using ReadBufferUsage = wgpu::BufferUsage;
using OOBRead = bool;
using OOBWrite = bool;

DAWN_TEST_PARAM_STRUCT(ClampedOOBDynamicBufferOffsetParams, ReadBufferUsage, OOBRead, OOBWrite);
}  // anonymous namespace

class ClampedOOBDynamicBufferOffsetTests
    : public DawnTestWithParams<ClampedOOBDynamicBufferOffsetParams> {};

// Test robust buffer access behavior for out of bounds accesses to dynamic buffer bindings.
TEST_P(ClampedOOBDynamicBufferOffsetTests, CheckOOBAccess) {
    static constexpr uint32_t kArrayLength = 10u;

    // Out-of-bounds access will start halfway into the array and index off the end.
    static constexpr uint32_t kOOBOffset = kArrayLength / 2;

    wgpu::BufferBindingType sourceBindingType;
    switch (GetParam().mReadBufferUsage) {
        case wgpu::BufferUsage::Uniform:
            sourceBindingType = wgpu::BufferBindingType::Uniform;
            break;
        case wgpu::BufferUsage::Storage:
            sourceBindingType = wgpu::BufferBindingType::ReadOnlyStorage;
            break;
        default:
            DAWN_UNREACHABLE();
    }
    wgpu::BindGroupLayout bgl = utils::MakeBindGroupLayout(
        device, {{0, wgpu::ShaderStage::Compute, sourceBindingType, true},
                 {1, wgpu::ShaderStage::Compute, wgpu::BufferBindingType::Storage, true}});
    wgpu::PipelineLayout layout = utils::MakeBasicPipelineLayout(device, &bgl);

    wgpu::ComputePipeline pipeline;
    {
        std::ostringstream shader;
        shader << "const kArrayLength: u32 = " << kArrayLength << "u;\n";
        if (GetParam().mOOBRead) {
            shader << "const kReadOffset: u32 = " << kOOBOffset << "u;\n";
        } else {
            shader << "const kReadOffset: u32 = 0u;\n";
        }

        if (GetParam().mOOBWrite) {
            shader << "const kWriteOffset: u32 = " << kOOBOffset << "u;\n";
        } else {
            shader << "const kWriteOffset: u32 = 0u;\n";
        }
        switch (GetParam().mReadBufferUsage) {
            case wgpu::BufferUsage::Uniform:
                shader << R"(
                    struct Src {
                        values : array<vec4u, kArrayLength>
                    }
                    @group(0) @binding(0) var<uniform> src : Src;
                )";
                break;
            case wgpu::BufferUsage::Storage:
                shader << R"(
                    struct Src {
                        values : array<vec4u>
                    }
                    @group(0) @binding(0) var<storage, read> src : Src;
                )";
                break;
            default:
                DAWN_UNREACHABLE();
        }

        shader << R"(
            struct Dst {
                values : array<vec4u>
            }
            @group(0) @binding(1) var<storage, read_write> dst : Dst;
        )";
        shader << R"(
            @compute @workgroup_size(1) fn main() {
                for (var i: u32 = 0u; i < kArrayLength; i = i + 1u) {
                    dst.values[i + kWriteOffset] = src.values[i + kReadOffset];
                }
            }
        )";
        wgpu::ComputePipelineDescriptor pipelineDesc;
        pipelineDesc.layout = layout;
        pipelineDesc.compute.module = utils::CreateShaderModule(device, shader.str().c_str());
        pipeline = device.CreateComputePipeline(&pipelineDesc);
    }

    uint32_t minUniformBufferOffsetAlignment = GetSupportedLimits().minUniformBufferOffsetAlignment;
    uint32_t minStorageBufferOffsetAlignment = GetSupportedLimits().minStorageBufferOffsetAlignment;

    uint32_t arrayByteLength = kArrayLength * 4 * sizeof(uint32_t);

    uint32_t uniformBufferOffset = Align(arrayByteLength, minUniformBufferOffsetAlignment);
    uint32_t storageBufferOffset = Align(arrayByteLength, minStorageBufferOffsetAlignment);

    // Enough space to bind at a dynamic offset.
    uint32_t uniformBufferSize = uniformBufferOffset + arrayByteLength;
    uint32_t storageBufferSize = storageBufferOffset + arrayByteLength;

    // Buffers are padded so we can check that bytes after the bound range are not changed.
    static constexpr uint32_t kEndPadding = 16;

    uint64_t srcBufferSize;
    uint32_t srcBufferByteOffset;
    uint32_t dstBufferByteOffset = storageBufferOffset;
    uint64_t dstBufferSize = storageBufferSize + kEndPadding;
    switch (GetParam().mReadBufferUsage) {
        case wgpu::BufferUsage::Uniform:
            srcBufferSize = uniformBufferSize + kEndPadding;
            srcBufferByteOffset = uniformBufferOffset;
            break;
        case wgpu::BufferUsage::Storage:
            srcBufferSize = storageBufferSize + kEndPadding;
            srcBufferByteOffset = storageBufferOffset;
            break;
        default:
            DAWN_UNREACHABLE();
    }

    std::vector<uint32_t> srcData(srcBufferSize / sizeof(uint32_t));
    std::vector<uint32_t> expectedDst(dstBufferSize / sizeof(uint32_t));

    // Fill the src buffer with 0, 1, 2, ...
    std::iota(srcData.begin(), srcData.end(), 0);
    wgpu::Buffer src = utils::CreateBufferFromData(device, &srcData[0], srcBufferSize,
                                                   GetParam().mReadBufferUsage);

    // Fill the dst buffer with 0xFF.
    memset(expectedDst.data(), 0xFF, dstBufferSize);
    wgpu::Buffer dst =
        utils::CreateBufferFromData(device, &expectedDst[0], dstBufferSize,
                                    wgpu::BufferUsage::Storage | wgpu::BufferUsage::CopySrc);

    // Produce expected data assuming the implementation performs clamping.
    for (uint32_t i = 0; i < kArrayLength; ++i) {
        uint32_t readIndex = GetParam().mOOBRead ? std::min(kOOBOffset + i, kArrayLength - 1) : i;
        uint32_t writeIndex = GetParam().mOOBWrite ? std::min(kOOBOffset + i, kArrayLength - 1) : i;

        for (uint32_t c = 0; c < 4; ++c) {
            uint32_t value = srcData[srcBufferByteOffset / 4 + 4 * readIndex + c];
            expectedDst[dstBufferByteOffset / 4 + 4 * writeIndex + c] = value;
        }
    }

    std::array<uint32_t, 2> dynamicOffsets = {srcBufferByteOffset, dstBufferByteOffset};

    wgpu::BindGroup bindGroup = utils::MakeBindGroup(device, bgl,
                                                     {
                                                         {0, src, 0, arrayByteLength},
                                                         {1, dst, 0, arrayByteLength},
                                                     });

    wgpu::CommandEncoder commandEncoder = device.CreateCommandEncoder();
    wgpu::ComputePassEncoder computePassEncoder = commandEncoder.BeginComputePass();
    computePassEncoder.SetPipeline(pipeline);
    computePassEncoder.SetBindGroup(0, bindGroup, dynamicOffsets.size(), dynamicOffsets.data());
    computePassEncoder.DispatchWorkgroups(1);
    computePassEncoder.End();
    wgpu::CommandBuffer commands = commandEncoder.Finish();
    queue.Submit(1, &commands);

    EXPECT_BUFFER_U32_RANGE_EQ(expectedDst.data(), dst, 0, dstBufferSize / sizeof(uint32_t));
}

DAWN_INSTANTIATE_TEST(DynamicBufferOffsetTests,
                      D3D11Backend(),
                      D3D12Backend(),
                      D3D12Backend({}, {"d3d12_use_root_signature_version_1_1"}),
                      MetalBackend(),
                      OpenGLBackend(),
                      OpenGLESBackend(),
                      VulkanBackend());

// Only instantiate on D3D12 / Metal where we are sure of the robustness implementation.
// Tint injects clamping in the shader. OpenGL(ES) / Vulkan robustness is less constrained.
DAWN_INSTANTIATE_TEST_P(ClampedOOBDynamicBufferOffsetTests,
                        {D3D12Backend(), D3D12Backend({}, {"d3d12_use_root_signature_version_1_1"}),
                         MetalBackend()},
                        {wgpu::BufferUsage::Uniform, wgpu::BufferUsage::Storage},
                        {false, true},
                        {false, true});

}  // anonymous namespace
}  // namespace dawn
