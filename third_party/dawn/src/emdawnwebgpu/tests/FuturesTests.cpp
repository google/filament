// Copyright 2024 The Dawn & Tint Authors
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

#include <dawn/webgpu_cpp_print.h>
#include <gmock/gmock.h>
#include <gtest/gtest.h>
#include <webgpu/webgpu_cpp.h>

#include <string>
#include <utility>

namespace {

using testing::_;
using testing::HasSubstr;

class InstanceLevelTests : public testing::Test {
  public:
    void SetUp() override { instance = wgpu::CreateInstance(); }

  protected:
    wgpu::Adapter RequestAdapter(const wgpu::RequestAdapterOptions* adapterOptions = nullptr) {
        wgpu::RequestAdapterStatus status;
        wgpu::Adapter result = nullptr;
        EXPECT_EQ(instance.WaitAny(
                      instance.RequestAdapter(
                          adapterOptions, wgpu::CallbackMode::AllowSpontaneous,
                          [&status, &result](wgpu::RequestAdapterStatus s, wgpu::Adapter adapter,
                                             wgpu::StringView message) {
                              status = s;
                              result = std::move(adapter);
                          }),
                      UINT64_MAX),
                  wgpu::WaitStatus::Success);
        EXPECT_EQ(status, wgpu::RequestAdapterStatus::Success);
        return result;
    }

    wgpu::Instance instance;
};

TEST_F(InstanceLevelTests, RequestAdapter) {
    EXPECT_NE(RequestAdapter(), nullptr);
}

class AdapterLevelTests : public InstanceLevelTests {
  public:
    void SetUp() override {
        InstanceLevelTests::SetUp();
        adapter = RequestAdapter();
    }

  protected:
    wgpu::Device RequestDevice(const wgpu::DeviceDescriptor* descriptor = nullptr) {
        wgpu::RequestDeviceStatus status;
        wgpu::Device result = nullptr;
        EXPECT_EQ(
            instance.WaitAny(adapter.RequestDevice(
                                 descriptor, wgpu::CallbackMode::AllowSpontaneous,
                                 [&status, &result](wgpu::RequestDeviceStatus s,
                                                    wgpu::Device device, wgpu::StringView message) {
                                     status = s;
                                     result = std::move(device);
                                 }),
                             UINT64_MAX),
            wgpu::WaitStatus::Success);
        EXPECT_EQ(status, wgpu::RequestDeviceStatus::Success);
        return result;
    }

    wgpu::Adapter adapter;
};

TEST_F(AdapterLevelTests, RequestDevice) {
    EXPECT_NE(RequestDevice(), nullptr);
}

TEST_F(AdapterLevelTests, RequestDeviceThenDestroy) {
    wgpu::Device device = nullptr;
    wgpu::DeviceLostReason reason;

    wgpu::DeviceDescriptor descriptor = {};
    descriptor.SetDeviceLostCallback(
        wgpu::CallbackMode::AllowSpontaneous,
        [&device, &reason](const wgpu::Device& d, wgpu::DeviceLostReason r, wgpu::StringView) {
            reason = r;
            EXPECT_EQ(device.Get(), d.Get());
        });
    device = RequestDevice(&descriptor);

    auto deviceLostFuture = device.GetLostFuture();
    device.Destroy();
    ASSERT_EQ(instance.WaitAny(deviceLostFuture, UINT64_MAX), wgpu::WaitStatus::Success);
    EXPECT_EQ(reason, wgpu::DeviceLostReason::Destroyed);
}

TEST_F(AdapterLevelTests, RequestDeviceThenDrop) {
    wgpu::DeviceLostReason reason;

    wgpu::DeviceDescriptor descriptor = {};
    descriptor.SetDeviceLostCallback(
        wgpu::CallbackMode::AllowSpontaneous,
        [&reason](const wgpu::Device&, wgpu::DeviceLostReason r, wgpu::StringView) { reason = r; });
    wgpu::Device device = RequestDevice(&descriptor);

    auto deviceLostFuture = device.GetLostFuture();
    device = nullptr;
    ASSERT_EQ(instance.WaitAny(deviceLostFuture, UINT64_MAX), wgpu::WaitStatus::Success);
    EXPECT_EQ(reason, wgpu::DeviceLostReason::Destroyed);
}

class DeviceLevelTests : public AdapterLevelTests {
  public:
    void SetUp() override {
        AdapterLevelTests::SetUp();

        wgpu::DeviceDescriptor descriptor = {};
        descriptor.SetDeviceLostCallback(
            wgpu::CallbackMode::AllowSpontaneous,
            [](const wgpu::Device&, wgpu::DeviceLostReason reason, wgpu::StringView) {
                EXPECT_EQ(reason, wgpu::DeviceLostReason::Destroyed);
            });
        descriptor.SetUncapturedErrorCallback(
            [](const wgpu::Device& d, wgpu::ErrorType t, wgpu::StringView m,
               DeviceLevelTests* self) { self->uncapturedErrorCb.Call(d, t, m); },
            this);
        device = RequestDevice(&descriptor);
    }

    void TearDown() override {
        // For teardown, we explicitly wait for the device lost so that we can ensure that errors
        // have been flushed.
        auto deviceLostFuture = device.GetLostFuture();
        device = nullptr;
        EXPECT_EQ(instance.WaitAny(deviceLostFuture, UINT64_MAX), wgpu::WaitStatus::Success);
    }

  protected:
    wgpu::ShaderModule CreateShaderModule(const char* source) {
        wgpu::ShaderSourceWGSL wgsl;
        wgsl.code = source;
        wgpu::ShaderModuleDescriptor desc;
        desc.nextInChain = &wgsl;
        return device.CreateShaderModule(&desc);
    }

    wgpu::Device device;

    // Mock callback used for uncaptured errors so that test writers can add expectations on this
    // callback which will enforce the expectations at teardown of the test.
    testing::StrictMock<
        testing::MockFunction<void(const wgpu::Device&, wgpu::ErrorType, wgpu::StringView)>>
        uncapturedErrorCb;
};

TEST_F(DeviceLevelTests, ValidationError) {
    EXPECT_CALL(uncapturedErrorCb, Call(_, wgpu::ErrorType::Validation, _)).Times(1);

    wgpu::BufferDescriptor desc = {};
    desc.size = 1024;
    desc.usage = static_cast<wgpu::BufferUsage>(UINT64_MAX);
    wgpu::Buffer buffer = device.CreateBuffer(&desc);
}

TEST_F(DeviceLevelTests, PopErrorScope) {
    device.PushErrorScope(wgpu::ErrorFilter::Validation);

    wgpu::BufferDescriptor desc = {};
    desc.size = 1024;
    desc.usage = static_cast<wgpu::BufferUsage>(UINT64_MAX);
    wgpu::Buffer buffer = device.CreateBuffer(&desc);

    wgpu::PopErrorScopeStatus status;
    wgpu::ErrorType type;
    EXPECT_EQ(instance.WaitAny(
                  device.PopErrorScope(wgpu::CallbackMode::AllowSpontaneous,
                                       [&status, &type](wgpu::PopErrorScopeStatus s,
                                                        wgpu::ErrorType t, wgpu::StringView) {
                                           status = s;
                                           type = t;
                                       }),
                  UINT64_MAX),
              wgpu::WaitStatus::Success);
    EXPECT_EQ(status, wgpu::PopErrorScopeStatus::Success);
    EXPECT_EQ(type, wgpu::ErrorType::Validation);
}

TEST_F(DeviceLevelTests, BufferMapAndWorkDone) {
    static constexpr uint32_t kData = 100u;
    size_t kSize = sizeof(uint32_t);

    wgpu::Buffer src;
    wgpu::Buffer dst;
    {
        wgpu::BufferDescriptor desc;
        desc.label = "src";
        desc.size = kSize;
        desc.usage = wgpu::BufferUsage::CopySrc | wgpu::BufferUsage::MapWrite;
        src = device.CreateBuffer(&desc);
    }
    {
        wgpu::BufferDescriptor desc;
        desc.label = "dst";
        desc.size = kSize;
        desc.usage = wgpu::BufferUsage::CopyDst | wgpu::BufferUsage::MapRead;
        dst = device.CreateBuffer(&desc);
    }

    // Map the writable buffer and write to it.
    wgpu::MapAsyncStatus writeStatus;
    EXPECT_EQ(instance.WaitAny(
                  src.MapAsync(wgpu::MapMode::Write, 0, kSize, wgpu::CallbackMode::AllowSpontaneous,
                               [&writeStatus](wgpu::MapAsyncStatus status, wgpu::StringView) {
                                   writeStatus = status;
                               }),
                  UINT64_MAX),
              wgpu::WaitStatus::Success);
    ASSERT_EQ(writeStatus, wgpu::MapAsyncStatus::Success);
    auto writeData = static_cast<uint32_t*>(src.GetMappedRange());
    ASSERT_NE(writeData, nullptr);
    *writeData = kData;
    src.Unmap();

    // Copy the buffer to the readable one, and wait for the copy to complete. Note that the wait
    // for the copy is not strictly necessary since the map async call following it will already
    // wait for it, but we do it explicitly here to test the additional entry point.
    wgpu::CommandEncoder encoder = device.CreateCommandEncoder();
    encoder.CopyBufferToBuffer(src, 0, dst, 0, kSize);
    wgpu::CommandBuffer commands = encoder.Finish();
    wgpu::Queue queue = device.GetQueue();
    queue.Submit(1, &commands);

    wgpu::QueueWorkDoneStatus copyStatus;
    EXPECT_EQ(
        instance.WaitAny(queue.OnSubmittedWorkDone(wgpu::CallbackMode::AllowSpontaneous,
                                                   [&copyStatus](wgpu::QueueWorkDoneStatus status) {
                                                       copyStatus = status;
                                                   }),
                         UINT64_MAX),
        wgpu::WaitStatus::Success);
    ASSERT_EQ(copyStatus, wgpu::QueueWorkDoneStatus::Success);

    // Map the readable buffer and verify the contents.
    wgpu::MapAsyncStatus readStatus;
    EXPECT_EQ(instance.WaitAny(
                  dst.MapAsync(wgpu::MapMode::Read, 0, kSize, wgpu::CallbackMode::AllowSpontaneous,
                               [&readStatus](wgpu::MapAsyncStatus status, wgpu::StringView) {
                                   readStatus = status;
                               }),
                  UINT64_MAX),
              wgpu::WaitStatus::Success);
    ASSERT_EQ(readStatus, wgpu::MapAsyncStatus::Success);
    auto readData = static_cast<const uint32_t*>(dst.GetConstMappedRange());
    ASSERT_NE(readData, nullptr);
    EXPECT_EQ(*readData, kData);
    dst.Unmap();
}

TEST_F(DeviceLevelTests, CreateComputePipelineAsync) {
    wgpu::ComputePipelineDescriptor desc;
    desc.compute.module = CreateShaderModule(R"(
        @compute @workgroup_size(1) fn main() {}
    )");

    wgpu::CreatePipelineAsyncStatus status;
    wgpu::ComputePipeline pipeline = nullptr;
    EXPECT_EQ(instance.WaitAny(device.CreateComputePipelineAsync(
                                   &desc, wgpu::CallbackMode::AllowSpontaneous,
                                   [&status, &pipeline](wgpu::CreatePipelineAsyncStatus s,
                                                        wgpu::ComputePipeline p, wgpu::StringView) {
                                       status = s;
                                       pipeline = std::move(p);
                                   }),
                               UINT64_MAX),
              wgpu::WaitStatus::Success);
    EXPECT_EQ(status, wgpu::CreatePipelineAsyncStatus::Success);
    EXPECT_NE(pipeline, nullptr);
}

TEST_F(DeviceLevelTests, CreateRenderPipelineAsync) {
    wgpu::RenderPipelineDescriptor desc;
    desc.vertex.module = CreateShaderModule(R"(
        @vertex fn main() -> @builtin(position) vec4f {
            return vec4f(0.0, 0.0, 0.0, 1.0);
        }
    )");

    wgpu::FragmentState frag;
    frag.module = CreateShaderModule(R"(
        @fragment fn main() -> @location(0) vec4f {
            return vec4f(0.0, 1.0, 0.0, 1.0);
        }
    )");
    wgpu::ColorTargetState target;
    target.format = wgpu::TextureFormat::RGBA8Unorm;
    frag.targetCount = 1;
    frag.targets = &target;
    desc.fragment = &frag;

    wgpu::CreatePipelineAsyncStatus status;
    wgpu::RenderPipeline pipeline = nullptr;
    EXPECT_EQ(instance.WaitAny(device.CreateRenderPipelineAsync(
                                   &desc, wgpu::CallbackMode::AllowSpontaneous,
                                   [&status, &pipeline](wgpu::CreatePipelineAsyncStatus s,
                                                        wgpu::RenderPipeline p, wgpu::StringView) {
                                       status = s;
                                       pipeline = std::move(p);
                                   }),
                               UINT64_MAX),
              wgpu::WaitStatus::Success);
    EXPECT_EQ(status, wgpu::CreatePipelineAsyncStatus::Success);
    EXPECT_NE(pipeline, nullptr);
}

TEST_F(DeviceLevelTests, GetCompilationInfo) {
    wgpu::ShaderModule shader = CreateShaderModule(R"(
        @fragment fn main(@location(0) x : f32) {
            return;
            return;
        }
    )");

    wgpu::CompilationMessageType messageType;
    std::string message;
    EXPECT_EQ(instance.WaitAny(shader.GetCompilationInfo(
                                   wgpu::CallbackMode::AllowSpontaneous,
                                   [&message, &messageType](wgpu::CompilationInfoRequestStatus s,
                                                            const wgpu::CompilationInfo* info) {
                                       ASSERT_EQ(s, wgpu::CompilationInfoRequestStatus::Success);
                                       ASSERT_NE(info, nullptr);
                                       ASSERT_EQ(info->messageCount, 1);

                                       message = info->messages[0].message;
                                       messageType = info->messages[0].type;
                                   }),
                               UINT64_MAX),
              wgpu::WaitStatus::Success);
    EXPECT_EQ(messageType, wgpu::CompilationMessageType::Warning);
    EXPECT_THAT(message, HasSubstr("unreachable"));
}

}  // namespace
