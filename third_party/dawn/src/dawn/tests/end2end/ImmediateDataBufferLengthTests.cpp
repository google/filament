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

#include <string>
#include <vector>

#include "dawn/common/Assert.h"
#include "dawn/common/Constants.h"
#include "dawn/tests/DawnTest.h"
#include "dawn/utils/ComboRenderPipelineDescriptor.h"
#include "dawn/utils/WGPUHelpers.h"

namespace dawn {
namespace {

// Storage buffer size in bytes.
enum class Size { Small = 64u, Medium = 256u, Large = 512u };

// Test fixture specifically for testing the interaction between immediate data (SetImmediates)
// and storage buffer length tracking. This is critical to ensure that when both immediate
// constants and buffer length data are uploaded to the same GPU buffer, they don't interfere
// with each other across multiple Apply calls with changing pipeline/bind group state.
class ImmediateDataBufferLengthTest : public DawnTest {
  protected:
    void GetRequiredLimits(const dawn::utils::ComboLimits& supported,
                           dawn::utils::ComboLimits& required) override {
        // Skip compat backends
        // TODO(crbug.com/458102548): Remove when implemented for GLES.
        DAWN_TEST_UNSUPPORTED_IF(supported.maxImmediateSize == 0);
    }

    void SetUp() override {
        DawnTest::SetUp();

        // Create a result buffer.
        // Maximum: 2 draw calls, 3 storage buffers and 4 immediates.
        wgpu::BufferDescriptor bufferDesc;
        bufferDesc.size =
            kMaxExecutionTime * kResultSizeInExpectation * kImmediateConstantElementByteSize;
        bufferDesc.usage = wgpu::BufferUsage::Storage | wgpu::BufferUsage::CopySrc;
        mResultBuffer = device.CreateBuffer(&bufferDesc);
    }

    // Helper to create bind groups with different buffer configurations
    // Creates storage buffers of specified sizes and binds them to a bind group.
    // This tests buffer length tracking by creating buffers with known sizes that
    // can be queried in shaders using arrayLength().
    wgpu::BindGroup MakeBindGroupWithBuffers(wgpu::BindGroupLayout layout,
                                             const std::vector<Size>& bufferSizes) {
        const uint32_t bufferCount = bufferSizes.size();
        wgpu::BufferDescriptor bufferDesc;
        bufferDesc.usage = wgpu::BufferUsage::Storage;

        std::vector<wgpu::BindGroupEntry> entries;
        wgpu::BindGroupEntry entry;

        for (uint32_t i = 0; i < bufferCount; ++i) {
            bufferDesc.size = static_cast<uint32_t>(bufferSizes[i]);
            wgpu::Buffer buffer = device.CreateBuffer(&bufferDesc);
            mStorageBuffers.push_back(buffer);
            entry.binding = i;
            entry.buffer = buffer;
            entries.push_back(entry);
        }

        wgpu::BindGroupDescriptor bindGroupDesc;
        bindGroupDesc.layout = layout;
        bindGroupDesc.entryCount = entries.size();
        bindGroupDesc.entries = entries.data();
        return device.CreateBindGroup(&bindGroupDesc);
    }

    // Helper to create unified shader with compute, vertex, and fragment entry points
    // This shader contains arrayLength() calls to query storage buffer sizes and
    // accesses immediate constants. Both data types are written to a shared result buffer
    // to verify they don't interfere with each other during GPU uploads.
    wgpu::ShaderModule CreateUnifiedShaderModule(uint32_t immediateCount, uint32_t bufferCount) {
        std::string bufferDeclarations;
        std::string lengthComputation;

        for (uint32_t i = 0; i < bufferCount; i++) {
            bufferDeclarations += "    @group(0) @binding(" + std::to_string(i) +
                                  ") var<storage, read> buffer" + std::to_string(i) +
                                  " : array<f32>;\n";
            lengthComputation += "    result[resultIndex].bufferLengths[" + std::to_string(i) +
                                 "] = arrayLength(&buffer" + std::to_string(i) + ");\n";
        }

        std::string immediateStruct = "struct ImmediateData {\n";
        immediateStruct += "        resultIndex : u32,\n";  // Add result index as first field
        for (uint32_t i = 0; i < immediateCount; i++) {
            immediateStruct += "        value" + std::to_string(i) + " : u32,\n";
        }
        immediateStruct += "    }\n";

        std::string immediateAccess;
        for (uint32_t i = 0; i < immediateCount; i++) {
            immediateAccess += "    result[resultIndex].immediates[" + std::to_string(i) +
                               "] = immediateData.value" + std::to_string(i) + ";\n";
        }

        // Unified shader with all entry points
        std::string shader = R"(
            struct ResultData {
                bufferLengths : array<u32, 3>,
                immediates : array<u32, 4>,
            }
            @group(1) @binding(0) var<storage, read_write> result : array<ResultData, 2>;
)" + bufferDeclarations + immediateStruct +
                             R"(
            var<immediate> immediateData: ImmediateData;

            // Shared function for buffer length and immediate data processing
            fn processData() {
                let resultIndex = immediateData.resultIndex;
)" + lengthComputation + immediateAccess +
                             R"(
            }

            // Compute entry point
            @compute @workgroup_size(1) fn csMain() {
                processData();
            }

            // Vertex entry point
            @vertex fn vsMain() -> @builtin(position) vec4f {
                return vec4f(0.0, 0.0, 0.0, 1.0);
            }

            // Fragment entry point
            @fragment fn fsMain() -> @location(0) vec4f {
                processData();
                return vec4f(1.0, 0.0, 0.0, 1.0);
            }
        )";

        return utils::CreateShaderModule(device, shader.c_str());
    }

    // Helper to create a bind group layout with storage buffers for any shader stage
    // Creates layouts with 1-3 storage buffers that can be used by both compute and fragment
    // shaders. This enables testing buffer length tracking across different pipeline types.
    wgpu::BindGroupLayout MakeStorageBindGroupLayout(uint32_t bufferCount) {
        switch (bufferCount) {
            case 1u:
                return utils::MakeBindGroupLayout(
                    device, {{0, wgpu::ShaderStage::Compute | wgpu::ShaderStage::Fragment,
                              wgpu::BufferBindingType::ReadOnlyStorage}});
            case 2u:
                return utils::MakeBindGroupLayout(
                    device, {{0, wgpu::ShaderStage::Compute | wgpu::ShaderStage::Fragment,
                              wgpu::BufferBindingType::ReadOnlyStorage},
                             {1, wgpu::ShaderStage::Compute | wgpu::ShaderStage::Fragment,
                              wgpu::BufferBindingType::ReadOnlyStorage}});
            case 3u:
                return utils::MakeBindGroupLayout(
                    device, {{0, wgpu::ShaderStage::Compute | wgpu::ShaderStage::Fragment,
                              wgpu::BufferBindingType::ReadOnlyStorage},
                             {1, wgpu::ShaderStage::Compute | wgpu::ShaderStage::Fragment,
                              wgpu::BufferBindingType::ReadOnlyStorage},
                             {2, wgpu::ShaderStage::Compute | wgpu::ShaderStage::Fragment,
                              wgpu::BufferBindingType::ReadOnlyStorage}});
            default:
                DAWN_UNREACHABLE();
                return {};
        }
    }

    // Helper to create unified result bind group (for both compute and render)
    // The result buffer stores both buffer length data and immediate data from shader execution.
    // Layout: [bufferLength0, bufferLength1, bufferLength2, immediate0, immediate1, immediate2,
    // immediate3] This unified approach allows both compute and render pipelines to write results
    // to the same buffer.
    wgpu::BindGroup MakeResultBindGroup() {
        wgpu::BindGroupLayout resultLayout = utils::MakeBindGroupLayout(
            device, {{0, wgpu::ShaderStage::Compute | wgpu::ShaderStage::Fragment,
                      wgpu::BufferBindingType::Storage}});
        return utils::MakeBindGroup(device, resultLayout, {{0, mResultBuffer}});
    }

    // Creates a pipeline layout that supports both immediate data and storage buffer binding.
    // The immediate size calculation includes space for:
    // - User immediate data (immediateCount values)
    // - One additional slot for draw/dispatch index tracking
    // This layout is used by both compute and render pipelines in the tests.
    wgpu::PipelineLayout CreatePipelineLayout(uint32_t immediateCount, uint32_t bufferCount) {
        wgpu::BindGroupLayout storageLayout = MakeStorageBindGroupLayout(bufferCount);
        wgpu::BindGroupLayout resultLayout = utils::MakeBindGroupLayout(
            device, {{0, wgpu::ShaderStage::Compute | wgpu::ShaderStage::Fragment,
                      wgpu::BufferBindingType::Storage}});

        wgpu::PipelineLayoutDescriptor plDesc;
        wgpu::BindGroupLayout layouts[] = {storageLayout, resultLayout};
        plDesc.bindGroupLayoutCount = 2;
        plDesc.bindGroupLayouts = layouts;

        // Calculate exact size needed and +1 for drawIndex/dispatchIndex parameter.
        plDesc.immediateSize = (immediateCount + 1) * kImmediateConstantElementByteSize;

        return device.CreatePipelineLayout(&plDesc);
    }

    template <typename Pass>
    void SetStorageBuffer(Pass pass, const std::vector<Size>& bufferSizes) {
        const uint32_t bufferCount = bufferSizes.size();
        pass.SetBindGroup(
            0, MakeBindGroupWithBuffers(MakeStorageBindGroupLayout(bufferCount), bufferSizes));
        for (uint32_t i = 0; i < bufferCount; ++i) {
            mCurrentBufferLengths[i] = static_cast<uint32_t>(bufferSizes[i]) / sizeof(uint32_t);
        }
    }

    template <typename Pass>
    void SetImmediates(Pass pass, const std::vector<uint32_t>& immediates) {
        std::vector<uint32_t> immediateData;
        immediateData.push_back(resultIndex);
        immediateData.insert(immediateData.end(), immediates.begin(), immediates.end());
        pass.SetImmediates(0, reinterpret_cast<uint8_t*>(immediateData.data()),
                           immediateData.size() * sizeof(uint32_t));
        for (uint32_t i = 0; i < immediates.size(); ++i) {
            mCurrentImmediateData[i] = immediates[i];
        }
    }

    // Records expected results for validation after GPU execution.
    void OnOperationWritingExpected() {
        uint32_t startOffset = resultIndex * kResultSizeInExpectation;

        for (uint32_t i = 0; i < mCurrentPipelineStorageBufferCount; ++i) {
            expectedResults[startOffset + i] = mCurrentBufferLengths[i];
        }

        startOffset += kMaxStorageBufferCount;
        for (uint32_t i = 0; i < mCurrentPipelineImmediateDataCount; ++i) {
            expectedResults[startOffset + i] = mCurrentImmediateData[i];
        }

        resultIndex++;
    }

    static constexpr uint32_t kMaxExecutionTime = 2u;
    static constexpr uint32_t kMaxImmediateCount = 4u;
    static constexpr uint32_t kMaxStorageBufferCount = 3u;
    static constexpr uint32_t kResultSizeInExpectation =
        kMaxImmediateCount + kMaxStorageBufferCount;

    std::vector<wgpu::Buffer> mStorageBuffers;
    wgpu::Buffer mResultBuffer;

    // Test state tracking - maintains current pipeline configuration and expected results
    std::array<uint32_t, kMaxStorageBufferCount> mCurrentBufferLengths = {};
    std::array<uint32_t, kMaxImmediateCount> mCurrentImmediateData = {};
    uint32_t mCurrentPipelineStorageBufferCount = 0;
    uint32_t mCurrentPipelineImmediateDataCount = 0;

    std::array<uint32_t, kMaxExecutionTime * kResultSizeInExpectation> expectedResults = {};
    uint32_t resultIndex = 0;
};

class ImmediateDataBufferLengthComputePipelineTest : public ImmediateDataBufferLengthTest {
  protected:
    // Helper to create compute pipelines with different numbers of immediate constants and storage
    // buffers
    wgpu::ComputePipeline CreateComputePipeline(uint32_t immediateCount, uint32_t bufferCount) {
        wgpu::ComputePipelineDescriptor pipelineDesc;
        pipelineDesc.layout = CreatePipelineLayout(immediateCount, bufferCount);
        pipelineDesc.compute.module = CreateUnifiedShaderModule(immediateCount, bufferCount);
        return device.CreateComputePipeline(&pipelineDesc);
    }

    void UseComputePipeline(wgpu::ComputePassEncoder pass,
                            uint32_t immediateCount,
                            uint32_t bufferCount) {
        mCurrentPipelineStorageBufferCount = bufferCount;
        mCurrentPipelineImmediateDataCount = immediateCount;
        pass.SetPipeline(CreateComputePipeline(immediateCount, bufferCount));
    }

    void Dispatch(wgpu::ComputePassEncoder pass) {
        if (resultIndex == 0) {
            wgpu::BindGroup resultBindGroup = MakeResultBindGroup();
            pass.SetBindGroup(1, resultBindGroup);
        }

        pass.DispatchWorkgroups(1);

        OnOperationWritingExpected();
    }

    void CheckExpectedResults() {
        EXPECT_BUFFER_U32_RANGE_EQ(expectedResults.data(), mResultBuffer, 0, expectedResults.size())
            << "dispatch index: " << std::to_string(resultIndex);
    }
};

class ImmediateDataBufferLengthRenderPipelineTest : public ImmediateDataBufferLengthTest {
  protected:
    void SetUp() override {
        ImmediateDataBufferLengthTest::SetUp();

        // Create render target for render pipeline tests
        wgpu::TextureDescriptor textureDesc;
        textureDesc.size = {1, 1, 1};
        textureDesc.format = wgpu::TextureFormat::RGBA8Unorm;
        textureDesc.usage = wgpu::TextureUsage::RenderAttachment;
        mRenderTarget = device.CreateTexture(&textureDesc);
        mRenderTargetView = mRenderTarget.CreateView();
    }

    // Helper to create render pipelines with different numbers of immediate constants and storage
    // buffers
    wgpu::RenderPipeline CreateRenderPipeline(uint32_t immediateCount, uint32_t bufferCount) {
        utils::ComboRenderPipelineDescriptor pipelineDesc;
        pipelineDesc.layout = CreatePipelineLayout(immediateCount, bufferCount);

        wgpu::ShaderModule module = CreateUnifiedShaderModule(immediateCount, bufferCount);
        pipelineDesc.vertex.module = module;
        pipelineDesc.cFragment.module = module;

        pipelineDesc.primitive.topology = wgpu::PrimitiveTopology::PointList;
        pipelineDesc.cTargets[0].format = wgpu::TextureFormat::RGBA8Unorm;

        return device.CreateRenderPipeline(&pipelineDesc);
    }

    void UseRenderPipeline(wgpu::RenderPassEncoder pass,
                           uint32_t immediateCount,
                           uint32_t bufferCount) {
        mCurrentPipelineStorageBufferCount = bufferCount;
        mCurrentPipelineImmediateDataCount = immediateCount;
        pass.SetPipeline(CreateRenderPipeline(immediateCount, bufferCount));
    }

    void Draw(wgpu::RenderPassEncoder pass) {
        if (resultIndex == 0) {
            wgpu::BindGroup resultBindGroup = MakeResultBindGroup();
            pass.SetBindGroup(1, resultBindGroup);
        }

        pass.Draw(1);

        OnOperationWritingExpected();
    }

    void CheckExpectedResults() {
        EXPECT_BUFFER_U32_RANGE_EQ(expectedResults.data(), mResultBuffer, 0, expectedResults.size())
            << "draw index: " << std::to_string(resultIndex);
    }

    wgpu::Texture mRenderTarget;
    wgpu::TextureView mRenderTargetView;
};

// Test: SetBindGroup + SetImmediates + Draw
// This is the basic case to ensure immediate data and buffer lengths work together
TEST_P(ImmediateDataBufferLengthComputePipelineTest, BasicImmediateAndBufferLength) {
    wgpu::CommandEncoder encoder = device.CreateCommandEncoder();
    wgpu::ComputePassEncoder pass = encoder.BeginComputePass();

    UseComputePipeline(pass, 3u, 3u);
    SetStorageBuffer(pass, {Size::Small, Size::Medium, Size::Large});
    SetImmediates(pass, {42, 123, 456});
    Dispatch(pass);

    pass.End();
    wgpu::CommandBuffer commands = encoder.Finish();
    queue.Submit(1, &commands);

    CheckExpectedResults();
}

// Test: SetBindGroup + SetImmediates + Draw + SetPipeline (different immediates) + Draw
// This tests that changing pipelines with different immediate counts still preserves buffer lengths
TEST_P(ImmediateDataBufferLengthComputePipelineTest, PipelineChangeWithDifferentImmediates) {
    wgpu::CommandEncoder encoder = device.CreateCommandEncoder();
    wgpu::ComputePassEncoder pass = encoder.BeginComputePass();

    // First draw with pipeline1 - writes to index 0
    UseComputePipeline(pass, 1u, 2u);
    SetStorageBuffer(pass, {Size::Small, Size::Medium});
    SetImmediates(pass, {100});
    Dispatch(pass);

    // Change to pipeline2 with more immediates
    UseComputePipeline(pass, 2u, 2u);
    SetImmediates(pass, {300, 400});
    Dispatch(pass);

    pass.End();
    wgpu::CommandBuffer commands = encoder.Finish();
    queue.Submit(1, &commands);

    CheckExpectedResults();
}

// Test: Compute pipeline change with same layout but different immediate counts
// This tests a potentially problematic case where pipelines have the same bind group layout
// but different immediate data sizes. This might expose issues with immediate data validation.
TEST_P(ImmediateDataBufferLengthComputePipelineTest, PipelineChangeWithReducedImmediateCounts) {
    wgpu::CommandEncoder encoder = device.CreateCommandEncoder();
    wgpu::ComputePassEncoder pass = encoder.BeginComputePass();

    // First dispatch with pipeline1 (3 immediates) - writes to index 0
    UseComputePipeline(pass, 3u, 2u);
    SetStorageBuffer(pass, {Size::Small, Size::Medium});
    SetImmediates(pass, {100, 200, 300});
    Dispatch(pass);

    // Second dispatch with pipeline2 (2 immediates, same layout) - writes to index 1
    UseComputePipeline(pass, 2u, 2u);
    SetImmediates(pass, {500, 600});
    Dispatch(pass);

    pass.End();
    wgpu::CommandBuffer commands = encoder.Finish();
    queue.Submit(1, &commands);

    CheckExpectedResults();
}

// Test: SetBindGroup + SetImmediates + Draw + SetPipeline (different buffer count) +
// SetBindGroup + Draw This tests changing both pipeline and bind group, ensuring buffer length
// tracking adapts correctly
TEST_P(ImmediateDataBufferLengthComputePipelineTest, PipelineAndBindGroupChange) {
    wgpu::CommandEncoder encoder = device.CreateCommandEncoder();
    wgpu::ComputePassEncoder pass = encoder.BeginComputePass();

    // First draw with 2 buffers
    UseComputePipeline(pass, 2u, 2u);
    SetStorageBuffer(pass, {Size::Small, Size::Medium});
    SetImmediates(pass, {111, 222});
    Dispatch(pass);

    // Change to pipeline with 3 buffers and update bind group
    UseComputePipeline(pass, 2u, 3u);
    SetStorageBuffer(pass, {Size::Large, Size::Small, Size::Medium});
    SetImmediates(pass, {333, 444});
    Dispatch(pass);

    pass.End();

    wgpu::CommandBuffer commands = encoder.Finish();
    queue.Submit(1, &commands);

    CheckExpectedResults();
}

// Test: Multiple SetImmediates calls with buffer length updates
// This tests the edge case where immediate data is updated multiple times
TEST_P(ImmediateDataBufferLengthComputePipelineTest, MultipleImmediateDataUpdates) {
    wgpu::CommandEncoder encoder = device.CreateCommandEncoder();
    wgpu::ComputePassEncoder pass = encoder.BeginComputePass();

    UseComputePipeline(pass, 3u, 1u);
    SetStorageBuffer(pass, {Size::Small});
    SetImmediates(pass, {10, 20, 30});
    SetImmediates(pass, {40, 50, 60});
    SetImmediates(pass, {70, 80, 90});
    Dispatch(pass);

    pass.End();

    wgpu::CommandBuffer commands = encoder.Finish();
    queue.Submit(1, &commands);

    CheckExpectedResults();
}

// Test: Interleaved SetBindGroup and SetImmediates calls
// This tests complex state management scenarios
TEST_P(ImmediateDataBufferLengthComputePipelineTest, InterleavedUpdates) {
    wgpu::CommandEncoder encoder = device.CreateCommandEncoder();
    wgpu::ComputePassEncoder pass = encoder.BeginComputePass();

    // Interleave bind group and immediate data updates
    UseComputePipeline(pass, 2u, 2u);
    SetStorageBuffer(pass, {Size::Small, Size::Medium});
    SetImmediates(pass, {1000, 2000});
    Dispatch(pass);

    SetStorageBuffer(pass, {Size::Medium, Size::Large});
    SetImmediates(pass, {3000, 4000});
    Dispatch(pass);

    pass.End();

    wgpu::CommandBuffer commands = encoder.Finish();
    queue.Submit(1, &commands);

    CheckExpectedResults();
}

DAWN_INSTANTIATE_TEST(ImmediateDataBufferLengthComputePipelineTest,
                      D3D11Backend(),
                      D3D12Backend(),
                      MetalBackend(),
                      OpenGLBackend(),
                      OpenGLESBackend(),
                      VulkanBackend(),
                      WebGPUBackend());

// Test: Render pipeline with SetBindGroup + SetImmediates + Draw
// This tests the basic case for render pipelines
TEST_P(ImmediateDataBufferLengthRenderPipelineTest, RenderPipelineBasicImmediateAndBufferLength) {
    wgpu::CommandEncoder encoder = device.CreateCommandEncoder();
    utils::ComboRenderPassDescriptor renderPassDesc({mRenderTargetView});
    wgpu::RenderPassEncoder pass = encoder.BeginRenderPass(&renderPassDesc);

    UseRenderPipeline(pass, 3u, 2u);
    SetStorageBuffer(pass, {Size::Small, Size::Medium});
    SetImmediates(pass, {789, 987, 654});
    Draw(pass);

    pass.End();

    wgpu::CommandBuffer commands = encoder.Finish();
    queue.Submit(1, &commands);

    CheckExpectedResults();
}

// Test: Render pipeline with multiple draws and pipeline changes (updated to use single result
// buffer)
TEST_P(ImmediateDataBufferLengthRenderPipelineTest, PipelineChangeWithIncreasedImmediates) {
    wgpu::CommandEncoder encoder = device.CreateCommandEncoder();

    utils::ComboRenderPassDescriptor renderPassDesc({mRenderTargetView});
    wgpu::RenderPassEncoder pass = encoder.BeginRenderPass(&renderPassDesc);

    // First draw with pipeline1
    UseRenderPipeline(pass, 2u, 2u);
    SetStorageBuffer(pass, {Size::Small, Size::Medium});
    SetImmediates(pass, {1100, 1200});
    Draw(pass);

    // Second draw with pipeline2
    UseRenderPipeline(pass, 3u, 2u);
    SetImmediates(pass, {1300, 1400, 1500});
    Draw(pass);

    pass.End();

    wgpu::CommandBuffer commands = encoder.Finish();
    queue.Submit(1, &commands);

    CheckExpectedResults();
}

// Test: Pipeline change with same layout but different immediate counts
// This tests a potentially problematic case where pipelines have the same bind group layout
// but different immediate data sizes. This might expose issues with immediate data validation.
TEST_P(ImmediateDataBufferLengthRenderPipelineTest, PipelineChangeWithReducedImmediateCounts) {
    wgpu::CommandEncoder encoder = device.CreateCommandEncoder();

    utils::ComboRenderPassDescriptor renderPassDesc({mRenderTargetView});
    wgpu::RenderPassEncoder pass = encoder.BeginRenderPass(&renderPassDesc);

    // First draw with pipeline1 (3 immediates) - writes to index 0
    UseRenderPipeline(pass, 3u, 2u);
    SetStorageBuffer(pass, {Size::Small, Size::Medium});
    SetImmediates(pass, {1000, 2000, 3000});
    Draw(pass);

    // Second draw with pipeline2 (2 immediates, same layout) - writes to index 1
    UseRenderPipeline(pass, 2u, 2u);
    SetImmediates(pass, {5000, 6000});
    Draw(pass);

    pass.End();

    wgpu::CommandBuffer commands = encoder.Finish();
    queue.Submit(1, &commands);

    CheckExpectedResults();
}

// Test: Render pipeline with buffer count changes (updated to use single result buffer)
TEST_P(ImmediateDataBufferLengthRenderPipelineTest, PipelineAndBindGroupChange) {
    wgpu::CommandEncoder encoder = device.CreateCommandEncoder();

    utils::ComboRenderPassDescriptor renderPassDesc({mRenderTargetView});
    wgpu::RenderPassEncoder pass = encoder.BeginRenderPass(&renderPassDesc);

    // First draw with 2 buffers
    UseRenderPipeline(pass, 2u, 2u);
    SetStorageBuffer(pass, {Size::Small, Size::Medium});
    SetImmediates(pass, {2111, 2222});
    Draw(pass);

    // Second draw with 3 buffers - writes to index 1
    UseRenderPipeline(pass, 2u, 3u);
    SetStorageBuffer(pass, {Size::Large, Size::Small, Size::Medium});
    SetImmediates(pass, {2333, 2444});
    Draw(pass);

    pass.End();

    wgpu::CommandBuffer commands = encoder.Finish();
    queue.Submit(1, &commands);

    CheckExpectedResults();
}

// Test: Render pipeline with multiple immediate data updates
TEST_P(ImmediateDataBufferLengthRenderPipelineTest, MultipleImmediateDataUpdates) {
    wgpu::CommandEncoder encoder = device.CreateCommandEncoder();

    utils::ComboRenderPassDescriptor renderPassDesc({mRenderTargetView});
    wgpu::RenderPassEncoder pass = encoder.BeginRenderPass(&renderPassDesc);

    UseRenderPipeline(pass, 3u, 1u);
    SetStorageBuffer(pass, {Size::Small});
    SetImmediates(pass, {3010, 3020, 3030});
    SetImmediates(pass, {3040, 3050, 3060});
    SetImmediates(pass, {3070, 3080, 3090});
    Draw(pass);

    pass.End();

    wgpu::CommandBuffer commands = encoder.Finish();
    queue.Submit(1, &commands);

    CheckExpectedResults();
}

// Test: Render pipeline with interleaved updates
TEST_P(ImmediateDataBufferLengthRenderPipelineTest, InterleavedUpdates) {
    // TODO(crbug.com/366291600): Android bots failed with incorrect arrayLength value in second
    // dispatch. Should be Size::Medium, Size::Large but got Size::Small, Size::Medium.
    DAWN_SUPPRESS_TEST_IF(IsAndroid());

    wgpu::CommandEncoder encoder = device.CreateCommandEncoder();

    utils::ComboRenderPassDescriptor renderPassDesc({mRenderTargetView});
    wgpu::RenderPassEncoder pass = encoder.BeginRenderPass(&renderPassDesc);

    UseRenderPipeline(pass, 2u, 2u);
    SetStorageBuffer(pass, {Size::Small, Size::Medium});
    SetImmediates(pass, {4000, 5000});
    Draw(pass);

    SetStorageBuffer(pass, {Size::Medium, Size::Large});
    SetImmediates(pass, {6000, 7000});
    Draw(pass);

    pass.End();

    wgpu::CommandBuffer commands = encoder.Finish();
    queue.Submit(1, &commands);

    CheckExpectedResults();
}

DAWN_INSTANTIATE_TEST(ImmediateDataBufferLengthRenderPipelineTest,
                      D3D11Backend(),
                      D3D12Backend(),
                      MetalBackend(),
                      OpenGLBackend(),
                      OpenGLESBackend(),
                      VulkanBackend(),
                      WebGPUBackend());

}  // anonymous namespace
}  // namespace dawn
