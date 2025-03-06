// Copyright 2020 The Dawn & Tint Authors
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
#include "src/tint/lang/hlsl/writer/ast_printer/helper_test.h"
#include "src/tint/lang/wgsl/ast/stage_attribute.h"
#include "src/tint/lang/wgsl/ast/variable_decl_statement.h"
#include "src/tint/lang/wgsl/ast/workgroup_attribute.h"

using ::testing::HasSubstr;

namespace tint::hlsl::writer {
namespace {

using namespace tint::core::fluent_types;     // NOLINT
using namespace tint::core::number_suffixes;  // NOLINT

using HlslASTPrinterTest_Function = TestHelper;

TEST_F(HlslASTPrinterTest_Function, Emit_Function) {
    Func("my_func", tint::Empty, ty.void_(),
         Vector{
             Return(),
         });

    ASTPrinter& gen = Build();

    gen.IncrementIndent();

    ASSERT_TRUE(gen.Generate()) << gen.Diagnostics();
    EXPECT_EQ(gen.Result(), R"(  void my_func() {
    return;
  }
)");
}

TEST_F(HlslASTPrinterTest_Function, Emit_Function_Name_Collision) {
    Func("GeometryShader", tint::Empty, ty.void_(),
         Vector{
             Return(),
         });

    ASTPrinter& gen = SanitizeAndBuild();

    gen.IncrementIndent();

    ASSERT_TRUE(gen.Generate()) << gen.Diagnostics();
    EXPECT_THAT(gen.Result(), HasSubstr(R"(  void tint_symbol() {
    return;
  })"));
}

TEST_F(HlslASTPrinterTest_Function, Emit_Function_WithParams) {
    Func("my_func",
         Vector{
             Param("a", ty.f32()),
             Param("b", ty.i32()),
         },
         ty.void_(),
         Vector{
             Return(),
         });

    ASTPrinter& gen = Build();

    gen.IncrementIndent();

    ASSERT_TRUE(gen.Generate()) << gen.Diagnostics();
    EXPECT_EQ(gen.Result(), R"(  void my_func(float a, int b) {
    return;
  }
)");
}

TEST_F(HlslASTPrinterTest_Function, Emit_Attribute_EntryPoint_NoReturn_Void) {
    Func("main", tint::Empty, ty.void_(), tint::Empty /* no explicit return */,
         Vector{
             Stage(ast::PipelineStage::kFragment),
         });

    ASTPrinter& gen = Build();

    ASSERT_TRUE(gen.Generate()) << gen.Diagnostics();
    EXPECT_EQ(gen.Result(), R"(void main() {
  return;
}
)");
}

TEST_F(HlslASTPrinterTest_Function, PtrParameter) {
    // fn f(foo : ptr<function, f32>) -> f32 {
    //   return *foo;
    // }
    Func("f", Vector{Param("foo", ty.ptr<function, f32>())}, ty.f32(),
         Vector{Return(Deref("foo"))});

    ASTPrinter& gen = SanitizeAndBuild();

    ASSERT_TRUE(gen.Generate()) << gen.Diagnostics();
    EXPECT_THAT(gen.Result(), HasSubstr(R"(float f(inout float foo) {
  return foo;
}
)"));
}

TEST_F(HlslASTPrinterTest_Function, Emit_Attribute_EntryPoint_WithInOutVars) {
    // fn frag_main(@location(0) foo : f32) -> @location(1) f32 {
    //   return foo;
    // }
    auto* foo_in = Param("foo", ty.f32(), Vector{Location(0_a)});
    Func("frag_main", Vector{foo_in}, ty.f32(),
         Vector{
             Return("foo"),
         },
         Vector{
             Stage(ast::PipelineStage::kFragment),
         },
         Vector{
             Location(1_a),
         });

    ASTPrinter& gen = SanitizeAndBuild();

    ASSERT_TRUE(gen.Generate()) << gen.Diagnostics();
    EXPECT_EQ(gen.Result(), R"(struct tint_symbol_1 {
  float foo : TEXCOORD0;
};
struct tint_symbol_2 {
  float value : SV_Target1;
};

float frag_main_inner(float foo) {
  return foo;
}

tint_symbol_2 frag_main(tint_symbol_1 tint_symbol) {
  float inner_result = frag_main_inner(tint_symbol.foo);
  tint_symbol_2 wrapper_result = (tint_symbol_2)0;
  wrapper_result.value = inner_result;
  return wrapper_result;
}
)");
}

TEST_F(HlslASTPrinterTest_Function, Emit_Attribute_EntryPoint_WithInOut_Builtins) {
    // fn frag_main(@position(0) coord : vec4<f32>) -> @frag_depth f32 {
    //   return coord.x;
    // }
    auto* coord_in = Param("coord", ty.vec4<f32>(), Vector{Builtin(core::BuiltinValue::kPosition)});
    Func("frag_main", Vector{coord_in}, ty.f32(),
         Vector{
             Return(MemberAccessor("coord", "x")),
         },
         Vector{
             Stage(ast::PipelineStage::kFragment),
         },
         Vector{
             Builtin(core::BuiltinValue::kFragDepth),
         });

    ASTPrinter& gen = SanitizeAndBuild();

    ASSERT_TRUE(gen.Generate()) << gen.Diagnostics();
    EXPECT_EQ(gen.Result(), R"(struct tint_symbol_1 {
  float4 coord : SV_Position;
};
struct tint_symbol_2 {
  float value : SV_Depth;
};

float frag_main_inner(float4 coord) {
  return coord.x;
}

tint_symbol_2 frag_main(tint_symbol_1 tint_symbol) {
  float inner_result = frag_main_inner(float4(tint_symbol.coord.xyz, (1.0f / tint_symbol.coord.w)));
  tint_symbol_2 wrapper_result = (tint_symbol_2)0;
  wrapper_result.value = inner_result;
  return wrapper_result;
}
)");
}

TEST_F(HlslASTPrinterTest_Function, Emit_Attribute_EntryPoint_SharedStruct_DifferentStages) {
    // struct Interface {
    //   @builtin(position) pos : vec4<f32>;
    //   @location(1) col1 : f32;
    //   @location(2) col2 : f32;
    // };
    // fn vert_main() -> Interface {
    //   return Interface(vec4<f32>(), 0.4, 0.6);
    // }
    // fn frag_main(inputs : Interface) {
    //   const r = inputs.col1;
    //   const g = inputs.col2;
    //   const p = inputs.pos;
    // }
    auto* interface_struct =
        Structure("Interface",
                  Vector{
                      Member("pos", ty.vec4<f32>(), Vector{Builtin(core::BuiltinValue::kPosition)}),
                      Member("col1", ty.f32(), Vector{Location(1_a)}),
                      Member("col2", ty.f32(), Vector{Location(2_a)}),
                  });

    Func("vert_main", tint::Empty, ty.Of(interface_struct),
         Vector{
             Return(Call(ty.Of(interface_struct), Call<vec4<f32>>(), 0.5_f, 0.25_f)),
         },
         Vector{Stage(ast::PipelineStage::kVertex)});

    Func("frag_main", Vector{Param("inputs", ty.Of(interface_struct))}, ty.void_(),
         Vector{
             Decl(Let("r", ty.f32(), MemberAccessor("inputs", "col1"))),
             Decl(Let("g", ty.f32(), MemberAccessor("inputs", "col2"))),
             Decl(Let("p", ty.vec4<f32>(), MemberAccessor("inputs", "pos"))),
         },
         Vector{Stage(ast::PipelineStage::kFragment)});

    ASTPrinter& gen = SanitizeAndBuild();

    ASSERT_TRUE(gen.Generate()) << gen.Diagnostics();
    EXPECT_EQ(gen.Result(), R"(struct Interface {
  float4 pos;
  float col1;
  float col2;
};
struct tint_symbol {
  float col1 : TEXCOORD1;
  float col2 : TEXCOORD2;
  float4 pos : SV_Position;
};

Interface vert_main_inner() {
  Interface tint_symbol_3 = {(0.0f).xxxx, 0.5f, 0.25f};
  return tint_symbol_3;
}

tint_symbol vert_main() {
  Interface inner_result = vert_main_inner();
  tint_symbol wrapper_result = (tint_symbol)0;
  wrapper_result.pos = inner_result.pos;
  wrapper_result.col1 = inner_result.col1;
  wrapper_result.col2 = inner_result.col2;
  return wrapper_result;
}

struct tint_symbol_2 {
  float col1 : TEXCOORD1;
  float col2 : TEXCOORD2;
  float4 pos : SV_Position;
};

void frag_main_inner(Interface inputs) {
  float r = inputs.col1;
  float g = inputs.col2;
  float4 p = inputs.pos;
}

void frag_main(tint_symbol_2 tint_symbol_1) {
  Interface tint_symbol_4 = {float4(tint_symbol_1.pos.xyz, (1.0f / tint_symbol_1.pos.w)), tint_symbol_1.col1, tint_symbol_1.col2};
  frag_main_inner(tint_symbol_4);
  return;
}
)");
}

TEST_F(HlslASTPrinterTest_Function, Emit_Attribute_EntryPoint_SharedStruct_HelperFunction) {
    // struct VertexOutput {
    //   @builtin(position) pos : vec4<f32>;
    // };
    // fn foo(x : f32) -> VertexOutput {
    //   return VertexOutput(vec4<f32>(x, x, x, 1.0));
    // }
    // fn vert_main1() -> VertexOutput {
    //   return foo(0.5);
    // }
    // fn vert_main2() -> VertexOutput {
    //   return foo(0.25);
    // }
    auto* vertex_output_struct = Structure(
        "VertexOutput",
        Vector{Member("pos", ty.vec4<f32>(), Vector{Builtin(core::BuiltinValue::kPosition)})});

    Func("foo", Vector{Param("x", ty.f32())}, ty.Of(vertex_output_struct),
         Vector{
             Return(Call(ty.Of(vertex_output_struct), Call<vec4<f32>>("x", "x", "x", 1_f))),
         },
         tint::Empty);

    Func("vert_main1", tint::Empty, ty.Of(vertex_output_struct),
         Vector{
             Return(Call("foo", Expr(0.5_f))),
         },
         Vector{Stage(ast::PipelineStage::kVertex)});

    Func("vert_main2", tint::Empty, ty.Of(vertex_output_struct),
         Vector{
             Return(Call("foo", Expr(0.25_f))),
         },
         Vector{Stage(ast::PipelineStage::kVertex)});

    ASTPrinter& gen = SanitizeAndBuild();

    ASSERT_TRUE(gen.Generate()) << gen.Diagnostics();
    EXPECT_EQ(gen.Result(), R"(struct VertexOutput {
  float4 pos;
};

VertexOutput foo(float x) {
  VertexOutput tint_symbol_2 = {float4(x, x, x, 1.0f)};
  return tint_symbol_2;
}

struct tint_symbol {
  float4 pos : SV_Position;
};

VertexOutput vert_main1_inner() {
  return foo(0.5f);
}

tint_symbol vert_main1() {
  VertexOutput inner_result = vert_main1_inner();
  tint_symbol wrapper_result = (tint_symbol)0;
  wrapper_result.pos = inner_result.pos;
  return wrapper_result;
}

struct tint_symbol_1 {
  float4 pos : SV_Position;
};

VertexOutput vert_main2_inner() {
  return foo(0.25f);
}

tint_symbol_1 vert_main2() {
  VertexOutput inner_result_1 = vert_main2_inner();
  tint_symbol_1 wrapper_result_1 = (tint_symbol_1)0;
  wrapper_result_1.pos = inner_result_1.pos;
  return wrapper_result_1;
}
)");
}

TEST_F(HlslASTPrinterTest_Function, Emit_Attribute_EntryPoint_With_Uniform) {
    auto* ubo_ty = Structure("UBO", Vector{Member("coord", ty.vec4<f32>())});
    auto* ubo =
        GlobalVar("ubo", ty.Of(ubo_ty), core::AddressSpace::kUniform, Binding(0_a), Group(1_a));

    Func("sub_func",
         Vector{
             Param("param", ty.f32()),
         },
         ty.f32(),
         Vector{
             Return(MemberAccessor(MemberAccessor(ubo, "coord"), "x")),
         });

    auto* var = Var("v", ty.f32(), Call("sub_func", 1_f));

    Func("frag_main", tint::Empty, ty.void_(),
         Vector{
             Decl(var),
             Return(),
         },
         Vector{
             Stage(ast::PipelineStage::kFragment),
         });

    ASTPrinter& gen = SanitizeAndBuild();

    ASSERT_TRUE(gen.Generate()) << gen.Diagnostics();
    EXPECT_EQ(gen.Result(), R"(cbuffer cbuffer_ubo : register(b0, space1) {
  uint4 ubo[1];
};

float sub_func(float param) {
  return asfloat(ubo[0].x);
}

void frag_main() {
  float v = sub_func(1.0f);
  return;
}
)");
}

TEST_F(HlslASTPrinterTest_Function, Emit_Attribute_EntryPoint_With_UniformStruct) {
    auto* s = Structure("Uniforms", Vector{Member("coord", ty.vec4<f32>())});

    GlobalVar("uniforms", ty.Of(s), core::AddressSpace::kUniform, Binding(0_a), Group(1_a));

    auto* var = Var("v", ty.f32(), MemberAccessor(MemberAccessor("uniforms", "coord"), "x"));

    Func("frag_main", tint::Empty, ty.void_(),
         Vector{
             Decl(var),
             Return(),
         },
         Vector{
             Stage(ast::PipelineStage::kFragment),
         });

    ASTPrinter& gen = Build();

    ASSERT_TRUE(gen.Generate()) << gen.Diagnostics();
    EXPECT_EQ(gen.Result(), R"(cbuffer cbuffer_uniforms : register(b0, space1) {
  uint4 uniforms[1];
};

void frag_main() {
  float v = uniforms.coord.x;
  return;
}
)");
}

TEST_F(HlslASTPrinterTest_Function, Emit_Attribute_EntryPoint_With_RW_StorageBuffer_Read) {
    auto* s = Structure("Data", Vector{
                                    Member("a", ty.i32()),
                                    Member("b", ty.f32()),
                                });

    GlobalVar("coord", ty.Of(s), core::AddressSpace::kStorage, core::Access::kReadWrite,
              Binding(0_a), Group(1_a));

    auto* var = Var("v", ty.f32(), MemberAccessor("coord", "b"));

    Func("frag_main", tint::Empty, ty.void_(),
         Vector{
             Decl(var),
             Return(),
         },
         Vector{
             Stage(ast::PipelineStage::kFragment),
         });

    ASTPrinter& gen = SanitizeAndBuild();

    ASSERT_TRUE(gen.Generate()) << gen.Diagnostics();
    EXPECT_EQ(gen.Result(),
              R"(RWByteAddressBuffer coord : register(u0, space1);

void frag_main() {
  float v = asfloat(coord.Load(4u));
  return;
}
)");
}

TEST_F(HlslASTPrinterTest_Function, Emit_Attribute_EntryPoint_With_RO_StorageBuffer_Read) {
    auto* s = Structure("Data", Vector{
                                    Member("a", ty.i32()),
                                    Member("b", ty.f32()),
                                });

    GlobalVar("coord", ty.Of(s), core::AddressSpace::kStorage, core::Access::kRead, Binding(0_a),
              Group(1_a));

    auto* var = Var("v", ty.f32(), MemberAccessor("coord", "b"));

    Func("frag_main", tint::Empty, ty.void_(),
         Vector{
             Decl(var),
             Return(),
         },
         Vector{
             Stage(ast::PipelineStage::kFragment),
         });

    ASTPrinter& gen = SanitizeAndBuild();

    ASSERT_TRUE(gen.Generate()) << gen.Diagnostics();
    EXPECT_EQ(gen.Result(),
              R"(ByteAddressBuffer coord : register(t0, space1);

void frag_main() {
  float v = asfloat(coord.Load(4u));
  return;
}
)");
}

TEST_F(HlslASTPrinterTest_Function, Emit_Attribute_EntryPoint_With_WO_StorageBuffer_Store) {
    auto* s = Structure("Data", Vector{
                                    Member("a", ty.i32()),
                                    Member("b", ty.f32()),
                                });

    GlobalVar("coord", ty.Of(s), core::AddressSpace::kStorage, core::Access::kReadWrite,
              Binding(0_a), Group(1_a));

    Func("frag_main", tint::Empty, ty.void_(),
         Vector{
             Assign(MemberAccessor("coord", "b"), Expr(2_f)),
             Return(),
         },
         Vector{
             Stage(ast::PipelineStage::kFragment),
         });

    ASTPrinter& gen = SanitizeAndBuild();

    ASSERT_TRUE(gen.Generate()) << gen.Diagnostics();
    EXPECT_EQ(gen.Result(),
              R"(RWByteAddressBuffer coord : register(u0, space1);

void frag_main() {
  coord.Store(4u, asuint(2.0f));
  return;
}
)");
}

TEST_F(HlslASTPrinterTest_Function, Emit_Attribute_EntryPoint_With_StorageBuffer_Store) {
    auto* s = Structure("Data", Vector{
                                    Member("a", ty.i32()),
                                    Member("b", ty.f32()),
                                });

    GlobalVar("coord", ty.Of(s), core::AddressSpace::kStorage, core::Access::kReadWrite,
              Binding(0_a), Group(1_a));

    Func("frag_main", tint::Empty, ty.void_(),
         Vector{
             Assign(MemberAccessor("coord", "b"), Expr(2_f)),
             Return(),
         },
         Vector{
             Stage(ast::PipelineStage::kFragment),
         });

    ASTPrinter& gen = SanitizeAndBuild();

    ASSERT_TRUE(gen.Generate()) << gen.Diagnostics();
    EXPECT_EQ(gen.Result(),
              R"(RWByteAddressBuffer coord : register(u0, space1);

void frag_main() {
  coord.Store(4u, asuint(2.0f));
  return;
}
)");
}

TEST_F(HlslASTPrinterTest_Function, Emit_Attribute_Called_By_EntryPoint_With_Uniform) {
    auto* s = Structure("S", Vector{Member("x", ty.f32())});
    GlobalVar("coord", ty.Of(s), core::AddressSpace::kUniform, Binding(0_a), Group(1_a));

    Func("sub_func",
         Vector{
             Param("param", ty.f32()),
         },
         ty.f32(),
         Vector{
             Return(MemberAccessor("coord", "x")),
         });

    auto* var = Var("v", ty.f32(), Call("sub_func", 1_f));

    Func("frag_main", tint::Empty, ty.void_(),
         Vector{
             Decl(var),
             Return(),
         },
         Vector{
             Stage(ast::PipelineStage::kFragment),
         });

    ASTPrinter& gen = Build();

    ASSERT_TRUE(gen.Generate()) << gen.Diagnostics();
    EXPECT_EQ(gen.Result(), R"(cbuffer cbuffer_coord : register(b0, space1) {
  uint4 coord[1];
};

float sub_func(float param) {
  return coord.x;
}

void frag_main() {
  float v = sub_func(1.0f);
  return;
}
)");
}

TEST_F(HlslASTPrinterTest_Function, Emit_Attribute_Called_By_EntryPoint_With_StorageBuffer) {
    auto* s = Structure("S", Vector{Member("x", ty.f32())});
    GlobalVar("coord", ty.Of(s), core::AddressSpace::kStorage, core::Access::kReadWrite,
              Binding(0_a), Group(1_a));

    Func("sub_func",
         Vector{
             Param("param", ty.f32()),
         },
         ty.f32(),
         Vector{
             Return(MemberAccessor("coord", "x")),
         });

    auto* var = Var("v", ty.f32(), Call("sub_func", 1_f));

    Func("frag_main", tint::Empty, ty.void_(),
         Vector{
             Decl(var),
             Return(),
         },
         Vector{
             Stage(ast::PipelineStage::kFragment),
         });

    ASTPrinter& gen = SanitizeAndBuild();

    ASSERT_TRUE(gen.Generate()) << gen.Diagnostics();
    EXPECT_EQ(gen.Result(),
              R"(RWByteAddressBuffer coord : register(u0, space1);

float sub_func(float param) {
  return asfloat(coord.Load(0u));
}

void frag_main() {
  float v = sub_func(1.0f);
  return;
}
)");
}

TEST_F(HlslASTPrinterTest_Function, Emit_Attribute_EntryPoint_WithNameCollision) {
    Func("GeometryShader", tint::Empty, ty.void_(), tint::Empty,
         Vector{
             Stage(ast::PipelineStage::kFragment),
         });

    ASTPrinter& gen = SanitizeAndBuild();

    ASSERT_TRUE(gen.Generate()) << gen.Diagnostics();
    EXPECT_EQ(gen.Result(), R"(void tint_symbol() {
  return;
}
)");
}

TEST_F(HlslASTPrinterTest_Function, Emit_Attribute_EntryPoint_Compute) {
    Func("main", tint::Empty, ty.void_(),
         Vector{
             Return(),
         },
         Vector{Stage(ast::PipelineStage::kCompute), WorkgroupSize(1_i)});

    ASTPrinter& gen = Build();

    ASSERT_TRUE(gen.Generate()) << gen.Diagnostics();
    EXPECT_EQ(gen.Result(), R"([numthreads(1, 1, 1)]
void main() {
  return;
}
)");
}

TEST_F(HlslASTPrinterTest_Function, Emit_Attribute_EntryPoint_Compute_WithWorkgroup_Literal) {
    Func("main", tint::Empty, ty.void_(), tint::Empty,
         Vector{
             Stage(ast::PipelineStage::kCompute),
             WorkgroupSize(2_i, 4_i, 6_i),
         });

    ASTPrinter& gen = Build();

    ASSERT_TRUE(gen.Generate()) << gen.Diagnostics();
    EXPECT_EQ(gen.Result(), R"([numthreads(2, 4, 6)]
void main() {
  return;
}
)");
}

TEST_F(HlslASTPrinterTest_Function, Emit_Attribute_EntryPoint_Compute_WithWorkgroup_Const) {
    GlobalConst("width", ty.i32(), Call<i32>(2_i));
    GlobalConst("height", ty.i32(), Call<i32>(3_i));
    GlobalConst("depth", ty.i32(), Call<i32>(4_i));
    Func("main", tint::Empty, ty.void_(), tint::Empty,
         Vector{
             Stage(ast::PipelineStage::kCompute),
             WorkgroupSize("width", "height", "depth"),
         });

    ASTPrinter& gen = Build();

    ASSERT_TRUE(gen.Generate()) << gen.Diagnostics();
    EXPECT_EQ(gen.Result(), R"([numthreads(2, 3, 4)]
void main() {
  return;
}
)");
}

TEST_F(HlslASTPrinterTest_Function,
       Emit_Attribute_EntryPoint_Compute_WithWorkgroup_OverridableConst) {
    Override("width", ty.i32(), Call<i32>(2_i), Id(7_u));
    Override("height", ty.i32(), Call<i32>(3_i), Id(8_u));
    Override("depth", ty.i32(), Call<i32>(4_i), Id(9_u));
    Func("main", tint::Empty, ty.void_(), tint::Empty,
         Vector{
             Stage(ast::PipelineStage::kCompute),
             WorkgroupSize("width", "height", "depth"),
         });

    ASTPrinter& gen = Build();

    EXPECT_FALSE(gen.Generate()) << gen.Diagnostics();
    EXPECT_EQ(
        gen.Diagnostics().Str(),
        R"(error: override-expressions should have been removed with the SubstituteOverride transform)");
}

TEST_F(HlslASTPrinterTest_Function, Emit_Function_WithArrayParams) {
    Func("my_func",
         Vector{
             Param("a", ty.array<f32, 5>()),
         },
         ty.void_(),
         Vector{
             Return(),
         });

    ASTPrinter& gen = Build();

    ASSERT_TRUE(gen.Generate()) << gen.Diagnostics();
    EXPECT_EQ(gen.Result(), R"(void my_func(float a[5]) {
  return;
}
)");
}

TEST_F(HlslASTPrinterTest_Function, Emit_Function_WithArrayReturn) {
    Func("my_func", tint::Empty, ty.array<f32, 5>(),
         Vector{
             Return(Call<array<f32, 5>>()),
         });

    ASTPrinter& gen = Build();

    ASSERT_TRUE(gen.Generate()) << gen.Diagnostics();
    EXPECT_EQ(gen.Result(), R"(typedef float my_func_ret[5];
my_func_ret my_func() {
  return (float[5])0;
}
)");
}

TEST_F(HlslASTPrinterTest_Function, Emit_Function_WithDiscardAndVoidReturn) {
    Func("my_func", Vector{Param("a", ty.i32())}, ty.void_(),
         Vector{
             If(Equal("a", 0_i),  //
                Block(Discard())),
             Return(),
         });

    ASTPrinter& gen = Build();

    ASSERT_TRUE(gen.Generate()) << gen.Diagnostics();
    EXPECT_EQ(gen.Result(), R"(void my_func(int a) {
  if ((a == 0)) {
    discard;
  }
  return;
}
)");
}

TEST_F(HlslASTPrinterTest_Function, Emit_Function_WithDiscardAndNonVoidReturn) {
    Func("my_func", Vector{Param("a", ty.i32())}, ty.i32(),
         Vector{
             If(Equal("a", 0_i),  //
                Block(Discard())),
             Return(42_i),
         });

    ASTPrinter& gen = Build();

    ASSERT_TRUE(gen.Generate()) << gen.Diagnostics();
    EXPECT_EQ(gen.Result(), R"(int my_func(int a) {
  if (true) {
    if ((a == 0)) {
      discard;
    }
    return 42;
  }
  int unused;
  return unused;
}
)");
}

// https://crbug.com/tint/297
TEST_F(HlslASTPrinterTest_Function, Emit_Multiple_EntryPoint_With_Same_ModuleVar) {
    // struct Data {
    //   d : f32;
    // };
    // @binding(0) @group(0) var<storage> data : Data;
    //
    // @compute @workgroup_size(1)
    // fn a() {
    //   var v = data.d;
    //   return;
    // }
    //
    // @compute @workgroup_size(1)
    // fn b() {
    //   var v = data.d;
    //   return;
    // }

    auto* s = Structure("Data", Vector{Member("d", ty.f32())});

    GlobalVar("data", ty.Of(s), core::AddressSpace::kStorage, core::Access::kReadWrite,
              Binding(0_a), Group(0_a));

    {
        auto* var = Var("v", ty.f32(), MemberAccessor("data", "d"));

        Func("a", tint::Empty, ty.void_(),
             Vector{
                 Decl(var),
                 Return(),
             },
             Vector{
                 Stage(ast::PipelineStage::kCompute),
                 WorkgroupSize(1_i),
             });
    }

    {
        auto* var = Var("v", ty.f32(), MemberAccessor("data", "d"));

        Func("b", tint::Empty, ty.void_(),
             Vector{
                 Decl(var),
                 Return(),
             },
             Vector{
                 Stage(ast::PipelineStage::kCompute),
                 WorkgroupSize(1_i),
             });
    }

    ASTPrinter& gen = SanitizeAndBuild();

    ASSERT_TRUE(gen.Generate()) << gen.Diagnostics();
    EXPECT_EQ(gen.Result(), R"(RWByteAddressBuffer data : register(u0);

[numthreads(1, 1, 1)]
void a() {
  float v = asfloat(data.Load(0u));
  return;
}

[numthreads(1, 1, 1)]
void b() {
  float v = asfloat(data.Load(0u));
  return;
}
)");
}

}  // namespace
}  // namespace tint::hlsl::writer
