// Copyright 2023 The Dawn & Tint Authors
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

#include "src/tint/lang/msl/writer/helper_test.h"

namespace tint::msl::writer {
namespace {

using namespace tint::core::fluent_types;     // NOLINT
using namespace tint::core::number_suffixes;  // NOLINT

TEST_F(MslWriterTest, VarF32) {
    auto* func = b.Function("foo", ty.void_());
    b.Append(func->Block(), [&] {
        b.Var("a", ty.ptr<core::AddressSpace::kFunction, f32>());
        b.Return(func);
    });

    ASSERT_TRUE(Generate()) << err_ << output_.msl;
    EXPECT_EQ(output_.msl, MetalHeader() + R"(
void foo() {
  float a = 0.0f;
}
)");
}

TEST_F(MslWriterTest, VarI32) {
    auto* func = b.Function("foo", ty.void_());
    b.Append(func->Block(), [&] {
        auto* v = b.Var("a", ty.ptr<core::AddressSpace::kFunction, i32>());
        v->SetInitializer(b.Constant(1_i));
        b.Return(func);
    });

    ASSERT_TRUE(Generate()) << err_ << output_.msl;
    EXPECT_EQ(output_.msl, MetalHeader() + R"(
void foo() {
  int a = 1;
}
)");
}

TEST_F(MslWriterTest, VarU32) {
    auto* func = b.Function("foo", ty.void_());
    b.Append(func->Block(), [&] {
        auto* v = b.Var("a", ty.ptr<core::AddressSpace::kFunction, u32>());
        v->SetInitializer(b.Constant(1_u));
        b.Return(func);
    });

    ASSERT_TRUE(Generate()) << err_ << output_.msl;
    EXPECT_EQ(output_.msl, MetalHeader() + R"(
void foo() {
  uint a = 1u;
}
)");
}

TEST_F(MslWriterTest, VarArrayF32) {
    auto* func = b.Function("foo", ty.void_());
    b.Append(func->Block(), [&] {
        b.Var("a", ty.ptr<core::AddressSpace::kFunction, array<f32, 5>>());
        b.Return(func);
    });

    ASSERT_TRUE(Generate()) << err_ << output_.msl;
    EXPECT_EQ(output_.msl, MetalHeader() + MetalArray() + R"(
void foo() {
  tint_array<float, 5> a = {};
}
)");
}

TEST_F(MslWriterTest, VarStruct) {
    auto* s = ty.Struct(mod.symbols.New("MyStruct"), {{mod.symbols.Register("a"), ty.f32()},  //
                                                      {mod.symbols.Register("b"), ty.vec4<i32>()}});

    auto* func = b.Function("foo", ty.void_());
    b.Append(func->Block(), [&] {
        b.Var("a", ty.ptr(core::AddressSpace::kFunction, s));
        b.Return(func);
    });

    ASSERT_TRUE(Generate()) << err_ << output_.msl;
    EXPECT_EQ(output_.msl, MetalHeader() + R"(
struct MyStruct {
  float a;
  int4 b;
};

void foo() {
  MyStruct a = {};
}
)");
}

TEST_F(MslWriterTest, VarVecF32) {
    auto* func = b.Function("foo", ty.void_());
    b.Append(func->Block(), [&] {
        b.Var("a", ty.ptr<core::AddressSpace::kFunction, vec2<f32>>());
        b.Return(func);
    });

    ASSERT_TRUE(Generate()) << err_ << output_.msl;
    EXPECT_EQ(output_.msl, MetalHeader() + R"(
void foo() {
  float2 a = 0.0f;
}
)");
}

TEST_F(MslWriterTest, VarVecF16) {
    // Enable f16?
    auto* func = b.Function("foo", ty.void_());
    b.Append(func->Block(), [&] {
        b.Var("a", ty.ptr<core::AddressSpace::kFunction, vec2<f16>>());
        b.Return(func);
    });

    ASSERT_TRUE(Generate()) << err_ << output_.msl;
    EXPECT_EQ(output_.msl, MetalHeader() + R"(
void foo() {
  half2 a = 0.0h;
}
)");
}

TEST_F(MslWriterTest, VarMatF32) {
    auto* func = b.Function("foo", ty.void_());
    b.Append(func->Block(), [&] {
        b.Var("a", ty.ptr<core::AddressSpace::kFunction, mat3x2<f32>>());
        b.Return(func);
    });

    ASSERT_TRUE(Generate()) << err_ << output_.msl;
    EXPECT_EQ(output_.msl, MetalHeader() + R"(
void foo() {
  float3x2 a = float3x2(0.0f);
}
)");
}

TEST_F(MslWriterTest, VarMatF16) {
    // Enable f16?
    auto* func = b.Function("foo", ty.void_());
    b.Append(func->Block(), [&] {
        b.Var("a", ty.ptr<core::AddressSpace::kFunction, mat3x2<f16>>());
        b.Return(func);
    });

    ASSERT_TRUE(Generate()) << err_ << output_.msl;
    EXPECT_EQ(output_.msl, MetalHeader() + R"(
void foo() {
  half3x2 a = half3x2(0.0h);
}
)");
}

TEST_F(MslWriterTest, VarVecF32SplatZero) {
    auto* func = b.Function("foo", ty.void_());
    b.Append(func->Block(), [&] {
        auto* v = b.Var("a", ty.ptr<core::AddressSpace::kFunction, vec3<f32>>());
        v->SetInitializer(b.Splat<vec3<f32>>(0_f));
        b.Return(func);
    });

    ASSERT_TRUE(Generate()) << err_ << output_.msl;
    EXPECT_EQ(output_.msl, MetalHeader() + R"(
void foo() {
  float3 a = float3(0.0f);
}
)");
}

TEST_F(MslWriterTest, VarVecF16SplatZero) {
    // Enable f16
    auto* func = b.Function("foo", ty.void_());
    b.Append(func->Block(), [&] {
        auto* v = b.Var("a", ty.ptr<core::AddressSpace::kFunction, vec3<f16>>());
        v->SetInitializer(b.Splat<vec3<f16>>(0_h));
        b.Return(func);
    });

    ASSERT_TRUE(Generate()) << err_ << output_.msl;
    EXPECT_EQ(output_.msl, MetalHeader() + R"(
void foo() {
  half3 a = half3(0.0h);
}
)");
}

TEST_F(MslWriterTest, VarMatF32SplatZero) {
    auto* func = b.Function("foo", ty.void_());
    b.Append(func->Block(), [&] {
        auto* v = b.Var("a", ty.ptr<core::AddressSpace::kFunction, mat2x3<f32>>());
        v->SetInitializer(
            b.Composite<mat2x3<f32>>(b.Splat<vec3<f32>>(0_f), b.Splat<vec3<f32>>(0_f)));
        b.Return(func);
    });

    ASSERT_TRUE(Generate()) << err_ << output_.msl;
    EXPECT_EQ(output_.msl, MetalHeader() + R"(
void foo() {
  float2x3 a = float2x3(float3(0.0f), float3(0.0f));
}
)");
}

TEST_F(MslWriterTest, VarMatF16SplatZero) {
    // Enable f16?
    auto* func = b.Function("foo", ty.void_());
    b.Append(func->Block(), [&] {
        auto* v = b.Var("a", ty.ptr<core::AddressSpace::kFunction, mat2x3<f16>>());
        v->SetInitializer(
            b.Composite<mat2x3<f16>>(b.Splat<vec3<f16>>(0_h), b.Splat<vec3<f16>>(0_h)));
        b.Return(func);
    });

    ASSERT_TRUE(Generate()) << err_ << output_.msl;
    EXPECT_EQ(output_.msl, MetalHeader() + R"(
void foo() {
  half2x3 a = half2x3(half3(0.0h), half3(0.0h));
}
)");
}

TEST_F(MslWriterTest, VarGlobalPrivate) {
    core::ir::Var* v = nullptr;
    b.Append(mod.root_block, [&] {  //
        v = b.Var("v", ty.ptr<core::AddressSpace::kPrivate, f32>());
    });

    auto* foo = b.Function("foo", ty.void_());
    b.Append(foo->Block(), [&] {
        auto* ld = b.Load(v->Result(0));
        b.Let("a", ld);
        b.Return(foo);
    });

    auto* frag = b.Function("frag", ty.void_(), core::ir::Function::PipelineStage::kFragment);
    b.Append(frag->Block(), [&] {
        b.Call(foo);
        b.Return(frag);
    });

    ASSERT_TRUE(Generate()) << err_ << output_.msl;
    EXPECT_EQ(output_.msl, MetalHeader() + R"(
struct tint_module_vars_struct {
  thread float* v;
};

void foo(tint_module_vars_struct tint_module_vars) {
  float const a = (*tint_module_vars.v);
}

fragment void frag() {
  thread float v = 0.0f;
  tint_module_vars_struct const tint_module_vars = tint_module_vars_struct{.v=(&v)};
  foo(tint_module_vars);
}
)");
}

TEST_F(MslWriterTest, VarGlobalWorkgroup) {
    core::ir::Var* v = nullptr;
    b.Append(mod.root_block,
             [&] { v = b.Var("v", ty.ptr<core::AddressSpace::kWorkgroup, f32>()); });

    auto* func = b.Function("foo", ty.void_());
    b.Append(func->Block(), [&] {
        auto* ld = b.Load(v->Result(0));
        auto* a = b.Var("a", ty.ptr<core::AddressSpace::kFunction, f32>());
        a->SetInitializer(ld->Result(0));
        b.Return(func);
    });

    ASSERT_TRUE(Generate()) << err_ << output_.msl;
    EXPECT_EQ(output_.msl, MetalHeader() + R"(
struct tint_module_vars_struct {
  threadgroup float* v;
};

void foo(tint_module_vars_struct tint_module_vars) {
  float a = (*tint_module_vars.v);
}
)");
}

}  // namespace
}  // namespace tint::msl::writer
