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

#include <memory>
#include <utility>
#include <vector>

#include "dawn/dawn_proc.h"
#include "dawn/tests/DawnTest.h"
#include "dawn/utils/SystemUtils.h"
#include "dawn/utils/WGPUHelpers.h"

namespace dawn {
namespace {

class DeviceInitializationTest : public testing::Test {
  protected:
    void SetUp() override { dawnProcSetProcs(&native::GetProcs()); }

    void TearDown() override { dawnProcSetProcs(nullptr); }

    // Test that the device can still be used by creating an async pipeline. Note that this test
    // would be better if we did something like a buffer copy instead, but that can only be done
    // once wgpu::CallbackMode::AllowSpontaneous is completely implemented.
    // TODO(crbug.com/42241003): Update this test do a buffer copy instead.
    void ExpectDeviceUsable(wgpu::Device device) {
        device.PushErrorScope(wgpu::ErrorFilter::Validation);

        wgpu::ComputePipelineDescriptor desc;
        desc.compute.module = utils::CreateShaderModule(device, R"(
            @compute @workgroup_size(1) fn main() {}
        )");

        std::atomic<uint8_t> callbacks = 0;
        device.CreateComputePipelineAsync(
            &desc, wgpu::CallbackMode::AllowSpontaneous,
            [&callbacks](wgpu::CreatePipelineAsyncStatus status, wgpu::ComputePipeline pipeline,
                         wgpu::StringView) {
                EXPECT_EQ(status, wgpu::CreatePipelineAsyncStatus::Success);
                EXPECT_NE(pipeline, nullptr);
                callbacks++;
            });

        device.PopErrorScope(
            wgpu::CallbackMode::AllowSpontaneous,
            [&callbacks](wgpu::PopErrorScopeStatus status, wgpu::ErrorType type, wgpu::StringView) {
                EXPECT_EQ(status, wgpu::PopErrorScopeStatus::Success);
                EXPECT_EQ(type, wgpu::ErrorType::NoError);
                callbacks++;
            });

        while (callbacks != 2) {
            utils::USleep(100);
        }
    }
};

// Test that device operations are still valid if the reference to the instance
// is dropped.
TEST_F(DeviceInitializationTest, DeviceOutlivesInstance) {
    // Get info of all available adapters and then free the instance.
    // We want to create a device on a fresh instance and adapter each time.
    std::vector<wgpu::AdapterInfo> availableAdapterInfo;
    {
        auto instance = std::make_unique<native::Instance>();
        // TODO(347047627): Use a webgpu.h version of enumerateAdapters
        for (const native::Adapter& nativeAdapter : instance->EnumerateAdapters()) {
            wgpu::Adapter adapter = wgpu::Adapter(nativeAdapter.Get());
            wgpu::AdapterInfo info;
            adapter.GetInfo(&info);

            if (info.backendType == wgpu::BackendType::Null) {
                continue;
            }

            availableAdapterInfo.push_back(std::move(info));
        }
    }

    for (const wgpu::AdapterInfo& desiredInfo : availableAdapterInfo) {
        wgpu::Device device;

        auto instance = std::make_unique<native::Instance>();
        // TODO(347047627): Use a webgpu.h version of enumerateAdapters
        for (native::Adapter& nativeAdapter : instance->EnumerateAdapters()) {
            wgpu::Adapter adapter = wgpu::Adapter(nativeAdapter.Get());
            wgpu::AdapterInfo info;
            adapter.GetInfo(&info);

            if (info.deviceID == desiredInfo.deviceID && info.vendorID == desiredInfo.vendorID &&
                info.adapterType == desiredInfo.adapterType &&
                info.backendType == desiredInfo.backendType) {
                // Create the device, destroy the instance, and break out of the loop.
                device = wgpu::Device::Acquire(nativeAdapter.CreateDevice());
                instance.reset();
                break;
            }
        }

        if (device) {
            ExpectDeviceUsable(std::move(device));
        }
    }
}

// Test that it is still possible to create a device from an adapter after the reference to the
// instance is dropped.
TEST_F(DeviceInitializationTest, AdapterOutlivesInstance) {
    // Get info of all available adapters and then free the instance.
    // We want to create a device on a fresh instance and adapter each time.
    std::vector<wgpu::AdapterInfo> availableAdapterInfo;
    {
        auto instance = std::make_unique<native::Instance>();
        // TODO(347047627): Use a webgpu.h version of enumerateAdapters
        for (const native::Adapter& nativeAdapter : instance->EnumerateAdapters()) {
            wgpu::Adapter adapter = wgpu::Adapter(nativeAdapter.Get());
            wgpu::AdapterInfo info;
            adapter.GetInfo(&info);

            if (info.backendType == wgpu::BackendType::Null) {
                continue;
            }
            availableAdapterInfo.push_back(std::move(info));
        }
    }

    for (const wgpu::AdapterInfo& desiredInfo : availableAdapterInfo) {
        wgpu::Adapter adapter;

        auto instance = std::make_unique<native::Instance>();
        // TODO(347047627): Use a webgpu.h version of enumerateAdapters
        for (native::Adapter& nativeAdapter : instance->EnumerateAdapters()) {
            wgpu::AdapterInfo info;
            wgpu::Adapter(nativeAdapter.Get()).GetInfo(&info);

            if (info.deviceID == desiredInfo.deviceID && info.vendorID == desiredInfo.vendorID &&
                info.adapterType == desiredInfo.adapterType &&
                info.backendType == desiredInfo.backendType) {
                // Save the adapter, and reset the instance.
                adapter = wgpu::Adapter(nativeAdapter.Get());
                instance.reset();
                break;
            }
        }

        if (adapter) {
            ExpectDeviceUsable(adapter.CreateDevice());
        }
    }
}

}  // anonymous namespace
}  // namespace dawn
