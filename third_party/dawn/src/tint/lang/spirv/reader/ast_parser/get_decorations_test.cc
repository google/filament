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
#include "src/tint/lang/spirv/reader/ast_parser/helper_test.h"
#include "src/tint/lang/spirv/reader/ast_parser/spirv_tools_helpers_test.h"

namespace tint::spirv::reader::ast_parser {
namespace {

using ::testing::Eq;
using ::testing::UnorderedElementsAre;

using SpvParserGetDecorationsTest = SpirvASTParserTest;

const char* kSkipReason = "This example is deliberately a SPIR-V fragment";

TEST_F(SpvParserGetDecorationsTest, GetDecorationsFor_NotAnId) {
    auto p = parser(test::Assemble(""));
    ASSERT_TRUE(p->BuildAndParseInternalModule());
    auto decorations = p->GetDecorationsFor(42);
    EXPECT_TRUE(decorations.empty());
    EXPECT_TRUE(p->error().empty());
    p->SkipDumpingPending(kSkipReason);
}

TEST_F(SpvParserGetDecorationsTest, GetDecorationsFor_NoDecorations) {
    auto p = parser(test::Assemble("%1 = OpTypeVoid"));
    ASSERT_TRUE(p->BuildAndParseInternalModule());
    auto decorations = p->GetDecorationsFor(1);
    EXPECT_TRUE(decorations.empty());
    EXPECT_TRUE(p->error().empty());
    p->SkipDumpingPending(kSkipReason);
}

TEST_F(SpvParserGetDecorationsTest, GetDecorationsFor_OneDecoration) {
    auto p = parser(test::Assemble(R"(
    OpDecorate %10 Block
    %float = OpTypeFloat 32
    %10 = OpTypeStruct %float
  )"));
    ASSERT_TRUE(p->BuildAndParseInternalModule());
    auto decorations = p->GetDecorationsFor(10);
    EXPECT_THAT(decorations, UnorderedElementsAre(Decoration{uint32_t(spv::Decoration::Block)}));
    EXPECT_TRUE(p->error().empty());
    p->SkipDumpingPending(kSkipReason);
}

TEST_F(SpvParserGetDecorationsTest, GetDecorationsFor_Duplicate) {
    auto p = parser(test::Assemble(R"(
    OpDecorate %10 Block
    OpDecorate %10 Block
    %float = OpTypeFloat 32
    %10 = OpTypeStruct %float
  )"));
    ASSERT_TRUE(p->BuildAndParseInternalModule());
    auto decorations = p->GetDecorationsFor(10);
    EXPECT_THAT(decorations, UnorderedElementsAre(Decoration{uint32_t(spv::Decoration::Block)}));
    EXPECT_TRUE(p->error().empty());
    p->SkipDumpingPending(kSkipReason);
}

TEST_F(SpvParserGetDecorationsTest, GetDecorationsFor_MultiDecoration) {
    auto p = parser(test::Assemble(R"(
    OpDecorate %5 RelaxedPrecision
    OpDecorate %5 Location 7      ; Invalid case made up for test
    %float = OpTypeFloat 32
    %5 = OpConstant %float 3.14
  )"));
    ASSERT_TRUE(p->BuildAndParseInternalModule());
    auto decorations = p->GetDecorationsFor(5);
    EXPECT_THAT(decorations,
                UnorderedElementsAre(Decoration{uint32_t(spv::Decoration::RelaxedPrecision)},
                                     Decoration{uint32_t(spv::Decoration::Location), 7}));
    EXPECT_TRUE(p->error().empty());
    p->SkipDumpingPending(kSkipReason);
}

TEST_F(SpvParserGetDecorationsTest, GetDecorationsForMember_NotAnId) {
    auto p = parser(test::Assemble(""));
    ASSERT_TRUE(p->BuildAndParseInternalModule());
    auto decorations = p->GetDecorationsForMember(42, 9);
    EXPECT_TRUE(decorations.empty());
    EXPECT_TRUE(p->error().empty());
    p->SkipDumpingPending(kSkipReason);
}

TEST_F(SpvParserGetDecorationsTest, GetDecorationsForMember_NotAStruct) {
    auto p = parser(test::Assemble("%1 = OpTypeVoid"));
    ASSERT_TRUE(p->BuildAndParseInternalModule());
    auto decorations = p->GetDecorationsFor(1);
    EXPECT_TRUE(decorations.empty());
    EXPECT_TRUE(p->error().empty());
    p->SkipDumpingPending(kSkipReason);
}

TEST_F(SpvParserGetDecorationsTest, GetDecorationsForMember_MemberWithoutDecoration) {
    auto p = parser(test::Assemble(R"(
    %uint = OpTypeInt 32 0
    %10 = OpTypeStruct %uint
  )"));
    ASSERT_TRUE(p->BuildAndParseInternalModule());
    auto decorations = p->GetDecorationsForMember(10, 0);
    EXPECT_TRUE(decorations.empty());
    EXPECT_TRUE(p->error().empty());
    p->SkipDumpingPending(kSkipReason);
}

TEST_F(SpvParserGetDecorationsTest, GetDecorationsForMember_RelaxedPrecision) {
    auto p = parser(test::Assemble(R"(
    OpMemberDecorate %10 0 RelaxedPrecision
    %float = OpTypeFloat 32
    %10 = OpTypeStruct %float
  )"));
    ASSERT_TRUE(p->BuildAndParseInternalModule()) << p->error();
    auto decorations = p->GetDecorationsForMember(10, 0);
    EXPECT_THAT(decorations,
                UnorderedElementsAre(Decoration{uint32_t(spv::Decoration::RelaxedPrecision)}));
    EXPECT_TRUE(p->error().empty());
    p->SkipDumpingPending(kSkipReason);
}

TEST_F(SpvParserGetDecorationsTest, GetDecorationsForMember_Duplicate) {
    auto p = parser(test::Assemble(R"(
    OpMemberDecorate %10 0 RelaxedPrecision
    OpMemberDecorate %10 0 RelaxedPrecision
    %float = OpTypeFloat 32
    %10 = OpTypeStruct %float
  )"));
    ASSERT_TRUE(p->BuildAndParseInternalModule()) << p->error();
    auto decorations = p->GetDecorationsForMember(10, 0);
    EXPECT_THAT(decorations,
                UnorderedElementsAre(Decoration{uint32_t(spv::Decoration::RelaxedPrecision)}));
    EXPECT_TRUE(p->error().empty());
    p->SkipDumpingPending(kSkipReason);
}

// TODO(dneto): Enable when ArrayStride is handled
TEST_F(SpvParserGetDecorationsTest, DISABLED_GetDecorationsForMember_OneDecoration) {
    auto p = parser(test::Assemble(R"(
    OpMemberDecorate %10 1 ArrayStride 12
    %uint = OpTypeInt 32 0
    %uint_2 = OpConstant %uint 2
    %arr = OpTypeArray %uint %uint_2
    %10 = OpTypeStruct %uint %arr
  )"));
    ASSERT_TRUE(p->BuildAndParseInternalModule()) << p->error();
    auto decorations = p->GetDecorationsForMember(10, 1);
    EXPECT_THAT(decorations,
                UnorderedElementsAre(Decoration{uint32_t(spv::Decoration::ArrayStride), 12}));
    EXPECT_TRUE(p->error().empty());
}

// TODO(dneto): Enable when ArrayStride, MatrixStride, ColMajor are handled
// crbug.com/tint/30 for ArrayStride
// crbug.com/tint/31 for matrix layout
TEST_F(SpvParserGetDecorationsTest, DISABLED_GetDecorationsForMember_MultiDecoration) {
    auto p = parser(test::Assemble(R"(
    OpMemberDecorate %50 1 RelaxedPrecision
    OpMemberDecorate %50 2 ArrayStride 16
    OpMemberDecorate %50 2 MatrixStride 8
    OpMemberDecorate %50 2 ColMajor
    %float = OpTypeFloat 32
    %vec = OpTypeVector %float 2
    %mat = OpTypeMatrix %vec 2
    %uint = OpTypeInt 32 0
    %uint_2 = OpConstant %uint 2
    %arr = OpTypeArray %mat %uint_2
    %50 = OpTypeStruct %uint %float %arr
  )"));
    ASSERT_TRUE(p->BuildAndParseInternalModule()) << p->error();

    EXPECT_TRUE(p->GetDecorationsForMember(50, 0).empty());
    EXPECT_THAT(p->GetDecorationsForMember(50, 1),
                UnorderedElementsAre(Decoration{uint32_t(spv::Decoration::RelaxedPrecision)}));
    EXPECT_THAT(p->GetDecorationsForMember(50, 2),
                UnorderedElementsAre(Decoration{uint32_t(spv::Decoration::ColMajor)},
                                     Decoration{uint32_t(spv::Decoration::MatrixStride), 8},
                                     Decoration{uint32_t(spv::Decoration::ArrayStride), 16}));
    EXPECT_TRUE(p->error().empty());
}

TEST_F(SpvParserGetDecorationsTest, GetDecorationsFor_Restrict) {
    // RestrictPointer applies to a memory object declaration. Use a variable.
    auto p = parser(test::Assemble(R"(
    OpDecorate %10 Restrict
    %float = OpTypeFloat 32
    %ptr = OpTypePointer Workgroup %float
    %10 = OpVariable %ptr Workgroup
  )"));
    ASSERT_TRUE(p->BuildAndParseInternalModule());
    auto decorations = p->GetDecorationsFor(10);
    EXPECT_TRUE(decorations.empty());
    EXPECT_TRUE(p->error().empty());
    p->SkipDumpingPending(kSkipReason);
}

TEST_F(SpvParserGetDecorationsTest, GetDecorationsForMember_Restrict) {
    // Restrict applies to a memory object declaration.
    // But OpMemberDecorate can only be applied to a structure type.
    // Test the reader's ability to be resilient to more than what SPIR-V allows.
    auto p = parser(test::Assemble(R"(
    OpMemberDecorate %10 0 Restrict
    %float = OpTypeFloat 32
    %10 = OpTypeStruct %float
  )"));
    ASSERT_TRUE(p->BuildAndParseInternalModule());
    auto decorations = p->GetDecorationsForMember(10, 0);
    EXPECT_TRUE(decorations.empty());
    EXPECT_TRUE(p->error().empty());
    p->SkipDumpingPending(kSkipReason);
}

TEST_F(SpvParserGetDecorationsTest, GetDecorationsFor_RestrictPointer) {
    // RestrictPointer applies to a memory object declaration. Use a variable.
    auto p = parser(test::Assemble(R"(
    OpDecorate %10 RestrictPointer
    %float = OpTypeFloat 32
    %ptr = OpTypePointer Workgroup %float
    %10 = OpVariable %ptr Workgroup
  )"));
    ASSERT_TRUE(p->BuildAndParseInternalModule()) << p->error();
    auto decorations = p->GetDecorationsFor(10);
    EXPECT_TRUE(decorations.empty());
    EXPECT_TRUE(p->error().empty());
    p->SkipDumpingPending(kSkipReason);
}

TEST_F(SpvParserGetDecorationsTest, GetDecorationsForMember_RestrictPointer) {
    // RestrictPointer applies to a memory object declaration.
    // But OpMemberDecorate can only be applied to a structure type.
    // Test the reader's ability to be resilient to more than what SPIR-V allows.
    auto p = parser(test::Assemble(R"(
    OpMemberDecorate %10 0 RestrictPointer
    %float = OpTypeFloat 32
    %10 = OpTypeStruct %float
  )"));
    ASSERT_TRUE(p->BuildAndParseInternalModule()) << p->error();
    auto decorations = p->GetDecorationsFor(10);
    EXPECT_TRUE(decorations.empty());
    EXPECT_TRUE(p->error().empty());
    p->SkipDumpingPending(kSkipReason);
}

}  // namespace
}  // namespace tint::spirv::reader::ast_parser
