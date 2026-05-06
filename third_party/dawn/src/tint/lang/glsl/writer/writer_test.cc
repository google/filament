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

#include "gmock/gmock.h"
#include "src/tint/lang/core/type/sampled_texture.h"
#include "src/tint/lang/glsl/writer/helper_test.h"

namespace tint::glsl::writer {
namespace {

using namespace tint::core::fluent_types;     // NOLINT
using namespace tint::core::number_suffixes;  // NOLINT

TEST_F(GlslWriterTest, StripAllNames) {
    auto* str = ty.Struct(mod.symbols.New("MyStruct"), {
                                                           {mod.symbols.Register("a"), ty.i32()},
                                                           {mod.symbols.Register("b"), ty.vec4i()},
                                                       });
    auto* foo = b.Function("foo", ty.u32());
    auto* param = b.FunctionParam("param", ty.u32());
    foo->AppendParam(param);
    b.Append(foo->Block(), [&] {  //
        b.Return(foo, param);
    });

    auto* func = b.ComputeFunction("main");
    auto* idx = b.FunctionParam("idx", ty.u32());
    idx->SetBuiltin(core::BuiltinValue::kLocalInvocationIndex);
    func->AppendParam(idx);
    b.Append(func->Block(), [&] {  //
        auto* var = b.Var("str", ty.ptr<function>(str));
        auto* val = b.Load(var);
        mod.SetName(val, "val");
        auto* a = b.Access<i32>(val, 0_u);
        mod.SetName(a, "a");
        b.Let("let", b.Call<u32>(foo, idx));
        b.Return(func);
    });

    Options options;
    options.strip_all_names = true;
    auto result = Generate(options);
    ASSERT_EQ(result, Success) << result.Failure().reason << output_.glsl;
    EXPECT_EQ(output_.glsl, GlslHeader() + R"(

struct tint_struct {
  int member_0;
  ivec4 member_1;
};

uint v(uint v_1) {
  return v_1;
}
void v_2(uint v_3) {
  tint_struct v_4 = tint_struct(0, ivec4(0));
  uint v_5 = v(v_3);
}
layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
  v_2(gl_LocalInvocationIndex);
}
)");
}

TEST_F(GlslWriterTest, StripAllNames_CombinedTextureSamplerName) {
    BindingPoint texture_bp{1, 2};
    BindingPoint sampler_bp{3, 4};
    auto* tex_ty = ty.sampled_texture(core::type::TextureDimension::k2d, ty.f32());
    auto* texture = b.Var("texture", ty.ptr<handle>(tex_ty));
    auto* sampler = b.Var("sampler", ty.ptr<handle>(ty.sampler()));
    texture->SetBindingPoint(texture_bp.group, texture_bp.binding);
    sampler->SetBindingPoint(sampler_bp.group, sampler_bp.binding);
    mod.root_block->Append(texture);
    mod.root_block->Append(sampler);

    auto* func = b.Function("main", ty.void_(), core::ir::Function::PipelineStage::kFragment);
    b.Append(func->Block(), [&] {  //
        auto* t = b.Load(texture);
        auto* s = b.Load(sampler);
        b.Let("value",
              b.Call<vec4<f32>>(core::BuiltinFn::kTextureSample, t, s, b.Zero<vec2<f32>>()));
        b.Return(func);
    });

    Options options;
    options.strip_all_names = true;
    options.sampler_texture_to_name.insert(
        {CombinedTextureSamplerPair{texture_bp, sampler_bp}, "tint_combined_texture_sampler"});
    auto result = Generate(options);
    ASSERT_EQ(result, Success) << result.Failure().reason << output_.glsl;
    EXPECT_EQ(output_.glsl, GlslHeader() + R"(precision highp float;
precision highp int;

uniform highp sampler2D tint_combined_texture_sampler;
void main() {
  vec4 v = texture(tint_combined_texture_sampler, vec2(0.0f));
}
)");
}

TEST_F(GlslWriterTest, CanGenerate_TexelBufferUnsupported) {
    auto* buffer_ty = ty.texel_buffer(core::TexelFormat::kRgba8Unorm, core::Access::kRead);
    auto* var = b.Var("buf", ty.ptr<handle>(buffer_ty));
    mod.root_block->Append(var);

    auto* ep = b.ComputeFunction("main");
    b.Append(ep->Block(), [&] {
        b.Let("x", var);
        b.Return(ep);
    });

    Options options;
    options.entry_point_name = "main";
    auto result = Generate(options);
    ASSERT_NE(result, Success);
    EXPECT_THAT(result.Failure().reason,
                testing::HasSubstr("texel buffers are not supported by the GLSL backend"));
}

TEST_F(GlslWriterTest, CanGenerate_AtomicStoreMax_Unsupported) {
    auto* sb =
        ty.Struct(mod.symbols.New("SB"), {
                                             {mod.symbols.Register("a"), ty.atomic(ty.u64())},
                                         });
    auto* var = b.Var("sb", ty.ptr<storage, read_write>(sb));
    var->SetBindingPoint(0, 0);
    mod.root_block->Append(var);

    auto* func = b.ComputeFunction("main");
    b.Append(func->Block(), [&] {
        auto* access = b.Access(ty.ptr<storage, read_write>(ty.atomic(ty.u64())), var, 0_u);
        b.Call<void>(core::BuiltinFn::kAtomicStoreMax, access, 1_u);
        b.Return(func);
    });

    Options options;
    options.entry_point_name = "main";
    auto result = Generate(options);
    ASSERT_NE(result, Success);
    EXPECT_THAT(result.Failure().reason,
                testing::HasSubstr(
                    "64-bit (vec2u) atomic operations are not yet supported by the GLSL backend"));
}

TEST_F(GlslWriterTest, CanGenerate_AtomicStoreMin_Unsupported) {
    auto* sb =
        ty.Struct(mod.symbols.New("SB"), {
                                             {mod.symbols.Register("a"), ty.atomic(ty.u64())},
                                         });
    auto* var = b.Var("sb", ty.ptr<storage, read_write>(sb));
    var->SetBindingPoint(0, 0);
    mod.root_block->Append(var);

    auto* func = b.ComputeFunction("main");
    b.Append(func->Block(), [&] {
        auto* access = b.Access(ty.ptr<storage, read_write>(ty.atomic(ty.u64())), var, 0_u);
        b.Call<void>(core::BuiltinFn::kAtomicStoreMin, access, 1_u);
        b.Return(func);
    });

    Options options;
    options.entry_point_name = "main";
    auto result = Generate(options);
    ASSERT_NE(result, Success);
    EXPECT_THAT(result.Failure().reason,
                testing::HasSubstr(
                    "64-bit (vec2u) atomic operations are not yet supported by the GLSL backend"));
}

TEST_F(GlslWriterTest, WorkgroupStorageSize_OverflowAfterAlign) {
    auto* var = mod.root_block->Append(b.Var<workgroup, array<u32, 0x3FFFFFFFu>>("a"));
    auto* foo = b.ComputeFunction("main", 64_u, 1_u, 1_u);
    b.Append(foo->Block(), [&] {  //
        b.Load(b.Access<ptr<workgroup, u32>>(var, 0_u));
        b.Return(foo);
    });

    auto result = Generate();
    ASSERT_EQ(result, Success) << result.Failure() << output_.glsl;
    EXPECT_EQ(output_.glsl, GlslHeader() + R"(
shared uint a[1073741823];
void main_inner(uint tint_local_index) {
  {
    uint v = 0u;
    v = tint_local_index;
    while(true) {
      uint v_1 = v;
      if ((v_1 >= 1073741823u)) {
        break;
      }
      a[v_1] = 0u;
      {
        v = (v_1 + 64u);
      }
    }
  }
  barrier();
}
layout(local_size_x = 64, local_size_y = 1, local_size_z = 1) in;
void main() {
  main_inner(gl_LocalInvocationIndex);
}
)");

    EXPECT_EQ(output_.workgroup_info.storage_size, 0x100000000ull);
}

}  // namespace
}  // namespace tint::glsl::writer
