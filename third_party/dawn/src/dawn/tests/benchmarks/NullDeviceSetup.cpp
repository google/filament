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

#include "dawn/tests/benchmarks/NullDeviceSetup.h"

#include <benchmark/benchmark.h>
#include <dawn/webgpu_cpp.h>
#include <dawn/webgpu_cpp_print.h>
#include <memory>
#include <utility>

#include "dawn/common/Assert.h"
#include "dawn/common/Log.h"
#include "dawn/dawn_proc.h"
#include "dawn/native/DawnNative.h"

namespace dawn {

void NullDeviceBenchmarkFixture::SetUp(const benchmark::State& state) {
    // Static initialization that only happens on the first time that a fixture is created.
    static std::unique_ptr<dawn::native::Instance> nativeInstance = []() {
        dawnProcSetProcs(&dawn::native::GetProcs());
        return std::make_unique<dawn::native::Instance>();
    }();

    if (state.thread_index() == 0) {
        // Only thread 0 is responsible for initializing the device on each iteration.
        {
            std::lock_guard<std::mutex> lock(mMutex);

            // Get an adapter to create the device with.
            wgpu::RequestAdapterOptions options = {};
            options.backendType = wgpu::BackendType::Null;
            auto nativeAdapter = nativeInstance->EnumerateAdapters(&options)[0];
            adapter = wgpu::Adapter(nativeAdapter.Get());
            DAWN_ASSERT(adapter != nullptr);

            // Create the device.
            wgpu::DeviceDescriptor desc = GetDeviceDescriptor();
            desc.SetDeviceLostCallback(
                wgpu::CallbackMode::AllowSpontaneous,
                [](const wgpu::Device&, wgpu::DeviceLostReason reason, wgpu::StringView message) {
                    if (reason == wgpu::DeviceLostReason::Unknown) {
                        dawn::ErrorLog() << message;
                        DAWN_UNREACHABLE();
                    }
                });
            desc.SetUncapturedErrorCallback(
                [](const wgpu::Device&, wgpu::ErrorType, wgpu::StringView message) {
                    dawn::ErrorLog() << message;
                    DAWN_UNREACHABLE();
                });

            adapter.RequestDevice(
                &desc, wgpu::CallbackMode::AllowSpontaneous,
                [this](wgpu::RequestDeviceStatus status, wgpu::Device result, wgpu::StringView) {
                    DAWN_ASSERT(status == wgpu::RequestDeviceStatus::Success);
                    device = std::move(result);
                });
        }
        mNumDoneThreads = 0;
        mCv.notify_all();
    } else {
        // All other threads should wait to proceed once the device is ready.
        std::unique_lock lock(mMutex);
        mCv.wait(lock, [this] { return device != nullptr; });
    }
}

void NullDeviceBenchmarkFixture::TearDown(const benchmark::State& state) {
    if (state.thread_index() == 0) {
        std::unique_lock lock(mMutex);
        mCv.wait(lock, [this, state] { return mNumDoneThreads == state.threads() - 1; });
        device = nullptr;
    } else {
        bool isDone = false;
        {
            std::lock_guard<std::mutex> lock(mMutex);
            mNumDoneThreads += 1;
            isDone = mNumDoneThreads == state.threads() - 1;
        }
        if (isDone) {
            mCv.notify_one();
        }
    }
}

}  // namespace dawn
