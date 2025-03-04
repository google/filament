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

#include "src/tint/lang/spirv/writer/common/instruction.h"

#include <string>

#include "gtest/gtest.h"

namespace tint::spirv::writer {
namespace {

using SpirvWriterInstructionTest = testing::Test;

TEST_F(SpirvWriterInstructionTest, Create) {
    Instruction i(spv::Op::OpEntryPoint, {Operand(1.2f), Operand(1u), Operand("my_str")});
    EXPECT_EQ(i.Opcode(), spv::Op::OpEntryPoint);
    ASSERT_EQ(i.Operands().size(), 3u);

    const auto& ops = i.Operands();
    ASSERT_TRUE(std::holds_alternative<float>(ops[0]));
    EXPECT_FLOAT_EQ(std::get<float>(ops[0]), 1.2f);

    ASSERT_TRUE(std::holds_alternative<uint32_t>(ops[1]));
    EXPECT_EQ(std::get<uint32_t>(ops[1]), 1u);

    ASSERT_TRUE(std::holds_alternative<std::string>(ops[2]));
    EXPECT_EQ(std::get<std::string>(ops[2]), "my_str");
}

TEST_F(SpirvWriterInstructionTest, Length) {
    Instruction i(spv::Op::OpEntryPoint, {Operand(1.2f), Operand(1u), Operand("my_str")});
    EXPECT_EQ(i.WordLength(), 5u);
}

}  // namespace
}  // namespace tint::spirv::writer
