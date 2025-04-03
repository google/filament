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

#include "src/tint/lang/spirv/writer/common/binary_writer.h"
#include "gtest/gtest.h"

namespace tint::spirv::writer {
namespace {

using SpirvWriterBinaryWriterTest = testing::Test;

TEST_F(SpirvWriterBinaryWriterTest, Preamble) {
    BinaryWriter bw;
    bw.WriteHeader(5);

    auto res = bw.Result();
    ASSERT_EQ(res.size(), 5u);
    EXPECT_EQ(res[0], spv::MagicNumber);
    EXPECT_EQ(res[1], 0x00010300u);  // SPIR-V 1.3
    EXPECT_EQ(res[2], 23u << 16);    // Generator ID
    EXPECT_EQ(res[3], 5u);           // ID Bound
    EXPECT_EQ(res[4], 0u);           // Reserved
}

TEST_F(SpirvWriterBinaryWriterTest, Float) {
    Module m;

    m.PushAnnot(spv::Op::OpKill, {Operand(2.4f)});
    BinaryWriter bw;
    bw.WriteModule(m);

    auto res = bw.Result();
    ASSERT_EQ(res.size(), 2u);
    float f;
    memcpy(&f, &res[1], 4);
    EXPECT_EQ(f, 2.4f);
}

TEST_F(SpirvWriterBinaryWriterTest, Int) {
    Module m;

    m.PushAnnot(spv::Op::OpKill, {Operand(2u)});
    BinaryWriter bw;
    bw.WriteModule(m);

    auto res = bw.Result();
    ASSERT_EQ(res.size(), 2u);
    EXPECT_EQ(res[1], 2u);
}

TEST_F(SpirvWriterBinaryWriterTest, String) {
    Module m;

    m.PushAnnot(spv::Op::OpKill, {Operand("my_string")});
    BinaryWriter bw;
    bw.WriteModule(m);

    auto res = bw.Result();
    ASSERT_EQ(res.size(), 4u);

    auto v = std::vector<uint8_t>();
    for (size_t idx = 1; idx < res.size(); idx++) {
        uint32_t* res_u32 = &res[idx];
        v.push_back(*res_u32 & 0xff);
        v.push_back(*res_u32 >> 8 & 0xff);
        v.push_back(*res_u32 >> 16 & 0xff);
        v.push_back(*res_u32 >> 24 & 0xff);
    }

    EXPECT_EQ(v[0], 'm');
    EXPECT_EQ(v[1], 'y');
    EXPECT_EQ(v[2], '_');
    EXPECT_EQ(v[3], 's');
    EXPECT_EQ(v[4], 't');
    EXPECT_EQ(v[5], 'r');
    EXPECT_EQ(v[6], 'i');
    EXPECT_EQ(v[7], 'n');
    EXPECT_EQ(v[8], 'g');
    EXPECT_EQ(v[9], '\0');
    EXPECT_EQ(v[10], '\0');
    EXPECT_EQ(v[11], '\0');
}

TEST_F(SpirvWriterBinaryWriterTest, String_Multiple4Length) {
    Module m;

    m.PushAnnot(spv::Op::OpKill, {Operand("mystring")});
    BinaryWriter bw;
    bw.WriteModule(m);

    auto res = bw.Result();
    ASSERT_EQ(res.size(), 4u);

    auto v = std::vector<uint8_t>();
    for (size_t idx = 1; idx < res.size(); idx++) {
        uint32_t* res_u32 = &res[idx];
        v.push_back(*res_u32 & 0xff);
        v.push_back(*res_u32 >> 8 & 0xff);
        v.push_back(*res_u32 >> 16 & 0xff);
        v.push_back(*res_u32 >> 24 & 0xff);
    }

    EXPECT_EQ(v[0], 'm');
    EXPECT_EQ(v[1], 'y');
    EXPECT_EQ(v[2], 's');
    EXPECT_EQ(v[3], 't');
    EXPECT_EQ(v[4], 'r');
    EXPECT_EQ(v[5], 'i');
    EXPECT_EQ(v[6], 'n');
    EXPECT_EQ(v[7], 'g');
    EXPECT_EQ(v[8], '\0');
    EXPECT_EQ(v[9], '\0');
    EXPECT_EQ(v[10], '\0');
    EXPECT_EQ(v[11], '\0');
}

TEST_F(SpirvWriterBinaryWriterTest, TestInstructionWriter) {
    Instruction i1{spv::Op::OpKill, {Operand(2u)}};
    Instruction i2{spv::Op::OpKill, {Operand(4u)}};

    BinaryWriter bw;
    bw.WriteInstruction(i1);
    bw.WriteInstruction(i2);

    auto res = bw.Result();
    ASSERT_EQ(res.size(), 4u);
    EXPECT_EQ(res[1], 2u);
    EXPECT_EQ(res[3], 4u);
}

}  // namespace
}  // namespace tint::spirv::writer
