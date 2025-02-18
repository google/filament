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

#include <vector>

#include "dawn/tests/unittests/validation/ValidationTest.h"

namespace dawn {
namespace {

class ToggleValidationTest : public ValidationTest {
    void SetUp() override {
        ValidationTest::SetUp();
        DAWN_SKIP_TEST_IF(UsesWire());
    }
};

// Tests querying the detail of a toggle from native::InstanceBase works correctly.
TEST_F(ToggleValidationTest, QueryToggleInfo) {
    // Query with a valid toggle name
    {
        const char* kValidToggleName = "emulate_store_and_msaa_resolve";
        const native::ToggleInfo* toggleInfo = GetToggleInfo(kValidToggleName);
        ASSERT_NE(nullptr, toggleInfo);
        ASSERT_NE(nullptr, toggleInfo->name);
        ASSERT_NE(nullptr, toggleInfo->description);
        ASSERT_NE(nullptr, toggleInfo->url);
    }

    // Query with an invalid toggle name
    {
        const char* kInvalidToggleName = "!@#$%^&*";
        const native::ToggleInfo* toggleInfo = GetToggleInfo(kInvalidToggleName);
        ASSERT_EQ(nullptr, toggleInfo);
    }
}

// Tests overriding toggles when creating a device works correctly.
TEST_F(ToggleValidationTest, OverrideToggleUsage) {
    // Create device with a valid name of a toggle
    {
        const char* kValidToggleName = "emulate_store_and_msaa_resolve";
        wgpu::DeviceDescriptor descriptor;
        wgpu::DawnTogglesDescriptor deviceTogglesDesc;
        descriptor.nextInChain = &deviceTogglesDesc;
        deviceTogglesDesc.enabledToggles = &kValidToggleName;
        deviceTogglesDesc.enabledToggleCount = 1;

        wgpu::Device deviceWithToggle =
            wgpu::Device::Acquire(GetBackendAdapter().CreateDevice(&descriptor));
        std::vector<const char*> toggleNames = native::GetTogglesUsed(deviceWithToggle.Get());
        bool validToggleExists = false;
        for (const char* toggle : toggleNames) {
            if (strcmp(toggle, kValidToggleName) == 0) {
                validToggleExists = true;
            }
        }
        ASSERT_EQ(validToggleExists, true);
    }

    // Create device with an invalid toggle name
    {
        const char* kInvalidToggleName = "!@#$%^&*";
        wgpu::DeviceDescriptor descriptor;
        wgpu::DawnTogglesDescriptor deviceTogglesDesc;
        descriptor.nextInChain = &deviceTogglesDesc;
        deviceTogglesDesc.enabledToggles = &kInvalidToggleName;
        deviceTogglesDesc.enabledToggleCount = 1;

        wgpu::Device deviceWithToggle =
            wgpu::Device::Acquire(GetBackendAdapter().CreateDevice(&descriptor));
        std::vector<const char*> toggleNames = native::GetTogglesUsed(deviceWithToggle.Get());
        bool InvalidToggleExists = false;
        for (const char* toggle : toggleNames) {
            if (strcmp(toggle, kInvalidToggleName) == 0) {
                InvalidToggleExists = true;
            }
        }
        ASSERT_EQ(InvalidToggleExists, false);
    }
}

TEST_F(ToggleValidationTest, TurnOffVsyncWithToggle) {
    const char* kValidToggleName = "turn_off_vsync";
    wgpu::DeviceDescriptor descriptor;
    wgpu::DawnTogglesDescriptor deviceTogglesDesc;
    descriptor.nextInChain = &deviceTogglesDesc;
    deviceTogglesDesc.enabledToggles = &kValidToggleName;
    deviceTogglesDesc.enabledToggleCount = 1;

    wgpu::Device deviceWithToggle =
        wgpu::Device::Acquire(GetBackendAdapter().CreateDevice(&descriptor));
    std::vector<const char*> toggleNames = native::GetTogglesUsed(deviceWithToggle.Get());
    bool validToggleExists = false;
    for (const char* toggle : toggleNames) {
        if (strcmp(toggle, kValidToggleName) == 0) {
            validToggleExists = true;
        }
    }
    ASSERT_EQ(validToggleExists, true);
}

}  // anonymous namespace
}  // namespace dawn
