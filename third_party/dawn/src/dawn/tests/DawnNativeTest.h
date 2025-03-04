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

#ifndef SRC_DAWN_TESTS_DAWNNATIVETEST_H_
#define SRC_DAWN_TESTS_DAWNNATIVETEST_H_

#include <gtest/gtest.h>
#include <webgpu/webgpu_cpp.h>

#include <memory>

#include "dawn/native/DawnNative.h"
#include "dawn/native/ErrorData.h"

namespace dawn::native {

// This is similar to DAWN_TRY_ASSIGN but produces a fatal GTest error if EXPR is an error.
#define DAWN_ASSERT_AND_ASSIGN(VAR, EXPR) \
    DAWN_TRY_ASSIGN_WITH_CLEANUP(VAR, EXPR, {}, AddFatalDawnFailure(#EXPR, error.get()))

void AddFatalDawnFailure(const char* expression, const ErrorData* error);

}  // namespace dawn::native

class DawnNativeTest : public ::testing::Test {
  public:
    DawnNativeTest();
    ~DawnNativeTest() override;

    void SetUp() override;

    virtual std::unique_ptr<dawn::platform::Platform> CreateTestPlatform();
    WGPUDevice CreateTestDevice();

  protected:
    std::unique_ptr<dawn::platform::Platform> platform;
    std::unique_ptr<dawn::native::Instance> instance;
    dawn::native::Adapter adapter;
    wgpu::Device device;

  private:
    static void OnDeviceError(WGPUErrorType type, const char* message, void* userdata);
};

#endif  // SRC_DAWN_TESTS_DAWNNATIVETEST_H_
