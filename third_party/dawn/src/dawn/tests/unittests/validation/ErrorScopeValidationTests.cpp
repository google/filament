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

#include <memory>
#include <vector>

#include "dawn/tests/MockCallback.h"
#include "dawn/tests/StringViewMatchers.h"
#include "dawn/tests/unittests/validation/ValidationTest.h"
#include "dawn/utils/BinarySemaphore.h"
#include "gmock/gmock.h"

namespace dawn {
namespace {

using testing::_;
using testing::EmptySizedString;
using testing::HasSubstr;
using testing::MockCppCallback;
using testing::NonEmptySizedString;
using testing::SizedStringMatches;
using utils::BinarySemaphore;

class ErrorScopeValidationTest : public ValidationTest {
  protected:
    void FlushWireAndProcessEvents() {
        FlushWire();
        instance.ProcessEvents();
    }

    MockCppCallback<void (*)(wgpu::PopErrorScopeStatus, wgpu::ErrorType, wgpu::StringView)>
        mPopErrorScopeCb;
};

// Test the simple success case.
TEST_F(ErrorScopeValidationTest, Success) {
    device.PushErrorScope(wgpu::ErrorFilter::Validation);

    EXPECT_CALL(mPopErrorScopeCb,
                Call(wgpu::PopErrorScopeStatus::Success, wgpu::ErrorType::NoError, _))
        .Times(1);
    device.PopErrorScope(wgpu::CallbackMode::AllowProcessEvents, mPopErrorScopeCb.Callback());
    FlushWireAndProcessEvents();
}

// Test the simple case where the error scope catches an error.
TEST_F(ErrorScopeValidationTest, CatchesError) {
    device.PushErrorScope(wgpu::ErrorFilter::Validation);

    wgpu::BufferDescriptor desc = {};
    desc.usage = static_cast<wgpu::BufferUsage>(UINT64_MAX);
    device.CreateBuffer(&desc);

    EXPECT_CALL(mPopErrorScopeCb,
                Call(wgpu::PopErrorScopeStatus::Success, wgpu::ErrorType::Validation, _))
        .Times(1);
    device.PopErrorScope(wgpu::CallbackMode::AllowProcessEvents, mPopErrorScopeCb.Callback());
    FlushWireAndProcessEvents();
}

// Test that errors bubble to the parent scope if not handled by the current scope.
TEST_F(ErrorScopeValidationTest, ErrorBubbles) {
    device.PushErrorScope(wgpu::ErrorFilter::Validation);
    device.PushErrorScope(wgpu::ErrorFilter::OutOfMemory);

    wgpu::BufferDescriptor desc = {};
    desc.usage = static_cast<wgpu::BufferUsage>(UINT64_MAX);
    device.CreateBuffer(&desc);

    // OutOfMemory does not match Validation error.
    EXPECT_CALL(mPopErrorScopeCb,
                Call(wgpu::PopErrorScopeStatus::Success, wgpu::ErrorType::NoError, _))
        .Times(1);
    device.PopErrorScope(wgpu::CallbackMode::AllowProcessEvents, mPopErrorScopeCb.Callback());
    FlushWireAndProcessEvents();

    // Parent validation error scope captures the error.
    EXPECT_CALL(mPopErrorScopeCb,
                Call(wgpu::PopErrorScopeStatus::Success, wgpu::ErrorType::Validation, _))
        .Times(1);
    device.PopErrorScope(wgpu::CallbackMode::AllowProcessEvents, mPopErrorScopeCb.Callback());
    FlushWireAndProcessEvents();
}

// Test that if an error scope matches an error, it does not bubble to the parent scope.
TEST_F(ErrorScopeValidationTest, HandledErrorsStopBubbling) {
    device.PushErrorScope(wgpu::ErrorFilter::OutOfMemory);
    device.PushErrorScope(wgpu::ErrorFilter::Validation);

    wgpu::BufferDescriptor desc = {};
    desc.usage = static_cast<wgpu::BufferUsage>(UINT64_MAX);
    device.CreateBuffer(&desc);

    // Inner scope catches the error.
    EXPECT_CALL(mPopErrorScopeCb,
                Call(wgpu::PopErrorScopeStatus::Success, wgpu::ErrorType::Validation, _))
        .Times(1);
    device.PopErrorScope(wgpu::CallbackMode::AllowProcessEvents, mPopErrorScopeCb.Callback());
    FlushWireAndProcessEvents();

    // Parent scope does not see the error.
    EXPECT_CALL(mPopErrorScopeCb,
                Call(wgpu::PopErrorScopeStatus::Success, wgpu::ErrorType::NoError, _))
        .Times(1);
    device.PopErrorScope(wgpu::CallbackMode::AllowProcessEvents, mPopErrorScopeCb.Callback());
    FlushWireAndProcessEvents();
}

// Test that if no error scope handles an error, it goes to the device UncapturedError callback
TEST_F(ErrorScopeValidationTest, UnhandledErrorsMatchUncapturedErrorCallback) {
    device.PushErrorScope(wgpu::ErrorFilter::OutOfMemory);

    wgpu::BufferDescriptor desc = {};
    desc.usage = static_cast<wgpu::BufferUsage>(UINT64_MAX);
    ASSERT_DEVICE_ERROR(device.CreateBuffer(&desc));

    EXPECT_CALL(mPopErrorScopeCb,
                Call(wgpu::PopErrorScopeStatus::Success, wgpu::ErrorType::NoError, _))
        .Times(1);
    device.PopErrorScope(wgpu::CallbackMode::AllowProcessEvents, mPopErrorScopeCb.Callback());
    FlushWireAndProcessEvents();
}

// Test that an error that happens in another thread is NOT caught.
TEST_F(ErrorScopeValidationTest, ThreadLocalError) {
    // Thread local error scope is only implemented on native.
    DAWN_SKIP_TEST_IF(UsesWire());

    // ThreadA pushes the scope, yields to ThreadB to cause an error, and then tries to pop the
    // scope.
    BinarySemaphore semA, semB;
    auto threadA = [&] {
        device.PushErrorScope(wgpu::ErrorFilter::Validation);
        semA.Release();
        semB.Acquire();
        EXPECT_CALL(mPopErrorScopeCb,
                    Call(wgpu::PopErrorScopeStatus::Success, wgpu::ErrorType::NoError, _))
            .Times(1);
        device.PopErrorScope(wgpu::CallbackMode::AllowSpontaneous, mPopErrorScopeCb.Callback());
    };
    auto threadB = [&] {
        semA.Acquire();
        wgpu::BufferDescriptor desc = {};
        desc.usage = static_cast<wgpu::BufferUsage>(UINT64_MAX);
        ASSERT_DEVICE_ERROR(device.CreateBuffer(&desc));
        semB.Release();
    };

    std::thread tA(threadA);
    std::thread tB(threadB);
    tA.join();
    tB.join();
}

// Test that an error that happens in a thread is only caught by that thread.
TEST_F(ErrorScopeValidationTest, ThreadLocalErrorOnlyOneThread) {
    // Thread local error scope is only implemented on native.
    DAWN_SKIP_TEST_IF(UsesWire());

    // ThreadA pushes the scope, yields to ThreadB to push scope, then error happens on ThreadA. We
    // first pop from ThreadB which should not see an error, then pop from ThreadA which should see
    // the error.
    BinarySemaphore semA, semB;
    auto threadA = [&] {
        device.PushErrorScope(wgpu::ErrorFilter::Validation);
        semA.Release();
        semB.Acquire();

        wgpu::BufferDescriptor desc = {};
        desc.usage = static_cast<wgpu::BufferUsage>(UINT64_MAX);
        device.CreateBuffer(&desc);
        semA.Release();
        semB.Acquire();

        EXPECT_CALL(mPopErrorScopeCb,
                    Call(wgpu::PopErrorScopeStatus::Success, wgpu::ErrorType::Validation, _))
            .Times(1);
        device.PopErrorScope(wgpu::CallbackMode::AllowSpontaneous, mPopErrorScopeCb.Callback());
    };
    auto threadB = [&] {
        semA.Acquire();

        device.PushErrorScope(wgpu::ErrorFilter::Validation);
        semB.Release();
        semA.Acquire();

        EXPECT_CALL(mPopErrorScopeCb,
                    Call(wgpu::PopErrorScopeStatus::Success, wgpu::ErrorType::NoError, _))
            .Times(1);
        device.PopErrorScope(wgpu::CallbackMode::AllowSpontaneous, mPopErrorScopeCb.Callback());
        semB.Release();
    };

    std::thread tA(threadA);
    std::thread tB(threadB);
    tA.join();
    tB.join();
}

// Check that push/popping error scopes must be balanced.
TEST_F(ErrorScopeValidationTest, PushPopBalanced) {
    // No error scopes to pop.
    {
        EXPECT_CALL(mPopErrorScopeCb,
                    Call(wgpu::PopErrorScopeStatus::Error, wgpu::ErrorType::NoError,
                         SizedStringMatches(HasSubstr("No error scopes"))))
            .Times(1);
        device.PopErrorScope(wgpu::CallbackMode::AllowProcessEvents, mPopErrorScopeCb.Callback());
        FlushWireAndProcessEvents();
    }
    // Too many pops
    {
        device.PushErrorScope(wgpu::ErrorFilter::Validation);

        EXPECT_CALL(mPopErrorScopeCb,
                    Call(wgpu::PopErrorScopeStatus::Success, wgpu::ErrorType::NoError, _))
            .Times(1);
        device.PopErrorScope(wgpu::CallbackMode::AllowProcessEvents, mPopErrorScopeCb.Callback());
        FlushWireAndProcessEvents();

        EXPECT_CALL(mPopErrorScopeCb,
                    Call(wgpu::PopErrorScopeStatus::Error, wgpu::ErrorType::NoError,
                         SizedStringMatches(HasSubstr("No error scopes"))))
            .Times(1);
        device.PopErrorScope(wgpu::CallbackMode::AllowProcessEvents, mPopErrorScopeCb.Callback());
        FlushWireAndProcessEvents();
    }
}

// Test that if the device is destroyed before the callback occurs, it is called with NoError.
TEST_F(ErrorScopeValidationTest, DeviceDestroyedBeforeCallback) {
    device.PushErrorScope(wgpu::ErrorFilter::OutOfMemory);
    {
        // Note: this is in its own scope to be clear the queue does not outlive the device.
        wgpu::Queue queue = device.GetQueue();
        queue.Submit(0, nullptr);
    }

    EXPECT_CALL(mPopErrorScopeCb,
                Call(wgpu::PopErrorScopeStatus::Success, wgpu::ErrorType::NoError, _))
        .Times(1);
    device.PopErrorScope(wgpu::CallbackMode::AllowProcessEvents, mPopErrorScopeCb.Callback());
    ExpectDeviceDestruction();
    device = nullptr;

    FlushWireAndProcessEvents();
}

// If the device is destroyed, pop error scope should callback with NoError.
TEST_F(ErrorScopeValidationTest, DeviceDestroyedBeforePop) {
    device.PushErrorScope(wgpu::ErrorFilter::Validation);
    ExpectDeviceDestruction();
    device.Destroy();
    FlushWireAndProcessEvents();

    EXPECT_CALL(mPopErrorScopeCb, Call(wgpu::PopErrorScopeStatus::Success, wgpu::ErrorType::NoError,
                                       EmptySizedString()))
        .Times(1);
    device.PopErrorScope(wgpu::CallbackMode::AllowProcessEvents, mPopErrorScopeCb.Callback());
    FlushWireAndProcessEvents();
}

// Regression test that on device shutdown, we don't get a recursion in O(pushed error scope) that
// would lead to a stack overflow
TEST_F(ErrorScopeValidationTest, ShutdownStackOverflow) {
    for (size_t i = 0; i < 1'000'000; i++) {
        device.PushErrorScope(wgpu::ErrorFilter::Validation);
    }
}

}  // anonymous namespace
}  // namespace dawn
