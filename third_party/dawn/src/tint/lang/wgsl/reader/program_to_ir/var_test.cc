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

using ProgramToIRVarTest = helpers::IRProgramTest;

TEST_F(ProgramToIRVarTest, Emit_GlobalVar_NoInit) {
    GlobalVar("a", ty.u32(), core::AddressSpace::kPrivate);

    auto m = Build();
    ASSERT_EQ(m, Success);

    EXPECT_EQ(core::ir::Disassembler(m.Get()).Plain(), R"($B1: {  # root
  %a:ptr<private, u32, read_write> = var undef
}

)");
}

TEST_F(ProgramToIRVarTest, Emit_GlobalVar_Init) {
    auto* expr = Expr(2_u);
    GlobalVar("a", ty.u32(), core::AddressSpace::kPrivate, expr);

    auto m = Build();
    ASSERT_EQ(m, Success);

    EXPECT_EQ(core::ir::Disassembler(m.Get()).Plain(), R"($B1: {  # root
  %a:ptr<private, u32, read_write> = var 2u
}

)");
}

TEST_F(ProgramToIRVarTest, Emit_GlobalVar_GroupBinding) {
    GlobalVar("a", ty.u32(), core::AddressSpace::kStorage, Vector{Group(2_u), Binding(3_u)});

    auto m = Build();
    ASSERT_EQ(m, Success);

    EXPECT_EQ(core::ir::Disassembler(m.Get()).Plain(), R"($B1: {  # root
  %a:ptr<storage, u32, read> = var undef @binding_point(2, 3)
}

)");
}

TEST_F(ProgramToIRVarTest, Emit_Var_NoInit) {
    auto* a = Var("a", ty.u32(), core::AddressSpace::kFunction);
    WrapInFunction(a);

    auto m = Build();
    ASSERT_EQ(m, Success);

    EXPECT_EQ(core::ir::Disassembler(m.Get()).Plain(),
              R"(%test_function = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    %a:ptr<function, u32, read_write> = var undef
    ret
  }
}
)");
}

TEST_F(ProgramToIRVarTest, Emit_Var_Init_Constant) {
    auto* expr = Expr(2_u);
    auto* a = Var("a", ty.u32(), core::AddressSpace::kFunction, expr);
    WrapInFunction(a);

    auto m = Build();
    ASSERT_EQ(m, Success);

    EXPECT_EQ(core::ir::Disassembler(m.Get()).Plain(),
              R"(%test_function = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    %a:ptr<function, u32, read_write> = var 2u
    ret
  }
}
)");
}

TEST_F(ProgramToIRVarTest, Emit_Var_Init_NonConstant) {
    auto* a = Var("a", ty.u32(), core::AddressSpace::kFunction);
    auto* b = Var("b", ty.u32(), core::AddressSpace::kFunction, Add("a", 2_u));
    WrapInFunction(a, b);

    auto m = Build();
    ASSERT_EQ(m, Success);

    EXPECT_EQ(core::ir::Disassembler(m.Get()).Plain(),
              R"(%test_function = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    %a:ptr<function, u32, read_write> = var undef
    %3:u32 = load %a
    %4:u32 = add %3, 2u
    %b:ptr<function, u32, read_write> = var %4
    ret
  }
}
)");
}

TEST_F(ProgramToIRVarTest, Emit_Var_Assign_42i) {
    WrapInFunction(Var("a", ty.i32(), core::AddressSpace::kFunction),  //
                   Assign("a", 42_i));

    auto m = Build();
    ASSERT_EQ(m, Success);

    EXPECT_EQ(core::ir::Disassembler(m.Get()).Plain(),
              R"(%test_function = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    %a:ptr<function, i32, read_write> = var undef
    store %a, 42i
    ret
  }
}
)");
}

TEST_F(ProgramToIRVarTest, Emit_Var_Assign_ArrayOfArray_EvalOrder) {
    Func("f", Vector{Param("p", ty.i32())}, ty.i32(), Vector{Return("p")});

    auto* lhs =                                 //
        IndexAccessor(                          //
            IndexAccessor(                      //
                IndexAccessor("a",              //
                              Call("f", 1_i)),  //
                Call("f", 2_i)),                //
            Call("f", 3_i));

    auto* rhs =                                 //
        IndexAccessor(                          //
            IndexAccessor(                      //
                IndexAccessor("a",              //
                              Call("f", 4_i)),  //
                Call("f", 5_i)),                //
            Call("f", 6_i));

    WrapInFunction(
        Var("a", ty.array<array<array<i32, 5>, 5>, 5>(), core::AddressSpace::kFunction),  //
        Assign(lhs, rhs));

    auto m = Build();
    ASSERT_EQ(m, Success);

    EXPECT_EQ(core::ir::Disassembler(m.Get()).Plain(),
              R"(%f = func(%p:i32):i32 {
  $B1: {
    ret %p
  }
}
%test_function = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B2: {
    %a:ptr<function, array<array<array<i32, 5>, 5>, 5>, read_write> = var undef
    %5:i32 = call %f, 1i
    %6:i32 = call %f, 2i
    %7:i32 = call %f, 3i
    %8:ptr<function, i32, read_write> = access %a, %5, %6, %7
    %9:i32 = call %f, 4i
    %10:i32 = call %f, 5i
    %11:i32 = call %f, 6i
    %12:ptr<function, i32, read_write> = access %a, %9, %10, %11
    %13:i32 = load %12
    store %8, %13
    ret
  }
}
)");
}

TEST_F(ProgramToIRVarTest, Emit_Var_Assign_ArrayOfVec_EvalOrder) {
    Func("f", Vector{Param("p", ty.i32())}, ty.i32(), Vector{Return("p")});

    auto* lhs =                             //
        IndexAccessor(                      //
            IndexAccessor("a",              //
                          Call("f", 1_i)),  //
            Call("f", 2_i));

    auto* rhs =                             //
        IndexAccessor(                      //
            IndexAccessor("a",              //
                          Call("f", 4_i)),  //
            Call("f", 5_i));

    WrapInFunction(Var("a", ty.array<vec4<f32>, 5>(), core::AddressSpace::kFunction),  //
                   Assign(lhs, rhs));

    auto m = Build();
    ASSERT_EQ(m, Success);

    EXPECT_EQ(core::ir::Disassembler(m.Get()).Plain(),
              R"(%f = func(%p:i32):i32 {
  $B1: {
    ret %p
  }
}
%test_function = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B2: {
    %a:ptr<function, array<vec4<f32>, 5>, read_write> = var undef
    %5:i32 = call %f, 1i
    %6:ptr<function, vec4<f32>, read_write> = access %a, %5
    %7:i32 = call %f, 2i
    %8:i32 = call %f, 4i
    %9:ptr<function, vec4<f32>, read_write> = access %a, %8
    %10:i32 = call %f, 5i
    %11:f32 = load_vector_element %9, %10
    store_vector_element %6, %7, %11
    ret
  }
}
)");
}

TEST_F(ProgramToIRVarTest, Emit_Var_Assign_ArrayOfMatrix_EvalOrder) {
    Func("f", Vector{Param("p", ty.i32())}, ty.i32(), Vector{Return("p")});

    auto* lhs =                                 //
        IndexAccessor(                          //
            IndexAccessor(                      //
                IndexAccessor("a",              //
                              Call("f", 1_i)),  //
                Call("f", 2_i)),                //
            Call("f", 3_i));

    auto* rhs =                                 //
        IndexAccessor(                          //
            IndexAccessor(                      //
                IndexAccessor("a",              //
                              Call("f", 4_i)),  //
                Call("f", 5_i)),                //
            Call("f", 6_i));

    WrapInFunction(Var("a", ty.array<mat3x4<f32>, 5>(), core::AddressSpace::kFunction),  //
                   Assign(lhs, rhs));

    auto m = Build();
    ASSERT_EQ(m, Success);

    EXPECT_EQ(core::ir::Disassembler(m.Get()).Plain(),
              R"(%f = func(%p:i32):i32 {
  $B1: {
    ret %p
  }
}
%test_function = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B2: {
    %a:ptr<function, array<mat3x4<f32>, 5>, read_write> = var undef
    %5:i32 = call %f, 1i
    %6:i32 = call %f, 2i
    %7:ptr<function, vec4<f32>, read_write> = access %a, %5, %6
    %8:i32 = call %f, 3i
    %9:i32 = call %f, 4i
    %10:i32 = call %f, 5i
    %11:ptr<function, vec4<f32>, read_write> = access %a, %9, %10
    %12:i32 = call %f, 6i
    %13:f32 = load_vector_element %11, %12
    store_vector_element %7, %8, %13
    ret
  }
}
)");
}

TEST_F(ProgramToIRVarTest, Emit_Var_CompoundAssign_42i) {
    WrapInFunction(Var("a", ty.i32(), core::AddressSpace::kFunction),  //
                   CompoundAssign("a", 42_i, core::BinaryOp::kAdd));

    auto m = Build();
    ASSERT_EQ(m, Success);

    EXPECT_EQ(core::ir::Disassembler(m.Get()).Plain(),
              R"(%test_function = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    %a:ptr<function, i32, read_write> = var undef
    %3:i32 = load %a
    %4:i32 = add %3, 42i
    store %a, %4
    ret
  }
}
)");
}

TEST_F(ProgramToIRVarTest, Emit_Var_CompoundAssign_ArrayOfArray_EvalOrder) {
    Func("f", Vector{Param("p", ty.i32())}, ty.i32(), Vector{Return("p")});

    auto* lhs =                                 //
        IndexAccessor(                          //
            IndexAccessor(                      //
                IndexAccessor("a",              //
                              Call("f", 1_i)),  //
                Call("f", 2_i)),                //
            Call("f", 3_i));

    auto* rhs =                                 //
        IndexAccessor(                          //
            IndexAccessor(                      //
                IndexAccessor("a",              //
                              Call("f", 4_i)),  //
                Call("f", 5_i)),                //
            Call("f", 6_i));

    WrapInFunction(
        Var("a", ty.array<array<array<i32, 5>, 5>, 5>(), core::AddressSpace::kFunction),  //
        CompoundAssign(lhs, rhs, core::BinaryOp::kAdd));

    auto m = Build();
    ASSERT_EQ(m, Success);

    EXPECT_EQ(core::ir::Disassembler(m.Get()).Plain(),
              R"(%f = func(%p:i32):i32 {
  $B1: {
    ret %p
  }
}
%test_function = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B2: {
    %a:ptr<function, array<array<array<i32, 5>, 5>, 5>, read_write> = var undef
    %5:i32 = call %f, 1i
    %6:i32 = call %f, 2i
    %7:i32 = call %f, 3i
    %8:ptr<function, i32, read_write> = access %a, %5, %6, %7
    %9:i32 = call %f, 4i
    %10:i32 = call %f, 5i
    %11:i32 = call %f, 6i
    %12:ptr<function, i32, read_write> = access %a, %9, %10, %11
    %13:i32 = load %12
    %14:i32 = load %8
    %15:i32 = add %14, %13
    store %8, %15
    ret
  }
}
)");
}

TEST_F(ProgramToIRVarTest, Emit_Var_CompoundAssign_ArrayOfMatrix_EvalOrder) {
    Func("f", Vector{Param("p", ty.i32())}, ty.i32(), Vector{Return("p")});

    auto* lhs =                                 //
        IndexAccessor(                          //
            IndexAccessor(                      //
                IndexAccessor("a",              //
                              Call("f", 1_i)),  //
                Call("f", 2_i)),                //
            Call("f", 3_i));

    auto* rhs =                                 //
        IndexAccessor(                          //
            IndexAccessor(                      //
                IndexAccessor("a",              //
                              Call("f", 4_i)),  //
                Call("f", 5_i)),                //
            Call("f", 6_i));

    WrapInFunction(Var("a", ty.array<mat3x4<f32>, 5>(), core::AddressSpace::kFunction),  //
                   CompoundAssign(lhs, rhs, core::BinaryOp::kAdd));

    auto m = Build();
    ASSERT_EQ(m, Success);

    EXPECT_EQ(core::ir::Disassembler(m.Get()).Plain(),
              R"(%f = func(%p:i32):i32 {
  $B1: {
    ret %p
  }
}
%test_function = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B2: {
    %a:ptr<function, array<mat3x4<f32>, 5>, read_write> = var undef
    %5:i32 = call %f, 1i
    %6:i32 = call %f, 2i
    %7:ptr<function, vec4<f32>, read_write> = access %a, %5, %6
    %8:i32 = call %f, 3i
    %9:i32 = call %f, 4i
    %10:i32 = call %f, 5i
    %11:ptr<function, vec4<f32>, read_write> = access %a, %9, %10
    %12:i32 = call %f, 6i
    %13:f32 = load_vector_element %11, %12
    %14:f32 = load_vector_element %7, %8
    %15:f32 = add %14, %13
    store_vector_element %7, %8, %15
    ret
  }
}
)");
}

}  // namespace
}  // namespace tint::wgsl::reader
