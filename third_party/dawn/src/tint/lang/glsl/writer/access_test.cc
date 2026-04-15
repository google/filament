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
#include "src/tint/lang/glsl/writer/helper_test.h"

using namespace tint::core::number_suffixes;  // NOLINT
using namespace tint::core::fluent_types;     // NOLINT

namespace tint::glsl::writer {
namespace {

TEST_F(GlslWriterTest, AccessArray) {
    auto* func = b.ComputeFunction("main");

    b.Append(func->Block(), [&] {
        auto* v = b.Var("v", b.Zero<array<f32, 3>>());
        b.Let("x", b.Load(b.Access(ty.ptr<function, f32>(), v, 1_u)));
        b.Return(func);
    });

    auto result = Generate();
    ASSERT_EQ(result, Success) << result.Failure().reason << output_.glsl;
    EXPECT_EQ(output_.glsl, GlslHeader() + R"(
layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
  float v[3] = float[3](0.0f, 0.0f, 0.0f);
  float x = v[1u];
}
)");
}

TEST_F(GlslWriterTest, AccessStruct) {
    Vector members{
        ty.Get<core::type::StructMember>(b.ir.symbols.New("a"), ty.i32(), 0u, 0u, 4u, 4u,
                                         core::IOAttributes{}),
        ty.Get<core::type::StructMember>(b.ir.symbols.New("b"), ty.f32(), 1u, 4u, 4u, 4u,
                                         core::IOAttributes{}),
    };
    auto* strct = ty.Struct(b.ir.symbols.New("S"), std::move(members));

    auto* f = b.ComputeFunction("main");

    b.Append(f->Block(), [&] {
        auto* v = b.Var("v", b.Zero(strct));
        b.Let("x", b.Load(b.Access(ty.ptr<function, f32>(), v, 1_u)));
        b.Return(f);
    });

    auto result = Generate();
    ASSERT_EQ(result, Success) << result.Failure().reason << output_.glsl;
    EXPECT_EQ(output_.glsl, GlslHeader() + R"(

struct S {
  int a;
  float b;
};

layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
  S v = S(0, 0.0f);
  float x = v.b;
}
)");
}

TEST_F(GlslWriterTest, AccessVector) {
    auto* func = b.ComputeFunction("main");

    b.Append(func->Block(), [&] {
        auto* v = b.Var("v", b.Zero<vec3<f32>>());
        b.Let("x", b.LoadVectorElement(v, 1_u));
        b.Return(func);
    });

    auto result = Generate();
    ASSERT_EQ(result, Success) << result.Failure().reason << output_.glsl;
    EXPECT_EQ(output_.glsl, GlslHeader() + R"(
layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
  vec3 v = vec3(0.0f);
  float x = v.y;
}
)");
}

TEST_F(GlslWriterTest, AccessMatrix) {
    auto* func = b.ComputeFunction("main");

    b.Append(func->Block(), [&] {
        auto* v = b.Var("v", b.Zero<mat4x4<f32>>());
        auto* v1 = b.Access(ty.ptr<function, vec4<f32>>(), v, 1_u);
        b.Let("x", b.LoadVectorElement(v1, 2_u));
        b.Return(func);
    });

    auto result = Generate();
    ASSERT_EQ(result, Success) << result.Failure().reason << output_.glsl;
    EXPECT_EQ(output_.glsl, GlslHeader() + R"(
layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
  mat4 v = mat4(vec4(0.0f), vec4(0.0f), vec4(0.0f), vec4(0.0f));
  float x = v[1u].z;
}
)");
}

TEST_F(GlslWriterTest, AccessStoreVectorElementConstantIndex) {
    auto* func = b.ComputeFunction("main");
    b.Append(func->Block(), [&] {
        auto* vec_var = b.Var("vec", ty.ptr<function, vec4<i32>>());
        b.StoreVectorElement(vec_var, 1_u, b.Constant(42_i));
        b.Return(func);
    });

    auto result = Generate();
    ASSERT_EQ(result, Success) << result.Failure().reason << output_.glsl;
    EXPECT_EQ(output_.glsl, GlslHeader() + R"(
layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
  ivec4 vec = ivec4(0);
  vec.y = 42;
}
)");
}

TEST_F(GlslWriterTest, AccessStoreVectorElementDynamicIndex) {
    auto* idx = b.FunctionParam("idx", ty.i32());
    auto* func = b.Function("foo", ty.void_());
    func->SetParams({idx});
    b.Append(func->Block(), [&] {
        auto* vec_var = b.Var("vec", ty.ptr<function, vec4<i32>>());
        b.StoreVectorElement(vec_var, idx, b.Constant(42_i));
        b.Return(func);
    });

    auto* eb = b.ComputeFunction("main");
    b.Append(eb->Block(), [&] {
        b.Call(func, b.Zero(ty.i32()));
        b.Return(eb);
    });

    auto result = Generate();
    ASSERT_EQ(result, Success) << result.Failure().reason << output_.glsl;
    EXPECT_EQ(output_.glsl, GlslHeader() + R"(
void foo(int idx) {
  ivec4 vec = ivec4(0);
  vec[min(uint(idx), 3u)] = 42;
}
layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
  foo(0);
}
)");
}

TEST_F(GlslWriterTest, AccessNested) {
    Vector members_a{
        ty.Get<core::type::StructMember>(b.ir.symbols.New("d"), ty.i32(), 0u, 0u, 4u, 4u,
                                         core::IOAttributes{}),
        ty.Get<core::type::StructMember>(b.ir.symbols.New("e"), ty.array<f32, 3>(), 1u, 4u, 4u, 12u,
                                         core::IOAttributes{}),
    };
    auto* a_strct = ty.Struct(b.ir.symbols.New("A"), std::move(members_a));

    Vector members_s{
        ty.Get<core::type::StructMember>(b.ir.symbols.New("a"), ty.i32(), 0u, 0u, 4u, 4u,
                                         core::IOAttributes{}),
        ty.Get<core::type::StructMember>(b.ir.symbols.New("b"), ty.f32(), 1u, 4u, 4u, 4u,
                                         core::IOAttributes{}),
        ty.Get<core::type::StructMember>(b.ir.symbols.New("c"), a_strct, 2u, 8u, 8u, 16u,
                                         core::IOAttributes{}),
    };
    auto* s_strct = ty.Struct(b.ir.symbols.New("S"), std::move(members_s));

    auto* f = b.ComputeFunction("main");

    b.Append(f->Block(), [&] {
        auto* v = b.Var("v", b.Zero(s_strct));
        b.Let("x", b.Load(b.Access(ty.ptr<function, f32>(), v, 2_u, 1_u, 1_i)));
        b.Return(f);
    });

    auto result = Generate();
    ASSERT_EQ(result, Success) << result.Failure().reason << output_.glsl;
    EXPECT_EQ(output_.glsl, GlslHeader() + R"(

struct A {
  int d;
  float e[3];
};

struct S {
  int a;
  float b;
  A c;
};

layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
  S v = S(0, 0.0f, A(0, float[3](0.0f, 0.0f, 0.0f)));
  float x = v.c.e[1u];
}
)");
}

TEST_F(GlslWriterTest, AccessSwizzle) {
    auto* f = b.ComputeFunction("main");

    b.Append(f->Block(), [&] {
        auto* v = b.Var("v", b.Zero<vec3<f32>>());
        b.Let("b", b.Swizzle(ty.f32(), b.Load(v), {1u}));
        b.Return(f);
    });

    auto result = Generate();
    ASSERT_EQ(result, Success) << result.Failure().reason << output_.glsl;
    EXPECT_EQ(output_.glsl, GlslHeader() + R"(
layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
  vec3 v = vec3(0.0f);
  float b = v.y;
}
)");
}

TEST_F(GlslWriterTest, AccessSwizzleMulti) {
    auto* f = b.ComputeFunction("main");

    b.Append(f->Block(), [&] {
        auto* v = b.Var("v", b.Zero<vec4<f32>>());
        b.Let("b", b.Swizzle(ty.vec4f(), b.Load(v), {3u, 2u, 1u, 0u}));
        b.Return(f);
    });

    auto result = Generate();
    ASSERT_EQ(result, Success) << result.Failure().reason << output_.glsl;
    EXPECT_EQ(output_.glsl, GlslHeader() + R"(
layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
  vec4 v = vec4(0.0f);
  vec4 b = v.wzyx;
}
)");
}

TEST_F(GlslWriterTest, AccessStorageVector) {
    auto* var = b.Var<storage, vec4<f32>, core::Access::kRead>("v");
    var->SetBindingPoint(0, 0);

    b.ir.root_block->Append(var);
    auto* func = b.Function("main", ty.void_(), core::ir::Function::PipelineStage::kFragment);
    b.Append(func->Block(), [&] {
        b.Let("a", b.Load(var));
        b.Let("b", b.LoadVectorElement(var, 0_u));
        b.Let("c", b.LoadVectorElement(var, 1_u));
        b.Let("d", b.LoadVectorElement(var, 2_u));
        b.Let("e", b.LoadVectorElement(var, 3_u));
        b.Return(func);
    });

    auto result = Generate();
    ASSERT_EQ(result, Success) << result.Failure().reason << output_.glsl;
    EXPECT_EQ(output_.glsl, GlslHeader() + R"(precision highp float;
precision highp int;

layout(binding = 0, std430)
buffer f_v_block_ssbo {
  vec4 inner;
} v_1;
void main() {
  vec4 a = v_1.inner;
  float b = v_1.inner.x;
  float c = v_1.inner.y;
  float d = v_1.inner.z;
  float e = v_1.inner.w;
}
)");
}

TEST_F(GlslWriterTest, AccessStorageVectorF16) {
    auto* var = b.Var<storage, vec4<f16>, core::Access::kRead>("v");
    var->SetBindingPoint(0, 0);

    b.ir.root_block->Append(var);
    auto* func = b.Function("main", ty.void_(), core::ir::Function::PipelineStage::kFragment);
    b.Append(func->Block(), [&] {
        b.Let("a", b.Load(var));
        b.Let("b", b.LoadVectorElement(var, 0_u));
        b.Let("c", b.LoadVectorElement(var, 1_u));
        b.Let("d", b.LoadVectorElement(var, 2_u));
        b.Let("e", b.LoadVectorElement(var, 3_u));
        b.Return(func);
    });

    auto result = Generate();
    ASSERT_EQ(result, Success) << result.Failure().reason << output_.glsl;
    EXPECT_EQ(output_.glsl, GlslHeader() + R"(#extension GL_AMD_gpu_shader_half_float: require
precision highp float;
precision highp int;

layout(binding = 0, std430)
buffer f_v_block_ssbo {
  f16vec4 inner;
} v_1;
void main() {
  f16vec4 a = v_1.inner;
  float16_t b = v_1.inner.x;
  float16_t c = v_1.inner.y;
  float16_t d = v_1.inner.z;
  float16_t e = v_1.inner.w;
}
)");
}

TEST_F(GlslWriterTest, AccessStorageMatrix) {
    auto* var = b.Var<storage, mat4x4<f32>, core::Access::kRead>("v");
    var->SetBindingPoint(0, 0);

    b.ir.root_block->Append(var);
    auto* func = b.Function("main", ty.void_(), core::ir::Function::PipelineStage::kFragment);
    b.Append(func->Block(), [&] {
        b.Let("a", b.Load(var));
        b.Let("b", b.Load(b.Access(ty.ptr<storage, vec4<f32>, core::Access::kRead>(), var, 3_u)));
        b.Let("c", b.LoadVectorElement(
                       b.Access(ty.ptr<storage, vec4<f32>, core::Access::kRead>(), var, 1_u), 2_u));
        b.Return(func);
    });

    auto result = Generate();
    ASSERT_EQ(result, Success) << result.Failure().reason << output_.glsl;
    EXPECT_EQ(output_.glsl, GlslHeader() + R"(precision highp float;
precision highp int;

layout(binding = 0, std430)
buffer f_v_block_ssbo {
  mat4 inner;
} v_1;
void main() {
  mat4 a = v_1.inner;
  vec4 b = v_1.inner[3u];
  float c = v_1.inner[1u].z;
}
)");
}

TEST_F(GlslWriterTest, AccessStorageArray) {
    auto* var = b.Var<storage, array<vec3<f32>, 5>, core::Access::kRead>("v");
    var->SetBindingPoint(0, 0);

    b.ir.root_block->Append(var);
    auto* func = b.Function("main", ty.void_(), core::ir::Function::PipelineStage::kFragment);
    b.Append(func->Block(), [&] {
        b.Let("a", b.Load(var));
        b.Let("b", b.Load(b.Access(ty.ptr<storage, vec3<f32>, core::Access::kRead>(), var, 3_u)));
        b.Return(func);
    });

    auto result = Generate();
    ASSERT_EQ(result, Success) << result.Failure().reason << output_.glsl;
    EXPECT_EQ(output_.glsl, GlslHeader() + R"(precision highp float;
precision highp int;

layout(binding = 0, std430)
buffer f_v_block_ssbo {
  vec3 inner[5];
} v_1;
void main() {
  vec3 a[5] = v_1.inner;
  vec3 b = v_1.inner[3u];
}
)");
}

TEST_F(GlslWriterTest, AccessStorageStruct) {
    auto* SB = ty.Struct(mod.symbols.New("SB"), {
                                                    {mod.symbols.New("a"), ty.i32()},
                                                    {mod.symbols.New("b"), ty.f32()},
                                                });

    auto* var = b.Var("v", storage, SB, core::Access::kRead);
    var->SetBindingPoint(0, 0);

    b.ir.root_block->Append(var);
    auto* func = b.Function("main", ty.void_(), core::ir::Function::PipelineStage::kFragment);
    b.Append(func->Block(), [&] {
        b.Let("a", b.Load(var));
        b.Let("b", b.Load(b.Access(ty.ptr<storage, f32, core::Access::kRead>(), var, 1_u)));
        b.Return(func);
    });

    auto result = Generate();
    ASSERT_EQ(result, Success) << result.Failure().reason << output_.glsl;
    EXPECT_EQ(output_.glsl, GlslHeader() + R"(precision highp float;
precision highp int;


struct SB {
  int a;
  float b;
};

layout(binding = 0, std430)
buffer f_v_block_ssbo {
  SB inner;
} v_1;
void main() {
  SB a = v_1.inner;
  float b = v_1.inner.b;
}
)");
}

TEST_F(GlslWriterTest, AccessStorageNested) {
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
    auto* func = b.Function("main", ty.void_(), core::ir::Function::PipelineStage::kFragment);
    b.Append(func->Block(), [&] {
        b.Let("a", b.Load(var));
        b.Let("b", b.LoadVectorElement(b.Access(ty.ptr<storage, vec3<f32>, core::Access::kRead>(),
                                                var, 1_u, 1_u, 1_u, 3_u),
                                       2_u));
        b.Return(func);
    });

    auto result = Generate();
    ASSERT_EQ(result, Success) << result.Failure().reason << output_.glsl;
    EXPECT_EQ(output_.glsl, GlslHeader() + R"(precision highp float;
precision highp int;


struct Inner {
  mat3 s;
  vec3 t[5];
};

struct Outer {
  float x;
  uint tint_pad_0;
  uint tint_pad_1;
  uint tint_pad_2;
  Inner y;
};

struct SB {
  int a;
  uint tint_pad_0;
  uint tint_pad_1;
  uint tint_pad_2;
  Outer b;
};

layout(binding = 0, std430)
buffer f_v_block_ssbo {
  SB inner;
} v_1;
void main() {
  SB a = v_1.inner;
  float b = v_1.inner.b.y.t[3u].z;
}
)");
}

TEST_F(GlslWriterTest, AccessStorageStoreVector) {
    auto* var = b.Var<storage, vec4<f32>, core::Access::kReadWrite>("v");
    var->SetBindingPoint(0, 0);

    b.ir.root_block->Append(var);
    auto* func = b.Function("main", ty.void_(), core::ir::Function::PipelineStage::kFragment);
    b.Append(func->Block(), [&] {
        b.StoreVectorElement(var, 0_u, 2_f);
        b.StoreVectorElement(var, 1_u, 4_f);
        b.StoreVectorElement(var, 2_u, 8_f);
        b.StoreVectorElement(var, 3_u, 16_f);
        b.Return(func);
    });

    auto result = Generate();
    ASSERT_EQ(result, Success) << result.Failure().reason << output_.glsl;
    EXPECT_EQ(output_.glsl, GlslHeader() + R"(precision highp float;
precision highp int;

layout(binding = 0, std430)
buffer f_v_block_ssbo {
  vec4 inner;
} v_1;
void main() {
  v_1.inner.x = 2.0f;
  v_1.inner.y = 4.0f;
  v_1.inner.z = 8.0f;
  v_1.inner.w = 16.0f;
}
)");
}

TEST_F(GlslWriterTest, AccessDirectVariable) {
    auto* var1 = b.Var<storage, vec4<f32>, core::Access::kRead>("v1");
    var1->SetBindingPoint(0, 0);
    b.ir.root_block->Append(var1);

    auto* var2 = b.Var<storage, vec4<f32>, core::Access::kRead>("v2");
    var2->SetBindingPoint(0, 1);
    b.ir.root_block->Append(var2);

    auto* p = b.FunctionParam("x", ty.ptr<storage, vec4<f32>, core::Access::kRead>());
    auto* bar = b.Function("bar", ty.void_());
    bar->SetParams({p});
    b.Append(bar->Block(), [&] {
        b.Let("a", b.LoadVectorElement(p, 1_u));
        b.Return(bar);
    });

    auto* func = b.Function("main", ty.void_(), core::ir::Function::PipelineStage::kFragment);
    b.Append(func->Block(), [&] {
        b.Call(bar, var1);
        b.Call(bar, var2);
        b.Return(func);
    });

    auto result = Generate();
    ASSERT_EQ(result, Success) << result.Failure().reason << output_.glsl;
    EXPECT_EQ(output_.glsl, GlslHeader() + R"(precision highp float;
precision highp int;

layout(binding = 0, std430)
buffer f_v1_block_ssbo {
  vec4 inner;
} v;
layout(binding = 1, std430)
buffer f_v2_block_ssbo {
  vec4 inner;
} v_1;
void bar() {
  float a = v.inner.y;
}
void bar_1() {
  float a = v_1.inner.y;
}
void main() {
  bar();
  bar_1();
}
)");
}

TEST_F(GlslWriterTest, AccessChainFromUnnamedAccessChain) {
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

    auto* func = b.Function("main", ty.void_(), core::ir::Function::PipelineStage::kFragment);
    b.Append(func->Block(), [&] {
        auto* x = b.Access(ty.ptr(storage, sb, core::Access::kReadWrite), var, 2_u);
        auto* y = b.Access(ty.ptr(storage, Inner, core::Access::kReadWrite), x->Result(), 1_u);
        b.Let("b", b.Load(b.Access(ty.ptr(storage, ty.u32(), core::Access::kReadWrite), y->Result(),
                                   1_u)));
        b.Return(func);
    });

    auto result = Generate();
    ASSERT_EQ(result, Success) << result.Failure().reason << output_.glsl;
    EXPECT_EQ(output_.glsl, GlslHeader() + R"(precision highp float;
precision highp int;


struct Inner {
  float c;
  uint d;
};

struct SB {
  int a;
  Inner b;
};

layout(binding = 0, std430)
buffer f_v_block_ssbo {
  SB inner[4];
} v_1;
void main() {
  uint b = v_1.inner[2u].b.d;
}
)");
}

TEST_F(GlslWriterTest, AccessChainFromLetAccessChain) {
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

    auto* func = b.Function("main", ty.void_(), core::ir::Function::PipelineStage::kFragment);
    b.Append(func->Block(), [&] {
        auto* x = b.Let("x", var);
        auto* y = b.Let(
            "y", b.Access(ty.ptr(storage, Inner, core::Access::kReadWrite), x->Result(), 1_u));
        auto* z = b.Let(
            "z", b.Access(ty.ptr(storage, ty.f32(), core::Access::kReadWrite), y->Result(), 0_u));
        b.Let("a", b.Load(z));
        b.Return(func);
    });

    auto result = Generate();
    ASSERT_EQ(result, Success) << result.Failure().reason << output_.glsl;
    EXPECT_EQ(output_.glsl, GlslHeader() + R"(precision highp float;
precision highp int;


struct Inner {
  float c;
};

struct SB {
  int a;
  Inner b;
};

layout(binding = 0, std430)
buffer f_v_block_ssbo {
  SB inner;
} v_1;
void main() {
  float a = v_1.inner.b.c;
}
)");
}

TEST_F(GlslWriterTest, AccessComplexDynamicAccessChain) {
    auto* S1 = ty.Struct(mod.symbols.New("S1"), {
                                                    {mod.symbols.New("a"), ty.i32()},
                                                    {mod.symbols.New("b"), ty.vec3f()},
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
    auto* func = b.Function("main", ty.void_(), core::ir::Function::PipelineStage::kFragment);
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

    auto result = Generate();
    ASSERT_EQ(result, Success) << result.Failure().reason << output_.glsl;
    EXPECT_EQ(output_.glsl, GlslHeader() + R"(precision highp float;
precision highp int;


struct S1 {
  int a;
  uint tint_pad_0;
  uint tint_pad_1;
  uint tint_pad_2;
  vec3 b;
  int c;
};

struct S2 {
  int a_1;
  uint tint_pad_0;
  uint tint_pad_1;
  uint tint_pad_2;
  S1 b_1[3];
  int c_1;
  uint tint_pad_3;
  uint tint_pad_4;
  uint tint_pad_5;
};

layout(binding = 0, std430)
buffer f_SB_ssbo {
  int a_2;
  uint tint_pad_0;
  uint tint_pad_1;
  uint tint_pad_2;
  S2 b_2[];
} sb;
void main() {
  int i = 4;
  int v = i;
  uint j = 1u;
  uint v_1 = j;
  int k = 2;
  int v_2 = k;
  uint v_3 = (uint(sb.b_2.length()) - 1u);
  uint v_4 = min(uint(v), v_3);
  float x = sb.b_2[v_4].b_1[min(v_1, 2u)].b[min(uint(v_2), 2u)];
}
)");
}

TEST_F(GlslWriterTest, AccessComplexDynamicAccessChainSplit) {
    auto* S1 = ty.Struct(mod.symbols.New("S1"), {
                                                    {mod.symbols.New("a"), ty.i32()},
                                                    {mod.symbols.New("b"), ty.vec3f()},
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
    auto* func = b.Function("main", ty.void_(), core::ir::Function::PipelineStage::kFragment);
    b.Append(func->Block(), [&] {
        auto* j = b.Load(b.Var("j", 1_u));
        b.Let("x", b.LoadVectorElement(b.Access(ty.ptr<storage, vec3<f32>, read_write>(), var, 1_u,
                                                4_u, 1_u, j, 1_u),
                                       2_u));
        b.Return(func);
    });

    auto result = Generate();
    ASSERT_EQ(result, Success) << result.Failure().reason << output_.glsl;
    EXPECT_EQ(output_.glsl, GlslHeader() + R"(precision highp float;
precision highp int;


struct S1 {
  int a;
  uint tint_pad_0;
  uint tint_pad_1;
  uint tint_pad_2;
  vec3 b;
  int c;
};

struct S2 {
  int a_1;
  uint tint_pad_0;
  uint tint_pad_1;
  uint tint_pad_2;
  S1 b_1[3];
  int c_1;
  uint tint_pad_3;
  uint tint_pad_4;
  uint tint_pad_5;
};

layout(binding = 0, std430)
buffer f_SB_ssbo {
  int a_2;
  uint tint_pad_0;
  uint tint_pad_1;
  uint tint_pad_2;
  S2 b_2[];
} sb;
void main() {
  uint j = 1u;
  uint v = j;
  uint v_1 = min(4u, (uint(sb.b_2.length()) - 1u));
  float x = sb.b_2[v_1].b_1[min(v, 2u)].b.z;
}
)");
}

TEST_F(GlslWriterTest, AccessUniformChainFromUnnamedAccessChain) {
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

    auto* func = b.Function("main", ty.void_(), core::ir::Function::PipelineStage::kFragment);
    b.Append(func->Block(), [&] {
        auto* x = b.Access(ty.ptr(uniform, sb, core::Access::kRead), var, 2_u);
        auto* y = b.Access(ty.ptr(uniform, Inner, core::Access::kRead), x->Result(), 1_u);
        b.Let("b",
              b.Load(b.Access(ty.ptr(uniform, ty.u32(), core::Access::kRead), y->Result(), 1_u)));
        b.Return(func);
    });

    auto result = Generate();
    ASSERT_EQ(result, Success) << result.Failure().reason << output_.glsl;
    EXPECT_EQ(output_.glsl, GlslHeader() + R"(precision highp float;
precision highp int;

layout(binding = 0, std140)
uniform f_v_block_ubo {
  uvec4 inner[8];
} v_1;
void main() {
  uvec4 v_2 = v_1.inner[5u];
  uint b = v_2.y;
}
)");
}

TEST_F(GlslWriterTest, AccessUniformChainFromLetAccessChain) {
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

    auto* func = b.Function("main", ty.void_(), core::ir::Function::PipelineStage::kFragment);
    b.Append(func->Block(), [&] {
        auto* x = b.Let("x", var);
        auto* y =
            b.Let("y", b.Access(ty.ptr(uniform, Inner, core::Access::kRead), x->Result(), 1_u));
        auto* z =
            b.Let("z", b.Access(ty.ptr(uniform, ty.f32(), core::Access::kRead), y->Result(), 0_u));
        b.Let("a", b.Load(z));
        b.Return(func);
    });

    auto result = Generate();
    ASSERT_EQ(result, Success) << result.Failure().reason << output_.glsl;
    EXPECT_EQ(output_.glsl, GlslHeader() + R"(precision highp float;
precision highp int;

layout(binding = 0, std140)
uniform f_v_block_ubo {
  uvec4 inner[2];
} v_1;
void main() {
  uvec4 v_2 = v_1.inner[1u];
  float a = uintBitsToFloat(v_2.x);
}
)");
}

TEST_F(GlslWriterTest, AccessUniformScalar) {
    auto* var = b.Var<uniform, f32, core::Access::kRead>("v");
    var->SetBindingPoint(0, 0);

    b.ir.root_block->Append(var);
    auto* func = b.Function("main", ty.void_(), core::ir::Function::PipelineStage::kFragment);
    b.Append(func->Block(), [&] {
        b.Let("a", b.Load(var));
        b.Return(func);
    });

    auto result = Generate();
    ASSERT_EQ(result, Success) << result.Failure().reason << output_.glsl;
    EXPECT_EQ(output_.glsl, GlslHeader() + R"(precision highp float;
precision highp int;

layout(binding = 0, std140)
uniform f_v_block_ubo {
  uvec4 inner[1];
} v_1;
void main() {
  uvec4 v_2 = v_1.inner[0u];
  float a = uintBitsToFloat(v_2.x);
}
)");
}

TEST_F(GlslWriterTest, AccessUniformScalarF16) {
    auto* var = b.Var<uniform, f16, core::Access::kRead>("v");
    var->SetBindingPoint(0, 0);

    b.ir.root_block->Append(var);
    auto* func = b.Function("main", ty.void_(), core::ir::Function::PipelineStage::kFragment);
    b.Append(func->Block(), [&] {
        b.Let("a", b.Load(var));
        b.Return(func);
    });

    auto result = Generate();
    ASSERT_EQ(result, Success) << result.Failure().reason << output_.glsl;
    EXPECT_EQ(output_.glsl, GlslHeader() + R"(#extension GL_AMD_gpu_shader_half_float: require
precision highp float;
precision highp int;

layout(binding = 0, std140)
uniform f_v_block_ubo {
  uvec4 inner[1];
} v_1;
f16vec2 tint_bitcast_to_f16(uint src) {
  return unpackFloat2x16(src);
}
void main() {
  uvec4 v_2 = v_1.inner[0u];
  float16_t a = tint_bitcast_to_f16(v_2.x).x;
}
)");
}

TEST_F(GlslWriterTest, AccessUniformVector) {
    auto* var = b.Var<uniform, vec4<f32>, core::Access::kRead>("v");
    var->SetBindingPoint(0, 0);

    b.ir.root_block->Append(var);
    auto* func = b.Function("main", ty.void_(), core::ir::Function::PipelineStage::kFragment);
    b.Append(func->Block(), [&] {
        b.Let("a", b.Load(var));
        b.Let("b", b.LoadVectorElement(var, 0_u));
        b.Let("c", b.LoadVectorElement(var, 1_u));
        b.Let("d", b.LoadVectorElement(var, 2_u));
        b.Let("e", b.LoadVectorElement(var, 3_u));
        b.Return(func);
    });

    auto result = Generate();
    ASSERT_EQ(result, Success) << result.Failure().reason << output_.glsl;
    EXPECT_EQ(output_.glsl, GlslHeader() + R"(precision highp float;
precision highp int;

layout(binding = 0, std140)
uniform f_v_block_ubo {
  uvec4 inner[1];
} v_1;
void main() {
  vec4 a = uintBitsToFloat(v_1.inner[0u]);
  uvec4 v_2 = v_1.inner[0u];
  float b = uintBitsToFloat(v_2.x);
  uvec4 v_3 = v_1.inner[0u];
  float c = uintBitsToFloat(v_3.y);
  uvec4 v_4 = v_1.inner[0u];
  float d = uintBitsToFloat(v_4.z);
  uvec4 v_5 = v_1.inner[0u];
  float e = uintBitsToFloat(v_5.w);
}
)");
}

TEST_F(GlslWriterTest, AccessUniformVectorF16) {
    auto* var = b.Var<uniform, vec4<f16>, core::Access::kRead>("v");
    var->SetBindingPoint(0, 0);

    b.ir.root_block->Append(var);
    auto* func = b.Function("main", ty.void_(), core::ir::Function::PipelineStage::kFragment);
    b.Append(func->Block(), [&] {
        auto* x = b.Var("x", 1_u);
        b.Let("a", b.Load(var));
        b.Let("b", b.LoadVectorElement(var, 0_u));
        b.Let("c", b.LoadVectorElement(var, b.Load(x)));
        b.Let("d", b.LoadVectorElement(var, 2_u));
        b.Let("e", b.LoadVectorElement(var, 3_u));
        b.Return(func);
    });

    auto result = Generate();
    ASSERT_EQ(result, Success) << result.Failure().reason << output_.glsl;
    EXPECT_EQ(output_.glsl, GlslHeader() + R"(#extension GL_AMD_gpu_shader_half_float: require
precision highp float;
precision highp int;

layout(binding = 0, std140)
uniform f_v_block_ubo {
  uvec4 inner[1];
} v_1;
f16vec2 tint_bitcast_to_f16(uint src) {
  return unpackFloat2x16(src);
}
f16vec4 tint_bitcast_to_f16_1(uvec2 src) {
  return f16vec4(unpackFloat2x16(src.x), unpackFloat2x16(src.y));
}
void main() {
  uint x = 1u;
  f16vec4 a = tint_bitcast_to_f16_1(v_1.inner[0u].xy);
  uvec4 v_2 = v_1.inner[0u];
  float16_t b = tint_bitcast_to_f16(v_2.x).x;
  uint v_3 = (min(x, 3u) * 2u);
  uvec4 v_4 = v_1.inner[(v_3 / 16u)];
  float16_t c = tint_bitcast_to_f16(v_4[((v_3 & 15u) >> 2u)])[mix(1u, 0u, ((v_3 % 4u) == 0u))];
  uvec4 v_5 = v_1.inner[0u];
  float16_t d = tint_bitcast_to_f16(v_5.y).x;
  uvec4 v_6 = v_1.inner[0u];
  float16_t e = tint_bitcast_to_f16(v_6.y).y;
}
)");
}

TEST_F(GlslWriterTest, AccessUniformMatrix) {
    auto* var = b.Var<uniform, mat4x4<f32>, core::Access::kRead>("v");
    var->SetBindingPoint(0, 0);

    b.ir.root_block->Append(var);
    auto* func = b.Function("main", ty.void_(), core::ir::Function::PipelineStage::kFragment);
    b.Append(func->Block(), [&] {
        b.Let("a", b.Load(var));
        b.Let("b", b.Load(b.Access(ty.ptr<uniform, vec4<f32>, core::Access::kRead>(), var, 3_u)));
        b.Let("c", b.LoadVectorElement(
                       b.Access(ty.ptr<uniform, vec4<f32>, core::Access::kRead>(), var, 1_u), 2_u));
        b.Return(func);
    });

    auto result = Generate();
    ASSERT_EQ(result, Success) << result.Failure().reason << output_.glsl;
    EXPECT_EQ(output_.glsl, GlslHeader() + R"(precision highp float;
precision highp int;

layout(binding = 0, std140)
uniform f_v_block_ubo {
  uvec4 inner[4];
} v_1;
mat4 v_2(uint start_byte_offset) {
  return mat4(uintBitsToFloat(v_1.inner[(start_byte_offset / 16u)]), uintBitsToFloat(v_1.inner[((16u + start_byte_offset) / 16u)]), uintBitsToFloat(v_1.inner[((32u + start_byte_offset) / 16u)]), uintBitsToFloat(v_1.inner[((48u + start_byte_offset) / 16u)]));
}
void main() {
  mat4 a = v_2(0u);
  vec4 b = uintBitsToFloat(v_1.inner[3u]);
  uvec4 v_3 = v_1.inner[1u];
  float c = uintBitsToFloat(v_3.z);
}
)");
}

TEST_F(GlslWriterTest, AccessUniformMatrix2x3) {
    auto* var = b.Var<uniform, mat2x3<f32>, core::Access::kRead>("v");
    var->SetBindingPoint(0, 0);

    b.ir.root_block->Append(var);
    auto* func = b.Function("main", ty.void_(), core::ir::Function::PipelineStage::kFragment);
    b.Append(func->Block(), [&] {
        b.Let("a", b.Load(var));
        b.Let("b", b.Load(b.Access(ty.ptr<uniform, vec3<f32>, core::Access::kRead>(), var, 1_u)));
        b.Let("c", b.LoadVectorElement(
                       b.Access(ty.ptr<uniform, vec3<f32>, core::Access::kRead>(), var, 1_u), 2_u));
        b.Return(func);
    });

    auto result = Generate();
    ASSERT_EQ(result, Success) << result.Failure().reason << output_.glsl;
    EXPECT_EQ(output_.glsl, GlslHeader() + R"(precision highp float;
precision highp int;

layout(binding = 0, std140)
uniform f_v_block_ubo {
  uvec4 inner[2];
} v_1;
mat2x3 v_2(uint start_byte_offset) {
  return mat2x3(uintBitsToFloat(v_1.inner[(start_byte_offset / 16u)].xyz), uintBitsToFloat(v_1.inner[((16u + start_byte_offset) / 16u)].xyz));
}
void main() {
  mat2x3 a = v_2(0u);
  vec3 b = uintBitsToFloat(v_1.inner[1u].xyz);
  uvec4 v_3 = v_1.inner[1u];
  float c = uintBitsToFloat(v_3.z);
}
)");
}

TEST_F(GlslWriterTest, AccessUniformMat2x3F16) {
    auto* var = b.Var<uniform, mat2x3<f16>, core::Access::kRead>("v");
    var->SetBindingPoint(0, 0);
    b.ir.root_block->Append(var);

    auto* func = b.Function("main", ty.void_(), core::ir::Function::PipelineStage::kFragment);
    b.Append(func->Block(), [&] {
        b.Let("a", b.Load(var));
        b.Let("b", b.Load(b.Access(ty.ptr(uniform, ty.vec3h()), var, 1_u)));
        b.Let("c", b.LoadVectorElement(b.Access(ty.ptr(uniform, ty.vec3h()), var, 1_u), 2_u));
        b.Return(func);
    });

    auto result = Generate();
    ASSERT_EQ(result, Success) << result.Failure().reason << output_.glsl;
    EXPECT_EQ(output_.glsl, GlslHeader() + R"(#extension GL_AMD_gpu_shader_half_float: require
precision highp float;
precision highp int;

layout(binding = 0, std140)
uniform f_v_block_ubo {
  uvec4 inner[1];
} v_1;
f16vec2 tint_bitcast_to_f16(uint src) {
  return unpackFloat2x16(src);
}
f16vec4 tint_bitcast_to_f16_1(uvec2 src) {
  return f16vec4(unpackFloat2x16(src.x), unpackFloat2x16(src.y));
}
f16mat2x3 v_2(uint start_byte_offset) {
  uvec4 v_3 = v_1.inner[(start_byte_offset / 16u)];
  f16vec3 v_4 = tint_bitcast_to_f16_1(mix(v_3.xy, v_3.zw, bvec2((((start_byte_offset & 15u) >> 2u) == 2u)))).xyz;
  uvec4 v_5 = v_1.inner[((8u + start_byte_offset) / 16u)];
  return f16mat2x3(v_4, tint_bitcast_to_f16_1(mix(v_5.xy, v_5.zw, bvec2(((((8u + start_byte_offset) & 15u) >> 2u) == 2u)))).xyz);
}
void main() {
  f16mat2x3 a = v_2(0u);
  f16vec3 b = tint_bitcast_to_f16_1(v_1.inner[0u].zw).xyz;
  uvec4 v_6 = v_1.inner[0u];
  float16_t c = tint_bitcast_to_f16(v_6.w).x;
}
)");
}

TEST_F(GlslWriterTest, AccessUniformMatrix3x2) {
    auto* var = b.Var<uniform, mat3x2<f32>, core::Access::kRead>("v");
    var->SetBindingPoint(0, 0);

    b.ir.root_block->Append(var);
    auto* func = b.Function("main", ty.void_(), core::ir::Function::PipelineStage::kFragment);
    b.Append(func->Block(), [&] {
        b.Let("a", b.Load(var));
        b.Let("b", b.Load(b.Access(ty.ptr<uniform, vec2<f32>, core::Access::kRead>(), var, 1_u)));
        b.Let("c", b.LoadVectorElement(
                       b.Access(ty.ptr<uniform, vec2<f32>, core::Access::kRead>(), var, 1_u), 1_u));
        b.Return(func);
    });

    auto result = Generate();
    ASSERT_EQ(result, Success) << result.Failure().reason << output_.glsl;
    EXPECT_EQ(output_.glsl, GlslHeader() + R"(precision highp float;
precision highp int;

layout(binding = 0, std140)
uniform f_v_block_ubo {
  uvec4 inner[2];
} v_1;
mat3x2 v_2(uint start_byte_offset) {
  uvec4 v_3 = v_1.inner[(start_byte_offset / 16u)];
  vec2 v_4 = uintBitsToFloat(mix(v_3.xy, v_3.zw, bvec2((((start_byte_offset & 15u) >> 2u) == 2u))));
  uvec4 v_5 = v_1.inner[((8u + start_byte_offset) / 16u)];
  vec2 v_6 = uintBitsToFloat(mix(v_5.xy, v_5.zw, bvec2(((((8u + start_byte_offset) & 15u) >> 2u) == 2u))));
  uvec4 v_7 = v_1.inner[((16u + start_byte_offset) / 16u)];
  return mat3x2(v_4, v_6, uintBitsToFloat(mix(v_7.xy, v_7.zw, bvec2(((((16u + start_byte_offset) & 15u) >> 2u) == 2u)))));
}
void main() {
  mat3x2 a = v_2(0u);
  vec2 b = uintBitsToFloat(v_1.inner[0u].zw);
  uvec4 v_8 = v_1.inner[0u];
  float c = uintBitsToFloat(v_8.w);
}
)");
}

TEST_F(GlslWriterTest, AccessUniformMatrix2x2) {
    auto* var = b.Var<uniform, mat2x2<f32>, core::Access::kRead>("v");
    var->SetBindingPoint(0, 0);

    b.ir.root_block->Append(var);
    auto* func = b.Function("main", ty.void_(), core::ir::Function::PipelineStage::kFragment);
    b.Append(func->Block(), [&] {
        b.Let("a", b.Load(var));
        b.Let("b", b.Load(b.Access(ty.ptr<uniform, vec2<f32>, core::Access::kRead>(), var, 1_u)));
        b.Let("c", b.LoadVectorElement(
                       b.Access(ty.ptr<uniform, vec2<f32>, core::Access::kRead>(), var, 1_u), 1_u));
        b.Return(func);
    });

    auto result = Generate();
    ASSERT_EQ(result, Success) << result.Failure().reason << output_.glsl;
    EXPECT_EQ(output_.glsl, GlslHeader() + R"(precision highp float;
precision highp int;

layout(binding = 0, std140)
uniform f_v_block_ubo {
  uvec4 inner[1];
} v_1;
mat2 v_2(uint start_byte_offset) {
  uvec4 v_3 = v_1.inner[(start_byte_offset / 16u)];
  vec2 v_4 = uintBitsToFloat(mix(v_3.xy, v_3.zw, bvec2((((start_byte_offset & 15u) >> 2u) == 2u))));
  uvec4 v_5 = v_1.inner[((8u + start_byte_offset) / 16u)];
  return mat2(v_4, uintBitsToFloat(mix(v_5.xy, v_5.zw, bvec2(((((8u + start_byte_offset) & 15u) >> 2u) == 2u)))));
}
void main() {
  mat2 a = v_2(0u);
  vec2 b = uintBitsToFloat(v_1.inner[0u].zw);
  uvec4 v_6 = v_1.inner[0u];
  float c = uintBitsToFloat(v_6.w);
}
)");
}

TEST_F(GlslWriterTest, AccessUniformMatrix2x2F16) {
    auto* var = b.Var<uniform, mat2x2<f16>, core::Access::kRead>("v");
    var->SetBindingPoint(0, 0);

    b.ir.root_block->Append(var);
    auto* func = b.Function("main", ty.void_(), core::ir::Function::PipelineStage::kFragment);
    b.Append(func->Block(), [&] {
        b.Let("a", b.Load(var));
        b.Let("b", b.Load(b.Access(ty.ptr<uniform, vec2<f16>, core::Access::kRead>(), var, 1_u)));
        b.Let("c", b.LoadVectorElement(
                       b.Access(ty.ptr<uniform, vec2<f16>, core::Access::kRead>(), var, 1_u), 1_u));
        b.Return(func);
    });

    auto result = Generate();
    ASSERT_EQ(result, Success) << result.Failure().reason << output_.glsl;
    EXPECT_EQ(output_.glsl, GlslHeader() + R"(#extension GL_AMD_gpu_shader_half_float: require
precision highp float;
precision highp int;

layout(binding = 0, std140)
uniform f_v_block_ubo {
  uvec4 inner[1];
} v_1;
f16vec2 tint_bitcast_to_f16(uint src) {
  return unpackFloat2x16(src);
}
f16mat2 v_2(uint start_byte_offset) {
  f16vec2 v_3 = tint_bitcast_to_f16(v_1.inner[(start_byte_offset / 16u)][((start_byte_offset & 15u) >> 2u)]);
  return f16mat2(v_3, tint_bitcast_to_f16(v_1.inner[((4u + start_byte_offset) / 16u)][(((4u + start_byte_offset) & 15u) >> 2u)]));
}
void main() {
  f16mat2 a = v_2(0u);
  f16vec2 b = tint_bitcast_to_f16(v_1.inner[0u].y);
  uvec4 v_4 = v_1.inner[0u];
  float16_t c = tint_bitcast_to_f16(v_4.y).y;
}
)");
}

TEST_F(GlslWriterTest, AccessUniformArray) {
    auto* var = b.Var<uniform, array<vec3<f32>, 5>, core::Access::kRead>("v");
    var->SetBindingPoint(0, 0);

    b.ir.root_block->Append(var);
    auto* func = b.Function("main", ty.void_(), core::ir::Function::PipelineStage::kFragment);
    b.Append(func->Block(), [&] {
        b.Let("a", b.Load(var));
        b.Let("b", b.Load(b.Access(ty.ptr<uniform, vec3<f32>, core::Access::kRead>(), var, 3_u)));
        b.Return(func);
    });

    auto result = Generate();
    ASSERT_EQ(result, Success) << result.Failure().reason << output_.glsl;
    EXPECT_EQ(output_.glsl, GlslHeader() + R"(precision highp float;
precision highp int;

layout(binding = 0, std140)
uniform f_v_block_ubo {
  uvec4 inner[5];
} v_1;
vec3[5] v_2(uint start_byte_offset) {
  vec3 a[5] = vec3[5](vec3(0.0f), vec3(0.0f), vec3(0.0f), vec3(0.0f), vec3(0.0f));
  {
    uint v_3 = 0u;
    v_3 = 0u;
    while(true) {
      uint v_4 = v_3;
      if ((v_4 >= 5u)) {
        break;
      }
      a[v_4] = uintBitsToFloat(v_1.inner[((start_byte_offset + (v_4 * 16u)) / 16u)].xyz);
      {
        v_3 = (v_4 + 1u);
      }
    }
  }
  return a;
}
void main() {
  vec3 a[5] = v_2(0u);
  vec3 b = uintBitsToFloat(v_1.inner[3u].xyz);
}
)");
}

TEST_F(GlslWriterTest, AccessUniformArrayF16) {
    auto* var = b.Var<uniform, array<vec3<f16>, 5>, core::Access::kRead>("v");
    var->SetBindingPoint(0, 0);

    b.ir.root_block->Append(var);
    auto* func = b.Function("main", ty.void_(), core::ir::Function::PipelineStage::kFragment);
    b.Append(func->Block(), [&] {
        b.Let("a", b.Load(var));
        b.Let("b", b.Load(b.Access(ty.ptr<uniform, vec3<f16>, core::Access::kRead>(), var, 3_u)));
        b.Return(func);
    });

    auto result = Generate();
    ASSERT_EQ(result, Success) << result.Failure().reason << output_.glsl;
    EXPECT_EQ(output_.glsl, GlslHeader() + R"(#extension GL_AMD_gpu_shader_half_float: require
precision highp float;
precision highp int;

layout(binding = 0, std140)
uniform f_v_block_ubo {
  uvec4 inner[3];
} v_1;
f16vec4 tint_bitcast_to_f16(uvec2 src) {
  return f16vec4(unpackFloat2x16(src.x), unpackFloat2x16(src.y));
}
f16vec3[5] v_2(uint start_byte_offset) {
  f16vec3 a[5] = f16vec3[5](f16vec3(0.0hf), f16vec3(0.0hf), f16vec3(0.0hf), f16vec3(0.0hf), f16vec3(0.0hf));
  {
    uint v_3 = 0u;
    v_3 = 0u;
    while(true) {
      uint v_4 = v_3;
      if ((v_4 >= 5u)) {
        break;
      }
      uvec4 v_5 = v_1.inner[((start_byte_offset + (v_4 * 8u)) / 16u)];
      a[v_4] = tint_bitcast_to_f16(mix(v_5.xy, v_5.zw, bvec2(((((start_byte_offset + (v_4 * 8u)) & 15u) >> 2u) == 2u)))).xyz;
      {
        v_3 = (v_4 + 1u);
      }
    }
  }
  return a;
}
void main() {
  f16vec3 a[5] = v_2(0u);
  f16vec3 b = tint_bitcast_to_f16(v_1.inner[1u].zw).xyz;
}
)");
}

TEST_F(GlslWriterTest, AccessUniformArrayWhichCanHaveSizesOtherThenFive) {
    auto* var = b.Var<uniform, array<vec3<f32>, 42>, core::Access::kRead>("v");
    var->SetBindingPoint(0, 0);

    b.ir.root_block->Append(var);
    auto* func = b.Function("main", ty.void_(), core::ir::Function::PipelineStage::kFragment);
    b.Append(func->Block(), [&] {
        b.Let("a", b.Load(var));
        b.Let("b", b.Load(b.Access(ty.ptr<uniform, vec3<f32>, core::Access::kRead>(), var, 3_u)));
        b.Return(func);
    });

    auto result = Generate();
    ASSERT_EQ(result, Success) << result.Failure().reason << output_.glsl;
    EXPECT_EQ(output_.glsl, GlslHeader() + R"(precision highp float;
precision highp int;

layout(binding = 0, std140)
uniform f_v_block_ubo {
  uvec4 inner[42];
} v_1;
vec3[42] v_2(uint start_byte_offset) {
  vec3 a[42] = vec3[42](vec3(0.0f), vec3(0.0f), vec3(0.0f), vec3(0.0f), vec3(0.0f), vec3(0.0f), vec3(0.0f), vec3(0.0f), vec3(0.0f), vec3(0.0f), vec3(0.0f), vec3(0.0f), vec3(0.0f), vec3(0.0f), vec3(0.0f), vec3(0.0f), vec3(0.0f), vec3(0.0f), vec3(0.0f), vec3(0.0f), vec3(0.0f), vec3(0.0f), vec3(0.0f), vec3(0.0f), vec3(0.0f), vec3(0.0f), vec3(0.0f), vec3(0.0f), vec3(0.0f), vec3(0.0f), vec3(0.0f), vec3(0.0f), vec3(0.0f), vec3(0.0f), vec3(0.0f), vec3(0.0f), vec3(0.0f), vec3(0.0f), vec3(0.0f), vec3(0.0f), vec3(0.0f), vec3(0.0f));
  {
    uint v_3 = 0u;
    v_3 = 0u;
    while(true) {
      uint v_4 = v_3;
      if ((v_4 >= 42u)) {
        break;
      }
      a[v_4] = uintBitsToFloat(v_1.inner[((start_byte_offset + (v_4 * 16u)) / 16u)].xyz);
      {
        v_3 = (v_4 + 1u);
      }
    }
  }
  return a;
}
void main() {
  vec3 a[42] = v_2(0u);
  vec3 b = uintBitsToFloat(v_1.inner[3u].xyz);
}
)");
}

TEST_F(GlslWriterTest, AccessUniformStruct) {
    auto* SB = ty.Struct(mod.symbols.New("SB"), {
                                                    {mod.symbols.New("a"), ty.i32()},
                                                    {mod.symbols.New("b"), ty.f32()},
                                                });

    auto* var = b.Var("v", uniform, SB, core::Access::kRead);
    var->SetBindingPoint(0, 0);

    b.ir.root_block->Append(var);
    auto* func = b.Function("main", ty.void_(), core::ir::Function::PipelineStage::kFragment);
    b.Append(func->Block(), [&] {
        b.Let("a", b.Load(var));
        b.Let("b", b.Load(b.Access(ty.ptr<uniform, f32, core::Access::kRead>(), var, 1_u)));
        b.Return(func);
    });

    auto result = Generate();
    ASSERT_EQ(result, Success) << result.Failure().reason << output_.glsl;
    EXPECT_EQ(output_.glsl, GlslHeader() + R"(precision highp float;
precision highp int;


struct SB {
  int a;
  float b;
};

layout(binding = 0, std140)
uniform f_v_block_ubo {
  uvec4 inner[1];
} v_1;
SB v_2(uint start_byte_offset) {
  uvec4 v_3 = v_1.inner[(start_byte_offset / 16u)];
  int v_4 = int(v_3[((start_byte_offset & 15u) >> 2u)]);
  uvec4 v_5 = v_1.inner[((4u + start_byte_offset) / 16u)];
  return SB(v_4, uintBitsToFloat(v_5[(((4u + start_byte_offset) & 15u) >> 2u)]));
}
void main() {
  SB a = v_2(0u);
  uvec4 v_6 = v_1.inner[0u];
  float b = uintBitsToFloat(v_6.y);
}
)");
}

TEST_F(GlslWriterTest, AccessUniformStructF16) {
    auto* SB = ty.Struct(mod.symbols.New("SB"), {
                                                    {mod.symbols.New("a"), ty.i32()},
                                                    {mod.symbols.New("b"), ty.f16()},
                                                });

    auto* var = b.Var("v", uniform, SB, core::Access::kRead);
    var->SetBindingPoint(0, 0);

    b.ir.root_block->Append(var);
    auto* func = b.Function("main", ty.void_(), core::ir::Function::PipelineStage::kFragment);
    b.Append(func->Block(), [&] {
        b.Let("a", b.Load(var));
        b.Let("b", b.Load(b.Access(ty.ptr<uniform, f16, core::Access::kRead>(), var, 1_u)));
        b.Return(func);
    });

    auto result = Generate();
    ASSERT_EQ(result, Success) << result.Failure().reason << output_.glsl;
    EXPECT_EQ(output_.glsl, GlslHeader() + R"(#extension GL_AMD_gpu_shader_half_float: require
precision highp float;
precision highp int;


struct SB {
  int a;
  float16_t b;
};

layout(binding = 0, std140)
uniform f_v_block_ubo {
  uvec4 inner[1];
} v_1;
f16vec2 tint_bitcast_to_f16(uint src) {
  return unpackFloat2x16(src);
}
SB v_2(uint start_byte_offset) {
  uvec4 v_3 = v_1.inner[(start_byte_offset / 16u)];
  int v_4 = int(v_3[((start_byte_offset & 15u) >> 2u)]);
  uvec4 v_5 = v_1.inner[((4u + start_byte_offset) / 16u)];
  return SB(v_4, tint_bitcast_to_f16(v_5[(((4u + start_byte_offset) & 15u) >> 2u)])[mix(1u, 0u, (((4u + start_byte_offset) % 4u) == 0u))]);
}
void main() {
  SB a = v_2(0u);
  uvec4 v_6 = v_1.inner[0u];
  float16_t b = tint_bitcast_to_f16(v_6.y).x;
}
)");
}

TEST_F(GlslWriterTest, AccessUniformStructNested) {
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
    auto* func = b.Function("main", ty.void_(), core::ir::Function::PipelineStage::kFragment);
    b.Append(func->Block(), [&] {
        b.Let("a", b.Load(var));
        b.Let("b", b.LoadVectorElement(b.Access(ty.ptr<uniform, vec3<f32>, core::Access::kRead>(),
                                                var, 1_u, 1_u, 1_u, 3_u),
                                       2_u));
        b.Return(func);
    });

    auto result = Generate();
    ASSERT_EQ(result, Success) << result.Failure().reason << output_.glsl;
    EXPECT_EQ(output_.glsl, GlslHeader() + R"(precision highp float;
precision highp int;


struct Inner {
  mat3 s;
  vec3 t[5];
};

struct Outer {
  float x;
  Inner y;
};

struct SB {
  int a;
  Outer b;
};

layout(binding = 0, std140)
uniform f_v_block_ubo {
  uvec4 inner[10];
} v_1;
vec3[5] v_2(uint start_byte_offset) {
  vec3 a[5] = vec3[5](vec3(0.0f), vec3(0.0f), vec3(0.0f), vec3(0.0f), vec3(0.0f));
  {
    uint v_3 = 0u;
    v_3 = 0u;
    while(true) {
      uint v_4 = v_3;
      if ((v_4 >= 5u)) {
        break;
      }
      a[v_4] = uintBitsToFloat(v_1.inner[((start_byte_offset + (v_4 * 16u)) / 16u)].xyz);
      {
        v_3 = (v_4 + 1u);
      }
    }
  }
  return a;
}
mat3 v_5(uint start_byte_offset) {
  return mat3(uintBitsToFloat(v_1.inner[(start_byte_offset / 16u)].xyz), uintBitsToFloat(v_1.inner[((16u + start_byte_offset) / 16u)].xyz), uintBitsToFloat(v_1.inner[((32u + start_byte_offset) / 16u)].xyz));
}
Inner v_6(uint start_byte_offset) {
  mat3 v_7 = v_5(start_byte_offset);
  return Inner(v_7, v_2((48u + start_byte_offset)));
}
Outer v_8(uint start_byte_offset) {
  uvec4 v_9 = v_1.inner[(start_byte_offset / 16u)];
  return Outer(uintBitsToFloat(v_9[((start_byte_offset & 15u) >> 2u)]), v_6((16u + start_byte_offset)));
}
SB v_10(uint start_byte_offset) {
  uvec4 v_11 = v_1.inner[(start_byte_offset / 16u)];
  int v_12 = int(v_11[((start_byte_offset & 15u) >> 2u)]);
  return SB(v_12, v_8((16u + start_byte_offset)));
}
void main() {
  SB a = v_10(0u);
  uvec4 v_13 = v_1.inner[8u];
  float b = uintBitsToFloat(v_13.z);
}
)");
}

TEST_F(GlslWriterTest, AccessStoreScalar) {
    auto* var = b.Var<storage, f32, core::Access::kReadWrite>("v");
    var->SetBindingPoint(0, 0);

    b.ir.root_block->Append(var);
    auto* func = b.Function("main", ty.void_(), core::ir::Function::PipelineStage::kFragment);
    b.Append(func->Block(), [&] {
        b.Store(var, 2_f);
        b.Return(func);
    });

    auto result = Generate();
    ASSERT_EQ(result, Success) << result.Failure().reason << output_.glsl;
    EXPECT_EQ(output_.glsl, GlslHeader() + R"(precision highp float;
precision highp int;

layout(binding = 0, std430)
buffer f_v_block_ssbo {
  float inner;
} v_1;
void main() {
  v_1.inner = 2.0f;
}
)");
}

TEST_F(GlslWriterTest, AccessStoreScalarF16) {
    auto* var = b.Var<storage, f16, core::Access::kReadWrite>("v");
    var->SetBindingPoint(0, 0);

    b.ir.root_block->Append(var);
    auto* func = b.Function("main", ty.void_(), core::ir::Function::PipelineStage::kFragment);
    b.Append(func->Block(), [&] {
        b.Store(var, 2_h);
        b.Return(func);
    });

    auto result = Generate();
    ASSERT_EQ(result, Success) << result.Failure().reason << output_.glsl;
    EXPECT_EQ(output_.glsl, GlslHeader() + R"(#extension GL_AMD_gpu_shader_half_float: require
precision highp float;
precision highp int;

layout(binding = 0, std430)
buffer f_v_block_ssbo {
  float16_t inner;
} v_1;
void main() {
  v_1.inner = 2.0hf;
}
)");
}

TEST_F(GlslWriterTest, AccessStoreVectorElement) {
    auto* var = b.Var<storage, vec3<f32>, core::Access::kReadWrite>("v");
    var->SetBindingPoint(0, 0);

    b.ir.root_block->Append(var);
    auto* func = b.Function("main", ty.void_(), core::ir::Function::PipelineStage::kFragment);
    b.Append(func->Block(), [&] {
        b.StoreVectorElement(var, 1_u, 2_f);
        b.Return(func);
    });

    auto result = Generate();
    ASSERT_EQ(result, Success) << result.Failure().reason << output_.glsl;
    EXPECT_EQ(output_.glsl, GlslHeader() + R"(precision highp float;
precision highp int;

layout(binding = 0, std430)
buffer f_v_block_ssbo {
  vec3 inner;
} v_1;
void main() {
  v_1.inner.y = 2.0f;
}
)");
}

TEST_F(GlslWriterTest, AccessStoreVectorElementF16) {
    auto* var = b.Var<storage, vec3<f16>, core::Access::kReadWrite>("v");
    var->SetBindingPoint(0, 0);

    b.ir.root_block->Append(var);
    auto* func = b.Function("main", ty.void_(), core::ir::Function::PipelineStage::kFragment);
    b.Append(func->Block(), [&] {
        b.StoreVectorElement(var, 1_u, 2_h);
        b.Return(func);
    });

    auto result = Generate();
    ASSERT_EQ(result, Success) << result.Failure().reason << output_.glsl;
    EXPECT_EQ(output_.glsl, GlslHeader() + R"(#extension GL_AMD_gpu_shader_half_float: require
precision highp float;
precision highp int;

layout(binding = 0, std430)
buffer f_v_block_ssbo {
  f16vec3 inner;
} v_1;
void main() {
  v_1.inner.y = 2.0hf;
}
)");
}

TEST_F(GlslWriterTest, AccessStoreVector) {
    auto* var = b.Var<storage, vec3<f32>, core::Access::kReadWrite>("v");
    var->SetBindingPoint(0, 0);

    b.ir.root_block->Append(var);
    auto* func = b.Function("main", ty.void_(), core::ir::Function::PipelineStage::kFragment);
    b.Append(func->Block(), [&] {
        b.Store(var, b.Composite(ty.vec3f(), 2_f, 3_f, 4_f));
        b.Return(func);
    });

    auto result = Generate();
    ASSERT_EQ(result, Success) << result.Failure().reason << output_.glsl;
    EXPECT_EQ(output_.glsl, GlslHeader() + R"(precision highp float;
precision highp int;

layout(binding = 0, std430)
buffer f_v_block_ssbo {
  vec3 inner;
} v_1;
void main() {
  v_1.inner = vec3(2.0f, 3.0f, 4.0f);
}
)");
}

TEST_F(GlslWriterTest, AccessStoreVectorF16) {
    auto* var = b.Var<storage, vec3<f16>, core::Access::kReadWrite>("v");
    var->SetBindingPoint(0, 0);

    b.ir.root_block->Append(var);
    auto* func = b.Function("main", ty.void_(), core::ir::Function::PipelineStage::kFragment);
    b.Append(func->Block(), [&] {
        b.Store(var, b.Composite(ty.vec3h(), 2_h, 3_h, 4_h));
        b.Return(func);
    });

    auto result = Generate();
    ASSERT_EQ(result, Success) << result.Failure().reason << output_.glsl;
    EXPECT_EQ(output_.glsl, GlslHeader() + R"(#extension GL_AMD_gpu_shader_half_float: require
precision highp float;
precision highp int;

layout(binding = 0, std430)
buffer f_v_block_ssbo {
  f16vec3 inner;
} v_1;
void main() {
  v_1.inner = f16vec3(2.0hf, 3.0hf, 4.0hf);
}
)");
}

TEST_F(GlslWriterTest, AccessStoreMatrixElement) {
    auto* var = b.Var<storage, mat4x4<f32>, core::Access::kReadWrite>("v");
    var->SetBindingPoint(0, 0);

    b.ir.root_block->Append(var);
    auto* func = b.Function("main", ty.void_(), core::ir::Function::PipelineStage::kFragment);
    b.Append(func->Block(), [&] {
        b.StoreVectorElement(
            b.Access(ty.ptr<storage, vec4<f32>, core::Access::kReadWrite>(), var, 1_u), 2_u, 5_f);
        b.Return(func);
    });

    auto result = Generate();
    ASSERT_EQ(result, Success) << result.Failure().reason << output_.glsl;
    EXPECT_EQ(output_.glsl, GlslHeader() + R"(precision highp float;
precision highp int;

layout(binding = 0, std430)
buffer f_v_block_ssbo {
  mat4 inner;
} v_1;
void main() {
  v_1.inner[1u].z = 5.0f;
}
)");
}

TEST_F(GlslWriterTest, AccessStoreMatrixElementF16) {
    auto* var = b.Var<storage, mat3x2<f16>, core::Access::kReadWrite>("v");
    var->SetBindingPoint(0, 0);

    b.ir.root_block->Append(var);
    auto* func = b.Function("main", ty.void_(), core::ir::Function::PipelineStage::kFragment);
    b.Append(func->Block(), [&] {
        b.StoreVectorElement(
            b.Access(ty.ptr<storage, vec2<f16>, core::Access::kReadWrite>(), var, 2_u), 1_u, 5_h);
        b.Return(func);
    });

    auto result = Generate();
    ASSERT_EQ(result, Success) << result.Failure().reason << output_.glsl;
    EXPECT_EQ(output_.glsl, GlslHeader() + R"(#extension GL_AMD_gpu_shader_half_float: require
precision highp float;
precision highp int;

layout(binding = 0, std430)
buffer f_v_block_ssbo {
  f16mat3x2 inner;
} v_1;
void main() {
  v_1.inner[2u].y = 5.0hf;
}
)");
}

TEST_F(GlslWriterTest, AccessStoreMatrixColumn) {
    auto* var = b.Var<storage, mat4x4<f32>, core::Access::kReadWrite>("v");
    var->SetBindingPoint(0, 0);

    b.ir.root_block->Append(var);
    auto* func = b.Function("main", ty.void_(), core::ir::Function::PipelineStage::kFragment);
    b.Append(func->Block(), [&] {
        b.Store(b.Access(ty.ptr<storage, vec4<f32>, core::Access::kReadWrite>(), var, 1_u),
                b.Splat<vec4<f32>>(5_f));
        b.Return(func);
    });

    auto result = Generate();
    ASSERT_EQ(result, Success) << result.Failure().reason << output_.glsl;
    EXPECT_EQ(output_.glsl, GlslHeader() + R"(precision highp float;
precision highp int;

layout(binding = 0, std430)
buffer f_v_block_ssbo {
  mat4 inner;
} v_1;
void main() {
  v_1.inner[1u] = vec4(5.0f);
}
)");
}

TEST_F(GlslWriterTest, AccessStoreMatrixColumnF16) {
    auto* var = b.Var<storage, mat2x3<f16>, core::Access::kReadWrite>("v");
    var->SetBindingPoint(0, 0);

    b.ir.root_block->Append(var);
    auto* func = b.Function("main", ty.void_(), core::ir::Function::PipelineStage::kFragment);
    b.Append(func->Block(), [&] {
        b.Store(b.Access(ty.ptr<storage, vec3<f16>, core::Access::kReadWrite>(), var, 1_u),
                b.Splat<vec3<f16>>(5_h));
        b.Return(func);
    });

    auto result = Generate();
    ASSERT_EQ(result, Success) << result.Failure().reason << output_.glsl;
    EXPECT_EQ(output_.glsl, GlslHeader() + R"(#extension GL_AMD_gpu_shader_half_float: require
precision highp float;
precision highp int;

layout(binding = 0, std430)
buffer f_v_block_ssbo {
  f16mat2x3 inner;
} v_1;
void main() {
  v_1.inner[1u] = f16vec3(5.0hf);
}
)");
}

TEST_F(GlslWriterTest, AccessStoreMatrix) {
    auto* var = b.Var<storage, mat4x4<f32>, core::Access::kReadWrite>("v");
    var->SetBindingPoint(0, 0);

    b.ir.root_block->Append(var);
    auto* func = b.Function("main", ty.void_(), core::ir::Function::PipelineStage::kFragment);
    b.Append(func->Block(), [&] {
        b.Store(var, b.Zero<mat4x4<f32>>());
        b.Return(func);
    });

    auto result = Generate();
    ASSERT_EQ(result, Success) << result.Failure().reason << output_.glsl;
    EXPECT_EQ(output_.glsl, GlslHeader() + R"(precision highp float;
precision highp int;

layout(binding = 0, std430)
buffer f_v_block_ssbo {
  mat4 inner;
} v_1;
void main() {
  v_1.inner = mat4(vec4(0.0f), vec4(0.0f), vec4(0.0f), vec4(0.0f));
}
)");
}

TEST_F(GlslWriterTest, AccessStoreMatrixF16) {
    auto* var = b.Var<storage, mat4x4<f16>, core::Access::kReadWrite>("v");
    var->SetBindingPoint(0, 0);

    b.ir.root_block->Append(var);
    auto* func = b.Function("main", ty.void_(), core::ir::Function::PipelineStage::kFragment);
    b.Append(func->Block(), [&] {
        b.Store(var, b.Zero<mat4x4<f16>>());
        b.Return(func);
    });

    auto result = Generate();
    ASSERT_EQ(result, Success) << result.Failure().reason << output_.glsl;
    EXPECT_EQ(output_.glsl, GlslHeader() + R"(#extension GL_AMD_gpu_shader_half_float: require
precision highp float;
precision highp int;

layout(binding = 0, std430)
buffer f_v_block_ssbo {
  f16mat4 inner;
} v_1;
void main() {
  v_1.inner = f16mat4(f16vec4(0.0hf), f16vec4(0.0hf), f16vec4(0.0hf), f16vec4(0.0hf));
}
)");
}

TEST_F(GlslWriterTest, AccessStoreArrayElement) {
    auto* var = b.Var<storage, array<f32, 5>, core::Access::kReadWrite>("v");
    var->SetBindingPoint(0, 0);

    b.ir.root_block->Append(var);
    auto* func = b.Function("main", ty.void_(), core::ir::Function::PipelineStage::kFragment);
    b.Append(func->Block(), [&] {
        b.Store(b.Access(ty.ptr<storage, f32, core::Access::kReadWrite>(), var, 3_u), 1_f);
        b.Return(func);
    });

    auto result = Generate();
    ASSERT_EQ(result, Success) << result.Failure().reason << output_.glsl;
    EXPECT_EQ(output_.glsl, GlslHeader() + R"(precision highp float;
precision highp int;

layout(binding = 0, std430)
buffer f_v_block_ssbo {
  float inner[5];
} v_1;
void main() {
  v_1.inner[3u] = 1.0f;
}
)");
}

TEST_F(GlslWriterTest, AccessStoreArrayElementF16) {
    auto* var = b.Var<storage, array<f16, 5>, core::Access::kReadWrite>("v");
    var->SetBindingPoint(0, 0);

    b.ir.root_block->Append(var);
    auto* func = b.Function("main", ty.void_(), core::ir::Function::PipelineStage::kFragment);
    b.Append(func->Block(), [&] {
        b.Store(b.Access(ty.ptr<storage, f16, core::Access::kReadWrite>(), var, 3_u), 1_h);
        b.Return(func);
    });

    auto result = Generate();
    ASSERT_EQ(result, Success) << result.Failure().reason << output_.glsl;
    EXPECT_EQ(output_.glsl, GlslHeader() + R"(#extension GL_AMD_gpu_shader_half_float: require
precision highp float;
precision highp int;

layout(binding = 0, std430)
buffer f_v_block_ssbo {
  float16_t inner[5];
} v_1;
void main() {
  v_1.inner[3u] = 1.0hf;
}
)");
}

TEST_F(GlslWriterTest, AccessStoreArray) {
    auto* var = b.Var<storage, array<vec3<f32>, 5>, core::Access::kReadWrite>("v");
    var->SetBindingPoint(0, 0);

    b.ir.root_block->Append(var);
    auto* func = b.Function("main", ty.void_(), core::ir::Function::PipelineStage::kFragment);
    b.Append(func->Block(), [&] {
        auto* ary = b.Let("ary", b.Zero<array<vec3<f32>, 5>>());
        b.Store(var, ary);
        b.Return(func);
    });

    auto result = Generate();
    ASSERT_EQ(result, Success) << result.Failure().reason << output_.glsl;
    EXPECT_EQ(output_.glsl, GlslHeader() + R"(precision highp float;
precision highp int;

layout(binding = 0, std430)
buffer f_v_block_ssbo {
  vec3 inner[5];
} v_1;
void tint_store_and_preserve_padding(vec3 value_param[5]) {
  {
    uint v_2 = 0u;
    v_2 = 0u;
    while(true) {
      uint v_3 = v_2;
      if ((v_3 >= 5u)) {
        break;
      }
      v_1.inner[v_3] = value_param[v_3];
      {
        v_2 = (v_3 + 1u);
      }
    }
  }
}
void main() {
  vec3 ary[5] = vec3[5](vec3(0.0f), vec3(0.0f), vec3(0.0f), vec3(0.0f), vec3(0.0f));
  tint_store_and_preserve_padding(ary);
}
)");
}

TEST_F(GlslWriterTest, AccessStoreStructMember) {
    auto* SB = ty.Struct(mod.symbols.New("SB"), {
                                                    {mod.symbols.New("a"), ty.i32()},
                                                    {mod.symbols.New("b"), ty.f32()},
                                                });

    auto* var = b.Var("v", storage, SB, core::Access::kReadWrite);
    var->SetBindingPoint(0, 0);

    b.ir.root_block->Append(var);
    auto* func = b.Function("main", ty.void_(), core::ir::Function::PipelineStage::kFragment);
    b.Append(func->Block(), [&] {
        b.Store(b.Access(ty.ptr<storage, f32, core::Access::kReadWrite>(), var, 1_u), 3_f);
        b.Return(func);
    });

    auto result = Generate();
    ASSERT_EQ(result, Success) << result.Failure().reason << output_.glsl;
    EXPECT_EQ(output_.glsl, GlslHeader() + R"(precision highp float;
precision highp int;


struct SB {
  int a;
  float b;
};

layout(binding = 0, std430)
buffer f_v_block_ssbo {
  SB inner;
} v_1;
void main() {
  v_1.inner.b = 3.0f;
}
)");
}

TEST_F(GlslWriterTest, AccessStoreStructMemberF16) {
    auto* SB = ty.Struct(mod.symbols.New("SB"), {
                                                    {mod.symbols.New("a"), ty.i32()},
                                                    {mod.symbols.New("b"), ty.f16()},
                                                });

    auto* var = b.Var("v", storage, SB, core::Access::kReadWrite);
    var->SetBindingPoint(0, 0);

    b.ir.root_block->Append(var);
    auto* func = b.Function("main", ty.void_(), core::ir::Function::PipelineStage::kFragment);
    b.Append(func->Block(), [&] {
        b.Store(b.Access(ty.ptr<storage, f16, core::Access::kReadWrite>(), var, 1_u), 3_h);
        b.Return(func);
    });

    auto result = Generate();
    ASSERT_EQ(result, Success) << result.Failure().reason << output_.glsl;
    EXPECT_EQ(output_.glsl, GlslHeader() + R"(#extension GL_AMD_gpu_shader_half_float: require
precision highp float;
precision highp int;


struct SB {
  int a;
  float16_t b;
};

layout(binding = 0, std430)
buffer f_v_block_ssbo {
  SB inner;
} v_1;
void main() {
  v_1.inner.b = 3.0hf;
}
)");
}

TEST_F(GlslWriterTest, AccessStoreStructNested) {
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
    auto* func = b.Function("main", ty.void_(), core::ir::Function::PipelineStage::kFragment);
    b.Append(func->Block(), [&] {
        b.Store(b.Access(ty.ptr<storage, f32, core::Access::kReadWrite>(), var, 1_u, 0_u), 2_f);
        b.Return(func);
    });

    auto result = Generate();
    ASSERT_EQ(result, Success) << result.Failure().reason << output_.glsl;
    EXPECT_EQ(output_.glsl, GlslHeader() + R"(precision highp float;
precision highp int;


struct Inner {
  mat3 s;
  vec3 t[5];
};

struct Outer {
  float x;
  uint tint_pad_0;
  uint tint_pad_1;
  uint tint_pad_2;
  Inner y;
};

struct SB {
  int a;
  uint tint_pad_0;
  uint tint_pad_1;
  uint tint_pad_2;
  Outer b;
};

layout(binding = 0, std430)
buffer f_v_block_ssbo {
  SB inner;
} v_1;
void main() {
  v_1.inner.b.x = 2.0f;
}
)");
}

TEST_F(GlslWriterTest, AccessStoreStruct) {
    auto* Inner = ty.Struct(mod.symbols.New("Inner"), {
                                                          {mod.symbols.New("s"), ty.f32()},
                                                          {mod.symbols.New("t"), ty.vec3f()},
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
    auto* func = b.Function("main", ty.void_(), core::ir::Function::PipelineStage::kFragment);
    b.Append(func->Block(), [&] {
        auto* s = b.Let("s", b.Zero(SB));
        b.Store(var, s);
        b.Return(func);
    });

    auto result = Generate();
    ASSERT_EQ(result, Success) << result.Failure().reason << output_.glsl;
    EXPECT_EQ(output_.glsl, GlslHeader() + R"(precision highp float;
precision highp int;


struct Inner {
  float s;
  uint tint_pad_0;
  uint tint_pad_1;
  uint tint_pad_2;
  vec3 t;
  uint tint_pad_3;
};

struct Outer {
  float x;
  uint tint_pad_0;
  uint tint_pad_1;
  uint tint_pad_2;
  Inner y;
};

struct SB {
  int a;
  uint tint_pad_0;
  uint tint_pad_1;
  uint tint_pad_2;
  Outer b;
};

layout(binding = 0, std430)
buffer f_v_block_ssbo {
  SB inner;
} v_1;
void tint_store_and_preserve_padding_2(Inner value_param) {
  v_1.inner.b.y.s = value_param.s;
  v_1.inner.b.y.t = value_param.t;
}
void tint_store_and_preserve_padding_1(Outer value_param) {
  v_1.inner.b.x = value_param.x;
  tint_store_and_preserve_padding_2(value_param.y);
}
void tint_store_and_preserve_padding(SB value_param) {
  v_1.inner.a = value_param.a;
  tint_store_and_preserve_padding_1(value_param.b);
}
void main() {
  SB s = SB(0, 0u, 0u, 0u, Outer(0.0f, 0u, 0u, 0u, Inner(0.0f, 0u, 0u, 0u, vec3(0.0f), 0u)));
  tint_store_and_preserve_padding(s);
}
)");
}

TEST_F(GlslWriterTest, AccessStoreStructComplex) {
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
    auto* func = b.Function("main", ty.void_(), core::ir::Function::PipelineStage::kFragment);
    b.Append(func->Block(), [&] {
        auto* s = b.Let("s", b.Zero(SB));
        b.Store(var, s);
        b.Return(func);
    });

    auto result = Generate();
    ASSERT_EQ(result, Success) << result.Failure().reason << output_.glsl;
    EXPECT_EQ(output_.glsl, GlslHeader() + R"(precision highp float;
precision highp int;


struct Inner {
  mat3 s;
  vec3 t[5];
};

struct Outer {
  float x;
  uint tint_pad_0;
  uint tint_pad_1;
  uint tint_pad_2;
  Inner y;
};

struct SB {
  int a;
  uint tint_pad_0;
  uint tint_pad_1;
  uint tint_pad_2;
  Outer b;
};

layout(binding = 0, std430)
buffer f_v_block_ssbo {
  SB inner;
} v_1;
void tint_store_and_preserve_padding_4(vec3 value_param[5]) {
  {
    uint v_2 = 0u;
    v_2 = 0u;
    while(true) {
      uint v_3 = v_2;
      if ((v_3 >= 5u)) {
        break;
      }
      v_1.inner.b.y.t[v_3] = value_param[v_3];
      {
        v_2 = (v_3 + 1u);
      }
    }
  }
}
void tint_store_and_preserve_padding_3(mat3 value_param) {
  v_1.inner.b.y.s[0u] = value_param[0u];
  v_1.inner.b.y.s[1u] = value_param[1u];
  v_1.inner.b.y.s[2u] = value_param[2u];
}
void tint_store_and_preserve_padding_2(Inner value_param) {
  tint_store_and_preserve_padding_3(value_param.s);
  tint_store_and_preserve_padding_4(value_param.t);
}
void tint_store_and_preserve_padding_1(Outer value_param) {
  v_1.inner.b.x = value_param.x;
  tint_store_and_preserve_padding_2(value_param.y);
}
void tint_store_and_preserve_padding(SB value_param) {
  v_1.inner.a = value_param.a;
  tint_store_and_preserve_padding_1(value_param.b);
}
void main() {
  SB s = SB(0, 0u, 0u, 0u, Outer(0.0f, 0u, 0u, 0u, Inner(mat3(vec3(0.0f), vec3(0.0f), vec3(0.0f)), vec3[5](vec3(0.0f), vec3(0.0f), vec3(0.0f), vec3(0.0f), vec3(0.0f)))));
  tint_store_and_preserve_padding(s);
}
)");
}

TEST_F(GlslWriterTest, AccessChainReused) {
    auto* sb = ty.Struct(mod.symbols.New("SB"), {
                                                    {mod.symbols.New("a"), ty.i32()},
                                                    {mod.symbols.New("b"), ty.vec3f()},
                                                });

    auto* var = b.Var("v", storage, sb, core::Access::kReadWrite);
    var->SetBindingPoint(0, 0);
    b.ir.root_block->Append(var);

    auto* func = b.Function("main", ty.void_(), core::ir::Function::PipelineStage::kFragment);
    b.Append(func->Block(), [&] {
        auto* x = b.Access(ty.ptr(storage, ty.vec3f(), core::Access::kReadWrite), var, 1_u);
        b.Let("b", b.LoadVectorElement(x, 1_u));
        b.Let("c", b.LoadVectorElement(x, 2_u));
        b.Return(func);
    });

    auto result = Generate();
    ASSERT_EQ(result, Success) << result.Failure().reason << output_.glsl;
    EXPECT_EQ(output_.glsl, GlslHeader() + R"(precision highp float;
precision highp int;


struct SB {
  int a;
  uint tint_pad_0;
  uint tint_pad_1;
  uint tint_pad_2;
  vec3 b;
  uint tint_pad_3;
};

layout(binding = 0, std430)
buffer f_v_block_ssbo {
  SB inner;
} v_1;
void main() {
  float b = v_1.inner.b.y;
  float c = v_1.inner.b.z;
}
)");
}

TEST_F(GlslWriterTest, AccessUniformChainReused) {
    auto* sb = ty.Struct(mod.symbols.New("SB"), {
                                                    {mod.symbols.New("c"), ty.f32()},
                                                    {mod.symbols.New("d"), ty.vec3f()},
                                                });

    auto* var = b.Var("v", uniform, sb, core::Access::kRead);
    var->SetBindingPoint(0, 0);
    b.ir.root_block->Append(var);

    auto* func = b.Function("main", ty.void_(), core::ir::Function::PipelineStage::kFragment);
    b.Append(func->Block(), [&] {
        auto* x = b.Access(ty.ptr(uniform, ty.vec3f(), core::Access::kRead), var, 1_u);
        b.Let("b", b.LoadVectorElement(x, 1_u));
        b.Let("c", b.LoadVectorElement(x, 2_u));
        b.Return(func);
    });

    auto result = Generate();
    ASSERT_EQ(result, Success) << result.Failure().reason << output_.glsl;
    EXPECT_EQ(output_.glsl, GlslHeader() + R"(precision highp float;
precision highp int;

layout(binding = 0, std140)
uniform f_v_block_ubo {
  uvec4 inner[2];
} v_1;
void main() {
  uvec4 v_2 = v_1.inner[1u];
  float b = uintBitsToFloat(v_2.y);
  uvec4 v_3 = v_1.inner[1u];
  float c = uintBitsToFloat(v_3.z);
}
)");
}

TEST_F(GlslWriterTest, AccessToLetWithFunctionParams) {
    auto* f = b.Function("f", ty.i32());
    b.Append(f->Block(), [&] { b.Return(f, 0_i); });

    auto* g = b.Function("g", ty.i32());
    b.Append(g->Block(), [&] { b.Return(g, 0_i); });

    auto* foo = b.ComputeFunction("main");
    b.Append(foo->Block(), [&] {
        auto* arr = b.Var("arr", ty.ptr<function, array<i32, 4>, read_write>());
        auto* c = b.Call(ty.i32(), f);
        auto* access = b.Access(ty.ptr<function, i32, read_write>(), arr, c);
        auto* p = b.Let("p", access);
        auto* c2 = b.Call(ty.i32(), g);
        b.Let("y", c2);
        b.Let("x", b.Load(p));
        b.Return(foo);
    });

    auto result = Generate();
    ASSERT_EQ(result, Success) << result.Failure().reason << output_.glsl;
    EXPECT_EQ(output_.glsl, GlslHeader() + R"(
int f() {
  return 0;
}
int g() {
  return 0;
}
layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
  int arr[4] = int[4](0, 0, 0, 0);
  uint v = min(uint(f()), 3u);
  int y = g();
  int x = arr[v];
}
)");
}

TEST_F(GlslWriterTest, AccessToLetWithNestedFunctionParams) {
    auto* f = b.Function("f", ty.i32());
    b.Append(f->Block(), [&] { b.Return(f, 0_i); });

    auto* g = b.Function("g", ty.i32());
    b.Append(g->Block(), [&] { b.Return(g, 0_i); });

    auto* foo = b.ComputeFunction("main");
    b.Append(foo->Block(), [&] {
        auto* arr = b.Var("arr", ty.ptr<function, array<i32, 4>, read_write>());
        auto* c = b.Call(ty.i32(), f);
        auto* d = b.Add(c, 1_i);
        auto* access = b.Access(ty.ptr<function, i32, read_write>(), arr, d);
        auto* p = b.Let("p", access);
        auto* c2 = b.Call(ty.i32(), g);
        b.Let("y", c2);
        b.Let("x", b.Load(p));
        b.Return(foo);
    });

    auto result = Generate();
    ASSERT_EQ(result, Success) << result.Failure().reason << output_.glsl;
    EXPECT_EQ(output_.glsl, GlslHeader() + R"(
int f() {
  return 0;
}
int g() {
  return 0;
}
layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
  int arr[4] = int[4](0, 0, 0, 0);
  uint v = uint(f());
  uint v_1 = min(uint(int((v + uint(1)))), 3u);
  int y = g();
  int x = arr[v_1];
}
)");
}

}  // namespace
}  // namespace tint::glsl::writer
