// Copyright 2020 The Dawn & Tint Authors
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

#include "src/tint/lang/spirv/reader/ast_parser/fail_stream.h"

#include "gmock/gmock.h"
#include "src/tint/utils/text/string_stream.h"

namespace tint::spirv::reader::ast_parser {
namespace {

using ::testing::Eq;

using FailStreamTest = ::testing::Test;

TEST_F(FailStreamTest, ConversionToBoolIsSameAsStatusMethod) {
    bool flag = true;
    FailStream fs(&flag, nullptr);

    EXPECT_TRUE(fs.status());
    EXPECT_TRUE(bool(fs));  // NOLINT
    flag = false;
    EXPECT_FALSE(fs.status());
    EXPECT_FALSE(bool(fs));  // NOLINT
    flag = true;
    EXPECT_TRUE(fs.status());
    EXPECT_TRUE(bool(fs));  // NOLINT
}

TEST_F(FailStreamTest, FailMethodChangesStatusToFalse) {
    bool flag = true;
    FailStream fs(&flag, nullptr);
    EXPECT_TRUE(flag);
    EXPECT_TRUE(bool(fs));  // NOLINT
    fs.Fail();
    EXPECT_FALSE(flag);
    EXPECT_FALSE(bool(fs));  // NOLINT
}

TEST_F(FailStreamTest, FailMethodReturnsSelf) {
    bool flag = true;
    FailStream fs(&flag, nullptr);
    FailStream& result = fs.Fail();
    EXPECT_THAT(&result, Eq(&fs));
}

TEST_F(FailStreamTest, ShiftOperatorAccumulatesValues) {
    bool flag = true;
    StringStream ss;
    FailStream fs(&flag, &ss);

    ss << "prefix ";
    fs << "cat " << 42;

    EXPECT_THAT(ss.str(), Eq("prefix cat 42"));
}

}  // namespace
}  // namespace tint::spirv::reader::ast_parser
