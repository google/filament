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

using namespace tint::core::number_suffixes;  // NOLINT

using ProgramToIRBinaryTest = helpers::IRProgramTest;

TEST_F(ProgramToIRBinaryTest, EmitExpression_Binary_Add) {
    Func("my_func", tint::Empty, ty.u32(), Vector{Return(0_u)});
    auto* expr = Add(Call("my_func"), 4_u);
    WrapInFunction(expr);

    auto m = Build();
    ASSERT_EQ(m, Success);

    EXPECT_EQ(core::ir::Disassembler(m.Get()).Plain(), R"(%my_func = func():u32 {
  $B1: {
    ret 0u
  }
}
%test_function = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B2: {
    %3:u32 = call %my_func
    %4:u32 = add %3, 4u
    %tint_symbol:u32 = let %4
    ret
  }
}
)");
}

TEST_F(ProgramToIRBinaryTest, EmitExpression_Binary_Increment) {
    GlobalVar("v1", core::AddressSpace::kPrivate, ty.u32());
    auto* expr = Increment("v1");
    WrapInFunction(expr);

    auto m = Build();
    ASSERT_EQ(m, Success);

    EXPECT_EQ(core::ir::Disassembler(m.Get()).Plain(), R"($B1: {  # root
  %v1:ptr<private, u32, read_write> = var
}

%test_function = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B2: {
    %3:u32 = load %v1
    %4:u32 = add %3, 1u
    store %v1, %4
    ret
  }
}
)");
}

TEST_F(ProgramToIRBinaryTest, EmitExpression_Binary_CompoundAdd) {
    GlobalVar("v1", core::AddressSpace::kPrivate, ty.u32());
    auto* expr = CompoundAssign("v1", 1_u, core::BinaryOp::kAdd);
    WrapInFunction(expr);

    auto m = Build();
    ASSERT_EQ(m, Success);

    EXPECT_EQ(core::ir::Disassembler(m.Get()).Plain(), R"($B1: {  # root
  %v1:ptr<private, u32, read_write> = var
}

%test_function = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B2: {
    %3:u32 = load %v1
    %4:u32 = add %3, 1u
    store %v1, %4
    ret
  }
}
)");
}

TEST_F(ProgramToIRBinaryTest, EmitExpression_Binary_Subtract) {
    Func("my_func", tint::Empty, ty.u32(), Vector{Return(0_u)});
    auto* expr = Sub(Call("my_func"), 4_u);
    WrapInFunction(expr);

    auto m = Build();
    ASSERT_EQ(m, Success);

    EXPECT_EQ(core::ir::Disassembler(m.Get()).Plain(), R"(%my_func = func():u32 {
  $B1: {
    ret 0u
  }
}
%test_function = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B2: {
    %3:u32 = call %my_func
    %4:u32 = sub %3, 4u
    %tint_symbol:u32 = let %4
    ret
  }
}
)");
}

TEST_F(ProgramToIRBinaryTest, EmitExpression_Binary_Decrement) {
    GlobalVar("v1", core::AddressSpace::kPrivate, ty.i32());
    auto* expr = Decrement("v1");
    WrapInFunction(expr);

    auto m = Build();
    ASSERT_EQ(m, Success);

    EXPECT_EQ(core::ir::Disassembler(m.Get()).Plain(), R"($B1: {  # root
  %v1:ptr<private, i32, read_write> = var
}

%test_function = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B2: {
    %3:i32 = load %v1
    %4:i32 = sub %3, 1i
    store %v1, %4
    ret
  }
}
)");
}

TEST_F(ProgramToIRBinaryTest, EmitExpression_Binary_CompoundSubtract) {
    GlobalVar("v1", core::AddressSpace::kPrivate, ty.u32());
    auto* expr = CompoundAssign("v1", 1_u, core::BinaryOp::kSubtract);
    WrapInFunction(expr);

    auto m = Build();
    ASSERT_EQ(m, Success);

    EXPECT_EQ(core::ir::Disassembler(m.Get()).Plain(), R"($B1: {  # root
  %v1:ptr<private, u32, read_write> = var
}

%test_function = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B2: {
    %3:u32 = load %v1
    %4:u32 = sub %3, 1u
    store %v1, %4
    ret
  }
}
)");
}

TEST_F(ProgramToIRBinaryTest, EmitExpression_Binary_Multiply) {
    Func("my_func", tint::Empty, ty.u32(), Vector{Return(0_u)});
    auto* expr = Mul(Call("my_func"), 4_u);
    WrapInFunction(expr);

    auto m = Build();
    ASSERT_EQ(m, Success);

    EXPECT_EQ(core::ir::Disassembler(m.Get()).Plain(), R"(%my_func = func():u32 {
  $B1: {
    ret 0u
  }
}
%test_function = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B2: {
    %3:u32 = call %my_func
    %4:u32 = mul %3, 4u
    %tint_symbol:u32 = let %4
    ret
  }
}
)");
}

TEST_F(ProgramToIRBinaryTest, EmitExpression_Binary_CompoundMultiply) {
    GlobalVar("v1", core::AddressSpace::kPrivate, ty.u32());
    auto* expr = CompoundAssign("v1", 1_u, core::BinaryOp::kMultiply);
    WrapInFunction(expr);

    auto m = Build();
    ASSERT_EQ(m, Success);

    EXPECT_EQ(core::ir::Disassembler(m.Get()).Plain(), R"($B1: {  # root
  %v1:ptr<private, u32, read_write> = var
}

%test_function = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B2: {
    %3:u32 = load %v1
    %4:u32 = mul %3, 1u
    store %v1, %4
    ret
  }
}
)");
}

TEST_F(ProgramToIRBinaryTest, EmitExpression_Binary_Div) {
    Func("my_func", tint::Empty, ty.u32(), Vector{Return(0_u)});
    auto* expr = Div(Call("my_func"), 4_u);
    WrapInFunction(expr);

    auto m = Build();
    ASSERT_EQ(m, Success);

    EXPECT_EQ(core::ir::Disassembler(m.Get()).Plain(), R"(%my_func = func():u32 {
  $B1: {
    ret 0u
  }
}
%test_function = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B2: {
    %3:u32 = call %my_func
    %4:u32 = div %3, 4u
    %tint_symbol:u32 = let %4
    ret
  }
}
)");
}

TEST_F(ProgramToIRBinaryTest, EmitExpression_Binary_CompoundDiv) {
    GlobalVar("v1", core::AddressSpace::kPrivate, ty.u32());
    auto* expr = CompoundAssign("v1", 1_u, core::BinaryOp::kDivide);
    WrapInFunction(expr);

    auto m = Build();
    ASSERT_EQ(m, Success);

    EXPECT_EQ(core::ir::Disassembler(m.Get()).Plain(), R"($B1: {  # root
  %v1:ptr<private, u32, read_write> = var
}

%test_function = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B2: {
    %3:u32 = load %v1
    %4:u32 = div %3, 1u
    store %v1, %4
    ret
  }
}
)");
}

TEST_F(ProgramToIRBinaryTest, EmitExpression_Binary_Modulo) {
    Func("my_func", tint::Empty, ty.u32(), Vector{Return(0_u)});
    auto* expr = Mod(Call("my_func"), 4_u);
    WrapInFunction(expr);

    auto m = Build();
    ASSERT_EQ(m, Success);

    EXPECT_EQ(core::ir::Disassembler(m.Get()).Plain(), R"(%my_func = func():u32 {
  $B1: {
    ret 0u
  }
}
%test_function = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B2: {
    %3:u32 = call %my_func
    %4:u32 = mod %3, 4u
    %tint_symbol:u32 = let %4
    ret
  }
}
)");
}

TEST_F(ProgramToIRBinaryTest, EmitExpression_Binary_CompoundModulo) {
    GlobalVar("v1", core::AddressSpace::kPrivate, ty.u32());
    auto* expr = CompoundAssign("v1", 1_u, core::BinaryOp::kModulo);
    WrapInFunction(expr);

    auto m = Build();
    ASSERT_EQ(m, Success);

    EXPECT_EQ(core::ir::Disassembler(m.Get()).Plain(), R"($B1: {  # root
  %v1:ptr<private, u32, read_write> = var
}

%test_function = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B2: {
    %3:u32 = load %v1
    %4:u32 = mod %3, 1u
    store %v1, %4
    ret
  }
}
)");
}

TEST_F(ProgramToIRBinaryTest, EmitExpression_Binary_And) {
    Func("my_func", tint::Empty, ty.u32(), Vector{Return(0_u)});
    auto* expr = And(Call("my_func"), 4_u);
    WrapInFunction(expr);

    auto m = Build();
    ASSERT_EQ(m, Success);

    EXPECT_EQ(core::ir::Disassembler(m.Get()).Plain(), R"(%my_func = func():u32 {
  $B1: {
    ret 0u
  }
}
%test_function = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B2: {
    %3:u32 = call %my_func
    %4:u32 = and %3, 4u
    %tint_symbol:u32 = let %4
    ret
  }
}
)");
}

TEST_F(ProgramToIRBinaryTest, EmitExpression_Binary_CompoundAnd) {
    GlobalVar("v1", core::AddressSpace::kPrivate, ty.bool_());
    auto* expr = CompoundAssign("v1", false, core::BinaryOp::kAnd);
    WrapInFunction(expr);

    auto m = Build();
    ASSERT_EQ(m, Success);

    EXPECT_EQ(core::ir::Disassembler(m.Get()).Plain(), R"($B1: {  # root
  %v1:ptr<private, bool, read_write> = var
}

%test_function = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B2: {
    %3:bool = load %v1
    %4:bool = and %3, false
    store %v1, %4
    ret
  }
}
)");
}

TEST_F(ProgramToIRBinaryTest, EmitExpression_Binary_Or) {
    Func("my_func", tint::Empty, ty.u32(), Vector{Return(0_u)});
    auto* expr = Or(Call("my_func"), 4_u);
    WrapInFunction(expr);

    auto m = Build();
    ASSERT_EQ(m, Success);

    EXPECT_EQ(core::ir::Disassembler(m.Get()).Plain(), R"(%my_func = func():u32 {
  $B1: {
    ret 0u
  }
}
%test_function = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B2: {
    %3:u32 = call %my_func
    %4:u32 = or %3, 4u
    %tint_symbol:u32 = let %4
    ret
  }
}
)");
}

TEST_F(ProgramToIRBinaryTest, EmitExpression_Binary_CompoundOr) {
    GlobalVar("v1", core::AddressSpace::kPrivate, ty.bool_());
    auto* expr = CompoundAssign("v1", false, core::BinaryOp::kOr);
    WrapInFunction(expr);

    auto m = Build();
    ASSERT_EQ(m, Success);

    EXPECT_EQ(core::ir::Disassembler(m.Get()).Plain(), R"($B1: {  # root
  %v1:ptr<private, bool, read_write> = var
}

%test_function = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B2: {
    %3:bool = load %v1
    %4:bool = or %3, false
    store %v1, %4
    ret
  }
}
)");
}

TEST_F(ProgramToIRBinaryTest, EmitExpression_Binary_Xor) {
    Func("my_func", tint::Empty, ty.u32(), Vector{Return(0_u)});
    auto* expr = Xor(Call("my_func"), 4_u);
    WrapInFunction(expr);

    auto m = Build();
    ASSERT_EQ(m, Success);

    EXPECT_EQ(core::ir::Disassembler(m.Get()).Plain(), R"(%my_func = func():u32 {
  $B1: {
    ret 0u
  }
}
%test_function = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B2: {
    %3:u32 = call %my_func
    %4:u32 = xor %3, 4u
    %tint_symbol:u32 = let %4
    ret
  }
}
)");
}

TEST_F(ProgramToIRBinaryTest, EmitExpression_Binary_CompoundXor) {
    GlobalVar("v1", core::AddressSpace::kPrivate, ty.u32());
    auto* expr = CompoundAssign("v1", 1_u, core::BinaryOp::kXor);
    WrapInFunction(expr);

    auto m = Build();
    ASSERT_EQ(m, Success);

    EXPECT_EQ(core::ir::Disassembler(m.Get()).Plain(), R"($B1: {  # root
  %v1:ptr<private, u32, read_write> = var
}

%test_function = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B2: {
    %3:u32 = load %v1
    %4:u32 = xor %3, 1u
    store %v1, %4
    ret
  }
}
)");
}

TEST_F(ProgramToIRBinaryTest, EmitExpression_Binary_LogicalAnd) {
    Func("my_func", tint::Empty, ty.bool_(), Vector{Return(true)});
    auto* let = Let("logical_and", LogicalAnd(Call("my_func"), false));
    auto* expr = If(let, Block());
    WrapInFunction(let, expr);

    auto m = Build();
    ASSERT_EQ(m, Success);

    EXPECT_EQ(core::ir::Disassembler(m.Get()).Plain(), R"(%my_func = func():bool {
  $B1: {
    ret true
  }
}
%test_function = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B2: {
    %3:bool = call %my_func
    %4:bool = if %3 [t: $B3, f: $B4] {  # if_1
      $B3: {  # true
        exit_if false  # if_1
      }
      $B4: {  # false
        exit_if false  # if_1
      }
    }
    %logical_and:bool = let %4
    if %logical_and [t: $B5] {  # if_2
      $B5: {  # true
        exit_if  # if_2
      }
    }
    ret
  }
}
)");
}

TEST_F(ProgramToIRBinaryTest, EmitExpression_Binary_LogicalOr) {
    Func("my_func", tint::Empty, ty.bool_(), Vector{Return(true)});
    auto* let = Let("logical_or", LogicalOr(Call("my_func"), true));
    auto* expr = If(let, Block());
    WrapInFunction(let, expr);

    auto m = Build();
    ASSERT_EQ(m, Success);

    EXPECT_EQ(core::ir::Disassembler(m.Get()).Plain(), R"(%my_func = func():bool {
  $B1: {
    ret true
  }
}
%test_function = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B2: {
    %3:bool = call %my_func
    %4:bool = if %3 [t: $B3, f: $B4] {  # if_1
      $B3: {  # true
        exit_if true  # if_1
      }
      $B4: {  # false
        exit_if true  # if_1
      }
    }
    %logical_or:bool = let %4
    if %logical_or [t: $B5] {  # if_2
      $B5: {  # true
        exit_if  # if_2
      }
    }
    ret
  }
}
)");
}

TEST_F(ProgramToIRBinaryTest, EmitExpression_Binary_Equal) {
    Func("my_func", tint::Empty, ty.u32(), Vector{Return(0_u)});
    auto* expr = Equal(Call("my_func"), 4_u);
    WrapInFunction(expr);

    auto m = Build();
    ASSERT_EQ(m, Success);

    EXPECT_EQ(core::ir::Disassembler(m.Get()).Plain(), R"(%my_func = func():u32 {
  $B1: {
    ret 0u
  }
}
%test_function = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B2: {
    %3:u32 = call %my_func
    %4:bool = eq %3, 4u
    %tint_symbol:bool = let %4
    ret
  }
}
)");
}

TEST_F(ProgramToIRBinaryTest, EmitExpression_Binary_NotEqual) {
    Func("my_func", tint::Empty, ty.u32(), Vector{Return(0_u)});
    auto* expr = NotEqual(Call("my_func"), 4_u);
    WrapInFunction(expr);

    auto m = Build();
    ASSERT_EQ(m, Success);

    EXPECT_EQ(core::ir::Disassembler(m.Get()).Plain(), R"(%my_func = func():u32 {
  $B1: {
    ret 0u
  }
}
%test_function = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B2: {
    %3:u32 = call %my_func
    %4:bool = neq %3, 4u
    %tint_symbol:bool = let %4
    ret
  }
}
)");
}

TEST_F(ProgramToIRBinaryTest, EmitExpression_Binary_LessThan) {
    Func("my_func", tint::Empty, ty.u32(), Vector{Return(0_u)});
    auto* expr = LessThan(Call("my_func"), 4_u);
    WrapInFunction(expr);

    auto m = Build();
    ASSERT_EQ(m, Success);

    EXPECT_EQ(core::ir::Disassembler(m.Get()).Plain(), R"(%my_func = func():u32 {
  $B1: {
    ret 0u
  }
}
%test_function = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B2: {
    %3:u32 = call %my_func
    %4:bool = lt %3, 4u
    %tint_symbol:bool = let %4
    ret
  }
}
)");
}

TEST_F(ProgramToIRBinaryTest, EmitExpression_Binary_GreaterThan) {
    Func("my_func", tint::Empty, ty.u32(), Vector{Return(0_u)});
    auto* expr = GreaterThan(Call("my_func"), 4_u);
    WrapInFunction(expr);

    auto m = Build();
    ASSERT_EQ(m, Success);

    EXPECT_EQ(core::ir::Disassembler(m.Get()).Plain(), R"(%my_func = func():u32 {
  $B1: {
    ret 0u
  }
}
%test_function = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B2: {
    %3:u32 = call %my_func
    %4:bool = gt %3, 4u
    %tint_symbol:bool = let %4
    ret
  }
}
)");
}

TEST_F(ProgramToIRBinaryTest, EmitExpression_Binary_LessThanEqual) {
    Func("my_func", tint::Empty, ty.u32(), Vector{Return(0_u)});
    auto* expr = LessThanEqual(Call("my_func"), 4_u);
    WrapInFunction(expr);

    auto m = Build();
    ASSERT_EQ(m, Success);

    EXPECT_EQ(core::ir::Disassembler(m.Get()).Plain(), R"(%my_func = func():u32 {
  $B1: {
    ret 0u
  }
}
%test_function = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B2: {
    %3:u32 = call %my_func
    %4:bool = lte %3, 4u
    %tint_symbol:bool = let %4
    ret
  }
}
)");
}

TEST_F(ProgramToIRBinaryTest, EmitExpression_Binary_GreaterThanEqual) {
    Func("my_func", tint::Empty, ty.u32(), Vector{Return(0_u)});
    auto* expr = GreaterThanEqual(Call("my_func"), 4_u);
    WrapInFunction(expr);

    auto m = Build();
    ASSERT_EQ(m, Success);

    EXPECT_EQ(core::ir::Disassembler(m.Get()).Plain(), R"(%my_func = func():u32 {
  $B1: {
    ret 0u
  }
}
%test_function = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B2: {
    %3:u32 = call %my_func
    %4:bool = gte %3, 4u
    %tint_symbol:bool = let %4
    ret
  }
}
)");
}

TEST_F(ProgramToIRBinaryTest, EmitExpression_Binary_ShiftLeft) {
    Func("my_func", tint::Empty, ty.u32(), Vector{Return(0_u)});
    auto* expr = Shl(Call("my_func"), 4_u);
    WrapInFunction(expr);

    auto m = Build();
    ASSERT_EQ(m, Success);

    EXPECT_EQ(core::ir::Disassembler(m.Get()).Plain(), R"(%my_func = func():u32 {
  $B1: {
    ret 0u
  }
}
%test_function = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B2: {
    %3:u32 = call %my_func
    %4:u32 = shl %3, 4u
    %tint_symbol:u32 = let %4
    ret
  }
}
)");
}

TEST_F(ProgramToIRBinaryTest, EmitExpression_Binary_CompoundShiftLeft) {
    GlobalVar("v1", core::AddressSpace::kPrivate, ty.u32());
    auto* expr = CompoundAssign("v1", 1_u, core::BinaryOp::kShiftLeft);
    WrapInFunction(expr);

    auto m = Build();
    ASSERT_EQ(m, Success);

    EXPECT_EQ(core::ir::Disassembler(m.Get()).Plain(), R"($B1: {  # root
  %v1:ptr<private, u32, read_write> = var
}

%test_function = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B2: {
    %3:u32 = load %v1
    %4:u32 = shl %3, 1u
    store %v1, %4
    ret
  }
}
)");
}

TEST_F(ProgramToIRBinaryTest, EmitExpression_Binary_ShiftRight) {
    Func("my_func", tint::Empty, ty.u32(), Vector{Return(0_u)});
    auto* expr = Shr(Call("my_func"), 4_u);
    WrapInFunction(expr);

    auto m = Build();
    ASSERT_EQ(m, Success);

    EXPECT_EQ(core::ir::Disassembler(m.Get()).Plain(), R"(%my_func = func():u32 {
  $B1: {
    ret 0u
  }
}
%test_function = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B2: {
    %3:u32 = call %my_func
    %4:u32 = shr %3, 4u
    %tint_symbol:u32 = let %4
    ret
  }
}
)");
}

TEST_F(ProgramToIRBinaryTest, EmitExpression_Binary_CompoundShiftRight) {
    GlobalVar("v1", core::AddressSpace::kPrivate, ty.u32());
    auto* expr = CompoundAssign("v1", 1_u, core::BinaryOp::kShiftRight);
    WrapInFunction(expr);

    auto m = Build();
    ASSERT_EQ(m, Success);

    EXPECT_EQ(core::ir::Disassembler(m.Get()).Plain(), R"($B1: {  # root
  %v1:ptr<private, u32, read_write> = var
}

%test_function = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B2: {
    %3:u32 = load %v1
    %4:u32 = shr %3, 1u
    store %v1, %4
    ret
  }
}
)");
}

TEST_F(ProgramToIRBinaryTest, EmitExpression_Binary_Compound) {
    Func("my_func", tint::Empty, ty.f32(), Vector{Return(0_f)});

    auto* expr = LogicalAnd(LessThan(Call("my_func"), 2_f),
                            GreaterThan(2.5_f, Div(Call("my_func"), Mul(2.3_f, Call("my_func")))));
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
    %4:bool = lt %3, 2.0f
    %5:bool = if %4 [t: $B3, f: $B4] {  # if_1
      $B3: {  # true
        %6:f32 = call %my_func
        %7:f32 = call %my_func
        %8:f32 = mul 2.29999995231628417969f, %7
        %9:f32 = div %6, %8
        %10:bool = gt 2.5f, %9
        exit_if %10  # if_1
      }
      $B4: {  # false
        exit_if false  # if_1
      }
    }
    %tint_symbol:bool = let %5
    ret
  }
}
)");
}

TEST_F(ProgramToIRBinaryTest, EmitExpression_Binary_Compound_WithConstEval) {
    Func("my_func", Vector{Param("p", ty.bool_())}, ty.bool_(), Vector{Return(true)});
    auto* expr = Call("my_func", LogicalAnd(LessThan(2.4_f, 2_f),
                                            GreaterThan(2.5_f, Div(10_f, Mul(2.3_f, 9.4_f)))));
    WrapInFunction(expr);

    auto m = Build();
    ASSERT_EQ(m, Success);

    EXPECT_EQ(core::ir::Disassembler(m.Get()).Plain(), R"(%my_func = func(%p:bool):bool {
  $B1: {
    ret true
  }
}
%test_function = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B2: {
    %4:bool = call %my_func, false
    %tint_symbol:bool = let %4
    ret
  }
}
)");
}

}  // namespace
}  // namespace tint::wgsl::reader
