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

using ProgramToIRLetTest = helpers::IRProgramTest;

TEST_F(ProgramToIRLetTest, Constant) {
    WrapInFunction(Let("a", Expr(42_i)));

    auto m = Build();
    ASSERT_EQ(m, Success);

    EXPECT_EQ(core::ir::Disassembler(m.Get()).Plain(),
              R"(%test_function = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    %a:i32 = let 42i
    ret
  }
}
)");
}

TEST_F(ProgramToIRLetTest, BinaryOp) {
    WrapInFunction(Let("a", Add(1_i, 2_i)));

    auto m = Build();
    ASSERT_EQ(m, Success);

    EXPECT_EQ(core::ir::Disassembler(m.Get()).Plain(),
              R"(%test_function = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    %a:i32 = let 3i
    ret
  }
}
)");
}

TEST_F(ProgramToIRLetTest, Chain) {
    WrapInFunction(Let("a", Expr(1_i)),  //
                   Let("b", Expr("a")),  //
                   Let("c", Expr("b")));

    auto m = Build();
    ASSERT_EQ(m, Success);

    EXPECT_EQ(core::ir::Disassembler(m.Get()).Plain(),
              R"(%test_function = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    %a:i32 = let 1i
    %b:i32 = let %a
    %c:i32 = let %b
    ret
  }
}
)");
}

}  // namespace
}  // namespace tint::wgsl::reader
