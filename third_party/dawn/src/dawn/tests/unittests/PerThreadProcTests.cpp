// Copyright 2020 The Dawn & Tint Authors
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

#include <gtest/gtest.h>
#include <webgpu/webgpu_cpp.h>

#include <atomic>
#include <thread>

#include "dawn/dawn_thread_dispatch_proc.h"
#include "dawn/native/DawnNative.h"
#include "dawn/native/Instance.h"
#include "dawn/native/null/DeviceNull.h"

namespace dawn {
namespace {

class PerThreadProcTests : public testing::Test {
  public:
    PerThreadProcTests()
        : mNativeInstance(native::APICreateInstance(nullptr)),
          mAdapterBase(mNativeInstance.Get(),
                       AcquireRef(new native::null::PhysicalDevice()),
                       wgpu::FeatureLevel::Core,
                       native::TogglesState(native::ToggleStage::Adapter),
                       wgpu::PowerPreference::Undefined) {}
    ~PerThreadProcTests() override = default;

  protected:
    Ref<native::InstanceBase> mNativeInstance;
    native::AdapterBase mAdapterBase;
};

// Test that procs can be set per thread. This test overrides deviceCreateBuffer with a placeholder
// proc for each thread that increments a counter. Because each thread has their own proc and
// counter, there should be no data races. The per-thread procs also check that the current thread
// id is exactly equal to the expected thread id.
TEST_F(PerThreadProcTests, DispatchesPerThread) {
    dawnProcSetProcs(&dawnThreadDispatchProcTable);

    // Threads will block on this atomic to be sure we set procs on both threads before
    // either thread calls the procs.
    std::atomic<bool> ready(false);

    static int threadACounter = 0;
    static int threadBCounter = 0;

    static std::atomic<std::thread::id> threadIdA;
    static std::atomic<std::thread::id> threadIdB;

    constexpr int kThreadATargetCount = 28347;
    constexpr int kThreadBTargetCount = 40420;

    // Note: Acquire doesn't call reference or release.
    wgpu::Device deviceA =
        wgpu::Device::Acquire(reinterpret_cast<WGPUDevice>(mAdapterBase.APICreateDevice()));

    wgpu::Device deviceB =
        wgpu::Device::Acquire(reinterpret_cast<WGPUDevice>(mAdapterBase.APICreateDevice()));

    std::thread threadA([&] {
        DawnProcTable procs = native::GetProcs();
        procs.deviceCreateBuffer = [](WGPUDevice device,
                                      WGPUBufferDescriptor const* descriptor) -> WGPUBuffer {
            EXPECT_EQ(std::this_thread::get_id(), threadIdA);
            threadACounter++;
            return nullptr;
        };
        dawnProcSetPerThreadProcs(&procs);

        while (!ready) {
        }  // Should be fast, so just spin.

        for (int i = 0; i < kThreadATargetCount; ++i) {
            deviceA.CreateBuffer(nullptr);
        }

        deviceA = nullptr;
        dawnProcSetPerThreadProcs(nullptr);
    });

    std::thread threadB([&] {
        DawnProcTable procs = native::GetProcs();
        procs.deviceCreateBuffer = [](WGPUDevice device,
                                      WGPUBufferDescriptor const* bufferDesc) -> WGPUBuffer {
            EXPECT_EQ(std::this_thread::get_id(), threadIdB);
            threadBCounter++;
            return nullptr;
        };
        dawnProcSetPerThreadProcs(&procs);

        while (!ready) {
        }  // Should be fast, so just spin.

        for (int i = 0; i < kThreadBTargetCount; ++i) {
            deviceB.CreateBuffer(nullptr);
        }

        deviceB = nullptr;
        dawnProcSetPerThreadProcs(nullptr);
    });

    threadIdA = threadA.get_id();
    threadIdB = threadB.get_id();

    ready = true;
    threadA.join();
    threadB.join();

    EXPECT_EQ(threadACounter, kThreadATargetCount);
    EXPECT_EQ(threadBCounter, kThreadBTargetCount);

    dawnProcSetProcs(nullptr);
}

TEST_F(PerThreadProcTests, DispatchesDefaultThread) {
    dawnProcSetProcs(&dawnThreadDispatchProcTable);

    // Threads will block on this atomic to be sure we set procs on both threads before
    // either thread calls the procs.
    std::atomic<bool> ready(false);

    static int threadDefaultCounter = 0;
    static int threadACounter = 0;

    static std::atomic<std::thread::id> threadIdA;
    static std::atomic<std::thread::id> threadIdB;

    constexpr int kThreadATargetCount = 28347;
    constexpr int kThreadBTargetCount = 40420;

    {
        DawnProcTable procs = native::GetProcs();
        procs.deviceCreateBuffer = [](WGPUDevice device,
                                      WGPUBufferDescriptor const* descriptor) -> WGPUBuffer {
            EXPECT_EQ(std::this_thread::get_id(), threadIdB);
            threadDefaultCounter++;
            return nullptr;
        };
        dawnProcSetDefaultThreadProcs(&procs);
    }

    // Note: Acquire doesn't call reference or release.
    wgpu::Device deviceA =
        wgpu::Device::Acquire(reinterpret_cast<WGPUDevice>(mAdapterBase.APICreateDevice()));

    wgpu::Device deviceB =
        wgpu::Device::Acquire(reinterpret_cast<WGPUDevice>(mAdapterBase.APICreateDevice()));

    std::thread threadA([&] {
        DawnProcTable procs = native::GetProcs();
        procs.deviceCreateBuffer = [](WGPUDevice device,
                                      WGPUBufferDescriptor const* descriptor) -> WGPUBuffer {
            EXPECT_EQ(std::this_thread::get_id(), threadIdA);
            threadACounter++;
            return nullptr;
        };
        dawnProcSetPerThreadProcs(&procs);

        while (!ready) {
        }  // Should be fast, so just spin.

        for (int i = 0; i < kThreadATargetCount; ++i) {
            deviceA.CreateBuffer(nullptr);
        }

        deviceA = nullptr;
        dawnProcSetPerThreadProcs(nullptr);
    });

    std::thread threadB([&] {
        while (!ready) {
        }  // Should be fast, so just spin.

        for (int i = 0; i < kThreadBTargetCount; ++i) {
            deviceB.CreateBuffer(nullptr);
        }

        deviceB = nullptr;
        dawnProcSetPerThreadProcs(nullptr);
    });

    threadIdA = threadA.get_id();
    threadIdB = threadB.get_id();

    ready = true;
    threadA.join();
    threadB.join();

    EXPECT_EQ(threadDefaultCounter, kThreadBTargetCount);
    EXPECT_EQ(threadACounter, kThreadATargetCount);

    dawnProcSetDefaultThreadProcs(nullptr);
    dawnProcSetProcs(nullptr);
}

}  // anonymous namespace
}  // namespace dawn
