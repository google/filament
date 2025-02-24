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

#include "src/tint/lang/msl/ir/builtin_call.h"

#include "gtest/gtest.h"
#include "src/tint/lang/core/fluent_types.h"
#include "src/tint/lang/core/ir/ir_helper_test.h"
#include "src/tint/lang/core/ir/validator.h"
#include "src/tint/lang/core/type/sampled_texture.h"
#include "src/tint/lang/core/type/texture_dimension.h"
#include "src/tint/utils/result/result.h"

using namespace tint::core::fluent_types;  // NOLINT

namespace tint::msl::ir {
namespace {

using namespace tint::core::number_suffixes;  // NOLINT
                                              //
using IR_MslBuiltinCallTest = core::ir::IRTestHelper;

TEST_F(IR_MslBuiltinCallTest, Clone) {
    auto* builtin = b.Call<BuiltinCall>(mod.Types().void_(), BuiltinFn::kThreadgroupBarrier, 0_u);

    auto* new_b = clone_ctx.Clone(builtin);

    EXPECT_NE(builtin, new_b);
    EXPECT_NE(builtin->Result(0), new_b->Result(0));
    EXPECT_EQ(mod.Types().void_(), new_b->Result(0)->Type());

    EXPECT_EQ(BuiltinFn::kThreadgroupBarrier, new_b->Func());

    auto args = new_b->Args();
    EXPECT_EQ(1u, args.Length());

    auto* val0 = args[0]->As<core::ir::Constant>()->Value();
    EXPECT_EQ(0_u, val0->As<core::constant::Scalar<core::u32>>()->ValueAs<core::u32>());
}

TEST_F(IR_MslBuiltinCallTest, DoesNotMatchMemberFunction) {
    auto* t = b.FunctionParam(
        "t", ty.Get<core::type::SampledTexture>(core::type::TextureDimension::k2d, ty.f32()));
    auto* s = b.FunctionParam("s", ty.sampler());
    auto* coords = b.FunctionParam("coords", ty.vec2<f32>());
    auto* func = b.Function("foo", ty.vec4<f32>());
    func->SetParams({t, s, coords});
    b.Append(func->Block(), [&] {
        auto* result = b.Call<BuiltinCall>(ty.vec4<f32>(), msl::BuiltinFn::kSample, t, s, coords);
        b.Return(func, result);
    });

    auto res = core::ir::Validate(mod);
    ASSERT_NE(res, Success);
    EXPECT_EQ(
        res.Failure().reason.Str(),
        R"(:3:20 error: msl.sample: no matching call to 'msl.sample(texture_2d<f32>, sampler, vec2<f32>)'

    %5:vec4<f32> = msl.sample %t, %s, %coords
                   ^^^^^^^^^^

:2:3 note: in block
  $B1: {
  ^^^

note: # Disassembly
%foo = func(%t:texture_2d<f32>, %s:sampler, %coords:vec2<f32>):vec4<f32> {
  $B1: {
    %5:vec4<f32> = msl.sample %t, %s, %coords
    ret %5
  }
}
)");
}

}  // namespace
}  // namespace tint::msl::ir
