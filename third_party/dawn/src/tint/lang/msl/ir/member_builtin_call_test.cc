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

#include "src/tint/lang/msl/ir/member_builtin_call.h"

#include "gtest/gtest.h"

#include "src/tint/lang/core/fluent_types.h"
#include "src/tint/lang/core/ir/ir_helper_test.h"
#include "src/tint/lang/core/ir/validator.h"
#include "src/tint/lang/core/number.h"
#include "src/tint/lang/core/type/sampled_texture.h"
#include "src/tint/lang/core/type/sampler.h"
#include "src/tint/lang/core/type/texture_dimension.h"
#include "src/tint/lang/core/type/vector.h"
#include "src/tint/lang/msl/builtin_fn.h"
#include "src/tint/utils/result/result.h"

using namespace tint::core::fluent_types;     // NOLINT
using namespace tint::core::number_suffixes;  // NOLINT

namespace tint::msl::ir {
namespace {

using IR_MslMemberBuiltinCallTest = core::ir::IRTestHelper;

TEST_F(IR_MslMemberBuiltinCallTest, Clone) {
    auto* t = b.FunctionParam(
        "t", ty.Get<core::type::SampledTexture>(core::type::TextureDimension::k2d, ty.f32()));
    auto* s = b.FunctionParam("s", ty.sampler());
    auto* coords = b.FunctionParam("coords", ty.vec2<f32>());
    auto* builtin =
        b.MemberCall<MemberBuiltinCall>(mod.Types().void_(), BuiltinFn::kSample, t, s, coords);

    auto* new_b = clone_ctx.Clone(builtin);

    EXPECT_NE(builtin, new_b);
    EXPECT_NE(builtin->Result(0), new_b->Result(0));
    EXPECT_EQ(mod.Types().void_(), new_b->Result(0)->Type());

    EXPECT_EQ(BuiltinFn::kSample, new_b->Func());

    EXPECT_TRUE(new_b->Object()->Type()->Is<core::type::SampledTexture>());

    auto args = new_b->Args();
    ASSERT_EQ(2u, args.Length());
    EXPECT_TRUE(args[0]->Type()->Is<core::type::Sampler>());
    EXPECT_TRUE(args[1]->Type()->Is<core::type::Vector>());
}

TEST_F(IR_MslMemberBuiltinCallTest, DoesNotMatchNonMemberFunction) {
    auto* a = b.FunctionParam("t", ty.ptr<workgroup>(ty.atomic<u32>()));
    auto* func = b.Function("foo", ty.u32());
    func->SetParams({a});
    b.Append(func->Block(), [&] {
        auto* result =
            b.MemberCall<MemberBuiltinCall>(ty.u32(), msl::BuiltinFn::kAtomicLoadExplicit, a, 0_u);
        b.Return(func, result);
    });

    auto res = core::ir::Validate(mod);
    ASSERT_NE(res, Success);
    EXPECT_EQ(
        res.Failure().reason.Str(),
        R"(:3:17 error: atomic_load_explicit: no matching call to 'atomic_load_explicit(ptr<workgroup, atomic<u32>, read_write>, u32)'

    %3:u32 = %t.atomic_load_explicit 0u
                ^^^^^^^^^^^^^^^^^^^^

:2:3 note: in block
  $B1: {
  ^^^

note: # Disassembly
%foo = func(%t:ptr<workgroup, atomic<u32>, read_write>):u32 {
  $B1: {
    %3:u32 = %t.atomic_load_explicit 0u
    ret %3
  }
}
)");
}

}  // namespace
}  // namespace tint::msl::ir
