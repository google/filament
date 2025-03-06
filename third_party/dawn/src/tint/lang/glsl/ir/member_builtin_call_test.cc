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

#include "src/tint/lang/glsl/ir/member_builtin_call.h"

#include "gtest/gtest.h"

#include "src/tint/lang/core/fluent_types.h"
#include "src/tint/lang/core/ir/ir_helper_test.h"
#include "src/tint/lang/core/ir/validator.h"
#include "src/tint/lang/glsl/builtin_fn.h"
#include "src/tint/utils/result/result.h"

using namespace tint::core::fluent_types;     // NOLINT
using namespace tint::core::number_suffixes;  // NOLINT

namespace tint::glsl::ir {
namespace {

using IR_GlslMemberBuiltinCallTest = core::ir::IRTestHelper;

TEST_F(IR_GlslMemberBuiltinCallTest, Clone) {
    auto* sb = ty.Struct(mod.symbols.New("SB"), {
                                                    {mod.symbols.New("a"), ty.array<u32>()},
                                                });
    auto* var = b.Var("v", storage, sb, core::Access::kReadWrite);
    var->SetBindingPoint(0, 0);
    b.ir.root_block->Append(var);

    auto* access = b.Access(ty.ptr<storage, array<u32>, read_write>(), var, 0_u);
    auto* builtin = b.MemberCall<MemberBuiltinCall>(mod.Types().i32(), BuiltinFn::kLength, access);

    auto* new_b = clone_ctx.Clone(builtin);

    EXPECT_NE(builtin, new_b);
    EXPECT_NE(builtin->Result(0), new_b->Result(0));
    EXPECT_EQ(mod.Types().i32(), new_b->Result(0)->Type());

    EXPECT_EQ(BuiltinFn::kLength, new_b->Func());
    EXPECT_TRUE(new_b->Object()->Type()->UnwrapPtr()->Is<core::type::Array>());

    auto args = new_b->Args();
    ASSERT_EQ(0u, args.Length());
}

TEST_F(IR_GlslMemberBuiltinCallTest, DoesNotMatchIncorrectType) {
    auto* sb = ty.Struct(mod.symbols.New("SB"), {
                                                    {mod.symbols.New("a"), ty.u32()},
                                                });
    auto* var = b.Var("v", storage, sb, core::Access::kReadWrite);
    var->SetBindingPoint(0, 0);
    b.ir.root_block->Append(var);

    auto* func = b.Function("foo", ty.void_(), core::ir::Function::PipelineStage::kFragment);
    b.Append(func->Block(), [&] {
        auto* access = b.Access(ty.ptr<storage, u32, read_write>(), var, 0_u);
        b.Let("x", b.MemberCall<MemberBuiltinCall>(mod.Types().i32(), BuiltinFn::kLength, access));

        b.Return(func);
    });

    auto res = core::ir::Validate(mod);
    ASSERT_NE(res, Success);
    EXPECT_EQ(res.Failure().reason.Str(),
              R"(:12:17 error: length: no matching call to 'length(ptr<storage, u32, read_write>)'

1 candidate function:
 • 'length(ptr<storage, array<T>, A>  ✗ ) -> i32'

    %4:i32 = %3.length
                ^^^^^^

:10:3 note: in block
  $B2: {
  ^^^

note: # Disassembly
SB = struct @align(4) {
  a:u32 @offset(0)
}

$B1: {  # root
  %v:ptr<storage, SB, read_write> = var @binding_point(0, 0)
}

%foo = @fragment func():void {
  $B2: {
    %3:ptr<storage, u32, read_write> = access %v, 0u
    %4:i32 = %3.length
    %x:i32 = let %4
    ret
  }
}
)");
}

}  // namespace
}  // namespace tint::glsl::ir
