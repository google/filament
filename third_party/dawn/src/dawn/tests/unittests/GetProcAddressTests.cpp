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
#include <string>
#include <vector>

#include "gtest/gtest.h"

#include "dawn/dawn_proc.h"
#include "dawn/native/Instance.h"
#include "dawn/native/null/DeviceNull.h"
#include "dawn/utils/TerribleCommandBuffer.h"
#include "dawn/wire/WireClient.h"

namespace dawn {
namespace {

// dawn_wire and dawn_native contain duplicated code for the handling of GetProcAddress
// so we run the tests against both implementations. This enum is used as a test parameters to
// know which implementation to test.
enum class DawnFlavor {
    Native,
    Wire,
};

std::ostream& operator<<(std::ostream& stream, DawnFlavor flavor) {
    switch (flavor) {
        case DawnFlavor::Native:
            stream << "dawn_native";
            break;

        case DawnFlavor::Wire:
            stream << "dawn_wire";
            break;

        default:
            DAWN_UNREACHABLE();
            break;
    }
    return stream;
}

class GetProcAddressTests : public testing::TestWithParam<DawnFlavor> {
  public:
    GetProcAddressTests()
        : testing::TestWithParam<DawnFlavor>(),
          mNativeInstance(native::APICreateInstance(nullptr)),
          mAdapterBase(mNativeInstance.Get(),
                       AcquireRef(new native::null::PhysicalDevice()),
                       wgpu::FeatureLevel::Core,
                       native::TogglesState(native::ToggleStage::Adapter),
                       wgpu::PowerPreference::Undefined) {}

    void SetUp() override {
        switch (GetParam()) {
            case DawnFlavor::Native: {
                mDevice = wgpu::Device::Acquire(
                    reinterpret_cast<WGPUDevice>(mAdapterBase.APICreateDevice()));
                mProcs = native::GetProcs();
                break;
            }

            case DawnFlavor::Wire: {
                mC2sBuf = std::make_unique<utils::TerribleCommandBuffer>();

                wire::WireClientDescriptor clientDesc = {};
                clientDesc.serializer = mC2sBuf.get();
                mWireClient = std::make_unique<wire::WireClient>(clientDesc);

                // Note that currently we are passing a null device since we do not actually use the
                // device in determining the resulting proc addresses. If/when we actually care
                // about features on the device to determine a proc address, this should be updated
                // accordingly.
                mDevice = nullptr;
                mProcs = wire::client::GetProcs();
                mC2sBuf->SetHandler(mWireClient.get());
                break;
            }

            default:
                DAWN_UNREACHABLE();
                break;
        }

        dawnProcSetProcs(&mProcs);
    }

    void TearDown() override {
        // Destroy the device before freeing the instance or the wire client in the destructor
        mDevice = wgpu::Device();
        if (mC2sBuf != nullptr) {
            mC2sBuf->SetHandler(nullptr);
        }
    }

  protected:
    Ref<native::InstanceBase> mNativeInstance;
    native::AdapterBase mAdapterBase;

    std::unique_ptr<utils::TerribleCommandBuffer> mC2sBuf;
    std::unique_ptr<wire::WireClient> mWireClient;

    wgpu::Device mDevice;
    DawnProcTable mProcs;
};

// Test GetProcAddress with and without devices on some valid examples
TEST_P(GetProcAddressTests, ValidExamples) {
    ASSERT_EQ(mProcs.getProcAddress({"wgpuDeviceCreateBuffer", WGPU_STRLEN}),
              reinterpret_cast<WGPUProc>(mProcs.deviceCreateBuffer));
    ASSERT_EQ(mProcs.getProcAddress({"wgpuQueueSubmit", WGPU_STRLEN}),
              reinterpret_cast<WGPUProc>(mProcs.queueSubmit));
    // Test a longer string, truncated correctly with the length field.
    ASSERT_EQ(mProcs.getProcAddress({"wgpuQueueSubmitExtraString", 15}),
              reinterpret_cast<WGPUProc>(mProcs.queueSubmit));
}

// Test GetProcAddress with and without devices on nullptr procName
TEST_P(GetProcAddressTests, Nullptr) {
    ASSERT_EQ(mProcs.getProcAddress({nullptr, WGPU_STRLEN}), nullptr);
}

// Test GetProcAddress with and without devices on some invalid
TEST_P(GetProcAddressTests, InvalidExamples) {
    ASSERT_EQ(mProcs.getProcAddress({"wgpuDeviceDoSomething", WGPU_STRLEN}), nullptr);

    // Test a "valid" string, truncated to not match with the length field.
    ASSERT_EQ(mProcs.getProcAddress({"wgpuQueueSubmit", 14}), nullptr);

    // Trigger the condition where lower_bound will return the end of the procMap.
    ASSERT_EQ(mProcs.getProcAddress({"zzzzzzz", WGPU_STRLEN}), nullptr);
    ASSERT_EQ(mProcs.getProcAddress({"ZZ", WGPU_STRLEN}), nullptr);

    // Some more potential corner cases.
    ASSERT_EQ(mProcs.getProcAddress({"", WGPU_STRLEN}), nullptr);
    ASSERT_EQ(mProcs.getProcAddress({"0", WGPU_STRLEN}), nullptr);
}

// Test that GetProcAddress supports freestanding function that are handled specially
TEST_P(GetProcAddressTests, FreeStandingFunctions) {
    ASSERT_EQ(mProcs.getProcAddress({"wgpuGetProcAddress", WGPU_STRLEN}),
              reinterpret_cast<WGPUProc>(mProcs.getProcAddress));
    ASSERT_EQ(mProcs.getProcAddress({"wgpuCreateInstance", WGPU_STRLEN}),
              reinterpret_cast<WGPUProc>(mProcs.createInstance));
}

INSTANTIATE_TEST_SUITE_P(,
                         GetProcAddressTests,
                         testing::Values(DawnFlavor::Native, DawnFlavor::Wire),
                         testing::PrintToStringParamName());

TEST(GetProcAddressInternalTests, CheckDawnNativeProcMapOrder) {
    std::vector<std::string_view> names = native::GetProcMapNamesForTesting();
    for (size_t i = 1; i < names.size(); i++) {
        ASSERT_LT(names[i - 1], names[i]);
    }
}

TEST(GetProcAddressInternalTests, CheckDawnWireClientProcMapOrder) {
    std::vector<std::string_view> names = wire::client::GetProcMapNamesForTesting();
    for (size_t i = 1; i < names.size(); i++) {
        ASSERT_LT(names[i - 1], names[i]);
    }
}

}  // anonymous namespace
}  // namespace dawn
