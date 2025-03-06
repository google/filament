// Copyright 2022 The Dawn & Tint Authors
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

#include "src/tint/lang/wgsl/resolver/resolver.h"

#include "gmock/gmock.h"
#include "src/tint/lang/wgsl/resolver/resolver_helper_test.h"

using namespace tint::core::number_suffixes;  // NOLINT

namespace tint::resolver {
namespace {

using ResolverConstAssertTest = ResolverTest;

TEST_F(ResolverConstAssertTest, Global_True_Pass) {
    GlobalConstAssert(true);
    ASSERT_TRUE(r()->Resolve()) << r()->error();
}

TEST_F(ResolverConstAssertTest, Global_False_Fail) {
    GlobalConstAssert(Source{{12, 34}}, false);
    EXPECT_FALSE(r()->Resolve());
    EXPECT_EQ(r()->error(), "12:34 error: const assertion failed");
}

TEST_F(ResolverConstAssertTest, Global_Const_Pass) {
    GlobalConst("C", ty.bool_(), Expr(true));
    GlobalConstAssert("C");
    ASSERT_TRUE(r()->Resolve()) << r()->error();
}

TEST_F(ResolverConstAssertTest, Global_Const_Fail) {
    GlobalConst("C", ty.bool_(), Expr(false));
    GlobalConstAssert(Source{{12, 34}}, "C");
    EXPECT_FALSE(r()->Resolve());
    EXPECT_EQ(r()->error(), "12:34 error: const assertion failed");
}

TEST_F(ResolverConstAssertTest, Global_LessThan_Pass) {
    GlobalConstAssert(LessThan(2_i, 3_i));
    ASSERT_TRUE(r()->Resolve()) << r()->error();
}

TEST_F(ResolverConstAssertTest, Global_LessThan_Fail) {
    GlobalConstAssert(Source{{12, 34}}, LessThan(4_i, 3_i));
    EXPECT_FALSE(r()->Resolve());
    EXPECT_EQ(r()->error(), "12:34 error: const assertion failed");
}

TEST_F(ResolverConstAssertTest, Local_True_Pass) {
    WrapInFunction(ConstAssert(true));
    ASSERT_TRUE(r()->Resolve()) << r()->error();
}

TEST_F(ResolverConstAssertTest, Local_False_Fail) {
    WrapInFunction(ConstAssert(Source{{12, 34}}, false));
    EXPECT_FALSE(r()->Resolve());
    EXPECT_EQ(r()->error(), "12:34 error: const assertion failed");
}

TEST_F(ResolverConstAssertTest, Local_Const_Pass) {
    GlobalConst("C", ty.bool_(), Expr(true));
    WrapInFunction(ConstAssert("C"));
    ASSERT_TRUE(r()->Resolve()) << r()->error();
}

TEST_F(ResolverConstAssertTest, Local_Const_Fail) {
    GlobalConst("C", ty.bool_(), Expr(false));
    WrapInFunction(ConstAssert(Source{{12, 34}}, "C"));
    EXPECT_FALSE(r()->Resolve());
    EXPECT_EQ(r()->error(), "12:34 error: const assertion failed");
}

TEST_F(ResolverConstAssertTest, Local_NonConst) {
    GlobalVar("V", ty.bool_(), Expr(true), core::AddressSpace::kPrivate);
    WrapInFunction(ConstAssert(Expr(Source{{12, 34}}, "V")));
    EXPECT_FALSE(r()->Resolve());
    EXPECT_EQ(r()->error(),
              "12:34 error: const assertion requires a const-expression, but expression is a "
              "runtime-expression");
}

TEST_F(ResolverConstAssertTest, Local_LessThan_Pass) {
    WrapInFunction(ConstAssert(LessThan(2_i, 3_i)));
    ASSERT_TRUE(r()->Resolve()) << r()->error();
}

TEST_F(ResolverConstAssertTest, Local_LessThan_Fail) {
    WrapInFunction(ConstAssert(Source{{12, 34}}, LessThan(4_i, 3_i)));
    EXPECT_FALSE(r()->Resolve());
    EXPECT_EQ(r()->error(), "12:34 error: const assertion failed");
}

}  // namespace
}  // namespace tint::resolver
