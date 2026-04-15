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

using IR_DecomposeAccessTest = core::ir::transform::TransformTest;

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
    DecomposeAccessOptions options{.uniform = true};
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

    DecomposeAccessOptions options{.uniform = true};
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

    DecomposeAccessOptions options{.uniform = true};
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
    DecomposeAccessOptions options{.uniform = true};
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
    DecomposeAccessOptions options{.uniform = true};
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
    DecomposeAccessOptions options{.uniform = true};
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
    DecomposeAccessOptions options{.uniform = true};
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
    DecomposeAccessOptions options{.uniform = true};
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
    DecomposeAccessOptions options{.uniform = true};
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
    DecomposeAccessOptions options{.uniform = true};
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
    DecomposeAccessOptions options{.uniform = true};
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
    DecomposeAccessOptions options{.uniform = true};
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

    DecomposeAccessOptions options{.uniform = true};
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

    DecomposeAccessOptions options{.uniform = true};
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

    DecomposeAccessOptions options{.uniform = true};
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

    DecomposeAccessOptions options{.uniform = true};
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

    capabilities.Add(Capability::kAllow16BitIntegers);
    DecomposeAccessOptions options{.storage = true};
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

    DecomposeAccessOptions options{.storage = true};
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

    DecomposeAccessOptions options{.workgroup = true};
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
    %7:u16 = load %6
    %8:ptr<storage, u16, read_write> = access %v, 3u
    %9:u16 = load %8
    %10:vec2<u16> = construct %7, %9
    %11:u32 = bitcast<u32> %10
    %b:u32 = let %11
    ret
  }
}
)";

    capabilities.Add(Capability::kAllow16BitIntegers);
    DecomposeAccessOptions options{.storage = true};
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
    %7:u16 = load %6
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

    capabilities.Add(Capability::kAllow16BitIntegers);
    DecomposeAccessOptions options{.workgroup = true};
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
    %7:u16 = load %6
    %8:ptr<storage, u16, read_write> = access %v, 3u
    %9:u16 = load %8
    %10:vec2<u16> = construct %7, %9
    %11:vec2<f16> = bitcast<vec2<f16>> %10
    %b:vec2<f16> = let %11
    ret
  }
}
)";

    capabilities.Add(Capability::kAllow16BitIntegers);
    DecomposeAccessOptions options{.storage = true};
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
    %7:u16 = load %6
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

    capabilities.Add(Capability::kAllow16BitIntegers);
    DecomposeAccessOptions options{.storage = true};
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
    %7:u16 = load %6
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

    capabilities.Add(Capability::kAllow16BitIntegers);
    DecomposeAccessOptions options{.storage = true};
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
    %7:u16 = load %6
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

    capabilities.Add(Capability::kAllow16BitIntegers);
    DecomposeAccessOptions options{.storage = true};
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
    %7:u16 = load %6
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

    capabilities.Add(Capability::kAllow16BitIntegers);
    DecomposeAccessOptions options{.storage = true};
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
    %7:u16 = load %6
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

    capabilities.Add(Capability::kAllow16BitIntegers);
    DecomposeAccessOptions options{.storage = true};
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
    %7:u16 = load %6
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

    capabilities.Add(Capability::kAllow16BitIntegers);
    DecomposeAccessOptions options{.workgroup = true};
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
    %7:u16 = load %6
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

    capabilities.Add(Capability::kAllow16BitIntegers);
    DecomposeAccessOptions options{.workgroup = true};
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
    %7:u16 = load %6
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

    capabilities.Add(Capability::kAllow16BitIntegers);
    DecomposeAccessOptions options{.workgroup = true};
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
    %7:u32 = load %6
    %8:ptr<storage, u32, read_write> = access %v, 3u
    %9:u32 = load %8
    %10:vec2<u32> = construct %7, %9
    %b:vec2<u32> = let %10
    ret
  }
}
)";

    capabilities.Add(Capability::kAllow16BitIntegers);
    DecomposeAccessOptions options{.storage = true};
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
    %7:u32 = load %6
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

    capabilities.Add(Capability::kAllow16BitIntegers);
    DecomposeAccessOptions options{.storage = true};
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
    %7:u32 = load %6
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

    capabilities.Add(Capability::kAllow16BitIntegers);
    DecomposeAccessOptions options{.storage = true};
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
    %7:u32 = load %6
    %8:ptr<workgroup, u32, read_write> = access %v, 3u
    %9:u32 = load %8
    %10:vec2<u32> = construct %7, %9
    %11:vec2<bool> = convert %10
    %b:vec2<bool> = let %11
    ret
  }
}
)";

    capabilities.Add(Capability::kAllow16BitIntegers);
    DecomposeAccessOptions options{.workgroup = true};
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
    %7:u32 = load %6
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

    capabilities.Add(Capability::kAllow16BitIntegers);
    DecomposeAccessOptions options{.workgroup = true};
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
    %7:u32 = load %6
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

    capabilities.Add(Capability::kAllow16BitIntegers);
    DecomposeAccessOptions options{.workgroup = true};
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

    capabilities.Add(Capability::kAllow16BitIntegers);
    DecomposeAccessOptions options{.storage = true};
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
    %4:u32 = load %3
    %5:ptr<storage, u32, read_write> = access %v, 1u
    %6:u32 = load %5
    %7:vec2<u32> = construct %4, %6
    %a:vec2<u32> = let %7
    %9:ptr<storage, u32, read_write> = access %v, 4u
    %10:u32 = load %9
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

    capabilities.Add(Capability::kAllow16BitIntegers);
    DecomposeAccessOptions options{.storage = true};
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
    %7:vec2<u32> = load %6
    %8:ptr<storage, vec2<u32>, read_write> = access %v, 3u
    %9:vec2<u32> = load %8
    %10:vec4<u32> = construct %7, %9
    %b:vec4<u32> = let %10
    ret
  }
}
)";

    capabilities.Add(Capability::kAllow16BitIntegers);
    DecomposeAccessOptions options{.storage = true};
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

    capabilities.Add(Capability::kAllow16BitIntegers);
    DecomposeAccessOptions options{.workgroup = true};
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
    %4:u32 = load %3
    %5:ptr<workgroup, u32, read_write> = access %v, 1u
    %6:u32 = load %5
    %7:vec2<u32> = construct %4, %6
    %a:vec2<u32> = let %7
    %9:ptr<workgroup, u32, read_write> = access %v, 4u
    %10:u32 = load %9
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

    capabilities.Add(Capability::kAllow16BitIntegers);
    DecomposeAccessOptions options{.workgroup = true};
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
    %7:vec2<u32> = load %6
    %8:ptr<workgroup, vec2<u32>, read_write> = access %v, 3u
    %9:vec2<u32> = load %8
    %10:vec4<u32> = construct %7, %9
    %11:vec4<bool> = convert %10
    %b:vec4<bool> = let %11
    ret
  }
}
)";

    capabilities.Add(Capability::kAllow16BitIntegers);
    DecomposeAccessOptions options{.workgroup = true};
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
        %23:u16 = load %22
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

    capabilities.Add(Capability::kAllow16BitIntegers);
    DecomposeAccessOptions options{.workgroup = true};
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
        store %23, %24
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

    capabilities.Add(Capability::kAllow16BitIntegers);
    DecomposeAccessOptions options{.workgroup = true};
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

    capabilities.Add(Capability::kAllow16BitIntegers);
    DecomposeAccessOptions options{.storage = true};
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

    capabilities.Add(Capability::kAllow16BitIntegers);
    DecomposeAccessOptions options{.storage = true};
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

    capabilities.Add(Capability::kAllow16BitIntegers);
    DecomposeAccessOptions options{.storage = true};
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

    capabilities.Add(Capability::kAllow16BitIntegers);
    DecomposeAccessOptions options{.storage = true};
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

    capabilities.Add(Capability::kAllow16BitIntegers);
    DecomposeAccessOptions options{.workgroup = true};
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
    store %5, %6
    %7:ptr<storage, u16, read_write> = access %v, 3u
    %8:u16 = access %4, 1u
    store %7, %8
    ret
  }
}
)";

    capabilities.Add(Capability::kAllow16BitIntegers);
    DecomposeAccessOptions options{.storage = true};
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
    store %6, %7
    %8:ptr<workgroup, u16, read_write> = access %v, 3u
    %9:u16 = access %5, 1u
    store %8, %9
    ret
  }
}
)";

    capabilities.Add(Capability::kAllow16BitIntegers);
    DecomposeAccessOptions options{.workgroup = true};
    Run(DecomposeAccess, options);
    EXPECT_EQ(expect, str());
}

TEST_F(IR_DecomposeAccessTest, Storage_AccessU16_StoreVec2h) {
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
    store %6, %5
    %7:f16 = access vec2<f16>(0.0h), 1u
    %8:u16 = bitcast<u16> %7
    %9:ptr<storage, u16, read_write> = access %v, 3u
    store %9, %8
    ret
  }
}
)";

    capabilities.Add(Capability::kAllow16BitIntegers);
    DecomposeAccessOptions options{.storage = true};
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
    store %6, %5
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

    capabilities.Add(Capability::kAllow16BitIntegers);
    DecomposeAccessOptions options{.storage = true};
    Run(DecomposeAccess, options);
    EXPECT_EQ(expect, str());
}

TEST_F(IR_DecomposeAccessTest, Storage_AccessU16_StoreVec4h) {
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
    store %6, %5
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

    capabilities.Add(Capability::kAllow16BitIntegers);
    DecomposeAccessOptions options{.storage = true};
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
    store %7, %6
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

    capabilities.Add(Capability::kAllow16BitIntegers);
    DecomposeAccessOptions options{.storage = true};
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
    store %7, %6
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

    capabilities.Add(Capability::kAllow16BitIntegers);
    DecomposeAccessOptions options{.storage = true};
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
    store %7, %6
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

    capabilities.Add(Capability::kAllow16BitIntegers);
    DecomposeAccessOptions options{.storage = true};
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
    store %8, %7
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

    capabilities.Add(Capability::kAllow16BitIntegers);
    DecomposeAccessOptions options{.workgroup = true};
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
    store %8, %7
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

    capabilities.Add(Capability::kAllow16BitIntegers);
    DecomposeAccessOptions options{.workgroup = true};
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
    store %8, %7
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

    capabilities.Add(Capability::kAllow16BitIntegers);
    DecomposeAccessOptions options{.workgroup = true};
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
    store %5, %4
    %6:u32 = access vec2<u32>(0u), 1u
    %7:ptr<storage, u32, read_write> = access %v, 3u
    store %7, %6
    ret
  }
}
)";

    capabilities.Add(Capability::kAllow16BitIntegers);
    DecomposeAccessOptions options{.storage = true};
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
    store %5, %4
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

    capabilities.Add(Capability::kAllow16BitIntegers);
    DecomposeAccessOptions options{.storage = true};
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
    store %5, %4
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

    capabilities.Add(Capability::kAllow16BitIntegers);
    DecomposeAccessOptions options{.storage = true};
    Run(DecomposeAccess, options);
    EXPECT_EQ(expect, str());
}

TEST_F(IR_DecomposeAccessTest, Storage_AccessU32_StoreVec2h) {
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
  %v:ptr<storage, array<u32, 2>, read_write> = var undef @binding_point(0, 0)
}

%foo = @fragment func():void {
  $B2: {
    %3:ptr<storage, u32, read_write> = access %v, 0u
    store %3, 0u
    %4:u32 = bitcast<u32> vec2<f16>(0.0h)
    %5:ptr<storage, u32, read_write> = access %v, 1u
    store %5, %4
    ret
  }
}
)";

    capabilities.Add(Capability::kAllow16BitIntegers);
    DecomposeAccessOptions options{.storage = true};
    Run(DecomposeAccess, options);
    EXPECT_EQ(expect, str());
}

// Note: No Storage_AccessU32_StoreVec3h (vec3<f16> uses u16, SmallestElementSize=2; covered by
// Storage_AccessU16_StoreVec3h above).

TEST_F(IR_DecomposeAccessTest, Storage_AccessU32_StoreVec4h) {
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
  %v:ptr<storage, array<u32, 4>, read_write> = var undef @binding_point(0, 0)
}

%foo = @fragment func():void {
  $B2: {
    %3:ptr<storage, u32, read_write> = access %v, 0u
    store %3, 0u
    %4:vec2<u32> = bitcast<vec2<u32>> vec4<f16>(0.0h)
    %5:u32 = access %4, 0u
    %6:ptr<storage, u32, read_write> = access %v, 2u
    store %6, %5
    %7:u32 = access %4, 1u
    %8:ptr<storage, u32, read_write> = access %v, 3u
    store %8, %7
    ret
  }
}
)";

    capabilities.Add(Capability::kAllow16BitIntegers);
    DecomposeAccessOptions options{.storage = true};
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
    store %6, %5
    %7:bool = access vec2<bool>(false), 1u
    %8:u32 = convert %7
    %9:ptr<workgroup, u32, read_write> = access %v, 3u
    store %9, %8
    ret
  }
}
)";

    capabilities.Add(Capability::kAllow16BitIntegers);
    DecomposeAccessOptions options{.workgroup = true};
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
    store %6, %5
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

    capabilities.Add(Capability::kAllow16BitIntegers);
    DecomposeAccessOptions options{.workgroup = true};
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
    store %6, %5
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

    capabilities.Add(Capability::kAllow16BitIntegers);
    DecomposeAccessOptions options{.workgroup = true};
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

    capabilities.Add(Capability::kAllow16BitIntegers);
    DecomposeAccessOptions options{.storage = true};
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

    capabilities.Add(Capability::kAllow16BitIntegers);
    DecomposeAccessOptions options{.storage = true};
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
            Vector{ty.u32()}, var, 16_u);
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

    capabilities.Add(Capability::kAllow16BitIntegers);
    DecomposeAccessOptions options{.storage = true};
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
            Vector{ty.u32()}, var, 16_u);
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

    capabilities.Add(Capability::kAllow16BitIntegers);
    DecomposeAccessOptions options{.workgroup = true};
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
            Vector{ty.u32()}, var, 36_u);
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

    capabilities.Add(Capability::kAllow16BitIntegers);
    DecomposeAccessOptions options{.uniform = true};
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

    capabilities.Add(Capability::kAllow16BitIntegers);
    DecomposeAccessOptions options{.uniform = true};
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
  %v:ptr<storage, array<vec4<u32>>, read> = var undef @binding_point(0, 0)
}

%foo = @fragment func():void {
  $B2: {
    %3:u32 = arrayLength %v
    %4:u32 = mul %3, 16u
    %a:u32 = let %4
    ret
  }
}
)";

    capabilities.Add(Capability::kAllow16BitIntegers);
    DecomposeAccessOptions options{.uniform = true};
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

    capabilities.Add(Capability::kAllow16BitIntegers);
    DecomposeAccessOptions options{.storage = true};
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

    capabilities.Add(Capability::kAllow16BitIntegers);
    DecomposeAccessOptions options{.storage = true};
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

    capabilities.Add(Capability::kAllow16BitIntegers);
    DecomposeAccessOptions options{.storage = true};
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

    capabilities.Add(Capability::kAllow16BitIntegers);
    DecomposeAccessOptions options{.storage = true};
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
        auto* view =
            b.CallExplicit(ty.ptr(storage, ty.runtime_array(sb), core::Access::kRead),
                           core::BuiltinFn::kBufferView, Vector{ty.runtime_array(sb)}, var, 64_u);
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

    capabilities.Add(Capability::kAllow16BitIntegers);
    DecomposeAccessOptions options{.storage = true};
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
        auto* view = b.CallExplicit(ty.ptr(storage, ty.runtime_array(sb), core::Access::kRead),
                                    core::BuiltinFn::kBufferView, Vector{ty.runtime_array(sb)}, var,
                                    b.Load(val));
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

    capabilities.Add(Capability::kAllow16BitIntegers);
    DecomposeAccessOptions options{.storage = true};
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
                                    core::BuiltinFn::kBufferView, Vector{outer}, var, b.Load(val));
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

    capabilities.Add(Capability::kAllow16BitIntegers);
    DecomposeAccessOptions options{.storage = true};
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

    capabilities.Add(Capability::kAllow16BitIntegers);
    DecomposeAccessOptions options{.uniform = true};
    Run(DecomposeAccess, options);
    EXPECT_EQ(expect, str());
}

}  // namespace
}  // namespace tint::core::ir::transform
