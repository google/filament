// Copyright 2025 The Dawn & Tint Authors
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

#include "src/tint/lang/core/ir/transform/change_immediate_to_uniform.h"

#include <gtest/gtest.h>

#include "src/tint/lang/core/fluent_types.h"
#include "src/tint/lang/core/ir/function.h"
#include "src/tint/lang/core/ir/transform/helper_test.h"
#include "src/tint/lang/core/number.h"

using namespace tint::core::fluent_types;     // NOLINT
using namespace tint::core::number_suffixes;  // NOLINT

namespace tint::core::ir::transform {
namespace {

using IRChangeImmediateToUniformTest = core::ir::transform::TransformTest;

TEST_F(IRChangeImmediateToUniformTest, NoPushConstantVariable) {
    auto* func = b.Function("foo", ty.void_(), core::ir::Function::PipelineStage::kFragment);
    b.Append(func->Block(), [&] { b.Return(func); });

    auto* src = R"(
%foo = @fragment func():void {
  $B1: {
    ret
  }
}
)";
    EXPECT_EQ(src, str());

    auto* expect = src;

    ChangeImmediateToUniformConfig config = {};
    Run(ChangeImmediateToUniform, config);
    EXPECT_EQ(expect, str());
}

TEST_F(IRChangeImmediateToUniformTest, PushConstantAccessChain) {
    auto* Inner = ty.Struct(mod.symbols.New("Inner"), {
                                                          {mod.symbols.New("c"), ty.f32()},
                                                          {mod.symbols.New("d"), ty.u32()},
                                                      });

    tint::Vector<const core::type::StructMember*, 2> members;
    members.Push(ty.Get<core::type::StructMember>(mod.symbols.New("a"), ty.i32(), 0u, 0u, 4u,
                                                  ty.i32()->Size(), core::IOAttributes{}));
    members.Push(ty.Get<core::type::StructMember>(mod.symbols.New("b"), Inner, 1u, 16u, 16u,
                                                  Inner->Size(), core::IOAttributes{}));
    auto* sb = ty.Struct(mod.symbols.New("SB"), members);

    auto* var = b.Var("v", immediate, sb, core::Access::kRead);
    b.ir.root_block->Append(var);

    auto* func = b.Function("foo", ty.void_(), core::ir::Function::PipelineStage::kFragment);
    b.Append(func->Block(), [&] {
        auto* x = b.Access(ty.ptr(immediate, Inner, core::Access::kRead), var, 1_u);
        b.Let("b",
              b.Load(b.Access(ty.ptr(immediate, ty.u32(), core::Access::kRead), x->Result(), 1_u)));
        b.Return(func);
    });

    auto* src = R"(
Inner = struct @align(4) {
  c:f32 @offset(0)
  d:u32 @offset(4)
}

SB = struct @align(16) {
  a:i32 @offset(0)
  b:Inner @offset(16)
}

$B1: {  # root
  %v:ptr<immediate, SB, read> = var undef
}

%foo = @fragment func():void {
  $B2: {
    %3:ptr<immediate, Inner, read> = access %v, 1u
    %4:ptr<immediate, u32, read> = access %3, 1u
    %5:u32 = load %4
    %b:u32 = let %5
    ret
  }
}
)";
    ASSERT_EQ(src, str());

    auto* expect = R"(
Inner = struct @align(4) {
  c:f32 @offset(0)
  d:u32 @offset(4)
}

SB = struct @align(16) {
  a:i32 @offset(0)
  b:Inner @offset(16)
}

$B1: {  # root
  %v:ptr<uniform, SB, read> = var undef @binding_point(0, 0)
}

%foo = @fragment func():void {
  $B2: {
    %3:ptr<uniform, Inner, read> = access %v, 1u
    %4:ptr<uniform, u32, read> = access %3, 1u
    %5:u32 = load %4
    %b:u32 = let %5
    ret
  }
}
)";
    ChangeImmediateToUniformConfig config = {};
    Run(ChangeImmediateToUniform, config);
    EXPECT_EQ(expect, str());
}

TEST_F(IRChangeImmediateToUniformTest, PushConstantAccessChainFromLetAccessChain) {
    auto* Inner = ty.Struct(mod.symbols.New("Inner"), {
                                                          {mod.symbols.New("c"), ty.f32()},
                                                      });

    tint::Vector<const core::type::StructMember*, 2> members;
    members.Push(ty.Get<core::type::StructMember>(mod.symbols.New("a"), ty.i32(), 0u, 0u, 4u,
                                                  ty.i32()->Size(), core::IOAttributes{}));
    members.Push(ty.Get<core::type::StructMember>(mod.symbols.New("b"), Inner, 1u, 16u, 16u,
                                                  Inner->Size(), core::IOAttributes{}));
    auto* sb = ty.Struct(mod.symbols.New("SB"), members);

    auto* var = b.Var("v", immediate, sb, core::Access::kRead);
    b.ir.root_block->Append(var);

    auto* func = b.Function("foo", ty.void_(), core::ir::Function::PipelineStage::kFragment);
    b.Append(func->Block(), [&] {
        auto* x = b.Let("x", var);
        auto* y =
            b.Let("y", b.Access(ty.ptr(immediate, Inner, core::Access::kRead), x->Result(), 1_u));
        auto* z = b.Let(
            "z", b.Access(ty.ptr(immediate, ty.f32(), core::Access::kRead), y->Result(), 0_u));
        b.Let("a", b.Load(z));
        b.Return(func);
    });

    auto* src = R"(
Inner = struct @align(4) {
  c:f32 @offset(0)
}

SB = struct @align(16) {
  a:i32 @offset(0)
  b:Inner @offset(16)
}

$B1: {  # root
  %v:ptr<immediate, SB, read> = var undef
}

%foo = @fragment func():void {
  $B2: {
    %x:ptr<immediate, SB, read> = let %v
    %4:ptr<immediate, Inner, read> = access %x, 1u
    %y:ptr<immediate, Inner, read> = let %4
    %6:ptr<immediate, f32, read> = access %y, 0u
    %z:ptr<immediate, f32, read> = let %6
    %8:f32 = load %z
    %a:f32 = let %8
    ret
  }
}
)";
    ASSERT_EQ(src, str());

    auto* expect = R"(
Inner = struct @align(4) {
  c:f32 @offset(0)
}

SB = struct @align(16) {
  a:i32 @offset(0)
  b:Inner @offset(16)
}

$B1: {  # root
  %v:ptr<uniform, SB, read> = var undef @binding_point(0, 0)
}

%foo = @fragment func():void {
  $B2: {
    %x:ptr<uniform, SB, read> = let %v
    %4:ptr<uniform, Inner, read> = access %x, 1u
    %y:ptr<uniform, Inner, read> = let %4
    %6:ptr<uniform, f32, read> = access %y, 0u
    %z:ptr<uniform, f32, read> = let %6
    %8:f32 = load %z
    %a:f32 = let %8
    ret
  }
}
)";
    ChangeImmediateToUniformConfig config = {};
    Run(ChangeImmediateToUniform, config);
    EXPECT_EQ(expect, str());
}

TEST_F(IRChangeImmediateToUniformTest, PushConstantAccessVectorLoad) {
    auto* var = b.Var<immediate, vec4<f32>, core::Access::kRead>("v");

    b.ir.root_block->Append(var);
    auto* func = b.Function("foo", ty.void_(), core::ir::Function::PipelineStage::kFragment);
    b.Append(func->Block(), [&] {
        b.Let("a", b.Load(var));
        b.Let("b", b.LoadVectorElement(var, 0_u));
        b.Let("c", b.LoadVectorElement(var, 1_u));
        b.Let("d", b.LoadVectorElement(var, 2_u));
        b.Let("e", b.LoadVectorElement(var, 3_u));
        b.Return(func);
    });

    auto* src = R"(
$B1: {  # root
  %v:ptr<immediate, vec4<f32>, read> = var undef
}

%foo = @fragment func():void {
  $B2: {
    %3:vec4<f32> = load %v
    %a:vec4<f32> = let %3
    %5:f32 = load_vector_element %v, 0u
    %b:f32 = let %5
    %7:f32 = load_vector_element %v, 1u
    %c:f32 = let %7
    %9:f32 = load_vector_element %v, 2u
    %d:f32 = let %9
    %11:f32 = load_vector_element %v, 3u
    %e:f32 = let %11
    ret
  }
}
)";
    ASSERT_EQ(src, str());

    auto* expect = R"(
$B1: {  # root
  %v:ptr<uniform, vec4<f32>, read> = var undef @binding_point(0, 0)
}

%foo = @fragment func():void {
  $B2: {
    %3:vec4<f32> = load %v
    %a:vec4<f32> = let %3
    %5:f32 = load_vector_element %v, 0u
    %b:f32 = let %5
    %7:f32 = load_vector_element %v, 1u
    %c:f32 = let %7
    %9:f32 = load_vector_element %v, 2u
    %d:f32 = let %9
    %11:f32 = load_vector_element %v, 3u
    %e:f32 = let %11
    ret
  }
}
)";
    ChangeImmediateToUniformConfig config = {};
    Run(ChangeImmediateToUniform, config);
    EXPECT_EQ(expect, str());
}

TEST_F(IRChangeImmediateToUniformTest, PushConstantAccessMatrix) {
    auto* var = b.Var<immediate, mat4x4<f32>, core::Access::kRead>("v");

    b.ir.root_block->Append(var);
    auto* func = b.Function("foo", ty.void_(), core::ir::Function::PipelineStage::kFragment);
    b.Append(func->Block(), [&] {
        b.Let("a", b.Load(var));
        b.Let("b", b.Load(b.Access(ty.ptr<immediate, vec4<f32>, core::Access::kRead>(), var, 3_u)));
        b.Let("c",
              b.LoadVectorElement(
                  b.Access(ty.ptr<immediate, vec4<f32>, core::Access::kRead>(), var, 1_u), 2_u));
        b.Return(func);
    });

    auto* src = R"(
$B1: {  # root
  %v:ptr<immediate, mat4x4<f32>, read> = var undef
}

%foo = @fragment func():void {
  $B2: {
    %3:mat4x4<f32> = load %v
    %a:mat4x4<f32> = let %3
    %5:ptr<immediate, vec4<f32>, read> = access %v, 3u
    %6:vec4<f32> = load %5
    %b:vec4<f32> = let %6
    %8:ptr<immediate, vec4<f32>, read> = access %v, 1u
    %9:f32 = load_vector_element %8, 2u
    %c:f32 = let %9
    ret
  }
}
)";
    ASSERT_EQ(src, str());

    auto* expect = R"(
$B1: {  # root
  %v:ptr<uniform, mat4x4<f32>, read> = var undef @binding_point(0, 0)
}

%foo = @fragment func():void {
  $B2: {
    %3:mat4x4<f32> = load %v
    %a:mat4x4<f32> = let %3
    %5:ptr<uniform, vec4<f32>, read> = access %v, 3u
    %6:vec4<f32> = load %5
    %b:vec4<f32> = let %6
    %8:ptr<uniform, vec4<f32>, read> = access %v, 1u
    %9:f32 = load_vector_element %8, 2u
    %c:f32 = let %9
    ret
  }
}
)";
    ChangeImmediateToUniformConfig config = {};
    Run(ChangeImmediateToUniform, config);
    EXPECT_EQ(expect, str());
}

TEST_F(IRChangeImmediateToUniformTest, PushConstantAccessStruct) {
    auto* SB = ty.Struct(mod.symbols.New("SB"), {
                                                    {mod.symbols.New("a"), ty.i32()},
                                                    {mod.symbols.New("b"), ty.f32()},
                                                });

    auto* var = b.Var("v", immediate, SB, core::Access::kRead);

    b.ir.root_block->Append(var);
    auto* func = b.Function("foo", ty.void_(), core::ir::Function::PipelineStage::kFragment);
    b.Append(func->Block(), [&] {
        b.Let("a", b.Load(var));
        b.Let("b", b.Load(b.Access(ty.ptr<immediate, f32, core::Access::kRead>(), var, 1_u)));
        b.Return(func);
    });

    auto* src = R"(
SB = struct @align(4) {
  a:i32 @offset(0)
  b:f32 @offset(4)
}

$B1: {  # root
  %v:ptr<immediate, SB, read> = var undef
}

%foo = @fragment func():void {
  $B2: {
    %3:SB = load %v
    %a:SB = let %3
    %5:ptr<immediate, f32, read> = access %v, 1u
    %6:f32 = load %5
    %b:f32 = let %6
    ret
  }
}
)";
    ASSERT_EQ(src, str());

    auto* expect = R"(
SB = struct @align(4) {
  a:i32 @offset(0)
  b:f32 @offset(4)
}

$B1: {  # root
  %v:ptr<uniform, SB, read> = var undef @binding_point(0, 0)
}

%foo = @fragment func():void {
  $B2: {
    %3:SB = load %v
    %a:SB = let %3
    %5:ptr<uniform, f32, read> = access %v, 1u
    %6:f32 = load %5
    %b:f32 = let %6
    ret
  }
}
)";
    ChangeImmediateToUniformConfig config = {};
    Run(ChangeImmediateToUniform, config);
    EXPECT_EQ(expect, str());
}

TEST_F(IRChangeImmediateToUniformTest, PushConstantStructNested) {
    auto* Inner =
        ty.Struct(mod.symbols.New("Inner"), {
                                                {mod.symbols.New("s"), ty.mat3x3<f32>()},
                                                {mod.symbols.New("t"), ty.array<vec3<f32>, 5>()},
                                            });
    auto* Outer = ty.Struct(mod.symbols.New("Outer"), {
                                                          {mod.symbols.New("x"), ty.f32()},
                                                          {mod.symbols.New("y"), Inner},
                                                      });

    auto* SB = ty.Struct(mod.symbols.New("SB"), {
                                                    {mod.symbols.New("a"), ty.i32()},
                                                    {mod.symbols.New("b"), Outer},
                                                });

    auto* var = b.Var("v", immediate, SB, core::Access::kRead);

    b.ir.root_block->Append(var);
    auto* func = b.Function("foo", ty.void_(), core::ir::Function::PipelineStage::kFragment);
    b.Append(func->Block(), [&] {
        b.Let("a", b.Load(var));
        b.Let("b", b.LoadVectorElement(b.Access(ty.ptr<immediate, vec3<f32>, core::Access::kRead>(),
                                                var, 1_u, 1_u, 1_u, 3_u),
                                       2_u));
        b.Return(func);
    });

    auto* src = R"(
Inner = struct @align(16) {
  s:mat3x3<f32> @offset(0)
  t:array<vec3<f32>, 5> @offset(48)
}

Outer = struct @align(16) {
  x:f32 @offset(0)
  y:Inner @offset(16)
}

SB = struct @align(16) {
  a:i32 @offset(0)
  b:Outer @offset(16)
}

$B1: {  # root
  %v:ptr<immediate, SB, read> = var undef
}

%foo = @fragment func():void {
  $B2: {
    %3:SB = load %v
    %a:SB = let %3
    %5:ptr<immediate, vec3<f32>, read> = access %v, 1u, 1u, 1u, 3u
    %6:f32 = load_vector_element %5, 2u
    %b:f32 = let %6
    ret
  }
}
)";
    ASSERT_EQ(src, str());

    auto* expect = R"(
Inner = struct @align(16) {
  s:mat3x3<f32> @offset(0)
  t:array<vec3<f32>, 5> @offset(48)
}

Outer = struct @align(16) {
  x:f32 @offset(0)
  y:Inner @offset(16)
}

SB = struct @align(16) {
  a:i32 @offset(0)
  b:Outer @offset(16)
}

$B1: {  # root
  %v:ptr<uniform, SB, read> = var undef @binding_point(0, 0)
}

%foo = @fragment func():void {
  $B2: {
    %3:SB = load %v
    %a:SB = let %3
    %5:ptr<uniform, vec3<f32>, read> = access %v, 1u, 1u, 1u, 3u
    %6:f32 = load_vector_element %5, 2u
    %b:f32 = let %6
    ret
  }
}
)";
    ChangeImmediateToUniformConfig config = {};
    Run(ChangeImmediateToUniform, config);
    EXPECT_EQ(expect, str());
}

TEST_F(IRChangeImmediateToUniformTest, PushConstantAccessChainReused) {
    auto* sb = ty.Struct(mod.symbols.New("SB"), {
                                                    {mod.symbols.New("c"), ty.f32()},
                                                    {mod.symbols.New("d"), ty.vec3<f32>()},
                                                });

    auto* var = b.Var("v", immediate, sb, core::Access::kRead);
    b.ir.root_block->Append(var);

    auto* func = b.Function("foo", ty.void_(), core::ir::Function::PipelineStage::kFragment);
    b.Append(func->Block(), [&] {
        auto* x = b.Access(ty.ptr(immediate, ty.vec3<f32>(), core::Access::kRead), var, 1_u);
        b.Let("b", b.LoadVectorElement(x, 1_u));
        b.Let("c", b.LoadVectorElement(x, 2_u));
        b.Return(func);
    });

    auto* src = R"(
SB = struct @align(16) {
  c:f32 @offset(0)
  d:vec3<f32> @offset(16)
}

$B1: {  # root
  %v:ptr<immediate, SB, read> = var undef
}

%foo = @fragment func():void {
  $B2: {
    %3:ptr<immediate, vec3<f32>, read> = access %v, 1u
    %4:f32 = load_vector_element %3, 1u
    %b:f32 = let %4
    %6:f32 = load_vector_element %3, 2u
    %c:f32 = let %6
    ret
  }
}
)";
    ASSERT_EQ(src, str());

    auto* expect = R"(
SB = struct @align(16) {
  c:f32 @offset(0)
  d:vec3<f32> @offset(16)
}

$B1: {  # root
  %v:ptr<uniform, SB, read> = var undef @binding_point(0, 0)
}

%foo = @fragment func():void {
  $B2: {
    %3:ptr<uniform, vec3<f32>, read> = access %v, 1u
    %4:f32 = load_vector_element %3, 1u
    %b:f32 = let %4
    %6:f32 = load_vector_element %3, 2u
    %c:f32 = let %6
    ret
  }
}
)";

    ChangeImmediateToUniformConfig config = {};
    Run(ChangeImmediateToUniform, config);
    EXPECT_EQ(expect, str());
}

TEST_F(IRChangeImmediateToUniformTest, Determinism_MultipleUsesOfLetFromVar) {
    auto* sb =
        ty.Struct(mod.symbols.New("SB"), {
                                             {mod.symbols.New("a"), ty.array<vec4<f32>, 2>()},
                                             {mod.symbols.New("b"), ty.array<vec4<i32>, 2>()},
                                         });

    auto* var = b.Var("v", immediate, sb, core::Access::kRead);
    b.ir.root_block->Append(var);

    auto* func = b.Function("foo", ty.void_(), core::ir::Function::PipelineStage::kFragment);
    b.Append(func->Block(), [&] {
        auto* let = b.Let("l", var);
        auto* pa =
            b.Access(ty.ptr(immediate, ty.array<vec4<f32>, 2>(), core::Access::kRead), let, 0_u);
        b.Let("a", b.Load(pa));
        auto* pb =
            b.Access(ty.ptr(immediate, ty.array<vec4<i32>, 2>(), core::Access::kRead), let, 1_u);
        b.Let("b", b.Load(pb));
        b.Return(func);
    });

    auto* src = R"(
SB = struct @align(16) {
  a:array<vec4<f32>, 2> @offset(0)
  b:array<vec4<i32>, 2> @offset(32)
}

$B1: {  # root
  %v:ptr<immediate, SB, read> = var undef
}

%foo = @fragment func():void {
  $B2: {
    %l:ptr<immediate, SB, read> = let %v
    %4:ptr<immediate, array<vec4<f32>, 2>, read> = access %l, 0u
    %5:array<vec4<f32>, 2> = load %4
    %a:array<vec4<f32>, 2> = let %5
    %7:ptr<immediate, array<vec4<i32>, 2>, read> = access %l, 1u
    %8:array<vec4<i32>, 2> = load %7
    %b:array<vec4<i32>, 2> = let %8
    ret
  }
}
)";
    ASSERT_EQ(src, str());

    auto* expect = R"(
SB = struct @align(16) {
  a:array<vec4<f32>, 2> @offset(0)
  b:array<vec4<i32>, 2> @offset(32)
}

$B1: {  # root
  %v:ptr<uniform, SB, read> = var undef @binding_point(0, 0)
}

%foo = @fragment func():void {
  $B2: {
    %l:ptr<uniform, SB, read> = let %v
    %4:ptr<uniform, array<vec4<f32>, 2>, read> = access %l, 0u
    %5:array<vec4<f32>, 2> = load %4
    %a:array<vec4<f32>, 2> = let %5
    %7:ptr<uniform, array<vec4<i32>, 2>, read> = access %l, 1u
    %8:array<vec4<i32>, 2> = load %7
    %b:array<vec4<i32>, 2> = let %8
    ret
  }
}
)";

    ChangeImmediateToUniformConfig config = {};
    Run(ChangeImmediateToUniform, config);
    EXPECT_EQ(expect, str());
}

}  // namespace
}  // namespace tint::core::ir::transform
