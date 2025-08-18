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

#include "dawn/tests/DawnNativeTest.h"

#include <vector>

#include "absl/strings/str_cat.h"
#include "dawn/common/Assert.h"
#include "dawn/dawn_proc.h"
#include "dawn/native/ErrorData.h"
#include "dawn/native/Instance.h"
#include "dawn/native/dawn_platform.h"
#include "dawn/platform/DawnPlatform.h"
#include "dawn/webgpu_cpp_print.h"

namespace dawn::native {

void AddFatalDawnFailure(const char* expression, const ErrorData* error) {
    const auto& backtrace = error->GetBacktrace();
    GTEST_MESSAGE_AT_(backtrace.at(0).file, backtrace.at(0).line,
                      absl::StrCat(expression, " returned error: ", error->GetMessage()).c_str(),
                      ::testing::TestPartResult::kFatalFailure);
}

}  // namespace dawn::native

DawnNativeTest::DawnNativeTest() {
    dawnProcSetProcs(&dawn::native::GetProcs());
}

DawnNativeTest::~DawnNativeTest() {
    device = wgpu::Device();
    dawnProcSetProcs(nullptr);
}

void DawnNativeTest::SetUp() {
    // Create an instance with toggle AllowUnsafeAPIs enabled, which would be inherited to
    // adapter and device toggles and allow us to test unsafe apis (including experimental
    // features).
    const char* allowUnsafeApisToggle = "allow_unsafe_apis";
    wgpu::DawnTogglesDescriptor instanceToggles;
    instanceToggles.enabledToggleCount = 1;
    instanceToggles.enabledToggles = &allowUnsafeApisToggle;

    platform = CreateTestPlatform();
    dawn::native::DawnInstanceDescriptor dawnInstanceDesc;
    dawnInstanceDesc.platform = platform.get();
    dawnInstanceDesc.nextInChain = &instanceToggles;

    wgpu::InstanceDescriptor instanceDesc;
    instanceDesc.nextInChain = &dawnInstanceDesc;
    instance = std::make_unique<dawn::native::Instance>(
        reinterpret_cast<const WGPUInstanceDescriptor*>(&instanceDesc));

    wgpu::RequestAdapterOptions options = {};
    options.backendType = wgpu::BackendType::Null;

    adapter = instance->EnumerateAdapters(&options)[0];
    device = wgpu::Device::Acquire(CreateTestDevice());
}

std::unique_ptr<dawn::platform::Platform> DawnNativeTest::CreateTestPlatform() {
    return nullptr;
}

wgpu::DawnTogglesDescriptor DawnNativeTest::DeviceToggles() {
    return wgpu::DawnTogglesDescriptor{};
}

WGPUDevice DawnNativeTest::CreateTestDevice() {
    wgpu::DeviceDescriptor desc = {};
    desc.SetUncapturedErrorCallback(
        [](const wgpu::Device&, wgpu::ErrorType type, wgpu::StringView message) {
            DAWN_ASSERT(type != wgpu::ErrorType::NoError);
            FAIL() << "Unexpected error: " << message;
        });

    wgpu::DawnTogglesDescriptor toggles = DeviceToggles();
    desc.nextInChain = &toggles;

    return adapter.CreateDevice(&desc);
}
