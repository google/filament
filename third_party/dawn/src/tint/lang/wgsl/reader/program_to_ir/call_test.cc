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

#include "src/tint/lang/core/ir/disassembler.h"
#include "src/tint/lang/wgsl/reader/program_to_ir/ir_program_test.h"

namespace tint::wgsl::reader {
namespace {

using namespace tint::core::fluent_types;     // NOLINT
using namespace tint::core::number_suffixes;  // NOLINT

using ProgramToIRCallTest = helpers::IRProgramTest;

TEST_F(ProgramToIRCallTest, EmitExpression_Bitcast) {
    Func("my_func", tint::Empty, ty.f32(), Vector{Return(0_f)});

    auto* expr = Bitcast<f32>(Call("my_func"));
    WrapInFunction(expr);

    auto m = Build();
    ASSERT_EQ(m, Success);

    EXPECT_EQ(core::ir::Disassembler(m.Get()).Plain(), R"(%my_func = func():f32 {
  $B1: {
    ret 0.0f
  }
}
%test_function = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B2: {
    %3:f32 = call %my_func
    %4:f32 = bitcast %3
    %tint_symbol:f32 = let %4
    ret
  }
}
)");
}

TEST_F(ProgramToIRCallTest, EmitStatement_Discard) {
    auto* expr = Discard();
    Func("test_function", {}, ty.void_(), expr,
         Vector{
             create<ast::StageAttribute>(ast::PipelineStage::kFragment),
         });

    auto m = Build();
    ASSERT_EQ(m, Success);

    EXPECT_EQ(core::ir::Disassembler(m.Get()).Plain(), R"(%test_function = @fragment func():void {
  $B1: {
    discard
    ret
  }
}
)");
}

TEST_F(ProgramToIRCallTest, EmitStatement_UserFunction) {
    Func("my_func", Vector{Param("p", ty.f32())}, ty.void_(), tint::Empty);

    auto* stmt = CallStmt(Call("my_func", Mul(2_a, 3_a)));
    WrapInFunction(stmt);
    auto m = Build();
    ASSERT_EQ(m, Success);

    EXPECT_EQ(core::ir::Disassembler(m.Get()).Plain(), R"(%my_func = func(%p:f32):void {
  $B1: {
    ret
  }
}
%test_function = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B2: {
    %4:void = call %my_func, 6.0f
    ret
  }
}
)");
}

TEST_F(ProgramToIRCallTest, EmitExpression_Convert) {
    auto i = GlobalVar("i", core::AddressSpace::kPrivate, Expr(1_i));
    auto* expr = Call<f32>(i);
    WrapInFunction(expr);

    auto m = Build();
    ASSERT_EQ(m, Success);

    EXPECT_EQ(core::ir::Disassembler(m.Get()).Plain(), R"($B1: {  # root
  %i:ptr<private, i32, read_write> = var, 1i
}

%test_function = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B2: {
    %3:i32 = load %i
    %4:f32 = convert %3
    %tint_symbol:f32 = let %4
    ret
  }
}
)");
}

TEST_F(ProgramToIRCallTest, EmitExpression_ConstructEmpty) {
    auto* expr = Call<vec3<f32>>();
    GlobalVar("i", core::AddressSpace::kPrivate, expr);

    auto m = Build();
    ASSERT_EQ(m, Success);

    EXPECT_EQ(core::ir::Disassembler(m.Get()).Plain(), R"($B1: {  # root
  %i:ptr<private, vec3<f32>, read_write> = var, vec3<f32>(0.0f)
}

)");
}

TEST_F(ProgramToIRCallTest, EmitExpression_Construct) {
    auto i = GlobalVar("i", core::AddressSpace::kPrivate, Expr(1_f));
    auto* expr = Call<vec3<f32>>(2_f, 3_f, i);
    WrapInFunction(expr);

    auto m = Build();
    ASSERT_EQ(m, Success);

    EXPECT_EQ(core::ir::Disassembler(m.Get()).Plain(), R"($B1: {  # root
  %i:ptr<private, f32, read_write> = var, 1.0f
}

%test_function = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B2: {
    %3:f32 = load %i
    %4:vec3<f32> = construct 2.0f, 3.0f, %3
    %tint_symbol:vec3<f32> = let %4
    ret
  }
}
)");
}

}  // namespace
}  // namespace tint::wgsl::reader
