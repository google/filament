// Copyright 2025 The Dawn & Tint Authors
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

#include "dawn/tests/unittests/wire/WireTest.h"
#include "dawn/utils/WGPUHelpers.h"
#include "dawn/wire/WireClient.h"

namespace dawn::wire {
namespace {

enum class TestState {
    // Mix the two wires while both clients are connected.
    Alive,
    // Disconnect WireTwo before creating an object on it.
    DisconnectBefore,
    // Disconnect WireTwo after creating an object on it.
    DisconnectMid,
};
std::ostream& operator<<(std::ostream& stream, TestState flavor) {
    switch (flavor) {
        case TestState::Alive:
            stream << "Alive";
            break;
        case TestState::DisconnectBefore:
            stream << "DisconnectBefore";
            break;
        case TestState::DisconnectMid:
            stream << "DisconnectMid";
            break;
    }
    return stream;
}

// Two copies of WireTest to set up a second parallel wire.
// Create two classes that inherit WireTest, so that the test can inherit both
// of them, to set up two parallel wires to test with.
class WireOne : public WireTest {};
class WireTwo : public WireTest {};

// Tests for intentional wire client CHECK crashes when trying to use objects
// from the wrong wire which would have bogus IDs. (The crash is intended, but
// in principle we shouldn't actually crash. See crbug.com/440387003.)
// Name "*DeathTest" per https://google.github.io/googletest/advanced.html#death-test-naming
class WireConfusionDeathTest : public WireOne,
                               public WireTwo,
                               public ::testing::WithParamInterface<TestState> {
  protected:
    void SetUp() override {
        WireOne::SetUp();
        WireTwo::SetUp();

        if (GetParam() == TestState::DisconnectBefore) {
            WireTwo::GetWireClient()->Disconnect();
        }
    }

    void TearDown() override {
        WireTwo::TearDown();
        WireOne::TearDown();
    }

    template <typename Lambda>
    void MaybeDisconnectAndExpectDeath(bool expectDeath, Lambda lambda) {
        if (GetParam() == TestState::DisconnectMid) {
            WireTwo::GetWireClient()->Disconnect();
        }
        if (expectDeath) {
#if defined(DAWN_ENABLE_ASSERTS)
            EXPECT_DEATH(lambda(), "forClient == mClient");
#else
            EXPECT_DEATH(lambda(), "");
#endif
        } else {
            lambda();
        }
    }
};

// Test calling queue.WriteBuffer using a buffer from another device.
TEST_P(WireConfusionDeathTest, WriteBuffer) {
    wgpu::BufferDescriptor bufDesc{
        .usage = wgpu::BufferUsage::CopyDst,
        .size = 4,
    };
    wgpu::Buffer two_buf = WireTwo::device.CreateBuffer(&bufDesc);

    MaybeDisconnectAndExpectDeath(true, [&]() {
        WireOne::queue.WriteBuffer(two_buf, 0, nullptr, 0);  //
    });
}

// Test creating a bind group using a layout from an old device.
TEST_P(WireConfusionDeathTest, NewBindGroupFromOldLayout) {
    wgpu::BindGroupLayout two_bgl = utils::MakeBindGroupLayout(WireTwo::device, {});

    wgpu::BindGroupDescriptor bgDesc{.layout = two_bgl};
    MaybeDisconnectAndExpectDeath(true, [&]() {
        WireOne::device.CreateBindGroup(&bgDesc);  //
    });
}

// Test creating a bind group on an old device using a layout from a new device.
TEST_P(WireConfusionDeathTest, OldBindGroupFromNewLayout) {
    wgpu::BindGroupLayout one_bgl = utils::MakeBindGroupLayout(WireOne::device, {});

    wgpu::BindGroupDescriptor bgDesc{.layout = one_bgl};
    // Should not crash if wire two is already disconnected.
    MaybeDisconnectAndExpectDeath(GetParam() == TestState::Alive,
                                  [&]() { WireTwo::device.CreateBindGroup(&bgDesc); });
}

INSTANTIATE_TEST_SUITE_P(,
                         WireConfusionDeathTest,
                         ::testing::Values(TestState::Alive,
                                           TestState::DisconnectBefore,
                                           TestState::DisconnectMid),
                         testing::PrintToStringParamName());

}  // anonymous namespace
}  // namespace dawn::wire
