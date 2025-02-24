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

#include "src/tint/lang/wgsl/ast/helper_test.h"

#include "src/tint/utils/text/string_stream.h"

namespace tint::ast {
namespace {

using FloatLiteralExpressionTest = TestHelper;

TEST_F(FloatLiteralExpressionTest, SuffixNone) {
    auto* i = create<FloatLiteralExpression>(42.0, FloatLiteralExpression::Suffix::kNone);
    ASSERT_TRUE(i->Is<FloatLiteralExpression>());
    EXPECT_EQ(i->value, 42);
    EXPECT_EQ(i->suffix, FloatLiteralExpression::Suffix::kNone);
}

TEST_F(FloatLiteralExpressionTest, SuffixF) {
    auto* i = create<FloatLiteralExpression>(42.0, FloatLiteralExpression::Suffix::kF);
    ASSERT_TRUE(i->Is<FloatLiteralExpression>());
    EXPECT_EQ(i->value, 42);
    EXPECT_EQ(i->suffix, FloatLiteralExpression::Suffix::kF);
}

TEST_F(FloatLiteralExpressionTest, SuffixH) {
    auto* i = create<FloatLiteralExpression>(42.0, FloatLiteralExpression::Suffix::kH);
    ASSERT_TRUE(i->Is<FloatLiteralExpression>());
    EXPECT_EQ(i->value, 42);
    EXPECT_EQ(i->suffix, FloatLiteralExpression::Suffix::kH);
}

TEST_F(FloatLiteralExpressionTest, SuffixStringStream) {
    auto to_str = [](FloatLiteralExpression::Suffix suffix) {
        StringStream ss;
        ss << suffix;
        return ss.str();
    };

    EXPECT_EQ("", to_str(FloatLiteralExpression::Suffix::kNone));
    EXPECT_EQ("f", to_str(FloatLiteralExpression::Suffix::kF));
    EXPECT_EQ("h", to_str(FloatLiteralExpression::Suffix::kH));
}

}  // namespace
}  // namespace tint::ast
