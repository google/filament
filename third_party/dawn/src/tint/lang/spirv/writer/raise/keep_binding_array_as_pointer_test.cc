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

#include "src/tint/lang/spirv/writer/raise/keep_binding_array_as_pointer.h"

#include "src/tint/lang/core/ir/transform/helper_test.h"
#include "src/tint/lang/core/type/binding_array.h"
#include "src/tint/lang/core/type/sampled_texture.h"

namespace tint::spirv::writer::raise {
namespace {

using namespace tint::core::fluent_types;     // NOLINT
using namespace tint::core::number_suffixes;  // NOLINT

using SpirvWriter_KeepBindingArrayAsPointer = core::ir::transform::TransformTest;

TEST_F(SpirvWriter_KeepBindingArrayAsPointer, SkippedForPtrToBindingArray) {
    auto* texture_type = ty.sampled_texture(core::type::TextureDimension::k2d, ty.f32());
    auto* var_ts = b.Var("ts", ty.ptr<handle>(ty.binding_array(texture_type, 3u)));
    var_ts->SetBindingPoint(0, 0);
    b.ir.root_block->Append(var_ts);

    auto* fn = b.Function("f", ty.void_());
    b.Append(fn->Block(), [&] {
        auto* t_ptr = b.Access(ty.ptr<handle>(texture_type), var_ts, 0_i);
        auto* t = b.Load(t_ptr);
        b.Call(ty.vec4<f32>(), core::BuiltinFn::kTextureLoad, t, b.Zero(ty.vec2<u32>()), 0_u);
        b.Return(fn);
    });

    auto* src = R"(
$B1: {  # root
  %ts:ptr<handle, binding_array<texture_2d<f32>, 3>, read> = var undef @binding_point(0, 0)
}

%f = func():void {
  $B2: {
    %3:ptr<handle, texture_2d<f32>, read> = access %ts, 0i
    %4:texture_2d<f32> = load %3
    %5:vec4<f32> = textureLoad %4, vec2<u32>(0u), 0u
    ret
  }
}
)";

    EXPECT_EQ(src, str());

    Run(KeepBindingArrayAsPointer);
    EXPECT_EQ(src, str());
}

TEST_F(SpirvWriter_KeepBindingArrayAsPointer, LoadOfBindingArrayIsReplaced) {
    auto* texture_type = ty.sampled_texture(core::type::TextureDimension::k2d, ty.f32());
    auto* var_ts = b.Var("ts", ty.ptr<handle>(ty.binding_array(texture_type, 3u)));
    var_ts->SetBindingPoint(0, 0);
    b.ir.root_block->Append(var_ts);

    auto* fn = b.Function("f", ty.void_());
    b.Append(fn->Block(), [&] {
        auto* ts = b.Load(var_ts);
        auto* t = b.Access(texture_type, ts, 0_i);
        b.Call(ty.vec4<f32>(), core::BuiltinFn::kTextureLoad, t, b.Zero(ty.vec2<u32>()), 0_u);
        b.Return(fn);
    });

    auto* src = R"(
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
)";

    EXPECT_EQ(src, str());

    auto* expected = R"(
$B1: {  # root
  %ts:ptr<handle, binding_array<texture_2d<f32>, 3>, read> = var undef @binding_point(0, 0)
}

%f = func():void {
  $B2: {
    %3:ptr<handle, texture_2d<f32>, read> = access %ts, 0i
    %4:texture_2d<f32> = load %3
    %5:vec4<f32> = textureLoad %4, vec2<u32>(0u), 0u
    ret
  }
}
)";

    Run(KeepBindingArrayAsPointer);
    EXPECT_EQ(expected, str());
}

}  // namespace
}  // namespace tint::spirv::writer::raise
