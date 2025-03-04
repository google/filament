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

#include "src/tint/lang/hlsl/ir/member_builtin_call.h"

#include "gtest/gtest.h"

#include "src/tint/lang/core/fluent_types.h"
#include "src/tint/lang/core/ir/ir_helper_test.h"
#include "src/tint/lang/core/ir/validator.h"
#include "src/tint/lang/core/number.h"
#include "src/tint/lang/core/type/sampled_texture.h"
#include "src/tint/lang/core/type/sampler.h"
#include "src/tint/lang/core/type/texture_dimension.h"
#include "src/tint/lang/core/type/vector.h"
#include "src/tint/lang/hlsl/builtin_fn.h"
#include "src/tint/lang/hlsl/type/byte_address_buffer.h"
#include "src/tint/utils/result/result.h"

using namespace tint::core::fluent_types;     // NOLINT
using namespace tint::core::number_suffixes;  // NOLINT

namespace tint::hlsl::ir {
namespace {

using IR_HlslMemberBuiltinCallTest = core::ir::IRTestHelper;

TEST_F(IR_HlslMemberBuiltinCallTest, Clone) {
    auto* buf = ty.Get<hlsl::type::ByteAddressBuffer>(core::Access::kReadWrite);

    auto* t = b.FunctionParam("t", buf);
    auto* builtin = b.MemberCall<MemberBuiltinCall>(mod.Types().u32(), BuiltinFn::kLoad, t, 2_u);

    auto* new_b = clone_ctx.Clone(builtin);

    EXPECT_NE(builtin, new_b);
    EXPECT_NE(builtin->Result(0), new_b->Result(0));
    EXPECT_EQ(mod.Types().u32(), new_b->Result(0)->Type());

    EXPECT_EQ(BuiltinFn::kLoad, new_b->Func());

    EXPECT_TRUE(new_b->Object()->Type()->Is<hlsl::type::ByteAddressBuffer>());

    auto args = new_b->Args();
    ASSERT_EQ(1u, args.Length());
    EXPECT_TRUE(args[0]->Type()->Is<core::type::U32>());
}

TEST_F(IR_HlslMemberBuiltinCallTest, DoesNotMatchNonMemberFunction) {
    auto* buf_ty = ty.Get<hlsl::type::ByteAddressBuffer>(core::Access::kRead);
    auto* t = b.Var("t", buf_ty);
    t->SetBindingPoint(0, 0);
    mod.root_block->Append(t);

    auto* func = b.Function("foo", ty.u32());
    b.Append(func->Block(), [&] {
        auto* builtin =
            b.MemberCall<MemberBuiltinCall>(mod.Types().u32(), BuiltinFn::kAsint, t, 2_u);
        b.Return(func, builtin);
    });

    auto res = core::ir::Validate(mod);
    ASSERT_NE(res, Success);
    EXPECT_EQ(
        res.Failure().reason.Str(),
        R"(:7:17 error: asint: no matching call to 'asint(hlsl.byte_address_buffer<read>, u32)'

    %3:u32 = %t.asint 2u
                ^^^^^

:6:3 note: in block
  $B2: {
  ^^^

note: # Disassembly
$B1: {  # root
  %t:hlsl.byte_address_buffer<read> = var @binding_point(0, 0)
}

%foo = func():u32 {
  $B2: {
    %3:u32 = %t.asint 2u
    ret %3
  }
}
)");
}

TEST_F(IR_HlslMemberBuiltinCallTest, DoesNotMatchIncorrectType) {
    auto* buf_ty = ty.Get<hlsl::type::ByteAddressBuffer>(core::Access::kRead);
    auto* t = b.Var("t", buf_ty);
    t->SetBindingPoint(0, 0);
    mod.root_block->Append(t);

    auto* func = b.Function("foo", ty.u32());
    b.Append(func->Block(), [&] {
        auto* builtin =
            b.MemberCall<MemberBuiltinCall>(mod.Types().u32(), BuiltinFn::kStore, t, 2_u, 2_u);
        b.Return(func, builtin);
    });

    auto res = core::ir::Validate(mod);
    ASSERT_NE(res, Success);
    EXPECT_EQ(
        res.Failure().reason.Str(),
        R"(:7:17 error: Store: no matching call to 'Store(hlsl.byte_address_buffer<read>, u32, u32)'

1 candidate function:
 • 'Store(byte_address_buffer<write' or 'read_write>  ✗ , offset: u32  ✓ , value: u32  ✓ )'

    %3:u32 = %t.Store 2u, 2u
                ^^^^^

:6:3 note: in block
  $B2: {
  ^^^

note: # Disassembly
$B1: {  # root
  %t:hlsl.byte_address_buffer<read> = var @binding_point(0, 0)
}

%foo = func():u32 {
  $B2: {
    %3:u32 = %t.Store 2u, 2u
    ret %3
  }
}
)");
}

}  // namespace
}  // namespace tint::hlsl::ir
