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

#include "src/tint/lang/core/ir/transform/decompose_access.h"

#include <gtest/gtest.h>

#include "src/tint/lang/core/fluent_types.h"
#include "src/tint/lang/core/ir/function.h"
#include "src/tint/lang/core/ir/transform/helper_test.h"
#include "src/tint/lang/core/number.h"

using namespace tint::core::fluent_types;     // NOLINT
using namespace tint::core::number_suffixes;  // NOLINT

namespace tint::core::ir::transform {
namespace {

struct IR_DecomposeAccessTest : public core::ir::transform::TransformTest {
    void SetUp() override {
        mod.properties.Add(Property::kAllow16BitFloats);
        mod.properties.Add(Property::kAllow16BitIntegers);
        mod.properties.Add(Property::kAllowBufferTypes);
    }
};

TEST_F(IR_DecomposeAccessTest, OverflowArraySize) {
    auto* S =
        ty.Struct(mod.symbols.New("S"),
                  {
                      {mod.symbols.New("a"), ty.array(ty.array(ty.mat3x2(ty.f32()), 3235), 55319)},
                      {mod.symbols.New("b"), ty.array(ty.mat3x2(ty.f32()), 5)},
                      {mod.symbols.New("c"), ty.u32()},
                  });

    auto* v = b.Var("v", ty.ptr(uniform, S));
    v->SetBindingPoint(0, 0);
    mod.root_block->Append(v);

    auto* foo = b.Function("foo", ty.void_());
    b.Append(foo->Block(), [&] {
        auto* access = b.Access(ty.ptr(uniform, ty.u32()), v, 2_u);
        b.Load(access);
        b.Return(foo);
    });

    auto* src = R"(
S = struct @align(8) {
  a:array<array<mat3x2<f32>, 3235>, 55319> @offset(0)
  b:array<mat3x2<f32>, 5> @offset(4294967160)
  c:u32 @offset(4294967280)
}

$B1: {  # root
  %v:ptr<uniform, S, read> = var undef @binding_point(0, 0)
}

%foo = func():void {
  $B2: {
    %3:ptr<uniform, u32, read> = access %v, 2u
    %4:u32 = load %3
    ret
  }
}
)";

    EXPECT_EQ(src, str());

    auto* expect = R"(
S = struct @align(8) {
  a:array<array<mat3x2<f32>, 3235>, 55319> @offset(0)
  b:array<mat3x2<f32>, 5> @offset(4294967160)
  c:u32 @offset(4294967280)
}

$B1: {  # root
  %v:ptr<uniform, array<vec4<u32>, 268435456>, read> = var undef @binding_point(0, 0)
}

%foo = func():void {
  $B2: {
    %3:ptr<uniform, vec4<u32>, read> = access %v, 268435455u
    %4:u32 = load_vector_element %3, 0u
    ret
  }
}
)";

    DecomposeAccessConfig options{.uniform = true};
    Run(DecomposeAccess, options);

    EXPECT_EQ(expect, str());
}

TEST_F(IR_DecomposeAccessTest, NoBufferAccess) {
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
    DecomposeAccessConfig options{.uniform = true};
    Run(DecomposeAccess, options);

    EXPECT_EQ(expect, str());
}

TEST_F(IR_DecomposeAccessTest, UniformAccessChainFromUnnamedAccessChain) {
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

    auto* var = b.Var("v", uniform, ty.array(sb, 4), core::Access::kRead);
    var->SetBindingPoint(0, 0);
    b.ir.root_block->Append(var);

    auto* func = b.Function("foo", ty.void_(), core::ir::Function::PipelineStage::kFragment);
    b.Append(func->Block(), [&] {
        auto* x = b.Access(ty.ptr(uniform, sb, core::Access::kRead), var, 2_u);
        auto* y = b.Access(ty.ptr(uniform, Inner, core::Access::kRead), x->Result(), 1_u);
        b.Let("b",
              b.Load(b.Access(ty.ptr(uniform, ty.u32(), core::Access::kRead), y->Result(), 1_u)));
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
  %v:ptr<uniform, array<SB, 4>, read> = var undef @binding_point(0, 0)
}

%foo = @fragment func():void {
  $B2: {
    %3:ptr<uniform, SB, read> = access %v, 2u
    %4:ptr<uniform, Inner, read> = access %3, 1u
    %5:ptr<uniform, u32, read> = access %4, 1u
    %6:u32 = load %5
    %b:u32 = let %6
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
  %v:ptr<uniform, array<vec4<u32>, 8>, read> = var undef @binding_point(0, 0)
}

%foo = @fragment func():void {
  $B2: {
    %3:ptr<uniform, vec4<u32>, read> = access %v, 5u
    %4:u32 = load_vector_element %3, 1u
    %b:u32 = let %4
    ret
  }
}
)";

    DecomposeAccessConfig options{.uniform = true};
    Run(DecomposeAccess, options);
    EXPECT_EQ(expect, str());
}

TEST_F(IR_DecomposeAccessTest, UniformAccessChainFromLetAccessChain) {
    auto* Inner = ty.Struct(mod.symbols.New("Inner"), {
                                                          {mod.symbols.New("c"), ty.f32()},
                                                      });

    tint::Vector<const core::type::StructMember*, 2> members;
    members.Push(ty.Get<core::type::StructMember>(mod.symbols.New("a"), ty.i32(), 0u, 0u, 4u,
                                                  ty.i32()->Size(), core::IOAttributes{}));
    members.Push(ty.Get<core::type::StructMember>(mod.symbols.New("b"), Inner, 1u, 16u, 16u,
                                                  Inner->Size(), core::IOAttributes{}));
    auto* sb = ty.Struct(mod.symbols.New("SB"), members);

    auto* var = b.Var("v", uniform, sb, core::Access::kRead);
    var->SetBindingPoint(0, 0);
    b.ir.root_block->Append(var);

    auto* func = b.Function("foo", ty.void_(), core::ir::Function::PipelineStage::kFragment);
    b.Append(func->Block(), [&] {
        auto* x = b.Let("x", var);
        auto* y =
            b.Let("y", b.Access(ty.ptr(uniform, Inner, core::Access::kRead), x->Result(), 1_u));
        auto* z =
            b.Let("z", b.Access(ty.ptr(uniform, ty.f32(), core::Access::kRead), y->Result(), 0_u));
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
  %v:ptr<uniform, array<vec4<u32>, 2>, read> = var undef @binding_point(0, 0)
}

%foo = @fragment func():void {
  $B2: {
    %3:ptr<uniform, vec4<u32>, read> = access %v, 1u
    %4:u32 = load_vector_element %3, 0u
    %5:f32 = bitcast<f32> %4
    %a:f32 = let %5
    ret
  }
}
)";

    DecomposeAccessConfig options{.uniform = true};
    Run(DecomposeAccess, options);
    EXPECT_EQ(expect, str());
}

TEST_F(IR_DecomposeAccessTest, UniformAccessVectorLoad) {
    auto* var = b.Var<uniform, vec4<f32>, core::Access::kRead>("v");
    var->SetBindingPoint(0, 0);

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
    ASSERT_EQ(src, str());

    auto* expect = R"(
$B1: {  # root
  %v:ptr<uniform, array<vec4<u32>, 1>, read> = var undef @binding_point(0, 0)
}

%foo = @fragment func():void {
  $B2: {
    %3:ptr<uniform, vec4<u32>, read> = access %v, 0u
    %4:vec4<u32> = load %3
    %5:vec4<f32> = bitcast<vec4<f32>> %4
    %a:vec4<f32> = let %5
    %7:ptr<uniform, vec4<u32>, read> = access %v, 0u
    %8:u32 = load_vector_element %7, 0u
    %9:f32 = bitcast<f32> %8
    %b:f32 = let %9
    %11:ptr<uniform, vec4<u32>, read> = access %v, 0u
    %12:u32 = load_vector_element %11, 1u
    %13:f32 = bitcast<f32> %12
    %c:f32 = let %13
    %15:ptr<uniform, vec4<u32>, read> = access %v, 0u
    %16:u32 = load_vector_element %15, 2u
    %17:f32 = bitcast<f32> %16
    %d:f32 = let %17
    %19:ptr<uniform, vec4<u32>, read> = access %v, 0u
    %20:u32 = load_vector_element %19, 3u
    %21:f32 = bitcast<f32> %20
    %e:f32 = let %21
    ret
  }
}
)";
    DecomposeAccessConfig options{.uniform = true};
    Run(DecomposeAccess, options);
    EXPECT_EQ(expect, str());
}

TEST_F(IR_DecomposeAccessTest, UniformAccessScalarF16) {
    auto* var = b.Var<uniform, f16, core::Access::kRead>("v");
    var->SetBindingPoint(0, 0);

    b.ir.root_block->Append(var);
    auto* func = b.Function("foo", ty.void_(), core::ir::Function::PipelineStage::kFragment);
    b.Append(func->Block(), [&] {
        b.Let("a", b.Load(var));
        b.Return(func);
    });

    auto* src = R"(
$B1: {  # root
  %v:ptr<uniform, f16, read> = var undef @binding_point(0, 0)
}

%foo = @fragment func():void {
  $B2: {
    %3:f16 = load %v
    %a:f16 = let %3
    ret
  }
}
)";
    ASSERT_EQ(src, str());

    auto* expect = R"(
$B1: {  # root
  %v:ptr<uniform, array<vec4<u32>, 1>, read> = var undef @binding_point(0, 0)
}

%foo = @fragment func():void {
  $B2: {
    %3:ptr<uniform, vec4<u32>, read> = access %v, 0u
    %4:u32 = load_vector_element %3, 0u
    %5:vec2<f16> = bitcast<vec2<f16>> %4
    %6:f16 = access %5, 0u
    %a:f16 = let %6
    ret
  }
}
)";
    DecomposeAccessConfig options{.uniform = true};
    Run(DecomposeAccess, options);
    EXPECT_EQ(expect, str());
}

TEST_F(IR_DecomposeAccessTest, UniformAccessVectorF16) {
    auto* var = b.Var<uniform, vec4<f16>, core::Access::kRead>("v");
    var->SetBindingPoint(0, 0);

    b.ir.root_block->Append(var);
    auto* func = b.Function("foo", ty.void_(), core::ir::Function::PipelineStage::kFragment);
    b.Append(func->Block(), [&] {
        auto* x = b.Let("x", 1_u);
        b.Let("a", b.Load(var));
        b.Let("b", b.LoadVectorElement(var, 0_u));
        b.Let("c", b.LoadVectorElement(var, x));
        b.Let("d", b.LoadVectorElement(var, 2_u));
        b.Let("e", b.LoadVectorElement(var, 3_u));
        b.Return(func);
    });

    auto* src = R"(
$B1: {  # root
  %v:ptr<uniform, vec4<f16>, read> = var undef @binding_point(0, 0)
}

%foo = @fragment func():void {
  $B2: {
    %x:u32 = let 1u
    %4:vec4<f16> = load %v
    %a:vec4<f16> = let %4
    %6:f16 = load_vector_element %v, 0u
    %b:f16 = let %6
    %8:f16 = load_vector_element %v, %x
    %c:f16 = let %8
    %10:f16 = load_vector_element %v, 2u
    %d:f16 = let %10
    %12:f16 = load_vector_element %v, 3u
    %e:f16 = let %12
    ret
  }
}
)";
    ASSERT_EQ(src, str());

    auto* expect = R"(
$B1: {  # root
  %v:ptr<uniform, array<vec4<u32>, 1>, read> = var undef @binding_point(0, 0)
}

%foo = @fragment func():void {
  $B2: {
    %x:u32 = let 1u
    %4:ptr<uniform, vec4<u32>, read> = access %v, 0u
    %5:vec4<u32> = load %4
    %6:vec2<u32> = swizzle %5, xy
    %7:vec4<f16> = bitcast<vec4<f16>> %6
    %a:vec4<f16> = let %7
    %9:ptr<uniform, vec4<u32>, read> = access %v, 0u
    %10:u32 = load_vector_element %9, 0u
    %11:vec2<f16> = bitcast<vec2<f16>> %10
    %12:f16 = access %11, 0u
    %b:f16 = let %12
    %14:u32 = mul %x, 2u
    %15:u32 = div %14, 16u
    %16:ptr<uniform, vec4<u32>, read> = access %v, %15
    %17:u32 = and %14, 15u
    %18:u32 = shr %17, 2u
    %19:u32 = load_vector_element %16, %18
    %20:u32 = mod %14, 4u
    %21:bool = eq %20, 0u
    %22:u32 = select 1u, 0u, %21
    %23:vec2<f16> = bitcast<vec2<f16>> %19
    %24:f16 = access %23, %22
    %c:f16 = let %24
    %26:ptr<uniform, vec4<u32>, read> = access %v, 0u
    %27:u32 = load_vector_element %26, 1u
    %28:vec2<f16> = bitcast<vec2<f16>> %27
    %29:f16 = access %28, 0u
    %d:f16 = let %29
    %31:ptr<uniform, vec4<u32>, read> = access %v, 0u
    %32:u32 = load_vector_element %31, 1u
    %33:vec2<f16> = bitcast<vec2<f16>> %32
    %34:f16 = access %33, 1u
    %e:f16 = let %34
    ret
  }
}
)";
    DecomposeAccessConfig options{.uniform = true};
    Run(DecomposeAccess, options);
    EXPECT_EQ(expect, str());
}

TEST_F(IR_DecomposeAccessTest, UniformAccessMat2x3F16) {
    auto* var = b.Var<uniform, mat2x3<f16>, core::Access::kRead>("v");
    var->SetBindingPoint(0, 0);

    b.ir.root_block->Append(var);
    auto* func = b.Function("foo", ty.void_(), core::ir::Function::PipelineStage::kFragment);
    b.Append(func->Block(), [&] {
        b.Let("a", b.Load(var));
        b.Let("b", b.Load(b.Access(ty.ptr(uniform, ty.vec3h()), var, 1_u)));
        b.Let("c", b.LoadVectorElement(b.Access(ty.ptr(uniform, ty.vec3h()), var, 1_u), 2_u));
        b.Return(func);
    });

    auto* src = R"(
$B1: {  # root
  %v:ptr<uniform, mat2x3<f16>, read> = var undef @binding_point(0, 0)
}

%foo = @fragment func():void {
  $B2: {
    %3:mat2x3<f16> = load %v
    %a:mat2x3<f16> = let %3
    %5:ptr<uniform, vec3<f16>, read> = access %v, 1u
    %6:vec3<f16> = load %5
    %b:vec3<f16> = let %6
    %8:ptr<uniform, vec3<f16>, read> = access %v, 1u
    %9:f16 = load_vector_element %8, 2u
    %c:f16 = let %9
    ret
  }
}
)";
    ASSERT_EQ(src, str());

    auto* expect = R"(
$B1: {  # root
  %v:ptr<uniform, array<vec4<u32>, 1>, read> = var undef @binding_point(0, 0)
}

%foo = @fragment func():void {
  $B2: {
    %3:mat2x3<f16> = call %4, 0u
    %a:mat2x3<f16> = let %3
    %6:ptr<uniform, vec4<u32>, read> = access %v, 0u
    %7:vec4<u32> = load %6
    %8:vec2<u32> = swizzle %7, zw
    %9:vec4<f16> = bitcast<vec4<f16>> %8
    %10:vec3<f16> = swizzle %9, xyz
    %b:vec3<f16> = let %10
    %12:ptr<uniform, vec4<u32>, read> = access %v, 0u
    %13:u32 = load_vector_element %12, 3u
    %14:vec2<f16> = bitcast<vec2<f16>> %13
    %15:f16 = access %14, 0u
    %c:f16 = let %15
    ret
  }
}
%4 = func(%start_byte_offset:u32):mat2x3<f16> {
  $B3: {
    %18:u32 = div %start_byte_offset, 16u
    %19:ptr<uniform, vec4<u32>, read> = access %v, %18
    %20:vec4<u32> = load %19
    %21:u32 = and %start_byte_offset, 15u
    %22:u32 = shr %21, 2u
    %23:vec2<u32> = swizzle %20, zw
    %24:vec2<u32> = swizzle %20, xy
    %25:bool = eq %22, 2u
    %26:vec2<u32> = select %24, %23, %25
    %27:vec4<f16> = bitcast<vec4<f16>> %26
    %28:vec3<f16> = swizzle %27, xyz
    %29:u32 = add 8u, %start_byte_offset
    %30:u32 = div %29, 16u
    %31:ptr<uniform, vec4<u32>, read> = access %v, %30
    %32:vec4<u32> = load %31
    %33:u32 = and %29, 15u
    %34:u32 = shr %33, 2u
    %35:vec2<u32> = swizzle %32, zw
    %36:vec2<u32> = swizzle %32, xy
    %37:bool = eq %34, 2u
    %38:vec2<u32> = select %36, %35, %37
    %39:vec4<f16> = bitcast<vec4<f16>> %38
    %40:vec3<f16> = swizzle %39, xyz
    %41:mat2x3<f16> = construct %28, %40
    ret %41
  }
}
)";
    DecomposeAccessConfig options{.uniform = true};
    Run(DecomposeAccess, options);
    EXPECT_EQ(expect, str());
}

TEST_F(IR_DecomposeAccessTest, UniformAccessMatrix) {
    auto* var = b.Var<uniform, mat4x4<f32>, core::Access::kRead>("v");
    var->SetBindingPoint(0, 0);

    b.ir.root_block->Append(var);
    auto* func = b.Function("foo", ty.void_(), core::ir::Function::PipelineStage::kFragment);
    b.Append(func->Block(), [&] {
        b.Let("a", b.Load(var));
        b.Let("b", b.Load(b.Access(ty.ptr<uniform, vec4<f32>, core::Access::kRead>(), var, 3_u)));
        b.Let("c", b.LoadVectorElement(
                       b.Access(ty.ptr<uniform, vec4<f32>, core::Access::kRead>(), var, 1_u), 2_u));
        b.Return(func);
    });

    auto* src = R"(
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
    ASSERT_EQ(src, str());

    auto* expect = R"(
$B1: {  # root
  %v:ptr<uniform, array<vec4<u32>, 4>, read> = var undef @binding_point(0, 0)
}

%foo = @fragment func():void {
  $B2: {
    %3:mat4x4<f32> = call %4, 0u
    %a:mat4x4<f32> = let %3
    %6:ptr<uniform, vec4<u32>, read> = access %v, 3u
    %7:vec4<u32> = load %6
    %8:vec4<f32> = bitcast<vec4<f32>> %7
    %b:vec4<f32> = let %8
    %10:ptr<uniform, vec4<u32>, read> = access %v, 1u
    %11:u32 = load_vector_element %10, 2u
    %12:f32 = bitcast<f32> %11
    %c:f32 = let %12
    ret
  }
}
%4 = func(%start_byte_offset:u32):mat4x4<f32> {
  $B3: {
    %15:u32 = div %start_byte_offset, 16u
    %16:ptr<uniform, vec4<u32>, read> = access %v, %15
    %17:vec4<u32> = load %16
    %18:vec4<f32> = bitcast<vec4<f32>> %17
    %19:u32 = add 16u, %start_byte_offset
    %20:u32 = div %19, 16u
    %21:ptr<uniform, vec4<u32>, read> = access %v, %20
    %22:vec4<u32> = load %21
    %23:vec4<f32> = bitcast<vec4<f32>> %22
    %24:u32 = add 32u, %start_byte_offset
    %25:u32 = div %24, 16u
    %26:ptr<uniform, vec4<u32>, read> = access %v, %25
    %27:vec4<u32> = load %26
    %28:vec4<f32> = bitcast<vec4<f32>> %27
    %29:u32 = add 48u, %start_byte_offset
    %30:u32 = div %29, 16u
    %31:ptr<uniform, vec4<u32>, read> = access %v, %30
    %32:vec4<u32> = load %31
    %33:vec4<f32> = bitcast<vec4<f32>> %32
    %34:mat4x4<f32> = construct %18, %23, %28, %33
    ret %34
  }
}
)";
    DecomposeAccessConfig options{.uniform = true};
    Run(DecomposeAccess, options);
    EXPECT_EQ(expect, str());
}

TEST_F(IR_DecomposeAccessTest, UniformAccessArray) {
    auto* var = b.Var<uniform, array<vec3<f32>, 5>, core::Access::kRead>("v");
    var->SetBindingPoint(0, 0);

    b.ir.root_block->Append(var);
    auto* func = b.Function("foo", ty.void_(), core::ir::Function::PipelineStage::kFragment);
    b.Append(func->Block(), [&] {
        b.Let("a", b.Load(var));
        b.Let("b", b.Load(b.Access(ty.ptr<uniform, vec3<f32>, core::Access::kRead>(), var, 3_u)));
        b.Return(func);
    });

    auto* src = R"(
$B1: {  # root
  %v:ptr<uniform, array<vec3<f32>, 5>, read> = var undef @binding_point(0, 0)
}

%foo = @fragment func():void {
  $B2: {
    %3:array<vec3<f32>, 5> = load %v
    %a:array<vec3<f32>, 5> = let %3
    %5:ptr<uniform, vec3<f32>, read> = access %v, 3u
    %6:vec3<f32> = load %5
    %b:vec3<f32> = let %6
    ret
  }
}
)";
    ASSERT_EQ(src, str());

    auto* expect = R"(
$B1: {  # root
  %v:ptr<uniform, array<vec4<u32>, 5>, read> = var undef @binding_point(0, 0)
}

%foo = @fragment func():void {
  $B2: {
    %3:array<vec3<f32>, 5> = call %4, 0u
    %a:array<vec3<f32>, 5> = let %3
    %6:ptr<uniform, vec4<u32>, read> = access %v, 3u
    %7:vec4<u32> = load %6
    %8:vec3<u32> = swizzle %7, xyz
    %9:vec3<f32> = bitcast<vec3<f32>> %8
    %b:vec3<f32> = let %9
    ret
  }
}
%4 = func(%start_byte_offset:u32):array<vec3<f32>, 5> {
  $B3: {
    %a_1:ptr<function, array<vec3<f32>, 5>, read_write> = var array<vec3<f32>, 5>(vec3<f32>(0.0f))  # %a_1: 'a'
    loop [i: $B4, b: $B5, c: $B6] {  # loop_1
      $B4: {  # initializer
        next_iteration 0u  # -> $B5
      }
      $B5 (%idx:u32): {  # body
        %14:bool = gte %idx, 5u
        if %14 [t: $B7] {  # if_1
          $B7: {  # true
            exit_loop  # loop_1
          }
        }
        %15:u32 = mul %idx, 16u
        %16:u32 = add %start_byte_offset, %15
        %17:ptr<function, vec3<f32>, read_write> = access %a_1, %idx
        %18:u32 = div %16, 16u
        %19:ptr<uniform, vec4<u32>, read> = access %v, %18
        %20:vec4<u32> = load %19
        %21:vec3<u32> = swizzle %20, xyz
        %22:vec3<f32> = bitcast<vec3<f32>> %21
        store %17, %22
        continue  # -> $B6
      }
      $B6: {  # continuing
        %23:u32 = add %idx, 1u
        next_iteration %23  # -> $B5
      }
    }
    %24:array<vec3<f32>, 5> = load %a_1
    ret %24
  }
}
)";
    DecomposeAccessConfig options{.uniform = true};
    Run(DecomposeAccess, options);
    EXPECT_EQ(expect, str());
}

TEST_F(IR_DecomposeAccessTest, UniformAccessArrayWhichCanHaveSizesOtherThenFive) {
    auto* var = b.Var<uniform, array<vec3<f32>, 42>, core::Access::kRead>("v");
    var->SetBindingPoint(0, 0);

    b.ir.root_block->Append(var);
    auto* func = b.Function("foo", ty.void_(), core::ir::Function::PipelineStage::kFragment);
    b.Append(func->Block(), [&] {
        b.Let("a", b.Load(var));
        b.Let("b", b.Load(b.Access(ty.ptr<uniform, vec3<f32>, core::Access::kRead>(), var, 3_u)));
        b.Return(func);
    });

    auto* src = R"(
$B1: {  # root
  %v:ptr<uniform, array<vec3<f32>, 42>, read> = var undef @binding_point(0, 0)
}

%foo = @fragment func():void {
  $B2: {
    %3:array<vec3<f32>, 42> = load %v
    %a:array<vec3<f32>, 42> = let %3
    %5:ptr<uniform, vec3<f32>, read> = access %v, 3u
    %6:vec3<f32> = load %5
    %b:vec3<f32> = let %6
    ret
  }
}
)";
    ASSERT_EQ(src, str());

    auto* expect = R"(
$B1: {  # root
  %v:ptr<uniform, array<vec4<u32>, 42>, read> = var undef @binding_point(0, 0)
}

%foo = @fragment func():void {
  $B2: {
    %3:array<vec3<f32>, 42> = call %4, 0u
    %a:array<vec3<f32>, 42> = let %3
    %6:ptr<uniform, vec4<u32>, read> = access %v, 3u
    %7:vec4<u32> = load %6
    %8:vec3<u32> = swizzle %7, xyz
    %9:vec3<f32> = bitcast<vec3<f32>> %8
    %b:vec3<f32> = let %9
    ret
  }
}
%4 = func(%start_byte_offset:u32):array<vec3<f32>, 42> {
  $B3: {
    %a_1:ptr<function, array<vec3<f32>, 42>, read_write> = var array<vec3<f32>, 42>(vec3<f32>(0.0f))  # %a_1: 'a'
    loop [i: $B4, b: $B5, c: $B6] {  # loop_1
      $B4: {  # initializer
        next_iteration 0u  # -> $B5
      }
      $B5 (%idx:u32): {  # body
        %14:bool = gte %idx, 42u
        if %14 [t: $B7] {  # if_1
          $B7: {  # true
            exit_loop  # loop_1
          }
        }
        %15:u32 = mul %idx, 16u
        %16:u32 = add %start_byte_offset, %15
        %17:ptr<function, vec3<f32>, read_write> = access %a_1, %idx
        %18:u32 = div %16, 16u
        %19:ptr<uniform, vec4<u32>, read> = access %v, %18
        %20:vec4<u32> = load %19
        %21:vec3<u32> = swizzle %20, xyz
        %22:vec3<f32> = bitcast<vec3<f32>> %21
        store %17, %22
        continue  # -> $B6
      }
      $B6: {  # continuing
        %23:u32 = add %idx, 1u
        next_iteration %23  # -> $B5
      }
    }
    %24:array<vec3<f32>, 42> = load %a_1
    ret %24
  }
}
)";
    DecomposeAccessConfig options{.uniform = true};
    Run(DecomposeAccess, options);
    EXPECT_EQ(expect, str());
}

TEST_F(IR_DecomposeAccessTest, UniformAccessStruct) {
    auto* SB = ty.Struct(mod.symbols.New("SB"), {
                                                    {mod.symbols.New("a"), ty.i32()},
                                                    {mod.symbols.New("b"), ty.f32()},
                                                });

    auto* var = b.Var("v", uniform, SB, core::Access::kRead);
    var->SetBindingPoint(0, 0);

    b.ir.root_block->Append(var);
    auto* func = b.Function("foo", ty.void_(), core::ir::Function::PipelineStage::kFragment);
    b.Append(func->Block(), [&] {
        b.Let("a", b.Load(var));
        b.Let("b", b.Load(b.Access(ty.ptr<uniform, f32, core::Access::kRead>(), var, 1_u)));
        b.Return(func);
    });

    auto* src = R"(
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
    ASSERT_EQ(src, str());

    auto* expect = R"(
SB = struct @align(4) {
  a:i32 @offset(0)
  b:f32 @offset(4)
}

$B1: {  # root
  %v:ptr<uniform, array<vec4<u32>, 1>, read> = var undef @binding_point(0, 0)
}

%foo = @fragment func():void {
  $B2: {
    %3:SB = call %4, 0u
    %a:SB = let %3
    %6:ptr<uniform, vec4<u32>, read> = access %v, 0u
    %7:u32 = load_vector_element %6, 1u
    %8:f32 = bitcast<f32> %7
    %b:f32 = let %8
    ret
  }
}
%4 = func(%start_byte_offset:u32):SB {
  $B3: {
    %11:u32 = div %start_byte_offset, 16u
    %12:ptr<uniform, vec4<u32>, read> = access %v, %11
    %13:u32 = and %start_byte_offset, 15u
    %14:u32 = shr %13, 2u
    %15:u32 = load_vector_element %12, %14
    %16:i32 = bitcast<i32> %15
    %17:u32 = add 4u, %start_byte_offset
    %18:u32 = div %17, 16u
    %19:ptr<uniform, vec4<u32>, read> = access %v, %18
    %20:u32 = and %17, 15u
    %21:u32 = shr %20, 2u
    %22:u32 = load_vector_element %19, %21
    %23:f32 = bitcast<f32> %22
    %24:SB = construct %16, %23
    ret %24
  }
}
)";
    DecomposeAccessConfig options{.uniform = true};
    Run(DecomposeAccess, options);
    EXPECT_EQ(expect, str());
}

TEST_F(IR_DecomposeAccessTest, UniformAccessStructNested) {
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

    auto* var = b.Var("v", uniform, SB, core::Access::kRead);
    var->SetBindingPoint(0, 0);

    b.ir.root_block->Append(var);
    auto* func = b.Function("foo", ty.void_(), core::ir::Function::PipelineStage::kFragment);
    b.Append(func->Block(), [&] {
        b.Let("a", b.Load(var));
        b.Let("b", b.LoadVectorElement(b.Access(ty.ptr<uniform, vec3<f32>, core::Access::kRead>(),
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
  %v:ptr<uniform, array<vec4<u32>, 10>, read> = var undef @binding_point(0, 0)
}

%foo = @fragment func():void {
  $B2: {
    %3:SB = call %4, 0u
    %a:SB = let %3
    %6:ptr<uniform, vec4<u32>, read> = access %v, 8u
    %7:u32 = load_vector_element %6, 2u
    %8:f32 = bitcast<f32> %7
    %b:f32 = let %8
    ret
  }
}
%4 = func(%start_byte_offset:u32):SB {
  $B3: {
    %11:u32 = div %start_byte_offset, 16u
    %12:ptr<uniform, vec4<u32>, read> = access %v, %11
    %13:u32 = and %start_byte_offset, 15u
    %14:u32 = shr %13, 2u
    %15:u32 = load_vector_element %12, %14
    %16:i32 = bitcast<i32> %15
    %17:u32 = add 16u, %start_byte_offset
    %18:Outer = call %19, %17
    %20:SB = construct %16, %18
    ret %20
  }
}
%19 = func(%start_byte_offset_1:u32):Outer {  # %start_byte_offset_1: 'start_byte_offset'
  $B4: {
    %22:u32 = div %start_byte_offset_1, 16u
    %23:ptr<uniform, vec4<u32>, read> = access %v, %22
    %24:u32 = and %start_byte_offset_1, 15u
    %25:u32 = shr %24, 2u
    %26:u32 = load_vector_element %23, %25
    %27:f32 = bitcast<f32> %26
    %28:u32 = add 16u, %start_byte_offset_1
    %29:Inner = call %30, %28
    %31:Outer = construct %27, %29
    ret %31
  }
}
%30 = func(%start_byte_offset_2:u32):Inner {  # %start_byte_offset_2: 'start_byte_offset'
  $B5: {
    %33:mat3x3<f32> = call %34, %start_byte_offset_2
    %35:u32 = add 48u, %start_byte_offset_2
    %36:array<vec3<f32>, 5> = call %37, %35
    %38:Inner = construct %33, %36
    ret %38
  }
}
%34 = func(%start_byte_offset_3:u32):mat3x3<f32> {  # %start_byte_offset_3: 'start_byte_offset'
  $B6: {
    %40:u32 = div %start_byte_offset_3, 16u
    %41:ptr<uniform, vec4<u32>, read> = access %v, %40
    %42:vec4<u32> = load %41
    %43:vec3<u32> = swizzle %42, xyz
    %44:vec3<f32> = bitcast<vec3<f32>> %43
    %45:u32 = add 16u, %start_byte_offset_3
    %46:u32 = div %45, 16u
    %47:ptr<uniform, vec4<u32>, read> = access %v, %46
    %48:vec4<u32> = load %47
    %49:vec3<u32> = swizzle %48, xyz
    %50:vec3<f32> = bitcast<vec3<f32>> %49
    %51:u32 = add 32u, %start_byte_offset_3
    %52:u32 = div %51, 16u
    %53:ptr<uniform, vec4<u32>, read> = access %v, %52
    %54:vec4<u32> = load %53
    %55:vec3<u32> = swizzle %54, xyz
    %56:vec3<f32> = bitcast<vec3<f32>> %55
    %57:mat3x3<f32> = construct %44, %50, %56
    ret %57
  }
}
%37 = func(%start_byte_offset_4:u32):array<vec3<f32>, 5> {  # %start_byte_offset_4: 'start_byte_offset'
  $B7: {
    %a_1:ptr<function, array<vec3<f32>, 5>, read_write> = var array<vec3<f32>, 5>(vec3<f32>(0.0f))  # %a_1: 'a'
    loop [i: $B8, b: $B9, c: $B10] {  # loop_1
      $B8: {  # initializer
        next_iteration 0u  # -> $B9
      }
      $B9 (%idx:u32): {  # body
        %61:bool = gte %idx, 5u
        if %61 [t: $B11] {  # if_1
          $B11: {  # true
            exit_loop  # loop_1
          }
        }
        %62:u32 = mul %idx, 16u
        %63:u32 = add %start_byte_offset_4, %62
        %64:ptr<function, vec3<f32>, read_write> = access %a_1, %idx
        %65:u32 = div %63, 16u
        %66:ptr<uniform, vec4<u32>, read> = access %v, %65
        %67:vec4<u32> = load %66
        %68:vec3<u32> = swizzle %67, xyz
        %69:vec3<f32> = bitcast<vec3<f32>> %68
        store %64, %69
        continue  # -> $B10
      }
      $B10: {  # continuing
        %70:u32 = add %idx, 1u
        next_iteration %70  # -> $B9
      }
    }
    %71:array<vec3<f32>, 5> = load %a_1
    ret %71
  }
}
)";
    DecomposeAccessConfig options{.uniform = true};
    Run(DecomposeAccess, options);
    EXPECT_EQ(expect, str());
}

TEST_F(IR_DecomposeAccessTest, UniformAccessChainReused) {
    auto* sb = ty.Struct(mod.symbols.New("SB"), {
                                                    {mod.symbols.New("c"), ty.f32()},
                                                    {mod.symbols.New("d"), ty.vec3f()},
                                                });

    auto* var = b.Var("v", uniform, sb, core::Access::kRead);
    var->SetBindingPoint(0, 0);
    b.ir.root_block->Append(var);

    auto* func = b.Function("foo", ty.void_(), core::ir::Function::PipelineStage::kFragment);
    b.Append(func->Block(), [&] {
        auto* x = b.Access(ty.ptr(uniform, ty.vec3f(), core::Access::kRead), var, 1_u);
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
    ASSERT_EQ(src, str());

    auto* expect = R"(
SB = struct @align(16) {
  c:f32 @offset(0)
  d:vec3<f32> @offset(16)
}

$B1: {  # root
  %v:ptr<uniform, array<vec4<u32>, 2>, read> = var undef @binding_point(0, 0)
}

%foo = @fragment func():void {
  $B2: {
    %3:ptr<uniform, vec4<u32>, read> = access %v, 1u
    %4:u32 = load_vector_element %3, 1u
    %5:f32 = bitcast<f32> %4
    %b:f32 = let %5
    %7:ptr<uniform, vec4<u32>, read> = access %v, 1u
    %8:u32 = load_vector_element %7, 2u
    %9:f32 = bitcast<f32> %8
    %c:f32 = let %9
    ret
  }
}
)";

    DecomposeAccessConfig options{.uniform = true};
    Run(DecomposeAccess, options);
    EXPECT_EQ(expect, str());
}

TEST_F(IR_DecomposeAccessTest, Determinism_MultipleUsesOfLetFromVar) {
    auto* sb =
        ty.Struct(mod.symbols.New("SB"), {
                                             {mod.symbols.New("a"), ty.array<vec4<f32>, 2>()},
                                             {mod.symbols.New("b"), ty.array<vec4<i32>, 2>()},
                                         });

    auto* var = b.Var("v", uniform, sb, core::Access::kRead);
    var->SetBindingPoint(0, 0);
    b.ir.root_block->Append(var);

    auto* func = b.Function("foo", ty.void_(), core::ir::Function::PipelineStage::kFragment);
    b.Append(func->Block(), [&] {
        auto* let = b.Let("l", var);
        auto* pa =
            b.Access(ty.ptr(uniform, ty.array<vec4<f32>, 2>(), core::Access::kRead), let, 0_u);
        b.Let("a", b.Load(pa));
        auto* pb =
            b.Access(ty.ptr(uniform, ty.array<vec4<i32>, 2>(), core::Access::kRead), let, 1_u);
        b.Let("b", b.Load(pb));
        b.Return(func);
    });

    auto* src = R"(
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
    ASSERT_EQ(src, str());

    auto* expect = R"(
SB = struct @align(16) {
  a:array<vec4<f32>, 2> @offset(0)
  b:array<vec4<i32>, 2> @offset(32)
}

$B1: {  # root
  %v:ptr<uniform, array<vec4<u32>, 4>, read> = var undef @binding_point(0, 0)
}

%foo = @fragment func():void {
  $B2: {
    %3:array<vec4<f32>, 2> = call %4, 0u
    %a:array<vec4<f32>, 2> = let %3
    %6:array<vec4<i32>, 2> = call %7, 32u
    %b:array<vec4<i32>, 2> = let %6
    ret
  }
}
%7 = func(%start_byte_offset:u32):array<vec4<i32>, 2> {
  $B3: {
    %a_1:ptr<function, array<vec4<i32>, 2>, read_write> = var array<vec4<i32>, 2>(vec4<i32>(0i))  # %a_1: 'a'
    loop [i: $B4, b: $B5, c: $B6] {  # loop_1
      $B4: {  # initializer
        next_iteration 0u  # -> $B5
      }
      $B5 (%idx:u32): {  # body
        %12:bool = gte %idx, 2u
        if %12 [t: $B7] {  # if_1
          $B7: {  # true
            exit_loop  # loop_1
          }
        }
        %13:u32 = mul %idx, 16u
        %14:u32 = add %start_byte_offset, %13
        %15:ptr<function, vec4<i32>, read_write> = access %a_1, %idx
        %16:u32 = div %14, 16u
        %17:ptr<uniform, vec4<u32>, read> = access %v, %16
        %18:vec4<u32> = load %17
        %19:vec4<i32> = bitcast<vec4<i32>> %18
        store %15, %19
        continue  # -> $B6
      }
      $B6: {  # continuing
        %20:u32 = add %idx, 1u
        next_iteration %20  # -> $B5
      }
    }
    %21:array<vec4<i32>, 2> = load %a_1
    ret %21
  }
}
%4 = func(%start_byte_offset_1:u32):array<vec4<f32>, 2> {  # %start_byte_offset_1: 'start_byte_offset'
  $B8: {
    %a_2:ptr<function, array<vec4<f32>, 2>, read_write> = var array<vec4<f32>, 2>(vec4<f32>(0.0f))  # %a_2: 'a'
    loop [i: $B9, b: $B10, c: $B11] {  # loop_2
      $B9: {  # initializer
        next_iteration 0u  # -> $B10
      }
      $B10 (%idx_1:u32): {  # body
        %25:bool = gte %idx_1, 2u
        if %25 [t: $B12] {  # if_2
          $B12: {  # true
            exit_loop  # loop_2
          }
        }
        %26:u32 = mul %idx_1, 16u
        %27:u32 = add %start_byte_offset_1, %26
        %28:ptr<function, vec4<f32>, read_write> = access %a_2, %idx_1
        %29:u32 = div %27, 16u
        %30:ptr<uniform, vec4<u32>, read> = access %v, %29
        %31:vec4<u32> = load %30
        %32:vec4<f32> = bitcast<vec4<f32>> %31
        store %28, %32
        continue  # -> $B11
      }
      $B11: {  # continuing
        %33:u32 = add %idx_1, 1u
        next_iteration %33  # -> $B10
      }
    }
    %34:array<vec4<f32>, 2> = load %a_2
    ret %34
  }
}
)";

    DecomposeAccessConfig options{.uniform = true};
    Run(DecomposeAccess, options);
    EXPECT_EQ(expect, str());
}

TEST_F(IR_DecomposeAccessTest, Determinism_MultipleUsesOfLetFromAccess) {
    auto* sb =
        ty.Struct(mod.symbols.New("SB"), {
                                             {mod.symbols.New("a"), ty.array<vec4<f32>, 2>()},
                                             {mod.symbols.New("b"), ty.array<vec4<i32>, 2>()},
                                         });

    auto* var = b.Var("v", uniform, ty.array(sb, 2), core::Access::kRead);
    var->SetBindingPoint(0, 0);
    b.ir.root_block->Append(var);

    auto* func = b.Function("foo", ty.void_(), core::ir::Function::PipelineStage::kFragment);
    b.Append(func->Block(), [&] {
        auto* let = b.Let("l", b.Access(ty.ptr(uniform, sb, core::Access::kRead), var, 0_u));
        auto* pa =
            b.Access(ty.ptr(uniform, ty.array<vec4<f32>, 2>(), core::Access::kRead), let, 0_u);
        b.Let("a", b.Load(pa));
        auto* pb =
            b.Access(ty.ptr(uniform, ty.array<vec4<i32>, 2>(), core::Access::kRead), let, 1_u);
        b.Let("b", b.Load(pb));
        b.Return(func);
    });

    auto* src = R"(
SB = struct @align(16) {
  a:array<vec4<f32>, 2> @offset(0)
  b:array<vec4<i32>, 2> @offset(32)
}

$B1: {  # root
  %v:ptr<uniform, array<SB, 2>, read> = var undef @binding_point(0, 0)
}

%foo = @fragment func():void {
  $B2: {
    %3:ptr<uniform, SB, read> = access %v, 0u
    %l:ptr<uniform, SB, read> = let %3
    %5:ptr<uniform, array<vec4<f32>, 2>, read> = access %l, 0u
    %6:array<vec4<f32>, 2> = load %5
    %a:array<vec4<f32>, 2> = let %6
    %8:ptr<uniform, array<vec4<i32>, 2>, read> = access %l, 1u
    %9:array<vec4<i32>, 2> = load %8
    %b:array<vec4<i32>, 2> = let %9
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
  %v:ptr<uniform, array<vec4<u32>, 8>, read> = var undef @binding_point(0, 0)
}

%foo = @fragment func():void {
  $B2: {
    %3:array<vec4<f32>, 2> = call %4, 0u
    %a:array<vec4<f32>, 2> = let %3
    %6:array<vec4<i32>, 2> = call %7, 32u
    %b:array<vec4<i32>, 2> = let %6
    ret
  }
}
%7 = func(%start_byte_offset:u32):array<vec4<i32>, 2> {
  $B3: {
    %a_1:ptr<function, array<vec4<i32>, 2>, read_write> = var array<vec4<i32>, 2>(vec4<i32>(0i))  # %a_1: 'a'
    loop [i: $B4, b: $B5, c: $B6] {  # loop_1
      $B4: {  # initializer
        next_iteration 0u  # -> $B5
      }
      $B5 (%idx:u32): {  # body
        %12:bool = gte %idx, 2u
        if %12 [t: $B7] {  # if_1
          $B7: {  # true
            exit_loop  # loop_1
          }
        }
        %13:u32 = mul %idx, 16u
        %14:u32 = add %start_byte_offset, %13
        %15:ptr<function, vec4<i32>, read_write> = access %a_1, %idx
        %16:u32 = div %14, 16u
        %17:ptr<uniform, vec4<u32>, read> = access %v, %16
        %18:vec4<u32> = load %17
        %19:vec4<i32> = bitcast<vec4<i32>> %18
        store %15, %19
        continue  # -> $B6
      }
      $B6: {  # continuing
        %20:u32 = add %idx, 1u
        next_iteration %20  # -> $B5
      }
    }
    %21:array<vec4<i32>, 2> = load %a_1
    ret %21
  }
}
%4 = func(%start_byte_offset_1:u32):array<vec4<f32>, 2> {  # %start_byte_offset_1: 'start_byte_offset'
  $B8: {
    %a_2:ptr<function, array<vec4<f32>, 2>, read_write> = var array<vec4<f32>, 2>(vec4<f32>(0.0f))  # %a_2: 'a'
    loop [i: $B9, b: $B10, c: $B11] {  # loop_2
      $B9: {  # initializer
        next_iteration 0u  # -> $B10
      }
      $B10 (%idx_1:u32): {  # body
        %25:bool = gte %idx_1, 2u
        if %25 [t: $B12] {  # if_2
          $B12: {  # true
            exit_loop  # loop_2
          }
        }
        %26:u32 = mul %idx_1, 16u
        %27:u32 = add %start_byte_offset_1, %26
        %28:ptr<function, vec4<f32>, read_write> = access %a_2, %idx_1
        %29:u32 = div %27, 16u
        %30:ptr<uniform, vec4<u32>, read> = access %v, %29
        %31:vec4<u32> = load %30
        %32:vec4<f32> = bitcast<vec4<f32>> %31
        store %28, %32
        continue  # -> $B11
      }
      $B11: {  # continuing
        %33:u32 = add %idx_1, 1u
        next_iteration %33  # -> $B10
      }
    }
    %34:array<vec4<f32>, 2> = load %a_2
    ret %34
  }
}
)";

    DecomposeAccessConfig options{.uniform = true};
    Run(DecomposeAccess, options);
    EXPECT_EQ(expect, str());
}

TEST_F(IR_DecomposeAccessTest, Determinism_MultipleUsesOfAccess) {
    auto* sb =
        ty.Struct(mod.symbols.New("SB"), {
                                             {mod.symbols.New("a"), ty.array<vec4<f32>, 2>()},
                                             {mod.symbols.New("b"), ty.array<vec4<i32>, 2>()},
                                         });

    auto* var = b.Var("v", uniform, ty.array(sb, 2), core::Access::kRead);
    var->SetBindingPoint(0, 0);
    b.ir.root_block->Append(var);

    auto* func = b.Function("foo", ty.void_(), core::ir::Function::PipelineStage::kFragment);
    b.Append(func->Block(), [&] {
        auto* access = b.Access(ty.ptr(uniform, sb, core::Access::kRead), var, 0_u);
        auto* pa =
            b.Access(ty.ptr(uniform, ty.array<vec4<f32>, 2>(), core::Access::kRead), access, 0_u);
        b.Let("a", b.Load(pa));
        auto* pb =
            b.Access(ty.ptr(uniform, ty.array<vec4<i32>, 2>(), core::Access::kRead), access, 1_u);
        b.Let("b", b.Load(pb));
        b.Return(func);
    });

    auto* src = R"(
SB = struct @align(16) {
  a:array<vec4<f32>, 2> @offset(0)
  b:array<vec4<i32>, 2> @offset(32)
}

$B1: {  # root
  %v:ptr<uniform, array<SB, 2>, read> = var undef @binding_point(0, 0)
}

%foo = @fragment func():void {
  $B2: {
    %3:ptr<uniform, SB, read> = access %v, 0u
    %4:ptr<uniform, array<vec4<f32>, 2>, read> = access %3, 0u
    %5:array<vec4<f32>, 2> = load %4
    %a:array<vec4<f32>, 2> = let %5
    %7:ptr<uniform, array<vec4<i32>, 2>, read> = access %3, 1u
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
  %v:ptr<uniform, array<vec4<u32>, 8>, read> = var undef @binding_point(0, 0)
}

%foo = @fragment func():void {
  $B2: {
    %3:array<vec4<f32>, 2> = call %4, 0u
    %a:array<vec4<f32>, 2> = let %3
    %6:array<vec4<i32>, 2> = call %7, 32u
    %b:array<vec4<i32>, 2> = let %6
    ret
  }
}
%7 = func(%start_byte_offset:u32):array<vec4<i32>, 2> {
  $B3: {
    %a_1:ptr<function, array<vec4<i32>, 2>, read_write> = var array<vec4<i32>, 2>(vec4<i32>(0i))  # %a_1: 'a'
    loop [i: $B4, b: $B5, c: $B6] {  # loop_1
      $B4: {  # initializer
        next_iteration 0u  # -> $B5
      }
      $B5 (%idx:u32): {  # body
        %12:bool = gte %idx, 2u
        if %12 [t: $B7] {  # if_1
          $B7: {  # true
            exit_loop  # loop_1
          }
        }
        %13:u32 = mul %idx, 16u
        %14:u32 = add %start_byte_offset, %13
        %15:ptr<function, vec4<i32>, read_write> = access %a_1, %idx
        %16:u32 = div %14, 16u
        %17:ptr<uniform, vec4<u32>, read> = access %v, %16
        %18:vec4<u32> = load %17
        %19:vec4<i32> = bitcast<vec4<i32>> %18
        store %15, %19
        continue  # -> $B6
      }
      $B6: {  # continuing
        %20:u32 = add %idx, 1u
        next_iteration %20  # -> $B5
      }
    }
    %21:array<vec4<i32>, 2> = load %a_1
    ret %21
  }
}
%4 = func(%start_byte_offset_1:u32):array<vec4<f32>, 2> {  # %start_byte_offset_1: 'start_byte_offset'
  $B8: {
    %a_2:ptr<function, array<vec4<f32>, 2>, read_write> = var array<vec4<f32>, 2>(vec4<f32>(0.0f))  # %a_2: 'a'
    loop [i: $B9, b: $B10, c: $B11] {  # loop_2
      $B9: {  # initializer
        next_iteration 0u  # -> $B10
      }
      $B10 (%idx_1:u32): {  # body
        %25:bool = gte %idx_1, 2u
        if %25 [t: $B12] {  # if_2
          $B12: {  # true
            exit_loop  # loop_2
          }
        }
        %26:u32 = mul %idx_1, 16u
        %27:u32 = add %start_byte_offset_1, %26
        %28:ptr<function, vec4<f32>, read_write> = access %a_2, %idx_1
        %29:u32 = div %27, 16u
        %30:ptr<uniform, vec4<u32>, read> = access %v, %29
        %31:vec4<u32> = load %30
        %32:vec4<f32> = bitcast<vec4<f32>> %31
        store %28, %32
        continue  # -> $B11
      }
      $B11: {  # continuing
        %33:u32 = add %idx_1, 1u
        next_iteration %33  # -> $B10
      }
    }
    %34:array<vec4<f32>, 2> = load %a_2
    ret %34
  }
}
)";

    DecomposeAccessConfig options{.uniform = true};
    Run(DecomposeAccess, options);
    EXPECT_EQ(expect, str());
}

TEST_F(IR_DecomposeAccessTest, Storage_AccessU16_LoadF16) {
    auto* var = b.Var("v", storage, ty.f16(), core::Access::kRead);
    var->SetBindingPoint(0, 0);
    b.ir.root_block->Append(var);

    auto* func = b.Function("foo", ty.void_(), core::ir::Function::PipelineStage::kFragment);
    b.Append(func->Block(), [&] {
        b.Let("a", b.Load(var));
        b.Return(func);
    });

    auto* src = R"(
$B1: {  # root
  %v:ptr<storage, f16, read> = var undef @binding_point(0, 0)
}

%foo = @fragment func():void {
  $B2: {
    %3:f16 = load %v
    %a:f16 = let %3
    ret
  }
}
)";

    ASSERT_EQ(src, str());

    auto* expect = R"(
$B1: {  # root
  %v:ptr<storage, array<u16, 1>, read> = var undef @binding_point(0, 0)
}

%foo = @fragment func():void {
  $B2: {
    %3:ptr<storage, u16, read> = access %v, 0u
    %4:u16 = load %3
    %5:f16 = bitcast<f16> %4
    %a:f16 = let %5
    ret
  }
}
)";

    DecomposeAccessConfig options{.storage = true};
    Run(DecomposeAccess, options);
    EXPECT_EQ(expect, str());
}

TEST_F(IR_DecomposeAccessTest, Storage_AccessU32_LoadF32) {
    auto* var = b.Var("v", storage, ty.f32(), core::Access::kRead);
    var->SetBindingPoint(0, 0);
    b.ir.root_block->Append(var);

    auto* func = b.Function("foo", ty.void_(), core::ir::Function::PipelineStage::kFragment);
    b.Append(func->Block(), [&] {
        b.Let("a", b.Load(var));
        b.Return(func);
    });

    auto* src = R"(
$B1: {  # root
  %v:ptr<storage, f32, read> = var undef @binding_point(0, 0)
}

%foo = @fragment func():void {
  $B2: {
    %3:f32 = load %v
    %a:f32 = let %3
    ret
  }
}
)";

    ASSERT_EQ(src, str());

    auto* expect = R"(
$B1: {  # root
  %v:ptr<storage, array<u32, 1>, read> = var undef @binding_point(0, 0)
}

%foo = @fragment func():void {
  $B2: {
    %3:ptr<storage, u32, read> = access %v, 0u
    %4:u32 = load %3
    %5:f32 = bitcast<f32> %4
    %a:f32 = let %5
    ret
  }
}
)";

    DecomposeAccessConfig options{.storage = true};
    Run(DecomposeAccess, options);
    EXPECT_EQ(expect, str());
}

TEST_F(IR_DecomposeAccessTest, Workgroup_AccessU32_LoadBool) {
    auto* var = b.Var("v", workgroup, ty.bool_());
    b.ir.root_block->Append(var);

    auto* func = b.Function("foo", ty.void_());
    b.Append(func->Block(), [&] {
        b.Let("a", b.Load(var));
        b.Return(func);
    });

    auto* src = R"(
$B1: {  # root
  %v:ptr<workgroup, bool, read_write> = var undef
}

%foo = func():void {
  $B2: {
    %3:bool = load %v
    %a:bool = let %3
    ret
  }
}
)";

    ASSERT_EQ(src, str());

    auto* expect = R"(
$B1: {  # root
  %v:ptr<workgroup, array<u32, 1>, read_write> = var undef
}

%foo = func():void {
  $B2: {
    %3:ptr<workgroup, u32, read_write> = access %v, 0u
    %4:u32 = load %3
    %5:bool = convert %4
    %a:bool = let %5
    ret
  }
}
)";

    DecomposeAccessConfig options{.workgroup = true};
    Run(DecomposeAccess, options);
    EXPECT_EQ(expect, str());
}

TEST_F(IR_DecomposeAccessTest, Storage_AccessU16_LoadU32) {
    auto* sb = ty.Struct(mod.symbols.New("SB"), {
                                                    {mod.symbols.New("a"), ty.u16()},
                                                    {mod.symbols.New("b"), ty.u32()},
                                                });
    auto* var = b.Var("v", storage, sb, core::Access::kReadWrite);
    var->SetBindingPoint(0, 0);
    b.ir.root_block->Append(var);

    auto* func = b.Function("foo", ty.void_(), core::ir::Function::PipelineStage::kFragment);
    b.Append(func->Block(), [&] {
        b.Let(
            "a",
            b.Load(
                b.Access(ty.ptr(storage, ty.u16(), core::Access::kReadWrite), var, 0_u)->Result()));
        b.Let(
            "b",
            b.Load(
                b.Access(ty.ptr(storage, ty.u32(), core::Access::kReadWrite), var, 1_u)->Result()));
        b.Return(func);
    });

    auto* src = R"(
SB = struct @align(4) {
  a:u16 @offset(0)
  b:u32 @offset(4)
}

$B1: {  # root
  %v:ptr<storage, SB, read_write> = var undef @binding_point(0, 0)
}

%foo = @fragment func():void {
  $B2: {
    %3:ptr<storage, u16, read_write> = access %v, 0u
    %4:u16 = load %3
    %a:u16 = let %4
    %6:ptr<storage, u32, read_write> = access %v, 1u
    %7:u32 = load %6
    %b:u32 = let %7
    ret
  }
}
)";

    ASSERT_EQ(src, str());

    auto* expect = R"(
SB = struct @align(4) {
  a:u16 @offset(0)
  b:u32 @offset(4)
}

$B1: {  # root
  %v:ptr<storage, array<u16, 4>, read_write> = var undef @binding_point(0, 0)
}

%foo = @fragment func():void {
  $B2: {
    %3:ptr<storage, u16, read_write> = access %v, 0u
    %4:u16 = load %3
    %a:u16 = let %4
    %6:ptr<storage, u16, read_write> = access %v, 2u
    %7:u16 = load %6 @align(4)
    %8:ptr<storage, u16, read_write> = access %v, 3u
    %9:u16 = load %8
    %10:vec2<u16> = construct %7, %9
    %11:u32 = bitcast<u32> %10
    %b:u32 = let %11
    ret
  }
}
)";

    DecomposeAccessConfig options{.storage = true};
    Run(DecomposeAccess, options);
    EXPECT_EQ(expect, str());
}

TEST_F(IR_DecomposeAccessTest, Workgroup_AccessU16_LoadBool) {
    auto* sb = ty.Struct(mod.symbols.New("SB"), {
                                                    {mod.symbols.New("a"), ty.u16()},
                                                    {mod.symbols.New("b"), ty.bool_()},
                                                });
    auto* var = b.Var("v", workgroup, sb, core::Access::kReadWrite);
    b.ir.root_block->Append(var);

    auto* func = b.Function("foo", ty.void_());
    b.Append(func->Block(), [&] {
        b.Let("a", b.Load(b.Access(ty.ptr(workgroup, ty.u16(), core::Access::kReadWrite), var, 0_u)
                              ->Result()));
        b.Let("b",
              b.Load(b.Access(ty.ptr(workgroup, ty.bool_(), core::Access::kReadWrite), var, 1_u)
                         ->Result()));
        b.Return(func);
    });

    auto* src = R"(
SB = struct @align(4) {
  a:u16 @offset(0)
  b:bool @offset(4)
}

$B1: {  # root
  %v:ptr<workgroup, SB, read_write> = var undef
}

%foo = func():void {
  $B2: {
    %3:ptr<workgroup, u16, read_write> = access %v, 0u
    %4:u16 = load %3
    %a:u16 = let %4
    %6:ptr<workgroup, bool, read_write> = access %v, 1u
    %7:bool = load %6
    %b:bool = let %7
    ret
  }
}
)";

    ASSERT_EQ(src, str());

    auto* expect = R"(
SB = struct @align(4) {
  a:u16 @offset(0)
  b:bool @offset(4)
}

$B1: {  # root
  %v:ptr<workgroup, array<u16, 4>, read_write> = var undef
}

%foo = func():void {
  $B2: {
    %3:ptr<workgroup, u16, read_write> = access %v, 0u
    %4:u16 = load %3
    %a:u16 = let %4
    %6:ptr<workgroup, u16, read_write> = access %v, 2u
    %7:u16 = load %6 @align(4)
    %8:ptr<workgroup, u16, read_write> = access %v, 3u
    %9:u16 = load %8
    %10:vec2<u16> = construct %7, %9
    %11:u32 = bitcast<u32> %10
    %12:bool = convert %11
    %b:bool = let %12
    ret
  }
}
)";

    DecomposeAccessConfig options{.workgroup = true};
    Run(DecomposeAccess, options);
    EXPECT_EQ(expect, str());
}

TEST_F(IR_DecomposeAccessTest, Storage_AccessU16_LoadVec2h) {
    auto* sb = ty.Struct(mod.symbols.New("SB"), {
                                                    {mod.symbols.New("a"), ty.u16()},
                                                    {mod.symbols.New("b"), ty.vec2h()},
                                                });
    auto* var = b.Var("v", storage, sb, core::Access::kReadWrite);
    var->SetBindingPoint(0, 0);
    b.ir.root_block->Append(var);

    auto* func = b.Function("foo", ty.void_(), core::ir::Function::PipelineStage::kFragment);
    b.Append(func->Block(), [&] {
        b.Let(
            "a",
            b.Load(
                b.Access(ty.ptr(storage, ty.u16(), core::Access::kReadWrite), var, 0_u)->Result()));
        b.Let("b", b.Load(b.Access(ty.ptr(storage, ty.vec2h(), core::Access::kReadWrite), var, 1_u)
                              ->Result()));
        b.Return(func);
    });

    auto* src = R"(
SB = struct @align(4) {
  a:u16 @offset(0)
  b:vec2<f16> @offset(4)
}

$B1: {  # root
  %v:ptr<storage, SB, read_write> = var undef @binding_point(0, 0)
}

%foo = @fragment func():void {
  $B2: {
    %3:ptr<storage, u16, read_write> = access %v, 0u
    %4:u16 = load %3
    %a:u16 = let %4
    %6:ptr<storage, vec2<f16>, read_write> = access %v, 1u
    %7:vec2<f16> = load %6
    %b:vec2<f16> = let %7
    ret
  }
}
)";

    ASSERT_EQ(src, str());

    auto* expect = R"(
SB = struct @align(4) {
  a:u16 @offset(0)
  b:vec2<f16> @offset(4)
}

$B1: {  # root
  %v:ptr<storage, array<u16, 4>, read_write> = var undef @binding_point(0, 0)
}

%foo = @fragment func():void {
  $B2: {
    %3:ptr<storage, u16, read_write> = access %v, 0u
    %4:u16 = load %3
    %a:u16 = let %4
    %6:ptr<storage, u16, read_write> = access %v, 2u
    %7:u16 = load %6 @align(4)
    %8:ptr<storage, u16, read_write> = access %v, 3u
    %9:u16 = load %8
    %10:vec2<u16> = construct %7, %9
    %11:vec2<f16> = bitcast<vec2<f16>> %10
    %b:vec2<f16> = let %11
    ret
  }
}
)";

    DecomposeAccessConfig options{.storage = true};
    Run(DecomposeAccess, options);
    EXPECT_EQ(expect, str());
}

TEST_F(IR_DecomposeAccessTest, Storage_AccessU16_LoadVec3h) {
    auto* sb = ty.Struct(mod.symbols.New("SB"), {
                                                    {mod.symbols.New("a"), ty.u16()},
                                                    {mod.symbols.New("b"), ty.vec3h()},
                                                });
    auto* var = b.Var("v", storage, sb, core::Access::kReadWrite);
    var->SetBindingPoint(0, 0);
    b.ir.root_block->Append(var);

    auto* func = b.Function("foo", ty.void_(), core::ir::Function::PipelineStage::kFragment);
    b.Append(func->Block(), [&] {
        b.Let(
            "a",
            b.Load(
                b.Access(ty.ptr(storage, ty.u16(), core::Access::kReadWrite), var, 0_u)->Result()));
        b.Let("b", b.Load(b.Access(ty.ptr(storage, ty.vec3h(), core::Access::kReadWrite), var, 1_u)
                              ->Result()));
        b.Return(func);
    });

    auto* src = R"(
SB = struct @align(8) {
  a:u16 @offset(0)
  b:vec3<f16> @offset(8)
}

$B1: {  # root
  %v:ptr<storage, SB, read_write> = var undef @binding_point(0, 0)
}

%foo = @fragment func():void {
  $B2: {
    %3:ptr<storage, u16, read_write> = access %v, 0u
    %4:u16 = load %3
    %a:u16 = let %4
    %6:ptr<storage, vec3<f16>, read_write> = access %v, 1u
    %7:vec3<f16> = load %6
    %b:vec3<f16> = let %7
    ret
  }
}
)";

    ASSERT_EQ(src, str());

    auto* expect = R"(
SB = struct @align(8) {
  a:u16 @offset(0)
  b:vec3<f16> @offset(8)
}

$B1: {  # root
  %v:ptr<storage, array<u16, 8>, read_write> = var undef @binding_point(0, 0)
}

%foo = @fragment func():void {
  $B2: {
    %3:ptr<storage, u16, read_write> = access %v, 0u
    %4:u16 = load %3
    %a:u16 = let %4
    %6:ptr<storage, u16, read_write> = access %v, 4u
    %7:u16 = load %6 @align(8)
    %8:ptr<storage, u16, read_write> = access %v, 5u
    %9:u16 = load %8
    %10:ptr<storage, u16, read_write> = access %v, 6u
    %11:u16 = load %10
    %12:vec3<u16> = construct %7, %9, %11
    %13:vec3<f16> = bitcast<vec3<f16>> %12
    %b:vec3<f16> = let %13
    ret
  }
}
)";

    DecomposeAccessConfig options{.storage = true};
    Run(DecomposeAccess, options);
    EXPECT_EQ(expect, str());
}

TEST_F(IR_DecomposeAccessTest, Storage_AccessU16_LoadVec4h) {
    auto* sb = ty.Struct(mod.symbols.New("SB"), {
                                                    {mod.symbols.New("a"), ty.u16()},
                                                    {mod.symbols.New("b"), ty.vec4h()},
                                                });
    auto* var = b.Var("v", storage, sb, core::Access::kReadWrite);
    var->SetBindingPoint(0, 0);
    b.ir.root_block->Append(var);

    auto* func = b.Function("foo", ty.void_(), core::ir::Function::PipelineStage::kFragment);
    b.Append(func->Block(), [&] {
        b.Let(
            "a",
            b.Load(
                b.Access(ty.ptr(storage, ty.u16(), core::Access::kReadWrite), var, 0_u)->Result()));
        b.Let("b", b.Load(b.Access(ty.ptr(storage, ty.vec4h(), core::Access::kReadWrite), var, 1_u)
                              ->Result()));
        b.Return(func);
    });

    auto* src = R"(
SB = struct @align(8) {
  a:u16 @offset(0)
  b:vec4<f16> @offset(8)
}

$B1: {  # root
  %v:ptr<storage, SB, read_write> = var undef @binding_point(0, 0)
}

%foo = @fragment func():void {
  $B2: {
    %3:ptr<storage, u16, read_write> = access %v, 0u
    %4:u16 = load %3
    %a:u16 = let %4
    %6:ptr<storage, vec4<f16>, read_write> = access %v, 1u
    %7:vec4<f16> = load %6
    %b:vec4<f16> = let %7
    ret
  }
}
)";

    ASSERT_EQ(src, str());

    auto* expect = R"(
SB = struct @align(8) {
  a:u16 @offset(0)
  b:vec4<f16> @offset(8)
}

$B1: {  # root
  %v:ptr<storage, array<u16, 8>, read_write> = var undef @binding_point(0, 0)
}

%foo = @fragment func():void {
  $B2: {
    %3:ptr<storage, u16, read_write> = access %v, 0u
    %4:u16 = load %3
    %a:u16 = let %4
    %6:ptr<storage, u16, read_write> = access %v, 4u
    %7:u16 = load %6 @align(8)
    %8:ptr<storage, u16, read_write> = access %v, 5u
    %9:u16 = load %8
    %10:ptr<storage, u16, read_write> = access %v, 6u
    %11:u16 = load %10
    %12:ptr<storage, u16, read_write> = access %v, 7u
    %13:u16 = load %12
    %14:vec4<u16> = construct %7, %9, %11, %13
    %15:vec4<f16> = bitcast<vec4<f16>> %14
    %b:vec4<f16> = let %15
    ret
  }
}
)";

    DecomposeAccessConfig options{.storage = true};
    Run(DecomposeAccess, options);
    EXPECT_EQ(expect, str());
}

TEST_F(IR_DecomposeAccessTest, Storage_AccessU16_LoadVec2u) {
    auto* sb = ty.Struct(mod.symbols.New("SB"), {
                                                    {mod.symbols.New("a"), ty.u16()},
                                                    {mod.symbols.New("b"), ty.vec2u()},
                                                });
    auto* var = b.Var("v", storage, sb, core::Access::kReadWrite);
    var->SetBindingPoint(0, 0);
    b.ir.root_block->Append(var);

    auto* func = b.Function("foo", ty.void_(), core::ir::Function::PipelineStage::kFragment);
    b.Append(func->Block(), [&] {
        b.Let(
            "a",
            b.Load(
                b.Access(ty.ptr(storage, ty.u16(), core::Access::kReadWrite), var, 0_u)->Result()));
        b.Let("b", b.Load(b.Access(ty.ptr(storage, ty.vec2u(), core::Access::kReadWrite), var, 1_u)
                              ->Result()));
        b.Return(func);
    });

    auto* src = R"(
SB = struct @align(8) {
  a:u16 @offset(0)
  b:vec2<u32> @offset(8)
}

$B1: {  # root
  %v:ptr<storage, SB, read_write> = var undef @binding_point(0, 0)
}

%foo = @fragment func():void {
  $B2: {
    %3:ptr<storage, u16, read_write> = access %v, 0u
    %4:u16 = load %3
    %a:u16 = let %4
    %6:ptr<storage, vec2<u32>, read_write> = access %v, 1u
    %7:vec2<u32> = load %6
    %b:vec2<u32> = let %7
    ret
  }
}
)";

    ASSERT_EQ(src, str());

    auto* expect = R"(
SB = struct @align(8) {
  a:u16 @offset(0)
  b:vec2<u32> @offset(8)
}

$B1: {  # root
  %v:ptr<storage, array<u16, 8>, read_write> = var undef @binding_point(0, 0)
}

%foo = @fragment func():void {
  $B2: {
    %3:ptr<storage, u16, read_write> = access %v, 0u
    %4:u16 = load %3
    %a:u16 = let %4
    %6:ptr<storage, u16, read_write> = access %v, 4u
    %7:u16 = load %6 @align(8)
    %8:ptr<storage, u16, read_write> = access %v, 5u
    %9:u16 = load %8
    %10:ptr<storage, u16, read_write> = access %v, 6u
    %11:u16 = load %10
    %12:ptr<storage, u16, read_write> = access %v, 7u
    %13:u16 = load %12
    %14:vec2<u16> = construct %7, %9
    %15:u32 = bitcast<u32> %14
    %16:vec2<u16> = construct %11, %13
    %17:u32 = bitcast<u32> %16
    %18:vec2<u32> = construct %15, %17
    %b:vec2<u32> = let %18
    ret
  }
}
)";

    DecomposeAccessConfig options{.storage = true};
    Run(DecomposeAccess, options);
    EXPECT_EQ(expect, str());
}

TEST_F(IR_DecomposeAccessTest, Storage_AccessU16_LoadVec3u) {
    auto* sb = ty.Struct(mod.symbols.New("SB"), {
                                                    {mod.symbols.New("a"), ty.u16()},
                                                    {mod.symbols.New("b"), ty.vec3u()},
                                                });
    auto* var = b.Var("v", storage, sb, core::Access::kReadWrite);
    var->SetBindingPoint(0, 0);
    b.ir.root_block->Append(var);

    auto* func = b.Function("foo", ty.void_(), core::ir::Function::PipelineStage::kFragment);
    b.Append(func->Block(), [&] {
        b.Let(
            "a",
            b.Load(
                b.Access(ty.ptr(storage, ty.u16(), core::Access::kReadWrite), var, 0_u)->Result()));
        b.Let("b", b.Load(b.Access(ty.ptr(storage, ty.vec3u(), core::Access::kReadWrite), var, 1_u)
                              ->Result()));
        b.Return(func);
    });

    auto* src = R"(
SB = struct @align(16) {
  a:u16 @offset(0)
  b:vec3<u32> @offset(16)
}

$B1: {  # root
  %v:ptr<storage, SB, read_write> = var undef @binding_point(0, 0)
}

%foo = @fragment func():void {
  $B2: {
    %3:ptr<storage, u16, read_write> = access %v, 0u
    %4:u16 = load %3
    %a:u16 = let %4
    %6:ptr<storage, vec3<u32>, read_write> = access %v, 1u
    %7:vec3<u32> = load %6
    %b:vec3<u32> = let %7
    ret
  }
}
)";

    ASSERT_EQ(src, str());

    auto* expect = R"(
SB = struct @align(16) {
  a:u16 @offset(0)
  b:vec3<u32> @offset(16)
}

$B1: {  # root
  %v:ptr<storage, array<u16, 16>, read_write> = var undef @binding_point(0, 0)
}

%foo = @fragment func():void {
  $B2: {
    %3:ptr<storage, u16, read_write> = access %v, 0u
    %4:u16 = load %3
    %a:u16 = let %4
    %6:ptr<storage, u16, read_write> = access %v, 8u
    %7:u16 = load %6 @align(16)
    %8:ptr<storage, u16, read_write> = access %v, 9u
    %9:u16 = load %8
    %10:ptr<storage, u16, read_write> = access %v, 10u
    %11:u16 = load %10
    %12:ptr<storage, u16, read_write> = access %v, 11u
    %13:u16 = load %12
    %14:ptr<storage, u16, read_write> = access %v, 12u
    %15:u16 = load %14
    %16:ptr<storage, u16, read_write> = access %v, 13u
    %17:u16 = load %16
    %18:vec2<u16> = construct %7, %9
    %19:u32 = bitcast<u32> %18
    %20:vec2<u16> = construct %11, %13
    %21:u32 = bitcast<u32> %20
    %22:vec2<u16> = construct %15, %17
    %23:u32 = bitcast<u32> %22
    %24:vec3<u32> = construct %19, %21, %23
    %25:vec3<u32> = swizzle %24, xyz
    %b:vec3<u32> = let %25
    ret
  }
}
)";

    DecomposeAccessConfig options{.storage = true};
    Run(DecomposeAccess, options);
    EXPECT_EQ(expect, str());
}

TEST_F(IR_DecomposeAccessTest, Storage_AccessU16_LoadVec4u) {
    auto* sb = ty.Struct(mod.symbols.New("SB"), {
                                                    {mod.symbols.New("a"), ty.u16()},
                                                    {mod.symbols.New("b"), ty.vec4u()},
                                                });
    auto* var = b.Var("v", storage, sb, core::Access::kReadWrite);
    var->SetBindingPoint(0, 0);
    b.ir.root_block->Append(var);

    auto* func = b.Function("foo", ty.void_(), core::ir::Function::PipelineStage::kFragment);
    b.Append(func->Block(), [&] {
        b.Let(
            "a",
            b.Load(
                b.Access(ty.ptr(storage, ty.u16(), core::Access::kReadWrite), var, 0_u)->Result()));
        b.Let("b", b.Load(b.Access(ty.ptr(storage, ty.vec4u(), core::Access::kReadWrite), var, 1_u)
                              ->Result()));
        b.Return(func);
    });

    auto* src = R"(
SB = struct @align(16) {
  a:u16 @offset(0)
  b:vec4<u32> @offset(16)
}

$B1: {  # root
  %v:ptr<storage, SB, read_write> = var undef @binding_point(0, 0)
}

%foo = @fragment func():void {
  $B2: {
    %3:ptr<storage, u16, read_write> = access %v, 0u
    %4:u16 = load %3
    %a:u16 = let %4
    %6:ptr<storage, vec4<u32>, read_write> = access %v, 1u
    %7:vec4<u32> = load %6
    %b:vec4<u32> = let %7
    ret
  }
}
)";

    ASSERT_EQ(src, str());

    auto* expect = R"(
SB = struct @align(16) {
  a:u16 @offset(0)
  b:vec4<u32> @offset(16)
}

$B1: {  # root
  %v:ptr<storage, array<u16, 16>, read_write> = var undef @binding_point(0, 0)
}

%foo = @fragment func():void {
  $B2: {
    %3:ptr<storage, u16, read_write> = access %v, 0u
    %4:u16 = load %3
    %a:u16 = let %4
    %6:ptr<storage, u16, read_write> = access %v, 8u
    %7:u16 = load %6 @align(16)
    %8:ptr<storage, u16, read_write> = access %v, 9u
    %9:u16 = load %8
    %10:ptr<storage, u16, read_write> = access %v, 10u
    %11:u16 = load %10
    %12:ptr<storage, u16, read_write> = access %v, 11u
    %13:u16 = load %12
    %14:ptr<storage, u16, read_write> = access %v, 12u
    %15:u16 = load %14
    %16:ptr<storage, u16, read_write> = access %v, 13u
    %17:u16 = load %16
    %18:ptr<storage, u16, read_write> = access %v, 14u
    %19:u16 = load %18
    %20:ptr<storage, u16, read_write> = access %v, 15u
    %21:u16 = load %20
    %22:vec2<u16> = construct %7, %9
    %23:u32 = bitcast<u32> %22
    %24:vec2<u16> = construct %11, %13
    %25:u32 = bitcast<u32> %24
    %26:vec2<u16> = construct %15, %17
    %27:u32 = bitcast<u32> %26
    %28:vec2<u16> = construct %19, %21
    %29:u32 = bitcast<u32> %28
    %30:vec4<u32> = construct %23, %25, %27, %29
    %b:vec4<u32> = let %30
    ret
  }
}
)";

    DecomposeAccessConfig options{.storage = true};
    Run(DecomposeAccess, options);
    EXPECT_EQ(expect, str());
}

TEST_F(IR_DecomposeAccessTest, Workgroup_AccessU16_LoadVec2b) {
    auto* sb = ty.Struct(mod.symbols.New("SB"), {
                                                    {mod.symbols.New("a"), ty.u16()},
                                                    {mod.symbols.New("b"), ty.vec2(ty.bool_())},
                                                });
    auto* var = b.Var("v", workgroup, sb, core::Access::kReadWrite);
    b.ir.root_block->Append(var);

    auto* func = b.Function("foo", ty.void_());
    b.Append(func->Block(), [&] {
        b.Let("a", b.Load(b.Access(ty.ptr(workgroup, ty.u16(), core::Access::kReadWrite), var, 0_u)
                              ->Result()));
        b.Let("b", b.Load(b.Access(ty.ptr(workgroup, ty.vec2(ty.bool_()), core::Access::kReadWrite),
                                   var, 1_u)
                              ->Result()));
        b.Return(func);
    });

    auto* src = R"(
SB = struct @align(8) {
  a:u16 @offset(0)
  b:vec2<bool> @offset(8)
}

$B1: {  # root
  %v:ptr<workgroup, SB, read_write> = var undef
}

%foo = func():void {
  $B2: {
    %3:ptr<workgroup, u16, read_write> = access %v, 0u
    %4:u16 = load %3
    %a:u16 = let %4
    %6:ptr<workgroup, vec2<bool>, read_write> = access %v, 1u
    %7:vec2<bool> = load %6
    %b:vec2<bool> = let %7
    ret
  }
}
)";

    ASSERT_EQ(src, str());

    auto* expect = R"(
SB = struct @align(8) {
  a:u16 @offset(0)
  b:vec2<bool> @offset(8)
}

$B1: {  # root
  %v:ptr<workgroup, array<u16, 8>, read_write> = var undef
}

%foo = func():void {
  $B2: {
    %3:ptr<workgroup, u16, read_write> = access %v, 0u
    %4:u16 = load %3
    %a:u16 = let %4
    %6:ptr<workgroup, u16, read_write> = access %v, 4u
    %7:u16 = load %6 @align(8)
    %8:ptr<workgroup, u16, read_write> = access %v, 5u
    %9:u16 = load %8
    %10:ptr<workgroup, u16, read_write> = access %v, 6u
    %11:u16 = load %10
    %12:ptr<workgroup, u16, read_write> = access %v, 7u
    %13:u16 = load %12
    %14:vec2<u16> = construct %7, %9
    %15:u32 = bitcast<u32> %14
    %16:vec2<u16> = construct %11, %13
    %17:u32 = bitcast<u32> %16
    %18:vec2<u32> = construct %15, %17
    %19:vec2<bool> = convert %18
    %b:vec2<bool> = let %19
    ret
  }
}
)";

    DecomposeAccessConfig options{.workgroup = true};
    Run(DecomposeAccess, options);
    EXPECT_EQ(expect, str());
}

TEST_F(IR_DecomposeAccessTest, Workgroup_AccessU16_LoadVec3b) {
    auto* sb = ty.Struct(mod.symbols.New("SB"), {
                                                    {mod.symbols.New("a"), ty.u16()},
                                                    {mod.symbols.New("b"), ty.vec3(ty.bool_())},
                                                });
    auto* var = b.Var("v", workgroup, sb, core::Access::kReadWrite);
    b.ir.root_block->Append(var);

    auto* func = b.Function("foo", ty.void_());
    b.Append(func->Block(), [&] {
        b.Let("a", b.Load(b.Access(ty.ptr(workgroup, ty.u16(), core::Access::kReadWrite), var, 0_u)
                              ->Result()));
        b.Let("b", b.Load(b.Access(ty.ptr(workgroup, ty.vec3(ty.bool_()), core::Access::kReadWrite),
                                   var, 1_u)
                              ->Result()));
        b.Return(func);
    });

    auto* src = R"(
SB = struct @align(16) {
  a:u16 @offset(0)
  b:vec3<bool> @offset(16)
}

$B1: {  # root
  %v:ptr<workgroup, SB, read_write> = var undef
}

%foo = func():void {
  $B2: {
    %3:ptr<workgroup, u16, read_write> = access %v, 0u
    %4:u16 = load %3
    %a:u16 = let %4
    %6:ptr<workgroup, vec3<bool>, read_write> = access %v, 1u
    %7:vec3<bool> = load %6
    %b:vec3<bool> = let %7
    ret
  }
}
)";

    ASSERT_EQ(src, str());

    auto* expect = R"(
SB = struct @align(16) {
  a:u16 @offset(0)
  b:vec3<bool> @offset(16)
}

$B1: {  # root
  %v:ptr<workgroup, array<u16, 16>, read_write> = var undef
}

%foo = func():void {
  $B2: {
    %3:ptr<workgroup, u16, read_write> = access %v, 0u
    %4:u16 = load %3
    %a:u16 = let %4
    %6:ptr<workgroup, u16, read_write> = access %v, 8u
    %7:u16 = load %6 @align(16)
    %8:ptr<workgroup, u16, read_write> = access %v, 9u
    %9:u16 = load %8
    %10:ptr<workgroup, u16, read_write> = access %v, 10u
    %11:u16 = load %10
    %12:ptr<workgroup, u16, read_write> = access %v, 11u
    %13:u16 = load %12
    %14:ptr<workgroup, u16, read_write> = access %v, 12u
    %15:u16 = load %14
    %16:ptr<workgroup, u16, read_write> = access %v, 13u
    %17:u16 = load %16
    %18:vec2<u16> = construct %7, %9
    %19:u32 = bitcast<u32> %18
    %20:vec2<u16> = construct %11, %13
    %21:u32 = bitcast<u32> %20
    %22:vec2<u16> = construct %15, %17
    %23:u32 = bitcast<u32> %22
    %24:vec3<u32> = construct %19, %21, %23
    %25:vec3<u32> = swizzle %24, xyz
    %26:vec3<bool> = convert %25
    %b:vec3<bool> = let %26
    ret
  }
}
)";

    DecomposeAccessConfig options{.workgroup = true};
    Run(DecomposeAccess, options);
    EXPECT_EQ(expect, str());
}

TEST_F(IR_DecomposeAccessTest, Workgroup_AccessU16_LoadVec4b) {
    auto* sb = ty.Struct(mod.symbols.New("SB"), {
                                                    {mod.symbols.New("a"), ty.u16()},
                                                    {mod.symbols.New("b"), ty.vec4(ty.bool_())},
                                                });
    auto* var = b.Var("v", workgroup, sb, core::Access::kReadWrite);
    b.ir.root_block->Append(var);

    auto* func = b.Function("foo", ty.void_());
    b.Append(func->Block(), [&] {
        b.Let("a", b.Load(b.Access(ty.ptr(workgroup, ty.u16(), core::Access::kReadWrite), var, 0_u)
                              ->Result()));
        b.Let("b", b.Load(b.Access(ty.ptr(workgroup, ty.vec4(ty.bool_()), core::Access::kReadWrite),
                                   var, 1_u)
                              ->Result()));
        b.Return(func);
    });

    auto* src = R"(
SB = struct @align(16) {
  a:u16 @offset(0)
  b:vec4<bool> @offset(16)
}

$B1: {  # root
  %v:ptr<workgroup, SB, read_write> = var undef
}

%foo = func():void {
  $B2: {
    %3:ptr<workgroup, u16, read_write> = access %v, 0u
    %4:u16 = load %3
    %a:u16 = let %4
    %6:ptr<workgroup, vec4<bool>, read_write> = access %v, 1u
    %7:vec4<bool> = load %6
    %b:vec4<bool> = let %7
    ret
  }
}
)";

    ASSERT_EQ(src, str());

    auto* expect = R"(
SB = struct @align(16) {
  a:u16 @offset(0)
  b:vec4<bool> @offset(16)
}

$B1: {  # root
  %v:ptr<workgroup, array<u16, 16>, read_write> = var undef
}

%foo = func():void {
  $B2: {
    %3:ptr<workgroup, u16, read_write> = access %v, 0u
    %4:u16 = load %3
    %a:u16 = let %4
    %6:ptr<workgroup, u16, read_write> = access %v, 8u
    %7:u16 = load %6 @align(16)
    %8:ptr<workgroup, u16, read_write> = access %v, 9u
    %9:u16 = load %8
    %10:ptr<workgroup, u16, read_write> = access %v, 10u
    %11:u16 = load %10
    %12:ptr<workgroup, u16, read_write> = access %v, 11u
    %13:u16 = load %12
    %14:ptr<workgroup, u16, read_write> = access %v, 12u
    %15:u16 = load %14
    %16:ptr<workgroup, u16, read_write> = access %v, 13u
    %17:u16 = load %16
    %18:ptr<workgroup, u16, read_write> = access %v, 14u
    %19:u16 = load %18
    %20:ptr<workgroup, u16, read_write> = access %v, 15u
    %21:u16 = load %20
    %22:vec2<u16> = construct %7, %9
    %23:u32 = bitcast<u32> %22
    %24:vec2<u16> = construct %11, %13
    %25:u32 = bitcast<u32> %24
    %26:vec2<u16> = construct %15, %17
    %27:u32 = bitcast<u32> %26
    %28:vec2<u16> = construct %19, %21
    %29:u32 = bitcast<u32> %28
    %30:vec4<u32> = construct %23, %25, %27, %29
    %31:vec4<bool> = convert %30
    %b:vec4<bool> = let %31
    ret
  }
}
)";

    DecomposeAccessConfig options{.workgroup = true};
    Run(DecomposeAccess, options);
    EXPECT_EQ(expect, str());
}

TEST_F(IR_DecomposeAccessTest, Storage_AccessU32_LoadVec2u) {
    auto* sb = ty.Struct(mod.symbols.New("SB"), {
                                                    {mod.symbols.New("a"), ty.u32()},
                                                    {mod.symbols.New("b"), ty.vec2u()},
                                                });
    auto* var = b.Var("v", storage, sb, core::Access::kReadWrite);
    var->SetBindingPoint(0, 0);
    b.ir.root_block->Append(var);

    auto* func = b.Function("foo", ty.void_(), core::ir::Function::PipelineStage::kFragment);
    b.Append(func->Block(), [&] {
        b.Let(
            "a",
            b.Load(
                b.Access(ty.ptr(storage, ty.u32(), core::Access::kReadWrite), var, 0_u)->Result()));
        b.Let("b", b.Load(b.Access(ty.ptr(storage, ty.vec2u(), core::Access::kReadWrite), var, 1_u)
                              ->Result()));
        b.Return(func);
    });

    auto* src = R"(
SB = struct @align(8) {
  a:u32 @offset(0)
  b:vec2<u32> @offset(8)
}

$B1: {  # root
  %v:ptr<storage, SB, read_write> = var undef @binding_point(0, 0)
}

%foo = @fragment func():void {
  $B2: {
    %3:ptr<storage, u32, read_write> = access %v, 0u
    %4:u32 = load %3
    %a:u32 = let %4
    %6:ptr<storage, vec2<u32>, read_write> = access %v, 1u
    %7:vec2<u32> = load %6
    %b:vec2<u32> = let %7
    ret
  }
}
)";

    ASSERT_EQ(src, str());

    auto* expect = R"(
SB = struct @align(8) {
  a:u32 @offset(0)
  b:vec2<u32> @offset(8)
}

$B1: {  # root
  %v:ptr<storage, array<u32, 4>, read_write> = var undef @binding_point(0, 0)
}

%foo = @fragment func():void {
  $B2: {
    %3:ptr<storage, u32, read_write> = access %v, 0u
    %4:u32 = load %3
    %a:u32 = let %4
    %6:ptr<storage, u32, read_write> = access %v, 2u
    %7:u32 = load %6 @align(8)
    %8:ptr<storage, u32, read_write> = access %v, 3u
    %9:u32 = load %8
    %10:vec2<u32> = construct %7, %9
    %b:vec2<u32> = let %10
    ret
  }
}
)";

    DecomposeAccessConfig options{.storage = true};
    Run(DecomposeAccess, options);
    EXPECT_EQ(expect, str());
}

TEST_F(IR_DecomposeAccessTest, Storage_AccessU32_LoadVec3u) {
    auto* sb = ty.Struct(mod.symbols.New("SB"), {
                                                    {mod.symbols.New("a"), ty.u32()},
                                                    {mod.symbols.New("b"), ty.vec3u()},
                                                });
    auto* var = b.Var("v", storage, sb, core::Access::kReadWrite);
    var->SetBindingPoint(0, 0);
    b.ir.root_block->Append(var);

    auto* func = b.Function("foo", ty.void_(), core::ir::Function::PipelineStage::kFragment);
    b.Append(func->Block(), [&] {
        b.Let(
            "a",
            b.Load(
                b.Access(ty.ptr(storage, ty.u32(), core::Access::kReadWrite), var, 0_u)->Result()));
        b.Let("b", b.Load(b.Access(ty.ptr(storage, ty.vec3u(), core::Access::kReadWrite), var, 1_u)
                              ->Result()));
        b.Return(func);
    });

    auto* src = R"(
SB = struct @align(16) {
  a:u32 @offset(0)
  b:vec3<u32> @offset(16)
}

$B1: {  # root
  %v:ptr<storage, SB, read_write> = var undef @binding_point(0, 0)
}

%foo = @fragment func():void {
  $B2: {
    %3:ptr<storage, u32, read_write> = access %v, 0u
    %4:u32 = load %3
    %a:u32 = let %4
    %6:ptr<storage, vec3<u32>, read_write> = access %v, 1u
    %7:vec3<u32> = load %6
    %b:vec3<u32> = let %7
    ret
  }
}
)";

    ASSERT_EQ(src, str());

    auto* expect = R"(
SB = struct @align(16) {
  a:u32 @offset(0)
  b:vec3<u32> @offset(16)
}

$B1: {  # root
  %v:ptr<storage, array<u32, 8>, read_write> = var undef @binding_point(0, 0)
}

%foo = @fragment func():void {
  $B2: {
    %3:ptr<storage, u32, read_write> = access %v, 0u
    %4:u32 = load %3
    %a:u32 = let %4
    %6:ptr<storage, u32, read_write> = access %v, 4u
    %7:u32 = load %6 @align(16)
    %8:ptr<storage, u32, read_write> = access %v, 5u
    %9:u32 = load %8
    %10:ptr<storage, u32, read_write> = access %v, 6u
    %11:u32 = load %10
    %12:vec3<u32> = construct %7, %9, %11
    %13:vec3<u32> = swizzle %12, xyz
    %b:vec3<u32> = let %13
    ret
  }
}
)";

    DecomposeAccessConfig options{.storage = true};
    Run(DecomposeAccess, options);
    EXPECT_EQ(expect, str());
}

TEST_F(IR_DecomposeAccessTest, Storage_AccessU32_LoadVec4u) {
    auto* sb = ty.Struct(mod.symbols.New("SB"), {
                                                    {mod.symbols.New("a"), ty.u32()},
                                                    {mod.symbols.New("b"), ty.vec4u()},
                                                });
    auto* var = b.Var("v", storage, sb, core::Access::kReadWrite);
    var->SetBindingPoint(0, 0);
    b.ir.root_block->Append(var);

    auto* func = b.Function("foo", ty.void_(), core::ir::Function::PipelineStage::kFragment);
    b.Append(func->Block(), [&] {
        b.Let(
            "a",
            b.Load(
                b.Access(ty.ptr(storage, ty.u32(), core::Access::kReadWrite), var, 0_u)->Result()));
        b.Let("b", b.Load(b.Access(ty.ptr(storage, ty.vec4u(), core::Access::kReadWrite), var, 1_u)
                              ->Result()));
        b.Return(func);
    });

    auto* src = R"(
SB = struct @align(16) {
  a:u32 @offset(0)
  b:vec4<u32> @offset(16)
}

$B1: {  # root
  %v:ptr<storage, SB, read_write> = var undef @binding_point(0, 0)
}

%foo = @fragment func():void {
  $B2: {
    %3:ptr<storage, u32, read_write> = access %v, 0u
    %4:u32 = load %3
    %a:u32 = let %4
    %6:ptr<storage, vec4<u32>, read_write> = access %v, 1u
    %7:vec4<u32> = load %6
    %b:vec4<u32> = let %7
    ret
  }
}
)";

    ASSERT_EQ(src, str());

    auto* expect = R"(
SB = struct @align(16) {
  a:u32 @offset(0)
  b:vec4<u32> @offset(16)
}

$B1: {  # root
  %v:ptr<storage, array<u32, 8>, read_write> = var undef @binding_point(0, 0)
}

%foo = @fragment func():void {
  $B2: {
    %3:ptr<storage, u32, read_write> = access %v, 0u
    %4:u32 = load %3
    %a:u32 = let %4
    %6:ptr<storage, u32, read_write> = access %v, 4u
    %7:u32 = load %6 @align(16)
    %8:ptr<storage, u32, read_write> = access %v, 5u
    %9:u32 = load %8
    %10:ptr<storage, u32, read_write> = access %v, 6u
    %11:u32 = load %10
    %12:ptr<storage, u32, read_write> = access %v, 7u
    %13:u32 = load %12
    %14:vec4<u32> = construct %7, %9, %11, %13
    %b:vec4<u32> = let %14
    ret
  }
}
)";

    DecomposeAccessConfig options{.storage = true};
    Run(DecomposeAccess, options);
    EXPECT_EQ(expect, str());
}

TEST_F(IR_DecomposeAccessTest, Workgroup_AccessU32_LoadVec2b) {
    auto* sb = ty.Struct(mod.symbols.New("SB"), {
                                                    {mod.symbols.New("a"), ty.u32()},
                                                    {mod.symbols.New("b"), ty.vec2(ty.bool_())},
                                                });
    auto* var = b.Var("v", workgroup, sb, core::Access::kReadWrite);
    b.ir.root_block->Append(var);

    auto* func = b.Function("foo", ty.void_());
    b.Append(func->Block(), [&] {
        b.Let("a", b.Load(b.Access(ty.ptr(workgroup, ty.u32(), core::Access::kReadWrite), var, 0_u)
                              ->Result()));
        b.Let("b", b.Load(b.Access(ty.ptr(workgroup, ty.vec2(ty.bool_()), core::Access::kReadWrite),
                                   var, 1_u)
                              ->Result()));
        b.Return(func);
    });

    auto* src = R"(
SB = struct @align(8) {
  a:u32 @offset(0)
  b:vec2<bool> @offset(8)
}

$B1: {  # root
  %v:ptr<workgroup, SB, read_write> = var undef
}

%foo = func():void {
  $B2: {
    %3:ptr<workgroup, u32, read_write> = access %v, 0u
    %4:u32 = load %3
    %a:u32 = let %4
    %6:ptr<workgroup, vec2<bool>, read_write> = access %v, 1u
    %7:vec2<bool> = load %6
    %b:vec2<bool> = let %7
    ret
  }
}
)";

    ASSERT_EQ(src, str());

    auto* expect = R"(
SB = struct @align(8) {
  a:u32 @offset(0)
  b:vec2<bool> @offset(8)
}

$B1: {  # root
  %v:ptr<workgroup, array<u32, 4>, read_write> = var undef
}

%foo = func():void {
  $B2: {
    %3:ptr<workgroup, u32, read_write> = access %v, 0u
    %4:u32 = load %3
    %a:u32 = let %4
    %6:ptr<workgroup, u32, read_write> = access %v, 2u
    %7:u32 = load %6 @align(8)
    %8:ptr<workgroup, u32, read_write> = access %v, 3u
    %9:u32 = load %8
    %10:vec2<u32> = construct %7, %9
    %11:vec2<bool> = convert %10
    %b:vec2<bool> = let %11
    ret
  }
}
)";

    DecomposeAccessConfig options{.workgroup = true};
    Run(DecomposeAccess, options);
    EXPECT_EQ(expect, str());
}

TEST_F(IR_DecomposeAccessTest, Workgroup_AccessU32_LoadVec3b) {
    auto* sb = ty.Struct(mod.symbols.New("SB"), {
                                                    {mod.symbols.New("a"), ty.u32()},
                                                    {mod.symbols.New("b"), ty.vec3(ty.bool_())},
                                                });
    auto* var = b.Var("v", workgroup, sb, core::Access::kReadWrite);
    b.ir.root_block->Append(var);

    auto* func = b.Function("foo", ty.void_());
    b.Append(func->Block(), [&] {
        b.Let("a", b.Load(b.Access(ty.ptr(workgroup, ty.u32(), core::Access::kReadWrite), var, 0_u)
                              ->Result()));
        b.Let("b", b.Load(b.Access(ty.ptr(workgroup, ty.vec3(ty.bool_()), core::Access::kReadWrite),
                                   var, 1_u)
                              ->Result()));
        b.Return(func);
    });

    auto* src = R"(
SB = struct @align(16) {
  a:u32 @offset(0)
  b:vec3<bool> @offset(16)
}

$B1: {  # root
  %v:ptr<workgroup, SB, read_write> = var undef
}

%foo = func():void {
  $B2: {
    %3:ptr<workgroup, u32, read_write> = access %v, 0u
    %4:u32 = load %3
    %a:u32 = let %4
    %6:ptr<workgroup, vec3<bool>, read_write> = access %v, 1u
    %7:vec3<bool> = load %6
    %b:vec3<bool> = let %7
    ret
  }
}
)";

    ASSERT_EQ(src, str());

    auto* expect = R"(
SB = struct @align(16) {
  a:u32 @offset(0)
  b:vec3<bool> @offset(16)
}

$B1: {  # root
  %v:ptr<workgroup, array<u32, 8>, read_write> = var undef
}

%foo = func():void {
  $B2: {
    %3:ptr<workgroup, u32, read_write> = access %v, 0u
    %4:u32 = load %3
    %a:u32 = let %4
    %6:ptr<workgroup, u32, read_write> = access %v, 4u
    %7:u32 = load %6 @align(16)
    %8:ptr<workgroup, u32, read_write> = access %v, 5u
    %9:u32 = load %8
    %10:ptr<workgroup, u32, read_write> = access %v, 6u
    %11:u32 = load %10
    %12:vec3<u32> = construct %7, %9, %11
    %13:vec3<u32> = swizzle %12, xyz
    %14:vec3<bool> = convert %13
    %b:vec3<bool> = let %14
    ret
  }
}
)";

    DecomposeAccessConfig options{.workgroup = true};
    Run(DecomposeAccess, options);
    EXPECT_EQ(expect, str());
}

TEST_F(IR_DecomposeAccessTest, Workgroup_AccessU32_LoadVec4b) {
    auto* sb = ty.Struct(mod.symbols.New("SB"), {
                                                    {mod.symbols.New("a"), ty.u32()},
                                                    {mod.symbols.New("b"), ty.vec4(ty.bool_())},
                                                });
    auto* var = b.Var("v", workgroup, sb, core::Access::kReadWrite);
    b.ir.root_block->Append(var);

    auto* func = b.Function("foo", ty.void_());
    b.Append(func->Block(), [&] {
        b.Let("a", b.Load(b.Access(ty.ptr(workgroup, ty.u32(), core::Access::kReadWrite), var, 0_u)
                              ->Result()));
        b.Let("b", b.Load(b.Access(ty.ptr(workgroup, ty.vec4(ty.bool_()), core::Access::kReadWrite),
                                   var, 1_u)
                              ->Result()));
        b.Return(func);
    });

    auto* src = R"(
SB = struct @align(16) {
  a:u32 @offset(0)
  b:vec4<bool> @offset(16)
}

$B1: {  # root
  %v:ptr<workgroup, SB, read_write> = var undef
}

%foo = func():void {
  $B2: {
    %3:ptr<workgroup, u32, read_write> = access %v, 0u
    %4:u32 = load %3
    %a:u32 = let %4
    %6:ptr<workgroup, vec4<bool>, read_write> = access %v, 1u
    %7:vec4<bool> = load %6
    %b:vec4<bool> = let %7
    ret
  }
}
)";

    ASSERT_EQ(src, str());

    auto* expect = R"(
SB = struct @align(16) {
  a:u32 @offset(0)
  b:vec4<bool> @offset(16)
}

$B1: {  # root
  %v:ptr<workgroup, array<u32, 8>, read_write> = var undef
}

%foo = func():void {
  $B2: {
    %3:ptr<workgroup, u32, read_write> = access %v, 0u
    %4:u32 = load %3
    %a:u32 = let %4
    %6:ptr<workgroup, u32, read_write> = access %v, 4u
    %7:u32 = load %6 @align(16)
    %8:ptr<workgroup, u32, read_write> = access %v, 5u
    %9:u32 = load %8
    %10:ptr<workgroup, u32, read_write> = access %v, 6u
    %11:u32 = load %10
    %12:ptr<workgroup, u32, read_write> = access %v, 7u
    %13:u32 = load %12
    %14:vec4<u32> = construct %7, %9, %11, %13
    %15:vec4<bool> = convert %14
    %b:vec4<bool> = let %15
    ret
  }
}
)";

    DecomposeAccessConfig options{.workgroup = true};
    Run(DecomposeAccess, options);
    EXPECT_EQ(expect, str());
}

TEST_F(IR_DecomposeAccessTest, Storage_AccessVec2u_LoadVec2u) {
    auto* sb = ty.Struct(mod.symbols.New("SB"), {
                                                    {mod.symbols.New("a"), ty.vec2u()},
                                                    {mod.symbols.New("b"), ty.vec2u()},
                                                });
    auto* var = b.Var("v", storage, sb, core::Access::kReadWrite);
    var->SetBindingPoint(0, 0);
    b.ir.root_block->Append(var);

    auto* func = b.Function("foo", ty.void_(), core::ir::Function::PipelineStage::kFragment);
    b.Append(func->Block(), [&] {
        b.Let("a", b.Load(b.Access(ty.ptr(storage, ty.vec2u(), core::Access::kReadWrite), var, 0_u)
                              ->Result()));
        b.Let("b", b.Load(b.Access(ty.ptr(storage, ty.vec2u(), core::Access::kReadWrite), var, 1_u)
                              ->Result()));
        b.Return(func);
    });

    auto* src = R"(
SB = struct @align(8) {
  a:vec2<u32> @offset(0)
  b:vec2<u32> @offset(8)
}

$B1: {  # root
  %v:ptr<storage, SB, read_write> = var undef @binding_point(0, 0)
}

%foo = @fragment func():void {
  $B2: {
    %3:ptr<storage, vec2<u32>, read_write> = access %v, 0u
    %4:vec2<u32> = load %3
    %a:vec2<u32> = let %4
    %6:ptr<storage, vec2<u32>, read_write> = access %v, 1u
    %7:vec2<u32> = load %6
    %b:vec2<u32> = let %7
    ret
  }
}
)";

    ASSERT_EQ(src, str());

    auto* expect = R"(
SB = struct @align(8) {
  a:vec2<u32> @offset(0)
  b:vec2<u32> @offset(8)
}

$B1: {  # root
  %v:ptr<storage, array<vec2<u32>, 2>, read_write> = var undef @binding_point(0, 0)
}

%foo = @fragment func():void {
  $B2: {
    %3:ptr<storage, vec2<u32>, read_write> = access %v, 0u
    %4:vec2<u32> = load %3
    %a:vec2<u32> = let %4
    %6:ptr<storage, vec2<u32>, read_write> = access %v, 1u
    %7:vec2<u32> = load %6
    %b:vec2<u32> = let %7
    ret
  }
}
)";

    DecomposeAccessConfig options{.storage = true};
    Run(DecomposeAccess, options);
    EXPECT_EQ(expect, str());
}

TEST_F(IR_DecomposeAccessTest, Storage_AccessVec2u_LoadVec3u) {
    auto* sb = ty.Struct(mod.symbols.New("SB"), {
                                                    {mod.symbols.New("a"), ty.vec2u()},
                                                    {mod.symbols.New("b"), ty.vec3u()},
                                                });
    auto* var = b.Var("v", storage, sb, core::Access::kReadWrite);
    var->SetBindingPoint(0, 0);
    b.ir.root_block->Append(var);

    auto* func = b.Function("foo", ty.void_(), core::ir::Function::PipelineStage::kFragment);
    b.Append(func->Block(), [&] {
        b.Let("a", b.Load(b.Access(ty.ptr(storage, ty.vec2u(), core::Access::kReadWrite), var, 0_u)
                              ->Result()));
        b.Let("b", b.Load(b.Access(ty.ptr(storage, ty.vec3u(), core::Access::kReadWrite), var, 1_u)
                              ->Result()));
        b.Return(func);
    });

    auto* src = R"(
SB = struct @align(16) {
  a:vec2<u32> @offset(0)
  b:vec3<u32> @offset(16)
}

$B1: {  # root
  %v:ptr<storage, SB, read_write> = var undef @binding_point(0, 0)
}

%foo = @fragment func():void {
  $B2: {
    %3:ptr<storage, vec2<u32>, read_write> = access %v, 0u
    %4:vec2<u32> = load %3
    %a:vec2<u32> = let %4
    %6:ptr<storage, vec3<u32>, read_write> = access %v, 1u
    %7:vec3<u32> = load %6
    %b:vec3<u32> = let %7
    ret
  }
}
)";

    ASSERT_EQ(src, str());

    // The base type of the variable drops to u32 here since 2 vec2u is too large.
    auto* expect = R"(
SB = struct @align(16) {
  a:vec2<u32> @offset(0)
  b:vec3<u32> @offset(16)
}

$B1: {  # root
  %v:ptr<storage, array<u32, 8>, read_write> = var undef @binding_point(0, 0)
}

%foo = @fragment func():void {
  $B2: {
    %3:ptr<storage, u32, read_write> = access %v, 0u
    %4:u32 = load %3 @align(8)
    %5:ptr<storage, u32, read_write> = access %v, 1u
    %6:u32 = load %5
    %7:vec2<u32> = construct %4, %6
    %a:vec2<u32> = let %7
    %9:ptr<storage, u32, read_write> = access %v, 4u
    %10:u32 = load %9 @align(16)
    %11:ptr<storage, u32, read_write> = access %v, 5u
    %12:u32 = load %11
    %13:ptr<storage, u32, read_write> = access %v, 6u
    %14:u32 = load %13
    %15:vec3<u32> = construct %10, %12, %14
    %16:vec3<u32> = swizzle %15, xyz
    %b:vec3<u32> = let %16
    ret
  }
}
)";

    DecomposeAccessConfig options{.storage = true};
    Run(DecomposeAccess, options);
    EXPECT_EQ(expect, str());
}

TEST_F(IR_DecomposeAccessTest, Storage_AccessVec2u_LoadVec4u) {
    auto* sb = ty.Struct(mod.symbols.New("SB"), {
                                                    {mod.symbols.New("a"), ty.vec2u()},
                                                    {mod.symbols.New("b"), ty.vec4u()},
                                                });
    auto* var = b.Var("v", storage, sb, core::Access::kReadWrite);
    var->SetBindingPoint(0, 0);
    b.ir.root_block->Append(var);

    auto* func = b.Function("foo", ty.void_(), core::ir::Function::PipelineStage::kFragment);
    b.Append(func->Block(), [&] {
        b.Let("a", b.Load(b.Access(ty.ptr(storage, ty.vec2u(), core::Access::kReadWrite), var, 0_u)
                              ->Result()));
        b.Let("b", b.Load(b.Access(ty.ptr(storage, ty.vec4u(), core::Access::kReadWrite), var, 1_u)
                              ->Result()));
        b.Return(func);
    });

    auto* src = R"(
SB = struct @align(16) {
  a:vec2<u32> @offset(0)
  b:vec4<u32> @offset(16)
}

$B1: {  # root
  %v:ptr<storage, SB, read_write> = var undef @binding_point(0, 0)
}

%foo = @fragment func():void {
  $B2: {
    %3:ptr<storage, vec2<u32>, read_write> = access %v, 0u
    %4:vec2<u32> = load %3
    %a:vec2<u32> = let %4
    %6:ptr<storage, vec4<u32>, read_write> = access %v, 1u
    %7:vec4<u32> = load %6
    %b:vec4<u32> = let %7
    ret
  }
}
)";

    ASSERT_EQ(src, str());

    auto* expect = R"(
SB = struct @align(16) {
  a:vec2<u32> @offset(0)
  b:vec4<u32> @offset(16)
}

$B1: {  # root
  %v:ptr<storage, array<vec2<u32>, 4>, read_write> = var undef @binding_point(0, 0)
}

%foo = @fragment func():void {
  $B2: {
    %3:ptr<storage, vec2<u32>, read_write> = access %v, 0u
    %4:vec2<u32> = load %3
    %a:vec2<u32> = let %4
    %6:ptr<storage, vec2<u32>, read_write> = access %v, 2u
    %7:vec2<u32> = load %6 @align(16)
    %8:ptr<storage, vec2<u32>, read_write> = access %v, 3u
    %9:vec2<u32> = load %8
    %10:vec4<u32> = construct %7, %9
    %b:vec4<u32> = let %10
    ret
  }
}
)";

    DecomposeAccessConfig options{.storage = true};
    Run(DecomposeAccess, options);
    EXPECT_EQ(expect, str());
}

TEST_F(IR_DecomposeAccessTest, Workgroup_AccessVec2u_LoadVec2b) {
    auto* sb = ty.Struct(mod.symbols.New("SB"), {
                                                    {mod.symbols.New("a"), ty.vec2u()},
                                                    {mod.symbols.New("b"), ty.vec2(ty.bool_())},
                                                });
    auto* var = b.Var("v", workgroup, sb, core::Access::kReadWrite);
    b.ir.root_block->Append(var);

    auto* func = b.Function("foo", ty.void_());
    b.Append(func->Block(), [&] {
        b.Let("a",
              b.Load(b.Access(ty.ptr(workgroup, ty.vec2u(), core::Access::kReadWrite), var, 0_u)
                         ->Result()));
        b.Let("b", b.Load(b.Access(ty.ptr(workgroup, ty.vec2(ty.bool_()), core::Access::kReadWrite),
                                   var, 1_u)
                              ->Result()));
        b.Return(func);
    });

    auto* src = R"(
SB = struct @align(8) {
  a:vec2<u32> @offset(0)
  b:vec2<bool> @offset(8)
}

$B1: {  # root
  %v:ptr<workgroup, SB, read_write> = var undef
}

%foo = func():void {
  $B2: {
    %3:ptr<workgroup, vec2<u32>, read_write> = access %v, 0u
    %4:vec2<u32> = load %3
    %a:vec2<u32> = let %4
    %6:ptr<workgroup, vec2<bool>, read_write> = access %v, 1u
    %7:vec2<bool> = load %6
    %b:vec2<bool> = let %7
    ret
  }
}
)";

    ASSERT_EQ(src, str());

    auto* expect = R"(
SB = struct @align(8) {
  a:vec2<u32> @offset(0)
  b:vec2<bool> @offset(8)
}

$B1: {  # root
  %v:ptr<workgroup, array<vec2<u32>, 2>, read_write> = var undef
}

%foo = func():void {
  $B2: {
    %3:ptr<workgroup, vec2<u32>, read_write> = access %v, 0u
    %4:vec2<u32> = load %3
    %a:vec2<u32> = let %4
    %6:ptr<workgroup, vec2<u32>, read_write> = access %v, 1u
    %7:vec2<u32> = load %6
    %8:vec2<bool> = convert %7
    %b:vec2<bool> = let %8
    ret
  }
}
)";

    DecomposeAccessConfig options{.workgroup = true};
    Run(DecomposeAccess, options);
    EXPECT_EQ(expect, str());
}

TEST_F(IR_DecomposeAccessTest, Workgroup_AccessVec2u_LoadVec3b) {
    auto* sb = ty.Struct(mod.symbols.New("SB"), {
                                                    {mod.symbols.New("a"), ty.vec2u()},
                                                    {mod.symbols.New("b"), ty.vec3(ty.bool_())},
                                                });
    auto* var = b.Var("v", workgroup, sb, core::Access::kReadWrite);
    b.ir.root_block->Append(var);

    auto* func = b.Function("foo", ty.void_());
    b.Append(func->Block(), [&] {
        b.Let("a",
              b.Load(b.Access(ty.ptr(workgroup, ty.vec2u(), core::Access::kReadWrite), var, 0_u)
                         ->Result()));
        b.Let("b", b.Load(b.Access(ty.ptr(workgroup, ty.vec3(ty.bool_()), core::Access::kReadWrite),
                                   var, 1_u)
                              ->Result()));
        b.Return(func);
    });

    auto* src = R"(
SB = struct @align(16) {
  a:vec2<u32> @offset(0)
  b:vec3<bool> @offset(16)
}

$B1: {  # root
  %v:ptr<workgroup, SB, read_write> = var undef
}

%foo = func():void {
  $B2: {
    %3:ptr<workgroup, vec2<u32>, read_write> = access %v, 0u
    %4:vec2<u32> = load %3
    %a:vec2<u32> = let %4
    %6:ptr<workgroup, vec3<bool>, read_write> = access %v, 1u
    %7:vec3<bool> = load %6
    %b:vec3<bool> = let %7
    ret
  }
}
)";

    ASSERT_EQ(src, str());

    // The base type of the variable drops to u32 here since 2 vec2u is too large.
    auto* expect = R"(
SB = struct @align(16) {
  a:vec2<u32> @offset(0)
  b:vec3<bool> @offset(16)
}

$B1: {  # root
  %v:ptr<workgroup, array<u32, 8>, read_write> = var undef
}

%foo = func():void {
  $B2: {
    %3:ptr<workgroup, u32, read_write> = access %v, 0u
    %4:u32 = load %3 @align(8)
    %5:ptr<workgroup, u32, read_write> = access %v, 1u
    %6:u32 = load %5
    %7:vec2<u32> = construct %4, %6
    %a:vec2<u32> = let %7
    %9:ptr<workgroup, u32, read_write> = access %v, 4u
    %10:u32 = load %9 @align(16)
    %11:ptr<workgroup, u32, read_write> = access %v, 5u
    %12:u32 = load %11
    %13:ptr<workgroup, u32, read_write> = access %v, 6u
    %14:u32 = load %13
    %15:vec3<u32> = construct %10, %12, %14
    %16:vec3<u32> = swizzle %15, xyz
    %17:vec3<bool> = convert %16
    %b:vec3<bool> = let %17
    ret
  }
}
)";

    DecomposeAccessConfig options{.workgroup = true};
    Run(DecomposeAccess, options);
    EXPECT_EQ(expect, str());
}

TEST_F(IR_DecomposeAccessTest, Workgroup_AccessVec2u_LoadVec4b) {
    auto* sb = ty.Struct(mod.symbols.New("SB"), {
                                                    {mod.symbols.New("a"), ty.vec2u()},
                                                    {mod.symbols.New("b"), ty.vec4(ty.bool_())},
                                                });
    auto* var = b.Var("v", workgroup, sb, core::Access::kReadWrite);
    b.ir.root_block->Append(var);

    auto* func = b.Function("foo", ty.void_());
    b.Append(func->Block(), [&] {
        b.Let("a",
              b.Load(b.Access(ty.ptr(workgroup, ty.vec2u(), core::Access::kReadWrite), var, 0_u)
                         ->Result()));
        b.Let("b", b.Load(b.Access(ty.ptr(workgroup, ty.vec4(ty.bool_()), core::Access::kReadWrite),
                                   var, 1_u)
                              ->Result()));
        b.Return(func);
    });

    auto* src = R"(
SB = struct @align(16) {
  a:vec2<u32> @offset(0)
  b:vec4<bool> @offset(16)
}

$B1: {  # root
  %v:ptr<workgroup, SB, read_write> = var undef
}

%foo = func():void {
  $B2: {
    %3:ptr<workgroup, vec2<u32>, read_write> = access %v, 0u
    %4:vec2<u32> = load %3
    %a:vec2<u32> = let %4
    %6:ptr<workgroup, vec4<bool>, read_write> = access %v, 1u
    %7:vec4<bool> = load %6
    %b:vec4<bool> = let %7
    ret
  }
}
)";

    ASSERT_EQ(src, str());

    auto* expect = R"(
SB = struct @align(16) {
  a:vec2<u32> @offset(0)
  b:vec4<bool> @offset(16)
}

$B1: {  # root
  %v:ptr<workgroup, array<vec2<u32>, 4>, read_write> = var undef
}

%foo = func():void {
  $B2: {
    %3:ptr<workgroup, vec2<u32>, read_write> = access %v, 0u
    %4:vec2<u32> = load %3
    %a:vec2<u32> = let %4
    %6:ptr<workgroup, vec2<u32>, read_write> = access %v, 2u
    %7:vec2<u32> = load %6 @align(16)
    %8:ptr<workgroup, vec2<u32>, read_write> = access %v, 3u
    %9:vec2<u32> = load %8
    %10:vec4<u32> = construct %7, %9
    %11:vec4<bool> = convert %10
    %b:vec4<bool> = let %11
    ret
  }
}
)";

    DecomposeAccessConfig options{.workgroup = true};
    Run(DecomposeAccess, options);
    EXPECT_EQ(expect, str());
}

TEST_F(IR_DecomposeAccessTest, Workgroup_AccessU16_LoadStruct) {
    auto* sb = ty.Struct(mod.symbols.New("SB"), {
                                                    {mod.symbols.New("a"), ty.u16()},
                                                    {mod.symbols.New("b"), ty.array(ty.u32(), 2_u)},
                                                });
    auto* var = b.Var("v", workgroup, sb, core::Access::kReadWrite);
    b.ir.root_block->Append(var);

    auto* func = b.Function("foo", ty.void_());
    b.Append(func->Block(), [&] {
        b.Let("a", b.Load(var)->Result());
        b.Return(func);
    });

    auto* src = R"(
SB = struct @align(4) {
  a:u16 @offset(0)
  b:array<u32, 2> @offset(4)
}

$B1: {  # root
  %v:ptr<workgroup, SB, read_write> = var undef
}

%foo = func():void {
  $B2: {
    %3:SB = load %v
    %a:SB = let %3
    ret
  }
}
)";

    ASSERT_EQ(src, str());

    auto* expect = R"(
SB = struct @align(4) {
  a:u16 @offset(0)
  b:array<u32, 2> @offset(4)
}

$B1: {  # root
  %v:ptr<workgroup, array<u16, 6>, read_write> = var undef
}

%foo = func():void {
  $B2: {
    %3:SB = call %4, 0u
    %a:SB = let %3
    ret
  }
}
%4 = func(%start_byte_offset:u32):SB {
  $B3: {
    %7:u32 = div %start_byte_offset, 2u
    %8:ptr<workgroup, u16, read_write> = access %v, %7
    %9:u16 = load %8
    %10:u32 = add 4u, %start_byte_offset
    %11:array<u32, 2> = call %12, %10
    %13:SB = construct %9, %11
    ret %13
  }
}
%12 = func(%start_byte_offset_1:u32):array<u32, 2> {  # %start_byte_offset_1: 'start_byte_offset'
  $B4: {
    %a_1:ptr<function, array<u32, 2>, read_write> = var array<u32, 2>(0u)  # %a_1: 'a'
    loop [i: $B5, b: $B6, c: $B7] {  # loop_1
      $B5: {  # initializer
        next_iteration 0u  # -> $B6
      }
      $B6 (%idx:u32): {  # body
        %17:bool = gte %idx, 2u
        if %17 [t: $B8] {  # if_1
          $B8: {  # true
            exit_loop  # loop_1
          }
        }
        %18:u32 = mul %idx, 4u
        %19:u32 = add %start_byte_offset_1, %18
        %20:ptr<function, u32, read_write> = access %a_1, %idx
        %21:u32 = div %19, 2u
        %22:ptr<workgroup, u16, read_write> = access %v, %21
        %23:u16 = load %22 @align(4)
        %24:u32 = add %21, 1u
        %25:ptr<workgroup, u16, read_write> = access %v, %24
        %26:u16 = load %25
        %27:vec2<u16> = construct %23, %26
        %28:u32 = bitcast<u32> %27
        store %20, %28
        continue  # -> $B7
      }
      $B7: {  # continuing
        %29:u32 = add %idx, 1u
        next_iteration %29  # -> $B6
      }
    }
    %30:array<u32, 2> = load %a_1
    ret %30
  }
}
)";

    DecomposeAccessConfig options{.workgroup = true};
    Run(DecomposeAccess, options);
    EXPECT_EQ(expect, str());
}

TEST_F(IR_DecomposeAccessTest, Workgroup_AccessU16_StoreStruct) {
    auto* sb = ty.Struct(mod.symbols.New("SB"), {
                                                    {mod.symbols.New("a"), ty.u16()},
                                                    {mod.symbols.New("b"), ty.array(ty.u32(), 2_u)},
                                                });
    auto* var = b.Var("v", workgroup, sb, core::Access::kReadWrite);
    b.ir.root_block->Append(var);

    auto* func = b.Function("foo", ty.void_());
    b.Append(func->Block(), [&] {
        b.Store(var, b.Zero(sb));
        b.Return(func);
    });

    auto* src = R"(
SB = struct @align(4) {
  a:u16 @offset(0)
  b:array<u32, 2> @offset(4)
}

$B1: {  # root
  %v:ptr<workgroup, SB, read_write> = var undef
}

%foo = func():void {
  $B2: {
    store %v, SB(0u16, array<u32, 2>(0u))
    ret
  }
}
)";

    ASSERT_EQ(src, str());

    auto* expect = R"(
SB = struct @align(4) {
  a:u16 @offset(0)
  b:array<u32, 2> @offset(4)
}

$B1: {  # root
  %v:ptr<workgroup, array<u16, 6>, read_write> = var undef
}

%foo = func():void {
  $B2: {
    %3:void = call %4, 0u, SB(0u16, array<u32, 2>(0u))
    ret
  }
}
%4 = func(%start_byte_offset:u32, %object:SB):void {
  $B3: {
    %7:u16 = access %object, 0u
    %8:u32 = div %start_byte_offset, 2u
    %9:ptr<workgroup, u16, read_write> = access %v, %8
    store %9, %7
    %10:u32 = add 4u, %start_byte_offset
    %11:array<u32, 2> = access %object, 1u
    %12:void = call %13, %10, %11
    ret
  }
}
%13 = func(%start_byte_offset_1:u32, %object_1:array<u32, 2>):void {  # %start_byte_offset_1: 'start_byte_offset', %object_1: 'object'
  $B4: {
    loop [i: $B5, b: $B6, c: $B7] {  # loop_1
      $B5: {  # initializer
        next_iteration 0u  # -> $B6
      }
      $B6 (%idx:u32): {  # body
        %17:bool = gte %idx, 2u
        if %17 [t: $B8] {  # if_1
          $B8: {  # true
            exit_loop  # loop_1
          }
        }
        %18:u32 = mul %idx, 4u
        %19:u32 = add %start_byte_offset_1, %18
        %20:u32 = access %object_1, %idx
        %21:u32 = div %19, 2u
        %22:vec2<u16> = bitcast<vec2<u16>> %20
        %23:ptr<workgroup, u16, read_write> = access %v, %21
        %24:u16 = access %22, 0u
        store %23, %24 @align(4)
        %25:u32 = add %21, 1u
        %26:ptr<workgroup, u16, read_write> = access %v, %25
        %27:u16 = access %22, 1u
        store %26, %27
        continue  # -> $B7
      }
      $B7: {  # continuing
        %28:u32 = add %idx, 1u
        next_iteration %28  # -> $B6
      }
    }
    ret
  }
}
)";

    DecomposeAccessConfig options{.workgroup = true};
    Run(DecomposeAccess, options);
    EXPECT_EQ(expect, str());
}

TEST_F(IR_DecomposeAccessTest, Store_AccessU16_StoreF16) {
    auto* var = b.Var("v", storage, ty.f16(), core::Access::kReadWrite);
    var->SetBindingPoint(0, 0);
    b.ir.root_block->Append(var);

    auto* func = b.Function("foo", ty.void_(), core::ir::Function::PipelineStage::kFragment);
    b.Append(func->Block(), [&] {
        b.Store(var, b.Constant(f16(0)));
        b.Return(func);
    });

    auto* src = R"(
$B1: {  # root
  %v:ptr<storage, f16, read_write> = var undef @binding_point(0, 0)
}

%foo = @fragment func():void {
  $B2: {
    store %v, 0.0h
    ret
  }
}
)";

    ASSERT_EQ(src, str());

    auto* expect = R"(
$B1: {  # root
  %v:ptr<storage, array<u16, 1>, read_write> = var undef @binding_point(0, 0)
}

%foo = @fragment func():void {
  $B2: {
    %3:u16 = bitcast<u16> 0.0h
    %4:ptr<storage, u16, read_write> = access %v, 0u
    store %4, %3
    ret
  }
}
)";

    DecomposeAccessConfig options{.storage = true};
    Run(DecomposeAccess, options);
    EXPECT_EQ(expect, str());
}

TEST_F(IR_DecomposeAccessTest, Storage_AccessU32_StoreF32) {
    auto* var = b.Var("v", storage, ty.f32(), core::Access::kReadWrite);
    var->SetBindingPoint(0, 0);
    b.ir.root_block->Append(var);

    auto* func = b.Function("foo", ty.void_(), core::ir::Function::PipelineStage::kFragment);
    b.Append(func->Block(), [&] {
        b.Store(var, f32(0));
        b.Return(func);
    });

    auto* src = R"(
$B1: {  # root
  %v:ptr<storage, f32, read_write> = var undef @binding_point(0, 0)
}

%foo = @fragment func():void {
  $B2: {
    store %v, 0.0f
    ret
  }
}
)";

    ASSERT_EQ(src, str());

    auto* expect = R"(
$B1: {  # root
  %v:ptr<storage, array<u32, 1>, read_write> = var undef @binding_point(0, 0)
}

%foo = @fragment func():void {
  $B2: {
    %3:u32 = bitcast<u32> 0.0f
    %4:ptr<storage, u32, read_write> = access %v, 0u
    store %4, %3
    ret
  }
}
)";

    DecomposeAccessConfig options{.storage = true};
    Run(DecomposeAccess, options);
    EXPECT_EQ(expect, str());
}

TEST_F(IR_DecomposeAccessTest, Storage_AccessVec2u_StoreVec2f) {
    auto* var = b.Var("v", storage, ty.vec2f(), core::Access::kReadWrite);
    var->SetBindingPoint(0, 0);
    b.ir.root_block->Append(var);

    auto* func = b.Function("foo", ty.void_(), core::ir::Function::PipelineStage::kFragment);
    b.Append(func->Block(), [&] {
        b.Store(var, b.Zero(ty.vec2f()));
        b.Return(func);
    });

    auto* src = R"(
$B1: {  # root
  %v:ptr<storage, vec2<f32>, read_write> = var undef @binding_point(0, 0)
}

%foo = @fragment func():void {
  $B2: {
    store %v, vec2<f32>(0.0f)
    ret
  }
}
)";

    ASSERT_EQ(src, str());

    auto* expect = R"(
$B1: {  # root
  %v:ptr<storage, array<vec2<u32>, 1>, read_write> = var undef @binding_point(0, 0)
}

%foo = @fragment func():void {
  $B2: {
    %3:vec2<u32> = bitcast<vec2<u32>> vec2<f32>(0.0f)
    %4:ptr<storage, vec2<u32>, read_write> = access %v, 0u
    store %4, %3
    ret
  }
}
)";

    DecomposeAccessConfig options{.storage = true};
    Run(DecomposeAccess, options);
    EXPECT_EQ(expect, str());
}

TEST_F(IR_DecomposeAccessTest, Storage_AccessVec4u_StoreVec4f) {
    auto* var = b.Var("v", storage, ty.vec4f(), core::Access::kReadWrite);
    var->SetBindingPoint(0, 0);
    b.ir.root_block->Append(var);

    auto* func = b.Function("foo", ty.void_(), core::ir::Function::PipelineStage::kFragment);
    b.Append(func->Block(), [&] {
        b.Store(var, b.Zero(ty.vec4f()));
        b.Return(func);
    });

    auto* src = R"(
$B1: {  # root
  %v:ptr<storage, vec4<f32>, read_write> = var undef @binding_point(0, 0)
}

%foo = @fragment func():void {
  $B2: {
    store %v, vec4<f32>(0.0f)
    ret
  }
}
)";

    ASSERT_EQ(src, str());

    auto* expect = R"(
$B1: {  # root
  %v:ptr<storage, array<vec4<u32>, 1>, read_write> = var undef @binding_point(0, 0)
}

%foo = @fragment func():void {
  $B2: {
    %3:vec4<u32> = bitcast<vec4<u32>> vec4<f32>(0.0f)
    %4:ptr<storage, vec4<u32>, read_write> = access %v, 0u
    store %4, %3
    ret
  }
}
)";

    DecomposeAccessConfig options{.storage = true};
    Run(DecomposeAccess, options);
    EXPECT_EQ(expect, str());
}

TEST_F(IR_DecomposeAccessTest, Workgroup_AccessVec4u_StoreVec4b) {
    auto* var = b.Var("v", workgroup, ty.vec4(ty.bool_()), core::Access::kReadWrite);
    b.ir.root_block->Append(var);

    auto* func = b.Function("foo", ty.void_());
    b.Append(func->Block(), [&] {
        b.Store(var, b.Zero(ty.vec4(ty.bool_())));
        b.Return(func);
    });

    auto* src = R"(
$B1: {  # root
  %v:ptr<workgroup, vec4<bool>, read_write> = var undef
}

%foo = func():void {
  $B2: {
    store %v, vec4<bool>(false)
    ret
  }
}
)";

    ASSERT_EQ(src, str());

    auto* expect = R"(
$B1: {  # root
  %v:ptr<workgroup, array<vec4<u32>, 1>, read_write> = var undef
}

%foo = func():void {
  $B2: {
    %3:vec4<u32> = convert vec4<bool>(false)
    %4:ptr<workgroup, vec4<u32>, read_write> = access %v, 0u
    store %4, %3
    ret
  }
}
)";

    DecomposeAccessConfig options{.workgroup = true};
    Run(DecomposeAccess, options);
    EXPECT_EQ(expect, str());
}

TEST_F(IR_DecomposeAccessTest, Storage_AccessU16_StoreU32) {
    auto* sb = ty.Struct(mod.symbols.New("SB"), {
                                                    {mod.symbols.New("a"), ty.u16()},
                                                    {mod.symbols.New("b"), ty.u32()},
                                                });
    auto* var = b.Var("v", storage, sb, core::Access::kReadWrite);
    var->SetBindingPoint(0, 0);
    b.ir.root_block->Append(var);

    auto* func = b.Function("foo", ty.void_(), core::ir::Function::PipelineStage::kFragment);
    b.Append(func->Block(), [&] {
        b.Store(b.Access(ty.ptr(storage, ty.u16(), core::Access::kReadWrite), var, 0_u), u16(0));
        b.Store(b.Access(ty.ptr(storage, ty.u32(), core::Access::kReadWrite), var, 1_u), u32(0));
        b.Return(func);
    });

    auto* src = R"(
SB = struct @align(4) {
  a:u16 @offset(0)
  b:u32 @offset(4)
}

$B1: {  # root
  %v:ptr<storage, SB, read_write> = var undef @binding_point(0, 0)
}

%foo = @fragment func():void {
  $B2: {
    %3:ptr<storage, u16, read_write> = access %v, 0u
    store %3, 0u16
    %4:ptr<storage, u32, read_write> = access %v, 1u
    store %4, 0u
    ret
  }
}
)";

    ASSERT_EQ(src, str());

    auto* expect = R"(
SB = struct @align(4) {
  a:u16 @offset(0)
  b:u32 @offset(4)
}

$B1: {  # root
  %v:ptr<storage, array<u16, 4>, read_write> = var undef @binding_point(0, 0)
}

%foo = @fragment func():void {
  $B2: {
    %3:ptr<storage, u16, read_write> = access %v, 0u
    store %3, 0u16
    %4:vec2<u16> = bitcast<vec2<u16>> 0u
    %5:ptr<storage, u16, read_write> = access %v, 2u
    %6:u16 = access %4, 0u
    store %5, %6 @align(4)
    %7:ptr<storage, u16, read_write> = access %v, 3u
    %8:u16 = access %4, 1u
    store %7, %8
    ret
  }
}
)";

    DecomposeAccessConfig options{.storage = true};
    Run(DecomposeAccess, options);
    EXPECT_EQ(expect, str());
}

TEST_F(IR_DecomposeAccessTest, Workgroup_AccessU16_StoreBool) {
    auto* sb = ty.Struct(mod.symbols.New("SB"), {
                                                    {mod.symbols.New("a"), ty.u16()},
                                                    {mod.symbols.New("b"), ty.bool_()},
                                                });
    auto* var = b.Var("v", workgroup, sb, core::Access::kReadWrite);
    b.ir.root_block->Append(var);

    auto* func = b.Function("foo", ty.void_());
    b.Append(func->Block(), [&] {
        b.Store(b.Access(ty.ptr(workgroup, ty.u16(), core::Access::kReadWrite), var, 0_u), u16(0));
        b.Store(b.Access(ty.ptr(workgroup, ty.bool_(), core::Access::kReadWrite), var, 1_u),
                b.Constant(false));
        b.Return(func);
    });

    auto* src = R"(
SB = struct @align(4) {
  a:u16 @offset(0)
  b:bool @offset(4)
}

$B1: {  # root
  %v:ptr<workgroup, SB, read_write> = var undef
}

%foo = func():void {
  $B2: {
    %3:ptr<workgroup, u16, read_write> = access %v, 0u
    store %3, 0u16
    %4:ptr<workgroup, bool, read_write> = access %v, 1u
    store %4, false
    ret
  }
}
)";

    ASSERT_EQ(src, str());

    auto* expect = R"(
SB = struct @align(4) {
  a:u16 @offset(0)
  b:bool @offset(4)
}

$B1: {  # root
  %v:ptr<workgroup, array<u16, 4>, read_write> = var undef
}

%foo = func():void {
  $B2: {
    %3:ptr<workgroup, u16, read_write> = access %v, 0u
    store %3, 0u16
    %4:u32 = convert false
    %5:vec2<u16> = bitcast<vec2<u16>> %4
    %6:ptr<workgroup, u16, read_write> = access %v, 2u
    %7:u16 = access %5, 0u
    store %6, %7 @align(4)
    %8:ptr<workgroup, u16, read_write> = access %v, 3u
    %9:u16 = access %5, 1u
    store %8, %9
    ret
  }
}
)";

    DecomposeAccessConfig options{.workgroup = true};
    Run(DecomposeAccess, options);
    EXPECT_EQ(expect, str());
}

TEST_F(IR_DecomposeAccessTest, Storage_AccessU16_StoreVec2h_WithU16) {
    auto* sb = ty.Struct(mod.symbols.New("SB"), {
                                                    {mod.symbols.New("a"), ty.u16()},
                                                    {mod.symbols.New("b"), ty.vec2h()},
                                                });
    auto* var = b.Var("v", storage, sb, core::Access::kReadWrite);
    var->SetBindingPoint(0, 0);
    b.ir.root_block->Append(var);

    auto* func = b.Function("foo", ty.void_(), core::ir::Function::PipelineStage::kFragment);
    b.Append(func->Block(), [&] {
        b.Store(b.Access(ty.ptr(storage, ty.u16(), core::Access::kReadWrite), var, 0_u), u16(0));
        b.Store(b.Access(ty.ptr(storage, ty.vec2h(), core::Access::kReadWrite), var, 1_u),
                b.Zero(ty.vec2h()));
        b.Return(func);
    });

    auto* src = R"(
SB = struct @align(4) {
  a:u16 @offset(0)
  b:vec2<f16> @offset(4)
}

$B1: {  # root
  %v:ptr<storage, SB, read_write> = var undef @binding_point(0, 0)
}

%foo = @fragment func():void {
  $B2: {
    %3:ptr<storage, u16, read_write> = access %v, 0u
    store %3, 0u16
    %4:ptr<storage, vec2<f16>, read_write> = access %v, 1u
    store %4, vec2<f16>(0.0h)
    ret
  }
}
)";

    ASSERT_EQ(src, str());

    auto* expect = R"(
SB = struct @align(4) {
  a:u16 @offset(0)
  b:vec2<f16> @offset(4)
}

$B1: {  # root
  %v:ptr<storage, array<u16, 4>, read_write> = var undef @binding_point(0, 0)
}

%foo = @fragment func():void {
  $B2: {
    %3:ptr<storage, u16, read_write> = access %v, 0u
    store %3, 0u16
    %4:f16 = access vec2<f16>(0.0h), 0u
    %5:u16 = bitcast<u16> %4
    %6:ptr<storage, u16, read_write> = access %v, 2u
    store %6, %5 @align(4)
    %7:f16 = access vec2<f16>(0.0h), 1u
    %8:u16 = bitcast<u16> %7
    %9:ptr<storage, u16, read_write> = access %v, 3u
    store %9, %8
    ret
  }
}
)";

    DecomposeAccessConfig options{.storage = true};
    Run(DecomposeAccess, options);
    EXPECT_EQ(expect, str());
}

TEST_F(IR_DecomposeAccessTest, Storage_AccessU16_StoreVec3h) {
    auto* sb = ty.Struct(mod.symbols.New("SB"), {
                                                    {mod.symbols.New("a"), ty.u16()},
                                                    {mod.symbols.New("b"), ty.vec3h()},
                                                });
    auto* var = b.Var("v", storage, sb, core::Access::kReadWrite);
    var->SetBindingPoint(0, 0);
    b.ir.root_block->Append(var);

    auto* func = b.Function("foo", ty.void_(), core::ir::Function::PipelineStage::kFragment);
    b.Append(func->Block(), [&] {
        b.Store(b.Access(ty.ptr(storage, ty.u16(), core::Access::kReadWrite), var, 0_u), u16(0));
        b.Store(b.Access(ty.ptr(storage, ty.vec3h(), core::Access::kReadWrite), var, 1_u),
                b.Zero(ty.vec3h()));
        b.Return(func);
    });

    auto* src = R"(
SB = struct @align(8) {
  a:u16 @offset(0)
  b:vec3<f16> @offset(8)
}

$B1: {  # root
  %v:ptr<storage, SB, read_write> = var undef @binding_point(0, 0)
}

%foo = @fragment func():void {
  $B2: {
    %3:ptr<storage, u16, read_write> = access %v, 0u
    store %3, 0u16
    %4:ptr<storage, vec3<f16>, read_write> = access %v, 1u
    store %4, vec3<f16>(0.0h)
    ret
  }
}
)";

    ASSERT_EQ(src, str());

    auto* expect = R"(
SB = struct @align(8) {
  a:u16 @offset(0)
  b:vec3<f16> @offset(8)
}

$B1: {  # root
  %v:ptr<storage, array<u16, 8>, read_write> = var undef @binding_point(0, 0)
}

%foo = @fragment func():void {
  $B2: {
    %3:ptr<storage, u16, read_write> = access %v, 0u
    store %3, 0u16
    %4:f16 = access vec3<f16>(0.0h), 0u
    %5:u16 = bitcast<u16> %4
    %6:ptr<storage, u16, read_write> = access %v, 4u
    store %6, %5 @align(8)
    %7:f16 = access vec3<f16>(0.0h), 1u
    %8:u16 = bitcast<u16> %7
    %9:ptr<storage, u16, read_write> = access %v, 5u
    store %9, %8
    %10:f16 = access vec3<f16>(0.0h), 2u
    %11:u16 = bitcast<u16> %10
    %12:ptr<storage, u16, read_write> = access %v, 6u
    store %12, %11
    ret
  }
}
)";

    DecomposeAccessConfig options{.storage = true};
    Run(DecomposeAccess, options);
    EXPECT_EQ(expect, str());
}

TEST_F(IR_DecomposeAccessTest, Storage_AccessU16_StoreVec4h_WithU16) {
    auto* sb = ty.Struct(mod.symbols.New("SB"), {
                                                    {mod.symbols.New("a"), ty.u16()},
                                                    {mod.symbols.New("b"), ty.vec4h()},
                                                });
    auto* var = b.Var("v", storage, sb, core::Access::kReadWrite);
    var->SetBindingPoint(0, 0);
    b.ir.root_block->Append(var);

    auto* func = b.Function("foo", ty.void_(), core::ir::Function::PipelineStage::kFragment);
    b.Append(func->Block(), [&] {
        b.Store(b.Access(ty.ptr(storage, ty.u16(), core::Access::kReadWrite), var, 0_u), u16(0));
        b.Store(b.Access(ty.ptr(storage, ty.vec4h(), core::Access::kReadWrite), var, 1_u),
                b.Zero(ty.vec4h()));
        b.Return(func);
    });

    auto* src = R"(
SB = struct @align(8) {
  a:u16 @offset(0)
  b:vec4<f16> @offset(8)
}

$B1: {  # root
  %v:ptr<storage, SB, read_write> = var undef @binding_point(0, 0)
}

%foo = @fragment func():void {
  $B2: {
    %3:ptr<storage, u16, read_write> = access %v, 0u
    store %3, 0u16
    %4:ptr<storage, vec4<f16>, read_write> = access %v, 1u
    store %4, vec4<f16>(0.0h)
    ret
  }
}
)";

    ASSERT_EQ(src, str());

    auto* expect = R"(
SB = struct @align(8) {
  a:u16 @offset(0)
  b:vec4<f16> @offset(8)
}

$B1: {  # root
  %v:ptr<storage, array<u16, 8>, read_write> = var undef @binding_point(0, 0)
}

%foo = @fragment func():void {
  $B2: {
    %3:ptr<storage, u16, read_write> = access %v, 0u
    store %3, 0u16
    %4:f16 = access vec4<f16>(0.0h), 0u
    %5:u16 = bitcast<u16> %4
    %6:ptr<storage, u16, read_write> = access %v, 4u
    store %6, %5 @align(8)
    %7:f16 = access vec4<f16>(0.0h), 1u
    %8:u16 = bitcast<u16> %7
    %9:ptr<storage, u16, read_write> = access %v, 5u
    store %9, %8
    %10:f16 = access vec4<f16>(0.0h), 2u
    %11:u16 = bitcast<u16> %10
    %12:ptr<storage, u16, read_write> = access %v, 6u
    store %12, %11
    %13:f16 = access vec4<f16>(0.0h), 3u
    %14:u16 = bitcast<u16> %13
    %15:ptr<storage, u16, read_write> = access %v, 7u
    store %15, %14
    ret
  }
}
)";

    DecomposeAccessConfig options{.storage = true};
    Run(DecomposeAccess, options);
    EXPECT_EQ(expect, str());
}

TEST_F(IR_DecomposeAccessTest, Storage_AccessU16_StoreVec2u) {
    auto* sb = ty.Struct(mod.symbols.New("SB"), {
                                                    {mod.symbols.New("a"), ty.u16()},
                                                    {mod.symbols.New("b"), ty.vec2u()},
                                                });
    auto* var = b.Var("v", storage, sb, core::Access::kReadWrite);
    var->SetBindingPoint(0, 0);
    b.ir.root_block->Append(var);

    auto* func = b.Function("foo", ty.void_(), core::ir::Function::PipelineStage::kFragment);
    b.Append(func->Block(), [&] {
        b.Store(b.Access(ty.ptr(storage, ty.u16(), core::Access::kReadWrite), var, 0_u), u16(0));
        b.Store(b.Access(ty.ptr(storage, ty.vec2u(), core::Access::kReadWrite), var, 1_u),
                b.Zero(ty.vec2u()));
        b.Return(func);
    });

    auto* src = R"(
SB = struct @align(8) {
  a:u16 @offset(0)
  b:vec2<u32> @offset(8)
}

$B1: {  # root
  %v:ptr<storage, SB, read_write> = var undef @binding_point(0, 0)
}

%foo = @fragment func():void {
  $B2: {
    %3:ptr<storage, u16, read_write> = access %v, 0u
    store %3, 0u16
    %4:ptr<storage, vec2<u32>, read_write> = access %v, 1u
    store %4, vec2<u32>(0u)
    ret
  }
}
)";

    ASSERT_EQ(src, str());

    auto* expect = R"(
SB = struct @align(8) {
  a:u16 @offset(0)
  b:vec2<u32> @offset(8)
}

$B1: {  # root
  %v:ptr<storage, array<u16, 8>, read_write> = var undef @binding_point(0, 0)
}

%foo = @fragment func():void {
  $B2: {
    %3:ptr<storage, u16, read_write> = access %v, 0u
    store %3, 0u16
    %4:u32 = access vec2<u32>(0u), 0u
    %5:vec2<u16> = bitcast<vec2<u16>> %4
    %6:u16 = access %5, 0u
    %7:ptr<storage, u16, read_write> = access %v, 4u
    store %7, %6 @align(8)
    %8:u32 = access vec2<u32>(0u), 0u
    %9:vec2<u16> = bitcast<vec2<u16>> %8
    %10:u16 = access %9, 1u
    %11:ptr<storage, u16, read_write> = access %v, 5u
    store %11, %10
    %12:u32 = access vec2<u32>(0u), 1u
    %13:vec2<u16> = bitcast<vec2<u16>> %12
    %14:u16 = access %13, 0u
    %15:ptr<storage, u16, read_write> = access %v, 6u
    store %15, %14
    %16:u32 = access vec2<u32>(0u), 1u
    %17:vec2<u16> = bitcast<vec2<u16>> %16
    %18:u16 = access %17, 1u
    %19:ptr<storage, u16, read_write> = access %v, 7u
    store %19, %18
    ret
  }
}
)";

    DecomposeAccessConfig options{.storage = true};
    Run(DecomposeAccess, options);
    EXPECT_EQ(expect, str());
}

TEST_F(IR_DecomposeAccessTest, Storage_AccessU16_StoreVec3u) {
    auto* sb = ty.Struct(mod.symbols.New("SB"), {
                                                    {mod.symbols.New("a"), ty.u16()},
                                                    {mod.symbols.New("b"), ty.vec3u()},
                                                });
    auto* var = b.Var("v", storage, sb, core::Access::kReadWrite);
    var->SetBindingPoint(0, 0);
    b.ir.root_block->Append(var);

    auto* func = b.Function("foo", ty.void_(), core::ir::Function::PipelineStage::kFragment);
    b.Append(func->Block(), [&] {
        b.Store(b.Access(ty.ptr(storage, ty.u16(), core::Access::kReadWrite), var, 0_u), u16(0));
        b.Store(b.Access(ty.ptr(storage, ty.vec3u(), core::Access::kReadWrite), var, 1_u),
                b.Zero(ty.vec3u()));
        b.Return(func);
    });

    auto* src = R"(
SB = struct @align(16) {
  a:u16 @offset(0)
  b:vec3<u32> @offset(16)
}

$B1: {  # root
  %v:ptr<storage, SB, read_write> = var undef @binding_point(0, 0)
}

%foo = @fragment func():void {
  $B2: {
    %3:ptr<storage, u16, read_write> = access %v, 0u
    store %3, 0u16
    %4:ptr<storage, vec3<u32>, read_write> = access %v, 1u
    store %4, vec3<u32>(0u)
    ret
  }
}
)";

    ASSERT_EQ(src, str());

    auto* expect = R"(
SB = struct @align(16) {
  a:u16 @offset(0)
  b:vec3<u32> @offset(16)
}

$B1: {  # root
  %v:ptr<storage, array<u16, 16>, read_write> = var undef @binding_point(0, 0)
}

%foo = @fragment func():void {
  $B2: {
    %3:ptr<storage, u16, read_write> = access %v, 0u
    store %3, 0u16
    %4:u32 = access vec3<u32>(0u), 0u
    %5:vec2<u16> = bitcast<vec2<u16>> %4
    %6:u16 = access %5, 0u
    %7:ptr<storage, u16, read_write> = access %v, 8u
    store %7, %6 @align(16)
    %8:u32 = access vec3<u32>(0u), 0u
    %9:vec2<u16> = bitcast<vec2<u16>> %8
    %10:u16 = access %9, 1u
    %11:ptr<storage, u16, read_write> = access %v, 9u
    store %11, %10
    %12:u32 = access vec3<u32>(0u), 1u
    %13:vec2<u16> = bitcast<vec2<u16>> %12
    %14:u16 = access %13, 0u
    %15:ptr<storage, u16, read_write> = access %v, 10u
    store %15, %14
    %16:u32 = access vec3<u32>(0u), 1u
    %17:vec2<u16> = bitcast<vec2<u16>> %16
    %18:u16 = access %17, 1u
    %19:ptr<storage, u16, read_write> = access %v, 11u
    store %19, %18
    %20:u32 = access vec3<u32>(0u), 2u
    %21:vec2<u16> = bitcast<vec2<u16>> %20
    %22:u16 = access %21, 0u
    %23:ptr<storage, u16, read_write> = access %v, 12u
    store %23, %22
    %24:u32 = access vec3<u32>(0u), 2u
    %25:vec2<u16> = bitcast<vec2<u16>> %24
    %26:u16 = access %25, 1u
    %27:ptr<storage, u16, read_write> = access %v, 13u
    store %27, %26
    ret
  }
}
)";

    DecomposeAccessConfig options{.storage = true};
    Run(DecomposeAccess, options);
    EXPECT_EQ(expect, str());
}

TEST_F(IR_DecomposeAccessTest, Storage_AccessU16_StoreVec4u) {
    auto* sb = ty.Struct(mod.symbols.New("SB"), {
                                                    {mod.symbols.New("a"), ty.u16()},
                                                    {mod.symbols.New("b"), ty.vec4u()},
                                                });
    auto* var = b.Var("v", storage, sb, core::Access::kReadWrite);
    var->SetBindingPoint(0, 0);
    b.ir.root_block->Append(var);

    auto* func = b.Function("foo", ty.void_(), core::ir::Function::PipelineStage::kFragment);
    b.Append(func->Block(), [&] {
        b.Store(b.Access(ty.ptr(storage, ty.u16(), core::Access::kReadWrite), var, 0_u), u16(0));
        b.Store(b.Access(ty.ptr(storage, ty.vec4u(), core::Access::kReadWrite), var, 1_u),
                b.Zero(ty.vec4u()));
        b.Return(func);
    });

    auto* src = R"(
SB = struct @align(16) {
  a:u16 @offset(0)
  b:vec4<u32> @offset(16)
}

$B1: {  # root
  %v:ptr<storage, SB, read_write> = var undef @binding_point(0, 0)
}

%foo = @fragment func():void {
  $B2: {
    %3:ptr<storage, u16, read_write> = access %v, 0u
    store %3, 0u16
    %4:ptr<storage, vec4<u32>, read_write> = access %v, 1u
    store %4, vec4<u32>(0u)
    ret
  }
}
)";

    ASSERT_EQ(src, str());

    auto* expect = R"(
SB = struct @align(16) {
  a:u16 @offset(0)
  b:vec4<u32> @offset(16)
}

$B1: {  # root
  %v:ptr<storage, array<u16, 16>, read_write> = var undef @binding_point(0, 0)
}

%foo = @fragment func():void {
  $B2: {
    %3:ptr<storage, u16, read_write> = access %v, 0u
    store %3, 0u16
    %4:u32 = access vec4<u32>(0u), 0u
    %5:vec2<u16> = bitcast<vec2<u16>> %4
    %6:u16 = access %5, 0u
    %7:ptr<storage, u16, read_write> = access %v, 8u
    store %7, %6 @align(16)
    %8:u32 = access vec4<u32>(0u), 0u
    %9:vec2<u16> = bitcast<vec2<u16>> %8
    %10:u16 = access %9, 1u
    %11:ptr<storage, u16, read_write> = access %v, 9u
    store %11, %10
    %12:u32 = access vec4<u32>(0u), 1u
    %13:vec2<u16> = bitcast<vec2<u16>> %12
    %14:u16 = access %13, 0u
    %15:ptr<storage, u16, read_write> = access %v, 10u
    store %15, %14
    %16:u32 = access vec4<u32>(0u), 1u
    %17:vec2<u16> = bitcast<vec2<u16>> %16
    %18:u16 = access %17, 1u
    %19:ptr<storage, u16, read_write> = access %v, 11u
    store %19, %18
    %20:u32 = access vec4<u32>(0u), 2u
    %21:vec2<u16> = bitcast<vec2<u16>> %20
    %22:u16 = access %21, 0u
    %23:ptr<storage, u16, read_write> = access %v, 12u
    store %23, %22
    %24:u32 = access vec4<u32>(0u), 2u
    %25:vec2<u16> = bitcast<vec2<u16>> %24
    %26:u16 = access %25, 1u
    %27:ptr<storage, u16, read_write> = access %v, 13u
    store %27, %26
    %28:u32 = access vec4<u32>(0u), 3u
    %29:vec2<u16> = bitcast<vec2<u16>> %28
    %30:u16 = access %29, 0u
    %31:ptr<storage, u16, read_write> = access %v, 14u
    store %31, %30
    %32:u32 = access vec4<u32>(0u), 3u
    %33:vec2<u16> = bitcast<vec2<u16>> %32
    %34:u16 = access %33, 1u
    %35:ptr<storage, u16, read_write> = access %v, 15u
    store %35, %34
    ret
  }
}
)";

    DecomposeAccessConfig options{.storage = true};
    Run(DecomposeAccess, options);
    EXPECT_EQ(expect, str());
}

TEST_F(IR_DecomposeAccessTest, Workgroup_AccessU16_StoreVec2b) {
    auto* sb = ty.Struct(mod.symbols.New("SB"), {
                                                    {mod.symbols.New("a"), ty.u16()},
                                                    {mod.symbols.New("b"), ty.vec2(ty.bool_())},
                                                });
    auto* var = b.Var("v", workgroup, sb, core::Access::kReadWrite);
    b.ir.root_block->Append(var);

    auto* func = b.Function("foo", ty.void_());
    b.Append(func->Block(), [&] {
        b.Store(b.Access(ty.ptr(workgroup, ty.u16(), core::Access::kReadWrite), var, 0_u), u16(0));
        b.Store(
            b.Access(ty.ptr(workgroup, ty.vec2(ty.bool_()), core::Access::kReadWrite), var, 1_u),
            b.Zero(ty.vec2(ty.bool_())));
        b.Return(func);
    });

    auto* src = R"(
SB = struct @align(8) {
  a:u16 @offset(0)
  b:vec2<bool> @offset(8)
}

$B1: {  # root
  %v:ptr<workgroup, SB, read_write> = var undef
}

%foo = func():void {
  $B2: {
    %3:ptr<workgroup, u16, read_write> = access %v, 0u
    store %3, 0u16
    %4:ptr<workgroup, vec2<bool>, read_write> = access %v, 1u
    store %4, vec2<bool>(false)
    ret
  }
}
)";

    ASSERT_EQ(src, str());

    auto* expect = R"(
SB = struct @align(8) {
  a:u16 @offset(0)
  b:vec2<bool> @offset(8)
}

$B1: {  # root
  %v:ptr<workgroup, array<u16, 8>, read_write> = var undef
}

%foo = func():void {
  $B2: {
    %3:ptr<workgroup, u16, read_write> = access %v, 0u
    store %3, 0u16
    %4:bool = access vec2<bool>(false), 0u
    %5:u32 = convert %4
    %6:vec2<u16> = bitcast<vec2<u16>> %5
    %7:u16 = access %6, 0u
    %8:ptr<workgroup, u16, read_write> = access %v, 4u
    store %8, %7 @align(8)
    %9:bool = access vec2<bool>(false), 0u
    %10:u32 = convert %9
    %11:vec2<u16> = bitcast<vec2<u16>> %10
    %12:u16 = access %11, 1u
    %13:ptr<workgroup, u16, read_write> = access %v, 5u
    store %13, %12
    %14:bool = access vec2<bool>(false), 1u
    %15:u32 = convert %14
    %16:vec2<u16> = bitcast<vec2<u16>> %15
    %17:u16 = access %16, 0u
    %18:ptr<workgroup, u16, read_write> = access %v, 6u
    store %18, %17
    %19:bool = access vec2<bool>(false), 1u
    %20:u32 = convert %19
    %21:vec2<u16> = bitcast<vec2<u16>> %20
    %22:u16 = access %21, 1u
    %23:ptr<workgroup, u16, read_write> = access %v, 7u
    store %23, %22
    ret
  }
}
)";

    DecomposeAccessConfig options{.workgroup = true};
    Run(DecomposeAccess, options);
    EXPECT_EQ(expect, str());
}

TEST_F(IR_DecomposeAccessTest, Workgroup_AccessU16_StoreVec3b) {
    auto* sb = ty.Struct(mod.symbols.New("SB"), {
                                                    {mod.symbols.New("a"), ty.u16()},
                                                    {mod.symbols.New("b"), ty.vec3(ty.bool_())},
                                                });
    auto* var = b.Var("v", workgroup, sb, core::Access::kReadWrite);
    b.ir.root_block->Append(var);

    auto* func = b.Function("foo", ty.void_());
    b.Append(func->Block(), [&] {
        b.Store(b.Access(ty.ptr(workgroup, ty.u16(), core::Access::kReadWrite), var, 0_u), u16(0));
        b.Store(
            b.Access(ty.ptr(workgroup, ty.vec3(ty.bool_()), core::Access::kReadWrite), var, 1_u),
            b.Zero(ty.vec3(ty.bool_())));
        b.Return(func);
    });

    auto* src = R"(
SB = struct @align(16) {
  a:u16 @offset(0)
  b:vec3<bool> @offset(16)
}

$B1: {  # root
  %v:ptr<workgroup, SB, read_write> = var undef
}

%foo = func():void {
  $B2: {
    %3:ptr<workgroup, u16, read_write> = access %v, 0u
    store %3, 0u16
    %4:ptr<workgroup, vec3<bool>, read_write> = access %v, 1u
    store %4, vec3<bool>(false)
    ret
  }
}
)";

    ASSERT_EQ(src, str());

    auto* expect = R"(
SB = struct @align(16) {
  a:u16 @offset(0)
  b:vec3<bool> @offset(16)
}

$B1: {  # root
  %v:ptr<workgroup, array<u16, 16>, read_write> = var undef
}

%foo = func():void {
  $B2: {
    %3:ptr<workgroup, u16, read_write> = access %v, 0u
    store %3, 0u16
    %4:bool = access vec3<bool>(false), 0u
    %5:u32 = convert %4
    %6:vec2<u16> = bitcast<vec2<u16>> %5
    %7:u16 = access %6, 0u
    %8:ptr<workgroup, u16, read_write> = access %v, 8u
    store %8, %7 @align(16)
    %9:bool = access vec3<bool>(false), 0u
    %10:u32 = convert %9
    %11:vec2<u16> = bitcast<vec2<u16>> %10
    %12:u16 = access %11, 1u
    %13:ptr<workgroup, u16, read_write> = access %v, 9u
    store %13, %12
    %14:bool = access vec3<bool>(false), 1u
    %15:u32 = convert %14
    %16:vec2<u16> = bitcast<vec2<u16>> %15
    %17:u16 = access %16, 0u
    %18:ptr<workgroup, u16, read_write> = access %v, 10u
    store %18, %17
    %19:bool = access vec3<bool>(false), 1u
    %20:u32 = convert %19
    %21:vec2<u16> = bitcast<vec2<u16>> %20
    %22:u16 = access %21, 1u
    %23:ptr<workgroup, u16, read_write> = access %v, 11u
    store %23, %22
    %24:bool = access vec3<bool>(false), 2u
    %25:u32 = convert %24
    %26:vec2<u16> = bitcast<vec2<u16>> %25
    %27:u16 = access %26, 0u
    %28:ptr<workgroup, u16, read_write> = access %v, 12u
    store %28, %27
    %29:bool = access vec3<bool>(false), 2u
    %30:u32 = convert %29
    %31:vec2<u16> = bitcast<vec2<u16>> %30
    %32:u16 = access %31, 1u
    %33:ptr<workgroup, u16, read_write> = access %v, 13u
    store %33, %32
    ret
  }
}
)";

    DecomposeAccessConfig options{.workgroup = true};
    Run(DecomposeAccess, options);
    EXPECT_EQ(expect, str());
}

TEST_F(IR_DecomposeAccessTest, Workgroup_AccessU16_StoreVec4b) {
    auto* sb = ty.Struct(mod.symbols.New("SB"), {
                                                    {mod.symbols.New("a"), ty.u16()},
                                                    {mod.symbols.New("b"), ty.vec4(ty.bool_())},
                                                });
    auto* var = b.Var("v", workgroup, sb, core::Access::kReadWrite);
    b.ir.root_block->Append(var);

    auto* func = b.Function("foo", ty.void_());
    b.Append(func->Block(), [&] {
        b.Store(b.Access(ty.ptr(workgroup, ty.u16(), core::Access::kReadWrite), var, 0_u), u16(0));
        b.Store(
            b.Access(ty.ptr(workgroup, ty.vec4(ty.bool_()), core::Access::kReadWrite), var, 1_u),
            b.Zero(ty.vec4(ty.bool_())));
        b.Return(func);
    });

    auto* src = R"(
SB = struct @align(16) {
  a:u16 @offset(0)
  b:vec4<bool> @offset(16)
}

$B1: {  # root
  %v:ptr<workgroup, SB, read_write> = var undef
}

%foo = func():void {
  $B2: {
    %3:ptr<workgroup, u16, read_write> = access %v, 0u
    store %3, 0u16
    %4:ptr<workgroup, vec4<bool>, read_write> = access %v, 1u
    store %4, vec4<bool>(false)
    ret
  }
}
)";

    ASSERT_EQ(src, str());

    auto* expect = R"(
SB = struct @align(16) {
  a:u16 @offset(0)
  b:vec4<bool> @offset(16)
}

$B1: {  # root
  %v:ptr<workgroup, array<u16, 16>, read_write> = var undef
}

%foo = func():void {
  $B2: {
    %3:ptr<workgroup, u16, read_write> = access %v, 0u
    store %3, 0u16
    %4:bool = access vec4<bool>(false), 0u
    %5:u32 = convert %4
    %6:vec2<u16> = bitcast<vec2<u16>> %5
    %7:u16 = access %6, 0u
    %8:ptr<workgroup, u16, read_write> = access %v, 8u
    store %8, %7 @align(16)
    %9:bool = access vec4<bool>(false), 0u
    %10:u32 = convert %9
    %11:vec2<u16> = bitcast<vec2<u16>> %10
    %12:u16 = access %11, 1u
    %13:ptr<workgroup, u16, read_write> = access %v, 9u
    store %13, %12
    %14:bool = access vec4<bool>(false), 1u
    %15:u32 = convert %14
    %16:vec2<u16> = bitcast<vec2<u16>> %15
    %17:u16 = access %16, 0u
    %18:ptr<workgroup, u16, read_write> = access %v, 10u
    store %18, %17
    %19:bool = access vec4<bool>(false), 1u
    %20:u32 = convert %19
    %21:vec2<u16> = bitcast<vec2<u16>> %20
    %22:u16 = access %21, 1u
    %23:ptr<workgroup, u16, read_write> = access %v, 11u
    store %23, %22
    %24:bool = access vec4<bool>(false), 2u
    %25:u32 = convert %24
    %26:vec2<u16> = bitcast<vec2<u16>> %25
    %27:u16 = access %26, 0u
    %28:ptr<workgroup, u16, read_write> = access %v, 12u
    store %28, %27
    %29:bool = access vec4<bool>(false), 2u
    %30:u32 = convert %29
    %31:vec2<u16> = bitcast<vec2<u16>> %30
    %32:u16 = access %31, 1u
    %33:ptr<workgroup, u16, read_write> = access %v, 13u
    store %33, %32
    %34:bool = access vec4<bool>(false), 3u
    %35:u32 = convert %34
    %36:vec2<u16> = bitcast<vec2<u16>> %35
    %37:u16 = access %36, 0u
    %38:ptr<workgroup, u16, read_write> = access %v, 14u
    store %38, %37
    %39:bool = access vec4<bool>(false), 3u
    %40:u32 = convert %39
    %41:vec2<u16> = bitcast<vec2<u16>> %40
    %42:u16 = access %41, 1u
    %43:ptr<workgroup, u16, read_write> = access %v, 15u
    store %43, %42
    ret
  }
}
)";

    DecomposeAccessConfig options{.workgroup = true};
    Run(DecomposeAccess, options);
    EXPECT_EQ(expect, str());
}

TEST_F(IR_DecomposeAccessTest, Storage_AccessU32_StoreVec2u) {
    auto* sb = ty.Struct(mod.symbols.New("SB"), {
                                                    {mod.symbols.New("a"), ty.u32()},
                                                    {mod.symbols.New("b"), ty.vec2u()},
                                                });
    auto* var = b.Var("v", storage, sb, core::Access::kReadWrite);
    var->SetBindingPoint(0, 0);
    b.ir.root_block->Append(var);

    auto* func = b.Function("foo", ty.void_(), core::ir::Function::PipelineStage::kFragment);
    b.Append(func->Block(), [&] {
        b.Store(b.Access(ty.ptr(storage, ty.u32(), core::Access::kReadWrite), var, 0_u), u32(0));
        b.Store(b.Access(ty.ptr(storage, ty.vec2u(), core::Access::kReadWrite), var, 1_u),
                b.Zero(ty.vec2u()));
        b.Return(func);
    });

    auto* src = R"(
SB = struct @align(8) {
  a:u32 @offset(0)
  b:vec2<u32> @offset(8)
}

$B1: {  # root
  %v:ptr<storage, SB, read_write> = var undef @binding_point(0, 0)
}

%foo = @fragment func():void {
  $B2: {
    %3:ptr<storage, u32, read_write> = access %v, 0u
    store %3, 0u
    %4:ptr<storage, vec2<u32>, read_write> = access %v, 1u
    store %4, vec2<u32>(0u)
    ret
  }
}
)";

    ASSERT_EQ(src, str());

    auto* expect = R"(
SB = struct @align(8) {
  a:u32 @offset(0)
  b:vec2<u32> @offset(8)
}

$B1: {  # root
  %v:ptr<storage, array<u32, 4>, read_write> = var undef @binding_point(0, 0)
}

%foo = @fragment func():void {
  $B2: {
    %3:ptr<storage, u32, read_write> = access %v, 0u
    store %3, 0u
    %4:u32 = access vec2<u32>(0u), 0u
    %5:ptr<storage, u32, read_write> = access %v, 2u
    store %5, %4 @align(8)
    %6:u32 = access vec2<u32>(0u), 1u
    %7:ptr<storage, u32, read_write> = access %v, 3u
    store %7, %6
    ret
  }
}
)";

    DecomposeAccessConfig options{.storage = true};
    Run(DecomposeAccess, options);
    EXPECT_EQ(expect, str());
}

TEST_F(IR_DecomposeAccessTest, Storage_AccessU32_StoreVec3u) {
    auto* sb = ty.Struct(mod.symbols.New("SB"), {
                                                    {mod.symbols.New("a"), ty.u32()},
                                                    {mod.symbols.New("b"), ty.vec3u()},
                                                });
    auto* var = b.Var("v", storage, sb, core::Access::kReadWrite);
    var->SetBindingPoint(0, 0);
    b.ir.root_block->Append(var);

    auto* func = b.Function("foo", ty.void_(), core::ir::Function::PipelineStage::kFragment);
    b.Append(func->Block(), [&] {
        b.Store(b.Access(ty.ptr(storage, ty.u32(), core::Access::kReadWrite), var, 0_u), u32(0));
        b.Store(b.Access(ty.ptr(storage, ty.vec3u(), core::Access::kReadWrite), var, 1_u),
                b.Zero(ty.vec3u()));
        b.Return(func);
    });

    auto* src = R"(
SB = struct @align(16) {
  a:u32 @offset(0)
  b:vec3<u32> @offset(16)
}

$B1: {  # root
  %v:ptr<storage, SB, read_write> = var undef @binding_point(0, 0)
}

%foo = @fragment func():void {
  $B2: {
    %3:ptr<storage, u32, read_write> = access %v, 0u
    store %3, 0u
    %4:ptr<storage, vec3<u32>, read_write> = access %v, 1u
    store %4, vec3<u32>(0u)
    ret
  }
}
)";

    ASSERT_EQ(src, str());

    auto* expect = R"(
SB = struct @align(16) {
  a:u32 @offset(0)
  b:vec3<u32> @offset(16)
}

$B1: {  # root
  %v:ptr<storage, array<u32, 8>, read_write> = var undef @binding_point(0, 0)
}

%foo = @fragment func():void {
  $B2: {
    %3:ptr<storage, u32, read_write> = access %v, 0u
    store %3, 0u
    %4:u32 = access vec3<u32>(0u), 0u
    %5:ptr<storage, u32, read_write> = access %v, 4u
    store %5, %4 @align(16)
    %6:u32 = access vec3<u32>(0u), 1u
    %7:ptr<storage, u32, read_write> = access %v, 5u
    store %7, %6
    %8:u32 = access vec3<u32>(0u), 2u
    %9:ptr<storage, u32, read_write> = access %v, 6u
    store %9, %8
    ret
  }
}
)";

    DecomposeAccessConfig options{.storage = true};
    Run(DecomposeAccess, options);
    EXPECT_EQ(expect, str());
}

TEST_F(IR_DecomposeAccessTest, Storage_AccessU32_StoreVec4u) {
    auto* sb = ty.Struct(mod.symbols.New("SB"), {
                                                    {mod.symbols.New("a"), ty.u32()},
                                                    {mod.symbols.New("b"), ty.vec4u()},
                                                });
    auto* var = b.Var("v", storage, sb, core::Access::kReadWrite);
    var->SetBindingPoint(0, 0);
    b.ir.root_block->Append(var);

    auto* func = b.Function("foo", ty.void_(), core::ir::Function::PipelineStage::kFragment);
    b.Append(func->Block(), [&] {
        b.Store(b.Access(ty.ptr(storage, ty.u32(), core::Access::kReadWrite), var, 0_u), u32(0));
        b.Store(b.Access(ty.ptr(storage, ty.vec4u(), core::Access::kReadWrite), var, 1_u),
                b.Zero(ty.vec4u()));
        b.Return(func);
    });

    auto* src = R"(
SB = struct @align(16) {
  a:u32 @offset(0)
  b:vec4<u32> @offset(16)
}

$B1: {  # root
  %v:ptr<storage, SB, read_write> = var undef @binding_point(0, 0)
}

%foo = @fragment func():void {
  $B2: {
    %3:ptr<storage, u32, read_write> = access %v, 0u
    store %3, 0u
    %4:ptr<storage, vec4<u32>, read_write> = access %v, 1u
    store %4, vec4<u32>(0u)
    ret
  }
}
)";

    ASSERT_EQ(src, str());

    auto* expect = R"(
SB = struct @align(16) {
  a:u32 @offset(0)
  b:vec4<u32> @offset(16)
}

$B1: {  # root
  %v:ptr<storage, array<u32, 8>, read_write> = var undef @binding_point(0, 0)
}

%foo = @fragment func():void {
  $B2: {
    %3:ptr<storage, u32, read_write> = access %v, 0u
    store %3, 0u
    %4:u32 = access vec4<u32>(0u), 0u
    %5:ptr<storage, u32, read_write> = access %v, 4u
    store %5, %4 @align(16)
    %6:u32 = access vec4<u32>(0u), 1u
    %7:ptr<storage, u32, read_write> = access %v, 5u
    store %7, %6
    %8:u32 = access vec4<u32>(0u), 2u
    %9:ptr<storage, u32, read_write> = access %v, 6u
    store %9, %8
    %10:u32 = access vec4<u32>(0u), 3u
    %11:ptr<storage, u32, read_write> = access %v, 7u
    store %11, %10
    ret
  }
}
)";

    DecomposeAccessConfig options{.storage = true};
    Run(DecomposeAccess, options);
    EXPECT_EQ(expect, str());
}

TEST_F(IR_DecomposeAccessTest, Storage_AccessU16_StoreVec2h_WithU32) {
    auto* sb = ty.Struct(mod.symbols.New("SB"), {
                                                    {mod.symbols.New("a"), ty.u32()},
                                                    {mod.symbols.New("b"), ty.vec2h()},
                                                });
    auto* var = b.Var("v", storage, sb, core::Access::kReadWrite);
    var->SetBindingPoint(0, 0);
    b.ir.root_block->Append(var);

    auto* func = b.Function("foo", ty.void_(), core::ir::Function::PipelineStage::kFragment);
    b.Append(func->Block(), [&] {
        b.Store(b.Access(ty.ptr(storage, ty.u32(), core::Access::kReadWrite), var, 0_u), u32(0));
        b.Store(b.Access(ty.ptr(storage, ty.vec2h(), core::Access::kReadWrite), var, 1_u),
                b.Zero(ty.vec2h()));
        b.Return(func);
    });

    auto* src = R"(
SB = struct @align(4) {
  a:u32 @offset(0)
  b:vec2<f16> @offset(4)
}

$B1: {  # root
  %v:ptr<storage, SB, read_write> = var undef @binding_point(0, 0)
}

%foo = @fragment func():void {
  $B2: {
    %3:ptr<storage, u32, read_write> = access %v, 0u
    store %3, 0u
    %4:ptr<storage, vec2<f16>, read_write> = access %v, 1u
    store %4, vec2<f16>(0.0h)
    ret
  }
}
)";

    ASSERT_EQ(src, str());

    auto* expect = R"(
SB = struct @align(4) {
  a:u32 @offset(0)
  b:vec2<f16> @offset(4)
}

$B1: {  # root
  %v:ptr<storage, array<u16, 4>, read_write> = var undef @binding_point(0, 0)
}

%foo = @fragment func():void {
  $B2: {
    %3:vec2<u16> = bitcast<vec2<u16>> 0u
    %4:ptr<storage, u16, read_write> = access %v, 0u
    %5:u16 = access %3, 0u
    store %4, %5 @align(4)
    %6:ptr<storage, u16, read_write> = access %v, 1u
    %7:u16 = access %3, 1u
    store %6, %7
    %8:f16 = access vec2<f16>(0.0h), 0u
    %9:u16 = bitcast<u16> %8
    %10:ptr<storage, u16, read_write> = access %v, 2u
    store %10, %9 @align(4)
    %11:f16 = access vec2<f16>(0.0h), 1u
    %12:u16 = bitcast<u16> %11
    %13:ptr<storage, u16, read_write> = access %v, 3u
    store %13, %12
    ret
  }
}
)";

    DecomposeAccessConfig options{.storage = true};
    Run(DecomposeAccess, options);
    EXPECT_EQ(expect, str());
}

// Note: vec4<f16> also uses u16 (SmallestElementSize=2, due to Width==4 && Type()->Size()==2).

TEST_F(IR_DecomposeAccessTest, Storage_AccessU16_StoreVec4h_WithU32) {
    auto* sb = ty.Struct(mod.symbols.New("SB"), {
                                                    {mod.symbols.New("a"), ty.u32()},
                                                    {mod.symbols.New("b"), ty.vec4h()},
                                                });
    auto* var = b.Var("v", storage, sb, core::Access::kReadWrite);
    var->SetBindingPoint(0, 0);
    b.ir.root_block->Append(var);

    auto* func = b.Function("foo", ty.void_(), core::ir::Function::PipelineStage::kFragment);
    b.Append(func->Block(), [&] {
        b.Store(b.Access(ty.ptr(storage, ty.u32(), core::Access::kReadWrite), var, 0_u), u32(0));
        b.Store(b.Access(ty.ptr(storage, ty.vec4h(), core::Access::kReadWrite), var, 1_u),
                b.Zero(ty.vec4h()));
        b.Return(func);
    });

    auto* src = R"(
SB = struct @align(8) {
  a:u32 @offset(0)
  b:vec4<f16> @offset(8)
}

$B1: {  # root
  %v:ptr<storage, SB, read_write> = var undef @binding_point(0, 0)
}

%foo = @fragment func():void {
  $B2: {
    %3:ptr<storage, u32, read_write> = access %v, 0u
    store %3, 0u
    %4:ptr<storage, vec4<f16>, read_write> = access %v, 1u
    store %4, vec4<f16>(0.0h)
    ret
  }
}
)";

    ASSERT_EQ(src, str());

    auto* expect = R"(
SB = struct @align(8) {
  a:u32 @offset(0)
  b:vec4<f16> @offset(8)
}

$B1: {  # root
  %v:ptr<storage, array<u16, 8>, read_write> = var undef @binding_point(0, 0)
}

%foo = @fragment func():void {
  $B2: {
    %3:vec2<u16> = bitcast<vec2<u16>> 0u
    %4:ptr<storage, u16, read_write> = access %v, 0u
    %5:u16 = access %3, 0u
    store %4, %5 @align(4)
    %6:ptr<storage, u16, read_write> = access %v, 1u
    %7:u16 = access %3, 1u
    store %6, %7
    %8:f16 = access vec4<f16>(0.0h), 0u
    %9:u16 = bitcast<u16> %8
    %10:ptr<storage, u16, read_write> = access %v, 4u
    store %10, %9 @align(8)
    %11:f16 = access vec4<f16>(0.0h), 1u
    %12:u16 = bitcast<u16> %11
    %13:ptr<storage, u16, read_write> = access %v, 5u
    store %13, %12
    %14:f16 = access vec4<f16>(0.0h), 2u
    %15:u16 = bitcast<u16> %14
    %16:ptr<storage, u16, read_write> = access %v, 6u
    store %16, %15
    %17:f16 = access vec4<f16>(0.0h), 3u
    %18:u16 = bitcast<u16> %17
    %19:ptr<storage, u16, read_write> = access %v, 7u
    store %19, %18
    ret
  }
}
)";

    DecomposeAccessConfig options{.storage = true};
    Run(DecomposeAccess, options);
    EXPECT_EQ(expect, str());
}

TEST_F(IR_DecomposeAccessTest, Workgroup_AccessU32_StoreVec2b) {
    auto* sb = ty.Struct(mod.symbols.New("SB"), {
                                                    {mod.symbols.New("a"), ty.u32()},
                                                    {mod.symbols.New("b"), ty.vec2(ty.bool_())},
                                                });
    auto* var = b.Var("v", workgroup, sb, core::Access::kReadWrite);
    b.ir.root_block->Append(var);

    auto* func = b.Function("foo", ty.void_());
    b.Append(func->Block(), [&] {
        b.Store(b.Access(ty.ptr(workgroup, ty.u32(), core::Access::kReadWrite), var, 0_u), u32(0));
        b.Store(
            b.Access(ty.ptr(workgroup, ty.vec2(ty.bool_()), core::Access::kReadWrite), var, 1_u),
            b.Zero(ty.vec2(ty.bool_())));
        b.Return(func);
    });

    auto* src = R"(
SB = struct @align(8) {
  a:u32 @offset(0)
  b:vec2<bool> @offset(8)
}

$B1: {  # root
  %v:ptr<workgroup, SB, read_write> = var undef
}

%foo = func():void {
  $B2: {
    %3:ptr<workgroup, u32, read_write> = access %v, 0u
    store %3, 0u
    %4:ptr<workgroup, vec2<bool>, read_write> = access %v, 1u
    store %4, vec2<bool>(false)
    ret
  }
}
)";

    ASSERT_EQ(src, str());

    auto* expect = R"(
SB = struct @align(8) {
  a:u32 @offset(0)
  b:vec2<bool> @offset(8)
}

$B1: {  # root
  %v:ptr<workgroup, array<u32, 4>, read_write> = var undef
}

%foo = func():void {
  $B2: {
    %3:ptr<workgroup, u32, read_write> = access %v, 0u
    store %3, 0u
    %4:bool = access vec2<bool>(false), 0u
    %5:u32 = convert %4
    %6:ptr<workgroup, u32, read_write> = access %v, 2u
    store %6, %5 @align(8)
    %7:bool = access vec2<bool>(false), 1u
    %8:u32 = convert %7
    %9:ptr<workgroup, u32, read_write> = access %v, 3u
    store %9, %8
    ret
  }
}
)";

    DecomposeAccessConfig options{.workgroup = true};
    Run(DecomposeAccess, options);
    EXPECT_EQ(expect, str());
}

TEST_F(IR_DecomposeAccessTest, Workgroup_AccessU32_StoreVec3b) {
    auto* sb = ty.Struct(mod.symbols.New("SB"), {
                                                    {mod.symbols.New("a"), ty.u32()},
                                                    {mod.symbols.New("b"), ty.vec3(ty.bool_())},
                                                });
    auto* var = b.Var("v", workgroup, sb, core::Access::kReadWrite);
    b.ir.root_block->Append(var);

    auto* func = b.Function("foo", ty.void_());
    b.Append(func->Block(), [&] {
        b.Store(b.Access(ty.ptr(workgroup, ty.u32(), core::Access::kReadWrite), var, 0_u), u32(0));
        b.Store(
            b.Access(ty.ptr(workgroup, ty.vec3(ty.bool_()), core::Access::kReadWrite), var, 1_u),
            b.Zero(ty.vec3(ty.bool_())));
        b.Return(func);
    });

    auto* src = R"(
SB = struct @align(16) {
  a:u32 @offset(0)
  b:vec3<bool> @offset(16)
}

$B1: {  # root
  %v:ptr<workgroup, SB, read_write> = var undef
}

%foo = func():void {
  $B2: {
    %3:ptr<workgroup, u32, read_write> = access %v, 0u
    store %3, 0u
    %4:ptr<workgroup, vec3<bool>, read_write> = access %v, 1u
    store %4, vec3<bool>(false)
    ret
  }
}
)";

    ASSERT_EQ(src, str());

    auto* expect = R"(
SB = struct @align(16) {
  a:u32 @offset(0)
  b:vec3<bool> @offset(16)
}

$B1: {  # root
  %v:ptr<workgroup, array<u32, 8>, read_write> = var undef
}

%foo = func():void {
  $B2: {
    %3:ptr<workgroup, u32, read_write> = access %v, 0u
    store %3, 0u
    %4:bool = access vec3<bool>(false), 0u
    %5:u32 = convert %4
    %6:ptr<workgroup, u32, read_write> = access %v, 4u
    store %6, %5 @align(16)
    %7:bool = access vec3<bool>(false), 1u
    %8:u32 = convert %7
    %9:ptr<workgroup, u32, read_write> = access %v, 5u
    store %9, %8
    %10:bool = access vec3<bool>(false), 2u
    %11:u32 = convert %10
    %12:ptr<workgroup, u32, read_write> = access %v, 6u
    store %12, %11
    ret
  }
}
)";

    DecomposeAccessConfig options{.workgroup = true};
    Run(DecomposeAccess, options);
    EXPECT_EQ(expect, str());
}

TEST_F(IR_DecomposeAccessTest, Workgroup_AccessU32_StoreVec4b) {
    auto* sb = ty.Struct(mod.symbols.New("SB"), {
                                                    {mod.symbols.New("a"), ty.u32()},
                                                    {mod.symbols.New("b"), ty.vec4(ty.bool_())},
                                                });
    auto* var = b.Var("v", workgroup, sb, core::Access::kReadWrite);
    b.ir.root_block->Append(var);

    auto* func = b.Function("foo", ty.void_());
    b.Append(func->Block(), [&] {
        b.Store(b.Access(ty.ptr(workgroup, ty.u32(), core::Access::kReadWrite), var, 0_u), u32(0));
        b.Store(
            b.Access(ty.ptr(workgroup, ty.vec4(ty.bool_()), core::Access::kReadWrite), var, 1_u),
            b.Zero(ty.vec4(ty.bool_())));
        b.Return(func);
    });

    auto* src = R"(
SB = struct @align(16) {
  a:u32 @offset(0)
  b:vec4<bool> @offset(16)
}

$B1: {  # root
  %v:ptr<workgroup, SB, read_write> = var undef
}

%foo = func():void {
  $B2: {
    %3:ptr<workgroup, u32, read_write> = access %v, 0u
    store %3, 0u
    %4:ptr<workgroup, vec4<bool>, read_write> = access %v, 1u
    store %4, vec4<bool>(false)
    ret
  }
}
)";

    ASSERT_EQ(src, str());

    auto* expect = R"(
SB = struct @align(16) {
  a:u32 @offset(0)
  b:vec4<bool> @offset(16)
}

$B1: {  # root
  %v:ptr<workgroup, array<u32, 8>, read_write> = var undef
}

%foo = func():void {
  $B2: {
    %3:ptr<workgroup, u32, read_write> = access %v, 0u
    store %3, 0u
    %4:bool = access vec4<bool>(false), 0u
    %5:u32 = convert %4
    %6:ptr<workgroup, u32, read_write> = access %v, 4u
    store %6, %5 @align(16)
    %7:bool = access vec4<bool>(false), 1u
    %8:u32 = convert %7
    %9:ptr<workgroup, u32, read_write> = access %v, 5u
    store %9, %8
    %10:bool = access vec4<bool>(false), 2u
    %11:u32 = convert %10
    %12:ptr<workgroup, u32, read_write> = access %v, 6u
    store %12, %11
    %13:bool = access vec4<bool>(false), 3u
    %14:u32 = convert %13
    %15:ptr<workgroup, u32, read_write> = access %v, 7u
    store %15, %14
    ret
  }
}
)";

    DecomposeAccessConfig options{.workgroup = true};
    Run(DecomposeAccess, options);
    EXPECT_EQ(expect, str());
}

TEST_F(IR_DecomposeAccessTest, Storage_AccessVec2u_StoreVec4u) {
    auto* sb = ty.Struct(mod.symbols.New("SB"), {
                                                    {mod.symbols.New("a"), ty.vec2u()},
                                                    {mod.symbols.New("b"), ty.vec4u()},
                                                });
    auto* var = b.Var("v", storage, sb, core::Access::kReadWrite);
    var->SetBindingPoint(0, 0);
    b.ir.root_block->Append(var);

    auto* func = b.Function("foo", ty.void_(), core::ir::Function::PipelineStage::kFragment);
    b.Append(func->Block(), [&] {
        b.Store(b.Access(ty.ptr(storage, ty.vec2u(), core::Access::kReadWrite), var, 0_u),
                b.Zero(ty.vec2u()));
        b.Store(b.Access(ty.ptr(storage, ty.vec4u(), core::Access::kReadWrite), var, 1_u),
                b.Zero(ty.vec4u()));
        b.Return(func);
    });

    auto* src = R"(
SB = struct @align(16) {
  a:vec2<u32> @offset(0)
  b:vec4<u32> @offset(16)
}

$B1: {  # root
  %v:ptr<storage, SB, read_write> = var undef @binding_point(0, 0)
}

%foo = @fragment func():void {
  $B2: {
    %3:ptr<storage, vec2<u32>, read_write> = access %v, 0u
    store %3, vec2<u32>(0u)
    %4:ptr<storage, vec4<u32>, read_write> = access %v, 1u
    store %4, vec4<u32>(0u)
    ret
  }
}
)";

    ASSERT_EQ(src, str());

    auto* expect = R"(
SB = struct @align(16) {
  a:vec2<u32> @offset(0)
  b:vec4<u32> @offset(16)
}

$B1: {  # root
  %v:ptr<storage, array<vec2<u32>, 4>, read_write> = var undef @binding_point(0, 0)
}

%foo = @fragment func():void {
  $B2: {
    %3:ptr<storage, vec2<u32>, read_write> = access %v, 0u
    store %3, vec2<u32>(0u)
    %4:vec2<u32> = swizzle vec4<u32>(0u), xy
    %5:ptr<storage, vec2<u32>, read_write> = access %v, 2u
    store %5, %4
    %6:vec2<u32> = swizzle vec4<u32>(0u), zw
    %7:ptr<storage, vec2<u32>, read_write> = access %v, 3u
    store %7, %6
    ret
  }
}
)";

    DecomposeAccessConfig options{.storage = true};
    Run(DecomposeAccess, options);
    EXPECT_EQ(expect, str());
}

TEST_F(IR_DecomposeAccessTest, Storage_RuntimeArray) {
    auto* sb =
        ty.Struct(mod.symbols.New("SB"), {
                                             {mod.symbols.New("a"), ty.runtime_array(ty.u32())},
                                         });
    auto* var = b.Var("v", storage, sb, core::Access::kReadWrite);
    var->SetBindingPoint(0, 0);
    b.ir.root_block->Append(var);

    auto* func = b.Function("foo", ty.void_(), core::ir::Function::PipelineStage::kFragment);
    b.Append(func->Block(), [&] {
        b.Store(b.Access(ty.ptr(storage, ty.u32(), core::Access::kReadWrite), var, 0_u, 5_u), 33_u);
        b.Return(func);
    });

    auto* src = R"(
SB = struct @align(4) {
  a:array<u32> @offset(0)
}

$B1: {  # root
  %v:ptr<storage, SB, read_write> = var undef @binding_point(0, 0)
}

%foo = @fragment func():void {
  $B2: {
    %3:ptr<storage, u32, read_write> = access %v, 0u, 5u
    store %3, 33u
    ret
  }
}
)";
    ASSERT_EQ(src, str());

    auto* expect = R"(
SB = struct @align(4) {
  a:array<u32> @offset(0)
}

$B1: {  # root
  %v:ptr<storage, array<u32>, read_write> = var undef @binding_point(0, 0)
}

%foo = @fragment func():void {
  $B2: {
    %3:ptr<storage, u32, read_write> = access %v, 5u
    store %3, 33u
    ret
  }
}
)";

    DecomposeAccessConfig options{.storage = true};
    Run(DecomposeAccess, options);
    EXPECT_EQ(expect, str());
}

TEST_F(IR_DecomposeAccessTest, Storage_UnsizedBuffer) {
    auto* var = b.Var("v", storage, ty.unsized_buffer(), core::Access::kReadWrite);
    var->SetBindingPoint(0, 0);
    b.ir.root_block->Append(var);

    auto* func = b.Function("foo", ty.void_(), core::ir::Function::PipelineStage::kFragment);
    b.Append(func->Block(), [&] {
        auto* call = b.CallExplicit<core::ir::CoreBuiltinCall>(
            ty.ptr(storage, ty.u32(), core::Access::kReadWrite), core::BuiltinFn::kBufferView,
            Vector<TemplateParameter, 1>{ty.u32()}, var, 16_u);
        b.Store(call, 33_u);
        b.Return(func);
    });

    auto* src = R"(
$B1: {  # root
  %v:ptr<storage, buffer, read_write> = var undef @binding_point(0, 0)
}

%foo = @fragment func():void {
  $B2: {
    %3:ptr<storage, u32, read_write> = bufferView<u32> %v, 16u
    store %3, 33u
    ret
  }
}
)";
    ASSERT_EQ(src, str());

    auto* expect = R"(
$B1: {  # root
  %v:ptr<storage, array<u32>, read_write> = var undef @binding_point(0, 0)
}

%foo = @fragment func():void {
  $B2: {
    %3:ptr<storage, u32, read_write> = access %v, 4u
    store %3, 33u
    ret
  }
}
)";

    DecomposeAccessConfig options{.storage = true};
    Run(DecomposeAccess, options);
    EXPECT_EQ(expect, str());
}

TEST_F(IR_DecomposeAccessTest, Workgroup_SizedBuffer) {
    auto* var = b.Var("v", workgroup, ty.buffer(64u), core::Access::kReadWrite);
    b.ir.root_block->Append(var);

    auto* func = b.Function("foo", ty.void_());
    b.Append(func->Block(), [&] {
        auto* call = b.CallExplicit<core::ir::CoreBuiltinCall>(
            ty.ptr(workgroup, ty.u32(), core::Access::kReadWrite), core::BuiltinFn::kBufferView,
            Vector<TemplateParameter, 1>{ty.u32()}, var, 16_u);
        b.Store(call, 33_u);
        b.Return(func);
    });

    auto* src = R"(
$B1: {  # root
  %v:ptr<workgroup, buffer<64>, read_write> = var undef
}

%foo = func():void {
  $B2: {
    %3:ptr<workgroup, u32, read_write> = bufferView<u32> %v, 16u
    store %3, 33u
    ret
  }
}
)";
    ASSERT_EQ(src, str());

    auto* expect = R"(
$B1: {  # root
  %v:ptr<workgroup, array<u32, 16>, read_write> = var undef
}

%foo = func():void {
  $B2: {
    %3:ptr<workgroup, u32, read_write> = access %v, 4u
    store %3, 33u
    ret
  }
}
)";

    DecomposeAccessConfig options{.workgroup = true};
    Run(DecomposeAccess, options);
    EXPECT_EQ(expect, str());
}

TEST_F(IR_DecomposeAccessTest, Uniform_SizedBuffer) {
    auto* var = b.Var("v", uniform, ty.buffer(128u), core::Access::kRead);
    var->SetBindingPoint(0, 0);
    b.ir.root_block->Append(var);

    auto* func = b.Function("foo", ty.void_(), core::ir::Function::PipelineStage::kFragment);
    b.Append(func->Block(), [&] {
        auto* call = b.CallExplicit<core::ir::CoreBuiltinCall>(
            ty.ptr(uniform, ty.u32(), core::Access::kRead), core::BuiltinFn::kBufferView,
            Vector<TemplateParameter, 1>{ty.u32()}, var, 36_u);
        b.Load(call);
        b.Return(func);
    });

    auto* src = R"(
$B1: {  # root
  %v:ptr<uniform, buffer<128>, read> = var undef @binding_point(0, 0)
}

%foo = @fragment func():void {
  $B2: {
    %3:ptr<uniform, u32, read> = bufferView<u32> %v, 36u
    %4:u32 = load %3
    ret
  }
}
)";
    ASSERT_EQ(src, str());

    auto* expect = R"(
$B1: {  # root
  %v:ptr<uniform, array<vec4<u32>, 8>, read> = var undef @binding_point(0, 0)
}

%foo = @fragment func():void {
  $B2: {
    %3:ptr<uniform, vec4<u32>, read> = access %v, 2u
    %4:u32 = load_vector_element %3, 1u
    ret
  }
}
)";

    DecomposeAccessConfig options{.uniform = true};
    Run(DecomposeAccess, options);
    EXPECT_EQ(expect, str());
}

TEST_F(IR_DecomposeAccessTest, BufferLength_Sized_FromType) {
    auto* var = b.Var("v", uniform, ty.buffer(128u), core::Access::kRead);
    var->SetBindingPoint(0, 0);
    b.ir.root_block->Append(var);

    auto* func = b.Function("foo", ty.void_(), core::ir::Function::PipelineStage::kFragment);
    b.Append(func->Block(), [&] {
        auto* call = b.Call(ty.u32(), core::BuiltinFn::kBufferLength, var);
        b.Let("a", call->Result());
        b.Return(func);
    });

    auto* src = R"(
$B1: {  # root
  %v:ptr<uniform, buffer<128>, read> = var undef @binding_point(0, 0)
}

%foo = @fragment func():void {
  $B2: {
    %3:u32 = bufferLength %v
    %a:u32 = let %3
    ret
  }
}
)";
    ASSERT_EQ(src, str());

    auto* expect = R"(
$B1: {  # root
  %v:ptr<uniform, array<vec4<u32>, 8>, read> = var undef @binding_point(0, 0)
}

%foo = @fragment func():void {
  $B2: {
    %a:u32 = let 128u
    ret
  }
}
)";

    DecomposeAccessConfig options{.uniform = true};
    Run(DecomposeAccess, options);
    EXPECT_EQ(expect, str());
}

TEST_F(IR_DecomposeAccessTest, BufferLength_Unsized) {
    auto* var = b.Var("v", storage, ty.unsized_buffer(), core::Access::kRead);
    var->SetBindingPoint(0, 0);
    b.ir.root_block->Append(var);

    auto* func = b.Function("foo", ty.void_(), core::ir::Function::PipelineStage::kFragment);
    b.Append(func->Block(), [&] {
        auto* call = b.Call(ty.u32(), core::BuiltinFn::kBufferLength, var);
        b.Let("a", call->Result());
        b.Return(func);
    });

    auto* src = R"(
$B1: {  # root
  %v:ptr<storage, buffer, read> = var undef @binding_point(0, 0)
}

%foo = @fragment func():void {
  $B2: {
    %3:u32 = bufferLength %v
    %a:u32 = let %3
    ret
  }
}
)";
    ASSERT_EQ(src, str());

    auto* expect = R"(
$B1: {  # root
  %v:ptr<storage, array<u32>, read> = var undef @binding_point(0, 0)
}

%foo = @fragment func():void {
  $B2: {
    %3:u32 = arrayLength %v
    %4:u32 = mul %3, 4u
    %a:u32 = let %4
    ret
  }
}
)";

    DecomposeAccessConfig options{.uniform = true};
    Run(DecomposeAccess, options);
    EXPECT_EQ(expect, str());
}

TEST_F(IR_DecomposeAccessTest, ArrayLength_U32) {
    auto* var = b.Var("v", storage, ty.runtime_array(ty.u32()), core::Access::kRead);
    var->SetBindingPoint(0, 0);
    b.ir.root_block->Append(var);

    auto* func = b.Function("foo", ty.void_(), core::ir::Function::PipelineStage::kFragment);
    b.Append(func->Block(), [&] {
        auto* call = b.Call(ty.u32(), core::BuiltinFn::kArrayLength, var);
        b.Let("a", call->Result());
        b.Return(func);
    });

    auto* src = R"(
$B1: {  # root
  %v:ptr<storage, array<u32>, read> = var undef @binding_point(0, 0)
}

%foo = @fragment func():void {
  $B2: {
    %3:u32 = arrayLength %v
    %a:u32 = let %3
    ret
  }
}
)";
    ASSERT_EQ(src, str());

    auto* expect = R"(
$B1: {  # root
  %v:ptr<storage, array<u32>, read> = var undef @binding_point(0, 0)
}

%foo = @fragment func():void {
  $B2: {
    %3:u32 = arrayLength %v
    %a:u32 = let %3
    ret
  }
}
)";

    DecomposeAccessConfig options{.storage = true};
    Run(DecomposeAccess, options);
    EXPECT_EQ(expect, str());
}

TEST_F(IR_DecomposeAccessTest, ArrayLength_F16) {
    auto* var = b.Var("v", storage, ty.runtime_array(ty.f16()), core::Access::kRead);
    var->SetBindingPoint(0, 0);
    b.ir.root_block->Append(var);

    auto* func = b.Function("foo", ty.void_(), core::ir::Function::PipelineStage::kFragment);
    b.Append(func->Block(), [&] {
        auto* call = b.Call(ty.u32(), core::BuiltinFn::kArrayLength, var);
        b.Let("a", call->Result());
        b.Return(func);
    });

    auto* src = R"(
$B1: {  # root
  %v:ptr<storage, array<f16>, read> = var undef @binding_point(0, 0)
}

%foo = @fragment func():void {
  $B2: {
    %3:u32 = arrayLength %v
    %a:u32 = let %3
    ret
  }
}
)";
    ASSERT_EQ(src, str());

    auto* expect = R"(
$B1: {  # root
  %v:ptr<storage, array<u16>, read> = var undef @binding_point(0, 0)
}

%foo = @fragment func():void {
  $B2: {
    %3:u32 = arrayLength %v
    %a:u32 = let %3
    ret
  }
}
)";

    DecomposeAccessConfig options{.storage = true};
    Run(DecomposeAccess, options);
    EXPECT_EQ(expect, str());
}

TEST_F(IR_DecomposeAccessTest, ArrayLength_StructMinF16) {
    auto* sb = ty.Struct(mod.symbols.New("SB"), {
                                                    {mod.symbols.New("a"), ty.u16()},
                                                    {mod.symbols.New("b"), ty.array(ty.u32(), 2_u)},
                                                });
    auto* var = b.Var("v", storage, ty.runtime_array(sb), core::Access::kRead);
    var->SetBindingPoint(0, 0);
    b.ir.root_block->Append(var);

    auto* func = b.Function("foo", ty.void_(), core::ir::Function::PipelineStage::kFragment);
    b.Append(func->Block(), [&] {
        auto* call = b.Call(ty.u32(), core::BuiltinFn::kArrayLength, var);
        b.Let("a", call->Result());
        b.Return(func);
    });

    auto* src = R"(
SB = struct @align(4) {
  a:u16 @offset(0)
  b:array<u32, 2> @offset(4)
}

$B1: {  # root
  %v:ptr<storage, array<SB>, read> = var undef @binding_point(0, 0)
}

%foo = @fragment func():void {
  $B2: {
    %3:u32 = arrayLength %v
    %a:u32 = let %3
    ret
  }
}
)";
    ASSERT_EQ(src, str());

    auto* expect = R"(
SB = struct @align(4) {
  a:u16 @offset(0)
  b:array<u32, 2> @offset(4)
}

$B1: {  # root
  %v:ptr<storage, array<u16>, read> = var undef @binding_point(0, 0)
}

%foo = @fragment func():void {
  $B2: {
    %3:u32 = arrayLength %v
    %4:u32 = div %3, 6u
    %a:u32 = let %4
    ret
  }
}
)";

    DecomposeAccessConfig options{.storage = true};
    Run(DecomposeAccess, options);
    EXPECT_EQ(expect, str());
}

TEST_F(IR_DecomposeAccessTest, ArrayLength_StructMinF16_Offset_Access) {
    auto* sb = ty.Struct(mod.symbols.New("SB"), {
                                                    {mod.symbols.New("a"), ty.u16()},
                                                    {mod.symbols.New("b"), ty.array(ty.u32(), 2_u)},
                                                });
    auto* outer =
        ty.Struct(mod.symbols.New("outer"), {
                                                {mod.symbols.New("x"), ty.array(ty.vec4u(), 4_u)},
                                                {mod.symbols.New("y"), ty.runtime_array(sb)},
                                            });
    auto* var = b.Var("v", storage, outer, core::Access::kRead);
    var->SetBindingPoint(0, 0);
    b.ir.root_block->Append(var);

    auto* func = b.Function("foo", ty.void_(), core::ir::Function::PipelineStage::kFragment);
    b.Append(func->Block(), [&] {
        auto* call =
            b.Call(ty.u32(), core::BuiltinFn::kArrayLength,
                   b.Access(ty.ptr(storage, ty.runtime_array(sb), core::Access::kRead), var, 1_u));
        b.Let("a", call->Result());
        b.Return(func);
    });

    auto* src = R"(
SB = struct @align(4) {
  a:u16 @offset(0)
  b:array<u32, 2> @offset(4)
}

outer = struct @align(16) {
  x:array<vec4<u32>, 4> @offset(0)
  y:array<SB> @offset(64)
}

$B1: {  # root
  %v:ptr<storage, outer, read> = var undef @binding_point(0, 0)
}

%foo = @fragment func():void {
  $B2: {
    %3:ptr<storage, array<SB>, read> = access %v, 1u
    %4:u32 = arrayLength %3
    %a:u32 = let %4
    ret
  }
}
)";
    ASSERT_EQ(src, str());

    auto* expect = R"(
SB = struct @align(4) {
  a:u16 @offset(0)
  b:array<u32, 2> @offset(4)
}

outer = struct @align(16) {
  x:array<vec4<u32>, 4> @offset(0)
  y:array<SB> @offset(64)
}

$B1: {  # root
  %v:ptr<storage, array<u16>, read> = var undef @binding_point(0, 0)
}

%foo = @fragment func():void {
  $B2: {
    %3:u32 = arrayLength %v
    %4:u32 = sub %3, 32u
    %5:u32 = div %4, 6u
    %a:u32 = let %5
    ret
  }
}
)";

    DecomposeAccessConfig options{.storage = true};
    Run(DecomposeAccess, options);
    EXPECT_EQ(expect, str());
}

TEST_F(IR_DecomposeAccessTest, ArrayLength_StructMinF16_Offset_BufferView) {
    auto* sb = ty.Struct(mod.symbols.New("SB"), {
                                                    {mod.symbols.New("a"), ty.u16()},
                                                    {mod.symbols.New("b"), ty.array(ty.u32(), 2_u)},
                                                });
    ty.Struct(mod.symbols.New("outer"), {
                                            {mod.symbols.New("x"), ty.array(ty.vec4u(), 4_u)},
                                            {mod.symbols.New("y"), ty.runtime_array(sb)},
                                        });
    auto* var = b.Var("v", storage, ty.unsized_buffer(), core::Access::kRead);
    var->SetBindingPoint(0, 0);
    b.ir.root_block->Append(var);

    auto* func = b.Function("foo", ty.void_(), core::ir::Function::PipelineStage::kFragment);
    b.Append(func->Block(), [&] {
        auto* view = b.CallExplicit(ty.ptr(storage, ty.runtime_array(sb), core::Access::kRead),
                                    core::BuiltinFn::kBufferView,
                                    Vector<TemplateParameter, 1>{ty.runtime_array(sb)}, var, 64_u);
        auto* call = b.Call(ty.u32(), core::BuiltinFn::kArrayLength, view);
        b.Let("a", call->Result());
        b.Return(func);
    });

    auto* src = R"(
SB = struct @align(4) {
  a:u16 @offset(0)
  b:array<u32, 2> @offset(4)
}

outer = struct @align(16) {
  x:array<vec4<u32>, 4> @offset(0)
  y:array<SB> @offset(64)
}

$B1: {  # root
  %v:ptr<storage, buffer, read> = var undef @binding_point(0, 0)
}

%foo = @fragment func():void {
  $B2: {
    %3:ptr<storage, array<SB>, read> = bufferView<array<SB>> %v, 64u
    %4:u32 = arrayLength %3
    %a:u32 = let %4
    ret
  }
}
)";
    ASSERT_EQ(src, str());

    auto* expect = R"(
SB = struct @align(4) {
  a:u16 @offset(0)
  b:array<u32, 2> @offset(4)
}

outer = struct @align(16) {
  x:array<vec4<u32>, 4> @offset(0)
  y:array<SB> @offset(64)
}

$B1: {  # root
  %v:ptr<storage, array<u16>, read> = var undef @binding_point(0, 0)
}

%foo = @fragment func():void {
  $B2: {
    %3:u32 = arrayLength %v
    %4:u32 = sub %3, 32u
    %5:u32 = div %4, 6u
    %a:u32 = let %5
    ret
  }
}
)";

    DecomposeAccessConfig options{.storage = true};
    Run(DecomposeAccess, options);
    EXPECT_EQ(expect, str());
}

TEST_F(IR_DecomposeAccessTest, ArrayLength_StructMinF16_Offset_BufferView_Runtime) {
    auto* sb = ty.Struct(mod.symbols.New("SB"), {
                                                    {mod.symbols.New("a"), ty.u16()},
                                                    {mod.symbols.New("b"), ty.array(ty.u32(), 2_u)},
                                                });
    ty.Struct(mod.symbols.New("outer"), {
                                            {mod.symbols.New("x"), ty.array(ty.vec4u(), 4_u)},
                                            {mod.symbols.New("y"), ty.runtime_array(sb)},
                                        });
    auto* var = b.Var("v", storage, ty.unsized_buffer(), core::Access::kRead);
    var->SetBindingPoint(0, 0);
    b.ir.root_block->Append(var);
    auto* val = b.Var("val", uniform, ty.u32());
    val->SetBindingPoint(0, 1);
    b.ir.root_block->Append(val);

    auto* func = b.Function("foo", ty.void_(), core::ir::Function::PipelineStage::kFragment);
    b.Append(func->Block(), [&] {
        auto* view =
            b.CallExplicit(ty.ptr(storage, ty.runtime_array(sb), core::Access::kRead),
                           core::BuiltinFn::kBufferView,
                           Vector<TemplateParameter, 1>{ty.runtime_array(sb)}, var, b.Load(val));
        auto* call = b.Call(ty.u32(), core::BuiltinFn::kArrayLength, view);
        b.Let("a", call->Result());
        b.Return(func);
    });

    auto* src = R"(
SB = struct @align(4) {
  a:u16 @offset(0)
  b:array<u32, 2> @offset(4)
}

outer = struct @align(16) {
  x:array<vec4<u32>, 4> @offset(0)
  y:array<SB> @offset(64)
}

$B1: {  # root
  %v:ptr<storage, buffer, read> = var undef @binding_point(0, 0)
  %val:ptr<uniform, u32, read> = var undef @binding_point(0, 1)
}

%foo = @fragment func():void {
  $B2: {
    %4:u32 = load %val
    %5:ptr<storage, array<SB>, read> = bufferView<array<SB>> %v, %4
    %6:u32 = arrayLength %5
    %a:u32 = let %6
    ret
  }
}
)";
    ASSERT_EQ(src, str());

    auto* expect = R"(
SB = struct @align(4) {
  a:u16 @offset(0)
  b:array<u32, 2> @offset(4)
}

outer = struct @align(16) {
  x:array<vec4<u32>, 4> @offset(0)
  y:array<SB> @offset(64)
}

$B1: {  # root
  %v:ptr<storage, array<u16>, read> = var undef @binding_point(0, 0)
  %val:ptr<uniform, u32, read> = var undef @binding_point(0, 1)
}

%foo = @fragment func():void {
  $B2: {
    %4:u32 = load %val
    %5:u32 = mul %4, 1u
    %6:u32 = arrayLength %v
    %7:u32 = div %5, 2u
    %8:u32 = sub %6, %7
    %9:u32 = div %8, 6u
    %a:u32 = let %9
    ret
  }
}
)";

    DecomposeAccessConfig options{.storage = true};
    Run(DecomposeAccess, options);
    EXPECT_EQ(expect, str());
}

TEST_F(IR_DecomposeAccessTest, ArrayLength_StructMinF16_Offset_Both) {
    auto* sb = ty.Struct(mod.symbols.New("SB"), {
                                                    {mod.symbols.New("a"), ty.u16()},
                                                    {mod.symbols.New("b"), ty.array(ty.u32(), 2_u)},
                                                });
    auto* outer =
        ty.Struct(mod.symbols.New("outer"), {
                                                {mod.symbols.New("x"), ty.array(ty.vec4u(), 4_u)},
                                                {mod.symbols.New("y"), ty.runtime_array(sb)},
                                            });
    auto* var = b.Var("v", storage, ty.unsized_buffer(), core::Access::kRead);
    var->SetBindingPoint(0, 0);
    b.ir.root_block->Append(var);
    auto* val = b.Var("val", uniform, ty.u32());
    val->SetBindingPoint(0, 1);
    b.ir.root_block->Append(val);

    auto* func = b.Function("foo", ty.void_(), core::ir::Function::PipelineStage::kFragment);
    b.Append(func->Block(), [&] {
        auto* view = b.CallExplicit(ty.ptr(storage, outer, core::Access::kRead),
                                    core::BuiltinFn::kBufferView,
                                    Vector<TemplateParameter, 1>{outer}, var, b.Load(val));
        auto* call =
            b.Call(ty.u32(), core::BuiltinFn::kArrayLength,
                   b.Access(ty.ptr(storage, ty.runtime_array(sb), core::Access::kRead), view, 1_u));
        b.Let("a", call->Result());
        b.Return(func);
    });

    auto* src = R"(
SB = struct @align(4) {
  a:u16 @offset(0)
  b:array<u32, 2> @offset(4)
}

outer = struct @align(16) {
  x:array<vec4<u32>, 4> @offset(0)
  y:array<SB> @offset(64)
}

$B1: {  # root
  %v:ptr<storage, buffer, read> = var undef @binding_point(0, 0)
  %val:ptr<uniform, u32, read> = var undef @binding_point(0, 1)
}

%foo = @fragment func():void {
  $B2: {
    %4:u32 = load %val
    %5:ptr<storage, outer, read> = bufferView<outer> %v, %4
    %6:ptr<storage, array<SB>, read> = access %5, 1u
    %7:u32 = arrayLength %6
    %a:u32 = let %7
    ret
  }
}
)";
    ASSERT_EQ(src, str());

    auto* expect = R"(
SB = struct @align(4) {
  a:u16 @offset(0)
  b:array<u32, 2> @offset(4)
}

outer = struct @align(16) {
  x:array<vec4<u32>, 4> @offset(0)
  y:array<SB> @offset(64)
}

$B1: {  # root
  %v:ptr<storage, array<u16>, read> = var undef @binding_point(0, 0)
  %val:ptr<uniform, u32, read> = var undef @binding_point(0, 1)
}

%foo = @fragment func():void {
  $B2: {
    %4:u32 = load %val
    %5:u32 = mul %4, 1u
    %6:u32 = arrayLength %v
    %7:u32 = add %5, 64u
    %8:u32 = div %7, 2u
    %9:u32 = sub %6, %8
    %10:u32 = div %9, 6u
    %a:u32 = let %10
    ret
  }
}
)";

    DecomposeAccessConfig options{.storage = true};
    Run(DecomposeAccess, options);
    EXPECT_EQ(expect, str());
}

TEST_F(IR_DecomposeAccessTest, LargeUBOIndexing) {
    auto* Input = ty.Struct(mod.symbols.New("Input"),
                            {
                                {mod.symbols.New("vector_index"), ty.u32()},
                                {mod.symbols.New("component_index"), ty.u32()},
                                {mod.symbols.New("data"), ty.array(ty.vec4u(), 500)},
                            });
    auto* input = b.Var("input", uniform, Input, core::Access::kRead);
    input->SetBindingPoint(0, 0);
    b.ir.root_block->Append(input);
    auto* output = b.Var("output", storage, ty.u32(), core::Access::kReadWrite);
    output->SetBindingPoint(0, 1);
    b.ir.root_block->Append(output);

    auto* func = b.Function("foo", ty.void_(), core::ir::Function::PipelineStage::kFragment);
    b.Append(func->Block(), [&] {
        auto* ld_v = b.Load(b.Access(ty.ptr(uniform, ty.u32()), input, 0_u));
        auto* min = b.Call(ty.u32(), core::BuiltinFn::kMin, ld_v, 499_u);
        auto* data_access = b.Access(ty.ptr(uniform, ty.vec4u()), input, 2_u, min);
        auto* ld_cmp = b.Load(b.Access(ty.ptr(uniform, ty.u32()), input, 1_u));
        min = b.Call(ty.u32(), core::BuiltinFn::kMin, ld_cmp, 3_u);
        b.Store(output, b.LoadVectorElement(data_access, min));
        b.Return(func);
    });

    auto* src = R"(
Input = struct @align(16) {
  vector_index:u32 @offset(0)
  component_index:u32 @offset(4)
  data:array<vec4<u32>, 500> @offset(16)
}

$B1: {  # root
  %input:ptr<uniform, Input, read> = var undef @binding_point(0, 0)
  %output:ptr<storage, u32, read_write> = var undef @binding_point(0, 1)
}

%foo = @fragment func():void {
  $B2: {
    %4:ptr<uniform, u32, read> = access %input, 0u
    %5:u32 = load %4
    %6:u32 = min %5, 499u
    %7:ptr<uniform, vec4<u32>, read> = access %input, 2u, %6
    %8:ptr<uniform, u32, read> = access %input, 1u
    %9:u32 = load %8
    %10:u32 = min %9, 3u
    %11:u32 = load_vector_element %7, %10
    store %output, %11
    ret
  }
}
)";
    ASSERT_EQ(src, str());

    auto* expect = R"(
Input = struct @align(16) {
  vector_index:u32 @offset(0)
  component_index:u32 @offset(4)
  data:array<vec4<u32>, 500> @offset(16)
}

$B1: {  # root
  %input:ptr<uniform, array<vec4<u32>, 501>, read> = var undef @binding_point(0, 0)
  %output:ptr<storage, u32, read_write> = var undef @binding_point(0, 1)
}

%foo = @fragment func():void {
  $B2: {
    %4:ptr<uniform, vec4<u32>, read> = access %input, 0u
    %5:u32 = load_vector_element %4, 0u
    %6:u32 = min %5, 499u
    %7:u32 = mul %6, 16u
    %8:ptr<uniform, vec4<u32>, read> = access %input, 0u
    %9:u32 = load_vector_element %8, 1u
    %10:u32 = min %9, 3u
    %11:u32 = mul %10, 4u
    %12:u32 = add 16u, %7
    %13:u32 = add %12, %11
    %14:u32 = div %13, 16u
    %15:ptr<uniform, vec4<u32>, read> = access %input, %14
    %16:u32 = and %13, 15u
    %17:u32 = shr %16, 2u
    %18:u32 = load_vector_element %15, %17
    store %output, %18
    ret
  }
}
)";

    DecomposeAccessConfig options{.uniform = true};
    Run(DecomposeAccess, options);
    EXPECT_EQ(expect, str());
}

TEST_F(IR_DecomposeAccessTest, BufferArrayView_Basic_U32) {
    auto* var = b.Var("v", storage, ty.unsized_buffer(), core::Access::kReadWrite);
    var->SetBindingPoint(0, 0);
    b.ir.root_block->Append(var);

    auto* func = b.Function("foo", ty.void_());
    b.Append(func->Block(), [&] {
        auto* arr_ty = ty.array<u32>();
        auto* call = b.CallExplicit<core::ir::CoreBuiltinCall>(
            ty.ptr(storage, arr_ty, core::Access::kReadWrite), core::BuiltinFn::kBufferArrayView,
            Vector<TemplateParameter, 1>{arr_ty}, var, 16_u, 100_u);
        auto* access = b.Access(ty.ptr(storage, ty.u32(), core::Access::kReadWrite), call, 5_u);
        b.Load(access);
        b.Return(func);
    });

    auto* src = R"(
$B1: {  # root
  %v:ptr<storage, buffer, read_write> = var undef @binding_point(0, 0)
}

%foo = func():void {
  $B2: {
    %3:ptr<storage, array<u32>, read_write> = bufferArrayView<array<u32>> %v, 16u, 100u
    %4:ptr<storage, u32, read_write> = access %3, 5u
    %5:u32 = load %4
    ret
  }
}
)";

    EXPECT_EQ(src, str());

    auto* expect = R"(
$B1: {  # root
  %v:ptr<storage, array<u32>, read_write> = var undef @binding_point(0, 0)
}

%foo = func():void {
  $B2: {
    %3:ptr<storage, u32, read_write> = access %v, 9u
    %4:u32 = load %3
    ret
  }
}
)";

    DecomposeAccessConfig options{};
    Run(DecomposeAccess, options);
    EXPECT_EQ(expect, str());
}

TEST_F(IR_DecomposeAccessTest, BufferArrayView_Basic_Vec4f) {
    auto* var = b.Var("v", storage, ty.unsized_buffer(), core::Access::kReadWrite);
    var->SetBindingPoint(0, 0);
    b.ir.root_block->Append(var);

    auto* func = b.Function("foo", ty.void_());
    b.Append(func->Block(), [&] {
        auto* arr_ty = ty.runtime_array(ty.vec4(ty.f32()));
        auto* call = b.CallExplicit<core::ir::CoreBuiltinCall>(
            ty.ptr(storage, arr_ty, core::Access::kReadWrite), core::BuiltinFn::kBufferArrayView,
            Vector<TemplateParameter, 1>{arr_ty}, var, 16_u, 100_u);
        auto* access =
            b.Access(ty.ptr(storage, ty.vec4(ty.f32()), core::Access::kReadWrite), call, 5_u);
        b.Load(access);
        b.Return(func);
    });

    auto* src = R"(
$B1: {  # root
  %v:ptr<storage, buffer, read_write> = var undef @binding_point(0, 0)
}

%foo = func():void {
  $B2: {
    %3:ptr<storage, array<vec4<f32>>, read_write> = bufferArrayView<array<vec4<f32>>> %v, 16u, 100u
    %4:ptr<storage, vec4<f32>, read_write> = access %3, 5u
    %5:vec4<f32> = load %4
    ret
  }
}
)";

    EXPECT_EQ(src, str());

    auto* expect = R"(
$B1: {  # root
  %v:ptr<storage, array<vec4<u32>>, read_write> = var undef @binding_point(0, 0)
}

%foo = func():void {
  $B2: {
    %3:ptr<storage, vec4<u32>, read_write> = access %v, 6u
    %4:vec4<u32> = load %3
    %5:vec4<f32> = bitcast<vec4<f32>> %4
    ret
  }
}
)";

    DecomposeAccessConfig options{};
    Run(DecomposeAccess, options);
    EXPECT_EQ(expect, str());
}

TEST_F(IR_DecomposeAccessTest, BufferArrayView_Basic_Struct) {
    auto* var = b.Var("v", storage, ty.unsized_buffer(), core::Access::kReadWrite);
    var->SetBindingPoint(0, 0);
    b.ir.root_block->Append(var);

    auto* str_ = ty.Struct(mod.symbols.New("S"), {
                                                     {mod.symbols.Register("a"), ty.vec4(ty.f32())},
                                                     {mod.symbols.Register("b"), ty.vec4(ty.f32())},
                                                 });

    auto* func = b.Function("foo", ty.void_());
    b.Append(func->Block(), [&] {
        auto* arr_ty = ty.runtime_array(str_);
        auto* call = b.CallExplicit<core::ir::CoreBuiltinCall>(
            ty.ptr(storage, arr_ty, core::Access::kReadWrite), core::BuiltinFn::kBufferArrayView,
            Vector<TemplateParameter, 1>{arr_ty}, var, 16_u, 100_u);
        auto* access = b.Access(ty.ptr(storage, str_, core::Access::kReadWrite), call, 5_u);
        b.Load(access);
        b.Return(func);
    });

    auto* src = R"(
S = struct @align(16) {
  a:vec4<f32> @offset(0)
  b:vec4<f32> @offset(16)
}

$B1: {  # root
  %v:ptr<storage, buffer, read_write> = var undef @binding_point(0, 0)
}

%foo = func():void {
  $B2: {
    %3:ptr<storage, array<S>, read_write> = bufferArrayView<array<S>> %v, 16u, 100u
    %4:ptr<storage, S, read_write> = access %3, 5u
    %5:S = load %4
    ret
  }
}
)";

    EXPECT_EQ(src, str());

    auto* expect = R"(
S = struct @align(16) {
  a:vec4<f32> @offset(0)
  b:vec4<f32> @offset(16)
}

$B1: {  # root
  %v:ptr<storage, array<vec4<u32>>, read_write> = var undef @binding_point(0, 0)
}

%foo = func():void {
  $B2: {
    %3:S = call %4, 176u
    ret
  }
}
%4 = func(%start_byte_offset:u32):S {
  $B3: {
    %6:u32 = div %start_byte_offset, 16u
    %7:ptr<storage, vec4<u32>, read_write> = access %v, %6
    %8:vec4<u32> = load %7
    %9:vec4<f32> = bitcast<vec4<f32>> %8
    %10:u32 = add 16u, %start_byte_offset
    %11:u32 = div %10, 16u
    %12:ptr<storage, vec4<u32>, read_write> = access %v, %11
    %13:vec4<u32> = load %12
    %14:vec4<f32> = bitcast<vec4<f32>> %13
    %15:S = construct %9, %14
    ret %15
  }
}
)";

    DecomposeAccessConfig options{};
    Run(DecomposeAccess, options);
    EXPECT_EQ(expect, str());
}

TEST_F(IR_DecomposeAccessTest, ArrayLength_BufferArrayView_Size_U32) {
    auto* var = b.Var("v", storage, ty.unsized_buffer(), core::Access::kReadWrite);
    var->SetBindingPoint(0, 0);
    b.ir.root_block->Append(var);

    auto* func = b.Function("foo", ty.void_());
    b.Append(func->Block(), [&] {
        auto* arr_ty = ty.array<u32>();
        auto* call = b.CallExplicit<core::ir::CoreBuiltinCall>(
            ty.ptr(storage, arr_ty, core::Access::kReadWrite), core::BuiltinFn::kBufferArrayView,
            Vector<TemplateParameter, 1>{arr_ty}, var, 16_u, 100_u);
        auto* len = b.Call(ty.u32(), core::BuiltinFn::kArrayLength, call);
        b.Let("a", len->Result());
        b.Return(func);
    });

    auto* src = R"(
$B1: {  # root
  %v:ptr<storage, buffer, read_write> = var undef @binding_point(0, 0)
}

%foo = func():void {
  $B2: {
    %3:ptr<storage, array<u32>, read_write> = bufferArrayView<array<u32>> %v, 16u, 100u
    %4:u32 = arrayLength %3
    %a:u32 = let %4
    ret
  }
}
)";

    EXPECT_EQ(src, str());

    auto* expect = R"(
$B1: {  # root
  %v:ptr<storage, array<u32>, read_write> = var undef @binding_point(0, 0)
}

%foo = func():void {
  $B2: {
    %3:u32 = div 100u, 4u
    %a:u32 = let %3
    ret
  }
}
)";

    DecomposeAccessConfig options{};
    Run(DecomposeAccess, options);
    EXPECT_EQ(expect, str());
}

TEST_F(IR_DecomposeAccessTest, ArrayLength_BufferArrayView_Size_Vec4f) {
    auto* var = b.Var("v", storage, ty.unsized_buffer(), core::Access::kReadWrite);
    var->SetBindingPoint(0, 0);
    b.ir.root_block->Append(var);

    auto* func = b.Function("foo", ty.void_());
    b.Append(func->Block(), [&] {
        auto* arr_ty = ty.runtime_array(ty.vec4(ty.f32()));
        auto* call = b.CallExplicit<core::ir::CoreBuiltinCall>(
            ty.ptr(storage, arr_ty, core::Access::kReadWrite), core::BuiltinFn::kBufferArrayView,
            Vector<TemplateParameter, 1>{arr_ty}, var, 16_u, 100_u);
        auto* len = b.Call(ty.u32(), core::BuiltinFn::kArrayLength, call);
        b.Let("a", len->Result());
        b.Return(func);
    });

    auto* src = R"(
$B1: {  # root
  %v:ptr<storage, buffer, read_write> = var undef @binding_point(0, 0)
}

%foo = func():void {
  $B2: {
    %3:ptr<storage, array<vec4<f32>>, read_write> = bufferArrayView<array<vec4<f32>>> %v, 16u, 100u
    %4:u32 = arrayLength %3
    %a:u32 = let %4
    ret
  }
}
)";

    EXPECT_EQ(src, str());

    auto* expect = R"(
$B1: {  # root
  %v:ptr<storage, array<vec4<u32>>, read_write> = var undef @binding_point(0, 0)
}

%foo = func():void {
  $B2: {
    %3:u32 = div 100u, 16u
    %a:u32 = let %3
    ret
  }
}
)";

    DecomposeAccessConfig options{};
    Run(DecomposeAccess, options);
    EXPECT_EQ(expect, str());
}

TEST_F(IR_DecomposeAccessTest, ArrayLength_BufferArrayView_Size_Struct) {
    auto* var = b.Var("v", storage, ty.unsized_buffer(), core::Access::kReadWrite);
    var->SetBindingPoint(0, 0);
    b.ir.root_block->Append(var);

    auto* str_ = ty.Struct(mod.symbols.New("S"), {
                                                     {mod.symbols.Register("a"), ty.vec4(ty.f32())},
                                                     {mod.symbols.Register("b"), ty.vec4(ty.f32())},
                                                 });

    auto* func = b.Function("foo", ty.void_());
    b.Append(func->Block(), [&] {
        auto* arr_ty = ty.runtime_array(str_);
        auto* call = b.CallExplicit<core::ir::CoreBuiltinCall>(
            ty.ptr(storage, arr_ty, core::Access::kReadWrite), core::BuiltinFn::kBufferArrayView,
            Vector<TemplateParameter, 1>{arr_ty}, var, 16_u, 100_u);
        auto* len = b.Call(ty.u32(), core::BuiltinFn::kArrayLength, call);
        b.Let("a", len->Result());
        b.Return(func);
    });

    auto* src = R"(
S = struct @align(16) {
  a:vec4<f32> @offset(0)
  b:vec4<f32> @offset(16)
}

$B1: {  # root
  %v:ptr<storage, buffer, read_write> = var undef @binding_point(0, 0)
}

%foo = func():void {
  $B2: {
    %3:ptr<storage, array<S>, read_write> = bufferArrayView<array<S>> %v, 16u, 100u
    %4:u32 = arrayLength %3
    %a:u32 = let %4
    ret
  }
}
)";

    EXPECT_EQ(src, str());

    auto* expect = R"(
S = struct @align(16) {
  a:vec4<f32> @offset(0)
  b:vec4<f32> @offset(16)
}

$B1: {  # root
  %v:ptr<storage, array<vec4<u32>>, read_write> = var undef @binding_point(0, 0)
}

%foo = func():void {
  $B2: {
    %3:u32 = div 100u, 32u
    %a:u32 = let %3
    ret
  }
}
)";

    DecomposeAccessConfig options{};
    Run(DecomposeAccess, options);
    EXPECT_EQ(expect, str());
}

TEST_F(IR_DecomposeAccessTest, ArrayLength_BufferArrayView_Size_RuntimeStruct) {
    auto* var = b.Var("v", storage, ty.unsized_buffer(), core::Access::kReadWrite);
    var->SetBindingPoint(0, 0);
    b.ir.root_block->Append(var);

    auto* str_ =
        ty.Struct(mod.symbols.New("S"), {
                                            {mod.symbols.Register("a"), ty.vec4(ty.f32())},
                                            {mod.symbols.Register("b"), ty.runtime_array(ty.f32())},
                                        });

    auto* func = b.Function("foo", ty.void_());
    auto* offset = b.FunctionParam("offset", ty.u32());
    auto* size = b.FunctionParam("size", ty.u32());
    func->SetParams({offset, size});
    b.Append(func->Block(), [&] {
        auto* call = b.CallExplicit<core::ir::CoreBuiltinCall>(
            ty.ptr(storage, str_, core::Access::kReadWrite), core::BuiltinFn::kBufferArrayView,
            Vector<TemplateParameter, 1>{str_}, var, offset, size);
        auto* access = b.Access(ty.ptr(storage, ty.runtime_array(ty.f32())), call, 1_u);
        auto* len = b.Call(ty.u32(), core::BuiltinFn::kArrayLength, access);
        b.Let("a", len->Result());
        b.Return(func);
    });

    auto* src = R"(
S = struct @align(16) {
  a:vec4<f32> @offset(0)
  b:array<f32> @offset(16)
}

$B1: {  # root
  %v:ptr<storage, buffer, read_write> = var undef @binding_point(0, 0)
}

%foo = func(%offset:u32, %size:u32):void {
  $B2: {
    %5:ptr<storage, S, read_write> = bufferArrayView<S> %v, %offset, %size
    %6:ptr<storage, array<f32>, read_write> = access %5, 1u
    %7:u32 = arrayLength %6
    %a:u32 = let %7
    ret
  }
}
)";

    EXPECT_EQ(src, str());

    auto* expect = R"(
S = struct @align(16) {
  a:vec4<f32> @offset(0)
  b:array<f32> @offset(16)
}

$B1: {  # root
  %v:ptr<storage, array<u32>, read_write> = var undef @binding_point(0, 0)
}

%foo = func(%offset:u32, %size:u32):void {
  $B2: {
    %5:u32 = mul %offset, 1u
    %6:u32 = sub %size, 16u
    %7:u32 = div %6, 4u
    %a:u32 = let %7
    ret
  }
}
)";

    DecomposeAccessConfig options{};
    Run(DecomposeAccess, options);
    EXPECT_EQ(expect, str());
}

TEST_F(IR_DecomposeAccessTest, ArrayLength_BufferView_Length_U32) {
    auto* var = b.Var("v", storage, ty.unsized_buffer(), core::Access::kReadWrite);
    var->SetBindingPoint(0, 0);
    b.ir.root_block->Append(var);

    auto* str_ =
        ty.Struct(mod.symbols.New("S"), {
                                            {mod.symbols.Register("a"), ty.vec4(ty.f32())},
                                            {mod.symbols.Register("b"), ty.runtime_array(ty.f32())},
                                        });

    auto* func = b.Function("foo", ty.void_());
    auto* offset = b.FunctionParam("offset", ty.u32());
    auto* length = b.FunctionParam("length", ty.u32());
    func->SetParams({offset, length});
    b.Append(func->Block(), [&] {
        auto* arr_ty = ty.runtime_array(ty.f32());
        auto* call = b.CallExplicit<core::ir::CoreBuiltinCall>(
            ty.ptr(storage, str_, core::Access::kReadWrite), core::BuiltinFn::kBufferView,
            Vector<TemplateParameter, 1>{str_}, var, offset, length);
        auto* access = b.Access(ty.ptr(storage, arr_ty), call, 1_u);
        auto* len = b.Call(ty.u32(), core::BuiltinFn::kArrayLength, access);
        b.Let("a", len->Result());
        b.Return(func);
    });

    auto* src = R"(
S = struct @align(16) {
  a:vec4<f32> @offset(0)
  b:array<f32> @offset(16)
}

$B1: {  # root
  %v:ptr<storage, buffer, read_write> = var undef @binding_point(0, 0)
}

%foo = func(%offset:u32, %length:u32):void {
  $B2: {
    %5:ptr<storage, S, read_write> = bufferView<S> %v, %offset, %length
    %6:ptr<storage, array<f32>, read_write> = access %5, 1u
    %7:u32 = arrayLength %6
    %a:u32 = let %7
    ret
  }
}
)";

    EXPECT_EQ(src, str());

    auto* expect = R"(
S = struct @align(16) {
  a:vec4<f32> @offset(0)
  b:array<f32> @offset(16)
}

$B1: {  # root
  %v:ptr<storage, array<u32>, read_write> = var undef @binding_point(0, 0)
}

%foo = func(%offset:u32, %length:u32):void {
  $B2: {
    %5:u32 = mul %offset, 1u
    %6:u32 = add 16u, %5
    %7:u32 = sub %length, %6
    %8:u32 = div %7, 4u
    %a:u32 = let %8
    ret
  }
}
)";

    DecomposeAccessConfig options{};
    Run(DecomposeAccess, options);
    EXPECT_EQ(expect, str());
}

TEST_F(IR_DecomposeAccessTest, Workgroup_PhonyLoad_ArrayOfMat2x2F16_Skipped) {
    auto* arr_ty = ty.array(ty.mat2x2<f16>(), 2048);
    auto* var = b.Var("d0", workgroup, arr_ty, core::Access::kReadWrite);
    b.ir.root_block->Append(var);

    auto* func = b.ComputeFunction("main");
    b.Append(func->Block(), [&] {
        b.Load(var);  // phony: `_ = d0;`
        b.Return(func);
    });

    auto* src = R"(
$B1: {  # root
  %d0:ptr<workgroup, array<mat2x2<f16>, 2048>, read_write> = var undef
}

%main = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B2: {
    %3:array<mat2x2<f16>, 2048> = load %d0
    ret
  }
}
)";

    ASSERT_EQ(src, str());

    auto* expect = R"(
$B1: {  # root
  %d0:ptr<workgroup, array<u16, 8192>, read_write> = var undef
}

%main = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B2: {
    ret
  }
}
)";

    DecomposeAccessConfig options{.workgroup = true};
    Run(DecomposeAccess, options);
    EXPECT_EQ(expect, str());
}

TEST_F(IR_DecomposeAccessTest, Workgroup_PhonyLoad_Scalar_NotSkipped) {
    auto* var = b.Var("v", workgroup, ty.u32(), core::Access::kReadWrite);
    b.ir.root_block->Append(var);

    auto* func = b.Function("foo", ty.void_());
    b.Append(func->Block(), [&] {
        b.Load(var);  // phony: `_ = v;`
        b.Return(func);
    });

    auto* src = R"(
$B1: {  # root
  %v:ptr<workgroup, u32, read_write> = var undef
}

%foo = func():void {
  $B2: {
    %3:u32 = load %v
    ret
  }
}
)";

    ASSERT_EQ(src, str());

    auto* expect = R"(
$B1: {  # root
  %v:ptr<workgroup, array<u32, 1>, read_write> = var undef
}

%foo = func():void {
  $B2: {
    %3:ptr<workgroup, u32, read_write> = access %v, 0u
    %4:u32 = load %3
    ret
  }
}
)";

    DecomposeAccessConfig options{.workgroup = true};
    Run(DecomposeAccess, options);
    EXPECT_EQ(expect, str());
}

TEST_F(IR_DecomposeAccessTest, Workgroup_UsedArrayElementLoad_NotSkipped) {
    auto* var = b.Var("v", workgroup, ty.array<u32, 4>(), core::Access::kReadWrite);
    b.ir.root_block->Append(var);

    auto* func = b.Function("foo", ty.void_());
    b.Append(func->Block(), [&] {
        // Load one element of the array and bind the result via `let` -> the result is used.
        auto* access = b.Access(ty.ptr(workgroup, ty.u32(), core::Access::kReadWrite), var, 0_u);
        b.Let("a", b.Load(access));
        b.Return(func);
    });

    auto* src = R"(
$B1: {  # root
  %v:ptr<workgroup, array<u32, 4>, read_write> = var undef
}

%foo = func():void {
  $B2: {
    %3:ptr<workgroup, u32, read_write> = access %v, 0u
    %4:u32 = load %3
    %a:u32 = let %4
    ret
  }
}
)";

    ASSERT_EQ(src, str());

    auto* expect = R"(
$B1: {  # root
  %v:ptr<workgroup, array<u32, 4>, read_write> = var undef
}

%foo = func():void {
  $B2: {
    %3:ptr<workgroup, u32, read_write> = access %v, 0u
    %4:u32 = load %3
    %a:u32 = let %4
    ret
  }
}
)";

    DecomposeAccessConfig options{.workgroup = true};
    Run(DecomposeAccess, options);
    EXPECT_EQ(expect, str());
}

TEST_F(IR_DecomposeAccessTest, Workgroup_SubgroupMatrix) {
    // Subgroup matrix load/store should be no-ops.
    auto* var = b.Var("v", workgroup, ty.array<u32, 1024>());
    b.ir.root_block->Append(var);

    auto* mat_ty = ty.subgroup_matrix(core::SubgroupMatrixKind::kLeft, ty.u32(), 8, 8);
    auto* func = b.Function("foo", ty.void_());
    b.Append(func->Block(), [&] {
        auto* ld = b.CallExplicit(mat_ty, BuiltinFn::kSubgroupMatrixLoad,
                                  Vector<TemplateParameter, 2>{mat_ty, core::Majorness::kRowMajor},
                                  var, 0_u, 8_u);
        b.CallExplicit(ty.void_(), BuiltinFn::kSubgroupMatrixStore,
                       Vector<TemplateParameter, 1>{core::Majorness::kRowMajor}, var, 0_u, ld, 8_u);
        b.Return(func);
    });

    auto* src = R"(
$B1: {  # root
  %v:ptr<workgroup, array<u32, 1024>, read_write> = var undef
}

%foo = func():void {
  $B2: {
    %3:subgroup_matrix_left<u32, 8, 8> = subgroupMatrixLoad<subgroup_matrix_left<u32, 8, 8>, row_major> %v, 0u, 8u
    %4:void = subgroupMatrixStore<row_major> %v, 0u, %3, 8u
    ret
  }
}
)";

    ASSERT_EQ(src, str());

    auto* expect = R"(
$B1: {  # root
  %v:ptr<workgroup, array<u32, 1024>, read_write> = var undef
}

%foo = func():void {
  $B2: {
    %3:subgroup_matrix_left<u32, 8, 8> = subgroupMatrixLoad<subgroup_matrix_left<u32, 8, 8>, row_major> %v, 0u, 8u
    %4:void = subgroupMatrixStore<row_major> %v, 0u, %3, 8u
    ret
  }
}
)";

    DecomposeAccessConfig options{.workgroup = true};
    Run(DecomposeAccess, options);
    EXPECT_EQ(expect, str());
}

TEST_F(IR_DecomposeAccessTest, Workgroup_SubgroupMatrix_U8_Buffer) {
    auto* v = b.Var("v", workgroup, ty.buffer(1024));
    mod.root_block->Append(v);

    auto* mat_ty = ty.subgroup_matrix(core::SubgroupMatrixKind::kLeft, ty.u8(), 8, 8);
    auto* func = b.Function("foo", ty.void_());
    auto* m = b.FunctionParam("m", mat_ty);
    auto* b_offset = b.FunctionParam("b_offset", ty.u32());
    auto* m_offset = b.FunctionParam("m_offset", ty.u32());
    auto* m_stride = b.FunctionParam("m_stride", ty.u32());
    func->SetParams({m, b_offset, m_offset, m_stride});
    b.Append(func->Block(), [&] {
        auto* view =
            b.CallExplicit(ty.ptr(workgroup, ty.runtime_array(ty.u32())), BuiltinFn::kBufferView,
                           Vector<TemplateParameter, 1>{ty.runtime_array(ty.u32())}, v, b_offset);
        b.CallExplicit(ty.void_(), BuiltinFn::kSubgroupMatrixStore,
                       Vector<TemplateParameter, 1>{Majorness::kColMajor}, view, m_offset, m,
                       m_stride);
        b.Return(func);
    });

    auto* src = R"(
$B1: {  # root
  %v:ptr<workgroup, buffer<1024>, read_write> = var undef
}

%foo = func(%m:subgroup_matrix_left<u8, 8, 8>, %b_offset:u32, %m_offset:u32, %m_stride:u32):void {
  $B2: {
    %7:ptr<workgroup, array<u32>, read_write> = bufferView<array<u32>> %v, %b_offset
    %8:void = subgroupMatrixStore<col_major> %7, %m_offset, %m, %m_stride
    ret
  }
}
)";

    ASSERT_EQ(src, str());

    auto* expect = R"(
$B1: {  # root
  %v:ptr<workgroup, array<u32, 256>, read_write> = var undef
}

%foo = func(%m:subgroup_matrix_left<u8, 8, 8>, %b_offset:u32, %m_offset:u32, %m_stride:u32):void {
  $B2: {
    %7:u32 = mul %b_offset, 1u
    %8:u32 = div %7, 4u
    %9:u32 = add %8, %m_offset
    %10:void = subgroupMatrixStore<col_major> %v, %9, %m, %m_stride
    ret
  }
}
)";

    DecomposeAccessConfig options{.workgroup = true};
    Run(DecomposeAccess, options);
    EXPECT_EQ(expect, str());
}

TEST_F(IR_DecomposeAccessTest, Workgroup_SubgroupMatrix_U32_Buffer_SmallerAccess) {
    mod.properties.Add(core::ir::Property::kAllow16BitFloats);
    auto* S =
        ty.Struct(mod.symbols.New("S"), {
                                            {mod.symbols.New("a"), ty.f16()},
                                            {mod.symbols.New("b"), ty.runtime_array(ty.u32())},
                                        });
    auto* v = b.Var("v", workgroup, ty.buffer(1024));
    mod.root_block->Append(v);

    auto* mat_ty = ty.subgroup_matrix(core::SubgroupMatrixKind::kLeft, ty.u32(), 8, 8);
    auto* func = b.Function("foo", ty.void_());
    auto* b_offset = b.FunctionParam("b_offset", ty.u32());
    auto* m_offset = b.FunctionParam("m_offset", ty.i32());
    auto* m_stride = b.FunctionParam("m_stride", ty.i32());
    func->SetParams({b_offset, m_offset, m_stride});
    b.Append(func->Block(), [&] {
        auto* view = b.CallExplicit(ty.ptr(workgroup, S), BuiltinFn::kBufferView,
                                    Vector<TemplateParameter, 1>{S}, v, b_offset);
        auto* access = b.Access(ty.ptr(workgroup, ty.runtime_array(ty.u32())), view, 1_u);
        b.CallExplicit(mat_ty, BuiltinFn::kSubgroupMatrixLoad,
                       Vector<TemplateParameter, 2>{mat_ty, Majorness::kRowMajor}, access, m_offset,
                       m_stride);
        b.Load(b.Access(ty.ptr(workgroup, ty.f16()), view, 0_u));
        b.Return(func);
    });

    auto* src = R"(
S = struct @align(4) {
  a:f16 @offset(0)
  b:array<u32> @offset(4)
}

$B1: {  # root
  %v:ptr<workgroup, buffer<1024>, read_write> = var undef
}

%foo = func(%b_offset:u32, %m_offset:i32, %m_stride:i32):void {
  $B2: {
    %6:ptr<workgroup, S, read_write> = bufferView<S> %v, %b_offset
    %7:ptr<workgroup, array<u32>, read_write> = access %6, 1u
    %8:subgroup_matrix_left<u32, 8, 8> = subgroupMatrixLoad<subgroup_matrix_left<u32, 8, 8>, row_major> %7, %m_offset, %m_stride
    %9:ptr<workgroup, f16, read_write> = access %6, 0u
    %10:f16 = load %9
    ret
  }
}
)";

    ASSERT_EQ(src, str());

    auto* expect = R"(
S = struct @align(4) {
  a:f16 @offset(0)
  b:array<u32> @offset(4)
}

$B1: {  # root
  %v:ptr<workgroup, array<u16, 512>, read_write> = var undef
}

%foo = func(%b_offset:u32, %m_offset:i32, %m_stride:i32):void {
  $B2: {
    %6:u32 = mul %b_offset, 1u
    %7:u32 = add 4u, %6
    %8:u32 = div %7, 2u
    %9:u32 = bitcast<u32> %m_offset
    %10:u32 = mul %9, 4u
    %11:u32 = div %10, 2u
    %12:u32 = add %8, %11
    %13:u32 = bitcast<u32> %m_stride
    %14:u32 = mul %13, 4u
    %15:u32 = div %14, 2u
    %16:subgroup_matrix_left<u32, 8, 8> = subgroupMatrixLoad<subgroup_matrix_left<u32, 8, 8>, row_major> %v, %12, %15 @align(4)
    %17:u32 = div %6, 2u
    %18:ptr<workgroup, u16, read_write> = access %v, %17
    %19:u16 = load %18
    %20:f16 = bitcast<f16> %19
    ret
  }
}
)";

    DecomposeAccessConfig options{.workgroup = true};
    Run(DecomposeAccess, options);
    EXPECT_EQ(expect, str());
}

TEST_F(IR_DecomposeAccessTest, Storage_SubgroupMatrix_F16_RuntimeArray_F32_Access_F32) {
    auto* mat_ty = ty.subgroup_matrix(SubgroupMatrixKind::kLeft, ty.f16(), 8, 8);
    auto* v = b.Var("v", ty.ptr(storage, ty.runtime_array(ty.f32())));
    v->SetBindingPoint(0, 0);
    mod.root_block->Append(v);

    auto* foo = b.Function("foo", ty.void_());
    auto* m = b.FunctionParam("m", mat_ty);
    auto* offset = b.FunctionParam("offset", ty.u32());
    auto* stride = b.FunctionParam("stride", ty.u32());
    foo->SetParams({m, offset, stride});
    b.Append(foo->Block(), [&] {
        b.CallExplicit(ty.void_(), BuiltinFn::kSubgroupMatrixStore,
                       Vector<TemplateParameter, 1>{Majorness::kColMajor}, v, offset, m, stride);
        b.Return(foo);
    });

    auto* src = R"(
$B1: {  # root
  %v:ptr<storage, array<f32>, read_write> = var undef @binding_point(0, 0)
}

%foo = func(%m:subgroup_matrix_left<f16, 8, 8>, %offset:u32, %stride:u32):void {
  $B2: {
    %6:void = subgroupMatrixStore<col_major> %v, %offset, %m, %stride
    ret
  }
}
)";

    ASSERT_EQ(src, str());

    auto* expect = R"(
$B1: {  # root
  %v:ptr<storage, array<u32>, read_write> = var undef @binding_point(0, 0)
}

%foo = func(%m:subgroup_matrix_left<f16, 8, 8>, %offset:u32, %stride:u32):void {
  $B2: {
    %6:void = subgroupMatrixStore<col_major> %v, %offset, %m, %stride
    ret
  }
}
)";

    DecomposeAccessConfig options{.storage = true};
    Run(DecomposeAccess, options);
    EXPECT_EQ(expect, str());
}

TEST_F(IR_DecomposeAccessTest, Storage_SubgroupMatrix_F16_RuntimeArray_Vec2F_Access_F32) {
    auto* mat_ty = ty.subgroup_matrix(SubgroupMatrixKind::kLeft, ty.f16(), 8, 8);
    auto* v = b.Var("v", ty.ptr(storage, ty.runtime_array(ty.vec2f())));
    v->SetBindingPoint(0, 0);
    mod.root_block->Append(v);

    auto* foo = b.Function("foo", ty.void_());
    auto* m = b.FunctionParam("m", mat_ty);
    auto* offset = b.FunctionParam("offset", ty.u32());
    auto* stride = b.FunctionParam("stride", ty.u32());
    foo->SetParams({m, offset, stride});
    b.Append(foo->Block(), [&] {
        b.CallExplicit(ty.void_(), BuiltinFn::kSubgroupMatrixStore,
                       Vector<TemplateParameter, 1>{Majorness::kColMajor}, v, offset, m, stride);
        b.LoadVectorElement(b.Access(ty.ptr(storage, ty.vec2f()), v, 0_u), 0_u);
        b.Return(foo);
    });

    auto* src = R"(
$B1: {  # root
  %v:ptr<storage, array<vec2<f32>>, read_write> = var undef @binding_point(0, 0)
}

%foo = func(%m:subgroup_matrix_left<f16, 8, 8>, %offset:u32, %stride:u32):void {
  $B2: {
    %6:void = subgroupMatrixStore<col_major> %v, %offset, %m, %stride
    %7:ptr<storage, vec2<f32>, read_write> = access %v, 0u
    %8:f32 = load_vector_element %7, 0u
    ret
  }
}
)";

    ASSERT_EQ(src, str());

    auto* expect = R"(
$B1: {  # root
  %v:ptr<storage, array<u32>, read_write> = var undef @binding_point(0, 0)
}

%foo = func(%m:subgroup_matrix_left<f16, 8, 8>, %offset:u32, %stride:u32):void {
  $B2: {
    %6:u32 = div 0u, 4u
    %7:u32 = mul %offset, 8u
    %8:u32 = div %7, 4u
    %9:u32 = add %6, %8
    %10:u32 = mul %stride, 8u
    %11:u32 = div %10, 4u
    %12:void = subgroupMatrixStore<col_major> %v, %9, %m, %11 @align(8)
    %13:ptr<storage, u32, read_write> = access %v, 0u
    %14:u32 = load %13
    %15:f32 = bitcast<f32> %14
    ret
  }
}
)";

    DecomposeAccessConfig options{.storage = true};
    Run(DecomposeAccess, options);
    EXPECT_EQ(expect, str());
}

TEST_F(IR_DecomposeAccessTest, Storage_SubgroupMatrix_F16_RuntimeArray_Vec3F_Access_F32) {
    auto* mat_ty = ty.subgroup_matrix(SubgroupMatrixKind::kLeft, ty.f16(), 8, 8);
    auto* v = b.Var("v", ty.ptr(storage, ty.runtime_array(ty.vec3f())));
    v->SetBindingPoint(0, 0);
    mod.root_block->Append(v);

    auto* foo = b.Function("foo", ty.void_());
    auto* m = b.FunctionParam("m", mat_ty);
    auto* offset = b.FunctionParam("offset", ty.u32());
    auto* stride = b.FunctionParam("stride", ty.u32());
    foo->SetParams({m, offset, stride});
    b.Append(foo->Block(), [&] {
        b.CallExplicit(ty.void_(), BuiltinFn::kSubgroupMatrixStore,
                       Vector<TemplateParameter, 1>{Majorness::kColMajor}, v, offset, m, stride);
        b.LoadVectorElement(b.Access(ty.ptr(storage, ty.vec3f()), v, 0_u), 0_u);
        b.Return(foo);
    });

    auto* src = R"(
$B1: {  # root
  %v:ptr<storage, array<vec3<f32>>, read_write> = var undef @binding_point(0, 0)
}

%foo = func(%m:subgroup_matrix_left<f16, 8, 8>, %offset:u32, %stride:u32):void {
  $B2: {
    %6:void = subgroupMatrixStore<col_major> %v, %offset, %m, %stride
    %7:ptr<storage, vec3<f32>, read_write> = access %v, 0u
    %8:f32 = load_vector_element %7, 0u
    ret
  }
}
)";

    ASSERT_EQ(src, str());

    auto* expect = R"(
$B1: {  # root
  %v:ptr<storage, array<u32>, read_write> = var undef @binding_point(0, 0)
}

%foo = func(%m:subgroup_matrix_left<f16, 8, 8>, %offset:u32, %stride:u32):void {
  $B2: {
    %6:u32 = div 0u, 4u
    %7:u32 = mul %offset, 16u
    %8:u32 = div %7, 4u
    %9:u32 = add %6, %8
    %10:u32 = mul %stride, 16u
    %11:u32 = div %10, 4u
    %12:void = subgroupMatrixStore<col_major> %v, %9, %m, %11 @align(16)
    %13:ptr<storage, u32, read_write> = access %v, 0u
    %14:u32 = load %13
    %15:f32 = bitcast<f32> %14
    ret
  }
}
)";

    DecomposeAccessConfig options{.storage = true};
    Run(DecomposeAccess, options);
    EXPECT_EQ(expect, str());
}

TEST_F(IR_DecomposeAccessTest, Storage_SubgroupMatrix_F16_RuntimeArray_Vec4F_Access_F32) {
    auto* mat_ty = ty.subgroup_matrix(SubgroupMatrixKind::kLeft, ty.f16(), 8, 8);
    auto* v = b.Var("v", ty.ptr(storage, ty.runtime_array(ty.vec4f())));
    v->SetBindingPoint(0, 0);
    mod.root_block->Append(v);

    auto* foo = b.Function("foo", ty.void_());
    auto* m = b.FunctionParam("m", mat_ty);
    auto* offset = b.FunctionParam("offset", ty.u32());
    auto* stride = b.FunctionParam("stride", ty.u32());
    foo->SetParams({m, offset, stride});
    b.Append(foo->Block(), [&] {
        b.CallExplicit(ty.void_(), BuiltinFn::kSubgroupMatrixStore,
                       Vector<TemplateParameter, 1>{Majorness::kColMajor}, v, offset, m, stride);
        b.LoadVectorElement(b.Access(ty.ptr(storage, ty.vec4f()), v, 0_u), 0_u);
        b.Return(foo);
    });

    auto* src = R"(
$B1: {  # root
  %v:ptr<storage, array<vec4<f32>>, read_write> = var undef @binding_point(0, 0)
}

%foo = func(%m:subgroup_matrix_left<f16, 8, 8>, %offset:u32, %stride:u32):void {
  $B2: {
    %6:void = subgroupMatrixStore<col_major> %v, %offset, %m, %stride
    %7:ptr<storage, vec4<f32>, read_write> = access %v, 0u
    %8:f32 = load_vector_element %7, 0u
    ret
  }
}
)";

    ASSERT_EQ(src, str());

    auto* expect = R"(
$B1: {  # root
  %v:ptr<storage, array<u32>, read_write> = var undef @binding_point(0, 0)
}

%foo = func(%m:subgroup_matrix_left<f16, 8, 8>, %offset:u32, %stride:u32):void {
  $B2: {
    %6:u32 = div 0u, 4u
    %7:u32 = mul %offset, 16u
    %8:u32 = div %7, 4u
    %9:u32 = add %6, %8
    %10:u32 = mul %stride, 16u
    %11:u32 = div %10, 4u
    %12:void = subgroupMatrixStore<col_major> %v, %9, %m, %11 @align(16)
    %13:ptr<storage, u32, read_write> = access %v, 0u
    %14:u32 = load %13
    %15:f32 = bitcast<f32> %14
    ret
  }
}
)";

    DecomposeAccessConfig options{.storage = true};
    Run(DecomposeAccess, options);
    EXPECT_EQ(expect, str());
}

TEST_F(IR_DecomposeAccessTest, Workgroup_SubgroupMatrix_F32_Array_U32_Access_F16) {
    auto* arr_ty = ty.array(ty.u32(), 1024);
    auto* S = ty.Struct(mod.symbols.New("S"), {
                                                  {mod.symbols.New("a"), ty.f16()},
                                                  {mod.symbols.New("b"), arr_ty},
                                              });
    auto* mat_ty = ty.subgroup_matrix(core::SubgroupMatrixKind::kRight, ty.f32(), 8, 8);
    auto* v = b.Var("v", ty.ptr(workgroup, S));
    mod.root_block->Append(v);

    auto* foo = b.Function("foo", ty.void_());
    auto* offset = b.FunctionParam("offset", ty.u32());
    auto* stride = b.FunctionParam("stride", ty.u32());
    foo->SetParams({offset, stride});
    b.Append(foo->Block(), [&] {
        auto* access = b.Access(ty.ptr(workgroup, arr_ty), v, 1_u);
        b.CallExplicit(mat_ty, BuiltinFn::kSubgroupMatrixLoad,
                       Vector<TemplateParameter, 2>{mat_ty, Majorness::kRowMajor}, access, offset,
                       stride);
        access = b.Access(ty.ptr(workgroup, ty.f16()), v, 0_u);
        b.Load(access);
        b.Return(foo);
    });

    auto* src = R"(
S = struct @align(4) {
  a:f16 @offset(0)
  b:array<u32, 1024> @offset(4)
}

$B1: {  # root
  %v:ptr<workgroup, S, read_write> = var undef
}

%foo = func(%offset:u32, %stride:u32):void {
  $B2: {
    %5:ptr<workgroup, array<u32, 1024>, read_write> = access %v, 1u
    %6:subgroup_matrix_right<f32, 8, 8> = subgroupMatrixLoad<subgroup_matrix_right<f32, 8, 8>, row_major> %5, %offset, %stride
    %7:ptr<workgroup, f16, read_write> = access %v, 0u
    %8:f16 = load %7
    ret
  }
}
)";

    ASSERT_EQ(src, str());

    auto* expect = R"(
S = struct @align(4) {
  a:f16 @offset(0)
  b:array<u32, 1024> @offset(4)
}

$B1: {  # root
  %v:ptr<workgroup, array<u16, 2050>, read_write> = var undef
}

%foo = func(%offset:u32, %stride:u32):void {
  $B2: {
    %5:u32 = div 4u, 2u
    %6:u32 = mul %offset, 4u
    %7:u32 = div %6, 2u
    %8:u32 = add %5, %7
    %9:u32 = mul %stride, 4u
    %10:u32 = div %9, 2u
    %11:subgroup_matrix_right<f32, 8, 8> = subgroupMatrixLoad<subgroup_matrix_right<f32, 8, 8>, row_major> %v, %8, %10 @align(4)
    %12:ptr<workgroup, u16, read_write> = access %v, 0u
    %13:u16 = load %12
    %14:f16 = bitcast<f16> %13
    ret
  }
}
)";

    DecomposeAccessConfig options{.workgroup = true};
    Run(DecomposeAccess, options);
    EXPECT_EQ(expect, str());
}

TEST_F(IR_DecomposeAccessTest, Workgroup_SubgroupMatrix_F32_Array_Vec2U_Access_F16) {
    auto* arr_ty = ty.array(ty.vec2u(), 1024);
    auto* S = ty.Struct(mod.symbols.New("S"), {
                                                  {mod.symbols.New("a"), ty.f16()},
                                                  {mod.symbols.New("b"), arr_ty},
                                              });
    auto* mat_ty = ty.subgroup_matrix(core::SubgroupMatrixKind::kRight, ty.f32(), 8, 8);
    auto* v = b.Var("v", ty.ptr(workgroup, S));
    mod.root_block->Append(v);

    auto* foo = b.Function("foo", ty.void_());
    auto* offset = b.FunctionParam("offset", ty.u32());
    auto* stride = b.FunctionParam("stride", ty.u32());
    foo->SetParams({offset, stride});
    b.Append(foo->Block(), [&] {
        auto* access = b.Access(ty.ptr(workgroup, arr_ty), v, 1_u);
        b.CallExplicit(mat_ty, BuiltinFn::kSubgroupMatrixLoad,
                       Vector<TemplateParameter, 2>{mat_ty, Majorness::kRowMajor}, access, offset,
                       stride);
        access = b.Access(ty.ptr(workgroup, ty.f16()), v, 0_u);
        b.Load(access);
        b.Return(foo);
    });

    auto* src = R"(
S = struct @align(8) {
  a:f16 @offset(0)
  b:array<vec2<u32>, 1024> @offset(8)
}

$B1: {  # root
  %v:ptr<workgroup, S, read_write> = var undef
}

%foo = func(%offset:u32, %stride:u32):void {
  $B2: {
    %5:ptr<workgroup, array<vec2<u32>, 1024>, read_write> = access %v, 1u
    %6:subgroup_matrix_right<f32, 8, 8> = subgroupMatrixLoad<subgroup_matrix_right<f32, 8, 8>, row_major> %5, %offset, %stride
    %7:ptr<workgroup, f16, read_write> = access %v, 0u
    %8:f16 = load %7
    ret
  }
}
)";

    ASSERT_EQ(src, str());

    auto* expect = R"(
S = struct @align(8) {
  a:f16 @offset(0)
  b:array<vec2<u32>, 1024> @offset(8)
}

$B1: {  # root
  %v:ptr<workgroup, array<u16, 4100>, read_write> = var undef
}

%foo = func(%offset:u32, %stride:u32):void {
  $B2: {
    %5:u32 = div 8u, 2u
    %6:u32 = mul %offset, 8u
    %7:u32 = div %6, 2u
    %8:u32 = add %5, %7
    %9:u32 = mul %stride, 8u
    %10:u32 = div %9, 2u
    %11:subgroup_matrix_right<f32, 8, 8> = subgroupMatrixLoad<subgroup_matrix_right<f32, 8, 8>, row_major> %v, %8, %10 @align(8)
    %12:ptr<workgroup, u16, read_write> = access %v, 0u
    %13:u16 = load %12
    %14:f16 = bitcast<f16> %13
    ret
  }
}
)";

    DecomposeAccessConfig options{.workgroup = true};
    Run(DecomposeAccess, options);
    EXPECT_EQ(expect, str());
}

TEST_F(IR_DecomposeAccessTest, Workgroup_SubgroupMatrix_F32_Array_Vec3U_Access_F16) {
    auto* arr_ty = ty.array(ty.vec3u(), 1024);
    auto* S = ty.Struct(mod.symbols.New("S"), {
                                                  {mod.symbols.New("a"), ty.f16()},
                                                  {mod.symbols.New("b"), arr_ty},
                                              });
    auto* mat_ty = ty.subgroup_matrix(core::SubgroupMatrixKind::kRight, ty.f32(), 8, 8);
    auto* v = b.Var("v", ty.ptr(workgroup, S));
    mod.root_block->Append(v);

    auto* foo = b.Function("foo", ty.void_());
    auto* offset = b.FunctionParam("offset", ty.u32());
    auto* stride = b.FunctionParam("stride", ty.u32());
    foo->SetParams({offset, stride});
    b.Append(foo->Block(), [&] {
        auto* access = b.Access(ty.ptr(workgroup, arr_ty), v, 1_u);
        b.CallExplicit(mat_ty, BuiltinFn::kSubgroupMatrixLoad,
                       Vector<TemplateParameter, 2>{mat_ty, Majorness::kRowMajor}, access, offset,
                       stride);
        access = b.Access(ty.ptr(workgroup, ty.f16()), v, 0_u);
        b.Load(access);
        b.Return(foo);
    });

    auto* src = R"(
S = struct @align(16) {
  a:f16 @offset(0)
  b:array<vec3<u32>, 1024> @offset(16)
}

$B1: {  # root
  %v:ptr<workgroup, S, read_write> = var undef
}

%foo = func(%offset:u32, %stride:u32):void {
  $B2: {
    %5:ptr<workgroup, array<vec3<u32>, 1024>, read_write> = access %v, 1u
    %6:subgroup_matrix_right<f32, 8, 8> = subgroupMatrixLoad<subgroup_matrix_right<f32, 8, 8>, row_major> %5, %offset, %stride
    %7:ptr<workgroup, f16, read_write> = access %v, 0u
    %8:f16 = load %7
    ret
  }
}
)";

    ASSERT_EQ(src, str());

    auto* expect = R"(
S = struct @align(16) {
  a:f16 @offset(0)
  b:array<vec3<u32>, 1024> @offset(16)
}

$B1: {  # root
  %v:ptr<workgroup, array<u16, 8200>, read_write> = var undef
}

%foo = func(%offset:u32, %stride:u32):void {
  $B2: {
    %5:u32 = div 16u, 2u
    %6:u32 = mul %offset, 16u
    %7:u32 = div %6, 2u
    %8:u32 = add %5, %7
    %9:u32 = mul %stride, 16u
    %10:u32 = div %9, 2u
    %11:subgroup_matrix_right<f32, 8, 8> = subgroupMatrixLoad<subgroup_matrix_right<f32, 8, 8>, row_major> %v, %8, %10 @align(16)
    %12:ptr<workgroup, u16, read_write> = access %v, 0u
    %13:u16 = load %12
    %14:f16 = bitcast<f16> %13
    ret
  }
}
)";

    DecomposeAccessConfig options{.workgroup = true};
    Run(DecomposeAccess, options);
    EXPECT_EQ(expect, str());
}

TEST_F(IR_DecomposeAccessTest, Workgroup_SubgroupMatrix_F32_Array_Vec4U_Access_F16) {
    auto* arr_ty = ty.array(ty.vec4u(), 1024);
    auto* S = ty.Struct(mod.symbols.New("S"), {
                                                  {mod.symbols.New("a"), ty.f16()},
                                                  {mod.symbols.New("b"), arr_ty},
                                              });
    auto* mat_ty = ty.subgroup_matrix(core::SubgroupMatrixKind::kRight, ty.f32(), 8, 8);
    auto* v = b.Var("v", ty.ptr(workgroup, S));
    mod.root_block->Append(v);

    auto* foo = b.Function("foo", ty.void_());
    auto* offset = b.FunctionParam("offset", ty.u32());
    auto* stride = b.FunctionParam("stride", ty.u32());
    foo->SetParams({offset, stride});
    b.Append(foo->Block(), [&] {
        auto* access = b.Access(ty.ptr(workgroup, arr_ty), v, 1_u);
        b.CallExplicit(mat_ty, BuiltinFn::kSubgroupMatrixLoad,
                       Vector<TemplateParameter, 2>{mat_ty, Majorness::kRowMajor}, access, offset,
                       stride);
        access = b.Access(ty.ptr(workgroup, ty.f16()), v, 0_u);
        b.Load(access);
        b.Return(foo);
    });

    auto* src = R"(
S = struct @align(16) {
  a:f16 @offset(0)
  b:array<vec4<u32>, 1024> @offset(16)
}

$B1: {  # root
  %v:ptr<workgroup, S, read_write> = var undef
}

%foo = func(%offset:u32, %stride:u32):void {
  $B2: {
    %5:ptr<workgroup, array<vec4<u32>, 1024>, read_write> = access %v, 1u
    %6:subgroup_matrix_right<f32, 8, 8> = subgroupMatrixLoad<subgroup_matrix_right<f32, 8, 8>, row_major> %5, %offset, %stride
    %7:ptr<workgroup, f16, read_write> = access %v, 0u
    %8:f16 = load %7
    ret
  }
}
)";

    ASSERT_EQ(src, str());

    auto* expect = R"(
S = struct @align(16) {
  a:f16 @offset(0)
  b:array<vec4<u32>, 1024> @offset(16)
}

$B1: {  # root
  %v:ptr<workgroup, array<u16, 8200>, read_write> = var undef
}

%foo = func(%offset:u32, %stride:u32):void {
  $B2: {
    %5:u32 = div 16u, 2u
    %6:u32 = mul %offset, 16u
    %7:u32 = div %6, 2u
    %8:u32 = add %5, %7
    %9:u32 = mul %stride, 16u
    %10:u32 = div %9, 2u
    %11:subgroup_matrix_right<f32, 8, 8> = subgroupMatrixLoad<subgroup_matrix_right<f32, 8, 8>, row_major> %v, %8, %10 @align(16)
    %12:ptr<workgroup, u16, read_write> = access %v, 0u
    %13:u16 = load %12
    %14:f16 = bitcast<f16> %13
    ret
  }
}
)";

    DecomposeAccessConfig options{.workgroup = true};
    Run(DecomposeAccess, options);
    EXPECT_EQ(expect, str());
}

TEST_F(IR_DecomposeAccessTest, Workgroup_UsedWithSubgroupMatrix_Load_SameType) {
    auto* mat_ty = ty.subgroup_matrix(core::SubgroupMatrixKind::kLeft, ty.f32(), 8, 8);
    auto* v = b.Var("v", ty.ptr(workgroup, ty.array(ty.f32(), 1024)));
    mod.root_block->Append(v);

    auto* foo = b.Function("foo", ty.void_());
    auto* offset = b.FunctionParam("offset", ty.u32());
    auto* stride = b.FunctionParam("stride", ty.u32());
    foo->SetParams({offset, stride});
    b.Append(foo->Block(), [&] {
        auto* l = b.Let("l", v);
        b.CallExplicit(mat_ty, BuiltinFn::kSubgroupMatrixLoad,
                       Vector<TemplateParameter, 2>{mat_ty, Majorness::kColMajor}, l, offset,
                       stride);
        b.Return(foo);
    });

    auto* src = R"(
$B1: {  # root
  %v:ptr<workgroup, array<f32, 1024>, read_write> = var undef
}

%foo = func(%offset:u32, %stride:u32):void {
  $B2: {
    %l:ptr<workgroup, array<f32, 1024>, read_write> = let %v
    %6:subgroup_matrix_left<f32, 8, 8> = subgroupMatrixLoad<subgroup_matrix_left<f32, 8, 8>, col_major> %l, %offset, %stride
    ret
  }
}
)";

    ASSERT_EQ(src, str());

    DecomposeAccessConfig options{.workgroup_subgroup_matrix = true};
    Run(DecomposeAccess, options);
    EXPECT_EQ(src, str());
}

TEST_F(IR_DecomposeAccessTest, Workgroup_UsedWithSubgroupMatrix_Load_DifferentType) {
    auto* S = ty.Struct(mod.symbols.New("S"), {
                                                  {mod.symbols.New("a"), ty.array(ty.f32(), 1024)},
                                              });
    auto* mat_ty = ty.subgroup_matrix(core::SubgroupMatrixKind::kLeft, ty.u8(), 8, 8);
    auto* v = b.Var("v", ty.ptr(workgroup, S));
    mod.root_block->Append(v);

    auto* foo = b.Function("foo", ty.void_());
    auto* offset = b.FunctionParam("offset", ty.u32());
    auto* stride = b.FunctionParam("stride", ty.u32());
    foo->SetParams({offset, stride});
    b.Append(foo->Block(), [&] {
        auto* l = b.Let("l", v);
        auto* a = b.Access(ty.ptr(workgroup, ty.array(ty.f32(), 1024)), l, 0_u);
        b.CallExplicit(mat_ty, BuiltinFn::kSubgroupMatrixLoad,
                       Vector<TemplateParameter, 2>{mat_ty, Majorness::kColMajor}, a, offset,
                       stride);
        b.Return(foo);
    });

    auto* src = R"(
S = struct @align(4) {
  a:array<f32, 1024> @offset(0)
}

$B1: {  # root
  %v:ptr<workgroup, S, read_write> = var undef
}

%foo = func(%offset:u32, %stride:u32):void {
  $B2: {
    %l:ptr<workgroup, S, read_write> = let %v
    %6:ptr<workgroup, array<f32, 1024>, read_write> = access %l, 0u
    %7:subgroup_matrix_left<u8, 8, 8> = subgroupMatrixLoad<subgroup_matrix_left<u8, 8, 8>, col_major> %6, %offset, %stride
    ret
  }
}
)";

    ASSERT_EQ(src, str());

    auto* expect = R"(
S = struct @align(4) {
  a:array<f32, 1024> @offset(0)
}

$B1: {  # root
  %v:ptr<workgroup, array<u32, 1024>, read_write> = var undef
}

%foo = func(%offset:u32, %stride:u32):void {
  $B2: {
    %5:subgroup_matrix_left<u8, 8, 8> = subgroupMatrixLoad<subgroup_matrix_left<u8, 8, 8>, col_major> %v, %offset, %stride
    ret
  }
}
)";

    DecomposeAccessConfig options{.workgroup_subgroup_matrix = true};
    Run(DecomposeAccess, options);
    EXPECT_EQ(expect, str());
}

TEST_F(IR_DecomposeAccessTest, Workgroup_UsedWithSubgroupMatrix_Store_SameType) {
    auto* mat_ty = ty.subgroup_matrix(core::SubgroupMatrixKind::kLeft, ty.f32(), 8, 8);
    auto* v = b.Var("v", ty.ptr(workgroup, ty.array(ty.f32(), 1024)));
    mod.root_block->Append(v);

    auto* foo = b.Function("foo", ty.void_());
    auto* offset = b.FunctionParam("offset", ty.u32());
    auto* stride = b.FunctionParam("stride", ty.u32());
    auto* m = b.FunctionParam("m", mat_ty);
    foo->SetParams({offset, stride, m});
    b.Append(foo->Block(), [&] {
        auto* l = b.Let("l", v);
        b.CallExplicit(ty.void_(), BuiltinFn::kSubgroupMatrixStore,
                       Vector<TemplateParameter, 1>{Majorness::kColMajor}, l, offset, m, stride);
        b.Return(foo);
    });

    auto* src = R"(
$B1: {  # root
  %v:ptr<workgroup, array<f32, 1024>, read_write> = var undef
}

%foo = func(%offset:u32, %stride:u32, %m:subgroup_matrix_left<f32, 8, 8>):void {
  $B2: {
    %l:ptr<workgroup, array<f32, 1024>, read_write> = let %v
    %7:void = subgroupMatrixStore<col_major> %l, %offset, %m, %stride
    ret
  }
}
)";

    ASSERT_EQ(src, str());

    DecomposeAccessConfig options{.workgroup_subgroup_matrix = true};
    Run(DecomposeAccess, options);
    EXPECT_EQ(src, str());
}

TEST_F(IR_DecomposeAccessTest, Workgroup_UsedWithSubgroupMatrix_Store_DifferentType) {
    auto* S = ty.Struct(mod.symbols.New("S"), {
                                                  {mod.symbols.New("a"), ty.array(ty.f32(), 1024)},
                                              });
    auto* mat_ty = ty.subgroup_matrix(core::SubgroupMatrixKind::kLeft, ty.u8(), 8, 8);
    auto* v = b.Var("v", ty.ptr(workgroup, S));
    mod.root_block->Append(v);

    auto* foo = b.Function("foo", ty.void_());
    auto* offset = b.FunctionParam("offset", ty.u32());
    auto* stride = b.FunctionParam("stride", ty.u32());
    auto* m = b.FunctionParam("m", mat_ty);
    foo->SetParams({offset, stride, m});
    b.Append(foo->Block(), [&] {
        auto* l = b.Let("l", v);
        auto* a = b.Access(ty.ptr(workgroup, ty.array(ty.f32(), 1024)), l, 0_u);
        b.CallExplicit(ty.void_(), BuiltinFn::kSubgroupMatrixStore,
                       Vector<TemplateParameter, 1>{Majorness::kColMajor}, a, offset, m, stride);
        b.Return(foo);
    });

    auto* src = R"(
S = struct @align(4) {
  a:array<f32, 1024> @offset(0)
}

$B1: {  # root
  %v:ptr<workgroup, S, read_write> = var undef
}

%foo = func(%offset:u32, %stride:u32, %m:subgroup_matrix_left<u8, 8, 8>):void {
  $B2: {
    %l:ptr<workgroup, S, read_write> = let %v
    %7:ptr<workgroup, array<f32, 1024>, read_write> = access %l, 0u
    %8:void = subgroupMatrixStore<col_major> %7, %offset, %m, %stride
    ret
  }
}
)";

    ASSERT_EQ(src, str());

    auto* expect = R"(
S = struct @align(4) {
  a:array<f32, 1024> @offset(0)
}

$B1: {  # root
  %v:ptr<workgroup, array<u32, 1024>, read_write> = var undef
}

%foo = func(%offset:u32, %stride:u32, %m:subgroup_matrix_left<u8, 8, 8>):void {
  $B2: {
    %6:void = subgroupMatrixStore<col_major> %v, %offset, %m, %stride
    ret
  }
}
)";

    DecomposeAccessConfig options{.workgroup_subgroup_matrix = true};
    Run(DecomposeAccess, options);
    EXPECT_EQ(expect, str());
}

// Regression test: an immediate struct whose Size() is rounded up by member alignment (here a
// vec4 forces 16-byte alignment, padding 24 bytes of content to 32) must be decomposed to an array
// sized by minimum_array_size (the reserved push constant range), not by the padded Size(). Using
// the padded Size() emitted a block larger than the reserved range and failed Vulkan push constant
// validation (VUID-VkGraphicsPipelineCreateInfo-layout-10069).
TEST_F(IR_DecomposeAccessTest, ImmediateAccessPaddedStructCappedByMinimumArraySize) {
    auto* SB = ty.Struct(mod.symbols.New("SB"), {
                                                    {mod.symbols.New("a"), ty.vec4<f32>()},
                                                    {mod.symbols.New("b"), ty.f32()},
                                                    {mod.symbols.New("c"), ty.f32()},
                                                });

    auto* var = b.Var("v", immediate, SB, core::Access::kRead);

    b.ir.root_block->Append(var);
    auto* func = b.Function("foo", ty.void_(), core::ir::Function::PipelineStage::kFragment);
    b.Append(func->Block(), [&] {
        b.Let("b", b.Load(b.Access(ty.ptr<immediate, f32, core::Access::kRead>(), var, 2_u)));
        b.Return(func);
    });

    auto* expect = R"(
SB = struct @align(16) {
  a:vec4<f32> @offset(0)
  b:f32 @offset(16)
  c:f32 @offset(20)
}

$B1: {  # root
  %v:ptr<immediate, array<u32, 6>, read> = var undef
}

%foo = @fragment func():void {
  $B2: {
    %3:ptr<immediate, u32, read> = access %v, 5u
    %4:u32 = load %3
    %5:f32 = bitcast<f32> %4
    %b:f32 = let %5
    ret
  }
}
)";

    // SB has align 16 (from the vec4), so SB->Size() is roundUp(16, 24) = 32 -> 8 u32 elements.
    // minimum_array_size is the reserved range of 24 bytes -> the array must be capped at 6.
    DecomposeAccessConfig options{.immediate = true, .minimum_array_size = 24};
    Run(DecomposeAccess, options);

    EXPECT_EQ(expect, str());
}

TEST_F(IR_DecomposeAccessTest, ImmediateNoDynamicIndices_Vector) {
    auto* v = b.Var("v", immediate, ty.vec4u());
    mod.root_block->Append(v);

    auto* foo = b.Function("foo", ty.void_());
    auto* idx = b.FunctionParam("idx", ty.u32());
    foo->SetParams({idx});
    b.Append(foo->Block(), [&] {
        auto* load = b.LoadVectorElement(v, idx);
        b.Let("value", load);
        b.Return(foo);
    });

    auto* src = R"(
$B1: {  # root
  %v:ptr<immediate, vec4<u32>, read> = var undef
}

%foo = func(%idx:u32):void {
  $B2: {
    %4:u32 = load_vector_element %v, %idx
    %value:u32 = let %4
    ret
  }
}
)";

    EXPECT_EQ(src, str());

    auto* expect = R"(
$B1: {  # root
  %v:ptr<immediate, array<u32, 4>, read> = var undef
}

%foo = func(%idx:u32):void {
  $B2: {
    %4:ptr<immediate, u32, read> = access %v, 0u
    %5:u32 = load %4 @align(16)
    %6:ptr<immediate, u32, read> = access %v, 1u
    %7:u32 = load %6
    %8:ptr<immediate, u32, read> = access %v, 2u
    %9:u32 = load %8
    %10:ptr<immediate, u32, read> = access %v, 3u
    %11:u32 = load %10
    %12:vec4<u32> = construct %5, %7, %9, %11
    %13:u32 = access %12, %idx
    %value:u32 = let %13
    ret
  }
}
)";

    DecomposeAccessConfig options{.immediate = true, .allow_dynamic_immediate_indices = false};
    Run(DecomposeAccess, options);

    EXPECT_EQ(expect, str());
}

TEST_F(IR_DecomposeAccessTest, ImmediateNoDynamicIndices_Matrix) {
    auto* v = b.Var("v", immediate, ty.mat2x2(ty.f32()));
    mod.root_block->Append(v);

    auto* foo = b.Function("foo", ty.void_());
    auto* idx = b.FunctionParam("idx", ty.u32());
    foo->SetParams({idx});
    b.Append(foo->Block(), [&] {
        auto* access = b.Access(ty.ptr(immediate, ty.vec2(ty.f32())), v, idx);
        auto* load = b.Load(access);
        b.Let("value", load);
        b.Return(foo);
    });

    auto* src = R"(
$B1: {  # root
  %v:ptr<immediate, mat2x2<f32>, read> = var undef
}

%foo = func(%idx:u32):void {
  $B2: {
    %4:ptr<immediate, vec2<f32>, read> = access %v, %idx
    %5:vec2<f32> = load %4
    %value:vec2<f32> = let %5
    ret
  }
}
)";

    EXPECT_EQ(src, str());

    auto* expect = R"(
$B1: {  # root
  %v:ptr<immediate, array<u32, 4>, read> = var undef
}

%foo = func(%idx:u32):void {
  $B2: {
    %4:mat2x2<f32> = call %5, 0u
    %6:vec2<f32> = access %4, %idx
    %value:vec2<f32> = let %6
    ret
  }
}
%5 = func(%start_byte_offset:u32):mat2x2<f32> {
  $B3: {
    %9:u32 = div %start_byte_offset, 4u
    %10:ptr<immediate, u32, read> = access %v, %9
    %11:u32 = load %10 @align(8)
    %12:u32 = add %9, 1u
    %13:ptr<immediate, u32, read> = access %v, %12
    %14:u32 = load %13
    %15:vec2<u32> = construct %11, %14
    %16:vec2<f32> = bitcast<vec2<f32>> %15
    %17:u32 = add 8u, %start_byte_offset
    %18:u32 = div %17, 4u
    %19:ptr<immediate, u32, read> = access %v, %18
    %20:u32 = load %19 @align(8)
    %21:u32 = add %18, 1u
    %22:ptr<immediate, u32, read> = access %v, %21
    %23:u32 = load %22
    %24:vec2<u32> = construct %20, %23
    %25:vec2<f32> = bitcast<vec2<f32>> %24
    %26:mat2x2<f32> = construct %16, %25
    ret %26
  }
}
)";

    DecomposeAccessConfig options{.immediate = true, .allow_dynamic_immediate_indices = false};
    Run(DecomposeAccess, options);

    EXPECT_EQ(expect, str());
}

TEST_F(IR_DecomposeAccessTest, ImmediateNoDynamicIndices_MatrixAndVector) {
    auto* v = b.Var("v", immediate, ty.mat2x2(ty.f32()));
    mod.root_block->Append(v);

    auto* foo = b.Function("foo", ty.void_());
    auto* idx1 = b.FunctionParam("idx1", ty.u32());
    auto* idx2 = b.FunctionParam("idx2", ty.u32());
    foo->SetParams({idx1, idx2});
    b.Append(foo->Block(), [&] {
        auto* access = b.Access(ty.ptr(immediate, ty.vec2(ty.f32())), v, idx1);
        auto* load = b.LoadVectorElement(access, idx2);
        b.Let("value", load);
        b.Return(foo);
    });

    auto* src = R"(
$B1: {  # root
  %v:ptr<immediate, mat2x2<f32>, read> = var undef
}

%foo = func(%idx1:u32, %idx2:u32):void {
  $B2: {
    %5:ptr<immediate, vec2<f32>, read> = access %v, %idx1
    %6:f32 = load_vector_element %5, %idx2
    %value:f32 = let %6
    ret
  }
}
)";

    EXPECT_EQ(src, str());

    auto* expect = R"(
$B1: {  # root
  %v:ptr<immediate, array<u32, 4>, read> = var undef
}

%foo = func(%idx1:u32, %idx2:u32):void {
  $B2: {
    %5:mat2x2<f32> = call %6, 0u
    %7:vec2<f32> = access %5, %idx1
    %8:f32 = access %7, %idx2
    %value:f32 = let %8
    ret
  }
}
%6 = func(%start_byte_offset:u32):mat2x2<f32> {
  $B3: {
    %11:u32 = div %start_byte_offset, 4u
    %12:ptr<immediate, u32, read> = access %v, %11
    %13:u32 = load %12 @align(8)
    %14:u32 = add %11, 1u
    %15:ptr<immediate, u32, read> = access %v, %14
    %16:u32 = load %15
    %17:vec2<u32> = construct %13, %16
    %18:vec2<f32> = bitcast<vec2<f32>> %17
    %19:u32 = add 8u, %start_byte_offset
    %20:u32 = div %19, 4u
    %21:ptr<immediate, u32, read> = access %v, %20
    %22:u32 = load %21 @align(8)
    %23:u32 = add %20, 1u
    %24:ptr<immediate, u32, read> = access %v, %23
    %25:u32 = load %24
    %26:vec2<u32> = construct %22, %25
    %27:vec2<f32> = bitcast<vec2<f32>> %26
    %28:mat2x2<f32> = construct %18, %27
    ret %28
  }
}
)";

    DecomposeAccessConfig options{.immediate = true, .allow_dynamic_immediate_indices = false};
    Run(DecomposeAccess, options);

    EXPECT_EQ(expect, str());
}

TEST_F(IR_DecomposeAccessTest, ImmediateNoDynamicIndices_StructMatrix) {
    auto* S = ty.Struct(mod.symbols.New("S"), {
                                                  {mod.symbols.New("a"), ty.vec4u()},
                                                  {mod.symbols.New("b"), ty.mat2x2(ty.f32())},
                                              });
    auto* v = b.Var("v", immediate, S);
    mod.root_block->Append(v);

    auto* foo = b.Function("foo", ty.void_());
    auto* idx1 = b.FunctionParam("idx1", ty.u32());
    auto* idx2 = b.FunctionParam("idx2", ty.u32());
    foo->SetParams({idx1, idx2});
    b.Append(foo->Block(), [&] {
        auto* l1 = b.Let("l1", v);
        auto* access = b.Access(ty.ptr(immediate, ty.vec2(ty.f32())), l1, 1_u, idx1);
        auto* l2 = b.Let("l2", access);
        auto* load = b.Load(l2);
        b.Let("value", load);
        b.Return(foo);
    });

    auto* src = R"(
S = struct @align(16) {
  a:vec4<u32> @offset(0)
  b:mat2x2<f32> @offset(16)
}

$B1: {  # root
  %v:ptr<immediate, S, read> = var undef
}

%foo = func(%idx1:u32, %idx2:u32):void {
  $B2: {
    %l1:ptr<immediate, S, read> = let %v
    %6:ptr<immediate, vec2<f32>, read> = access %l1, 1u, %idx1
    %l2:ptr<immediate, vec2<f32>, read> = let %6
    %8:vec2<f32> = load %l2
    %value:vec2<f32> = let %8
    ret
  }
}
)";

    EXPECT_EQ(src, str());

    auto* expect = R"(
S = struct @align(16) {
  a:vec4<u32> @offset(0)
  b:mat2x2<f32> @offset(16)
}

$B1: {  # root
  %v:ptr<immediate, array<u32, 8>, read> = var undef
}

%foo = func(%idx1:u32, %idx2:u32):void {
  $B2: {
    %5:mat2x2<f32> = call %6, 16u
    %7:vec2<f32> = access %5, %idx1
    %l2:vec2<f32> = let %7
    %value:vec2<f32> = let %l2
    ret
  }
}
%6 = func(%start_byte_offset:u32):mat2x2<f32> {
  $B3: {
    %11:u32 = div %start_byte_offset, 4u
    %12:ptr<immediate, u32, read> = access %v, %11
    %13:u32 = load %12 @align(8)
    %14:u32 = add %11, 1u
    %15:ptr<immediate, u32, read> = access %v, %14
    %16:u32 = load %15
    %17:vec2<u32> = construct %13, %16
    %18:vec2<f32> = bitcast<vec2<f32>> %17
    %19:u32 = add 8u, %start_byte_offset
    %20:u32 = div %19, 4u
    %21:ptr<immediate, u32, read> = access %v, %20
    %22:u32 = load %21 @align(8)
    %23:u32 = add %20, 1u
    %24:ptr<immediate, u32, read> = access %v, %23
    %25:u32 = load %24
    %26:vec2<u32> = construct %22, %25
    %27:vec2<f32> = bitcast<vec2<f32>> %26
    %28:mat2x2<f32> = construct %18, %27
    ret %28
  }
}
)";

    DecomposeAccessConfig options{.immediate = true, .allow_dynamic_immediate_indices = false};
    Run(DecomposeAccess, options);

    EXPECT_EQ(expect, str());
}

TEST_F(IR_DecomposeAccessTest, ImmediateNoDynamicIndices_StructMatrixVector) {
    auto* S = ty.Struct(mod.symbols.New("S"), {
                                                  {mod.symbols.New("a"), ty.vec4u()},
                                                  {mod.symbols.New("b"), ty.mat2x2(ty.f32())},
                                              });
    auto* v = b.Var("v", immediate, S);
    mod.root_block->Append(v);

    auto* foo = b.Function("foo", ty.void_());
    auto* idx1 = b.FunctionParam("idx1", ty.u32());
    auto* idx2 = b.FunctionParam("idx2", ty.u32());
    foo->SetParams({idx1, idx2});
    b.Append(foo->Block(), [&] {
        auto* l1 = b.Let("l1", v);
        auto* access = b.Access(ty.ptr(immediate, ty.vec2(ty.f32())), l1, 1_u, idx1);
        auto* l2 = b.Let("l2", access);
        auto* load = b.LoadVectorElement(l2, idx2);
        b.Let("value", load);
        b.Return(foo);
    });

    auto* src = R"(
S = struct @align(16) {
  a:vec4<u32> @offset(0)
  b:mat2x2<f32> @offset(16)
}

$B1: {  # root
  %v:ptr<immediate, S, read> = var undef
}

%foo = func(%idx1:u32, %idx2:u32):void {
  $B2: {
    %l1:ptr<immediate, S, read> = let %v
    %6:ptr<immediate, vec2<f32>, read> = access %l1, 1u, %idx1
    %l2:ptr<immediate, vec2<f32>, read> = let %6
    %8:f32 = load_vector_element %l2, %idx2
    %value:f32 = let %8
    ret
  }
}
)";

    EXPECT_EQ(src, str());

    auto* expect = R"(
S = struct @align(16) {
  a:vec4<u32> @offset(0)
  b:mat2x2<f32> @offset(16)
}

$B1: {  # root
  %v:ptr<immediate, array<u32, 8>, read> = var undef
}

%foo = func(%idx1:u32, %idx2:u32):void {
  $B2: {
    %5:mat2x2<f32> = call %6, 16u
    %7:vec2<f32> = access %5, %idx1
    %l2:vec2<f32> = let %7
    %9:f32 = access %l2, %idx2
    %value:f32 = let %9
    ret
  }
}
%6 = func(%start_byte_offset:u32):mat2x2<f32> {
  $B3: {
    %12:u32 = div %start_byte_offset, 4u
    %13:ptr<immediate, u32, read> = access %v, %12
    %14:u32 = load %13 @align(8)
    %15:u32 = add %12, 1u
    %16:ptr<immediate, u32, read> = access %v, %15
    %17:u32 = load %16
    %18:vec2<u32> = construct %14, %17
    %19:vec2<f32> = bitcast<vec2<f32>> %18
    %20:u32 = add 8u, %start_byte_offset
    %21:u32 = div %20, 4u
    %22:ptr<immediate, u32, read> = access %v, %21
    %23:u32 = load %22 @align(8)
    %24:u32 = add %21, 1u
    %25:ptr<immediate, u32, read> = access %v, %24
    %26:u32 = load %25
    %27:vec2<u32> = construct %23, %26
    %28:vec2<f32> = bitcast<vec2<f32>> %27
    %29:mat2x2<f32> = construct %19, %28
    ret %29
  }
}
)";

    DecomposeAccessConfig options{.immediate = true, .allow_dynamic_immediate_indices = false};
    Run(DecomposeAccess, options);

    EXPECT_EQ(expect, str());
}

TEST_F(IR_DecomposeAccessTest, ImmediateNoDynamicIndices_StructMatrix_MultiUse) {
    auto* S = ty.Struct(mod.symbols.New("S"), {
                                                  {mod.symbols.New("a"), ty.vec4u()},
                                                  {mod.symbols.New("b"), ty.mat2x2(ty.f32())},
                                              });
    auto* v = b.Var("v", immediate, S);
    mod.root_block->Append(v);

    auto* foo = b.Function("foo", ty.void_());
    auto* idx1 = b.FunctionParam("idx1", ty.u32());
    auto* idx2 = b.FunctionParam("idx2", ty.u32());
    foo->SetParams({idx1, idx2});
    b.Append(foo->Block(), [&] {
        auto* l1 = b.Let("l1", v);
        auto* access = b.Access(ty.ptr(immediate, ty.vec2(ty.f32())), l1, 1_u, idx1);
        auto* l2 = b.Let("l2", access);
        auto* load = b.LoadVectorElement(l2, idx2);
        b.Let("value1", load);
        load = b.LoadVectorElement(l2, idx2);
        b.Let("value2", load);
        b.Return(foo);
    });

    auto* src = R"(
S = struct @align(16) {
  a:vec4<u32> @offset(0)
  b:mat2x2<f32> @offset(16)
}

$B1: {  # root
  %v:ptr<immediate, S, read> = var undef
}

%foo = func(%idx1:u32, %idx2:u32):void {
  $B2: {
    %l1:ptr<immediate, S, read> = let %v
    %6:ptr<immediate, vec2<f32>, read> = access %l1, 1u, %idx1
    %l2:ptr<immediate, vec2<f32>, read> = let %6
    %8:f32 = load_vector_element %l2, %idx2
    %value1:f32 = let %8
    %10:f32 = load_vector_element %l2, %idx2
    %value2:f32 = let %10
    ret
  }
}
)";

    EXPECT_EQ(src, str());

    auto* expect = R"(
S = struct @align(16) {
  a:vec4<u32> @offset(0)
  b:mat2x2<f32> @offset(16)
}

$B1: {  # root
  %v:ptr<immediate, array<u32, 8>, read> = var undef
}

%foo = func(%idx1:u32, %idx2:u32):void {
  $B2: {
    %5:mat2x2<f32> = call %6, 16u
    %7:vec2<f32> = access %5, %idx1
    %l2:vec2<f32> = let %7
    %9:f32 = access %l2, %idx2
    %value1:f32 = let %9
    %11:f32 = access %l2, %idx2
    %value2:f32 = let %11
    ret
  }
}
%6 = func(%start_byte_offset:u32):mat2x2<f32> {
  $B3: {
    %14:u32 = div %start_byte_offset, 4u
    %15:ptr<immediate, u32, read> = access %v, %14
    %16:u32 = load %15 @align(8)
    %17:u32 = add %14, 1u
    %18:ptr<immediate, u32, read> = access %v, %17
    %19:u32 = load %18
    %20:vec2<u32> = construct %16, %19
    %21:vec2<f32> = bitcast<vec2<f32>> %20
    %22:u32 = add 8u, %start_byte_offset
    %23:u32 = div %22, 4u
    %24:ptr<immediate, u32, read> = access %v, %23
    %25:u32 = load %24 @align(8)
    %26:u32 = add %23, 1u
    %27:ptr<immediate, u32, read> = access %v, %26
    %28:u32 = load %27
    %29:vec2<u32> = construct %25, %28
    %30:vec2<f32> = bitcast<vec2<f32>> %29
    %31:mat2x2<f32> = construct %21, %30
    ret %31
  }
}
)";

    DecomposeAccessConfig options{.immediate = true, .allow_dynamic_immediate_indices = false};
    Run(DecomposeAccess, options);

    EXPECT_EQ(expect, str());
}

TEST_F(IR_DecomposeAccessTest, AddAlignment_Struct_Load) {
    auto* S = ty.Struct(mod.symbols.New("S"), {
                                                  {mod.symbols.New("a"), ty.vec4u()},
                                                  {mod.symbols.New("b"), ty.f16()},
                                                  {mod.symbols.New("c"), ty.vec4f()},
                                              });

    auto* v = b.Var("v", ty.ptr(storage, S));
    v->SetBindingPoint(0, 0);
    mod.root_block->Append(v);

    auto* foo = b.Function("foo", ty.void_());
    b.Append(foo->Block(), [&] {
        b.Load(v);
        b.Return(foo);
    });

    auto* src = R"(
S = struct @align(16) {
  a:vec4<u32> @offset(0)
  b:f16 @offset(16)
  c:vec4<f32> @offset(32)
}

$B1: {  # root
  %v:ptr<storage, S, read_write> = var undef @binding_point(0, 0)
}

%foo = func():void {
  $B2: {
    %3:S = load %v
    ret
  }
}
)";

    ASSERT_EQ(src, str());

    auto* expect = R"(
S = struct @align(16) {
  a:vec4<u32> @offset(0)
  b:f16 @offset(16)
  c:vec4<f32> @offset(32)
}

$B1: {  # root
  %v:ptr<storage, array<u16, 24>, read_write> = var undef @binding_point(0, 0)
}

%foo = func():void {
  $B2: {
    %3:S = call %4, 0u
    ret
  }
}
%4 = func(%start_byte_offset:u32):S {
  $B3: {
    %6:u32 = div %start_byte_offset, 2u
    %7:ptr<storage, u16, read_write> = access %v, %6
    %8:u16 = load %7 @align(16)
    %9:u32 = add %6, 1u
    %10:ptr<storage, u16, read_write> = access %v, %9
    %11:u16 = load %10
    %12:u32 = add %9, 1u
    %13:ptr<storage, u16, read_write> = access %v, %12
    %14:u16 = load %13
    %15:u32 = add %12, 1u
    %16:ptr<storage, u16, read_write> = access %v, %15
    %17:u16 = load %16
    %18:u32 = add %15, 1u
    %19:ptr<storage, u16, read_write> = access %v, %18
    %20:u16 = load %19
    %21:u32 = add %18, 1u
    %22:ptr<storage, u16, read_write> = access %v, %21
    %23:u16 = load %22
    %24:u32 = add %21, 1u
    %25:ptr<storage, u16, read_write> = access %v, %24
    %26:u16 = load %25
    %27:u32 = add %24, 1u
    %28:ptr<storage, u16, read_write> = access %v, %27
    %29:u16 = load %28
    %30:vec2<u16> = construct %8, %11
    %31:u32 = bitcast<u32> %30
    %32:vec2<u16> = construct %14, %17
    %33:u32 = bitcast<u32> %32
    %34:vec2<u16> = construct %20, %23
    %35:u32 = bitcast<u32> %34
    %36:vec2<u16> = construct %26, %29
    %37:u32 = bitcast<u32> %36
    %38:vec4<u32> = construct %31, %33, %35, %37
    %39:u32 = add 16u, %start_byte_offset
    %40:u32 = div %39, 2u
    %41:ptr<storage, u16, read_write> = access %v, %40
    %42:u16 = load %41
    %43:f16 = bitcast<f16> %42
    %44:u32 = add 32u, %start_byte_offset
    %45:u32 = div %44, 2u
    %46:ptr<storage, u16, read_write> = access %v, %45
    %47:u16 = load %46 @align(16)
    %48:u32 = add %45, 1u
    %49:ptr<storage, u16, read_write> = access %v, %48
    %50:u16 = load %49
    %51:u32 = add %48, 1u
    %52:ptr<storage, u16, read_write> = access %v, %51
    %53:u16 = load %52
    %54:u32 = add %51, 1u
    %55:ptr<storage, u16, read_write> = access %v, %54
    %56:u16 = load %55
    %57:u32 = add %54, 1u
    %58:ptr<storage, u16, read_write> = access %v, %57
    %59:u16 = load %58
    %60:u32 = add %57, 1u
    %61:ptr<storage, u16, read_write> = access %v, %60
    %62:u16 = load %61
    %63:u32 = add %60, 1u
    %64:ptr<storage, u16, read_write> = access %v, %63
    %65:u16 = load %64
    %66:u32 = add %63, 1u
    %67:ptr<storage, u16, read_write> = access %v, %66
    %68:u16 = load %67
    %69:vec2<u16> = construct %47, %50
    %70:u32 = bitcast<u32> %69
    %71:vec2<u16> = construct %53, %56
    %72:u32 = bitcast<u32> %71
    %73:vec2<u16> = construct %59, %62
    %74:u32 = bitcast<u32> %73
    %75:vec2<u16> = construct %65, %68
    %76:u32 = bitcast<u32> %75
    %77:vec4<u32> = construct %70, %72, %74, %76
    %78:vec4<f32> = bitcast<vec4<f32>> %77
    %79:S = construct %38, %43, %78
    ret %79
  }
}
)";

    DecomposeAccessConfig options{.storage = true};
    Run(DecomposeAccess, options);

    EXPECT_EQ(expect, str());
}

TEST_F(IR_DecomposeAccessTest, AddAlignment_Struct_Store) {
    auto* S = ty.Struct(mod.symbols.New("S"), {
                                                  {mod.symbols.New("a"), ty.vec4u()},
                                                  {mod.symbols.New("b"), ty.f16()},
                                                  {mod.symbols.New("c"), ty.vec4f()},
                                              });

    auto* v = b.Var("v", ty.ptr(storage, S));
    v->SetBindingPoint(0, 0);
    mod.root_block->Append(v);

    auto* foo = b.Function("foo", ty.void_());
    b.Append(foo->Block(), [&] {
        b.Store(v, b.Zero(S));
        b.Return(foo);
    });

    auto* src = R"(
S = struct @align(16) {
  a:vec4<u32> @offset(0)
  b:f16 @offset(16)
  c:vec4<f32> @offset(32)
}

$B1: {  # root
  %v:ptr<storage, S, read_write> = var undef @binding_point(0, 0)
}

%foo = func():void {
  $B2: {
    store %v, S(vec4<u32>(0u), 0.0h, vec4<f32>(0.0f))
    ret
  }
}
)";

    ASSERT_EQ(src, str());

    auto* expect = R"(
S = struct @align(16) {
  a:vec4<u32> @offset(0)
  b:f16 @offset(16)
  c:vec4<f32> @offset(32)
}

$B1: {  # root
  %v:ptr<storage, array<u16, 24>, read_write> = var undef @binding_point(0, 0)
}

%foo = func():void {
  $B2: {
    %3:void = call %4, 0u, S(vec4<u32>(0u), 0.0h, vec4<f32>(0.0f))
    ret
  }
}
%4 = func(%start_byte_offset:u32, %object:S):void {
  $B3: {
    %7:vec4<u32> = access %object, 0u
    %8:u32 = div %start_byte_offset, 2u
    %9:u32 = access %7, 0u
    %10:vec2<u16> = bitcast<vec2<u16>> %9
    %11:u16 = access %10, 0u
    %12:ptr<storage, u16, read_write> = access %v, %8
    store %12, %11 @align(16)
    %13:u32 = add %8, 1u
    %14:u32 = access %7, 0u
    %15:vec2<u16> = bitcast<vec2<u16>> %14
    %16:u16 = access %15, 1u
    %17:ptr<storage, u16, read_write> = access %v, %13
    store %17, %16
    %18:u32 = add %13, 1u
    %19:u32 = access %7, 1u
    %20:vec2<u16> = bitcast<vec2<u16>> %19
    %21:u16 = access %20, 0u
    %22:ptr<storage, u16, read_write> = access %v, %18
    store %22, %21
    %23:u32 = add %18, 1u
    %24:u32 = access %7, 1u
    %25:vec2<u16> = bitcast<vec2<u16>> %24
    %26:u16 = access %25, 1u
    %27:ptr<storage, u16, read_write> = access %v, %23
    store %27, %26
    %28:u32 = add %23, 1u
    %29:u32 = access %7, 2u
    %30:vec2<u16> = bitcast<vec2<u16>> %29
    %31:u16 = access %30, 0u
    %32:ptr<storage, u16, read_write> = access %v, %28
    store %32, %31
    %33:u32 = add %28, 1u
    %34:u32 = access %7, 2u
    %35:vec2<u16> = bitcast<vec2<u16>> %34
    %36:u16 = access %35, 1u
    %37:ptr<storage, u16, read_write> = access %v, %33
    store %37, %36
    %38:u32 = add %33, 1u
    %39:u32 = access %7, 3u
    %40:vec2<u16> = bitcast<vec2<u16>> %39
    %41:u16 = access %40, 0u
    %42:ptr<storage, u16, read_write> = access %v, %38
    store %42, %41
    %43:u32 = add %38, 1u
    %44:u32 = access %7, 3u
    %45:vec2<u16> = bitcast<vec2<u16>> %44
    %46:u16 = access %45, 1u
    %47:ptr<storage, u16, read_write> = access %v, %43
    store %47, %46
    %48:u32 = add 16u, %start_byte_offset
    %49:f16 = access %object, 1u
    %50:u32 = div %48, 2u
    %51:u16 = bitcast<u16> %49
    %52:ptr<storage, u16, read_write> = access %v, %50
    store %52, %51
    %53:u32 = add 32u, %start_byte_offset
    %54:vec4<f32> = access %object, 2u
    %55:u32 = div %53, 2u
    %56:f32 = access %54, 0u
    %57:vec2<u16> = bitcast<vec2<u16>> %56
    %58:u16 = access %57, 0u
    %59:ptr<storage, u16, read_write> = access %v, %55
    store %59, %58 @align(16)
    %60:u32 = add %55, 1u
    %61:f32 = access %54, 0u
    %62:vec2<u16> = bitcast<vec2<u16>> %61
    %63:u16 = access %62, 1u
    %64:ptr<storage, u16, read_write> = access %v, %60
    store %64, %63
    %65:u32 = add %60, 1u
    %66:f32 = access %54, 1u
    %67:vec2<u16> = bitcast<vec2<u16>> %66
    %68:u16 = access %67, 0u
    %69:ptr<storage, u16, read_write> = access %v, %65
    store %69, %68
    %70:u32 = add %65, 1u
    %71:f32 = access %54, 1u
    %72:vec2<u16> = bitcast<vec2<u16>> %71
    %73:u16 = access %72, 1u
    %74:ptr<storage, u16, read_write> = access %v, %70
    store %74, %73
    %75:u32 = add %70, 1u
    %76:f32 = access %54, 2u
    %77:vec2<u16> = bitcast<vec2<u16>> %76
    %78:u16 = access %77, 0u
    %79:ptr<storage, u16, read_write> = access %v, %75
    store %79, %78
    %80:u32 = add %75, 1u
    %81:f32 = access %54, 2u
    %82:vec2<u16> = bitcast<vec2<u16>> %81
    %83:u16 = access %82, 1u
    %84:ptr<storage, u16, read_write> = access %v, %80
    store %84, %83
    %85:u32 = add %80, 1u
    %86:f32 = access %54, 3u
    %87:vec2<u16> = bitcast<vec2<u16>> %86
    %88:u16 = access %87, 0u
    %89:ptr<storage, u16, read_write> = access %v, %85
    store %89, %88
    %90:u32 = add %85, 1u
    %91:f32 = access %54, 3u
    %92:vec2<u16> = bitcast<vec2<u16>> %91
    %93:u16 = access %92, 1u
    %94:ptr<storage, u16, read_write> = access %v, %90
    store %94, %93
    ret
  }
}
)";

    DecomposeAccessConfig options{.storage = true};
    Run(DecomposeAccess, options);

    EXPECT_EQ(expect, str());
}

TEST_F(IR_DecomposeAccessTest, AddAlignment_Array_Load) {
    auto* S = ty.Struct(mod.symbols.New("S"), {
                                                  {mod.symbols.New("a"), ty.array(ty.vec4u(), 2)},
                                                  {mod.symbols.New("b"), ty.f16()},
                                              });
    auto* v = b.Var("v", ty.ptr(storage, S));
    v->SetBindingPoint(0, 0);
    mod.root_block->Append(v);

    auto* foo = b.Function("foo", ty.void_());
    b.Append(foo->Block(), [&] {
        b.Load(b.Access(ty.ptr(storage, ty.array(ty.vec4u(), 2)), v, 0_u));
        b.Load(b.Access(ty.ptr(storage, ty.f16()), v, 1_u));
        b.Return(foo);
    });

    auto* src = R"(
S = struct @align(16) {
  a:array<vec4<u32>, 2> @offset(0)
  b:f16 @offset(32)
}

$B1: {  # root
  %v:ptr<storage, S, read_write> = var undef @binding_point(0, 0)
}

%foo = func():void {
  $B2: {
    %3:ptr<storage, array<vec4<u32>, 2>, read_write> = access %v, 0u
    %4:array<vec4<u32>, 2> = load %3
    %5:ptr<storage, f16, read_write> = access %v, 1u
    %6:f16 = load %5
    ret
  }
}
)";

    ASSERT_EQ(src, str());

    auto* expect = R"(
S = struct @align(16) {
  a:array<vec4<u32>, 2> @offset(0)
  b:f16 @offset(32)
}

$B1: {  # root
  %v:ptr<storage, array<u16, 24>, read_write> = var undef @binding_point(0, 0)
}

%foo = func():void {
  $B2: {
    %3:array<vec4<u32>, 2> = call %4, 0u
    %5:ptr<storage, u16, read_write> = access %v, 16u
    %6:u16 = load %5
    %7:f16 = bitcast<f16> %6
    ret
  }
}
%4 = func(%start_byte_offset:u32):array<vec4<u32>, 2> {
  $B3: {
    %a:ptr<function, array<vec4<u32>, 2>, read_write> = var array<vec4<u32>, 2>(vec4<u32>(0u))
    loop [i: $B4, b: $B5, c: $B6] {  # loop_1
      $B4: {  # initializer
        next_iteration 0u  # -> $B5
      }
      $B5 (%idx:u32): {  # body
        %11:bool = gte %idx, 2u
        if %11 [t: $B7] {  # if_1
          $B7: {  # true
            exit_loop  # loop_1
          }
        }
        %12:u32 = mul %idx, 16u
        %13:u32 = add %start_byte_offset, %12
        %14:ptr<function, vec4<u32>, read_write> = access %a, %idx
        %15:u32 = div %13, 2u
        %16:ptr<storage, u16, read_write> = access %v, %15
        %17:u16 = load %16 @align(16)
        %18:u32 = add %15, 1u
        %19:ptr<storage, u16, read_write> = access %v, %18
        %20:u16 = load %19
        %21:u32 = add %18, 1u
        %22:ptr<storage, u16, read_write> = access %v, %21
        %23:u16 = load %22
        %24:u32 = add %21, 1u
        %25:ptr<storage, u16, read_write> = access %v, %24
        %26:u16 = load %25
        %27:u32 = add %24, 1u
        %28:ptr<storage, u16, read_write> = access %v, %27
        %29:u16 = load %28
        %30:u32 = add %27, 1u
        %31:ptr<storage, u16, read_write> = access %v, %30
        %32:u16 = load %31
        %33:u32 = add %30, 1u
        %34:ptr<storage, u16, read_write> = access %v, %33
        %35:u16 = load %34
        %36:u32 = add %33, 1u
        %37:ptr<storage, u16, read_write> = access %v, %36
        %38:u16 = load %37
        %39:vec2<u16> = construct %17, %20
        %40:u32 = bitcast<u32> %39
        %41:vec2<u16> = construct %23, %26
        %42:u32 = bitcast<u32> %41
        %43:vec2<u16> = construct %29, %32
        %44:u32 = bitcast<u32> %43
        %45:vec2<u16> = construct %35, %38
        %46:u32 = bitcast<u32> %45
        %47:vec4<u32> = construct %40, %42, %44, %46
        store %14, %47
        continue  # -> $B6
      }
      $B6: {  # continuing
        %48:u32 = add %idx, 1u
        next_iteration %48  # -> $B5
      }
    }
    %49:array<vec4<u32>, 2> = load %a
    ret %49
  }
}
)";

    DecomposeAccessConfig options{.storage = true};
    Run(DecomposeAccess, options);

    EXPECT_EQ(expect, str());
}

TEST_F(IR_DecomposeAccessTest, AddAlignment_Array_Store) {
    auto* S = ty.Struct(mod.symbols.New("S"), {
                                                  {mod.symbols.New("a"), ty.array(ty.vec4u(), 2)},
                                                  {mod.symbols.New("b"), ty.f16()},
                                              });
    auto* v = b.Var("v", ty.ptr(storage, S));
    v->SetBindingPoint(0, 0);
    mod.root_block->Append(v);

    auto* foo = b.Function("foo", ty.void_());
    b.Append(foo->Block(), [&] {
        b.Store(b.Access(ty.ptr(storage, ty.array(ty.vec4u(), 2)), v, 0_u),
                b.Zero(ty.array(ty.vec4u(), 2)));
        b.Load(b.Access(ty.ptr(storage, ty.f16()), v, 1_u));
        b.Return(foo);
    });

    auto* src = R"(
S = struct @align(16) {
  a:array<vec4<u32>, 2> @offset(0)
  b:f16 @offset(32)
}

$B1: {  # root
  %v:ptr<storage, S, read_write> = var undef @binding_point(0, 0)
}

%foo = func():void {
  $B2: {
    %3:ptr<storage, array<vec4<u32>, 2>, read_write> = access %v, 0u
    store %3, array<vec4<u32>, 2>(vec4<u32>(0u))
    %4:ptr<storage, f16, read_write> = access %v, 1u
    %5:f16 = load %4
    ret
  }
}
)";

    ASSERT_EQ(src, str());

    auto* expect = R"(
S = struct @align(16) {
  a:array<vec4<u32>, 2> @offset(0)
  b:f16 @offset(32)
}

$B1: {  # root
  %v:ptr<storage, array<u16, 24>, read_write> = var undef @binding_point(0, 0)
}

%foo = func():void {
  $B2: {
    %3:void = call %4, 0u, array<vec4<u32>, 2>(vec4<u32>(0u))
    %5:ptr<storage, u16, read_write> = access %v, 16u
    %6:u16 = load %5
    %7:f16 = bitcast<f16> %6
    ret
  }
}
%4 = func(%start_byte_offset:u32, %object:array<vec4<u32>, 2>):void {
  $B3: {
    loop [i: $B4, b: $B5, c: $B6] {  # loop_1
      $B4: {  # initializer
        next_iteration 0u  # -> $B5
      }
      $B5 (%idx:u32): {  # body
        %11:bool = gte %idx, 2u
        if %11 [t: $B7] {  # if_1
          $B7: {  # true
            exit_loop  # loop_1
          }
        }
        %12:u32 = mul %idx, 16u
        %13:u32 = add %start_byte_offset, %12
        %14:vec4<u32> = access %object, %idx
        %15:u32 = div %13, 2u
        %16:u32 = access %14, 0u
        %17:vec2<u16> = bitcast<vec2<u16>> %16
        %18:u16 = access %17, 0u
        %19:ptr<storage, u16, read_write> = access %v, %15
        store %19, %18 @align(16)
        %20:u32 = add %15, 1u
        %21:u32 = access %14, 0u
        %22:vec2<u16> = bitcast<vec2<u16>> %21
        %23:u16 = access %22, 1u
        %24:ptr<storage, u16, read_write> = access %v, %20
        store %24, %23
        %25:u32 = add %20, 1u
        %26:u32 = access %14, 1u
        %27:vec2<u16> = bitcast<vec2<u16>> %26
        %28:u16 = access %27, 0u
        %29:ptr<storage, u16, read_write> = access %v, %25
        store %29, %28
        %30:u32 = add %25, 1u
        %31:u32 = access %14, 1u
        %32:vec2<u16> = bitcast<vec2<u16>> %31
        %33:u16 = access %32, 1u
        %34:ptr<storage, u16, read_write> = access %v, %30
        store %34, %33
        %35:u32 = add %30, 1u
        %36:u32 = access %14, 2u
        %37:vec2<u16> = bitcast<vec2<u16>> %36
        %38:u16 = access %37, 0u
        %39:ptr<storage, u16, read_write> = access %v, %35
        store %39, %38
        %40:u32 = add %35, 1u
        %41:u32 = access %14, 2u
        %42:vec2<u16> = bitcast<vec2<u16>> %41
        %43:u16 = access %42, 1u
        %44:ptr<storage, u16, read_write> = access %v, %40
        store %44, %43
        %45:u32 = add %40, 1u
        %46:u32 = access %14, 3u
        %47:vec2<u16> = bitcast<vec2<u16>> %46
        %48:u16 = access %47, 0u
        %49:ptr<storage, u16, read_write> = access %v, %45
        store %49, %48
        %50:u32 = add %45, 1u
        %51:u32 = access %14, 3u
        %52:vec2<u16> = bitcast<vec2<u16>> %51
        %53:u16 = access %52, 1u
        %54:ptr<storage, u16, read_write> = access %v, %50
        store %54, %53
        continue  # -> $B6
      }
      $B6: {  # continuing
        %55:u32 = add %idx, 1u
        next_iteration %55  # -> $B5
      }
    }
    ret
  }
}
)";

    DecomposeAccessConfig options{.storage = true};
    Run(DecomposeAccess, options);

    EXPECT_EQ(expect, str());
}

TEST_F(IR_DecomposeAccessTest, AddAlignment_U32_LoadVectorElement) {
    auto* S = ty.Struct(mod.symbols.New("S"), {
                                                  {mod.symbols.New("a"), ty.vec4u()},
                                                  {mod.symbols.New("b"), ty.f16()},
                                              });

    auto* v = b.Var("v", ty.ptr(storage, S));
    v->SetBindingPoint(0, 0);
    mod.root_block->Append(v);

    auto* foo = b.Function("foo", ty.void_());
    b.Append(foo->Block(), [&] {
        b.LoadVectorElement(b.Access(ty.ptr(storage, ty.vec4u()), v, 0_u), 1_u);
        b.Load(b.Access(ty.ptr(storage, ty.f16()), v, 1_u));
        b.Return(foo);
    });

    auto* src = R"(
S = struct @align(16) {
  a:vec4<u32> @offset(0)
  b:f16 @offset(16)
}

$B1: {  # root
  %v:ptr<storage, S, read_write> = var undef @binding_point(0, 0)
}

%foo = func():void {
  $B2: {
    %3:ptr<storage, vec4<u32>, read_write> = access %v, 0u
    %4:u32 = load_vector_element %3, 1u
    %5:ptr<storage, f16, read_write> = access %v, 1u
    %6:f16 = load %5
    ret
  }
}
)";

    ASSERT_EQ(src, str());

    auto* expect = R"(
S = struct @align(16) {
  a:vec4<u32> @offset(0)
  b:f16 @offset(16)
}

$B1: {  # root
  %v:ptr<storage, array<u16, 16>, read_write> = var undef @binding_point(0, 0)
}

%foo = func():void {
  $B2: {
    %3:ptr<storage, u16, read_write> = access %v, 2u
    %4:u16 = load %3 @align(4)
    %5:ptr<storage, u16, read_write> = access %v, 3u
    %6:u16 = load %5
    %7:vec2<u16> = construct %4, %6
    %8:u32 = bitcast<u32> %7
    %9:ptr<storage, u16, read_write> = access %v, 8u
    %10:u16 = load %9
    %11:f16 = bitcast<f16> %10
    ret
  }
}
)";

    DecomposeAccessConfig options{.storage = true};
    Run(DecomposeAccess, options);

    EXPECT_EQ(expect, str());
}

TEST_F(IR_DecomposeAccessTest, AddAlignment_U32_StoreVectorElement) {
    auto* S = ty.Struct(mod.symbols.New("S"), {
                                                  {mod.symbols.New("a"), ty.vec4u()},
                                                  {mod.symbols.New("b"), ty.f16()},
                                              });

    auto* v = b.Var("v", ty.ptr(workgroup, S));
    mod.root_block->Append(v);

    auto* foo = b.Function("foo", ty.void_());
    b.Append(foo->Block(), [&] {
        b.StoreVectorElement(b.Access(ty.ptr(workgroup, ty.vec4u()), v, 0_u), 1_u, u32(0));
        b.Load(b.Access(ty.ptr(workgroup, ty.f16()), v, 1_u));
        b.Return(foo);
    });

    auto* src = R"(
S = struct @align(16) {
  a:vec4<u32> @offset(0)
  b:f16 @offset(16)
}

$B1: {  # root
  %v:ptr<workgroup, S, read_write> = var undef
}

%foo = func():void {
  $B2: {
    %3:ptr<workgroup, vec4<u32>, read_write> = access %v, 0u
    store_vector_element %3, 1u, 0u
    %4:ptr<workgroup, f16, read_write> = access %v, 1u
    %5:f16 = load %4
    ret
  }
}
)";

    ASSERT_EQ(src, str());

    auto* expect = R"(
S = struct @align(16) {
  a:vec4<u32> @offset(0)
  b:f16 @offset(16)
}

$B1: {  # root
  %v:ptr<workgroup, array<u16, 16>, read_write> = var undef
}

%foo = func():void {
  $B2: {
    %3:vec2<u16> = bitcast<vec2<u16>> 0u
    %4:ptr<workgroup, u16, read_write> = access %v, 2u
    %5:u16 = access %3, 0u
    store %4, %5 @align(4)
    %6:ptr<workgroup, u16, read_write> = access %v, 3u
    %7:u16 = access %3, 1u
    store %6, %7
    %8:ptr<workgroup, u16, read_write> = access %v, 8u
    %9:u16 = load %8
    %10:f16 = bitcast<f16> %9
    ret
  }
}
)";

    DecomposeAccessConfig options{.workgroup = true};
    Run(DecomposeAccess, options);

    EXPECT_EQ(expect, str());
}

TEST_F(IR_DecomposeAccessTest, AddAlignment_SubgroupMatrixLoad) {
    auto* mat_ty = ty.subgroup_matrix(SubgroupMatrixKind::kLeft, ty.f32(), 8, 8);
    auto* S =
        ty.Struct(mod.symbols.New("S"), {
                                            {mod.symbols.New("a"), ty.f16()},
                                            {mod.symbols.New("b"), ty.runtime_array(ty.f32())},
                                        });
    auto* v = b.Var("v", ty.ptr(storage, S));
    v->SetBindingPoint(0, 0);
    mod.root_block->Append(v);

    auto* foo = b.Function("foo", ty.void_());
    b.Append(foo->Block(), [&] {
        b.Load(b.Access(ty.ptr(storage, ty.f16()), v, 0_u));
        b.CallExplicit(mat_ty, BuiltinFn::kSubgroupMatrixLoad,
                       Vector<TemplateParameter, 2>{mat_ty, Majorness::kRowMajor},
                       b.Access(ty.ptr(storage, ty.runtime_array(ty.f32())), v, 1_u), 0_u, 8_u);
        b.Return(foo);
    });

    auto* src = R"(
S = struct @align(4) {
  a:f16 @offset(0)
  b:array<f32> @offset(4)
}

$B1: {  # root
  %v:ptr<storage, S, read_write> = var undef @binding_point(0, 0)
}

%foo = func():void {
  $B2: {
    %3:ptr<storage, f16, read_write> = access %v, 0u
    %4:f16 = load %3
    %5:ptr<storage, array<f32>, read_write> = access %v, 1u
    %6:subgroup_matrix_left<f32, 8, 8> = subgroupMatrixLoad<subgroup_matrix_left<f32, 8, 8>, row_major> %5, 0u, 8u
    ret
  }
}
)";

    ASSERT_EQ(src, str());

    auto* expect = R"(
S = struct @align(4) {
  a:f16 @offset(0)
  b:array<f32> @offset(4)
}

$B1: {  # root
  %v:ptr<storage, array<u16>, read_write> = var undef @binding_point(0, 0)
}

%foo = func():void {
  $B2: {
    %3:ptr<storage, u16, read_write> = access %v, 0u
    %4:u16 = load %3
    %5:f16 = bitcast<f16> %4
    %6:u32 = div 4u, 2u
    %7:u32 = mul 0u, 4u
    %8:u32 = div %7, 2u
    %9:u32 = add %6, %8
    %10:u32 = mul 8u, 4u
    %11:u32 = div %10, 2u
    %12:subgroup_matrix_left<f32, 8, 8> = subgroupMatrixLoad<subgroup_matrix_left<f32, 8, 8>, row_major> %v, %9, %11 @align(4)
    ret
  }
}
)";

    DecomposeAccessConfig options{.storage = true};
    Run(DecomposeAccess, options);

    EXPECT_EQ(expect, str());
}

TEST_F(IR_DecomposeAccessTest, AddAlignment_SubgroupMatrixStore) {
    auto* mat_ty = ty.subgroup_matrix(SubgroupMatrixKind::kLeft, ty.f32(), 8, 8);
    auto* S =
        ty.Struct(mod.symbols.New("S"), {
                                            {mod.symbols.New("a"), ty.f16()},
                                            {mod.symbols.New("b"), ty.runtime_array(ty.f32())},
                                        });
    auto* v = b.Var("v", ty.ptr(storage, S));
    v->SetBindingPoint(0, 0);
    mod.root_block->Append(v);

    auto* foo = b.Function("foo", ty.void_());
    b.Append(foo->Block(), [&] {
        b.Load(b.Access(ty.ptr(storage, ty.f16()), v, 0_u));
        auto* m = b.Construct(mat_ty);
        b.CallExplicit(ty.void_(), BuiltinFn::kSubgroupMatrixStore,
                       Vector<TemplateParameter, 1>{Majorness::kRowMajor},
                       b.Access(ty.ptr(storage, ty.runtime_array(ty.f32())), v, 1_u), 0_u, m, 8_u);
        b.Return(foo);
    });

    auto* src = R"(
S = struct @align(4) {
  a:f16 @offset(0)
  b:array<f32> @offset(4)
}

$B1: {  # root
  %v:ptr<storage, S, read_write> = var undef @binding_point(0, 0)
}

%foo = func():void {
  $B2: {
    %3:ptr<storage, f16, read_write> = access %v, 0u
    %4:f16 = load %3
    %5:subgroup_matrix_left<f32, 8, 8> = construct
    %6:ptr<storage, array<f32>, read_write> = access %v, 1u
    %7:void = subgroupMatrixStore<row_major> %6, 0u, %5, 8u
    ret
  }
}
)";

    ASSERT_EQ(src, str());

    auto* expect = R"(
S = struct @align(4) {
  a:f16 @offset(0)
  b:array<f32> @offset(4)
}

$B1: {  # root
  %v:ptr<storage, array<u16>, read_write> = var undef @binding_point(0, 0)
}

%foo = func():void {
  $B2: {
    %3:ptr<storage, u16, read_write> = access %v, 0u
    %4:u16 = load %3
    %5:f16 = bitcast<f16> %4
    %6:subgroup_matrix_left<f32, 8, 8> = construct
    %7:u32 = div 4u, 2u
    %8:u32 = mul 0u, 4u
    %9:u32 = div %8, 2u
    %10:u32 = add %7, %9
    %11:u32 = mul 8u, 4u
    %12:u32 = div %11, 2u
    %13:void = subgroupMatrixStore<row_major> %v, %10, %6, %12 @align(4)
    ret
  }
}
)";

    DecomposeAccessConfig options{.storage = true};
    Run(DecomposeAccess, options);

    EXPECT_EQ(expect, str());
}

}  // namespace

}  // namespace tint::core::ir::transform
