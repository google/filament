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

#include "src/tint/lang/hlsl/writer/raise/decompose_storage_access.h"

#include <gtest/gtest.h>

#include <string>

#include "src/tint/lang/core/fluent_types.h"
#include "src/tint/lang/core/ir/function.h"
#include "src/tint/lang/core/ir/transform/helper_test.h"
#include "src/tint/lang/core/number.h"
#include "src/tint/lang/core/type/builtin_structs.h"

using namespace tint::core::fluent_types;     // NOLINT
using namespace tint::core::number_suffixes;  // NOLINT

namespace tint::hlsl::writer::raise {
namespace {

using HlslWriterDecomposeStorageAccessTest = core::ir::transform::TransformTest;

TEST_F(HlslWriterDecomposeStorageAccessTest, NoBufferAccess) {
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
    Run(DecomposeStorageAccess);

    EXPECT_EQ(expect, str());
}

TEST_F(HlslWriterDecomposeStorageAccessTest, AccessChainFromUnnamedAccessChain) {
    auto* Inner = ty.Struct(mod.symbols.New("Inner"), {
                                                          {mod.symbols.New("c"), ty.f32()},
                                                          {mod.symbols.New("d"), ty.u32()},
                                                      });
    auto* sb = ty.Struct(mod.symbols.New("SB"), {
                                                    {mod.symbols.New("a"), ty.i32()},
                                                    {mod.symbols.New("b"), Inner},
                                                });

    auto* var = b.Var("v", storage, ty.array(sb, 4), core::Access::kReadWrite);
    var->SetBindingPoint(0, 0);
    b.ir.root_block->Append(var);

    auto* func = b.Function("foo", ty.void_(), core::ir::Function::PipelineStage::kFragment);
    b.Append(func->Block(), [&] {
        auto* x = b.Access(ty.ptr(storage, sb, core::Access::kReadWrite), var, 2_u);
        auto* y = b.Access(ty.ptr(storage, Inner, core::Access::kReadWrite), x->Result(), 1_u);
        b.Let("b", b.Load(b.Access(ty.ptr(storage, ty.u32(), core::Access::kReadWrite), y->Result(),
                                   1_u)));
        b.Return(func);
    });

    auto* src = R"(
Inner = struct @align(4) {
  c:f32 @offset(0)
  d:u32 @offset(4)
}

SB = struct @align(4) {
  a:i32 @offset(0)
  b:Inner @offset(4)
}

$B1: {  # root
  %v:ptr<storage, array<SB, 4>, read_write> = var undef @binding_point(0, 0)
}

%foo = @fragment func():void {
  $B2: {
    %3:ptr<storage, SB, read_write> = access %v, 2u
    %4:ptr<storage, Inner, read_write> = access %3, 1u
    %5:ptr<storage, u32, read_write> = access %4, 1u
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

SB = struct @align(4) {
  a:i32 @offset(0)
  b:Inner @offset(4)
}

$B1: {  # root
  %v:hlsl.byte_address_buffer<read_write> = var undef @binding_point(0, 0)
}

%foo = @fragment func():void {
  $B2: {
    %3:u32 = %v.Load 32u
    %4:u32 = bitcast %3
    %b:u32 = let %4
    ret
  }
}
)";

    Run(DecomposeStorageAccess);
    EXPECT_EQ(expect, str());
}

TEST_F(HlslWriterDecomposeStorageAccessTest, AccessChainFromLetAccessChain) {
    auto* Inner = ty.Struct(mod.symbols.New("Inner"), {
                                                          {mod.symbols.New("c"), ty.f32()},
                                                      });
    auto* sb = ty.Struct(mod.symbols.New("SB"), {
                                                    {mod.symbols.New("a"), ty.i32()},
                                                    {mod.symbols.New("b"), Inner},
                                                });

    auto* var = b.Var("v", storage, sb, core::Access::kReadWrite);
    var->SetBindingPoint(0, 0);
    b.ir.root_block->Append(var);

    auto* func = b.Function("foo", ty.void_(), core::ir::Function::PipelineStage::kFragment);
    b.Append(func->Block(), [&] {
        auto* x = b.Let("x", var);
        auto* y = b.Let(
            "y", b.Access(ty.ptr(storage, Inner, core::Access::kReadWrite), x->Result(), 1_u));
        auto* z = b.Let(
            "z", b.Access(ty.ptr(storage, ty.f32(), core::Access::kReadWrite), y->Result(), 0_u));
        b.Let("a", b.Load(z));
        b.Return(func);
    });

    auto* src = R"(
Inner = struct @align(4) {
  c:f32 @offset(0)
}

SB = struct @align(4) {
  a:i32 @offset(0)
  b:Inner @offset(4)
}

$B1: {  # root
  %v:ptr<storage, SB, read_write> = var undef @binding_point(0, 0)
}

%foo = @fragment func():void {
  $B2: {
    %x:ptr<storage, SB, read_write> = let %v
    %4:ptr<storage, Inner, read_write> = access %x, 1u
    %y:ptr<storage, Inner, read_write> = let %4
    %6:ptr<storage, f32, read_write> = access %y, 0u
    %z:ptr<storage, f32, read_write> = let %6
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

SB = struct @align(4) {
  a:i32 @offset(0)
  b:Inner @offset(4)
}

$B1: {  # root
  %v:hlsl.byte_address_buffer<read_write> = var undef @binding_point(0, 0)
}

%foo = @fragment func():void {
  $B2: {
    %3:u32 = %v.Load 4u
    %4:f32 = bitcast %3
    %a:f32 = let %4
    ret
  }
}
)";

    Run(DecomposeStorageAccess);
    EXPECT_EQ(expect, str());
}

TEST_F(HlslWriterDecomposeStorageAccessTest, AccessRwByteAddressBuffer) {
    auto* sb = ty.Struct(mod.symbols.New("SB"), {
                                                    {mod.symbols.New("a"), ty.i32()},
                                                    {mod.symbols.New("b"), ty.vec3<f32>()},
                                                });

    auto* var = b.Var("v", storage, sb, core::Access::kReadWrite);
    var->SetBindingPoint(0, 0);
    b.ir.root_block->Append(var);

    auto* func = b.Function("foo", ty.void_(), core::ir::Function::PipelineStage::kFragment);
    b.Append(func->Block(), [&] {
        b.Let("a", b.Load(b.Access(ty.ptr(storage, ty.i32(), core::Access::kReadWrite), var, 0_u)));
        b.Let("b", b.Load(b.Access(ty.ptr(storage, ty.vec3<f32>(), core::Access::kReadWrite), var,
                                   1_u)));
        b.Return(func);
    });

    auto* src = R"(
SB = struct @align(16) {
  a:i32 @offset(0)
  b:vec3<f32> @offset(16)
}

$B1: {  # root
  %v:ptr<storage, SB, read_write> = var undef @binding_point(0, 0)
}

%foo = @fragment func():void {
  $B2: {
    %3:ptr<storage, i32, read_write> = access %v, 0u
    %4:i32 = load %3
    %a:i32 = let %4
    %6:ptr<storage, vec3<f32>, read_write> = access %v, 1u
    %7:vec3<f32> = load %6
    %b:vec3<f32> = let %7
    ret
  }
}
)";
    ASSERT_EQ(src, str());

    auto* expect = R"(
SB = struct @align(16) {
  a:i32 @offset(0)
  b:vec3<f32> @offset(16)
}

$B1: {  # root
  %v:hlsl.byte_address_buffer<read_write> = var undef @binding_point(0, 0)
}

%foo = @fragment func():void {
  $B2: {
    %3:u32 = %v.Load 0u
    %4:i32 = bitcast %3
    %a:i32 = let %4
    %6:vec3<u32> = %v.Load3 16u
    %7:vec3<f32> = bitcast %6
    %b:vec3<f32> = let %7
    ret
  }
}
)";

    Run(DecomposeStorageAccess);
    EXPECT_EQ(expect, str());
}

TEST_F(HlslWriterDecomposeStorageAccessTest, AccessByteAddressBuffer) {
    auto* sb = ty.Struct(mod.symbols.New("SB"), {
                                                    {mod.symbols.New("a"), ty.i32()},
                                                });
    auto* var = b.Var("v", storage, sb, core::Access::kRead);
    var->SetBindingPoint(0, 0);
    b.ir.root_block->Append(var);

    auto* func = b.Function("foo", ty.void_(), core::ir::Function::PipelineStage::kFragment);
    b.Append(func->Block(), [&] {
        b.Let("a", b.Load(b.Access(ty.ptr(storage, ty.i32(), core::Access::kRead), var, 0_u)));
        b.Return(func);
    });

    auto* src = R"(
SB = struct @align(4) {
  a:i32 @offset(0)
}

$B1: {  # root
  %v:ptr<storage, SB, read> = var undef @binding_point(0, 0)
}

%foo = @fragment func():void {
  $B2: {
    %3:ptr<storage, i32, read> = access %v, 0u
    %4:i32 = load %3
    %a:i32 = let %4
    ret
  }
}
)";
    ASSERT_EQ(src, str());

    auto* expect = R"(
SB = struct @align(4) {
  a:i32 @offset(0)
}

$B1: {  # root
  %v:hlsl.byte_address_buffer<read> = var undef @binding_point(0, 0)
}

%foo = @fragment func():void {
  $B2: {
    %3:u32 = %v.Load 0u
    %4:i32 = bitcast %3
    %a:i32 = let %4
    ret
  }
}
)";

    Run(DecomposeStorageAccess);
    EXPECT_EQ(expect, str());
}

TEST_F(HlslWriterDecomposeStorageAccessTest, AccessStorageVector) {
    auto* var = b.Var<storage, vec4<f32>, core::Access::kRead>("v");
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
  %v:ptr<storage, vec4<f32>, read> = var undef @binding_point(0, 0)
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
  %v:hlsl.byte_address_buffer<read> = var undef @binding_point(0, 0)
}

%foo = @fragment func():void {
  $B2: {
    %3:vec4<u32> = %v.Load4 0u
    %4:vec4<f32> = bitcast %3
    %a:vec4<f32> = let %4
    %6:u32 = %v.Load 0u
    %7:f32 = bitcast %6
    %b:f32 = let %7
    %9:u32 = %v.Load 4u
    %10:f32 = bitcast %9
    %c:f32 = let %10
    %12:u32 = %v.Load 8u
    %13:f32 = bitcast %12
    %d:f32 = let %13
    %15:u32 = %v.Load 12u
    %16:f32 = bitcast %15
    %e:f32 = let %16
    ret
  }
}
)";
    Run(DecomposeStorageAccess);
    EXPECT_EQ(expect, str());
}

TEST_F(HlslWriterDecomposeStorageAccessTest, AccessStorageVectorF16) {
    auto* var = b.Var<storage, vec4<f16>, core::Access::kRead>("v");
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
  %v:ptr<storage, vec4<f16>, read> = var undef @binding_point(0, 0)
}

%foo = @fragment func():void {
  $B2: {
    %3:vec4<f16> = load %v
    %a:vec4<f16> = let %3
    %5:f16 = load_vector_element %v, 0u
    %b:f16 = let %5
    %7:f16 = load_vector_element %v, 1u
    %c:f16 = let %7
    %9:f16 = load_vector_element %v, 2u
    %d:f16 = let %9
    %11:f16 = load_vector_element %v, 3u
    %e:f16 = let %11
    ret
  }
}
)";
    ASSERT_EQ(src, str());

    auto* expect = R"(
$B1: {  # root
  %v:hlsl.byte_address_buffer<read> = var undef @binding_point(0, 0)
}

%foo = @fragment func():void {
  $B2: {
    %3:vec4<f16> = %v.Load4F16 0u
    %a:vec4<f16> = let %3
    %5:f16 = %v.LoadF16 0u
    %b:f16 = let %5
    %7:f16 = %v.LoadF16 2u
    %c:f16 = let %7
    %9:f16 = %v.LoadF16 4u
    %d:f16 = let %9
    %11:f16 = %v.LoadF16 6u
    %e:f16 = let %11
    ret
  }
}
)";
    Run(DecomposeStorageAccess);
    EXPECT_EQ(expect, str());
}

TEST_F(HlslWriterDecomposeStorageAccessTest, AccessStorageMatrix) {
    auto* var = b.Var<storage, mat4x4<f32>, core::Access::kRead>("v");
    var->SetBindingPoint(0, 0);

    b.ir.root_block->Append(var);
    auto* func = b.Function("foo", ty.void_(), core::ir::Function::PipelineStage::kFragment);
    b.Append(func->Block(), [&] {
        b.Let("a", b.Load(var));
        b.Let("b", b.Load(b.Access(ty.ptr<storage, vec4<f32>, core::Access::kRead>(), var, 3_u)));
        b.Let("c", b.LoadVectorElement(
                       b.Access(ty.ptr<storage, vec4<f32>, core::Access::kRead>(), var, 1_u), 2_u));
        b.Return(func);
    });

    auto* src = R"(
$B1: {  # root
  %v:ptr<storage, mat4x4<f32>, read> = var undef @binding_point(0, 0)
}

%foo = @fragment func():void {
  $B2: {
    %3:mat4x4<f32> = load %v
    %a:mat4x4<f32> = let %3
    %5:ptr<storage, vec4<f32>, read> = access %v, 3u
    %6:vec4<f32> = load %5
    %b:vec4<f32> = let %6
    %8:ptr<storage, vec4<f32>, read> = access %v, 1u
    %9:f32 = load_vector_element %8, 2u
    %c:f32 = let %9
    ret
  }
}
)";
    ASSERT_EQ(src, str());

    auto* expect = R"(
$B1: {  # root
  %v:hlsl.byte_address_buffer<read> = var undef @binding_point(0, 0)
}

%foo = @fragment func():void {
  $B2: {
    %3:mat4x4<f32> = call %4, 0u
    %a:mat4x4<f32> = let %3
    %6:vec4<u32> = %v.Load4 48u
    %7:vec4<f32> = bitcast %6
    %b:vec4<f32> = let %7
    %9:u32 = %v.Load 24u
    %10:f32 = bitcast %9
    %c:f32 = let %10
    ret
  }
}
%4 = func(%offset:u32):mat4x4<f32> {
  $B3: {
    %13:u32 = add %offset, 0u
    %14:vec4<u32> = %v.Load4 %13
    %15:vec4<f32> = bitcast %14
    %16:u32 = add %offset, 16u
    %17:vec4<u32> = %v.Load4 %16
    %18:vec4<f32> = bitcast %17
    %19:u32 = add %offset, 32u
    %20:vec4<u32> = %v.Load4 %19
    %21:vec4<f32> = bitcast %20
    %22:u32 = add %offset, 48u
    %23:vec4<u32> = %v.Load4 %22
    %24:vec4<f32> = bitcast %23
    %25:mat4x4<f32> = construct %15, %18, %21, %24
    ret %25
  }
}
)";
    Run(DecomposeStorageAccess);
    EXPECT_EQ(expect, str());
}

TEST_F(HlslWriterDecomposeStorageAccessTest, AccessStorageArray) {
    auto* var = b.Var<storage, array<vec3<f32>, 5>, core::Access::kRead>("v");
    var->SetBindingPoint(0, 0);

    b.ir.root_block->Append(var);
    auto* func = b.Function("foo", ty.void_(), core::ir::Function::PipelineStage::kFragment);
    b.Append(func->Block(), [&] {
        b.Let("a", b.Load(var));
        b.Let("b", b.Load(b.Access(ty.ptr<storage, vec3<f32>, core::Access::kRead>(), var, 3_u)));
        b.Return(func);
    });

    auto* src = R"(
$B1: {  # root
  %v:ptr<storage, array<vec3<f32>, 5>, read> = var undef @binding_point(0, 0)
}

%foo = @fragment func():void {
  $B2: {
    %3:array<vec3<f32>, 5> = load %v
    %a:array<vec3<f32>, 5> = let %3
    %5:ptr<storage, vec3<f32>, read> = access %v, 3u
    %6:vec3<f32> = load %5
    %b:vec3<f32> = let %6
    ret
  }
}
)";
    ASSERT_EQ(src, str());

    auto* expect = R"(
$B1: {  # root
  %v:hlsl.byte_address_buffer<read> = var undef @binding_point(0, 0)
}

%foo = @fragment func():void {
  $B2: {
    %3:array<vec3<f32>, 5> = call %4, 0u
    %a:array<vec3<f32>, 5> = let %3
    %6:vec3<u32> = %v.Load3 48u
    %7:vec3<f32> = bitcast %6
    %b:vec3<f32> = let %7
    ret
  }
}
%4 = func(%offset:u32):array<vec3<f32>, 5> {
  $B3: {
    %a_1:ptr<function, array<vec3<f32>, 5>, read_write> = var array<vec3<f32>, 5>(vec3<f32>(0.0f))  # %a_1: 'a'
    loop [i: $B4, b: $B5, c: $B6] {  # loop_1
      $B4: {  # initializer
        next_iteration 0u  # -> $B5
      }
      $B5 (%idx:u32): {  # body
        %12:bool = gte %idx, 5u
        if %12 [t: $B7] {  # if_1
          $B7: {  # true
            exit_loop  # loop_1
          }
        }
        %13:ptr<function, vec3<f32>, read_write> = access %a_1, %idx
        %14:u32 = mul %idx, 16u
        %15:u32 = add %offset, %14
        %16:vec3<u32> = %v.Load3 %15
        %17:vec3<f32> = bitcast %16
        store %13, %17
        continue  # -> $B6
      }
      $B6: {  # continuing
        %18:u32 = add %idx, 1u
        next_iteration %18  # -> $B5
      }
    }
    %19:array<vec3<f32>, 5> = load %a_1
    ret %19
  }
}
)";
    Run(DecomposeStorageAccess);
    EXPECT_EQ(expect, str());
}

TEST_F(HlslWriterDecomposeStorageAccessTest, AccessStorageArrayWhichCanHaveSizesOtherThenFive) {
    auto* var = b.Var<storage, array<vec3<f32>, 42>, core::Access::kRead>("v");
    var->SetBindingPoint(0, 0);

    b.ir.root_block->Append(var);
    auto* func = b.Function("foo", ty.void_(), core::ir::Function::PipelineStage::kFragment);
    b.Append(func->Block(), [&] {
        b.Let("a", b.Load(var));
        b.Let("b", b.Load(b.Access(ty.ptr<storage, vec3<f32>, core::Access::kRead>(), var, 3_u)));
        b.Return(func);
    });

    auto* src = R"(
$B1: {  # root
  %v:ptr<storage, array<vec3<f32>, 42>, read> = var undef @binding_point(0, 0)
}

%foo = @fragment func():void {
  $B2: {
    %3:array<vec3<f32>, 42> = load %v
    %a:array<vec3<f32>, 42> = let %3
    %5:ptr<storage, vec3<f32>, read> = access %v, 3u
    %6:vec3<f32> = load %5
    %b:vec3<f32> = let %6
    ret
  }
}
)";
    ASSERT_EQ(src, str());

    auto* expect = R"(
$B1: {  # root
  %v:hlsl.byte_address_buffer<read> = var undef @binding_point(0, 0)
}

%foo = @fragment func():void {
  $B2: {
    %3:array<vec3<f32>, 42> = call %4, 0u
    %a:array<vec3<f32>, 42> = let %3
    %6:vec3<u32> = %v.Load3 48u
    %7:vec3<f32> = bitcast %6
    %b:vec3<f32> = let %7
    ret
  }
}
%4 = func(%offset:u32):array<vec3<f32>, 42> {
  $B3: {
    %a_1:ptr<function, array<vec3<f32>, 42>, read_write> = var array<vec3<f32>, 42>(vec3<f32>(0.0f))  # %a_1: 'a'
    loop [i: $B4, b: $B5, c: $B6] {  # loop_1
      $B4: {  # initializer
        next_iteration 0u  # -> $B5
      }
      $B5 (%idx:u32): {  # body
        %12:bool = gte %idx, 42u
        if %12 [t: $B7] {  # if_1
          $B7: {  # true
            exit_loop  # loop_1
          }
        }
        %13:ptr<function, vec3<f32>, read_write> = access %a_1, %idx
        %14:u32 = mul %idx, 16u
        %15:u32 = add %offset, %14
        %16:vec3<u32> = %v.Load3 %15
        %17:vec3<f32> = bitcast %16
        store %13, %17
        continue  # -> $B6
      }
      $B6: {  # continuing
        %18:u32 = add %idx, 1u
        next_iteration %18  # -> $B5
      }
    }
    %19:array<vec3<f32>, 42> = load %a_1
    ret %19
  }
}
)";
    Run(DecomposeStorageAccess);
    EXPECT_EQ(expect, str());
}

TEST_F(HlslWriterDecomposeStorageAccessTest, AccessStorageStruct) {
    auto* SB = ty.Struct(mod.symbols.New("SB"), {
                                                    {mod.symbols.New("a"), ty.i32()},
                                                    {mod.symbols.New("b"), ty.f32()},
                                                });

    auto* var = b.Var("v", storage, SB, core::Access::kRead);
    var->SetBindingPoint(0, 0);

    b.ir.root_block->Append(var);
    auto* func = b.Function("foo", ty.void_(), core::ir::Function::PipelineStage::kFragment);
    b.Append(func->Block(), [&] {
        b.Let("a", b.Load(var));
        b.Let("b", b.Load(b.Access(ty.ptr<storage, f32, core::Access::kRead>(), var, 1_u)));
        b.Return(func);
    });

    auto* src = R"(
SB = struct @align(4) {
  a:i32 @offset(0)
  b:f32 @offset(4)
}

$B1: {  # root
  %v:ptr<storage, SB, read> = var undef @binding_point(0, 0)
}

%foo = @fragment func():void {
  $B2: {
    %3:SB = load %v
    %a:SB = let %3
    %5:ptr<storage, f32, read> = access %v, 1u
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
  %v:hlsl.byte_address_buffer<read> = var undef @binding_point(0, 0)
}

%foo = @fragment func():void {
  $B2: {
    %3:SB = call %4, 0u
    %a:SB = let %3
    %6:u32 = %v.Load 4u
    %7:f32 = bitcast %6
    %b:f32 = let %7
    ret
  }
}
%4 = func(%offset:u32):SB {
  $B3: {
    %10:u32 = add %offset, 0u
    %11:u32 = %v.Load %10
    %12:i32 = bitcast %11
    %13:u32 = add %offset, 4u
    %14:u32 = %v.Load %13
    %15:f32 = bitcast %14
    %16:SB = construct %12, %15
    ret %16
  }
}
)";
    Run(DecomposeStorageAccess);
    EXPECT_EQ(expect, str());
}

TEST_F(HlslWriterDecomposeStorageAccessTest, AccessStorageNested) {
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

    auto* var = b.Var("v", storage, SB, core::Access::kRead);
    var->SetBindingPoint(0, 0);

    b.ir.root_block->Append(var);
    auto* func = b.Function("foo", ty.void_(), core::ir::Function::PipelineStage::kFragment);
    b.Append(func->Block(), [&] {
        b.Let("a", b.Load(var));
        b.Let("b", b.LoadVectorElement(b.Access(ty.ptr<storage, vec3<f32>, core::Access::kRead>(),
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
  %v:ptr<storage, SB, read> = var undef @binding_point(0, 0)
}

%foo = @fragment func():void {
  $B2: {
    %3:SB = load %v
    %a:SB = let %3
    %5:ptr<storage, vec3<f32>, read> = access %v, 1u, 1u, 1u, 3u
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
  %v:hlsl.byte_address_buffer<read> = var undef @binding_point(0, 0)
}

%foo = @fragment func():void {
  $B2: {
    %3:SB = call %4, 0u
    %a:SB = let %3
    %6:u32 = %v.Load 136u
    %7:f32 = bitcast %6
    %b:f32 = let %7
    ret
  }
}
%4 = func(%offset:u32):SB {
  $B3: {
    %10:u32 = add %offset, 0u
    %11:u32 = %v.Load %10
    %12:i32 = bitcast %11
    %13:u32 = add %offset, 16u
    %14:Outer = call %15, %13
    %16:SB = construct %12, %14
    ret %16
  }
}
%15 = func(%offset_1:u32):Outer {  # %offset_1: 'offset'
  $B4: {
    %18:u32 = add %offset_1, 0u
    %19:u32 = %v.Load %18
    %20:f32 = bitcast %19
    %21:u32 = add %offset_1, 16u
    %22:Inner = call %23, %21
    %24:Outer = construct %20, %22
    ret %24
  }
}
%23 = func(%offset_2:u32):Inner {  # %offset_2: 'offset'
  $B5: {
    %26:u32 = add %offset_2, 0u
    %27:mat3x3<f32> = call %28, %26
    %29:u32 = add %offset_2, 48u
    %30:array<vec3<f32>, 5> = call %31, %29
    %32:Inner = construct %27, %30
    ret %32
  }
}
%28 = func(%offset_3:u32):mat3x3<f32> {  # %offset_3: 'offset'
  $B6: {
    %34:u32 = add %offset_3, 0u
    %35:vec3<u32> = %v.Load3 %34
    %36:vec3<f32> = bitcast %35
    %37:u32 = add %offset_3, 16u
    %38:vec3<u32> = %v.Load3 %37
    %39:vec3<f32> = bitcast %38
    %40:u32 = add %offset_3, 32u
    %41:vec3<u32> = %v.Load3 %40
    %42:vec3<f32> = bitcast %41
    %43:mat3x3<f32> = construct %36, %39, %42
    ret %43
  }
}
%31 = func(%offset_4:u32):array<vec3<f32>, 5> {  # %offset_4: 'offset'
  $B7: {
    %a_1:ptr<function, array<vec3<f32>, 5>, read_write> = var array<vec3<f32>, 5>(vec3<f32>(0.0f))  # %a_1: 'a'
    loop [i: $B8, b: $B9, c: $B10] {  # loop_1
      $B8: {  # initializer
        next_iteration 0u  # -> $B9
      }
      $B9 (%idx:u32): {  # body
        %47:bool = gte %idx, 5u
        if %47 [t: $B11] {  # if_1
          $B11: {  # true
            exit_loop  # loop_1
          }
        }
        %48:ptr<function, vec3<f32>, read_write> = access %a_1, %idx
        %49:u32 = mul %idx, 16u
        %50:u32 = add %offset_4, %49
        %51:vec3<u32> = %v.Load3 %50
        %52:vec3<f32> = bitcast %51
        store %48, %52
        continue  # -> $B10
      }
      $B10: {  # continuing
        %53:u32 = add %idx, 1u
        next_iteration %53  # -> $B9
      }
    }
    %54:array<vec3<f32>, 5> = load %a_1
    ret %54
  }
}
)";
    Run(DecomposeStorageAccess);
    EXPECT_EQ(expect, str());
}

TEST_F(HlslWriterDecomposeStorageAccessTest, ComplexStaticAccessChain) {
    auto* S1 = ty.Struct(mod.symbols.New("S1"), {
                                                    {mod.symbols.New("a"), ty.i32()},
                                                    {mod.symbols.New("b"), ty.vec3<f32>()},
                                                    {mod.symbols.New("c"), ty.i32()},
                                                });
    auto* S2 = ty.Struct(mod.symbols.New("S2"), {
                                                    {mod.symbols.New("a"), ty.i32()},
                                                    {mod.symbols.New("b"), ty.array(S1, 3)},
                                                    {mod.symbols.New("c"), ty.i32()},
                                                });

    auto* SB = ty.Struct(mod.symbols.New("SB"), {
                                                    {mod.symbols.New("a"), ty.i32()},
                                                    {mod.symbols.New("b"), ty.runtime_array(S2)},
                                                });

    auto* var = b.Var("sb", storage, SB, core::Access::kReadWrite);
    var->SetBindingPoint(0, 0);

    b.ir.root_block->Append(var);
    auto* func = b.Function("foo", ty.void_(), core::ir::Function::PipelineStage::kFragment);
    b.Append(func->Block(), [&] {
        b.Let("x", b.LoadVectorElement(b.Access(ty.ptr<storage, vec3<f32>, read_write>(),
                                                // var.b[4].b[1].b.z
                                                //   .b   [4]  .b   [1] .b
                                                var, 1_u, 4_u, 1_u, 1_u, 1_u),
                                       2_u));
        b.Return(func);
    });

    auto* src = R"(
S1 = struct @align(16) {
  a:i32 @offset(0)
  b:vec3<f32> @offset(16)
  c:i32 @offset(28)
}

S2 = struct @align(16) {
  a_1:i32 @offset(0)
  b_1:array<S1, 3> @offset(16)
  c_1:i32 @offset(112)
}

SB = struct @align(16) {
  a_2:i32 @offset(0)
  b_2:array<S2> @offset(16)
}

$B1: {  # root
  %sb:ptr<storage, SB, read_write> = var undef @binding_point(0, 0)
}

%foo = @fragment func():void {
  $B2: {
    %3:ptr<storage, vec3<f32>, read_write> = access %sb, 1u, 4u, 1u, 1u, 1u
    %4:f32 = load_vector_element %3, 2u
    %x:f32 = let %4
    ret
  }
}
)";
    ASSERT_EQ(src, str());

    // sb.b[4].b[1].b.z
    //    ^  ^ ^  ^ ^ ^
    //    |  | |  | | |
    //   16  | |576 | 600
    //       | |    |
    //     528 544  592

    auto* expect = R"(
S1 = struct @align(16) {
  a:i32 @offset(0)
  b:vec3<f32> @offset(16)
  c:i32 @offset(28)
}

S2 = struct @align(16) {
  a_1:i32 @offset(0)
  b_1:array<S1, 3> @offset(16)
  c_1:i32 @offset(112)
}

SB = struct @align(16) {
  a_2:i32 @offset(0)
  b_2:array<S2> @offset(16)
}

$B1: {  # root
  %sb:hlsl.byte_address_buffer<read_write> = var undef @binding_point(0, 0)
}

%foo = @fragment func():void {
  $B2: {
    %3:u32 = %sb.Load 600u
    %4:f32 = bitcast %3
    %x:f32 = let %4
    ret
  }
}
)";

    Run(DecomposeStorageAccess);
    EXPECT_EQ(expect, str());
}

TEST_F(HlslWriterDecomposeStorageAccessTest, ComplexDynamicAccessChain) {
    auto* S1 = ty.Struct(mod.symbols.New("S1"), {
                                                    {mod.symbols.New("a"), ty.i32()},
                                                    {mod.symbols.New("b"), ty.vec3<f32>()},
                                                    {mod.symbols.New("c"), ty.i32()},
                                                });
    auto* S2 = ty.Struct(mod.symbols.New("S2"), {
                                                    {mod.symbols.New("a"), ty.i32()},
                                                    {mod.symbols.New("b"), ty.array(S1, 3)},
                                                    {mod.symbols.New("c"), ty.i32()},
                                                });

    auto* SB = ty.Struct(mod.symbols.New("SB"), {
                                                    {mod.symbols.New("a"), ty.i32()},
                                                    {mod.symbols.New("b"), ty.runtime_array(S2)},
                                                });

    auto* var = b.Var("sb", storage, SB, core::Access::kReadWrite);
    var->SetBindingPoint(0, 0);

    b.ir.root_block->Append(var);
    auto* func = b.Function("foo", ty.void_(), core::ir::Function::PipelineStage::kFragment);
    b.Append(func->Block(), [&] {
        auto* i = b.Load(b.Var("i", 4_i));
        auto* j = b.Load(b.Var("j", 1_u));
        auto* k = b.Load(b.Var("k", 2_i));
        // let x : f32 = sb.b[i].b[j].b[k];
        b.Let("x",
              b.LoadVectorElement(
                  b.Access(ty.ptr<storage, vec3<f32>, read_write>(), var, 1_u, i, 1_u, j, 1_u), k));
        b.Return(func);
    });

    auto* src = R"(
S1 = struct @align(16) {
  a:i32 @offset(0)
  b:vec3<f32> @offset(16)
  c:i32 @offset(28)
}

S2 = struct @align(16) {
  a_1:i32 @offset(0)
  b_1:array<S1, 3> @offset(16)
  c_1:i32 @offset(112)
}

SB = struct @align(16) {
  a_2:i32 @offset(0)
  b_2:array<S2> @offset(16)
}

$B1: {  # root
  %sb:ptr<storage, SB, read_write> = var undef @binding_point(0, 0)
}

%foo = @fragment func():void {
  $B2: {
    %i:ptr<function, i32, read_write> = var 4i
    %4:i32 = load %i
    %j:ptr<function, u32, read_write> = var 1u
    %6:u32 = load %j
    %k:ptr<function, i32, read_write> = var 2i
    %8:i32 = load %k
    %9:ptr<storage, vec3<f32>, read_write> = access %sb, 1u, %4, 1u, %6, 1u
    %10:f32 = load_vector_element %9, %8
    %x:f32 = let %10
    ret
  }
}
)";
    ASSERT_EQ(src, str());

    auto* expect = R"(
S1 = struct @align(16) {
  a:i32 @offset(0)
  b:vec3<f32> @offset(16)
  c:i32 @offset(28)
}

S2 = struct @align(16) {
  a_1:i32 @offset(0)
  b_1:array<S1, 3> @offset(16)
  c_1:i32 @offset(112)
}

SB = struct @align(16) {
  a_2:i32 @offset(0)
  b_2:array<S2> @offset(16)
}

$B1: {  # root
  %sb:hlsl.byte_address_buffer<read_write> = var undef @binding_point(0, 0)
}

%foo = @fragment func():void {
  $B2: {
    %i:ptr<function, i32, read_write> = var 4i
    %4:i32 = load %i
    %j:ptr<function, u32, read_write> = var 1u
    %6:u32 = load %j
    %k:ptr<function, i32, read_write> = var 2i
    %8:i32 = load %k
    %9:u32 = convert %4
    %10:u32 = mul %9, 128u
    %11:u32 = mul %6, 32u
    %12:u32 = convert %8
    %13:u32 = mul %12, 4u
    %14:u32 = add 48u, %10
    %15:u32 = add %14, %11
    %16:u32 = add %15, %13
    %17:u32 = %sb.Load %16
    %18:f32 = bitcast %17
    %x:f32 = let %18
    ret
  }
}
)";

    Run(DecomposeStorageAccess);
    EXPECT_EQ(expect, str());
}

TEST_F(HlslWriterDecomposeStorageAccessTest, ComplexDynamicAccessChainDynamicAccessInMiddle) {
    auto* S1 = ty.Struct(mod.symbols.New("S1"), {
                                                    {mod.symbols.New("a"), ty.i32()},
                                                    {mod.symbols.New("b"), ty.vec3<f32>()},
                                                    {mod.symbols.New("c"), ty.i32()},
                                                });
    auto* S2 = ty.Struct(mod.symbols.New("S2"), {
                                                    {mod.symbols.New("a"), ty.i32()},
                                                    {mod.symbols.New("b"), ty.array(S1, 3)},
                                                    {mod.symbols.New("c"), ty.i32()},
                                                });

    auto* SB = ty.Struct(mod.symbols.New("SB"), {
                                                    {mod.symbols.New("a"), ty.i32()},
                                                    {mod.symbols.New("b"), ty.runtime_array(S2)},
                                                });

    auto* var = b.Var("sb", storage, SB, core::Access::kReadWrite);
    var->SetBindingPoint(0, 0);

    b.ir.root_block->Append(var);
    auto* func = b.Function("foo", ty.void_(), core::ir::Function::PipelineStage::kFragment);
    b.Append(func->Block(), [&] {
        auto* j = b.Load(b.Var("j", 1_u));
        // let x : f32 = sb.b[4].b[j].b[2];
        b.Let("x", b.LoadVectorElement(b.Access(ty.ptr<storage, vec3<f32>, read_write>(), var, 1_u,
                                                4_u, 1_u, j, 1_u),
                                       2_u));
        b.Return(func);
    });

    auto* src = R"(
S1 = struct @align(16) {
  a:i32 @offset(0)
  b:vec3<f32> @offset(16)
  c:i32 @offset(28)
}

S2 = struct @align(16) {
  a_1:i32 @offset(0)
  b_1:array<S1, 3> @offset(16)
  c_1:i32 @offset(112)
}

SB = struct @align(16) {
  a_2:i32 @offset(0)
  b_2:array<S2> @offset(16)
}

$B1: {  # root
  %sb:ptr<storage, SB, read_write> = var undef @binding_point(0, 0)
}

%foo = @fragment func():void {
  $B2: {
    %j:ptr<function, u32, read_write> = var 1u
    %4:u32 = load %j
    %5:ptr<storage, vec3<f32>, read_write> = access %sb, 1u, 4u, 1u, %4, 1u
    %6:f32 = load_vector_element %5, 2u
    %x:f32 = let %6
    ret
  }
}
)";
    ASSERT_EQ(src, str());

    auto* expect = R"(
S1 = struct @align(16) {
  a:i32 @offset(0)
  b:vec3<f32> @offset(16)
  c:i32 @offset(28)
}

S2 = struct @align(16) {
  a_1:i32 @offset(0)
  b_1:array<S1, 3> @offset(16)
  c_1:i32 @offset(112)
}

SB = struct @align(16) {
  a_2:i32 @offset(0)
  b_2:array<S2> @offset(16)
}

$B1: {  # root
  %sb:hlsl.byte_address_buffer<read_write> = var undef @binding_point(0, 0)
}

%foo = @fragment func():void {
  $B2: {
    %j:ptr<function, u32, read_write> = var 1u
    %4:u32 = load %j
    %5:u32 = mul %4, 32u
    %6:u32 = add 568u, %5
    %7:u32 = %sb.Load %6
    %8:f32 = bitcast %7
    %x:f32 = let %8
    ret
  }
}
)";

    Run(DecomposeStorageAccess);
    EXPECT_EQ(expect, str());
}

TEST_F(HlslWriterDecomposeStorageAccessTest, StorageAtomicStore) {
    auto* sb = ty.Struct(mod.symbols.New("SB"), {
                                                    {mod.symbols.New("padding"), ty.vec4<f32>()},
                                                    {mod.symbols.New("a"), ty.atomic<i32>()},
                                                    {mod.symbols.New("b"), ty.atomic<u32>()},
                                                });

    auto* var = b.Var("v", storage, sb, core::Access::kReadWrite);
    var->SetBindingPoint(0, 0);
    b.ir.root_block->Append(var);

    auto* func = b.Function("foo", ty.void_(), core::ir::Function::PipelineStage::kFragment);
    b.Append(func->Block(), [&] {
        b.Call(ty.void_(), core::BuiltinFn::kAtomicStore,
               b.Access(ty.ptr<storage, atomic<i32>, read_write>(), var, 1_u), 123_i);
        b.Return(func);
    });

    auto* src = R"(
SB = struct @align(16) {
  padding:vec4<f32> @offset(0)
  a:atomic<i32> @offset(16)
  b:atomic<u32> @offset(20)
}

$B1: {  # root
  %v:ptr<storage, SB, read_write> = var undef @binding_point(0, 0)
}

%foo = @fragment func():void {
  $B2: {
    %3:ptr<storage, atomic<i32>, read_write> = access %v, 1u
    %4:void = atomicStore %3, 123i
    ret
  }
}
)";
    EXPECT_EQ(src, str());

    auto* expect = R"(
SB = struct @align(16) {
  padding:vec4<f32> @offset(0)
  a:atomic<i32> @offset(16)
  b:atomic<u32> @offset(20)
}

$B1: {  # root
  %v:hlsl.byte_address_buffer<read_write> = var undef @binding_point(0, 0)
}

%foo = @fragment func():void {
  $B2: {
    %3:ptr<function, i32, read_write> = var 0i
    %4:i32 = convert 16u
    %5:void = %v.InterlockedExchange %4, 123i, %3
    ret
  }
}
)";
    Run(DecomposeStorageAccess);

    EXPECT_EQ(expect, str());
}

TEST_F(HlslWriterDecomposeStorageAccessTest, StorageAtomicStoreDynamicAccessChain) {
    auto* S1 =
        ty.Struct(mod.symbols.New("SB"), {
                                             {mod.symbols.New("padding"), ty.vec4<f32>()},
                                             {mod.symbols.New("a"), ty.array(ty.atomic<i32>(), 3)},
                                         });
    auto* S2 = ty.Struct(mod.symbols.New("S2"), {
                                                    {mod.symbols.New("arr_s1"), ty.array(S1, 3)},
                                                });

    auto* var = b.Var("v", storage, S2, core::Access::kReadWrite);
    var->SetBindingPoint(0, 0);
    b.ir.root_block->Append(var);

    auto* func = b.Function("foo", ty.void_(), core::ir::Function::PipelineStage::kFragment);
    core::IOAttributes index_attr;
    index_attr.location = 0;
    auto index = b.FunctionParam(ty.u32());
    index->SetAttributes(index_attr);
    func->SetParams({index});
    b.Append(func->Block(), [&] {
        auto* access = b.Access(ty.ptr<storage>(ty.atomic<i32>()), var, 0_u, index, 1_u, index);
        b.Call(ty.void_(), core::BuiltinFn::kAtomicStore, access, 123_i);
        b.Return(func);
    });

    auto* src = R"(
SB = struct @align(16) {
  padding:vec4<f32> @offset(0)
  a:array<atomic<i32>, 3> @offset(16)
}

S2 = struct @align(16) {
  arr_s1:array<SB, 3> @offset(0)
}

$B1: {  # root
  %v:ptr<storage, S2, read_write> = var undef @binding_point(0, 0)
}

%foo = @fragment func(%3:u32 [@location(0)]):void {
  $B2: {
    %4:ptr<storage, atomic<i32>, read_write> = access %v, 0u, %3, 1u, %3
    %5:void = atomicStore %4, 123i
    ret
  }
}
)";
    EXPECT_EQ(src, str());

    auto* expect = R"(
SB = struct @align(16) {
  padding:vec4<f32> @offset(0)
  a:array<atomic<i32>, 3> @offset(16)
}

S2 = struct @align(16) {
  arr_s1:array<SB, 3> @offset(0)
}

$B1: {  # root
  %v:hlsl.byte_address_buffer<read_write> = var undef @binding_point(0, 0)
}

%foo = @fragment func(%3:u32 [@location(0)]):void {
  $B2: {
    %4:u32 = mul %3, 32u
    %5:u32 = mul %3, 4u
    %6:ptr<function, i32, read_write> = var 0i
    %7:u32 = add 16u, %4
    %8:u32 = add %7, %5
    %9:i32 = convert %8
    %10:void = %v.InterlockedExchange %9, 123i, %6
    ret
  }
}
)";
    Run(DecomposeStorageAccess);

    EXPECT_EQ(expect, str());
}

TEST_F(HlslWriterDecomposeStorageAccessTest, StorageAtomicStoreDirect) {
    auto* var = b.Var("v", storage, ty.atomic<i32>(), core::Access::kReadWrite);
    var->SetBindingPoint(0, 0);
    b.ir.root_block->Append(var);

    auto* func = b.Function("foo", ty.void_(), core::ir::Function::PipelineStage::kFragment);
    b.Append(func->Block(), [&] {
        b.Call(ty.void_(), core::BuiltinFn::kAtomicStore, var, 123_i);
        b.Return(func);
    });

    auto* src = R"(
$B1: {  # root
  %v:ptr<storage, atomic<i32>, read_write> = var undef @binding_point(0, 0)
}

%foo = @fragment func():void {
  $B2: {
    %3:void = atomicStore %v, 123i
    ret
  }
}
)";
    EXPECT_EQ(src, str());

    auto* expect = R"(
$B1: {  # root
  %v:hlsl.byte_address_buffer<read_write> = var undef @binding_point(0, 0)
}

%foo = @fragment func():void {
  $B2: {
    %3:ptr<function, i32, read_write> = var 0i
    %4:i32 = convert 0u
    %5:void = %v.InterlockedExchange %4, 123i, %3
    ret
  }
}
)";
    Run(DecomposeStorageAccess);

    EXPECT_EQ(expect, str());
}

TEST_F(HlslWriterDecomposeStorageAccessTest, StorageAtomicLoad) {
    auto* sb = ty.Struct(mod.symbols.New("SB"), {
                                                    {mod.symbols.New("padding"), ty.vec4<f32>()},
                                                    {mod.symbols.New("a"), ty.atomic<i32>()},
                                                    {mod.symbols.New("b"), ty.atomic<u32>()},
                                                });

    auto* var = b.Var("v", storage, sb, core::Access::kReadWrite);
    var->SetBindingPoint(0, 0);
    b.ir.root_block->Append(var);

    auto* func = b.Function("foo", ty.void_(), core::ir::Function::PipelineStage::kFragment);
    b.Append(func->Block(), [&] {
        b.Let("x", b.Call(ty.i32(), core::BuiltinFn::kAtomicLoad,
                          b.Access(ty.ptr<storage, atomic<i32>, read_write>(), var, 1_u)));
        b.Return(func);
    });

    auto* src = R"(
SB = struct @align(16) {
  padding:vec4<f32> @offset(0)
  a:atomic<i32> @offset(16)
  b:atomic<u32> @offset(20)
}

$B1: {  # root
  %v:ptr<storage, SB, read_write> = var undef @binding_point(0, 0)
}

%foo = @fragment func():void {
  $B2: {
    %3:ptr<storage, atomic<i32>, read_write> = access %v, 1u
    %4:i32 = atomicLoad %3
    %x:i32 = let %4
    ret
  }
}
)";
    EXPECT_EQ(src, str());

    auto* expect = R"(
SB = struct @align(16) {
  padding:vec4<f32> @offset(0)
  a:atomic<i32> @offset(16)
  b:atomic<u32> @offset(20)
}

$B1: {  # root
  %v:hlsl.byte_address_buffer<read_write> = var undef @binding_point(0, 0)
}

%foo = @fragment func():void {
  $B2: {
    %3:ptr<function, i32, read_write> = var 0i
    %4:i32 = convert 16u
    %5:void = %v.InterlockedOr %4, 0i, %3
    %6:i32 = load %3
    %x:i32 = let %6
    ret
  }
}
)";
    Run(DecomposeStorageAccess);

    EXPECT_EQ(expect, str());
}

TEST_F(HlslWriterDecomposeStorageAccessTest, StorageAtomicLoadDynamicAccessChain) {
    auto* S1 =
        ty.Struct(mod.symbols.New("SB"), {
                                             {mod.symbols.New("padding"), ty.vec4<f32>()},
                                             {mod.symbols.New("a"), ty.array(ty.atomic<i32>(), 3)},
                                         });
    auto* S2 = ty.Struct(mod.symbols.New("S2"), {
                                                    {mod.symbols.New("arr_s1"), ty.array(S1, 3)},
                                                });

    auto* var = b.Var("v", storage, S2, core::Access::kReadWrite);
    var->SetBindingPoint(0, 0);
    b.ir.root_block->Append(var);

    auto* func = b.Function("foo", ty.void_(), core::ir::Function::PipelineStage::kFragment);
    core::IOAttributes index_attr;
    index_attr.location = 0;
    auto index = b.FunctionParam(ty.u32());
    index->SetAttributes(index_attr);
    func->SetParams({index});
    b.Append(func->Block(), [&] {
        auto* access = b.Access(ty.ptr<storage>(ty.atomic<i32>()), var, 0_u, index, 1_u, index);
        b.Let("x", b.Call(ty.i32(), core::BuiltinFn::kAtomicLoad, access));
        b.Return(func);
    });

    auto* src = R"(
SB = struct @align(16) {
  padding:vec4<f32> @offset(0)
  a:array<atomic<i32>, 3> @offset(16)
}

S2 = struct @align(16) {
  arr_s1:array<SB, 3> @offset(0)
}

$B1: {  # root
  %v:ptr<storage, S2, read_write> = var undef @binding_point(0, 0)
}

%foo = @fragment func(%3:u32 [@location(0)]):void {
  $B2: {
    %4:ptr<storage, atomic<i32>, read_write> = access %v, 0u, %3, 1u, %3
    %5:i32 = atomicLoad %4
    %x:i32 = let %5
    ret
  }
}
)";
    EXPECT_EQ(src, str());

    auto* expect = R"(
SB = struct @align(16) {
  padding:vec4<f32> @offset(0)
  a:array<atomic<i32>, 3> @offset(16)
}

S2 = struct @align(16) {
  arr_s1:array<SB, 3> @offset(0)
}

$B1: {  # root
  %v:hlsl.byte_address_buffer<read_write> = var undef @binding_point(0, 0)
}

%foo = @fragment func(%3:u32 [@location(0)]):void {
  $B2: {
    %4:u32 = mul %3, 32u
    %5:u32 = mul %3, 4u
    %6:ptr<function, i32, read_write> = var 0i
    %7:u32 = add 16u, %4
    %8:u32 = add %7, %5
    %9:i32 = convert %8
    %10:void = %v.InterlockedOr %9, 0i, %6
    %11:i32 = load %6
    %x:i32 = let %11
    ret
  }
}
)";
    Run(DecomposeStorageAccess);

    EXPECT_EQ(expect, str());
}

TEST_F(HlslWriterDecomposeStorageAccessTest, StorageAtomicLoadDirect) {
    auto* var = b.Var("v", storage, ty.atomic<i32>(), core::Access::kReadWrite);
    var->SetBindingPoint(0, 0);
    b.ir.root_block->Append(var);

    auto* func = b.Function("foo", ty.void_(), core::ir::Function::PipelineStage::kFragment);
    b.Append(func->Block(), [&] {
        b.Let("x", b.Call(ty.i32(), core::BuiltinFn::kAtomicLoad, var));
        b.Return(func);
    });

    auto* src = R"(
$B1: {  # root
  %v:ptr<storage, atomic<i32>, read_write> = var undef @binding_point(0, 0)
}

%foo = @fragment func():void {
  $B2: {
    %3:i32 = atomicLoad %v
    %x:i32 = let %3
    ret
  }
}
)";
    EXPECT_EQ(src, str());

    auto* expect = R"(
$B1: {  # root
  %v:hlsl.byte_address_buffer<read_write> = var undef @binding_point(0, 0)
}

%foo = @fragment func():void {
  $B2: {
    %3:ptr<function, i32, read_write> = var 0i
    %4:i32 = convert 0u
    %5:void = %v.InterlockedOr %4, 0i, %3
    %6:i32 = load %3
    %x:i32 = let %6
    ret
  }
}
)";
    Run(DecomposeStorageAccess);

    EXPECT_EQ(expect, str());
}

TEST_F(HlslWriterDecomposeStorageAccessTest, StorageAtomicSub) {
    auto* sb = ty.Struct(mod.symbols.New("SB"), {
                                                    {mod.symbols.New("padding"), ty.vec4<f32>()},
                                                    {mod.symbols.New("a"), ty.atomic<i32>()},
                                                    {mod.symbols.New("b"), ty.atomic<u32>()},
                                                });

    auto* var = b.Var("v", storage, sb, core::Access::kReadWrite);
    var->SetBindingPoint(0, 0);
    b.ir.root_block->Append(var);

    auto* func = b.Function("foo", ty.void_(), core::ir::Function::PipelineStage::kFragment);
    b.Append(func->Block(), [&] {
        b.Let("x", b.Call(ty.i32(), core::BuiltinFn::kAtomicSub,
                          b.Access(ty.ptr<storage, atomic<i32>, read_write>(), var, 1_u), 123_i));
        b.Return(func);
    });

    auto* src = R"(
SB = struct @align(16) {
  padding:vec4<f32> @offset(0)
  a:atomic<i32> @offset(16)
  b:atomic<u32> @offset(20)
}

$B1: {  # root
  %v:ptr<storage, SB, read_write> = var undef @binding_point(0, 0)
}

%foo = @fragment func():void {
  $B2: {
    %3:ptr<storage, atomic<i32>, read_write> = access %v, 1u
    %4:i32 = atomicSub %3, 123i
    %x:i32 = let %4
    ret
  }
}
)";
    EXPECT_EQ(src, str());

    auto* expect = R"(
SB = struct @align(16) {
  padding:vec4<f32> @offset(0)
  a:atomic<i32> @offset(16)
  b:atomic<u32> @offset(20)
}

$B1: {  # root
  %v:hlsl.byte_address_buffer<read_write> = var undef @binding_point(0, 0)
}

%foo = @fragment func():void {
  $B2: {
    %3:ptr<function, i32, read_write> = var 0i
    %4:i32 = sub 0i, 123i
    %5:i32 = convert 16u
    %6:void = %v.InterlockedAdd %5, %4, %3
    %7:i32 = load %3
    %x:i32 = let %7
    ret
  }
}
)";
    Run(DecomposeStorageAccess);

    EXPECT_EQ(expect, str());
}

TEST_F(HlslWriterDecomposeStorageAccessTest, StorageAtomicSubDynamicAccessChain) {
    auto* S1 =
        ty.Struct(mod.symbols.New("SB"), {
                                             {mod.symbols.New("padding"), ty.vec4<f32>()},
                                             {mod.symbols.New("a"), ty.array(ty.atomic<i32>(), 3)},
                                         });
    auto* S2 = ty.Struct(mod.symbols.New("S2"), {
                                                    {mod.symbols.New("arr_s1"), ty.array(S1, 3)},
                                                });

    auto* var = b.Var("v", storage, S2, core::Access::kReadWrite);
    var->SetBindingPoint(0, 0);
    b.ir.root_block->Append(var);

    auto* func = b.Function("foo", ty.void_(), core::ir::Function::PipelineStage::kFragment);
    core::IOAttributes index_attr;
    index_attr.location = 0;
    auto index = b.FunctionParam(ty.u32());
    index->SetAttributes(index_attr);
    func->SetParams({index});
    b.Append(func->Block(), [&] {
        auto* access = b.Access(ty.ptr<storage>(ty.atomic<i32>()), var, 0_u, index, 1_u, index);
        b.Let("x", b.Call(ty.i32(), core::BuiltinFn::kAtomicSub, access, 123_i));
        b.Return(func);
    });

    auto* src = R"(
SB = struct @align(16) {
  padding:vec4<f32> @offset(0)
  a:array<atomic<i32>, 3> @offset(16)
}

S2 = struct @align(16) {
  arr_s1:array<SB, 3> @offset(0)
}

$B1: {  # root
  %v:ptr<storage, S2, read_write> = var undef @binding_point(0, 0)
}

%foo = @fragment func(%3:u32 [@location(0)]):void {
  $B2: {
    %4:ptr<storage, atomic<i32>, read_write> = access %v, 0u, %3, 1u, %3
    %5:i32 = atomicSub %4, 123i
    %x:i32 = let %5
    ret
  }
}
)";
    EXPECT_EQ(src, str());

    auto* expect = R"(
SB = struct @align(16) {
  padding:vec4<f32> @offset(0)
  a:array<atomic<i32>, 3> @offset(16)
}

S2 = struct @align(16) {
  arr_s1:array<SB, 3> @offset(0)
}

$B1: {  # root
  %v:hlsl.byte_address_buffer<read_write> = var undef @binding_point(0, 0)
}

%foo = @fragment func(%3:u32 [@location(0)]):void {
  $B2: {
    %4:u32 = mul %3, 32u
    %5:u32 = mul %3, 4u
    %6:ptr<function, i32, read_write> = var 0i
    %7:i32 = sub 0i, 123i
    %8:u32 = add 16u, %4
    %9:u32 = add %8, %5
    %10:i32 = convert %9
    %11:void = %v.InterlockedAdd %10, %7, %6
    %12:i32 = load %6
    %x:i32 = let %12
    ret
  }
}
)";
    Run(DecomposeStorageAccess);

    EXPECT_EQ(expect, str());
}

TEST_F(HlslWriterDecomposeStorageAccessTest, StorageAtomicSubDirect) {
    auto* var = b.Var("v", storage, ty.atomic<i32>(), core::Access::kReadWrite);
    var->SetBindingPoint(0, 0);
    b.ir.root_block->Append(var);

    auto* func = b.Function("foo", ty.void_(), core::ir::Function::PipelineStage::kFragment);
    b.Append(func->Block(), [&] {
        b.Let("x", b.Call(ty.i32(), core::BuiltinFn::kAtomicSub, var, 123_i));
        b.Return(func);
    });

    auto* src = R"(
$B1: {  # root
  %v:ptr<storage, atomic<i32>, read_write> = var undef @binding_point(0, 0)
}

%foo = @fragment func():void {
  $B2: {
    %3:i32 = atomicSub %v, 123i
    %x:i32 = let %3
    ret
  }
}
)";
    EXPECT_EQ(src, str());

    auto* expect = R"(
$B1: {  # root
  %v:hlsl.byte_address_buffer<read_write> = var undef @binding_point(0, 0)
}

%foo = @fragment func():void {
  $B2: {
    %3:ptr<function, i32, read_write> = var 0i
    %4:i32 = sub 0i, 123i
    %5:i32 = convert 0u
    %6:void = %v.InterlockedAdd %5, %4, %3
    %7:i32 = load %3
    %x:i32 = let %7
    ret
  }
}
)";
    Run(DecomposeStorageAccess);

    EXPECT_EQ(expect, str());
}

TEST_F(HlslWriterDecomposeStorageAccessTest, StorageAtomicCompareExchangeWeak) {
    auto* sb = ty.Struct(mod.symbols.New("SB"), {
                                                    {mod.symbols.New("padding"), ty.vec4<f32>()},
                                                    {mod.symbols.New("a"), ty.atomic<i32>()},
                                                    {mod.symbols.New("b"), ty.atomic<u32>()},
                                                });

    auto* var = b.Var("v", storage, sb, core::Access::kReadWrite);
    var->SetBindingPoint(0, 0);
    b.ir.root_block->Append(var);

    auto* func = b.Function("foo", ty.void_(), core::ir::Function::PipelineStage::kFragment);
    b.Append(func->Block(), [&] {
        b.Let("x",
              b.Call(core::type::CreateAtomicCompareExchangeResult(ty, mod.symbols, ty.i32()),
                     core::BuiltinFn::kAtomicCompareExchangeWeak,
                     b.Access(ty.ptr<storage, atomic<i32>, read_write>(), var, 1_u), 123_i, 345_i));
        b.Return(func);
    });

    auto* src = R"(
SB = struct @align(16) {
  padding:vec4<f32> @offset(0)
  a:atomic<i32> @offset(16)
  b:atomic<u32> @offset(20)
}

__atomic_compare_exchange_result_i32 = struct @align(4) {
  old_value:i32 @offset(0)
  exchanged:bool @offset(4)
}

$B1: {  # root
  %v:ptr<storage, SB, read_write> = var undef @binding_point(0, 0)
}

%foo = @fragment func():void {
  $B2: {
    %3:ptr<storage, atomic<i32>, read_write> = access %v, 1u
    %4:__atomic_compare_exchange_result_i32 = atomicCompareExchangeWeak %3, 123i, 345i
    %x:__atomic_compare_exchange_result_i32 = let %4
    ret
  }
}
)";
    EXPECT_EQ(src, str());

    auto* expect = R"(
SB = struct @align(16) {
  padding:vec4<f32> @offset(0)
  a:atomic<i32> @offset(16)
  b:atomic<u32> @offset(20)
}

__atomic_compare_exchange_result_i32 = struct @align(4) {
  old_value:i32 @offset(0)
  exchanged:bool @offset(4)
}

$B1: {  # root
  %v:hlsl.byte_address_buffer<read_write> = var undef @binding_point(0, 0)
}

%foo = @fragment func():void {
  $B2: {
    %3:ptr<function, i32, read_write> = var 0i
    %4:i32 = convert 16u
    %5:void = %v.InterlockedCompareExchange %4, 123i, 345i, %3
    %6:i32 = load %3
    %7:bool = eq %6, 123i
    %8:__atomic_compare_exchange_result_i32 = construct %6, %7
    %x:__atomic_compare_exchange_result_i32 = let %8
    ret
  }
}
)";
    Run(DecomposeStorageAccess);

    EXPECT_EQ(expect, str());
}

TEST_F(HlslWriterDecomposeStorageAccessTest, StorageAtomicCompareExchangeWeakDynamicAccessChain) {
    auto* S1 =
        ty.Struct(mod.symbols.New("SB"), {
                                             {mod.symbols.New("padding"), ty.vec4<f32>()},
                                             {mod.symbols.New("a"), ty.array(ty.atomic<i32>(), 3)},
                                         });
    auto* S2 = ty.Struct(mod.symbols.New("S2"), {
                                                    {mod.symbols.New("arr_s1"), ty.array(S1, 3)},
                                                });

    auto* var = b.Var("v", storage, S2, core::Access::kReadWrite);
    var->SetBindingPoint(0, 0);
    b.ir.root_block->Append(var);

    auto* func = b.Function("foo", ty.void_(), core::ir::Function::PipelineStage::kFragment);
    core::IOAttributes index_attr;
    index_attr.location = 0;
    auto index = b.FunctionParam(ty.u32());
    index->SetAttributes(index_attr);
    func->SetParams({index});
    b.Append(func->Block(), [&] {
        auto* access = b.Access(ty.ptr<storage>(ty.atomic<i32>()), var, 0_u, index, 1_u, index);
        b.Let("x", b.Call(core::type::CreateAtomicCompareExchangeResult(ty, mod.symbols, ty.i32()),
                          core::BuiltinFn::kAtomicCompareExchangeWeak, access, 123_i, 345_i));
        b.Return(func);
    });

    auto* src = R"(
SB = struct @align(16) {
  padding:vec4<f32> @offset(0)
  a:array<atomic<i32>, 3> @offset(16)
}

S2 = struct @align(16) {
  arr_s1:array<SB, 3> @offset(0)
}

__atomic_compare_exchange_result_i32 = struct @align(4) {
  old_value:i32 @offset(0)
  exchanged:bool @offset(4)
}

$B1: {  # root
  %v:ptr<storage, S2, read_write> = var undef @binding_point(0, 0)
}

%foo = @fragment func(%3:u32 [@location(0)]):void {
  $B2: {
    %4:ptr<storage, atomic<i32>, read_write> = access %v, 0u, %3, 1u, %3
    %5:__atomic_compare_exchange_result_i32 = atomicCompareExchangeWeak %4, 123i, 345i
    %x:__atomic_compare_exchange_result_i32 = let %5
    ret
  }
}
)";
    EXPECT_EQ(src, str());

    auto* expect = R"(
SB = struct @align(16) {
  padding:vec4<f32> @offset(0)
  a:array<atomic<i32>, 3> @offset(16)
}

S2 = struct @align(16) {
  arr_s1:array<SB, 3> @offset(0)
}

__atomic_compare_exchange_result_i32 = struct @align(4) {
  old_value:i32 @offset(0)
  exchanged:bool @offset(4)
}

$B1: {  # root
  %v:hlsl.byte_address_buffer<read_write> = var undef @binding_point(0, 0)
}

%foo = @fragment func(%3:u32 [@location(0)]):void {
  $B2: {
    %4:u32 = mul %3, 32u
    %5:u32 = mul %3, 4u
    %6:ptr<function, i32, read_write> = var 0i
    %7:u32 = add 16u, %4
    %8:u32 = add %7, %5
    %9:i32 = convert %8
    %10:void = %v.InterlockedCompareExchange %9, 123i, 345i, %6
    %11:i32 = load %6
    %12:bool = eq %11, 123i
    %13:__atomic_compare_exchange_result_i32 = construct %11, %12
    %x:__atomic_compare_exchange_result_i32 = let %13
    ret
  }
}
)";
    Run(DecomposeStorageAccess);

    EXPECT_EQ(expect, str());
}

TEST_F(HlslWriterDecomposeStorageAccessTest, StorageAtomicCompareExchangeWeakDirect) {
    auto* var = b.Var("v", storage, ty.atomic<i32>(), core::Access::kReadWrite);
    var->SetBindingPoint(0, 0);
    b.ir.root_block->Append(var);

    auto* func = b.Function("foo", ty.void_(), core::ir::Function::PipelineStage::kFragment);
    b.Append(func->Block(), [&] {
        b.Let("x", b.Call(core::type::CreateAtomicCompareExchangeResult(ty, mod.symbols, ty.i32()),
                          core::BuiltinFn::kAtomicCompareExchangeWeak, var, 123_i, 345_i));
        b.Return(func);
    });

    auto* src = R"(
__atomic_compare_exchange_result_i32 = struct @align(4) {
  old_value:i32 @offset(0)
  exchanged:bool @offset(4)
}

$B1: {  # root
  %v:ptr<storage, atomic<i32>, read_write> = var undef @binding_point(0, 0)
}

%foo = @fragment func():void {
  $B2: {
    %3:__atomic_compare_exchange_result_i32 = atomicCompareExchangeWeak %v, 123i, 345i
    %x:__atomic_compare_exchange_result_i32 = let %3
    ret
  }
}
)";
    EXPECT_EQ(src, str());

    auto* expect = R"(
__atomic_compare_exchange_result_i32 = struct @align(4) {
  old_value:i32 @offset(0)
  exchanged:bool @offset(4)
}

$B1: {  # root
  %v:hlsl.byte_address_buffer<read_write> = var undef @binding_point(0, 0)
}

%foo = @fragment func():void {
  $B2: {
    %3:ptr<function, i32, read_write> = var 0i
    %4:i32 = convert 0u
    %5:void = %v.InterlockedCompareExchange %4, 123i, 345i, %3
    %6:i32 = load %3
    %7:bool = eq %6, 123i
    %8:__atomic_compare_exchange_result_i32 = construct %6, %7
    %x:__atomic_compare_exchange_result_i32 = let %8
    ret
  }
}
)";
    Run(DecomposeStorageAccess);

    EXPECT_EQ(expect, str());
}

struct AtomicData {
    core::BuiltinFn fn;
    const char* atomic_name;
    const char* interlock;
};
[[maybe_unused]] std::ostream& operator<<(std::ostream& out, const AtomicData& data) {
    out << data.interlock;
    return out;
}
using DecomposeBuiltinAtomic = core::ir::transform::TransformTestWithParam<AtomicData>;
TEST_P(DecomposeBuiltinAtomic, IndirectAccess) {
    auto params = GetParam();

    auto* sb = ty.Struct(mod.symbols.New("SB"), {
                                                    {mod.symbols.New("padding"), ty.vec4<f32>()},
                                                    {mod.symbols.New("a"), ty.atomic<i32>()},
                                                    {mod.symbols.New("b"), ty.atomic<u32>()},
                                                });

    auto* var = b.Var("v", storage, sb, core::Access::kReadWrite);
    var->SetBindingPoint(0, 0);
    b.ir.root_block->Append(var);

    auto* func = b.Function("foo", ty.void_(), core::ir::Function::PipelineStage::kFragment);
    b.Append(func->Block(), [&] {
        b.Let("x", b.Call(ty.i32(), params.fn,
                          b.Access(ty.ptr<storage, atomic<i32>, read_write>(), var, 1_u), 123_i));
        b.Return(func);
    });

    auto src = R"(
SB = struct @align(16) {
  padding:vec4<f32> @offset(0)
  a:atomic<i32> @offset(16)
  b:atomic<u32> @offset(20)
}

$B1: {  # root
  %v:ptr<storage, SB, read_write> = var undef @binding_point(0, 0)
}

%foo = @fragment func():void {
  $B2: {
    %3:ptr<storage, atomic<i32>, read_write> = access %v, 1u
    %4:i32 = )" +
               std::string(params.atomic_name) + R"( %3, 123i
    %x:i32 = let %4
    ret
  }
}
)";
    EXPECT_EQ(src, str());

    auto expect = R"(
SB = struct @align(16) {
  padding:vec4<f32> @offset(0)
  a:atomic<i32> @offset(16)
  b:atomic<u32> @offset(20)
}

$B1: {  # root
  %v:hlsl.byte_address_buffer<read_write> = var undef @binding_point(0, 0)
}

%foo = @fragment func():void {
  $B2: {
    %3:ptr<function, i32, read_write> = var 0i
    %4:i32 = convert 16u
    %5:void = %v.)" +
                  std::string(params.interlock) + R"( %4, 123i, %3
    %6:i32 = load %3
    %x:i32 = let %6
    ret
  }
}
)";
    Run(DecomposeStorageAccess);

    EXPECT_EQ(expect, str());
}

TEST_P(DecomposeBuiltinAtomic, DirectAccess) {
    auto param = GetParam();

    auto* var = b.Var("v", storage, ty.atomic<u32>(), core::Access::kReadWrite);
    var->SetBindingPoint(0, 0);
    b.ir.root_block->Append(var);

    auto* func = b.Function("foo", ty.void_(), core::ir::Function::PipelineStage::kFragment);
    b.Append(func->Block(), [&] {
        b.Let("x", b.Call(ty.u32(), param.fn, var, 123_u));
        b.Return(func);
    });

    auto src = R"(
$B1: {  # root
  %v:ptr<storage, atomic<u32>, read_write> = var undef @binding_point(0, 0)
}

%foo = @fragment func():void {
  $B2: {
    %3:u32 = )" +
               std::string(param.atomic_name) + R"( %v, 123u
    %x:u32 = let %3
    ret
  }
}
)";
    EXPECT_EQ(src, str());

    auto expect = R"(
$B1: {  # root
  %v:hlsl.byte_address_buffer<read_write> = var undef @binding_point(0, 0)
}

%foo = @fragment func():void {
  $B2: {
    %3:ptr<function, u32, read_write> = var 0u
    %4:void = %v.)" +
                  std::string(param.interlock) + R"( 0u, 123u, %3
    %5:u32 = load %3
    %x:u32 = let %5
    ret
  }
}
)";
    Run(DecomposeStorageAccess);

    EXPECT_EQ(expect, str());
}

TEST_P(DecomposeBuiltinAtomic, DynamicAccessChain) {
    auto param = GetParam();

    auto* S1 = ty.Struct(mod.symbols.New("S1"),
                         {
                             {mod.symbols.New("arr_u32"), ty.array(ty.atomic<u32>(), 3)},
                         });
    auto* S2 = ty.Struct(mod.symbols.New("S2"), {
                                                    {mod.symbols.New("arr_s1"), ty.array(S1, 3)},
                                                });

    auto* var = b.Var("v", storage, S2, core::Access::kReadWrite);
    var->SetBindingPoint(0, 0);
    b.ir.root_block->Append(var);

    auto* func = b.Function("foo", ty.void_(), core::ir::Function::PipelineStage::kFragment);
    auto index = b.FunctionParam(ty.u32());
    index->SetLocation(0);
    func->SetParams({index});
    b.Append(func->Block(), [&] {
        auto* access = b.Access(ty.ptr<storage>(ty.atomic<u32>()), var, 0_u, index, 0_u, index);
        b.Let("x", b.Call(ty.u32(), param.fn, access, 123_u));
        b.Return(func);
    });

    auto src = R"(
S1 = struct @align(4) {
  arr_u32:array<atomic<u32>, 3> @offset(0)
}

S2 = struct @align(4) {
  arr_s1:array<S1, 3> @offset(0)
}

$B1: {  # root
  %v:ptr<storage, S2, read_write> = var undef @binding_point(0, 0)
}

%foo = @fragment func(%3:u32 [@location(0)]):void {
  $B2: {
    %4:ptr<storage, atomic<u32>, read_write> = access %v, 0u, %3, 0u, %3
    %5:u32 = )" +
               std::string(param.atomic_name) + R"( %4, 123u
    %x:u32 = let %5
    ret
  }
}
)";
    EXPECT_EQ(src, str());

    auto expect = R"(
S1 = struct @align(4) {
  arr_u32:array<atomic<u32>, 3> @offset(0)
}

S2 = struct @align(4) {
  arr_s1:array<S1, 3> @offset(0)
}

$B1: {  # root
  %v:hlsl.byte_address_buffer<read_write> = var undef @binding_point(0, 0)
}

%foo = @fragment func(%3:u32 [@location(0)]):void {
  $B2: {
    %4:u32 = mul %3, 12u
    %5:u32 = mul %3, 4u
    %6:ptr<function, u32, read_write> = var 0u
    %7:u32 = add 0u, %4
    %8:u32 = add %7, %5
    %9:void = %v.)" +
                  std::string(param.interlock) + R"( %8, 123u, %6
    %10:u32 = load %6
    %x:u32 = let %10
    ret
  }
}
)";
    Run(DecomposeStorageAccess);

    EXPECT_EQ(expect, str());
}

INSTANTIATE_TEST_SUITE_P(
    HlslWriterDecomposeStorageAccessTest,
    DecomposeBuiltinAtomic,
    testing::Values(AtomicData{core::BuiltinFn::kAtomicAdd, "atomicAdd", "InterlockedAdd"},
                    AtomicData{core::BuiltinFn::kAtomicMax, "atomicMax", "InterlockedMax"},
                    AtomicData{core::BuiltinFn::kAtomicMin, "atomicMin", "InterlockedMin"},
                    AtomicData{core::BuiltinFn::kAtomicAnd, "atomicAnd", "InterlockedAnd"},
                    AtomicData{core::BuiltinFn::kAtomicOr, "atomicOr", "InterlockedOr"},
                    AtomicData{core::BuiltinFn::kAtomicXor, "atomicXor", "InterlockedXor"},
                    AtomicData{core::BuiltinFn::kAtomicExchange, "atomicExchange",
                               "InterlockedExchange"}));

TEST_F(HlslWriterDecomposeStorageAccessTest, StoreVecF32) {
    auto* var = b.Var<storage, vec4<f32>, core::Access::kReadWrite>("v");
    var->SetBindingPoint(0, 0);
    b.ir.root_block->Append(var);

    auto* func = b.Function("foo", ty.void_(), core::ir::Function::PipelineStage::kFragment);
    b.Append(func->Block(), [&] {
        b.StoreVectorElement(var, 0_u, 2_f);
        b.StoreVectorElement(var, 1_u, 4_f);
        b.StoreVectorElement(var, 2_u, 8_f);
        b.StoreVectorElement(var, 3_u, 16_f);
        b.Return(func);
    });

    auto* src = R"(
$B1: {  # root
  %v:ptr<storage, vec4<f32>, read_write> = var undef @binding_point(0, 0)
}

%foo = @fragment func():void {
  $B2: {
    store_vector_element %v, 0u, 2.0f
    store_vector_element %v, 1u, 4.0f
    store_vector_element %v, 2u, 8.0f
    store_vector_element %v, 3u, 16.0f
    ret
  }
}
)";
    ASSERT_EQ(src, str());

    auto* expect = R"(
$B1: {  # root
  %v:hlsl.byte_address_buffer<read_write> = var undef @binding_point(0, 0)
}

%foo = @fragment func():void {
  $B2: {
    %3:u32 = bitcast 2.0f
    %4:void = %v.Store 0u, %3
    %5:u32 = bitcast 4.0f
    %6:void = %v.Store 4u, %5
    %7:u32 = bitcast 8.0f
    %8:void = %v.Store 8u, %7
    %9:u32 = bitcast 16.0f
    %10:void = %v.Store 12u, %9
    ret
  }
}
)";

    Run(DecomposeStorageAccess);
    EXPECT_EQ(expect, str());
}

TEST_F(HlslWriterDecomposeStorageAccessTest, StoreScalar) {
    auto* var = b.Var<storage, f32, core::Access::kReadWrite>("v");
    var->SetBindingPoint(0, 0);

    b.ir.root_block->Append(var);
    auto* func = b.Function("foo", ty.void_(), core::ir::Function::PipelineStage::kFragment);
    b.Append(func->Block(), [&] {
        b.Store(var, 2_f);
        b.Return(func);
    });

    auto* src = R"(
$B1: {  # root
  %v:ptr<storage, f32, read_write> = var undef @binding_point(0, 0)
}

%foo = @fragment func():void {
  $B2: {
    store %v, 2.0f
    ret
  }
}
)";
    ASSERT_EQ(src, str());

    auto* expect = R"(
$B1: {  # root
  %v:hlsl.byte_address_buffer<read_write> = var undef @binding_point(0, 0)
}

%foo = @fragment func():void {
  $B2: {
    %3:u32 = bitcast 2.0f
    %4:void = %v.Store 0u, %3
    ret
  }
}
)";
    Run(DecomposeStorageAccess);
    EXPECT_EQ(expect, str());
}

TEST_F(HlslWriterDecomposeStorageAccessTest, StoreScalarF16) {
    auto* var = b.Var<storage, f16, core::Access::kReadWrite>("v");
    var->SetBindingPoint(0, 0);

    b.ir.root_block->Append(var);
    auto* func = b.Function("foo", ty.void_(), core::ir::Function::PipelineStage::kFragment);
    b.Append(func->Block(), [&] {
        b.Store(var, 2_h);
        b.Return(func);
    });

    auto* src = R"(
$B1: {  # root
  %v:ptr<storage, f16, read_write> = var undef @binding_point(0, 0)
}

%foo = @fragment func():void {
  $B2: {
    store %v, 2.0h
    ret
  }
}
)";
    ASSERT_EQ(src, str());

    auto* expect = R"(
$B1: {  # root
  %v:hlsl.byte_address_buffer<read_write> = var undef @binding_point(0, 0)
}

%foo = @fragment func():void {
  $B2: {
    %3:void = %v.StoreF16 0u, 2.0h
    ret
  }
}
)";
    Run(DecomposeStorageAccess);
    EXPECT_EQ(expect, str());
}

TEST_F(HlslWriterDecomposeStorageAccessTest, StoreVectorElement) {
    auto* var = b.Var<storage, vec3<f32>, core::Access::kReadWrite>("v");
    var->SetBindingPoint(0, 0);

    b.ir.root_block->Append(var);
    auto* func = b.Function("foo", ty.void_(), core::ir::Function::PipelineStage::kFragment);
    b.Append(func->Block(), [&] {
        b.StoreVectorElement(var, 1_u, 2_f);
        b.Return(func);
    });

    auto* src = R"(
$B1: {  # root
  %v:ptr<storage, vec3<f32>, read_write> = var undef @binding_point(0, 0)
}

%foo = @fragment func():void {
  $B2: {
    store_vector_element %v, 1u, 2.0f
    ret
  }
}
)";
    ASSERT_EQ(src, str());

    auto* expect = R"(
$B1: {  # root
  %v:hlsl.byte_address_buffer<read_write> = var undef @binding_point(0, 0)
}

%foo = @fragment func():void {
  $B2: {
    %3:u32 = bitcast 2.0f
    %4:void = %v.Store 4u, %3
    ret
  }
}
)";
    Run(DecomposeStorageAccess);
    EXPECT_EQ(expect, str());
}

TEST_F(HlslWriterDecomposeStorageAccessTest, StoreVectorElementF16) {
    auto* var = b.Var<storage, vec3<f16>, core::Access::kReadWrite>("v");
    var->SetBindingPoint(0, 0);

    b.ir.root_block->Append(var);
    auto* func = b.Function("foo", ty.void_(), core::ir::Function::PipelineStage::kFragment);
    b.Append(func->Block(), [&] {
        b.StoreVectorElement(var, 1_u, 2_h);
        b.Return(func);
    });

    auto* src = R"(
$B1: {  # root
  %v:ptr<storage, vec3<f16>, read_write> = var undef @binding_point(0, 0)
}

%foo = @fragment func():void {
  $B2: {
    store_vector_element %v, 1u, 2.0h
    ret
  }
}
)";
    ASSERT_EQ(src, str());

    auto* expect = R"(
$B1: {  # root
  %v:hlsl.byte_address_buffer<read_write> = var undef @binding_point(0, 0)
}

%foo = @fragment func():void {
  $B2: {
    %3:void = %v.StoreF16 2u, 2.0h
    ret
  }
}
)";
    Run(DecomposeStorageAccess);
    EXPECT_EQ(expect, str());
}

TEST_F(HlslWriterDecomposeStorageAccessTest, StoreVector) {
    auto* var = b.Var<storage, vec3<f32>, core::Access::kReadWrite>("v");
    var->SetBindingPoint(0, 0);

    b.ir.root_block->Append(var);
    auto* func = b.Function("foo", ty.void_(), core::ir::Function::PipelineStage::kFragment);
    b.Append(func->Block(), [&] {
        b.Store(var, b.Composite(ty.vec3<f32>(), 2_f, 3_f, 4_f));
        b.Return(func);
    });

    auto* src = R"(
$B1: {  # root
  %v:ptr<storage, vec3<f32>, read_write> = var undef @binding_point(0, 0)
}

%foo = @fragment func():void {
  $B2: {
    store %v, vec3<f32>(2.0f, 3.0f, 4.0f)
    ret
  }
}
)";
    ASSERT_EQ(src, str());

    auto* expect = R"(
$B1: {  # root
  %v:hlsl.byte_address_buffer<read_write> = var undef @binding_point(0, 0)
}

%foo = @fragment func():void {
  $B2: {
    %3:vec3<u32> = bitcast vec3<f32>(2.0f, 3.0f, 4.0f)
    %4:void = %v.Store3 0u, %3
    ret
  }
}
)";
    Run(DecomposeStorageAccess);
    EXPECT_EQ(expect, str());
}

TEST_F(HlslWriterDecomposeStorageAccessTest, StoreVectorF16) {
    auto* var = b.Var<storage, vec3<f16>, core::Access::kReadWrite>("v");
    var->SetBindingPoint(0, 0);

    b.ir.root_block->Append(var);
    auto* func = b.Function("foo", ty.void_(), core::ir::Function::PipelineStage::kFragment);
    b.Append(func->Block(), [&] {
        b.Store(var, b.Composite(ty.vec3<f16>(), 2_h, 3_h, 4_h));

        b.Return(func);
    });

    auto* src = R"(
$B1: {  # root
  %v:ptr<storage, vec3<f16>, read_write> = var undef @binding_point(0, 0)
}

%foo = @fragment func():void {
  $B2: {
    store %v, vec3<f16>(2.0h, 3.0h, 4.0h)
    ret
  }
}
)";
    ASSERT_EQ(src, str());

    auto* expect = R"(
$B1: {  # root
  %v:hlsl.byte_address_buffer<read_write> = var undef @binding_point(0, 0)
}

%foo = @fragment func():void {
  $B2: {
    %3:void = %v.Store3F16 0u, vec3<f16>(2.0h, 3.0h, 4.0h)
    ret
  }
}
)";
    Run(DecomposeStorageAccess);
    EXPECT_EQ(expect, str());
}

TEST_F(HlslWriterDecomposeStorageAccessTest, StoreMatrixElement) {
    auto* var = b.Var<storage, mat2x3<f32>, core::Access::kReadWrite>("v");
    var->SetBindingPoint(0, 0);

    b.ir.root_block->Append(var);
    auto* func = b.Function("foo", ty.void_(), core::ir::Function::PipelineStage::kFragment);
    b.Append(func->Block(), [&] {
        b.StoreVectorElement(
            b.Access(ty.ptr<storage, vec3<f32>, core::Access::kReadWrite>(), var, 1_u), 2_u, 5_f);
        b.Return(func);
    });

    auto* src = R"(
$B1: {  # root
  %v:ptr<storage, mat2x3<f32>, read_write> = var undef @binding_point(0, 0)
}

%foo = @fragment func():void {
  $B2: {
    %3:ptr<storage, vec3<f32>, read_write> = access %v, 1u
    store_vector_element %3, 2u, 5.0f
    ret
  }
}
)";
    ASSERT_EQ(src, str());

    auto* expect = R"(
$B1: {  # root
  %v:hlsl.byte_address_buffer<read_write> = var undef @binding_point(0, 0)
}

%foo = @fragment func():void {
  $B2: {
    %3:u32 = bitcast 5.0f
    %4:void = %v.Store 24u, %3
    ret
  }
}
)";
    Run(DecomposeStorageAccess);
    EXPECT_EQ(expect, str());
}

TEST_F(HlslWriterDecomposeStorageAccessTest, StoreMatrixElementF16) {
    auto* var = b.Var<storage, mat2x3<f16>, core::Access::kReadWrite>("v");
    var->SetBindingPoint(0, 0);

    b.ir.root_block->Append(var);
    auto* func = b.Function("foo", ty.void_(), core::ir::Function::PipelineStage::kFragment);
    b.Append(func->Block(), [&] {
        b.StoreVectorElement(
            b.Access(ty.ptr<storage, vec3<f16>, core::Access::kReadWrite>(), var, 1_u), 2_u, 5_h);
        b.Return(func);
    });

    auto* src = R"(
$B1: {  # root
  %v:ptr<storage, mat2x3<f16>, read_write> = var undef @binding_point(0, 0)
}

%foo = @fragment func():void {
  $B2: {
    %3:ptr<storage, vec3<f16>, read_write> = access %v, 1u
    store_vector_element %3, 2u, 5.0h
    ret
  }
}
)";
    ASSERT_EQ(src, str());

    auto* expect = R"(
$B1: {  # root
  %v:hlsl.byte_address_buffer<read_write> = var undef @binding_point(0, 0)
}

%foo = @fragment func():void {
  $B2: {
    %3:void = %v.StoreF16 12u, 5.0h
    ret
  }
}
)";
    Run(DecomposeStorageAccess);
    EXPECT_EQ(expect, str());
}

TEST_F(HlslWriterDecomposeStorageAccessTest, StoreMatrixColumn) {
    auto* var = b.Var<storage, mat2x3<f32>, core::Access::kReadWrite>("v");
    var->SetBindingPoint(0, 0);

    b.ir.root_block->Append(var);
    auto* func = b.Function("foo", ty.void_(), core::ir::Function::PipelineStage::kFragment);
    b.Append(func->Block(), [&] {
        b.Store(b.Access(ty.ptr<storage, vec3<f32>, core::Access::kReadWrite>(), var, 1_u),
                b.Splat<vec3<f32>>(5_f));
        b.Return(func);
    });

    auto* src = R"(
$B1: {  # root
  %v:ptr<storage, mat2x3<f32>, read_write> = var undef @binding_point(0, 0)
}

%foo = @fragment func():void {
  $B2: {
    %3:ptr<storage, vec3<f32>, read_write> = access %v, 1u
    store %3, vec3<f32>(5.0f)
    ret
  }
}
)";
    ASSERT_EQ(src, str());

    auto* expect = R"(
$B1: {  # root
  %v:hlsl.byte_address_buffer<read_write> = var undef @binding_point(0, 0)
}

%foo = @fragment func():void {
  $B2: {
    %3:vec3<u32> = bitcast vec3<f32>(5.0f)
    %4:void = %v.Store3 16u, %3
    ret
  }
}
)";
    Run(DecomposeStorageAccess);
    EXPECT_EQ(expect, str());
}

TEST_F(HlslWriterDecomposeStorageAccessTest, StoreMatrixColumnF16) {
    auto* var = b.Var<storage, mat2x3<f16>, core::Access::kReadWrite>("v");
    var->SetBindingPoint(0, 0);

    b.ir.root_block->Append(var);
    auto* func = b.Function("foo", ty.void_(), core::ir::Function::PipelineStage::kFragment);
    b.Append(func->Block(), [&] {
        b.Store(b.Access(ty.ptr<storage, vec3<f16>, core::Access::kReadWrite>(), var, 1_u),
                b.Splat<vec3<f16>>(5_h));
        b.Return(func);
    });

    auto* src = R"(
$B1: {  # root
  %v:ptr<storage, mat2x3<f16>, read_write> = var undef @binding_point(0, 0)
}

%foo = @fragment func():void {
  $B2: {
    %3:ptr<storage, vec3<f16>, read_write> = access %v, 1u
    store %3, vec3<f16>(5.0h)
    ret
  }
}
)";
    ASSERT_EQ(src, str());

    auto* expect = R"(
$B1: {  # root
  %v:hlsl.byte_address_buffer<read_write> = var undef @binding_point(0, 0)
}

%foo = @fragment func():void {
  $B2: {
    %3:void = %v.Store3F16 8u, vec3<f16>(5.0h)
    ret
  }
}
)";
    Run(DecomposeStorageAccess);
    EXPECT_EQ(expect, str());
}

TEST_F(HlslWriterDecomposeStorageAccessTest, StoreMatrix) {
    auto* var = b.Var<storage, mat2x3<f32>, core::Access::kReadWrite>("v");
    var->SetBindingPoint(0, 0);

    b.ir.root_block->Append(var);
    auto* func = b.Function("foo", ty.void_(), core::ir::Function::PipelineStage::kFragment);
    b.Append(func->Block(), [&] {
        b.Store(var, b.Zero<mat2x3<f32>>());
        b.Return(func);
    });

    auto* src = R"(
$B1: {  # root
  %v:ptr<storage, mat2x3<f32>, read_write> = var undef @binding_point(0, 0)
}

%foo = @fragment func():void {
  $B2: {
    store %v, mat2x3<f32>(vec3<f32>(0.0f))
    ret
  }
}
)";
    ASSERT_EQ(src, str());

    auto* expect = R"(
$B1: {  # root
  %v:hlsl.byte_address_buffer<read_write> = var undef @binding_point(0, 0)
}

%foo = @fragment func():void {
  $B2: {
    %3:void = call %4, 0u, mat2x3<f32>(vec3<f32>(0.0f))
    ret
  }
}
%4 = func(%offset:u32, %obj:mat2x3<f32>):void {
  $B3: {
    %7:vec3<f32> = access %obj, 0u
    %8:u32 = add %offset, 0u
    %9:vec3<u32> = bitcast %7
    %10:void = %v.Store3 %8, %9
    %11:vec3<f32> = access %obj, 1u
    %12:u32 = add %offset, 16u
    %13:vec3<u32> = bitcast %11
    %14:void = %v.Store3 %12, %13
    ret
  }
}
)";
    Run(DecomposeStorageAccess);
    EXPECT_EQ(expect, str());
}

TEST_F(HlslWriterDecomposeStorageAccessTest, StoreMatrixF16) {
    auto* var = b.Var<storage, mat2x3<f16>, core::Access::kReadWrite>("v");
    var->SetBindingPoint(0, 0);

    b.ir.root_block->Append(var);
    auto* func = b.Function("foo", ty.void_(), core::ir::Function::PipelineStage::kFragment);
    b.Append(func->Block(), [&] {
        b.Store(var, b.Zero<mat2x3<f16>>());
        b.Return(func);
    });

    auto* src = R"(
$B1: {  # root
  %v:ptr<storage, mat2x3<f16>, read_write> = var undef @binding_point(0, 0)
}

%foo = @fragment func():void {
  $B2: {
    store %v, mat2x3<f16>(vec3<f16>(0.0h))
    ret
  }
}
)";
    ASSERT_EQ(src, str());

    auto* expect = R"(
$B1: {  # root
  %v:hlsl.byte_address_buffer<read_write> = var undef @binding_point(0, 0)
}

%foo = @fragment func():void {
  $B2: {
    %3:void = call %4, 0u, mat2x3<f16>(vec3<f16>(0.0h))
    ret
  }
}
%4 = func(%offset:u32, %obj:mat2x3<f16>):void {
  $B3: {
    %7:vec3<f16> = access %obj, 0u
    %8:u32 = add %offset, 0u
    %9:void = %v.Store3F16 %8, %7
    %10:vec3<f16> = access %obj, 1u
    %11:u32 = add %offset, 8u
    %12:void = %v.Store3F16 %11, %10
    ret
  }
}
)";
    Run(DecomposeStorageAccess);
    EXPECT_EQ(expect, str());
}

TEST_F(HlslWriterDecomposeStorageAccessTest, StoreArrayElement) {
    auto* var = b.Var<storage, array<f32, 5>, core::Access::kReadWrite>("v");
    var->SetBindingPoint(0, 0);

    b.ir.root_block->Append(var);
    auto* func = b.Function("foo", ty.void_(), core::ir::Function::PipelineStage::kFragment);
    b.Append(func->Block(), [&] {
        b.Store(b.Access(ty.ptr<storage, f32, core::Access::kReadWrite>(), var, 3_u), 1_f);
        b.Return(func);
    });

    auto* src = R"(
$B1: {  # root
  %v:ptr<storage, array<f32, 5>, read_write> = var undef @binding_point(0, 0)
}

%foo = @fragment func():void {
  $B2: {
    %3:ptr<storage, f32, read_write> = access %v, 3u
    store %3, 1.0f
    ret
  }
}
)";
    ASSERT_EQ(src, str());

    auto* expect = R"(
$B1: {  # root
  %v:hlsl.byte_address_buffer<read_write> = var undef @binding_point(0, 0)
}

%foo = @fragment func():void {
  $B2: {
    %3:u32 = bitcast 1.0f
    %4:void = %v.Store 12u, %3
    ret
  }
}
)";
    Run(DecomposeStorageAccess);
    EXPECT_EQ(expect, str());
}

TEST_F(HlslWriterDecomposeStorageAccessTest, StoreArrayElementF16) {
    auto* var = b.Var<storage, array<f16, 5>, core::Access::kReadWrite>("v");
    var->SetBindingPoint(0, 0);

    b.ir.root_block->Append(var);
    auto* func = b.Function("foo", ty.void_(), core::ir::Function::PipelineStage::kFragment);
    b.Append(func->Block(), [&] {
        b.Store(b.Access(ty.ptr<storage, f16, core::Access::kReadWrite>(), var, 3_u), 1_h);
        b.Return(func);
    });

    auto* src = R"(
$B1: {  # root
  %v:ptr<storage, array<f16, 5>, read_write> = var undef @binding_point(0, 0)
}

%foo = @fragment func():void {
  $B2: {
    %3:ptr<storage, f16, read_write> = access %v, 3u
    store %3, 1.0h
    ret
  }
}
)";
    ASSERT_EQ(src, str());

    auto* expect = R"(
$B1: {  # root
  %v:hlsl.byte_address_buffer<read_write> = var undef @binding_point(0, 0)
}

%foo = @fragment func():void {
  $B2: {
    %3:void = %v.StoreF16 6u, 1.0h
    ret
  }
}
)";
    Run(DecomposeStorageAccess);
    EXPECT_EQ(expect, str());
}

TEST_F(HlslWriterDecomposeStorageAccessTest, StoreArray) {
    auto* var = b.Var<storage, array<vec3<f32>, 5>, core::Access::kReadWrite>("v");
    var->SetBindingPoint(0, 0);

    b.ir.root_block->Append(var);
    auto* func = b.Function("foo", ty.void_(), core::ir::Function::PipelineStage::kFragment);
    b.Append(func->Block(), [&] {
        auto* ary = b.Let("ary", b.Zero<array<vec3<f32>, 5>>());
        b.Store(var, ary);
        b.Return(func);
    });

    auto* src = R"(
$B1: {  # root
  %v:ptr<storage, array<vec3<f32>, 5>, read_write> = var undef @binding_point(0, 0)
}

%foo = @fragment func():void {
  $B2: {
    %ary:array<vec3<f32>, 5> = let array<vec3<f32>, 5>(vec3<f32>(0.0f))
    store %v, %ary
    ret
  }
}
)";
    ASSERT_EQ(src, str());

    auto* expect = R"(
$B1: {  # root
  %v:hlsl.byte_address_buffer<read_write> = var undef @binding_point(0, 0)
}

%foo = @fragment func():void {
  $B2: {
    %ary:array<vec3<f32>, 5> = let array<vec3<f32>, 5>(vec3<f32>(0.0f))
    %4:void = call %5, 0u, %ary
    ret
  }
}
%5 = func(%offset:u32, %obj:array<vec3<f32>, 5>):void {
  $B3: {
    loop [i: $B4, b: $B5, c: $B6] {  # loop_1
      $B4: {  # initializer
        next_iteration 0u  # -> $B5
      }
      $B5 (%idx:u32): {  # body
        %9:bool = gte %idx, 5u
        if %9 [t: $B7] {  # if_1
          $B7: {  # true
            exit_loop  # loop_1
          }
        }
        %10:vec3<f32> = access %obj, %idx
        %11:u32 = mul %idx, 16u
        %12:u32 = add %offset, %11
        %13:vec3<u32> = bitcast %10
        %14:void = %v.Store3 %12, %13
        continue  # -> $B6
      }
      $B6: {  # continuing
        %15:u32 = add %idx, 1u
        next_iteration %15  # -> $B5
      }
    }
    ret
  }
}
)";
    Run(DecomposeStorageAccess);
    EXPECT_EQ(expect, str());
}

TEST_F(HlslWriterDecomposeStorageAccessTest, StoreStructMember) {
    auto* SB = ty.Struct(mod.symbols.New("SB"), {
                                                    {mod.symbols.New("a"), ty.i32()},
                                                    {mod.symbols.New("b"), ty.f32()},
                                                });

    auto* var = b.Var("v", storage, SB, core::Access::kReadWrite);
    var->SetBindingPoint(0, 0);

    b.ir.root_block->Append(var);
    auto* func = b.Function("foo", ty.void_(), core::ir::Function::PipelineStage::kFragment);
    b.Append(func->Block(), [&] {
        b.Store(b.Access(ty.ptr<storage, f32, core::Access::kReadWrite>(), var, 1_u), 3_f);
        b.Return(func);
    });

    auto* src = R"(
SB = struct @align(4) {
  a:i32 @offset(0)
  b:f32 @offset(4)
}

$B1: {  # root
  %v:ptr<storage, SB, read_write> = var undef @binding_point(0, 0)
}

%foo = @fragment func():void {
  $B2: {
    %3:ptr<storage, f32, read_write> = access %v, 1u
    store %3, 3.0f
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
  %v:hlsl.byte_address_buffer<read_write> = var undef @binding_point(0, 0)
}

%foo = @fragment func():void {
  $B2: {
    %3:u32 = bitcast 3.0f
    %4:void = %v.Store 4u, %3
    ret
  }
}
)";
    Run(DecomposeStorageAccess);
    EXPECT_EQ(expect, str());
}

TEST_F(HlslWriterDecomposeStorageAccessTest, StoreStructMemberF16) {
    auto* SB = ty.Struct(mod.symbols.New("SB"), {
                                                    {mod.symbols.New("a"), ty.i32()},
                                                    {mod.symbols.New("b"), ty.f16()},
                                                });

    auto* var = b.Var("v", storage, SB, core::Access::kReadWrite);
    var->SetBindingPoint(0, 0);

    b.ir.root_block->Append(var);
    auto* func = b.Function("foo", ty.void_(), core::ir::Function::PipelineStage::kFragment);
    b.Append(func->Block(), [&] {
        b.Store(b.Access(ty.ptr<storage, f16, core::Access::kReadWrite>(), var, 1_u), 3_h);
        b.Return(func);
    });

    auto* src = R"(
SB = struct @align(4) {
  a:i32 @offset(0)
  b:f16 @offset(4)
}

$B1: {  # root
  %v:ptr<storage, SB, read_write> = var undef @binding_point(0, 0)
}

%foo = @fragment func():void {
  $B2: {
    %3:ptr<storage, f16, read_write> = access %v, 1u
    store %3, 3.0h
    ret
  }
}
)";
    ASSERT_EQ(src, str());

    auto* expect = R"(
SB = struct @align(4) {
  a:i32 @offset(0)
  b:f16 @offset(4)
}

$B1: {  # root
  %v:hlsl.byte_address_buffer<read_write> = var undef @binding_point(0, 0)
}

%foo = @fragment func():void {
  $B2: {
    %3:void = %v.StoreF16 4u, 3.0h
    ret
  }
}
)";
    Run(DecomposeStorageAccess);
    EXPECT_EQ(expect, str());
}

TEST_F(HlslWriterDecomposeStorageAccessTest, StoreStructNested) {
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

    auto* var = b.Var("v", storage, SB, core::Access::kReadWrite);
    var->SetBindingPoint(0, 0);

    b.ir.root_block->Append(var);
    auto* func = b.Function("foo", ty.void_(), core::ir::Function::PipelineStage::kFragment);
    b.Append(func->Block(), [&] {
        b.Store(b.Access(ty.ptr<storage, f32, core::Access::kReadWrite>(), var, 1_u, 0_u), 2_f);
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
  %v:ptr<storage, SB, read_write> = var undef @binding_point(0, 0)
}

%foo = @fragment func():void {
  $B2: {
    %3:ptr<storage, f32, read_write> = access %v, 1u, 0u
    store %3, 2.0f
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
  %v:hlsl.byte_address_buffer<read_write> = var undef @binding_point(0, 0)
}

%foo = @fragment func():void {
  $B2: {
    %3:u32 = bitcast 2.0f
    %4:void = %v.Store 16u, %3
    ret
  }
}
)";
    Run(DecomposeStorageAccess);
    EXPECT_EQ(expect, str());
}

TEST_F(HlslWriterDecomposeStorageAccessTest, StoreStruct) {
    auto* Inner = ty.Struct(mod.symbols.New("Inner"), {
                                                          {mod.symbols.New("s"), ty.f32()},
                                                          {mod.symbols.New("t"), ty.vec3<f32>()},
                                                      });
    auto* Outer = ty.Struct(mod.symbols.New("Outer"), {
                                                          {mod.symbols.New("x"), ty.f32()},
                                                          {mod.symbols.New("y"), Inner},
                                                      });

    auto* SB = ty.Struct(mod.symbols.New("SB"), {
                                                    {mod.symbols.New("a"), ty.i32()},
                                                    {mod.symbols.New("b"), Outer},
                                                });

    auto* var = b.Var("v", storage, SB, core::Access::kReadWrite);
    var->SetBindingPoint(0, 0);

    b.ir.root_block->Append(var);
    auto* func = b.Function("foo", ty.void_(), core::ir::Function::PipelineStage::kFragment);
    b.Append(func->Block(), [&] {
        auto* s = b.Let("s", b.Zero(SB));
        b.Store(var, s);
        b.Return(func);
    });

    auto* src = R"(
Inner = struct @align(16) {
  s:f32 @offset(0)
  t:vec3<f32> @offset(16)
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
  %v:ptr<storage, SB, read_write> = var undef @binding_point(0, 0)
}

%foo = @fragment func():void {
  $B2: {
    %s:SB = let SB(0i, Outer(0.0f, Inner(0.0f, vec3<f32>(0.0f))))
    store %v, %s
    ret
  }
}
)";
    ASSERT_EQ(src, str());

    auto* expect = R"(
Inner = struct @align(16) {
  s:f32 @offset(0)
  t:vec3<f32> @offset(16)
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
  %v:hlsl.byte_address_buffer<read_write> = var undef @binding_point(0, 0)
}

%foo = @fragment func():void {
  $B2: {
    %s:SB = let SB(0i, Outer(0.0f, Inner(0.0f, vec3<f32>(0.0f))))
    %4:void = call %5, 0u, %s
    ret
  }
}
%5 = func(%offset:u32, %obj:SB):void {
  $B3: {
    %8:i32 = access %obj, 0u
    %9:u32 = add %offset, 0u
    %10:u32 = bitcast %8
    %11:void = %v.Store %9, %10
    %12:Outer = access %obj, 1u
    %13:u32 = add %offset, 16u
    %14:void = call %15, %13, %12
    ret
  }
}
%15 = func(%offset_1:u32, %obj_1:Outer):void {  # %offset_1: 'offset', %obj_1: 'obj'
  $B4: {
    %18:f32 = access %obj_1, 0u
    %19:u32 = add %offset_1, 0u
    %20:u32 = bitcast %18
    %21:void = %v.Store %19, %20
    %22:Inner = access %obj_1, 1u
    %23:u32 = add %offset_1, 16u
    %24:void = call %25, %23, %22
    ret
  }
}
%25 = func(%offset_2:u32, %obj_2:Inner):void {  # %offset_2: 'offset', %obj_2: 'obj'
  $B5: {
    %28:f32 = access %obj_2, 0u
    %29:u32 = add %offset_2, 0u
    %30:u32 = bitcast %28
    %31:void = %v.Store %29, %30
    %32:vec3<f32> = access %obj_2, 1u
    %33:u32 = add %offset_2, 16u
    %34:vec3<u32> = bitcast %32
    %35:void = %v.Store3 %33, %34
    ret
  }
}
)";
    Run(DecomposeStorageAccess);
    EXPECT_EQ(expect, str());
}

TEST_F(HlslWriterDecomposeStorageAccessTest, StoreStructComplex) {
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

    auto* var = b.Var("v", storage, SB, core::Access::kReadWrite);
    var->SetBindingPoint(0, 0);

    b.ir.root_block->Append(var);
    auto* func = b.Function("foo", ty.void_(), core::ir::Function::PipelineStage::kFragment);
    b.Append(func->Block(), [&] {
        auto* s = b.Let("s", b.Zero(SB));
        b.Store(var, s);
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
  %v:ptr<storage, SB, read_write> = var undef @binding_point(0, 0)
}

%foo = @fragment func():void {
  $B2: {
    %s:SB = let SB(0i, Outer(0.0f, Inner(mat3x3<f32>(vec3<f32>(0.0f)), array<vec3<f32>, 5>(vec3<f32>(0.0f)))))
    store %v, %s
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
  %v:hlsl.byte_address_buffer<read_write> = var undef @binding_point(0, 0)
}

%foo = @fragment func():void {
  $B2: {
    %s:SB = let SB(0i, Outer(0.0f, Inner(mat3x3<f32>(vec3<f32>(0.0f)), array<vec3<f32>, 5>(vec3<f32>(0.0f)))))
    %4:void = call %5, 0u, %s
    ret
  }
}
%5 = func(%offset:u32, %obj:SB):void {
  $B3: {
    %8:i32 = access %obj, 0u
    %9:u32 = add %offset, 0u
    %10:u32 = bitcast %8
    %11:void = %v.Store %9, %10
    %12:Outer = access %obj, 1u
    %13:u32 = add %offset, 16u
    %14:void = call %15, %13, %12
    ret
  }
}
%15 = func(%offset_1:u32, %obj_1:Outer):void {  # %offset_1: 'offset', %obj_1: 'obj'
  $B4: {
    %18:f32 = access %obj_1, 0u
    %19:u32 = add %offset_1, 0u
    %20:u32 = bitcast %18
    %21:void = %v.Store %19, %20
    %22:Inner = access %obj_1, 1u
    %23:u32 = add %offset_1, 16u
    %24:void = call %25, %23, %22
    ret
  }
}
%25 = func(%offset_2:u32, %obj_2:Inner):void {  # %offset_2: 'offset', %obj_2: 'obj'
  $B5: {
    %28:mat3x3<f32> = access %obj_2, 0u
    %29:u32 = add %offset_2, 0u
    %30:void = call %31, %29, %28
    %32:array<vec3<f32>, 5> = access %obj_2, 1u
    %33:u32 = add %offset_2, 48u
    %34:void = call %35, %33, %32
    ret
  }
}
%31 = func(%offset_3:u32, %obj_3:mat3x3<f32>):void {  # %offset_3: 'offset', %obj_3: 'obj'
  $B6: {
    %38:vec3<f32> = access %obj_3, 0u
    %39:u32 = add %offset_3, 0u
    %40:vec3<u32> = bitcast %38
    %41:void = %v.Store3 %39, %40
    %42:vec3<f32> = access %obj_3, 1u
    %43:u32 = add %offset_3, 16u
    %44:vec3<u32> = bitcast %42
    %45:void = %v.Store3 %43, %44
    %46:vec3<f32> = access %obj_3, 2u
    %47:u32 = add %offset_3, 32u
    %48:vec3<u32> = bitcast %46
    %49:void = %v.Store3 %47, %48
    ret
  }
}
%35 = func(%offset_4:u32, %obj_4:array<vec3<f32>, 5>):void {  # %offset_4: 'offset', %obj_4: 'obj'
  $B7: {
    loop [i: $B8, b: $B9, c: $B10] {  # loop_1
      $B8: {  # initializer
        next_iteration 0u  # -> $B9
      }
      $B9 (%idx:u32): {  # body
        %53:bool = gte %idx, 5u
        if %53 [t: $B11] {  # if_1
          $B11: {  # true
            exit_loop  # loop_1
          }
        }
        %54:vec3<f32> = access %obj_4, %idx
        %55:u32 = mul %idx, 16u
        %56:u32 = add %offset_4, %55
        %57:vec3<u32> = bitcast %54
        %58:void = %v.Store3 %56, %57
        continue  # -> $B10
      }
      $B10: {  # continuing
        %59:u32 = add %idx, 1u
        next_iteration %59  # -> $B9
      }
    }
    ret
  }
}
)";
    Run(DecomposeStorageAccess);
    EXPECT_EQ(expect, str());
}

TEST_F(HlslWriterDecomposeStorageAccessTest, ArrayLengthDirect) {
    auto* sb = b.Var("sb", ty.ptr<storage, array<i32>>());
    sb->SetBindingPoint(0, 0);
    b.ir.root_block->Append(sb);

    auto* func = b.Function("foo", ty.void_(), core::ir::Function::PipelineStage::kFragment);
    b.Append(func->Block(), [&] {
        b.Let("len", b.Call(ty.u32(), core::BuiltinFn::kArrayLength, sb));
        b.Return(func);
    });

    auto* src = R"(
$B1: {  # root
  %sb:ptr<storage, array<i32>, read_write> = var undef @binding_point(0, 0)
}

%foo = @fragment func():void {
  $B2: {
    %3:u32 = arrayLength %sb
    %len:u32 = let %3
    ret
  }
}
)";
    ASSERT_EQ(src, str());

    auto* expect = R"(
$B1: {  # root
  %sb:hlsl.byte_address_buffer<read_write> = var undef @binding_point(0, 0)
}

%foo = @fragment func():void {
  $B2: {
    %3:ptr<function, u32, read_write> = var undef
    %4:void = %sb.GetDimensions %3
    %5:u32 = load %3
    %6:u32 = div %5, 4u
    %len:u32 = let %6
    ret
  }
}
)";
    Run(DecomposeStorageAccess);
    EXPECT_EQ(expect, str());
}

TEST_F(HlslWriterDecomposeStorageAccessTest, ArrayLengthInStruct) {
    auto* SB =
        ty.Struct(mod.symbols.New("SB"), {
                                             {mod.symbols.New("x"), ty.i32()},
                                             {mod.symbols.New("arr"), ty.runtime_array(ty.i32())},
                                         });

    auto* sb = b.Var("sb", ty.ptr(storage, SB));
    sb->SetBindingPoint(0, 0);
    b.ir.root_block->Append(sb);

    auto* func = b.Function("foo", ty.void_(), core::ir::Function::PipelineStage::kFragment);
    b.Append(func->Block(), [&] {
        b.Let("len", b.Call(ty.u32(), core::BuiltinFn::kArrayLength,
                            b.Access(ty.ptr<storage, array<i32>>(), sb, 1_u)));
        b.Return(func);
    });

    auto* src = R"(
SB = struct @align(4) {
  x:i32 @offset(0)
  arr:array<i32> @offset(4)
}

$B1: {  # root
  %sb:ptr<storage, SB, read_write> = var undef @binding_point(0, 0)
}

%foo = @fragment func():void {
  $B2: {
    %3:ptr<storage, array<i32>, read_write> = access %sb, 1u
    %4:u32 = arrayLength %3
    %len:u32 = let %4
    ret
  }
}
)";
    ASSERT_EQ(src, str());

    auto* expect = R"(
SB = struct @align(4) {
  x:i32 @offset(0)
  arr:array<i32> @offset(4)
}

$B1: {  # root
  %sb:hlsl.byte_address_buffer<read_write> = var undef @binding_point(0, 0)
}

%foo = @fragment func():void {
  $B2: {
    %3:ptr<function, u32, read_write> = var undef
    %4:void = %sb.GetDimensions %3
    %5:u32 = load %3
    %6:u32 = sub %5, 4u
    %7:u32 = div %6, 4u
    %len:u32 = let %7
    ret
  }
}
)";
    Run(DecomposeStorageAccess);
    EXPECT_EQ(expect, str());
}

TEST_F(HlslWriterDecomposeStorageAccessTest, ArrayLengthOfStruct) {
    auto* SB = ty.Struct(mod.symbols.New("SB"), {
                                                    {mod.symbols.New("f"), ty.f32()},
                                                });

    auto* sb = b.Var("sb", ty.ptr(storage, ty.runtime_array(SB), core::Access::kRead));
    sb->SetBindingPoint(0, 0);
    b.ir.root_block->Append(sb);

    auto* func = b.Function("foo", ty.void_(), core::ir::Function::PipelineStage::kFragment);
    b.Append(func->Block(), [&] {
        b.Let("len", b.Call(ty.u32(), core::BuiltinFn::kArrayLength, sb));
        b.Return(func);
    });

    auto* src = R"(
SB = struct @align(4) {
  f:f32 @offset(0)
}

$B1: {  # root
  %sb:ptr<storage, array<SB>, read> = var undef @binding_point(0, 0)
}

%foo = @fragment func():void {
  $B2: {
    %3:u32 = arrayLength %sb
    %len:u32 = let %3
    ret
  }
}
)";
    ASSERT_EQ(src, str());

    auto* expect = R"(
SB = struct @align(4) {
  f:f32 @offset(0)
}

$B1: {  # root
  %sb:hlsl.byte_address_buffer<read> = var undef @binding_point(0, 0)
}

%foo = @fragment func():void {
  $B2: {
    %3:ptr<function, u32, read_write> = var undef
    %4:void = %sb.GetDimensions %3
    %5:u32 = load %3
    %6:u32 = div %5, 4u
    %len:u32 = let %6
    ret
  }
}
)";
    Run(DecomposeStorageAccess);
    EXPECT_EQ(expect, str());
}

TEST_F(HlslWriterDecomposeStorageAccessTest, ArrayLengthArrayOfArrayOfStruct) {
    auto* SB = ty.Struct(mod.symbols.New("SB"), {
                                                    {mod.symbols.New("f"), ty.f32()},
                                                });
    auto* sb = b.Var("sb", ty.ptr(storage, ty.runtime_array(ty.array(SB, 4))));
    sb->SetBindingPoint(0, 0);
    b.ir.root_block->Append(sb);

    auto* func = b.Function("foo", ty.void_(), core::ir::Function::PipelineStage::kFragment);
    b.Append(func->Block(), [&] {
        b.Let("len", b.Call(ty.u32(), core::BuiltinFn::kArrayLength, sb));
        b.Return(func);
    });

    auto* src = R"(
SB = struct @align(4) {
  f:f32 @offset(0)
}

$B1: {  # root
  %sb:ptr<storage, array<array<SB, 4>>, read_write> = var undef @binding_point(0, 0)
}

%foo = @fragment func():void {
  $B2: {
    %3:u32 = arrayLength %sb
    %len:u32 = let %3
    ret
  }
}
)";
    ASSERT_EQ(src, str());

    auto* expect = R"(
SB = struct @align(4) {
  f:f32 @offset(0)
}

$B1: {  # root
  %sb:hlsl.byte_address_buffer<read_write> = var undef @binding_point(0, 0)
}

%foo = @fragment func():void {
  $B2: {
    %3:ptr<function, u32, read_write> = var undef
    %4:void = %sb.GetDimensions %3
    %5:u32 = load %3
    %6:u32 = div %5, 16u
    %len:u32 = let %6
    ret
  }
}
)";
    Run(DecomposeStorageAccess);
    EXPECT_EQ(expect, str());
}

TEST_F(HlslWriterDecomposeStorageAccessTest, ArrayLengthMultiple) {
    auto* sb = b.Var("sb", ty.ptr<storage, array<i32>>());
    sb->SetBindingPoint(0, 0);
    b.ir.root_block->Append(sb);

    auto* func = b.Function("foo", ty.void_(), core::ir::Function::PipelineStage::kFragment);
    b.Append(func->Block(), [&] {
        b.Let("a", b.Call(ty.u32(), core::BuiltinFn::kArrayLength, sb));
        b.Let("b", b.Call(ty.u32(), core::BuiltinFn::kArrayLength, sb));
        b.Let("c", b.Call(ty.u32(), core::BuiltinFn::kArrayLength, sb));
        b.Return(func);
    });

    auto* src = R"(
$B1: {  # root
  %sb:ptr<storage, array<i32>, read_write> = var undef @binding_point(0, 0)
}

%foo = @fragment func():void {
  $B2: {
    %3:u32 = arrayLength %sb
    %a:u32 = let %3
    %5:u32 = arrayLength %sb
    %b:u32 = let %5
    %7:u32 = arrayLength %sb
    %c:u32 = let %7
    ret
  }
}
)";
    ASSERT_EQ(src, str());

    auto* expect = R"(
$B1: {  # root
  %sb:hlsl.byte_address_buffer<read_write> = var undef @binding_point(0, 0)
}

%foo = @fragment func():void {
  $B2: {
    %3:ptr<function, u32, read_write> = var undef
    %4:void = %sb.GetDimensions %3
    %5:u32 = load %3
    %6:u32 = div %5, 4u
    %a:u32 = let %6
    %8:ptr<function, u32, read_write> = var undef
    %9:void = %sb.GetDimensions %8
    %10:u32 = load %8
    %11:u32 = div %10, 4u
    %b:u32 = let %11
    %13:ptr<function, u32, read_write> = var undef
    %14:void = %sb.GetDimensions %13
    %15:u32 = load %13
    %16:u32 = div %15, 4u
    %c:u32 = let %16
    ret
  }
}
)";
    Run(DecomposeStorageAccess);
    EXPECT_EQ(expect, str());
}

TEST_F(HlslWriterDecomposeStorageAccessTest, ArrayLengthMultipleStorageBuffers) {
    auto* SB1 =
        ty.Struct(mod.symbols.New("SB1"), {
                                              {mod.symbols.New("x"), ty.i32()},
                                              {mod.symbols.New("arr1"), ty.runtime_array(ty.i32())},
                                          });
    auto* SB2 = ty.Struct(mod.symbols.New("SB2"),
                          {
                              {mod.symbols.New("x"), ty.i32()},
                              {mod.symbols.New("arr2"), ty.runtime_array(ty.vec4<f32>())},
                          });
    auto* sb1 = b.Var("sb1", ty.ptr(storage, SB1));
    sb1->SetBindingPoint(0, 0);
    b.ir.root_block->Append(sb1);

    auto* sb2 = b.Var("sb2", ty.ptr(storage, SB2));
    sb2->SetBindingPoint(0, 1);
    b.ir.root_block->Append(sb2);

    auto* sb3 = b.Var("sb3", ty.ptr(storage, ty.runtime_array(ty.i32())));
    sb3->SetBindingPoint(0, 2);
    b.ir.root_block->Append(sb3);

    auto* func = b.Function("foo", ty.void_(), core::ir::Function::PipelineStage::kFragment);
    b.Append(func->Block(), [&] {
        b.Let("len1", b.Call(ty.u32(), core::BuiltinFn::kArrayLength,
                             b.Access(ty.ptr<storage, array<i32>>(), sb1, 1_u)));
        b.Let("len2", b.Call(ty.u32(), core::BuiltinFn::kArrayLength,
                             b.Access(ty.ptr<storage, array<vec4<f32>>>(), sb2, 1_u)));
        b.Let("len3", b.Call(ty.u32(), core::BuiltinFn::kArrayLength, sb3));
        b.Return(func);
    });

    auto* src = R"(
SB1 = struct @align(4) {
  x:i32 @offset(0)
  arr1:array<i32> @offset(4)
}

SB2 = struct @align(16) {
  x_1:i32 @offset(0)
  arr2:array<vec4<f32>> @offset(16)
}

$B1: {  # root
  %sb1:ptr<storage, SB1, read_write> = var undef @binding_point(0, 0)
  %sb2:ptr<storage, SB2, read_write> = var undef @binding_point(0, 1)
  %sb3:ptr<storage, array<i32>, read_write> = var undef @binding_point(0, 2)
}

%foo = @fragment func():void {
  $B2: {
    %5:ptr<storage, array<i32>, read_write> = access %sb1, 1u
    %6:u32 = arrayLength %5
    %len1:u32 = let %6
    %8:ptr<storage, array<vec4<f32>>, read_write> = access %sb2, 1u
    %9:u32 = arrayLength %8
    %len2:u32 = let %9
    %11:u32 = arrayLength %sb3
    %len3:u32 = let %11
    ret
  }
}
)";
    ASSERT_EQ(src, str());

    auto* expect = R"(
SB1 = struct @align(4) {
  x:i32 @offset(0)
  arr1:array<i32> @offset(4)
}

SB2 = struct @align(16) {
  x_1:i32 @offset(0)
  arr2:array<vec4<f32>> @offset(16)
}

$B1: {  # root
  %sb1:hlsl.byte_address_buffer<read_write> = var undef @binding_point(0, 0)
  %sb2:hlsl.byte_address_buffer<read_write> = var undef @binding_point(0, 1)
  %sb3:hlsl.byte_address_buffer<read_write> = var undef @binding_point(0, 2)
}

%foo = @fragment func():void {
  $B2: {
    %5:ptr<function, u32, read_write> = var undef
    %6:void = %sb1.GetDimensions %5
    %7:u32 = load %5
    %8:u32 = sub %7, 4u
    %9:u32 = div %8, 4u
    %len1:u32 = let %9
    %11:ptr<function, u32, read_write> = var undef
    %12:void = %sb2.GetDimensions %11
    %13:u32 = load %11
    %14:u32 = sub %13, 16u
    %15:u32 = div %14, 16u
    %len2:u32 = let %15
    %17:ptr<function, u32, read_write> = var undef
    %18:void = %sb3.GetDimensions %17
    %19:u32 = load %17
    %20:u32 = div %19, 4u
    %len3:u32 = let %20
    ret
  }
}
)";
    Run(DecomposeStorageAccess);
    EXPECT_EQ(expect, str());
}

TEST_F(HlslWriterDecomposeStorageAccessTest, AccessChainReused) {
    auto* sb = ty.Struct(mod.symbols.New("SB"), {
                                                    {mod.symbols.New("a"), ty.i32()},
                                                    {mod.symbols.New("b"), ty.vec3<f32>()},
                                                });

    auto* var = b.Var("v", storage, sb, core::Access::kReadWrite);
    var->SetBindingPoint(0, 0);
    b.ir.root_block->Append(var);

    auto* func = b.Function("foo", ty.void_(), core::ir::Function::PipelineStage::kFragment);
    b.Append(func->Block(), [&] {
        auto* x = b.Access(ty.ptr(storage, ty.vec3<f32>(), core::Access::kReadWrite), var, 1_u);
        b.Let("b", b.LoadVectorElement(x, 1_u));
        b.Let("c", b.LoadVectorElement(x, 2_u));
        b.Return(func);
    });

    auto* src = R"(
SB = struct @align(16) {
  a:i32 @offset(0)
  b:vec3<f32> @offset(16)
}

$B1: {  # root
  %v:ptr<storage, SB, read_write> = var undef @binding_point(0, 0)
}

%foo = @fragment func():void {
  $B2: {
    %3:ptr<storage, vec3<f32>, read_write> = access %v, 1u
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
  a:i32 @offset(0)
  b:vec3<f32> @offset(16)
}

$B1: {  # root
  %v:hlsl.byte_address_buffer<read_write> = var undef @binding_point(0, 0)
}

%foo = @fragment func():void {
  $B2: {
    %3:u32 = %v.Load 20u
    %4:f32 = bitcast %3
    %b:f32 = let %4
    %6:u32 = %v.Load 24u
    %7:f32 = bitcast %6
    %c:f32 = let %7
    ret
  }
}
)";

    Run(DecomposeStorageAccess);
    EXPECT_EQ(expect, str());
}

TEST_F(HlslWriterDecomposeStorageAccessTest, Determinism_MultipleUsesOfLetFromVar) {
    auto* sb =
        ty.Struct(mod.symbols.New("SB"), {
                                             {mod.symbols.New("a"), ty.array<vec4<f32>, 2>()},
                                             {mod.symbols.New("b"), ty.array<vec4<i32>, 2>()},
                                         });

    auto* var = b.Var("v", storage, sb, core::Access::kRead);
    var->SetBindingPoint(0, 0);
    b.ir.root_block->Append(var);

    auto* func = b.Function("foo", ty.void_(), core::ir::Function::PipelineStage::kFragment);
    b.Append(func->Block(), [&] {
        auto* let = b.Let("l", var);
        auto* pa =
            b.Access(ty.ptr(storage, ty.array<vec4<f32>, 2>(), core::Access::kRead), let, 0_u);
        b.Let("a", b.Load(pa));
        auto* pb =
            b.Access(ty.ptr(storage, ty.array<vec4<i32>, 2>(), core::Access::kRead), let, 1_u);
        b.Let("b", b.Load(pb));
        b.Return(func);
    });

    auto* src = R"(
SB = struct @align(16) {
  a:array<vec4<f32>, 2> @offset(0)
  b:array<vec4<i32>, 2> @offset(32)
}

$B1: {  # root
  %v:ptr<storage, SB, read> = var undef @binding_point(0, 0)
}

%foo = @fragment func():void {
  $B2: {
    %l:ptr<storage, SB, read> = let %v
    %4:ptr<storage, array<vec4<f32>, 2>, read> = access %l, 0u
    %5:array<vec4<f32>, 2> = load %4
    %a:array<vec4<f32>, 2> = let %5
    %7:ptr<storage, array<vec4<i32>, 2>, read> = access %l, 1u
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
  %v:hlsl.byte_address_buffer<read> = var undef @binding_point(0, 0)
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
%7 = func(%offset:u32):array<vec4<i32>, 2> {
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
        %13:ptr<function, vec4<i32>, read_write> = access %a_1, %idx
        %14:u32 = mul %idx, 16u
        %15:u32 = add %offset, %14
        %16:vec4<u32> = %v.Load4 %15
        %17:vec4<i32> = bitcast %16
        store %13, %17
        continue  # -> $B6
      }
      $B6: {  # continuing
        %18:u32 = add %idx, 1u
        next_iteration %18  # -> $B5
      }
    }
    %19:array<vec4<i32>, 2> = load %a_1
    ret %19
  }
}
%4 = func(%offset_1:u32):array<vec4<f32>, 2> {  # %offset_1: 'offset'
  $B8: {
    %a_2:ptr<function, array<vec4<f32>, 2>, read_write> = var array<vec4<f32>, 2>(vec4<f32>(0.0f))  # %a_2: 'a'
    loop [i: $B9, b: $B10, c: $B11] {  # loop_2
      $B9: {  # initializer
        next_iteration 0u  # -> $B10
      }
      $B10 (%idx_1:u32): {  # body
        %23:bool = gte %idx_1, 2u
        if %23 [t: $B12] {  # if_2
          $B12: {  # true
            exit_loop  # loop_2
          }
        }
        %24:ptr<function, vec4<f32>, read_write> = access %a_2, %idx_1
        %25:u32 = mul %idx_1, 16u
        %26:u32 = add %offset_1, %25
        %27:vec4<u32> = %v.Load4 %26
        %28:vec4<f32> = bitcast %27
        store %24, %28
        continue  # -> $B11
      }
      $B11: {  # continuing
        %29:u32 = add %idx_1, 1u
        next_iteration %29  # -> $B10
      }
    }
    %30:array<vec4<f32>, 2> = load %a_2
    ret %30
  }
}
)";

    Run(DecomposeStorageAccess);
    EXPECT_EQ(expect, str());
}

TEST_F(HlslWriterDecomposeStorageAccessTest, Determinism_MultipleUsesOfLetFromAccess) {
    auto* sb =
        ty.Struct(mod.symbols.New("SB"), {
                                             {mod.symbols.New("a"), ty.array<vec4<f32>, 2>()},
                                             {mod.symbols.New("b"), ty.array<vec4<i32>, 2>()},
                                         });

    auto* var = b.Var("v", storage, ty.array(sb, 2), core::Access::kRead);
    var->SetBindingPoint(0, 0);
    b.ir.root_block->Append(var);

    auto* func = b.Function("foo", ty.void_(), core::ir::Function::PipelineStage::kFragment);
    b.Append(func->Block(), [&] {
        auto* let = b.Let("l", b.Access(ty.ptr(storage, sb, core::Access::kRead), var, 0_u));
        auto* pa =
            b.Access(ty.ptr(storage, ty.array<vec4<f32>, 2>(), core::Access::kRead), let, 0_u);
        b.Let("a", b.Load(pa));
        auto* pb =
            b.Access(ty.ptr(storage, ty.array<vec4<i32>, 2>(), core::Access::kRead), let, 1_u);
        b.Let("b", b.Load(pb));
        b.Return(func);
    });

    auto* src = R"(
SB = struct @align(16) {
  a:array<vec4<f32>, 2> @offset(0)
  b:array<vec4<i32>, 2> @offset(32)
}

$B1: {  # root
  %v:ptr<storage, array<SB, 2>, read> = var undef @binding_point(0, 0)
}

%foo = @fragment func():void {
  $B2: {
    %3:ptr<storage, SB, read> = access %v, 0u
    %l:ptr<storage, SB, read> = let %3
    %5:ptr<storage, array<vec4<f32>, 2>, read> = access %l, 0u
    %6:array<vec4<f32>, 2> = load %5
    %a:array<vec4<f32>, 2> = let %6
    %8:ptr<storage, array<vec4<i32>, 2>, read> = access %l, 1u
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
  %v:hlsl.byte_address_buffer<read> = var undef @binding_point(0, 0)
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
%7 = func(%offset:u32):array<vec4<i32>, 2> {
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
        %13:ptr<function, vec4<i32>, read_write> = access %a_1, %idx
        %14:u32 = mul %idx, 16u
        %15:u32 = add %offset, %14
        %16:vec4<u32> = %v.Load4 %15
        %17:vec4<i32> = bitcast %16
        store %13, %17
        continue  # -> $B6
      }
      $B6: {  # continuing
        %18:u32 = add %idx, 1u
        next_iteration %18  # -> $B5
      }
    }
    %19:array<vec4<i32>, 2> = load %a_1
    ret %19
  }
}
%4 = func(%offset_1:u32):array<vec4<f32>, 2> {  # %offset_1: 'offset'
  $B8: {
    %a_2:ptr<function, array<vec4<f32>, 2>, read_write> = var array<vec4<f32>, 2>(vec4<f32>(0.0f))  # %a_2: 'a'
    loop [i: $B9, b: $B10, c: $B11] {  # loop_2
      $B9: {  # initializer
        next_iteration 0u  # -> $B10
      }
      $B10 (%idx_1:u32): {  # body
        %23:bool = gte %idx_1, 2u
        if %23 [t: $B12] {  # if_2
          $B12: {  # true
            exit_loop  # loop_2
          }
        }
        %24:ptr<function, vec4<f32>, read_write> = access %a_2, %idx_1
        %25:u32 = mul %idx_1, 16u
        %26:u32 = add %offset_1, %25
        %27:vec4<u32> = %v.Load4 %26
        %28:vec4<f32> = bitcast %27
        store %24, %28
        continue  # -> $B11
      }
      $B11: {  # continuing
        %29:u32 = add %idx_1, 1u
        next_iteration %29  # -> $B10
      }
    }
    %30:array<vec4<f32>, 2> = load %a_2
    ret %30
  }
}
)";

    Run(DecomposeStorageAccess);
    EXPECT_EQ(expect, str());
}

TEST_F(HlslWriterDecomposeStorageAccessTest, Determinism_MultipleUsesOfAccess) {
    auto* sb =
        ty.Struct(mod.symbols.New("SB"), {
                                             {mod.symbols.New("a"), ty.array<vec4<f32>, 2>()},
                                             {mod.symbols.New("b"), ty.array<vec4<i32>, 2>()},
                                         });

    auto* var = b.Var("v", storage, ty.array(sb, 2), core::Access::kRead);
    var->SetBindingPoint(0, 0);
    b.ir.root_block->Append(var);

    auto* func = b.Function("foo", ty.void_(), core::ir::Function::PipelineStage::kFragment);
    b.Append(func->Block(), [&] {
        auto* access = b.Access(ty.ptr(storage, sb, core::Access::kRead), var, 0_u);
        auto* pa =
            b.Access(ty.ptr(storage, ty.array<vec4<f32>, 2>(), core::Access::kRead), access, 0_u);
        b.Let("a", b.Load(pa));
        auto* pb =
            b.Access(ty.ptr(storage, ty.array<vec4<i32>, 2>(), core::Access::kRead), access, 1_u);
        b.Let("b", b.Load(pb));
        b.Return(func);
    });

    auto* src = R"(
SB = struct @align(16) {
  a:array<vec4<f32>, 2> @offset(0)
  b:array<vec4<i32>, 2> @offset(32)
}

$B1: {  # root
  %v:ptr<storage, array<SB, 2>, read> = var undef @binding_point(0, 0)
}

%foo = @fragment func():void {
  $B2: {
    %3:ptr<storage, SB, read> = access %v, 0u
    %4:ptr<storage, array<vec4<f32>, 2>, read> = access %3, 0u
    %5:array<vec4<f32>, 2> = load %4
    %a:array<vec4<f32>, 2> = let %5
    %7:ptr<storage, array<vec4<i32>, 2>, read> = access %3, 1u
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
  %v:hlsl.byte_address_buffer<read> = var undef @binding_point(0, 0)
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
%7 = func(%offset:u32):array<vec4<i32>, 2> {
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
        %13:ptr<function, vec4<i32>, read_write> = access %a_1, %idx
        %14:u32 = mul %idx, 16u
        %15:u32 = add %offset, %14
        %16:vec4<u32> = %v.Load4 %15
        %17:vec4<i32> = bitcast %16
        store %13, %17
        continue  # -> $B6
      }
      $B6: {  # continuing
        %18:u32 = add %idx, 1u
        next_iteration %18  # -> $B5
      }
    }
    %19:array<vec4<i32>, 2> = load %a_1
    ret %19
  }
}
%4 = func(%offset_1:u32):array<vec4<f32>, 2> {  # %offset_1: 'offset'
  $B8: {
    %a_2:ptr<function, array<vec4<f32>, 2>, read_write> = var array<vec4<f32>, 2>(vec4<f32>(0.0f))  # %a_2: 'a'
    loop [i: $B9, b: $B10, c: $B11] {  # loop_2
      $B9: {  # initializer
        next_iteration 0u  # -> $B10
      }
      $B10 (%idx_1:u32): {  # body
        %23:bool = gte %idx_1, 2u
        if %23 [t: $B12] {  # if_2
          $B12: {  # true
            exit_loop  # loop_2
          }
        }
        %24:ptr<function, vec4<f32>, read_write> = access %a_2, %idx_1
        %25:u32 = mul %idx_1, 16u
        %26:u32 = add %offset_1, %25
        %27:vec4<u32> = %v.Load4 %26
        %28:vec4<f32> = bitcast %27
        store %24, %28
        continue  # -> $B11
      }
      $B11: {  # continuing
        %29:u32 = add %idx_1, 1u
        next_iteration %29  # -> $B10
      }
    }
    %30:array<vec4<f32>, 2> = load %a_2
    ret %30
  }
}
)";

    Run(DecomposeStorageAccess);
    EXPECT_EQ(expect, str());
}

}  // namespace
}  // namespace tint::hlsl::writer::raise
