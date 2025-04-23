// Copyright 2021 The Dawn & Tint Authors
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

#include "src/tint/lang/core/type/sampled_texture.h"
#include "src/tint/lang/core/type/texture_dimension.h"
#include "src/tint/lang/wgsl/ast/stage_attribute.h"
#include "src/tint/lang/wgsl/ast/variable_decl_statement.h"
#include "src/tint/lang/wgsl/writer/ast_printer/helper_test.h"

#include "gmock/gmock.h"

using namespace tint::core::number_suffixes;  // NOLINT

namespace tint::wgsl::writer {
namespace {

using WgslASTPrinterTest = TestHelper;

TEST_F(WgslASTPrinterTest, Emit_GlobalDeclAfterFunction) {
    auto* func_var = Var("a", ty.f32());
    WrapInFunction(func_var);

    GlobalVar("a", ty.f32(), core::AddressSpace::kPrivate);

    ASTPrinter& gen = Build();

    gen.IncrementIndent();
    gen.Generate();
    EXPECT_THAT(gen.Diagnostics(), testing::IsEmpty());
    EXPECT_EQ(gen.Result(), R"(  @compute @workgroup_size(1u, 1u, 1u)
  fn test_function() {
    var a : f32;
  }

  var<private> a : f32;
)");
}

TEST_F(WgslASTPrinterTest, Emit_GlobalsInterleaved) {
    GlobalVar("a0", ty.f32(), core::AddressSpace::kPrivate);

    auto* s0 = Structure("S0", Vector{
                                   Member("a", ty.i32()),
                               });

    Func("func", {}, ty.f32(),
         Vector{
             Return("a0"),
         },
         tint::Empty);

    GlobalVar("a1", ty.f32(), core::AddressSpace::kPrivate);

    auto* s1 = Structure("S1", Vector{
                                   Member("a", ty.i32()),
                               });

    Func("main", {}, ty.void_(),
         Vector{
             Decl(Var("s0", ty.Of(s0))),
             Decl(Var("s1", ty.Of(s1))),
             Assign("a1", Call("func")),
         },
         Vector{
             Stage(ast::PipelineStage::kCompute),
             WorkgroupSize(1_i),
         });

    ASTPrinter& gen = Build();

    gen.IncrementIndent();
    gen.Generate();
    EXPECT_THAT(gen.Diagnostics(), testing::IsEmpty());
    EXPECT_EQ(gen.Result(), R"(  var<private> a0 : f32;

  struct S0 {
    a : i32,
  }

  fn func() -> f32 {
    return a0;
  }

  var<private> a1 : f32;

  struct S1 {
    a : i32,
  }

  @compute @workgroup_size(1i)
  fn main() {
    var s0 : S0;
    var s1 : S1;
    a1 = func();
  }
)");
}

TEST_F(WgslASTPrinterTest, Emit_Global_Sampler) {
    GlobalVar("s", ty.sampler(core::type::SamplerKind::kSampler), Group(0_a), Binding(0_a));

    ASTPrinter& gen = Build();

    gen.IncrementIndent();
    gen.Generate();
    EXPECT_THAT(gen.Diagnostics(), testing::IsEmpty());
    EXPECT_EQ(gen.Result(), "  @group(0) @binding(0) var s : sampler;\n");
}

TEST_F(WgslASTPrinterTest, Emit_Global_Texture) {
    auto st = ty.sampled_texture(core::type::TextureDimension::k1d, ty.f32());
    GlobalVar("t", st, Group(0_a), Binding(0_a));

    ASTPrinter& gen = Build();

    gen.IncrementIndent();
    gen.Generate();
    EXPECT_THAT(gen.Diagnostics(), testing::IsEmpty());
    EXPECT_EQ(gen.Result(), "  @group(0) @binding(0) var t : texture_1d<f32>;\n");
}

TEST_F(WgslASTPrinterTest, Emit_GlobalConst) {
    GlobalConst("explicit", ty.f32(), Expr(1_f));
    GlobalConst("inferred", Expr(1_f));

    ASTPrinter& gen = Build();

    gen.IncrementIndent();
    gen.Generate();
    EXPECT_THAT(gen.Diagnostics(), testing::IsEmpty());
    EXPECT_EQ(gen.Result(), R"(  const explicit : f32 = 1.0f;

  const inferred = 1.0f;
)");
}

TEST_F(WgslASTPrinterTest, Emit_OverridableConstants) {
    Override("a", ty.f32());
    Override("b", ty.f32(), Id(7_a));

    ASTPrinter& gen = Build();

    gen.IncrementIndent();
    gen.Generate();
    EXPECT_THAT(gen.Diagnostics(), testing::IsEmpty());
    EXPECT_EQ(gen.Result(), R"(  override a : f32;

  @id(7) override b : f32;
)");
}

}  // namespace
}  // namespace tint::wgsl::writer
