// Copyright 2023 The Dawn & Tint Authors
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

#include "src/tint/lang/core/ir/return.h"

#include "gmock/gmock.h"
#include "src/tint/lang/core/ir/ir_helper_test.h"

namespace tint::core::ir {
namespace {

using namespace tint::core::number_suffixes;  // NOLINT
using IR_ReturnTest = IRTestHelper;

TEST_F(IR_ReturnTest, ImplicitNoValue) {
    auto* func = b.Function("myfunc", ty.void_());
    auto* ret = b.Return(func);
    ASSERT_EQ(ret->Func(), func);
    EXPECT_TRUE(ret->Args().IsEmpty());
    EXPECT_EQ(ret->Value(), nullptr);
    EXPECT_THAT(func->UsagesUnsorted(), testing::UnorderedElementsAre(Usage{ret, 0u}));
}

TEST_F(IR_ReturnTest, WithValue) {
    auto* func = b.Function("myfunc", ty.i32());
    auto* val = b.Constant(42_i);
    auto* ret = b.Return(func, val);
    ASSERT_EQ(ret->Func(), func);
    ASSERT_EQ(ret->Args().Length(), 1u);
    EXPECT_EQ(ret->Args()[0], val);
    EXPECT_EQ(ret->Value(), val);
    EXPECT_THAT(func->UsagesUnsorted(), testing::UnorderedElementsAre(Usage{ret, 0u}));
    EXPECT_THAT(val->UsagesUnsorted(), testing::UnorderedElementsAre(Usage{ret, 1u}));
}

TEST_F(IR_ReturnTest, Result) {
    auto* vfunc = b.Function("vfunc", ty.void_());
    auto* ifunc = b.Function("ifunc", ty.i32());

    {
        auto* ret1 = b.Return(vfunc);
        EXPECT_TRUE(ret1->Results().IsEmpty());
    }

    {
        auto* ret2 = b.Return(ifunc, b.Constant(42_i));
        EXPECT_TRUE(ret2->Results().IsEmpty());
    }
}

TEST_F(IR_ReturnTest, Clone) {
    auto* func = b.Function("func", ty.i32());
    auto* ret = b.Return(func, b.Constant(1_i));

    auto* new_func = clone_ctx.Clone(func);
    auto* new_ret = clone_ctx.Clone(ret);

    EXPECT_NE(ret, new_ret);
    EXPECT_EQ(new_func, new_ret->Func());

    EXPECT_EQ(1u, new_ret->Args().Length());

    auto new_val = new_ret->Value()->As<Constant>()->Value();
    ASSERT_TRUE(new_val->Is<core::constant::Scalar<i32>>());
    EXPECT_EQ(1_i, new_val->As<core::constant::Scalar<i32>>()->ValueAs<i32>());
}

TEST_F(IR_ReturnTest, CloneWithoutArgs) {
    auto* func = b.Function("func", ty.i32());
    auto* ret = b.Return(func);

    auto* new_func = clone_ctx.Clone(func);
    auto* new_ret = clone_ctx.Clone(ret);

    EXPECT_EQ(new_func, new_ret->Func());
    EXPECT_EQ(nullptr, new_ret->Value());
}

}  // namespace
}  // namespace tint::core::ir
