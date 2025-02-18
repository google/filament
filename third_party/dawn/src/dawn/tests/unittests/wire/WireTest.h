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

#ifndef SRC_DAWN_TESTS_UNITTESTS_WIRE_WIRETEST_H_
#define SRC_DAWN_TESTS_UNITTESTS_WIRE_WIRETEST_H_

#include <memory>

#include "dawn/common/Log.h"
#include "dawn/mock_webgpu.h"
#include "dawn/tests/MockCallback.h"
#include "gtest/gtest.h"

#include "webgpu/webgpu_cpp.h"

namespace dawn {

// Definition of a "Lambda predicate matcher" for GMock to allow checking deep structures
// are passed correctly by the wire.

// Helper templates to extract the argument type of a lambda.
template <typename T>
struct MatcherMethodArgument;

template <typename Lambda, typename Arg>
struct MatcherMethodArgument<bool (Lambda::*)(Arg) const> {
    using Type = Arg;
};

template <typename Lambda>
using MatcherLambdaArgument = typename MatcherMethodArgument<decltype(&Lambda::operator())>::Type;

// The matcher itself, unfortunately it isn't able to return detailed information like other
// matchers do.
template <typename Lambda, typename Arg>
class LambdaMatcherImpl : public testing::MatcherInterface<Arg> {
  public:
    explicit LambdaMatcherImpl(Lambda lambda) : mLambda(lambda) {}

    void DescribeTo(std::ostream* os) const override { *os << "with a custom matcher"; }

    bool MatchAndExplain(Arg value, testing::MatchResultListener* listener) const override {
        if (!mLambda(value)) {
            *listener << "which doesn't satisfy the custom predicate";
            return false;
        }
        return true;
    }

  private:
    Lambda mLambda;
};

// Use the MatchesLambda as follows:
//
//   EXPECT_CALL(foo, Bar(MatchesLambda([](ArgType arg) -> bool {
//       return CheckPredicateOnArg(arg);
//   })));
template <typename Lambda>
inline testing::Matcher<MatcherLambdaArgument<Lambda>> MatchesLambda(Lambda lambda) {
    return MakeMatcher(new LambdaMatcherImpl<Lambda, MatcherLambdaArgument<Lambda>>(lambda));
}

class StringMessageMatcher : public testing::MatcherInterface<const char*> {
  public:
    StringMessageMatcher() {}

    bool MatchAndExplain(const char* message,
                         testing::MatchResultListener* listener) const override {
        if (message == nullptr) {
            *listener << "missing error message";
            return false;
        }
        if (std::strlen(message) <= 1) {
            *listener << "message is truncated";
            return false;
        }
        return true;
    }

    void DescribeTo(std::ostream* os) const override { *os << "valid error message"; }

    void DescribeNegationTo(std::ostream* os) const override { *os << "invalid error message"; }
};

inline testing::Matcher<const char*> ValidStringMessage() {
    return MakeMatcher(new StringMessageMatcher());
}

// Matcher for C++ types to verify that their internal C-handles are identical.
MATCHER_P(CHandleIs, cType, "") {
    return arg.Get() == cType;
}

// Skip a test when the given condition is satisfied.
#define DAWN_SKIP_TEST_IF(condition)                            \
    do {                                                        \
        if (condition) {                                        \
            dawn::InfoLog() << "Test skipped: " #condition "."; \
            GTEST_SKIP();                                       \
            return;                                             \
        }                                                       \
    } while (0)

namespace wire {
class WireClient;
class WireServer;
namespace client {
class MemoryTransferService;
}  // namespace client
namespace server {
class MemoryTransferService;
}  // namespace server
}  // namespace wire

namespace utils {
class TerribleCommandBuffer;
}  // namespace utils

class WireTest : public testing::Test {
  protected:
    WireTest();
    ~WireTest() override;

    void SetUp() override;
    void TearDown() override;

    void FlushClient(bool success = true);
    void FlushServer(bool success = true);

    void DefaultApiDeviceWasReleased();
    void DefaultApiAdapterWasReleased();

    testing::StrictMock<MockProcTable> api;

    // Mock callbacks tracking errors and destruction. These are strict mocks because any errors or
    // device loss that aren't expected should result in test failures and not just some warnings
    // printed to stdout.
    testing::StrictMock<testing::MockCppCallback<wgpu::DeviceLostCallback<void>*>>
        deviceLostCallback;
    testing::StrictMock<testing::MockCppCallback<wgpu::UncapturedErrorCallback<void>*>>
        uncapturedErrorCallback;

    wgpu::Instance instance;
    WGPUInstance apiInstance;
    wgpu::Adapter adapter;
    WGPUAdapter apiAdapter;
    wgpu::Device device;
    WGPUDevice apiDevice;
    wgpu::Queue queue;
    WGPUQueue apiQueue;

    WGPUDevice cDevice;
    WGPUQueue cQueue;

    dawn::wire::WireServer* GetWireServer();
    dawn::wire::WireClient* GetWireClient();

    void DeleteServer();
    void DeleteClient();

  private:
    void SetupIgnoredCallExpectations();

    virtual dawn::wire::client::MemoryTransferService* GetClientMemoryTransferService();
    virtual dawn::wire::server::MemoryTransferService* GetServerMemoryTransferService();

    std::unique_ptr<dawn::wire::WireServer> mWireServer;
    std::unique_ptr<dawn::wire::WireClient> mWireClient;
    std::unique_ptr<dawn::utils::TerribleCommandBuffer> mS2cBuf;
    std::unique_ptr<dawn::utils::TerribleCommandBuffer> mC2sBuf;
};

}  // namespace dawn

#endif  // SRC_DAWN_TESTS_UNITTESTS_WIRE_WIRETEST_H_
