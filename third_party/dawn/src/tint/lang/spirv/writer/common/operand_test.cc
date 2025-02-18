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

#include "src/tint/lang/spirv/writer/common/operand.h"

#include "gtest/gtest.h"

namespace tint::spirv::writer {
namespace {

using SpirvWriterOperandtest = testing::Test;

TEST_F(SpirvWriterOperandtest, CreateFloat) {
    auto o = Operand(1.2f);
    ASSERT_TRUE(std::holds_alternative<float>(o));
    EXPECT_FLOAT_EQ(std::get<float>(o), 1.2f);
}

TEST_F(SpirvWriterOperandtest, CreateInt) {
    auto o = Operand(1u);
    ASSERT_TRUE(std::holds_alternative<uint32_t>(o));
    EXPECT_EQ(std::get<uint32_t>(o), 1u);
}

TEST_F(SpirvWriterOperandtest, CreateString) {
    auto o = Operand("my string");
    ASSERT_TRUE(std::holds_alternative<std::string>(o));
    EXPECT_EQ(std::get<std::string>(o), "my string");
}

TEST_F(SpirvWriterOperandtest, Length_Float) {
    auto o = Operand(1.2f);
    EXPECT_EQ(OperandLength(o), 1u);
}

TEST_F(SpirvWriterOperandtest, Length_Int) {
    auto o = U32Operand(1);
    EXPECT_EQ(OperandLength(o), 1u);
}

TEST_F(SpirvWriterOperandtest, Length_String) {
    auto o = Operand("my string");
    EXPECT_EQ(OperandLength(o), 3u);
}

TEST_F(SpirvWriterOperandtest, Length_String_Empty) {
    auto o = Operand("");
    EXPECT_EQ(OperandLength(o), 1u);
}

}  // namespace
}  // namespace tint::spirv::writer
