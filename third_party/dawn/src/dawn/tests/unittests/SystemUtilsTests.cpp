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

#include <string>

#include "dawn/common/Assert.h"
#include "dawn/common/SystemUtils.h"
#include "gmock/gmock-matchers.h"
#include "gtest/gtest.h"

namespace dawn {
namespace {

using ::testing::_;
using ::testing::Pair;

// Tests for GetEnvironmentVar
TEST(SystemUtilsTests, GetEnvironmentVar) {
    // Test nonexistent environment variable
    EXPECT_THAT(GetEnvironmentVar("NonexistentEnvironmentVar"), Pair("", false));
}

// Tests for SetEnvironmentVar
TEST(SystemUtilsTests, SetEnvironmentVar) {
    // Test new environment variable
    EXPECT_TRUE(SetEnvironmentVar("EnvironmentVarForTest", "NewEnvironmentVarValue"));
    EXPECT_THAT(GetEnvironmentVar("EnvironmentVarForTest"), Pair("NewEnvironmentVarValue", true));
    // Test override environment variable
    EXPECT_TRUE(SetEnvironmentVar("EnvironmentVarForTest", "OverrideEnvironmentVarValue"));
    EXPECT_THAT(GetEnvironmentVar("EnvironmentVarForTest"),
                Pair("OverrideEnvironmentVarValue", true));
}

// Tests for GetExecutableDirectory
TEST(SystemUtilsTests, GetExecutableDirectory) {
    auto dir = GetExecutableDirectory();
    // Test returned value is non-empty string
    EXPECT_NE(dir, std::optional{std::string("")});
    ASSERT_NE(dir, std::nullopt);
    // Test last character in path
    EXPECT_EQ(dir->back(), *GetPathSeparator());
}

// Tests for ScopedEnvironmentVar
TEST(SystemUtilsTests, ScopedEnvironmentVar) {
    SetEnvironmentVar("ScopedEnvironmentVarForTest", "original");

    // Test empty environment variable doesn't crash
    { ScopedEnvironmentVar var; }

    // Test setting empty environment variable
    {
        ScopedEnvironmentVar var;
        var.Set("ScopedEnvironmentVarForTest", "NewEnvironmentVarValue");
        EXPECT_THAT(GetEnvironmentVar("ScopedEnvironmentVarForTest"),
                    Pair("NewEnvironmentVarValue", true));
    }
    EXPECT_THAT(GetEnvironmentVar("ScopedEnvironmentVarForTest"), Pair("original", true));

    // Test that the environment variable can be set, and it is unset at the end of the scope.
    {
        ScopedEnvironmentVar var("ScopedEnvironmentVarForTest", "NewEnvironmentVarValue");
        EXPECT_THAT(GetEnvironmentVar("ScopedEnvironmentVarForTest"),
                    Pair("NewEnvironmentVarValue", true));
    }
    EXPECT_THAT(GetEnvironmentVar("ScopedEnvironmentVarForTest"), Pair("original", true));

    // Test nested scopes
    {
        ScopedEnvironmentVar outer("ScopedEnvironmentVarForTest", "outer");
        EXPECT_THAT(GetEnvironmentVar("ScopedEnvironmentVarForTest"), Pair("outer", true));
        {
            ScopedEnvironmentVar inner("ScopedEnvironmentVarForTest", "inner");
            EXPECT_THAT(GetEnvironmentVar("ScopedEnvironmentVarForTest"), Pair("inner", true));
        }
        EXPECT_THAT(GetEnvironmentVar("ScopedEnvironmentVarForTest"), Pair("outer", true));
    }
    EXPECT_THAT(GetEnvironmentVar("ScopedEnvironmentVarForTest"), Pair("original", true));

    // Test redundantly setting scoped variables
    {
        ScopedEnvironmentVar var1("ScopedEnvironmentVarForTest", "var1");
        EXPECT_THAT(GetEnvironmentVar("ScopedEnvironmentVarForTest"), Pair("var1", true));

        ScopedEnvironmentVar var2("ScopedEnvironmentVarForTest", "var2");
        EXPECT_THAT(GetEnvironmentVar("ScopedEnvironmentVarForTest"), Pair("var2", true));
    }
    EXPECT_THAT(GetEnvironmentVar("ScopedEnvironmentVarForTest"), Pair("original", true));
}

// Test that restoring a scoped environment variable to the empty string.
TEST(SystemUtilsTests, ScopedEnvironmentVarRestoresEmptyString) {
    ScopedEnvironmentVar empty("ScopedEnvironmentVarForTest", "");
    {
        ScopedEnvironmentVar var1("ScopedEnvironmentVarForTest", "var1");
        EXPECT_THAT(GetEnvironmentVar("ScopedEnvironmentVarForTest"), Pair("var1", true));
    }
    EXPECT_THAT(GetEnvironmentVar("ScopedEnvironmentVarForTest"), Pair("", true));
}

// Test that restoring a scoped environment variable to not set (distuishable from empty string)
// works.
TEST(SystemUtilsTests, ScopedEnvironmentVarRestoresNotSet) {
    ScopedEnvironmentVar null("ScopedEnvironmentVarForTest", nullptr);
    {
        ScopedEnvironmentVar var1("ScopedEnvironmentVarForTest", "var1");
        EXPECT_THAT(GetEnvironmentVar("ScopedEnvironmentVarForTest"), Pair("var1", true));
    }
    EXPECT_THAT(GetEnvironmentVar("ScopedEnvironmentVarForTest"), Pair("", false));
}

}  // anonymous namespace
}  // namespace dawn
