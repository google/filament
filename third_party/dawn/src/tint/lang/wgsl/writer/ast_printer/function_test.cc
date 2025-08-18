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

#include "src/tint/lang/core/enums.h"
#include "src/tint/lang/core/fluent_types.h"
#include "src/tint/lang/wgsl/ast/stage_attribute.h"
#include "src/tint/lang/wgsl/ast/variable_decl_statement.h"
#include "src/tint/lang/wgsl/ast/workgroup_attribute.h"
#include "src/tint/lang/wgsl/writer/ast_printer/helper_test.h"

#include "gmock/gmock.h"

using namespace tint::core::number_suffixes;  // NOLINT
using namespace tint::core::fluent_types;     // NOLINT

namespace tint::wgsl::writer {
namespace {

using WgslASTPrinterTest = TestHelper;

TEST_F(WgslASTPrinterTest, Emit_Function) {
    auto* func = Func("my_func", tint::Empty, ty.void_(),
                      Vector{
                          Return(),
                      });

    ASTPrinter& gen = Build();

    gen.IncrementIndent();
    gen.EmitFunction(func);
    EXPECT_THAT(gen.Diagnostics(), testing::IsEmpty());
    EXPECT_EQ(gen.Result(), R"(  fn my_func() {
    return;
  }
)");
}

TEST_F(WgslASTPrinterTest, Emit_Function_WithParams) {
    auto* func = Func("my_func",
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
    gen.EmitFunction(func);
    EXPECT_THAT(gen.Diagnostics(), testing::IsEmpty());
    EXPECT_EQ(gen.Result(), R"(  fn my_func(a : f32, b : i32) {
    return;
  }
)");
}

TEST_F(WgslASTPrinterTest, Emit_Function_WithAttribute_WorkgroupSize) {
    auto* func = Func("my_func", tint::Empty, ty.void_(),
                      Vector{
                          Return(),
                      },
                      Vector{
                          Stage(ast::PipelineStage::kCompute),
                          WorkgroupSize(2_i, 4_i, 6_i),
                      });

    ASTPrinter& gen = Build();

    gen.IncrementIndent();
    gen.EmitFunction(func);
    EXPECT_THAT(gen.Diagnostics(), testing::IsEmpty());
    EXPECT_EQ(gen.Result(), R"(  @compute @workgroup_size(2i, 4i, 6i)
  fn my_func() {
    return;
  }
)");
}

TEST_F(WgslASTPrinterTest, Emit_Function_WithAttribute_MustUse) {
    auto* func = Func("my_func", tint::Empty, ty.i32(),
                      Vector{
                          Return(1_i),
                      },
                      Vector{
                          MustUse(),
                      });

    ASTPrinter& gen = Build();

    gen.IncrementIndent();
    gen.EmitFunction(func);
    EXPECT_THAT(gen.Diagnostics(), testing::IsEmpty());
    EXPECT_EQ(gen.Result(), R"(  @must_use
  fn my_func() -> i32 {
    return 1i;
  }
)");
}

TEST_F(WgslASTPrinterTest, Emit_Function_WithAttribute_WorkgroupSize_WithIdent) {
    GlobalConst("height", ty.i32(), Expr(2_i));
    auto* func = Func("my_func", tint::Empty, ty.void_(),
                      Vector{
                          Return(),
                      },
                      Vector{
                          Stage(ast::PipelineStage::kCompute),
                          WorkgroupSize(2_i, "height"),
                      });

    ASTPrinter& gen = Build();

    gen.IncrementIndent();
    gen.EmitFunction(func);
    EXPECT_THAT(gen.Diagnostics(), testing::IsEmpty());
    EXPECT_EQ(gen.Result(), R"(  @compute @workgroup_size(2i, height)
  fn my_func() {
    return;
  }
)");
}

TEST_F(WgslASTPrinterTest, Emit_Function_EntryPoint_Parameters) {
    auto vec4 = ty.vec4<f32>();
    auto* coord = Param("coord", vec4,
                        Vector{
                            Builtin(core::BuiltinValue::kPosition),
                        });
    auto* loc1 = Param("loc1", ty.f32(),
                       Vector{
                           Location(1_a),
                       });
    auto* func = Func("frag_main", Vector{coord, loc1}, ty.void_(), tint::Empty,
                      Vector{
                          Stage(ast::PipelineStage::kFragment),
                      });

    ASTPrinter& gen = Build();

    gen.IncrementIndent();
    gen.EmitFunction(func);
    EXPECT_THAT(gen.Diagnostics(), testing::IsEmpty());
    EXPECT_EQ(gen.Result(), R"(  @fragment
  fn frag_main(@builtin(position) coord : vec4<f32>, @location(1) loc1 : f32) {
  }
)");
}

TEST_F(WgslASTPrinterTest, Emit_Function_EntryPoint_ReturnValue) {
    auto* func = Func("frag_main", tint::Empty, ty.f32(),
                      Vector{
                          Return(1_f),
                      },
                      Vector{
                          Stage(ast::PipelineStage::kFragment),
                      },
                      Vector{
                          Location(1_a),
                      });

    ASTPrinter& gen = Build();

    gen.IncrementIndent();
    gen.EmitFunction(func);
    EXPECT_THAT(gen.Diagnostics(), testing::IsEmpty());
    EXPECT_EQ(gen.Result(), R"(  @fragment
  fn frag_main() -> @location(1) f32 {
    return 1.0f;
  }
)");
}

// https://crbug.com/tint/297
TEST_F(WgslASTPrinterTest, Emit_Function_Multiple_EntryPoint_With_Same_ModuleVar) {
    // struct Data {
    //   d : f32;
    // };
    // @binding(0) @group(0) var<storage> data : Data;
    //
    // @compute @workgroup_size(1)
    // fn a() {
    //   return;
    // }
    //
    // @compute @workgroup_size(1)
    // fn b() {
    //   return;
    // }

    auto* s = Structure("Data", Vector{
                                    Member("d", ty.f32()),
                                });

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

    ASTPrinter& gen = Build();

    gen.Generate();
    EXPECT_THAT(gen.Diagnostics(), testing::IsEmpty());
    EXPECT_EQ(gen.Result(), R"(struct Data {
  d : f32,
}

@binding(0) @group(0) var<storage, read_write> data : Data;

@compute @workgroup_size(1i)
fn a() {
  var v : f32 = data.d;
  return;
}

@compute @workgroup_size(1i)
fn b() {
  var v : f32 = data.d;
  return;
}
)");
}

}  // namespace
}  // namespace tint::wgsl::writer
