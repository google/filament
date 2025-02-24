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

#include "src/tint/lang/hlsl/writer/raise/decompose_uniform_access.h"

#include <gtest/gtest.h>

#include "src/tint/lang/core/fluent_types.h"
#include "src/tint/lang/core/ir/function.h"
#include "src/tint/lang/core/ir/transform/helper_test.h"
#include "src/tint/lang/core/number.h"

using namespace tint::core::fluent_types;     // NOLINT
using namespace tint::core::number_suffixes;  // NOLINT

namespace tint::hlsl::writer::raise {
namespace {

using HlslWriterDecomposeUniformAccessTest = core::ir::transform::TransformTest;

TEST_F(HlslWriterDecomposeUniformAccessTest, NoBufferAccess) {
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
    Run(DecomposeUniformAccess);

    EXPECT_EQ(expect, str());
}

TEST_F(HlslWriterDecomposeUniformAccessTest, UniformAccessChainFromUnnamedAccessChain) {
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
        auto* y = b.Access(ty.ptr(uniform, Inner, core::Access::kRead), x->Result(0), 1_u);
        b.Let("b",
              b.Load(b.Access(ty.ptr(uniform, ty.u32(), core::Access::kRead), y->Result(0), 1_u)));
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
  %v:ptr<uniform, array<SB, 4>, read> = var @binding_point(0, 0)
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
  %v:ptr<uniform, array<vec4<u32>, 8>, read> = var @binding_point(0, 0)
}

%foo = @fragment func():void {
  $B2: {
    %3:ptr<uniform, vec4<u32>, read> = access %v, 5u
    %4:u32 = load_vector_element %3, 1u
    %5:u32 = bitcast %4
    %b:u32 = let %5
    ret
  }
}
)";

    Run(DecomposeUniformAccess);
    EXPECT_EQ(expect, str());
}

TEST_F(HlslWriterDecomposeUniformAccessTest, UniformAccessChainFromLetAccessChain) {
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
            b.Let("y", b.Access(ty.ptr(uniform, Inner, core::Access::kRead), x->Result(0), 1_u));
        auto* z =
            b.Let("z", b.Access(ty.ptr(uniform, ty.f32(), core::Access::kRead), y->Result(0), 0_u));
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
  %v:ptr<uniform, SB, read> = var @binding_point(0, 0)
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
  %v:ptr<uniform, array<vec4<u32>, 2>, read> = var @binding_point(0, 0)
}

%foo = @fragment func():void {
  $B2: {
    %3:ptr<uniform, vec4<u32>, read> = access %v, 1u
    %4:u32 = load_vector_element %3, 0u
    %5:f32 = bitcast %4
    %a:f32 = let %5
    ret
  }
}
)";

    Run(DecomposeUniformAccess);
    EXPECT_EQ(expect, str());
}

TEST_F(HlslWriterDecomposeUniformAccessTest, UniformAccessVectorLoad) {
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
  %v:ptr<uniform, vec4<f32>, read> = var @binding_point(0, 0)
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
  %v:ptr<uniform, array<vec4<u32>, 1>, read> = var @binding_point(0, 0)
}

%foo = @fragment func():void {
  $B2: {
    %3:ptr<uniform, vec4<u32>, read> = access %v, 0u
    %4:vec4<u32> = load %3
    %5:vec4<f32> = bitcast %4
    %a:vec4<f32> = let %5
    %7:ptr<uniform, vec4<u32>, read> = access %v, 0u
    %8:u32 = load_vector_element %7, 0u
    %9:f32 = bitcast %8
    %b:f32 = let %9
    %11:ptr<uniform, vec4<u32>, read> = access %v, 0u
    %12:u32 = load_vector_element %11, 1u
    %13:f32 = bitcast %12
    %c:f32 = let %13
    %15:ptr<uniform, vec4<u32>, read> = access %v, 0u
    %16:u32 = load_vector_element %15, 2u
    %17:f32 = bitcast %16
    %d:f32 = let %17
    %19:ptr<uniform, vec4<u32>, read> = access %v, 0u
    %20:u32 = load_vector_element %19, 3u
    %21:f32 = bitcast %20
    %e:f32 = let %21
    ret
  }
}
)";
    Run(DecomposeUniformAccess);
    EXPECT_EQ(expect, str());
}

TEST_F(HlslWriterDecomposeUniformAccessTest, UniformAccessScalarF16) {
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
  %v:ptr<uniform, f16, read> = var @binding_point(0, 0)
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
  %v:ptr<uniform, array<vec4<u32>, 1>, read> = var @binding_point(0, 0)
}

%foo = @fragment func():void {
  $B2: {
    %3:ptr<uniform, vec4<u32>, read> = access %v, 0u
    %4:u32 = load_vector_element %3, 0u
    %5:f32 = hlsl.f16tof32 %4
    %6:f16 = convert %5
    %a:f16 = let %6
    ret
  }
}
)";
    Run(DecomposeUniformAccess);
    EXPECT_EQ(expect, str());
}

TEST_F(HlslWriterDecomposeUniformAccessTest, UniformAccessVectorF16) {
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
  %v:ptr<uniform, vec4<f16>, read> = var @binding_point(0, 0)
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
  %v:ptr<uniform, array<vec4<u32>, 1>, read> = var @binding_point(0, 0)
}

%foo = @fragment func():void {
  $B2: {
    %x:u32 = let 1u
    %4:ptr<uniform, vec4<u32>, read> = access %v, 0u
    %5:vec4<u32> = load %4
    %6:vec2<u32> = swizzle %5, xy
    %7:vec4<f16> = bitcast %6
    %a:vec4<f16> = let %7
    %9:ptr<uniform, vec4<u32>, read> = access %v, 0u
    %10:u32 = load_vector_element %9, 0u
    %11:f32 = hlsl.f16tof32 %10
    %12:f16 = convert %11
    %b:f16 = let %12
    %14:u32 = mul %x, 2u
    %15:u32 = div %14, 16u
    %16:ptr<uniform, vec4<u32>, read> = access %v, %15
    %17:u32 = mod %14, 16u
    %18:u32 = div %17, 4u
    %19:u32 = load_vector_element %16, %18
    %20:u32 = mod %14, 4u
    %21:bool = eq %20, 0u
    %22:u32 = hlsl.ternary 16u, 0u, %21
    %23:u32 = shr %19, %22
    %24:f32 = hlsl.f16tof32 %23
    %25:f16 = convert %24
    %c:f16 = let %25
    %27:ptr<uniform, vec4<u32>, read> = access %v, 0u
    %28:u32 = load_vector_element %27, 1u
    %29:f32 = hlsl.f16tof32 %28
    %30:f16 = convert %29
    %d:f16 = let %30
    %32:ptr<uniform, vec4<u32>, read> = access %v, 0u
    %33:u32 = load_vector_element %32, 1u
    %34:u32 = shr %33, 16u
    %35:f32 = hlsl.f16tof32 %34
    %36:f16 = convert %35
    %e:f16 = let %36
    ret
  }
}
)";
    Run(DecomposeUniformAccess);
    EXPECT_EQ(expect, str());
}

TEST_F(HlslWriterDecomposeUniformAccessTest, UniformAccessMat2x3F16) {
    auto* var = b.Var<uniform, mat2x3<f16>, core::Access::kRead>("v");
    var->SetBindingPoint(0, 0);

    b.ir.root_block->Append(var);
    auto* func = b.Function("foo", ty.void_(), core::ir::Function::PipelineStage::kFragment);
    b.Append(func->Block(), [&] {
        b.Let("a", b.Load(var));
        b.Let("b", b.Load(b.Access(ty.ptr(uniform, ty.vec3<f16>()), var, 1_u)));
        b.Let("c", b.LoadVectorElement(b.Access(ty.ptr(uniform, ty.vec3<f16>()), var, 1_u), 2_u));
        b.Return(func);
    });

    auto* src = R"(
$B1: {  # root
  %v:ptr<uniform, mat2x3<f16>, read> = var @binding_point(0, 0)
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
  %v:ptr<uniform, array<vec4<u32>, 1>, read> = var @binding_point(0, 0)
}

%foo = @fragment func():void {
  $B2: {
    %3:mat2x3<f16> = call %4, 0u
    %a:mat2x3<f16> = let %3
    %6:ptr<uniform, vec4<u32>, read> = access %v, 0u
    %7:vec4<u32> = load %6
    %8:vec2<u32> = swizzle %7, zw
    %9:vec4<f16> = bitcast %8
    %10:vec3<f16> = swizzle %9, xyz
    %b:vec3<f16> = let %10
    %12:ptr<uniform, vec4<u32>, read> = access %v, 0u
    %13:u32 = load_vector_element %12, 3u
    %14:f32 = hlsl.f16tof32 %13
    %15:f16 = convert %14
    %c:f16 = let %15
    ret
  }
}
%4 = func(%start_byte_offset:u32):mat2x3<f16> {
  $B3: {
    %18:u32 = div %start_byte_offset, 16u
    %19:ptr<uniform, vec4<u32>, read> = access %v, %18
    %20:u32 = mod %start_byte_offset, 16u
    %21:u32 = div %20, 4u
    %22:vec4<u32> = load %19
    %23:vec2<u32> = swizzle %22, zw
    %24:vec2<u32> = swizzle %22, xy
    %25:bool = eq %21, 2u
    %26:vec2<u32> = hlsl.ternary %24, %23, %25
    %27:vec4<f16> = bitcast %26
    %28:vec3<f16> = swizzle %27, xyz
    %29:u32 = add 8u, %start_byte_offset
    %30:u32 = div %29, 16u
    %31:ptr<uniform, vec4<u32>, read> = access %v, %30
    %32:u32 = mod %29, 16u
    %33:u32 = div %32, 4u
    %34:vec4<u32> = load %31
    %35:vec2<u32> = swizzle %34, zw
    %36:vec2<u32> = swizzle %34, xy
    %37:bool = eq %33, 2u
    %38:vec2<u32> = hlsl.ternary %36, %35, %37
    %39:vec4<f16> = bitcast %38
    %40:vec3<f16> = swizzle %39, xyz
    %41:mat2x3<f16> = construct %28, %40
    ret %41
  }
}
)";
    Run(DecomposeUniformAccess);
    EXPECT_EQ(expect, str());
}

TEST_F(HlslWriterDecomposeUniformAccessTest, UniformAccessMatrix) {
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
  %v:ptr<uniform, mat4x4<f32>, read> = var @binding_point(0, 0)
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
  %v:ptr<uniform, array<vec4<u32>, 4>, read> = var @binding_point(0, 0)
}

%foo = @fragment func():void {
  $B2: {
    %3:mat4x4<f32> = call %4, 0u
    %a:mat4x4<f32> = let %3
    %6:ptr<uniform, vec4<u32>, read> = access %v, 3u
    %7:vec4<u32> = load %6
    %8:vec4<f32> = bitcast %7
    %b:vec4<f32> = let %8
    %10:ptr<uniform, vec4<u32>, read> = access %v, 1u
    %11:u32 = load_vector_element %10, 2u
    %12:f32 = bitcast %11
    %c:f32 = let %12
    ret
  }
}
%4 = func(%start_byte_offset:u32):mat4x4<f32> {
  $B3: {
    %15:u32 = div %start_byte_offset, 16u
    %16:ptr<uniform, vec4<u32>, read> = access %v, %15
    %17:vec4<u32> = load %16
    %18:vec4<f32> = bitcast %17
    %19:u32 = add 16u, %start_byte_offset
    %20:u32 = div %19, 16u
    %21:ptr<uniform, vec4<u32>, read> = access %v, %20
    %22:vec4<u32> = load %21
    %23:vec4<f32> = bitcast %22
    %24:u32 = add 32u, %start_byte_offset
    %25:u32 = div %24, 16u
    %26:ptr<uniform, vec4<u32>, read> = access %v, %25
    %27:vec4<u32> = load %26
    %28:vec4<f32> = bitcast %27
    %29:u32 = add 48u, %start_byte_offset
    %30:u32 = div %29, 16u
    %31:ptr<uniform, vec4<u32>, read> = access %v, %30
    %32:vec4<u32> = load %31
    %33:vec4<f32> = bitcast %32
    %34:mat4x4<f32> = construct %18, %23, %28, %33
    ret %34
  }
}
)";
    Run(DecomposeUniformAccess);
    EXPECT_EQ(expect, str());
}

TEST_F(HlslWriterDecomposeUniformAccessTest, UniformAccessArray) {
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
  %v:ptr<uniform, array<vec3<f32>, 5>, read> = var @binding_point(0, 0)
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
  %v:ptr<uniform, array<vec4<u32>, 5>, read> = var @binding_point(0, 0)
}

%foo = @fragment func():void {
  $B2: {
    %3:array<vec3<f32>, 5> = call %4, 0u
    %a:array<vec3<f32>, 5> = let %3
    %6:ptr<uniform, vec4<u32>, read> = access %v, 3u
    %7:vec4<u32> = load %6
    %8:vec3<u32> = swizzle %7, xyz
    %9:vec3<f32> = bitcast %8
    %b:vec3<f32> = let %9
    ret
  }
}
%4 = func(%start_byte_offset:u32):array<vec3<f32>, 5> {
  $B3: {
    %a_1:ptr<function, array<vec3<f32>, 5>, read_write> = var, array<vec3<f32>, 5>(vec3<f32>(0.0f))  # %a_1: 'a'
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
        %22:vec3<f32> = bitcast %21
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
    Run(DecomposeUniformAccess);
    EXPECT_EQ(expect, str());
}

TEST_F(HlslWriterDecomposeUniformAccessTest, UniformAccessArrayWhichCanHaveSizesOtherThenFive) {
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
  %v:ptr<uniform, array<vec3<f32>, 42>, read> = var @binding_point(0, 0)
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
  %v:ptr<uniform, array<vec4<u32>, 42>, read> = var @binding_point(0, 0)
}

%foo = @fragment func():void {
  $B2: {
    %3:array<vec3<f32>, 42> = call %4, 0u
    %a:array<vec3<f32>, 42> = let %3
    %6:ptr<uniform, vec4<u32>, read> = access %v, 3u
    %7:vec4<u32> = load %6
    %8:vec3<u32> = swizzle %7, xyz
    %9:vec3<f32> = bitcast %8
    %b:vec3<f32> = let %9
    ret
  }
}
%4 = func(%start_byte_offset:u32):array<vec3<f32>, 42> {
  $B3: {
    %a_1:ptr<function, array<vec3<f32>, 42>, read_write> = var, array<vec3<f32>, 42>(vec3<f32>(0.0f))  # %a_1: 'a'
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
        %22:vec3<f32> = bitcast %21
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
    Run(DecomposeUniformAccess);
    EXPECT_EQ(expect, str());
}

TEST_F(HlslWriterDecomposeUniformAccessTest, UniformAccessStruct) {
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
  %v:ptr<uniform, SB, read> = var @binding_point(0, 0)
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
  %v:ptr<uniform, array<vec4<u32>, 1>, read> = var @binding_point(0, 0)
}

%foo = @fragment func():void {
  $B2: {
    %3:SB = call %4, 0u
    %a:SB = let %3
    %6:ptr<uniform, vec4<u32>, read> = access %v, 0u
    %7:u32 = load_vector_element %6, 1u
    %8:f32 = bitcast %7
    %b:f32 = let %8
    ret
  }
}
%4 = func(%start_byte_offset:u32):SB {
  $B3: {
    %11:u32 = div %start_byte_offset, 16u
    %12:ptr<uniform, vec4<u32>, read> = access %v, %11
    %13:u32 = mod %start_byte_offset, 16u
    %14:u32 = div %13, 4u
    %15:u32 = load_vector_element %12, %14
    %16:i32 = bitcast %15
    %17:u32 = add 4u, %start_byte_offset
    %18:u32 = div %17, 16u
    %19:ptr<uniform, vec4<u32>, read> = access %v, %18
    %20:u32 = mod %17, 16u
    %21:u32 = div %20, 4u
    %22:u32 = load_vector_element %19, %21
    %23:f32 = bitcast %22
    %24:SB = construct %16, %23
    ret %24
  }
}
)";
    Run(DecomposeUniformAccess);
    EXPECT_EQ(expect, str());
}

TEST_F(HlslWriterDecomposeUniformAccessTest, UniformAccessStructNested) {
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
  %v:ptr<uniform, SB, read> = var @binding_point(0, 0)
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
  %v:ptr<uniform, array<vec4<u32>, 10>, read> = var @binding_point(0, 0)
}

%foo = @fragment func():void {
  $B2: {
    %3:SB = call %4, 0u
    %a:SB = let %3
    %6:ptr<uniform, vec4<u32>, read> = access %v, 8u
    %7:u32 = load_vector_element %6, 2u
    %8:f32 = bitcast %7
    %b:f32 = let %8
    ret
  }
}
%4 = func(%start_byte_offset:u32):SB {
  $B3: {
    %11:u32 = div %start_byte_offset, 16u
    %12:ptr<uniform, vec4<u32>, read> = access %v, %11
    %13:u32 = mod %start_byte_offset, 16u
    %14:u32 = div %13, 4u
    %15:u32 = load_vector_element %12, %14
    %16:i32 = bitcast %15
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
    %24:u32 = mod %start_byte_offset_1, 16u
    %25:u32 = div %24, 4u
    %26:u32 = load_vector_element %23, %25
    %27:f32 = bitcast %26
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
    %44:vec3<f32> = bitcast %43
    %45:u32 = add 16u, %start_byte_offset_3
    %46:u32 = div %45, 16u
    %47:ptr<uniform, vec4<u32>, read> = access %v, %46
    %48:vec4<u32> = load %47
    %49:vec3<u32> = swizzle %48, xyz
    %50:vec3<f32> = bitcast %49
    %51:u32 = add 32u, %start_byte_offset_3
    %52:u32 = div %51, 16u
    %53:ptr<uniform, vec4<u32>, read> = access %v, %52
    %54:vec4<u32> = load %53
    %55:vec3<u32> = swizzle %54, xyz
    %56:vec3<f32> = bitcast %55
    %57:mat3x3<f32> = construct %44, %50, %56
    ret %57
  }
}
%37 = func(%start_byte_offset_4:u32):array<vec3<f32>, 5> {  # %start_byte_offset_4: 'start_byte_offset'
  $B7: {
    %a_1:ptr<function, array<vec3<f32>, 5>, read_write> = var, array<vec3<f32>, 5>(vec3<f32>(0.0f))  # %a_1: 'a'
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
        %69:vec3<f32> = bitcast %68
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
    Run(DecomposeUniformAccess);
    EXPECT_EQ(expect, str());
}

TEST_F(HlslWriterDecomposeUniformAccessTest, UniformAccessChainReused) {
    auto* sb = ty.Struct(mod.symbols.New("SB"), {
                                                    {mod.symbols.New("c"), ty.f32()},
                                                    {mod.symbols.New("d"), ty.vec3<f32>()},
                                                });

    auto* var = b.Var("v", uniform, sb, core::Access::kRead);
    var->SetBindingPoint(0, 0);
    b.ir.root_block->Append(var);

    auto* func = b.Function("foo", ty.void_(), core::ir::Function::PipelineStage::kFragment);
    b.Append(func->Block(), [&] {
        auto* x = b.Access(ty.ptr(uniform, ty.vec3<f32>(), core::Access::kRead), var, 1_u);
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
  %v:ptr<uniform, SB, read> = var @binding_point(0, 0)
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
  %v:ptr<uniform, array<vec4<u32>, 2>, read> = var @binding_point(0, 0)
}

%foo = @fragment func():void {
  $B2: {
    %3:ptr<uniform, vec4<u32>, read> = access %v, 1u
    %4:u32 = load_vector_element %3, 1u
    %5:f32 = bitcast %4
    %b:f32 = let %5
    %7:ptr<uniform, vec4<u32>, read> = access %v, 1u
    %8:u32 = load_vector_element %7, 2u
    %9:f32 = bitcast %8
    %c:f32 = let %9
    ret
  }
}
)";

    Run(DecomposeUniformAccess);
    EXPECT_EQ(expect, str());
}

TEST_F(HlslWriterDecomposeUniformAccessTest, Determinism_MultipleUsesOfLetFromVar) {
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
  %v:ptr<uniform, SB, read> = var @binding_point(0, 0)
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
  %v:ptr<uniform, array<vec4<u32>, 4>, read> = var @binding_point(0, 0)
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
    %a_1:ptr<function, array<vec4<i32>, 2>, read_write> = var, array<vec4<i32>, 2>(vec4<i32>(0i))  # %a_1: 'a'
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
        %19:vec4<i32> = bitcast %18
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
    %a_2:ptr<function, array<vec4<f32>, 2>, read_write> = var, array<vec4<f32>, 2>(vec4<f32>(0.0f))  # %a_2: 'a'
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
        %32:vec4<f32> = bitcast %31
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

    Run(DecomposeUniformAccess);
    EXPECT_EQ(expect, str());
}

TEST_F(HlslWriterDecomposeUniformAccessTest, Determinism_MultipleUsesOfLetFromAccess) {
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
  %v:ptr<uniform, array<SB, 2>, read> = var @binding_point(0, 0)
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
  %v:ptr<uniform, array<vec4<u32>, 8>, read> = var @binding_point(0, 0)
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
    %a_1:ptr<function, array<vec4<i32>, 2>, read_write> = var, array<vec4<i32>, 2>(vec4<i32>(0i))  # %a_1: 'a'
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
        %19:vec4<i32> = bitcast %18
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
    %a_2:ptr<function, array<vec4<f32>, 2>, read_write> = var, array<vec4<f32>, 2>(vec4<f32>(0.0f))  # %a_2: 'a'
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
        %32:vec4<f32> = bitcast %31
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

    Run(DecomposeUniformAccess);
    EXPECT_EQ(expect, str());
}

TEST_F(HlslWriterDecomposeUniformAccessTest, Determinism_MultipleUsesOfAccess) {
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
  %v:ptr<uniform, array<SB, 2>, read> = var @binding_point(0, 0)
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
  %v:ptr<uniform, array<vec4<u32>, 8>, read> = var @binding_point(0, 0)
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
    %a_1:ptr<function, array<vec4<i32>, 2>, read_write> = var, array<vec4<i32>, 2>(vec4<i32>(0i))  # %a_1: 'a'
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
        %19:vec4<i32> = bitcast %18
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
    %a_2:ptr<function, array<vec4<f32>, 2>, read_write> = var, array<vec4<f32>, 2>(vec4<f32>(0.0f))  # %a_2: 'a'
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
        %32:vec4<f32> = bitcast %31
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

    Run(DecomposeUniformAccess);
    EXPECT_EQ(expect, str());
}

}  // namespace
}  // namespace tint::hlsl::writer::raise
