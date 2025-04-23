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

using namespace tint::core::fluent_types;     // NOLINT
using namespace tint::core::number_suffixes;  // NOLINT

namespace tint::msl::ir {
namespace {

using IR_MslMemberBuiltinCallTest = core::ir::IRTestHelper;

TEST_F(IR_MslMemberBuiltinCallTest, Clone) {
    auto* t = b.FunctionParam("t", ty.sampled_texture(core::type::TextureDimension::k2d, ty.f32()));
    auto* s = b.FunctionParam("s", ty.sampler());
    auto* coords = b.FunctionParam("coords", ty.vec2<f32>());
    auto* builtin =
        b.MemberCall<MemberBuiltinCall>(mod.Types().void_(), BuiltinFn::kSample, t, s, coords);

    auto* new_b = clone_ctx.Clone(builtin);

    EXPECT_NE(builtin, new_b);
    EXPECT_NE(builtin->Result(), new_b->Result());
    EXPECT_EQ(mod.Types().void_(), new_b->Result()->Type());

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
        res.Failure().reason,
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

TEST_F(IR_MslMemberBuiltinCallTest, Valid) {
    auto* t = b.FunctionParam("t", ty.sampled_texture(core::type::TextureDimension::k2d, ty.f32()));
    auto* func = b.Function("foo", ty.void_());
    func->SetParams({t});
    b.Append(func->Block(), [&] {
        b.MemberCall<MemberBuiltinCall>(mod.Types().u32(), BuiltinFn::kGetWidth, t, 0_u);
        b.Return(func);
    });

    auto res = core::ir::Validate(mod);
    ASSERT_EQ(res, Success);
}

TEST_F(IR_MslMemberBuiltinCallTest, MissingResults) {
    auto* t = b.FunctionParam("t", ty.sampled_texture(core::type::TextureDimension::k2d, ty.f32()));
    auto* func = b.Function("foo", ty.void_());
    func->SetParams({t});
    b.Append(func->Block(), [&] {
        auto* m = b.MemberCall<MemberBuiltinCall>(mod.Types().u32(), BuiltinFn::kGetWidth, t, 0_u);
        m->ClearResults();
        b.Return(func);
    });

    auto res = core::ir::Validate(mod);
    ASSERT_NE(res, Success);
    EXPECT_EQ(res.Failure().reason,
              R"(:3:16 error: get_width: expected exactly 1 results, got 0
    undef = %t.get_width 0u
               ^^^^^^^^^

:2:3 note: in block
  $B1: {
  ^^^

note: # Disassembly
%foo = func(%t:texture_2d<f32>):void {
  $B1: {
    undef = %t.get_width 0u
    ret
  }
}
)");
}

TEST_F(IR_MslMemberBuiltinCallTest, TooFewArgs) {
    auto* t = b.FunctionParam("t", ty.sampled_texture(core::type::TextureDimension::k2d, ty.f32()));
    auto* func = b.Function("foo", ty.void_());
    func->SetParams({t});
    b.Append(func->Block(), [&] {
        b.MemberCall<MemberBuiltinCall>(mod.Types().u32(), BuiltinFn::kGetWidth, t);
        b.Return(func);
    });

    auto res = core::ir::Validate(mod);
    ASSERT_NE(res, Success);
    EXPECT_EQ(res.Failure().reason,
              R"(:3:17 error: get_width: no matching call to 'get_width(texture_2d<f32>)'

16 candidate functions:
 • 'get_width(texture: texture_depth_multisampled_2d  ✗ ) -> u32'
 • 'get_width(texture: texture_storage_1d<F, A>  ✗ ) -> u32'
 • 'get_width(texture: texture_1d<T>  ✗ ) -> u32' where:
      ✗  'T' is 'f32', 'i32' or 'u32'
 • 'get_width(texture: texture_2d<T>  ✓ , u32  ✗ ) -> u32' where:
      ✓  'T' is 'f32', 'i32' or 'u32'
 • 'get_width(texture: texture_multisampled_2d<T>  ✗ ) -> u32' where:
      ✗  'T' is 'f32', 'i32' or 'u32'
 • 'get_width(texture: texture_depth_2d  ✗ , u32  ✗ ) -> u32'
 • 'get_width(texture: texture_depth_2d_array  ✗ , u32  ✗ ) -> u32'
 • 'get_width(texture: texture_depth_cube  ✗ , u32  ✗ ) -> u32'
 • 'get_width(texture: texture_depth_cube_array  ✗ , u32  ✗ ) -> u32'
 • 'get_width(texture: texture_storage_2d<F, A>  ✗ , u32  ✗ ) -> u32'
 • 'get_width(texture: texture_storage_2d_array<F, A>  ✗ , u32  ✗ ) -> u32'
 • 'get_width(texture: texture_storage_3d<F, A>  ✗ , u32  ✗ ) -> u32'
 • 'get_width(texture: texture_2d_array<T>  ✗ , u32  ✗ ) -> u32' where:
      ✗  'T' is 'f32', 'i32' or 'u32'
 • 'get_width(texture: texture_3d<T>  ✗ , u32  ✗ ) -> u32' where:
      ✗  'T' is 'f32', 'i32' or 'u32'
 • 'get_width(texture: texture_cube<T>  ✗ , u32  ✗ ) -> u32' where:
      ✗  'T' is 'f32', 'i32' or 'u32'
 • 'get_width(texture: texture_cube_array<T>  ✗ , u32  ✗ ) -> u32' where:
      ✗  'T' is 'f32', 'i32' or 'u32'

    %3:u32 = %t.get_width
                ^^^^^^^^^

:2:3 note: in block
  $B1: {
  ^^^

note: # Disassembly
%foo = func(%t:texture_2d<f32>):void {
  $B1: {
    %3:u32 = %t.get_width
    ret
  }
}
)");
}

TEST_F(IR_MslMemberBuiltinCallTest, TooManyArgs) {
    auto* t = b.FunctionParam("t", ty.sampled_texture(core::type::TextureDimension::k2d, ty.f32()));
    auto* func = b.Function("foo", ty.void_());
    func->SetParams({t});
    b.Append(func->Block(), [&] {
        b.MemberCall<MemberBuiltinCall>(mod.Types().u32(), BuiltinFn::kGetWidth, t, 0_u, 1_u, 2_u);
        b.Return(func);
    });

    auto res = core::ir::Validate(mod);
    ASSERT_NE(res, Success);
    EXPECT_EQ(
        res.Failure().reason,
        R"(:3:17 error: get_width: no matching call to 'get_width(texture_2d<f32>, u32, u32, u32)'

16 candidate functions:
 • 'get_width(texture: texture_2d<T>  ✓ , u32  ✓ ) -> u32' where:
      ✗  overload expects 2 arguments, call passed 4 arguments
      ✓  'T' is 'f32', 'i32' or 'u32'
 • 'get_width(texture: texture_depth_2d  ✗ , u32  ✓ ) -> u32'
 • 'get_width(texture: texture_depth_2d_array  ✗ , u32  ✓ ) -> u32'
 • 'get_width(texture: texture_depth_cube  ✗ , u32  ✓ ) -> u32'
 • 'get_width(texture: texture_depth_cube_array  ✗ , u32  ✓ ) -> u32'
 • 'get_width(texture: texture_storage_2d<F, A>  ✗ , u32  ✓ ) -> u32'
 • 'get_width(texture: texture_storage_2d_array<F, A>  ✗ , u32  ✓ ) -> u32'
 • 'get_width(texture: texture_storage_3d<F, A>  ✗ , u32  ✓ ) -> u32'
 • 'get_width(texture: texture_2d_array<T>  ✗ , u32  ✓ ) -> u32' where:
      ✗  'T' is 'f32', 'i32' or 'u32'
 • 'get_width(texture: texture_3d<T>  ✗ , u32  ✓ ) -> u32' where:
      ✗  'T' is 'f32', 'i32' or 'u32'
 • 'get_width(texture: texture_cube<T>  ✗ , u32  ✓ ) -> u32' where:
      ✗  'T' is 'f32', 'i32' or 'u32'
 • 'get_width(texture: texture_cube_array<T>  ✗ , u32  ✓ ) -> u32' where:
      ✗  'T' is 'f32', 'i32' or 'u32'
 • 'get_width(texture: texture_depth_multisampled_2d  ✗ ) -> u32'
 • 'get_width(texture: texture_storage_1d<F, A>  ✗ ) -> u32'
 • 'get_width(texture: texture_1d<T>  ✗ ) -> u32' where:
      ✗  'T' is 'f32', 'i32' or 'u32'
 • 'get_width(texture: texture_multisampled_2d<T>  ✗ ) -> u32' where:
      ✗  'T' is 'f32', 'i32' or 'u32'

    %3:u32 = %t.get_width 0u, 1u, 2u
                ^^^^^^^^^

:2:3 note: in block
  $B1: {
  ^^^

note: # Disassembly
%foo = func(%t:texture_2d<f32>):void {
  $B1: {
    %3:u32 = %t.get_width 0u, 1u, 2u
    ret
  }
}
)");
}

}  // namespace
}  // namespace tint::msl::ir
