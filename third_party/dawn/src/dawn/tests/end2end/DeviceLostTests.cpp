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

#include <cstring>
#include <memory>
#include <string>

#include "dawn/native/DawnNative.h"
#include "dawn/tests/DawnTest.h"
#include "dawn/tests/MockCallback.h"
#include "dawn/utils/ComboRenderPipelineDescriptor.h"
#include "dawn/utils/WGPUHelpers.h"
#include "gmock/gmock.h"

namespace dawn {
namespace {

using testing::_;
using testing::Exactly;
using testing::HasSubstr;
using testing::MockCppCallback;

using MockMapAsyncCallback = MockCppCallback<wgpu::BufferMapCallback<void>*>;
using MockQueueWorkDoneCallback = MockCppCallback<wgpu::QueueWorkDoneCallback<void>*>;

class DeviceLostTest : public DawnTest {
  protected:
    wgpu::Limits GetRequiredLimits(const wgpu::Limits& supported) override {
        // TODO(crbug.com/383593270): Enable all the limits.
        wgpu::Limits required = {};
        required.maxStorageBuffersInFragmentStage = supported.maxStorageBuffersInFragmentStage;
        required.maxStorageBuffersPerShaderStage = supported.maxStorageBuffersPerShaderStage;
        return required;
    }

    void SetUp() override { DawnTest::SetUp(); }

    void TearDown() override {
        WaitABit();
        DawnTest::TearDown();
    }

    template <typename T>
    void ExpectObjectIsError(const T& object) {
        EXPECT_TRUE(dawn::native::CheckIsErrorForTesting(object.Get()));
    }

    MockQueueWorkDoneCallback mWorkDoneCb;
    MockMapAsyncCallback mMapAsyncCb;
};

// Test that DeviceLostCallback is invoked when LostForTesting is called
TEST_P(DeviceLostTest, DeviceLostCallbackIsCalled) {
    LoseDeviceForTesting();
}

// Test that submit fails after the device is lost
TEST_P(DeviceLostTest, SubmitAfterDeviceLost) {
    wgpu::CommandBuffer commands;
    wgpu::CommandEncoder encoder = device.CreateCommandEncoder();
    commands = encoder.Finish();

    LoseDeviceForTesting();
    queue.Submit(0, &commands);
}

// Test that CreateBindGroupLayout fails when device is lost
TEST_P(DeviceLostTest, CreateBindGroupLayoutFails) {
    LoseDeviceForTesting();

    wgpu::BindGroupLayoutEntry entry;
    entry.binding = 0;
    entry.visibility = wgpu::ShaderStage::None;
    entry.buffer.type = wgpu::BufferBindingType::Uniform;
    wgpu::BindGroupLayoutDescriptor descriptor;
    descriptor.entryCount = 1;
    descriptor.entries = &entry;
    ExpectObjectIsError(device.CreateBindGroupLayout(&descriptor));
}

// Test that GetBindGroupLayout fails when device is lost
TEST_P(DeviceLostTest, GetBindGroupLayoutFails) {
    wgpu::ShaderModule csModule = utils::CreateShaderModule(device, R"(
        struct UniformBuffer {
            pos : vec4f
        }
        @group(0) @binding(0) var<uniform> ubo : UniformBuffer;
        @compute @workgroup_size(1) fn main() {
        })");

    wgpu::ComputePipelineDescriptor descriptor;
    descriptor.layout = nullptr;
    descriptor.compute.module = csModule;

    wgpu::ComputePipeline pipeline = device.CreateComputePipeline(&descriptor);

    LoseDeviceForTesting();
    ExpectObjectIsError(pipeline.GetBindGroupLayout(0));
}

// Test that CreateBindGroup fails when device is lost
TEST_P(DeviceLostTest, CreateBindGroupFails) {
    wgpu::BindGroupLayout layout;
    {
        wgpu::BindGroupLayoutEntry entry;
        entry.binding = 0;
        entry.visibility = wgpu::ShaderStage::None;
        entry.buffer.type = wgpu::BufferBindingType::Uniform;
        wgpu::BindGroupLayoutDescriptor descriptor;
        descriptor.entryCount = 1;
        descriptor.entries = &entry;
        layout = device.CreateBindGroupLayout(&descriptor);
    }

    WaitABit();
    LoseDeviceForTesting();

    {
        wgpu::BindGroupEntry entry;
        entry.binding = 0;
        entry.sampler = nullptr;
        entry.textureView = nullptr;
        entry.buffer = nullptr;
        entry.offset = 0;
        entry.size = 0;

        wgpu::BindGroupDescriptor descriptor;
        descriptor.layout = layout;
        descriptor.entryCount = 1;
        descriptor.entries = &entry;
        ExpectObjectIsError(device.CreateBindGroup(&descriptor));
    }
}

// Test that CreatePipelineLayout fails when device is lost
TEST_P(DeviceLostTest, CreatePipelineLayoutFails) {
    LoseDeviceForTesting();

    wgpu::PipelineLayoutDescriptor descriptor;
    descriptor.bindGroupLayoutCount = 0;
    descriptor.bindGroupLayouts = nullptr;
    ExpectObjectIsError(device.CreatePipelineLayout(&descriptor));
}

// Tests that CreateRenderBundleEncoder fails when device is lost
TEST_P(DeviceLostTest, CreateRenderBundleEncoderFails) {
    LoseDeviceForTesting();

    wgpu::RenderBundleEncoderDescriptor descriptor;
    descriptor.colorFormatCount = 0;
    descriptor.colorFormats = nullptr;
    ExpectObjectIsError(device.CreateRenderBundleEncoder(&descriptor));
}

// Tests that CreateComputePipeline fails when device is lost
TEST_P(DeviceLostTest, CreateComputePipelineFails) {
    wgpu::ShaderModule shader = utils::CreateShaderModule(device, "");

    WaitABit();
    LoseDeviceForTesting();

    wgpu::ComputePipelineDescriptor descriptor = {};
    descriptor.layout = nullptr;
    descriptor.compute.module = shader;
    ExpectObjectIsError(device.CreateComputePipeline(&descriptor));
}

// Tests that CreateRenderPipeline fails when device is lost
TEST_P(DeviceLostTest, CreateRenderPipelineFails) {
    wgpu::ShaderModule shader = utils::CreateShaderModule(device, "");

    WaitABit();
    LoseDeviceForTesting();

    utils::ComboRenderPipelineDescriptor descriptor = {};
    descriptor.vertex.module = shader;
    descriptor.fragment = nullptr;
    ExpectObjectIsError(device.CreateRenderPipeline(&descriptor));
}

// Tests that CreateSampler fails when device is lost
TEST_P(DeviceLostTest, CreateSamplerFails) {
    LoseDeviceForTesting();

    ExpectObjectIsError(device.CreateSampler());
}

// Tests that CreateShaderModule fails when device is lost
TEST_P(DeviceLostTest, CreateShaderModuleFails) {
    LoseDeviceForTesting();

    ExpectObjectIsError(utils::CreateShaderModule(device, R"(
        @fragment
        fn main(@location(0) color : vec4f) -> @location(0) vec4f {
            return color;
        })"));
}

// Note that no device lost tests are done for surface because it is awkward to create a
// wgpu::Surface in this file. SurfaceTests.GetAfterDeviceLoss covers this validation.

// Tests that CreateTexture fails when device is lost
TEST_P(DeviceLostTest, CreateTextureFails) {
    LoseDeviceForTesting();

    wgpu::TextureDescriptor descriptor;
    descriptor.size.width = 4;
    descriptor.size.height = 4;
    descriptor.size.depthOrArrayLayers = 1;
    descriptor.mipLevelCount = 1;
    descriptor.dimension = wgpu::TextureDimension::e2D;
    descriptor.usage = wgpu::TextureUsage::RenderAttachment;

    ExpectObjectIsError(device.CreateTexture(&descriptor));
}

// Test that CreateBuffer fails when device is lost
TEST_P(DeviceLostTest, CreateBufferFails) {
    LoseDeviceForTesting();

    wgpu::BufferDescriptor bufferDescriptor;
    bufferDescriptor.size = sizeof(float);
    bufferDescriptor.usage = wgpu::BufferUsage::CopySrc;
    ExpectObjectIsError(device.CreateBuffer(&bufferDescriptor));
}

// Test that buffer.MapAsync for writing fails after device is lost
TEST_P(DeviceLostTest, BufferMapAsyncFailsForWriting) {
    wgpu::BufferDescriptor bufferDescriptor;
    bufferDescriptor.size = 4;
    bufferDescriptor.usage = wgpu::BufferUsage::MapWrite;
    wgpu::Buffer buffer = device.CreateBuffer(&bufferDescriptor);

    LoseDeviceForTesting();

    EXPECT_CALL(mMapAsyncCb, Call(wgpu::MapAsyncStatus::Aborted,
                                  HasSubstr(UsesWire() ? "destroyed before mapping" : "is lost")))
        .Times(1);
    buffer.MapAsync(wgpu::MapMode::Write, 0, 4, wgpu::CallbackMode::AllowProcessEvents,
                    mMapAsyncCb.Callback());
}

// Test that BufferMapAsync for writing calls back with success when device lost after
// mapping
TEST_P(DeviceLostTest, BufferMapAsyncBeforeLossFailsForWriting) {
    wgpu::BufferDescriptor bufferDescriptor;
    bufferDescriptor.size = 4;
    bufferDescriptor.usage = wgpu::BufferUsage::MapWrite;
    wgpu::Buffer buffer = device.CreateBuffer(&bufferDescriptor);

    EXPECT_CALL(mMapAsyncCb, Call(wgpu::MapAsyncStatus::Success, _)).Times(1);
    buffer.MapAsync(wgpu::MapMode::Write, 0, 4, wgpu::CallbackMode::AllowProcessEvents,
                    mMapAsyncCb.Callback());

    LoseDeviceForTesting();
    WaitForAllOperations();
}

// Test that buffer.Unmap after device is lost
TEST_P(DeviceLostTest, BufferUnmapAfterDeviceLost) {
    wgpu::BufferDescriptor bufferDescriptor;
    bufferDescriptor.size = sizeof(float);
    bufferDescriptor.usage = wgpu::BufferUsage::MapWrite;
    bufferDescriptor.mappedAtCreation = true;
    wgpu::Buffer buffer = device.CreateBuffer(&bufferDescriptor);

    LoseDeviceForTesting();
    buffer.Unmap();
}

// Test CreateBuffer behavior after device is lost or destroyed
TEST_P(DeviceLostTest, CreateBuffer) {
    // Fails on TSAN due to allowing too much memory. TSAN max is `0x10000000000` and the test
    // allocates `0x8000000000000000`
    DAWN_SUPPRESS_TEST_IF(IsTsan());

    uint64_t kStupidLarge = uint64_t(1) << uint64_t(63);
    LoseDeviceForTesting();

    // Each test either expects null or an ErrorBuffer.
    auto Test = [&](bool expectNull, wgpu::BufferDescriptor* desc) {
        wgpu::Buffer buffer = device.CreateBuffer(desc);
        if (expectNull) {
            EXPECT_EQ(nullptr, buffer.Get());
        } else {
            ExpectObjectIsError(buffer);
            // Even if it's an ErrorBuffer, mappedAtCreation means it can be mapped.
            if (desc->mappedAtCreation) {
                EXPECT_NE(nullptr, buffer.GetMappedRange());
            }
        }
    };

    auto Tests = [&]() {
        for (auto usage : {wgpu::BufferUsage::MapWrite, wgpu::BufferUsage::CopyDst}) {
            wgpu::BufferDescriptor bufferDescriptor;
            bufferDescriptor.usage = usage;

            // Not mapped. Error is that the device is lost.
            bufferDescriptor.size = 4;
            bufferDescriptor.mappedAtCreation = false;
            Test(false, &bufferDescriptor);

            // Not mapped. Error is that the device is lost AND the size is too big.
            bufferDescriptor.size = kStupidLarge;
            bufferDescriptor.mappedAtCreation = false;
            Test(false, &bufferDescriptor);

            // Mapped at creation. Error is that the device is lost.
            bufferDescriptor.size = 4;
            bufferDescriptor.mappedAtCreation = true;
            Test(false, &bufferDescriptor);

            // Mapped at creation. Error is that the device is lost AND the size is too big.
            bufferDescriptor.size = kStupidLarge;
            bufferDescriptor.mappedAtCreation = true;
            Test(true, &bufferDescriptor);
        }
    };

    Tests();
    device.Destroy();
    Tests();
}

// Test that BufferMapAsync for reading fails after device is lost
TEST_P(DeviceLostTest, BufferMapAsyncFailsForReading) {
    wgpu::BufferDescriptor bufferDescriptor;
    bufferDescriptor.size = 4;
    bufferDescriptor.usage = wgpu::BufferUsage::MapRead | wgpu::BufferUsage::CopyDst;

    wgpu::Buffer buffer = device.CreateBuffer(&bufferDescriptor);

    LoseDeviceForTesting();

    EXPECT_CALL(mMapAsyncCb, Call(wgpu::MapAsyncStatus::Aborted,
                                  HasSubstr(UsesWire() ? "destroyed before mapping" : "is lost")))
        .Times(1);
    buffer.MapAsync(wgpu::MapMode::Read, 0, 4, wgpu::CallbackMode::AllowProcessEvents,
                    mMapAsyncCb.Callback());
}

// Test that BufferMapAsync for reading calls back with success when device lost after
// mapping
TEST_P(DeviceLostTest, BufferMapAsyncBeforeLossFailsForReading) {
    wgpu::BufferDescriptor bufferDescriptor;
    bufferDescriptor.size = sizeof(float);
    bufferDescriptor.usage = wgpu::BufferUsage::MapRead | wgpu::BufferUsage::CopyDst;

    wgpu::Buffer buffer = device.CreateBuffer(&bufferDescriptor);

    EXPECT_CALL(mMapAsyncCb, Call(wgpu::MapAsyncStatus::Success, _)).Times(1);
    buffer.MapAsync(wgpu::MapMode::Read, 0, 4, wgpu::CallbackMode::AllowProcessEvents,
                    mMapAsyncCb.Callback());

    LoseDeviceForTesting();
    WaitForAllOperations();
}

// Test that WriteBuffer after device is lost
TEST_P(DeviceLostTest, WriteBufferAfterDeviceLost) {
    wgpu::BufferDescriptor bufferDescriptor;
    bufferDescriptor.size = sizeof(float);
    bufferDescriptor.usage = wgpu::BufferUsage::MapRead | wgpu::BufferUsage::CopyDst;

    wgpu::Buffer buffer = device.CreateBuffer(&bufferDescriptor);

    LoseDeviceForTesting();
    float data = 12.0f;
    queue.WriteBuffer(buffer, 0, &data, sizeof(data));
}

// Test it's possible to GetMappedRange on a buffer created mapped after device loss
TEST_P(DeviceLostTest, GetMappedRange_CreateBufferMappedAtCreationAfterLoss) {
    LoseDeviceForTesting();

    wgpu::BufferDescriptor desc;
    desc.size = 4;
    desc.usage = wgpu::BufferUsage::CopySrc;
    desc.mappedAtCreation = true;
    wgpu::Buffer buffer = device.CreateBuffer(&desc);
    ExpectObjectIsError(buffer);

    ASSERT_NE(buffer.GetMappedRange(), nullptr);
}

// Test that device loss doesn't change the result of GetMappedRange, mappedAtCreation version.
TEST_P(DeviceLostTest, GetMappedRange_CreateBufferMappedAtCreationBeforeLoss) {
    wgpu::BufferDescriptor desc;
    desc.size = 4;
    desc.usage = wgpu::BufferUsage::CopySrc;
    desc.mappedAtCreation = true;
    wgpu::Buffer buffer = device.CreateBuffer(&desc);

    void* rangeBeforeLoss = buffer.GetMappedRange();
    LoseDeviceForTesting();

    ASSERT_NE(buffer.GetMappedRange(), nullptr);
    ASSERT_EQ(buffer.GetMappedRange(), rangeBeforeLoss);
}

// Test that device loss doesn't change the result of GetMappedRange, mapping for reading version.
TEST_P(DeviceLostTest, GetMappedRange_MapAsyncReading) {
    wgpu::BufferDescriptor desc;
    desc.size = 4;
    desc.usage = wgpu::BufferUsage::MapRead | wgpu::BufferUsage::CopyDst;
    wgpu::Buffer buffer = device.CreateBuffer(&desc);

    MapAsyncAndWait(buffer, wgpu::MapMode::Read, 0, 4);
    queue.Submit(0, nullptr);

    const void* rangeBeforeLoss = buffer.GetConstMappedRange();
    LoseDeviceForTesting();

    ASSERT_NE(buffer.GetConstMappedRange(), nullptr);
    ASSERT_EQ(buffer.GetConstMappedRange(), rangeBeforeLoss);
}

// Test that device loss doesn't change the result of GetMappedRange, mapping for writing version.
TEST_P(DeviceLostTest, GetMappedRange_MapAsyncWriting) {
    wgpu::BufferDescriptor desc;
    desc.size = 4;
    desc.usage = wgpu::BufferUsage::MapWrite | wgpu::BufferUsage::CopySrc;
    wgpu::Buffer buffer = device.CreateBuffer(&desc);

    MapAsyncAndWait(buffer, wgpu::MapMode::Write, 0, 4);
    queue.Submit(0, nullptr);

    const void* rangeBeforeLoss = buffer.GetConstMappedRange();
    LoseDeviceForTesting();

    ASSERT_NE(buffer.GetConstMappedRange(), nullptr);
    ASSERT_EQ(buffer.GetConstMappedRange(), rangeBeforeLoss);
}

// TODO(dawn:929): mapasync read + resolve + loss getmappedrange != nullptr.
// TODO(dawn:929): mapasync write + resolve + loss getmappedrange != nullptr.

// Test that Command Encoder Finish fails when device lost
TEST_P(DeviceLostTest, CommandEncoderFinishFails) {
    wgpu::CommandBuffer commands;
    wgpu::CommandEncoder encoder = device.CreateCommandEncoder();

    LoseDeviceForTesting();
    ExpectObjectIsError(encoder.Finish());
}

// Test QueueOnSubmittedWorkDone after device is lost.
TEST_P(DeviceLostTest, QueueOnSubmittedWorkDoneAfterDeviceLost) {
    LoseDeviceForTesting();

    // Callback should have success status
    EXPECT_CALL(mWorkDoneCb, Call(wgpu::QueueWorkDoneStatus::Success));
    queue.OnSubmittedWorkDone(wgpu::CallbackMode::AllowProcessEvents, mWorkDoneCb.Callback());
    WaitABit();
}

// Test QueueOnSubmittedWorkDone when the device is lost after calling OnSubmittedWorkDone
TEST_P(DeviceLostTest, QueueOnSubmittedWorkDoneBeforeLossFails) {
    // Callback should have success status
    EXPECT_CALL(mWorkDoneCb, Call(wgpu::QueueWorkDoneStatus::Success));
    queue.OnSubmittedWorkDone(wgpu::CallbackMode::AllowProcessEvents, mWorkDoneCb.Callback());

    LoseDeviceForTesting();
    WaitABit();
}

// Test that LostForTesting can only be called on one time
TEST_P(DeviceLostTest, LoseDeviceForTestingOnce) {
    // First LoseDeviceForTesting call should occur normally. The callback is already set in SetUp.
    LoseDeviceForTesting();

    device.ForceLoss(wgpu::DeviceLostReason::Unknown, "Device lost for testing");
    FlushWire();
    testing::Mock::VerifyAndClearExpectations(&mDeviceLostCallback);
}

TEST_P(DeviceLostTest, DeviceLostDoesntCallUncapturedError) {
    // Since the device has a default error callback set that fails if it is called, we just need
    // to lose the device and verify no failures.
    LoseDeviceForTesting();
}

// Test that WGPUCreatePipelineAsyncStatus_Success is returned when device is lost
// before the callback of Create*PipelineAsync() is called.
TEST_P(DeviceLostTest, DeviceLostBeforeCreatePipelineAsyncCallback) {
    wgpu::ShaderModule csModule = utils::CreateShaderModule(device, R"(
        @compute @workgroup_size(1) fn main() {
        })");

    wgpu::ComputePipelineDescriptor descriptor;
    descriptor.compute.module = csModule;

    device.CreateComputePipelineAsync(&descriptor, wgpu::CallbackMode::AllowProcessEvents,
                                      [](wgpu::CreatePipelineAsyncStatus status,
                                         wgpu::ComputePipeline pipeline, wgpu::StringView) {
                                          EXPECT_EQ(wgpu::CreatePipelineAsyncStatus::Success,
                                                    status);
                                          EXPECT_NE(pipeline, nullptr);
                                      });

    LoseDeviceForTesting();
}

// This is a regression test for crbug.com/1212385 where Dawn didn't clean up all
// references to bind group layouts such that the cache was non-empty at the end
// of shut down.
TEST_P(DeviceLostTest, FreeBindGroupAfterDeviceLossWithPendingCommands) {
    DAWN_SUPPRESS_TEST_IF(GetSupportedLimits().maxStorageBuffersInFragmentStage < 1);

    wgpu::BindGroupLayout bgl = utils::MakeBindGroupLayout(
        device, {{0, wgpu::ShaderStage::Fragment, wgpu::BufferBindingType::Storage}});

    wgpu::BufferDescriptor bufferDesc;
    bufferDesc.size = sizeof(float);
    bufferDesc.usage = wgpu::BufferUsage::Storage;
    wgpu::Buffer buffer = device.CreateBuffer(&bufferDesc);

    wgpu::BindGroup bg = utils::MakeBindGroup(device, bgl, {{0, buffer, 0, sizeof(float)}});

    // Advance the pending command serial. We only a need a couple of these to repro the bug,
    // but include extra so this does not become a change-detecting test if the specific serial
    // value is sensitive.
    queue.Submit(0, nullptr);
    queue.Submit(0, nullptr);
    queue.Submit(0, nullptr);
    queue.Submit(0, nullptr);
    queue.Submit(0, nullptr);
    queue.Submit(0, nullptr);

    LoseDeviceForTesting();

    // Releasing the bing group places the bind group layout into a queue in the Vulkan backend
    // for recycling of descriptor sets. So, after these release calls there is still one last
    // reference to the BGL which wouldn't be freed until the pending serial passes.
    // Since the device is lost, destruction will clean up immediately without waiting for the
    // serial. The implementation needs to be sure to clear these BGL references. At the end of
    // Device shut down, we DAWN_ASSERT that the BGL cache is empty.
    bgl = nullptr;
    bg = nullptr;
}

// This is a regression test for crbug.com/1365011 where ending a render pass with an indirect draw
// in it after the device is lost would cause render commands to be leaked.
TEST_P(DeviceLostTest, DeviceLostInRenderPassWithDrawIndirect) {
    utils::BasicRenderPass renderPass = utils::CreateBasicRenderPass(device, 4u, 4u);
    utils::ComboRenderPipelineDescriptor desc;
    desc.vertex.module = utils::CreateShaderModule(device, R"(
        @vertex fn main(@builtin(vertex_index) i : u32) -> @builtin(position) vec4f {
            var pos = array(
                vec2f(-1.0, -1.0),
                vec2f(3.0, -1.0),
                vec2f(-1.0, 3.0));
            return vec4f(pos[i], 0.0, 1.0);
        }
    )");
    desc.cFragment.module = utils::CreateShaderModule(device, R"(
        @fragment fn main() -> @location(0) vec4f {
            return vec4f(0.0, 1.0, 0.0, 1.0);
        }
    )");
    desc.cTargets[0].format = renderPass.colorFormat;
    wgpu::Buffer indirectBuffer =
        utils::CreateBufferFromData<uint32_t>(device, wgpu::BufferUsage::Indirect, {3, 1, 0, 0});
    wgpu::RenderPipeline pipeline = device.CreateRenderPipeline(&desc);
    wgpu::CommandEncoder encoder = device.CreateCommandEncoder();
    wgpu::RenderPassEncoder pass = encoder.BeginRenderPass(&renderPass.renderPassInfo);
    pass.SetPipeline(pipeline);
    pass.DrawIndirect(indirectBuffer, 0);
    LoseDeviceForTesting();
    pass.End();
}

// Attempting to set an object label after device loss should not cause an error.
TEST_P(DeviceLostTest, SetLabelAfterDeviceLoss) {
    std::string label = "test";
    wgpu::BufferDescriptor descriptor;
    descriptor.size = 4;
    descriptor.usage = wgpu::BufferUsage::Uniform;
    wgpu::Buffer buffer = device.CreateBuffer(&descriptor);
    LoseDeviceForTesting();
    buffer.SetLabel(label.c_str());
}

DAWN_INSTANTIATE_TEST(DeviceLostTest,
                      D3D11Backend(),
                      D3D12Backend(),
                      MetalBackend(),
                      NullBackend(),
                      OpenGLBackend(),
                      OpenGLESBackend(),
                      VulkanBackend());

}  // anonymous namespace
}  // namespace dawn
