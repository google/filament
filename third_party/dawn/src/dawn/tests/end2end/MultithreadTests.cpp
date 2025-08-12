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

#include <atomic>
#include <condition_variable>
#include <functional>
#include <limits>
#include <memory>
#include <mutex>
#include <sstream>
#include <string>
#include <thread>
#include <utility>
#include <vector>

#include "dawn/common/Constants.h"
#include "dawn/common/Math.h"
#include "dawn/tests/DawnTest.h"
#include "dawn/utils/ComboRenderPipelineDescriptor.h"
#include "dawn/utils/TestUtils.h"
#include "dawn/utils/TextureUtils.h"
#include "dawn/utils/WGPUHelpers.h"

namespace dawn {
namespace {

template <typename Step>
class LockStep {
  public:
    LockStep() = delete;
    explicit LockStep(Step startStep) : mStep(startStep) {}

    void Signal(Step step) {
        std::lock_guard<std::mutex> lg(mMutex);
        mStep = step;
        mCv.notify_all();
    }

    void Wait(Step step) {
        std::unique_lock<std::mutex> lg(mMutex);
        mCv.wait(lg, [this, step] { return mStep == step; });
    }

  private:
    Step mStep;
    std::mutex mMutex;
    std::condition_variable mCv;
};

class MultithreadTests : public DawnTest {
  protected:
    std::vector<wgpu::FeatureName> GetRequiredFeatures() override {
        std::vector<wgpu::FeatureName> features;
        // TODO(crbug.com/dawn/1678): DawnWire doesn't support thread safe API yet.
        if (!UsesWire()) {
            features.push_back(wgpu::FeatureName::ImplicitDeviceSynchronization);
        }
        return features;
    }

    void SetUp() override {
        DawnTest::SetUp();
        // TODO(crbug.com/dawn/1678): DawnWire doesn't support thread safe API yet.
        DAWN_TEST_UNSUPPORTED_IF(UsesWire());

        // TODO(crbug.com/dawn/1679): OpenGL backend doesn't support thread safe API yet.
        DAWN_TEST_UNSUPPORTED_IF(IsOpenGL() || IsOpenGLES());
    }

    wgpu::Buffer CreateBuffer(uint32_t size, wgpu::BufferUsage usage) {
        wgpu::BufferDescriptor descriptor;
        descriptor.size = size;
        descriptor.usage = usage;
        return device.CreateBuffer(&descriptor);
    }

    wgpu::Texture CreateTexture(uint32_t width,
                                uint32_t height,
                                wgpu::TextureFormat format,
                                wgpu::TextureUsage usage,
                                uint32_t mipLevelCount = 1,
                                uint32_t sampleCount = 1) {
        wgpu::TextureDescriptor texDescriptor = {};
        texDescriptor.size = {width, height, 1};
        texDescriptor.format = format;
        texDescriptor.usage = usage;
        texDescriptor.mipLevelCount = mipLevelCount;
        texDescriptor.sampleCount = sampleCount;
        return device.CreateTexture(&texDescriptor);
    }
};

// Test that dropping a device's last ref on another thread won't crash Instance::ProcessEvents.
TEST_P(MultithreadTests, Device_DroppedOnAnotherThread) {
    // TODO(crbug.com/dawn/1779): This test seems to cause flakiness in other sampling tests on
    // NVIDIA.
    DAWN_SUPPRESS_TEST_IF(IsD3D12() && IsNvidia());
    // TODO(crbug.com/42240870): Flaky on Linux TSAN Release
    DAWN_SUPPRESS_TEST_IF(IsLinux() && IsVulkan() && IsTsan());

    std::vector<wgpu::Device> devices(5);

    // Create devices.
    for (size_t i = 0; i < devices.size(); ++i) {
        devices[i] = CreateDevice();
    }

    std::atomic<uint32_t> numAliveDevices = static_cast<uint32_t>(devices.size());

    // Create threads
    utils::RunInParallel(
        numAliveDevices.load(),
        [&devices, &numAliveDevices](uint32_t index) {
            EXPECT_NE(devices[index].Get(), nullptr);

            // Drop device.
            devices[index] = nullptr;

            numAliveDevices--;
        },
        [this, &numAliveDevices] {
            while (numAliveDevices.load() > 0) {
                // main thread process events from all devices
                WaitABit();
            }
        });
}

// Test that dropping a device's last ref inside a callback on another thread won't crash
// Instance::ProcessEvents.
TEST_P(MultithreadTests, Device_DroppedInCallback_OnAnotherThread) {
    // TODO(crbug.com/dawn/1779): This test seems to cause flakiness in other sampling tests on
    // NVIDIA.
    DAWN_SUPPRESS_TEST_IF(IsD3D12() && IsNvidia());
    // TODO(crbug.com/407567233): Seeing a lot of timeouts on this test on TSAN bots.
    DAWN_SUPPRESS_TEST_IF(IsLinux() && IsTsan());

    std::vector<wgpu::Device> devices(10);

    // Create devices.
    for (auto& device : devices) {
        device = CreateDevice();
    }

    // Create threads
    utils::RunInParallel(static_cast<uint32_t>(devices.size()), [&devices, this](uint32_t index) {
        auto additionalDevice = std::move(devices[index]);
        wgpu::Device device2ndRef = additionalDevice;
        std::atomic_bool isCompleted{false};

        // Drop the last ref inside a callback.
        additionalDevice.PushErrorScope(wgpu::ErrorFilter::Validation);
        additionalDevice.PopErrorScope(
            wgpu::CallbackMode::AllowProcessEvents,
            [&device2ndRef, &isCompleted](wgpu::PopErrorScopeStatus, wgpu::ErrorType,
                                          wgpu::StringView) {
                device2ndRef = nullptr;
                isCompleted = true;
            });
        // main ref dropped.
        additionalDevice = nullptr;

        do {
            WaitABit();
        } while (!isCompleted.load());

        EXPECT_EQ(device2ndRef, nullptr);
    });
}

// Test that waiting for a device lost after it's lost does not block.
TEST_P(MultithreadTests, Device_WaitForDroppedAfterDropped) {
    auto future = device.GetLostFuture();

    LoseDeviceForTesting();
    EXPECT_EQ(GetInstance().WaitAny(future, 0), wgpu::WaitStatus::Success);

    EXPECT_EQ(future.id, device.GetLostFuture().id);
}

// Test that we can wait for a device lost on another thread.
TEST_P(MultithreadTests, Device_WaitForDroppedInAnotherThread) {
    // TODO(crbug.com/dawn/1779): This test seems to cause flakiness in other sampling tests on
    // NVIDIA.
    DAWN_SUPPRESS_TEST_IF(IsD3D12() && IsNvidia());

    enum class Step {
        Begin,
        Waiting,
    };

    LockStep<Step> lockStep(Step::Begin);
    std::thread waitThread([&] {
        auto future = device.GetLostFuture();
        EXPECT_EQ(GetInstance().WaitAny(future, 0), wgpu::WaitStatus::TimedOut);
        lockStep.Signal(Step::Waiting);
        EXPECT_EQ(GetInstance().WaitAny(future, UINT64_MAX), wgpu::WaitStatus::Success);
    });

    lockStep.Wait(Step::Waiting);
    LoseDeviceForTesting();
    waitThread.join();
}

// Test that multiple buffers being created and mapped on multiple threads won't interfere with
// each other.
TEST_P(MultithreadTests, Buffers_MapInParallel) {
    constexpr uint32_t kDataSize = 1000;
    std::vector<uint32_t> myData;
    for (uint32_t i = 0; i < kDataSize; ++i) {
        myData.push_back(i);
    }

    constexpr uint32_t kSize = static_cast<uint32_t>(kDataSize * sizeof(uint32_t));

    utils::RunInParallel(10, [this, &myData = std::as_const(myData)](uint32_t) {
        // Create buffer and request mapping.
        wgpu::Buffer buffer =
            CreateBuffer(kSize, wgpu::BufferUsage::MapWrite | wgpu::BufferUsage::CopySrc);

        // Wait for the mapping to complete
        ASSERT_EQ(
            instance.WaitAny(buffer.MapAsync(wgpu::MapMode::Write, 0, kSize,
                                             wgpu::CallbackMode::AllowProcessEvents,
                                             [](wgpu::MapAsyncStatus status, wgpu::StringView) {
                                                 ASSERT_EQ(status, wgpu::MapAsyncStatus::Success);
                                             }),
                             UINT64_MAX),
            wgpu::WaitStatus::Success);

        // Buffer is mapped, write into it and unmap .
        memcpy(buffer.GetMappedRange(0, kSize), myData.data(), kSize);
        buffer.Unmap();

        // Check the content of the buffer.
        EXPECT_BUFFER_U32_RANGE_EQ(myData.data(), buffer, 0, kDataSize);
    });
}

// Test that copy a texture to a buffer then map that buffer in parallel works.
TEST_P(MultithreadTests, T2BThenMapInParallel) {
    constexpr uint32_t kTextureSize = 512;
    constexpr wgpu::TextureFormat kTextureFormat = wgpu::TextureFormat::RGBA8Unorm;
    constexpr uint32_t kBytesPerPixel = 4;
    constexpr uint64_t kBufferSize = kTextureSize * kTextureSize * kBytesPerPixel;

    std::vector<utils::RGBA8> textureData(kTextureSize * kTextureSize);
    for (uint32_t y = 0; y < kTextureSize; ++y) {
        for (uint32_t x = 0; x < kTextureSize; ++x) {
            textureData[y * kTextureSize + x] =
                utils::RGBA8(static_cast<uint8_t>(x % 256), static_cast<uint8_t>(y % 256),
                             static_cast<uint8_t>((x + y) % 256), 255);
        }
    }

    // Create and initialize the source texture.
    wgpu::Texture texture =
        CreateTexture(kTextureSize, kTextureSize, kTextureFormat,
                      wgpu::TextureUsage::CopySrc | wgpu::TextureUsage::CopyDst);

    wgpu::TexelCopyTextureInfo textureCopyView = utils::CreateTexelCopyTextureInfo(texture);
    wgpu::TexelCopyBufferLayout dataLayout =
        utils::CreateTexelCopyBufferLayout(0, kTextureSize * kBytesPerPixel);
    wgpu::Extent3D writeSize = {kTextureSize, kTextureSize, 1};
    queue.WriteTexture(&textureCopyView, textureData.data(),
                       textureData.size() * sizeof(utils::RGBA8), &dataLayout, &writeSize);

    for (int i = 0; i < 50; ++i) {
        utils::RunInParallel(20, [this, texture, &textureData = std::as_const(textureData),
                                  &textureCopyView = std::as_const(textureCopyView),
                                  &writeSize = std::as_const(writeSize)](uint32_t) {
            // Create a buffer for copying texture data to
            wgpu::Buffer buffer =
                CreateBuffer(kBufferSize, wgpu::BufferUsage::MapRead | wgpu::BufferUsage::CopyDst);

            // Copy texture to buffer.
            wgpu::CommandEncoder encoder = device.CreateCommandEncoder();
            wgpu::TexelCopyBufferInfo bufferCopyView =
                utils::CreateTexelCopyBufferInfo(buffer, 0, kTextureSize * kBytesPerPixel);
            encoder.CopyTextureToBuffer(&textureCopyView, &bufferCopyView, &writeSize);
            wgpu::CommandBuffer commands = encoder.Finish();
            queue.Submit(1, &commands);

            // Wait for the mapping to complete
            ASSERT_EQ(instance.WaitAny(
                          buffer.MapAsync(wgpu::MapMode::Read, 0, kBufferSize,
                                          wgpu::CallbackMode::WaitAnyOnly,
                                          [](wgpu::MapAsyncStatus status, wgpu::StringView) {
                                              ASSERT_EQ(status, wgpu::MapAsyncStatus::Success);
                                          }),
                          UINT64_MAX),
                      wgpu::WaitStatus::Success);

            // Buffer is mapped, check its content.
            const uint32_t* mappedData =
                static_cast<const uint32_t*>(buffer.GetConstMappedRange(0, kBufferSize));
            ASSERT_NE(mappedData, nullptr);
            EXPECT_EQ(0, memcmp(mappedData, textureData.data(), kBufferSize));
            buffer.Unmap();
        });
    }
}

// Test CreateShaderModule on multiple threads. Cache hits should share compilation warnings.
TEST_P(MultithreadTests, CreateShaderModuleInParallel) {
    constexpr uint32_t kCacheHitFactor = 4;  // 4 threads will create the same shader module.

    std::vector<std::string> shaderSources(10);
    std::vector<wgpu::ShaderModule> shaderModules(shaderSources.size() * kCacheHitFactor);

    std::string shader = R"(@fragment
    fn main(@location(0) x : f32) {
        return;
        return;
    };)";
    for (uint32_t i = 0; i < shaderSources.size(); ++i) {
        // Insert newlines to make the shaders unique.
        shader = "\n" + shader;
        shaderSources[i] = shader;
    }

    // Create shader modules in parallel.
    utils::RunInParallel(static_cast<uint32_t>(shaderModules.size()), [&](uint32_t index) {
        uint32_t sourceIndex = index / kCacheHitFactor;
        shaderModules[index] =
            utils::CreateShaderModule(device, shaderSources[sourceIndex].c_str());
    });

    // Check that the compilation info is correct for every shader module.
    for (uint32_t index = 0; index < shaderModules.size(); ++index) {
        uint32_t sourceIndex = index / kCacheHitFactor;
        shaderModules[index].GetCompilationInfo(
            wgpu::CallbackMode::AllowProcessEvents,
            [sourceIndex](wgpu::CompilationInfoRequestStatus status,
                          const wgpu::CompilationInfo* info) {
                ASSERT_EQ(wgpu::CompilationInfoRequestStatus::Success, status);
                for (size_t i = 0; i < info->messageCount; ++i) {
                    EXPECT_THAT(info->messages[i].message, testing::HasSubstr("unreachable"));
                    EXPECT_EQ(info->messages[i].lineNum, 5u + sourceIndex);
                }
            });
    }
}

// Test CreateComputePipelineAsync on multiple threads.
TEST_P(MultithreadTests, CreateComputePipelineAsyncInParallel) {
    // TODO(crbug.com/dawn/1766): TSAN reported race conditions in NVIDIA's vk driver.
    DAWN_SUPPRESS_TEST_IF(IsVulkan() && IsNvidia() && IsTsan());

    std::vector<wgpu::ComputePipeline> pipelines(10);
    std::vector<std::string> shaderSources(pipelines.size());
    std::vector<uint32_t> expectedValues(shaderSources.size());

    for (uint32_t i = 0; i < pipelines.size(); ++i) {
        expectedValues[i] = i + 1;

        std::ostringstream ss;
        ss << R"(
        struct SSBO {
            value : u32
        }
        @group(0) @binding(0) var<storage, read_write> ssbo : SSBO;

        @compute @workgroup_size(1) fn main() {
            ssbo.value =
        )";
        ss << expectedValues[i];
        ss << ";}";

        shaderSources[i] = ss.str();
    }

    // Create pipelines in parallel
    utils::RunInParallel(static_cast<uint32_t>(pipelines.size()), [&](uint32_t index) {
        wgpu::ComputePipelineDescriptor csDesc;
        csDesc.compute.module = utils::CreateShaderModule(device, shaderSources[index].c_str());

        struct Task {
            wgpu::ComputePipeline computePipeline;
            std::atomic<bool> isCompleted{false};
        } task;
        device.CreateComputePipelineAsync(
            &csDesc, wgpu::CallbackMode::AllowProcessEvents,
            [&task](wgpu::CreatePipelineAsyncStatus status, wgpu::ComputePipeline pipeline,
                    wgpu::StringView) {
                EXPECT_EQ(wgpu::CreatePipelineAsyncStatus::Success, status);

                task.computePipeline = std::move(pipeline);
                task.isCompleted = true;
            });

        while (!task.isCompleted.load()) {
            WaitABit();
        }

        pipelines[index] = task.computePipeline;
    });

    // Verify pipelines' executions
    for (uint32_t i = 0; i < pipelines.size(); ++i) {
        wgpu::Buffer ssbo =
            CreateBuffer(sizeof(uint32_t), wgpu::BufferUsage::Storage | wgpu::BufferUsage::CopySrc);

        wgpu::CommandBuffer commands;
        {
            wgpu::CommandEncoder encoder = device.CreateCommandEncoder();
            wgpu::ComputePassEncoder pass = encoder.BeginComputePass();

            ASSERT_NE(nullptr, pipelines[i].Get());
            wgpu::BindGroup bindGroup =
                utils::MakeBindGroup(device, pipelines[i].GetBindGroupLayout(0),
                                     {
                                         {0, ssbo, 0, sizeof(uint32_t)},
                                     });
            pass.SetBindGroup(0, bindGroup);
            pass.SetPipeline(pipelines[i]);

            pass.DispatchWorkgroups(1);
            pass.End();

            commands = encoder.Finish();
        }

        queue.Submit(1, &commands);

        EXPECT_BUFFER_U32_EQ(expectedValues[i], ssbo, 0);
    }
}

// Test CreateComputePipeline on multiple threads.
TEST_P(MultithreadTests, CreateComputePipelineInParallel) {
    // TODO(crbug.com/dawn/1766): TSAN reported race conditions in NVIDIA's vk driver.
    DAWN_SUPPRESS_TEST_IF(IsVulkan() && IsNvidia() && IsTsan());

    std::vector<wgpu::ComputePipeline> pipelines(10);
    std::vector<std::string> shaderSources(pipelines.size());
    std::vector<uint32_t> expectedValues(shaderSources.size());

    for (uint32_t i = 0; i < pipelines.size(); ++i) {
        expectedValues[i] = i + 1;

        std::ostringstream ss;
        ss << R"(
        struct SSBO {
            value : u32
        }
        @group(0) @binding(0) var<storage, read_write> ssbo : SSBO;

        @compute @workgroup_size(1) fn main() {
            ssbo.value =
        )";
        ss << expectedValues[i];
        ss << ";}";

        shaderSources[i] = ss.str();
    }

    // Create pipelines in parallel
    utils::RunInParallel(static_cast<uint32_t>(pipelines.size()), [&](uint32_t index) {
        wgpu::ComputePipelineDescriptor csDesc;
        csDesc.compute.module = utils::CreateShaderModule(device, shaderSources[index].c_str());
        pipelines[index] = device.CreateComputePipeline(&csDesc);
    });

    // Verify pipelines' executions
    for (uint32_t i = 0; i < pipelines.size(); ++i) {
        wgpu::Buffer ssbo =
            CreateBuffer(sizeof(uint32_t), wgpu::BufferUsage::Storage | wgpu::BufferUsage::CopySrc);

        wgpu::CommandBuffer commands;
        {
            wgpu::CommandEncoder encoder = device.CreateCommandEncoder();
            wgpu::ComputePassEncoder pass = encoder.BeginComputePass();

            ASSERT_NE(nullptr, pipelines[i].Get());
            wgpu::BindGroup bindGroup =
                utils::MakeBindGroup(device, pipelines[i].GetBindGroupLayout(0),
                                     {
                                         {0, ssbo, 0, sizeof(uint32_t)},
                                     });
            pass.SetBindGroup(0, bindGroup);
            pass.SetPipeline(pipelines[i]);

            pass.DispatchWorkgroups(1);
            pass.End();

            commands = encoder.Finish();
        }

        queue.Submit(1, &commands);

        EXPECT_BUFFER_U32_EQ(expectedValues[i], ssbo, 0);
    }
}

// Test CreateRenderPipelineAsync on multiple threads.
TEST_P(MultithreadTests, CreateRenderPipelineAsyncInParallel) {
    // TODO(crbug.com/dawn/1766): TSAN reported race conditions in NVIDIA's vk driver.
    DAWN_SUPPRESS_TEST_IF(IsVulkan() && IsNvidia() && IsTsan());

    constexpr uint32_t kNumThreads = 10;
    constexpr wgpu::TextureFormat kRenderAttachmentFormat = wgpu::TextureFormat::RGBA8Unorm;
    constexpr uint8_t kColorStep = 250 / kNumThreads;

    std::vector<wgpu::RenderPipeline> pipelines(kNumThreads);
    std::vector<std::string> fragmentShaderSources(kNumThreads);
    std::vector<utils::RGBA8> minExpectedValues(kNumThreads);
    std::vector<utils::RGBA8> maxExpectedValues(kNumThreads);

    for (uint32_t i = 0; i < kNumThreads; ++i) {
        // Due to floating point precision, we need to use min & max values to compare the
        // expectations.
        auto expectedGreen = kColorStep * i;
        minExpectedValues[i] =
            utils::RGBA8(0, expectedGreen == 0 ? 0 : (expectedGreen - 2), 0, 255);
        maxExpectedValues[i] =
            utils::RGBA8(0, expectedGreen == 255 ? 255 : (expectedGreen + 2), 0, 255);

        std::ostringstream ss;
        ss << R"(
        @fragment fn main() -> @location(0) vec4f {
            return vec4f(0.0,
        )";
        ss << expectedGreen / 255.0;
        ss << ", 0.0, 1.0);}";

        fragmentShaderSources[i] = ss.str();
    }

    // Create pipelines in parallel
    utils::RunInParallel(kNumThreads, [&](uint32_t index) {
        utils::ComboRenderPipelineDescriptor renderPipelineDescriptor;
        wgpu::ShaderModule vsModule = utils::CreateShaderModule(device, R"(
        @vertex fn main() -> @builtin(position) vec4f {
            return vec4f(0.0, 0.0, 0.0, 1.0);
        })");
        wgpu::ShaderModule fsModule =
            utils::CreateShaderModule(device, fragmentShaderSources[index].c_str());
        renderPipelineDescriptor.vertex.module = vsModule;
        renderPipelineDescriptor.cFragment.module = fsModule;
        renderPipelineDescriptor.cTargets[0].format = kRenderAttachmentFormat;
        renderPipelineDescriptor.primitive.topology = wgpu::PrimitiveTopology::PointList;

        struct Task {
            wgpu::RenderPipeline renderPipeline;
            std::atomic<bool> isCompleted{false};
        } task;
        device.CreateRenderPipelineAsync(
            &renderPipelineDescriptor, wgpu::CallbackMode::AllowProcessEvents,
            [&task](wgpu::CreatePipelineAsyncStatus status, wgpu::RenderPipeline pipeline,
                    wgpu::StringView) {
                EXPECT_EQ(wgpu::CreatePipelineAsyncStatus::Success, status);

                task.renderPipeline = std::move(pipeline);
                task.isCompleted = true;
            });

        while (!task.isCompleted) {
            WaitABit();
        }

        pipelines[index] = task.renderPipeline;
    });

    // Verify pipelines' executions
    for (uint32_t i = 0; i < pipelines.size(); ++i) {
        wgpu::Texture outputTexture =
            CreateTexture(1, 1, kRenderAttachmentFormat,
                          wgpu::TextureUsage::RenderAttachment | wgpu::TextureUsage::CopySrc);

        utils::ComboRenderPassDescriptor renderPassDescriptor({outputTexture.CreateView()});
        renderPassDescriptor.cColorAttachments[0].loadOp = wgpu::LoadOp::Clear;
        renderPassDescriptor.cColorAttachments[0].clearValue = {1.f, 0.f, 0.f, 1.f};

        wgpu::CommandBuffer commands;
        {
            wgpu::CommandEncoder encoder = device.CreateCommandEncoder();
            wgpu::RenderPassEncoder renderPassEncoder =
                encoder.BeginRenderPass(&renderPassDescriptor);

            ASSERT_NE(nullptr, pipelines[i].Get());

            renderPassEncoder.SetPipeline(pipelines[i]);
            renderPassEncoder.Draw(1);
            renderPassEncoder.End();
            commands = encoder.Finish();
        }

        queue.Submit(1, &commands);

        EXPECT_PIXEL_RGBA8_BETWEEN(minExpectedValues[i], maxExpectedValues[i], outputTexture, 0, 0);
    }
}

// Test CreateRenderPipeline on multiple threads.
TEST_P(MultithreadTests, CreateRenderPipelineInParallel) {
    // TODO(crbug.com/dawn/1766): TSAN reported race conditions in NVIDIA's vk driver.
    DAWN_SUPPRESS_TEST_IF(IsVulkan() && IsNvidia() && IsTsan());

    constexpr uint32_t kNumThreads = 10;
    constexpr wgpu::TextureFormat kRenderAttachmentFormat = wgpu::TextureFormat::RGBA8Unorm;
    constexpr uint8_t kColorStep = 250 / kNumThreads;

    std::vector<wgpu::RenderPipeline> pipelines(kNumThreads);
    std::vector<std::string> fragmentShaderSources(kNumThreads);
    std::vector<utils::RGBA8> minExpectedValues(kNumThreads);
    std::vector<utils::RGBA8> maxExpectedValues(kNumThreads);

    for (uint32_t i = 0; i < kNumThreads; ++i) {
        // Due to floating point precision, we need to use min & max values to compare the
        // expectations.
        auto expectedGreen = kColorStep * i;
        minExpectedValues[i] =
            utils::RGBA8(0, expectedGreen == 0 ? 0 : (expectedGreen - 2), 0, 255);
        maxExpectedValues[i] =
            utils::RGBA8(0, expectedGreen == 255 ? 255 : (expectedGreen + 2), 0, 255);

        std::ostringstream ss;
        ss << R"(
        @fragment fn main() -> @location(0) vec4f {
            return vec4f(0.0,
        )";
        ss << expectedGreen / 255.0;
        ss << ", 0.0, 1.0);}";

        fragmentShaderSources[i] = ss.str();
    }

    // Create pipelines in parallel
    utils::RunInParallel(kNumThreads, [&](uint32_t index) {
        utils::ComboRenderPipelineDescriptor renderPipelineDescriptor;
        wgpu::ShaderModule vsModule = utils::CreateShaderModule(device, R"(
        @vertex fn main() -> @builtin(position) vec4f {
            return vec4f(0.0, 0.0, 0.0, 1.0);
        })");
        wgpu::ShaderModule fsModule =
            utils::CreateShaderModule(device, fragmentShaderSources[index].c_str());
        renderPipelineDescriptor.vertex.module = vsModule;
        renderPipelineDescriptor.cFragment.module = fsModule;
        renderPipelineDescriptor.cTargets[0].format = kRenderAttachmentFormat;
        renderPipelineDescriptor.primitive.topology = wgpu::PrimitiveTopology::PointList;

        pipelines[index] = device.CreateRenderPipeline(&renderPipelineDescriptor);
    });

    // Verify pipelines' executions
    for (uint32_t i = 0; i < pipelines.size(); ++i) {
        wgpu::Texture outputTexture =
            CreateTexture(1, 1, kRenderAttachmentFormat,
                          wgpu::TextureUsage::RenderAttachment | wgpu::TextureUsage::CopySrc);

        utils::ComboRenderPassDescriptor renderPassDescriptor({outputTexture.CreateView()});
        renderPassDescriptor.cColorAttachments[0].loadOp = wgpu::LoadOp::Clear;
        renderPassDescriptor.cColorAttachments[0].clearValue = {1.f, 0.f, 0.f, 1.f};

        wgpu::CommandBuffer commands;
        {
            wgpu::CommandEncoder encoder = device.CreateCommandEncoder();
            wgpu::RenderPassEncoder renderPassEncoder =
                encoder.BeginRenderPass(&renderPassDescriptor);

            ASSERT_NE(nullptr, pipelines[i].Get());

            renderPassEncoder.SetPipeline(pipelines[i]);
            renderPassEncoder.Draw(1);
            renderPassEncoder.End();
            commands = encoder.Finish();
        }

        queue.Submit(1, &commands);

        EXPECT_PIXEL_RGBA8_BETWEEN(minExpectedValues[i], maxExpectedValues[i], outputTexture, 0, 0);
    }
}

// Test that creating and destroying bind groups from the same bind group layout on multiple threads
// won't race. The bind groups will all be allocated by same SlabAllocator.
TEST_P(MultithreadTests, CreateAndDestroyBindGroupsInParallel) {
    wgpu::BindGroupLayout layout = utils::MakeBindGroupLayout(
        device, {
                    {0, wgpu::ShaderStage::Fragment, wgpu::BufferBindingType::Storage},
                });
    wgpu::Buffer ssbo =
        CreateBuffer(sizeof(uint32_t), wgpu::BufferUsage::Storage | wgpu::BufferUsage::CopySrc |
                                           wgpu::BufferUsage::CopyDst);
    utils::RunInParallel(100, [&, this](uint32_t) {
        for (int i = 0; i < 10; ++i) {
            wgpu::BindGroup bindGroup = utils::MakeBindGroup(device, layout,
                                                             {
                                                                 {0, ssbo, 0, sizeof(uint32_t)},
                                                             });
            EXPECT_NE(nullptr, bindGroup.Get());
        }
    });
}

// Test that destroying Texture and associated TextureViews simultaneously on different threads
// works. These was a data race here previously, see https://crbug.com/396294899 for more details.
TEST_P(MultithreadTests, DestroyTextureAndViewsAtSameTime) {
    constexpr uint32_t kNumViews = 10;
    constexpr uint32_t kNumThreads = kNumViews + 1;
    constexpr wgpu::TextureFormat kTextureFormat = wgpu::TextureFormat::R8Unorm;
    constexpr wgpu::TextureUsage kTextureUsage = wgpu::TextureUsage::CopySrc |
                                                 wgpu::TextureUsage::CopyDst |
                                                 wgpu::TextureUsage::RenderAttachment;

    for (uint32_t j = 0; j < 50; ++j) {
        wgpu::TextureDescriptor texDescriptor = {};
        texDescriptor.size = {1, 1, kNumViews};
        texDescriptor.format = kTextureFormat;
        texDescriptor.usage = kTextureUsage;
        texDescriptor.mipLevelCount = 1;
        texDescriptor.sampleCount = 1;

        wgpu::Texture texture = device.CreateTexture(&texDescriptor);
        wgpu::TextureView textureViews[kNumViews];

        for (uint32_t i = 0; i < kNumViews; ++i) {
            wgpu::TextureViewDescriptor viewDescriptor = {};
            viewDescriptor.format = kTextureFormat;
            viewDescriptor.usage = kTextureUsage;
            viewDescriptor.mipLevelCount = 1;
            viewDescriptor.baseArrayLayer = i;
            viewDescriptor.arrayLayerCount = 1;
            textureViews[i] = texture.CreateView(&viewDescriptor);
        }

        // Wait for all threads to be ready to destroy the Texture or TextureViews at the same time.
        // TODO(kylechar): std::latch would be much simpler here but MSVC bots fail to run
        // dawn_end2end_tests when it's used.
        std::mutex mutex;
        std::condition_variable cv;
        uint32_t waiting = kNumThreads;

        utils::RunInParallel(kNumThreads, [&](uint32_t id) {
            bool notify = false;
            {
                std::lock_guard lock(mutex);
                if (--waiting == 0) {
                    notify = true;
                }
            }
            if (notify) {
                cv.notify_all();
            } else {
                std::unique_lock lock(mutex);
                cv.wait(lock, [&waiting]() { return waiting == 0; });
            }

            if (id == 0) {
                texture.Destroy();
            } else {
                textureViews[id - 1] = {};
            }
        });
    }
}

class MultithreadCachingTests : public MultithreadTests {
  protected:
    wgpu::ShaderModule CreateComputeShaderModule() const {
        return utils::CreateShaderModule(device, R"(
            struct SSBO {
                value : u32
            }
            @group(0) @binding(0) var<storage, read_write> ssbo : SSBO;

            @compute @workgroup_size(1) fn main() {
                ssbo.value = 1;
            })");
    }

    wgpu::BindGroupLayout CreateComputeBindGroupLayout() const {
        return utils::MakeBindGroupLayout(
            device, {
                        {0, wgpu::ShaderStage::Compute, wgpu::BufferBindingType::Storage},
                    });
    }
};

// Test that creating a same shader module (which will return the cached shader module) and release
// it on multiple threads won't race.
TEST_P(MultithreadCachingTests, RefAndReleaseCachedShaderModulesInParallel) {
    utils::RunInParallel(100, [this](uint32_t) {
        wgpu::ShaderModule csModule = CreateComputeShaderModule();
        EXPECT_NE(nullptr, csModule.Get());
    });
}

// Test that creating a same compute pipeline (which will return the cached pipeline) and release it
// on multiple threads won't race.
TEST_P(MultithreadCachingTests, RefAndReleaseCachedComputePipelinesInParallel) {
    wgpu::ShaderModule csModule = CreateComputeShaderModule();
    wgpu::BindGroupLayout bglayout = CreateComputeBindGroupLayout();
    wgpu::PipelineLayout pipelineLayout = utils::MakePipelineLayout(device, {bglayout});

    wgpu::ComputePipelineDescriptor csDesc;
    csDesc.compute.module = csModule;
    csDesc.layout = pipelineLayout;

    utils::RunInParallel(100, [&, this](uint32_t) {
        wgpu::ComputePipeline pipeline = device.CreateComputePipeline(&csDesc);
        EXPECT_NE(nullptr, pipeline.Get());
    });
}

// Test that creating a same bind group layout (which will return the cached layout) and
// release it on multiple threads won't race.
TEST_P(MultithreadCachingTests, RefAndReleaseCachedBindGroupLayoutsInParallel) {
    utils::RunInParallel(100, [&, this](uint32_t) {
        wgpu::BindGroupLayout layout = CreateComputeBindGroupLayout();
        EXPECT_NE(nullptr, layout.Get());
    });
}

// Test that creating a same pipeline layout (which will return the cached layout) and
// release it on multiple threads won't race.
TEST_P(MultithreadCachingTests, RefAndReleaseCachedPipelineLayoutsInParallel) {
    wgpu::BindGroupLayout bglayout = CreateComputeBindGroupLayout();

    utils::RunInParallel(100, [&, this](uint32_t) {
        wgpu::PipelineLayout pipelineLayout = utils::MakePipelineLayout(device, {bglayout});
        EXPECT_NE(nullptr, pipelineLayout.Get());
    });
}

// Test that creating a same render pipeline (which will return the cached pipeline) and release it
// on multiple threads won't race.
TEST_P(MultithreadCachingTests, RefAndReleaseCachedRenderPipelinesInParallel) {
    utils::ComboRenderPipelineDescriptor renderPipelineDescriptor;
    wgpu::ShaderModule vsModule = utils::CreateShaderModule(device, R"(
        @vertex fn main() -> @builtin(position) vec4f {
            return vec4f(0.0, 0.0, 0.0, 1.0);
        })");
    wgpu::ShaderModule fsModule = utils::CreateShaderModule(device, R"(
        @fragment fn main() -> @location(0) vec4f {
            return vec4f(0.0, 1.0, 0.0, 1.0);
        })");
    renderPipelineDescriptor.vertex.module = vsModule;
    renderPipelineDescriptor.cFragment.module = fsModule;
    renderPipelineDescriptor.cTargets[0].format = wgpu::TextureFormat::RGBA8Unorm;
    renderPipelineDescriptor.primitive.topology = wgpu::PrimitiveTopology::PointList;

    utils::RunInParallel(100, [&, this](uint32_t) {
        wgpu::RenderPipeline pipeline = device.CreateRenderPipeline(&renderPipelineDescriptor);
        EXPECT_NE(nullptr, pipeline.Get());
    });
}

// Test that creating a same sampler pipeline (which will return the cached sampler) and release it
// on multiple threads won't race.
TEST_P(MultithreadCachingTests, RefAndReleaseCachedSamplersInParallel) {
    wgpu::SamplerDescriptor desc = {};
    utils::RunInParallel(100, [&, this](uint32_t) {
        wgpu::Sampler sampler = device.CreateSampler(&desc);
        EXPECT_NE(nullptr, sampler.Get());
    });
}

class MultithreadEncodingTests : public MultithreadTests {};

// Test that encoding render passes in parallel should work
TEST_P(MultithreadEncodingTests, RenderPassEncodersInParallel) {
    constexpr uint32_t kRTSize = 16;
    constexpr uint32_t kNumThreads = 10;

    wgpu::Texture msaaRenderTarget =
        CreateTexture(kRTSize, kRTSize, wgpu::TextureFormat::RGBA8Unorm,
                      wgpu::TextureUsage::RenderAttachment | wgpu::TextureUsage::CopySrc,
                      /*mipLevelCount=*/1, /*sampleCount=*/4);
    wgpu::TextureView msaaRenderTargetView = msaaRenderTarget.CreateView();

    wgpu::Texture resolveTarget =
        CreateTexture(kRTSize, kRTSize, wgpu::TextureFormat::RGBA8Unorm,
                      wgpu::TextureUsage::RenderAttachment | wgpu::TextureUsage::CopySrc);
    wgpu::TextureView resolveTargetView = resolveTarget.CreateView();

    std::vector<wgpu::CommandBuffer> commandBuffers(kNumThreads);

    utils::RunInParallel(kNumThreads, [this, msaaRenderTargetView, resolveTargetView,
                                       &commandBuffers](uint32_t index) {
        wgpu::CommandEncoder encoder = device.CreateCommandEncoder();

        // Clear the renderTarget to red.
        utils::ComboRenderPassDescriptor renderPass({msaaRenderTargetView});
        renderPass.cColorAttachments[0].resolveTarget = resolveTargetView;
        renderPass.cColorAttachments[0].clearValue = {1.0f, 0.0f, 0.0f, 1.0f};

        wgpu::RenderPassEncoder pass = encoder.BeginRenderPass(&renderPass);
        pass.End();

        commandBuffers[index] = encoder.Finish();
    });

    // Verify that the command buffers executed correctly.
    for (auto& commandBuffer : commandBuffers) {
        queue.Submit(1, &commandBuffer);

        EXPECT_TEXTURE_EQ(utils::RGBA8::kRed, resolveTarget, {0, 0});
        EXPECT_TEXTURE_EQ(utils::RGBA8::kRed, resolveTarget, {kRTSize - 1, kRTSize - 1});
    }
}

// Test that encoding render passes that resolve to a mip level in parallel should work
TEST_P(MultithreadEncodingTests, RenderPassEncoders_ResolveToMipLevelOne_InParallel) {
    constexpr uint32_t kRTSize = 16;
    constexpr uint32_t kNumThreads = 10;

    wgpu::Texture msaaRenderTarget =
        CreateTexture(kRTSize, kRTSize, wgpu::TextureFormat::RGBA8Unorm,
                      wgpu::TextureUsage::RenderAttachment | wgpu::TextureUsage::CopySrc,
                      /*mipLevelCount=*/1, /*sampleCount=*/4);
    wgpu::TextureView msaaRenderTargetView = msaaRenderTarget.CreateView();

    // Resolve to mip level = 1 to force render pass workarounds (there shouldn't be any deadlock
    // happening).
    wgpu::Texture resolveTarget =
        CreateTexture(kRTSize * 2, kRTSize * 2, wgpu::TextureFormat::RGBA8Unorm,
                      wgpu::TextureUsage::RenderAttachment | wgpu::TextureUsage::CopySrc,
                      /*mipLevelCount=*/2, /*sampleCount=*/1);
    wgpu::TextureViewDescriptor resolveTargetViewDesc;
    resolveTargetViewDesc.baseMipLevel = 1;
    resolveTargetViewDesc.mipLevelCount = 1;
    wgpu::TextureView resolveTargetView = resolveTarget.CreateView(&resolveTargetViewDesc);

    std::vector<wgpu::CommandBuffer> commandBuffers(kNumThreads);

    utils::RunInParallel(kNumThreads, [this, msaaRenderTargetView, resolveTargetView,
                                       &commandBuffers](uint32_t index) {
        wgpu::CommandEncoder encoder = device.CreateCommandEncoder();

        // Clear the renderTarget to red.
        utils::ComboRenderPassDescriptor renderPass({msaaRenderTargetView});
        renderPass.cColorAttachments[0].resolveTarget = resolveTargetView;
        renderPass.cColorAttachments[0].clearValue = {1.0f, 0.0f, 0.0f, 1.0f};

        wgpu::RenderPassEncoder pass = encoder.BeginRenderPass(&renderPass);
        pass.End();

        commandBuffers[index] = encoder.Finish();
    });

    // Verify that the command buffers executed correctly.
    for (auto& commandBuffer : commandBuffers) {
        queue.Submit(1, &commandBuffer);

        EXPECT_TEXTURE_EQ(utils::RGBA8::kRed, resolveTarget, {0, 0}, 1);
        EXPECT_TEXTURE_EQ(utils::RGBA8::kRed, resolveTarget, {kRTSize - 1, kRTSize - 1}, 1);
    }
}

// Test that encoding compute passes in parallel should work
TEST_P(MultithreadEncodingTests, ComputePassEncodersInParallel) {
    constexpr uint32_t kNumThreads = 10;
    constexpr uint32_t kExpected = 0xFFFFFFFFu;

    wgpu::ShaderModule module = utils::CreateShaderModule(device, R"(
            @group(0) @binding(0) var<storage, read_write> output : u32;

            @compute @workgroup_size(1, 1, 1)
            fn main(@builtin(global_invocation_id) GlobalInvocationID : vec3u) {
                output = 0xFFFFFFFFu;
            })");
    wgpu::ComputePipelineDescriptor csDesc;
    csDesc.compute.module = module;
    auto pipeline = device.CreateComputePipeline(&csDesc);

    wgpu::Buffer dstBuffer =
        CreateBuffer(sizeof(uint32_t), wgpu::BufferUsage::Storage | wgpu::BufferUsage::CopySrc |
                                           wgpu::BufferUsage::CopyDst);
    wgpu::BindGroup bindGroup = utils::MakeBindGroup(device, pipeline.GetBindGroupLayout(0),
                                                     {
                                                         {0, dstBuffer, 0, sizeof(uint32_t)},
                                                     });

    std::vector<wgpu::CommandBuffer> commandBuffers(kNumThreads);

    utils::RunInParallel(kNumThreads, [this, pipeline, bindGroup, &commandBuffers](uint32_t index) {
        wgpu::CommandEncoder encoder = device.CreateCommandEncoder();
        wgpu::ComputePassEncoder pass = encoder.BeginComputePass();
        pass.SetPipeline(pipeline);
        pass.SetBindGroup(0, bindGroup);
        pass.DispatchWorkgroups(1, 1, 1);
        pass.End();

        commandBuffers[index] = encoder.Finish();
    });

    // Verify that the command buffers executed correctly.
    for (auto& commandBuffer : commandBuffers) {
        constexpr uint32_t kSentinelData = 0;
        queue.WriteBuffer(dstBuffer, 0, &kSentinelData, sizeof(kSentinelData));
        queue.Submit(1, &commandBuffer);

        EXPECT_BUFFER_U32_EQ(kExpected, dstBuffer, 0);
    }
}

class MultithreadTextureCopyTests : public MultithreadTests {
  protected:
    void SetUp() override { MultithreadTests::SetUp(); }

    wgpu::Texture CreateAndWriteTexture(uint32_t width,
                                        uint32_t height,
                                        wgpu::TextureFormat format,
                                        wgpu::TextureUsage usage,
                                        const void* data,
                                        size_t dataSize) {
        auto texture = CreateTexture(width, height, format, wgpu::TextureUsage::CopyDst | usage);

        wgpu::Extent3D textureSize = {width, height, 1};

        wgpu::TexelCopyTextureInfo texelCopyTextureInfo =
            utils::CreateTexelCopyTextureInfo(texture, 0, {0, 0, 0}, wgpu::TextureAspect::All);
        wgpu::TexelCopyBufferLayout texelCopyBufferLayout =
            utils::CreateTexelCopyBufferLayout(0, dataSize / height);

        queue.WriteTexture(&texelCopyTextureInfo, data, dataSize, &texelCopyBufferLayout,
                           &textureSize);

        return texture;
    }

    uint32_t BufferSizeForTextureCopy(uint32_t width, uint32_t height, wgpu::TextureFormat format) {
        uint32_t bytesPerRow = utils::GetMinimumBytesPerRow(format, width);
        return utils::RequiredBytesInCopy(bytesPerRow, height, {width, height, 1}, format);
    }

    void CopyTextureToTextureHelper(
        const wgpu::Texture& srcTexture,
        const wgpu::TexelCopyTextureInfo& dst,
        const wgpu::Extent3D& dstSize,
        const wgpu::CommandEncoder& encoder,
        const wgpu::CopyTextureForBrowserOptions* copyForBrowerOptions = nullptr) {
        wgpu::TexelCopyTextureInfo srcView =
            utils::CreateTexelCopyTextureInfo(srcTexture, 0, {0, 0, 0}, wgpu::TextureAspect::All);

        if (copyForBrowerOptions == nullptr) {
            encoder.CopyTextureToTexture(&srcView, &dst, &dstSize);

            wgpu::CommandBuffer commands = encoder.Finish();
            queue.Submit(1, &commands);
        } else {
            // Don't need encoder
            DAWN_ASSERT(encoder == nullptr);
            queue.CopyTextureForBrowser(&srcView, &dst, &dstSize, copyForBrowerOptions);
        }
    }

    void CopyBufferToTextureHelper(const wgpu::Buffer& srcBuffer,
                                   uint32_t srcBytesPerRow,
                                   const wgpu::TexelCopyTextureInfo& dst,
                                   const wgpu::Extent3D& dstSize,
                                   const wgpu::CommandEncoder& encoder) {
        wgpu::TexelCopyBufferInfo srcView =
            utils::CreateTexelCopyBufferInfo(srcBuffer, 0, srcBytesPerRow, dstSize.height);

        encoder.CopyBufferToTexture(&srcView, &dst, &dstSize);

        wgpu::CommandBuffer commands = encoder.Finish();
        queue.Submit(1, &commands);
    }
};

// Test that depth texture's CopyTextureToTexture() can work in parallel with other commands (such
// resources creation and texture to buffer copy for texture expectations).
// This test is needed since most of command encoder's commands are not synchronized, but
// CopyTextureToTexture() command might internally allocate resources and we need to make sure that
// it won't race with other threads' works.
TEST_P(MultithreadTextureCopyTests, CopyDepthToDepthNoRace) {
    // TODO(crbug.com/dawn/1766): TSAN reported race conditions in NVIDIA's vk driver.
    DAWN_SUPPRESS_TEST_IF(IsVulkan() && IsNvidia() && IsTsan());

    enum class Step {
        Begin,
        WriteTexture,
    };

    constexpr uint32_t kWidth = 4;
    constexpr uint32_t kHeight = 4;

    const std::vector<float> kExpectedData32 = {
        0,    0,    0,    0,  //
        0,    0,    0.4f, 0,  //
        1.0f, 1.0f, 0,    0,  //
        1.0f, 1.0f, 0,    0,  //
    };

    std::vector<uint16_t> kExpectedData16(kExpectedData32.size());
    for (size_t i = 0; i < kExpectedData32.size(); ++i) {
        kExpectedData16[i] = kExpectedData32[i] * std::numeric_limits<uint16_t>::max();
    }

    const size_t kExpectedDataSize16 = kExpectedData16.size() * sizeof(kExpectedData16[0]);

    LockStep<Step> lockStep(Step::Begin);

    wgpu::Texture depthTexture;
    std::thread writeThread([&] {
        depthTexture = CreateAndWriteTexture(
            kWidth, kHeight, wgpu::TextureFormat::Depth16Unorm,
            wgpu::TextureUsage::CopySrc | wgpu::TextureUsage::RenderAttachment,
            kExpectedData16.data(), kExpectedDataSize16);

        lockStep.Signal(Step::WriteTexture);

        // Verify the initial data
        ExpectAttachmentDepthTestData(depthTexture, wgpu::TextureFormat::Depth16Unorm, kWidth,
                                      kHeight, 0, /*mipLevel=*/0, kExpectedData32);
    });

    std::thread copyThread([&] {
        auto destTexture =
            CreateTexture(kWidth * 2, kHeight * 2, wgpu::TextureFormat::Depth16Unorm,
                          wgpu::TextureUsage::RenderAttachment | wgpu::TextureUsage::CopyDst |
                              wgpu::TextureUsage::CopySrc,
                          /*mipLevelCount=*/2);

        // Copy from depthTexture to destTexture.
        const wgpu::Extent3D dstSize = {kWidth, kHeight, 1};
        wgpu::TexelCopyTextureInfo dest = utils::CreateTexelCopyTextureInfo(
            destTexture, /*dstMipLevel=*/1, {0, 0, 0}, wgpu::TextureAspect::All);
        auto encoder = device.CreateCommandEncoder();
        lockStep.Wait(Step::WriteTexture);
        CopyTextureToTextureHelper(depthTexture, dest, dstSize, encoder);

        // Verify the copied data
        ExpectAttachmentDepthTestData(destTexture, wgpu::TextureFormat::Depth16Unorm, kWidth,
                                      kHeight, 0, /*mipLevel=*/1, kExpectedData32);
    });

    writeThread.join();
    copyThread.join();
}

// Test that depth texture's CopyBufferToTexture() can work in parallel with other commands (such
// resources creation and texture to buffer copy for texture expectations).
// This test is needed since most of command encoder's commands are not synchronized, but
// CopyBufferToTexture() command might internally allocate resources and we need to make sure that
// it won't race with other threads' works.
TEST_P(MultithreadTextureCopyTests, CopyBufferToDepthNoRace) {
    enum class Step {
        Begin,
        WriteBuffer,
    };

    constexpr uint32_t kWidth = 16;
    constexpr uint32_t kHeight = 1;

    const std::vector<float> kExpectedData32 = {
        0,    0,    0,    0,  //
        0,    0,    0.4f, 0,  //
        1.0f, 1.0f, 0,    0,  //
        1.0f, 1.0f, 0,    0,  //
    };

    std::vector<uint16_t> kExpectedData16(kExpectedData32.size());
    for (size_t i = 0; i < kExpectedData32.size(); ++i) {
        kExpectedData16[i] = kExpectedData32[i] * std::numeric_limits<uint16_t>::max();
    }

    const uint32_t kExpectedDataSize16 = kExpectedData16.size() * sizeof(kExpectedData16[0]);

    const wgpu::Extent3D kSize = {kWidth, kHeight, 1};
    LockStep<Step> lockStep(Step::Begin);

    wgpu::Buffer buffer;
    std::thread writeThread([&] {
        buffer = CreateBuffer(
            BufferSizeForTextureCopy(kWidth, kHeight, wgpu::TextureFormat::Depth16Unorm),
            wgpu::BufferUsage::CopyDst | wgpu::BufferUsage::CopySrc);

        queue.WriteBuffer(buffer, 0, kExpectedData16.data(), kExpectedDataSize16);
        device.Tick();

        lockStep.Signal(Step::WriteBuffer);

        EXPECT_BUFFER_U16_RANGE_EQ(kExpectedData16.data(), buffer, 0, kExpectedData16.size());
    });

    std::thread copyThread([&] {
        auto destTexture =
            CreateTexture(kWidth, kHeight, wgpu::TextureFormat::Depth16Unorm,
                          wgpu::TextureUsage::RenderAttachment | wgpu::TextureUsage::CopyDst |
                              wgpu::TextureUsage::CopySrc);

        auto encoder = device.CreateCommandEncoder();

        wgpu::TexelCopyTextureInfo dest = utils::CreateTexelCopyTextureInfo(
            destTexture, /*dstMipLevel=*/0, {0, 0, 0}, wgpu::TextureAspect::All);

        // Wait until src buffer is written.
        lockStep.Wait(Step::WriteBuffer);
        CopyBufferToTextureHelper(buffer, kTextureBytesPerRowAlignment, dest, kSize, encoder);

        // Verify the copied data
        ExpectAttachmentDepthTestData(destTexture, wgpu::TextureFormat::Depth16Unorm, kWidth,
                                      kHeight, 0, /*mipLevel=*/0, kExpectedData32);
    });

    writeThread.join();
    copyThread.join();
}

// Test that stencil texture's CopyTextureToTexture() can work in parallel with other commands (such
// resources creation and texture to buffer copy for texture expectations).
// This test is needed since most of command encoder's commands are not synchronized, but
// CopyTextureToTexture() command might internally allocate resources and we need to make sure that
// it won't race with other threads' works.
TEST_P(MultithreadTextureCopyTests, CopyStencilToStencilNoRace) {
    // TODO(crbug.com/dawn/1497): glReadPixels: GL error: HIGH: Invalid format and type
    // combination.
    DAWN_SUPPRESS_TEST_IF(IsANGLE());

    // TODO(dawn:1924): Intel Gen9 specific.
    DAWN_SUPPRESS_TEST_IF(IsD3D11() && IsIntelGen9());

    // TODO(crbug.com/dawn/1766): TSAN reported race conditions in NVIDIA's vk driver.
    DAWN_SUPPRESS_TEST_IF(IsVulkan() && IsNvidia() && IsTsan());

    enum class Step {
        Begin,
        WriteTexture,
    };

    constexpr uint32_t kWidth = 1;
    constexpr uint32_t kHeight = 1;

    constexpr uint8_t kExpectedData = 177;
    constexpr size_t kExpectedDataSize = sizeof(kExpectedData);

    LockStep<Step> lockStep(Step::Begin);

    wgpu::Texture stencilTexture;
    std::thread writeThread([&] {
        stencilTexture = CreateAndWriteTexture(
            kWidth, kHeight, wgpu::TextureFormat::Stencil8,
            wgpu::TextureUsage::CopySrc | wgpu::TextureUsage::RenderAttachment, &kExpectedData,
            kExpectedDataSize);

        lockStep.Signal(Step::WriteTexture);

        // Verify the initial data
        ExpectAttachmentStencilTestData(stencilTexture, wgpu::TextureFormat::Stencil8, kWidth,
                                        kHeight, 0, /*mipLevel=*/0, kExpectedData);
    });

    std::thread copyThread([&] {
        auto destTexture =
            CreateTexture(kWidth * 2, kHeight * 2, wgpu::TextureFormat::Stencil8,
                          wgpu::TextureUsage::RenderAttachment | wgpu::TextureUsage::CopyDst |
                              wgpu::TextureUsage::CopySrc,
                          /*mipLevelCount=*/2);

        // Copy from stencilTexture to destTexture.
        const wgpu::Extent3D dstSize = {kWidth, kHeight, 1};
        wgpu::TexelCopyTextureInfo dest = utils::CreateTexelCopyTextureInfo(
            destTexture, /*dstMipLevel=*/1, {0, 0, 0}, wgpu::TextureAspect::All);
        auto encoder = device.CreateCommandEncoder();
        lockStep.Wait(Step::WriteTexture);

        CopyTextureToTextureHelper(stencilTexture, dest, dstSize, encoder);

        // Verify the copied data
        ExpectAttachmentStencilTestData(destTexture, wgpu::TextureFormat::Stencil8, kWidth, kHeight,
                                        0, /*mipLevel=*/1, kExpectedData);
    });

    writeThread.join();
    copyThread.join();
}

// Test that stencil texture's CopyBufferToTexture() can work in parallel with other commands (such
// resources creation and texture to buffer copy for texture expectations).
// This test is needed since most of command encoder's commands are not synchronized, but
// CopyBufferToTexture() command might internally allocate resources and we need to make sure that
// it won't race with other threads' works.
TEST_P(MultithreadTextureCopyTests, CopyBufferToStencilNoRace) {
    enum class Step {
        Begin,
        WriteBuffer,
    };

    constexpr uint32_t kWidth = 1;
    constexpr uint32_t kHeight = 1;

    constexpr uint8_t kExpectedData = 177;

    const wgpu::Extent3D kSize = {kWidth, kHeight, 1};
    LockStep<Step> lockStep(Step::Begin);

    wgpu::Buffer buffer;
    std::thread writeThread([&] {
        const auto kBufferSize = kTextureBytesPerRowAlignment;
        buffer = CreateBuffer(kBufferSize, wgpu::BufferUsage::CopyDst | wgpu::BufferUsage::CopySrc);

        std::vector<uint8_t> bufferData(kBufferSize);
        bufferData[0] = kExpectedData;

        queue.WriteBuffer(buffer.Get(), 0, bufferData.data(), kBufferSize);
        device.Tick();

        lockStep.Signal(Step::WriteBuffer);

        EXPECT_BUFFER_U8_EQ(kExpectedData, buffer, 0);
    });

    std::thread copyThread([&] {
        auto destTexture =
            CreateTexture(kWidth, kHeight, wgpu::TextureFormat::Stencil8,
                          wgpu::TextureUsage::RenderAttachment | wgpu::TextureUsage::CopyDst |
                              wgpu::TextureUsage::CopySrc);

        auto encoder = device.CreateCommandEncoder();

        wgpu::TexelCopyTextureInfo dest = utils::CreateTexelCopyTextureInfo(
            destTexture, /*dstMipLevel=*/0, {0, 0, 0}, wgpu::TextureAspect::All);

        // Wait until src buffer is written.
        lockStep.Wait(Step::WriteBuffer);
        CopyBufferToTextureHelper(buffer, kTextureBytesPerRowAlignment, dest, kSize, encoder);

        // Verify the copied data
        ExpectAttachmentStencilTestData(destTexture, wgpu::TextureFormat::Stencil8, kWidth, kHeight,
                                        0, /*mipLevel=*/0, kExpectedData);
    });

    writeThread.join();
    copyThread.join();
}

// Test that color texture's CopyTextureForBrowser() can work in parallel with other commands (such
// resources creation and texture to buffer copy for texture expectations).
// This test is needed since CopyTextureForBrowser() command might internally allocate resources and
// we need to make sure that it won't race with other threads' works.
TEST_P(MultithreadTextureCopyTests, CopyTextureForBrowserNoRace) {
    DAWN_SUPPRESS_TEST_IF(IsOpenGL() && IsLinux());

    enum class Step {
        Begin,
        WriteTexture,
    };

    constexpr uint32_t kWidth = 4;
    constexpr uint32_t kHeight = 4;

    const std::vector<utils::RGBA8> kExpectedData = {
        utils::RGBA8::kBlack, utils::RGBA8::kBlack, utils::RGBA8::kBlack, utils::RGBA8::kBlack,  //
        utils::RGBA8::kBlack, utils::RGBA8::kBlack, utils::RGBA8::kGreen, utils::RGBA8::kBlack,  //
        utils::RGBA8::kRed,   utils::RGBA8::kRed,   utils::RGBA8::kBlack, utils::RGBA8::kBlack,  //
        utils::RGBA8::kRed,   utils::RGBA8::kBlue,  utils::RGBA8::kBlack, utils::RGBA8::kBlack,  //
    };

    const std::vector<utils::RGBA8> kExpectedFlippedData = {
        utils::RGBA8::kRed,   utils::RGBA8::kBlue,  utils::RGBA8::kBlack, utils::RGBA8::kBlack,  //
        utils::RGBA8::kRed,   utils::RGBA8::kRed,   utils::RGBA8::kBlack, utils::RGBA8::kBlack,  //
        utils::RGBA8::kBlack, utils::RGBA8::kBlack, utils::RGBA8::kGreen, utils::RGBA8::kBlack,  //
        utils::RGBA8::kBlack, utils::RGBA8::kBlack, utils::RGBA8::kBlack, utils::RGBA8::kBlack,  //
    };

    const size_t kExpectedDataSize = kExpectedData.size() * sizeof(kExpectedData[0]);

    LockStep<Step> lockStep(Step::Begin);

    wgpu::Texture srcTexture;
    std::thread writeThread([&] {
        srcTexture =
            CreateAndWriteTexture(kWidth, kHeight, wgpu::TextureFormat::RGBA8Unorm,
                                  wgpu::TextureUsage::CopySrc | wgpu::TextureUsage::TextureBinding,
                                  kExpectedData.data(), kExpectedDataSize);

        lockStep.Signal(Step::WriteTexture);

        // Verify the initial data
        EXPECT_TEXTURE_EQ(kExpectedData.data(), srcTexture, {0, 0}, {kWidth, kHeight});
    });

    std::thread copyThread([&] {
        auto destTexture =
            CreateTexture(kWidth, kHeight, wgpu::TextureFormat::RGBA8Unorm,
                          wgpu::TextureUsage::RenderAttachment | wgpu::TextureUsage::CopyDst |
                              wgpu::TextureUsage::CopySrc);

        // Copy from srcTexture to destTexture.
        const wgpu::Extent3D dstSize = {kWidth, kHeight, 1};
        wgpu::TexelCopyTextureInfo dest = utils::CreateTexelCopyTextureInfo(
            destTexture, /*dstMipLevel=*/0, {0, 0, 0}, wgpu::TextureAspect::All);
        wgpu::CopyTextureForBrowserOptions options;
        options.flipY = true;

        lockStep.Wait(Step::WriteTexture);
        CopyTextureToTextureHelper(srcTexture, dest, dstSize, nullptr, &options);

        // Verify the copied data
        EXPECT_TEXTURE_EQ(kExpectedFlippedData.data(), destTexture, {0, 0}, {kWidth, kHeight});
    });

    writeThread.join();
    copyThread.join();
}

// Test that error from CopyTextureForBrowser() won't cause deadlock.
TEST_P(MultithreadTextureCopyTests, CopyTextureForBrowserErrorNoDeadLock) {
    DAWN_SUPPRESS_TEST_IF(IsOpenGL() && IsLinux());

    DAWN_TEST_UNSUPPORTED_IF(HasToggleEnabled("skip_validation"));

    enum class Step {
        Begin,
        WriteTexture,
    };

    constexpr uint32_t kWidth = 4;
    constexpr uint32_t kHeight = 4;

    const std::vector<utils::RGBA8> kExpectedData = {
        utils::RGBA8::kBlack, utils::RGBA8::kBlack, utils::RGBA8::kBlack, utils::RGBA8::kBlack,  //
        utils::RGBA8::kBlack, utils::RGBA8::kBlack, utils::RGBA8::kGreen, utils::RGBA8::kBlack,  //
        utils::RGBA8::kRed,   utils::RGBA8::kRed,   utils::RGBA8::kBlack, utils::RGBA8::kBlack,  //
        utils::RGBA8::kRed,   utils::RGBA8::kBlue,  utils::RGBA8::kBlack, utils::RGBA8::kBlack,  //
    };

    const size_t kExpectedDataSize = kExpectedData.size() * sizeof(kExpectedData[0]);

    LockStep<Step> lockStep(Step::Begin);

    wgpu::Texture srcTexture;
    std::thread writeThread([&] {
        srcTexture =
            CreateAndWriteTexture(kWidth, kHeight, wgpu::TextureFormat::RGBA8Unorm,
                                  wgpu::TextureUsage::CopySrc | wgpu::TextureUsage::TextureBinding,
                                  kExpectedData.data(), kExpectedDataSize);

        lockStep.Signal(Step::WriteTexture);

        // Verify the initial data
        EXPECT_TEXTURE_EQ(kExpectedData.data(), srcTexture, {0, 0}, {kWidth, kHeight});
    });

    std::thread copyThread([&] {
        wgpu::Texture invalidSrcTexture;
        invalidSrcTexture = CreateTexture(kWidth, kHeight, wgpu::TextureFormat::RGBA8Unorm,
                                          wgpu::TextureUsage::CopySrc);
        auto destTexture =
            CreateTexture(kWidth, kHeight, wgpu::TextureFormat::RGBA8Unorm,
                          wgpu::TextureUsage::RenderAttachment | wgpu::TextureUsage::CopyDst |
                              wgpu::TextureUsage::CopySrc);

        // Copy from srcTexture to destTexture.
        const wgpu::Extent3D dstSize = {kWidth, kHeight, 1};
        wgpu::TexelCopyTextureInfo dest = utils::CreateTexelCopyTextureInfo(
            destTexture, /*dstMipLevel=*/0, {0, 0, 0}, wgpu::TextureAspect::All);
        wgpu::CopyTextureForBrowserOptions options = {};

        device.PushErrorScope(wgpu::ErrorFilter::Validation);

        // The first copy should be an error because of missing TextureBinding from src texture.
        lockStep.Wait(Step::WriteTexture);
        CopyTextureToTextureHelper(invalidSrcTexture, dest, dstSize, nullptr, &options);

        std::atomic<bool> errorThrown(false);
        device.PopErrorScope(wgpu::CallbackMode::AllowProcessEvents,
                             [&errorThrown](wgpu::PopErrorScopeStatus status, wgpu::ErrorType type,
                                            wgpu::StringView) {
                                 EXPECT_EQ(status, wgpu::PopErrorScopeStatus::Success);
                                 EXPECT_EQ(type, wgpu::ErrorType::Validation);
                                 errorThrown = true;
                             });
        instance.ProcessEvents();
        EXPECT_TRUE(errorThrown.load());

        // Second copy is valid.
        CopyTextureToTextureHelper(srcTexture, dest, dstSize, nullptr, &options);

        // Verify the copied data
        EXPECT_TEXTURE_EQ(kExpectedData.data(), destTexture, {0, 0}, {kWidth, kHeight});
    });

    writeThread.join();
    copyThread.join();
}

class MultithreadDrawIndexedIndirectTests : public MultithreadTests {
  protected:
    void SetUp() override {
        MultithreadTests::SetUp();

        wgpu::ShaderModule vsModule = utils::CreateShaderModule(device, R"(
            @vertex
            fn main(@location(0) pos : vec4f) -> @builtin(position) vec4f {
                return pos;
            })");

        wgpu::ShaderModule fsModule = utils::CreateShaderModule(device, R"(
            @fragment fn main() -> @location(0) vec4f {
                return vec4f(0.0, 1.0, 0.0, 1.0);
            })");

        utils::ComboRenderPipelineDescriptor descriptor;
        descriptor.vertex.module = vsModule;
        descriptor.cFragment.module = fsModule;
        descriptor.primitive.topology = wgpu::PrimitiveTopology::TriangleStrip;
        descriptor.primitive.stripIndexFormat = wgpu::IndexFormat::Uint32;
        descriptor.vertex.bufferCount = 1;
        descriptor.cBuffers[0].arrayStride = 4 * sizeof(float);
        descriptor.cBuffers[0].attributeCount = 1;
        descriptor.cAttributes[0].format = wgpu::VertexFormat::Float32x4;
        descriptor.cTargets[0].format = utils::BasicRenderPass::kDefaultColorFormat;

        pipeline = device.CreateRenderPipeline(&descriptor);

        vertexBuffer = utils::CreateBufferFromData<float>(
            device, wgpu::BufferUsage::Vertex,
            {// First quad: the first 3 vertices represent the bottom left triangle
             -1.0f, 1.0f, 0.0f, 1.0f, 1.0f, -1.0f, 0.0f, 1.0f, -1.0f, -1.0f, 0.0f, 1.0f, 1.0f, 1.0f,
             0.0f, 1.0f,

             // Second quad: the first 3 vertices represent the top right triangle
             -1.0f, 1.0f, 0.0f, 1.0f, 1.0f, -1.0f, 0.0f, 1.0f, 1.0f, 1.0f, 0.0f, 1.0f, -1.0f, -1.0f,
             0.0f, 1.0f});
    }

    void Test(std::initializer_list<uint32_t> bufferList,
              uint64_t indexOffset,
              uint64_t indirectOffset,
              utils::RGBA8 bottomLeftExpected,
              utils::RGBA8 topRightExpected) {
        utils::BasicRenderPass renderPass = utils::CreateBasicRenderPass(
            device, kRTSize, kRTSize, utils::BasicRenderPass::kDefaultColorFormat);
        wgpu::Buffer indexBuffer =
            CreateIndexBuffer({0, 1, 2, 0, 3, 1,
                               // The indices below are added to test negatve baseVertex
                               0 + 4, 1 + 4, 2 + 4, 0 + 4, 3 + 4, 1 + 4});
        TestDraw(
            renderPass, bottomLeftExpected, topRightExpected,
            EncodeDrawCommands(bufferList, indexBuffer, indexOffset, indirectOffset, renderPass));
    }

  private:
    wgpu::Buffer CreateIndirectBuffer(std::initializer_list<uint32_t> indirectParamList) {
        return utils::CreateBufferFromData<uint32_t>(
            device, wgpu::BufferUsage::Indirect | wgpu::BufferUsage::Storage, indirectParamList);
    }

    wgpu::Buffer CreateIndexBuffer(std::initializer_list<uint32_t> indexList) {
        return utils::CreateBufferFromData<uint32_t>(device, wgpu::BufferUsage::Index, indexList);
    }

    wgpu::CommandBuffer EncodeDrawCommands(std::initializer_list<uint32_t> bufferList,
                                           wgpu::Buffer indexBuffer,
                                           uint64_t indexOffset,
                                           uint64_t indirectOffset,
                                           const utils::BasicRenderPass& renderPass) {
        wgpu::Buffer indirectBuffer = CreateIndirectBuffer(bufferList);

        wgpu::CommandEncoder encoder = device.CreateCommandEncoder();
        {
            wgpu::RenderPassEncoder pass = encoder.BeginRenderPass(&renderPass.renderPassInfo);
            pass.SetPipeline(pipeline);
            pass.SetVertexBuffer(0, vertexBuffer);
            pass.SetIndexBuffer(indexBuffer, wgpu::IndexFormat::Uint32, indexOffset);
            pass.DrawIndexedIndirect(indirectBuffer, indirectOffset);
            pass.End();
        }

        return encoder.Finish();
    }

    void TestDraw(const utils::BasicRenderPass& renderPass,
                  utils::RGBA8 bottomLeftExpected,
                  utils::RGBA8 topRightExpected,
                  wgpu::CommandBuffer commands) {
        queue.Submit(1, &commands);

        EXPECT_PIXEL_RGBA8_EQ(bottomLeftExpected, renderPass.color, 1, 3);
        EXPECT_PIXEL_RGBA8_EQ(topRightExpected, renderPass.color, 3, 1);
    }

    wgpu::RenderPipeline pipeline;
    wgpu::Buffer vertexBuffer;
    static constexpr uint32_t kRTSize = 4;
};

// Test indirect draws with offsets on multiple threads.
TEST_P(MultithreadDrawIndexedIndirectTests, IndirectOffsetInParallel) {
    // TODO(crbug.com/dawn/789): Test is failing after a roll on SwANGLE on Windows only.
    DAWN_SUPPRESS_TEST_IF(IsANGLE() && IsWindows());

    utils::RGBA8 filled(0, 255, 0, 255);
    utils::RGBA8 notFilled(0, 0, 0, 0);

    utils::RunInParallel(10, [this, filled, notFilled](uint32_t) {
        // Test an offset draw call, with indirect buffer containing 2 calls:
        // 1) first 3 indices of the second quad (top right triangle)
        // 2) last 3 indices of the second quad

        // Test #1 (no offset)
        Test({3, 1, 0, 4, 0, 3, 1, 3, 4, 0}, 0, 0, notFilled, filled);

        // Offset to draw #2
        Test({3, 1, 0, 4, 0, 3, 1, 3, 4, 0}, 0, 5 * sizeof(uint32_t), filled, notFilled);
    });
}

class TimestampExpectation : public detail::Expectation {
  public:
    ~TimestampExpectation() override = default;

    // Expect the timestamp results are greater than 0.
    testing::AssertionResult Check(const void* data, size_t size) override {
        DAWN_ASSERT(size % sizeof(uint64_t) == 0);
        const uint64_t* timestamps = static_cast<const uint64_t*>(data);
        for (size_t i = 0; i < size / sizeof(uint64_t); i++) {
            if (timestamps[i] == 0) {
                return testing::AssertionFailure()
                       << "Expected data[" << i << "] to be greater than 0.\n";
            }
        }

        return testing::AssertionSuccess();
    }
};

class MultithreadTimestampQueryTests : public MultithreadTests {
  protected:
    void SetUp() override {
        MultithreadTests::SetUp();

        // Skip all tests if timestamp feature is not supported
        DAWN_TEST_UNSUPPORTED_IF(!SupportsFeatures({wgpu::FeatureName::TimestampQuery}));
    }

    std::vector<wgpu::FeatureName> GetRequiredFeatures() override {
        std::vector<wgpu::FeatureName> requiredFeatures = MultithreadTests::GetRequiredFeatures();
        if (SupportsFeatures({wgpu::FeatureName::TimestampQuery})) {
            requiredFeatures.push_back(wgpu::FeatureName::TimestampQuery);
        }
        return requiredFeatures;
    }

    wgpu::QuerySet CreateQuerySetForTimestamp(uint32_t queryCount) {
        wgpu::QuerySetDescriptor descriptor;
        descriptor.count = queryCount;
        descriptor.type = wgpu::QueryType::Timestamp;
        return device.CreateQuerySet(&descriptor);
    }

    wgpu::Buffer CreateResolveBuffer(uint64_t size) {
        return CreateBuffer(size, /*usage=*/wgpu::BufferUsage::QueryResolve |
                                      wgpu::BufferUsage::CopySrc | wgpu::BufferUsage::CopyDst);
    }
};

// Test resolving timestamp queries on multiple threads. ResolveQuerySet() will create temp
// resources internally so we need to make sure they are thread safe.
TEST_P(MultithreadTimestampQueryTests, ResolveQuerySets_InParallel) {
    DAWN_SUPPRESS_TEST_IF(IsWARP());  // Flaky on WARP

    constexpr uint32_t kQueryCount = 2;
    constexpr uint32_t kNumThreads = 10;

    std::vector<wgpu::QuerySet> querySets(kNumThreads);
    std::vector<wgpu::Buffer> destinations(kNumThreads);

    for (size_t i = 0; i < kNumThreads; ++i) {
        querySets[i] = CreateQuerySetForTimestamp(kQueryCount);
        destinations[i] = CreateResolveBuffer(kQueryCount * sizeof(uint64_t));
    }

    utils::RunInParallel(kNumThreads, [&](uint32_t index) {
        const auto& querySet = querySets[index];
        const auto& destination = destinations[index];
        wgpu::CommandEncoder encoder = device.CreateCommandEncoder();
        encoder.WriteTimestamp(querySet, 0);
        encoder.WriteTimestamp(querySet, 1);
        encoder.ResolveQuerySet(querySet, 0, kQueryCount, destination, 0);
        wgpu::CommandBuffer commands = encoder.Finish();
        queue.Submit(1, &commands);

        EXPECT_BUFFER(destination, 0, kQueryCount * sizeof(uint64_t), new TimestampExpectation);
    });
}

DAWN_INSTANTIATE_TEST(MultithreadTests,
                      D3D11Backend(),
                      D3D11Backend({"d3d11_use_unmonitored_fence"}),
                      D3D11Backend({"d3d11_delay_flush_to_gpu"}),
                      D3D12Backend(),
                      MetalBackend(),
                      OpenGLBackend(),
                      OpenGLESBackend(),
                      VulkanBackend());

DAWN_INSTANTIATE_TEST(MultithreadCachingTests,
                      D3D11Backend(),
                      D3D11Backend({"d3d11_use_unmonitored_fence"}),
                      D3D11Backend({"d3d11_delay_flush_to_gpu"}),
                      D3D12Backend(),
                      MetalBackend(),
                      OpenGLBackend(),
                      OpenGLESBackend(),
                      VulkanBackend());

DAWN_INSTANTIATE_TEST(MultithreadEncodingTests,
                      D3D11Backend(),
                      D3D11Backend({"d3d11_use_unmonitored_fence"}),
                      D3D11Backend({"d3d11_delay_flush_to_gpu"}),
                      D3D12Backend(),
                      D3D12Backend({"always_resolve_into_zero_level_and_layer"}),
                      MetalBackend(),
                      MetalBackend({"always_resolve_into_zero_level_and_layer"}),
                      OpenGLBackend(),
                      OpenGLESBackend(),
                      VulkanBackend(),
                      VulkanBackend({"always_resolve_into_zero_level_and_layer"}));

DAWN_INSTANTIATE_TEST(
    MultithreadTextureCopyTests,
    D3D11Backend(),
    D3D12Backend(),
    MetalBackend(),
    MetalBackend({"use_blit_for_buffer_to_depth_texture_copy",
                  "use_blit_for_depth_texture_to_texture_copy_to_nonzero_subresource"}),
    MetalBackend({"use_blit_for_buffer_to_stencil_texture_copy"}),
    OpenGLBackend(),
    OpenGLESBackend(),
    VulkanBackend());

DAWN_INSTANTIATE_TEST(MultithreadDrawIndexedIndirectTests,
                      D3D11Backend(),
                      D3D12Backend(),
                      MetalBackend(),
                      OpenGLBackend(),
                      OpenGLESBackend(),
                      VulkanBackend());

DAWN_INSTANTIATE_TEST(MultithreadTimestampQueryTests,
                      D3D11Backend(),
                      D3D12Backend(),
                      MetalBackend(),
                      OpenGLBackend(),
                      OpenGLESBackend(),
                      VulkanBackend());

}  // anonymous namespace
}  // namespace dawn
