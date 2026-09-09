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

#include <utility>
#include <vector>

#include "src/dawn/common/Enumerator.h"
#include "src/dawn/native/CommandBuffer.h"
#include "src/dawn/native/Commands.h"
#include "src/dawn/native/ComputePassEncoder.h"
#include "src/dawn/tests/DawnNativeTest.h"
#include "src/dawn/utils/TestUtils.h"
#include "src/dawn/utils/WGPUHelpers.h"
#include "src/utils/compiler.h"

namespace dawn::native {
namespace {

class CommandBufferEncodingTests : public DawnNativeTest {
  protected:
    void ExpectCommands(dawn::native::CommandIterator* commands,
                        std::vector<std::pair<dawn::native::Command,
                                              std::function<void(dawn::native::CommandIterator*)>>>
                            expectedCommands) {
        dawn::native::Command commandId;
        for (uint32_t commandIndex = 0; commands->NextCommandId(&commandId); ++commandIndex) {
            ASSERT_LT(commandIndex, expectedCommands.size()) << "Unexpected command";
            ASSERT_EQ(commandId, expectedCommands[commandIndex].first)
                << "at command " << commandIndex;
            expectedCommands[commandIndex].second(commands);
        }
    }
};

// Indirect dispatch validation changes the bind groups in the middle
// of a pass. Test that bindings are restored after the validation runs.
TEST_F(CommandBufferEncodingTests, ComputePassEncoderIndirectDispatchStateRestoration) {
    wgpu::BindGroupLayout staticLayout =
        utils::MakeBindGroupLayout(device, {{
                                               0,
                                               wgpu::ShaderStage::Compute,
                                               wgpu::BufferBindingType::Uniform,
                                           }});

    wgpu::BindGroupLayout dynamicLayout =
        utils::MakeBindGroupLayout(device, {{
                                               0,
                                               wgpu::ShaderStage::Compute,
                                               wgpu::BufferBindingType::Uniform,
                                               true,
                                           }});

    // Create a simple pipeline
    wgpu::ComputePipelineDescriptor csDesc;
    csDesc.compute.module = utils::CreateShaderModule(device, R"(
        @compute @workgroup_size(1, 1, 1)
        fn main() {
        })");

    wgpu::PipelineLayout pl0 = utils::MakePipelineLayout(device, {staticLayout, dynamicLayout});
    csDesc.layout = pl0;
    wgpu::ComputePipeline pipeline0 = device.CreateComputePipeline(&csDesc);

    wgpu::PipelineLayout pl1 = utils::MakePipelineLayout(device, {dynamicLayout, staticLayout});
    csDesc.layout = pl1;
    wgpu::ComputePipeline pipeline1 = device.CreateComputePipeline(&csDesc);

    // Create buffers to use for both the indirect buffer and the bind groups.
    wgpu::Buffer indirectBuffer =
        utils::CreateBufferFromData<uint32_t>(device, wgpu::BufferUsage::Indirect, {1, 2, 3, 4});

    wgpu::BufferDescriptor uniformBufferDesc = {};
    uniformBufferDesc.size = 512;
    uniformBufferDesc.usage = wgpu::BufferUsage::Uniform;
    wgpu::Buffer uniformBuffer = device.CreateBuffer(&uniformBufferDesc);

    wgpu::BindGroup staticBG = utils::MakeBindGroup(device, staticLayout, {{0, uniformBuffer}});

    wgpu::BindGroup dynamicBG =
        utils::MakeBindGroup(device, dynamicLayout, {{0, uniformBuffer, 0, 256}});

    uint32_t dynamicOffset = 256;
    ityp::vector<BindGroupIndex, uint32_t> emptyDynamicOffsets = {};
    ityp::vector<BindGroupIndex, uint32_t> singleDynamicOffset = {dynamicOffset};

    // Begin encoding commands.
    wgpu::CommandEncoder encoder = device.CreateCommandEncoder();
    wgpu::ComputePassEncoder pass = encoder.BeginComputePass();

    CommandBufferStateTracker* stateTracker =
        FromAPI(pass.Get())->GetCommandBufferStateTrackerForTesting();

    // Perform a dispatch indirect which will be preceded by a validation dispatch.
    pass.SetPipeline(pipeline0);
    pass.SetBindGroup(0, staticBG);
    pass.SetBindGroup(1, dynamicBG, 1, &dynamicOffset);
    EXPECT_EQ(ToAPI(stateTracker->GetComputePipeline()), pipeline0.Get());

    pass.DispatchWorkgroupsIndirect(indirectBuffer, 0);

    // Expect restored state.
    EXPECT_EQ(ToAPI(stateTracker->GetComputePipeline()), pipeline0.Get());
    EXPECT_EQ(ToAPI(stateTracker->GetPipelineLayout()), pl0.Get());
    EXPECT_TRUE(std::ranges::equal(stateTracker->GetDynamicOffsets(BindGroupIndex(0u)),
                                   emptyDynamicOffsets));
    EXPECT_TRUE(std::ranges::equal(stateTracker->GetDynamicOffsets(BindGroupIndex(1u)),
                                   singleDynamicOffset));

    // Dispatch again to check that the restored state can be used.
    // Also pass an indirect offset which should get replaced with the offset
    // into the scratch indirect buffer (0).
    pass.DispatchWorkgroupsIndirect(indirectBuffer, 4);

    // Expect restored state.
    EXPECT_EQ(ToAPI(stateTracker->GetComputePipeline()), pipeline0.Get());
    EXPECT_EQ(ToAPI(stateTracker->GetPipelineLayout()), pl0.Get());
    EXPECT_TRUE(std::ranges::equal(stateTracker->GetDynamicOffsets(BindGroupIndex(0u)),
                                   emptyDynamicOffsets));
    EXPECT_TRUE(std::ranges::equal(stateTracker->GetDynamicOffsets(BindGroupIndex(1u)),
                                   singleDynamicOffset));

    // Change the pipeline
    pass.SetPipeline(pipeline1);
    pass.SetBindGroup(0, dynamicBG, 1, &dynamicOffset);
    pass.SetBindGroup(1, staticBG);
    EXPECT_EQ(ToAPI(stateTracker->GetComputePipeline()), pipeline1.Get());
    EXPECT_EQ(ToAPI(stateTracker->GetPipelineLayout()), pl1.Get());

    pass.DispatchWorkgroupsIndirect(indirectBuffer, 0);

    // Expect restored state.
    EXPECT_EQ(ToAPI(stateTracker->GetComputePipeline()), pipeline1.Get());
    EXPECT_EQ(ToAPI(stateTracker->GetPipelineLayout()), pl1.Get());
    EXPECT_TRUE(std::ranges::equal(stateTracker->GetDynamicOffsets(BindGroupIndex(0u)),
                                   singleDynamicOffset));
    EXPECT_TRUE(std::ranges::equal(stateTracker->GetDynamicOffsets(BindGroupIndex(1u)),
                                   emptyDynamicOffsets));

    pass.End();

    wgpu::CommandBuffer commandBuffer = encoder.Finish();

    auto ExpectSetPipeline = [](wgpu::ComputePipeline pipeline) {
        return [pipeline](CommandIterator* commands) {
            auto* cmd = commands->NextCommand<SetComputePipelineCmd>();
            EXPECT_EQ(ToAPI(cmd->pipeline.Get()), pipeline.Get());
        };
    };

    auto ExpectSetBindGroup = [](uint32_t index, wgpu::BindGroup bg,
                                 std::vector<uint32_t> offsets = {}) {
        return [index, bg, offsets](CommandIterator* commands) {
            auto* cmd = commands->NextCommand<SetBindGroupCmd>();
            ityp::span<BindingIndex, const uint32_t> cmdOffsets;
            if (cmd->dynamicOffsetCount > BindingIndex{0u}) {
                cmdOffsets = commands->NextData<uint32_t>(cmd->dynamicOffsetCount);
            }

            ASSERT_EQ(cmd->index, BindGroupIndex(index));
            ASSERT_EQ(ToAPI(cmd->group.Get()), bg.Get());
            ASSERT_EQ(uint32_t{cmdOffsets.size()}, offsets.size());
            for (auto [i, cmdOffset] : Enumerate(cmdOffsets)) {
                ASSERT_EQ(cmdOffset, offsets[uint32_t{i}]);
            }
        };
    };

    // Initialize as null. Once we know the pointer, we'll check
    // that it's the same buffer every time.
    WGPUBuffer indirectScratchBuffer = nullptr;
    auto ExpectDispatchIndirect = [&](CommandIterator* commands) {
        auto* cmd = commands->NextCommand<DispatchIndirectCmd>();
        if (indirectScratchBuffer == nullptr) {
            indirectScratchBuffer = ToAPI(cmd->indirectBuffer.Get());
        }
        ASSERT_EQ(ToAPI(cmd->indirectBuffer.Get()), indirectScratchBuffer);
        ASSERT_EQ(cmd->indirectOffset, uint64_t{0});
    };

    // Initialize as null. Once we know the pointer, we'll check
    // that it's the same pipeline every time.
    WGPUComputePipeline validationPipeline = nullptr;
    auto ExpectSetValidationPipeline = [&](CommandIterator* commands) {
        auto* cmd = commands->NextCommand<SetComputePipelineCmd>();
        WGPUComputePipeline pipeline = ToAPI(cmd->pipeline.Get());
        if (validationPipeline != nullptr) {
            EXPECT_EQ(pipeline, validationPipeline);
        } else {
            EXPECT_NE(pipeline, nullptr);
            validationPipeline = pipeline;
        }
    };

    auto ExpectSetValidationImmediates = [&](CommandIterator* commands) {
        auto* cmd = commands->NextCommand<SetImmediatesCmd>();
        commands->NextData<uint8_t>(cmd->size);
        ASSERT_EQ(cmd->offset, 0u);
        ASSERT_EQ(cmd->size, 6 * sizeof(uint32_t));
    };

    auto ExpectSetValidationBindGroup = [&](CommandIterator* commands) {
        auto* cmd = commands->NextCommand<SetBindGroupCmd>();
        ASSERT_EQ(cmd->index, BindGroupIndex(0u));
        ASSERT_NE(cmd->group.Get(), nullptr);
        ASSERT_EQ(cmd->dynamicOffsetCount, BindingIndex{0u});
    };

    auto ExpectSetValidationDispatch = [&](CommandIterator* commands) {
        auto* cmd = commands->NextCommand<DispatchCmd>();
        ASSERT_EQ(cmd->x, 1u);
        ASSERT_EQ(cmd->y, 1u);
        ASSERT_EQ(cmd->z, 1u);
    };

    ExpectCommands(
        FromAPI(commandBuffer.Get())->GetCommandIteratorForTesting(),
        {
            {Command::BeginComputePass,
             [&](CommandIterator* commands) { SkipCommand(commands, Command::BeginComputePass); }},
            // Expect the state to be set.
            {Command::SetComputePipeline, ExpectSetPipeline(pipeline0)},
            {Command::SetBindGroup, ExpectSetBindGroup(0, staticBG)},
            {Command::SetBindGroup, ExpectSetBindGroup(1, dynamicBG, {dynamicOffset})},

            // Expect the validation.
            {Command::SetComputePipeline, ExpectSetValidationPipeline},
            {Command::SetImmediates, ExpectSetValidationImmediates},
            {Command::SetBindGroup, ExpectSetValidationBindGroup},
            {Command::Dispatch, ExpectSetValidationDispatch},

            // Expect the state to be restored.
            {Command::SetComputePipeline, ExpectSetPipeline(pipeline0)},
            {Command::SetBindGroup, ExpectSetBindGroup(0, staticBG)},
            {Command::SetBindGroup, ExpectSetBindGroup(1, dynamicBG, {dynamicOffset})},

            // Expect the dispatchIndirect.
            {Command::DispatchIndirect, ExpectDispatchIndirect},

            // Expect the validation.
            {Command::SetComputePipeline, ExpectSetValidationPipeline},
            {Command::SetImmediates, ExpectSetValidationImmediates},
            {Command::SetBindGroup, ExpectSetValidationBindGroup},
            {Command::Dispatch, ExpectSetValidationDispatch},

            // Expect the state to be restored.
            {Command::SetComputePipeline, ExpectSetPipeline(pipeline0)},
            {Command::SetBindGroup, ExpectSetBindGroup(0, staticBG)},
            {Command::SetBindGroup, ExpectSetBindGroup(1, dynamicBG, {dynamicOffset})},

            // Expect the dispatchIndirect.
            {Command::DispatchIndirect, ExpectDispatchIndirect},

            // Expect the state to be set (new pipeline).
            {Command::SetComputePipeline, ExpectSetPipeline(pipeline1)},
            {Command::SetBindGroup, ExpectSetBindGroup(0, dynamicBG, {dynamicOffset})},
            {Command::SetBindGroup, ExpectSetBindGroup(1, staticBG)},

            // Expect the validation.
            {Command::SetComputePipeline, ExpectSetValidationPipeline},
            {Command::SetImmediates, ExpectSetValidationImmediates},
            {Command::SetBindGroup, ExpectSetValidationBindGroup},
            {Command::Dispatch, ExpectSetValidationDispatch},

            // Expect the state to be restored.
            {Command::SetComputePipeline, ExpectSetPipeline(pipeline1)},
            {Command::SetBindGroup, ExpectSetBindGroup(0, dynamicBG, {dynamicOffset})},
            {Command::SetBindGroup, ExpectSetBindGroup(1, staticBG)},

            // Expect the dispatchIndirect.
            {Command::DispatchIndirect, ExpectDispatchIndirect},

            {Command::EndComputePass,
             [&](CommandIterator* commands) { commands->NextCommand<EndComputePassCmd>(); }},
        });
}

// Test that after restoring state, it is fully applied to the state tracker
// and does not leak state changes that occurred between a snapshot and the
// state restoration.
TEST_F(CommandBufferEncodingTests, StateNotLeakedAfterRestore) {
    wgpu::CommandEncoder encoder = device.CreateCommandEncoder();
    wgpu::ComputePassEncoder pass = encoder.BeginComputePass();

    CommandBufferStateTracker* stateTracker =
        FromAPI(pass.Get())->GetCommandBufferStateTrackerForTesting();

    // Snapshot the state.
    CommandBufferStateTracker snapshot = *stateTracker;
    // Expect no pipeline in the snapshot
    EXPECT_FALSE(snapshot.HasPipeline());

    // Create a simple pipeline
    wgpu::ComputePipelineDescriptor csDesc;
    csDesc.compute.module = utils::CreateShaderModule(device, R"(
        @compute @workgroup_size(1, 1, 1)
        fn main() {
        })");
    wgpu::ComputePipeline pipeline = device.CreateComputePipeline(&csDesc);

    // Set the pipeline.
    pass.SetPipeline(pipeline);

    // Expect the pipeline to be set.
    EXPECT_EQ(ToAPI(stateTracker->GetComputePipeline()), pipeline.Get());

    // Restore the state.
    FromAPI(pass.Get())->RestoreCommandBufferStateForTesting(std::move(snapshot));

    // Expect no pipeline
    EXPECT_FALSE(stateTracker->HasPipeline());
}

// Regression test for crbug.com/405316877.
// Make sure a zero size no-op copy would not cause dereferencing to a dangling pointer,
// if the buffer is destroyed immediately after the encoding.
TEST_F(CommandBufferEncodingTests, DanglingDestroyedBufferPointerB2T) {
    wgpu::CommandEncoder encoder = device.CreateCommandEncoder();
    {
        wgpu::Buffer buffer;
        {
            wgpu::BufferDescriptor descriptor = {};
            descriptor.size = 1024;
            descriptor.usage = wgpu::BufferUsage::CopySrc | wgpu::BufferUsage::CopyDst;
            buffer = device.CreateBuffer(&descriptor);
        }

        constexpr wgpu::Extent3D kTextureSize = {4, 4, 1};
        wgpu::Texture texture;
        {
            wgpu::TextureDescriptor descriptor;
            descriptor.size = kTextureSize;
            descriptor.format = wgpu::TextureFormat::RGBA8Unorm;
            descriptor.usage = wgpu::TextureUsage::CopyDst | wgpu::TextureUsage::CopySrc;
            texture = device.CreateTexture(&descriptor);
        }

        wgpu::TexelCopyBufferInfo texelCopyBufferInfo = utils::CreateTexelCopyBufferInfo(
            buffer, 0, kTextureBytesPerRowAlignment, kTextureSize.height);
        wgpu::TexelCopyTextureInfo texelCopyTextureInfo =
            utils::CreateTexelCopyTextureInfo(texture, 0, {0, 0, 0});
        constexpr wgpu::Extent3D kCopySize = {0, 0, 0};
        encoder.CopyBufferToTexture(&texelCopyBufferInfo, &texelCopyTextureInfo, &kCopySize);

        // Destroy and release the buffer immediately.
        buffer.Destroy();
    }

    wgpu::CommandBuffer commands = encoder.Finish();

    device.PushErrorScope(wgpu::ErrorFilter::Validation);
    device.GetQueue().Submit(1, &commands);
    device.PopErrorScope(wgpu::CallbackMode::AllowProcessEvents,
                         [](wgpu::PopErrorScopeStatus, wgpu::ErrorType, wgpu::StringView) {});
}

// Regression test for crbug.com/405316877.
// Make sure a zero size no-op copy would not cause dereferencing to a dangling pointer,
// if the buffer is destroyed immediately after the encoding.
TEST_F(CommandBufferEncodingTests, DanglingDestroyedBufferPointerT2B) {
    wgpu::CommandEncoder encoder = device.CreateCommandEncoder();
    {
        wgpu::Buffer buffer;
        {
            wgpu::BufferDescriptor descriptor = {};
            descriptor.size = 1024;
            descriptor.usage = wgpu::BufferUsage::CopySrc | wgpu::BufferUsage::CopyDst;
            buffer = device.CreateBuffer(&descriptor);
        }

        constexpr wgpu::Extent3D kTextureSize = {4, 4, 1};
        wgpu::Texture texture;
        {
            wgpu::TextureDescriptor descriptor;
            descriptor.size = kTextureSize;
            descriptor.format = wgpu::TextureFormat::RGBA8Unorm;
            descriptor.usage = wgpu::TextureUsage::CopyDst | wgpu::TextureUsage::CopySrc;
            texture = device.CreateTexture(&descriptor);
        }

        wgpu::TexelCopyBufferInfo texelCopyBufferInfo = utils::CreateTexelCopyBufferInfo(
            buffer, 0, kTextureBytesPerRowAlignment, kTextureSize.height);
        wgpu::TexelCopyTextureInfo texelCopyTextureInfo =
            utils::CreateTexelCopyTextureInfo(texture, 0, {0, 0, 0});
        constexpr wgpu::Extent3D kCopySize = {0, 0, 0};
        encoder.CopyTextureToBuffer(&texelCopyTextureInfo, &texelCopyBufferInfo, &kCopySize);

        // Destroy and release the buffer immediately.
        buffer.Destroy();
    }

    wgpu::CommandBuffer commands = encoder.Finish();

    device.PushErrorScope(wgpu::ErrorFilter::Validation);
    device.GetQueue().Submit(1, &commands);
    device.PopErrorScope(wgpu::CallbackMode::AllowProcessEvents,
                         [](wgpu::PopErrorScopeStatus, wgpu::ErrorType, wgpu::StringView) {});
}

}  // anonymous namespace
}  // namespace dawn::native
