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

#include "src/tint/lang/core/type/depth_texture.h"
#include "src/tint/lang/core/type/storage_texture.h"
#include "src/tint/lang/glsl/writer/helper_test.h"

using namespace tint::core::fluent_types;     // NOLINT
using namespace tint::core::number_suffixes;  // NOLINT

namespace tint::glsl::writer {
namespace {

TEST_F(GlslWriterTest, Let) {
    auto* func = b.ComputeFunction("main");
    b.Append(func->Block(), [&] {
        b.Let("a", 2_f);
        b.Return(func);
    });

    ASSERT_TRUE(Generate()) << err_ << output_.glsl;
    EXPECT_EQ(output_.glsl, GlslHeader() + R"(
layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
  float a = 2.0f;
}
)");
}

TEST_F(GlslWriterTest, LetValue) {
    auto* func = b.ComputeFunction("main");
    b.Append(func->Block(), [&] {
        auto* a = b.Let("a", 2_f);
        b.Let("b", a);
        b.Return(func);
    });

    ASSERT_TRUE(Generate()) << err_ << output_.glsl;
    EXPECT_EQ(output_.glsl, GlslHeader() + R"(
layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
  float a = 2.0f;
  float b = a;
}
)");
}

TEST_F(GlslWriterTest, Var) {
    auto* func = b.ComputeFunction("main");
    b.Append(func->Block(), [&] {
        b.Var("a", 1_u);
        b.Return(func);
    });

    ASSERT_TRUE(Generate()) << err_ << output_.glsl;
    EXPECT_EQ(output_.glsl, GlslHeader() + R"(
layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
  uint a = 1u;
}
)");
}

TEST_F(GlslWriterTest, VarZeroInit) {
    auto* func = b.ComputeFunction("main");
    b.Append(func->Block(), [&] {
        b.Var("a", function, ty.f32());
        b.Return(func);
    });

    ASSERT_TRUE(Generate()) << err_ << output_.glsl;
    EXPECT_EQ(output_.glsl, GlslHeader() + R"(
layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
  float a = 0.0f;
}
)");
}

// Not emitted in GLSL
TEST_F(GlslWriterTest, VarSampler) {
    b.Append(b.ir.root_block, [&] {
        auto* v = b.Var("v", ty.ptr(core::AddressSpace::kHandle, ty.sampler()));
        v->SetBindingPoint(1, 2);
    });

    ASSERT_TRUE(Generate()) << err_ << output_.glsl;
    EXPECT_EQ(output_.glsl, GlslHeader() + R"(
layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
}
)");
}

// Not emitted in GLSL
TEST_F(GlslWriterTest, VarInBuiltin) {
    b.Append(b.ir.root_block, [&] {
        auto* v = b.Var("v", ty.ptr(core::AddressSpace::kIn, ty.u32()));
        core::IOAttributes attrs = {};
        attrs.builtin = core::BuiltinValue::kLocalInvocationIndex;
        v->SetAttributes(attrs);
    });

    ASSERT_TRUE(Generate()) << err_ << output_.glsl;
    EXPECT_EQ(output_.glsl, GlslHeader() + R"(
layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
}
)");
}

TEST_F(GlslWriterTest, VarIn) {
    b.Append(b.ir.root_block, [&] {
        auto* v = b.Var("v", ty.ptr(core::AddressSpace::kIn, ty.u32()));
        core::IOAttributes attrs = {};
        attrs.location = 1;
        attrs.interpolation = {core::InterpolationType::kFlat,
                               core::InterpolationSampling::kUndefined};
        v->SetAttributes(attrs);
    });

    auto* func = b.Function("main", ty.void_(), core::ir::Function::PipelineStage::kFragment);
    b.Append(func->Block(), [&] { b.Return(func); });
    ASSERT_TRUE(Generate({}, tint::ast::PipelineStage::kFragment)) << err_ << output_.glsl;
    EXPECT_EQ(output_.glsl, GlslHeader() + R"(precision highp float;
precision highp int;

layout(location = 1) flat in uint tint_interstage_location1;
void main() {
}
)");
}

TEST_F(GlslWriterTest, VarOutBlendSrc) {
    b.Append(b.ir.root_block, [&] {
        auto* v = b.Var("v", ty.ptr(core::AddressSpace::kOut, ty.u32()));
        core::IOAttributes attrs = {};
        attrs.location = 1;
        attrs.blend_src = 1;
        v->SetAttributes(attrs);
    });

    auto* func = b.Function("main", ty.void_(), core::ir::Function::PipelineStage::kFragment);
    b.Append(func->Block(), [&] { b.Return(func); });

    ASSERT_TRUE(Generate({}, tint::ast::PipelineStage::kFragment)) << err_ << output_.glsl;
    EXPECT_EQ(output_.glsl, GlslHeader() + R"(#extension GL_EXT_blend_func_extended: require
precision highp float;
precision highp int;

layout(location = 1, index = 1) out uint v;
void main() {
}
)");
}

// Not emitted in GLSL
TEST_F(GlslWriterTest, VarOutBuiltin) {
    b.Append(b.ir.root_block, [&] {
        auto* v = b.Var("v", ty.ptr(core::AddressSpace::kOut, ty.u32()));
        core::IOAttributes attrs = {};
        attrs.builtin = core::BuiltinValue::kFragDepth;
        v->SetAttributes(attrs);
    });

    auto* func = b.Function("main", ty.void_(), core::ir::Function::PipelineStage::kFragment);
    b.Append(func->Block(), [&] { b.Return(func); });

    ASSERT_TRUE(Generate({}, tint::ast::PipelineStage::kFragment)) << err_ << output_.glsl;
    EXPECT_EQ(output_.glsl, GlslHeader() + R"(precision highp float;
precision highp int;

void main() {
}
)");
}

TEST_F(GlslWriterTest, VarBuiltinSampleIndex_ES) {
    b.Append(b.ir.root_block, [&] {
        auto* v = b.Var("v", ty.ptr(core::AddressSpace::kOut, ty.u32()));
        core::IOAttributes attrs = {};
        attrs.builtin = core::BuiltinValue::kSampleIndex;
        v->SetAttributes(attrs);
    });

    auto* func = b.Function("main", ty.void_(), core::ir::Function::PipelineStage::kFragment);
    b.Append(func->Block(), [&] { b.Return(func); });

    ASSERT_TRUE(Generate({}, tint::ast::PipelineStage::kFragment)) << err_ << output_.glsl;
    EXPECT_EQ(output_.glsl, GlslHeader() + R"(#extension GL_OES_sample_variables: require
precision highp float;
precision highp int;

void main() {
}
)");
}

TEST_F(GlslWriterTest, VarBuiltinSampleMask_ES) {
    b.Append(b.ir.root_block, [&] {
        auto* v = b.Var("v", ty.ptr(core::AddressSpace::kOut, ty.u32()));
        core::IOAttributes attrs = {};
        attrs.builtin = core::BuiltinValue::kSampleMask;
        v->SetAttributes(attrs);
    });

    auto* func = b.Function("main", ty.void_(), core::ir::Function::PipelineStage::kFragment);
    b.Append(func->Block(), [&] { b.Return(func); });

    ASSERT_TRUE(Generate({}, tint::ast::PipelineStage::kFragment)) << err_ << output_.glsl;
    EXPECT_EQ(output_.glsl, GlslHeader() + R"(#extension GL_OES_sample_variables: require
precision highp float;
precision highp int;

void main() {
}
)");
}

TEST_F(GlslWriterTest, VarBuiltinSampled_NonES) {
    b.Append(b.ir.root_block, [&] {
        auto* v = b.Var("v", ty.ptr(core::AddressSpace::kOut, ty.u32()));
        core::IOAttributes attrs = {};
        attrs.builtin = core::BuiltinValue::kSampleIndex;
        v->SetAttributes(attrs);
    });

    auto* func = b.Function("main", ty.void_(), core::ir::Function::PipelineStage::kFragment);
    b.Append(func->Block(), [&] { b.Return(func); });

    Options opts{};
    opts.version = Version(Version::Standard::kDesktop, 4, 6);
    ASSERT_TRUE(Generate(opts, tint::ast::PipelineStage::kFragment)) << err_ << output_.glsl;
    EXPECT_EQ(output_.glsl, R"(#version 460
precision highp float;
precision highp int;

void main() {
}
)");
}

TEST_F(GlslWriterTest, VarStorageUint32) {
    b.Append(b.ir.root_block, [&] {
        auto* v = b.Var("v", ty.ptr(core::AddressSpace::kStorage, ty.u32()));
        v->SetBindingPoint(0, 1);
    });

    ASSERT_TRUE(Generate()) << err_ << output_.glsl;
    EXPECT_EQ(output_.glsl, GlslHeader() + R"(
layout(binding = 1, std430)
buffer v_block_1_ssbo {
  uint inner;
} v_1;
layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
}
)");
}

TEST_F(GlslWriterTest, VarStorageStruct) {
    auto* sb = ty.Struct(mod.symbols.New("SB"), {
                                                    {mod.symbols.New("a"), ty.i32()},
                                                    {mod.symbols.New("b"), ty.f32()},
                                                });

    b.Append(b.ir.root_block, [&] {
        auto* v = b.Var("v", ty.ptr(core::AddressSpace::kStorage, sb));
        v->SetBindingPoint(0, 1);
    });

    ASSERT_TRUE(Generate()) << err_ << output_.glsl;
    EXPECT_EQ(output_.glsl, GlslHeader() + R"(

struct SB {
  int a;
  float b;
};

layout(binding = 1, std430)
buffer v_block_1_ssbo {
  SB inner;
} v_1;
layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
}
)");
}

TEST_F(GlslWriterTest, VarUniform) {
    b.Append(b.ir.root_block, [&] {
        auto* v = b.Var("v", ty.ptr(core::AddressSpace::kUniform, ty.u32()));
        v->SetBindingPoint(0, 1);
    });

    ASSERT_TRUE(Generate()) << err_ << output_.glsl;
    EXPECT_EQ(output_.glsl, GlslHeader() + R"(
layout(binding = 1, std140)
uniform v_block_1_ubo {
  uint inner;
} v_1;
layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
}
)");
}

TEST_F(GlslWriterTest, VarHandleStorageTexture) {
    b.Append(b.ir.root_block, [&] {
        auto* v =
            b.Var("v", ty.ptr(core::AddressSpace::kHandle,
                              ty.Get<core::type::StorageTexture>(core::type::TextureDimension::k2d,
                                                                 core::TexelFormat::kR32Float,
                                                                 core::Access::kWrite, ty.f32())));
        v->SetBindingPoint(0, 1);
    });

    ASSERT_TRUE(Generate()) << err_ << output_.glsl;
    EXPECT_EQ(output_.glsl, GlslHeader() + R"(
layout(binding = 1, r32f) uniform highp writeonly image2D v;
layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
}
)");
}

TEST_F(GlslWriterTest, VarHandleDepthTexture) {
    b.Append(b.ir.root_block, [&] {
        auto* v =
            b.Var("v", ty.ptr(core::AddressSpace::kHandle,
                              ty.Get<core::type::DepthTexture>(core::type::TextureDimension::k2d)));
        v->SetBindingPoint(0, 1);
    });

    ASSERT_TRUE(Generate()) << err_ << output_.glsl;
    EXPECT_EQ(output_.glsl, GlslHeader() + R"(
uniform highp sampler2DShadow v;
layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
}
)");
}

TEST_F(GlslWriterTest, VarWorkgroup) {
    b.Append(b.ir.root_block,
             [&] { b.Var("v", ty.ptr(core::AddressSpace::kWorkgroup, ty.u32())); });

    ASSERT_TRUE(Generate()) << err_ << output_.glsl;
    EXPECT_EQ(output_.glsl, GlslHeader() + R"(
shared uint v;
layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
}
)");
}

}  // namespace
}  // namespace tint::glsl::writer
