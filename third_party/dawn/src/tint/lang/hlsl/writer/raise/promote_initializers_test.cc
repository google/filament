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

#include "src/tint/lang/hlsl/writer/raise/promote_initializers.h"

#include <utility>

#include "gtest/gtest.h"
#include "src/tint/lang/core/ir/transform/helper_test.h"

using namespace tint::core::fluent_types;     // NOLINT
using namespace tint::core::number_suffixes;  // NOLINT

namespace tint::hlsl::writer::raise {
namespace {

using HlslWriterPromoteInitializersTest = core::ir::transform::TransformTest;

TEST_F(HlslWriterPromoteInitializersTest, NoStructInitializers) {
    auto* func = b.Function("foo", ty.void_(), core::ir::Function::PipelineStage::kFragment);
    b.Append(func->Block(), [&] {
        b.Var<function>("a", b.Zero<i32>());
        b.Return(func);
    });

    auto* src = R"(
%foo = @fragment func():void {
  $B1: {
    %a:ptr<function, i32, read_write> = var 0i
    ret
  }
}
)";
    EXPECT_EQ(src, str());

    auto* expect = src;
    Run(PromoteInitializers);

    EXPECT_EQ(expect, str());
}

TEST_F(HlslWriterPromoteInitializersTest, StructInVarNoChange) {
    auto* str_ty = ty.Struct(mod.symbols.New("S"), {
                                                       {mod.symbols.New("a"), ty.i32()},
                                                   });

    auto* func = b.Function("foo", ty.void_(), core::ir::Function::PipelineStage::kFragment);
    b.Append(func->Block(), [&] {
        b.Var<function>("a", b.Composite(str_ty, 1_i));
        b.Return(func);
    });

    auto* src = R"(
S = struct @align(4) {
  a:i32 @offset(0)
}

%foo = @fragment func():void {
  $B1: {
    %a:ptr<function, S, read_write> = var S(1i)
    ret
  }
}
)";
    EXPECT_EQ(src, str());

    auto* expect = src;
    Run(PromoteInitializers);

    EXPECT_EQ(expect, str());
}

TEST_F(HlslWriterPromoteInitializersTest, ArrayInVarNoChange) {
    auto* func = b.Function("foo", ty.void_(), core::ir::Function::PipelineStage::kFragment);
    b.Append(func->Block(), [&] {
        b.Var<function>("a", b.Zero<array<i32, 2>>());
        b.Return(func);
    });

    auto* src = R"(
%foo = @fragment func():void {
  $B1: {
    %a:ptr<function, array<i32, 2>, read_write> = var array<i32, 2>(0i)
    ret
  }
}
)";
    EXPECT_EQ(src, str());

    auto* expect = src;
    Run(PromoteInitializers);

    EXPECT_EQ(expect, str());
}

TEST_F(HlslWriterPromoteInitializersTest, StructInLetNoChange) {
    auto* str_ty = ty.Struct(mod.symbols.New("S"), {
                                                       {mod.symbols.New("a"), ty.i32()},
                                                   });

    auto* func = b.Function("foo", ty.void_(), core::ir::Function::PipelineStage::kFragment);
    b.Append(func->Block(), [&] {
        b.Let("a", b.Composite(str_ty, 1_i));
        b.Return(func);
    });

    auto* src = R"(
S = struct @align(4) {
  a:i32 @offset(0)
}

%foo = @fragment func():void {
  $B1: {
    %a:S = let S(1i)
    ret
  }
}
)";
    EXPECT_EQ(src, str());

    auto* expect = src;
    Run(PromoteInitializers);

    EXPECT_EQ(expect, str());
}

TEST_F(HlslWriterPromoteInitializersTest, ArrayInLetNoChange) {
    auto* func = b.Function("foo", ty.void_(), core::ir::Function::PipelineStage::kFragment);
    b.Append(func->Block(), [&] {
        b.Let("a", b.Zero<array<i32, 2>>());
        b.Return(func);
    });

    auto* src = R"(
%foo = @fragment func():void {
  $B1: {
    %a:array<i32, 2> = let array<i32, 2>(0i)
    ret
  }
}
)";
    EXPECT_EQ(src, str());

    auto* expect = src;
    Run(PromoteInitializers);

    EXPECT_EQ(expect, str());
}

TEST_F(HlslWriterPromoteInitializersTest, StructInCall) {
    auto* str_ty = ty.Struct(mod.symbols.New("S"), {
                                                       {mod.symbols.New("a"), ty.i32()},
                                                   });

    auto* p = b.FunctionParam("p", str_ty);
    auto* dst = b.Function("dst", ty.void_());
    dst->SetParams({p});
    dst->Block()->Append(b.Return(dst));

    auto* func = b.Function("foo", ty.void_(), core::ir::Function::PipelineStage::kFragment);
    b.Append(func->Block(), [&] {
        b.Call(dst, b.Composite(str_ty, 1_i));
        b.Return(func);
    });

    auto* src = R"(
S = struct @align(4) {
  a:i32 @offset(0)
}

%dst = func(%p:S):void {
  $B1: {
    ret
  }
}
%foo = @fragment func():void {
  $B2: {
    %4:void = call %dst, S(1i)
    ret
  }
}
)";
    EXPECT_EQ(src, str());

    auto* expect = R"(
S = struct @align(4) {
  a:i32 @offset(0)
}

%dst = func(%p:S):void {
  $B1: {
    ret
  }
}
%foo = @fragment func():void {
  $B2: {
    %4:S = let S(1i)
    %5:void = call %dst, %4
    ret
  }
}
)";
    Run(PromoteInitializers);

    EXPECT_EQ(expect, str());
}

TEST_F(HlslWriterPromoteInitializersTest, ArrayInCall) {
    auto* p = b.FunctionParam("p", ty.array<i32, 2>());
    auto* dst = b.Function("dst", ty.void_());
    dst->SetParams({p});
    dst->Block()->Append(b.Return(dst));

    auto* func = b.Function("foo", ty.void_(), core::ir::Function::PipelineStage::kFragment);
    b.Append(func->Block(), [&] {
        b.Call(dst, b.Composite(ty.array<i32, 2>(), 1_i));
        b.Return(func);
    });

    auto* src = R"(
%dst = func(%p:array<i32, 2>):void {
  $B1: {
    ret
  }
}
%foo = @fragment func():void {
  $B2: {
    %4:void = call %dst, array<i32, 2>(1i)
    ret
  }
}
)";
    EXPECT_EQ(src, str());

    auto* expect = R"(
%dst = func(%p:array<i32, 2>):void {
  $B1: {
    ret
  }
}
%foo = @fragment func():void {
  $B2: {
    %4:array<i32, 2> = let array<i32, 2>(1i)
    %5:void = call %dst, %4
    ret
  }
}
)";
    Run(PromoteInitializers);

    EXPECT_EQ(expect, str());
}

TEST_F(HlslWriterPromoteInitializersTest, ModuleScopedStruct) {
    capabilities = core::ir::Capabilities{core::ir::Capability::kAllowModuleScopeLets};

    auto* str_ty = ty.Struct(mod.symbols.New("S"), {
                                                       {mod.symbols.New("a"), ty.i32()},
                                                   });

    b.ir.root_block->Append(b.Var<private_>("a", b.Composite(str_ty, 1_i)));

    auto* src = R"(
S = struct @align(4) {
  a:i32 @offset(0)
}

$B1: {  # root
  %a:ptr<private, S, read_write> = var S(1i)
}

)";
    EXPECT_EQ(src, str());

    auto* expect = R"(
S = struct @align(4) {
  a:i32 @offset(0)
}

$B1: {  # root
  %1:S = construct 1i
  %2:S = let %1
  %a:ptr<private, S, read_write> = var %2
}

)";
    Run(PromoteInitializers);

    EXPECT_EQ(expect, str());
}

TEST_F(HlslWriterPromoteInitializersTest, ModuleScopedStruct_SplatMultipleElements) {
    capabilities = core::ir::Capabilities{core::ir::Capability::kAllowModuleScopeLets};

    auto* str_ty = ty.Struct(mod.symbols.New("S"), {
                                                       {mod.symbols.New("a"), ty.i32()},
                                                       {mod.symbols.New("b"), ty.i32()},
                                                       {mod.symbols.New("c"), ty.i32()},
                                                   });

    b.ir.root_block->Append(b.Var<private_>("a", b.Splat(str_ty, 1_i)));

    auto* src = R"(
S = struct @align(4) {
  a:i32 @offset(0)
  b:i32 @offset(4)
  c:i32 @offset(8)
}

$B1: {  # root
  %a:ptr<private, S, read_write> = var S(1i)
}

)";
    EXPECT_EQ(src, str());

    auto* expect = R"(
S = struct @align(4) {
  a:i32 @offset(0)
  b:i32 @offset(4)
  c:i32 @offset(8)
}

$B1: {  # root
  %1:S = construct 1i, 1i, 1i
  %2:S = let %1
  %a:ptr<private, S, read_write> = var %2
}

)";
    Run(PromoteInitializers);

    EXPECT_EQ(expect, str());
}

TEST_F(HlslWriterPromoteInitializersTest, ModuleScopedArray) {
    capabilities = core::ir::Capabilities{core::ir::Capability::kAllowModuleScopeLets};

    b.ir.root_block->Append(b.Var<private_>("a", b.Zero<array<i32, 2>>()));

    auto* src = R"(
$B1: {  # root
  %a:ptr<private, array<i32, 2>, read_write> = var array<i32, 2>(0i)
}

)";
    EXPECT_EQ(src, str());

    auto* expect = R"(
$B1: {  # root
  %1:array<i32, 2> = let array<i32, 2>(0i)
  %a:ptr<private, array<i32, 2>, read_write> = var %1
}

)";
    Run(PromoteInitializers);

    EXPECT_EQ(expect, str());
}

TEST_F(HlslWriterPromoteInitializersTest, ModuleScopedStructNested) {
    capabilities = core::ir::Capabilities{core::ir::Capability::kAllowModuleScopeLets};

    auto* b_ty = ty.Struct(mod.symbols.New("B"), {
                                                     {mod.symbols.New("c"), ty.f32()},
                                                 });

    auto* a_ty = ty.Struct(mod.symbols.New("A"), {
                                                     {mod.symbols.New("z"), ty.i32()},
                                                     {mod.symbols.New("b"), b_ty},
                                                 });

    auto* str_ty = ty.Struct(mod.symbols.New("S"), {
                                                       {mod.symbols.New("a"), a_ty},
                                                   });

    b.ir.root_block->Append(
        b.Var<private_>("a", b.Composite(str_ty, b.Composite(a_ty, 1_i, b.Composite(b_ty, 1_f)))));

    auto* src = R"(
B = struct @align(4) {
  c:f32 @offset(0)
}

A = struct @align(4) {
  z:i32 @offset(0)
  b:B @offset(4)
}

S = struct @align(4) {
  a:A @offset(0)
}

$B1: {  # root
  %a:ptr<private, S, read_write> = var S(A(1i, B(1.0f)))
}

)";
    EXPECT_EQ(src, str());

    auto* expect = R"(
B = struct @align(4) {
  c:f32 @offset(0)
}

A = struct @align(4) {
  z:i32 @offset(0)
  b:B @offset(4)
}

S = struct @align(4) {
  a:A @offset(0)
}

$B1: {  # root
  %1:B = construct 1.0f
  %2:B = let %1
  %3:A = construct 1i, %2
  %4:A = let %3
  %5:S = construct %4
  %6:S = let %5
  %a:ptr<private, S, read_write> = var %6
}

)";
    Run(PromoteInitializers);

    EXPECT_EQ(expect, str());
}

TEST_F(HlslWriterPromoteInitializersTest, ModuleScopedArrayNestedInStruct) {
    capabilities = core::ir::Capabilities{core::ir::Capability::kAllowModuleScopeLets};

    auto* str_ty = ty.Struct(mod.symbols.New("S"), {
                                                       {mod.symbols.New("a"), ty.array<i32, 3>()},
                                                   });

    b.ir.root_block->Append(b.Var<private_>("a", b.Composite(str_ty, b.Zero(ty.array<i32, 3>()))));

    auto* src = R"(
S = struct @align(4) {
  a:array<i32, 3> @offset(0)
}

$B1: {  # root
  %a:ptr<private, S, read_write> = var S(array<i32, 3>(0i))
}

)";
    EXPECT_EQ(src, str());

    auto* expect = R"(
S = struct @align(4) {
  a:array<i32, 3> @offset(0)
}

$B1: {  # root
  %1:S = construct array<i32, 3>(0i)
  %2:S = let %1
  %a:ptr<private, S, read_write> = var %2
}

)";
    Run(PromoteInitializers);

    EXPECT_EQ(expect, str());
}

TEST_F(HlslWriterPromoteInitializersTest, Many) {
    capabilities = core::ir::Capabilities{core::ir::Capability::kAllowModuleScopeLets};

    auto* a_ty = ty.Struct(mod.symbols.New("A"), {
                                                     {mod.symbols.New("a"), ty.array<i32, 2>()},
                                                 });
    auto* b_ty =
        ty.Struct(mod.symbols.New("B"), {
                                            {mod.symbols.New("b"), ty.array<array<i32, 4>, 1>()},
                                        });
    auto* c_ty = ty.Struct(mod.symbols.New("C"), {
                                                     {mod.symbols.New("a"), a_ty},
                                                 });

    b.Append(b.ir.root_block, [&] {
        b.Var<private_>("a", b.Composite(a_ty, b.Composite(ty.array<i32, 2>(), 9_i, 10_i)));
        b.Var<private_>(
            "b",
            b.Composite(b_ty, b.Composite(ty.array<array<i32, 4>, 1>(),
                                          b.Composite(ty.array<i32, 4>(), 5_i, 6_i, 7_i, 8_i))));
        b.Var<private_>(
            "c", b.Composite(c_ty, b.Composite(a_ty, b.Composite(ty.array<i32, 2>(), 1_i, 2_i))));

        b.Var<private_>("d", b.Composite(ty.array<i32, 2>(), 11_i, 12_i));
        b.Var<private_>("e", b.Composite(ty.array<array<array<i32, 3>, 2>, 1>(),
                                         b.Composite(ty.array<array<i32, 3>, 2>(),
                                                     b.Composite(ty.array<i32, 3>(), 1_i, 2_i, 3_i),
                                                     b.Composite(ty.array<i32, 3>(), 4_i, 5_i, 6_i)

                                                         )));
    });

    auto* src = R"(
A = struct @align(4) {
  a:array<i32, 2> @offset(0)
}

B = struct @align(4) {
  b:array<array<i32, 4>, 1> @offset(0)
}

C = struct @align(4) {
  a_1:A @offset(0)
}

$B1: {  # root
  %a:ptr<private, A, read_write> = var A(array<i32, 2>(9i, 10i))
  %b:ptr<private, B, read_write> = var B(array<array<i32, 4>, 1>(array<i32, 4>(5i, 6i, 7i, 8i)))
  %c:ptr<private, C, read_write> = var C(A(array<i32, 2>(1i, 2i)))
  %d:ptr<private, array<i32, 2>, read_write> = var array<i32, 2>(11i, 12i)
  %e:ptr<private, array<array<array<i32, 3>, 2>, 1>, read_write> = var array<array<array<i32, 3>, 2>, 1>(array<array<i32, 3>, 2>(array<i32, 3>(1i, 2i, 3i), array<i32, 3>(4i, 5i, 6i)))
}

)";
    EXPECT_EQ(src, str());

    auto* expect = R"(
A = struct @align(4) {
  a:array<i32, 2> @offset(0)
}

B = struct @align(4) {
  b:array<array<i32, 4>, 1> @offset(0)
}

C = struct @align(4) {
  a_1:A @offset(0)
}

$B1: {  # root
  %1:A = construct array<i32, 2>(9i, 10i)
  %2:A = let %1
  %a:ptr<private, A, read_write> = var %2
  %4:B = construct array<array<i32, 4>, 1>(array<i32, 4>(5i, 6i, 7i, 8i))
  %5:B = let %4
  %b:ptr<private, B, read_write> = var %5
  %7:A = construct array<i32, 2>(1i, 2i)
  %8:A = let %7
  %9:C = construct %8
  %10:C = let %9
  %c:ptr<private, C, read_write> = var %10
  %12:array<i32, 2> = let array<i32, 2>(11i, 12i)
  %d:ptr<private, array<i32, 2>, read_write> = var %12
  %14:array<array<array<i32, 3>, 2>, 1> = let array<array<array<i32, 3>, 2>, 1>(array<array<i32, 3>, 2>(array<i32, 3>(1i, 2i, 3i), array<i32, 3>(4i, 5i, 6i)))
  %e:ptr<private, array<array<array<i32, 3>, 2>, 1>, read_write> = var %14
}

)";
    Run(PromoteInitializers);

    EXPECT_EQ(expect, str());
}

TEST_F(HlslWriterPromoteInitializersTest, DuplicateConstantInLet) {
    capabilities = core::ir::Capabilities{core::ir::Capability::kAllowModuleScopeLets};

    auto* ret_arr = b.Function("ret_arr", ty.array<vec4<i32>, 4>());
    b.Append(ret_arr->Block(), [&] { b.Return(ret_arr, b.Zero<array<vec4<i32>, 4>>()); });

    auto* func = b.Function("foo", ty.void_(), core::ir::Function::PipelineStage::kFragment);
    b.Append(func->Block(), [&] {
        b.Let("src_let", b.Zero<array<vec4<i32>, 4>>());
        b.Return(func);
    });

    auto* src = R"(
%ret_arr = func():array<vec4<i32>, 4> {
  $B1: {
    ret array<vec4<i32>, 4>(vec4<i32>(0i))
  }
}
%foo = @fragment func():void {
  $B2: {
    %src_let:array<vec4<i32>, 4> = let array<vec4<i32>, 4>(vec4<i32>(0i))
    ret
  }
}
)";
    EXPECT_EQ(src, str());

    auto* expect = R"(
%ret_arr = func():array<vec4<i32>, 4> {
  $B1: {
    %2:array<vec4<i32>, 4> = let array<vec4<i32>, 4>(vec4<i32>(0i))
    ret %2
  }
}
%foo = @fragment func():void {
  $B2: {
    %src_let:array<vec4<i32>, 4> = let array<vec4<i32>, 4>(vec4<i32>(0i))
    ret
  }
}
)";
    Run(PromoteInitializers);

    EXPECT_EQ(expect, str());
}

TEST_F(HlslWriterPromoteInitializersTest, DuplicateConstantInBlock) {
    capabilities = core::ir::Capabilities{core::ir::Capability::kAllowModuleScopeLets};

    auto* a_ty = ty.Struct(mod.symbols.New("A"), {
                                                     {mod.symbols.New("a"), ty.i32()},
                                                 });

    auto* param = b.FunctionParam("a", a_ty);
    auto* bar = b.Function("bar", ty.void_());
    bar->SetParams({param});
    bar->Block()->Append(b.Return(bar));

    auto* func = b.Function("foo", ty.void_(), core::ir::Function::PipelineStage::kFragment);
    b.Append(func->Block(), [&] {
        b.Call(bar, b.Composite(a_ty, 1_i));
        b.Call(bar, b.Composite(a_ty, 1_i));
        b.Return(func);
    });

    auto* src = R"(
A = struct @align(4) {
  a:i32 @offset(0)
}

%bar = func(%a:A):void {
  $B1: {
    ret
  }
}
%foo = @fragment func():void {
  $B2: {
    %4:void = call %bar, A(1i)
    %5:void = call %bar, A(1i)
    ret
  }
}
)";
    EXPECT_EQ(src, str());

    auto* expect = R"(
A = struct @align(4) {
  a:i32 @offset(0)
}

%bar = func(%a:A):void {
  $B1: {
    ret
  }
}
%foo = @fragment func():void {
  $B2: {
    %4:A = let A(1i)
    %5:void = call %bar, %4
    %6:void = call %bar, %4
    ret
  }
}
)";
    Run(PromoteInitializers);

    EXPECT_EQ(expect, str());
}

TEST_F(HlslWriterPromoteInitializersTest, DuplicateConstant) {
    capabilities = core::ir::Capabilities{core::ir::Capability::kAllowModuleScopeLets};

    auto* ret_arr = b.Function("ret_arr", ty.array<vec4<i32>, 4>());
    b.Append(ret_arr->Block(), [&] { b.Return(ret_arr, b.Zero<array<vec4<i32>, 4>>()); });

    auto* second_arr = b.Function("second_arr", ty.array<vec4<i32>, 4>());
    b.Append(second_arr->Block(), [&] { b.Return(second_arr, b.Zero<array<vec4<i32>, 4>>()); });

    auto* func = b.Function("foo", ty.void_(), core::ir::Function::PipelineStage::kFragment);
    b.Append(func->Block(), [&] {
        b.Let("src_let", b.Zero<array<vec4<i32>, 4>>());
        b.Return(func);
    });

    auto* src = R"(
%ret_arr = func():array<vec4<i32>, 4> {
  $B1: {
    ret array<vec4<i32>, 4>(vec4<i32>(0i))
  }
}
%second_arr = func():array<vec4<i32>, 4> {
  $B2: {
    ret array<vec4<i32>, 4>(vec4<i32>(0i))
  }
}
%foo = @fragment func():void {
  $B3: {
    %src_let:array<vec4<i32>, 4> = let array<vec4<i32>, 4>(vec4<i32>(0i))
    ret
  }
}
)";
    EXPECT_EQ(src, str());

    auto* expect = R"(
%ret_arr = func():array<vec4<i32>, 4> {
  $B1: {
    %2:array<vec4<i32>, 4> = let array<vec4<i32>, 4>(vec4<i32>(0i))
    ret %2
  }
}
%second_arr = func():array<vec4<i32>, 4> {
  $B2: {
    %4:array<vec4<i32>, 4> = let array<vec4<i32>, 4>(vec4<i32>(0i))
    ret %4
  }
}
%foo = @fragment func():void {
  $B3: {
    %src_let:array<vec4<i32>, 4> = let array<vec4<i32>, 4>(vec4<i32>(0i))
    ret
  }
}
)";
    Run(PromoteInitializers);

    EXPECT_EQ(expect, str());
}

TEST_F(HlslWriterPromoteInitializersTest, DuplicateAccess) {
    capabilities = core::ir::Capabilities{core::ir::Capability::kAllowModuleScopeLets};

    auto* func = b.Function("foo", ty.void_(), core::ir::Function::PipelineStage::kFragment);
    b.Append(func->Block(), [&] {
        auto* ary = b.Construct(ty.array(ty.f32(), 8));
        b.Access(ty.f32(), ary, 0_u);
        b.Access(ty.f32(), ary, 1_u);
        b.Access(ty.f32(), ary, 2_u);
        b.Return(func);
    });

    auto* src = R"(
%foo = @fragment func():void {
  $B1: {
    %2:array<f32, 8> = construct
    %3:f32 = access %2, 0u
    %4:f32 = access %2, 1u
    %5:f32 = access %2, 2u
    ret
  }
}
)";
    EXPECT_EQ(src, str());

    auto* expect = R"(
%foo = @fragment func():void {
  $B1: {
    %2:array<f32, 8> = construct
    %3:array<f32, 8> = let %2
    %4:f32 = access %3, 0u
    %5:f32 = access %3, 1u
    %6:f32 = access %3, 2u
    ret
  }
}
)";

    Run(PromoteInitializers);
    EXPECT_EQ(expect, str());
}

TEST_F(HlslWriterPromoteInitializersTest, DuplicateAccessDifferentFunction) {
    capabilities = core::ir::Capabilities{core::ir::Capability::kAllowModuleScopeLets};

    auto* a = b.Function("a", ty.void_());
    b.Append(a->Block(), [&] {
        auto* ary = b.Splat(ty.array(ty.f32(), 8), 8_f);
        b.Access(ty.f32(), ary, 0_u);
        b.Return(a);
    });

    auto* func = b.Function("foo", ty.void_(), core::ir::Function::PipelineStage::kFragment);
    b.Append(func->Block(), [&] {
        auto* ary = b.Splat(ty.array(ty.f32(), 8), 8_f);
        b.Access(ty.f32(), ary, 0_u);
        b.Return(func);
    });

    auto* src = R"(
%a = func():void {
  $B1: {
    %2:f32 = access array<f32, 8>(8.0f), 0u
    ret
  }
}
%foo = @fragment func():void {
  $B2: {
    %4:f32 = access array<f32, 8>(8.0f), 0u
    ret
  }
}
)";
    EXPECT_EQ(src, str());

    auto* expect = R"(
%a = func():void {
  $B1: {
    %2:array<f32, 8> = let array<f32, 8>(8.0f)
    %3:f32 = access %2, 0u
    ret
  }
}
%foo = @fragment func():void {
  $B2: {
    %5:array<f32, 8> = let array<f32, 8>(8.0f)
    %6:f32 = access %5, 0u
    ret
  }
}
)";

    Run(PromoteInitializers);
    EXPECT_EQ(expect, str());
}

TEST_F(HlslWriterPromoteInitializersTest, DuplicateAccessDifferentScope) {
    capabilities = core::ir::Capabilities{core::ir::Capability::kAllowModuleScopeLets};

    auto* func = b.Function("foo", ty.void_(), core::ir::Function::PipelineStage::kFragment);
    b.Append(func->Block(), [&] {
        auto* ary = b.Construct(ty.array(ty.f32(), 8));

        auto* if_ = b.If(true);
        b.Append(if_->True(), [&] {
            b.Access(ty.f32(), ary, 0_u);
            b.ExitIf(if_);
        });

        b.Access(ty.f32(), ary, 1_u);
        b.Access(ty.f32(), ary, 2_u);
        b.Return(func);
    });

    auto* src = R"(
%foo = @fragment func():void {
  $B1: {
    %2:array<f32, 8> = construct
    if true [t: $B2] {  # if_1
      $B2: {  # true
        %3:f32 = access %2, 0u
        exit_if  # if_1
      }
    }
    %4:f32 = access %2, 1u
    %5:f32 = access %2, 2u
    ret
  }
}
)";
    EXPECT_EQ(src, str());

    auto* expect = R"(
%foo = @fragment func():void {
  $B1: {
    %2:array<f32, 8> = construct
    if true [t: $B2] {  # if_1
      $B2: {  # true
        %3:array<f32, 8> = let %2
        %4:f32 = access %3, 0u
        exit_if  # if_1
      }
    }
    %5:array<f32, 8> = let %2
    %6:f32 = access %5, 1u
    %7:f32 = access %5, 2u
    ret
  }
}
)";

    Run(PromoteInitializers);
    EXPECT_EQ(expect, str());
}

TEST_F(HlslWriterPromoteInitializersTest, LetOfLet) {
    capabilities = core::ir::Capabilities{core::ir::Capability::kAllowModuleScopeLets};

    auto* str_ty = ty.Struct(mod.symbols.New("S"), {
                                                       {mod.symbols.New("a"), ty.vec4<i32>()},
                                                   });

    auto* inner = b.Function("inner", str_ty);
    b.Append(inner->Block(),
             [&] { b.Return(inner, b.Construct(str_ty, b.Splat(ty.vec4<i32>(), 1_i))); });

    auto* func = b.Function("foo", ty.void_(), core::ir::Function::PipelineStage::kFragment);
    b.Append(func->Block(), [&] {
        auto* in = b.Call(inner);
        auto* l = b.Let("a", in);
        b.Access(ty.vec4<i32>(), l, 0_u);
        b.Return(func);
    });

    auto* src = R"(
S = struct @align(16) {
  a:vec4<i32> @offset(0)
}

%inner = func():S {
  $B1: {
    %2:S = construct vec4<i32>(1i)
    ret %2
  }
}
%foo = @fragment func():void {
  $B2: {
    %4:S = call %inner
    %a:S = let %4
    %6:vec4<i32> = access %a, 0u
    ret
  }
}
)";
    EXPECT_EQ(src, str());

    auto* expect = R"(
S = struct @align(16) {
  a:vec4<i32> @offset(0)
}

%inner = func():S {
  $B1: {
    %2:S = construct vec4<i32>(1i)
    %3:S = let %2
    ret %3
  }
}
%foo = @fragment func():void {
  $B2: {
    %5:S = call %inner
    %a:S = let %5
    %7:vec4<i32> = access %a, 0u
    ret
  }
}
)";

    Run(PromoteInitializers);
    EXPECT_EQ(expect, str());
}

}  // namespace
}  // namespace tint::hlsl::writer::raise
