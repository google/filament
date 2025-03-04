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

#include "src/tint/lang/wgsl/ast/workgroup_attribute.h"

#include "src/tint/lang/wgsl/ast/helper_test.h"
#include "src/tint/lang/wgsl/ast/stage_attribute.h"

using namespace tint::core::number_suffixes;  // NOLINT

namespace tint::ast {
namespace {

using WorkgroupAttributeTest = TestHelper;

TEST_F(WorkgroupAttributeTest, Creation_1param) {
    auto* d = WorkgroupSize(2_i);
    auto values = d->Values();

    ASSERT_TRUE(values[0]->Is<IntLiteralExpression>());
    EXPECT_EQ(values[0]->As<IntLiteralExpression>()->value, 2);

    EXPECT_EQ(values[1], nullptr);
    EXPECT_EQ(values[2], nullptr);
}
TEST_F(WorkgroupAttributeTest, Creation_2param) {
    auto* d = WorkgroupSize(2_i, 4_i);
    auto values = d->Values();

    ASSERT_TRUE(values[0]->Is<IntLiteralExpression>());
    EXPECT_EQ(values[0]->As<IntLiteralExpression>()->value, 2);

    ASSERT_TRUE(values[1]->Is<IntLiteralExpression>());
    EXPECT_EQ(values[1]->As<IntLiteralExpression>()->value, 4);

    EXPECT_EQ(values[2], nullptr);
}

TEST_F(WorkgroupAttributeTest, Creation_3param) {
    auto* d = WorkgroupSize(2_i, 4_i, 6_i);
    auto values = d->Values();

    ASSERT_TRUE(values[0]->Is<IntLiteralExpression>());
    EXPECT_EQ(values[0]->As<IntLiteralExpression>()->value, 2);

    ASSERT_TRUE(values[1]->Is<IntLiteralExpression>());
    EXPECT_EQ(values[1]->As<IntLiteralExpression>()->value, 4);

    ASSERT_TRUE(values[2]->Is<IntLiteralExpression>());
    EXPECT_EQ(values[2]->As<IntLiteralExpression>()->value, 6);
}

TEST_F(WorkgroupAttributeTest, Creation_WithIdentifier) {
    auto* d = WorkgroupSize(2_i, 4_i, "depth");
    auto values = d->Values();

    ASSERT_TRUE(values[0]->Is<IntLiteralExpression>());
    EXPECT_EQ(values[0]->As<IntLiteralExpression>()->value, 2);

    ASSERT_TRUE(values[1]->Is<IntLiteralExpression>());
    EXPECT_EQ(values[1]->As<IntLiteralExpression>()->value, 4);

    auto* z_ident = As<IdentifierExpression>(values[2]);
    ASSERT_TRUE(z_ident);
    EXPECT_EQ(z_ident->identifier->symbol.Name(), "depth");
}

}  // namespace
}  // namespace tint::ast
