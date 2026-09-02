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

#include "src/utils/assert.h"

#include <cstdint>

#include "src/utils/gtest.h"

namespace dawn {
namespace {

using AssertTest = ::testing::Test;

// Test that there's no warning if a variable is unused other than DAWN_ASSERT.
TEST_F(AssertTest, AssertUnused) {
    int test_value = 1;
    DAWN_ASSERT(test_value == 1);
}

// Test that DAWN_ASSERT's condition expression is executed in debug, but not in release.
TEST_F(AssertTest, AssertSideEffects) {
    int test_value = 1;
    auto SetValue = [&](int x) {
        test_value = x;
        return true;
    };
    DAWN_ASSERT(SetValue(2));

#if defined(DAWN_ENABLE_ASSERTS)
    EXPECT_EQ(test_value, 2);
#else
    EXPECT_EQ(test_value, 1);
#endif
}

// Test that DAWN_RELEASE_ASSUME's condition expression is executed in debug, but not in release.
TEST_F(AssertTest, ReleaseAssumeSideEffects) {
    int test_value = 1;
    auto SetValue = [&](int x) {
        test_value = x;
        return true;
    };
    DAWN_RELEASE_ASSUME(SetValue(2));

#if defined(DAWN_ENABLE_ASSERTS)
    EXPECT_EQ(test_value, 2);
#else
    EXPECT_EQ(test_value, 1);
#endif
}

// Name "*DeathTest" per https://google.github.io/googletest/advanced.html#death-test-naming
using AssertDeathTest = ::testing::Test;

// DAWN_UNREACHABLE crashes in both debug and release.
TEST_F(AssertDeathTest, SimpleUnreachable) {
#if defined(DAWN_ENABLE_ASSERTS)
    EXPECT_DEATH_IF_SUPPORTED(DAWN_UNREACHABLE(), "Unreachable code hit");
#else
    EXPECT_DEATH_IF_SUPPORTED(DAWN_UNREACHABLE(), "");
#endif
}

enum class TestEnum : uint32_t {
    A = 0,
    B,
    C,
    D,
    E,
    F,
};

uint32_t DoFakeOp(TestEnum enum_val, uint32_t t) {
    // This should use a jump table in clang
    switch (enum_val) {
        case TestEnum::A:
            return (t + 1) % 13;
        case TestEnum::B:
            return (t + 1) % 43;
        case TestEnum::C:
            return (t + 1) % 33;
        case TestEnum::D:
            return (t + 1) % 53;
        case TestEnum::E:
            return (t + 1) % 93;
        case TestEnum::F:
            return (t + 1) % 713;
        default:
            DAWN_UNREACHABLE();
            break;
    }
    return 0;
}

// Value chosen to be outside the 'TestEnum' range.
volatile uint32_t g_var = 123;
TEST_F(AssertDeathTest, JumpTableUnreachable) {
    // We need to sneak in the out of bounds modification to the enum to prevent other safety checks
    // from triggering.
    g_var = g_var + 1;
    TestEnum enum_val = static_cast<TestEnum>(g_var);

#if defined(DAWN_ENABLE_ASSERTS)
    EXPECT_DEATH_IF_SUPPORTED(g_var = DoFakeOp(enum_val, g_var), "Unreachable code hit");
#else
    EXPECT_DEATH_IF_SUPPORTED(g_var = DoFakeOp(enum_val, g_var), "");
#endif

    g_var = g_var + 1;
}

// Volatile to make sure reads happen at runtime.
volatile uint32_t g_var2 = 123;
TEST_F(AssertDeathTest, AssertKills) {
    g_var2 = g_var2 + 1;
#ifdef DAWN_ENABLE_ASSERTS
    EXPECT_DEATH_IF_SUPPORTED(DAWN_ASSERT(g_var2 != 124), "g_var2 != 124");
#endif
}

#ifndef _WIN32
TEST_F(AssertDeathTest, StackTrace) {
    EXPECT_DEATH_IF_SUPPORTED(DAWN_UNREACHABLE(), "PC: @");
}

TEST_F(AssertDeathTest, CrashStackTrace) {
    EXPECT_DEATH_IF_SUPPORTED(
        {
            volatile int* ptr = nullptr;
            *ptr = 1;
        },
        "PC: @");
}
#endif  // !defined(_WIN32)

}  // anonymous namespace
}  // namespace dawn
