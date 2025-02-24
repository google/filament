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

#ifndef SRC_DAWN_TESTS_MOCKCALLBACK_H_
#define SRC_DAWN_TESTS_MOCKCALLBACK_H_

#include <memory>
#include <set>
#include <tuple>
#include <utility>

#include "dawn/common/Assert.h"
#include "gmock/gmock.h"

namespace testing {

template <typename F>
class MockCallback;

// Helper class for mocking callbacks used for Dawn callbacks with |void* userdata|
// as the last callback argument.
//
// Example Usage:
//   MockCallback<WGPUDeviceLostCallback> mock;
//
//   void* foo = XYZ; // this is the callback userdata
//
//   wgpuDeviceSetDeviceLostCallback(device, mock.Callback(), mock.MakeUserdata(foo));
//   EXPECT_CALL(mock, Call(_, foo));
template <typename R, typename... Args>
class MockCallback<R (*)(Args...)> : public ::testing::MockFunction<R(Args...)> {
    using CallbackType = R (*)(Args...);

  public:
    // Helper function makes it easier to get the callback using |foo.Callback()|
    // unstead of MockCallback<CallbackType>::Callback.
    static CallbackType Callback() { return CallUnboundCallback; }

    void* MakeUserdata(void* userdata) {
        auto mockAndUserdata =
            std::unique_ptr<MockAndUserdata>(new MockAndUserdata{this, userdata});

        // Add the userdata to a set of userdata for this mock. We never
        // remove from this set even if a callback should only be called once so that
        // repeated calls to the callback still forward the userdata correctly.
        // Userdata will be destroyed when the mock is destroyed.
        auto [result, inserted] = mUserdatas.insert(std::move(mockAndUserdata));
        DAWN_ASSERT(inserted);
        return result->get();
    }

  private:
    struct MockAndUserdata {
        MockCallback* mock;
        void* userdata;
    };

    static R CallUnboundCallback(Args... args) {
        std::tuple<Args...> tuple = std::make_tuple(args...);

        constexpr size_t ArgC = sizeof...(Args);
        static_assert(ArgC >= 1, "Mock callback requires at least one argument (the userdata)");

        // Get the userdata. It should be the last argument.
        auto userdata = std::get<ArgC - 1>(tuple);
        static_assert(std::is_same<decltype(userdata), void*>::value,
                      "Last callback argument must be void* userdata");

        // Extract the mock.
        DAWN_ASSERT(userdata != nullptr);
        auto* mockAndUserdata = reinterpret_cast<MockAndUserdata*>(userdata);
        MockCallback* mock = mockAndUserdata->mock;
        DAWN_ASSERT(mock != nullptr);

        // Replace the userdata
        std::get<ArgC - 1>(tuple) = mockAndUserdata->userdata;

        // Forward the callback to the mock.
        return mock->CallImpl(std::make_index_sequence<ArgC>{}, std::move(tuple));
    }

    // This helper cannot be inlined because we dependent on the templated index sequence
    // to unpack the tuple arguments.
    template <size_t... Is>
    R CallImpl(const std::index_sequence<Is...>&, std::tuple<Args...> args) {
        return this->Call(std::get<Is>(args)...);
    }

    std::set<std::unique_ptr<MockAndUserdata>> mUserdatas;
};

template <typename F>
class MockCppCallback;

// Helper wrapper class for C++ entry point callbacks.
// Example Usage:
//   MockCppCallback<void (*)(wgpu::PopErrorScopeStatus, wgpu::ErrorType, wgpu::StringView)> mock;
//
//   device.PopErrorScope(wgpu::CallbackMode::AllowProcessEvents, mock.Callback());
//   EXPECT_CALL(mock, Call(wgpu::PopErrorScopeStatus::Success, _, _));
template <typename R, typename... Args>
class MockCppCallback<R (*)(Args...)> : public ::testing::MockFunction<R(Args...)> {
  private:
    using TemplatedCallbackT = MockCppCallback<R (*)(Args...)>;

  public:
    auto Callback() {
        return [this](Args... args) -> R { return this->Call(args...); };
    }

    // Returns the templated version of the callback with a static function. Useful when we cannot
    // use a binding lambda. This must be used with TemplatedCallbackUserdata.
    auto TemplatedCallback() {
        return [](Args... args, TemplatedCallbackT* self) -> R { return self->Call(args...); };
    }
    TemplatedCallbackT* TemplatedCallbackUserdata() { return this; }
};

}  // namespace testing

#endif  // SRC_DAWN_TESTS_MOCKCALLBACK_H_
