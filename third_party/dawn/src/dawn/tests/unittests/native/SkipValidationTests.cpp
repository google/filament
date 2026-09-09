// Copyright 2026 The Dawn & Tint Authors
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

#include <functional>
#include <utility>

#include "src/dawn/tests/StringViewMatchers.h"
#include "src/dawn/tests/unittests/native/mocks/BufferMock.h"
#include "src/dawn/tests/unittests/native/mocks/DawnMockTest.h"

namespace dawn::native {
namespace {

using ::testing::_;
using ::testing::AnyNumber;
using ::testing::ByMove;
using ::testing::NiceMock;
using ::testing::Return;

static constexpr char kErrorMessage[] = "error";

class SkipValidationTests : public DawnMockTest {
  protected:
    void SetUp() override {
        mDeviceToggles.SetForTesting(Toggle::SkipValidation, true, true);
        DawnMockTest::SetUp();
    }

    // Verifies that validation is disabled initially, creates an invalid buffer (no usage bits)
    // that is accepted because validation is skipped, injects an error via the lambda, then
    // checks that validation is re-enabled and the same invalid descriptor is rejected before
    // reaching CreateBufferImpl.
    void TestValidationReenabledAfter(std::function<void()> injectError);
};

void SkipValidationTests::TestValidationReenabledAfter(std::function<void()> injectError) {
    EXPECT_FALSE(mDeviceMock->IsValidationEnabled());

    // A buffer descriptor with no usage bits set is invalid per the WebGPU spec.
    wgpu::BufferDescriptor invalidDesc = {};
    invalidDesc.size = 4;
    invalidDesc.usage = wgpu::BufferUsage::None;

    // With validation skipped, CreateBufferImpl is reached despite the invalid descriptor.
    BufferDescriptor nativeDesc = {};
    nativeDesc.size = invalidDesc.size;
    nativeDesc.usage = static_cast<wgpu::BufferUsage>(invalidDesc.usage);
    Ref<BufferMock> bufferMock = AcquireRef(new NiceMock<BufferMock>(mDeviceMock, &nativeDesc));
    EXPECT_CALL(*mDeviceMock, CreateBufferImpl).WillOnce(Return(ByMove(std::move(bufferMock))));

    wgpu::Buffer buffer = device.CreateBuffer(&invalidDesc);
    EXPECT_NE(buffer, nullptr);

    // Allow any error or device-lost callbacks triggered by the injected error, then inject it.
    EXPECT_CALL(mDeviceErrorCallback, Call(_, _, _)).Times(AnyNumber());
    EXPECT_CALL(mDeviceLostCallback, Call(_, _, _)).Times(AnyNumber());
    injectError();

    EXPECT_TRUE(mDeviceMock->IsValidationEnabled());

    // Now the same invalid descriptor triggers a validation error without reaching
    // CreateBufferImpl.
    EXPECT_CALL(*mDeviceMock, CreateBufferImpl).Times(0);
    device.CreateBuffer(&invalidDesc);
}

TEST_F(SkipValidationTests, ValidationReenabledAfterOOM) {
    TestValidationReenabledAfter(
        [&]() { device.InjectError(wgpu::ErrorType::OutOfMemory, kErrorMessage); });
}

TEST_F(SkipValidationTests, ValidationReenabledAfterDeviceLoss) {
    TestValidationReenabledAfter(
        [&]() { device.ForceLoss(wgpu::DeviceLostReason::Unknown, kErrorMessage); });
}

}  // namespace
}  // namespace dawn::native
