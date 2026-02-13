// Copyright 2022 The Dawn & Tint Authors
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

#include "dawn/tests/DawnTest.h"
#include "dawn/tests/MockCallback.h"
#include "dawn/utils/WGPUHelpers.h"

namespace dawn {
namespace {

using testing::_;
using testing::HasSubstr;
using testing::Invoke;
using testing::MockCppCallback;
using testing::Return;

using MockMapAsyncCallback = MockCppCallback<void (*)(wgpu::MapAsyncStatus, wgpu::StringView)>;

class DeviceLifetimeTests : public DawnTest {};

// Test that the device can be dropped before its queue.
TEST_P(DeviceLifetimeTests, DroppedBeforeQueue) {
    wgpu::Queue queue = device.GetQueue();

    device = nullptr;
}

// Test that the device can be dropped while an onSubmittedWorkDone callback is in flight.
TEST_P(DeviceLifetimeTests, DroppedWhileQueueOnSubmittedWorkDone) {
    // Submit some work.
    wgpu::CommandEncoder encoder = device.CreateCommandEncoder(nullptr);
    wgpu::CommandBuffer commandBuffer = encoder.Finish();
    queue.Submit(1, &commandBuffer);

    // Ask for an onSubmittedWorkDone callback and drop the device.
    queue.OnSubmittedWorkDone(wgpu::CallbackMode::AllowProcessEvents,
                              [](wgpu::QueueWorkDoneStatus status, wgpu::StringView) {
                                  EXPECT_EQ(status, wgpu::QueueWorkDoneStatus::Success);
                              });

    device = nullptr;
}

// Test that the device can be dropped inside an onSubmittedWorkDone callback.
TEST_P(DeviceLifetimeTests, DroppedInsideQueueOnSubmittedWorkDone) {
    // Submit some work.
    wgpu::CommandEncoder encoder = device.CreateCommandEncoder(nullptr);
    wgpu::CommandBuffer commandBuffer = encoder.Finish();
    queue.Submit(1, &commandBuffer);

    // Ask for an onSubmittedWorkDone callback and drop the device inside the callback.
    queue.OnSubmittedWorkDone(wgpu::CallbackMode::AllowProcessEvents,
                              [this](wgpu::QueueWorkDoneStatus status, wgpu::StringView) {
                                  EXPECT_EQ(status, wgpu::QueueWorkDoneStatus::Success);
                                  this->device = nullptr;
                              });

    WaitForAllOperations();
}

// Test that the device can be dropped while a popErrorScope callback is in flight.
TEST_P(DeviceLifetimeTests, DroppedWhilePopErrorScope) {
    device.PushErrorScope(wgpu::ErrorFilter::Validation);
    bool done = false;

    device.PopErrorScope(
        wgpu::CallbackMode::AllowProcessEvents,
        [](wgpu::PopErrorScopeStatus status, wgpu::ErrorType type, wgpu::StringView, bool* done) {
            *done = true;
            EXPECT_EQ(status, wgpu::PopErrorScopeStatus::Success);
            EXPECT_EQ(type, wgpu::ErrorType::NoError);
        },
        &done);
    device = nullptr;

    while (!done) {
        WaitABit();
    }
}

// Test that the device can be dropped inside an popErrorScope callback.
TEST_P(DeviceLifetimeTests, DroppedInsidePopErrorScope) {
    struct Userdata {
        wgpu::Device device;
        bool done;
    };
    device.PushErrorScope(wgpu::ErrorFilter::Validation);

    // Ask for a popErrorScope callback and drop the device inside the callback.
    Userdata data = Userdata{std::move(device), false};
    data.device.PopErrorScope(
        wgpu::CallbackMode::AllowProcessEvents,
        [](wgpu::PopErrorScopeStatus status, wgpu::ErrorType type, wgpu::StringView,
           Userdata* userdata) {
            EXPECT_EQ(status, wgpu::PopErrorScopeStatus::Success);
            EXPECT_EQ(type, wgpu::ErrorType::NoError);
            userdata->device = nullptr;
            userdata->done = true;
        },
        &data);

    while (!data.done) {
        WaitABit();
    }
}

// Test that the device can be dropped before a buffer created from it.
TEST_P(DeviceLifetimeTests, DroppedBeforeBuffer) {
    wgpu::BufferDescriptor desc = {};
    desc.size = 4;
    desc.usage = wgpu::BufferUsage::CopySrc | wgpu::BufferUsage::CopyDst;
    wgpu::Buffer buffer = device.CreateBuffer(&desc);

    device = nullptr;
}

// Test that the device can be dropped while a buffer created from it is being mapped.
TEST_P(DeviceLifetimeTests, DroppedWhileMappingBuffer) {
    wgpu::BufferDescriptor desc = {};
    desc.size = 4;
    desc.usage = wgpu::BufferUsage::MapRead | wgpu::BufferUsage::CopyDst;
    wgpu::Buffer buffer = device.CreateBuffer(&desc);

    MockMapAsyncCallback cb;
    EXPECT_CALL(cb, Call(wgpu::MapAsyncStatus::Aborted, _)).Times(1);

    buffer.MapAsync(wgpu::MapMode::Read, 0, wgpu::kWholeMapSize,
                    wgpu::CallbackMode::AllowProcessEvents, cb.Callback());

    device = nullptr;
    WaitForAllOperations();
}

// Test that the device can be dropped before a mapped buffer created from it.
TEST_P(DeviceLifetimeTests, DroppedBeforeMappedBuffer) {
    wgpu::BufferDescriptor desc = {};
    desc.size = 4;
    desc.usage = wgpu::BufferUsage::MapRead | wgpu::BufferUsage::CopyDst;
    wgpu::Buffer buffer = device.CreateBuffer(&desc);

    MapAsyncAndWait(buffer, wgpu::MapMode::Read, 0, wgpu::kWholeMapSize);

    device = nullptr;
}

// Test that the device can be dropped before a mapped at creation buffer created from it.
TEST_P(DeviceLifetimeTests, DroppedBeforeMappedAtCreationBuffer) {
    wgpu::BufferDescriptor desc = {};
    desc.size = 4;
    desc.usage = wgpu::BufferUsage::MapRead | wgpu::BufferUsage::CopyDst;
    desc.mappedAtCreation = true;
    wgpu::Buffer buffer = device.CreateBuffer(&desc);

    device = nullptr;
}

// Test that the device can be dropped before a buffer created from it, then mapping the buffer
// fails.
TEST_P(DeviceLifetimeTests, DroppedThenMapBuffer) {
    wgpu::BufferDescriptor desc = {};
    desc.size = 4;
    desc.usage = wgpu::BufferUsage::MapRead | wgpu::BufferUsage::CopyDst;
    wgpu::Buffer buffer = device.CreateBuffer(&desc);

    device = nullptr;

    MockMapAsyncCallback cb;
    EXPECT_CALL(cb, Call(wgpu::MapAsyncStatus::Aborted, HasSubstr("lost"))).Times(1);

    buffer.MapAsync(wgpu::MapMode::Read, 0, wgpu::kWholeMapSize,
                    wgpu::CallbackMode::AllowProcessEvents, cb.Callback());
    WaitForAllOperations();
}

// Test that the device can be dropped before a buffer created from it, then mapping the buffer
// twice (one inside callback) will both fail.
TEST_P(DeviceLifetimeTests, Dropped_ThenMapBuffer_ThenMapBufferInCallback) {
    wgpu::BufferDescriptor desc = {};
    desc.size = 4;
    desc.usage = wgpu::BufferUsage::MapRead | wgpu::BufferUsage::CopyDst;
    wgpu::Buffer buffer = device.CreateBuffer(&desc);

    device = nullptr;

    // First mapping.
    buffer.MapAsync(wgpu::MapMode::Read, 0, wgpu::kWholeMapSize,
                    wgpu::CallbackMode::AllowProcessEvents,
                    [&buffer](wgpu::MapAsyncStatus status, wgpu::StringView message) {
                        EXPECT_EQ(status, wgpu::MapAsyncStatus::Aborted);
                        EXPECT_THAT(message, HasSubstr("lost"));

                        // Second mapping
                        buffer.MapAsync(wgpu::MapMode::Read, 0, wgpu::kWholeMapSize,
                                        wgpu::CallbackMode::AllowProcessEvents,
                                        [](wgpu::MapAsyncStatus status, wgpu::StringView message) {
                                            EXPECT_EQ(status, wgpu::MapAsyncStatus::Aborted);
                                            EXPECT_THAT(message, HasSubstr("lost"));
                                        });
                    });

    WaitForAllOperations();
}

// Test that the device can be dropped inside a buffer map callback.
TEST_P(DeviceLifetimeTests, DroppedInsideBufferMapCallback) {
    wgpu::BufferDescriptor desc = {};
    desc.size = 4;
    desc.usage = wgpu::BufferUsage::MapRead | wgpu::BufferUsage::CopyDst;
    wgpu::Buffer buffer = device.CreateBuffer(&desc);

    buffer.MapAsync(wgpu::MapMode::Read, 0, wgpu::kWholeMapSize,
                    wgpu::CallbackMode::AllowProcessEvents,
                    [this, buffer](wgpu::MapAsyncStatus status, wgpu::StringView) {
                        EXPECT_EQ(status, wgpu::MapAsyncStatus::Success);
                        device = nullptr;

                        // Mapped data should be null since the buffer is implicitly destroyed.
                        // TODO(crbug.com/dawn/1424): On the wire client, we don't track device
                        // child objects so the mapped data is still available when the device is
                        // destroyed.
                        if (!UsesWire()) {
                            EXPECT_EQ(buffer.GetConstMappedRange(), nullptr);
                        }
                    });

    WaitForAllOperations();

    // Mapped data should be null since the buffer is implicitly destroyed.
    // TODO(crbug.com/dawn/1424): On the wire client, we don't track device child objects so the
    // mapped data is still available when the device is destroyed.
    if (!UsesWire()) {
        EXPECT_EQ(buffer.GetConstMappedRange(), nullptr);
    }
}

// Test that the device can be dropped while a write buffer operation is enqueued.
TEST_P(DeviceLifetimeTests, DroppedWhileWriteBuffer) {
    wgpu::BufferDescriptor desc = {};
    desc.size = 4;
    desc.usage = wgpu::BufferUsage::CopySrc | wgpu::BufferUsage::CopyDst;
    wgpu::Buffer buffer = device.CreateBuffer(&desc);

    uint32_t value = 7;
    queue.WriteBuffer(buffer, 0, &value, sizeof(value));
    device = nullptr;
}

// Test that the device can be dropped while a write buffer operation is enqueued and then
// a queue submit occurs. This is slightly different from the former test since it ensures
// that pending work is flushed.
TEST_P(DeviceLifetimeTests, DroppedWhileWriteBufferAndSubmit) {
    wgpu::BufferDescriptor desc = {};
    desc.size = 4;
    desc.usage = wgpu::BufferUsage::CopySrc | wgpu::BufferUsage::CopyDst;
    wgpu::Buffer buffer = device.CreateBuffer(&desc);

    uint32_t value = 7;
    queue.WriteBuffer(buffer, 0, &value, sizeof(value));
    queue.Submit(0, nullptr);
    device = nullptr;
}

// Test that the device can be dropped while createPipelineAsync is in flight
TEST_P(DeviceLifetimeTests, DroppedWhileCreatePipelineAsync) {
    // TODO(crbug.com/413053623): implement webgpu::ShaderModule
    DAWN_SUPPRESS_TEST_IF(IsWebGPUOnWebGPU());

    wgpu::ComputePipelineDescriptor desc;
    desc.compute.module = utils::CreateShaderModule(device, R"(
    @compute @workgroup_size(1) fn main() {
    })");

    device.CreateComputePipelineAsync(
        &desc,
        UsesWire() ? wgpu::CallbackMode::AllowSpontaneous : wgpu::CallbackMode::AllowProcessEvents,
        [](wgpu::CreatePipelineAsyncStatus status, wgpu::ComputePipeline pipeline,
           wgpu::StringView) {
            EXPECT_EQ(wgpu::CreatePipelineAsyncStatus::Success, status);
            EXPECT_NE(pipeline, nullptr);
        });

    device = nullptr;
}

// Test that the device can be dropped inside a createPipelineAsync callback
TEST_P(DeviceLifetimeTests, DroppedInsideCreatePipelineAsync) {
    // TODO(crbug.com/413053623): implement webgpu::ShaderModule
    DAWN_SUPPRESS_TEST_IF(IsWebGPUOnWebGPU());

    wgpu::ComputePipelineDescriptor desc;
    desc.compute.module = utils::CreateShaderModule(device, R"(
    @compute @workgroup_size(1) fn main() {
    })");

    bool done = false;
    device.CreateComputePipelineAsync(&desc, wgpu::CallbackMode::AllowProcessEvents,
                                      [this, &done](wgpu::CreatePipelineAsyncStatus status,
                                                    wgpu::ComputePipeline, wgpu::StringView) {
                                          EXPECT_EQ(wgpu::CreatePipelineAsyncStatus::Success,
                                                    status);
                                          device = nullptr;
                                          done = true;
                                      });

    while (!done) {
        WaitABit();
    }
}

// Test that the device can be dropped while createPipelineAsync which will hit the frontend cache
// is in flight
TEST_P(DeviceLifetimeTests, DroppedWhileCreatePipelineAsyncAlreadyCached) {
    // TODO(crbug.com/413053623): implement webgpu::ShaderModule
    DAWN_SUPPRESS_TEST_IF(IsWebGPUOnWebGPU());

    wgpu::ComputePipelineDescriptor desc;
    desc.compute.module = utils::CreateShaderModule(device, R"(
    @compute @workgroup_size(1) fn main() {
    })");

    // Create a pipeline ahead of time so it's in the cache.
    wgpu::ComputePipeline p = device.CreateComputePipeline(&desc);

    bool done = false;
    device.CreateComputePipelineAsync(&desc, wgpu::CallbackMode::AllowProcessEvents,
                                      [&done](wgpu::CreatePipelineAsyncStatus status,
                                              wgpu::ComputePipeline pipeline, wgpu::StringView) {
                                          EXPECT_EQ(wgpu::CreatePipelineAsyncStatus::Success,
                                                    status);
                                          EXPECT_NE(pipeline, nullptr);
                                          done = true;
                                      });
    device = nullptr;

    while (!done) {
        WaitABit();
    }
}

// Test that the device can be dropped inside a createPipelineAsync callback which will hit the
// frontend cache
TEST_P(DeviceLifetimeTests, DroppedInsideCreatePipelineAsyncAlreadyCached) {
    // TODO(crbug.com/413053623): implement webgpu::ShaderModule
    DAWN_SUPPRESS_TEST_IF(IsWebGPUOnWebGPU());

    wgpu::ComputePipelineDescriptor desc;
    desc.compute.module = utils::CreateShaderModule(device, R"(
    @compute @workgroup_size(1) fn main() {
    })");

    // Create a pipeline ahead of time so it's in the cache.
    wgpu::ComputePipeline p = device.CreateComputePipeline(&desc);

    bool done = false;
    device.CreateComputePipelineAsync(
        &desc, wgpu::CallbackMode::AllowProcessEvents,
        [this, &done](wgpu::CreatePipelineAsyncStatus status, wgpu::ComputePipeline pipeline,
                      wgpu::StringView) {
            EXPECT_EQ(wgpu::CreatePipelineAsyncStatus::Success, status);
            EXPECT_NE(pipeline, nullptr);
            device = nullptr;
            done = true;
        });

    while (!done) {
        WaitABit();
    }
}

// Test that the device can be dropped while createPipelineAsync which will race with a compilation
// to add the same pipeline to the frontend cache
TEST_P(DeviceLifetimeTests, DroppedWhileCreatePipelineAsyncRaceCache) {
    // TODO(crbug.com/413053623): implement webgpu::ShaderModule
    DAWN_SUPPRESS_TEST_IF(IsWebGPUOnWebGPU());

    wgpu::ComputePipelineDescriptor desc;
    desc.compute.module = utils::CreateShaderModule(device, R"(
    @compute @workgroup_size(1) fn main() {
    })");

    device.CreateComputePipelineAsync(
        &desc,
        UsesWire() ? wgpu::CallbackMode::AllowSpontaneous : wgpu::CallbackMode::AllowProcessEvents,
        [](wgpu::CreatePipelineAsyncStatus status, wgpu::ComputePipeline pipeline,
           wgpu::StringView) {
            EXPECT_EQ(wgpu::CreatePipelineAsyncStatus::Success, status);
            EXPECT_NE(pipeline, nullptr);
        });

    // Create the same pipeline synchronously which will get added to the cache.
    wgpu::ComputePipeline p = device.CreateComputePipeline(&desc);

    device = nullptr;
}

// Test that the device can be dropped inside a createPipelineAsync callback which will race
// with a compilation to add the same pipeline to the frontend cache
TEST_P(DeviceLifetimeTests, DroppedInsideCreatePipelineAsyncRaceCache) {
    // TODO(crbug.com/413053623): implement webgpu::ShaderModule
    DAWN_SUPPRESS_TEST_IF(IsWebGPUOnWebGPU());

    wgpu::ComputePipelineDescriptor desc;
    desc.compute.module = utils::CreateShaderModule(device, R"(
    @compute @workgroup_size(1) fn main() {
    })");

    bool done = false;
    device.CreateComputePipelineAsync(
        &desc, wgpu::CallbackMode::AllowProcessEvents,
        [this, &done](wgpu::CreatePipelineAsyncStatus status, wgpu::ComputePipeline pipeline,
                      wgpu::StringView) {
            EXPECT_EQ(wgpu::CreatePipelineAsyncStatus::Success, status);
            EXPECT_NE(pipeline, nullptr);
            device = nullptr;
            done = true;
        });

    // Create the same pipeline synchronously which will get added to the cache.
    wgpu::ComputePipeline p = device.CreateComputePipeline(&desc);

    while (!done) {
        WaitABit();
    }
}

// Tests that dropping 2nd device inside 1st device's callback triggered by instance.ProcessEvents
// won't crash.
TEST_P(DeviceLifetimeTests, DropDevice2InProcessEvents) {
    wgpu::Device device2 = CreateDevice();

    struct UserData {
        wgpu::Device device2;
        bool done = false;
    } userdata;

    userdata.device2 = std::move(device2);

    device.PushErrorScope(wgpu::ErrorFilter::Validation);

    // The following callback will drop the 2nd device. It won't be triggered until
    // instance.ProcessEvents() is called.
    device.PopErrorScope(
        wgpu::CallbackMode::AllowProcessEvents,
        [](wgpu::PopErrorScopeStatus status, wgpu::ErrorType type, wgpu::StringView,
           UserData* userdata) {
            userdata->device2 = nullptr;
            userdata->done = true;
        },
        &userdata);

    while (!userdata.done) {
        WaitABit();
    }
}

DAWN_INSTANTIATE_TEST(DeviceLifetimeTests,
                      D3D11Backend(),
                      D3D12Backend(),
                      MetalBackend(),
                      NullBackend(),
                      OpenGLBackend(),
                      OpenGLESBackend(),
                      VulkanBackend(),
                      WebGPUBackend());

}  // anonymous namespace
}  // namespace dawn
