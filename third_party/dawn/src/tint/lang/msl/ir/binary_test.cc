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

#include "src/tint/lang/msl/ir/binary.h"

#include "gtest/gtest.h"
#include "src/tint/lang/core/fluent_types.h"
#include "src/tint/lang/core/ir/ir_helper_test.h"
#include "src/tint/lang/core/ir/validator.h"
#include "src/tint/utils/result/result.h"

using namespace tint::core::fluent_types;  // NOLINT

namespace tint::msl::ir {
namespace {

using namespace tint::core::number_suffixes;  // NOLINT

using IR_MslBinaryTest = core::ir::IRTestHelper;

TEST_F(IR_MslBinaryTest, Clone) {
    auto* lhs = b.Constant(i8(2));
    auto* rhs = b.Constant(i8(4));
    auto* inst = b.Binary<msl::ir::Binary>(core::BinaryOp::kAdd, mod.Types().i8(), lhs, rhs);

    auto* c = clone_ctx.Clone(inst);

    EXPECT_NE(inst, c);

    EXPECT_EQ(mod.Types().i8(), c->Result(0)->Type());
    EXPECT_EQ(core::BinaryOp::kAdd, c->Op());

    auto new_lhs = c->LHS()->As<core::ir::Constant>()->Value();
    ASSERT_TRUE(new_lhs->Is<core::constant::Scalar<i8>>());
    EXPECT_EQ(2_i, new_lhs->As<core::constant::Scalar<i8>>()->ValueAs<i8>());

    auto new_rhs = c->RHS()->As<core::ir::Constant>()->Value();
    ASSERT_TRUE(new_rhs->Is<core::constant::Scalar<i8>>());
    EXPECT_EQ(4_i, new_rhs->As<core::constant::Scalar<i8>>()->ValueAs<i8>());
}

TEST_F(IR_MslBinaryTest, MatchOverloadFromDialect) {
    auto* v = b.FunctionParam("v", ty.i8());
    auto* func = b.Function("foo", ty.i8());
    func->SetParams({v});
    b.Append(func->Block(), [&] {
        auto* result = b.Binary<msl::ir::Binary>(core::BinaryOp::kAdd, mod.Types().i8(), v, v);
        b.Return(func, result);
    });

    core::ir::Capabilities caps;
    caps.Add(core::ir::Capability::kAllow8BitIntegers);
    auto res = core::ir::Validate(mod, caps);
    EXPECT_EQ(res, Success) << res.Failure().reason.Str();
}

TEST_F(IR_MslBinaryTest, DoesNotMatchOverloadFromCore) {
    auto* v = b.FunctionParam("v", ty.i32());
    auto* func = b.Function("foo", ty.i32());
    func->SetParams({v});
    b.Append(func->Block(), [&] {
        auto* result = b.Binary<msl::ir::Binary>(core::BinaryOp::kAdd, mod.Types().i32(), v, v);
        b.Return(func, result);
    });

    auto res = core::ir::Validate(mod);
    ASSERT_NE(res, Success);
    EXPECT_EQ(res.Failure().reason.Str(),
              R"(:3:5 error: binary: no matching overload for 'operator + (i32, i32)'

1 candidate operator:
 • 'operator + (T  ✗ , T  ✗ ) -> T' where:
      ✗  'T' is 'i8' or 'u8'

    %3:i32 = add %v, %v
    ^^^^^^^^^^^^^^^^^^^

:2:3 note: in block
  $B1: {
  ^^^

note: # Disassembly
%foo = func(%v:i32):i32 {
  $B1: {
    %3:i32 = add %v, %v
    ret %3
  }
}
)");
}

}  // namespace
}  // namespace tint::msl::ir
