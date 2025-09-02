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

#include "src/tint/lang/core/ir/transform/direct_variable_access.h"

#include <utility>

#include "src/tint/lang/core/ir/transform/helper_test.h"
#include "src/tint/lang/core/type/array.h"
#include "src/tint/lang/core/type/binding_array.h"
#include "src/tint/lang/core/type/matrix.h"
#include "src/tint/lang/core/type/pointer.h"
#include "src/tint/lang/core/type/sampled_texture.h"
#include "src/tint/lang/core/type/sampler.h"
#include "src/tint/lang/core/type/struct.h"

namespace tint::core::ir::transform {
namespace {

using namespace tint::core::fluent_types;     // NOLINT
using namespace tint::core::number_suffixes;  // NOLINT

namespace {

static constexpr DirectVariableAccessOptions kTransformHandle = {
    /* transform_private */ false,
    /* transform_function */ false,
    /* transform_handle */ true,
};

static constexpr DirectVariableAccessOptions kTransformPrivate = {
    /* transform_private */ true,
    /* transform_function */ false,
    /* transform_handle */ false,
};

static constexpr DirectVariableAccessOptions kTransformFunction = {
    /* transform_private */ false,
    /* transform_function */ true,
    /* transform_handle */ false,
};

}  // namespace

////////////////////////////////////////////////////////////////////////////////
// remove uncalled
////////////////////////////////////////////////////////////////////////////////
namespace remove_uncalled {

using IR_DirectVariableAccessTest_RemoveUncalled = TransformTest;

TEST_F(IR_DirectVariableAccessTest_RemoveUncalled, PtrUniform) {
    b.Append(b.ir.root_block, [&] { b.Var<private_>("keep_me", 42_i); });

    auto* u = b.Function("u", ty.i32());
    auto* p = b.FunctionParam("p", ty.ptr<uniform, i32>());
    u->SetParams({
        b.FunctionParam("pre", ty.i32()),
        p,
        b.FunctionParam("post", ty.i32()),
    });
    b.Append(u->Block(), [&] { b.Return(u, b.Load(p)); });

    auto* src = R"(
$B1: {  # root
  %keep_me:ptr<private, i32, read_write> = var 42i
}

%u = func(%pre:i32, %p:ptr<uniform, i32, read>, %post:i32):i32 {
  $B2: {
    %6:i32 = load %p
    ret %6
  }
}
)";

    EXPECT_EQ(src, str());

    auto* expect = R"(
$B1: {  # root
  %keep_me:ptr<private, i32, read_write> = var 42i
}

)";

    Run(DirectVariableAccess, DirectVariableAccessOptions{});

    EXPECT_EQ(expect, str());
}

TEST_F(IR_DirectVariableAccessTest_RemoveUncalled, PtrStorage) {
    b.Append(b.ir.root_block, [&] { b.Var<private_>("keep_me", 42_i); });

    auto* s = b.Function("s", ty.i32());
    auto* p = b.FunctionParam("p", ty.ptr<storage, i32, read>());
    s->SetParams({
        b.FunctionParam("pre", ty.i32()),
        p,
        b.FunctionParam("post", ty.i32()),
    });
    b.Append(s->Block(), [&] { b.Return(s, b.Load(p)); });

    auto* src = R"(
$B1: {  # root
  %keep_me:ptr<private, i32, read_write> = var 42i
}

%s = func(%pre:i32, %p:ptr<storage, i32, read>, %post:i32):i32 {
  $B2: {
    %6:i32 = load %p
    ret %6
  }
}
)";

    EXPECT_EQ(src, str());

    auto* expect = R"(
$B1: {  # root
  %keep_me:ptr<private, i32, read_write> = var 42i
}

)";

    Run(DirectVariableAccess, DirectVariableAccessOptions{});

    EXPECT_EQ(expect, str());
}

TEST_F(IR_DirectVariableAccessTest_RemoveUncalled, PtrWorkgroup) {
    b.Append(b.ir.root_block, [&] { b.Var<private_>("keep_me", 42_i); });

    auto* w = b.Function("w", ty.i32());
    auto* p = b.FunctionParam("p", ty.ptr<workgroup, i32>());
    w->SetParams({
        b.FunctionParam("pre", ty.i32()),
        p,
        b.FunctionParam("post", ty.i32()),
    });
    b.Append(w->Block(), [&] { b.Return(w, b.Load(p)); });

    auto* src = R"(
$B1: {  # root
  %keep_me:ptr<private, i32, read_write> = var 42i
}

%w = func(%pre:i32, %p:ptr<workgroup, i32, read_write>, %post:i32):i32 {
  $B2: {
    %6:i32 = load %p
    ret %6
  }
}
)";

    EXPECT_EQ(src, str());

    auto* expect = R"(
$B1: {  # root
  %keep_me:ptr<private, i32, read_write> = var 42i
}

)";

    Run(DirectVariableAccess, DirectVariableAccessOptions{});

    EXPECT_EQ(expect, str());
}

TEST_F(IR_DirectVariableAccessTest_RemoveUncalled, PtrPrivate_Disabled) {
    b.Append(b.ir.root_block, [&] { b.Var<private_>("keep_me", 42_i); });

    auto* f = b.Function("f", ty.i32());
    auto* p = b.FunctionParam("p", ty.ptr<private_, i32>());
    f->SetParams({
        b.FunctionParam("pre", ty.i32()),
        p,
        b.FunctionParam("post", ty.i32()),
    });
    b.Append(f->Block(), [&] { b.Return(f, b.Load(p)); });

    auto* src = R"(
$B1: {  # root
  %keep_me:ptr<private, i32, read_write> = var 42i
}

%f = func(%pre:i32, %p:ptr<private, i32, read_write>, %post:i32):i32 {
  $B2: {
    %6:i32 = load %p
    ret %6
  }
}
)";

    EXPECT_EQ(src, str());

    auto* expect = src;

    Run(DirectVariableAccess, DirectVariableAccessOptions{});

    EXPECT_EQ(expect, str());
}

TEST_F(IR_DirectVariableAccessTest_RemoveUncalled, PtrPrivate_Enabled) {
    b.Append(b.ir.root_block, [&] { b.Var<private_>("keep_me", 42_i); });

    auto* f = b.Function("f", ty.i32());
    auto* p = b.FunctionParam("p", ty.ptr<private_, i32>());
    f->SetParams({
        b.FunctionParam("pre", ty.i32()),
        p,
        b.FunctionParam("post", ty.i32()),
    });
    b.Append(f->Block(), [&] { b.Return(f, b.Load(p)); });

    auto* src = R"(
$B1: {  # root
  %keep_me:ptr<private, i32, read_write> = var 42i
}

%f = func(%pre:i32, %p:ptr<private, i32, read_write>, %post:i32):i32 {
  $B2: {
    %6:i32 = load %p
    ret %6
  }
}
)";

    EXPECT_EQ(src, str());

    auto* expect = R"(
$B1: {  # root
  %keep_me:ptr<private, i32, read_write> = var 42i
}

)";
    Run(DirectVariableAccess, kTransformPrivate);

    EXPECT_EQ(expect, str());
}

TEST_F(IR_DirectVariableAccessTest_RemoveUncalled, PtrFunction_Disabled) {
    b.Append(b.ir.root_block, [&] { b.Var<private_>("keep_me", 42_i); });

    auto* f = b.Function("f", ty.i32());
    auto* p = b.FunctionParam("p", ty.ptr<function, i32>());
    f->SetParams({
        b.FunctionParam("pre", ty.i32()),
        p,
        b.FunctionParam("post", ty.i32()),
    });
    b.Append(f->Block(), [&] { b.Return(f, b.Load(p)); });

    auto* src = R"(
$B1: {  # root
  %keep_me:ptr<private, i32, read_write> = var 42i
}

%f = func(%pre:i32, %p:ptr<function, i32, read_write>, %post:i32):i32 {
  $B2: {
    %6:i32 = load %p
    ret %6
  }
}
)";

    EXPECT_EQ(src, str());

    auto* expect = src;

    Run(DirectVariableAccess, DirectVariableAccessOptions{});

    EXPECT_EQ(expect, str());
}

TEST_F(IR_DirectVariableAccessTest_RemoveUncalled, PtrFunction_Enabled) {
    b.Append(b.ir.root_block, [&] { b.Var<private_>("keep_me", 42_i); });

    auto* f = b.Function("f", ty.i32());
    auto* p = b.FunctionParam("p", ty.ptr<function, i32>());
    f->SetParams({
        b.FunctionParam("pre", ty.i32()),
        p,
        b.FunctionParam("post", ty.i32()),
    });
    b.Append(f->Block(), [&] { b.Return(f, b.Load(p)); });

    auto* src = R"(
$B1: {  # root
  %keep_me:ptr<private, i32, read_write> = var 42i
}

%f = func(%pre:i32, %p:ptr<function, i32, read_write>, %post:i32):i32 {
  $B2: {
    %6:i32 = load %p
    ret %6
  }
}
)";
    EXPECT_EQ(src, str());

    auto* expect = R"(
$B1: {  # root
  %keep_me:ptr<private, i32, read_write> = var 42i
}

)";

    Run(DirectVariableAccess, kTransformFunction);

    EXPECT_EQ(expect, str());
}

TEST_F(IR_DirectVariableAccessTest_RemoveUncalled, HandleTexture_Disabled) {
    b.Append(b.ir.root_block, [&] { b.Var<private_>("keep_me", 42_i); });

    auto* f = b.Function("f", ty.i32());
    auto* p = b.FunctionParam("p", ty.sampled_texture(core::type::TextureDimension::k1d, ty.f32()));
    f->SetParams({
        b.FunctionParam("pre", ty.i32()),
        p,
        b.FunctionParam("post", ty.i32()),
    });
    b.Append(f->Block(), [&] { b.Return(f, b.Constant(2_i)); });

    auto* src = R"(
$B1: {  # root
  %keep_me:ptr<private, i32, read_write> = var 42i
}

%f = func(%pre:i32, %p:texture_1d<f32>, %post:i32):i32 {
  $B2: {
    ret 2i
  }
}
)";

    EXPECT_EQ(src, str());

    auto* expect = src;

    Run(DirectVariableAccess, DirectVariableAccessOptions{});

    EXPECT_EQ(expect, str());
}

TEST_F(IR_DirectVariableAccessTest_RemoveUncalled, HandleTexture_Enabled) {
    b.Append(b.ir.root_block, [&] { b.Var<private_>("keep_me", 42_i); });

    auto* f = b.Function("f", ty.u32());
    auto* p = b.FunctionParam("p", ty.sampled_texture(core::type::TextureDimension::k1d, ty.f32()));
    f->SetParams({
        b.FunctionParam("pre", ty.i32()),
        p,
        b.FunctionParam("post", ty.i32()),
    });
    b.Append(f->Block(),
             [&] { b.Return(f, b.Call(ty.u32(), core::BuiltinFn::kTextureDimensions, p, 0_u)); });

    auto* src = R"(
$B1: {  # root
  %keep_me:ptr<private, i32, read_write> = var 42i
}

%f = func(%pre:i32, %p:texture_1d<f32>, %post:i32):u32 {
  $B2: {
    %6:u32 = textureDimensions %p, 0u
    ret %6
  }
}
)";

    EXPECT_EQ(src, str());

    auto* expect = R"(
$B1: {  # root
  %keep_me:ptr<private, i32, read_write> = var 42i
}

)";
    Run(DirectVariableAccess, kTransformHandle);

    EXPECT_EQ(expect, str());
}

}  // namespace remove_uncalled

////////////////////////////////////////////////////////////////////////////////
// pointer chains
////////////////////////////////////////////////////////////////////////////////
namespace pointer_chains_tests {

using IR_DirectVariableAccessTest_PtrChains = TransformTest;

TEST_F(IR_DirectVariableAccessTest_PtrChains, ConstantIndices) {
    Var* U = nullptr;
    b.Append(b.ir.root_block,
             [&] {  //
                 U = b.Var<uniform, array<array<array<vec4<i32>, 8>, 8>, 8>>("U");
                 U->SetBindingPoint(0, 0);
             });

    auto* fn_a = b.Function("a", ty.vec4<i32>());
    auto* fn_a_p = b.FunctionParam("p", ty.ptr<uniform, vec4<i32>>());
    fn_a->SetParams({
        b.FunctionParam("pre", ty.i32()),
        fn_a_p,
        b.FunctionParam("post", ty.i32()),
    });
    b.Append(fn_a->Block(), [&] { b.Return(fn_a, b.Load(fn_a_p)); });

    auto* fn_b = b.Function("b", ty.void_());
    b.Append(fn_b->Block(), [&] {
        auto* p0 = b.Let("p0", U);
        auto* p1 = b.Access(ty.ptr<uniform, array<array<vec4<i32>, 8>, 8>>(), p0, 1_i);
        b.ir.SetName(p1, "p1");
        auto* p2 = b.Access(ty.ptr<uniform, array<vec4<i32>, 8>>(), p1, 2_i);
        b.ir.SetName(p2, "p2");
        auto* p3 = b.Access(ty.ptr<uniform, vec4<i32>>(), p2, 3_i);
        b.ir.SetName(p3, "p3");
        b.Call(ty.vec4<i32>(), fn_a, 10_i, p3, 20_i);
        b.Return(fn_b);
    });

    auto* fn_c = b.Function("c", ty.void_());
    auto* fn_c_p = b.FunctionParam("p", ty.ptr<uniform, array<array<array<vec4<i32>, 8>, 8>, 8>>());
    fn_c->SetParams({fn_c_p});
    b.Append(fn_c->Block(), [&] {
        auto* p0 = b.Let("p0", fn_c_p);
        auto* p1 = b.Access(ty.ptr<uniform, array<array<vec4<i32>, 8>, 8>>(), p0, 1_i);
        b.ir.SetName(p1, "p1");
        auto* p2 = b.Access(ty.ptr<uniform, array<vec4<i32>, 8>>(), p1, 2_i);
        b.ir.SetName(p2, "p2");
        auto* p3 = b.Access(ty.ptr<uniform, vec4<i32>>(), p2, 3_i);
        b.ir.SetName(p3, "p3");
        b.Call(ty.vec4<i32>(), fn_a, 10_i, p3, 20_i);
        b.Return(fn_c);
    });

    auto* fn_d = b.Function("d", ty.void_());
    b.Append(fn_d->Block(), [&] {
        b.Call(ty.void_(), fn_c, U);
        b.Return(fn_d);
    });

    auto* src = R"(
$B1: {  # root
  %U:ptr<uniform, array<array<array<vec4<i32>, 8>, 8>, 8>, read> = var undef @binding_point(0, 0)
}

%a = func(%pre:i32, %p:ptr<uniform, vec4<i32>, read>, %post:i32):vec4<i32> {
  $B2: {
    %6:vec4<i32> = load %p
    ret %6
  }
}
%b = func():void {
  $B3: {
    %p0:ptr<uniform, array<array<array<vec4<i32>, 8>, 8>, 8>, read> = let %U
    %p1:ptr<uniform, array<array<vec4<i32>, 8>, 8>, read> = access %p0, 1i
    %p2:ptr<uniform, array<vec4<i32>, 8>, read> = access %p1, 2i
    %p3:ptr<uniform, vec4<i32>, read> = access %p2, 3i
    %12:vec4<i32> = call %a, 10i, %p3, 20i
    ret
  }
}
%c = func(%p_1:ptr<uniform, array<array<array<vec4<i32>, 8>, 8>, 8>, read>):void {  # %p_1: 'p'
  $B4: {
    %p0_1:ptr<uniform, array<array<array<vec4<i32>, 8>, 8>, 8>, read> = let %p_1  # %p0_1: 'p0'
    %p1_1:ptr<uniform, array<array<vec4<i32>, 8>, 8>, read> = access %p0_1, 1i  # %p1_1: 'p1'
    %p2_1:ptr<uniform, array<vec4<i32>, 8>, read> = access %p1_1, 2i  # %p2_1: 'p2'
    %p3_1:ptr<uniform, vec4<i32>, read> = access %p2_1, 3i  # %p3_1: 'p3'
    %19:vec4<i32> = call %a, 10i, %p3_1, 20i
    ret
  }
}
%d = func():void {
  $B5: {
    %21:void = call %c, %U
    ret
  }
}
)";

    EXPECT_EQ(src, str());

    auto* expect =
        R"(
$B1: {  # root
  %U:ptr<uniform, array<array<array<vec4<i32>, 8>, 8>, 8>, read> = var undef @binding_point(0, 0)
}

%a = func(%pre:i32, %p_indices:array<u32, 3>, %post:i32):vec4<i32> {
  $B2: {
    %6:u32 = access %p_indices, 0u
    %7:u32 = access %p_indices, 1u
    %8:u32 = access %p_indices, 2u
    %9:ptr<uniform, vec4<i32>, read> = access %U, %6, %7, %8
    %10:vec4<i32> = load %9
    ret %10
  }
}
%b = func():void {
  $B3: {
    %12:u32 = convert 3i
    %13:u32 = convert 2i
    %14:u32 = convert 1i
    %15:array<u32, 3> = construct %14, %13, %12
    %16:vec4<i32> = call %a, 10i, %15, 20i
    ret
  }
}
%c = func():void {
  $B4: {
    %18:u32 = convert 3i
    %19:u32 = convert 2i
    %20:u32 = convert 1i
    %21:array<u32, 3> = construct %20, %19, %18
    %22:vec4<i32> = call %a, 10i, %21, 20i
    ret
  }
}
%d = func():void {
  $B5: {
    %24:void = call %c
    ret
  }
}
)";

    Run(DirectVariableAccess, DirectVariableAccessOptions{});

    EXPECT_EQ(expect, str());
}

TEST_F(IR_DirectVariableAccessTest_PtrChains, DynamicIndices) {
    Var* U = nullptr;
    Var* i = nullptr;
    b.Append(b.ir.root_block,
             [&] {  //
                 U = b.Var<uniform, array<array<array<vec4<i32>, 8>, 8>, 8>>("U");
                 U->SetBindingPoint(0, 0);
                 i = b.Var<private_, i32>("i");
             });

    auto* fn_first = b.Function("first", ty.i32());
    auto* fn_second = b.Function("second", ty.i32());
    auto* fn_third = b.Function("third", ty.i32());
    for (auto fn : {fn_first, fn_second, fn_third}) {
        b.Append(fn->Block(), [&] {
            b.Store(i, b.Add(ty.i32(), b.Load(i), 1_i));
            b.Return(fn, b.Load(i));
        });
    }

    auto* fn_a = b.Function("a", ty.vec4<i32>());
    auto* fn_a_p = b.FunctionParam("p", ty.ptr<uniform, vec4<i32>>());
    fn_a->SetParams({
        b.FunctionParam("pre", ty.i32()),
        fn_a_p,
        b.FunctionParam("post", ty.i32()),
    });
    b.Append(fn_a->Block(), [&] { b.Return(fn_a, b.Load(fn_a_p)); });

    auto* fn_b = b.Function("b", ty.void_());
    b.Append(fn_b->Block(), [&] {
        auto* p0 = b.Let("p0", U);
        auto* first = b.Call(fn_first);
        auto* p1 = b.Access(ty.ptr<uniform, array<array<vec4<i32>, 8>, 8>>(), p0, first);
        b.ir.SetName(p1, "p1");
        auto* second = b.Call(fn_second);
        auto* third = b.Call(fn_third);
        auto* p2 = b.Access(ty.ptr<uniform, vec4<i32>>(), p1, second, third);
        b.ir.SetName(p2, "p2");
        b.Call(ty.vec4<i32>(), fn_a, 10_i, p2, 20_i);
        b.Return(fn_b);
    });

    auto* fn_c = b.Function("c", ty.void_());
    auto* fn_c_p = b.FunctionParam("p", ty.ptr<uniform, array<array<array<vec4<i32>, 8>, 8>, 8>>());
    fn_c->SetParams({fn_c_p});
    b.Append(fn_c->Block(), [&] {
        auto* p0 = b.Let("p0", fn_c_p);
        auto* first = b.Call(fn_first);
        auto* p1 = b.Access(ty.ptr<uniform, array<array<vec4<i32>, 8>, 8>>(), p0, first);
        b.ir.SetName(p1, "p1");
        auto* second = b.Call(fn_second);
        auto* third = b.Call(fn_third);
        auto* p2 = b.Access(ty.ptr<uniform, vec4<i32>>(), p1, second, third);
        b.ir.SetName(p2, "p2");
        b.Call(ty.vec4<i32>(), fn_a, 10_i, p2, 20_i);
        b.Return(fn_c);
    });

    auto* fn_d = b.Function("d", ty.void_());
    b.Append(fn_d->Block(), [&] {
        b.Call(ty.void_(), fn_c, U);
        b.Return(fn_d);
    });

    auto* src = R"(
$B1: {  # root
  %U:ptr<uniform, array<array<array<vec4<i32>, 8>, 8>, 8>, read> = var undef @binding_point(0, 0)
  %i:ptr<private, i32, read_write> = var undef
}

%first = func():i32 {
  $B2: {
    %4:i32 = load %i
    %5:i32 = add %4, 1i
    store %i, %5
    %6:i32 = load %i
    ret %6
  }
}
%second = func():i32 {
  $B3: {
    %8:i32 = load %i
    %9:i32 = add %8, 1i
    store %i, %9
    %10:i32 = load %i
    ret %10
  }
}
%third = func():i32 {
  $B4: {
    %12:i32 = load %i
    %13:i32 = add %12, 1i
    store %i, %13
    %14:i32 = load %i
    ret %14
  }
}
%a = func(%pre:i32, %p:ptr<uniform, vec4<i32>, read>, %post:i32):vec4<i32> {
  $B5: {
    %19:vec4<i32> = load %p
    ret %19
  }
}
%b = func():void {
  $B6: {
    %p0:ptr<uniform, array<array<array<vec4<i32>, 8>, 8>, 8>, read> = let %U
    %22:i32 = call %first
    %p1:ptr<uniform, array<array<vec4<i32>, 8>, 8>, read> = access %p0, %22
    %24:i32 = call %second
    %25:i32 = call %third
    %p2:ptr<uniform, vec4<i32>, read> = access %p1, %24, %25
    %27:vec4<i32> = call %a, 10i, %p2, 20i
    ret
  }
}
%c = func(%p_1:ptr<uniform, array<array<array<vec4<i32>, 8>, 8>, 8>, read>):void {  # %p_1: 'p'
  $B7: {
    %p0_1:ptr<uniform, array<array<array<vec4<i32>, 8>, 8>, 8>, read> = let %p_1  # %p0_1: 'p0'
    %31:i32 = call %first
    %p1_1:ptr<uniform, array<array<vec4<i32>, 8>, 8>, read> = access %p0_1, %31  # %p1_1: 'p1'
    %33:i32 = call %second
    %34:i32 = call %third
    %p2_1:ptr<uniform, vec4<i32>, read> = access %p1_1, %33, %34  # %p2_1: 'p2'
    %36:vec4<i32> = call %a, 10i, %p2_1, 20i
    ret
  }
}
%d = func():void {
  $B8: {
    %38:void = call %c, %U
    ret
  }
}
)";

    EXPECT_EQ(src, str());

    auto* expect = R"(
$B1: {  # root
  %U:ptr<uniform, array<array<array<vec4<i32>, 8>, 8>, 8>, read> = var undef @binding_point(0, 0)
  %i:ptr<private, i32, read_write> = var undef
}

%first = func():i32 {
  $B2: {
    %4:i32 = load %i
    %5:i32 = add %4, 1i
    store %i, %5
    %6:i32 = load %i
    ret %6
  }
}
%second = func():i32 {
  $B3: {
    %8:i32 = load %i
    %9:i32 = add %8, 1i
    store %i, %9
    %10:i32 = load %i
    ret %10
  }
}
%third = func():i32 {
  $B4: {
    %12:i32 = load %i
    %13:i32 = add %12, 1i
    store %i, %13
    %14:i32 = load %i
    ret %14
  }
}
%a = func(%pre:i32, %p_indices:array<u32, 3>, %post:i32):vec4<i32> {
  $B5: {
    %19:u32 = access %p_indices, 0u
    %20:u32 = access %p_indices, 1u
    %21:u32 = access %p_indices, 2u
    %22:ptr<uniform, vec4<i32>, read> = access %U, %19, %20, %21
    %23:vec4<i32> = load %22
    ret %23
  }
}
%b = func():void {
  $B6: {
    %25:i32 = call %first
    %26:i32 = call %second
    %27:i32 = call %third
    %28:u32 = convert %26
    %29:u32 = convert %27
    %30:u32 = convert %25
    %31:array<u32, 3> = construct %30, %28, %29
    %32:vec4<i32> = call %a, 10i, %31, 20i
    ret
  }
}
%c = func():void {
  $B7: {
    %34:i32 = call %first
    %35:i32 = call %second
    %36:i32 = call %third
    %37:u32 = convert %35
    %38:u32 = convert %36
    %39:u32 = convert %34
    %40:array<u32, 3> = construct %39, %37, %38
    %41:vec4<i32> = call %a, 10i, %40, 20i
    ret
  }
}
%d = func():void {
  $B8: {
    %43:void = call %c
    ret
  }
}
)";

    Run(DirectVariableAccess, DirectVariableAccessOptions{});

    EXPECT_EQ(expect, str());
}

}  // namespace pointer_chains_tests

////////////////////////////////////////////////////////////////////////////////
// 'uniform' address space
////////////////////////////////////////////////////////////////////////////////
namespace uniform_as_tests {

using IR_DirectVariableAccessTest_UniformAS = TransformTest;

TEST_F(IR_DirectVariableAccessTest_UniformAS, Param_ptr_i32_read) {
    Var* U = nullptr;
    b.Append(b.ir.root_block,
             [&] {  //
                 U = b.Var<uniform, i32>("U");
                 U->SetBindingPoint(0, 0);
             });

    auto* fn_a = b.Function("a", ty.i32());
    auto* fn_a_p = b.FunctionParam("p", ty.ptr<uniform, i32>());
    fn_a->SetParams({
        b.FunctionParam("pre", ty.i32()),
        fn_a_p,
        b.FunctionParam("post", ty.i32()),
    });
    b.Append(fn_a->Block(), [&] { b.Return(fn_a, b.Load(fn_a_p)); });

    auto* fn_b = b.Function("b", ty.void_());
    b.Append(fn_b->Block(), [&] {
        b.Call(fn_a, 10_i, U, 20_i);
        b.Return(fn_b);
    });

    auto* src = R"(
$B1: {  # root
  %U:ptr<uniform, i32, read> = var undef @binding_point(0, 0)
}

%a = func(%pre:i32, %p:ptr<uniform, i32, read>, %post:i32):i32 {
  $B2: {
    %6:i32 = load %p
    ret %6
  }
}
%b = func():void {
  $B3: {
    %8:i32 = call %a, 10i, %U, 20i
    ret
  }
}
)";

    EXPECT_EQ(src, str());

    auto* expect = R"(
$B1: {  # root
  %U:ptr<uniform, i32, read> = var undef @binding_point(0, 0)
}

%a = func(%pre:i32, %post:i32):i32 {
  $B2: {
    %5:i32 = load %U
    ret %5
  }
}
%b = func():void {
  $B3: {
    %7:i32 = call %a, 10i, 20i
    ret
  }
}
)";

    Run(DirectVariableAccess, DirectVariableAccessOptions{});

    EXPECT_EQ(expect, str());
}

TEST_F(IR_DirectVariableAccessTest_UniformAS, Param_ptr_vec4i32_Via_array_DynamicRead) {
    Var* U = nullptr;
    b.Append(b.ir.root_block,
             [&] {  //
                 U = b.Var<uniform, array<vec4<i32>, 8>>("U");
                 U->SetBindingPoint(0, 0);
             });

    auto* fn_a = b.Function("a", ty.vec4<i32>());
    auto* fn_a_p = b.FunctionParam("p", ty.ptr<uniform, vec4<i32>>());
    fn_a->SetParams({
        b.FunctionParam("pre", ty.i32()),
        fn_a_p,
        b.FunctionParam("post", ty.i32()),
    });
    b.Append(fn_a->Block(), [&] { b.Return(fn_a, b.Load(fn_a_p)); });

    auto* fn_b = b.Function("b", ty.void_());
    b.Append(fn_b->Block(), [&] {
        auto* I = b.Let("I", 3_i);
        auto* access = b.Access(ty.ptr<uniform, vec4<i32>>(), U, I);
        b.Call(fn_a, 10_i, access, 20_i);
        b.Return(fn_b);
    });

    auto* src = R"(
$B1: {  # root
  %U:ptr<uniform, array<vec4<i32>, 8>, read> = var undef @binding_point(0, 0)
}

%a = func(%pre:i32, %p:ptr<uniform, vec4<i32>, read>, %post:i32):vec4<i32> {
  $B2: {
    %6:vec4<i32> = load %p
    ret %6
  }
}
%b = func():void {
  $B3: {
    %I:i32 = let 3i
    %9:ptr<uniform, vec4<i32>, read> = access %U, %I
    %10:vec4<i32> = call %a, 10i, %9, 20i
    ret
  }
}
)";

    EXPECT_EQ(src, str());

    auto* expect = R"(
$B1: {  # root
  %U:ptr<uniform, array<vec4<i32>, 8>, read> = var undef @binding_point(0, 0)
}

%a = func(%pre:i32, %p_indices:array<u32, 1>, %post:i32):vec4<i32> {
  $B2: {
    %6:u32 = access %p_indices, 0u
    %7:ptr<uniform, vec4<i32>, read> = access %U, %6
    %8:vec4<i32> = load %7
    ret %8
  }
}
%b = func():void {
  $B3: {
    %I:i32 = let 3i
    %11:u32 = convert %I
    %12:array<u32, 1> = construct %11
    %13:vec4<i32> = call %a, 10i, %12, 20i
    ret
  }
}
)";

    Run(DirectVariableAccess, DirectVariableAccessOptions{});

    EXPECT_EQ(expect, str());
}

TEST_F(IR_DirectVariableAccessTest_UniformAS, CallChaining) {
    auto* Inner =
        ty.Struct(mod.symbols.New("Inner"), {
                                                {mod.symbols.Register("mat"), ty.mat3x4<f32>()},
                                            });
    auto* Outer =
        ty.Struct(mod.symbols.New("Outer"), {
                                                {mod.symbols.Register("arr"), ty.array(Inner, 4)},
                                                {mod.symbols.Register("mat"), ty.mat3x4<f32>()},
                                            });
    Var* U = nullptr;
    b.Append(b.ir.root_block,
             [&] {  //
                 U = b.Var("U", ty.ptr<uniform>(Outer));
                 U->SetBindingPoint(0, 0);
             });

    auto* fn_0 = b.Function("f0", ty.f32());
    auto* fn_0_p = b.FunctionParam("p", ty.ptr<uniform, vec4<f32>>());
    fn_0->SetParams({fn_0_p});
    b.Append(fn_0->Block(), [&] { b.Return(fn_0, b.LoadVectorElement(fn_0_p, 0_u)); });

    auto* fn_1 = b.Function("f1", ty.f32());
    auto* fn_1_p = b.FunctionParam("p", ty.ptr<uniform, mat3x4<f32>>());
    fn_1->SetParams({fn_1_p});
    b.Append(fn_1->Block(), [&] {
        auto* res = b.Var<function, f32>("res");
        {
            // res += f0(&(*p)[1]);
            auto* call_0 = b.Call(fn_0, b.Access(ty.ptr<uniform, vec4<f32>>(), fn_1_p, 1_i));
            b.Store(res, b.Add(ty.f32(), b.Load(res), call_0));
        }
        {
            // let p_vec = &(*p)[1];
            // res += f0(p_vec);
            auto* p_vec = b.Access(ty.ptr<uniform, vec4<f32>>(), fn_1_p, 1_i);
            b.ir.SetName(p_vec, "p_vec");
            auto* call_0 = b.Call(fn_0, p_vec);
            b.Store(res, b.Add(ty.f32(), b.Load(res), call_0));
        }
        {
            // res += f0(&U.arr[2].mat[1]);
            auto* access = b.Access(ty.ptr<uniform, vec4<f32>>(), U, 0_u, 2_i, 0_u, 1_i);
            auto* call_0 = b.Call(fn_0, access);
            b.Store(res, b.Add(ty.f32(), b.Load(res), call_0));
        }
        {
            // let p_vec = &U.arr[2].mat[1];
            // res += f0(p_vec);
            auto* p_vec = b.Access(ty.ptr<uniform, vec4<f32>>(), U, 0_u, 2_i, 0_u, 1_i);
            b.ir.SetName(p_vec, "p_vec");
            auto* call_0 = b.Call(fn_0, p_vec);
            b.Store(res, b.Add(ty.f32(), b.Load(res), call_0));
        }

        b.Return(fn_1, b.Load(res));
    });

    auto* fn_2 = b.Function("f2", ty.f32());
    auto* fn_2_p = b.FunctionParam("p", ty.ptr<uniform>(Inner));
    fn_2->SetParams({fn_2_p});
    b.Append(fn_2->Block(), [&] {
        auto* p_mat = b.Access(ty.ptr<uniform, mat3x4<f32>>(), fn_2_p, 0_u);
        b.ir.SetName(p_mat, "p_mat");
        b.Return(fn_2, b.Call(fn_1, p_mat));
    });

    auto* fn_3 = b.Function("f3", ty.f32());
    auto* fn_3_p0 = b.FunctionParam("p0", ty.ptr<uniform>(ty.array(Inner, 4)));
    auto* fn_3_p1 = b.FunctionParam("p1", ty.ptr<uniform, mat3x4<f32>>());
    fn_3->SetParams({fn_3_p0, fn_3_p1});
    b.Append(fn_3->Block(), [&] {
        auto* p0_inner = b.Access(ty.ptr<uniform>(Inner), fn_3_p0, 3_i);
        b.ir.SetName(p0_inner, "p0_inner");
        auto* call_0 = b.Call(ty.f32(), fn_2, p0_inner);
        auto* call_1 = b.Call(ty.f32(), fn_1, fn_3_p1);
        b.Return(fn_3, b.Add(ty.f32(), call_0, call_1));
    });

    auto* fn_4 = b.Function("f4", ty.f32());
    auto* fn_4_p = b.FunctionParam("p", ty.ptr<uniform>(Outer));
    fn_4->SetParams({fn_4_p});
    b.Append(fn_4->Block(), [&] {
        auto* access_0 = b.Access(ty.ptr<uniform>(ty.array(Inner, 4)), fn_4_p, 0_u);
        auto* access_1 = b.Access(ty.ptr<uniform, mat3x4<f32>>(), U, 1_u);
        b.Return(fn_4, b.Call(ty.f32(), fn_3, access_0, access_1));
    });

    auto* fn_b = b.Function("b", ty.void_());
    b.Append(fn_b->Block(), [&] {
        b.Call(ty.f32(), fn_4, U);
        b.Return(fn_b);
    });

    auto* src = R"(
Inner = struct @align(16) {
  mat:mat3x4<f32> @offset(0)
}

Outer = struct @align(16) {
  arr:array<Inner, 4> @offset(0)
  mat:mat3x4<f32> @offset(192)
}

$B1: {  # root
  %U:ptr<uniform, Outer, read> = var undef @binding_point(0, 0)
}

%f0 = func(%p:ptr<uniform, vec4<f32>, read>):f32 {
  $B2: {
    %4:f32 = load_vector_element %p, 0u
    ret %4
  }
}
%f1 = func(%p_1:ptr<uniform, mat3x4<f32>, read>):f32 {  # %p_1: 'p'
  $B3: {
    %res:ptr<function, f32, read_write> = var undef
    %8:ptr<uniform, vec4<f32>, read> = access %p_1, 1i
    %9:f32 = call %f0, %8
    %10:f32 = load %res
    %11:f32 = add %10, %9
    store %res, %11
    %p_vec:ptr<uniform, vec4<f32>, read> = access %p_1, 1i
    %13:f32 = call %f0, %p_vec
    %14:f32 = load %res
    %15:f32 = add %14, %13
    store %res, %15
    %16:ptr<uniform, vec4<f32>, read> = access %U, 0u, 2i, 0u, 1i
    %17:f32 = call %f0, %16
    %18:f32 = load %res
    %19:f32 = add %18, %17
    store %res, %19
    %p_vec_1:ptr<uniform, vec4<f32>, read> = access %U, 0u, 2i, 0u, 1i  # %p_vec_1: 'p_vec'
    %21:f32 = call %f0, %p_vec_1
    %22:f32 = load %res
    %23:f32 = add %22, %21
    store %res, %23
    %24:f32 = load %res
    ret %24
  }
}
%f2 = func(%p_2:ptr<uniform, Inner, read>):f32 {  # %p_2: 'p'
  $B4: {
    %p_mat:ptr<uniform, mat3x4<f32>, read> = access %p_2, 0u
    %28:f32 = call %f1, %p_mat
    ret %28
  }
}
%f3 = func(%p0:ptr<uniform, array<Inner, 4>, read>, %p1:ptr<uniform, mat3x4<f32>, read>):f32 {
  $B5: {
    %p0_inner:ptr<uniform, Inner, read> = access %p0, 3i
    %33:f32 = call %f2, %p0_inner
    %34:f32 = call %f1, %p1
    %35:f32 = add %33, %34
    ret %35
  }
}
%f4 = func(%p_3:ptr<uniform, Outer, read>):f32 {  # %p_3: 'p'
  $B6: {
    %38:ptr<uniform, array<Inner, 4>, read> = access %p_3, 0u
    %39:ptr<uniform, mat3x4<f32>, read> = access %U, 1u
    %40:f32 = call %f3, %38, %39
    ret %40
  }
}
%b = func():void {
  $B7: {
    %42:f32 = call %f4, %U
    ret
  }
}
)";

    EXPECT_EQ(src, str());

    auto* expect = R"(
Inner = struct @align(16) {
  mat:mat3x4<f32> @offset(0)
}

Outer = struct @align(16) {
  arr:array<Inner, 4> @offset(0)
  mat:mat3x4<f32> @offset(192)
}

$B1: {  # root
  %U:ptr<uniform, Outer, read> = var undef @binding_point(0, 0)
}

%f0 = func(%p_indices:array<u32, 1>):f32 {
  $B2: {
    %4:u32 = access %p_indices, 0u
    %5:ptr<uniform, vec4<f32>, read> = access %U, 1u, %4
    %6:f32 = load_vector_element %5, 0u
    ret %6
  }
}
%f0_1 = func(%p_indices_1:array<u32, 2>):f32 {  # %f0_1: 'f0', %p_indices_1: 'p_indices'
  $B3: {
    %9:u32 = access %p_indices_1, 0u
    %10:u32 = access %p_indices_1, 1u
    %11:ptr<uniform, vec4<f32>, read> = access %U, 0u, %9, 0u, %10
    %12:f32 = load_vector_element %11, 0u
    ret %12
  }
}
%f1 = func():f32 {
  $B4: {
    %res:ptr<function, f32, read_write> = var undef
    %15:u32 = convert 1i
    %16:array<u32, 1> = construct %15
    %17:f32 = call %f0, %16
    %18:f32 = load %res
    %19:f32 = add %18, %17
    store %res, %19
    %20:u32 = convert 1i
    %21:array<u32, 1> = construct %20
    %22:f32 = call %f0, %21
    %23:f32 = load %res
    %24:f32 = add %23, %22
    store %res, %24
    %25:u32 = convert 2i
    %26:u32 = convert 1i
    %27:array<u32, 2> = construct %25, %26
    %28:f32 = call %f0_1, %27
    %29:f32 = load %res
    %30:f32 = add %29, %28
    store %res, %30
    %31:u32 = convert 2i
    %32:u32 = convert 1i
    %33:array<u32, 2> = construct %31, %32
    %34:f32 = call %f0_1, %33
    %35:f32 = load %res
    %36:f32 = add %35, %34
    store %res, %36
    %37:f32 = load %res
    ret %37
  }
}
%f1_1 = func(%p_indices_2:array<u32, 1>):f32 {  # %f1_1: 'f1', %p_indices_2: 'p_indices'
  $B5: {
    %40:u32 = access %p_indices_2, 0u
    %res_1:ptr<function, f32, read_write> = var undef  # %res_1: 'res'
    %42:u32 = convert 1i
    %43:array<u32, 2> = construct %40, %42
    %44:f32 = call %f0_1, %43
    %45:f32 = load %res_1
    %46:f32 = add %45, %44
    store %res_1, %46
    %47:u32 = convert 1i
    %48:array<u32, 2> = construct %40, %47
    %49:f32 = call %f0_1, %48
    %50:f32 = load %res_1
    %51:f32 = add %50, %49
    store %res_1, %51
    %52:u32 = convert 2i
    %53:u32 = convert 1i
    %54:array<u32, 2> = construct %52, %53
    %55:f32 = call %f0_1, %54
    %56:f32 = load %res_1
    %57:f32 = add %56, %55
    store %res_1, %57
    %58:u32 = convert 2i
    %59:u32 = convert 1i
    %60:array<u32, 2> = construct %58, %59
    %61:f32 = call %f0_1, %60
    %62:f32 = load %res_1
    %63:f32 = add %62, %61
    store %res_1, %63
    %64:f32 = load %res_1
    ret %64
  }
}
%f2 = func(%p_indices_3:array<u32, 1>):f32 {  # %p_indices_3: 'p_indices'
  $B6: {
    %67:u32 = access %p_indices_3, 0u
    %68:array<u32, 1> = construct %67
    %69:f32 = call %f1_1, %68
    ret %69
  }
}
%f3 = func():f32 {
  $B7: {
    %71:u32 = convert 3i
    %72:array<u32, 1> = construct %71
    %73:f32 = call %f2, %72
    %74:f32 = call %f1
    %75:f32 = add %73, %74
    ret %75
  }
}
%f4 = func():f32 {
  $B8: {
    %77:f32 = call %f3
    ret %77
  }
}
%b = func():void {
  $B9: {
    %79:f32 = call %f4
    ret
  }
}
)";

    Run(DirectVariableAccess, DirectVariableAccessOptions{});

    EXPECT_EQ(expect, str());
}

TEST_F(IR_DirectVariableAccessTest_UniformAS, CallChaining2) {
    auto* T3 = ty.vec4<i32>();
    auto* T2 = ty.array(T3, 5);
    auto* T1 = ty.array(T2, 5);
    auto* T = ty.array(T1, 5);

    Var* input = nullptr;
    b.Append(b.ir.root_block,
             [&] {  //
                 input = b.Var("U", ty.ptr<uniform>(T));
                 input->SetBindingPoint(0, 0);
             });

    auto* f2 = b.Function("f2", T3);
    {
        auto* p = b.FunctionParam("p", ty.ptr<uniform>(T2));
        f2->SetParams({p});
        b.Append(f2->Block(),
                 [&] { b.Return(f2, b.Load(b.Access<ptr<uniform, vec4<i32>>>(p, 3_u))); });
    }

    auto* f1 = b.Function("f1", T3);
    {
        auto* p = b.FunctionParam("p", ty.ptr<uniform>(T1));
        f1->SetParams({p});
        b.Append(f1->Block(),
                 [&] { b.Return(f1, b.Call(f2, b.Access(ty.ptr<uniform>(T2), p, 2_u))); });
    }

    auto* f0 = b.Function("f0", T3);
    {
        auto* p = b.FunctionParam("p", ty.ptr<uniform>(T));
        f0->SetParams({p});
        b.Append(f0->Block(),
                 [&] { b.Return(f0, b.Call(f1, b.Access(ty.ptr<uniform>(T1), p, 1_u))); });
    }

    auto* main = b.Function("main", ty.void_());
    b.Append(main->Block(), [&] {
        b.Call(f0, input);
        b.Return(main);
    });

    auto* src = R"(
$B1: {  # root
  %U:ptr<uniform, array<array<array<vec4<i32>, 5>, 5>, 5>, read> = var undef @binding_point(0, 0)
}

%f2 = func(%p:ptr<uniform, array<vec4<i32>, 5>, read>):vec4<i32> {
  $B2: {
    %4:ptr<uniform, vec4<i32>, read> = access %p, 3u
    %5:vec4<i32> = load %4
    ret %5
  }
}
%f1 = func(%p_1:ptr<uniform, array<array<vec4<i32>, 5>, 5>, read>):vec4<i32> {  # %p_1: 'p'
  $B3: {
    %8:ptr<uniform, array<vec4<i32>, 5>, read> = access %p_1, 2u
    %9:vec4<i32> = call %f2, %8
    ret %9
  }
}
%f0 = func(%p_2:ptr<uniform, array<array<array<vec4<i32>, 5>, 5>, 5>, read>):vec4<i32> {  # %p_2: 'p'
  $B4: {
    %12:ptr<uniform, array<array<vec4<i32>, 5>, 5>, read> = access %p_2, 1u
    %13:vec4<i32> = call %f1, %12
    ret %13
  }
}
%main = func():void {
  $B5: {
    %15:vec4<i32> = call %f0, %U
    ret
  }
}
)";

    EXPECT_EQ(src, str());

    auto* expect = R"(
$B1: {  # root
  %U:ptr<uniform, array<array<array<vec4<i32>, 5>, 5>, 5>, read> = var undef @binding_point(0, 0)
}

%f2 = func(%p_indices:array<u32, 2>):vec4<i32> {
  $B2: {
    %4:u32 = access %p_indices, 0u
    %5:u32 = access %p_indices, 1u
    %6:ptr<uniform, array<vec4<i32>, 5>, read> = access %U, %4, %5
    %7:ptr<uniform, vec4<i32>, read> = access %6, 3u
    %8:vec4<i32> = load %7
    ret %8
  }
}
%f1 = func(%p_indices_1:array<u32, 1>):vec4<i32> {  # %p_indices_1: 'p_indices'
  $B3: {
    %11:u32 = access %p_indices_1, 0u
    %12:array<u32, 2> = construct %11, 2u
    %13:vec4<i32> = call %f2, %12
    ret %13
  }
}
%f0 = func():vec4<i32> {
  $B4: {
    %15:array<u32, 1> = construct 1u
    %16:vec4<i32> = call %f1, %15
    ret %16
  }
}
%main = func():void {
  $B5: {
    %18:vec4<i32> = call %f0
    ret
  }
}
)";

    Run(DirectVariableAccess, DirectVariableAccessOptions{});

    EXPECT_EQ(expect, str());
}

}  // namespace uniform_as_tests

////////////////////////////////////////////////////////////////////////////////
// 'storage' address space
////////////////////////////////////////////////////////////////////////////////
namespace storage_as_tests {

using IR_DirectVariableAccessTest_StorageAS = TransformTest;

TEST_F(IR_DirectVariableAccessTest_StorageAS, Param_ptr_i32_Via_struct_read) {
    auto* str_ = ty.Struct(mod.symbols.New("str"), {
                                                       {mod.symbols.Register("i"), ty.i32()},
                                                   });

    Var* S = nullptr;
    b.Append(b.ir.root_block,
             [&] {  //
                 S = b.Var("S", ty.ptr<storage, read>(str_));
                 S->SetBindingPoint(0, 0);
             });

    auto* fn_a = b.Function("a", ty.i32());
    auto* fn_a_p = b.FunctionParam("p", ty.ptr<storage, i32, read>());
    fn_a->SetParams({
        b.FunctionParam("pre", ty.i32()),
        fn_a_p,
        b.FunctionParam("post", ty.i32()),
    });
    b.Append(fn_a->Block(), [&] { b.Return(fn_a, b.Load(fn_a_p)); });

    auto* fn_b = b.Function("b", ty.void_());
    b.Append(fn_b->Block(), [&] {
        auto* access = b.Access(ty.ptr<storage, i32, read>(), S, 0_u);
        b.Call(fn_a, 10_i, access, 20_i);
        b.Return(fn_b);
    });

    auto* src = R"(
str = struct @align(4) {
  i:i32 @offset(0)
}

$B1: {  # root
  %S:ptr<storage, str, read> = var undef @binding_point(0, 0)
}

%a = func(%pre:i32, %p:ptr<storage, i32, read>, %post:i32):i32 {
  $B2: {
    %6:i32 = load %p
    ret %6
  }
}
%b = func():void {
  $B3: {
    %8:ptr<storage, i32, read> = access %S, 0u
    %9:i32 = call %a, 10i, %8, 20i
    ret
  }
}
)";

    EXPECT_EQ(src, str());

    auto* expect = R"(
str = struct @align(4) {
  i:i32 @offset(0)
}

$B1: {  # root
  %S:ptr<storage, str, read> = var undef @binding_point(0, 0)
}

%a = func(%pre:i32, %post:i32):i32 {
  $B2: {
    %5:ptr<storage, i32, read> = access %S, 0u
    %6:i32 = load %5
    ret %6
  }
}
%b = func():void {
  $B3: {
    %8:i32 = call %a, 10i, 20i
    ret
  }
}
)";

    Run(DirectVariableAccess, DirectVariableAccessOptions{});

    EXPECT_EQ(expect, str());
}

TEST_F(IR_DirectVariableAccessTest_StorageAS, Param_ptr_arr_i32_Via_struct_write) {
    auto* str_ =
        ty.Struct(mod.symbols.New("str"), {
                                              {mod.symbols.Register("arr"), ty.array<i32, 4>()},
                                          });

    Var* S = nullptr;
    b.Append(b.ir.root_block,
             [&] {  //
                 S = b.Var("S", ty.ptr<storage>(str_));
                 S->SetBindingPoint(0, 0);
             });

    auto* fn_a = b.Function("a", ty.void_());
    auto* fn_a_p = b.FunctionParam("p", ty.ptr<storage, array<i32, 4>>());
    fn_a->SetParams({
        b.FunctionParam("pre", ty.i32()),
        fn_a_p,
        b.FunctionParam("post", ty.i32()),
    });
    b.Append(fn_a->Block(), [&] {
        b.Store(fn_a_p, b.Splat<array<i32, 4>>(0_i));
        b.Return(fn_a);
    });

    auto* fn_b = b.Function("b", ty.void_());
    b.Append(fn_b->Block(), [&] {
        auto* access = b.Access(ty.ptr<storage, array<i32, 4>>(), S, 0_u);
        b.Call(fn_a, 10_i, access, 20_i);
        b.Return(fn_b);
    });

    auto* src = R"(
str = struct @align(4) {
  arr:array<i32, 4> @offset(0)
}

$B1: {  # root
  %S:ptr<storage, str, read_write> = var undef @binding_point(0, 0)
}

%a = func(%pre:i32, %p:ptr<storage, array<i32, 4>, read_write>, %post:i32):void {
  $B2: {
    store %p, array<i32, 4>(0i)
    ret
  }
}
%b = func():void {
  $B3: {
    %7:ptr<storage, array<i32, 4>, read_write> = access %S, 0u
    %8:void = call %a, 10i, %7, 20i
    ret
  }
}
)";

    EXPECT_EQ(src, str());

    auto* expect = R"(
str = struct @align(4) {
  arr:array<i32, 4> @offset(0)
}

$B1: {  # root
  %S:ptr<storage, str, read_write> = var undef @binding_point(0, 0)
}

%a = func(%pre:i32, %post:i32):void {
  $B2: {
    %5:ptr<storage, array<i32, 4>, read_write> = access %S, 0u
    store %5, array<i32, 4>(0i)
    ret
  }
}
%b = func():void {
  $B3: {
    %7:void = call %a, 10i, 20i
    ret
  }
}
)";

    Run(DirectVariableAccess, DirectVariableAccessOptions{});

    EXPECT_EQ(expect, str());
}

TEST_F(IR_DirectVariableAccessTest_StorageAS, Param_ptr_vec4i32_Via_array_DynamicWrite) {
    Var* S = nullptr;
    b.Append(b.ir.root_block,
             [&] {  //
                 S = b.Var<storage, array<vec4<i32>, 8>>("S");
                 S->SetBindingPoint(0, 0);
             });

    auto* fn_a = b.Function("a", ty.void_());
    auto* fn_a_p = b.FunctionParam("p", ty.ptr<storage, vec4<i32>>());
    fn_a->SetParams({
        b.FunctionParam("pre", ty.i32()),
        fn_a_p,
        b.FunctionParam("post", ty.i32()),
    });
    b.Append(fn_a->Block(), [&] {
        b.Store(fn_a_p, b.Splat<vec4<i32>>(0_i));
        b.Return(fn_a);
    });

    auto* fn_b = b.Function("b", ty.void_());
    b.Append(fn_b->Block(), [&] {
        auto* I = b.Let("I", 3_i);
        auto* access = b.Access(ty.ptr<storage, vec4<i32>>(), S, I);
        b.Call(fn_a, 10_i, access, 20_i);
        b.Return(fn_b);
    });

    auto* src = R"(
$B1: {  # root
  %S:ptr<storage, array<vec4<i32>, 8>, read_write> = var undef @binding_point(0, 0)
}

%a = func(%pre:i32, %p:ptr<storage, vec4<i32>, read_write>, %post:i32):void {
  $B2: {
    store %p, vec4<i32>(0i)
    ret
  }
}
%b = func():void {
  $B3: {
    %I:i32 = let 3i
    %8:ptr<storage, vec4<i32>, read_write> = access %S, %I
    %9:void = call %a, 10i, %8, 20i
    ret
  }
}
)";

    EXPECT_EQ(src, str());

    auto* expect = R"(
$B1: {  # root
  %S:ptr<storage, array<vec4<i32>, 8>, read_write> = var undef @binding_point(0, 0)
}

%a = func(%pre:i32, %p_indices:array<u32, 1>, %post:i32):void {
  $B2: {
    %6:u32 = access %p_indices, 0u
    %7:ptr<storage, vec4<i32>, read_write> = access %S, %6
    store %7, vec4<i32>(0i)
    ret
  }
}
%b = func():void {
  $B3: {
    %I:i32 = let 3i
    %10:u32 = convert %I
    %11:array<u32, 1> = construct %10
    %12:void = call %a, 10i, %11, 20i
    ret
  }
}
)";

    Run(DirectVariableAccess, DirectVariableAccessOptions{});

    EXPECT_EQ(expect, str());
}

TEST_F(IR_DirectVariableAccessTest_StorageAS, CallChaining) {
    auto* Inner =
        ty.Struct(mod.symbols.New("Inner"), {
                                                {mod.symbols.Register("mat"), ty.mat3x4<f32>()},
                                            });
    auto* Outer =
        ty.Struct(mod.symbols.New("Outer"), {
                                                {mod.symbols.Register("arr"), ty.array(Inner, 4)},
                                                {mod.symbols.Register("mat"), ty.mat3x4<f32>()},
                                            });
    Var* S = nullptr;
    b.Append(b.ir.root_block,
             [&] {  //
                 S = b.Var("S", ty.ptr<storage, read>(Outer));
                 S->SetBindingPoint(0, 0);
             });

    auto* fn_0 = b.Function("f0", ty.f32());
    auto* fn_0_p = b.FunctionParam("p", ty.ptr<storage, vec4<f32>, read>());
    fn_0->SetParams({fn_0_p});
    b.Append(fn_0->Block(), [&] { b.Return(fn_0, b.LoadVectorElement(fn_0_p, 0_u)); });

    auto* fn_1 = b.Function("f1", ty.f32());
    auto* fn_1_p = b.FunctionParam("p", ty.ptr<storage, mat3x4<f32>, read>());
    fn_1->SetParams({fn_1_p});
    b.Append(fn_1->Block(), [&] {
        auto* res = b.Var<function, f32>("res");
        {
            // res += f0(&(*p)[1]);
            auto* call_0 = b.Call(fn_0, b.Access(ty.ptr<storage, vec4<f32>, read>(), fn_1_p, 1_i));
            b.Store(res, b.Add(ty.f32(), b.Load(res), call_0));
        }
        {
            // let p_vec = &(*p)[1];
            // res += f0(p_vec);
            auto* p_vec = b.Access(ty.ptr<storage, vec4<f32>, read>(), fn_1_p, 1_i);
            b.ir.SetName(p_vec, "p_vec");
            auto* call_0 = b.Call(fn_0, p_vec);
            b.Store(res, b.Add(ty.f32(), b.Load(res), call_0));
        }
        {
            // res += f0(&U.arr[2].mat[1]);
            auto* access = b.Access(ty.ptr<storage, vec4<f32>, read>(), S, 0_u, 2_i, 0_u, 1_i);
            auto* call_0 = b.Call(fn_0, access);
            b.Store(res, b.Add(ty.f32(), b.Load(res), call_0));
        }
        {
            // let p_vec = &U.arr[2].mat[1];
            // res += f0(p_vec);
            auto* p_vec = b.Access(ty.ptr<storage, vec4<f32>, read>(), S, 0_u, 2_i, 0_u, 1_i);
            b.ir.SetName(p_vec, "p_vec");
            auto* call_0 = b.Call(fn_0, p_vec);
            b.Store(res, b.Add(ty.f32(), b.Load(res), call_0));
        }

        b.Return(fn_1, b.Load(res));
    });

    auto* fn_2 = b.Function("f2", ty.f32());
    auto* fn_2_p = b.FunctionParam("p", ty.ptr<storage, read>(Inner));
    fn_2->SetParams({fn_2_p});
    b.Append(fn_2->Block(), [&] {
        auto* p_mat = b.Access(ty.ptr<storage, mat3x4<f32>, read>(), fn_2_p, 0_u);
        b.ir.SetName(p_mat, "p_mat");
        b.Return(fn_2, b.Call(fn_1, p_mat));
    });

    auto* fn_3 = b.Function("f3", ty.f32());
    auto* fn_3_p0 = b.FunctionParam("p0", ty.ptr<storage, read>(ty.array(Inner, 4)));
    auto* fn_3_p1 = b.FunctionParam("p1", ty.ptr<storage, mat3x4<f32>, read>());
    fn_3->SetParams({fn_3_p0, fn_3_p1});
    b.Append(fn_3->Block(), [&] {
        auto* p0_inner = b.Access(ty.ptr<storage, read>(Inner), fn_3_p0, 3_i);
        b.ir.SetName(p0_inner, "p0_inner");
        auto* call_0 = b.Call(ty.f32(), fn_2, p0_inner);
        auto* call_1 = b.Call(ty.f32(), fn_1, fn_3_p1);
        b.Return(fn_3, b.Add(ty.f32(), call_0, call_1));
    });

    auto* fn_4 = b.Function("f4", ty.f32());
    auto* fn_4_p = b.FunctionParam("p", ty.ptr<storage, read>(Outer));
    fn_4->SetParams({fn_4_p});
    b.Append(fn_4->Block(), [&] {
        auto* access_0 = b.Access(ty.ptr<storage, read>(ty.array(Inner, 4)), fn_4_p, 0_u);
        auto* access_1 = b.Access(ty.ptr<storage, mat3x4<f32>, read>(), S, 1_u);
        b.Return(fn_4, b.Call(ty.f32(), fn_3, access_0, access_1));
    });

    auto* fn_b = b.Function("b", ty.void_());
    b.Append(fn_b->Block(), [&] {
        b.Call(ty.f32(), fn_4, S);
        b.Return(fn_b);
    });

    auto* src = R"(
Inner = struct @align(16) {
  mat:mat3x4<f32> @offset(0)
}

Outer = struct @align(16) {
  arr:array<Inner, 4> @offset(0)
  mat:mat3x4<f32> @offset(192)
}

$B1: {  # root
  %S:ptr<storage, Outer, read> = var undef @binding_point(0, 0)
}

%f0 = func(%p:ptr<storage, vec4<f32>, read>):f32 {
  $B2: {
    %4:f32 = load_vector_element %p, 0u
    ret %4
  }
}
%f1 = func(%p_1:ptr<storage, mat3x4<f32>, read>):f32 {  # %p_1: 'p'
  $B3: {
    %res:ptr<function, f32, read_write> = var undef
    %8:ptr<storage, vec4<f32>, read> = access %p_1, 1i
    %9:f32 = call %f0, %8
    %10:f32 = load %res
    %11:f32 = add %10, %9
    store %res, %11
    %p_vec:ptr<storage, vec4<f32>, read> = access %p_1, 1i
    %13:f32 = call %f0, %p_vec
    %14:f32 = load %res
    %15:f32 = add %14, %13
    store %res, %15
    %16:ptr<storage, vec4<f32>, read> = access %S, 0u, 2i, 0u, 1i
    %17:f32 = call %f0, %16
    %18:f32 = load %res
    %19:f32 = add %18, %17
    store %res, %19
    %p_vec_1:ptr<storage, vec4<f32>, read> = access %S, 0u, 2i, 0u, 1i  # %p_vec_1: 'p_vec'
    %21:f32 = call %f0, %p_vec_1
    %22:f32 = load %res
    %23:f32 = add %22, %21
    store %res, %23
    %24:f32 = load %res
    ret %24
  }
}
%f2 = func(%p_2:ptr<storage, Inner, read>):f32 {  # %p_2: 'p'
  $B4: {
    %p_mat:ptr<storage, mat3x4<f32>, read> = access %p_2, 0u
    %28:f32 = call %f1, %p_mat
    ret %28
  }
}
%f3 = func(%p0:ptr<storage, array<Inner, 4>, read>, %p1:ptr<storage, mat3x4<f32>, read>):f32 {
  $B5: {
    %p0_inner:ptr<storage, Inner, read> = access %p0, 3i
    %33:f32 = call %f2, %p0_inner
    %34:f32 = call %f1, %p1
    %35:f32 = add %33, %34
    ret %35
  }
}
%f4 = func(%p_3:ptr<storage, Outer, read>):f32 {  # %p_3: 'p'
  $B6: {
    %38:ptr<storage, array<Inner, 4>, read> = access %p_3, 0u
    %39:ptr<storage, mat3x4<f32>, read> = access %S, 1u
    %40:f32 = call %f3, %38, %39
    ret %40
  }
}
%b = func():void {
  $B7: {
    %42:f32 = call %f4, %S
    ret
  }
}
)";

    EXPECT_EQ(src, str());

    auto* expect = R"(
Inner = struct @align(16) {
  mat:mat3x4<f32> @offset(0)
}

Outer = struct @align(16) {
  arr:array<Inner, 4> @offset(0)
  mat:mat3x4<f32> @offset(192)
}

$B1: {  # root
  %S:ptr<storage, Outer, read> = var undef @binding_point(0, 0)
}

%f0 = func(%p_indices:array<u32, 1>):f32 {
  $B2: {
    %4:u32 = access %p_indices, 0u
    %5:ptr<storage, vec4<f32>, read> = access %S, 1u, %4
    %6:f32 = load_vector_element %5, 0u
    ret %6
  }
}
%f0_1 = func(%p_indices_1:array<u32, 2>):f32 {  # %f0_1: 'f0', %p_indices_1: 'p_indices'
  $B3: {
    %9:u32 = access %p_indices_1, 0u
    %10:u32 = access %p_indices_1, 1u
    %11:ptr<storage, vec4<f32>, read> = access %S, 0u, %9, 0u, %10
    %12:f32 = load_vector_element %11, 0u
    ret %12
  }
}
%f1 = func():f32 {
  $B4: {
    %res:ptr<function, f32, read_write> = var undef
    %15:u32 = convert 1i
    %16:array<u32, 1> = construct %15
    %17:f32 = call %f0, %16
    %18:f32 = load %res
    %19:f32 = add %18, %17
    store %res, %19
    %20:u32 = convert 1i
    %21:array<u32, 1> = construct %20
    %22:f32 = call %f0, %21
    %23:f32 = load %res
    %24:f32 = add %23, %22
    store %res, %24
    %25:u32 = convert 2i
    %26:u32 = convert 1i
    %27:array<u32, 2> = construct %25, %26
    %28:f32 = call %f0_1, %27
    %29:f32 = load %res
    %30:f32 = add %29, %28
    store %res, %30
    %31:u32 = convert 2i
    %32:u32 = convert 1i
    %33:array<u32, 2> = construct %31, %32
    %34:f32 = call %f0_1, %33
    %35:f32 = load %res
    %36:f32 = add %35, %34
    store %res, %36
    %37:f32 = load %res
    ret %37
  }
}
%f1_1 = func(%p_indices_2:array<u32, 1>):f32 {  # %f1_1: 'f1', %p_indices_2: 'p_indices'
  $B5: {
    %40:u32 = access %p_indices_2, 0u
    %res_1:ptr<function, f32, read_write> = var undef  # %res_1: 'res'
    %42:u32 = convert 1i
    %43:array<u32, 2> = construct %40, %42
    %44:f32 = call %f0_1, %43
    %45:f32 = load %res_1
    %46:f32 = add %45, %44
    store %res_1, %46
    %47:u32 = convert 1i
    %48:array<u32, 2> = construct %40, %47
    %49:f32 = call %f0_1, %48
    %50:f32 = load %res_1
    %51:f32 = add %50, %49
    store %res_1, %51
    %52:u32 = convert 2i
    %53:u32 = convert 1i
    %54:array<u32, 2> = construct %52, %53
    %55:f32 = call %f0_1, %54
    %56:f32 = load %res_1
    %57:f32 = add %56, %55
    store %res_1, %57
    %58:u32 = convert 2i
    %59:u32 = convert 1i
    %60:array<u32, 2> = construct %58, %59
    %61:f32 = call %f0_1, %60
    %62:f32 = load %res_1
    %63:f32 = add %62, %61
    store %res_1, %63
    %64:f32 = load %res_1
    ret %64
  }
}
%f2 = func(%p_indices_3:array<u32, 1>):f32 {  # %p_indices_3: 'p_indices'
  $B6: {
    %67:u32 = access %p_indices_3, 0u
    %68:array<u32, 1> = construct %67
    %69:f32 = call %f1_1, %68
    ret %69
  }
}
%f3 = func():f32 {
  $B7: {
    %71:u32 = convert 3i
    %72:array<u32, 1> = construct %71
    %73:f32 = call %f2, %72
    %74:f32 = call %f1
    %75:f32 = add %73, %74
    ret %75
  }
}
%f4 = func():f32 {
  $B8: {
    %77:f32 = call %f3
    ret %77
  }
}
%b = func():void {
  $B9: {
    %79:f32 = call %f4
    ret
  }
}
)";

    Run(DirectVariableAccess, DirectVariableAccessOptions{});

    EXPECT_EQ(expect, str());
}

TEST_F(IR_DirectVariableAccessTest_StorageAS, CallChaining2) {
    auto* T3 = ty.vec4<i32>();
    auto* T2 = ty.array(T3, 5);
    auto* T1 = ty.array(T2, 5);
    auto* T = ty.array(T1, 5);

    Var* input = nullptr;
    b.Append(b.ir.root_block,
             [&] {  //
                 input = b.Var("U", ty.ptr<storage>(T));
                 input->SetBindingPoint(0, 0);
             });

    auto* f2 = b.Function("f2", T3);
    {
        auto* p = b.FunctionParam("p", ty.ptr<storage>(T2));
        f2->SetParams({p});
        b.Append(f2->Block(),
                 [&] { b.Return(f2, b.Load(b.Access<ptr<storage, vec4<i32>>>(p, 3_u))); });
    }

    auto* f1 = b.Function("f1", T3);
    {
        auto* p = b.FunctionParam("p", ty.ptr<storage>(T1));
        f1->SetParams({p});
        b.Append(f1->Block(),
                 [&] { b.Return(f1, b.Call(f2, b.Access(ty.ptr<storage>(T2), p, 2_u))); });
    }

    auto* f0 = b.Function("f0", T3);
    {
        auto* p = b.FunctionParam("p", ty.ptr<storage>(T));
        f0->SetParams({p});
        b.Append(f0->Block(),
                 [&] { b.Return(f0, b.Call(f1, b.Access(ty.ptr<storage>(T1), p, 1_u))); });
    }

    auto* main = b.Function("main", ty.void_());
    b.Append(main->Block(), [&] {
        b.Call(f0, input);
        b.Return(main);
    });

    auto* src = R"(
$B1: {  # root
  %U:ptr<storage, array<array<array<vec4<i32>, 5>, 5>, 5>, read_write> = var undef @binding_point(0, 0)
}

%f2 = func(%p:ptr<storage, array<vec4<i32>, 5>, read_write>):vec4<i32> {
  $B2: {
    %4:ptr<storage, vec4<i32>, read_write> = access %p, 3u
    %5:vec4<i32> = load %4
    ret %5
  }
}
%f1 = func(%p_1:ptr<storage, array<array<vec4<i32>, 5>, 5>, read_write>):vec4<i32> {  # %p_1: 'p'
  $B3: {
    %8:ptr<storage, array<vec4<i32>, 5>, read_write> = access %p_1, 2u
    %9:vec4<i32> = call %f2, %8
    ret %9
  }
}
%f0 = func(%p_2:ptr<storage, array<array<array<vec4<i32>, 5>, 5>, 5>, read_write>):vec4<i32> {  # %p_2: 'p'
  $B4: {
    %12:ptr<storage, array<array<vec4<i32>, 5>, 5>, read_write> = access %p_2, 1u
    %13:vec4<i32> = call %f1, %12
    ret %13
  }
}
%main = func():void {
  $B5: {
    %15:vec4<i32> = call %f0, %U
    ret
  }
}
)";

    EXPECT_EQ(src, str());

    auto* expect = R"(
$B1: {  # root
  %U:ptr<storage, array<array<array<vec4<i32>, 5>, 5>, 5>, read_write> = var undef @binding_point(0, 0)
}

%f2 = func(%p_indices:array<u32, 2>):vec4<i32> {
  $B2: {
    %4:u32 = access %p_indices, 0u
    %5:u32 = access %p_indices, 1u
    %6:ptr<storage, array<vec4<i32>, 5>, read_write> = access %U, %4, %5
    %7:ptr<storage, vec4<i32>, read_write> = access %6, 3u
    %8:vec4<i32> = load %7
    ret %8
  }
}
%f1 = func(%p_indices_1:array<u32, 1>):vec4<i32> {  # %p_indices_1: 'p_indices'
  $B3: {
    %11:u32 = access %p_indices_1, 0u
    %12:array<u32, 2> = construct %11, 2u
    %13:vec4<i32> = call %f2, %12
    ret %13
  }
}
%f0 = func():vec4<i32> {
  $B4: {
    %15:array<u32, 1> = construct 1u
    %16:vec4<i32> = call %f1, %15
    ret %16
  }
}
%main = func():void {
  $B5: {
    %18:vec4<i32> = call %f0
    ret
  }
}
)";

    Run(DirectVariableAccess, DirectVariableAccessOptions{});

    EXPECT_EQ(expect, str());
}

}  // namespace storage_as_tests

////////////////////////////////////////////////////////////////////////////////
// 'workgroup' address space
////////////////////////////////////////////////////////////////////////////////
namespace workgroup_as_tests {

using IR_DirectVariableAccessTest_WorkgroupAS = TransformTest;

TEST_F(IR_DirectVariableAccessTest_WorkgroupAS, Param_ptr_vec4i32_Via_array_StaticRead) {
    Var* W = nullptr;
    b.Append(b.ir.root_block,
             [&] {  //
                 W = b.Var("W", ty.ptr<workgroup, array<vec4<i32>, 8>>());
             });

    auto* fn_a = b.Function("a", ty.vec4<i32>());
    auto* fn_a_p = b.FunctionParam("p", ty.ptr<workgroup, vec4<i32>>());
    fn_a->SetParams({
        b.FunctionParam("pre", ty.i32()),
        fn_a_p,
        b.FunctionParam("post", ty.i32()),
    });
    b.Append(fn_a->Block(), [&] { b.Return(fn_a, b.Load(fn_a_p)); });

    auto* fn_b = b.Function("b", ty.void_());
    b.Append(fn_b->Block(), [&] {
        auto* access = b.Access(ty.ptr<workgroup, vec4<i32>>(), W, 3_i);
        b.Call(fn_a, 10_i, access, 20_i);
        b.Return(fn_b);
    });

    auto* src = R"(
$B1: {  # root
  %W:ptr<workgroup, array<vec4<i32>, 8>, read_write> = var undef
}

%a = func(%pre:i32, %p:ptr<workgroup, vec4<i32>, read_write>, %post:i32):vec4<i32> {
  $B2: {
    %6:vec4<i32> = load %p
    ret %6
  }
}
%b = func():void {
  $B3: {
    %8:ptr<workgroup, vec4<i32>, read_write> = access %W, 3i
    %9:vec4<i32> = call %a, 10i, %8, 20i
    ret
  }
}
)";

    EXPECT_EQ(src, str());

    auto* expect = R"(
$B1: {  # root
  %W:ptr<workgroup, array<vec4<i32>, 8>, read_write> = var undef
}

%a = func(%pre:i32, %p_indices:array<u32, 1>, %post:i32):vec4<i32> {
  $B2: {
    %6:u32 = access %p_indices, 0u
    %7:ptr<workgroup, vec4<i32>, read_write> = access %W, %6
    %8:vec4<i32> = load %7
    ret %8
  }
}
%b = func():void {
  $B3: {
    %10:u32 = convert 3i
    %11:array<u32, 1> = construct %10
    %12:vec4<i32> = call %a, 10i, %11, 20i
    ret
  }
}
)";

    Run(DirectVariableAccess, DirectVariableAccessOptions{});

    EXPECT_EQ(expect, str());
}

TEST_F(IR_DirectVariableAccessTest_WorkgroupAS, Param_ptr_vec4i32_Via_array_StaticWrite) {
    Var* W = nullptr;
    b.Append(b.ir.root_block,
             [&] {  //
                 W = b.Var<workgroup, array<vec4<i32>, 8>>("W");
             });

    auto* fn_a = b.Function("a", ty.void_());
    auto* fn_a_p = b.FunctionParam("p", ty.ptr<workgroup, vec4<i32>>());
    fn_a->SetParams({
        b.FunctionParam("pre", ty.i32()),
        fn_a_p,
        b.FunctionParam("post", ty.i32()),
    });
    b.Append(fn_a->Block(), [&] {
        b.Store(fn_a_p, b.Splat<vec4<i32>>(0_i));
        b.Return(fn_a);
    });

    auto* fn_b = b.Function("b", ty.void_());
    b.Append(fn_b->Block(), [&] {
        auto* access = b.Access(ty.ptr<workgroup, vec4<i32>>(), W, 3_i);
        b.Call(fn_a, 10_i, access, 20_i);
        b.Return(fn_b);
    });

    auto* src = R"(
$B1: {  # root
  %W:ptr<workgroup, array<vec4<i32>, 8>, read_write> = var undef
}

%a = func(%pre:i32, %p:ptr<workgroup, vec4<i32>, read_write>, %post:i32):void {
  $B2: {
    store %p, vec4<i32>(0i)
    ret
  }
}
%b = func():void {
  $B3: {
    %7:ptr<workgroup, vec4<i32>, read_write> = access %W, 3i
    %8:void = call %a, 10i, %7, 20i
    ret
  }
}
)";

    EXPECT_EQ(src, str());

    auto* expect = R"(
$B1: {  # root
  %W:ptr<workgroup, array<vec4<i32>, 8>, read_write> = var undef
}

%a = func(%pre:i32, %p_indices:array<u32, 1>, %post:i32):void {
  $B2: {
    %6:u32 = access %p_indices, 0u
    %7:ptr<workgroup, vec4<i32>, read_write> = access %W, %6
    store %7, vec4<i32>(0i)
    ret
  }
}
%b = func():void {
  $B3: {
    %9:u32 = convert 3i
    %10:array<u32, 1> = construct %9
    %11:void = call %a, 10i, %10, 20i
    ret
  }
}
)";

    Run(DirectVariableAccess, DirectVariableAccessOptions{});

    EXPECT_EQ(expect, str());
}

TEST_F(IR_DirectVariableAccessTest_WorkgroupAS, CallChaining) {
    auto* Inner =
        ty.Struct(mod.symbols.New("Inner"), {
                                                {mod.symbols.Register("mat"), ty.mat3x4<f32>()},
                                            });
    auto* Outer =
        ty.Struct(mod.symbols.New("Outer"), {
                                                {mod.symbols.Register("arr"), ty.array(Inner, 4)},
                                                {mod.symbols.Register("mat"), ty.mat3x4<f32>()},
                                            });
    Var* W = nullptr;
    b.Append(b.ir.root_block,
             [&] {  //
                 W = b.Var("W", ty.ptr<workgroup>(Outer));
             });

    auto* fn_0 = b.Function("f0", ty.f32());
    auto* fn_0_p = b.FunctionParam("p", ty.ptr<workgroup, vec4<f32>>());
    fn_0->SetParams({fn_0_p});
    b.Append(fn_0->Block(), [&] { b.Return(fn_0, b.LoadVectorElement(fn_0_p, 0_u)); });

    auto* fn_1 = b.Function("f1", ty.f32());
    auto* fn_1_p = b.FunctionParam("p", ty.ptr<workgroup, mat3x4<f32>>());
    fn_1->SetParams({fn_1_p});
    b.Append(fn_1->Block(), [&] {
        auto* res = b.Var<function, f32>("res");
        {
            // res += f0(&(*p)[1]);
            auto* call_0 = b.Call(fn_0, b.Access(ty.ptr<workgroup, vec4<f32>>(), fn_1_p, 1_i));
            b.Store(res, b.Add(ty.f32(), b.Load(res), call_0));
        }
        {
            // let p_vec = &(*p)[1];
            // res += f0(p_vec);
            auto* p_vec = b.Access(ty.ptr<workgroup, vec4<f32>>(), fn_1_p, 1_i);
            b.ir.SetName(p_vec, "p_vec");
            auto* call_0 = b.Call(fn_0, p_vec);
            b.Store(res, b.Add(ty.f32(), b.Load(res), call_0));
        }
        {
            // res += f0(&U.arr[2].mat[1]);
            auto* access = b.Access(ty.ptr<workgroup, vec4<f32>>(), W, 0_u, 2_i, 0_u, 1_i);
            auto* call_0 = b.Call(fn_0, access);
            b.Store(res, b.Add(ty.f32(), b.Load(res), call_0));
        }
        {
            // let p_vec = &U.arr[2].mat[1];
            // res += f0(p_vec);
            auto* p_vec = b.Access(ty.ptr<workgroup, vec4<f32>>(), W, 0_u, 2_i, 0_u, 1_i);
            b.ir.SetName(p_vec, "p_vec");
            auto* call_0 = b.Call(fn_0, p_vec);
            b.Store(res, b.Add(ty.f32(), b.Load(res), call_0));
        }

        b.Return(fn_1, b.Load(res));
    });

    auto* fn_2 = b.Function("f2", ty.f32());
    auto* fn_2_p = b.FunctionParam("p", ty.ptr<workgroup>(Inner));
    fn_2->SetParams({fn_2_p});
    b.Append(fn_2->Block(), [&] {
        auto* p_mat = b.Access(ty.ptr<workgroup, mat3x4<f32>>(), fn_2_p, 0_u);
        b.ir.SetName(p_mat, "p_mat");
        b.Return(fn_2, b.Call(fn_1, p_mat));
    });

    auto* fn_3 = b.Function("f3", ty.f32());
    auto* fn_3_p0 = b.FunctionParam("p0", ty.ptr<workgroup>(ty.array(Inner, 4)));
    auto* fn_3_p1 = b.FunctionParam("p1", ty.ptr<workgroup, mat3x4<f32>>());
    fn_3->SetParams({fn_3_p0, fn_3_p1});
    b.Append(fn_3->Block(), [&] {
        auto* p0_inner = b.Access(ty.ptr<workgroup>(Inner), fn_3_p0, 3_i);
        b.ir.SetName(p0_inner, "p0_inner");
        auto* call_0 = b.Call(ty.f32(), fn_2, p0_inner);
        auto* call_1 = b.Call(ty.f32(), fn_1, fn_3_p1);
        b.Return(fn_3, b.Add(ty.f32(), call_0, call_1));
    });

    auto* fn_4 = b.Function("f4", ty.f32());
    auto* fn_4_p = b.FunctionParam("p", ty.ptr<workgroup>(Outer));
    fn_4->SetParams({fn_4_p});
    b.Append(fn_4->Block(), [&] {
        auto* access_0 = b.Access(ty.ptr<workgroup>(ty.array(Inner, 4)), fn_4_p, 0_u);
        auto* access_1 = b.Access(ty.ptr<workgroup, mat3x4<f32>>(), W, 1_u);
        b.Return(fn_4, b.Call(ty.f32(), fn_3, access_0, access_1));
    });

    auto* fn_b = b.Function("b", ty.void_());
    b.Append(fn_b->Block(), [&] {
        b.Call(ty.f32(), fn_4, W);
        b.Return(fn_b);
    });

    auto* src = R"(
Inner = struct @align(16) {
  mat:mat3x4<f32> @offset(0)
}

Outer = struct @align(16) {
  arr:array<Inner, 4> @offset(0)
  mat:mat3x4<f32> @offset(192)
}

$B1: {  # root
  %W:ptr<workgroup, Outer, read_write> = var undef
}

%f0 = func(%p:ptr<workgroup, vec4<f32>, read_write>):f32 {
  $B2: {
    %4:f32 = load_vector_element %p, 0u
    ret %4
  }
}
%f1 = func(%p_1:ptr<workgroup, mat3x4<f32>, read_write>):f32 {  # %p_1: 'p'
  $B3: {
    %res:ptr<function, f32, read_write> = var undef
    %8:ptr<workgroup, vec4<f32>, read_write> = access %p_1, 1i
    %9:f32 = call %f0, %8
    %10:f32 = load %res
    %11:f32 = add %10, %9
    store %res, %11
    %p_vec:ptr<workgroup, vec4<f32>, read_write> = access %p_1, 1i
    %13:f32 = call %f0, %p_vec
    %14:f32 = load %res
    %15:f32 = add %14, %13
    store %res, %15
    %16:ptr<workgroup, vec4<f32>, read_write> = access %W, 0u, 2i, 0u, 1i
    %17:f32 = call %f0, %16
    %18:f32 = load %res
    %19:f32 = add %18, %17
    store %res, %19
    %p_vec_1:ptr<workgroup, vec4<f32>, read_write> = access %W, 0u, 2i, 0u, 1i  # %p_vec_1: 'p_vec'
    %21:f32 = call %f0, %p_vec_1
    %22:f32 = load %res
    %23:f32 = add %22, %21
    store %res, %23
    %24:f32 = load %res
    ret %24
  }
}
%f2 = func(%p_2:ptr<workgroup, Inner, read_write>):f32 {  # %p_2: 'p'
  $B4: {
    %p_mat:ptr<workgroup, mat3x4<f32>, read_write> = access %p_2, 0u
    %28:f32 = call %f1, %p_mat
    ret %28
  }
}
%f3 = func(%p0:ptr<workgroup, array<Inner, 4>, read_write>, %p1:ptr<workgroup, mat3x4<f32>, read_write>):f32 {
  $B5: {
    %p0_inner:ptr<workgroup, Inner, read_write> = access %p0, 3i
    %33:f32 = call %f2, %p0_inner
    %34:f32 = call %f1, %p1
    %35:f32 = add %33, %34
    ret %35
  }
}
%f4 = func(%p_3:ptr<workgroup, Outer, read_write>):f32 {  # %p_3: 'p'
  $B6: {
    %38:ptr<workgroup, array<Inner, 4>, read_write> = access %p_3, 0u
    %39:ptr<workgroup, mat3x4<f32>, read_write> = access %W, 1u
    %40:f32 = call %f3, %38, %39
    ret %40
  }
}
%b = func():void {
  $B7: {
    %42:f32 = call %f4, %W
    ret
  }
}
)";

    EXPECT_EQ(src, str());

    auto* expect = R"(
Inner = struct @align(16) {
  mat:mat3x4<f32> @offset(0)
}

Outer = struct @align(16) {
  arr:array<Inner, 4> @offset(0)
  mat:mat3x4<f32> @offset(192)
}

$B1: {  # root
  %W:ptr<workgroup, Outer, read_write> = var undef
}

%f0 = func(%p_indices:array<u32, 1>):f32 {
  $B2: {
    %4:u32 = access %p_indices, 0u
    %5:ptr<workgroup, vec4<f32>, read_write> = access %W, 1u, %4
    %6:f32 = load_vector_element %5, 0u
    ret %6
  }
}
%f0_1 = func(%p_indices_1:array<u32, 2>):f32 {  # %f0_1: 'f0', %p_indices_1: 'p_indices'
  $B3: {
    %9:u32 = access %p_indices_1, 0u
    %10:u32 = access %p_indices_1, 1u
    %11:ptr<workgroup, vec4<f32>, read_write> = access %W, 0u, %9, 0u, %10
    %12:f32 = load_vector_element %11, 0u
    ret %12
  }
}
%f1 = func():f32 {
  $B4: {
    %res:ptr<function, f32, read_write> = var undef
    %15:u32 = convert 1i
    %16:array<u32, 1> = construct %15
    %17:f32 = call %f0, %16
    %18:f32 = load %res
    %19:f32 = add %18, %17
    store %res, %19
    %20:u32 = convert 1i
    %21:array<u32, 1> = construct %20
    %22:f32 = call %f0, %21
    %23:f32 = load %res
    %24:f32 = add %23, %22
    store %res, %24
    %25:u32 = convert 2i
    %26:u32 = convert 1i
    %27:array<u32, 2> = construct %25, %26
    %28:f32 = call %f0_1, %27
    %29:f32 = load %res
    %30:f32 = add %29, %28
    store %res, %30
    %31:u32 = convert 2i
    %32:u32 = convert 1i
    %33:array<u32, 2> = construct %31, %32
    %34:f32 = call %f0_1, %33
    %35:f32 = load %res
    %36:f32 = add %35, %34
    store %res, %36
    %37:f32 = load %res
    ret %37
  }
}
%f1_1 = func(%p_indices_2:array<u32, 1>):f32 {  # %f1_1: 'f1', %p_indices_2: 'p_indices'
  $B5: {
    %40:u32 = access %p_indices_2, 0u
    %res_1:ptr<function, f32, read_write> = var undef  # %res_1: 'res'
    %42:u32 = convert 1i
    %43:array<u32, 2> = construct %40, %42
    %44:f32 = call %f0_1, %43
    %45:f32 = load %res_1
    %46:f32 = add %45, %44
    store %res_1, %46
    %47:u32 = convert 1i
    %48:array<u32, 2> = construct %40, %47
    %49:f32 = call %f0_1, %48
    %50:f32 = load %res_1
    %51:f32 = add %50, %49
    store %res_1, %51
    %52:u32 = convert 2i
    %53:u32 = convert 1i
    %54:array<u32, 2> = construct %52, %53
    %55:f32 = call %f0_1, %54
    %56:f32 = load %res_1
    %57:f32 = add %56, %55
    store %res_1, %57
    %58:u32 = convert 2i
    %59:u32 = convert 1i
    %60:array<u32, 2> = construct %58, %59
    %61:f32 = call %f0_1, %60
    %62:f32 = load %res_1
    %63:f32 = add %62, %61
    store %res_1, %63
    %64:f32 = load %res_1
    ret %64
  }
}
%f2 = func(%p_indices_3:array<u32, 1>):f32 {  # %p_indices_3: 'p_indices'
  $B6: {
    %67:u32 = access %p_indices_3, 0u
    %68:array<u32, 1> = construct %67
    %69:f32 = call %f1_1, %68
    ret %69
  }
}
%f3 = func():f32 {
  $B7: {
    %71:u32 = convert 3i
    %72:array<u32, 1> = construct %71
    %73:f32 = call %f2, %72
    %74:f32 = call %f1
    %75:f32 = add %73, %74
    ret %75
  }
}
%f4 = func():f32 {
  $B8: {
    %77:f32 = call %f3
    ret %77
  }
}
%b = func():void {
  $B9: {
    %79:f32 = call %f4
    ret
  }
}
)";

    Run(DirectVariableAccess, DirectVariableAccessOptions{});

    EXPECT_EQ(expect, str());
}

TEST_F(IR_DirectVariableAccessTest_WorkgroupAS, CallChaining2) {
    auto* T3 = ty.vec4<i32>();
    auto* T2 = ty.array(T3, 5);
    auto* T1 = ty.array(T2, 5);
    auto* T = ty.array(T1, 5);

    Var* input = nullptr;
    b.Append(b.ir.root_block,
             [&] {  //
                 input = b.Var("U", ty.ptr<workgroup>(T));
             });

    auto* f2 = b.Function("f2", T3);
    {
        auto* p = b.FunctionParam("p", ty.ptr<workgroup>(T2));
        f2->SetParams({p});
        b.Append(f2->Block(),
                 [&] { b.Return(f2, b.Load(b.Access<ptr<workgroup, vec4<i32>>>(p, 3_u))); });
    }

    auto* f1 = b.Function("f1", T3);
    {
        auto* p = b.FunctionParam("p", ty.ptr<workgroup>(T1));
        f1->SetParams({p});
        b.Append(f1->Block(),
                 [&] { b.Return(f1, b.Call(f2, b.Access(ty.ptr<workgroup>(T2), p, 2_u))); });
    }

    auto* f0 = b.Function("f0", T3);
    {
        auto* p = b.FunctionParam("p", ty.ptr<workgroup>(T));
        f0->SetParams({p});
        b.Append(f0->Block(),
                 [&] { b.Return(f0, b.Call(f1, b.Access(ty.ptr<workgroup>(T1), p, 1_u))); });
    }

    auto* main = b.Function("main", ty.void_());
    b.Append(main->Block(), [&] {
        b.Call(f0, input);
        b.Return(main);
    });

    auto* src = R"(
$B1: {  # root
  %U:ptr<workgroup, array<array<array<vec4<i32>, 5>, 5>, 5>, read_write> = var undef
}

%f2 = func(%p:ptr<workgroup, array<vec4<i32>, 5>, read_write>):vec4<i32> {
  $B2: {
    %4:ptr<workgroup, vec4<i32>, read_write> = access %p, 3u
    %5:vec4<i32> = load %4
    ret %5
  }
}
%f1 = func(%p_1:ptr<workgroup, array<array<vec4<i32>, 5>, 5>, read_write>):vec4<i32> {  # %p_1: 'p'
  $B3: {
    %8:ptr<workgroup, array<vec4<i32>, 5>, read_write> = access %p_1, 2u
    %9:vec4<i32> = call %f2, %8
    ret %9
  }
}
%f0 = func(%p_2:ptr<workgroup, array<array<array<vec4<i32>, 5>, 5>, 5>, read_write>):vec4<i32> {  # %p_2: 'p'
  $B4: {
    %12:ptr<workgroup, array<array<vec4<i32>, 5>, 5>, read_write> = access %p_2, 1u
    %13:vec4<i32> = call %f1, %12
    ret %13
  }
}
%main = func():void {
  $B5: {
    %15:vec4<i32> = call %f0, %U
    ret
  }
}
)";

    EXPECT_EQ(src, str());

    auto* expect = R"(
$B1: {  # root
  %U:ptr<workgroup, array<array<array<vec4<i32>, 5>, 5>, 5>, read_write> = var undef
}

%f2 = func(%p_indices:array<u32, 2>):vec4<i32> {
  $B2: {
    %4:u32 = access %p_indices, 0u
    %5:u32 = access %p_indices, 1u
    %6:ptr<workgroup, array<vec4<i32>, 5>, read_write> = access %U, %4, %5
    %7:ptr<workgroup, vec4<i32>, read_write> = access %6, 3u
    %8:vec4<i32> = load %7
    ret %8
  }
}
%f1 = func(%p_indices_1:array<u32, 1>):vec4<i32> {  # %p_indices_1: 'p_indices'
  $B3: {
    %11:u32 = access %p_indices_1, 0u
    %12:array<u32, 2> = construct %11, 2u
    %13:vec4<i32> = call %f2, %12
    ret %13
  }
}
%f0 = func():vec4<i32> {
  $B4: {
    %15:array<u32, 1> = construct 1u
    %16:vec4<i32> = call %f1, %15
    ret %16
  }
}
%main = func():void {
  $B5: {
    %18:vec4<i32> = call %f0
    ret
  }
}
)";

    Run(DirectVariableAccess, DirectVariableAccessOptions{});

    EXPECT_EQ(expect, str());
}

}  // namespace workgroup_as_tests

////////////////////////////////////////////////////////////////////////////////
// 'private' address space
////////////////////////////////////////////////////////////////////////////////
namespace private_as_tests {

using IR_DirectVariableAccessTest_PrivateAS = TransformTest;

TEST_F(IR_DirectVariableAccessTest_PrivateAS, Enabled_Param_ptr_i32_read) {
    Var* P = nullptr;
    b.Append(b.ir.root_block,
             [&] {  //
                 P = b.Var("P", ty.ptr<private_, i32>());
             });

    auto* fn_a = b.Function("a", ty.i32());
    auto* fn_a_p = b.FunctionParam("p", ty.ptr<private_, i32>());
    fn_a->SetParams({
        b.FunctionParam("pre", ty.i32()),
        fn_a_p,
        b.FunctionParam("post", ty.i32()),
    });
    b.Append(fn_a->Block(), [&] { b.Return(fn_a, b.Load(fn_a_p)); });

    auto* fn_b = b.Function("b", ty.void_());
    b.Append(fn_b->Block(), [&] {
        b.Call(fn_a, 10_i, P, 20_i);
        b.Return(fn_b);
    });

    auto* src = R"(
$B1: {  # root
  %P:ptr<private, i32, read_write> = var undef
}

%a = func(%pre:i32, %p:ptr<private, i32, read_write>, %post:i32):i32 {
  $B2: {
    %6:i32 = load %p
    ret %6
  }
}
%b = func():void {
  $B3: {
    %8:i32 = call %a, 10i, %P, 20i
    ret
  }
}
)";

    EXPECT_EQ(src, str());

    auto* expect = R"(
$B1: {  # root
  %P:ptr<private, i32, read_write> = var undef
}

%a = func(%pre:i32, %post:i32):i32 {
  $B2: {
    %5:i32 = load %P
    ret %5
  }
}
%b = func():void {
  $B3: {
    %7:i32 = call %a, 10i, 20i
    ret
  }
}
)";

    Run(DirectVariableAccess, kTransformPrivate);

    EXPECT_EQ(expect, str());
}

TEST_F(IR_DirectVariableAccessTest_PrivateAS, Enabled_Param_ptr_i32_write) {
    Var* P = nullptr;
    b.Append(b.ir.root_block,
             [&] {  //
                 P = b.Var("P", ty.ptr<private_, i32>());
             });

    auto* fn_a = b.Function("a", ty.void_());
    auto* fn_a_p = b.FunctionParam("p", ty.ptr<private_, i32>());
    fn_a->SetParams({
        b.FunctionParam("pre", ty.i32()),
        fn_a_p,
        b.FunctionParam("post", ty.i32()),
    });
    b.Append(fn_a->Block(), [&] {
        b.Store(fn_a_p, 42_i);
        b.Return(fn_a);
    });

    auto* fn_b = b.Function("b", ty.void_());
    b.Append(fn_b->Block(), [&] {
        b.Call(fn_a, 10_i, P, 20_i);
        b.Return(fn_b);
    });

    auto* src = R"(
$B1: {  # root
  %P:ptr<private, i32, read_write> = var undef
}

%a = func(%pre:i32, %p:ptr<private, i32, read_write>, %post:i32):void {
  $B2: {
    store %p, 42i
    ret
  }
}
%b = func():void {
  $B3: {
    %7:void = call %a, 10i, %P, 20i
    ret
  }
}
)";

    EXPECT_EQ(src, str());

    auto* expect = R"(
$B1: {  # root
  %P:ptr<private, i32, read_write> = var undef
}

%a = func(%pre:i32, %post:i32):void {
  $B2: {
    store %P, 42i
    ret
  }
}
%b = func():void {
  $B3: {
    %6:void = call %a, 10i, 20i
    ret
  }
}
)";

    Run(DirectVariableAccess, kTransformPrivate);

    EXPECT_EQ(expect, str());
}

TEST_F(IR_DirectVariableAccessTest_PrivateAS, Enabled_Param_ptr_i32_Via_struct_read) {
    auto* str_ = ty.Struct(mod.symbols.New("str"), {
                                                       {mod.symbols.Register("i"), ty.i32()},
                                                   });

    Var* P = nullptr;
    b.Append(b.ir.root_block,
             [&] {  //
                 P = b.Var("P", ty.ptr<private_>(str_));
             });

    auto* fn_a = b.Function("a", ty.i32());
    auto* fn_a_p = b.FunctionParam("p", ty.ptr<private_, i32>());
    fn_a->SetParams({
        b.FunctionParam("pre", ty.i32()),
        fn_a_p,
        b.FunctionParam("post", ty.i32()),
    });
    b.Append(fn_a->Block(), [&] { b.Return(fn_a, b.Load(fn_a_p)); });

    auto* fn_b = b.Function("b", ty.void_());
    b.Append(fn_b->Block(), [&] {
        auto* access = b.Access(ty.ptr<private_, i32>(), P, 0_u);
        b.Call(fn_a, 10_i, access, 20_i);
        b.Return(fn_b);
    });

    auto* src = R"(
str = struct @align(4) {
  i:i32 @offset(0)
}

$B1: {  # root
  %P:ptr<private, str, read_write> = var undef
}

%a = func(%pre:i32, %p:ptr<private, i32, read_write>, %post:i32):i32 {
  $B2: {
    %6:i32 = load %p
    ret %6
  }
}
%b = func():void {
  $B3: {
    %8:ptr<private, i32, read_write> = access %P, 0u
    %9:i32 = call %a, 10i, %8, 20i
    ret
  }
}
)";

    EXPECT_EQ(src, str());

    auto* expect = R"(
str = struct @align(4) {
  i:i32 @offset(0)
}

$B1: {  # root
  %P:ptr<private, str, read_write> = var undef
}

%a = func(%pre:i32, %post:i32):i32 {
  $B2: {
    %5:ptr<private, i32, read_write> = access %P, 0u
    %6:i32 = load %5
    ret %6
  }
}
%b = func():void {
  $B3: {
    %8:i32 = call %a, 10i, 20i
    ret
  }
}
)";

    Run(DirectVariableAccess, kTransformPrivate);

    EXPECT_EQ(expect, str());
}

TEST_F(IR_DirectVariableAccessTest_PrivateAS, Disabled_Param_ptr_i32_Via_struct_read) {
    auto* str_ = ty.Struct(mod.symbols.New("str"), {
                                                       {mod.symbols.Register("i"), ty.i32()},
                                                   });

    Var* P = nullptr;
    b.Append(b.ir.root_block,
             [&] {  //
                 P = b.Var("P", ty.ptr<private_>(str_));
             });

    auto* fn_a = b.Function("a", ty.i32());
    auto* fn_a_p = b.FunctionParam("p", ty.ptr<private_, i32>());
    fn_a->SetParams({
        b.FunctionParam("pre", ty.i32()),
        fn_a_p,
        b.FunctionParam("post", ty.i32()),
    });
    b.Append(fn_a->Block(), [&] { b.Return(fn_a, b.Load(fn_a_p)); });

    auto* fn_b = b.Function("b", ty.void_());
    b.Append(fn_b->Block(), [&] {
        auto* access = b.Access(ty.ptr<private_, i32>(), P, 0_u);
        b.Call(fn_a, 10_i, access, 20_i);
        b.Return(fn_b);
    });

    auto* src = R"(
str = struct @align(4) {
  i:i32 @offset(0)
}

$B1: {  # root
  %P:ptr<private, str, read_write> = var undef
}

%a = func(%pre:i32, %p:ptr<private, i32, read_write>, %post:i32):i32 {
  $B2: {
    %6:i32 = load %p
    ret %6
  }
}
%b = func():void {
  $B3: {
    %8:ptr<private, i32, read_write> = access %P, 0u
    %9:i32 = call %a, 10i, %8, 20i
    ret
  }
}
)";

    EXPECT_EQ(src, str());

    auto* expect = src;

    Run(DirectVariableAccess, DirectVariableAccessOptions{});

    EXPECT_EQ(expect, str());
}

TEST_F(IR_DirectVariableAccessTest_PrivateAS, Enabled_Param_ptr_arr_i32_Via_struct_write) {
    auto* str_ =
        ty.Struct(mod.symbols.New("str"), {
                                              {mod.symbols.Register("arr"), ty.array<i32, 4>()},
                                          });

    Var* P = nullptr;
    b.Append(b.ir.root_block,
             [&] {  //
                 P = b.Var("P", ty.ptr<private_>(str_));
             });

    auto* fn_a = b.Function("a", ty.void_());
    auto* fn_a_p = b.FunctionParam("p", ty.ptr<private_, array<i32, 4>>());
    fn_a->SetParams({
        b.FunctionParam("pre", ty.i32()),
        fn_a_p,
        b.FunctionParam("post", ty.i32()),
    });
    b.Append(fn_a->Block(), [&] {
        b.Store(fn_a_p, b.Splat<array<i32, 4>>(0_i));
        b.Return(fn_a);
    });

    auto* fn_b = b.Function("b", ty.void_());
    b.Append(fn_b->Block(), [&] {
        auto* access = b.Access(ty.ptr<private_, array<i32, 4>>(), P, 0_u);
        b.Call(fn_a, 10_i, access, 20_i);
        b.Return(fn_b);
    });

    auto* src = R"(
str = struct @align(4) {
  arr:array<i32, 4> @offset(0)
}

$B1: {  # root
  %P:ptr<private, str, read_write> = var undef
}

%a = func(%pre:i32, %p:ptr<private, array<i32, 4>, read_write>, %post:i32):void {
  $B2: {
    store %p, array<i32, 4>(0i)
    ret
  }
}
%b = func():void {
  $B3: {
    %7:ptr<private, array<i32, 4>, read_write> = access %P, 0u
    %8:void = call %a, 10i, %7, 20i
    ret
  }
}
)";

    EXPECT_EQ(src, str());

    auto* expect = R"(
str = struct @align(4) {
  arr:array<i32, 4> @offset(0)
}

$B1: {  # root
  %P:ptr<private, str, read_write> = var undef
}

%a = func(%pre:i32, %post:i32):void {
  $B2: {
    %5:ptr<private, array<i32, 4>, read_write> = access %P, 0u
    store %5, array<i32, 4>(0i)
    ret
  }
}
%b = func():void {
  $B3: {
    %7:void = call %a, 10i, 20i
    ret
  }
}
)";

    Run(DirectVariableAccess, kTransformPrivate);

    EXPECT_EQ(expect, str());
}

TEST_F(IR_DirectVariableAccessTest_PrivateAS, Disabled_Param_ptr_arr_i32_Via_struct_write) {
    auto* str_ =
        ty.Struct(mod.symbols.New("str"), {
                                              {mod.symbols.Register("arr"), ty.array<i32, 4>()},
                                          });

    Var* P = nullptr;
    b.Append(b.ir.root_block,
             [&] {  //
                 P = b.Var("P", ty.ptr<private_>(str_));
             });

    auto* fn_a = b.Function("a", ty.void_());
    auto* fn_a_p = b.FunctionParam("p", ty.ptr<private_, array<i32, 4>>());
    fn_a->SetParams({
        b.FunctionParam("pre", ty.i32()),
        fn_a_p,
        b.FunctionParam("post", ty.i32()),
    });
    b.Append(fn_a->Block(), [&] {
        b.Store(fn_a_p, b.Splat<array<i32, 4>>(0_i));
        b.Return(fn_a);
    });

    auto* fn_b = b.Function("b", ty.void_());
    b.Append(fn_b->Block(), [&] {
        auto* access = b.Access(ty.ptr<private_, array<i32, 4>>(), P, 0_u);
        b.Call(fn_a, 10_i, access, 20_i);
        b.Return(fn_b);
    });

    auto* src = R"(
str = struct @align(4) {
  arr:array<i32, 4> @offset(0)
}

$B1: {  # root
  %P:ptr<private, str, read_write> = var undef
}

%a = func(%pre:i32, %p:ptr<private, array<i32, 4>, read_write>, %post:i32):void {
  $B2: {
    store %p, array<i32, 4>(0i)
    ret
  }
}
%b = func():void {
  $B3: {
    %7:ptr<private, array<i32, 4>, read_write> = access %P, 0u
    %8:void = call %a, 10i, %7, 20i
    ret
  }
}
)";

    EXPECT_EQ(src, str());

    auto* expect = src;

    Run(DirectVariableAccess, DirectVariableAccessOptions{});

    EXPECT_EQ(expect, str());
}

TEST_F(IR_DirectVariableAccessTest_PrivateAS, Enabled_Param_ptr_i32_mixed) {
    auto* str_ = ty.Struct(mod.symbols.New("str"), {
                                                       {mod.symbols.Register("i"), ty.i32()},
                                                   });

    Var* Pi = nullptr;
    Var* Ps = nullptr;
    Var* Pa = nullptr;
    b.Append(b.ir.root_block,
             [&] {  //
                 Pi = b.Var("Pi", ty.ptr<private_, i32>());
                 Ps = b.Var("Ps", ty.ptr<private_>(str_));
                 Pa = b.Var("Pa", ty.ptr<private_, array<i32, 4>>());
             });

    auto* fn_a = b.Function("a", ty.i32());
    auto* fn_a_p = b.FunctionParam("p", ty.ptr<private_, i32>());
    fn_a->SetParams({
        b.FunctionParam("pre", ty.i32()),
        fn_a_p,
        b.FunctionParam("post", ty.i32()),
    });
    b.Append(fn_a->Block(), [&] { b.Return(fn_a, b.Load(fn_a_p)); });

    auto* fn_b = b.Function("b", ty.void_());
    b.Append(fn_b->Block(), [&] {
        {  // a(10, &Pi, 20);
            b.Call(fn_a, 10_i, Pi, 20_i);
        }
        {  // a(30, &Ps.i, 40);
            auto* access = b.Access(ty.ptr<private_, i32>(), Ps, 0_u);
            b.Call(fn_a, 30_i, access, 40_i);
        }
        {  // a(50, &Pa[2], 60);
            auto* access = b.Access(ty.ptr<private_, i32>(), Pa, 2_i);
            b.Call(fn_a, 50_i, access, 60_i);
        }
        b.Return(fn_b);
    });

    auto* src = R"(
str = struct @align(4) {
  i:i32 @offset(0)
}

$B1: {  # root
  %Pi:ptr<private, i32, read_write> = var undef
  %Ps:ptr<private, str, read_write> = var undef
  %Pa:ptr<private, array<i32, 4>, read_write> = var undef
}

%a = func(%pre:i32, %p:ptr<private, i32, read_write>, %post:i32):i32 {
  $B2: {
    %8:i32 = load %p
    ret %8
  }
}
%b = func():void {
  $B3: {
    %10:i32 = call %a, 10i, %Pi, 20i
    %11:ptr<private, i32, read_write> = access %Ps, 0u
    %12:i32 = call %a, 30i, %11, 40i
    %13:ptr<private, i32, read_write> = access %Pa, 2i
    %14:i32 = call %a, 50i, %13, 60i
    ret
  }
}
)";

    EXPECT_EQ(src, str());

    auto* expect = R"(
str = struct @align(4) {
  i:i32 @offset(0)
}

$B1: {  # root
  %Pi:ptr<private, i32, read_write> = var undef
  %Ps:ptr<private, str, read_write> = var undef
  %Pa:ptr<private, array<i32, 4>, read_write> = var undef
}

%a = func(%pre:i32, %post:i32):i32 {
  $B2: {
    %7:i32 = load %Pi
    ret %7
  }
}
%a_1 = func(%pre_1:i32, %post_1:i32):i32 {  # %a_1: 'a', %pre_1: 'pre', %post_1: 'post'
  $B3: {
    %11:ptr<private, i32, read_write> = access %Ps, 0u
    %12:i32 = load %11
    ret %12
  }
}
%a_2 = func(%pre_2:i32, %p_indices:array<u32, 1>, %post_2:i32):i32 {  # %a_2: 'a', %pre_2: 'pre', %post_2: 'post'
  $B4: {
    %17:u32 = access %p_indices, 0u
    %18:ptr<private, i32, read_write> = access %Pa, %17
    %19:i32 = load %18
    ret %19
  }
}
%b = func():void {
  $B5: {
    %21:i32 = call %a, 10i, 20i
    %22:i32 = call %a_1, 30i, 40i
    %23:u32 = convert 2i
    %24:array<u32, 1> = construct %23
    %25:i32 = call %a_2, 50i, %24, 60i
    ret
  }
}
)";

    Run(DirectVariableAccess, kTransformPrivate);

    EXPECT_EQ(expect, str());
}

TEST_F(IR_DirectVariableAccessTest_PrivateAS, Disabled_Param_ptr_i32_mixed) {
    auto* str_ = ty.Struct(mod.symbols.New("str"), {
                                                       {mod.symbols.Register("i"), ty.i32()},
                                                   });

    Var* Pi = nullptr;
    Var* Ps = nullptr;
    Var* Pa = nullptr;
    b.Append(b.ir.root_block,
             [&] {  //
                 Pi = b.Var("Pi", ty.ptr<private_, i32>());
                 Ps = b.Var("Ps", ty.ptr<private_>(str_));
                 Pa = b.Var("Pa", ty.ptr<private_, array<i32, 4>>());
             });

    auto* fn_a = b.Function("a", ty.i32());
    auto* fn_a_p = b.FunctionParam("p", ty.ptr<private_, i32>());
    fn_a->SetParams({
        b.FunctionParam("pre", ty.i32()),
        fn_a_p,
        b.FunctionParam("post", ty.i32()),
    });
    b.Append(fn_a->Block(), [&] { b.Return(fn_a, b.Load(fn_a_p)); });

    auto* fn_b = b.Function("b", ty.void_());
    b.Append(fn_b->Block(), [&] {
        {  // a(10, &Pi, 20);
            b.Call(fn_a, 10_i, Pi, 20_i);
        }
        {  // a(30, &Ps.i, 40);
            auto* access = b.Access(ty.ptr<private_, i32>(), Ps, 0_u);
            b.Call(fn_a, 30_i, access, 40_i);
        }
        {  // a(50, &Pa[2], 60);
            auto* access = b.Access(ty.ptr<private_, i32>(), Pa, 2_i);
            b.Call(fn_a, 50_i, access, 60_i);
        }
        b.Return(fn_b);
    });

    auto* src = R"(
str = struct @align(4) {
  i:i32 @offset(0)
}

$B1: {  # root
  %Pi:ptr<private, i32, read_write> = var undef
  %Ps:ptr<private, str, read_write> = var undef
  %Pa:ptr<private, array<i32, 4>, read_write> = var undef
}

%a = func(%pre:i32, %p:ptr<private, i32, read_write>, %post:i32):i32 {
  $B2: {
    %8:i32 = load %p
    ret %8
  }
}
%b = func():void {
  $B3: {
    %10:i32 = call %a, 10i, %Pi, 20i
    %11:ptr<private, i32, read_write> = access %Ps, 0u
    %12:i32 = call %a, 30i, %11, 40i
    %13:ptr<private, i32, read_write> = access %Pa, 2i
    %14:i32 = call %a, 50i, %13, 60i
    ret
  }
}
)";

    EXPECT_EQ(src, str());

    auto* expect = src;

    Run(DirectVariableAccess, DirectVariableAccessOptions{});

    EXPECT_EQ(expect, str());
}

TEST_F(IR_DirectVariableAccessTest_PrivateAS, Enabled_CallChaining) {
    auto* Inner =
        ty.Struct(mod.symbols.New("Inner"), {
                                                {mod.symbols.Register("mat"), ty.mat3x4<f32>()},
                                            });
    auto* Outer =
        ty.Struct(mod.symbols.New("Outer"), {
                                                {mod.symbols.Register("arr"), ty.array(Inner, 4)},
                                                {mod.symbols.Register("mat"), ty.mat3x4<f32>()},
                                            });
    Var* P = nullptr;
    b.Append(b.ir.root_block,
             [&] {  //
                 P = b.Var("P", ty.ptr<private_>(Outer));
             });

    auto* fn_0 = b.Function("f0", ty.f32());
    auto* fn_0_p = b.FunctionParam("p", ty.ptr<private_, vec4<f32>>());
    fn_0->SetParams({fn_0_p});
    b.Append(fn_0->Block(), [&] { b.Return(fn_0, b.LoadVectorElement(fn_0_p, 0_u)); });

    auto* fn_1 = b.Function("f1", ty.f32());
    auto* fn_1_p = b.FunctionParam("p", ty.ptr<private_, mat3x4<f32>>());
    fn_1->SetParams({fn_1_p});
    b.Append(fn_1->Block(), [&] {
        auto* res = b.Var<function, f32>("res");
        {
            // res += f0(&(*p)[1]);
            auto* call_0 = b.Call(fn_0, b.Access(ty.ptr<private_, vec4<f32>>(), fn_1_p, 1_i));
            b.Store(res, b.Add(ty.f32(), b.Load(res), call_0));
        }
        {
            // let p_vec = &(*p)[1];
            // res += f0(p_vec);
            auto* p_vec = b.Access(ty.ptr<private_, vec4<f32>>(), fn_1_p, 1_i);
            b.ir.SetName(p_vec, "p_vec");
            auto* call_0 = b.Call(fn_0, p_vec);
            b.Store(res, b.Add(ty.f32(), b.Load(res), call_0));
        }
        {
            // res += f0(&U.arr[2].mat[1]);
            auto* access = b.Access(ty.ptr<private_, vec4<f32>>(), P, 0_u, 2_i, 0_u, 1_i);
            auto* call_0 = b.Call(fn_0, access);
            b.Store(res, b.Add(ty.f32(), b.Load(res), call_0));
        }
        {
            // let p_vec = &U.arr[2].mat[1];
            // res += f0(p_vec);
            auto* p_vec = b.Access(ty.ptr<private_, vec4<f32>>(), P, 0_u, 2_i, 0_u, 1_i);
            b.ir.SetName(p_vec, "p_vec");
            auto* call_0 = b.Call(fn_0, p_vec);
            b.Store(res, b.Add(ty.f32(), b.Load(res), call_0));
        }

        b.Return(fn_1, b.Load(res));
    });

    auto* fn_2 = b.Function("f2", ty.f32());
    auto* fn_2_p = b.FunctionParam("p", ty.ptr<private_>(Inner));
    fn_2->SetParams({fn_2_p});
    b.Append(fn_2->Block(), [&] {
        auto* p_mat = b.Access(ty.ptr<private_, mat3x4<f32>>(), fn_2_p, 0_u);
        b.ir.SetName(p_mat, "p_mat");
        b.Return(fn_2, b.Call(fn_1, p_mat));
    });

    auto* fn_3 = b.Function("f3", ty.f32());
    auto* fn_3_p0 = b.FunctionParam("p0", ty.ptr<private_>(ty.array(Inner, 4)));
    auto* fn_3_p1 = b.FunctionParam("p1", ty.ptr<private_, mat3x4<f32>>());
    fn_3->SetParams({fn_3_p0, fn_3_p1});
    b.Append(fn_3->Block(), [&] {
        auto* p0_inner = b.Access(ty.ptr<private_>(Inner), fn_3_p0, 3_i);
        b.ir.SetName(p0_inner, "p0_inner");
        auto* call_0 = b.Call(ty.f32(), fn_2, p0_inner);
        auto* call_1 = b.Call(ty.f32(), fn_1, fn_3_p1);
        b.Return(fn_3, b.Add(ty.f32(), call_0, call_1));
    });

    auto* fn_4 = b.Function("f4", ty.f32());
    auto* fn_4_p = b.FunctionParam("p", ty.ptr<private_>(Outer));
    fn_4->SetParams({fn_4_p});
    b.Append(fn_4->Block(), [&] {
        auto* access_0 = b.Access(ty.ptr<private_>(ty.array(Inner, 4)), fn_4_p, 0_u);
        auto* access_1 = b.Access(ty.ptr<private_, mat3x4<f32>>(), P, 1_u);
        b.Return(fn_4, b.Call(ty.f32(), fn_3, access_0, access_1));
    });

    auto* fn_b = b.Function("b", ty.void_());
    b.Append(fn_b->Block(), [&] {
        b.Call(ty.f32(), fn_4, P);
        b.Return(fn_b);
    });

    auto* src = R"(
Inner = struct @align(16) {
  mat:mat3x4<f32> @offset(0)
}

Outer = struct @align(16) {
  arr:array<Inner, 4> @offset(0)
  mat:mat3x4<f32> @offset(192)
}

$B1: {  # root
  %P:ptr<private, Outer, read_write> = var undef
}

%f0 = func(%p:ptr<private, vec4<f32>, read_write>):f32 {
  $B2: {
    %4:f32 = load_vector_element %p, 0u
    ret %4
  }
}
%f1 = func(%p_1:ptr<private, mat3x4<f32>, read_write>):f32 {  # %p_1: 'p'
  $B3: {
    %res:ptr<function, f32, read_write> = var undef
    %8:ptr<private, vec4<f32>, read_write> = access %p_1, 1i
    %9:f32 = call %f0, %8
    %10:f32 = load %res
    %11:f32 = add %10, %9
    store %res, %11
    %p_vec:ptr<private, vec4<f32>, read_write> = access %p_1, 1i
    %13:f32 = call %f0, %p_vec
    %14:f32 = load %res
    %15:f32 = add %14, %13
    store %res, %15
    %16:ptr<private, vec4<f32>, read_write> = access %P, 0u, 2i, 0u, 1i
    %17:f32 = call %f0, %16
    %18:f32 = load %res
    %19:f32 = add %18, %17
    store %res, %19
    %p_vec_1:ptr<private, vec4<f32>, read_write> = access %P, 0u, 2i, 0u, 1i  # %p_vec_1: 'p_vec'
    %21:f32 = call %f0, %p_vec_1
    %22:f32 = load %res
    %23:f32 = add %22, %21
    store %res, %23
    %24:f32 = load %res
    ret %24
  }
}
%f2 = func(%p_2:ptr<private, Inner, read_write>):f32 {  # %p_2: 'p'
  $B4: {
    %p_mat:ptr<private, mat3x4<f32>, read_write> = access %p_2, 0u
    %28:f32 = call %f1, %p_mat
    ret %28
  }
}
%f3 = func(%p0:ptr<private, array<Inner, 4>, read_write>, %p1:ptr<private, mat3x4<f32>, read_write>):f32 {
  $B5: {
    %p0_inner:ptr<private, Inner, read_write> = access %p0, 3i
    %33:f32 = call %f2, %p0_inner
    %34:f32 = call %f1, %p1
    %35:f32 = add %33, %34
    ret %35
  }
}
%f4 = func(%p_3:ptr<private, Outer, read_write>):f32 {  # %p_3: 'p'
  $B6: {
    %38:ptr<private, array<Inner, 4>, read_write> = access %p_3, 0u
    %39:ptr<private, mat3x4<f32>, read_write> = access %P, 1u
    %40:f32 = call %f3, %38, %39
    ret %40
  }
}
%b = func():void {
  $B7: {
    %42:f32 = call %f4, %P
    ret
  }
}
)";

    EXPECT_EQ(src, str());

    auto* expect = R"(
Inner = struct @align(16) {
  mat:mat3x4<f32> @offset(0)
}

Outer = struct @align(16) {
  arr:array<Inner, 4> @offset(0)
  mat:mat3x4<f32> @offset(192)
}

$B1: {  # root
  %P:ptr<private, Outer, read_write> = var undef
}

%f0 = func(%p_indices:array<u32, 1>):f32 {
  $B2: {
    %4:u32 = access %p_indices, 0u
    %5:ptr<private, vec4<f32>, read_write> = access %P, 1u, %4
    %6:f32 = load_vector_element %5, 0u
    ret %6
  }
}
%f0_1 = func(%p_indices_1:array<u32, 2>):f32 {  # %f0_1: 'f0', %p_indices_1: 'p_indices'
  $B3: {
    %9:u32 = access %p_indices_1, 0u
    %10:u32 = access %p_indices_1, 1u
    %11:ptr<private, vec4<f32>, read_write> = access %P, 0u, %9, 0u, %10
    %12:f32 = load_vector_element %11, 0u
    ret %12
  }
}
%f1 = func():f32 {
  $B4: {
    %res:ptr<function, f32, read_write> = var undef
    %15:u32 = convert 1i
    %16:array<u32, 1> = construct %15
    %17:f32 = call %f0, %16
    %18:f32 = load %res
    %19:f32 = add %18, %17
    store %res, %19
    %20:u32 = convert 1i
    %21:array<u32, 1> = construct %20
    %22:f32 = call %f0, %21
    %23:f32 = load %res
    %24:f32 = add %23, %22
    store %res, %24
    %25:u32 = convert 2i
    %26:u32 = convert 1i
    %27:array<u32, 2> = construct %25, %26
    %28:f32 = call %f0_1, %27
    %29:f32 = load %res
    %30:f32 = add %29, %28
    store %res, %30
    %31:u32 = convert 2i
    %32:u32 = convert 1i
    %33:array<u32, 2> = construct %31, %32
    %34:f32 = call %f0_1, %33
    %35:f32 = load %res
    %36:f32 = add %35, %34
    store %res, %36
    %37:f32 = load %res
    ret %37
  }
}
%f1_1 = func(%p_indices_2:array<u32, 1>):f32 {  # %f1_1: 'f1', %p_indices_2: 'p_indices'
  $B5: {
    %40:u32 = access %p_indices_2, 0u
    %res_1:ptr<function, f32, read_write> = var undef  # %res_1: 'res'
    %42:u32 = convert 1i
    %43:array<u32, 2> = construct %40, %42
    %44:f32 = call %f0_1, %43
    %45:f32 = load %res_1
    %46:f32 = add %45, %44
    store %res_1, %46
    %47:u32 = convert 1i
    %48:array<u32, 2> = construct %40, %47
    %49:f32 = call %f0_1, %48
    %50:f32 = load %res_1
    %51:f32 = add %50, %49
    store %res_1, %51
    %52:u32 = convert 2i
    %53:u32 = convert 1i
    %54:array<u32, 2> = construct %52, %53
    %55:f32 = call %f0_1, %54
    %56:f32 = load %res_1
    %57:f32 = add %56, %55
    store %res_1, %57
    %58:u32 = convert 2i
    %59:u32 = convert 1i
    %60:array<u32, 2> = construct %58, %59
    %61:f32 = call %f0_1, %60
    %62:f32 = load %res_1
    %63:f32 = add %62, %61
    store %res_1, %63
    %64:f32 = load %res_1
    ret %64
  }
}
%f2 = func(%p_indices_3:array<u32, 1>):f32 {  # %p_indices_3: 'p_indices'
  $B6: {
    %67:u32 = access %p_indices_3, 0u
    %68:array<u32, 1> = construct %67
    %69:f32 = call %f1_1, %68
    ret %69
  }
}
%f3 = func():f32 {
  $B7: {
    %71:u32 = convert 3i
    %72:array<u32, 1> = construct %71
    %73:f32 = call %f2, %72
    %74:f32 = call %f1
    %75:f32 = add %73, %74
    ret %75
  }
}
%f4 = func():f32 {
  $B8: {
    %77:f32 = call %f3
    ret %77
  }
}
%b = func():void {
  $B9: {
    %79:f32 = call %f4
    ret
  }
}
)";

    Run(DirectVariableAccess, kTransformPrivate);

    EXPECT_EQ(expect, str());
}

TEST_F(IR_DirectVariableAccessTest_PrivateAS, Disabled_CallChaining) {
    auto* Inner =
        ty.Struct(mod.symbols.New("Inner"), {
                                                {mod.symbols.Register("mat"), ty.mat3x4<f32>()},
                                            });
    auto* Outer =
        ty.Struct(mod.symbols.New("Outer"), {
                                                {mod.symbols.Register("arr"), ty.array(Inner, 4)},
                                                {mod.symbols.Register("mat"), ty.mat3x4<f32>()},
                                            });
    Var* P = nullptr;
    b.Append(b.ir.root_block,
             [&] {  //
                 P = b.Var("P", ty.ptr<private_>(Outer));
             });

    auto* fn_0 = b.Function("f0", ty.f32());
    auto* fn_0_p = b.FunctionParam("p", ty.ptr<private_, vec4<f32>>());
    fn_0->SetParams({fn_0_p});
    b.Append(fn_0->Block(), [&] { b.Return(fn_0, b.LoadVectorElement(fn_0_p, 0_u)); });

    auto* fn_1 = b.Function("f1", ty.f32());
    auto* fn_1_p = b.FunctionParam("p", ty.ptr<private_, mat3x4<f32>>());
    fn_1->SetParams({fn_1_p});
    b.Append(fn_1->Block(), [&] {
        auto* res = b.Var<function, f32>("res");
        {
            // res += f0(&(*p)[1]);
            auto* call_0 = b.Call(fn_0, b.Access(ty.ptr<private_, vec4<f32>>(), fn_1_p, 1_i));
            b.Store(res, b.Add(ty.f32(), b.Load(res), call_0));
        }
        {
            // let p_vec = &(*p)[1];
            // res += f0(p_vec);
            auto* p_vec = b.Access(ty.ptr<private_, vec4<f32>>(), fn_1_p, 1_i);
            b.ir.SetName(p_vec, "p_vec");
            auto* call_0 = b.Call(fn_0, p_vec);
            b.Store(res, b.Add(ty.f32(), b.Load(res), call_0));
        }
        {
            // res += f0(&U.arr[2].mat[1]);
            auto* access = b.Access(ty.ptr<private_, vec4<f32>>(), P, 0_u, 2_i, 0_u, 1_i);
            auto* call_0 = b.Call(fn_0, access);
            b.Store(res, b.Add(ty.f32(), b.Load(res), call_0));
        }
        {
            // let p_vec = &U.arr[2].mat[1];
            // res += f0(p_vec);
            auto* p_vec = b.Access(ty.ptr<private_, vec4<f32>>(), P, 0_u, 2_i, 0_u, 1_i);
            b.ir.SetName(p_vec, "p_vec");
            auto* call_0 = b.Call(fn_0, p_vec);
            b.Store(res, b.Add(ty.f32(), b.Load(res), call_0));
        }

        b.Return(fn_1, b.Load(res));
    });

    auto* fn_2 = b.Function("f2", ty.f32());
    auto* fn_2_p = b.FunctionParam("p", ty.ptr<private_>(Inner));
    fn_2->SetParams({fn_2_p});
    b.Append(fn_2->Block(), [&] {
        auto* p_mat = b.Access(ty.ptr<private_, mat3x4<f32>>(), fn_2_p, 0_u);
        b.ir.SetName(p_mat, "p_mat");
        b.Return(fn_2, b.Call(fn_1, p_mat));
    });

    auto* fn_3 = b.Function("f3", ty.f32());
    auto* fn_3_p0 = b.FunctionParam("p0", ty.ptr<private_>(ty.array(Inner, 4)));
    auto* fn_3_p1 = b.FunctionParam("p1", ty.ptr<private_, mat3x4<f32>>());
    fn_3->SetParams({fn_3_p0, fn_3_p1});
    b.Append(fn_3->Block(), [&] {
        auto* p0_inner = b.Access(ty.ptr<private_>(Inner), fn_3_p0, 3_i);
        b.ir.SetName(p0_inner, "p0_inner");
        auto* call_0 = b.Call(ty.f32(), fn_2, p0_inner);
        auto* call_1 = b.Call(ty.f32(), fn_1, fn_3_p1);
        b.Return(fn_3, b.Add(ty.f32(), call_0, call_1));
    });

    auto* fn_4 = b.Function("f4", ty.f32());
    auto* fn_4_p = b.FunctionParam("p", ty.ptr<private_>(Outer));
    fn_4->SetParams({fn_4_p});
    b.Append(fn_4->Block(), [&] {
        auto* access_0 = b.Access(ty.ptr<private_>(ty.array(Inner, 4)), fn_4_p, 0_u);
        auto* access_1 = b.Access(ty.ptr<private_, mat3x4<f32>>(), P, 1_u);
        b.Return(fn_4, b.Call(ty.f32(), fn_3, access_0, access_1));
    });

    auto* fn_b = b.Function("b", ty.void_());
    b.Append(fn_b->Block(), [&] {
        b.Call(ty.f32(), fn_4, P);
        b.Return(fn_b);
    });

    auto* src = R"(
Inner = struct @align(16) {
  mat:mat3x4<f32> @offset(0)
}

Outer = struct @align(16) {
  arr:array<Inner, 4> @offset(0)
  mat:mat3x4<f32> @offset(192)
}

$B1: {  # root
  %P:ptr<private, Outer, read_write> = var undef
}

%f0 = func(%p:ptr<private, vec4<f32>, read_write>):f32 {
  $B2: {
    %4:f32 = load_vector_element %p, 0u
    ret %4
  }
}
%f1 = func(%p_1:ptr<private, mat3x4<f32>, read_write>):f32 {  # %p_1: 'p'
  $B3: {
    %res:ptr<function, f32, read_write> = var undef
    %8:ptr<private, vec4<f32>, read_write> = access %p_1, 1i
    %9:f32 = call %f0, %8
    %10:f32 = load %res
    %11:f32 = add %10, %9
    store %res, %11
    %p_vec:ptr<private, vec4<f32>, read_write> = access %p_1, 1i
    %13:f32 = call %f0, %p_vec
    %14:f32 = load %res
    %15:f32 = add %14, %13
    store %res, %15
    %16:ptr<private, vec4<f32>, read_write> = access %P, 0u, 2i, 0u, 1i
    %17:f32 = call %f0, %16
    %18:f32 = load %res
    %19:f32 = add %18, %17
    store %res, %19
    %p_vec_1:ptr<private, vec4<f32>, read_write> = access %P, 0u, 2i, 0u, 1i  # %p_vec_1: 'p_vec'
    %21:f32 = call %f0, %p_vec_1
    %22:f32 = load %res
    %23:f32 = add %22, %21
    store %res, %23
    %24:f32 = load %res
    ret %24
  }
}
%f2 = func(%p_2:ptr<private, Inner, read_write>):f32 {  # %p_2: 'p'
  $B4: {
    %p_mat:ptr<private, mat3x4<f32>, read_write> = access %p_2, 0u
    %28:f32 = call %f1, %p_mat
    ret %28
  }
}
%f3 = func(%p0:ptr<private, array<Inner, 4>, read_write>, %p1:ptr<private, mat3x4<f32>, read_write>):f32 {
  $B5: {
    %p0_inner:ptr<private, Inner, read_write> = access %p0, 3i
    %33:f32 = call %f2, %p0_inner
    %34:f32 = call %f1, %p1
    %35:f32 = add %33, %34
    ret %35
  }
}
%f4 = func(%p_3:ptr<private, Outer, read_write>):f32 {  # %p_3: 'p'
  $B6: {
    %38:ptr<private, array<Inner, 4>, read_write> = access %p_3, 0u
    %39:ptr<private, mat3x4<f32>, read_write> = access %P, 1u
    %40:f32 = call %f3, %38, %39
    ret %40
  }
}
%b = func():void {
  $B7: {
    %42:f32 = call %f4, %P
    ret
  }
}
)";

    EXPECT_EQ(src, str());

    auto* expect = src;

    Run(DirectVariableAccess, DirectVariableAccessOptions{});

    EXPECT_EQ(expect, str());
}

TEST_F(IR_DirectVariableAccessTest_PrivateAS, Enabled_CallChaining2) {
    auto* T3 = ty.vec4<i32>();
    auto* T2 = ty.array(T3, 5);
    auto* T1 = ty.array(T2, 5);
    auto* T = ty.array(T1, 5);

    Var* P = nullptr;
    b.Append(b.ir.root_block,
             [&] {  //
                 P = b.Var("P", ty.ptr<private_>(T));
             });

    auto* f2 = b.Function("f2", T3);
    {
        auto* p = b.FunctionParam("p", ty.ptr<private_>(T2));
        f2->SetParams({p});
        b.Append(f2->Block(), [&] {
            b.Return(f2, b.Load(b.Access<ptr<private_, vec4<i32>, read_write>>(p, 3_u)));
        });
    }

    auto* f1 = b.Function("f1", T3);
    {
        auto* p = b.FunctionParam("p", ty.ptr<private_>(T1));
        f1->SetParams({p});
        b.Append(f1->Block(),
                 [&] { b.Return(f1, b.Call(f2, b.Access(ty.ptr<private_>(T2), p, 2_u))); });
    }

    auto* f0 = b.Function("f0", T3);
    {
        auto* p = b.FunctionParam("p", ty.ptr<private_>(T));
        f0->SetParams({p});
        b.Append(f0->Block(),
                 [&] { b.Return(f0, b.Call(f1, b.Access(ty.ptr<private_>(T1), p, 1_u))); });
    }

    auto* main = b.Function("main", ty.void_());
    b.Append(main->Block(), [&] {
        b.Call(f0, P);
        b.Return(main);
    });

    auto* src = R"(
$B1: {  # root
  %P:ptr<private, array<array<array<vec4<i32>, 5>, 5>, 5>, read_write> = var undef
}

%f2 = func(%p:ptr<private, array<vec4<i32>, 5>, read_write>):vec4<i32> {
  $B2: {
    %4:ptr<private, vec4<i32>, read_write> = access %p, 3u
    %5:vec4<i32> = load %4
    ret %5
  }
}
%f1 = func(%p_1:ptr<private, array<array<vec4<i32>, 5>, 5>, read_write>):vec4<i32> {  # %p_1: 'p'
  $B3: {
    %8:ptr<private, array<vec4<i32>, 5>, read_write> = access %p_1, 2u
    %9:vec4<i32> = call %f2, %8
    ret %9
  }
}
%f0 = func(%p_2:ptr<private, array<array<array<vec4<i32>, 5>, 5>, 5>, read_write>):vec4<i32> {  # %p_2: 'p'
  $B4: {
    %12:ptr<private, array<array<vec4<i32>, 5>, 5>, read_write> = access %p_2, 1u
    %13:vec4<i32> = call %f1, %12
    ret %13
  }
}
%main = func():void {
  $B5: {
    %15:vec4<i32> = call %f0, %P
    ret
  }
}
)";

    EXPECT_EQ(src, str());

    auto* expect = R"(
$B1: {  # root
  %P:ptr<private, array<array<array<vec4<i32>, 5>, 5>, 5>, read_write> = var undef
}

%f2 = func(%p_indices:array<u32, 2>):vec4<i32> {
  $B2: {
    %4:u32 = access %p_indices, 0u
    %5:u32 = access %p_indices, 1u
    %6:ptr<private, array<vec4<i32>, 5>, read_write> = access %P, %4, %5
    %7:ptr<private, vec4<i32>, read_write> = access %6, 3u
    %8:vec4<i32> = load %7
    ret %8
  }
}
%f1 = func(%p_indices_1:array<u32, 1>):vec4<i32> {  # %p_indices_1: 'p_indices'
  $B3: {
    %11:u32 = access %p_indices_1, 0u
    %12:array<u32, 2> = construct %11, 2u
    %13:vec4<i32> = call %f2, %12
    ret %13
  }
}
%f0 = func():vec4<i32> {
  $B4: {
    %15:array<u32, 1> = construct 1u
    %16:vec4<i32> = call %f1, %15
    ret %16
  }
}
%main = func():void {
  $B5: {
    %18:vec4<i32> = call %f0
    ret
  }
}
)";

    Run(DirectVariableAccess, kTransformPrivate);

    EXPECT_EQ(expect, str());
}

TEST_F(IR_DirectVariableAccessTest_PrivateAS, Disabled_CallChaining2) {
    auto* T3 = ty.vec4<i32>();
    auto* T2 = ty.array(T3, 5);
    auto* T1 = ty.array(T2, 5);
    auto* T = ty.array(T1, 5);

    Var* P = nullptr;
    b.Append(b.ir.root_block,
             [&] {  //
                 P = b.Var("P", ty.ptr<private_>(T));
             });

    auto* f2 = b.Function("f2", T3);
    {
        auto* p = b.FunctionParam("p", ty.ptr<private_>(T2));
        f2->SetParams({p});
        b.Append(f2->Block(), [&] {
            b.Return(f2, b.Load(b.Access<ptr<private_, vec4<i32>, read_write>>(p, 3_u)));
        });
    }

    auto* f1 = b.Function("f1", T3);
    {
        auto* p = b.FunctionParam("p", ty.ptr<private_>(T1));
        f1->SetParams({p});
        b.Append(f1->Block(),
                 [&] { b.Return(f1, b.Call(f2, b.Access(ty.ptr<private_>(T2), p, 2_u))); });
    }

    auto* f0 = b.Function("f0", T3);
    {
        auto* p = b.FunctionParam("p", ty.ptr<private_>(T));
        f0->SetParams({p});
        b.Append(f0->Block(),
                 [&] { b.Return(f0, b.Call(f1, b.Access(ty.ptr<private_>(T1), p, 1_u))); });
    }

    auto* main = b.Function("main", ty.void_());
    b.Append(main->Block(), [&] {
        b.Call(f0, P);
        b.Return(main);
    });

    auto* src = R"(
$B1: {  # root
  %P:ptr<private, array<array<array<vec4<i32>, 5>, 5>, 5>, read_write> = var undef
}

%f2 = func(%p:ptr<private, array<vec4<i32>, 5>, read_write>):vec4<i32> {
  $B2: {
    %4:ptr<private, vec4<i32>, read_write> = access %p, 3u
    %5:vec4<i32> = load %4
    ret %5
  }
}
%f1 = func(%p_1:ptr<private, array<array<vec4<i32>, 5>, 5>, read_write>):vec4<i32> {  # %p_1: 'p'
  $B3: {
    %8:ptr<private, array<vec4<i32>, 5>, read_write> = access %p_1, 2u
    %9:vec4<i32> = call %f2, %8
    ret %9
  }
}
%f0 = func(%p_2:ptr<private, array<array<array<vec4<i32>, 5>, 5>, 5>, read_write>):vec4<i32> {  # %p_2: 'p'
  $B4: {
    %12:ptr<private, array<array<vec4<i32>, 5>, 5>, read_write> = access %p_2, 1u
    %13:vec4<i32> = call %f1, %12
    ret %13
  }
}
%main = func():void {
  $B5: {
    %15:vec4<i32> = call %f0, %P
    ret
  }
}
)";

    EXPECT_EQ(src, str());

    auto* expect = src;

    Run(DirectVariableAccess, DirectVariableAccessOptions{});

    EXPECT_EQ(expect, str());
}

}  // namespace private_as_tests

////////////////////////////////////////////////////////////////////////////////
// 'function' address space
////////////////////////////////////////////////////////////////////////////////
namespace function_as_tests {

using IR_DirectVariableAccessTest_FunctionAS = TransformTest;

TEST_F(IR_DirectVariableAccessTest_FunctionAS, Enabled_LocalPtr) {
    auto* fn = b.Function("f", ty.void_());
    b.Append(fn->Block(), [&] {
        auto* v = b.Var<function, i32>("v");
        auto* p = b.Let("p", v);
        b.Var<function>("x", b.Load(p));
        b.Return(fn);
    });

    auto* src = R"(
%f = func():void {
  $B1: {
    %v:ptr<function, i32, read_write> = var undef
    %p:ptr<function, i32, read_write> = let %v
    %4:i32 = load %p
    %x:ptr<function, i32, read_write> = var %4
    ret
  }
}
)";

    EXPECT_EQ(src, str());

    auto* expect = src;  // Nothing changes

    Run(DirectVariableAccess, DirectVariableAccessOptions{});

    EXPECT_EQ(expect, str());
}

TEST_F(IR_DirectVariableAccessTest_FunctionAS, Enabled_Param_ptr_i32_read) {
    auto* fn_a = b.Function("a", ty.i32());
    auto* fn_a_p = b.FunctionParam("p", ty.ptr<function, i32>());
    fn_a->SetParams({
        b.FunctionParam("pre", ty.i32()),
        fn_a_p,
        b.FunctionParam("post", ty.i32()),
    });
    b.Append(fn_a->Block(), [&] { b.Return(fn_a, b.Load(fn_a_p)); });

    auto* fn_b = b.Function("b", ty.void_());
    b.Append(fn_b->Block(), [&] {
        auto* F = b.Var<function, i32>("F");
        b.Call(fn_a, 10_i, F, 20_i);
        b.Return(fn_b);
    });

    auto* src = R"(
%a = func(%pre:i32, %p:ptr<function, i32, read_write>, %post:i32):i32 {
  $B1: {
    %5:i32 = load %p
    ret %5
  }
}
%b = func():void {
  $B2: {
    %F:ptr<function, i32, read_write> = var undef
    %8:i32 = call %a, 10i, %F, 20i
    ret
  }
}
)";

    EXPECT_EQ(src, str());

    auto* expect = R"(
%a = func(%pre:i32, %p_root:ptr<function, i32, read_write>, %post:i32):i32 {
  $B1: {
    %5:i32 = load %p_root
    ret %5
  }
}
%b = func():void {
  $B2: {
    %F:ptr<function, i32, read_write> = var undef
    %8:i32 = call %a, 10i, %F, 20i
    ret
  }
}
)";

    Run(DirectVariableAccess, kTransformFunction);

    EXPECT_EQ(expect, str());
}

TEST_F(IR_DirectVariableAccessTest_FunctionAS, Enabled_Param_ptr_i32_write) {
    auto* fn_a = b.Function("a", ty.void_());
    auto* fn_a_p = b.FunctionParam("p", ty.ptr<function, i32>());
    fn_a->SetParams({
        b.FunctionParam("pre", ty.i32()),
        fn_a_p,
        b.FunctionParam("post", ty.i32()),
    });
    b.Append(fn_a->Block(), [&] {
        b.Store(fn_a_p, 42_i);
        b.Return(fn_a);
    });

    auto* fn_b = b.Function("b", ty.void_());
    b.Append(fn_b->Block(), [&] {
        auto* F = b.Var<function, i32>("F");
        b.Call(fn_a, 10_i, F, 20_i);
        b.Return(fn_b);
    });

    auto* src = R"(
%a = func(%pre:i32, %p:ptr<function, i32, read_write>, %post:i32):void {
  $B1: {
    store %p, 42i
    ret
  }
}
%b = func():void {
  $B2: {
    %F:ptr<function, i32, read_write> = var undef
    %7:void = call %a, 10i, %F, 20i
    ret
  }
}
)";

    EXPECT_EQ(src, str());

    auto* expect = R"(
%a = func(%pre:i32, %p_root:ptr<function, i32, read_write>, %post:i32):void {
  $B1: {
    store %p_root, 42i
    ret
  }
}
%b = func():void {
  $B2: {
    %F:ptr<function, i32, read_write> = var undef
    %7:void = call %a, 10i, %F, 20i
    ret
  }
}
)";

    Run(DirectVariableAccess, kTransformFunction);

    EXPECT_EQ(expect, str());
}

TEST_F(IR_DirectVariableAccessTest_FunctionAS, Enabled_Param_ptr_i32_Via_struct_read) {
    auto* str_ = ty.Struct(mod.symbols.New("str"), {
                                                       {mod.symbols.Register("i"), ty.i32()},
                                                   });

    auto* fn_a = b.Function("a", ty.i32());
    auto* fn_a_p = b.FunctionParam("p", ty.ptr<function, i32>());
    fn_a->SetParams({
        b.FunctionParam("pre", ty.i32()),
        fn_a_p,
        b.FunctionParam("post", ty.i32()),
    });
    b.Append(fn_a->Block(), [&] { b.Return(fn_a, b.Load(fn_a_p)); });

    auto* fn_b = b.Function("b", ty.void_());
    b.Append(fn_b->Block(), [&] {
        auto* F = b.Var("F", ty.ptr<function>(str_));
        auto* access = b.Access(ty.ptr<function, i32>(), F, 0_u);
        b.Call(fn_a, 10_i, access, 20_i);
        b.Return(fn_b);
    });

    auto* src = R"(
str = struct @align(4) {
  i:i32 @offset(0)
}

%a = func(%pre:i32, %p:ptr<function, i32, read_write>, %post:i32):i32 {
  $B1: {
    %5:i32 = load %p
    ret %5
  }
}
%b = func():void {
  $B2: {
    %F:ptr<function, str, read_write> = var undef
    %8:ptr<function, i32, read_write> = access %F, 0u
    %9:i32 = call %a, 10i, %8, 20i
    ret
  }
}
)";

    EXPECT_EQ(src, str());

    auto* expect = R"(
str = struct @align(4) {
  i:i32 @offset(0)
}

%a = func(%pre:i32, %p_root:ptr<function, str, read_write>, %post:i32):i32 {
  $B1: {
    %5:ptr<function, i32, read_write> = access %p_root, 0u
    %6:i32 = load %5
    ret %6
  }
}
%b = func():void {
  $B2: {
    %F:ptr<function, str, read_write> = var undef
    %9:i32 = call %a, 10i, %F, 20i
    ret
  }
}
)";

    Run(DirectVariableAccess, kTransformFunction);

    EXPECT_EQ(expect, str());
}

TEST_F(IR_DirectVariableAccessTest_FunctionAS, Enabled_Param_ptr_arr_i32_Via_struct_write) {
    auto* str_ =
        ty.Struct(mod.symbols.New("str"), {
                                              {mod.symbols.Register("arr"), ty.array<i32, 4>()},
                                          });

    auto* fn_a = b.Function("a", ty.void_());
    auto* fn_a_p = b.FunctionParam("p", ty.ptr<function, array<i32, 4>>());
    fn_a->SetParams({
        b.FunctionParam("pre", ty.i32()),
        fn_a_p,
        b.FunctionParam("post", ty.i32()),
    });
    b.Append(fn_a->Block(), [&] {
        b.Store(fn_a_p, b.Splat<array<i32, 4>>(0_i));
        b.Return(fn_a);
    });

    auto* fn_b = b.Function("b", ty.void_());
    b.Append(fn_b->Block(), [&] {
        auto* F = b.Var("F", ty.ptr<function>(str_));
        auto* access = b.Access(ty.ptr<function, array<i32, 4>>(), F, 0_u);
        b.Call(fn_a, 10_i, access, 20_i);
        b.Return(fn_b);
    });

    auto* src = R"(
str = struct @align(4) {
  arr:array<i32, 4> @offset(0)
}

%a = func(%pre:i32, %p:ptr<function, array<i32, 4>, read_write>, %post:i32):void {
  $B1: {
    store %p, array<i32, 4>(0i)
    ret
  }
}
%b = func():void {
  $B2: {
    %F:ptr<function, str, read_write> = var undef
    %7:ptr<function, array<i32, 4>, read_write> = access %F, 0u
    %8:void = call %a, 10i, %7, 20i
    ret
  }
}
)";

    EXPECT_EQ(src, str());

    auto* expect = R"(
str = struct @align(4) {
  arr:array<i32, 4> @offset(0)
}

%a = func(%pre:i32, %p_root:ptr<function, str, read_write>, %post:i32):void {
  $B1: {
    %5:ptr<function, array<i32, 4>, read_write> = access %p_root, 0u
    store %5, array<i32, 4>(0i)
    ret
  }
}
%b = func():void {
  $B2: {
    %F:ptr<function, str, read_write> = var undef
    %8:void = call %a, 10i, %F, 20i
    ret
  }
}
)";

    Run(DirectVariableAccess, kTransformFunction);

    EXPECT_EQ(expect, str());
}

TEST_F(IR_DirectVariableAccessTest_FunctionAS, Enabled_Param_ptr_i32_mixed) {
    auto* str_ = ty.Struct(mod.symbols.New("str"), {
                                                       {mod.symbols.Register("i"), ty.i32()},
                                                   });

    auto* fn_a = b.Function("a", ty.i32());
    auto* fn_a_p = b.FunctionParam("p", ty.ptr<function, i32>());
    fn_a->SetParams({
        b.FunctionParam("pre", ty.i32()),
        fn_a_p,
        b.FunctionParam("post", ty.i32()),
    });
    b.Append(fn_a->Block(), [&] { b.Return(fn_a, b.Load(fn_a_p)); });

    auto* fn_b = b.Function("b", ty.void_());
    b.Append(fn_b->Block(), [&] {
        auto* Fi = b.Var("Fi", ty.ptr<function, i32>());
        auto* Fs = b.Var("Fs", ty.ptr<function>(str_));
        auto* Fa = b.Var("Fa", ty.ptr<function, array<i32, 4>>());
        {  // a(10, &Fi, 20);
            b.Call(fn_a, 10_i, Fi, 20_i);
        }
        {  // a(30, &Fs.i, 40);
            auto* access = b.Access(ty.ptr<function, i32>(), Fs, 0_u);
            b.Call(fn_a, 30_i, access, 40_i);
        }
        {  // a(50, &Fa[2], 60);
            auto* access = b.Access(ty.ptr<function, i32>(), Fa, 2_i);
            b.Call(fn_a, 50_i, access, 60_i);
        }
        b.Return(fn_b);
    });

    auto* src = R"(
str = struct @align(4) {
  i:i32 @offset(0)
}

%a = func(%pre:i32, %p:ptr<function, i32, read_write>, %post:i32):i32 {
  $B1: {
    %5:i32 = load %p
    ret %5
  }
}
%b = func():void {
  $B2: {
    %Fi:ptr<function, i32, read_write> = var undef
    %Fs:ptr<function, str, read_write> = var undef
    %Fa:ptr<function, array<i32, 4>, read_write> = var undef
    %10:i32 = call %a, 10i, %Fi, 20i
    %11:ptr<function, i32, read_write> = access %Fs, 0u
    %12:i32 = call %a, 30i, %11, 40i
    %13:ptr<function, i32, read_write> = access %Fa, 2i
    %14:i32 = call %a, 50i, %13, 60i
    ret
  }
}
)";

    EXPECT_EQ(src, str());

    auto* expect = R"(
str = struct @align(4) {
  i:i32 @offset(0)
}

%a = func(%pre:i32, %p_root:ptr<function, i32, read_write>, %post:i32):i32 {
  $B1: {
    %5:i32 = load %p_root
    ret %5
  }
}
%a_1 = func(%pre_1:i32, %p_root_1:ptr<function, str, read_write>, %post_1:i32):i32 {  # %a_1: 'a', %pre_1: 'pre', %p_root_1: 'p_root', %post_1: 'post'
  $B2: {
    %10:ptr<function, i32, read_write> = access %p_root_1, 0u
    %11:i32 = load %10
    ret %11
  }
}
%a_2 = func(%pre_2:i32, %p_root_2:ptr<function, array<i32, 4>, read_write>, %p_indices:array<u32, 1>, %post_2:i32):i32 {  # %a_2: 'a', %pre_2: 'pre', %p_root_2: 'p_root', %post_2: 'post'
  $B3: {
    %17:u32 = access %p_indices, 0u
    %18:ptr<function, i32, read_write> = access %p_root_2, %17
    %19:i32 = load %18
    ret %19
  }
}
%b = func():void {
  $B4: {
    %Fi:ptr<function, i32, read_write> = var undef
    %Fs:ptr<function, str, read_write> = var undef
    %Fa:ptr<function, array<i32, 4>, read_write> = var undef
    %24:i32 = call %a, 10i, %Fi, 20i
    %25:i32 = call %a_1, 30i, %Fs, 40i
    %26:u32 = convert 2i
    %27:array<u32, 1> = construct %26
    %28:i32 = call %a_2, 50i, %Fa, %27, 60i
    ret
  }
}
)";

    Run(DirectVariableAccess, kTransformFunction);

    EXPECT_EQ(expect, str());
}

TEST_F(IR_DirectVariableAccessTest_FunctionAS, Disabled_Param_ptr_i32_Via_struct_read) {
    auto* str_ = ty.Struct(mod.symbols.New("str"), {
                                                       {mod.symbols.Register("i"), ty.i32()},
                                                   });

    auto* fn_a = b.Function("a", ty.i32());
    auto* fn_a_p = b.FunctionParam("p", ty.ptr<function, i32>());
    fn_a->SetParams({
        b.FunctionParam("pre", ty.i32()),
        fn_a_p,
        b.FunctionParam("post", ty.i32()),
    });
    b.Append(fn_a->Block(), [&] { b.Return(fn_a, b.Load(fn_a_p)); });

    auto* fn_b = b.Function("b", ty.void_());
    b.Append(fn_b->Block(), [&] {
        auto* F = b.Var("F", ty.ptr<function>(str_));
        auto* access = b.Access(ty.ptr<function, i32>(), F, 0_u);
        b.Call(fn_a, 10_i, access, 20_i);
        b.Return(fn_b);
    });

    auto* src = R"(
str = struct @align(4) {
  i:i32 @offset(0)
}

%a = func(%pre:i32, %p:ptr<function, i32, read_write>, %post:i32):i32 {
  $B1: {
    %5:i32 = load %p
    ret %5
  }
}
%b = func():void {
  $B2: {
    %F:ptr<function, str, read_write> = var undef
    %8:ptr<function, i32, read_write> = access %F, 0u
    %9:i32 = call %a, 10i, %8, 20i
    ret
  }
}
)";

    EXPECT_EQ(src, str());

    auto* expect = src;

    Run(DirectVariableAccess, DirectVariableAccessOptions{});

    EXPECT_EQ(expect, str());
}

TEST_F(IR_DirectVariableAccessTest_FunctionAS, Disabled_Param_ptr_arr_i32_Via_struct_write) {
    auto* str_ =
        ty.Struct(mod.symbols.New("str"), {
                                              {mod.symbols.Register("arr"), ty.array<i32, 4>()},
                                          });

    auto* fn_a = b.Function("a", ty.void_());
    auto* fn_a_p = b.FunctionParam("p", ty.ptr<function, array<i32, 4>>());
    fn_a->SetParams({
        b.FunctionParam("pre", ty.i32()),
        fn_a_p,
        b.FunctionParam("post", ty.i32()),
    });
    b.Append(fn_a->Block(), [&] {
        b.Store(fn_a_p, b.Splat<array<i32, 4>>(0_i));
        b.Return(fn_a);
    });

    auto* fn_b = b.Function("b", ty.void_());
    b.Append(fn_b->Block(), [&] {
        auto* F = b.Var("F", ty.ptr<function>(str_));
        auto* access = b.Access(ty.ptr<function, array<i32, 4>>(), F, 0_u);
        b.Call(fn_a, 10_i, access, 20_i);
        b.Return(fn_b);
    });

    auto* src = R"(
str = struct @align(4) {
  arr:array<i32, 4> @offset(0)
}

%a = func(%pre:i32, %p:ptr<function, array<i32, 4>, read_write>, %post:i32):void {
  $B1: {
    store %p, array<i32, 4>(0i)
    ret
  }
}
%b = func():void {
  $B2: {
    %F:ptr<function, str, read_write> = var undef
    %7:ptr<function, array<i32, 4>, read_write> = access %F, 0u
    %8:void = call %a, 10i, %7, 20i
    ret
  }
}
)";

    EXPECT_EQ(src, str());

    auto* expect = src;

    Run(DirectVariableAccess, DirectVariableAccessOptions{});

    EXPECT_EQ(expect, str());
}

TEST_F(IR_DirectVariableAccessTest_FunctionAS, Enabled_CallChaining) {
    auto* Inner =
        ty.Struct(mod.symbols.New("Inner"), {
                                                {mod.symbols.Register("mat"), ty.mat3x4<f32>()},
                                            });
    auto* Outer =
        ty.Struct(mod.symbols.New("Outer"), {
                                                {mod.symbols.Register("arr"), ty.array(Inner, 4)},
                                                {mod.symbols.Register("mat"), ty.mat3x4<f32>()},
                                            });

    auto* f0 = b.Function("f0", ty.f32());
    {
        auto* p = b.FunctionParam("p", ty.ptr<function, vec4<f32>>());
        f0->SetParams({p});
        b.Append(f0->Block(), [&] { b.Return(f0, b.LoadVectorElement(p, 0_u)); });
    }

    auto* f1 = b.Function("f1", ty.f32());
    {
        auto* p = b.FunctionParam("p", ty.ptr<function, mat3x4<f32>>());
        f1->SetParams({p});
        b.Append(f1->Block(), [&] {
            auto* res = b.Var<function, f32>("res");
            {
                // res += f0(&(*p)[1]);
                auto* call_0 = b.Call(f0, b.Access(ty.ptr<function, vec4<f32>>(), p, 1_i));
                b.Store(res, b.Add(ty.f32(), b.Load(res), call_0));
            }
            {
                // let p_vec = &(*p)[1];
                // res += f0(p_vec);
                auto* p_vec = b.Access(ty.ptr<function, vec4<f32>>(), p, 1_i);
                b.ir.SetName(p_vec, "p_vec");
                auto* call_0 = b.Call(f0, p_vec);
                b.Store(res, b.Add(ty.f32(), b.Load(res), call_0));
            }
            b.Return(f1, b.Load(res));
        });
    }

    auto* f2 = b.Function("f2", ty.f32());
    {
        auto* p = b.FunctionParam("p", ty.ptr<function>(Inner));
        f2->SetParams({p});
        b.Append(f2->Block(), [&] {
            auto* p_mat = b.Access(ty.ptr<function, mat3x4<f32>>(), p, 0_u);
            b.ir.SetName(p_mat, "p_mat");
            b.Return(f2, b.Call(f1, p_mat));
        });
    }

    auto* f3 = b.Function("f3", ty.f32());
    {
        auto* p = b.FunctionParam("p", ty.ptr<function>(ty.array(Inner, 4)));
        f3->SetParams({p});
        b.Append(f3->Block(), [&] {
            auto* p_inner = b.Access(ty.ptr<function>(Inner), p, 3_i);
            b.ir.SetName(p_inner, "p_inner");
            b.Return(f3, b.Call(f2, p_inner));
        });
    }

    auto* f4 = b.Function("f4", ty.f32());
    {
        auto* p = b.FunctionParam("p", ty.ptr<function>(Outer));
        f4->SetParams({p});
        b.Append(f4->Block(), [&] {
            auto* access = b.Access(ty.ptr<function>(ty.array(Inner, 4)), p, 0_u);
            b.Return(f4, b.Call(f3, access));
        });
    }

    auto* fn_b = b.Function("b", ty.void_());
    b.Append(fn_b->Block(), [&] {
        auto F = b.Var("F", ty.ptr<function>(Outer));
        b.Call(f4, F);
        b.Return(fn_b);
    });

    auto* src = R"(
Inner = struct @align(16) {
  mat:mat3x4<f32> @offset(0)
}

Outer = struct @align(16) {
  arr:array<Inner, 4> @offset(0)
  mat:mat3x4<f32> @offset(192)
}

%f0 = func(%p:ptr<function, vec4<f32>, read_write>):f32 {
  $B1: {
    %3:f32 = load_vector_element %p, 0u
    ret %3
  }
}
%f1 = func(%p_1:ptr<function, mat3x4<f32>, read_write>):f32 {  # %p_1: 'p'
  $B2: {
    %res:ptr<function, f32, read_write> = var undef
    %7:ptr<function, vec4<f32>, read_write> = access %p_1, 1i
    %8:f32 = call %f0, %7
    %9:f32 = load %res
    %10:f32 = add %9, %8
    store %res, %10
    %p_vec:ptr<function, vec4<f32>, read_write> = access %p_1, 1i
    %12:f32 = call %f0, %p_vec
    %13:f32 = load %res
    %14:f32 = add %13, %12
    store %res, %14
    %15:f32 = load %res
    ret %15
  }
}
%f2 = func(%p_2:ptr<function, Inner, read_write>):f32 {  # %p_2: 'p'
  $B3: {
    %p_mat:ptr<function, mat3x4<f32>, read_write> = access %p_2, 0u
    %19:f32 = call %f1, %p_mat
    ret %19
  }
}
%f3 = func(%p_3:ptr<function, array<Inner, 4>, read_write>):f32 {  # %p_3: 'p'
  $B4: {
    %p_inner:ptr<function, Inner, read_write> = access %p_3, 3i
    %23:f32 = call %f2, %p_inner
    ret %23
  }
}
%f4 = func(%p_4:ptr<function, Outer, read_write>):f32 {  # %p_4: 'p'
  $B5: {
    %26:ptr<function, array<Inner, 4>, read_write> = access %p_4, 0u
    %27:f32 = call %f3, %26
    ret %27
  }
}
%b = func():void {
  $B6: {
    %F:ptr<function, Outer, read_write> = var undef
    %30:f32 = call %f4, %F
    ret
  }
}
)";

    EXPECT_EQ(src, str());

    auto* expect = R"(
Inner = struct @align(16) {
  mat:mat3x4<f32> @offset(0)
}

Outer = struct @align(16) {
  arr:array<Inner, 4> @offset(0)
  mat:mat3x4<f32> @offset(192)
}

%f0 = func(%p_root:ptr<function, Outer, read_write>, %p_indices:array<u32, 2>):f32 {
  $B1: {
    %4:u32 = access %p_indices, 0u
    %5:u32 = access %p_indices, 1u
    %6:ptr<function, vec4<f32>, read_write> = access %p_root, 0u, %4, 0u, %5
    %7:f32 = load_vector_element %6, 0u
    ret %7
  }
}
%f1 = func(%p_root_1:ptr<function, Outer, read_write>, %p_indices_1:array<u32, 1>):f32 {  # %p_root_1: 'p_root', %p_indices_1: 'p_indices'
  $B2: {
    %11:u32 = access %p_indices_1, 0u
    %res:ptr<function, f32, read_write> = var undef
    %13:u32 = convert 1i
    %14:array<u32, 2> = construct %11, %13
    %15:f32 = call %f0, %p_root_1, %14
    %16:f32 = load %res
    %17:f32 = add %16, %15
    store %res, %17
    %18:u32 = convert 1i
    %19:array<u32, 2> = construct %11, %18
    %20:f32 = call %f0, %p_root_1, %19
    %21:f32 = load %res
    %22:f32 = add %21, %20
    store %res, %22
    %23:f32 = load %res
    ret %23
  }
}
%f2 = func(%p_root_2:ptr<function, Outer, read_write>, %p_indices_2:array<u32, 1>):f32 {  # %p_root_2: 'p_root', %p_indices_2: 'p_indices'
  $B3: {
    %27:u32 = access %p_indices_2, 0u
    %28:array<u32, 1> = construct %27
    %29:f32 = call %f1, %p_root_2, %28
    ret %29
  }
}
%f3 = func(%p_root_3:ptr<function, Outer, read_write>):f32 {  # %p_root_3: 'p_root'
  $B4: {
    %32:u32 = convert 3i
    %33:array<u32, 1> = construct %32
    %34:f32 = call %f2, %p_root_3, %33
    ret %34
  }
}
%f4 = func(%p_root_4:ptr<function, Outer, read_write>):f32 {  # %p_root_4: 'p_root'
  $B5: {
    %37:f32 = call %f3, %p_root_4
    ret %37
  }
}
%b = func():void {
  $B6: {
    %F:ptr<function, Outer, read_write> = var undef
    %40:f32 = call %f4, %F
    ret
  }
}
)";

    Run(DirectVariableAccess, kTransformFunction);

    EXPECT_EQ(expect, str());
}

TEST_F(IR_DirectVariableAccessTest_FunctionAS, Disabled_CallChaining) {
    auto* Inner =
        ty.Struct(mod.symbols.New("Inner"), {
                                                {mod.symbols.Register("mat"), ty.mat3x4<f32>()},
                                            });
    auto* Outer =
        ty.Struct(mod.symbols.New("Outer"), {
                                                {mod.symbols.Register("arr"), ty.array(Inner, 4)},
                                                {mod.symbols.Register("mat"), ty.mat3x4<f32>()},
                                            });

    auto* f0 = b.Function("f0", ty.f32());
    {
        auto* p = b.FunctionParam("p", ty.ptr<function, vec4<f32>>());
        f0->SetParams({p});
        b.Append(f0->Block(), [&] { b.Return(f0, b.LoadVectorElement(p, 0_u)); });
    }

    auto* f1 = b.Function("f1", ty.f32());
    {
        auto* p = b.FunctionParam("p", ty.ptr<function, mat3x4<f32>>());
        f1->SetParams({p});
        b.Append(f1->Block(), [&] {
            auto* res = b.Var<function, f32>("res");
            {
                // res += f0(&(*p)[1]);
                auto* call_0 = b.Call(f0, b.Access(ty.ptr<function, vec4<f32>>(), p, 1_i));
                b.Store(res, b.Add(ty.f32(), b.Load(res), call_0));
            }
            {
                // let p_vec = &(*p)[1];
                // res += f0(p_vec);
                auto* p_vec = b.Access(ty.ptr<function, vec4<f32>>(), p, 1_i);
                b.ir.SetName(p_vec, "p_vec");
                auto* call_0 = b.Call(f0, p_vec);
                b.Store(res, b.Add(ty.f32(), b.Load(res), call_0));
            }
            b.Return(f1, b.Load(res));
        });
    }

    auto* f2 = b.Function("f2", ty.f32());
    {
        auto* p = b.FunctionParam("p", ty.ptr<function>(Inner));
        f2->SetParams({p});
        b.Append(f2->Block(), [&] {
            auto* p_mat = b.Access(ty.ptr<function, mat3x4<f32>>(), p, 0_u);
            b.ir.SetName(p_mat, "p_mat");
            b.Return(f2, b.Call(f1, p_mat));
        });
    }

    auto* f3 = b.Function("f3", ty.f32());
    {
        auto* p = b.FunctionParam("p", ty.ptr<function>(ty.array(Inner, 4)));
        f3->SetParams({p});
        b.Append(f3->Block(), [&] {
            auto* p_inner = b.Access(ty.ptr<function>(Inner), p, 3_i);
            b.ir.SetName(p_inner, "p_inner");
            b.Return(f3, b.Call(f2, p_inner));
        });
    }

    auto* f4 = b.Function("f4", ty.f32());
    {
        auto* p = b.FunctionParam("p", ty.ptr<function>(Outer));
        f4->SetParams({p});
        b.Append(f4->Block(), [&] {
            auto* access = b.Access(ty.ptr<function>(ty.array(Inner, 4)), p, 0_u);
            b.Return(f4, b.Call(f3, access));
        });
    }

    auto* fn_b = b.Function("b", ty.void_());
    b.Append(fn_b->Block(), [&] {
        auto F = b.Var("F", ty.ptr<function>(Outer));
        b.Call(f4, F);
        b.Return(fn_b);
    });

    auto* src = R"(
Inner = struct @align(16) {
  mat:mat3x4<f32> @offset(0)
}

Outer = struct @align(16) {
  arr:array<Inner, 4> @offset(0)
  mat:mat3x4<f32> @offset(192)
}

%f0 = func(%p:ptr<function, vec4<f32>, read_write>):f32 {
  $B1: {
    %3:f32 = load_vector_element %p, 0u
    ret %3
  }
}
%f1 = func(%p_1:ptr<function, mat3x4<f32>, read_write>):f32 {  # %p_1: 'p'
  $B2: {
    %res:ptr<function, f32, read_write> = var undef
    %7:ptr<function, vec4<f32>, read_write> = access %p_1, 1i
    %8:f32 = call %f0, %7
    %9:f32 = load %res
    %10:f32 = add %9, %8
    store %res, %10
    %p_vec:ptr<function, vec4<f32>, read_write> = access %p_1, 1i
    %12:f32 = call %f0, %p_vec
    %13:f32 = load %res
    %14:f32 = add %13, %12
    store %res, %14
    %15:f32 = load %res
    ret %15
  }
}
%f2 = func(%p_2:ptr<function, Inner, read_write>):f32 {  # %p_2: 'p'
  $B3: {
    %p_mat:ptr<function, mat3x4<f32>, read_write> = access %p_2, 0u
    %19:f32 = call %f1, %p_mat
    ret %19
  }
}
%f3 = func(%p_3:ptr<function, array<Inner, 4>, read_write>):f32 {  # %p_3: 'p'
  $B4: {
    %p_inner:ptr<function, Inner, read_write> = access %p_3, 3i
    %23:f32 = call %f2, %p_inner
    ret %23
  }
}
%f4 = func(%p_4:ptr<function, Outer, read_write>):f32 {  # %p_4: 'p'
  $B5: {
    %26:ptr<function, array<Inner, 4>, read_write> = access %p_4, 0u
    %27:f32 = call %f3, %26
    ret %27
  }
}
%b = func():void {
  $B6: {
    %F:ptr<function, Outer, read_write> = var undef
    %30:f32 = call %f4, %F
    ret
  }
}
)";

    EXPECT_EQ(src, str());

    auto* expect = src;

    Run(DirectVariableAccess, DirectVariableAccessOptions{});

    EXPECT_EQ(expect, str());
}

TEST_F(IR_DirectVariableAccessTest_FunctionAS, Enabled_CallChaining2) {
    auto* T3 = ty.vec4<i32>();
    auto* T2 = ty.array(T3, 5);
    auto* T1 = ty.array(T2, 5);
    auto* T = ty.array(T1, 5);

    auto* f2 = b.Function("f2", T3);
    {
        auto* p = b.FunctionParam("p", ty.ptr<function>(T2));
        f2->SetParams({p});
        b.Append(f2->Block(), [&] {
            b.Return(f2, b.Load(b.Access<ptr<function, vec4<i32>, read_write>>(p, 3_u)));
        });
    }

    auto* f1 = b.Function("f1", T3);
    {
        auto* p = b.FunctionParam("p", ty.ptr<function>(T1));
        f1->SetParams({p});
        b.Append(f1->Block(),
                 [&] { b.Return(f1, b.Call(f2, b.Access(ty.ptr<function>(T2), p, 2_u))); });
    }

    auto* f0 = b.Function("f0", T3);
    {
        auto* p = b.FunctionParam("p", ty.ptr<function>(T));
        f0->SetParams({p});
        b.Append(f0->Block(),
                 [&] { b.Return(f0, b.Call(f1, b.Access(ty.ptr<function>(T1), p, 1_u))); });
    }

    auto* main = b.Function("main", ty.void_());
    b.Append(main->Block(), [&] {
        auto* F = b.Var("F", ty.ptr<function>(T));
        b.Call(f0, F);
        b.Return(main);
    });

    auto* src = R"(
%f2 = func(%p:ptr<function, array<vec4<i32>, 5>, read_write>):vec4<i32> {
  $B1: {
    %3:ptr<function, vec4<i32>, read_write> = access %p, 3u
    %4:vec4<i32> = load %3
    ret %4
  }
}
%f1 = func(%p_1:ptr<function, array<array<vec4<i32>, 5>, 5>, read_write>):vec4<i32> {  # %p_1: 'p'
  $B2: {
    %7:ptr<function, array<vec4<i32>, 5>, read_write> = access %p_1, 2u
    %8:vec4<i32> = call %f2, %7
    ret %8
  }
}
%f0 = func(%p_2:ptr<function, array<array<array<vec4<i32>, 5>, 5>, 5>, read_write>):vec4<i32> {  # %p_2: 'p'
  $B3: {
    %11:ptr<function, array<array<vec4<i32>, 5>, 5>, read_write> = access %p_2, 1u
    %12:vec4<i32> = call %f1, %11
    ret %12
  }
}
%main = func():void {
  $B4: {
    %F:ptr<function, array<array<array<vec4<i32>, 5>, 5>, 5>, read_write> = var undef
    %15:vec4<i32> = call %f0, %F
    ret
  }
}
)";

    EXPECT_EQ(src, str());

    auto* expect = R"(
%f2 = func(%p_root:ptr<function, array<array<array<vec4<i32>, 5>, 5>, 5>, read_write>, %p_indices:array<u32, 2>):vec4<i32> {
  $B1: {
    %4:u32 = access %p_indices, 0u
    %5:u32 = access %p_indices, 1u
    %6:ptr<function, array<vec4<i32>, 5>, read_write> = access %p_root, %4, %5
    %7:ptr<function, vec4<i32>, read_write> = access %6, 3u
    %8:vec4<i32> = load %7
    ret %8
  }
}
%f1 = func(%p_root_1:ptr<function, array<array<array<vec4<i32>, 5>, 5>, 5>, read_write>, %p_indices_1:array<u32, 1>):vec4<i32> {  # %p_root_1: 'p_root', %p_indices_1: 'p_indices'
  $B2: {
    %12:u32 = access %p_indices_1, 0u
    %13:array<u32, 2> = construct %12, 2u
    %14:vec4<i32> = call %f2, %p_root_1, %13
    ret %14
  }
}
%f0 = func(%p_root_2:ptr<function, array<array<array<vec4<i32>, 5>, 5>, 5>, read_write>):vec4<i32> {  # %p_root_2: 'p_root'
  $B3: {
    %17:array<u32, 1> = construct 1u
    %18:vec4<i32> = call %f1, %p_root_2, %17
    ret %18
  }
}
%main = func():void {
  $B4: {
    %F:ptr<function, array<array<array<vec4<i32>, 5>, 5>, 5>, read_write> = var undef
    %21:vec4<i32> = call %f0, %F
    ret
  }
}
)";

    Run(DirectVariableAccess, kTransformFunction);

    EXPECT_EQ(expect, str());
}

TEST_F(IR_DirectVariableAccessTest_FunctionAS, Disabled_CallChaining2) {
    auto* T3 = ty.vec4<i32>();
    auto* T2 = ty.array(T3, 5);
    auto* T1 = ty.array(T2, 5);
    auto* T = ty.array(T1, 5);

    auto* f2 = b.Function("f2", T3);
    {
        auto* p = b.FunctionParam("p", ty.ptr<function>(T2));
        f2->SetParams({p});
        b.Append(f2->Block(), [&] {
            b.Return(f2, b.Load(b.Access<ptr<function, vec4<i32>, read_write>>(p, 3_u)));
        });
    }

    auto* f1 = b.Function("f1", T3);
    {
        auto* p = b.FunctionParam("p", ty.ptr<function>(T1));
        f1->SetParams({p});
        b.Append(f1->Block(),
                 [&] { b.Return(f1, b.Call(f2, b.Access(ty.ptr<function>(T2), p, 2_u))); });
    }

    auto* f0 = b.Function("f0", T3);
    {
        auto* p = b.FunctionParam("p", ty.ptr<function>(T));
        f0->SetParams({p});
        b.Append(f0->Block(),
                 [&] { b.Return(f0, b.Call(f1, b.Access(ty.ptr<function>(T1), p, 1_u))); });
    }

    auto* main = b.Function("main", ty.void_());
    b.Append(main->Block(), [&] {
        auto* F = b.Var("F", ty.ptr<function>(T));
        b.Call(f0, F);
        b.Return(main);
    });

    auto* src = R"(
%f2 = func(%p:ptr<function, array<vec4<i32>, 5>, read_write>):vec4<i32> {
  $B1: {
    %3:ptr<function, vec4<i32>, read_write> = access %p, 3u
    %4:vec4<i32> = load %3
    ret %4
  }
}
%f1 = func(%p_1:ptr<function, array<array<vec4<i32>, 5>, 5>, read_write>):vec4<i32> {  # %p_1: 'p'
  $B2: {
    %7:ptr<function, array<vec4<i32>, 5>, read_write> = access %p_1, 2u
    %8:vec4<i32> = call %f2, %7
    ret %8
  }
}
%f0 = func(%p_2:ptr<function, array<array<array<vec4<i32>, 5>, 5>, 5>, read_write>):vec4<i32> {  # %p_2: 'p'
  $B3: {
    %11:ptr<function, array<array<vec4<i32>, 5>, 5>, read_write> = access %p_2, 1u
    %12:vec4<i32> = call %f1, %11
    ret %12
  }
}
%main = func():void {
  $B4: {
    %F:ptr<function, array<array<array<vec4<i32>, 5>, 5>, 5>, read_write> = var undef
    %15:vec4<i32> = call %f0, %F
    ret
  }
}
)";

    EXPECT_EQ(src, str());

    auto* expect = src;

    Run(DirectVariableAccess, DirectVariableAccessOptions{});

    EXPECT_EQ(expect, str());
}

}  // namespace function_as_tests

////////////////////////////////////////////////////////////////////////////////
// 'handle' address space
////////////////////////////////////////////////////////////////////////////////
namespace handle_as_tests {

using IR_DirectVariableAccessTest_HandleAS = TransformTest;

TEST_F(IR_DirectVariableAccessTest_HandleAS, Enabled_LocalTextureSampler) {
    auto* tex =
        b.Var("tex", handle, ty.sampled_texture(core::type::TextureDimension::k2d, ty.f32()),
              core::Access::kRead);
    tex->SetBindingPoint(0, 0);
    b.ir.root_block->Append(tex);

    auto* samp = b.Var("samp", handle, ty.sampler(), core::Access::kRead);
    samp->SetBindingPoint(0, 1);
    b.ir.root_block->Append(samp);

    auto* fn = b.Function("f", ty.void_());
    b.Append(fn->Block(), [&] {
        auto* t = b.Load(tex);
        auto* s = b.Load(samp);

        b.Let("p", b.Call(ty.vec4<f32>(), core::BuiltinFn::kTextureGather, 0_u, t, s,
                          b.Splat(ty.vec2<f32>(), 0_f)));
        b.Return(fn);
    });

    auto* src = R"(
$B1: {  # root
  %tex:ptr<handle, texture_2d<f32>, read> = var undef @binding_point(0, 0)
  %samp:ptr<handle, sampler, read> = var undef @binding_point(0, 1)
}

%f = func():void {
  $B2: {
    %4:texture_2d<f32> = load %tex
    %5:sampler = load %samp
    %6:vec4<f32> = textureGather 0u, %4, %5, vec2<f32>(0.0f)
    %p:vec4<f32> = let %6
    ret
  }
}
)";

    EXPECT_EQ(src, str());

    auto* expect = src;  // Nothing changes

    Run(DirectVariableAccess, kTransformHandle);

    EXPECT_EQ(expect, str());
}

TEST_F(IR_DirectVariableAccessTest_HandleAS, Enabled_LocalTextureParamSampler) {
    auto* tex =
        b.Var("tex", handle, ty.sampled_texture(core::type::TextureDimension::k2d, ty.f32()),
              core::Access::kRead);
    tex->SetBindingPoint(0, 0);
    b.ir.root_block->Append(tex);

    auto* samp = b.Var("samp", handle, ty.sampler(), core::Access::kRead);
    samp->SetBindingPoint(0, 1);
    b.ir.root_block->Append(samp);

    auto* s = b.FunctionParam("s", ty.sampler());

    auto* fn = b.Function("f", ty.void_());
    fn->SetParams({s});
    b.Append(fn->Block(), [&] {
        auto* t = b.Load(tex);

        b.Let("p", b.Call(ty.vec4<f32>(), core::BuiltinFn::kTextureGather, 0_u, t, s,
                          b.Splat(ty.vec2<f32>(), 0_f)));
        b.Return(fn);
    });

    auto* fn2 = b.Function("g", ty.void_());
    b.Append(fn2->Block(), [&] {
        auto* s2 = b.Load(samp);
        b.Call(ty.void_(), fn, s2);
        b.Return(fn2);
    });

    auto* src = R"(
$B1: {  # root
  %tex:ptr<handle, texture_2d<f32>, read> = var undef @binding_point(0, 0)
  %samp:ptr<handle, sampler, read> = var undef @binding_point(0, 1)
}

%f = func(%s:sampler):void {
  $B2: {
    %5:texture_2d<f32> = load %tex
    %6:vec4<f32> = textureGather 0u, %5, %s, vec2<f32>(0.0f)
    %p:vec4<f32> = let %6
    ret
  }
}
%g = func():void {
  $B3: {
    %9:sampler = load %samp
    %10:void = call %f, %9
    ret
  }
}
)";

    EXPECT_EQ(src, str());

    auto* expect = R"(
$B1: {  # root
  %tex:ptr<handle, texture_2d<f32>, read> = var undef @binding_point(0, 0)
  %samp:ptr<handle, sampler, read> = var undef @binding_point(0, 1)
}

%f = func():void {
  $B2: {
    %4:sampler = load %samp
    %5:texture_2d<f32> = load %tex
    %6:vec4<f32> = textureGather 0u, %5, %4, vec2<f32>(0.0f)
    %p:vec4<f32> = let %6
    ret
  }
}
%g = func():void {
  $B3: {
    %9:void = call %f
    ret
  }
}
)";

    Run(DirectVariableAccess, kTransformHandle);

    EXPECT_EQ(expect, str());
}

TEST_F(IR_DirectVariableAccessTest_HandleAS, Enabled_LocalTextureParamTextureLoad) {
    auto* tex =
        b.Var("tex", handle, ty.sampled_texture(core::type::TextureDimension::k2d, ty.f32()),
              core::Access::kRead);
    tex->SetBindingPoint(0, 0);
    b.ir.root_block->Append(tex);

    auto* t = b.FunctionParam("texparam",
                              ty.sampled_texture(core::type::TextureDimension::k2d, ty.f32()));

    auto* fn = b.Function("f", ty.void_());
    fn->SetParams({t});
    b.Append(fn->Block(), [&] {
        b.Let("p", b.Call(ty.vec4<f32>(), core::BuiltinFn::kTextureLoad, t,
                          b.Splat(ty.vec2<u32>(), 0_u), 0_u));
        b.Return(fn);
    });

    auto* fn2 = b.Function("g", ty.void_());
    b.Append(fn2->Block(), [&] {
        auto* t2 = b.Load(tex);
        b.Call(ty.void_(), fn, t2);
        b.Return(fn2);
    });

    auto* src = R"(
$B1: {  # root
  %tex:ptr<handle, texture_2d<f32>, read> = var undef @binding_point(0, 0)
}

%f = func(%texparam:texture_2d<f32>):void {
  $B2: {
    %4:vec4<f32> = textureLoad %texparam, vec2<u32>(0u), 0u
    %p:vec4<f32> = let %4
    ret
  }
}
%g = func():void {
  $B3: {
    %7:texture_2d<f32> = load %tex
    %8:void = call %f, %7
    ret
  }
}
)";

    EXPECT_EQ(src, str());

    auto* expect = R"(
$B1: {  # root
  %tex:ptr<handle, texture_2d<f32>, read> = var undef @binding_point(0, 0)
}

%f = func():void {
  $B2: {
    %3:texture_2d<f32> = load %tex
    %4:vec4<f32> = textureLoad %3, vec2<u32>(0u), 0u
    %p:vec4<f32> = let %4
    ret
  }
}
%g = func():void {
  $B3: {
    %7:void = call %f
    ret
  }
}
)";

    Run(DirectVariableAccess, kTransformHandle);

    EXPECT_EQ(expect, str());
}

TEST_F(IR_DirectVariableAccessTest_HandleAS, Enabled_ParamTextureLocalSampler) {
    auto* tex_ty = ty.sampled_texture(core::type::TextureDimension::k2d, ty.f32());
    auto* tex = b.Var("tex", handle, tex_ty, core::Access::kRead);
    tex->SetBindingPoint(0, 0);
    b.ir.root_block->Append(tex);

    auto* samp = b.Var("samp", handle, ty.sampler(), core::Access::kRead);
    samp->SetBindingPoint(0, 1);
    b.ir.root_block->Append(samp);

    auto* t = b.FunctionParam("t", tex_ty);

    auto* fn = b.Function("f", ty.void_());
    fn->SetParams({t});
    b.Append(fn->Block(), [&] {
        auto* s = b.Load(samp);

        b.Let("p", b.Call(ty.vec4<f32>(), core::BuiltinFn::kTextureGather, 0_u, t, s,
                          b.Splat(ty.vec2<f32>(), 0_f)));
        b.Return(fn);
    });

    auto* fn2 = b.Function("g", ty.void_());
    b.Append(fn2->Block(), [&] {
        auto* t2 = b.Load(tex);
        b.Call(ty.void_(), fn, t2);
        b.Return(fn2);
    });

    auto* src = R"(
$B1: {  # root
  %tex:ptr<handle, texture_2d<f32>, read> = var undef @binding_point(0, 0)
  %samp:ptr<handle, sampler, read> = var undef @binding_point(0, 1)
}

%f = func(%t:texture_2d<f32>):void {
  $B2: {
    %5:sampler = load %samp
    %6:vec4<f32> = textureGather 0u, %t, %5, vec2<f32>(0.0f)
    %p:vec4<f32> = let %6
    ret
  }
}
%g = func():void {
  $B3: {
    %9:texture_2d<f32> = load %tex
    %10:void = call %f, %9
    ret
  }
}
)";

    EXPECT_EQ(src, str());

    auto* expect = R"(
$B1: {  # root
  %tex:ptr<handle, texture_2d<f32>, read> = var undef @binding_point(0, 0)
  %samp:ptr<handle, sampler, read> = var undef @binding_point(0, 1)
}

%f = func():void {
  $B2: {
    %4:texture_2d<f32> = load %tex
    %5:sampler = load %samp
    %6:vec4<f32> = textureGather 0u, %4, %5, vec2<f32>(0.0f)
    %p:vec4<f32> = let %6
    ret
  }
}
%g = func():void {
  $B3: {
    %9:void = call %f
    ret
  }
}
)";

    Run(DirectVariableAccess, kTransformHandle);

    EXPECT_EQ(expect, str());
}

TEST_F(IR_DirectVariableAccessTest_HandleAS, Enabled_ParamTextureParamSampler) {
    auto* tex_ty = ty.sampled_texture(core::type::TextureDimension::k2d, ty.f32());
    auto* tex = b.Var("tex", handle, tex_ty, core::Access::kRead);
    tex->SetBindingPoint(0, 0);
    b.ir.root_block->Append(tex);

    auto* samp = b.Var("samp", handle, ty.sampler(), core::Access::kRead);
    samp->SetBindingPoint(0, 1);
    b.ir.root_block->Append(samp);

    auto* t = b.FunctionParam("t", tex_ty);
    auto* s = b.FunctionParam("s", ty.sampler());

    auto* fn = b.Function("f", ty.void_());
    fn->SetParams({t, s});
    b.Append(fn->Block(), [&] {
        b.Let("p", b.Call(ty.vec4<f32>(), core::BuiltinFn::kTextureGather, 0_u, t, s,
                          b.Splat(ty.vec2<f32>(), 0_f)));
        b.Return(fn);
    });

    auto* fn2 = b.Function("g", ty.void_());
    b.Append(fn2->Block(), [&] {
        auto* s2 = b.Load(samp);
        auto* t2 = b.Load(tex);
        b.Call(ty.void_(), fn, t2, s2);
        b.Return(fn2);
    });

    auto* src = R"(
$B1: {  # root
  %tex:ptr<handle, texture_2d<f32>, read> = var undef @binding_point(0, 0)
  %samp:ptr<handle, sampler, read> = var undef @binding_point(0, 1)
}

%f = func(%t:texture_2d<f32>, %s:sampler):void {
  $B2: {
    %6:vec4<f32> = textureGather 0u, %t, %s, vec2<f32>(0.0f)
    %p:vec4<f32> = let %6
    ret
  }
}
%g = func():void {
  $B3: {
    %9:sampler = load %samp
    %10:texture_2d<f32> = load %tex
    %11:void = call %f, %10, %9
    ret
  }
}
)";

    EXPECT_EQ(src, str());

    auto* expect = R"(
$B1: {  # root
  %tex:ptr<handle, texture_2d<f32>, read> = var undef @binding_point(0, 0)
  %samp:ptr<handle, sampler, read> = var undef @binding_point(0, 1)
}

%f = func():void {
  $B2: {
    %4:texture_2d<f32> = load %tex
    %5:sampler = load %samp
    %6:vec4<f32> = textureGather 0u, %4, %5, vec2<f32>(0.0f)
    %p:vec4<f32> = let %6
    ret
  }
}
%g = func():void {
  $B3: {
    %9:void = call %f
    ret
  }
}
)";

    Run(DirectVariableAccess, kTransformHandle);

    EXPECT_EQ(expect, str());
}

TEST_F(IR_DirectVariableAccessTest_HandleAS, Enabled_MultiFunction) {
    auto* tex_ty = ty.sampled_texture(core::type::TextureDimension::k2d, ty.f32());
    auto* tex = b.Var("tex", handle, tex_ty, core::Access::kRead);
    tex->SetBindingPoint(0, 0);
    b.ir.root_block->Append(tex);

    auto* samp = b.Var("samp", handle, ty.sampler(), core::Access::kRead);
    samp->SetBindingPoint(0, 1);
    b.ir.root_block->Append(samp);

    auto* t = b.FunctionParam("t", tex_ty);
    auto* s = b.FunctionParam("s", ty.sampler());

    auto* fn = b.Function("f", ty.void_());
    fn->SetParams({t, s});
    b.Append(fn->Block(), [&] {
        b.Let("p", b.Call(ty.vec4<f32>(), core::BuiltinFn::kTextureGather, 0_u, t, s,
                          b.Splat(ty.vec2<f32>(), 0_f)));
        b.Return(fn);
    });

    auto* t2 = b.FunctionParam("t", tex_ty);
    auto* fn2 = b.Function("g", ty.void_());
    fn2->SetParams({t2});
    b.Append(fn2->Block(), [&] {
        auto* s2 = b.Load(samp);
        b.Call(ty.void_(), fn, t2, s2);
        b.Return(fn2);
    });

    auto* fn3 = b.Function("h", ty.void_());
    b.Append(fn3->Block(), [&] {
        auto* t3 = b.Load(tex);
        b.Call(ty.void_(), fn2, t3);
        b.Return(fn3);
    });

    auto* src = R"(
$B1: {  # root
  %tex:ptr<handle, texture_2d<f32>, read> = var undef @binding_point(0, 0)
  %samp:ptr<handle, sampler, read> = var undef @binding_point(0, 1)
}

%f = func(%t:texture_2d<f32>, %s:sampler):void {
  $B2: {
    %6:vec4<f32> = textureGather 0u, %t, %s, vec2<f32>(0.0f)
    %p:vec4<f32> = let %6
    ret
  }
}
%g = func(%t_1:texture_2d<f32>):void {  # %t_1: 't'
  $B3: {
    %10:sampler = load %samp
    %11:void = call %f, %t_1, %10
    ret
  }
}
%h = func():void {
  $B4: {
    %13:texture_2d<f32> = load %tex
    %14:void = call %g, %13
    ret
  }
}
)";

    EXPECT_EQ(src, str());

    auto* expect = R"(
$B1: {  # root
  %tex:ptr<handle, texture_2d<f32>, read> = var undef @binding_point(0, 0)
  %samp:ptr<handle, sampler, read> = var undef @binding_point(0, 1)
}

%f = func():void {
  $B2: {
    %4:texture_2d<f32> = load %tex
    %5:sampler = load %samp
    %6:vec4<f32> = textureGather 0u, %4, %5, vec2<f32>(0.0f)
    %p:vec4<f32> = let %6
    ret
  }
}
%g = func():void {
  $B3: {
    %9:void = call %f
    ret
  }
}
%h = func():void {
  $B4: {
    %11:void = call %g
    ret
  }
}
)";

    Run(DirectVariableAccess, kTransformHandle);

    EXPECT_EQ(expect, str());
}

TEST_F(IR_DirectVariableAccessTest_HandleAS, Disabled_MultiFunction) {
    auto* tex_ty = ty.sampled_texture(core::type::TextureDimension::k2d, ty.f32());
    auto* tex = b.Var("tex", handle, tex_ty, core::Access::kRead);
    tex->SetBindingPoint(0, 0);
    b.ir.root_block->Append(tex);

    auto* samp = b.Var("samp", handle, ty.sampler(), core::Access::kRead);
    samp->SetBindingPoint(0, 1);
    b.ir.root_block->Append(samp);

    auto* t = b.FunctionParam("t", tex_ty);
    auto* s = b.FunctionParam("s", ty.sampler());

    auto* fn = b.Function("f", ty.void_());
    fn->SetParams({t, s});
    b.Append(fn->Block(), [&] {
        b.Let("p", b.Call(ty.vec4<f32>(), core::BuiltinFn::kTextureGather, 0_u, t, s,
                          b.Splat(ty.vec2<f32>(), 0_f)));
        b.Return(fn);
    });

    auto* t2 = b.FunctionParam("t", tex_ty);
    auto* fn2 = b.Function("g", ty.void_());
    fn2->SetParams({t2});
    b.Append(fn2->Block(), [&] {
        auto* s2 = b.Load(samp);
        b.Call(ty.void_(), fn, t2, s2);
        b.Return(fn2);
    });

    auto* fn3 = b.Function("h", ty.void_());
    b.Append(fn3->Block(), [&] {
        auto* t3 = b.Load(tex);
        b.Call(ty.void_(), fn2, t3);
        b.Return(fn3);
    });

    auto* src = R"(
$B1: {  # root
  %tex:ptr<handle, texture_2d<f32>, read> = var undef @binding_point(0, 0)
  %samp:ptr<handle, sampler, read> = var undef @binding_point(0, 1)
}

%f = func(%t:texture_2d<f32>, %s:sampler):void {
  $B2: {
    %6:vec4<f32> = textureGather 0u, %t, %s, vec2<f32>(0.0f)
    %p:vec4<f32> = let %6
    ret
  }
}
%g = func(%t_1:texture_2d<f32>):void {  # %t_1: 't'
  $B3: {
    %10:sampler = load %samp
    %11:void = call %f, %t_1, %10
    ret
  }
}
%h = func():void {
  $B4: {
    %13:texture_2d<f32> = load %tex
    %14:void = call %g, %13
    ret
  }
}
)";

    EXPECT_EQ(src, str());

    auto* expect = src;  // No change

    Run(DirectVariableAccess, DirectVariableAccessOptions{});

    EXPECT_EQ(expect, str());
}

TEST_F(IR_DirectVariableAccessTest_HandleAS, Enabled_DuplicateParam) {
    auto* tex_ty = ty.sampled_texture(core::type::TextureDimension::k2d, ty.f32());
    auto* tex = b.Var("tex", handle, tex_ty, core::Access::kRead);
    tex->SetBindingPoint(0, 0);
    b.ir.root_block->Append(tex);

    auto* samp = b.Var("samp", handle, ty.sampler(), core::Access::kRead);
    samp->SetBindingPoint(0, 1);
    b.ir.root_block->Append(samp);

    auto* t1 = b.FunctionParam("t1", tex_ty);
    auto* t2 = b.FunctionParam("t2", tex_ty);

    auto* fn = b.Function("f", ty.void_());
    fn->SetParams({t1, t2});
    b.Append(fn->Block(), [&] {
        auto* s = b.Load(samp);

        b.Let("p1", b.Call(ty.vec4<f32>(), core::BuiltinFn::kTextureGather, 0_u, t1, s,
                           b.Splat(ty.vec2<f32>(), 0_f)));
        b.Let("p2", b.Call(ty.vec4<f32>(), core::BuiltinFn::kTextureGather, 0_u, t2, s,
                           b.Splat(ty.vec2<f32>(), 0_f)));
        b.Return(fn);
    });

    auto* fn2 = b.Function("g", ty.void_());
    b.Append(fn2->Block(), [&] {
        auto* t3 = b.Load(tex);
        auto* t4 = b.Load(tex);
        b.Call(ty.void_(), fn, t3, t4);
        b.Return(fn2);
    });

    auto* src = R"(
$B1: {  # root
  %tex:ptr<handle, texture_2d<f32>, read> = var undef @binding_point(0, 0)
  %samp:ptr<handle, sampler, read> = var undef @binding_point(0, 1)
}

%f = func(%t1:texture_2d<f32>, %t2:texture_2d<f32>):void {
  $B2: {
    %6:sampler = load %samp
    %7:vec4<f32> = textureGather 0u, %t1, %6, vec2<f32>(0.0f)
    %p1:vec4<f32> = let %7
    %9:vec4<f32> = textureGather 0u, %t2, %6, vec2<f32>(0.0f)
    %p2:vec4<f32> = let %9
    ret
  }
}
%g = func():void {
  $B3: {
    %12:texture_2d<f32> = load %tex
    %13:texture_2d<f32> = load %tex
    %14:void = call %f, %12, %13
    ret
  }
}
)";

    EXPECT_EQ(src, str());

    auto* expect = R"(
$B1: {  # root
  %tex:ptr<handle, texture_2d<f32>, read> = var undef @binding_point(0, 0)
  %samp:ptr<handle, sampler, read> = var undef @binding_point(0, 1)
}

%f = func():void {
  $B2: {
    %4:texture_2d<f32> = load %tex
    %5:texture_2d<f32> = load %tex
    %6:sampler = load %samp
    %7:vec4<f32> = textureGather 0u, %4, %6, vec2<f32>(0.0f)
    %p1:vec4<f32> = let %7
    %9:vec4<f32> = textureGather 0u, %5, %6, vec2<f32>(0.0f)
    %p2:vec4<f32> = let %9
    ret
  }
}
%g = func():void {
  $B3: {
    %12:void = call %f
    ret
  }
}
)";

    Run(DirectVariableAccess, kTransformHandle);

    EXPECT_EQ(expect, str());
}

TEST_F(IR_DirectVariableAccessTest_HandleAS, Enabled_Fork) {
    auto* tex_ty = ty.sampled_texture(core::type::TextureDimension::k2d, ty.f32());
    auto* tex1 = b.Var("tex1", handle, tex_ty, core::Access::kRead);
    tex1->SetBindingPoint(0, 0);
    b.ir.root_block->Append(tex1);
    auto* tex2 = b.Var("tex2", handle, tex_ty, core::Access::kRead);
    tex2->SetBindingPoint(0, 1);
    b.ir.root_block->Append(tex2);

    auto* samp = b.Var("samp", handle, ty.sampler(), core::Access::kRead);
    samp->SetBindingPoint(0, 2);
    b.ir.root_block->Append(samp);

    auto* t = b.FunctionParam("t", tex_ty);

    auto* fn = b.Function("f", ty.void_());
    fn->SetParams({t});
    b.Append(fn->Block(), [&] {
        auto* s = b.Load(samp);
        b.Let("p", b.Call(ty.vec4<f32>(), core::BuiltinFn::kTextureGather, 0_u, t, s,
                          b.Splat(ty.vec2<f32>(), 0_f)));
        b.Return(fn);
    });

    auto* fn2 = b.Function("g", ty.void_());
    b.Append(fn2->Block(), [&] {
        auto* t2 = b.Load(tex1);
        b.Call(ty.void_(), fn, t2);
        b.Return(fn2);
    });

    auto* fn3 = b.Function("h", ty.void_());
    b.Append(fn3->Block(), [&] {
        auto* t2 = b.Load(tex2);
        b.Call(ty.void_(), fn, t2);
        b.Return(fn3);
    });

    auto* src = R"(
$B1: {  # root
  %tex1:ptr<handle, texture_2d<f32>, read> = var undef @binding_point(0, 0)
  %tex2:ptr<handle, texture_2d<f32>, read> = var undef @binding_point(0, 1)
  %samp:ptr<handle, sampler, read> = var undef @binding_point(0, 2)
}

%f = func(%t:texture_2d<f32>):void {
  $B2: {
    %6:sampler = load %samp
    %7:vec4<f32> = textureGather 0u, %t, %6, vec2<f32>(0.0f)
    %p:vec4<f32> = let %7
    ret
  }
}
%g = func():void {
  $B3: {
    %10:texture_2d<f32> = load %tex1
    %11:void = call %f, %10
    ret
  }
}
%h = func():void {
  $B4: {
    %13:texture_2d<f32> = load %tex2
    %14:void = call %f, %13
    ret
  }
}
)";

    EXPECT_EQ(src, str());

    auto* expect = R"(
$B1: {  # root
  %tex1:ptr<handle, texture_2d<f32>, read> = var undef @binding_point(0, 0)
  %tex2:ptr<handle, texture_2d<f32>, read> = var undef @binding_point(0, 1)
  %samp:ptr<handle, sampler, read> = var undef @binding_point(0, 2)
}

%f = func():void {
  $B2: {
    %5:texture_2d<f32> = load %tex1
    %6:sampler = load %samp
    %7:vec4<f32> = textureGather 0u, %5, %6, vec2<f32>(0.0f)
    %p:vec4<f32> = let %7
    ret
  }
}
%f_1 = func():void {  # %f_1: 'f'
  $B3: {
    %10:texture_2d<f32> = load %tex2
    %11:sampler = load %samp
    %12:vec4<f32> = textureGather 0u, %10, %11, vec2<f32>(0.0f)
    %p_1:vec4<f32> = let %12  # %p_1: 'p'
    ret
  }
}
%g = func():void {
  $B4: {
    %15:void = call %f
    ret
  }
}
%h = func():void {
  $B5: {
    %17:void = call %f_1
    ret
  }
}
)";

    Run(DirectVariableAccess, kTransformHandle);

    EXPECT_EQ(expect, str());
}

TEST_F(IR_DirectVariableAccessTest_HandleAS, Enabled_TextureBindingArrayParam) {
    auto* texture_type = ty.sampled_texture(core::type::TextureDimension::k2d, ty.f32());
    auto* var_ts = b.Var("ts", ty.ptr<handle>(ty.binding_array(texture_type, 3u)));
    var_ts->SetBindingPoint(0, 0);
    b.ir.root_block->Append(var_ts);

    auto* fn = b.Function("f", ty.void_());
    auto* p = b.FunctionParam("p", ty.binding_array(texture_type, 3u));
    fn->SetParams({p});

    b.Append(fn->Block(), [&] {
        auto* t = b.Access(texture_type, p, 0_i);
        b.Call(ty.vec4<f32>(), core::BuiltinFn::kTextureLoad, t, b.Splat(ty.vec2<u32>(), 0_u), 0_u);
        b.Return(fn);
    });

    auto* main = b.Function("main", ty.void_(), core::ir::Function::PipelineStage::kFragment);
    b.Append(main->Block(), [&] {
        auto* ts = b.Load(var_ts);
        b.Call(fn, ts);
        b.Return(main);
    });

    auto* src = R"(
$B1: {  # root
  %ts:ptr<handle, binding_array<texture_2d<f32>, 3>, read> = var undef @binding_point(0, 0)
}

%f = func(%p:binding_array<texture_2d<f32>, 3>):void {
  $B2: {
    %4:texture_2d<f32> = access %p, 0i
    %5:vec4<f32> = textureLoad %4, vec2<u32>(0u), 0u
    ret
  }
}
%main = @fragment func():void {
  $B3: {
    %7:binding_array<texture_2d<f32>, 3> = load %ts
    %8:void = call %f, %7
    ret
  }
}
)";

    EXPECT_EQ(src, str());

    auto* expect = R"(
$B1: {  # root
  %ts:ptr<handle, binding_array<texture_2d<f32>, 3>, read> = var undef @binding_point(0, 0)
}

%f = func():void {
  $B2: {
    %3:binding_array<texture_2d<f32>, 3> = load %ts
    %4:texture_2d<f32> = access %3, 0i
    %5:vec4<f32> = textureLoad %4, vec2<u32>(0u), 0u
    ret
  }
}
%main = @fragment func():void {
  $B3: {
    %7:void = call %f
    ret
  }
}
)";

    Run(DirectVariableAccess, kTransformHandle);

    EXPECT_EQ(expect, str());
}

TEST_F(IR_DirectVariableAccessTest_HandleAS, Enabled_TextureFromBindingArrayParam) {
    auto* texture_type = ty.sampled_texture(core::type::TextureDimension::k2d, ty.f32());
    auto* var_ts = b.Var("ts", ty.ptr<handle>(ty.binding_array(texture_type, 3u)));
    var_ts->SetBindingPoint(0, 0);
    b.ir.root_block->Append(var_ts);

    auto* fn = b.Function("f", ty.void_());
    auto* p = b.FunctionParam("p", texture_type);
    fn->SetParams({p});

    b.Append(fn->Block(), [&] {
        b.Call(ty.vec4<f32>(), core::BuiltinFn::kTextureLoad, p, b.Splat(ty.vec2<u32>(), 0_u), 0_u);
        b.Return(fn);
    });

    auto* main = b.Function("main", ty.void_(), core::ir::Function::PipelineStage::kFragment);
    b.Append(main->Block(), [&] {
        auto* t_ptr = b.Access(ty.ptr<handle>(texture_type), var_ts, 0_i);
        auto* t = b.Load(t_ptr);

        b.Call(fn, t);
        b.Return(main);
    });

    auto* src = R"(
$B1: {  # root
  %ts:ptr<handle, binding_array<texture_2d<f32>, 3>, read> = var undef @binding_point(0, 0)
}

%f = func(%p:texture_2d<f32>):void {
  $B2: {
    %4:vec4<f32> = textureLoad %p, vec2<u32>(0u), 0u
    ret
  }
}
%main = @fragment func():void {
  $B3: {
    %6:ptr<handle, texture_2d<f32>, read> = access %ts, 0i
    %7:texture_2d<f32> = load %6
    %8:void = call %f, %7
    ret
  }
}
)";

    EXPECT_EQ(src, str());

    auto* expect = R"(
$B1: {  # root
  %ts:ptr<handle, binding_array<texture_2d<f32>, 3>, read> = var undef @binding_point(0, 0)
}

%f = func(%p_indices:array<u32, 1>):void {
  $B2: {
    %4:u32 = access %p_indices, 0u
    %5:ptr<handle, texture_2d<f32>, read> = access %ts, %4
    %6:texture_2d<f32> = load %5
    %7:vec4<f32> = textureLoad %6, vec2<u32>(0u), 0u
    ret
  }
}
%main = @fragment func():void {
  $B3: {
    %9:u32 = convert 0i
    %10:array<u32, 1> = construct %9
    %11:void = call %f, %10
    ret
  }
}
)";

    Run(DirectVariableAccess, kTransformHandle);

    EXPECT_EQ(expect, str());
}
}  // namespace handle_as_tests

////////////////////////////////////////////////////////////////////////////////
// builtin function calls
////////////////////////////////////////////////////////////////////////////////
namespace builtin_fn_calls {

using IR_DirectVariableAccessTest_BuiltinFn = TransformTest;

TEST_F(IR_DirectVariableAccessTest_BuiltinFn, ArrayLength) {
    Var* S = nullptr;
    b.Append(b.ir.root_block,
             [&] {  //
                 S = b.Var<storage, array<f32>>("S");
                 S->SetBindingPoint(0, 0);
             });

    auto* fn_len = b.Function("len", ty.u32());
    auto* fn_len_p = b.FunctionParam("p", ty.ptr<storage, array<f32>>());
    fn_len->SetParams({fn_len_p});
    b.Append(fn_len->Block(),
             [&] {  //
                 b.Return(fn_len, b.Call(ty.u32(), core::BuiltinFn::kArrayLength, fn_len_p));
             });

    auto* fn_f = b.Function("b", ty.void_());
    b.Append(fn_f->Block(), [&] {
        b.Call(fn_len, S);
        b.Return(fn_f);
    });

    auto* src = R"(
$B1: {  # root
  %S:ptr<storage, array<f32>, read_write> = var undef @binding_point(0, 0)
}

%len = func(%p:ptr<storage, array<f32>, read_write>):u32 {
  $B2: {
    %4:u32 = arrayLength %p
    ret %4
  }
}
%b = func():void {
  $B3: {
    %6:u32 = call %len, %S
    ret
  }
}
)";

    EXPECT_EQ(src, str());

    auto* expect = R"(
$B1: {  # root
  %S:ptr<storage, array<f32>, read_write> = var undef @binding_point(0, 0)
}

%len = func():u32 {
  $B2: {
    %3:u32 = arrayLength %S
    ret %3
  }
}
%b = func():void {
  $B3: {
    %5:u32 = call %len
    ret
  }
}
)";

    Run(DirectVariableAccess, DirectVariableAccessOptions{});

    EXPECT_EQ(expect, str());
}

TEST_F(IR_DirectVariableAccessTest_BuiltinFn, AtomicLoad) {
    Var* W = nullptr;
    b.Append(b.ir.root_block,
             [&] {  //
                 W = b.Var("W", ty.ptr<workgroup>(ty.atomic<i32>()));
             });

    auto* fn_load = b.Function("load", ty.i32());
    auto* fn_load_p = b.FunctionParam("p", ty.ptr<workgroup>(ty.atomic<i32>()));
    fn_load->SetParams({fn_load_p});
    b.Append(fn_load->Block(),
             [&] {  //
                 b.Return(fn_load, b.Call(ty.i32(), core::BuiltinFn::kAtomicLoad, fn_load_p));
             });

    auto* fn_f = b.Function("b", ty.void_());
    b.Append(fn_f->Block(), [&] {
        b.Call(fn_load, W);
        b.Return(fn_f);
    });

    auto* src = R"(
$B1: {  # root
  %W:ptr<workgroup, atomic<i32>, read_write> = var undef
}

%load = func(%p:ptr<workgroup, atomic<i32>, read_write>):i32 {
  $B2: {
    %4:i32 = atomicLoad %p
    ret %4
  }
}
%b = func():void {
  $B3: {
    %6:i32 = call %load, %W
    ret
  }
}
)";

    EXPECT_EQ(src, str());

    auto* expect = R"(
$B1: {  # root
  %W:ptr<workgroup, atomic<i32>, read_write> = var undef
}

%load = func():i32 {
  $B2: {
    %3:i32 = atomicLoad %W
    ret %3
  }
}
%b = func():void {
  $B3: {
    %5:i32 = call %load
    ret
  }
}
)";

    Run(DirectVariableAccess, DirectVariableAccessOptions{});

    EXPECT_EQ(expect, str());
}

}  // namespace builtin_fn_calls

////////////////////////////////////////////////////////////////////////////////
// complex tests
////////////////////////////////////////////////////////////////////////////////
namespace complex_tests {

using IR_DirectVariableAccessTest_Complex = TransformTest;

TEST_F(IR_DirectVariableAccessTest_Complex, Param_ptr_mixed_vec4i32_ViaMultiple) {
    auto* str_ = ty.Struct(mod.symbols.New("str"), {
                                                       {mod.symbols.Register("i"), ty.vec4<i32>()},
                                                   });

    Var* U = nullptr;
    Var* U_str = nullptr;
    Var* U_arr = nullptr;
    Var* U_arr_arr = nullptr;
    Var* S = nullptr;
    Var* S_str = nullptr;
    Var* S_arr = nullptr;
    Var* S_arr_arr = nullptr;
    Var* W = nullptr;
    Var* W_str = nullptr;
    Var* W_arr = nullptr;
    Var* W_arr_arr = nullptr;
    b.Append(b.ir.root_block,
             [&] {  //
                 U = b.Var<uniform, vec4<i32>>("U");
                 U->SetBindingPoint(0, 0);
                 U_str = b.Var("U_str", ty.ptr<uniform>(str_));
                 U_str->SetBindingPoint(0, 1);
                 U_arr = b.Var<uniform, array<vec4<i32>, 8>>("U_arr");
                 U_arr->SetBindingPoint(0, 2);
                 U_arr_arr = b.Var<uniform, array<array<vec4<i32>, 8>, 4>>("U_arr_arr");
                 U_arr_arr->SetBindingPoint(0, 3);

                 S = b.Var<storage, vec4<i32>, read>("S");
                 S->SetBindingPoint(1, 0);
                 S_str = b.Var("S_str", ty.ptr<storage, read>(str_));
                 S_str->SetBindingPoint(1, 1);
                 S_arr = b.Var<storage, array<vec4<i32>, 8>, read>("S_arr");
                 S_arr->SetBindingPoint(1, 2);
                 S_arr_arr = b.Var<storage, array<array<vec4<i32>, 8>, 4>, read>("S_arr_arr");
                 S_arr_arr->SetBindingPoint(1, 3);

                 W = b.Var<workgroup, vec4<i32>>("W");
                 W_str = b.Var("W_str", ty.ptr<workgroup>(str_));
                 W_arr = b.Var<workgroup, array<vec4<i32>, 8>>("W_arr");
                 W_arr_arr = b.Var<workgroup, array<array<vec4<i32>, 8>, 4>>("W_arr_arr");
             });

    auto* fn_u = b.Function("fn_u", ty.vec4<i32>());
    auto* fn_u_p = b.FunctionParam("p", ty.ptr<uniform, vec4<i32>, read>());
    fn_u->SetParams({fn_u_p});
    b.Append(fn_u->Block(), [&] { b.Return(fn_u, b.Load(fn_u_p)); });

    auto* fn_s = b.Function("fn_s", ty.vec4<i32>());
    auto* fn_s_p = b.FunctionParam("p", ty.ptr<storage, vec4<i32>, read>());
    fn_s->SetParams({fn_s_p});
    b.Append(fn_s->Block(), [&] { b.Return(fn_s, b.Load(fn_s_p)); });

    auto* fn_w = b.Function("fn_w", ty.vec4<i32>());
    auto* fn_w_p = b.FunctionParam("p", ty.ptr<workgroup, vec4<i32>>());
    fn_w->SetParams({fn_w_p});
    b.Append(fn_w->Block(), [&] { b.Return(fn_w, b.Load(fn_w_p)); });

    auto* fn_b = b.Function("b", ty.void_());
    b.Append(fn_b->Block(), [&] {
        auto* I = b.Let("I", 3_i);
        auto* J = b.Let("J", 4_i);

        auto* u = b.Call(fn_u, U);
        b.ir.SetName(u, "u");
        auto* u_str = b.Call(fn_u, b.Access(ty.ptr<uniform, vec4<i32>>(), U_str, 0_u));
        b.ir.SetName(u_str, "u_str");
        auto* u_arr0 = b.Call(fn_u, b.Access(ty.ptr<uniform, vec4<i32>>(), U_arr, 0_i));
        b.ir.SetName(u_arr0, "u_arr0");
        auto* u_arr1 = b.Call(fn_u, b.Access(ty.ptr<uniform, vec4<i32>>(), U_arr, 1_i));
        b.ir.SetName(u_arr1, "u_arr1");
        auto* u_arrI = b.Call(fn_u, b.Access(ty.ptr<uniform, vec4<i32>>(), U_arr, I));
        b.ir.SetName(u_arrI, "u_arrI");
        auto* u_arr1_arr0 =
            b.Call(fn_u, b.Access(ty.ptr<uniform, vec4<i32>>(), U_arr_arr, 1_i, 0_i));
        b.ir.SetName(u_arr1_arr0, "u_arr1_arr0");
        auto* u_arr2_arrI = b.Call(fn_u, b.Access(ty.ptr<uniform, vec4<i32>>(), U_arr_arr, 2_i, I));
        b.ir.SetName(u_arr2_arrI, "u_arr2_arrI");
        auto* u_arrI_arr2 = b.Call(fn_u, b.Access(ty.ptr<uniform, vec4<i32>>(), U_arr_arr, I, 2_i));
        b.ir.SetName(u_arrI_arr2, "u_arrI_arr2");
        auto* u_arrI_arrJ = b.Call(fn_u, b.Access(ty.ptr<uniform, vec4<i32>>(), U_arr_arr, I, J));
        b.ir.SetName(u_arrI_arrJ, "u_arrI_arrJ");

        auto* s = b.Call(fn_s, S);
        b.ir.SetName(s, "s");
        auto* s_str = b.Call(fn_s, b.Access(ty.ptr<storage, vec4<i32>, read>(), S_str, 0_u));
        b.ir.SetName(s_str, "s_str");
        auto* s_arr0 = b.Call(fn_s, b.Access(ty.ptr<storage, vec4<i32>, read>(), S_arr, 0_i));
        b.ir.SetName(s_arr0, "s_arr0");
        auto* s_arr1 = b.Call(fn_s, b.Access(ty.ptr<storage, vec4<i32>, read>(), S_arr, 1_i));
        b.ir.SetName(s_arr1, "s_arr1");
        auto* s_arrI = b.Call(fn_s, b.Access(ty.ptr<storage, vec4<i32>, read>(), S_arr, I));
        b.ir.SetName(s_arrI, "s_arrI");
        auto* s_arr1_arr0 =
            b.Call(fn_s, b.Access(ty.ptr<storage, vec4<i32>, read>(), S_arr_arr, 1_i, 0_i));
        b.ir.SetName(s_arr1_arr0, "s_arr1_arr0");
        auto* s_arr2_arrI =
            b.Call(fn_s, b.Access(ty.ptr<storage, vec4<i32>, read>(), S_arr_arr, 2_i, I));
        b.ir.SetName(s_arr2_arrI, "s_arr2_arrI");
        auto* s_arrI_arr2 =
            b.Call(fn_s, b.Access(ty.ptr<storage, vec4<i32>, read>(), S_arr_arr, I, 2_i));
        b.ir.SetName(s_arrI_arr2, "s_arrI_arr2");
        auto* s_arrI_arrJ =
            b.Call(fn_s, b.Access(ty.ptr<storage, vec4<i32>, read>(), S_arr_arr, I, J));
        b.ir.SetName(s_arrI_arrJ, "s_arrI_arrJ");

        auto* w = b.Call(fn_w, W);
        b.ir.SetName(w, "w");
        auto* w_str = b.Call(fn_w, b.Access(ty.ptr<workgroup, vec4<i32>>(), W_str, 0_u));
        b.ir.SetName(w_str, "w_str");
        auto* w_arr0 = b.Call(fn_w, b.Access(ty.ptr<workgroup, vec4<i32>>(), W_arr, 0_i));
        b.ir.SetName(w_arr0, "w_arr0");
        auto* w_arr1 = b.Call(fn_w, b.Access(ty.ptr<workgroup, vec4<i32>>(), W_arr, 1_i));
        b.ir.SetName(w_arr1, "w_arr1");
        auto* w_arrI = b.Call(fn_w, b.Access(ty.ptr<workgroup, vec4<i32>>(), W_arr, I));
        b.ir.SetName(w_arrI, "w_arrI");
        auto* w_arr1_arr0 =
            b.Call(fn_w, b.Access(ty.ptr<workgroup, vec4<i32>>(), W_arr_arr, 1_i, 0_i));
        b.ir.SetName(w_arr1_arr0, "w_arr1_arr0");
        auto* w_arr2_arrI =
            b.Call(fn_w, b.Access(ty.ptr<workgroup, vec4<i32>>(), W_arr_arr, 2_i, I));
        b.ir.SetName(w_arr2_arrI, "w_arr2_arrI");
        auto* w_arrI_arr2 =
            b.Call(fn_w, b.Access(ty.ptr<workgroup, vec4<i32>>(), W_arr_arr, I, 2_i));
        b.ir.SetName(w_arrI_arr2, "w_arrI_arr2");
        auto* w_arrI_arrJ = b.Call(fn_w, b.Access(ty.ptr<workgroup, vec4<i32>>(), W_arr_arr, I, J));
        b.ir.SetName(w_arrI_arrJ, "w_arrI_arrJ");

        b.Return(fn_b);
    });

    auto* src = R"(
str = struct @align(16) {
  i:vec4<i32> @offset(0)
}

$B1: {  # root
  %U:ptr<uniform, vec4<i32>, read> = var undef @binding_point(0, 0)
  %U_str:ptr<uniform, str, read> = var undef @binding_point(0, 1)
  %U_arr:ptr<uniform, array<vec4<i32>, 8>, read> = var undef @binding_point(0, 2)
  %U_arr_arr:ptr<uniform, array<array<vec4<i32>, 8>, 4>, read> = var undef @binding_point(0, 3)
  %S:ptr<storage, vec4<i32>, read> = var undef @binding_point(1, 0)
  %S_str:ptr<storage, str, read> = var undef @binding_point(1, 1)
  %S_arr:ptr<storage, array<vec4<i32>, 8>, read> = var undef @binding_point(1, 2)
  %S_arr_arr:ptr<storage, array<array<vec4<i32>, 8>, 4>, read> = var undef @binding_point(1, 3)
  %W:ptr<workgroup, vec4<i32>, read_write> = var undef
  %W_str:ptr<workgroup, str, read_write> = var undef
  %W_arr:ptr<workgroup, array<vec4<i32>, 8>, read_write> = var undef
  %W_arr_arr:ptr<workgroup, array<array<vec4<i32>, 8>, 4>, read_write> = var undef
}

%fn_u = func(%p:ptr<uniform, vec4<i32>, read>):vec4<i32> {
  $B2: {
    %15:vec4<i32> = load %p
    ret %15
  }
}
%fn_s = func(%p_1:ptr<storage, vec4<i32>, read>):vec4<i32> {  # %p_1: 'p'
  $B3: {
    %18:vec4<i32> = load %p_1
    ret %18
  }
}
%fn_w = func(%p_2:ptr<workgroup, vec4<i32>, read_write>):vec4<i32> {  # %p_2: 'p'
  $B4: {
    %21:vec4<i32> = load %p_2
    ret %21
  }
}
%b = func():void {
  $B5: {
    %I:i32 = let 3i
    %J:i32 = let 4i
    %u:vec4<i32> = call %fn_u, %U
    %26:ptr<uniform, vec4<i32>, read> = access %U_str, 0u
    %u_str:vec4<i32> = call %fn_u, %26
    %28:ptr<uniform, vec4<i32>, read> = access %U_arr, 0i
    %u_arr0:vec4<i32> = call %fn_u, %28
    %30:ptr<uniform, vec4<i32>, read> = access %U_arr, 1i
    %u_arr1:vec4<i32> = call %fn_u, %30
    %32:ptr<uniform, vec4<i32>, read> = access %U_arr, %I
    %u_arrI:vec4<i32> = call %fn_u, %32
    %34:ptr<uniform, vec4<i32>, read> = access %U_arr_arr, 1i, 0i
    %u_arr1_arr0:vec4<i32> = call %fn_u, %34
    %36:ptr<uniform, vec4<i32>, read> = access %U_arr_arr, 2i, %I
    %u_arr2_arrI:vec4<i32> = call %fn_u, %36
    %38:ptr<uniform, vec4<i32>, read> = access %U_arr_arr, %I, 2i
    %u_arrI_arr2:vec4<i32> = call %fn_u, %38
    %40:ptr<uniform, vec4<i32>, read> = access %U_arr_arr, %I, %J
    %u_arrI_arrJ:vec4<i32> = call %fn_u, %40
    %s:vec4<i32> = call %fn_s, %S
    %43:ptr<storage, vec4<i32>, read> = access %S_str, 0u
    %s_str:vec4<i32> = call %fn_s, %43
    %45:ptr<storage, vec4<i32>, read> = access %S_arr, 0i
    %s_arr0:vec4<i32> = call %fn_s, %45
    %47:ptr<storage, vec4<i32>, read> = access %S_arr, 1i
    %s_arr1:vec4<i32> = call %fn_s, %47
    %49:ptr<storage, vec4<i32>, read> = access %S_arr, %I
    %s_arrI:vec4<i32> = call %fn_s, %49
    %51:ptr<storage, vec4<i32>, read> = access %S_arr_arr, 1i, 0i
    %s_arr1_arr0:vec4<i32> = call %fn_s, %51
    %53:ptr<storage, vec4<i32>, read> = access %S_arr_arr, 2i, %I
    %s_arr2_arrI:vec4<i32> = call %fn_s, %53
    %55:ptr<storage, vec4<i32>, read> = access %S_arr_arr, %I, 2i
    %s_arrI_arr2:vec4<i32> = call %fn_s, %55
    %57:ptr<storage, vec4<i32>, read> = access %S_arr_arr, %I, %J
    %s_arrI_arrJ:vec4<i32> = call %fn_s, %57
    %w:vec4<i32> = call %fn_w, %W
    %60:ptr<workgroup, vec4<i32>, read_write> = access %W_str, 0u
    %w_str:vec4<i32> = call %fn_w, %60
    %62:ptr<workgroup, vec4<i32>, read_write> = access %W_arr, 0i
    %w_arr0:vec4<i32> = call %fn_w, %62
    %64:ptr<workgroup, vec4<i32>, read_write> = access %W_arr, 1i
    %w_arr1:vec4<i32> = call %fn_w, %64
    %66:ptr<workgroup, vec4<i32>, read_write> = access %W_arr, %I
    %w_arrI:vec4<i32> = call %fn_w, %66
    %68:ptr<workgroup, vec4<i32>, read_write> = access %W_arr_arr, 1i, 0i
    %w_arr1_arr0:vec4<i32> = call %fn_w, %68
    %70:ptr<workgroup, vec4<i32>, read_write> = access %W_arr_arr, 2i, %I
    %w_arr2_arrI:vec4<i32> = call %fn_w, %70
    %72:ptr<workgroup, vec4<i32>, read_write> = access %W_arr_arr, %I, 2i
    %w_arrI_arr2:vec4<i32> = call %fn_w, %72
    %74:ptr<workgroup, vec4<i32>, read_write> = access %W_arr_arr, %I, %J
    %w_arrI_arrJ:vec4<i32> = call %fn_w, %74
    ret
  }
}
)";

    EXPECT_EQ(src, str());

    auto* expect = R"(
str = struct @align(16) {
  i:vec4<i32> @offset(0)
}

$B1: {  # root
  %U:ptr<uniform, vec4<i32>, read> = var undef @binding_point(0, 0)
  %U_str:ptr<uniform, str, read> = var undef @binding_point(0, 1)
  %U_arr:ptr<uniform, array<vec4<i32>, 8>, read> = var undef @binding_point(0, 2)
  %U_arr_arr:ptr<uniform, array<array<vec4<i32>, 8>, 4>, read> = var undef @binding_point(0, 3)
  %S:ptr<storage, vec4<i32>, read> = var undef @binding_point(1, 0)
  %S_str:ptr<storage, str, read> = var undef @binding_point(1, 1)
  %S_arr:ptr<storage, array<vec4<i32>, 8>, read> = var undef @binding_point(1, 2)
  %S_arr_arr:ptr<storage, array<array<vec4<i32>, 8>, 4>, read> = var undef @binding_point(1, 3)
  %W:ptr<workgroup, vec4<i32>, read_write> = var undef
  %W_str:ptr<workgroup, str, read_write> = var undef
  %W_arr:ptr<workgroup, array<vec4<i32>, 8>, read_write> = var undef
  %W_arr_arr:ptr<workgroup, array<array<vec4<i32>, 8>, 4>, read_write> = var undef
}

%fn_u = func():vec4<i32> {
  $B2: {
    %14:vec4<i32> = load %U
    ret %14
  }
}
%fn_u_1 = func():vec4<i32> {  # %fn_u_1: 'fn_u'
  $B3: {
    %16:ptr<uniform, vec4<i32>, read> = access %U_str, 0u
    %17:vec4<i32> = load %16
    ret %17
  }
}
%fn_u_2 = func(%p_indices:array<u32, 1>):vec4<i32> {  # %fn_u_2: 'fn_u'
  $B4: {
    %20:u32 = access %p_indices, 0u
    %21:ptr<uniform, vec4<i32>, read> = access %U_arr, %20
    %22:vec4<i32> = load %21
    ret %22
  }
}
%fn_u_3 = func(%p_indices_1:array<u32, 2>):vec4<i32> {  # %fn_u_3: 'fn_u', %p_indices_1: 'p_indices'
  $B5: {
    %25:u32 = access %p_indices_1, 0u
    %26:u32 = access %p_indices_1, 1u
    %27:ptr<uniform, vec4<i32>, read> = access %U_arr_arr, %25, %26
    %28:vec4<i32> = load %27
    ret %28
  }
}
%fn_s = func():vec4<i32> {
  $B6: {
    %30:vec4<i32> = load %S
    ret %30
  }
}
%fn_s_1 = func():vec4<i32> {  # %fn_s_1: 'fn_s'
  $B7: {
    %32:ptr<storage, vec4<i32>, read> = access %S_str, 0u
    %33:vec4<i32> = load %32
    ret %33
  }
}
%fn_s_2 = func(%p_indices_2:array<u32, 1>):vec4<i32> {  # %fn_s_2: 'fn_s', %p_indices_2: 'p_indices'
  $B8: {
    %36:u32 = access %p_indices_2, 0u
    %37:ptr<storage, vec4<i32>, read> = access %S_arr, %36
    %38:vec4<i32> = load %37
    ret %38
  }
}
%fn_s_3 = func(%p_indices_3:array<u32, 2>):vec4<i32> {  # %fn_s_3: 'fn_s', %p_indices_3: 'p_indices'
  $B9: {
    %41:u32 = access %p_indices_3, 0u
    %42:u32 = access %p_indices_3, 1u
    %43:ptr<storage, vec4<i32>, read> = access %S_arr_arr, %41, %42
    %44:vec4<i32> = load %43
    ret %44
  }
}
%fn_w = func():vec4<i32> {
  $B10: {
    %46:vec4<i32> = load %W
    ret %46
  }
}
%fn_w_1 = func():vec4<i32> {  # %fn_w_1: 'fn_w'
  $B11: {
    %48:ptr<workgroup, vec4<i32>, read_write> = access %W_str, 0u
    %49:vec4<i32> = load %48
    ret %49
  }
}
%fn_w_2 = func(%p_indices_4:array<u32, 1>):vec4<i32> {  # %fn_w_2: 'fn_w', %p_indices_4: 'p_indices'
  $B12: {
    %52:u32 = access %p_indices_4, 0u
    %53:ptr<workgroup, vec4<i32>, read_write> = access %W_arr, %52
    %54:vec4<i32> = load %53
    ret %54
  }
}
%fn_w_3 = func(%p_indices_5:array<u32, 2>):vec4<i32> {  # %fn_w_3: 'fn_w', %p_indices_5: 'p_indices'
  $B13: {
    %57:u32 = access %p_indices_5, 0u
    %58:u32 = access %p_indices_5, 1u
    %59:ptr<workgroup, vec4<i32>, read_write> = access %W_arr_arr, %57, %58
    %60:vec4<i32> = load %59
    ret %60
  }
}
%b = func():void {
  $B14: {
    %I:i32 = let 3i
    %J:i32 = let 4i
    %u:vec4<i32> = call %fn_u
    %u_str:vec4<i32> = call %fn_u_1
    %66:u32 = convert 0i
    %67:array<u32, 1> = construct %66
    %u_arr0:vec4<i32> = call %fn_u_2, %67
    %69:u32 = convert 1i
    %70:array<u32, 1> = construct %69
    %u_arr1:vec4<i32> = call %fn_u_2, %70
    %72:u32 = convert %I
    %73:array<u32, 1> = construct %72
    %u_arrI:vec4<i32> = call %fn_u_2, %73
    %75:u32 = convert 1i
    %76:u32 = convert 0i
    %77:array<u32, 2> = construct %75, %76
    %u_arr1_arr0:vec4<i32> = call %fn_u_3, %77
    %79:u32 = convert 2i
    %80:u32 = convert %I
    %81:array<u32, 2> = construct %79, %80
    %u_arr2_arrI:vec4<i32> = call %fn_u_3, %81
    %83:u32 = convert %I
    %84:u32 = convert 2i
    %85:array<u32, 2> = construct %83, %84
    %u_arrI_arr2:vec4<i32> = call %fn_u_3, %85
    %87:u32 = convert %I
    %88:u32 = convert %J
    %89:array<u32, 2> = construct %87, %88
    %u_arrI_arrJ:vec4<i32> = call %fn_u_3, %89
    %s:vec4<i32> = call %fn_s
    %s_str:vec4<i32> = call %fn_s_1
    %93:u32 = convert 0i
    %94:array<u32, 1> = construct %93
    %s_arr0:vec4<i32> = call %fn_s_2, %94
    %96:u32 = convert 1i
    %97:array<u32, 1> = construct %96
    %s_arr1:vec4<i32> = call %fn_s_2, %97
    %99:u32 = convert %I
    %100:array<u32, 1> = construct %99
    %s_arrI:vec4<i32> = call %fn_s_2, %100
    %102:u32 = convert 1i
    %103:u32 = convert 0i
    %104:array<u32, 2> = construct %102, %103
    %s_arr1_arr0:vec4<i32> = call %fn_s_3, %104
    %106:u32 = convert 2i
    %107:u32 = convert %I
    %108:array<u32, 2> = construct %106, %107
    %s_arr2_arrI:vec4<i32> = call %fn_s_3, %108
    %110:u32 = convert %I
    %111:u32 = convert 2i
    %112:array<u32, 2> = construct %110, %111
    %s_arrI_arr2:vec4<i32> = call %fn_s_3, %112
    %114:u32 = convert %I
    %115:u32 = convert %J
    %116:array<u32, 2> = construct %114, %115
    %s_arrI_arrJ:vec4<i32> = call %fn_s_3, %116
    %w:vec4<i32> = call %fn_w
    %w_str:vec4<i32> = call %fn_w_1
    %120:u32 = convert 0i
    %121:array<u32, 1> = construct %120
    %w_arr0:vec4<i32> = call %fn_w_2, %121
    %123:u32 = convert 1i
    %124:array<u32, 1> = construct %123
    %w_arr1:vec4<i32> = call %fn_w_2, %124
    %126:u32 = convert %I
    %127:array<u32, 1> = construct %126
    %w_arrI:vec4<i32> = call %fn_w_2, %127
    %129:u32 = convert 1i
    %130:u32 = convert 0i
    %131:array<u32, 2> = construct %129, %130
    %w_arr1_arr0:vec4<i32> = call %fn_w_3, %131
    %133:u32 = convert 2i
    %134:u32 = convert %I
    %135:array<u32, 2> = construct %133, %134
    %w_arr2_arrI:vec4<i32> = call %fn_w_3, %135
    %137:u32 = convert %I
    %138:u32 = convert 2i
    %139:array<u32, 2> = construct %137, %138
    %w_arrI_arr2:vec4<i32> = call %fn_w_3, %139
    %141:u32 = convert %I
    %142:u32 = convert %J
    %143:array<u32, 2> = construct %141, %142
    %w_arrI_arrJ:vec4<i32> = call %fn_w_3, %143
    ret
  }
}
)";

    Run(DirectVariableAccess, DirectVariableAccessOptions{});

    EXPECT_EQ(expect, str());
}

TEST_F(IR_DirectVariableAccessTest_Complex, Indexing) {
    Var* S = nullptr;
    b.Append(b.ir.root_block,
             [&] {  //
                 S = b.Var<storage, array<array<array<array<i32, 9>, 9>, 9>, 50>, read>("S");
                 S->SetBindingPoint(0, 0);
             });

    auto* fn_a = b.Function("a", ty.i32());
    auto* fn_a_i = b.FunctionParam("i", ty.i32());
    fn_a->SetParams({fn_a_i});
    b.Append(fn_a->Block(), [&] { b.Return(fn_a, fn_a_i); });

    auto* fn_b = b.Function("b", ty.i32());
    auto* fn_b_p = b.FunctionParam("p", ty.ptr<storage, array<array<array<i32, 9>, 9>, 9>, read>());
    fn_b->SetParams({fn_b_p});
    b.Append(fn_b->Block(), [&] {
        auto load_0 = b.Load(b.Access(ty.ptr<storage, i32, read>(), fn_b_p, 0_i, 1_i, 2_i));
        auto call_0 = b.Call(fn_a, load_0);
        auto call_1 = b.Call(fn_a, 3_i);
        auto load_1 = b.Load(b.Access(ty.ptr<storage, i32, read>(), fn_b_p, call_1, 4_i, 5_i));
        auto call_2 = b.Call(fn_a, load_1);
        auto call_3 = b.Call(fn_a, 7_i);
        auto load_2 = b.Load(b.Access(ty.ptr<storage, i32, read>(), fn_b_p, 6_i, call_3, 8_i));
        auto call_4 = b.Call(fn_a, load_2);
        auto load_3 =
            b.Load(b.Access(ty.ptr<storage, i32, read>(), fn_b_p, call_0, call_2, call_4));

        b.Return(fn_b, load_3);
    });

    auto* fn_c = b.Function("c", ty.void_());
    b.Append(fn_c->Block(), [&] {
        auto* access =
            b.Access(ty.ptr<storage, array<array<array<i32, 9>, 9>, 9>, read>(), S, 42_i);
        auto* v = b.Call(fn_b, access);
        b.ir.SetName(v, "v");
        b.Return(fn_c);
    });

    auto* src = R"(
$B1: {  # root
  %S:ptr<storage, array<array<array<array<i32, 9>, 9>, 9>, 50>, read> = var undef @binding_point(0, 0)
}

%a = func(%i:i32):i32 {
  $B2: {
    ret %i
  }
}
%b = func(%p:ptr<storage, array<array<array<i32, 9>, 9>, 9>, read>):i32 {
  $B3: {
    %6:ptr<storage, i32, read> = access %p, 0i, 1i, 2i
    %7:i32 = load %6
    %8:i32 = call %a, %7
    %9:i32 = call %a, 3i
    %10:ptr<storage, i32, read> = access %p, %9, 4i, 5i
    %11:i32 = load %10
    %12:i32 = call %a, %11
    %13:i32 = call %a, 7i
    %14:ptr<storage, i32, read> = access %p, 6i, %13, 8i
    %15:i32 = load %14
    %16:i32 = call %a, %15
    %17:ptr<storage, i32, read> = access %p, %8, %12, %16
    %18:i32 = load %17
    ret %18
  }
}
%c = func():void {
  $B4: {
    %20:ptr<storage, array<array<array<i32, 9>, 9>, 9>, read> = access %S, 42i
    %v:i32 = call %b, %20
    ret
  }
}
)";

    EXPECT_EQ(src, str());

    auto* expect = R"(
$B1: {  # root
  %S:ptr<storage, array<array<array<array<i32, 9>, 9>, 9>, 50>, read> = var undef @binding_point(0, 0)
}

%a = func(%i:i32):i32 {
  $B2: {
    ret %i
  }
}
%b = func(%p_indices:array<u32, 1>):i32 {
  $B3: {
    %6:u32 = access %p_indices, 0u
    %7:ptr<storage, array<array<array<i32, 9>, 9>, 9>, read> = access %S, %6
    %8:ptr<storage, i32, read> = access %7, 0i, 1i, 2i
    %9:i32 = load %8
    %10:i32 = call %a, %9
    %11:i32 = call %a, 3i
    %12:ptr<storage, i32, read> = access %7, %11, 4i, 5i
    %13:i32 = load %12
    %14:i32 = call %a, %13
    %15:i32 = call %a, 7i
    %16:ptr<storage, i32, read> = access %7, 6i, %15, 8i
    %17:i32 = load %16
    %18:i32 = call %a, %17
    %19:ptr<storage, i32, read> = access %7, %10, %14, %18
    %20:i32 = load %19
    ret %20
  }
}
%c = func():void {
  $B4: {
    %22:u32 = convert 42i
    %23:array<u32, 1> = construct %22
    %v:i32 = call %b, %23
    ret
  }
}
)";

    Run(DirectVariableAccess, DirectVariableAccessOptions{});

    EXPECT_EQ(expect, str());
}

TEST_F(IR_DirectVariableAccessTest_Complex, IndexingInPtrCall) {
    Var* S = nullptr;
    b.Append(b.ir.root_block,
             [&] {  //
                 S = b.Var<storage, array<array<array<array<i32, 9>, 9>, 9>, 50>, read>("S");
                 S->SetBindingPoint(0, 0);
             });

    auto* fn_a = b.Function("a", ty.i32());
    auto* fn_a_i = b.FunctionParam("i", ty.ptr<storage, i32, read>());
    fn_a->SetParams({
        b.FunctionParam("pre", ty.i32()),
        fn_a_i,
        b.FunctionParam("post", ty.i32()),
    });
    b.Append(fn_a->Block(), [&] { b.Return(fn_a, b.Load(fn_a_i)); });

    auto* fn_b = b.Function("b", ty.i32());
    auto* fn_b_p = b.FunctionParam("p", ty.ptr<storage, array<array<array<i32, 9>, 9>, 9>, read>());
    fn_b->SetParams({fn_b_p});
    b.Append(fn_b->Block(), [&] {
        auto access_0 = b.Access(ty.ptr<storage, i32, read>(), fn_b_p, 0_i, 1_i, 2_i);
        auto call_0 = b.Call(fn_a, 20_i, access_0, 30_i);

        auto access_1 = b.Access(ty.ptr<storage, i32, read>(), fn_b_p, 3_i, 4_i, 5_i);
        auto call_1 = b.Call(fn_a, 40_i, access_1, 50_i);

        auto access_2 = b.Access(ty.ptr<storage, i32, read>(), fn_b_p, 6_i, 7_i, 8_i);
        auto call_2 = b.Call(fn_a, 60_i, access_2, 70_i);

        auto access_3 = b.Access(ty.ptr<storage, i32, read>(), fn_b_p, call_0, call_1, call_2);
        auto call_3 = b.Call(fn_a, 10_i, access_3, 80_i);

        b.Return(fn_b, call_3);
    });

    auto* fn_c = b.Function("c", ty.void_());
    b.Append(fn_c->Block(), [&] {
        auto* access =
            b.Access(ty.ptr<storage, array<array<array<i32, 9>, 9>, 9>, read>(), S, 42_i);
        auto* v = b.Call(fn_b, access);
        b.ir.SetName(v, "v");
        b.Return(fn_c);
    });

    auto* src = R"(
$B1: {  # root
  %S:ptr<storage, array<array<array<array<i32, 9>, 9>, 9>, 50>, read> = var undef @binding_point(0, 0)
}

%a = func(%pre:i32, %i:ptr<storage, i32, read>, %post:i32):i32 {
  $B2: {
    %6:i32 = load %i
    ret %6
  }
}
%b = func(%p:ptr<storage, array<array<array<i32, 9>, 9>, 9>, read>):i32 {
  $B3: {
    %9:ptr<storage, i32, read> = access %p, 0i, 1i, 2i
    %10:i32 = call %a, 20i, %9, 30i
    %11:ptr<storage, i32, read> = access %p, 3i, 4i, 5i
    %12:i32 = call %a, 40i, %11, 50i
    %13:ptr<storage, i32, read> = access %p, 6i, 7i, 8i
    %14:i32 = call %a, 60i, %13, 70i
    %15:ptr<storage, i32, read> = access %p, %10, %12, %14
    %16:i32 = call %a, 10i, %15, 80i
    ret %16
  }
}
%c = func():void {
  $B4: {
    %18:ptr<storage, array<array<array<i32, 9>, 9>, 9>, read> = access %S, 42i
    %v:i32 = call %b, %18
    ret
  }
}
)";

    EXPECT_EQ(src, str());

    auto* expect = R"(
$B1: {  # root
  %S:ptr<storage, array<array<array<array<i32, 9>, 9>, 9>, 50>, read> = var undef @binding_point(0, 0)
}

%a = func(%pre:i32, %i_indices:array<u32, 4>, %post:i32):i32 {
  $B2: {
    %6:u32 = access %i_indices, 0u
    %7:u32 = access %i_indices, 1u
    %8:u32 = access %i_indices, 2u
    %9:u32 = access %i_indices, 3u
    %10:ptr<storage, i32, read> = access %S, %6, %7, %8, %9
    %11:i32 = load %10
    ret %11
  }
}
%b = func(%p_indices:array<u32, 1>):i32 {
  $B3: {
    %14:u32 = access %p_indices, 0u
    %15:u32 = convert 0i
    %16:u32 = convert 1i
    %17:u32 = convert 2i
    %18:array<u32, 4> = construct %14, %15, %16, %17
    %19:i32 = call %a, 20i, %18, 30i
    %20:u32 = convert 3i
    %21:u32 = convert 4i
    %22:u32 = convert 5i
    %23:array<u32, 4> = construct %14, %20, %21, %22
    %24:i32 = call %a, 40i, %23, 50i
    %25:u32 = convert 6i
    %26:u32 = convert 7i
    %27:u32 = convert 8i
    %28:array<u32, 4> = construct %14, %25, %26, %27
    %29:i32 = call %a, 60i, %28, 70i
    %30:u32 = convert %19
    %31:u32 = convert %24
    %32:u32 = convert %29
    %33:array<u32, 4> = construct %14, %30, %31, %32
    %34:i32 = call %a, 10i, %33, 80i
    ret %34
  }
}
%c = func():void {
  $B4: {
    %36:u32 = convert 42i
    %37:array<u32, 1> = construct %36
    %v:i32 = call %b, %37
    ret
  }
}
)";

    Run(DirectVariableAccess, DirectVariableAccessOptions{});

    EXPECT_EQ(expect, str());
}

TEST_F(IR_DirectVariableAccessTest_Complex, IndexingDualPointers) {
    Var* S = nullptr;
    Var* U = nullptr;
    b.Append(b.ir.root_block,
             [&] {  //
                 S = b.Var<storage, array<array<array<i32, 9>, 9>, 50>, read>("S");
                 S->SetBindingPoint(0, 0);
                 U = b.Var<uniform, array<array<array<vec4<i32>, 9>, 9>, 50>, read>("U");
                 U->SetBindingPoint(0, 0);
             });

    auto* fn_a = b.Function("a", ty.i32());
    auto* fn_a_i = b.FunctionParam("i", ty.i32());
    fn_a->SetParams({fn_a_i});
    b.Append(fn_a->Block(), [&] { b.Return(fn_a, fn_a_i); });

    auto* fn_b = b.Function("b", ty.i32());
    auto* fn_b_s = b.FunctionParam("s", ty.ptr<storage, array<array<i32, 9>, 9>, read>());
    auto* fn_b_u = b.FunctionParam("u", ty.ptr<uniform, array<array<vec4<i32>, 9>, 9>, read>());
    fn_b->SetParams({fn_b_s, fn_b_u});
    b.Append(fn_b->Block(), [&] {
        auto access_0 = b.Access(ty.ptr<uniform, vec4<i32>, read>(), fn_b_u, 0_i, 1_i);
        auto call_0 = b.Call(fn_a, b.LoadVectorElement(access_0, 0_u));
        auto call_1 = b.Call(fn_a, 3_i);

        auto access_1 = b.Access(ty.ptr<uniform, vec4<i32>, read>(), fn_b_u, call_1, 4_i);
        auto call_2 = b.Call(fn_a, b.LoadVectorElement(access_1, 1_u));

        auto access_2 = b.Access(ty.ptr<storage, i32, read>(), fn_b_s, call_0, call_2);

        b.Return(fn_b, b.Load(access_2));
    });

    auto* fn_c = b.Function("c", ty.void_());
    b.Append(fn_c->Block(), [&] {
        auto* access_0 = b.Access(ty.ptr<storage, array<array<i32, 9>, 9>, read>(), S, 42_i);
        auto* access_1 = b.Access(ty.ptr<uniform, array<array<vec4<i32>, 9>, 9>, read>(), U, 24_i);
        auto* v = b.Call(fn_b, access_0, access_1);
        b.ir.SetName(v, "v");
        b.Return(fn_c);
    });

    auto* src = R"(
$B1: {  # root
  %S:ptr<storage, array<array<array<i32, 9>, 9>, 50>, read> = var undef @binding_point(0, 0)
  %U:ptr<uniform, array<array<array<vec4<i32>, 9>, 9>, 50>, read> = var undef @binding_point(0, 0)
}

%a = func(%i:i32):i32 {
  $B2: {
    ret %i
  }
}
%b = func(%s:ptr<storage, array<array<i32, 9>, 9>, read>, %u:ptr<uniform, array<array<vec4<i32>, 9>, 9>, read>):i32 {
  $B3: {
    %8:ptr<uniform, vec4<i32>, read> = access %u, 0i, 1i
    %9:i32 = load_vector_element %8, 0u
    %10:i32 = call %a, %9
    %11:i32 = call %a, 3i
    %12:ptr<uniform, vec4<i32>, read> = access %u, %11, 4i
    %13:i32 = load_vector_element %12, 1u
    %14:i32 = call %a, %13
    %15:ptr<storage, i32, read> = access %s, %10, %14
    %16:i32 = load %15
    ret %16
  }
}
%c = func():void {
  $B4: {
    %18:ptr<storage, array<array<i32, 9>, 9>, read> = access %S, 42i
    %19:ptr<uniform, array<array<vec4<i32>, 9>, 9>, read> = access %U, 24i
    %v:i32 = call %b, %18, %19
    ret
  }
}
)";

    EXPECT_EQ(src, str());

    auto* expect = R"(
$B1: {  # root
  %S:ptr<storage, array<array<array<i32, 9>, 9>, 50>, read> = var undef @binding_point(0, 0)
  %U:ptr<uniform, array<array<array<vec4<i32>, 9>, 9>, 50>, read> = var undef @binding_point(0, 0)
}

%a = func(%i:i32):i32 {
  $B2: {
    ret %i
  }
}
%b = func(%s_indices:array<u32, 1>, %u_indices:array<u32, 1>):i32 {
  $B3: {
    %8:u32 = access %s_indices, 0u
    %9:ptr<storage, array<array<i32, 9>, 9>, read> = access %S, %8
    %10:u32 = access %u_indices, 0u
    %11:ptr<uniform, array<array<vec4<i32>, 9>, 9>, read> = access %U, %10
    %12:ptr<uniform, vec4<i32>, read> = access %11, 0i, 1i
    %13:i32 = load_vector_element %12, 0u
    %14:i32 = call %a, %13
    %15:i32 = call %a, 3i
    %16:ptr<uniform, vec4<i32>, read> = access %11, %15, 4i
    %17:i32 = load_vector_element %16, 1u
    %18:i32 = call %a, %17
    %19:ptr<storage, i32, read> = access %9, %14, %18
    %20:i32 = load %19
    ret %20
  }
}
%c = func():void {
  $B4: {
    %22:u32 = convert 42i
    %23:array<u32, 1> = construct %22
    %24:u32 = convert 24i
    %25:array<u32, 1> = construct %24
    %v:i32 = call %b, %23, %25
    ret
  }
}
)";

    Run(DirectVariableAccess, DirectVariableAccessOptions{});

    EXPECT_EQ(expect, str());
}

}  // namespace complex_tests

}  // namespace
}  // namespace tint::core::ir::transform
