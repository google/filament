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

#include "src/tint/utils/ice/ice.h"

#include "gtest/gtest.h"

namespace tint {
namespace {

TEST(ICETest_DeathTest, Unimplemented) {
    EXPECT_DEATH_IF_SUPPORTED(
        {
            if ((true)) {
                TINT_UNIMPLEMENTED();
            }
        },
        "ice_test.cc:.* internal compiler error: TINT_UNIMPLEMENTED");
}

TEST(ICETest_DeathTest, Unreachable) {
    EXPECT_DEATH_IF_SUPPORTED(
        {
            if ((true)) {
                TINT_UNREACHABLE();
            }
        },
        "ice_test.cc:.* internal compiler error: TINT_UNREACHABLE");
}

TEST(ICETest, AssertTrue) {
    TINT_ASSERT(true);
}

TEST(ICETest_DeathTest, AssertFalse) {
    EXPECT_DEATH_IF_SUPPORTED(
        {
            if ((true)) {
                TINT_ASSERT(false);
            }
        },
        "ice_test.cc:.* internal compiler error: TINT_ASSERT.false.");
}

TEST(ICETest_DeathTest, AssertFalse_WithCallback) {
    uint32_t data = 42;
    InternalCompilerErrorCallbackInfo callback{
        .callback =
            [](std::string err, void* userdata) {
                std::cerr << "callback called with " << *static_cast<uint32_t*>(userdata) << err;
            },
        .userdata = &data,
    };
    EXPECT_DEATH_IF_SUPPORTED(
        {
            if ((true)) {
                TINT_ASSERT(false, callback);
            }
        },
        "callback called with 42.*ice_test.cc:.* internal compiler error: TINT_ASSERT.false.");
}

TEST(ICETest_DeathTest, ICE) {
    EXPECT_DEATH_IF_SUPPORTED(
        {
            if ((true)) {
                TINT_ICE() << "custom message " << 123;
            }
        },
        "ice_test.cc:.* internal compiler error: custom message 123");
}

TEST(ICETest_DeathTest, ICE_WithCallback) {
    uint32_t data = 42;
    InternalCompilerErrorCallbackInfo callback{
        .callback =
            [](std::string err, void* userdata) {
                std::cerr << "callback called with " << *static_cast<uint32_t*>(userdata) << err;
            },
        .userdata = &data,
    };
    EXPECT_DEATH_IF_SUPPORTED(
        {
            if ((true)) {
                TINT_ICE(callback) << "custom message " << 123;
            }
        },
        "callback called with 42.*ice_test.cc:.* internal compiler error: custom message 123");
}

}  // namespace
}  // namespace tint
