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

#include "src/tint/lang/glsl/writer/raise/builtin_polyfill.h"

#include <string>

#include "gtest/gtest.h"
#include "src/tint/lang/core/fluent_types.h"
#include "src/tint/lang/core/ir/transform/helper_test.h"
#include "src/tint/lang/core/number.h"
#include "src/tint/lang/core/type/builtin_structs.h"
#include "src/tint/lang/core/type/depth_multisampled_texture.h"
#include "src/tint/lang/core/type/depth_texture.h"
#include "src/tint/lang/core/type/multisampled_texture.h"
#include "src/tint/lang/core/type/sampled_texture.h"
#include "src/tint/lang/core/type/storage_texture.h"

using namespace tint::core::fluent_types;     // NOLINT
using namespace tint::core::number_suffixes;  // NOLINT

namespace tint::glsl::writer::raise {
namespace {

using GlslWriter_BuiltinPolyfillTest = core::ir::transform::TransformTest;

TEST_F(GlslWriter_BuiltinPolyfillTest, SelectScalar) {
    auto* func = b.Function("foo", ty.f32());
    b.Append(func->Block(),
             [&] { b.Return(func, b.Call<f32>(core::BuiltinFn::kSelect, 2_f, 1_f, false)); });

    auto* src = R"(
%foo = func():f32 {
  $B1: {
    %2:f32 = select 2.0f, 1.0f, false
    ret %2
  }
}
)";
    EXPECT_EQ(src, str());

    auto* expect = R"(
%foo = func():f32 {
  $B1: {
    %2:f32 = glsl.mix 2.0f, 1.0f, false
    ret %2
  }
}
)";

    Run(BuiltinPolyfill);
    EXPECT_EQ(expect, str());
}

TEST_F(GlslWriter_BuiltinPolyfillTest, SelectVector) {
    auto* func = b.Function("foo", ty.vec3<f32>());
    b.Append(func->Block(), [&] {
        auto* false_ = b.Splat(ty.vec3<f32>(), 2_f);
        auto* true_ = b.Splat(ty.vec3<f32>(), 1_f);
        auto* cond = b.Splat(ty.vec3<bool>(), false);
        b.Return(func, b.Call<vec3<f32>>(core::BuiltinFn::kSelect, false_, true_, cond));
    });

    auto* src = R"(
%foo = func():vec3<f32> {
  $B1: {
    %2:vec3<f32> = select vec3<f32>(2.0f), vec3<f32>(1.0f), vec3<bool>(false)
    ret %2
  }
}
)";
    EXPECT_EQ(src, str());

    auto* expect = R"(
%foo = func():vec3<f32> {
  $B1: {
    %2:vec3<f32> = glsl.mix vec3<f32>(2.0f), vec3<f32>(1.0f), vec3<bool>(false)
    ret %2
  }
}
)";

    Run(BuiltinPolyfill);
    EXPECT_EQ(expect, str());
}

TEST_F(GlslWriter_BuiltinPolyfillTest, StorageBarrier) {
    auto* func = b.ComputeFunction("foo");
    b.Append(func->Block(), [&] {
        b.Call(ty.void_(), core::BuiltinFn::kStorageBarrier);
        b.Return(func);
    });

    auto* src = R"(
%foo = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    %2:void = storageBarrier
    ret
  }
}
)";
    EXPECT_EQ(src, str());

    auto* expect = R"(
%foo = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    %2:void = glsl.memoryBarrierBuffer
    %3:void = glsl.barrier
    ret
  }
}
)";

    Run(BuiltinPolyfill);
    EXPECT_EQ(expect, str());
}

TEST_F(GlslWriter_BuiltinPolyfillTest, TextureBarrier) {
    auto* func = b.ComputeFunction("foo");
    b.Append(func->Block(), [&] {
        b.Call(ty.void_(), core::BuiltinFn::kTextureBarrier);
        b.Return(func);
    });

    auto* src = R"(
%foo = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    %2:void = textureBarrier
    ret
  }
}
)";
    EXPECT_EQ(src, str());

    auto* expect = R"(
%foo = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    %2:void = glsl.memoryBarrierImage
    %3:void = glsl.barrier
    ret
  }
}
)";

    Run(BuiltinPolyfill);
    EXPECT_EQ(expect, str());
}

TEST_F(GlslWriter_BuiltinPolyfillTest, WorkgroupBarrier) {
    auto* func = b.ComputeFunction("foo");
    b.Append(func->Block(), [&] {
        b.Call(ty.void_(), core::BuiltinFn::kWorkgroupBarrier);
        b.Return(func);
    });

    auto* src = R"(
%foo = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    %2:void = workgroupBarrier
    ret
  }
}
)";
    EXPECT_EQ(src, str());

    auto* expect = R"(
%foo = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    %2:void = glsl.barrier
    ret
  }
}
)";

    Run(BuiltinPolyfill);
    EXPECT_EQ(expect, str());
}

TEST_F(GlslWriter_BuiltinPolyfillTest, AtomicCompareExchangeWeak) {
    auto* var = b.Var("v", workgroup, ty.atomic<i32>(), core::Access::kReadWrite);
    b.ir.root_block->Append(var);

    auto* func = b.ComputeFunction("foo");
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
  %v:ptr<workgroup, atomic<i32>, read_write> = var undef
}

%foo = @compute @workgroup_size(1u, 1u, 1u) func():void {
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
  %v:ptr<workgroup, atomic<i32>, read_write> = var undef
}

%foo = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B2: {
    %3:i32 = bitcast 123i
    %4:i32 = bitcast 345i
    %5:i32 = glsl.atomicCompSwap %v, %3, %4
    %6:bool = eq %5, 123i
    %7:__atomic_compare_exchange_result_i32 = construct %5, %6
    %x:__atomic_compare_exchange_result_i32 = let %7
    ret
  }
}
)";

    Run(BuiltinPolyfill);
    EXPECT_EQ(expect, str());
}

TEST_F(GlslWriter_BuiltinPolyfillTest, AtomicSub) {
    auto* var = b.Var("v", workgroup, ty.atomic<i32>(), core::Access::kReadWrite);
    b.ir.root_block->Append(var);

    auto* func = b.ComputeFunction("foo");
    b.Append(func->Block(), [&] {
        b.Let("x", b.Call(ty.i32(), core::BuiltinFn::kAtomicSub, var, 123_i));
        b.Return(func);
    });

    auto* src = R"(
$B1: {  # root
  %v:ptr<workgroup, atomic<i32>, read_write> = var undef
}

%foo = @compute @workgroup_size(1u, 1u, 1u) func():void {
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
  %v:ptr<workgroup, atomic<i32>, read_write> = var undef
}

%foo = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B2: {
    %3:i32 = negation 123i
    %4:i32 = atomicAdd %v, %3
    %x:i32 = let %4
    ret
  }
}
)";

    Run(BuiltinPolyfill);
    EXPECT_EQ(expect, str());
}

TEST_F(GlslWriter_BuiltinPolyfillTest, AtomicSub_u32) {
    auto* var = b.Var("v", workgroup, ty.atomic<u32>(), core::Access::kReadWrite);
    b.ir.root_block->Append(var);

    auto* func = b.ComputeFunction("foo");
    b.Append(func->Block(), [&] {
        b.Let("x", b.Call(ty.u32(), core::BuiltinFn::kAtomicSub, var, 123_u));
        b.Return(func);
    });

    auto* src = R"(
$B1: {  # root
  %v:ptr<workgroup, atomic<u32>, read_write> = var undef
}

%foo = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B2: {
    %3:u32 = atomicSub %v, 123u
    %x:u32 = let %3
    ret
  }
}
)";
    EXPECT_EQ(src, str());

    auto* expect = R"(
$B1: {  # root
  %v:ptr<workgroup, atomic<u32>, read_write> = var undef
}

%foo = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B2: {
    %3:u32 = glsl.atomicSub %v, 123u
    %x:u32 = let %3
    ret
  }
}
)";

    Run(BuiltinPolyfill);
    EXPECT_EQ(expect, str());
}

TEST_F(GlslWriter_BuiltinPolyfillTest, AtomicLoad) {
    auto* var = b.Var("v", workgroup, ty.atomic<i32>(), core::Access::kReadWrite);
    b.ir.root_block->Append(var);

    auto* func = b.ComputeFunction("foo");
    b.Append(func->Block(), [&] {
        b.Let("x", b.Call(ty.i32(), core::BuiltinFn::kAtomicLoad, var));
        b.Return(func);
    });

    auto* src = R"(
$B1: {  # root
  %v:ptr<workgroup, atomic<i32>, read_write> = var undef
}

%foo = @compute @workgroup_size(1u, 1u, 1u) func():void {
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
  %v:ptr<workgroup, atomic<i32>, read_write> = var undef
}

%foo = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B2: {
    %3:i32 = atomicOr %v, 0i
    %x:i32 = let %3
    ret
  }
}
)";

    Run(BuiltinPolyfill);
    EXPECT_EQ(expect, str());
}

TEST_F(GlslWriter_BuiltinPolyfillTest, CountOneBits) {
    auto* func = b.Function("foo", ty.void_(), core::ir::Function::PipelineStage::kFragment);
    b.Append(func->Block(), [&] {
        b.Let("x", b.Call(ty.u32(), core::BuiltinFn::kCountOneBits, 1_u));
        b.Return(func);
    });

    auto* src = R"(
%foo = @fragment func():void {
  $B1: {
    %2:u32 = countOneBits 1u
    %x:u32 = let %2
    ret
  }
}
)";
    EXPECT_EQ(src, str());

    auto* expect = R"(
%foo = @fragment func():void {
  $B1: {
    %2:i32 = glsl.bitCount 1u
    %3:u32 = convert %2
    %x:u32 = let %3
    ret
  }
}
)";

    Run(BuiltinPolyfill);
    EXPECT_EQ(expect, str());
}

TEST_F(GlslWriter_BuiltinPolyfillTest, ExtractBits) {
    auto* func = b.Function("foo", ty.void_(), core::ir::Function::PipelineStage::kFragment);
    b.Append(func->Block(), [&] {
        b.Let("x", b.Call(ty.u32(), core::BuiltinFn::kExtractBits, 1_u, 2_u, 3_u));
        b.Return(func);
    });

    auto* src = R"(
%foo = @fragment func():void {
  $B1: {
    %2:u32 = extractBits 1u, 2u, 3u
    %x:u32 = let %2
    ret
  }
}
)";
    EXPECT_EQ(src, str());

    auto* expect = R"(
%foo = @fragment func():void {
  $B1: {
    %2:i32 = convert 2u
    %3:i32 = convert 3u
    %4:u32 = glsl.bitfieldExtract 1u, %2, %3
    %x:u32 = let %4
    ret
  }
}
)";

    Run(BuiltinPolyfill);
    EXPECT_EQ(expect, str());
}

TEST_F(GlslWriter_BuiltinPolyfillTest, InsertBits) {
    auto* func = b.Function("foo", ty.void_(), core::ir::Function::PipelineStage::kFragment);
    b.Append(func->Block(), [&] {
        b.Let("x", b.Call(ty.u32(), core::BuiltinFn::kInsertBits, 1_u, 2_u, 3_u, 4_u));
        b.Return(func);
    });

    auto* src = R"(
%foo = @fragment func():void {
  $B1: {
    %2:u32 = insertBits 1u, 2u, 3u, 4u
    %x:u32 = let %2
    ret
  }
}
)";
    EXPECT_EQ(src, str());

    auto* expect = R"(
%foo = @fragment func():void {
  $B1: {
    %2:i32 = convert 3u
    %3:i32 = convert 4u
    %4:u32 = glsl.bitfieldInsert 1u, 2u, %2, %3
    %x:u32 = let %4
    ret
  }
}
)";

    Run(BuiltinPolyfill);
    EXPECT_EQ(expect, str());
}

TEST_F(GlslWriter_BuiltinPolyfillTest, FMA_f32) {
    auto* func = b.Function("foo", ty.void_(), core::ir::Function::PipelineStage::kFragment);
    b.Append(func->Block(), [&] {
        auto* x = b.Splat(ty.vec3<f32>(), 1_f);
        auto* y = b.Splat(ty.vec3<f32>(), 2_f);
        auto* z = b.Splat(ty.vec3<f32>(), 3_f);

        b.Let("x", b.Call(ty.vec3<f32>(), core::BuiltinFn::kFma, x, y, z));
        b.Return(func);
    });

    auto* src = R"(
%foo = @fragment func():void {
  $B1: {
    %2:vec3<f32> = fma vec3<f32>(1.0f), vec3<f32>(2.0f), vec3<f32>(3.0f)
    %x:vec3<f32> = let %2
    ret
  }
}
)";
    ASSERT_EQ(src, str());

    auto* expect = R"(
%foo = @fragment func():void {
  $B1: {
    %2:vec3<f32> = mul vec3<f32>(1.0f), vec3<f32>(2.0f)
    %3:vec3<f32> = add %2, vec3<f32>(3.0f)
    %x:vec3<f32> = let %3
    ret
  }
}
)";

    Run(BuiltinPolyfill);
    EXPECT_EQ(expect, str());
}

TEST_F(GlslWriter_BuiltinPolyfillTest, FMA_f16) {
    auto* func = b.Function("foo", ty.void_(), core::ir::Function::PipelineStage::kFragment);
    b.Append(func->Block(), [&] {
        auto* x = b.Splat(ty.vec3<f16>(), 1_h);
        auto* y = b.Splat(ty.vec3<f16>(), 2_h);
        auto* z = b.Splat(ty.vec3<f16>(), 3_h);

        b.Let("x", b.Call(ty.vec3<f16>(), core::BuiltinFn::kFma, x, y, z));
        b.Return(func);
    });

    auto* src = R"(
%foo = @fragment func():void {
  $B1: {
    %2:vec3<f16> = fma vec3<f16>(1.0h), vec3<f16>(2.0h), vec3<f16>(3.0h)
    %x:vec3<f16> = let %2
    ret
  }
}
)";
    ASSERT_EQ(src, str());

    auto* expect = R"(
%foo = @fragment func():void {
  $B1: {
    %2:vec3<f16> = mul vec3<f16>(1.0h), vec3<f16>(2.0h)
    %3:vec3<f16> = add %2, vec3<f16>(3.0h)
    %x:vec3<f16> = let %3
    ret
  }
}
)";

    Run(BuiltinPolyfill);
    EXPECT_EQ(expect, str());
}

TEST_F(GlslWriter_BuiltinPolyfillTest, ArrayLength) {
    auto* sb = ty.Struct(mod.symbols.New("SB"), {
                                                    {mod.symbols.New("b"), ty.array<u32>()},
                                                });

    auto* var = b.Var("v", storage, sb, core::Access::kReadWrite);
    var->SetBindingPoint(0, 0);
    b.ir.root_block->Append(var);

    auto* func = b.Function("foo", ty.void_(), core::ir::Function::PipelineStage::kFragment);
    b.Append(func->Block(), [&] {
        auto* ary = b.Access(ty.ptr<storage, array<u32>, read_write>(), var, 0_u);
        b.Let("x", b.Call(ty.u32(), core::BuiltinFn::kArrayLength, ary));
        b.Return(func);
    });

    auto* src = R"(
SB = struct @align(4) {
  b:array<u32> @offset(0)
}

$B1: {  # root
  %v:ptr<storage, SB, read_write> = var undef @binding_point(0, 0)
}

%foo = @fragment func():void {
  $B2: {
    %3:ptr<storage, array<u32>, read_write> = access %v, 0u
    %4:u32 = arrayLength %3
    %x:u32 = let %4
    ret
  }
}
)";
    ASSERT_EQ(src, str());

    auto* expect = R"(
SB = struct @align(4) {
  b:array<u32> @offset(0)
}

$B1: {  # root
  %v:ptr<storage, SB, read_write> = var undef @binding_point(0, 0)
}

%foo = @fragment func():void {
  $B2: {
    %3:ptr<storage, array<u32>, read_write> = access %v, 0u
    %4:i32 = %3.length
    %5:u32 = convert %4
    %x:u32 = let %5
    ret
  }
}
)";

    Run(BuiltinPolyfill);
    EXPECT_EQ(expect, str());
}

TEST_F(GlslWriter_BuiltinPolyfillTest, AnyScalar) {
    auto* func = b.Function("foo", ty.void_(), core::ir::Function::PipelineStage::kFragment);
    b.Append(func->Block(), [&] {
        b.Let("x", b.Call(ty.bool_(), core::BuiltinFn::kAny, true));
        b.Return(func);
    });

    auto* src = R"(
%foo = @fragment func():void {
  $B1: {
    %2:bool = any true
    %x:bool = let %2
    ret
  }
}
)";
    ASSERT_EQ(src, str());

    auto* expect = R"(
%foo = @fragment func():void {
  $B1: {
    %x:bool = let true
    ret
  }
}
)";

    Run(BuiltinPolyfill);
    EXPECT_EQ(expect, str());
}

TEST_F(GlslWriter_BuiltinPolyfillTest, AllScalar) {
    auto* func = b.Function("foo", ty.void_(), core::ir::Function::PipelineStage::kFragment);
    b.Append(func->Block(), [&] {
        b.Let("x", b.Call(ty.bool_(), core::BuiltinFn::kAll, false));
        b.Return(func);
    });

    auto* src = R"(
%foo = @fragment func():void {
  $B1: {
    %2:bool = all false
    %x:bool = let %2
    ret
  }
}
)";
    ASSERT_EQ(src, str());

    auto* expect = R"(
%foo = @fragment func():void {
  $B1: {
    %x:bool = let false
    ret
  }
}
)";

    Run(BuiltinPolyfill);
    EXPECT_EQ(expect, str());
}

TEST_F(GlslWriter_BuiltinPolyfillTest, DotF32) {
    auto* func = b.Function("foo", ty.void_(), core::ir::Function::PipelineStage::kFragment);
    b.Append(func->Block(), [&] {
        auto* x = b.Let("x", b.Splat(ty.vec3<f32>(), 2_f));
        auto* y = b.Let("y", b.Splat(ty.vec3<f32>(), 3_f));
        b.Let("z", b.Call(ty.f32(), core::BuiltinFn::kDot, x, y));
        b.Return(func);
    });

    auto* src = R"(
%foo = @fragment func():void {
  $B1: {
    %x:vec3<f32> = let vec3<f32>(2.0f)
    %y:vec3<f32> = let vec3<f32>(3.0f)
    %4:f32 = dot %x, %y
    %z:f32 = let %4
    ret
  }
}
)";
    ASSERT_EQ(src, str());

    auto* expected = R"(
%foo = @fragment func():void {
  $B1: {
    %x:vec3<f32> = let vec3<f32>(2.0f)
    %y:vec3<f32> = let vec3<f32>(3.0f)
    %4:f32 = glsl.dot %x, %y
    %z:f32 = let %4
    ret
  }
}
)";
    Run(BuiltinPolyfill);
    EXPECT_EQ(expected, str());
}

TEST_F(GlslWriter_BuiltinPolyfillTest, DotF16) {
    auto* func = b.Function("foo", ty.void_(), core::ir::Function::PipelineStage::kFragment);
    b.Append(func->Block(), [&] {
        auto* x = b.Let("x", b.Splat(ty.vec4<f16>(), 2_h));
        auto* y = b.Let("y", b.Splat(ty.vec4<f16>(), 3_h));
        b.Let("z", b.Call(ty.f16(), core::BuiltinFn::kDot, x, y));
        b.Return(func);
    });

    auto* src = R"(
%foo = @fragment func():void {
  $B1: {
    %x:vec4<f16> = let vec4<f16>(2.0h)
    %y:vec4<f16> = let vec4<f16>(3.0h)
    %4:f16 = dot %x, %y
    %z:f16 = let %4
    ret
  }
}
)";
    ASSERT_EQ(src, str());

    auto* expected = R"(
%foo = @fragment func():void {
  $B1: {
    %x:vec4<f16> = let vec4<f16>(2.0h)
    %y:vec4<f16> = let vec4<f16>(3.0h)
    %4:f16 = glsl.dot %x, %y
    %z:f16 = let %4
    ret
  }
}
)";

    Run(BuiltinPolyfill);
    EXPECT_EQ(expected, str());
}

TEST_F(GlslWriter_BuiltinPolyfillTest, DotI32) {
    auto* func = b.Function("foo", ty.void_(), core::ir::Function::PipelineStage::kFragment);
    b.Append(func->Block(), [&] {
        auto* x = b.Let("x", b.Splat(ty.vec4<i32>(), 2_i));
        auto* y = b.Let("y", b.Splat(ty.vec4<i32>(), 3_i));
        b.Let("z", b.Call(ty.i32(), core::BuiltinFn::kDot, x, y));
        b.Return(func);
    });

    auto* src = R"(
%foo = @fragment func():void {
  $B1: {
    %x:vec4<i32> = let vec4<i32>(2i)
    %y:vec4<i32> = let vec4<i32>(3i)
    %4:i32 = dot %x, %y
    %z:i32 = let %4
    ret
  }
}
)";
    ASSERT_EQ(src, str());

    auto* expected = R"(
%foo = @fragment func():void {
  $B1: {
    %x:vec4<i32> = let vec4<i32>(2i)
    %y:vec4<i32> = let vec4<i32>(3i)
    %4:i32 = call %tint_int_dot, %x, %y
    %z:i32 = let %4
    ret
  }
}
%tint_int_dot = func(%x_1:vec4<i32>, %y_1:vec4<i32>):i32 {  # %x_1: 'x', %y_1: 'y'
  $B2: {
    %9:i32 = swizzle %x_1, x
    %10:i32 = swizzle %y_1, x
    %11:i32 = mul %9, %10
    %12:i32 = swizzle %x_1, y
    %13:i32 = swizzle %y_1, y
    %14:i32 = mul %12, %13
    %15:i32 = add %11, %14
    %16:i32 = swizzle %x_1, z
    %17:i32 = swizzle %y_1, z
    %18:i32 = mul %16, %17
    %19:i32 = add %15, %18
    %20:i32 = swizzle %x_1, w
    %21:i32 = swizzle %y_1, w
    %22:i32 = mul %20, %21
    %23:i32 = add %19, %22
    ret %23
  }
}
)";

    Run(BuiltinPolyfill);
    EXPECT_EQ(expected, str());
}

TEST_F(GlslWriter_BuiltinPolyfillTest, DotU32) {
    auto* func = b.Function("foo", ty.void_(), core::ir::Function::PipelineStage::kFragment);
    b.Append(func->Block(), [&] {
        auto* x = b.Let("x", b.Splat(ty.vec2<u32>(), 2_u));
        auto* y = b.Let("y", b.Splat(ty.vec2<u32>(), 3_u));
        b.Let("z", b.Call(ty.u32(), core::BuiltinFn::kDot, x, y));
        b.Return(func);
    });

    auto* src = R"(
%foo = @fragment func():void {
  $B1: {
    %x:vec2<u32> = let vec2<u32>(2u)
    %y:vec2<u32> = let vec2<u32>(3u)
    %4:u32 = dot %x, %y
    %z:u32 = let %4
    ret
  }
}
)";
    ASSERT_EQ(src, str());

    auto* expected = R"(
%foo = @fragment func():void {
  $B1: {
    %x:vec2<u32> = let vec2<u32>(2u)
    %y:vec2<u32> = let vec2<u32>(3u)
    %4:u32 = call %tint_int_dot, %x, %y
    %z:u32 = let %4
    ret
  }
}
%tint_int_dot = func(%x_1:vec2<u32>, %y_1:vec2<u32>):u32 {  # %x_1: 'x', %y_1: 'y'
  $B2: {
    %9:u32 = swizzle %x_1, x
    %10:u32 = swizzle %y_1, x
    %11:u32 = mul %9, %10
    %12:u32 = swizzle %x_1, y
    %13:u32 = swizzle %y_1, y
    %14:u32 = mul %12, %13
    %15:u32 = add %11, %14
    ret %15
  }
}
)";

    Run(BuiltinPolyfill);
    EXPECT_EQ(expected, str());
}

TEST_F(GlslWriter_BuiltinPolyfillTest, Modf_Scalar) {
    auto* value = b.FunctionParam<f32>("value");
    auto* func = b.Function("foo", ty.f32());
    func->SetParams({value});
    b.Append(func->Block(), [&] {
        auto* result = b.Call(core::type::CreateModfResult(ty, mod.symbols, ty.f32()),
                              core::BuiltinFn::kModf, value);
        auto* fract = b.Access<f32>(result, 0_u);
        auto* whole = b.Access<f32>(result, 1_u);
        b.Return(func, b.Add<f32>(fract, whole));
    });

    auto* src = R"(
__modf_result_f32 = struct @align(4) {
  fract:f32 @offset(0)
  whole:f32 @offset(4)
}

%foo = func(%value:f32):f32 {
  $B1: {
    %3:__modf_result_f32 = modf %value
    %4:f32 = access %3, 0u
    %5:f32 = access %3, 1u
    %6:f32 = add %4, %5
    ret %6
  }
}
)";
    EXPECT_EQ(src, str());

    auto* expect = R"(
__modf_result_f32 = struct @align(4) {
  fract:f32 @offset(0)
  whole:f32 @offset(4)
}

%foo = func(%value:f32):f32 {
  $B1: {
    %3:ptr<function, __modf_result_f32, read_write> = var undef
    %4:ptr<function, f32, read_write> = access %3, 1u
    %5:f32 = glsl.modf %value, %4
    %6:ptr<function, f32, read_write> = access %3, 0u
    store %6, %5
    %7:__modf_result_f32 = load %3
    %8:f32 = access %7, 0u
    %9:f32 = access %7, 1u
    %10:f32 = add %8, %9
    ret %10
  }
}
)";

    Run(BuiltinPolyfill);
    EXPECT_EQ(expect, str());
}

TEST_F(GlslWriter_BuiltinPolyfillTest, Modf_Vector) {
    auto* value = b.FunctionParam<vec4<f32>>("value");
    auto* func = b.Function("foo", ty.vec4<f32>());
    func->SetParams({value});
    b.Append(func->Block(), [&] {
        auto* result = b.Call(core::type::CreateModfResult(ty, mod.symbols, ty.vec4<f32>()),
                              core::BuiltinFn::kModf, value);
        auto* fract = b.Access<vec4<f32>>(result, 0_u);
        auto* whole = b.Access<vec4<f32>>(result, 1_u);
        b.Return(func, b.Add<vec4<f32>>(fract, whole));
    });

    auto* src = R"(
__modf_result_vec4_f32 = struct @align(16) {
  fract:vec4<f32> @offset(0)
  whole:vec4<f32> @offset(16)
}

%foo = func(%value:vec4<f32>):vec4<f32> {
  $B1: {
    %3:__modf_result_vec4_f32 = modf %value
    %4:vec4<f32> = access %3, 0u
    %5:vec4<f32> = access %3, 1u
    %6:vec4<f32> = add %4, %5
    ret %6
  }
}
)";
    EXPECT_EQ(src, str());

    auto* expect = R"(
__modf_result_vec4_f32 = struct @align(16) {
  fract:vec4<f32> @offset(0)
  whole:vec4<f32> @offset(16)
}

%foo = func(%value:vec4<f32>):vec4<f32> {
  $B1: {
    %3:ptr<function, __modf_result_vec4_f32, read_write> = var undef
    %4:ptr<function, vec4<f32>, read_write> = access %3, 1u
    %5:vec4<f32> = glsl.modf %value, %4
    %6:ptr<function, vec4<f32>, read_write> = access %3, 0u
    store %6, %5
    %7:__modf_result_vec4_f32 = load %3
    %8:vec4<f32> = access %7, 0u
    %9:vec4<f32> = access %7, 1u
    %10:vec4<f32> = add %8, %9
    ret %10
  }
}
)";

    Run(BuiltinPolyfill);
    EXPECT_EQ(expect, str());
}

TEST_F(GlslWriter_BuiltinPolyfillTest, Frexp_Scalar) {
    auto* value = b.FunctionParam<f32>("value");
    auto* func = b.Function("foo", ty.f32());
    func->SetParams({value});
    b.Append(func->Block(), [&] {
        auto* result = b.Call(core::type::CreateFrexpResult(ty, mod.symbols, ty.f32()),
                              core::BuiltinFn::kFrexp, value);
        auto* fract = b.Access<f32>(result, 0_u);
        auto* exp = b.Access<i32>(result, 1_u);
        b.Return(func, b.Add<f32>(fract, b.Convert<f32>(exp)));
    });

    auto* src = R"(
__frexp_result_f32 = struct @align(4) {
  fract:f32 @offset(0)
  exp:i32 @offset(4)
}

%foo = func(%value:f32):f32 {
  $B1: {
    %3:__frexp_result_f32 = frexp %value
    %4:f32 = access %3, 0u
    %5:i32 = access %3, 1u
    %6:f32 = convert %5
    %7:f32 = add %4, %6
    ret %7
  }
}
)";
    EXPECT_EQ(src, str());

    auto* expect = R"(
__frexp_result_f32 = struct @align(4) {
  fract:f32 @offset(0)
  exp:i32 @offset(4)
}

%foo = func(%value:f32):f32 {
  $B1: {
    %3:ptr<function, __frexp_result_f32, read_write> = var undef
    %4:ptr<function, i32, read_write> = access %3, 1u
    %5:f32 = glsl.frexp %value, %4
    %6:ptr<function, f32, read_write> = access %3, 0u
    store %6, %5
    %7:__frexp_result_f32 = load %3
    %8:f32 = access %7, 0u
    %9:i32 = access %7, 1u
    %10:f32 = convert %9
    %11:f32 = add %8, %10
    ret %11
  }
}
)";

    Run(BuiltinPolyfill);
    EXPECT_EQ(expect, str());
}

TEST_F(GlslWriter_BuiltinPolyfillTest, Frexp_Vector) {
    auto* value = b.FunctionParam<vec4<f32>>("value");
    auto* func = b.Function("foo", ty.vec4<f32>());
    func->SetParams({value});
    b.Append(func->Block(), [&] {
        auto* result = b.Call(core::type::CreateFrexpResult(ty, mod.symbols, ty.vec4<f32>()),
                              core::BuiltinFn::kFrexp, value);
        auto* fract = b.Access<vec4<f32>>(result, 0_u);
        auto* exp = b.Access<vec4<i32>>(result, 1_u);
        b.Return(func, b.Add<vec4<f32>>(fract, b.Convert<vec4<f32>>(exp)));
    });

    auto* src = R"(
__frexp_result_vec4_f32 = struct @align(16) {
  fract:vec4<f32> @offset(0)
  exp:vec4<i32> @offset(16)
}

%foo = func(%value:vec4<f32>):vec4<f32> {
  $B1: {
    %3:__frexp_result_vec4_f32 = frexp %value
    %4:vec4<f32> = access %3, 0u
    %5:vec4<i32> = access %3, 1u
    %6:vec4<f32> = convert %5
    %7:vec4<f32> = add %4, %6
    ret %7
  }
}
)";
    EXPECT_EQ(src, str());

    auto* expect = R"(
__frexp_result_vec4_f32 = struct @align(16) {
  fract:vec4<f32> @offset(0)
  exp:vec4<i32> @offset(16)
}

%foo = func(%value:vec4<f32>):vec4<f32> {
  $B1: {
    %3:ptr<function, __frexp_result_vec4_f32, read_write> = var undef
    %4:ptr<function, vec4<i32>, read_write> = access %3, 1u
    %5:vec4<f32> = glsl.frexp %value, %4
    %6:ptr<function, vec4<f32>, read_write> = access %3, 0u
    store %6, %5
    %7:__frexp_result_vec4_f32 = load %3
    %8:vec4<f32> = access %7, 0u
    %9:vec4<i32> = access %7, 1u
    %10:vec4<f32> = convert %9
    %11:vec4<f32> = add %8, %10
    ret %11
  }
}
)";

    Run(BuiltinPolyfill);
    EXPECT_EQ(expect, str());
}

TEST_F(GlslWriter_BuiltinPolyfillTest, AbsScalar) {
    auto* func = b.Function("foo", ty.void_(), core::ir::Function::PipelineStage::kFragment);
    b.Append(func->Block(), [&] {
        b.Let("x", b.Call(ty.u32(), core::BuiltinFn::kAbs, 2_u));
        b.Return(func);
    });

    auto* src = R"(
%foo = @fragment func():void {
  $B1: {
    %2:u32 = abs 2u
    %x:u32 = let %2
    ret
  }
}
)";
    ASSERT_EQ(src, str());

    auto* expect = R"(
%foo = @fragment func():void {
  $B1: {
    %x:u32 = let 2u
    ret
  }
}
)";

    Run(BuiltinPolyfill);
    EXPECT_EQ(expect, str());
}

TEST_F(GlslWriter_BuiltinPolyfillTest, AbsVector) {
    auto* func = b.Function("foo", ty.void_(), core::ir::Function::PipelineStage::kFragment);
    b.Append(func->Block(), [&] {
        b.Let("x", b.Call(ty.vec2<u32>(), core::BuiltinFn::kAbs, b.Splat<vec2<u32>>(2_u)));
        b.Return(func);
    });

    auto* src = R"(
%foo = @fragment func():void {
  $B1: {
    %2:vec2<u32> = abs vec2<u32>(2u)
    %x:vec2<u32> = let %2
    ret
  }
}
)";
    ASSERT_EQ(src, str());

    auto* expect = R"(
%foo = @fragment func():void {
  $B1: {
    %x:vec2<u32> = let vec2<u32>(2u)
    ret
  }
}
)";

    Run(BuiltinPolyfill);
    EXPECT_EQ(expect, str());
}

TEST_F(GlslWriter_BuiltinPolyfillTest, QuantizeToF16) {
    auto* func = b.Function("foo", ty.void_(), core::ir::Function::PipelineStage::kFragment);
    b.Append(func->Block(), [&] {
        auto* v = b.Var("x", b.Zero(ty.vec2<f32>()));
        b.Let("a", b.Call(ty.vec2<f32>(), core::BuiltinFn::kQuantizeToF16, b.Load(v)));
        b.Return(func);
    });

    auto* src = R"(
%foo = @fragment func():void {
  $B1: {
    %x:ptr<function, vec2<f32>, read_write> = var vec2<f32>(0.0f)
    %3:vec2<f32> = load %x
    %4:vec2<f32> = quantizeToF16 %3
    %a:vec2<f32> = let %4
    ret
  }
}
)";
    ASSERT_EQ(src, str());

    auto* expect = R"(
%foo = @fragment func():void {
  $B1: {
    %x:ptr<function, vec2<f32>, read_write> = var vec2<f32>(0.0f)
    %3:vec2<f32> = load %x
    %4:vec2<f32> = call %tint_quantize_to_f16, %3
    %a:vec2<f32> = let %4
    ret
  }
}
%tint_quantize_to_f16 = func(%val:vec2<f32>):vec2<f32> {
  $B2: {
    %8:u32 = pack2x16float %val
    %9:vec2<f32> = unpack2x16float %8
    ret %9
  }
}
)";
    Run(BuiltinPolyfill);
    EXPECT_EQ(expect, str());
}

}  // namespace
}  // namespace tint::glsl::writer::raise
