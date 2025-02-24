// Copyright 2024 The Dawn & Tint Authors
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

#include "src/tint/utils/text/text_style.h"

#include "gtest/gtest.h"

namespace tint {
namespace {

using TextStyleTest = testing::Test;

TEST_F(TextStyleTest, Add) {
    // Plain + X = X
    EXPECT_EQ(style::Plain + style::Plain, style::Plain);
    EXPECT_EQ(style::Plain + style::Bold, style::Bold);
    EXPECT_EQ(style::Plain + style::Underlined, style::Underlined);
    EXPECT_EQ(style::Plain + style::Success, style::Success);
    EXPECT_EQ(style::Plain + style::Warning, style::Warning);
    EXPECT_EQ(style::Plain + style::Error, style::Error);
    EXPECT_EQ(style::Plain + style::Fatal, style::Fatal);
    EXPECT_EQ(style::Plain + style::Code, style::Code);
    EXPECT_EQ(style::Plain + style::Keyword, style::Keyword);
    EXPECT_EQ(style::Plain + style::Variable, style::Variable);
    EXPECT_EQ(style::Plain + style::Type, style::Type);
    EXPECT_EQ(style::Plain + style::Function, style::Function);
    EXPECT_EQ(style::Plain + style::Enum, style::Enum);
    EXPECT_EQ(style::Plain + style::Literal, style::Literal);
    EXPECT_EQ(style::Plain + style::Attribute, style::Attribute);
    EXPECT_EQ(style::Plain + style::Squiggle, style::Squiggle);

    // Non-colliding combinations
    EXPECT_EQ(style::Bold + style::Underlined, style::Bold + style::Underlined);
    EXPECT_EQ(style::Underlined + style::Success, style::Underlined + style::Success);
    EXPECT_EQ(style::Type + style::Error, style::Type + style::Error);
    EXPECT_EQ(style::Bold + style::Underlined + style::Variable + style::Squiggle,
              style::Bold + style::Underlined + style::Variable + style::Squiggle);

    // Colliding combinations resolve to RHS
    EXPECT_EQ(style::Error + style::Warning, style::Warning);
    EXPECT_EQ(style::Warning + style::Error, style::Error);
    EXPECT_EQ(style::Variable + style::Attribute + style::Type, style::Type);
}

}  // namespace
}  // namespace tint
