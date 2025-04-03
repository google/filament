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

#include <gmock/gmock.h>
#include <webgpu/webgpu.h>

#include <atomic>
#include <chrono>
#include <cstdint>
#include <utility>
#include <vector>

#include "dawn/common/FutureUtils.h"
#include "dawn/tests/DawnTest.h"

namespace dawn {
namespace {

using testing::AnyOf;
using testing::Eq;

wgpu::Device CreateExtraDevice(wgpu::Instance instance) {
    // IMPORTANT: DawnTest overrides RequestAdapter and RequestDevice and mixes
    // up the two instances. We use these to bypass the override.
    auto* requestAdapter = reinterpret_cast<WGPUProcInstanceRequestAdapter>(
        wgpu::GetProcAddress("wgpuInstanceRequestAdapter"));
    auto* requestDevice = reinterpret_cast<WGPUProcAdapterRequestDevice>(
        wgpu::GetProcAddress("wgpuAdapterRequestDevice"));

    wgpu::Adapter adapter2;
    requestAdapter(instance.Get(), nullptr,
                   {nullptr, WGPUCallbackMode_AllowSpontaneous,
                    [](WGPURequestAdapterStatus status, WGPUAdapter adapter, WGPUStringView, void*,
                       void* result) {
                        *reinterpret_cast<wgpu::Adapter*>(result) = wgpu::Adapter::Acquire(adapter);
                    },
                    nullptr, &adapter2});
    DAWN_ASSERT(adapter2);

    wgpu::Device device2;
    requestDevice(adapter2.Get(), nullptr,
                  {nullptr, WGPUCallbackMode_AllowSpontaneous,
                   [](WGPURequestDeviceStatus status, WGPUDevice device, WGPUStringView, void*,
                      void* result) {
                       ASSERT_EQ(status, WGPURequestDeviceStatus_Success);
                       *reinterpret_cast<wgpu::Device*>(result) = wgpu::Device::Acquire(device);
                   },
                   nullptr, &device2});
    DAWN_ASSERT(device2);

    return device2;
}

std::pair<wgpu::Instance, wgpu::Device> CreateExtraInstance(wgpu::InstanceDescriptor* desc) {
    wgpu::Instance instance2 = wgpu::CreateInstance(desc);

    wgpu::Device device2 = CreateExtraDevice(instance2);
    DAWN_ASSERT(device2);

    return std::pair(std::move(instance2), std::move(device2));
}

// EventCompletionTests

enum class WaitType {
    TimedWaitAny,
    SpinWaitAny,
    SpinProcessEvents,
};

enum class WaitTypeAndCallbackMode {
    TimedWaitAny_WaitAnyOnly,
    TimedWaitAny_AllowSpontaneous,
    SpinWaitAny_WaitAnyOnly,
    SpinWaitAny_AllowSpontaneous,
    SpinProcessEvents_AllowProcessEvents,
    SpinProcessEvents_AllowSpontaneous,
};

std::ostream& operator<<(std::ostream& o, WaitTypeAndCallbackMode waitMode) {
    switch (waitMode) {
        case WaitTypeAndCallbackMode::TimedWaitAny_WaitAnyOnly:
            return o << "TimedWaitAny_WaitAnyOnly";
        case WaitTypeAndCallbackMode::SpinWaitAny_WaitAnyOnly:
            return o << "SpinWaitAny_WaitAnyOnly";
        case WaitTypeAndCallbackMode::SpinProcessEvents_AllowProcessEvents:
            return o << "SpinProcessEvents_AllowProcessEvents";
        case WaitTypeAndCallbackMode::TimedWaitAny_AllowSpontaneous:
            return o << "TimedWaitAny_AllowSpontaneous";
        case WaitTypeAndCallbackMode::SpinWaitAny_AllowSpontaneous:
            return o << "SpinWaitAny_AllowSpontaneous";
        case WaitTypeAndCallbackMode::SpinProcessEvents_AllowSpontaneous:
            return o << "SpinProcessEvents_AllowSpontaneous";
    }
}

DAWN_TEST_PARAM_STRUCT(EventCompletionTestParams, WaitTypeAndCallbackMode);

class EventCompletionTests : public DawnTestWithParams<EventCompletionTestParams> {
  protected:
    wgpu::Instance testInstance;
    wgpu::Device testDevice;
    wgpu::Queue testQueue;
    std::vector<wgpu::FutureWaitInfo> mFutures;
    std::atomic<uint64_t> mCallbacksCompletedCount = 0;
    uint64_t mCallbacksIssuedCount = 0;
    uint64_t mCallbacksWaitedCount = 0;

    void SetUp() override {
        DawnTestWithParams::SetUp();
        WaitTypeAndCallbackMode mode = GetParam().mWaitTypeAndCallbackMode;
        if (UsesWire()) {
            DAWN_TEST_UNSUPPORTED_IF(mode == WaitTypeAndCallbackMode::TimedWaitAny_WaitAnyOnly ||
                                     mode ==
                                         WaitTypeAndCallbackMode::TimedWaitAny_AllowSpontaneous);
        }
        testInstance = GetInstance();
        testDevice = device;
        testQueue = queue;
        // Make sure these aren't used accidentally (unfortunately can't do the same for instance):
        device = nullptr;
        queue = nullptr;
    }

    void UseSecondInstance() {
        wgpu::InstanceDescriptor desc;
        desc.capabilities.timedWaitAnyEnable = !UsesWire();
        std::tie(testInstance, testDevice) = CreateExtraInstance(&desc);
        testQueue = testDevice.GetQueue();
    }

    void LoseTestDevice() {
        EXPECT_CALL(mDeviceLostCallback,
                    Call(CHandleIs(testDevice.Get()), wgpu::DeviceLostReason::Unknown, testing::_))
            .Times(1);
        testDevice.ForceLoss(wgpu::DeviceLostReason::Unknown, "Device lost for testing");
        testInstance.ProcessEvents();
    }

    void TrivialSubmit() {
        wgpu::CommandBuffer cb = testDevice.CreateCommandEncoder().Finish();
        testQueue.Submit(1, &cb);
    }

    wgpu::CallbackMode GetCallbackMode() {
        switch (GetParam().mWaitTypeAndCallbackMode) {
            case WaitTypeAndCallbackMode::TimedWaitAny_WaitAnyOnly:
            case WaitTypeAndCallbackMode::SpinWaitAny_WaitAnyOnly:
                return wgpu::CallbackMode::WaitAnyOnly;
            case WaitTypeAndCallbackMode::SpinProcessEvents_AllowProcessEvents:
                return wgpu::CallbackMode::AllowProcessEvents;
            case WaitTypeAndCallbackMode::TimedWaitAny_AllowSpontaneous:
            case WaitTypeAndCallbackMode::SpinWaitAny_AllowSpontaneous:
            case WaitTypeAndCallbackMode::SpinProcessEvents_AllowSpontaneous:
                return wgpu::CallbackMode::AllowSpontaneous;
        }
    }

    bool IsSpontaneous() { return GetCallbackMode() == wgpu::CallbackMode::AllowSpontaneous; }

    void TrackForTest(wgpu::Future future) {
        mCallbacksIssuedCount++;

        switch (GetParam().mWaitTypeAndCallbackMode) {
            case WaitTypeAndCallbackMode::TimedWaitAny_WaitAnyOnly:
            case WaitTypeAndCallbackMode::TimedWaitAny_AllowSpontaneous:
            case WaitTypeAndCallbackMode::SpinWaitAny_WaitAnyOnly:
            case WaitTypeAndCallbackMode::SpinWaitAny_AllowSpontaneous:
                mFutures.push_back(wgpu::FutureWaitInfo{future, false});
                break;
            case WaitTypeAndCallbackMode::SpinProcessEvents_AllowProcessEvents:
            case WaitTypeAndCallbackMode::SpinProcessEvents_AllowSpontaneous:
                break;
        }
    }

    wgpu::Future OnSubmittedWorkDone(wgpu::QueueWorkDoneStatus expectedStatus) {
        return testQueue.OnSubmittedWorkDone(
            GetCallbackMode(), [this, expectedStatus](wgpu::QueueWorkDoneStatus status) {
                mCallbacksCompletedCount++;
                ASSERT_EQ(status, expectedStatus);
            });
    }

    void TestWaitAll(bool loopOnlyOnce = false) {
        switch (GetParam().mWaitTypeAndCallbackMode) {
            case WaitTypeAndCallbackMode::TimedWaitAny_WaitAnyOnly:
            case WaitTypeAndCallbackMode::TimedWaitAny_AllowSpontaneous:
                return TestWaitImpl(WaitType::TimedWaitAny, loopOnlyOnce);
            case WaitTypeAndCallbackMode::SpinWaitAny_WaitAnyOnly:
            case WaitTypeAndCallbackMode::SpinWaitAny_AllowSpontaneous:
                return TestWaitImpl(WaitType::SpinWaitAny, loopOnlyOnce);
            case WaitTypeAndCallbackMode::SpinProcessEvents_AllowProcessEvents:
            case WaitTypeAndCallbackMode::SpinProcessEvents_AllowSpontaneous:
                return TestWaitImpl(WaitType::SpinProcessEvents, loopOnlyOnce);
        }
    }

    void TestWaitIncorrectly() {
        switch (GetParam().mWaitTypeAndCallbackMode) {
            case WaitTypeAndCallbackMode::TimedWaitAny_WaitAnyOnly:
            case WaitTypeAndCallbackMode::TimedWaitAny_AllowSpontaneous:
            case WaitTypeAndCallbackMode::SpinWaitAny_WaitAnyOnly:
            case WaitTypeAndCallbackMode::SpinWaitAny_AllowSpontaneous:
                return TestWaitImpl(WaitType::SpinProcessEvents);
            case WaitTypeAndCallbackMode::SpinProcessEvents_AllowProcessEvents:
            case WaitTypeAndCallbackMode::SpinProcessEvents_AllowSpontaneous:
                return TestWaitImpl(WaitType::SpinWaitAny);
        }
    }

  private:
    void TestWaitImpl(WaitType waitType, bool loopOnlyOnce = false) {
        uint64_t oldCompletedCount = mCallbacksCompletedCount;

        const auto start = std::chrono::high_resolution_clock::now();
        auto testTimeExceeded = [=]() -> bool {
            return std::chrono::high_resolution_clock::now() - start > std::chrono::seconds(5);
        };

        switch (waitType) {
            case WaitType::TimedWaitAny: {
                bool emptyWait = mFutures.size() == 0;
                // Loop at least once so we can test it with 0 futures.
                do {
                    ASSERT_FALSE(testTimeExceeded());
                    DAWN_ASSERT(!UsesWire());
                    wgpu::WaitStatus status;

                    uint64_t oldCompletionCount = mCallbacksCompletedCount;
                    // Any futures should succeed within a few milliseconds at most.
                    status = testInstance.WaitAny(mFutures.size(), mFutures.data(), UINT64_MAX);
                    ASSERT_EQ(status, wgpu::WaitStatus::Success);
                    bool mayHaveCompletedEarly = IsSpontaneous();
                    if (!mayHaveCompletedEarly && !emptyWait) {
                        ASSERT_GT(mCallbacksCompletedCount, oldCompletionCount);
                    }

                    // Verify this succeeds instantly because some futures completed already.
                    status = testInstance.WaitAny(mFutures.size(), mFutures.data(), 0);
                    ASSERT_EQ(status, wgpu::WaitStatus::Success);

                    RemoveCompletedFutures();
                    if (loopOnlyOnce) {
                        break;
                    }
                } while (mFutures.size() > 0);
            } break;
            case WaitType::SpinWaitAny: {
                bool emptyWait = mFutures.size() == 0;
                // Loop at least once so we can test it with 0 futures.
                do {
                    ASSERT_FALSE(testTimeExceeded());

                    uint64_t oldCompletionCount = mCallbacksCompletedCount;
                    FlushWire();
                    auto status = testInstance.WaitAny(mFutures.size(), mFutures.data(), 0);
                    if (status == wgpu::WaitStatus::TimedOut) {
                        continue;
                    }
                    ASSERT_TRUE(status == wgpu::WaitStatus::Success);
                    bool mayHaveCompletedEarly = IsSpontaneous();
                    if (!mayHaveCompletedEarly && !emptyWait) {
                        ASSERT_GT(mCallbacksCompletedCount, oldCompletionCount);
                    }

                    RemoveCompletedFutures();
                    if (loopOnlyOnce) {
                        break;
                    }
                } while (mFutures.size() > 0);
            } break;
            case WaitType::SpinProcessEvents: {
                do {
                    ASSERT_FALSE(testTimeExceeded());

                    FlushWire();
                    testInstance.ProcessEvents();

                    if (loopOnlyOnce) {
                        break;
                    }
                } while (mCallbacksCompletedCount < mCallbacksIssuedCount);
            } break;
        }

        if (!IsSpontaneous()) {
            ASSERT_EQ(mCallbacksCompletedCount - oldCompletedCount,
                      mCallbacksIssuedCount - mCallbacksWaitedCount);
        }
        ASSERT_EQ(mCallbacksCompletedCount, mCallbacksIssuedCount);
        mCallbacksWaitedCount = mCallbacksCompletedCount;
    }

    void RemoveCompletedFutures() {
        size_t oldSize = mFutures.size();
        if (oldSize > 0) {
            mFutures.erase(
                std::remove_if(mFutures.begin(), mFutures.end(),
                               [](const wgpu::FutureWaitInfo& info) { return info.completed; }),
                mFutures.end());
            ASSERT_LT(mFutures.size(), oldSize);
        }
    }
};

// Wait when no events have been requested.
TEST_P(EventCompletionTests, NoEvents) {
    TestWaitAll();
}

// WorkDone event after submitting some trivial work.
TEST_P(EventCompletionTests, WorkDoneSimple) {
    TrivialSubmit();
    TrackForTest(OnSubmittedWorkDone(wgpu::QueueWorkDoneStatus::Success));
    TestWaitAll();
}

// WorkDone event before device loss, wait afterward.
TEST_P(EventCompletionTests, WorkDoneAcrossDeviceLoss) {
    TrivialSubmit();
    TrackForTest(OnSubmittedWorkDone(wgpu::QueueWorkDoneStatus::Success));
    TestWaitAll();
}

// WorkDone event after device loss.
TEST_P(EventCompletionTests, WorkDoneAfterDeviceLoss) {
    TrivialSubmit();
    LoseTestDevice();
    TrackForTest(OnSubmittedWorkDone(wgpu::QueueWorkDoneStatus::Success));
    TestWaitAll();
}

// WorkDone event twice after submitting some trivial work.
TEST_P(EventCompletionTests, WorkDoneTwice) {
    TrivialSubmit();
    TrackForTest(OnSubmittedWorkDone(wgpu::QueueWorkDoneStatus::Success));
    TrackForTest(OnSubmittedWorkDone(wgpu::QueueWorkDoneStatus::Success));
    TestWaitAll();
}

// WorkDone event without ever having submitted any work.
TEST_P(EventCompletionTests, WorkDoneNoWork) {
    TrackForTest(OnSubmittedWorkDone(wgpu::QueueWorkDoneStatus::Success));
    TestWaitAll();
    TrackForTest(OnSubmittedWorkDone(wgpu::QueueWorkDoneStatus::Success));
    TrackForTest(OnSubmittedWorkDone(wgpu::QueueWorkDoneStatus::Success));
    TestWaitAll();
}

// WorkDone event after all work has completed already.
TEST_P(EventCompletionTests, WorkDoneAlreadyCompleted) {
    TrivialSubmit();
    TrackForTest(OnSubmittedWorkDone(wgpu::QueueWorkDoneStatus::Success));
    TestWaitAll();
    TrackForTest(OnSubmittedWorkDone(wgpu::QueueWorkDoneStatus::Success));
    TestWaitAll();
}

// WorkDone events waited in reverse order.
TEST_P(EventCompletionTests, WorkDoneOutOfOrder) {
    // With ProcessEvents or Spontaneous we can't control the order of completion.
    DAWN_TEST_UNSUPPORTED_IF(GetCallbackMode() != wgpu::CallbackMode::WaitAnyOnly);

    TrivialSubmit();
    wgpu::Future f1 = OnSubmittedWorkDone(wgpu::QueueWorkDoneStatus::Success);
    TrivialSubmit();
    wgpu::Future f2 = OnSubmittedWorkDone(wgpu::QueueWorkDoneStatus::Success);

    // When using WaitAny, normally callback ordering guarantees would guarantee f1 completes before
    // f2. But if we wait on f2 first, then f2 is allowed to complete first because f1 still hasn't
    // had an opportunity to complete.
    TrackForTest(f2);
    TestWaitAll();
    TrackForTest(f1);
    TestWaitAll(/*loopOnlyOnce=*/true);
}

constexpr wgpu::QueueWorkDoneStatus kStatusUninitialized =
    static_cast<wgpu::QueueWorkDoneStatus>(INT32_MAX);

TEST_P(EventCompletionTests, WorkDoneDropInstanceBeforeEvent) {
    // TODO(crbug.com/dawn/1987): Wire does not implement instance destruction correctly yet.
    DAWN_TEST_UNSUPPORTED_IF(UsesWire());

    UseSecondInstance();
    testInstance = nullptr;  // Drop the last external ref to the instance.

    wgpu::QueueWorkDoneStatus status = kStatusUninitialized;
    testQueue.OnSubmittedWorkDone(GetCallbackMode(),
                                  [&status](wgpu::QueueWorkDoneStatus result) { status = result; });

    if (IsSpontaneous()) {
        // TODO(crbug.com/dawn/2059): Once Spontaneous is implemented, this should no longer expect
        // the callback to be cleaned up immediately (and should expect it to happen on a future
        // Tick).
        ASSERT_THAT(status, AnyOf(Eq(wgpu::QueueWorkDoneStatus::Success),
                                  Eq(wgpu::QueueWorkDoneStatus::CallbackCancelled)));
    } else {
        ASSERT_EQ(status, wgpu::QueueWorkDoneStatus::CallbackCancelled);
    }
}

TEST_P(EventCompletionTests, WorkDoneDropInstanceAfterEvent) {
    // TODO(crbug.com/dawn/1987): Wire does not implement instance destruction correctly yet.
    DAWN_TEST_UNSUPPORTED_IF(UsesWire());

    UseSecondInstance();

    wgpu::QueueWorkDoneStatus status = kStatusUninitialized;
    testQueue.OnSubmittedWorkDone(GetCallbackMode(),
                                  [&status](wgpu::QueueWorkDoneStatus result) { status = result; });

    if (IsSpontaneous()) {
        testInstance = nullptr;  // Drop the last external ref to the instance.

        // TODO(crbug.com/dawn/2059): Once Spontaneous is implemented, this should no longer expect
        // the callback to be cleaned up immediately (and should expect it to happen on a future
        // Tick).
        ASSERT_THAT(status, AnyOf(Eq(wgpu::QueueWorkDoneStatus::Success),
                                  Eq(wgpu::QueueWorkDoneStatus::CallbackCancelled)));
    } else {
        ASSERT_EQ(status, kStatusUninitialized);
        testInstance = nullptr;  // Drop the last external ref to the instance.
        ASSERT_EQ(status, wgpu::QueueWorkDoneStatus::CallbackCancelled);
    }
}

// TODO(crbug.com/dawn/1987):
// - Test any reentrancy guarantees (for ProcessEvents or WaitAny inside a callback),
//   to make sure things don't blow up and we don't attempt to hold locks recursively.
// - Other tests?

DAWN_INSTANTIATE_TEST_P(EventCompletionTests,
                        {D3D11Backend(), D3D11Backend({"d3d11_use_unmonitored_fence"}),
                         D3D11Backend({"d3d11_disable_fence"}), D3D12Backend(), MetalBackend(),
                         VulkanBackend(), OpenGLBackend(), OpenGLESBackend()},
                        {
                            WaitTypeAndCallbackMode::TimedWaitAny_WaitAnyOnly,
                            WaitTypeAndCallbackMode::TimedWaitAny_AllowSpontaneous,
                            WaitTypeAndCallbackMode::SpinWaitAny_WaitAnyOnly,
                            WaitTypeAndCallbackMode::SpinWaitAny_AllowSpontaneous,
                            WaitTypeAndCallbackMode::SpinProcessEvents_AllowProcessEvents,
                            WaitTypeAndCallbackMode::SpinProcessEvents_AllowSpontaneous,

                            // TODO(crbug.com/dawn/2059): The cases with the Spontaneous flag
                            // enabled were added before we implemented all of the spontaneous
                            // completions. They might accidentally be overly strict.

                            // TODO(crbug.com/dawn/2059): Make guarantees that Spontaneous callbacks
                            // get called (as long as you're hitting "checkpoints"), and add the
                            // corresponding tests, for example:
                            // - SpinProcessEvents_Spontaneous,
                            // - SpinSubmit_Spontaneous,
                            // - SpinCheckpoint_Spontaneous (if wgpuDeviceCheckpoint is added).
                            // - (Note we don't want to guarantee Tick will process events - we
                            //   could even test that it doesn't, if we make that true.)
                        });

// WaitAnyTests

class WaitAnyTests : public DawnTest {};

TEST_P(WaitAnyTests, UnsupportedTimeout) {
    wgpu::Instance instance2;
    wgpu::Device device2;

    if (UsesWire()) {
        // The wire (currently) never supports timedWaitAnyEnable, so we can run this test on the
        // default instance/device.
        instance2 = GetInstance();
        device2 = device;
    } else {
        // When not using the wire, DawnTest will unconditionally set timedWaitAnyEnable since it's
        // useful for other tests. For this test, we need it to be false to test validation.
        wgpu::InstanceDescriptor desc;
        desc.capabilities.timedWaitAnyEnable = false;
        std::tie(instance2, device2) = CreateExtraInstance(&desc);
    }

    // UnsupportedTimeout is still validated if no futures are passed.
    for (uint64_t timeout : {uint64_t(1), uint64_t(0), UINT64_MAX}) {
        ASSERT_EQ(instance2.WaitAny(0, nullptr, timeout),
                  timeout > 0 ? wgpu::WaitStatus::Error : wgpu::WaitStatus::Success);
    }

    for (uint64_t timeout : {uint64_t(1), uint64_t(0), UINT64_MAX}) {
        wgpu::WaitStatus status = instance2.WaitAny(
            device2.GetQueue().OnSubmittedWorkDone(wgpu::CallbackMode::WaitAnyOnly,
                                                   [](wgpu::QueueWorkDoneStatus) {}),
            timeout);
        if (timeout == 0) {
            ASSERT_TRUE(status == wgpu::WaitStatus::Success ||
                        status == wgpu::WaitStatus::TimedOut);
        } else {
            ASSERT_EQ(status, wgpu::WaitStatus::Error);
        }
    }
}

TEST_P(WaitAnyTests, UnsupportedCount) {
    wgpu::Instance instance2;
    wgpu::Device device2;
    wgpu::Queue queue2;

    if (UsesWire()) {
        // The wire (currently) never supports timedWaitAnyEnable, so we can run this test on the
        // default instance/device.
        instance2 = GetInstance();
        device2 = device;
        queue2 = queue;
    } else {
        wgpu::InstanceDescriptor desc;
        desc.capabilities.timedWaitAnyEnable = true;
        std::tie(instance2, device2) = CreateExtraInstance(&desc);
        queue2 = device2.GetQueue();
    }

    for (uint64_t timeout : {uint64_t(0), uint64_t(1)}) {
        // We don't support values higher than the default (64), and if you ask for lower than 64
        // you still get 64. DawnTest doesn't request anything (so requests 0) so gets 64.
        for (size_t count : {kTimedWaitAnyMaxCountDefault, kTimedWaitAnyMaxCountDefault + 1}) {
            std::vector<wgpu::FutureWaitInfo> infos;
            for (size_t i = 0; i < count; ++i) {
                infos.push_back({queue2.OnSubmittedWorkDone(wgpu::CallbackMode::WaitAnyOnly,
                                                            [](wgpu::QueueWorkDoneStatus) {})});
            }
            wgpu::WaitStatus status = instance2.WaitAny(infos.size(), infos.data(), timeout);
            if (timeout == 0) {
                ASSERT_TRUE(status == wgpu::WaitStatus::Success ||
                            status == wgpu::WaitStatus::TimedOut);
            } else if (UsesWire()) {
                // Wire doesn't support timeouts at all.
                ASSERT_EQ(status, wgpu::WaitStatus::Error);
            } else if (count <= 64) {
                ASSERT_EQ(status, wgpu::WaitStatus::Success);
            } else {
                ASSERT_EQ(status, wgpu::WaitStatus::Error);
            }
        }
    }
}

TEST_P(WaitAnyTests, UnsupportedMixedSources) {
    wgpu::Instance instance2;
    wgpu::Device device2;
    wgpu::Queue queue2;
    wgpu::Device device3;
    wgpu::Queue queue3;

    if (UsesWire()) {
        // The wire (currently) never supports timedWaitAnyEnable, so we can run this test on the
        // default instance/device.
        instance2 = GetInstance();
        device2 = device;
        queue2 = queue;
        device3 = CreateDevice();
        queue3 = device3.GetQueue();
    } else {
        wgpu::InstanceDescriptor desc;
        desc.capabilities.timedWaitAnyEnable = true;
        std::tie(instance2, device2) = CreateExtraInstance(&desc);
        queue2 = device2.GetQueue();
        device3 = CreateExtraDevice(instance2);
        queue3 = device3.GetQueue();
    }

    for (uint64_t timeout : {uint64_t(0), uint64_t(1)}) {
        std::vector<wgpu::FutureWaitInfo> infos{{
            {queue2.OnSubmittedWorkDone(wgpu::CallbackMode::WaitAnyOnly,
                                        [](wgpu::QueueWorkDoneStatus) {})},
            {queue3.OnSubmittedWorkDone(wgpu::CallbackMode::WaitAnyOnly,
                                        [](wgpu::QueueWorkDoneStatus) {})},
        }};
        wgpu::WaitStatus status = instance2.WaitAny(infos.size(), infos.data(), timeout);
        if (timeout == 0) {
            ASSERT_TRUE(status == wgpu::WaitStatus::Success ||
                        status == wgpu::WaitStatus::TimedOut);
        } else if (UsesWire()) {
            // Wire doesn't support timeouts at all.
            ASSERT_EQ(status, wgpu::WaitStatus::Error);
        } else {
            ASSERT_EQ(status, wgpu::WaitStatus::Error);
        }
    }
}

DAWN_INSTANTIATE_TEST(WaitAnyTests,
                      D3D11Backend(),
                      D3D11Backend({"d3d11_use_unmonitored_fence"}),
                      D3D11Backend({"d3d11_disable_fence"}),
                      D3D12Backend(),
                      MetalBackend(),
                      VulkanBackend(),
                      OpenGLBackend(),
                      OpenGLESBackend());

class FutureTests : public DawnTest {};

// Regression test for crbug.com/dawn/2460 where when we have mixed source futures in a process
// events call we were crashing.
TEST_P(FutureTests, MixedSourcePolling) {
    // OnSubmittedWorkDone is implemented via a queue serial.
    device.GetQueue().OnSubmittedWorkDone(wgpu::CallbackMode::AllowProcessEvents,
                                          [](wgpu::QueueWorkDoneStatus) {});

    // PopErrorScope is implemented via a signal.
    device.PushErrorScope(wgpu::ErrorFilter::Validation);
    device.PopErrorScope(wgpu::CallbackMode::AllowProcessEvents,
                         [](wgpu::PopErrorScopeStatus, wgpu::ErrorType, wgpu::StringView) {});

    instance.ProcessEvents();
}

DAWN_INSTANTIATE_TEST(FutureTests,
                      D3D11Backend(),
                      D3D11Backend({"d3d11_use_unmonitored_fence"}),
                      D3D11Backend({"d3d11_disable_fence"}),
                      D3D12Backend(),
                      MetalBackend(),
                      VulkanBackend(),
                      OpenGLBackend(),
                      OpenGLESBackend());

}  // anonymous namespace
}  // namespace dawn
