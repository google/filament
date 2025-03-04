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
#include "src/tint/lang/spirv/reader/ast_parser/function.h"
#include "src/tint/lang/spirv/reader/ast_parser/helper_test.h"
#include "src/tint/lang/spirv/reader/ast_parser/spirv_tools_helpers_test.h"
#include "src/tint/utils/text/string_stream.h"

namespace tint::spirv::reader::ast_parser {
namespace {

using ::testing::Eq;
using ::testing::HasSubstr;

// Make a local name so it's easier to triage errors.
using SpvParserCFGTest = SpirvASTParserTest;

std::string Dump(const std::vector<uint32_t>& v) {
    tint::StringStream o;
    o << "{";
    for (auto a : v) {
        o << a << " ";
    }
    o << "}";
    return o.str();
}

using ::testing::ElementsAre;
using ::testing::Eq;
using ::testing::UnorderedElementsAre;

std::string CommonTypes() {
    return R"(
    OpCapability Shader
    OpMemoryModel Logical Simple
    OpEntryPoint Fragment %100 "main"
    OpExecutionMode %100 OriginUpperLeft

    OpName %var "var"

    %void = OpTypeVoid
    %voidfn = OpTypeFunction %void

    %bool = OpTypeBool
    %cond = OpConstantNull %bool
    %cond2 = OpConstantTrue %bool
    %cond3 = OpConstantFalse %bool

    %uint = OpTypeInt 32 0
    %int = OpTypeInt 32 1
    %selector = OpConstant %uint 42
    %signed_selector = OpConstant %int 42

    %uintfn = OpTypeFunction %uint

    %uint_0 = OpConstant %uint 0
    %uint_1 = OpConstant %uint 1
    %uint_2 = OpConstant %uint 2
    %uint_3 = OpConstant %uint 3
    %uint_4 = OpConstant %uint 4
    %uint_5 = OpConstant %uint 5
    %uint_6 = OpConstant %uint 6
    %uint_7 = OpConstant %uint 7
    %uint_8 = OpConstant %uint 8
    %uint_10 = OpConstant %uint 10
    %uint_20 = OpConstant %uint 20
    %uint_30 = OpConstant %uint 30
    %uint_40 = OpConstant %uint 40
    %uint_50 = OpConstant %uint 50
    %uint_90 = OpConstant %uint 90
    %uint_99 = OpConstant %uint 99

    %ptr_Private_uint = OpTypePointer Private %uint
    %var = OpVariable %ptr_Private_uint Private

    %999 = OpConstant %uint 999
  )";
}

/// Runs the necessary flow until and including labeling control
/// flow constructs.
/// @returns the result of labeling control flow constructs.
bool FlowLabelControlFlowConstructs(FunctionEmitter* fe) {
    fe->RegisterBasicBlocks();
    EXPECT_TRUE(fe->RegisterMerges()) << fe->parser()->error();
    fe->ComputeBlockOrderAndPositions();
    EXPECT_TRUE(fe->VerifyHeaderContinueMergeOrder()) << fe->parser()->error();
    return fe->LabelControlFlowConstructs();
}

/// Runs the necessary flow until and including finding switch case
/// headers.
/// @returns the result of finding switch case headers.
bool FlowFindSwitchCaseHeaders(FunctionEmitter* fe) {
    EXPECT_TRUE(FlowLabelControlFlowConstructs(fe)) << fe->parser()->error();
    return fe->FindSwitchCaseHeaders();
}

/// Runs the necessary flow until and including classify CFG edges,
/// @returns the result of classify CFG edges.
bool FlowClassifyCFGEdges(FunctionEmitter* fe) {
    EXPECT_TRUE(FlowFindSwitchCaseHeaders(fe)) << fe->parser()->error();
    return fe->ClassifyCFGEdges();
}

/// Runs the necessary flow until and including finding if-selection
/// internal headers.
/// @returns the result of classify CFG edges.
bool FlowFindIfSelectionInternalHeaders(FunctionEmitter* fe) {
    EXPECT_TRUE(FlowClassifyCFGEdges(fe)) << fe->parser()->error();
    return fe->FindIfSelectionInternalHeaders();
}

TEST_F(SpvParserCFGTest, TerminatorsAreValid_SingleBlock) {
    auto p = parser(test::Assemble(CommonTypes() + R"(
     %100 = OpFunction %void None %voidfn

     %42 = OpLabel
     OpReturn

     OpFunctionEnd
  )"));
    ASSERT_TRUE(p->BuildAndParseInternalModuleExceptFunctions()) << p->error();
    auto fe = p->function_emitter(100);
    fe.RegisterBasicBlocks();
    EXPECT_TRUE(fe.TerminatorsAreValid());
}

TEST_F(SpvParserCFGTest, TerminatorsAreValid_Sequence) {
    auto p = parser(test::Assemble(CommonTypes() + R"(
     %100 = OpFunction %void None %voidfn

     %20 = OpLabel
     OpBranch %30

     %30 = OpLabel
     OpReturn

     OpFunctionEnd
  )"));
    ASSERT_TRUE(p->BuildAndParseInternalModuleExceptFunctions()) << p->error();
    auto fe = p->function_emitter(100);
    fe.RegisterBasicBlocks();
    EXPECT_TRUE(fe.TerminatorsAreValid()) << p->error();
}

TEST_F(SpvParserCFGTest, TerminatorsAreValid_If) {
    auto p = parser(test::Assemble(CommonTypes() + R"(
     %100 = OpFunction %void None %voidfn

     %20 = OpLabel
     OpSelectionMerge %99 None
     OpBranchConditional %cond %30 %40

     %30 = OpLabel
     OpBranch %99

     %40 = OpLabel
     OpBranch %99

     %99 = OpLabel
     OpReturn

     OpFunctionEnd
  )"));
    ASSERT_TRUE(p->BuildAndParseInternalModuleExceptFunctions()) << p->error();
    auto fe = p->function_emitter(100);
    fe.RegisterBasicBlocks();
    EXPECT_TRUE(fe.TerminatorsAreValid()) << p->error();
}

TEST_F(SpvParserCFGTest, TerminatorsAreValid_Switch) {
    auto p = parser(test::Assemble(CommonTypes() + R"(
     %100 = OpFunction %void None %voidfn

     %10 = OpLabel
     OpSelectionMerge %99 None
     OpSwitch %selector %80 20 %20 30 %30

     %20 = OpLabel
     OpBranch %30 ; fall through

     %30 = OpLabel
     OpBranch %99

     %80 = OpLabel
     OpBranch %99

     %99 = OpLabel
     OpReturn

     OpFunctionEnd
  )"));
    ASSERT_TRUE(p->BuildAndParseInternalModuleExceptFunctions()) << p->error();
    auto fe = p->function_emitter(100);
    fe.RegisterBasicBlocks();
    EXPECT_TRUE(fe.TerminatorsAreValid());
}

TEST_F(SpvParserCFGTest, TerminatorsAreValid_Loop_SingleBlock) {
    auto p = parser(test::Assemble(CommonTypes() + R"(
     %100 = OpFunction %void None %voidfn

     %10 = OpLabel
     OpBranch %20

     %20 = OpLabel
     OpLoopMerge %99 %20 None
     OpBranchConditional %cond %20 %99

     %99 = OpLabel
     OpReturn

     OpFunctionEnd
  )"));
    ASSERT_TRUE(p->BuildAndParseInternalModuleExceptFunctions()) << p->error();
    auto fe = p->function_emitter(100);
    fe.RegisterBasicBlocks();
    EXPECT_TRUE(fe.TerminatorsAreValid());
}

TEST_F(SpvParserCFGTest, TerminatorsAreValid_Loop_Simple) {
    auto p = parser(test::Assemble(CommonTypes() + R"(
     %100 = OpFunction %void None %voidfn

     %10 = OpLabel
     OpBranch %20

     %20 = OpLabel
     OpLoopMerge %99 %40 None
     OpBranchConditional %cond %30 %99

     %30 = OpLabel
     OpBranch %40

     %40 = OpLabel
     OpBranch %20 ; back edge

     %99 = OpLabel
     OpReturn

     OpFunctionEnd
  )"));
    ASSERT_TRUE(p->BuildAndParseInternalModuleExceptFunctions()) << p->error();
    auto fe = p->function_emitter(100);
    fe.RegisterBasicBlocks();
    EXPECT_TRUE(fe.TerminatorsAreValid());
}

TEST_F(SpvParserCFGTest, TerminatorsAreValid_Kill) {
    auto p = parser(test::Assemble(CommonTypes() + R"(
     %100 = OpFunction %void None %voidfn

     %10 = OpLabel
     OpKill

     OpFunctionEnd
  )"));
    ASSERT_TRUE(p->BuildAndParseInternalModuleExceptFunctions()) << p->error();
    auto fe = p->function_emitter(100);
    fe.RegisterBasicBlocks();
    EXPECT_TRUE(fe.TerminatorsAreValid());
}

TEST_F(SpvParserCFGTest, TerminatorsAreValid_Unreachable) {
    auto p = parser(test::Assemble(CommonTypes() + R"(
     %100 = OpFunction %void None %voidfn

     %10 = OpLabel
     OpUnreachable

     OpFunctionEnd
  )"));
    ASSERT_TRUE(p->BuildAndParseInternalModuleExceptFunctions()) << p->error();
    auto fe = p->function_emitter(100);
    fe.RegisterBasicBlocks();
    EXPECT_TRUE(fe.TerminatorsAreValid());
}

TEST_F(SpvParserCFGTest, TerminatorsAreValid_MissingTerminator) {
    auto p = parser(test::Assemble(CommonTypes() + R"(
     %100 = OpFunction %void None %voidfn

     %10 = OpLabel

     OpFunctionEnd
  )"));
    // The SPIRV-Tools internal representation rejects this case earlier.
    EXPECT_FALSE(p->BuildAndParseInternalModuleExceptFunctions());
}

TEST_F(SpvParserCFGTest, TerminatorsAreValid_DisallowLoopToEntryBlock) {
    auto p = parser(test::Assemble(CommonTypes() + R"(
     %100 = OpFunction %void None %voidfn

     %10 = OpLabel
     OpBranch %20

     %20 = OpLabel
     OpBranch %10 ; not allowed

     OpFunctionEnd
  )"));
    ASSERT_TRUE(p->BuildAndParseInternalModuleExceptFunctions()) << p->error();
    auto fe = p->function_emitter(100);
    fe.RegisterBasicBlocks();
    EXPECT_FALSE(fe.TerminatorsAreValid());
    EXPECT_THAT(p->error(), Eq("Block 20 branches to function entry block 10"));
}

TEST_F(SpvParserCFGTest, TerminatorsAreValid_DisallowNonBlock) {
    auto p = parser(test::Assemble(CommonTypes() + R"(
     %100 = OpFunction %void None %voidfn

     %10 = OpLabel
     OpBranch %999 ; definitely wrong

     OpFunctionEnd
  )"));
    ASSERT_TRUE(p->BuildAndParseInternalModuleExceptFunctions()) << p->error();
    auto fe = p->function_emitter(100);
    fe.RegisterBasicBlocks();
    EXPECT_FALSE(fe.TerminatorsAreValid());
    EXPECT_THAT(p->error(), Eq("Block 10 in function 100 branches to 999 which is "
                               "not a block in the function"));
}

TEST_F(SpvParserCFGTest, TerminatorsAreValid_DisallowBlockInDifferentFunction) {
    auto p = parser(test::Assemble(CommonTypes() + R"(
     %100 = OpFunction %void None %voidfn

     %10 = OpLabel
     OpBranch %210

     OpFunctionEnd


     %200 = OpFunction %void None %voidfn

     %210 = OpLabel
     OpReturn

     OpFunctionEnd
  )"));
    ASSERT_TRUE(p->BuildAndParseInternalModuleExceptFunctions()) << p->error();
    auto fe = p->function_emitter(100);
    fe.RegisterBasicBlocks();
    EXPECT_FALSE(fe.TerminatorsAreValid());
    EXPECT_THAT(p->error(), Eq("Block 10 in function 100 branches to 210 which "
                               "is not a block in the function"));
}

TEST_F(SpvParserCFGTest, RegisterMerges_NoMerges) {
    auto p = parser(test::Assemble(CommonTypes() + R"(
     %100 = OpFunction %void None %voidfn

     %10 = OpLabel
     OpReturn

     OpFunctionEnd
  )"));
    ASSERT_TRUE(p->BuildAndParseInternalModuleExceptFunctions()) << p->error();
    auto fe = p->function_emitter(100);
    fe.RegisterBasicBlocks();
    EXPECT_TRUE(fe.RegisterMerges());

    const auto* bi = fe.GetBlockInfo(10);
    ASSERT_NE(bi, nullptr);
    EXPECT_EQ(bi->merge_for_header, 0u);
    EXPECT_EQ(bi->continue_for_header, 0u);
    EXPECT_EQ(bi->header_for_merge, 0u);
    EXPECT_EQ(bi->header_for_continue, 0u);
    EXPECT_FALSE(bi->is_continue_entire_loop);
}

TEST_F(SpvParserCFGTest, RegisterMerges_GoodSelectionMerge_BranchConditional) {
    auto p = parser(test::Assemble(CommonTypes() + R"(
     %100 = OpFunction %void None %voidfn

     %10 = OpLabel
     OpSelectionMerge %99 None
     OpBranchConditional %cond %20 %99

     %20 = OpLabel
     OpBranch %99

     %99 = OpLabel
     OpReturn

     OpFunctionEnd
  )"));
    ASSERT_TRUE(p->BuildAndParseInternalModuleExceptFunctions()) << p->error();
    auto fe = p->function_emitter(100);
    fe.RegisterBasicBlocks();
    EXPECT_TRUE(fe.RegisterMerges());

    // Header points to the merge
    const auto* bi10 = fe.GetBlockInfo(10);
    ASSERT_NE(bi10, nullptr);
    EXPECT_EQ(bi10->merge_for_header, 99u);
    EXPECT_EQ(bi10->continue_for_header, 0u);
    EXPECT_EQ(bi10->header_for_merge, 0u);
    EXPECT_EQ(bi10->header_for_continue, 0u);
    EXPECT_FALSE(bi10->is_continue_entire_loop);

    // Middle block is neither header nor merge
    const auto* bi20 = fe.GetBlockInfo(20);
    ASSERT_NE(bi20, nullptr);
    EXPECT_EQ(bi20->merge_for_header, 0u);
    EXPECT_EQ(bi20->continue_for_header, 0u);
    EXPECT_EQ(bi20->header_for_merge, 0u);
    EXPECT_EQ(bi20->header_for_continue, 0u);
    EXPECT_FALSE(bi20->is_continue_entire_loop);

    // Merge block points to the header
    const auto* bi99 = fe.GetBlockInfo(99);
    ASSERT_NE(bi99, nullptr);
    EXPECT_EQ(bi99->merge_for_header, 0u);
    EXPECT_EQ(bi99->continue_for_header, 0u);
    EXPECT_EQ(bi99->header_for_merge, 10u);
    EXPECT_EQ(bi99->header_for_continue, 0u);
    EXPECT_FALSE(bi99->is_continue_entire_loop);
}

TEST_F(SpvParserCFGTest, RegisterMerges_GoodSelectionMerge_Switch) {
    auto p = parser(test::Assemble(CommonTypes() + R"(
     %100 = OpFunction %void None %voidfn

     %10 = OpLabel
     OpSelectionMerge %99 None
     OpSwitch %selector %99 20 %20

     %20 = OpLabel
     OpBranch %99

     %99 = OpLabel
     OpReturn

     OpFunctionEnd
  )"));
    ASSERT_TRUE(p->BuildAndParseInternalModuleExceptFunctions()) << p->error();
    auto fe = p->function_emitter(100);
    fe.RegisterBasicBlocks();
    EXPECT_TRUE(fe.RegisterMerges());

    // Header points to the merge
    const auto* bi10 = fe.GetBlockInfo(10);
    ASSERT_NE(bi10, nullptr);
    EXPECT_EQ(bi10->merge_for_header, 99u);
    EXPECT_EQ(bi10->continue_for_header, 0u);
    EXPECT_EQ(bi10->header_for_merge, 0u);
    EXPECT_EQ(bi10->header_for_continue, 0u);
    EXPECT_FALSE(bi10->is_continue_entire_loop);

    // Middle block is neither header nor merge
    const auto* bi20 = fe.GetBlockInfo(20);
    ASSERT_NE(bi20, nullptr);
    EXPECT_EQ(bi20->merge_for_header, 0u);
    EXPECT_EQ(bi20->continue_for_header, 0u);
    EXPECT_EQ(bi20->header_for_merge, 0u);
    EXPECT_EQ(bi20->header_for_continue, 0u);
    EXPECT_FALSE(bi20->is_continue_entire_loop);

    // Merge block points to the header
    const auto* bi99 = fe.GetBlockInfo(99);
    ASSERT_NE(bi99, nullptr);
    EXPECT_EQ(bi99->merge_for_header, 0u);
    EXPECT_EQ(bi99->continue_for_header, 0u);
    EXPECT_EQ(bi99->header_for_merge, 10u);
    EXPECT_EQ(bi99->header_for_continue, 0u);
    EXPECT_FALSE(bi99->is_continue_entire_loop);
}

TEST_F(SpvParserCFGTest, RegisterMerges_GoodLoopMerge_SingleBlockLoop) {
    auto p = parser(test::Assemble(CommonTypes() + R"(
     %100 = OpFunction %void None %voidfn

     %10 = OpLabel
     OpBranch %20

     %20 = OpLabel
     OpLoopMerge %99 %20 None
     OpBranchConditional %cond %20 %99

     %99 = OpLabel
     OpReturn

     OpFunctionEnd
  )"));
    ASSERT_TRUE(p->BuildAndParseInternalModuleExceptFunctions()) << p->error();
    auto fe = p->function_emitter(100);
    fe.RegisterBasicBlocks();
    EXPECT_TRUE(fe.RegisterMerges());

    // Entry block is not special
    const auto* bi10 = fe.GetBlockInfo(10);
    ASSERT_NE(bi10, nullptr);
    EXPECT_EQ(bi10->merge_for_header, 0u);
    EXPECT_EQ(bi10->continue_for_header, 0u);
    EXPECT_EQ(bi10->header_for_merge, 0u);
    EXPECT_EQ(bi10->header_for_continue, 0u);
    EXPECT_FALSE(bi10->is_continue_entire_loop);

    // Single block loop is its own continue, and marked as single block loop.
    const auto* bi20 = fe.GetBlockInfo(20);
    ASSERT_NE(bi20, nullptr);
    EXPECT_EQ(bi20->merge_for_header, 99u);
    EXPECT_EQ(bi20->continue_for_header, 20u);
    EXPECT_EQ(bi20->header_for_merge, 0u);
    EXPECT_EQ(bi20->header_for_continue, 20u);
    EXPECT_TRUE(bi20->is_continue_entire_loop);

    // Merge block points to the header
    const auto* bi99 = fe.GetBlockInfo(99);
    ASSERT_NE(bi99, nullptr);
    EXPECT_EQ(bi99->merge_for_header, 0u);
    EXPECT_EQ(bi99->continue_for_header, 0u);
    EXPECT_EQ(bi99->header_for_merge, 20u);
    EXPECT_EQ(bi99->header_for_continue, 0u);
    EXPECT_FALSE(bi99->is_continue_entire_loop);
}

TEST_F(SpvParserCFGTest, RegisterMerges_GoodLoopMerge_MultiBlockLoop_ContinueIsHeader) {
    auto p = parser(test::Assemble(CommonTypes() + R"(
     %100 = OpFunction %void None %voidfn

     %10 = OpLabel
     OpBranch %20

     %20 = OpLabel
     OpLoopMerge %99 %20 None
     OpBranch %40

     %40 = OpLabel
     OpBranch %20

     %99 = OpLabel
     OpReturn

     OpFunctionEnd
  )"));
    ASSERT_TRUE(p->BuildAndParseInternalModuleExceptFunctions()) << p->error();
    auto fe = p->function_emitter(100);
    fe.RegisterBasicBlocks();
    EXPECT_TRUE(fe.RegisterMerges());

    // Loop header points to continue (itself) and merge
    const auto* bi20 = fe.GetBlockInfo(20);
    ASSERT_NE(bi20, nullptr);
    EXPECT_EQ(bi20->merge_for_header, 99u);
    EXPECT_EQ(bi20->continue_for_header, 20u);
    EXPECT_EQ(bi20->header_for_merge, 0u);
    EXPECT_EQ(bi20->header_for_continue, 20u);
    EXPECT_TRUE(bi20->is_continue_entire_loop);

    // Backedge block, but is not a declared header, merge, or continue
    const auto* bi40 = fe.GetBlockInfo(40);
    ASSERT_NE(bi40, nullptr);
    EXPECT_EQ(bi40->merge_for_header, 0u);
    EXPECT_EQ(bi40->continue_for_header, 0u);
    EXPECT_EQ(bi40->header_for_merge, 0u);
    EXPECT_EQ(bi40->header_for_continue, 0u);
    EXPECT_FALSE(bi40->is_continue_entire_loop);

    // Merge block points to the header
    const auto* bi99 = fe.GetBlockInfo(99);
    ASSERT_NE(bi99, nullptr);
    EXPECT_EQ(bi99->merge_for_header, 0u);
    EXPECT_EQ(bi99->continue_for_header, 0u);
    EXPECT_EQ(bi99->header_for_merge, 20u);
    EXPECT_EQ(bi99->header_for_continue, 0u);
    EXPECT_FALSE(bi99->is_continue_entire_loop);
}

TEST_F(SpvParserCFGTest, RegisterMerges_GoodLoopMerge_MultiBlockLoop_ContinueIsNotHeader_Branch) {
    auto p = parser(test::Assemble(CommonTypes() + R"(
     %100 = OpFunction %void None %voidfn

     %10 = OpLabel
     OpBranch %20

     %20 = OpLabel
     OpLoopMerge %99 %40 None
     OpBranch %30

     %30 = OpLabel
     OpBranchConditional %cond %40 %99

     %40 = OpLabel
     OpBranch %20

     %99 = OpLabel
     OpReturn

     OpFunctionEnd
  )"));
    ASSERT_TRUE(p->BuildAndParseInternalModuleExceptFunctions()) << p->error();
    auto fe = p->function_emitter(100);
    fe.RegisterBasicBlocks();
    EXPECT_TRUE(fe.RegisterMerges());

    // Loop header points to continue and merge
    const auto* bi20 = fe.GetBlockInfo(20);
    ASSERT_NE(bi20, nullptr);
    EXPECT_EQ(bi20->merge_for_header, 99u);
    EXPECT_EQ(bi20->continue_for_header, 40u);
    EXPECT_EQ(bi20->header_for_merge, 0u);
    EXPECT_EQ(bi20->header_for_continue, 0u);
    EXPECT_FALSE(bi20->is_continue_entire_loop);

    // Continue block points to header
    const auto* bi40 = fe.GetBlockInfo(40);
    ASSERT_NE(bi40, nullptr);
    EXPECT_EQ(bi40->merge_for_header, 0u);
    EXPECT_EQ(bi40->continue_for_header, 0u);
    EXPECT_EQ(bi40->header_for_merge, 0u);
    EXPECT_EQ(bi40->header_for_continue, 20u);
    EXPECT_FALSE(bi40->is_continue_entire_loop);

    // Merge block points to the header
    const auto* bi99 = fe.GetBlockInfo(99);
    ASSERT_NE(bi99, nullptr);
    EXPECT_EQ(bi99->merge_for_header, 0u);
    EXPECT_EQ(bi99->continue_for_header, 0u);
    EXPECT_EQ(bi99->header_for_merge, 20u);
    EXPECT_EQ(bi99->header_for_continue, 0u);
    EXPECT_FALSE(bi99->is_continue_entire_loop);
}

TEST_F(SpvParserCFGTest,
       RegisterMerges_GoodLoopMerge_MultiBlockLoop_ContinueIsNotHeader_BranchConditional) {
    auto p = parser(test::Assemble(CommonTypes() + R"(
     %100 = OpFunction %void None %voidfn

     %10 = OpLabel
     OpBranch %20

     %20 = OpLabel
     OpLoopMerge %99 %40 None
     OpBranchConditional %cond %30 %99

     %30 = OpLabel
     OpBranch %40

     %40 = OpLabel
     OpBranch %20

     %99 = OpLabel
     OpReturn

     OpFunctionEnd
  )"));
    ASSERT_TRUE(p->BuildAndParseInternalModuleExceptFunctions()) << p->error();
    auto fe = p->function_emitter(100);
    fe.RegisterBasicBlocks();
    EXPECT_TRUE(fe.RegisterMerges());

    // Loop header points to continue and merge
    const auto* bi20 = fe.GetBlockInfo(20);
    ASSERT_NE(bi20, nullptr);
    EXPECT_EQ(bi20->merge_for_header, 99u);
    EXPECT_EQ(bi20->continue_for_header, 40u);
    EXPECT_EQ(bi20->header_for_merge, 0u);
    EXPECT_EQ(bi20->header_for_continue, 0u);
    EXPECT_FALSE(bi20->is_continue_entire_loop);

    // Continue block points to header
    const auto* bi40 = fe.GetBlockInfo(40);
    ASSERT_NE(bi40, nullptr);
    EXPECT_EQ(bi40->merge_for_header, 0u);
    EXPECT_EQ(bi40->continue_for_header, 0u);
    EXPECT_EQ(bi40->header_for_merge, 0u);
    EXPECT_EQ(bi40->header_for_continue, 20u);
    EXPECT_FALSE(bi40->is_continue_entire_loop);

    // Merge block points to the header
    const auto* bi99 = fe.GetBlockInfo(99);
    ASSERT_NE(bi99, nullptr);
    EXPECT_EQ(bi99->merge_for_header, 0u);
    EXPECT_EQ(bi99->continue_for_header, 0u);
    EXPECT_EQ(bi99->header_for_merge, 20u);
    EXPECT_EQ(bi99->header_for_continue, 0u);
    EXPECT_FALSE(bi99->is_continue_entire_loop);
}

TEST_F(SpvParserCFGTest, RegisterMerges_SelectionMerge_BadTerminator) {
    auto p = parser(test::Assemble(CommonTypes() + R"(
     %100 = OpFunction %void None %voidfn

     %10 = OpLabel
     OpSelectionMerge %99 None
     OpBranch %20

     %20 = OpLabel
     OpBranch %99

     %99 = OpLabel
     OpReturn

     OpFunctionEnd
  )"));
    ASSERT_TRUE(p->BuildAndParseInternalModuleExceptFunctions()) << p->error();
    auto fe = p->function_emitter(100);
    fe.RegisterBasicBlocks();
    EXPECT_FALSE(fe.RegisterMerges());
    EXPECT_THAT(p->error(), Eq("Selection header 10 does not end in an "
                               "OpBranchConditional or OpSwitch instruction"));
}

TEST_F(SpvParserCFGTest, RegisterMerges_LoopMerge_BadTerminator) {
    auto p = parser(test::Assemble(CommonTypes() + R"(
     %100 = OpFunction %void None %voidfn

     %10 = OpLabel
     OpBranch %20

     %20 = OpLabel
     OpLoopMerge %99 %40 None
     OpSwitch %selector %99 30 %30

     %30 = OpLabel
     OpBranch %99

     %40 = OpLabel
     OpBranch %20

     %99 = OpLabel
     OpReturn

     OpFunctionEnd
  )"));
    ASSERT_TRUE(p->BuildAndParseInternalModuleExceptFunctions()) << p->error();
    auto fe = p->function_emitter(100);
    fe.RegisterBasicBlocks();
    EXPECT_FALSE(fe.RegisterMerges());
    EXPECT_THAT(p->error(), Eq("Loop header 20 does not end in an OpBranch or "
                               "OpBranchConditional instruction"));
}

TEST_F(SpvParserCFGTest, RegisterMerges_BadMergeBlock) {
    auto p = parser(test::Assemble(CommonTypes() + R"(
     %100 = OpFunction %void None %voidfn

     %10 = OpLabel
     OpSelectionMerge %void None
     OpBranchConditional %cond %30 %99

     %30 = OpLabel
     OpBranch %99

     %99 = OpLabel
     OpReturn

     OpFunctionEnd
  )"));
    ASSERT_TRUE(p->BuildAndParseInternalModuleExceptFunctions()) << p->error();
    auto fe = p->function_emitter(100);
    fe.RegisterBasicBlocks();
    EXPECT_FALSE(fe.RegisterMerges());
    EXPECT_THAT(p->error(), Eq("Structured header block 10 declares invalid merge block 2"));
}

TEST_F(SpvParserCFGTest, RegisterMerges_HeaderIsItsOwnMerge) {
    auto p = parser(test::Assemble(CommonTypes() + R"(
     %100 = OpFunction %void None %voidfn

     %10 = OpLabel
     OpSelectionMerge %10 None
     OpBranchConditional %cond %30 %99

     %30 = OpLabel
     OpBranch %99

     %99 = OpLabel
     OpReturn

     OpFunctionEnd
  )"));
    ASSERT_TRUE(p->BuildAndParseInternalModuleExceptFunctions()) << p->error();
    auto fe = p->function_emitter(100);
    fe.RegisterBasicBlocks();
    EXPECT_FALSE(fe.RegisterMerges());
    EXPECT_THAT(p->error(), Eq("Structured header block 10 cannot be its own merge block"));
}

TEST_F(SpvParserCFGTest, RegisterMerges_MergeReused) {
    auto p = parser(test::Assemble(CommonTypes() + R"(
     %100 = OpFunction %void None %voidfn

     %10 = OpLabel
     OpSelectionMerge %49 None
     OpBranchConditional %cond %20 %49

     %20 = OpLabel
     OpBranch %49

     %49 = OpLabel
     OpBranch %50

     %50 = OpLabel
     OpSelectionMerge %49 None  ; can't reuse merge block
     OpBranchConditional %cond %60 %99

     %60 = OpLabel
     OpBranch %99

     %99 = OpLabel
     OpReturn

     OpFunctionEnd
  )"));
    ASSERT_TRUE(p->BuildAndParseInternalModuleExceptFunctions()) << p->error();
    auto fe = p->function_emitter(100);
    fe.RegisterBasicBlocks();
    EXPECT_FALSE(fe.RegisterMerges());
    EXPECT_THAT(p->error(),
                Eq("Block 49 declared as merge block for more than one header: 10, 50"));
}

TEST_F(SpvParserCFGTest, RegisterMerges_EntryBlockIsLoopHeader) {
    auto p = parser(test::Assemble(CommonTypes() + R"(
     %100 = OpFunction %void None %voidfn

     %10 = OpLabel
     OpLoopMerge %99 %30 None
     OpBranchConditional %cond %10 %99

     %30 = OpLabel
     OpBranch %10

     %99 = OpLabel
     OpReturn

     OpFunctionEnd
  )"));
    ASSERT_TRUE(p->BuildAndParseInternalModuleExceptFunctions()) << p->error();
    auto fe = p->function_emitter(100);
    fe.RegisterBasicBlocks();
    EXPECT_FALSE(fe.RegisterMerges());
    EXPECT_THAT(p->error(), Eq("Function entry block 10 cannot be a loop header"));
}

TEST_F(SpvParserCFGTest, RegisterMerges_BadContinueTarget) {
    auto p = parser(test::Assemble(CommonTypes() + R"(
     %100 = OpFunction %void None %voidfn

     %10 = OpLabel
     OpBranch %20

     %20 = OpLabel
     OpLoopMerge %99 %999 None
     OpBranchConditional %cond %20 %99

     %99 = OpLabel
     OpReturn

     OpFunctionEnd
  )"));
    ASSERT_TRUE(p->BuildAndParseInternalModuleExceptFunctions()) << p->error();
    auto fe = p->function_emitter(100);
    fe.RegisterBasicBlocks();
    EXPECT_FALSE(fe.RegisterMerges());
    EXPECT_THAT(p->error(), Eq("Structured header 20 declares invalid continue target 999"));
}

TEST_F(SpvParserCFGTest, RegisterMerges_MergeSameAsContinue) {
    auto p = parser(test::Assemble(CommonTypes() + R"(
     %100 = OpFunction %void None %voidfn

     %10 = OpLabel
     OpBranch %20

     %20 = OpLabel
     OpLoopMerge %50 %50 None
     OpBranchConditional %cond %20 %99


     %50 = OpLabel
     OpBranch %20

     %99 = OpLabel
     OpReturn

     OpFunctionEnd
  )"));
    ASSERT_TRUE(p->BuildAndParseInternalModuleExceptFunctions()) << p->error();
    auto fe = p->function_emitter(100);
    fe.RegisterBasicBlocks();
    EXPECT_FALSE(fe.RegisterMerges());
    EXPECT_THAT(p->error(), Eq("Invalid structured header block 20: declares block 50 as "
                               "both its merge block and continue target"));
}

TEST_F(SpvParserCFGTest, RegisterMerges_ContinueReused) {
    auto p = parser(test::Assemble(CommonTypes() + R"(
     %100 = OpFunction %void None %voidfn

     %10 = OpLabel
     OpBranch %20

     %20 = OpLabel
     OpLoopMerge %49 %40 None
     OpBranchConditional %cond %30 %49

     %30 = OpLabel
     OpBranch %40

     %40 = OpLabel
     OpBranch %20

     %49 = OpLabel
     OpBranch %50

     %50 = OpLabel
     OpLoopMerge %99 %40 None
     OpBranchConditional %cond %60 %99

     %60 = OpLabel
     OpBranch %70

     %70 = OpLabel
     OpBranch %50

     %99 = OpLabel
     OpReturn

     OpFunctionEnd
  )"));
    ASSERT_TRUE(p->BuildAndParseInternalModuleExceptFunctions()) << p->error();
    auto fe = p->function_emitter(100);
    fe.RegisterBasicBlocks();
    EXPECT_FALSE(fe.RegisterMerges());
    EXPECT_THAT(p->error(), Eq("Block 40 declared as continue target for more "
                               "than one header: 20, 50"));
}

TEST_F(SpvParserCFGTest, RegisterMerges_SingleBlockLoop_NotItsOwnContinue) {
    auto p = parser(test::Assemble(CommonTypes() + R"(
     %100 = OpFunction %void None %voidfn

     %10 = OpLabel
     OpBranch %20

     %20 = OpLabel
     OpLoopMerge %99 %30 None
     OpBranchConditional %cond %20 %99

     %30 = OpLabel
     OpBranch %20

     %99 = OpLabel
     OpReturn

     OpFunctionEnd
  )"));
    ASSERT_TRUE(p->BuildAndParseInternalModuleExceptFunctions()) << p->error();
    auto fe = p->function_emitter(100);
    fe.RegisterBasicBlocks();
    EXPECT_FALSE(fe.RegisterMerges());
    EXPECT_THAT(p->error(), Eq("Block 20 branches to itself but is not its own continue target"));
}

TEST_F(SpvParserCFGTest, ComputeBlockOrder_OneBlock) {
    auto p = parser(test::Assemble(CommonTypes() + R"(
     %100 = OpFunction %void None %voidfn

     %42 = OpLabel
     OpReturn

     OpFunctionEnd
  )"));
    ASSERT_TRUE(p->BuildAndParseInternalModuleExceptFunctions()) << p->error();
    auto fe = p->function_emitter(100);
    fe.RegisterBasicBlocks();
    fe.ComputeBlockOrderAndPositions();

    EXPECT_THAT(fe.block_order(), ElementsAre(42));

    const auto* bi = fe.GetBlockInfo(42);
    ASSERT_NE(bi, nullptr);
    EXPECT_EQ(bi->pos, 0u);
}

TEST_F(SpvParserCFGTest, ComputeBlockOrder_IgnoreStaticalyUnreachable) {
    auto p = parser(test::Assemble(CommonTypes() + R"(
     %100 = OpFunction %void None %voidfn

     %10 = OpLabel
     OpBranch %20

     %15 = OpLabel ; statically dead
     OpReturn

     %20 = OpLabel
     OpReturn

     OpFunctionEnd
  )"));
    ASSERT_TRUE(p->BuildAndParseInternalModuleExceptFunctions()) << p->error();
    auto fe = p->function_emitter(100);
    fe.RegisterBasicBlocks();
    fe.ComputeBlockOrderAndPositions();

    EXPECT_THAT(fe.block_order(), ElementsAre(10, 20));
}

TEST_F(SpvParserCFGTest, ComputeBlockOrder_KillIsDeadEnd) {
    auto p = parser(test::Assemble(CommonTypes() + R"(
     %100 = OpFunction %void None %voidfn

     %10 = OpLabel
     OpBranch %20

     %15 = OpLabel ; statically dead
     OpReturn

     %20 = OpLabel
     OpKill        ; Kill doesn't lead anywhere

     OpFunctionEnd
  )"));
    ASSERT_TRUE(p->BuildAndParseInternalModuleExceptFunctions()) << p->error();
    auto fe = p->function_emitter(100);
    fe.RegisterBasicBlocks();
    fe.ComputeBlockOrderAndPositions();

    EXPECT_THAT(fe.block_order(), ElementsAre(10, 20));
}

TEST_F(SpvParserCFGTest, ComputeBlockOrder_UnreachableIsDeadEnd) {
    auto p = parser(test::Assemble(CommonTypes() + R"(
     %100 = OpFunction %void None %voidfn

     %10 = OpLabel
     OpBranch %20

     %15 = OpLabel ; statically dead
     OpReturn

     %20 = OpLabel
     OpUnreachable ; Unreachable doesn't lead anywhere

     OpFunctionEnd
  )"));
    ASSERT_TRUE(p->BuildAndParseInternalModuleExceptFunctions()) << p->error();
    auto fe = p->function_emitter(100);
    fe.RegisterBasicBlocks();
    fe.ComputeBlockOrderAndPositions();

    EXPECT_THAT(fe.block_order(), ElementsAre(10, 20));
}

TEST_F(SpvParserCFGTest, ComputeBlockOrder_ReorderSequence) {
    auto p = parser(test::Assemble(CommonTypes() + R"(
     %100 = OpFunction %void None %voidfn

     %10 = OpLabel
     OpSelectionMerge %99 None
     OpBranchConditional %cond %20 %30

     %30 = OpLabel
     OpReturn

     %20 = OpLabel
     OpBranch %30 ; backtrack, but does dominate %30

     %99 = OpLabel
     OpReturn

     OpFunctionEnd
  )"));
    ASSERT_TRUE(p->BuildAndParseInternalModuleExceptFunctions()) << p->error();
    auto fe = p->function_emitter(100);
    fe.RegisterBasicBlocks();
    fe.ComputeBlockOrderAndPositions();

    EXPECT_THAT(fe.block_order(), ElementsAre(10, 20, 30, 99));

    const auto* bi10 = fe.GetBlockInfo(10);
    ASSERT_NE(bi10, nullptr);
    EXPECT_EQ(bi10->pos, 0u);
    const auto* bi20 = fe.GetBlockInfo(20);
    ASSERT_NE(bi20, nullptr);
    EXPECT_EQ(bi20->pos, 1u);
    const auto* bi30 = fe.GetBlockInfo(30);
    ASSERT_NE(bi30, nullptr);
    EXPECT_EQ(bi30->pos, 2u);
    const auto* bi99 = fe.GetBlockInfo(99);
    ASSERT_NE(bi99, nullptr);
    EXPECT_EQ(bi99->pos, 3u);
}

TEST_F(SpvParserCFGTest, ComputeBlockOrder_DupConditionalBranch) {
    auto p = parser(test::Assemble(CommonTypes() + R"(
     %100 = OpFunction %void None %voidfn

     %10 = OpLabel
     OpSelectionMerge %99 None
     OpBranchConditional %cond %20 %20

     %20 = OpLabel
     OpBranch %99

     %99 = OpLabel
     OpReturn

     OpFunctionEnd
  )"));
    ASSERT_TRUE(p->BuildAndParseInternalModuleExceptFunctions()) << p->error();
    auto fe = p->function_emitter(100);
    fe.RegisterBasicBlocks();
    fe.ComputeBlockOrderAndPositions();

    EXPECT_THAT(fe.block_order(), ElementsAre(10, 20, 99));
}

TEST_F(SpvParserCFGTest, ComputeBlockOrder_RespectConditionalBranchOrder) {
    auto p = parser(test::Assemble(CommonTypes() + R"(
     %100 = OpFunction %void None %voidfn

     %10 = OpLabel
     OpSelectionMerge %99 None
     OpBranchConditional %cond %20 %30

     %30 = OpLabel
     OpReturn

     %20 = OpLabel
     OpBranch %99

     %99 = OpLabel ; dominated by %20, so follow %20
     OpReturn

     OpFunctionEnd
  )"));
    ASSERT_TRUE(p->BuildAndParseInternalModuleExceptFunctions()) << p->error();
    auto fe = p->function_emitter(100);
    fe.RegisterBasicBlocks();
    fe.ComputeBlockOrderAndPositions();

    EXPECT_THAT(fe.block_order(), ElementsAre(10, 20, 30, 99));
}

TEST_F(SpvParserCFGTest, ComputeBlockOrder_TrueOnlyBranch) {
    auto p = parser(test::Assemble(CommonTypes() + R"(
     %100 = OpFunction %void None %voidfn

     %10 = OpLabel
     OpSelectionMerge %99 None
     OpBranchConditional %cond %20 %99

     %99 = OpLabel
     OpReturn

     %20 = OpLabel
     OpBranch %99

     OpFunctionEnd
  )"));
    ASSERT_TRUE(p->BuildAndParseInternalModuleExceptFunctions()) << p->error();
    auto fe = p->function_emitter(100);
    fe.RegisterBasicBlocks();
    fe.ComputeBlockOrderAndPositions();

    EXPECT_THAT(fe.block_order(), ElementsAre(10, 20, 99));
}

TEST_F(SpvParserCFGTest, ComputeBlockOrder_FalseOnlyBranch) {
    auto p = parser(test::Assemble(CommonTypes() + R"(
     %100 = OpFunction %void None %voidfn

     %10 = OpLabel
     OpSelectionMerge %99 None
     OpBranchConditional %cond %99 %20

     %99 = OpLabel
     OpReturn

     %20 = OpLabel
     OpBranch %99

     OpFunctionEnd
  )"));
    ASSERT_TRUE(p->BuildAndParseInternalModuleExceptFunctions()) << p->error();
    auto fe = p->function_emitter(100);
    fe.RegisterBasicBlocks();
    fe.ComputeBlockOrderAndPositions();

    EXPECT_THAT(fe.block_order(), ElementsAre(10, 20, 99));
}

TEST_F(SpvParserCFGTest, ComputeBlockOrder_SwitchOrderNaturallyReversed) {
    auto p = parser(test::Assemble(CommonTypes() + R"(
     %100 = OpFunction %void None %voidfn

     %10 = OpLabel
     OpSelectionMerge %99 None
     OpSwitch %selector %99 20 %20 30 %30

     %99 = OpLabel
     OpReturn

     %30 = OpLabel
     OpReturn

     %20 = OpLabel
     OpBranch %99

     OpFunctionEnd
  )"));
    ASSERT_TRUE(p->BuildAndParseInternalModuleExceptFunctions()) << p->error();
    auto fe = p->function_emitter(100);
    fe.RegisterBasicBlocks();
    fe.ComputeBlockOrderAndPositions();

    EXPECT_THAT(fe.block_order(), ElementsAre(10, 30, 20, 99));
}

TEST_F(SpvParserCFGTest, ComputeBlockOrder_SwitchWithDefaultOrderNaturallyReversed) {
    auto p = parser(test::Assemble(CommonTypes() + R"(
     %100 = OpFunction %void None %voidfn

     %10 = OpLabel
     OpSelectionMerge %99 None
     OpSwitch %selector %80 20 %20 30 %30

     %80 = OpLabel ; the default case
     OpBranch %99

     %99 = OpLabel
     OpReturn

     %30 = OpLabel
     OpReturn

     %20 = OpLabel
     OpBranch %99

     OpFunctionEnd
  )"));
    ASSERT_TRUE(p->BuildAndParseInternalModuleExceptFunctions()) << p->error();
    auto fe = p->function_emitter(100);
    fe.RegisterBasicBlocks();
    fe.ComputeBlockOrderAndPositions();

    EXPECT_THAT(fe.block_order(), ElementsAre(10, 30, 20, 80, 99));
}

TEST_F(SpvParserCFGTest, ComputeBlockOrder_Switch_DefaultSameAsACase) {
    auto p = parser(test::Assemble(CommonTypes() + R"(
     %100 = OpFunction %void None %voidfn

     %10 = OpLabel
     OpSelectionMerge %99 None
     OpSwitch %selector %30 20 %20 30 %30 40 %40

     %99 = OpLabel
     OpReturn

     %30 = OpLabel
     OpBranch %99

     %20 = OpLabel
     OpBranch %99

     %40 = OpLabel
     OpBranch %99

     OpFunctionEnd
  )"));
    ASSERT_TRUE(p->BuildAndParseInternalModuleExceptFunctions()) << p->error();
    auto fe = p->function_emitter(100);
    fe.RegisterBasicBlocks();
    fe.ComputeBlockOrderAndPositions();

    EXPECT_THAT(fe.block_order(), ElementsAre(10, 40, 20, 30, 99));
}

TEST_F(SpvParserCFGTest, ClassifyCFGEdges_Fallthrough_IsError) {
    auto assembly = CommonTypes() + R"(
     %100 = OpFunction %void None %voidfn

     %10 = OpLabel
     OpSelectionMerge %99 None
     OpSwitch %selector %80 20 %20 30 %30 40 %40

     %80 = OpLabel ; the default case
     OpBranch %30 ; fallthrough to another case

     %99 = OpLabel
     OpReturn

     %40 = OpLabel
     OpBranch %99

     %30 = OpLabel
     OpBranch %40

     %20 = OpLabel
     OpBranch %99

     OpFunctionEnd
  )";
    auto p = parser(test::Assemble(assembly));
    ASSERT_TRUE(p->BuildAndParseInternalModuleExceptFunctions()) << p->error();
    auto fe = p->function_emitter(100);
    EXPECT_FALSE(FlowClassifyCFGEdges(&fe)) << p->error();
    // Some further processing
    EXPECT_THAT(p->error(), Eq("Fallthrough not permitted in WGSL"));
}

TEST_F(SpvParserCFGTest, ComputeBlockOrder_Nest_If_Contains_If) {
    auto assembly = CommonTypes() + R"(
     %100 = OpFunction %void None %voidfn

     %10 = OpLabel
     OpSelectionMerge %99 None
     OpBranchConditional %cond %20 %50

     %99 = OpLabel
     OpReturn

     %20 = OpLabel
     OpSelectionMerge %49 None
     OpBranchConditional %cond %30 %40

     %49 = OpLabel
     OpBranch %99

     %30 = OpLabel
     OpBranch %49

     %40 = OpLabel
     OpBranch %49

     %50 = OpLabel
     OpSelectionMerge %79 None
     OpBranchConditional %cond %60 %70

     %79 = OpLabel
     OpBranch %99

     %60 = OpLabel
     OpBranch %79

     %70 = OpLabel
     OpBranch %79

     OpFunctionEnd
  )";
    auto p = parser(test::Assemble(assembly));
    ASSERT_TRUE(p->BuildAndParseInternalModuleExceptFunctions()) << p->error();
    auto fe = p->function_emitter(100);
    fe.RegisterBasicBlocks();
    fe.ComputeBlockOrderAndPositions();

    EXPECT_THAT(fe.block_order(), ElementsAre(10, 20, 30, 40, 49, 50, 60, 70, 79, 99)) << assembly;
}

TEST_F(SpvParserCFGTest, ComputeBlockOrder_Nest_If_In_SwitchCase) {
    auto assembly = CommonTypes() + R"(
     %100 = OpFunction %void None %voidfn

     %10 = OpLabel
     OpSelectionMerge %99 None
     OpSwitch %selector %50 20 %20 50 %50

     %99 = OpLabel
     OpReturn

     %20 = OpLabel
     OpSelectionMerge %49 None
     OpBranchConditional %cond %30 %40

     %49 = OpLabel
     OpBranch %99

     %30 = OpLabel
     OpBranch %49

     %40 = OpLabel
     OpBranch %49

     %50 = OpLabel
     OpSelectionMerge %79 None
     OpBranchConditional %cond %60 %70

     %79 = OpLabel
     OpBranch %99

     %60 = OpLabel
     OpBranch %79

     %70 = OpLabel
     OpBranch %79

     OpFunctionEnd
  )";
    auto p = parser(test::Assemble(assembly));
    ASSERT_TRUE(p->BuildAndParseInternalModuleExceptFunctions()) << p->error();
    auto fe = p->function_emitter(100);
    fe.RegisterBasicBlocks();
    fe.ComputeBlockOrderAndPositions();

    EXPECT_THAT(fe.block_order(), ElementsAre(10, 20, 30, 40, 49, 50, 60, 70, 79, 99)) << assembly;
}

TEST_F(SpvParserCFGTest, ComputeBlockOrder_Nest_IfBreak_In_SwitchCase) {
    auto assembly = CommonTypes() + R"(
     %100 = OpFunction %void None %voidfn

     %10 = OpLabel
     OpSelectionMerge %99 None
     OpSwitch %selector %50 20 %20 50 %50

     %99 = OpLabel
     OpReturn

     %20 = OpLabel
     OpSelectionMerge %49 None
     OpBranchConditional %cond %99 %40 ; break-if

     %40 = OpLabel
     OpBranch %49

     %49 = OpLabel
     OpBranch %99

     %50 = OpLabel
     OpSelectionMerge %79 None
     OpBranchConditional %cond %60 %99 ; break-unless

     %60 = OpLabel
     OpBranch %79

     %79 = OpLabel ; dominated by 60, so must follow 60
     OpBranch %99

     OpFunctionEnd
  )";
    auto p = parser(test::Assemble(assembly));
    ASSERT_TRUE(p->BuildAndParseInternalModuleExceptFunctions()) << p->error();
    auto fe = p->function_emitter(100);
    fe.RegisterBasicBlocks();
    fe.ComputeBlockOrderAndPositions();

    EXPECT_THAT(fe.block_order(), ElementsAre(10, 20, 40, 49, 50, 60, 79, 99)) << assembly;
}

TEST_F(SpvParserCFGTest, ComputeBlockOrder_Loop_SingleBlock_Simple) {
    auto assembly = CommonTypes() + R"(
     %100 = OpFunction %void None %voidfn

     ; The entry block can't be the target of a branch
     %10 = OpLabel
     OpBranch %20

     %20 = OpLabel
     OpLoopMerge %99 %20 None
     OpBranchConditional %cond %20 %99

     %99 = OpLabel
     OpReturn

     OpFunctionEnd
  )";
    auto p = parser(test::Assemble(assembly));
    ASSERT_TRUE(p->BuildAndParseInternalModuleExceptFunctions()) << p->error();
    auto fe = p->function_emitter(100);
    fe.RegisterBasicBlocks();
    fe.ComputeBlockOrderAndPositions();

    EXPECT_THAT(fe.block_order(), ElementsAre(10, 20, 99)) << assembly;
}

TEST_F(SpvParserCFGTest, ComputeBlockOrder_Loop_SingleBlock_Infinite) {
    auto assembly = CommonTypes() + R"(
     %100 = OpFunction %void None %voidfn

     ; The entry block can't be the target of a branch
     %10 = OpLabel
     OpBranch %20

     %20 = OpLabel
     OpLoopMerge %99 %20 None
     OpBranch %20

     %99 = OpLabel
     OpReturn

     OpFunctionEnd
  )";
    auto p = parser(test::Assemble(assembly));
    ASSERT_TRUE(p->BuildAndParseInternalModuleExceptFunctions()) << p->error();
    auto fe = p->function_emitter(100);
    fe.RegisterBasicBlocks();
    fe.ComputeBlockOrderAndPositions();

    EXPECT_THAT(fe.block_order(), ElementsAre(10, 20, 99)) << assembly;
}

TEST_F(SpvParserCFGTest, ComputeBlockOrder_Loop_SingleBlock_DupInfinite) {
    auto assembly = CommonTypes() + R"(
     %100 = OpFunction %void None %voidfn

     ; The entry block can't be the target of a branch
     %10 = OpLabel
     OpBranch %20

     %20 = OpLabel
     OpLoopMerge %99 %20 None
     OpBranchConditional %cond %20 %20

     %99 = OpLabel
     OpReturn

     OpFunctionEnd
  )";
    auto p = parser(test::Assemble(assembly));
    ASSERT_TRUE(p->BuildAndParseInternalModuleExceptFunctions()) << p->error();
    auto fe = p->function_emitter(100);
    fe.RegisterBasicBlocks();
    fe.ComputeBlockOrderAndPositions();

    EXPECT_THAT(fe.block_order(), ElementsAre(10, 20, 99)) << assembly;
}

TEST_F(SpvParserCFGTest, ComputeBlockOrder_Loop_HeaderHasBreakIf) {
    auto assembly = CommonTypes() + R"(
     %100 = OpFunction %void None %voidfn

     %10 = OpLabel
     OpBranch %20

     %20 = OpLabel
     OpLoopMerge %99 %50 None
     OpBranchConditional %cond %30 %99 ; like While

     %30 = OpLabel ; trivial body
     OpBranch %50

     %50 = OpLabel
     OpBranch %20

     %99 = OpLabel
     OpReturn

     OpFunctionEnd
  )";
    auto p = parser(test::Assemble(assembly));
    ASSERT_TRUE(p->BuildAndParseInternalModuleExceptFunctions()) << p->error();
    auto fe = p->function_emitter(100);
    fe.RegisterBasicBlocks();
    fe.ComputeBlockOrderAndPositions();

    EXPECT_THAT(fe.block_order(), ElementsAre(10, 20, 30, 50, 99)) << assembly;
}

TEST_F(SpvParserCFGTest, ComputeBlockOrder_Loop_HeaderHasBreakUnless) {
    auto assembly = CommonTypes() + R"(
     %100 = OpFunction %void None %voidfn

     %10 = OpLabel
     OpBranch %20

     %20 = OpLabel
     OpLoopMerge %99 %50 None
     OpBranchConditional %cond %99 %30 ; has break-unless

     %30 = OpLabel ; trivial body
     OpBranch %50

     %50 = OpLabel
     OpBranch %20

     %99 = OpLabel
     OpReturn

     OpFunctionEnd
  )";
    auto p = parser(test::Assemble(assembly));
    ASSERT_TRUE(p->BuildAndParseInternalModuleExceptFunctions()) << p->error();
    auto fe = p->function_emitter(100);
    fe.RegisterBasicBlocks();
    fe.ComputeBlockOrderAndPositions();

    EXPECT_THAT(fe.block_order(), ElementsAre(10, 20, 30, 50, 99)) << assembly;
}

TEST_F(SpvParserCFGTest, ComputeBlockOrder_Loop_BodyHasBreak) {
    auto assembly = CommonTypes() + R"(
     %100 = OpFunction %void None %voidfn

     %10 = OpLabel
     OpBranch %20

     %20 = OpLabel
     OpLoopMerge %99 %50 None
     OpBranchConditional %cond %30 %99

     %30 = OpLabel
     OpBranch %99 ; break

     %50 = OpLabel
     OpBranch %20

     %99 = OpLabel
     OpReturn

     OpFunctionEnd
  )";
    auto p = parser(test::Assemble(assembly));
    ASSERT_TRUE(p->BuildAndParseInternalModuleExceptFunctions()) << p->error();
    auto fe = p->function_emitter(100);
    fe.RegisterBasicBlocks();
    fe.ComputeBlockOrderAndPositions();

    EXPECT_THAT(fe.block_order(), ElementsAre(10, 20, 30, 50, 99)) << assembly;
}

TEST_F(SpvParserCFGTest, ComputeBlockOrder_Loop_BodyHasBreakIf) {
    auto assembly = CommonTypes() + R"(
     %100 = OpFunction %void None %voidfn

     %10 = OpLabel
     OpBranch %20

     %20 = OpLabel
     OpLoopMerge %99 %50 None
     OpBranchConditional %cond %30 %99

     %30 = OpLabel
     OpBranchConditional %cond2 %99 %40 ; break-if

     %40 = OpLabel
     OpBranch %50

     %50 = OpLabel
     OpBranch %20

     %99 = OpLabel
     OpReturn

     OpFunctionEnd
  )";
    auto p = parser(test::Assemble(assembly));
    ASSERT_TRUE(p->BuildAndParseInternalModuleExceptFunctions()) << p->error();
    auto fe = p->function_emitter(100);
    fe.RegisterBasicBlocks();
    fe.ComputeBlockOrderAndPositions();

    EXPECT_THAT(fe.block_order(), ElementsAre(10, 20, 30, 40, 50, 99)) << assembly;
}

TEST_F(SpvParserCFGTest, ComputeBlockOrder_Loop_BodyHasBreakUnless) {
    auto assembly = CommonTypes() + R"(
     %100 = OpFunction %void None %voidfn

     %10 = OpLabel
     OpBranch %20

     %20 = OpLabel
     OpLoopMerge %99 %50 None
     OpBranchConditional %cond %30 %99

     %30 = OpLabel
     OpBranchConditional %cond2 %40 %99 ; break-unless

     %40 = OpLabel
     OpBranch %50

     %50 = OpLabel
     OpBranch %20

     %99 = OpLabel
     OpReturn

     OpFunctionEnd
  )";
    auto p = parser(test::Assemble(assembly));
    ASSERT_TRUE(p->BuildAndParseInternalModuleExceptFunctions()) << p->error();
    auto fe = p->function_emitter(100);
    fe.RegisterBasicBlocks();
    fe.ComputeBlockOrderAndPositions();

    EXPECT_THAT(fe.block_order(), ElementsAre(10, 20, 30, 40, 50, 99)) << assembly;
}

TEST_F(SpvParserCFGTest, ComputeBlockOrder_Loop_Body_If) {
    auto assembly = CommonTypes() + R"(
     %100 = OpFunction %void None %voidfn

     %10 = OpLabel
     OpBranch %20

     %20 = OpLabel
     OpLoopMerge %99 %50 None
     OpBranchConditional %cond %30 %99

     %30 = OpLabel
     OpSelectionMerge %49 None
     OpBranchConditional %cond2 %40 %45 ; nested if

     %40 = OpLabel
     OpBranch %49

     %45 = OpLabel
     OpBranch %49

     %49 = OpLabel
     OpBranch %50

     %50 = OpLabel
     OpBranch %20

     %99 = OpLabel
     OpReturn

     OpFunctionEnd
  )";
    auto p = parser(test::Assemble(assembly));
    ASSERT_TRUE(p->BuildAndParseInternalModuleExceptFunctions()) << p->error();
    auto fe = p->function_emitter(100);
    fe.RegisterBasicBlocks();
    fe.ComputeBlockOrderAndPositions();

    EXPECT_THAT(fe.block_order(), ElementsAre(10, 20, 30, 40, 45, 49, 50, 99)) << assembly;
}

TEST_F(SpvParserCFGTest, ComputeBlockOrder_Loop_Body_If_Break) {
    auto assembly = CommonTypes() + R"(
     %100 = OpFunction %void None %voidfn

     %10 = OpLabel
     OpBranch %20

     %20 = OpLabel
     OpLoopMerge %99 %50 None
     OpBranchConditional %cond %30 %99

     %30 = OpLabel
     OpSelectionMerge %49 None
     OpBranchConditional %cond2 %40 %49 ; nested if

     %40 = OpLabel
     OpBranch %99   ; break from nested if

     %49 = OpLabel
     OpBranch %50

     %50 = OpLabel
     OpBranch %20

     %99 = OpLabel
     OpReturn

     OpFunctionEnd
  )";
    auto p = parser(test::Assemble(assembly));
    ASSERT_TRUE(p->BuildAndParseInternalModuleExceptFunctions()) << p->error();
    auto fe = p->function_emitter(100);
    fe.RegisterBasicBlocks();
    fe.ComputeBlockOrderAndPositions();

    EXPECT_THAT(fe.block_order(), ElementsAre(10, 20, 30, 40, 49, 50, 99)) << assembly;
}

TEST_F(SpvParserCFGTest, ComputeBlockOrder_Loop_BodyHasContinueIf) {
    auto assembly = CommonTypes() + R"(
     %100 = OpFunction %void None %voidfn

     %10 = OpLabel
     OpBranch %20

     %20 = OpLabel
     OpLoopMerge %99 %50 None
     OpBranchConditional %cond %30 %99

     %30 = OpLabel
     OpBranchConditional %cond2 %50 %40 ; continue-if

     %40 = OpLabel
     OpBranch %50

     %50 = OpLabel
     OpBranch %20

     %99 = OpLabel
     OpReturn

     OpFunctionEnd
  )";
    auto p = parser(test::Assemble(assembly));
    ASSERT_TRUE(p->BuildAndParseInternalModuleExceptFunctions()) << p->error();
    auto fe = p->function_emitter(100);
    fe.RegisterBasicBlocks();
    fe.ComputeBlockOrderAndPositions();

    EXPECT_THAT(fe.block_order(), ElementsAre(10, 20, 30, 40, 50, 99)) << assembly;
}

TEST_F(SpvParserCFGTest, ComputeBlockOrder_Loop_BodyHasContinueUnless) {
    auto assembly = CommonTypes() + R"(
     %100 = OpFunction %void None %voidfn

     %10 = OpLabel
     OpBranch %20

     %20 = OpLabel
     OpLoopMerge %99 %50 None
     OpBranchConditional %cond %30 %99

     %30 = OpLabel
     OpBranchConditional %cond2 %40 %50 ; continue-unless

     %40 = OpLabel
     OpBranch %50

     %50 = OpLabel
     OpBranch %20

     %99 = OpLabel
     OpReturn

     OpFunctionEnd
  )";
    auto p = parser(test::Assemble(assembly));
    ASSERT_TRUE(p->BuildAndParseInternalModuleExceptFunctions()) << p->error();
    auto fe = p->function_emitter(100);
    fe.RegisterBasicBlocks();
    fe.ComputeBlockOrderAndPositions();

    EXPECT_THAT(fe.block_order(), ElementsAre(10, 20, 30, 40, 50, 99)) << assembly;
}

TEST_F(SpvParserCFGTest, ComputeBlockOrder_Loop_Body_If_Continue) {
    auto assembly = CommonTypes() + R"(
     %100 = OpFunction %void None %voidfn

     %10 = OpLabel
     OpBranch %20

     %20 = OpLabel
     OpLoopMerge %99 %50 None
     OpBranchConditional %cond %30 %99

     %30 = OpLabel
     OpSelectionMerge %49 None
     OpBranchConditional %cond2 %40 %49 ; nested if

     %40 = OpLabel
     OpBranch %50   ; continue from nested if

     %49 = OpLabel
     OpBranch %50

     %50 = OpLabel
     OpBranch %20

     %99 = OpLabel
     OpReturn

     OpFunctionEnd
  )";
    auto p = parser(test::Assemble(assembly));
    ASSERT_TRUE(p->BuildAndParseInternalModuleExceptFunctions()) << p->error();
    auto fe = p->function_emitter(100);
    fe.RegisterBasicBlocks();
    fe.ComputeBlockOrderAndPositions();

    EXPECT_THAT(fe.block_order(), ElementsAre(10, 20, 30, 40, 49, 50, 99)) << assembly;
}

TEST_F(SpvParserCFGTest, ComputeBlockOrder_Loop_Body_Switch) {
    auto assembly = CommonTypes() + R"(
     %100 = OpFunction %void None %voidfn

     %10 = OpLabel
     OpBranch %20

     %20 = OpLabel
     OpLoopMerge %99 %50 None
     OpBranchConditional %cond %30 %99

     %30 = OpLabel
     OpSelectionMerge %49 None
     OpSwitch %selector %49 40 %40 45 %45 ; fully nested switch

     %40 = OpLabel
     OpBranch %49

     %45 = OpLabel
     OpBranch %49

     %49 = OpLabel
     OpBranch %50

     %50 = OpLabel
     OpBranch %20

     %99 = OpLabel
     OpReturn

     OpFunctionEnd
  )";
    auto p = parser(test::Assemble(assembly));
    ASSERT_TRUE(p->BuildAndParseInternalModuleExceptFunctions()) << p->error();
    auto fe = p->function_emitter(100);
    fe.RegisterBasicBlocks();
    fe.ComputeBlockOrderAndPositions();

    EXPECT_THAT(fe.block_order(), ElementsAre(10, 20, 30, 45, 40, 49, 50, 99)) << assembly;
}

TEST_F(SpvParserCFGTest, ComputeBlockOrder_Loop_Body_Switch_CaseBreaks) {
    auto assembly = CommonTypes() + R"(
     %100 = OpFunction %void None %voidfn

     %10 = OpLabel
     OpBranch %20

     %20 = OpLabel
     OpLoopMerge %99 %50 None
     OpBranchConditional %cond %30 %99

     %30 = OpLabel
     OpSelectionMerge %49 None
     OpSwitch %selector %49 40 %40 45 %45

     %40 = OpLabel
     ; This case breaks out of the loop. This is not possible in C
     ; because "break" will escape the switch only.
     OpBranch %99

     %45 = OpLabel
     OpBranch %49

     %49 = OpLabel
     OpBranch %50

     %50 = OpLabel
     OpBranch %20

     %99 = OpLabel
     OpReturn

     OpFunctionEnd
  )";
    auto p = parser(test::Assemble(assembly));
    ASSERT_TRUE(p->BuildAndParseInternalModuleExceptFunctions()) << p->error();
    auto fe = p->function_emitter(100);
    fe.RegisterBasicBlocks();
    fe.ComputeBlockOrderAndPositions();

    EXPECT_THAT(fe.block_order(), ElementsAre(10, 20, 30, 45, 40, 49, 50, 99)) << assembly;

    // Fails SPIR-V validation:
    // Branch from block 40 to block 99 is an invalid exit from construct starting
    // at block 30; branch bypasses merge block 49
    p->DeliberatelyInvalidSpirv();
}

TEST_F(SpvParserCFGTest, ComputeBlockOrder_Loop_Body_Switch_CaseContinues) {
    auto assembly = CommonTypes() + R"(
     %100 = OpFunction %void None %voidfn

     %10 = OpLabel
     OpBranch %20

     %20 = OpLabel
     OpLoopMerge %99 %50 None
     OpBranchConditional %cond %30 %99

     %30 = OpLabel
     OpSelectionMerge %49 None
     OpSwitch %selector %49 40 %40 45 %45

     %40 = OpLabel
     OpBranch %50   ; continue bypasses switch merge

     %45 = OpLabel
     OpBranch %49

     %49 = OpLabel
     OpBranch %50

     %50 = OpLabel
     OpBranch %20

     %99 = OpLabel
     OpReturn

     OpFunctionEnd
  )";
    auto p = parser(test::Assemble(assembly));
    ASSERT_TRUE(p->BuildAndParseInternalModuleExceptFunctions()) << p->error();
    auto fe = p->function_emitter(100);
    fe.RegisterBasicBlocks();
    fe.ComputeBlockOrderAndPositions();

    EXPECT_THAT(fe.block_order(), ElementsAre(10, 20, 30, 45, 40, 49, 50, 99)) << assembly;
}

TEST_F(SpvParserCFGTest, ComputeBlockOrder_Loop_BodyHasSwitchContinueBreak) {
    auto assembly = CommonTypes() + R"(
     %100 = OpFunction %void None %voidfn

     %10 = OpLabel
     OpBranch %20

     %20 = OpLabel
     OpLoopMerge %99 %50 None
     OpBranchConditional %cond %30 %99

     %30 = OpLabel
     ; OpSwitch must be preceded by a selection merge
     OpSwitch %selector %99 50 %50 ; default is break, 50 is continue

     %50 = OpLabel
     OpBranch %20

     %99 = OpLabel
     OpReturn

     OpFunctionEnd
  )";
    auto p = parser(test::Assemble(assembly));
    EXPECT_FALSE(p->Parse());
    EXPECT_FALSE(p->success());
    EXPECT_THAT(p->error(), HasSubstr("OpSwitch must be preceded by an OpSelectionMerge"));
}

TEST_F(SpvParserCFGTest, ComputeBlockOrder_Loop_Continue_Sequence) {
    auto assembly = CommonTypes() + R"(
     %100 = OpFunction %void None %voidfn

     %10 = OpLabel
     OpBranch %20

     %20 = OpLabel
     OpLoopMerge %99 %50 None
     OpBranchConditional %cond %30 %99

     %30 = OpLabel
     OpBranch %50

     %50 = OpLabel
     OpBranch %60

     %60 = OpLabel
     OpBranch %20

     %99 = OpLabel
     OpReturn

     OpFunctionEnd
  )";
    auto p = parser(test::Assemble(assembly));
    ASSERT_TRUE(p->BuildAndParseInternalModuleExceptFunctions()) << p->error();
    auto fe = p->function_emitter(100);
    fe.RegisterBasicBlocks();
    fe.ComputeBlockOrderAndPositions();

    EXPECT_THAT(fe.block_order(), ElementsAre(10, 20, 30, 50, 60, 99));
}

TEST_F(SpvParserCFGTest, ComputeBlockOrder_Loop_Continue_ContainsIf) {
    auto assembly = CommonTypes() + R"(
     %100 = OpFunction %void None %voidfn

     %10 = OpLabel
     OpBranch %20

     %20 = OpLabel
     OpLoopMerge %99 %50 None
     OpBranchConditional %cond %30 %99

     %30 = OpLabel
     OpBranch %50

     %50 = OpLabel
     OpSelectionMerge %89 None
     OpBranchConditional %cond2 %60 %70

     %89 = OpLabel
     OpBranch %20 ; backedge

     %60 = OpLabel
     OpBranch %89

     %70 = OpLabel
     OpBranch %89

     %99 = OpLabel
     OpReturn

     OpFunctionEnd
  )";
    auto p = parser(test::Assemble(assembly));
    ASSERT_TRUE(p->BuildAndParseInternalModuleExceptFunctions()) << p->error();
    auto fe = p->function_emitter(100);
    fe.RegisterBasicBlocks();
    fe.ComputeBlockOrderAndPositions();

    EXPECT_THAT(fe.block_order(), ElementsAre(10, 20, 30, 50, 60, 70, 89, 99));
}

TEST_F(SpvParserCFGTest, ComputeBlockOrder_Loop_Continue_HasBreakIf) {
    auto assembly = CommonTypes() + R"(
     %100 = OpFunction %void None %voidfn

     %10 = OpLabel
     OpBranch %20

     %20 = OpLabel
     OpLoopMerge %99 %50 None
     OpBranchConditional %cond %30 %99

     %30 = OpLabel
     OpBranch %50

     %50 = OpLabel
     OpBranchConditional %cond2 %99 %20

     %99 = OpLabel
     OpReturn

     OpFunctionEnd
  )";
    auto p = parser(test::Assemble(assembly));
    ASSERT_TRUE(p->BuildAndParseInternalModuleExceptFunctions()) << p->error();
    auto fe = p->function_emitter(100);
    fe.RegisterBasicBlocks();
    fe.ComputeBlockOrderAndPositions();

    EXPECT_THAT(fe.block_order(), ElementsAre(10, 20, 30, 50, 99));
}

TEST_F(SpvParserCFGTest, ComputeBlockOrder_Loop_Continue_HasBreakUnless) {
    auto assembly = CommonTypes() + R"(
     %100 = OpFunction %void None %voidfn

     %10 = OpLabel
     OpBranch %20

     %20 = OpLabel
     OpLoopMerge %99 %50 None
     OpBranchConditional %cond %30 %99

     %30 = OpLabel
     OpBranch %50

     %50 = OpLabel
     OpBranchConditional %cond2 %20 %99

     %99 = OpLabel
     OpReturn

     OpFunctionEnd
  )";
    auto p = parser(test::Assemble(assembly));
    ASSERT_TRUE(p->BuildAndParseInternalModuleExceptFunctions()) << p->error();
    auto fe = p->function_emitter(100);
    fe.RegisterBasicBlocks();
    fe.ComputeBlockOrderAndPositions();

    EXPECT_THAT(fe.block_order(), ElementsAre(10, 20, 30, 50, 99));
}

TEST_F(SpvParserCFGTest, ComputeBlockOrder_Loop_Continue_SwitchBreak) {
    auto assembly = CommonTypes() + R"(
     %100 = OpFunction %void None %voidfn

     %10 = OpLabel
     OpBranch %20

     %20 = OpLabel
     OpLoopMerge %99 %50 None
     OpBranchConditional %cond %30 %99

     %30 = OpLabel
     OpBranch %50

     %50 = OpLabel
     ; Updated SPIR-V rule:
     ; OpSwitch must be preceded by a selection.
     OpSwitch %selector %20 99 %99

     %99 = OpLabel
     OpReturn

     OpFunctionEnd
  )";
    auto p = parser(test::Assemble(assembly));
    EXPECT_FALSE(p->Parse());
    EXPECT_FALSE(p->success());
    EXPECT_THAT(p->error(), HasSubstr("OpSwitch must be preceded by an OpSelectionMerge"));
}

TEST_F(SpvParserCFGTest, ComputeBlockOrder_Loop_Loop) {
    auto assembly = CommonTypes() + R"(
     %100 = OpFunction %void None %voidfn

     %10 = OpLabel
     OpBranch %20

     %20 = OpLabel
     OpLoopMerge %99 %50 None
     OpBranchConditional %cond %30 %99

     %30 = OpLabel
     OpLoopMerge %49 %40 None
     OpBranchConditional %cond2 %35 %49

     %35 = OpLabel
     OpBranch %37

     %37 = OpLabel
     OpBranch %40

     %40 = OpLabel ; inner loop's continue
     OpBranch %30 ; backedge

     %49 = OpLabel ; inner loop's merge
     OpBranch %50

     %50 = OpLabel ; outer loop's continue
     OpBranch %20 ; outer loop's backege

     %99 = OpLabel
     OpReturn

     OpFunctionEnd
  )";
    auto p = parser(test::Assemble(assembly));
    ASSERT_TRUE(p->BuildAndParseInternalModuleExceptFunctions()) << p->error();
    auto fe = p->function_emitter(100);
    fe.RegisterBasicBlocks();
    fe.ComputeBlockOrderAndPositions();

    EXPECT_THAT(fe.block_order(), ElementsAre(10, 20, 30, 35, 37, 40, 49, 50, 99));
}

TEST_F(SpvParserCFGTest, ComputeBlockOrder_Loop_Loop_InnerBreak) {
    auto assembly = CommonTypes() + R"(
     %100 = OpFunction %void None %voidfn

     %10 = OpLabel
     OpBranch %20

     %20 = OpLabel
     OpLoopMerge %99 %50 None
     OpBranchConditional %cond %30 %99

     %30 = OpLabel
     OpLoopMerge %49 %40 None
     OpBranchConditional %cond2 %35 %49

     %35 = OpLabel
     OpBranchConditional %cond3 %49 %37 ; break to inner merge

     %37 = OpLabel
     OpBranch %40

     %40 = OpLabel ; inner loop's continue
     OpBranch %30 ; backedge

     %49 = OpLabel ; inner loop's merge
     OpBranch %50

     %50 = OpLabel ; outer loop's continue
     OpBranch %20 ; outer loop's backege

     %99 = OpLabel
     OpReturn

     OpFunctionEnd
  )";
    auto p = parser(test::Assemble(assembly));
    ASSERT_TRUE(p->BuildAndParseInternalModuleExceptFunctions()) << p->error();
    auto fe = p->function_emitter(100);
    fe.RegisterBasicBlocks();
    fe.ComputeBlockOrderAndPositions();

    EXPECT_THAT(fe.block_order(), ElementsAre(10, 20, 30, 35, 37, 40, 49, 50, 99));
}

TEST_F(SpvParserCFGTest, ComputeBlockOrder_Loop_Loop_InnerContinue) {
    auto assembly = CommonTypes() + R"(
     %100 = OpFunction %void None %voidfn

     %10 = OpLabel
     OpBranch %20

     %20 = OpLabel
     OpLoopMerge %99 %50 None
     OpBranchConditional %cond %30 %99

     %30 = OpLabel
     OpLoopMerge %49 %40 None
     OpBranchConditional %cond2 %35 %49

     %35 = OpLabel
     OpBranchConditional %cond3 %37 %49 ; continue to inner continue target

     %37 = OpLabel
     OpBranch %40

     %40 = OpLabel ; inner loop's continue
     OpBranch %30 ; backedge

     %49 = OpLabel ; inner loop's merge
     OpBranch %50

     %50 = OpLabel ; outer loop's continue
     OpBranch %20 ; outer loop's backege

     %99 = OpLabel
     OpReturn

     OpFunctionEnd
  )";
    auto p = parser(test::Assemble(assembly));
    ASSERT_TRUE(p->BuildAndParseInternalModuleExceptFunctions()) << p->error();
    auto fe = p->function_emitter(100);
    fe.RegisterBasicBlocks();
    fe.ComputeBlockOrderAndPositions();

    EXPECT_THAT(fe.block_order(), ElementsAre(10, 20, 30, 35, 37, 40, 49, 50, 99));
}

TEST_F(SpvParserCFGTest, ComputeBlockOrder_Loop_Loop_InnerContinueBreaks) {
    auto assembly = CommonTypes() + R"(
     %100 = OpFunction %void None %voidfn

     %10 = OpLabel
     OpBranch %20

     %20 = OpLabel
     OpLoopMerge %99 %50 None
     OpBranchConditional %cond %30 %99

     %30 = OpLabel
     OpLoopMerge %49 %40 None
     OpBranchConditional %cond2 %35 %49

     %35 = OpLabel
     OpBranch %37

     %37 = OpLabel
     OpBranch %40

     %40 = OpLabel ; inner loop's continue
     OpBranchConditional %cond3 %30 %49 ; backedge and inner break

     %49 = OpLabel ; inner loop's merge
     OpBranch %50

     %50 = OpLabel ; outer loop's continue
     OpBranch %20 ; outer loop's backege

     %99 = OpLabel
     OpReturn

     OpFunctionEnd
  )";
    auto p = parser(test::Assemble(assembly));
    ASSERT_TRUE(p->BuildAndParseInternalModuleExceptFunctions()) << p->error();
    auto fe = p->function_emitter(100);
    fe.RegisterBasicBlocks();
    fe.ComputeBlockOrderAndPositions();

    EXPECT_THAT(fe.block_order(), ElementsAre(10, 20, 30, 35, 37, 40, 49, 50, 99));
}

TEST_F(SpvParserCFGTest, ComputeBlockOrder_Loop_Loop_InnerContinueContinues) {
    auto assembly = CommonTypes() + R"(
     %100 = OpFunction %void None %voidfn

     %10 = OpLabel
     OpBranch %20

     %20 = OpLabel
     OpLoopMerge %99 %50 None
     OpBranchConditional %cond %30 %99

     %30 = OpLabel
     OpLoopMerge %49 %40 None
     OpBranchConditional %cond2 %35 %49

     %35 = OpLabel
     OpBranch %37

     %37 = OpLabel
     OpBranch %40

     %40 = OpLabel ; inner loop's continue
     OpBranchConditional %cond3 %30 %50 ; backedge and continue to outer

     %49 = OpLabel ; inner loop's merge
     OpBranch %50

     %50 = OpLabel ; outer loop's continue
     OpBranch %20 ; outer loop's backege

     %99 = OpLabel
     OpReturn

     OpFunctionEnd
  )";
    auto p = parser(test::Assemble(assembly));
    ASSERT_TRUE(p->BuildAndParseInternalModuleExceptFunctions()) << p->error();
    auto fe = p->function_emitter(100);
    fe.RegisterBasicBlocks();
    fe.ComputeBlockOrderAndPositions();

    EXPECT_THAT(fe.block_order(), ElementsAre(10, 20, 30, 35, 37, 40, 49, 50, 99));

    p->DeliberatelyInvalidSpirv();
    // SPIR-V validation fails:
    //    block <ID> 40[%40] exits the continue headed by <ID> 40[%40], but not
    //    via a structured exit"
}

TEST_F(SpvParserCFGTest, ComputeBlockOrder_Loop_Loop_SwitchBackedgeBreakContinue) {
    auto assembly = CommonTypes() + R"(
     %100 = OpFunction %void None %voidfn

     %10 = OpLabel
     OpBranch %20

     %20 = OpLabel
     OpLoopMerge %99 %50 None
     OpBranchConditional %cond %30 %99

     %30 = OpLabel
     OpLoopMerge %49 %40 None
     OpBranchConditional %cond2 %35 %49

     %35 = OpLabel
     OpBranch %37

     %37 = OpLabel
     OpBranch %40

     %40 = OpLabel ; inner loop's continue
     ; This switch does triple duty:
     ; default -> backedge
     ; 49 -> loop break
     ; 49 -> inner loop break
     ; 50 -> outer loop continue
     OpSwitch %selector %30 49 %49 50 %50

     %49 = OpLabel ; inner loop's merge
     OpBranch %50

     %50 = OpLabel ; outer loop's continue
     OpBranch %20 ; outer loop's backege

     %99 = OpLabel
     OpReturn

     OpFunctionEnd
  )";
    auto p = parser(test::Assemble(assembly));
    ASSERT_TRUE(p->BuildAndParseInternalModuleExceptFunctions()) << p->error();
    auto fe = p->function_emitter(100);
    fe.RegisterBasicBlocks();
    fe.ComputeBlockOrderAndPositions();

    EXPECT_THAT(fe.block_order(), ElementsAre(10, 20, 30, 35, 37, 40, 49, 50, 99));

    p->DeliberatelyInvalidSpirv();
    // SPIR-V validation fails:
    //    block <ID> 40[%40] exits the continue headed by <ID> 40[%40], but not
    //    via a structured exit"
}

TEST_F(SpvParserCFGTest, VerifyHeaderContinueMergeOrder_Selection_Good) {
    auto assembly = CommonTypes() + R"(
     %100 = OpFunction %void None %voidfn

     %10 = OpLabel
     OpSelectionMerge %99 None
     OpBranchConditional %cond %20 %30

     %20 = OpLabel
     OpBranch %99

     %30 = OpLabel
     OpBranch %99

     %99 = OpLabel
     OpReturn

     OpFunctionEnd
)";
    auto p = parser(test::Assemble(assembly));
    ASSERT_TRUE(p->BuildAndParseInternalModuleExceptFunctions()) << p->error();
    auto fe = p->function_emitter(100);
    fe.RegisterBasicBlocks();
    fe.ComputeBlockOrderAndPositions();
    fe.RegisterMerges();
    EXPECT_TRUE(fe.VerifyHeaderContinueMergeOrder());
}

TEST_F(SpvParserCFGTest, VerifyHeaderContinueMergeOrder_SingleBlockLoop_Good) {
    auto assembly = CommonTypes() + R"(
     %100 = OpFunction %void None %voidfn

     %10 = OpLabel
     OpBranch %20

     %20 = OpLabel
     OpLoopMerge %99 %20 None
     OpBranchConditional %cond %20 %99

     %99 = OpLabel
     OpReturn

     OpFunctionEnd
)";
    auto p = parser(test::Assemble(assembly));
    ASSERT_TRUE(p->BuildAndParseInternalModuleExceptFunctions()) << p->error();
    auto fe = p->function_emitter(100);
    fe.RegisterBasicBlocks();
    fe.ComputeBlockOrderAndPositions();
    fe.RegisterMerges();
    EXPECT_TRUE(fe.VerifyHeaderContinueMergeOrder()) << p->error();
}

TEST_F(SpvParserCFGTest, VerifyHeaderContinueMergeOrder_MultiBlockLoop_Good) {
    auto assembly = CommonTypes() + R"(
     %100 = OpFunction %void None %voidfn

     %10 = OpLabel
     OpBranch %20

     %20 = OpLabel
     OpLoopMerge %99 %30 None
     OpBranchConditional %cond %30 %99

     %30 = OpLabel
     OpBranch %20

     %99 = OpLabel
     OpReturn

     OpFunctionEnd
)";
    auto p = parser(test::Assemble(assembly));
    ASSERT_TRUE(p->BuildAndParseInternalModuleExceptFunctions()) << p->error();
    auto fe = p->function_emitter(100);
    fe.RegisterBasicBlocks();
    fe.ComputeBlockOrderAndPositions();
    fe.RegisterMerges();
    EXPECT_TRUE(fe.VerifyHeaderContinueMergeOrder());
}

TEST_F(SpvParserCFGTest, VerifyHeaderContinueMergeOrder_HeaderDoesNotStrictlyDominateMerge) {
    auto assembly = CommonTypes() + R"(
     %100 = OpFunction %void None %voidfn

     %10 = OpLabel
     OpBranch %20

     %20 = OpLabel
     OpBranch %50

     %50 = OpLabel
     OpSelectionMerge %20 None ; this is backward
     OpBranchConditional %cond2 %60 %99

     %60 = OpLabel
     OpBranch %99

     %99 = OpLabel
     OpReturn

     OpFunctionEnd
)";
    auto p = parser(test::Assemble(assembly));
    ASSERT_TRUE(p->BuildAndParseInternalModuleExceptFunctions()) << p->error();
    auto fe = p->function_emitter(100);
    fe.RegisterBasicBlocks();
    fe.ComputeBlockOrderAndPositions();
    fe.RegisterMerges();
    EXPECT_FALSE(fe.VerifyHeaderContinueMergeOrder());

    tint::StringStream result;
    result << *fe.GetBlockInfo(50) << "\n" << *fe.GetBlockInfo(20) << "\n";
    EXPECT_THAT(p->error(), Eq("Header 50 does not strictly dominate its merge block 20"))
        << result.str() << Dump(fe.block_order());
}

TEST_F(SpvParserCFGTest,
       VerifyHeaderContinueMergeOrder_HeaderDoesNotStrictlyDominateContinueTarget) {
    auto assembly = CommonTypes() + R"(
     %100 = OpFunction %void None %voidfn

     %10 = OpLabel
     OpBranch %20

     %20 = OpLabel
     OpBranch %50

     %50 = OpLabel
     OpLoopMerge %99 %20 None ; this is backward
     OpBranchConditional %cond %60 %99

     %60 = OpLabel
     OpBranch %50

     %99 = OpLabel
     OpReturn

     OpFunctionEnd
)";
    auto p = parser(test::Assemble(assembly));
    ASSERT_TRUE(p->BuildAndParseInternalModuleExceptFunctions()) << p->error();
    auto fe = p->function_emitter(100);
    fe.RegisterBasicBlocks();
    fe.ComputeBlockOrderAndPositions();
    fe.RegisterMerges();
    EXPECT_FALSE(fe.VerifyHeaderContinueMergeOrder());
    tint::StringStream str;
    str << *fe.GetBlockInfo(50) << "\n" << *fe.GetBlockInfo(20) << "\n";
    EXPECT_THAT(p->error(), Eq("Loop header 50 does not dominate its continue target 20"))
        << str.str() << Dump(fe.block_order());
}

TEST_F(SpvParserCFGTest, VerifyHeaderContinueMergeOrder_MergeInsideContinueTarget) {
    auto assembly = CommonTypes() + R"(
     %100 = OpFunction %void None %voidfn

     %10 = OpLabel
     OpBranch %50

     %50 = OpLabel
     OpLoopMerge %60 %70 None
     OpBranchConditional %cond %60 %99

     %60 = OpLabel
     OpBranch %70

     %70 = OpLabel
     OpBranch %50

     %99 = OpLabel
     OpReturn

     OpFunctionEnd
)";
    auto p = parser(test::Assemble(assembly));
    ASSERT_TRUE(p->BuildAndParseInternalModuleExceptFunctions()) << p->error();
    auto fe = p->function_emitter(100);
    fe.RegisterBasicBlocks();
    fe.ComputeBlockOrderAndPositions();
    fe.RegisterMerges();
    EXPECT_FALSE(fe.VerifyHeaderContinueMergeOrder());
    EXPECT_THAT(p->error(), Eq("Merge block 60 for loop headed at block 50 appears at or "
                               "before the loop's continue construct headed by block 70"))
        << Dump(fe.block_order());
}

TEST_F(SpvParserCFGTest, LabelControlFlowConstructs_OuterConstructIsFunction_SingleBlock) {
    auto assembly = CommonTypes() + R"(
     %100 = OpFunction %void None %voidfn

     %10 = OpLabel
     OpReturn

     OpFunctionEnd
)";
    auto p = parser(test::Assemble(assembly));
    ASSERT_TRUE(p->BuildAndParseInternalModuleExceptFunctions()) << p->error();
    auto fe = p->function_emitter(100);
    fe.RegisterBasicBlocks();
    fe.ComputeBlockOrderAndPositions();
    fe.RegisterMerges();
    EXPECT_TRUE(fe.LabelControlFlowConstructs());
    EXPECT_EQ(fe.constructs().Length(), 1u);
    auto& c = fe.constructs().Front();
    EXPECT_THAT(ToString(c), Eq("Construct{ Function [0,1) begin_id:10 end_id:0 "
                                "depth:0 parent:null }"));
    EXPECT_EQ(fe.GetBlockInfo(10)->construct, c.get());
}

TEST_F(SpvParserCFGTest, LabelControlFlowConstructs_OuterConstructIsFunction_MultiBlock) {
    auto assembly = CommonTypes() + R"(
     %100 = OpFunction %void None %voidfn

     %10 = OpLabel
     OpBranch %5

     %5 = OpLabel
     OpReturn

     OpFunctionEnd
)";
    auto p = parser(test::Assemble(assembly));
    ASSERT_TRUE(p->BuildAndParseInternalModuleExceptFunctions()) << p->error();
    auto fe = p->function_emitter(100);
    fe.RegisterBasicBlocks();
    fe.ComputeBlockOrderAndPositions();
    fe.RegisterMerges();
    EXPECT_TRUE(fe.LabelControlFlowConstructs());
    EXPECT_EQ(fe.constructs().Length(), 1u);
    auto& c = fe.constructs().Front();
    EXPECT_THAT(ToString(c), Eq("Construct{ Function [0,2) begin_id:10 end_id:0 "
                                "depth:0 parent:null }"));
    EXPECT_EQ(fe.GetBlockInfo(10)->construct, c.get());
    EXPECT_EQ(fe.GetBlockInfo(5)->construct, c.get());
}

TEST_F(SpvParserCFGTest, LabelControlFlowConstructs_FunctionIsOnlyIfSelectionAndItsMerge) {
    auto assembly = CommonTypes() + R"(
     %100 = OpFunction %void None %voidfn

     %10 = OpLabel
     OpSelectionMerge %99 None
     OpBranchConditional %cond %20 %30

     %20 = OpLabel
     OpBranch %99

     %30 = OpLabel
     OpBranch %99

     %99 = OpLabel
     OpReturn

     OpFunctionEnd
)";
    auto p = parser(test::Assemble(assembly));
    ASSERT_TRUE(p->BuildAndParseInternalModuleExceptFunctions()) << p->error();
    auto fe = p->function_emitter(100);
    fe.RegisterBasicBlocks();
    fe.ComputeBlockOrderAndPositions();
    fe.RegisterMerges();
    EXPECT_TRUE(fe.LabelControlFlowConstructs());
    const auto& constructs = fe.constructs();
    EXPECT_EQ(constructs.Length(), 2u);

    tint::StringStream str;
    str << constructs;

    EXPECT_THAT(ToString(constructs), Eq(R"(ConstructList{
  Construct{ Function [0,4) begin_id:10 end_id:0 depth:0 parent:null }
  Construct{ IfSelection [0,3) begin_id:10 end_id:99 depth:1 parent:Function@10 }
})")) << str.str();

    // The block records the nearest enclosing construct.
    EXPECT_EQ(fe.GetBlockInfo(10)->construct, constructs[1].get());
    EXPECT_EQ(fe.GetBlockInfo(20)->construct, constructs[1].get());
    EXPECT_EQ(fe.GetBlockInfo(30)->construct, constructs[1].get());
    EXPECT_EQ(fe.GetBlockInfo(99)->construct, constructs[0].get());
}

TEST_F(SpvParserCFGTest,
       LabelControlFlowConstructs_PaddingBlocksBeforeAndAfterStructuredConstruct) {
    auto assembly = CommonTypes() + R"(
     %100 = OpFunction %void None %voidfn

     %5 = OpLabel
     OpBranch %10

     %10 = OpLabel
     OpSelectionMerge %99 None
     OpBranchConditional %cond %20 %30

     %20 = OpLabel
     OpBranch %99

     %30 = OpLabel
     OpBranch %99

     %99 = OpLabel
     OpBranch %200

     %200 = OpLabel
     OpReturn

     OpFunctionEnd
)";
    auto p = parser(test::Assemble(assembly));
    ASSERT_TRUE(p->BuildAndParseInternalModuleExceptFunctions()) << p->error();
    auto fe = p->function_emitter(100);
    fe.RegisterBasicBlocks();
    fe.ComputeBlockOrderAndPositions();
    fe.RegisterMerges();
    EXPECT_TRUE(fe.LabelControlFlowConstructs());
    const auto& constructs = fe.constructs();
    EXPECT_EQ(constructs.Length(), 2u);

    tint::StringStream str;
    str << constructs;

    EXPECT_THAT(ToString(constructs), Eq(R"(ConstructList{
  Construct{ Function [0,6) begin_id:5 end_id:0 depth:0 parent:null }
  Construct{ IfSelection [1,4) begin_id:10 end_id:99 depth:1 parent:Function@5 }
})")) << str.str();

    // The block records the nearest enclosing construct.
    EXPECT_EQ(fe.GetBlockInfo(5)->construct, constructs[0].get());
    EXPECT_EQ(fe.GetBlockInfo(10)->construct, constructs[1].get());
    EXPECT_EQ(fe.GetBlockInfo(20)->construct, constructs[1].get());
    EXPECT_EQ(fe.GetBlockInfo(30)->construct, constructs[1].get());
    EXPECT_EQ(fe.GetBlockInfo(99)->construct, constructs[0].get());
    EXPECT_EQ(fe.GetBlockInfo(200)->construct, constructs[0].get());
}

TEST_F(SpvParserCFGTest, LabelControlFlowConstructs_SwitchSelection) {
    auto assembly = CommonTypes() + R"(
     %100 = OpFunction %void None %voidfn

     %10 = OpLabel
     OpSelectionMerge %99 None
     OpSwitch %selector %40 20 %20 30 %30

     %20 = OpLabel
     OpBranch %99

     %30 = OpLabel
     OpBranch %99

     %40 = OpLabel
     OpBranch %99

     %99 = OpLabel
     OpReturn

     OpFunctionEnd
)";
    auto p = parser(test::Assemble(assembly));
    ASSERT_TRUE(p->BuildAndParseInternalModuleExceptFunctions()) << p->error();
    auto fe = p->function_emitter(100);
    fe.RegisterBasicBlocks();
    fe.ComputeBlockOrderAndPositions();
    fe.RegisterMerges();
    EXPECT_TRUE(fe.LabelControlFlowConstructs());
    const auto& constructs = fe.constructs();
    EXPECT_EQ(constructs.Length(), 2u);

    tint::StringStream str;
    str << constructs;

    EXPECT_THAT(ToString(constructs), Eq(R"(ConstructList{
  Construct{ Function [0,5) begin_id:10 end_id:0 depth:0 parent:null }
  Construct{ SwitchSelection [0,4) begin_id:10 end_id:99 depth:1 parent:Function@10 in-c-l-s:SwitchSelection@10 }
})")) << str.str();

    // The block records the nearest enclosing construct.
    EXPECT_EQ(fe.GetBlockInfo(10)->construct, constructs[1].get());
    EXPECT_EQ(fe.GetBlockInfo(20)->construct, constructs[1].get());
    EXPECT_EQ(fe.GetBlockInfo(30)->construct, constructs[1].get());
    EXPECT_EQ(fe.GetBlockInfo(40)->construct, constructs[1].get());
    EXPECT_EQ(fe.GetBlockInfo(99)->construct, constructs[0].get());
}

TEST_F(SpvParserCFGTest, LabelControlFlowConstructs_SingleBlockLoop) {
    auto assembly = CommonTypes() + R"(
     %100 = OpFunction %void None %voidfn

     %10 = OpLabel
     OpBranch %20

     %20 = OpLabel
     OpLoopMerge %99 %20 None
     OpBranchConditional %cond %20 %99

     %99 = OpLabel
     OpReturn

     OpFunctionEnd
)";
    auto p = parser(test::Assemble(assembly));
    ASSERT_TRUE(p->BuildAndParseInternalModuleExceptFunctions()) << p->error();
    auto fe = p->function_emitter(100);
    fe.RegisterBasicBlocks();
    fe.ComputeBlockOrderAndPositions();
    fe.RegisterMerges();
    EXPECT_TRUE(fe.LabelControlFlowConstructs());
    const auto& constructs = fe.constructs();
    EXPECT_EQ(constructs.Length(), 2u);

    tint::StringStream str;
    str << constructs;

    // A single-block loop consists *only* of a continue target with one block in
    // it.
    EXPECT_THAT(ToString(constructs), Eq(R"(ConstructList{
  Construct{ Function [0,3) begin_id:10 end_id:0 depth:0 parent:null }
  Construct{ Continue [1,2) begin_id:20 end_id:99 depth:1 parent:Function@10 in-c:Continue@20 }
})")) << str.str();

    // The block records the nearest enclosing construct.
    EXPECT_EQ(fe.GetBlockInfo(10)->construct, constructs[0].get());
    EXPECT_EQ(fe.GetBlockInfo(20)->construct, constructs[1].get());
    EXPECT_EQ(fe.GetBlockInfo(99)->construct, constructs[0].get());
}

TEST_F(SpvParserCFGTest, LabelControlFlowConstructs_MultiBlockLoop_HeaderIsNotContinue) {
    // In this case, we have a continue construct and a non-empty loop construct.
    auto assembly = CommonTypes() + R"(
     %100 = OpFunction %void None %voidfn

     %10 = OpLabel
     OpBranch %20

     %20 = OpLabel
     OpLoopMerge %99 %40 None
     OpBranchConditional %cond %30 %99

     %30 = OpLabel
     OpBranch %40

     %40 = OpLabel
     OpBranch %50

     %50 = OpLabel
     OpBranch %20

     %99 = OpLabel
     OpReturn

     OpFunctionEnd
)";
    auto p = parser(test::Assemble(assembly));
    ASSERT_TRUE(p->BuildAndParseInternalModuleExceptFunctions()) << p->error();
    auto fe = p->function_emitter(100);
    fe.RegisterBasicBlocks();
    fe.ComputeBlockOrderAndPositions();
    fe.RegisterMerges();
    EXPECT_TRUE(fe.LabelControlFlowConstructs());
    const auto& constructs = fe.constructs();

    tint::StringStream str;
    str << constructs;

    EXPECT_THAT(ToString(constructs), Eq(R"(ConstructList{
  Construct{ Function [0,6) begin_id:10 end_id:0 depth:0 parent:null }
  Construct{ Continue [3,5) begin_id:40 end_id:99 depth:1 parent:Function@10 in-c:Continue@40 }
  Construct{ Loop [1,3) begin_id:20 end_id:40 depth:1 parent:Function@10 scope:[1,5) in-l:Loop@20 }
})")) << str.str();

    // The block records the nearest enclosing construct.
    EXPECT_EQ(fe.GetBlockInfo(10)->construct, constructs[0].get());
    EXPECT_EQ(fe.GetBlockInfo(20)->construct, constructs[2].get());
    EXPECT_EQ(fe.GetBlockInfo(30)->construct, constructs[2].get());
    EXPECT_EQ(fe.GetBlockInfo(40)->construct, constructs[1].get());
    EXPECT_EQ(fe.GetBlockInfo(50)->construct, constructs[1].get());
    EXPECT_EQ(fe.GetBlockInfo(99)->construct, constructs[0].get());
}

TEST_F(SpvParserCFGTest, LabelControlFlowConstructs_MultiBlockLoop_HeaderIsContinue) {
    // In this case, we have only a continue construct and no loop construct.
    auto assembly = CommonTypes() + R"(
     %100 = OpFunction %void None %voidfn

     %10 = OpLabel
     OpBranch %20

     %20 = OpLabel
     OpLoopMerge %99 %20 None
     OpBranch %30

     %30 = OpLabel
     OpBranch %40

     %40 = OpLabel
     OpBranch %50

     %50 = OpLabel
     OpBranchConditional %cond %20 %99

     %99 = OpLabel
     OpReturn

     OpFunctionEnd
)";
    auto p = parser(test::Assemble(assembly));
    ASSERT_TRUE(p->BuildAndParseInternalModuleExceptFunctions()) << p->error();
    auto fe = p->function_emitter(100);
    fe.RegisterBasicBlocks();
    fe.ComputeBlockOrderAndPositions();
    fe.RegisterMerges();
    EXPECT_TRUE(fe.LabelControlFlowConstructs());
    const auto& constructs = fe.constructs();

    tint::StringStream str;
    str << constructs;

    EXPECT_THAT(ToString(constructs), Eq(R"(ConstructList{
  Construct{ Function [0,6) begin_id:10 end_id:0 depth:0 parent:null }
  Construct{ Continue [1,5) begin_id:20 end_id:99 depth:1 parent:Function@10 in-c:Continue@20 }
})")) << str.str();
    // The block records the nearest enclosing construct.
    EXPECT_EQ(fe.GetBlockInfo(10)->construct, constructs[0].get());
    EXPECT_EQ(fe.GetBlockInfo(20)->construct, constructs[1].get());
    EXPECT_EQ(fe.GetBlockInfo(30)->construct, constructs[1].get());
    EXPECT_EQ(fe.GetBlockInfo(40)->construct, constructs[1].get());
    EXPECT_EQ(fe.GetBlockInfo(50)->construct, constructs[1].get());
    EXPECT_EQ(fe.GetBlockInfo(99)->construct, constructs[0].get());

    // SPIR-V 1.6 Rev 2 made this invalid SPIR-V.
    p->DeliberatelyInvalidSpirv();
}

TEST_F(SpvParserCFGTest, LabelControlFlowConstructs_MergeBlockIsAlsoSingleBlockLoop) {
    auto assembly = CommonTypes() + R"(
     %100 = OpFunction %void None %voidfn

     %10 = OpLabel
     OpSelectionMerge %50 None
     OpBranchConditional %cond %20 %50

     %20 = OpLabel
     OpBranch %50

     ; %50 is the merge block for the selection starting at 10,
     ; and its own continue target.
     %50 = OpLabel
     OpLoopMerge %99 %50 None
     OpBranchConditional %cond %50 %99

     %99 = OpLabel
     OpReturn

     OpFunctionEnd
)";
    auto p = parser(test::Assemble(assembly));
    ASSERT_TRUE(p->BuildAndParseInternalModuleExceptFunctions()) << p->error();
    auto fe = p->function_emitter(100);
    fe.RegisterBasicBlocks();
    fe.ComputeBlockOrderAndPositions();
    fe.RegisterMerges();
    EXPECT_TRUE(fe.LabelControlFlowConstructs());
    const auto& constructs = fe.constructs();
    EXPECT_EQ(constructs.Length(), 3u);

    tint::StringStream str;
    str << constructs;

    // A single-block loop consists *only* of a continue target with one block in
    // it.
    EXPECT_THAT(ToString(constructs), Eq(R"(ConstructList{
  Construct{ Function [0,4) begin_id:10 end_id:0 depth:0 parent:null }
  Construct{ IfSelection [0,2) begin_id:10 end_id:50 depth:1 parent:Function@10 }
  Construct{ Continue [2,3) begin_id:50 end_id:99 depth:1 parent:Function@10 in-c:Continue@50 }
})")) << str.str();

    // The block records the nearest enclosing construct.
    EXPECT_EQ(fe.GetBlockInfo(10)->construct, constructs[1].get());
    EXPECT_EQ(fe.GetBlockInfo(20)->construct, constructs[1].get());
    EXPECT_EQ(fe.GetBlockInfo(50)->construct, constructs[2].get());
    EXPECT_EQ(fe.GetBlockInfo(99)->construct, constructs[0].get());
}

TEST_F(SpvParserCFGTest, LabelControlFlowConstructs_MergeBlockIsAlsoMultiBlockLoopHeader) {
    auto assembly = CommonTypes() + R"(
     %100 = OpFunction %void None %voidfn

     %10 = OpLabel
     OpSelectionMerge %50 None
     OpBranchConditional %cond %20 %50

     %20 = OpLabel
     OpBranch %50

     ; %50 is the merge block for the selection starting at 10,
     ; and a loop block header but not its own continue target.
     %50 = OpLabel
     OpLoopMerge %99 %60 None
     OpBranchConditional %cond %60 %99

     %60 = OpLabel
     OpBranch %50

     %99 = OpLabel
     OpReturn

     OpFunctionEnd
)";
    auto p = parser(test::Assemble(assembly));
    ASSERT_TRUE(p->BuildAndParseInternalModuleExceptFunctions()) << p->error();
    auto fe = p->function_emitter(100);
    fe.RegisterBasicBlocks();
    fe.ComputeBlockOrderAndPositions();
    fe.RegisterMerges();
    EXPECT_TRUE(fe.LabelControlFlowConstructs());
    const auto& constructs = fe.constructs();
    EXPECT_EQ(constructs.Length(), 4u);

    tint::StringStream str;
    str << constructs;

    EXPECT_THAT(ToString(constructs), Eq(R"(ConstructList{
  Construct{ Function [0,5) begin_id:10 end_id:0 depth:0 parent:null }
  Construct{ IfSelection [0,2) begin_id:10 end_id:50 depth:1 parent:Function@10 }
  Construct{ Continue [3,4) begin_id:60 end_id:99 depth:1 parent:Function@10 in-c:Continue@60 }
  Construct{ Loop [2,3) begin_id:50 end_id:60 depth:1 parent:Function@10 scope:[2,4) in-l:Loop@50 }
})")) << str.str();

    // The block records the nearest enclosing construct.
    EXPECT_EQ(fe.GetBlockInfo(10)->construct, constructs[1].get());
    EXPECT_EQ(fe.GetBlockInfo(20)->construct, constructs[1].get());
    EXPECT_EQ(fe.GetBlockInfo(50)->construct, constructs[3].get());
    EXPECT_EQ(fe.GetBlockInfo(60)->construct, constructs[2].get());
    EXPECT_EQ(fe.GetBlockInfo(99)->construct, constructs[0].get());
}

TEST_F(SpvParserCFGTest, LabelControlFlowConstructs_Nest_If_If) {
    auto assembly = CommonTypes() + R"(
     %100 = OpFunction %void None %voidfn

     %10 = OpLabel
     OpSelectionMerge %99 None
     OpBranchConditional %cond %20 %50

     %20 = OpLabel
     OpSelectionMerge %40 None
     OpBranchConditional %cond %30 %40 ;; true only

     %30 = OpLabel
     OpBranch %40

     %40 = OpLabel ; merge for first inner "if"
     OpBranch %49

     %49 = OpLabel ; an extra padding block
     OpBranch %99

     %50 = OpLabel
     OpSelectionMerge %89 None
     OpBranchConditional %cond %89 %60 ;; false only

     %60 = OpLabel
     OpBranch %89

     %89 = OpLabel
     OpBranch %99

     %99 = OpLabel
     OpReturn

     OpFunctionEnd
)";
    auto p = parser(test::Assemble(assembly));
    ASSERT_TRUE(p->BuildAndParseInternalModuleExceptFunctions()) << p->error();
    auto fe = p->function_emitter(100);
    fe.RegisterBasicBlocks();
    fe.ComputeBlockOrderAndPositions();
    fe.RegisterMerges();
    EXPECT_TRUE(fe.LabelControlFlowConstructs());
    const auto& constructs = fe.constructs();
    EXPECT_EQ(constructs.Length(), 4u);

    tint::StringStream str;
    str << constructs;

    EXPECT_THAT(ToString(constructs), Eq(R"(ConstructList{
  Construct{ Function [0,9) begin_id:10 end_id:0 depth:0 parent:null }
  Construct{ IfSelection [0,8) begin_id:10 end_id:99 depth:1 parent:Function@10 }
  Construct{ IfSelection [1,3) begin_id:20 end_id:40 depth:2 parent:IfSelection@10 }
  Construct{ IfSelection [5,7) begin_id:50 end_id:89 depth:2 parent:IfSelection@10 }
})")) << str.str();

    // The block records the nearest enclosing construct.
    EXPECT_EQ(fe.GetBlockInfo(10)->construct, constructs[1].get());
    EXPECT_EQ(fe.GetBlockInfo(20)->construct, constructs[2].get());
    EXPECT_EQ(fe.GetBlockInfo(30)->construct, constructs[2].get());
    EXPECT_EQ(fe.GetBlockInfo(40)->construct, constructs[1].get());
    EXPECT_EQ(fe.GetBlockInfo(49)->construct, constructs[1].get());
    EXPECT_EQ(fe.GetBlockInfo(50)->construct, constructs[3].get());
    EXPECT_EQ(fe.GetBlockInfo(60)->construct, constructs[3].get());
    EXPECT_EQ(fe.GetBlockInfo(89)->construct, constructs[1].get());
    EXPECT_EQ(fe.GetBlockInfo(99)->construct, constructs[0].get());
}

TEST_F(SpvParserCFGTest, LabelControlFlowConstructs_Nest_Switch_If) {
    auto assembly = CommonTypes() + R"(
     %100 = OpFunction %void None %voidfn

     %10 = OpLabel
     OpSelectionMerge %99 None
     OpSwitch %selector %99 20 %20 50 %50

     %20 = OpLabel ; if-then nested in case 20
     OpSelectionMerge %49 None
     OpBranchConditional %cond %30 %49

     %30 = OpLabel
     OpBranch %49

     %49 = OpLabel
     OpBranch %99

     %50 = OpLabel ; unles-then nested in case 50
     OpSelectionMerge %89 None
     OpBranchConditional %cond %89 %60

     %60 = OpLabel
     OpBranch %89

     %89 = OpLabel
     OpBranch %99

     %99 = OpLabel
     OpReturn

     OpFunctionEnd
)";
    auto p = parser(test::Assemble(assembly));
    ASSERT_TRUE(p->BuildAndParseInternalModuleExceptFunctions()) << p->error();
    auto fe = p->function_emitter(100);
    fe.RegisterBasicBlocks();
    fe.ComputeBlockOrderAndPositions();
    fe.RegisterMerges();
    EXPECT_TRUE(fe.LabelControlFlowConstructs());
    const auto& constructs = fe.constructs();
    EXPECT_EQ(constructs.Length(), 4u);

    tint::StringStream str;
    str << constructs;

    // The ordering among siblings depends on the computed block order.
    EXPECT_THAT(ToString(constructs), Eq(R"(ConstructList{
  Construct{ Function [0,8) begin_id:10 end_id:0 depth:0 parent:null }
  Construct{ SwitchSelection [0,7) begin_id:10 end_id:99 depth:1 parent:Function@10 in-c-l-s:SwitchSelection@10 }
  Construct{ IfSelection [1,3) begin_id:50 end_id:89 depth:2 parent:SwitchSelection@10 in-c-l-s:SwitchSelection@10 }
  Construct{ IfSelection [4,6) begin_id:20 end_id:49 depth:2 parent:SwitchSelection@10 in-c-l-s:SwitchSelection@10 }
})")) << str.str();

    // The block records the nearest enclosing construct.
    EXPECT_EQ(fe.GetBlockInfo(10)->construct, constructs[1].get());
    EXPECT_EQ(fe.GetBlockInfo(20)->construct, constructs[3].get());
    EXPECT_EQ(fe.GetBlockInfo(30)->construct, constructs[3].get());
    EXPECT_EQ(fe.GetBlockInfo(49)->construct, constructs[1].get());
    EXPECT_EQ(fe.GetBlockInfo(50)->construct, constructs[2].get());
    EXPECT_EQ(fe.GetBlockInfo(60)->construct, constructs[2].get());
    EXPECT_EQ(fe.GetBlockInfo(89)->construct, constructs[1].get());
    EXPECT_EQ(fe.GetBlockInfo(99)->construct, constructs[0].get());
}

TEST_F(SpvParserCFGTest, LabelControlFlowConstructs_Nest_If_Switch) {
    auto assembly = CommonTypes() + R"(
     %100 = OpFunction %void None %voidfn

     %10 = OpLabel
     OpSelectionMerge %99 None
     OpBranchConditional %cond %20 %99

     %20 = OpLabel
     OpSelectionMerge %89 None
     OpSwitch %selector %89 20 %30

     %30 = OpLabel
     OpBranch %89

     %89 = OpLabel
     OpBranch %99

     %99 = OpLabel
     OpReturn

     OpFunctionEnd
)";
    auto p = parser(test::Assemble(assembly));
    ASSERT_TRUE(p->BuildAndParseInternalModuleExceptFunctions()) << p->error();
    auto fe = p->function_emitter(100);
    fe.RegisterBasicBlocks();
    fe.ComputeBlockOrderAndPositions();
    fe.RegisterMerges();
    EXPECT_TRUE(fe.LabelControlFlowConstructs());
    const auto& constructs = fe.constructs();
    EXPECT_EQ(constructs.Length(), 3u);

    tint::StringStream str;
    str << constructs;

    EXPECT_THAT(ToString(constructs), Eq(R"(ConstructList{
  Construct{ Function [0,5) begin_id:10 end_id:0 depth:0 parent:null }
  Construct{ IfSelection [0,4) begin_id:10 end_id:99 depth:1 parent:Function@10 }
  Construct{ SwitchSelection [1,3) begin_id:20 end_id:89 depth:2 parent:IfSelection@10 in-c-l-s:SwitchSelection@20 }
})")) << str.str();

    // The block records the nearest enclosing construct.
    EXPECT_EQ(fe.GetBlockInfo(10)->construct, constructs[1].get());
    EXPECT_EQ(fe.GetBlockInfo(20)->construct, constructs[2].get());
    EXPECT_EQ(fe.GetBlockInfo(30)->construct, constructs[2].get());
    EXPECT_EQ(fe.GetBlockInfo(89)->construct, constructs[1].get());
    EXPECT_EQ(fe.GetBlockInfo(99)->construct, constructs[0].get());
}

TEST_F(SpvParserCFGTest, LabelControlFlowConstructs_Nest_Loop_Loop) {
    auto assembly = CommonTypes() + R"(
     %100 = OpFunction %void None %voidfn

     %10 = OpLabel
     OpBranch %20

     %20 = OpLabel
     OpLoopMerge %89 %50 None
     OpBranchConditional %cond %30 %89

     %30 = OpLabel ; single block loop
     OpLoopMerge %40 %30 None
     OpBranchConditional %cond2 %30 %40

     %40 = OpLabel ; padding block
     OpBranch %50

     %50 = OpLabel ; outer continue target
     OpBranch %60

     %60 = OpLabel
     OpBranch %20

     %89 = OpLabel ; outer merge
     OpBranch %99

     %99 = OpLabel
     OpReturn

     OpFunctionEnd
)";
    auto p = parser(test::Assemble(assembly));
    ASSERT_TRUE(p->BuildAndParseInternalModuleExceptFunctions()) << p->error();
    auto fe = p->function_emitter(100);
    fe.RegisterBasicBlocks();
    fe.ComputeBlockOrderAndPositions();
    fe.RegisterMerges();
    EXPECT_TRUE(fe.LabelControlFlowConstructs());
    const auto& constructs = fe.constructs();
    EXPECT_EQ(constructs.Length(), 4u);

    tint::StringStream str;
    str << constructs;

    EXPECT_THAT(ToString(constructs), Eq(R"(ConstructList{
  Construct{ Function [0,8) begin_id:10 end_id:0 depth:0 parent:null }
  Construct{ Continue [4,6) begin_id:50 end_id:89 depth:1 parent:Function@10 in-c:Continue@50 }
  Construct{ Loop [1,4) begin_id:20 end_id:50 depth:1 parent:Function@10 scope:[1,6) in-l:Loop@20 }
  Construct{ Continue [2,3) begin_id:30 end_id:40 depth:2 parent:Loop@20 in-l:Loop@20 in-c:Continue@30 }
})")) << str.str();

    // The block records the nearest enclosing construct.
    EXPECT_EQ(fe.GetBlockInfo(10)->construct, constructs[0].get());
    EXPECT_EQ(fe.GetBlockInfo(20)->construct, constructs[2].get());
    EXPECT_EQ(fe.GetBlockInfo(30)->construct, constructs[3].get());
    EXPECT_EQ(fe.GetBlockInfo(40)->construct, constructs[2].get());
    EXPECT_EQ(fe.GetBlockInfo(50)->construct, constructs[1].get());
    EXPECT_EQ(fe.GetBlockInfo(60)->construct, constructs[1].get());
    EXPECT_EQ(fe.GetBlockInfo(89)->construct, constructs[0].get());
    EXPECT_EQ(fe.GetBlockInfo(99)->construct, constructs[0].get());
}

TEST_F(SpvParserCFGTest, LabelControlFlowConstructs_Nest_Loop_If) {
    auto assembly = CommonTypes() + R"(
     %100 = OpFunction %void None %voidfn

     %10 = OpLabel
     OpBranch %20

     %20 = OpLabel
     OpLoopMerge %99 %80 None
     OpBranchConditional %cond %30 %99

     %30 = OpLabel ; If, nested in the loop construct
     OpSelectionMerge %49 None
     OpBranchConditional %cond2 %40 %49

     %40 = OpLabel
     OpBranch %49

     %49 = OpLabel ; merge for inner if
     OpBranch %80

     %80 = OpLabel ; continue target
     OpBranch %20

     %99 = OpLabel
     OpReturn

     OpFunctionEnd
)";
    auto p = parser(test::Assemble(assembly));
    ASSERT_TRUE(p->BuildAndParseInternalModuleExceptFunctions()) << p->error();
    auto fe = p->function_emitter(100);
    fe.RegisterBasicBlocks();
    fe.ComputeBlockOrderAndPositions();
    fe.RegisterMerges();
    EXPECT_TRUE(fe.LabelControlFlowConstructs());
    const auto& constructs = fe.constructs();
    EXPECT_EQ(constructs.Length(), 4u);

    tint::StringStream str;
    str << constructs;

    EXPECT_THAT(ToString(constructs), Eq(R"(ConstructList{
  Construct{ Function [0,7) begin_id:10 end_id:0 depth:0 parent:null }
  Construct{ Continue [5,6) begin_id:80 end_id:99 depth:1 parent:Function@10 in-c:Continue@80 }
  Construct{ Loop [1,5) begin_id:20 end_id:80 depth:1 parent:Function@10 scope:[1,6) in-l:Loop@20 }
  Construct{ IfSelection [2,4) begin_id:30 end_id:49 depth:2 parent:Loop@20 in-l:Loop@20 }
})")) << str.str();

    // The block records the nearest enclosing construct.
    EXPECT_EQ(fe.GetBlockInfo(10)->construct, constructs[0].get());
    EXPECT_EQ(fe.GetBlockInfo(20)->construct, constructs[2].get());
    EXPECT_EQ(fe.GetBlockInfo(30)->construct, constructs[3].get());
    EXPECT_EQ(fe.GetBlockInfo(40)->construct, constructs[3].get());
    EXPECT_EQ(fe.GetBlockInfo(49)->construct, constructs[2].get());
    EXPECT_EQ(fe.GetBlockInfo(80)->construct, constructs[1].get());
    EXPECT_EQ(fe.GetBlockInfo(99)->construct, constructs[0].get());
}

TEST_F(SpvParserCFGTest, LabelControlFlowConstructs_Nest_LoopContinue_If) {
    auto assembly = CommonTypes() + R"(
     %100 = OpFunction %void None %voidfn

     %10 = OpLabel
     OpBranch %20

     %20 = OpLabel
     OpLoopMerge %99 %30 None
     OpBranchConditional %cond %30 %99

     %30 = OpLabel ; If, nested at the top of the continue construct head
     OpSelectionMerge %49 None
     OpBranchConditional %cond2 %40 %49

     %40 = OpLabel
     OpBranch %49

     %49 = OpLabel ; merge for inner if, backedge
     OpBranch %20

     %99 = OpLabel
     OpReturn

     OpFunctionEnd
)";
    auto p = parser(test::Assemble(assembly));
    ASSERT_TRUE(p->BuildAndParseInternalModuleExceptFunctions()) << p->error();
    auto fe = p->function_emitter(100);
    fe.RegisterBasicBlocks();
    fe.ComputeBlockOrderAndPositions();
    fe.RegisterMerges();
    EXPECT_TRUE(fe.LabelControlFlowConstructs());
    const auto& constructs = fe.constructs();
    EXPECT_EQ(constructs.Length(), 4u);

    tint::StringStream str;
    str << constructs;

    EXPECT_THAT(ToString(constructs), Eq(R"(ConstructList{
  Construct{ Function [0,6) begin_id:10 end_id:0 depth:0 parent:null }
  Construct{ Continue [2,5) begin_id:30 end_id:99 depth:1 parent:Function@10 in-c:Continue@30 }
  Construct{ Loop [1,2) begin_id:20 end_id:30 depth:1 parent:Function@10 scope:[1,5) in-l:Loop@20 }
  Construct{ IfSelection [2,4) begin_id:30 end_id:49 depth:2 parent:Continue@30 in-c:Continue@30 }
})")) << str.str();
    // The block records the nearest enclosing construct.
    EXPECT_EQ(fe.GetBlockInfo(10)->construct, constructs[0].get());
    EXPECT_EQ(fe.GetBlockInfo(20)->construct, constructs[2].get());
    EXPECT_EQ(fe.GetBlockInfo(30)->construct, constructs[3].get());
    EXPECT_EQ(fe.GetBlockInfo(40)->construct, constructs[3].get());
    EXPECT_EQ(fe.GetBlockInfo(49)->construct, constructs[1].get());
    EXPECT_EQ(fe.GetBlockInfo(99)->construct, constructs[0].get());
}

TEST_F(SpvParserCFGTest, LabelControlFlowConstructs_Nest_If_SingleBlockLoop) {
    auto assembly = CommonTypes() + R"(
     %100 = OpFunction %void None %voidfn

     %10 = OpLabel
     OpSelectionMerge %99 None
     OpBranchConditional %cond %20 %99

     %20 = OpLabel
     OpLoopMerge %89 %20 None
     OpBranchConditional %cond %20 %89

     %89 = OpLabel
     OpBranch %99

     %99 = OpLabel
     OpReturn

     OpFunctionEnd
)";
    auto p = parser(test::Assemble(assembly));
    ASSERT_TRUE(p->BuildAndParseInternalModuleExceptFunctions()) << p->error();
    auto fe = p->function_emitter(100);
    fe.RegisterBasicBlocks();
    fe.ComputeBlockOrderAndPositions();
    fe.RegisterMerges();
    EXPECT_TRUE(fe.LabelControlFlowConstructs());
    const auto& constructs = fe.constructs();
    EXPECT_EQ(constructs.Length(), 3u);

    tint::StringStream str;
    str << constructs;

    EXPECT_THAT(ToString(constructs), Eq(R"(ConstructList{
  Construct{ Function [0,4) begin_id:10 end_id:0 depth:0 parent:null }
  Construct{ IfSelection [0,3) begin_id:10 end_id:99 depth:1 parent:Function@10 }
  Construct{ Continue [1,2) begin_id:20 end_id:89 depth:2 parent:IfSelection@10 in-c:Continue@20 }
})")) << str.str();
    // The block records the nearest enclosing construct.
    EXPECT_EQ(fe.GetBlockInfo(10)->construct, constructs[1].get());
    EXPECT_EQ(fe.GetBlockInfo(20)->construct, constructs[2].get());
    EXPECT_EQ(fe.GetBlockInfo(99)->construct, constructs[0].get());
}

TEST_F(SpvParserCFGTest, LabelControlFlowConstructs_Nest_If_MultiBlockLoop) {
    auto assembly = CommonTypes() + R"(
     %100 = OpFunction %void None %voidfn

     %10 = OpLabel
     OpSelectionMerge %99 None
     OpBranchConditional %cond %20 %99

     %20 = OpLabel ; start loop body
     OpLoopMerge %89 %40 None
     OpBranchConditional %cond %30 %89

     %30 = OpLabel ; body block
     OpBranch %40

     %40 = OpLabel ; continue target
     OpBranch %50

     %50 = OpLabel ; backedge block
     OpBranch %20

     %89 = OpLabel ; merge for the loop
     OpBranch %99

     %99 = OpLabel ; merge for the if
     OpReturn

     OpFunctionEnd
)";
    auto p = parser(test::Assemble(assembly));
    ASSERT_TRUE(p->BuildAndParseInternalModuleExceptFunctions()) << p->error();
    auto fe = p->function_emitter(100);
    fe.RegisterBasicBlocks();
    fe.ComputeBlockOrderAndPositions();
    fe.RegisterMerges();
    EXPECT_TRUE(fe.LabelControlFlowConstructs());
    const auto& constructs = fe.constructs();
    EXPECT_EQ(constructs.Length(), 4u);

    tint::StringStream str;
    str << constructs;

    EXPECT_THAT(ToString(constructs), Eq(R"(ConstructList{
  Construct{ Function [0,7) begin_id:10 end_id:0 depth:0 parent:null }
  Construct{ IfSelection [0,6) begin_id:10 end_id:99 depth:1 parent:Function@10 }
  Construct{ Continue [3,5) begin_id:40 end_id:89 depth:2 parent:IfSelection@10 in-c:Continue@40 }
  Construct{ Loop [1,3) begin_id:20 end_id:40 depth:2 parent:IfSelection@10 scope:[1,5) in-l:Loop@20 }
})")) << str.str();

    // The block records the nearest enclosing construct.
    EXPECT_EQ(fe.GetBlockInfo(10)->construct, constructs[1].get());
    EXPECT_EQ(fe.GetBlockInfo(20)->construct, constructs[3].get());
    EXPECT_EQ(fe.GetBlockInfo(30)->construct, constructs[3].get());
    EXPECT_EQ(fe.GetBlockInfo(40)->construct, constructs[2].get());
    EXPECT_EQ(fe.GetBlockInfo(50)->construct, constructs[2].get());
    EXPECT_EQ(fe.GetBlockInfo(89)->construct, constructs[1].get());
    EXPECT_EQ(fe.GetBlockInfo(99)->construct, constructs[0].get());
}

TEST_F(SpvParserCFGTest, LabelControlFlowConstructs_LoopInterallyDiverge) {
    // In this case, insert a synthetic if-selection with the same blocks
    // as the loop construct.
    // crbug.com/tint/524
    auto assembly = CommonTypes() + R"(
     %100 = OpFunction %void None %voidfn

     %10 = OpLabel
     OpBranch %20

     %20 = OpLabel
     OpLoopMerge %99 %90 None
     OpBranchConditional %cond %30 %40 ; divergence to distinct targets in the body

       %30 = OpLabel
       OpBranch %90

       %40 = OpLabel
       OpBranch %90

     %90 = OpLabel ; continue target
     OpBranch %20

     %99 = OpLabel ; loop merge
     OpReturn

     OpFunctionEnd
)";
    auto p = parser(test::Assemble(assembly));
    ASSERT_TRUE(p->BuildAndParseInternalModuleExceptFunctions()) << p->error();
    auto fe = p->function_emitter(100);
    ASSERT_TRUE(FlowLabelControlFlowConstructs(&fe)) << p->error();
    const auto& constructs = fe.constructs();
    EXPECT_EQ(constructs.Length(), 4u);

    tint::StringStream str;
    str << constructs;

    ASSERT_THAT(ToString(constructs), Eq(R"(ConstructList{
  Construct{ Function [0,6) begin_id:10 end_id:0 depth:0 parent:null }
  Construct{ Continue [4,5) begin_id:90 end_id:99 depth:1 parent:Function@10 in-c:Continue@90 }
  Construct{ Loop [1,4) begin_id:20 end_id:90 depth:1 parent:Function@10 scope:[1,5) in-l:Loop@20 }
  Construct{ IfSelection [1,4) begin_id:20 end_id:90 depth:2 parent:Loop@20 in-l:Loop@20 }
})")) << str.str();

    // The block records the nearest enclosing construct.
    EXPECT_EQ(fe.GetBlockInfo(10)->construct, constructs[0].get());
    EXPECT_EQ(fe.GetBlockInfo(20)->construct, constructs[3].get());
    EXPECT_EQ(fe.GetBlockInfo(30)->construct, constructs[3].get());
    EXPECT_EQ(fe.GetBlockInfo(40)->construct, constructs[3].get());
    EXPECT_EQ(fe.GetBlockInfo(90)->construct, constructs[1].get());
    EXPECT_EQ(fe.GetBlockInfo(99)->construct, constructs[0].get());
}

TEST_F(SpvParserCFGTest, FindSwitchCaseHeaders_DefaultIsLongRangeBackedge) {
    auto assembly = CommonTypes() + R"(
     %100 = OpFunction %void None %voidfn

     %10 = OpLabel
     OpBranch %20

     %20 = OpLabel
     OpSelectionMerge %99 None
     OpSwitch %selector %10 30 %30

     %30 = OpLabel
     OpBranch %99

     %99 = OpLabel
     OpReturn

     OpFunctionEnd
)";
    auto p = parser(test::Assemble(assembly));
    ASSERT_TRUE(p->BuildAndParseInternalModuleExceptFunctions()) << p->error();
    auto fe = p->function_emitter(100);
    fe.RegisterBasicBlocks();
    fe.ComputeBlockOrderAndPositions();
    EXPECT_TRUE(fe.VerifyHeaderContinueMergeOrder());
    fe.RegisterMerges();
    fe.LabelControlFlowConstructs();
    EXPECT_FALSE(fe.FindSwitchCaseHeaders());
    EXPECT_THAT(p->error(), Eq("Switch branch from block 20 to default target "
                               "block 10 can't be a back-edge"));
}

TEST_F(SpvParserCFGTest, FindSwitchCaseHeaders_DefaultIsSelfLoop) {
    auto assembly = CommonTypes() + R"(
     %100 = OpFunction %void None %voidfn

     %10 = OpLabel
     OpBranch %20

     %20 = OpLabel
     OpSelectionMerge %99 None
     OpSwitch %selector %20 30 %30

     %30 = OpLabel
     OpBranch %99

     %99 = OpLabel
     OpReturn

     OpFunctionEnd
)";
    auto p = parser(test::Assemble(assembly));
    ASSERT_TRUE(p->BuildAndParseInternalModuleExceptFunctions()) << p->error();
    auto fe = p->function_emitter(100);
    fe.RegisterBasicBlocks();
    fe.ComputeBlockOrderAndPositions();
    EXPECT_TRUE(fe.VerifyHeaderContinueMergeOrder());
    fe.RegisterMerges();
    fe.LabelControlFlowConstructs();
    EXPECT_FALSE(fe.FindSwitchCaseHeaders());
    // Self-loop that isn't its own continue target is already rejected with a
    // different message.
    EXPECT_THAT(p->error(), Eq("Block 20 branches to itself but is not its own continue target"));
}

TEST_F(SpvParserCFGTest, FindSwitchCaseHeaders_DefaultCantEscapeSwitch) {
    auto assembly = CommonTypes() + R"(
     %100 = OpFunction %void None %voidfn

     %10 = OpLabel
     OpSelectionMerge %50 None
     OpSwitch %selector %99 30 %30 ; default goes past the merge

     %30 = OpLabel
     OpBranch %50

     %50 = OpLabel ; merge
     OpBranch %99

     %99 = OpLabel
     OpReturn

     OpFunctionEnd
)";
    auto p = parser(test::Assemble(assembly));
    ASSERT_TRUE(p->BuildAndParseInternalModuleExceptFunctions()) << p->error();
    auto fe = p->function_emitter(100);
    fe.RegisterBasicBlocks();
    fe.ComputeBlockOrderAndPositions();
    EXPECT_TRUE(fe.VerifyHeaderContinueMergeOrder());
    fe.RegisterMerges();
    fe.LabelControlFlowConstructs();
    EXPECT_FALSE(fe.FindSwitchCaseHeaders());
    EXPECT_THAT(p->error(), Eq("Switch branch from block 10 to default block 99 "
                               "escapes the selection construct"));
}

TEST_F(SpvParserCFGTest, FindSwitchCaseHeaders_DefaultForTwoSwitches_AsMerge) {
    auto assembly = CommonTypes() + R"(
     %100 = OpFunction %void None %voidfn

     %10 = OpLabel
     OpSelectionMerge %99 None
     OpSwitch %selector %89 20 %20

     %20 = OpLabel
     OpBranch %50

     %50 = OpLabel
     OpSelectionMerge %89 None
     OpSwitch %selector %89 60 %60

     %60 = OpLabel
     OpBranch %89

     %89 = OpLabel
     OpBranch %99

     %99 = OpLabel
     OpReturn

     OpFunctionEnd
)";
    auto p = parser(test::Assemble(assembly));
    ASSERT_TRUE(p->BuildAndParseInternalModuleExceptFunctions()) << p->error();
    auto fe = p->function_emitter(100);
    fe.RegisterBasicBlocks();
    fe.ComputeBlockOrderAndPositions();
    EXPECT_TRUE(fe.VerifyHeaderContinueMergeOrder());
    fe.RegisterMerges();
    fe.LabelControlFlowConstructs();
    EXPECT_FALSE(fe.FindSwitchCaseHeaders());
    EXPECT_THAT(p->error(), Eq("Block 89 is the default block for switch-selection header 10 "
                               "and also the merge block for 50 (violates dominance rule)"));
}

TEST_F(SpvParserCFGTest, FindSwitchCaseHeaders_DefaultForTwoSwitches_AsCaseClause) {
    auto assembly = CommonTypes() + R"(
     %100 = OpFunction %void None %voidfn

     %10 = OpLabel
     OpSelectionMerge %99 None
     OpSwitch %selector %80 20 %20

     %20 = OpLabel
     OpBranch %50

     %50 = OpLabel
     OpSelectionMerge %89 None
     OpSwitch %selector %80 60 %60

     %60 = OpLabel
     OpBranch %89

     %80 = OpLabel ; default for both switches
     OpBranch %89

     %89 = OpLabel ; inner selection merge
     OpBranch %99

     %99 = OpLabel ; outer selection mege
     OpReturn

     OpFunctionEnd
)";
    auto p = parser(test::Assemble(assembly));
    ASSERT_TRUE(p->BuildAndParseInternalModuleExceptFunctions()) << p->error();
    auto fe = p->function_emitter(100);
    fe.RegisterBasicBlocks();
    fe.ComputeBlockOrderAndPositions();
    EXPECT_TRUE(fe.VerifyHeaderContinueMergeOrder());
    fe.RegisterMerges();
    fe.LabelControlFlowConstructs();
    EXPECT_FALSE(fe.FindSwitchCaseHeaders());
    EXPECT_THAT(p->error(), Eq("Block 80 is declared as the default target for "
                               "two OpSwitch instructions, at blocks 10 and 50"));
}

TEST_F(SpvParserCFGTest, FindSwitchCaseHeaders_CaseIsLongRangeBackedge) {
    auto assembly = CommonTypes() + R"(
     %100 = OpFunction %void None %voidfn

     %10 = OpLabel
     OpBranch %20

     %20 = OpLabel
     OpSelectionMerge %99 None
     OpSwitch %selector %99 10 %10

     %99 = OpLabel
     OpReturn

     OpFunctionEnd
)";
    auto p = parser(test::Assemble(assembly));
    ASSERT_TRUE(p->BuildAndParseInternalModuleExceptFunctions()) << p->error();
    auto fe = p->function_emitter(100);
    fe.RegisterBasicBlocks();
    fe.ComputeBlockOrderAndPositions();
    EXPECT_TRUE(fe.VerifyHeaderContinueMergeOrder());
    fe.RegisterMerges();
    fe.LabelControlFlowConstructs();
    EXPECT_FALSE(fe.FindSwitchCaseHeaders());
    EXPECT_THAT(p->error(), Eq("Switch branch from block 20 to case target "
                               "block 10 can't be a back-edge"));
}

TEST_F(SpvParserCFGTest, FindSwitchCaseHeaders_CaseIsSelfLoop) {
    auto assembly = CommonTypes() + R"(
     %100 = OpFunction %void None %voidfn

     %10 = OpLabel
     OpBranch %20

     %20 = OpLabel
     OpSelectionMerge %99 None
     OpSwitch %selector %99 20 %20

     %99 = OpLabel
     OpReturn

     OpFunctionEnd
)";
    auto p = parser(test::Assemble(assembly));
    ASSERT_TRUE(p->BuildAndParseInternalModuleExceptFunctions()) << p->error();
    auto fe = p->function_emitter(100);
    fe.RegisterBasicBlocks();
    fe.ComputeBlockOrderAndPositions();
    EXPECT_TRUE(fe.VerifyHeaderContinueMergeOrder());
    fe.RegisterMerges();
    fe.LabelControlFlowConstructs();
    EXPECT_FALSE(fe.FindSwitchCaseHeaders());
    // The error is caught earlier
    EXPECT_THAT(p->error(), Eq("Block 20 branches to itself but is not its own continue target"));
}

TEST_F(SpvParserCFGTest, FindSwitchCaseHeaders_CaseCanBeSwitchMerge) {
    auto assembly = CommonTypes() + R"(
     %100 = OpFunction %void None %voidfn

     %10 = OpLabel
     OpBranch %20

     %20 = OpLabel
     OpSelectionMerge %99 None
     OpSwitch %selector %99 20 %99

     %99 = OpLabel
     OpReturn

     OpFunctionEnd
)";
    auto p = parser(test::Assemble(assembly));
    ASSERT_TRUE(p->BuildAndParseInternalModuleExceptFunctions()) << p->error();
    auto fe = p->function_emitter(100);
    fe.RegisterBasicBlocks();
    fe.ComputeBlockOrderAndPositions();
    EXPECT_TRUE(fe.VerifyHeaderContinueMergeOrder());
    fe.RegisterMerges();
    fe.LabelControlFlowConstructs();
    EXPECT_TRUE(fe.FindSwitchCaseHeaders());

    // TODO(crbug.com/tint/774) Re-enable after codegen bug fixed.
    p->DeliberatelyInvalidSpirv();
}

TEST_F(SpvParserCFGTest, FindSwitchCaseHeaders_CaseCantEscapeSwitch) {
    auto assembly = CommonTypes() + R"(
     %100 = OpFunction %void None %voidfn

     %10 = OpLabel
     OpSelectionMerge %99 None ; force %99 to be very late in block order
     OpBranchConditional %cond %20 %99

     %20 = OpLabel
     OpSelectionMerge %89 None
     OpSwitch %selector %89 20 %99

     %89 = OpLabel
     OpBranch %99

     %99 = OpLabel
     OpReturn

     OpFunctionEnd
)";
    auto p = parser(test::Assemble(assembly));
    ASSERT_TRUE(p->BuildAndParseInternalModuleExceptFunctions()) << p->error();
    auto fe = p->function_emitter(100);
    fe.RegisterBasicBlocks();
    fe.ComputeBlockOrderAndPositions();
    EXPECT_TRUE(fe.VerifyHeaderContinueMergeOrder());
    fe.RegisterMerges();
    fe.LabelControlFlowConstructs();
    EXPECT_FALSE(fe.FindSwitchCaseHeaders());
    EXPECT_THAT(p->error(), Eq("Switch branch from block 20 to case target block "
                               "99 escapes the selection construct"));
}

TEST_F(SpvParserCFGTest, FindSwitchCaseHeaders_CaseForMoreThanOneSwitch) {
    auto assembly = CommonTypes() + R"(
     %100 = OpFunction %void None %voidfn

     %10 = OpLabel
     OpSelectionMerge %99 None
     OpSwitch %selector %99 20 %20 50 %50

     %20 = OpLabel
     OpSelectionMerge %89 None
     OpSwitch %selector %89 50 %50

     %50 = OpLabel
     OpBranch %89

     %89 = OpLabel
     OpBranch %99

     %99 = OpLabel
     OpReturn

     OpFunctionEnd
)";
    auto p = parser(test::Assemble(assembly));
    ASSERT_TRUE(p->BuildAndParseInternalModuleExceptFunctions()) << p->error();
    auto fe = p->function_emitter(100);
    fe.RegisterBasicBlocks();
    fe.ComputeBlockOrderAndPositions();
    EXPECT_TRUE(fe.VerifyHeaderContinueMergeOrder());
    fe.RegisterMerges();
    fe.LabelControlFlowConstructs();
    EXPECT_FALSE(fe.FindSwitchCaseHeaders());
    EXPECT_THAT(p->error(), Eq("Block 50 is declared as the switch case target for two "
                               "OpSwitch instructions, at blocks 10 and 20"));
}

TEST_F(SpvParserCFGTest, FindSwitchCaseHeaders_CaseIsMergeForAnotherConstruct) {
    auto assembly = CommonTypes() + R"(
     %100 = OpFunction %void None %voidfn

     %10 = OpLabel
     OpSelectionMerge %49 None
     OpSwitch %selector %49 20 %20

     %20 = OpLabel
     OpBranch %49

     %49 = OpLabel
     OpBranch %50

     %50 = OpLabel
     OpSelectionMerge %20 None ; points back to the case.
     OpBranchConditional %cond %60 %99

     %60 = OpLabel
     OpBranch %99

     %99 = OpLabel
     OpReturn

     OpFunctionEnd
)";
    auto p = parser(test::Assemble(assembly));
    ASSERT_TRUE(p->BuildAndParseInternalModuleExceptFunctions()) << p->error();
    auto fe = p->function_emitter(100);
    fe.RegisterBasicBlocks();
    fe.ComputeBlockOrderAndPositions();
    EXPECT_TRUE(fe.VerifyHeaderContinueMergeOrder());
    fe.RegisterMerges();
    fe.LabelControlFlowConstructs();
    EXPECT_FALSE(fe.FindSwitchCaseHeaders());
    EXPECT_THAT(p->error(), Eq("Switch branch from block 10 to case target block "
                               "20 escapes the selection construct"));
}

TEST_F(SpvParserCFGTest, FindSwitchCaseHeaders_NoSwitch) {
    auto assembly = CommonTypes() + R"(
     %100 = OpFunction %void None %voidfn

     %10 = OpLabel
     OpReturn

     OpFunctionEnd
)";
    auto p = parser(test::Assemble(assembly));
    ASSERT_TRUE(p->BuildAndParseInternalModuleExceptFunctions()) << p->error();
    auto fe = p->function_emitter(100);
    fe.RegisterBasicBlocks();
    fe.ComputeBlockOrderAndPositions();
    EXPECT_TRUE(fe.VerifyHeaderContinueMergeOrder());
    fe.RegisterMerges();
    fe.LabelControlFlowConstructs();
    EXPECT_TRUE(fe.FindSwitchCaseHeaders());

    const auto* bi10 = fe.GetBlockInfo(10);
    ASSERT_NE(bi10, nullptr);
    EXPECT_EQ(bi10->case_head_for, nullptr);
    EXPECT_EQ(bi10->default_head_for, nullptr);
    EXPECT_FALSE(bi10->default_is_merge);
    EXPECT_FALSE(bi10->case_values.has_value());
}

TEST_F(SpvParserCFGTest, FindSwitchCaseHeaders_DefaultIsMerge) {
    auto assembly = CommonTypes() + R"(
     %100 = OpFunction %void None %voidfn

     %10 = OpLabel
     OpSelectionMerge %99 None
     OpSwitch %selector %99 20 %20

     %20 = OpLabel
     OpBranch %99

     %99 = OpLabel
     OpReturn

     OpFunctionEnd
)";
    auto p = parser(test::Assemble(assembly));
    ASSERT_TRUE(p->BuildAndParseInternalModuleExceptFunctions()) << p->error();
    auto fe = p->function_emitter(100);
    fe.RegisterBasicBlocks();
    fe.ComputeBlockOrderAndPositions();
    EXPECT_TRUE(fe.VerifyHeaderContinueMergeOrder());
    fe.RegisterMerges();
    fe.LabelControlFlowConstructs();
    EXPECT_TRUE(fe.FindSwitchCaseHeaders());

    const auto* bi99 = fe.GetBlockInfo(99);
    ASSERT_NE(bi99, nullptr);
    EXPECT_EQ(bi99->case_head_for, nullptr);
    ASSERT_NE(bi99->default_head_for, nullptr);
    EXPECT_EQ(bi99->default_head_for->begin_id, 10u);
    EXPECT_TRUE(bi99->default_is_merge);
    EXPECT_FALSE(bi99->case_values.has_value());
}

TEST_F(SpvParserCFGTest, FindSwitchCaseHeaders_DefaultIsNotMerge) {
    auto assembly = CommonTypes() + R"(
     %100 = OpFunction %void None %voidfn

     %10 = OpLabel
     OpSelectionMerge %99 None
     OpSwitch %selector %30 20 %20

     %20 = OpLabel
     OpBranch %99

     %30 = OpLabel
     OpBranch %99

     %99 = OpLabel
     OpReturn

     OpFunctionEnd
)";
    auto p = parser(test::Assemble(assembly));
    ASSERT_TRUE(p->BuildAndParseInternalModuleExceptFunctions()) << p->error();
    auto fe = p->function_emitter(100);
    fe.RegisterBasicBlocks();
    fe.ComputeBlockOrderAndPositions();
    EXPECT_TRUE(fe.VerifyHeaderContinueMergeOrder());
    fe.RegisterMerges();
    fe.LabelControlFlowConstructs();
    EXPECT_TRUE(fe.FindSwitchCaseHeaders());

    const auto* bi30 = fe.GetBlockInfo(30);
    ASSERT_NE(bi30, nullptr);
    EXPECT_EQ(bi30->case_head_for, nullptr);
    ASSERT_NE(bi30->default_head_for, nullptr);
    EXPECT_EQ(bi30->default_head_for->begin_id, 10u);
    EXPECT_FALSE(bi30->default_is_merge);
    EXPECT_FALSE(bi30->case_values.has_value());
}

TEST_F(SpvParserCFGTest, FindSwitchCaseHeaders_CaseIsNotDefault) {
    auto assembly = CommonTypes() + R"(
     %100 = OpFunction %void None %voidfn

     %10 = OpLabel
     OpSelectionMerge %99 None
     OpSwitch %selector %30 200 %20

     %20 = OpLabel
     OpBranch %99

     %30 = OpLabel
     OpBranch %99

     %99 = OpLabel
     OpReturn

     OpFunctionEnd
)";
    auto p = parser(test::Assemble(assembly));
    ASSERT_TRUE(p->BuildAndParseInternalModuleExceptFunctions()) << p->error();
    auto fe = p->function_emitter(100);
    fe.RegisterBasicBlocks();
    fe.ComputeBlockOrderAndPositions();
    EXPECT_TRUE(fe.VerifyHeaderContinueMergeOrder());
    fe.RegisterMerges();
    fe.LabelControlFlowConstructs();
    EXPECT_TRUE(fe.FindSwitchCaseHeaders());

    const auto* bi20 = fe.GetBlockInfo(20);
    ASSERT_NE(bi20, nullptr);
    ASSERT_NE(bi20->case_head_for, nullptr);
    EXPECT_EQ(bi20->case_head_for->begin_id, 10u);
    EXPECT_EQ(bi20->default_head_for, nullptr);
    EXPECT_FALSE(bi20->default_is_merge);
    EXPECT_THAT(bi20->case_values.value(), UnorderedElementsAre(200));
}

TEST_F(SpvParserCFGTest, FindSwitchCaseHeaders_CaseIsDefault) {
    auto assembly = CommonTypes() + R"(
     %100 = OpFunction %void None %voidfn

     %10 = OpLabel
     OpSelectionMerge %99 None
     OpSwitch %selector %20 200 %20

     %20 = OpLabel
     OpBranch %99

     %99 = OpLabel
     OpReturn

     OpFunctionEnd
)";
    auto p = parser(test::Assemble(assembly));
    ASSERT_TRUE(p->BuildAndParseInternalModuleExceptFunctions()) << p->error();
    auto fe = p->function_emitter(100);
    fe.RegisterBasicBlocks();
    fe.ComputeBlockOrderAndPositions();
    EXPECT_TRUE(fe.VerifyHeaderContinueMergeOrder());
    fe.RegisterMerges();
    fe.LabelControlFlowConstructs();
    EXPECT_TRUE(fe.FindSwitchCaseHeaders());

    const auto* bi20 = fe.GetBlockInfo(20);
    ASSERT_NE(bi20, nullptr);
    ASSERT_NE(bi20->case_head_for, nullptr);
    EXPECT_EQ(bi20->case_head_for->begin_id, 10u);
    EXPECT_EQ(bi20->default_head_for, bi20->case_head_for);
    EXPECT_FALSE(bi20->default_is_merge);
    EXPECT_THAT(bi20->case_values.value(), UnorderedElementsAre(200));
}

TEST_F(SpvParserCFGTest, FindSwitchCaseHeaders_ManyCasesWithSameValue_IsError) {
    auto assembly = CommonTypes() + R"(
     %100 = OpFunction %void None %voidfn

     %10 = OpLabel
     OpSelectionMerge %99 None
     OpSwitch %selector %99 200 %20 200 %30

     %20 = OpLabel
     OpBranch %99

     %30 = OpLabel
     OpBranch %99

     %99 = OpLabel
     OpReturn

     OpFunctionEnd
)";
    auto p = parser(test::Assemble(assembly));
    ASSERT_TRUE(p->BuildAndParseInternalModuleExceptFunctions()) << p->error();
    auto fe = p->function_emitter(100);
    fe.RegisterBasicBlocks();
    fe.ComputeBlockOrderAndPositions();
    EXPECT_TRUE(fe.VerifyHeaderContinueMergeOrder());
    fe.RegisterMerges();
    fe.LabelControlFlowConstructs();
    EXPECT_FALSE(fe.FindSwitchCaseHeaders());

    EXPECT_THAT(p->error(), Eq("Duplicate case value 200 in OpSwitch in block 10"));
}

TEST_F(SpvParserCFGTest, FindSwitchCaseHeaders_ManyValuesWithSameCase) {
    auto assembly = CommonTypes() + R"(
     %100 = OpFunction %void None %voidfn

     %10 = OpLabel
     OpSelectionMerge %99 None
     OpSwitch %selector %99 200 %20 300 %20

     %20 = OpLabel
     OpBranch %99

     %99 = OpLabel
     OpReturn

     OpFunctionEnd
)";
    auto p = parser(test::Assemble(assembly));
    ASSERT_TRUE(p->BuildAndParseInternalModuleExceptFunctions()) << p->error();
    auto fe = p->function_emitter(100);
    fe.RegisterBasicBlocks();
    fe.ComputeBlockOrderAndPositions();
    EXPECT_TRUE(fe.VerifyHeaderContinueMergeOrder());
    fe.RegisterMerges();
    fe.LabelControlFlowConstructs();
    EXPECT_TRUE(fe.FindSwitchCaseHeaders());

    const auto* bi20 = fe.GetBlockInfo(20);
    ASSERT_NE(bi20, nullptr);
    ASSERT_NE(bi20->case_head_for, nullptr);
    EXPECT_EQ(bi20->case_head_for->begin_id, 10u);
    EXPECT_EQ(bi20->default_head_for, nullptr);
    EXPECT_FALSE(bi20->default_is_merge);
    EXPECT_THAT(bi20->case_values.value(), UnorderedElementsAre(200, 300));
}

TEST_F(SpvParserCFGTest, ClassifyCFGEdges_BranchEscapesIfConstruct) {
    auto assembly = CommonTypes() + R"(
     %100 = OpFunction %void None %voidfn

     %10 = OpLabel
     OpSelectionMerge %99 None
     OpBranchConditional %cond %20 %99

     %20 = OpLabel
     OpSelectionMerge %50 None
     OpBranchConditional %cond2 %30 %50

     %30 = OpLabel
     OpBranch %80   ; bad exit to %80

     %50 = OpLabel
     OpBranch %80

     %80 = OpLabel  ; bad target
     OpBranch %99

     %99 = OpLabel
     OpReturn

     OpFunctionEnd
)";
    auto p = parser(test::Assemble(assembly));
    ASSERT_TRUE(p->BuildAndParseInternalModuleExceptFunctions()) << p->error();
    auto fe = p->function_emitter(100);
    EXPECT_FALSE(FlowClassifyCFGEdges(&fe)) << p->error();
    // Some further processing
    EXPECT_THAT(p->error(), Eq("Branch from block 30 to block 80 is an invalid exit from construct "
                               "starting at block 20; branch bypasses merge block 50"));
}

TEST_F(SpvParserCFGTest, ClassifyCFGEdges_ReturnInContinueConstruct) {
    auto assembly = CommonTypes() + R"(
     %100 = OpFunction %void None %voidfn

     %10 = OpLabel
     OpBranch %20

     %20 = OpLabel
     OpLoopMerge %99 %50 None
     OpBranchConditional %cond %30 %99

     %30 = OpLabel ; body
     OpBranch %50

     %50 = OpLabel
     OpReturn

     %99 = OpLabel
     OpReturn

     OpFunctionEnd
)";
    auto p = parser(test::Assemble(assembly));
    ASSERT_TRUE(p->BuildAndParseInternalModuleExceptFunctions()) << p->error();
    auto fe = p->function_emitter(100);
    EXPECT_FALSE(FlowClassifyCFGEdges(&fe)) << p->error();
    EXPECT_THAT(p->error(), Eq("Invalid function exit at block 50 from continue "
                               "construct starting at 50"));
}

TEST_F(SpvParserCFGTest, ClassifyCFGEdges_KillInContinueConstruct) {
    auto assembly = CommonTypes() + R"(
     %100 = OpFunction %void None %voidfn

     %10 = OpLabel
     OpBranch %20

     %20 = OpLabel
     OpLoopMerge %99 %50 None
     OpBranchConditional %cond %30 %99

     %30 = OpLabel ; body
     OpBranch %50

     %50 = OpLabel
     OpKill

     %99 = OpLabel
     OpReturn

     OpFunctionEnd
)";
    auto p = parser(test::Assemble(assembly));
    ASSERT_TRUE(p->BuildAndParseInternalModuleExceptFunctions()) << p->error();
    auto fe = p->function_emitter(100);
    EXPECT_FALSE(FlowClassifyCFGEdges(&fe));
    EXPECT_THAT(p->error(), Eq("Invalid function exit at block 50 from continue "
                               "construct starting at 50"));
}

TEST_F(SpvParserCFGTest, ClassifyCFGEdges_UnreachableInContinueConstruct) {
    auto assembly = CommonTypes() + R"(
     %100 = OpFunction %void None %voidfn

     %10 = OpLabel
     OpBranch %20

     %20 = OpLabel
     OpLoopMerge %99 %50 None
     OpBranchConditional %cond %30 %99

     %30 = OpLabel ; body
     OpBranch %50

     %50 = OpLabel
     OpUnreachable

     %99 = OpLabel
     OpReturn

     OpFunctionEnd
)";
    auto p = parser(test::Assemble(assembly));
    ASSERT_TRUE(p->BuildAndParseInternalModuleExceptFunctions()) << p->error();
    auto fe = p->function_emitter(100);
    EXPECT_FALSE(FlowClassifyCFGEdges(&fe));
    EXPECT_THAT(p->error(), Eq("Invalid function exit at block 50 from continue "
                               "construct starting at 50"));
}

TEST_F(SpvParserCFGTest, ClassifyCFGEdges_BackEdge_NotInContinueConstruct) {
    auto assembly = CommonTypes() + R"(
     %100 = OpFunction %void None %voidfn

     %10 = OpLabel
     OpBranch %20

     %20 = OpLabel
     OpLoopMerge %99 %50 None
     OpBranchConditional %cond %30 %99

     %30 = OpLabel ; body
     OpBranch %20  ; bad backedge

     %50 = OpLabel ; continue target
     OpBranch %99

     %99 = OpLabel
     OpReturn

     OpFunctionEnd
)";
    auto p = parser(test::Assemble(assembly));
    ASSERT_TRUE(p->BuildAndParseInternalModuleExceptFunctions()) << p->error();
    auto fe = p->function_emitter(100);
    EXPECT_FALSE(FlowClassifyCFGEdges(&fe));
    EXPECT_THAT(p->error(), Eq("Invalid backedge (30->20): 30 is not in a continue construct"));
}

TEST_F(SpvParserCFGTest, ClassifyCFGEdges_BackEdge_NotInLastBlockOfContinueConstruct) {
    auto assembly = CommonTypes() + R"(
     %100 = OpFunction %void None %voidfn

     %10 = OpLabel
     OpBranch %20

     %20 = OpLabel
     OpLoopMerge %99 %50 None
     OpBranchConditional %cond %30 %99

     %30 = OpLabel ; body
     OpBranch %50

     %50 = OpLabel ; continue target
     OpBranchConditional %cond %20 %60 ; bad branch to %20

     %60 = OpLabel ; end of continue construct
     OpBranch %20

     %99 = OpLabel
     OpReturn

     OpFunctionEnd
)";
    auto p = parser(test::Assemble(assembly));
    ASSERT_TRUE(p->BuildAndParseInternalModuleExceptFunctions()) << p->error();
    auto fe = p->function_emitter(100);
    EXPECT_FALSE(FlowClassifyCFGEdges(&fe));
    EXPECT_THAT(p->error(), Eq("Invalid exit (50->20) from continue construct: 50 is not the "
                               "last block in the continue construct starting at 50 "
                               "(violates post-dominance rule)"));
}

TEST_F(SpvParserCFGTest, ClassifyCFGEdges_BackEdge_ToWrongHeader) {
    auto assembly = CommonTypes() + R"(
     %100 = OpFunction %void None %voidfn

     %10 = OpLabel
     OpSelectionMerge %99 None
     OpBranchConditional %cond %20 %99

     %20 = OpLabel
     OpLoopMerge %89 %50 None
     OpBranchConditional %cond %30 %89

     %30 = OpLabel ; loop body
     OpBranch %50

     %50 = OpLabel ; continue target
     OpBranch %10

     %89 = OpLabel ; inner merge
     OpBranch %99

     %99 = OpLabel ; outer merge
     OpReturn

     OpFunctionEnd
)";
    auto p = parser(test::Assemble(assembly));
    ASSERT_TRUE(p->BuildAndParseInternalModuleExceptFunctions()) << p->error();
    auto fe = p->function_emitter(100);
    EXPECT_FALSE(FlowClassifyCFGEdges(&fe));
    EXPECT_THAT(p->error(), Eq("Invalid backedge (50->10): does not branch to "
                               "the corresponding loop header, expected 20"));
}

TEST_F(SpvParserCFGTest, ClassifyCFGEdges_BackEdge_SingleBlockLoop) {
    auto assembly = CommonTypes() + R"(
     %100 = OpFunction %void None %voidfn

     %10 = OpLabel
     OpBranch %20

     %20 = OpLabel
     OpLoopMerge %99 %20 None
     OpBranchConditional %cond %20 %99

     %99 = OpLabel ; outer merge
     OpReturn

     OpFunctionEnd
)";
    auto p = parser(test::Assemble(assembly));
    ASSERT_TRUE(p->BuildAndParseInternalModuleExceptFunctions()) << p->error();
    auto fe = p->function_emitter(100);
    EXPECT_TRUE(FlowClassifyCFGEdges(&fe));

    auto* bi20 = fe.GetBlockInfo(20);
    ASSERT_NE(bi20, nullptr);
    EXPECT_EQ(bi20->succ_edge.count(20), 1u);
    EXPECT_EQ(bi20->succ_edge[20], EdgeKind::kBack);
}

TEST_F(SpvParserCFGTest, ClassifyCFGEdges_BackEdge_MultiBlockLoop_SingleBlockContinueConstruct) {
    auto assembly = CommonTypes() + R"(
     %100 = OpFunction %void None %voidfn

     %10 = OpLabel
     OpBranch %20

     %20 = OpLabel
     OpLoopMerge %99 %40 None
     OpBranchConditional %cond %30 %99

     %30 = OpLabel
     OpBranch %40

     %40 = OpLabel ; continue target
     OpBranch %20  ; good back edge

     %99 = OpLabel ; outer merge
     OpReturn

     OpFunctionEnd
)";
    auto p = parser(test::Assemble(assembly));
    ASSERT_TRUE(p->BuildAndParseInternalModuleExceptFunctions()) << p->error();
    auto fe = p->function_emitter(100);
    EXPECT_TRUE(FlowClassifyCFGEdges(&fe));

    auto* bi40 = fe.GetBlockInfo(40);
    ASSERT_NE(bi40, nullptr);
    EXPECT_EQ(bi40->succ_edge.count(20), 1u);
    EXPECT_EQ(bi40->succ_edge[20], EdgeKind::kBack);
}

TEST_F(SpvParserCFGTest,
       ClassifyCFGEdges_BackEdge_MultiBlockLoop_MultiBlockContinueConstruct_ContinueIsNotHeader) {
    auto assembly = CommonTypes() + R"(
     %100 = OpFunction %void None %voidfn

     %10 = OpLabel
     OpBranch %20

     %20 = OpLabel
     OpLoopMerge %99 %40 None
     OpBranchConditional %cond %30 %99

     %30 = OpLabel
     OpBranch %40

     %40 = OpLabel ; continue target
     OpBranch %50

     %50 = OpLabel
     OpBranch %20  ; good back edge

     %99 = OpLabel ; outer merge
     OpReturn

     OpFunctionEnd
)";
    auto p = parser(test::Assemble(assembly));
    ASSERT_TRUE(p->BuildAndParseInternalModuleExceptFunctions()) << p->error();
    auto fe = p->function_emitter(100);
    EXPECT_TRUE(FlowClassifyCFGEdges(&fe));

    auto* bi50 = fe.GetBlockInfo(50);
    ASSERT_NE(bi50, nullptr);
    EXPECT_EQ(bi50->succ_edge.count(20), 1u);
    EXPECT_EQ(bi50->succ_edge[20], EdgeKind::kBack);
}

TEST_F(SpvParserCFGTest,
       ClassifyCFGEdges_BackEdge_MultiBlockLoop_MultiBlockContinueConstruct_ContinueIsHeader) {
    auto assembly = CommonTypes() + R"(
     %100 = OpFunction %void None %voidfn

     %10 = OpLabel
     OpBranch %20

     %20 = OpLabel
     OpLoopMerge %99 %20 None ; continue target
     OpBranch %30

     %30 = OpLabel
     OpBranch %40

     %40 = OpLabel
     OpBranch %50

     %50 = OpLabel
     OpBranchConditional %cond %20 %99 ; good back edge

     %99 = OpLabel ; outer merge
     OpReturn

     OpFunctionEnd
)";
    auto p = parser(test::Assemble(assembly));
    ASSERT_TRUE(p->BuildAndParseInternalModuleExceptFunctions()) << p->error();
    auto fe = p->function_emitter(100);
    EXPECT_TRUE(FlowClassifyCFGEdges(&fe)) << p->error();

    auto* bi50 = fe.GetBlockInfo(50);
    ASSERT_NE(bi50, nullptr);
    EXPECT_EQ(bi50->succ_edge.count(20), 1u);
    EXPECT_EQ(bi50->succ_edge[20], EdgeKind::kBack);

    // SPIR-V 1.6 Rev 2 made this invalid SPIR-V.
    // The continue target also has the LoopMerge in it, but the continue
    // target 20 is not structurally post-dominated by the back-edge block 50.
    // Don't dump this as an end-to-end test.
    p->DeliberatelyInvalidSpirv();
}

TEST_F(SpvParserCFGTest, ClassifyCFGEdges_PrematureExitFromContinueConstruct) {
    auto assembly = CommonTypes() + R"(
     %100 = OpFunction %void None %voidfn

     %10 = OpLabel
     OpBranch %20

     %20 = OpLabel
     OpLoopMerge %99 %40 None
     OpBranchConditional %cond %30 %99

     %30 = OpLabel
     OpBranch %40

     %40 = OpLabel ; continue construct
     OpBranchConditional %cond2 %99 %50 ; invalid early exit

     %50 = OpLabel
     OpBranch %20  ; back edge

     %99 = OpLabel ; outer merge
     OpReturn

     OpFunctionEnd
)";
    auto p = parser(test::Assemble(assembly));
    ASSERT_TRUE(p->BuildAndParseInternalModuleExceptFunctions()) << p->error();
    auto fe = p->function_emitter(100);
    EXPECT_FALSE(FlowClassifyCFGEdges(&fe));
    EXPECT_THAT(p->error(), Eq("Invalid exit (40->99) from continue construct: 40 is not the "
                               "last block in the continue construct starting at 40 "
                               "(violates post-dominance rule)"));
}

TEST_F(SpvParserCFGTest, ClassifyCFGEdges_LoopBreak_FromLoopHeader_SingleBlockLoop_TrueBranch) {
    auto assembly = CommonTypes() + R"(
     %100 = OpFunction %void None %voidfn

     %10 = OpLabel
     OpBranch %20

     %20 = OpLabel    ; single block loop
     OpLoopMerge %99 %20 None
     OpBranchConditional %cond %99 %20

     %99 = OpLabel ; outer merge
     OpReturn

     OpFunctionEnd
)";
    auto p = parser(test::Assemble(assembly));
    ASSERT_TRUE(p->BuildAndParseInternalModuleExceptFunctions()) << p->error();
    auto fe = p->function_emitter(100);
    EXPECT_TRUE(FlowClassifyCFGEdges(&fe));

    auto* bi = fe.GetBlockInfo(20);
    ASSERT_NE(bi, nullptr);
    EXPECT_EQ(bi->succ_edge.count(99), 1u);
    EXPECT_EQ(bi->succ_edge[99], EdgeKind::kLoopBreak);
    EXPECT_EQ(bi->succ_edge.count(20), 1u);
    EXPECT_EQ(bi->succ_edge[20], EdgeKind::kBack);
}

TEST_F(SpvParserCFGTest, ClassifyCFGEdges_LoopBreak_FromLoopHeader_SingleBlockLoop_FalseBranch) {
    auto assembly = CommonTypes() + R"(
     %100 = OpFunction %void None %voidfn

     %10 = OpLabel
     OpBranch %20

     %20 = OpLabel    ; single block loop
     OpLoopMerge %99 %20 None
     OpBranchConditional %cond %20 %99

     %99 = OpLabel ; outer merge
     OpReturn

     OpFunctionEnd
)";
    auto p = parser(test::Assemble(assembly));
    ASSERT_TRUE(p->BuildAndParseInternalModuleExceptFunctions()) << p->error();
    auto fe = p->function_emitter(100);
    EXPECT_TRUE(FlowClassifyCFGEdges(&fe));

    auto* bi = fe.GetBlockInfo(20);
    ASSERT_NE(bi, nullptr);
    EXPECT_EQ(bi->succ_edge.count(99), 1u);
    EXPECT_EQ(bi->succ_edge[99], EdgeKind::kLoopBreak);
    EXPECT_EQ(bi->succ_edge.count(20), 1u);
    EXPECT_EQ(bi->succ_edge[20], EdgeKind::kBack);
}

TEST_F(SpvParserCFGTest, ClassifyCFGEdges_LoopBreak_FromLoopHeader_MultiBlockLoop) {
    auto assembly = CommonTypes() + R"(
     %100 = OpFunction %void None %voidfn

     %10 = OpLabel
     OpBranch %20

     %20 = OpLabel
     OpLoopMerge %99 %30 None
     OpBranchConditional %cond %30 %99

     %30 = OpLabel
     OpBranch %20

     %99 = OpLabel
     OpReturn

     OpFunctionEnd
)";
    auto p = parser(test::Assemble(assembly));
    ASSERT_TRUE(p->BuildAndParseInternalModuleExceptFunctions()) << p->error();
    auto fe = p->function_emitter(100);
    EXPECT_TRUE(FlowClassifyCFGEdges(&fe));

    auto* bi = fe.GetBlockInfo(20);
    ASSERT_NE(bi, nullptr);
    EXPECT_EQ(bi->succ_edge.count(99), 1u);
    EXPECT_EQ(bi->succ_edge[99], EdgeKind::kLoopBreak);
}

TEST_F(SpvParserCFGTest, ClassifyCFGEdges_LoopBreak_FromContinueConstructHeader) {
    auto assembly = CommonTypes() + R"(
     %100 = OpFunction %void None %voidfn

     %10 = OpLabel
     OpBranch %20

     %20 = OpLabel
     OpLoopMerge %99 %30 None
     OpBranchConditional %cond %30 %99

     %30 = OpLabel ; Single block continue construct
     OpBranchConditional %cond2 %20 %99

     %99 = OpLabel
     OpReturn

     OpFunctionEnd
)";
    auto p = parser(test::Assemble(assembly));
    ASSERT_TRUE(p->BuildAndParseInternalModuleExceptFunctions()) << p->error();
    auto fe = p->function_emitter(100);
    EXPECT_TRUE(FlowClassifyCFGEdges(&fe));

    auto* bi = fe.GetBlockInfo(30);
    ASSERT_NE(bi, nullptr);
    EXPECT_EQ(bi->succ_edge.count(99), 1u);
    EXPECT_EQ(bi->succ_edge[99], EdgeKind::kLoopBreak);
}

TEST_F(SpvParserCFGTest, ClassifyCFGEdges_IfBreak_FromIfHeader) {
    auto assembly = CommonTypes() + R"(
     %100 = OpFunction %void None %voidfn

     %10 = OpLabel
     OpSelectionMerge %99 None
     OpBranchConditional %cond %20 %99

     %20 = OpLabel
     OpBranch %99

     %99 = OpLabel
     OpReturn

     OpFunctionEnd
)";
    auto p = parser(test::Assemble(assembly));
    ASSERT_TRUE(p->BuildAndParseInternalModuleExceptFunctions()) << p->error();
    auto fe = p->function_emitter(100);
    EXPECT_TRUE(FlowClassifyCFGEdges(&fe));

    auto* bi = fe.GetBlockInfo(20);
    ASSERT_NE(bi, nullptr);
    EXPECT_EQ(bi->succ_edge.count(99), 1u);
    EXPECT_EQ(bi->succ_edge[99], EdgeKind::kIfBreak);
}

TEST_F(SpvParserCFGTest, ClassifyCFGEdges_IfBreak_FromIfThenElse) {
    auto assembly = CommonTypes() + R"(
     %100 = OpFunction %void None %voidfn

     %10 = OpLabel
     OpSelectionMerge %99 None
     OpBranchConditional %cond %20 %50

     %20 = OpLabel
     OpBranch %99

     %50 = OpLabel
     OpBranch %99

     %99 = OpLabel
     OpReturn

     OpFunctionEnd
)";
    auto p = parser(test::Assemble(assembly));
    ASSERT_TRUE(p->BuildAndParseInternalModuleExceptFunctions()) << p->error();
    auto fe = p->function_emitter(100);
    EXPECT_TRUE(FlowClassifyCFGEdges(&fe));

    // Then clause
    auto* bi20 = fe.GetBlockInfo(20);
    ASSERT_NE(bi20, nullptr);
    EXPECT_EQ(bi20->succ_edge.count(99), 1u);
    EXPECT_EQ(bi20->succ_edge[99], EdgeKind::kIfBreak);

    // Else clause
    auto* bi50 = fe.GetBlockInfo(50);
    ASSERT_NE(bi50, nullptr);
    EXPECT_EQ(bi50->succ_edge.count(99), 1u);
    EXPECT_EQ(bi50->succ_edge[99], EdgeKind::kIfBreak);
}

TEST_F(SpvParserCFGTest, ClassifyCFGEdges_IfBreak_BypassesMerge_IsError) {
    auto assembly = CommonTypes() + R"(
     %100 = OpFunction %void None %voidfn

     %10 = OpLabel
     OpSelectionMerge %50 None
     OpBranchConditional %cond %20 %50

     %20 = OpLabel
     OpBranch %99

     %50 = OpLabel ; merge
     OpBranch %99

     %99 = OpLabel
     OpReturn

     OpFunctionEnd
)";
    auto p = parser(test::Assemble(assembly));
    ASSERT_TRUE(p->BuildAndParseInternalModuleExceptFunctions()) << p->error();
    auto fe = p->function_emitter(100);
    EXPECT_FALSE(FlowClassifyCFGEdges(&fe));
    EXPECT_THAT(p->error(), Eq("Branch from block 20 to block 99 is an invalid exit from "
                               "construct starting at block 10; branch bypasses merge block 50"));
}

TEST_F(SpvParserCFGTest, ClassifyCFGEdges_IfBreak_EscapeSwitchCase_IsError) {
    // This checks one direction of that, where the IfBreak is shown it can't
    // escape a switch case.
    auto assembly = CommonTypes() + R"(
     %100 = OpFunction %void None %voidfn

     %10 = OpLabel
     OpSelectionMerge %99 None ; Set up if-break to %99
     OpBranchConditional %cond %20 %99

     %20 = OpLabel
     OpSelectionMerge %80 None ; switch-selection
     OpSwitch %selector %80 30 %30 40 %40

     %30 = OpLabel ; first case
        ; branch to %99 would be an if-break, but it bypasess the switch merge
        ; Also has case fall-through
     OpBranchConditional %cond2 %99 %40

     %40 = OpLabel ; second case
     OpBranch %80

     %80 = OpLabel ; switch-selection's merge
     OpBranch %99

     %99 = OpLabel ; if-selection's merge
     OpReturn

     OpFunctionEnd
)";
    auto p = parser(test::Assemble(assembly));
    ASSERT_TRUE(p->BuildAndParseInternalModuleExceptFunctions()) << p->error();
    auto fe = p->function_emitter(100);
    EXPECT_FALSE(FlowClassifyCFGEdges(&fe));
    EXPECT_THAT(p->error(), Eq("Branch from block 30 to block 99 is an invalid exit from "
                               "construct starting at block 20; branch bypasses merge block 80"));
}

TEST_F(SpvParserCFGTest, ClassifyCFGEdges_SwitchBreak_FromSwitchCaseDirect) {
    auto assembly = CommonTypes() + R"(
     %100 = OpFunction %void None %voidfn

     %10 = OpLabel
     OpSelectionMerge %99 None
     OpSwitch %selector %30 20 %99 ; directly to merge

     %30 = OpLabel
     OpBranch %99

     %99 = OpLabel
     OpReturn

     OpFunctionEnd
)";
    auto p = parser(test::Assemble(assembly));
    ASSERT_TRUE(p->BuildAndParseInternalModuleExceptFunctions()) << p->error();
    auto fe = p->function_emitter(100);
    EXPECT_TRUE(FlowClassifyCFGEdges(&fe));

    auto* bi = fe.GetBlockInfo(10);
    ASSERT_NE(bi, nullptr);
    EXPECT_EQ(bi->succ_edge.count(99), 1u);
    EXPECT_EQ(bi->succ_edge[99], EdgeKind::kSwitchBreak);
}

TEST_F(SpvParserCFGTest, ClassifyCFGEdges_SwitchBreak_FromSwitchCaseBody) {
    auto assembly = CommonTypes() + R"(
     %100 = OpFunction %void None %voidfn

     %10 = OpLabel
     OpSelectionMerge %99 None
     OpSwitch %selector %99 20 %20

     %20 = OpLabel
     OpBranch %99

     %99 = OpLabel
     OpReturn

     OpFunctionEnd
)";
    auto p = parser(test::Assemble(assembly));
    ASSERT_TRUE(p->BuildAndParseInternalModuleExceptFunctions()) << p->error();
    auto fe = p->function_emitter(100);
    EXPECT_TRUE(FlowClassifyCFGEdges(&fe));

    auto* bi = fe.GetBlockInfo(20);
    ASSERT_NE(bi, nullptr);
    EXPECT_EQ(bi->succ_edge.count(99), 1u);
    EXPECT_EQ(bi->succ_edge[99], EdgeKind::kSwitchBreak);
}

TEST_F(SpvParserCFGTest, ClassifyCFGEdges_SwitchBreak_FromSwitchDefaultBody) {
    auto assembly = CommonTypes() + R"(
     %100 = OpFunction %void None %voidfn

     %10 = OpLabel
     OpSelectionMerge %99 None
     OpSwitch %selector %30 20 %20

     %20 = OpLabel
     OpBranch %99

     %30 = OpLabel
     OpBranch %99

     %99 = OpLabel
     OpReturn

     OpFunctionEnd
)";
    auto p = parser(test::Assemble(assembly));
    ASSERT_TRUE(p->BuildAndParseInternalModuleExceptFunctions()) << p->error();
    auto fe = p->function_emitter(100);
    EXPECT_TRUE(FlowClassifyCFGEdges(&fe));

    auto* bi = fe.GetBlockInfo(30);
    ASSERT_NE(bi, nullptr);
    EXPECT_EQ(bi->succ_edge.count(99), 1u);
    EXPECT_EQ(bi->succ_edge[99], EdgeKind::kSwitchBreak);
}

TEST_F(SpvParserCFGTest, ClassifyCFGEdges_SwitchBreak_FromSwitchDefaultIsMerge) {
    auto assembly = CommonTypes() + R"(
     %100 = OpFunction %void None %voidfn

     %10 = OpLabel
     OpSelectionMerge %99 None
     OpSwitch %selector %99 20 %20

     %20 = OpLabel
     OpBranch %99

     %99 = OpLabel
     OpReturn

     OpFunctionEnd
)";
    auto p = parser(test::Assemble(assembly));
    ASSERT_TRUE(p->BuildAndParseInternalModuleExceptFunctions()) << p->error();
    auto fe = p->function_emitter(100);
    EXPECT_TRUE(FlowClassifyCFGEdges(&fe));

    auto* bi = fe.GetBlockInfo(10);
    ASSERT_NE(bi, nullptr);
    EXPECT_EQ(bi->succ_edge.count(99), 1u);
    EXPECT_EQ(bi->succ_edge[99], EdgeKind::kSwitchBreak);
}

TEST_F(SpvParserCFGTest, ClassifyCFGEdges_SwitchBreak_FromNestedIf_Unconditional) {
    auto assembly = CommonTypes() + R"(
     %100 = OpFunction %void None %voidfn

     %10 = OpLabel
     OpSelectionMerge %99 None
     OpSwitch %selector %99 20 %20

     %20 = OpLabel
     OpSelectionMerge %80 None
     OpBranchConditional %cond %30 %80

     %30 = OpLabel
     OpBranch %99

     %80 = OpLabel ; inner merge
     OpBranch %99

     %99 = OpLabel ; outer merge
     OpReturn

     OpFunctionEnd
)";
    auto p = parser(test::Assemble(assembly));
    ASSERT_TRUE(p->BuildAndParseInternalModuleExceptFunctions()) << p->error();
    auto fe = p->function_emitter(100);
    EXPECT_TRUE(FlowClassifyCFGEdges(&fe));

    auto* bi = fe.GetBlockInfo(30);
    ASSERT_NE(bi, nullptr);
    EXPECT_EQ(bi->succ_edge.count(99), 1u);
    EXPECT_EQ(bi->succ_edge[99], EdgeKind::kSwitchBreak);
}

TEST_F(SpvParserCFGTest, ClassifyCFGEdges_SwitchBreak_FromNestedIf_Conditional) {
    auto assembly = CommonTypes() + R"(
     %100 = OpFunction %void None %voidfn

     %10 = OpLabel
     OpSelectionMerge %99 None
     OpSwitch %selector %99 20 %20

     %20 = OpLabel
     OpSelectionMerge %80 None
     OpBranchConditional %cond %30 %80

     %30 = OpLabel
     OpBranchConditional %cond2 %99 %80 ; break-if

     %80 = OpLabel ; inner merge
     OpBranch %99

     %99 = OpLabel ; outer merge
     OpReturn

     OpFunctionEnd
)";
    auto p = parser(test::Assemble(assembly));
    ASSERT_TRUE(p->BuildAndParseInternalModuleExceptFunctions()) << p->error();
    auto fe = p->function_emitter(100);
    EXPECT_TRUE(FlowClassifyCFGEdges(&fe));

    auto* bi = fe.GetBlockInfo(30);
    ASSERT_NE(bi, nullptr);
    EXPECT_EQ(bi->succ_edge.count(99), 1u);
    EXPECT_EQ(bi->succ_edge[99], EdgeKind::kSwitchBreak);
}

TEST_F(SpvParserCFGTest, ClassifyCFGEdges_SwitchBreak_BypassesMerge_IsError) {
    auto assembly = CommonTypes() + R"(
     %100 = OpFunction %void None %voidfn

     %10 = OpLabel
     OpSelectionMerge %50 None
     OpSwitch %selector %50 20 %20

     %20 = OpLabel
     OpBranch %99 ; invalid exit

     %50 = OpLabel ; switch merge
     OpBranch %99

     %99 = OpLabel
     OpReturn

     OpFunctionEnd
)";
    auto p = parser(test::Assemble(assembly));
    ASSERT_TRUE(p->BuildAndParseInternalModuleExceptFunctions()) << p->error();
    auto fe = p->function_emitter(100);
    EXPECT_FALSE(FlowClassifyCFGEdges(&fe));
    EXPECT_THAT(p->error(), Eq("Branch from block 20 to block 99 is an invalid exit from "
                               "construct starting at block 10; branch bypasses merge block 50"));
}

TEST_F(SpvParserCFGTest, ClassifyCFGEdges_SwitchBreak_FromNestedLoop_IsError) {
    // It's an error because the break can only go as far as the loop.
    auto assembly = CommonTypes() + R"(
     %100 = OpFunction %void None %voidfn

     %10 = OpLabel
     OpSelectionMerge %99 None
     OpSwitch %selector %99 20 %20

     %20 = OpLabel
     OpLoopMerge %80 %70 None
     OpBranchConditional %cond %30 %80

     %30 = OpLabel ; in loop construct
     OpBranch %99 ; break

     %70 = OpLabel
     OpBranch %20

     %80 = OpLabel
     OpBranch %99

     %99 = OpLabel ; outer merge
     OpReturn

     OpFunctionEnd
)";
    auto p = parser(test::Assemble(assembly));
    ASSERT_TRUE(p->BuildAndParseInternalModuleExceptFunctions()) << p->error();
    auto fe = p->function_emitter(100);
    EXPECT_FALSE(FlowClassifyCFGEdges(&fe));
    EXPECT_THAT(p->error(), Eq("Branch from block 30 to block 99 is an invalid exit from "
                               "construct starting at block 20; branch bypasses merge block 80"));
}

TEST_F(SpvParserCFGTest, ClassifyCFGEdges_SwitchBreak_FromNestedSwitch_IsError) {
    // It's an error because the break can only go as far as inner switch
    auto assembly = CommonTypes() + R"(
     %100 = OpFunction %void None %voidfn

     %10 = OpLabel
     OpSelectionMerge %99 None
     OpSwitch %selector %99 20 %20

     %20 = OpLabel
     OpSelectionMerge %80 None
     OpSwitch %selector %80 30 %30

     %30 = OpLabel
     OpBranch %99 ; break

     %80 = OpLabel
     OpBranch %99

     %99 = OpLabel ; outer merge
     OpReturn

     OpFunctionEnd
)";
    auto p = parser(test::Assemble(assembly));
    ASSERT_TRUE(p->BuildAndParseInternalModuleExceptFunctions()) << p->error();
    auto fe = p->function_emitter(100);
    EXPECT_FALSE(FlowClassifyCFGEdges(&fe));
    EXPECT_THAT(p->error(), Eq("Branch from block 30 to block 99 is an invalid exit from "
                               "construct starting at block 20; branch bypasses merge block 80"));
}

TEST_F(SpvParserCFGTest, ClassifyCFGEdges_LoopBreak_FromLoopBody) {
    auto assembly = CommonTypes() + R"(
     %100 = OpFunction %void None %voidfn

     %10 = OpLabel
     OpBranch %20

     %20 = OpLabel
     OpLoopMerge %99 %50 None
     OpBranchConditional %cond %30 %99

     %30 = OpLabel
     OpBranchConditional %cond2 %50 %99 ; break-unless

     %50 = OpLabel
     OpBranch %20

     %99 = OpLabel
     OpReturn

     OpFunctionEnd
)";
    auto p = parser(test::Assemble(assembly));
    ASSERT_TRUE(p->BuildAndParseInternalModuleExceptFunctions()) << p->error();
    auto fe = p->function_emitter(100);
    EXPECT_TRUE(FlowClassifyCFGEdges(&fe));

    auto* bi = fe.GetBlockInfo(30);
    ASSERT_NE(bi, nullptr);
    EXPECT_EQ(bi->succ_edge.count(99), 1u);
    EXPECT_EQ(bi->succ_edge[99], EdgeKind::kLoopBreak);
}

TEST_F(SpvParserCFGTest, ClassifyCFGEdges_LoopBreak_FromContinueConstructTail) {
    auto assembly = CommonTypes() + R"(
     %100 = OpFunction %void None %voidfn

     %10 = OpLabel
     OpBranch %20

     %20 = OpLabel
     OpLoopMerge %99 %50 None
     OpBranchConditional %cond %30 %99

     %30 = OpLabel
     OpBranch %50

     %50 = OpLabel ; continue target
     OpBranch %60

     %60 = OpLabel ; continue construct tail
     OpBranchConditional %cond2 %20 %99

     %99 = OpLabel
     OpReturn

     OpFunctionEnd
)";
    auto p = parser(test::Assemble(assembly));
    ASSERT_TRUE(p->BuildAndParseInternalModuleExceptFunctions()) << p->error();
    auto fe = p->function_emitter(100);
    EXPECT_TRUE(FlowClassifyCFGEdges(&fe));

    auto* bi = fe.GetBlockInfo(60);
    ASSERT_NE(bi, nullptr);
    EXPECT_EQ(bi->succ_edge.count(99), 1u);
    EXPECT_EQ(bi->succ_edge[99], EdgeKind::kLoopBreak);
}

TEST_F(SpvParserCFGTest, ClassifyCFGEdges_LoopBreak_FromLoopBodyDirect) {
    auto assembly = CommonTypes() + R"(
     %100 = OpFunction %void None %voidfn

     %10 = OpLabel
     OpBranch %20

     %20 = OpLabel
     OpLoopMerge %99 %50 None
     OpBranchConditional %cond %30 %99

     %30 = OpLabel
     OpBranch %99  ; unconditional break

     %50 = OpLabel
     OpBranch %20

     %99 = OpLabel
     OpReturn

     OpFunctionEnd
)";
    auto p = parser(test::Assemble(assembly));
    ASSERT_TRUE(p->BuildAndParseInternalModuleExceptFunctions()) << p->error();
    auto fe = p->function_emitter(100);
    EXPECT_TRUE(FlowClassifyCFGEdges(&fe));

    auto* bi = fe.GetBlockInfo(30);
    ASSERT_NE(bi, nullptr);
    EXPECT_EQ(bi->succ_edge.count(99), 1u);
    EXPECT_EQ(bi->succ_edge[99], EdgeKind::kLoopBreak);
}

TEST_F(SpvParserCFGTest, ClassifyCFGEdges_LoopBreak_FromLoopBodyNestedSelection_Unconditional) {
    auto assembly = CommonTypes() + R"(
     %100 = OpFunction %void None %voidfn

     %10 = OpLabel
     OpBranch %20

     %20 = OpLabel
     OpLoopMerge %99 %80 None
     OpBranchConditional %cond %30 %99

     %30 = OpLabel
     OpSelectionMerge %50 None
     OpBranchConditional %cond2 %40 %50

     %40 = OpLabel
     OpBranch %99 ; deeply nested break

     %50 = OpLabel ; inner merge
     OpBranch %80

     %80 = OpLabel
     OpBranch %20  ; backedge

     %99 = OpLabel
     OpReturn

     OpFunctionEnd
)";
    auto p = parser(test::Assemble(assembly));
    ASSERT_TRUE(p->BuildAndParseInternalModuleExceptFunctions()) << p->error();
    auto fe = p->function_emitter(100);
    EXPECT_TRUE(FlowClassifyCFGEdges(&fe));

    auto* bi = fe.GetBlockInfo(40);
    ASSERT_NE(bi, nullptr);
    EXPECT_EQ(bi->succ_edge.count(99), 1u);
    EXPECT_EQ(bi->succ_edge[99], EdgeKind::kLoopBreak);
}

TEST_F(SpvParserCFGTest, ClassifyCFGEdges_LoopBreak_FromLoopBodyNestedSelection_Conditional) {
    auto assembly = CommonTypes() + R"(
     %100 = OpFunction %void None %voidfn

     %10 = OpLabel
     OpBranch %20

     %20 = OpLabel
     OpLoopMerge %99 %80 None
     OpBranchConditional %cond %30 %99

     %30 = OpLabel
     OpSelectionMerge %50 None
     OpBranchConditional %cond2 %40 %50

     %40 = OpLabel
     OpBranchConditional %cond3 %99 %50 ; break-if

     %50 = OpLabel ; inner merge
     OpBranch %80

     %80 = OpLabel
     OpBranch %20  ; backedge

     %99 = OpLabel
     OpReturn

     OpFunctionEnd
)";
    auto p = parser(test::Assemble(assembly));
    ASSERT_TRUE(p->BuildAndParseInternalModuleExceptFunctions()) << p->error();
    auto fe = p->function_emitter(100);
    EXPECT_TRUE(FlowClassifyCFGEdges(&fe));

    auto* bi = fe.GetBlockInfo(40);
    ASSERT_NE(bi, nullptr);
    EXPECT_EQ(bi->succ_edge.count(99), 1u);
    EXPECT_EQ(bi->succ_edge[99], EdgeKind::kLoopBreak);
}

TEST_F(SpvParserCFGTest, ClassifyCFGEdges_LoopBreak_FromContinueConstructNestedFlow_IsError) {
    auto assembly = CommonTypes() + R"(
     %100 = OpFunction %void None %voidfn

     %10 = OpLabel
     OpBranch %20

     %20 = OpLabel
     OpLoopMerge %99 %40 None
     OpBranchConditional %cond %30 %99

     %30 = OpLabel
     OpBranch %40

     %40 = OpLabel ; continue construct
     OpSelectionMerge %79 None
     OpBranchConditional %cond2 %50 %79

     %50 = OpLabel
     OpBranchConditional %cond3 %99 %79 ; attempt to break to 99 should fail

     %79 = OpLabel
     OpBranch %80  ; inner merge

     %80 = OpLabel
     OpBranch %20  ; backedge

     %99 = OpLabel
     OpReturn

     OpFunctionEnd
)";
    auto p = parser(test::Assemble(assembly));
    ASSERT_TRUE(p->BuildAndParseInternalModuleExceptFunctions()) << p->error();
    auto fe = p->function_emitter(100);
    EXPECT_FALSE(FlowClassifyCFGEdges(&fe));
    EXPECT_THAT(p->error(), Eq("Invalid exit (50->99) from continue construct: 50 is not the "
                               "last block in the continue construct starting at 40 "
                               "(violates post-dominance rule)"));
}

TEST_F(SpvParserCFGTest, ClassifyCFGEdges_LoopBreak_FromLoopBypassesMerge_IsError) {
    auto assembly = CommonTypes() + R"(
     %100 = OpFunction %void None %voidfn

     %10 = OpLabel
     OpBranch %20

     %20 = OpLabel
     OpLoopMerge %50 %40 None
     OpBranchConditional %cond %30 %50

     %30 = OpLabel
     OpBranch %99 ; bad exit

     %40 = OpLabel ; continue construct
     OpBranch %20

     %50 = OpLabel
     OpBranch %99

     %99 = OpLabel
     OpReturn

     OpFunctionEnd
)";
    auto p = parser(test::Assemble(assembly));
    ASSERT_TRUE(p->BuildAndParseInternalModuleExceptFunctions()) << p->error();
    auto fe = p->function_emitter(100);
    EXPECT_FALSE(FlowClassifyCFGEdges(&fe));
    EXPECT_THAT(p->error(), Eq("Branch from block 30 to block 99 is an invalid exit from "
                               "construct starting at block 20; branch bypasses merge block 50"));
}

TEST_F(SpvParserCFGTest, ClassifyCFGEdges_LoopBreak_FromContinueBypassesMerge_IsError) {
    auto assembly = CommonTypes() + R"(
     %100 = OpFunction %void None %voidfn

     %10 = OpLabel
     OpBranch %20

     %20 = OpLabel
     OpLoopMerge %50 %40 None
     OpBranchConditional %cond %30 %50

     %30 = OpLabel
     OpBranch %40

     %40 = OpLabel ; continue construct
     OpBranch %45

     %45 = OpLabel
     OpBranchConditional %cond2 %20 %99 ; branch to %99 is bad exit

     %50 = OpLabel
     OpBranch %99

     %99 = OpLabel
     OpReturn

     OpFunctionEnd
)";
    auto p = parser(test::Assemble(assembly));
    ASSERT_TRUE(p->BuildAndParseInternalModuleExceptFunctions()) << p->error();
    auto fe = p->function_emitter(100);
    EXPECT_FALSE(FlowClassifyCFGEdges(&fe));
    EXPECT_THAT(p->error(), Eq("Branch from block 45 to block 99 is an invalid exit from "
                               "construct starting at block 40; branch bypasses merge block 50"));
}

TEST_F(SpvParserCFGTest, ClassifyCFGEdges_LoopContinue_LoopBodyToContinue) {
    auto assembly = CommonTypes() + R"(
     %100 = OpFunction %void None %voidfn

     %10 = OpLabel
     OpBranch %20

     %20 = OpLabel
     OpLoopMerge %99 %80 None
     OpBranchConditional %cond %30 %99

     %30 = OpLabel
     OpBranch %80 ; a forward edge

     %80 = OpLabel
     OpBranch %20

     %99 = OpLabel
     OpReturn

     OpFunctionEnd
)";
    auto p = parser(test::Assemble(assembly));
    ASSERT_TRUE(p->BuildAndParseInternalModuleExceptFunctions()) << p->error();
    auto fe = p->function_emitter(100);
    EXPECT_TRUE(FlowClassifyCFGEdges(&fe));

    auto* bi = fe.GetBlockInfo(30);
    ASSERT_NE(bi, nullptr);
    EXPECT_EQ(bi->succ_edge.count(80), 1u);
    EXPECT_EQ(bi->succ_edge[80], EdgeKind::kLoopContinue);
}

TEST_F(SpvParserCFGTest, ClassifyCFGEdges_LoopContinue_FromNestedIf) {
    auto assembly = CommonTypes() + R"(
     %100 = OpFunction %void None %voidfn

     %10 = OpLabel
     OpBranch %20

     %20 = OpLabel
     OpLoopMerge %99 %80 None
     OpBranchConditional %cond %30 %99

     %30 = OpLabel
     OpSelectionMerge %79 None
     OpBranchConditional %cond2 %40 %79

     %40 = OpLabel
     OpBranch %80 ; continue

     %79 = OpLabel ; inner merge
     OpBranch %80

     %80 = OpLabel ; continue target
     OpBranch %20

     %99 = OpLabel
     OpReturn

     OpFunctionEnd
)";
    auto p = parser(test::Assemble(assembly));
    ASSERT_TRUE(p->BuildAndParseInternalModuleExceptFunctions()) << p->error();
    auto fe = p->function_emitter(100);
    EXPECT_TRUE(FlowClassifyCFGEdges(&fe));

    auto* bi = fe.GetBlockInfo(40);
    ASSERT_NE(bi, nullptr);
    EXPECT_EQ(bi->succ_edge.count(80), 1u);
    EXPECT_EQ(bi->succ_edge[80], EdgeKind::kLoopContinue);
}

TEST_F(SpvParserCFGTest, ClassifyCFGEdges_LoopContinue_ConditionalFromNestedIf) {
    auto assembly = CommonTypes() + R"(
     %100 = OpFunction %void None %voidfn

     %10 = OpLabel
     OpBranch %20

     %20 = OpLabel
     OpLoopMerge %99 %80 None
     OpBranchConditional %cond %30 %99

     %30 = OpLabel
     OpSelectionMerge %79 None
     OpBranchConditional %cond2 %40 %79

     %40 = OpLabel
     OpBranchConditional %cond2 %80 %79 ; continue-if

     %79 = OpLabel ; inner merge
     OpBranch %80

     %80 = OpLabel ; continue target
     OpBranch %20

     %99 = OpLabel
     OpReturn

     OpFunctionEnd
)";
    auto p = parser(test::Assemble(assembly));
    ASSERT_TRUE(p->BuildAndParseInternalModuleExceptFunctions()) << p->error();
    auto fe = p->function_emitter(100);
    EXPECT_TRUE(FlowClassifyCFGEdges(&fe));

    auto* bi = fe.GetBlockInfo(40);
    ASSERT_NE(bi, nullptr);
    EXPECT_EQ(bi->succ_edge.count(80), 1u);
    EXPECT_EQ(bi->succ_edge[80], EdgeKind::kLoopContinue);
}

TEST_F(SpvParserCFGTest, ClassifyCFGEdges_LoopContinue_FromNestedSwitchCaseBody_Unconditional) {
    auto assembly = CommonTypes() + R"(
     %100 = OpFunction %void None %voidfn

     %10 = OpLabel
     OpBranch %20

     %20 = OpLabel
     OpLoopMerge %99 %80 None
     OpBranchConditional %cond %30 %99

     %30 = OpLabel
     OpSelectionMerge %79 None
     OpSwitch %selector %79 40 %40

     %40 = OpLabel
     OpBranch %80

     %79 = OpLabel ; inner merge
     OpBranch %80

     %80 = OpLabel ; continue target
     OpBranch %20

     %99 = OpLabel
     OpReturn

     OpFunctionEnd
)";
    auto p = parser(test::Assemble(assembly));
    ASSERT_TRUE(p->BuildAndParseInternalModuleExceptFunctions()) << p->error();
    auto fe = p->function_emitter(100);
    EXPECT_TRUE(FlowClassifyCFGEdges(&fe)) << p->error();

    auto* bi = fe.GetBlockInfo(40);
    ASSERT_NE(bi, nullptr);
    EXPECT_EQ(bi->succ_edge.count(80), 1u);
    EXPECT_EQ(bi->succ_edge[80], EdgeKind::kLoopContinue);
}

TEST_F(SpvParserCFGTest, ClassifyCFGEdges_LoopContinue_FromNestedSwitchCaseDirect_IsError) {
    auto assembly = CommonTypes() + R"(
     %100 = OpFunction %void None %voidfn

     %10 = OpLabel
     OpBranch %20

     %20 = OpLabel
     OpLoopMerge %99 %80 None
     OpBranchConditional %cond %30 %99

     %30 = OpLabel
     OpSelectionMerge %79 None
     OpSwitch %selector %79 40 %80 ; continue here

     %79 = OpLabel ; inner merge
     OpBranch %80

     %80 = OpLabel ; continue target
     OpBranch %20

     %99 = OpLabel
     OpReturn

     OpFunctionEnd
)";
    auto p = parser(test::Assemble(assembly));
    ASSERT_TRUE(p->BuildAndParseInternalModuleExceptFunctions()) << p->error();
    auto fe = p->function_emitter(100);
    fe.RegisterBasicBlocks();
    fe.ComputeBlockOrderAndPositions();
    EXPECT_TRUE(fe.VerifyHeaderContinueMergeOrder());
    EXPECT_TRUE(fe.RegisterMerges());
    EXPECT_TRUE(fe.LabelControlFlowConstructs());
    EXPECT_FALSE(fe.FindSwitchCaseHeaders());
    EXPECT_THAT(p->error(), Eq("Switch branch from block 30 to case target block "
                               "80 escapes the selection construct"));
}

TEST_F(SpvParserCFGTest, ClassifyCFGEdges_LoopContinue_FromNestedSwitchDefaultDirect_IsError) {
    auto assembly = CommonTypes() + R"(
     %100 = OpFunction %void None %voidfn

     %10 = OpLabel
     OpBranch %20

     %20 = OpLabel
     OpLoopMerge %99 %80 None
     OpBranchConditional %cond %30 %99

     %30 = OpLabel
     OpSelectionMerge %79 None
     OpSwitch %selector %80 40 %79 ; continue here

     %79 = OpLabel ; inner merge
     OpBranch %80

     %80 = OpLabel ; continue target
     OpBranch %20

     %99 = OpLabel
     OpReturn

     OpFunctionEnd
)";
    auto p = parser(test::Assemble(assembly));
    ASSERT_TRUE(p->BuildAndParseInternalModuleExceptFunctions()) << p->error();
    auto fe = p->function_emitter(100);
    fe.RegisterBasicBlocks();
    fe.ComputeBlockOrderAndPositions();
    EXPECT_TRUE(fe.VerifyHeaderContinueMergeOrder());
    EXPECT_TRUE(fe.RegisterMerges());
    EXPECT_TRUE(fe.LabelControlFlowConstructs());
    EXPECT_FALSE(fe.FindSwitchCaseHeaders());
    EXPECT_THAT(p->error(), Eq("Switch branch from block 30 to default block 80 "
                               "escapes the selection construct"));
}

TEST_F(SpvParserCFGTest, ClassifyCFGEdges_LoopContinue_FromNestedSwitchDefaultBody_Conditional) {
    auto assembly = CommonTypes() + R"(
     %100 = OpFunction %void None %voidfn

     %10 = OpLabel
     OpBranch %20

     %20 = OpLabel
     OpLoopMerge %99 %80 None
     OpBranchConditional %cond %30 %99

     %30 = OpLabel
     OpSelectionMerge %79 None
     OpSwitch %selector %40 79 %79

     %40 = OpLabel
     OpBranchConditional %cond2 %80 %79

     %79 = OpLabel ; inner merge
     OpBranch %80

     %80 = OpLabel ; continue target
     OpBranch %20

     %99 = OpLabel
     OpReturn

     OpFunctionEnd
)";
    auto p = parser(test::Assemble(assembly));
    ASSERT_TRUE(p->BuildAndParseInternalModuleExceptFunctions()) << p->error();
    auto fe = p->function_emitter(100);
    EXPECT_TRUE(FlowClassifyCFGEdges(&fe)) << p->error();

    auto* bi = fe.GetBlockInfo(40);
    ASSERT_NE(bi, nullptr);
    EXPECT_EQ(bi->succ_edge.count(80), 1u);
    EXPECT_EQ(bi->succ_edge[80], EdgeKind::kLoopContinue);
}

TEST_F(SpvParserCFGTest, ClassifyCFGEdges_LoopContinue_FromNestedSwitchDefaultBody_Unconditional) {
    auto assembly = CommonTypes() + R"(
     %100 = OpFunction %void None %voidfn

     %10 = OpLabel
     OpBranch %20

     %20 = OpLabel
     OpLoopMerge %99 %80 None
     OpBranchConditional %cond %30 %99

     %30 = OpLabel
     OpSelectionMerge %79 None
     OpSwitch %selector %40 79 %79

     %40 = OpLabel
     OpBranch %80

     %79 = OpLabel ; inner merge
     OpBranch %80

     %80 = OpLabel ; continue target
     OpBranch %20

     %99 = OpLabel
     OpReturn

     OpFunctionEnd
)";
    auto p = parser(test::Assemble(assembly));
    ASSERT_TRUE(p->BuildAndParseInternalModuleExceptFunctions()) << p->error();
    auto fe = p->function_emitter(100);
    EXPECT_TRUE(FlowClassifyCFGEdges(&fe));

    auto* bi = fe.GetBlockInfo(40);
    ASSERT_NE(bi, nullptr);
    EXPECT_EQ(bi->succ_edge.count(80), 1u);
    EXPECT_EQ(bi->succ_edge[80], EdgeKind::kLoopContinue);
}

TEST_F(SpvParserCFGTest, ClassifyCFGEdges_LoopContinue_FromNestedLoopHeader_IsError) {
    // Inner loop header tries to do continue to outer loop continue target.
    // This is disallowed by the rule:
    //    "a continue block is valid only for the innermost loop it is nested
    //    inside of"
    auto assembly = CommonTypes() + R"(
     %100 = OpFunction %void None %voidfn

     %10 = OpLabel
     OpBranch %20

     %20 = OpLabel
     OpStore %var %uint_1
     OpLoopMerge %99 %80 None
     OpBranchConditional %cond %30 %99

     %30 = OpLabel ; inner loop.
     OpStore %var %uint_1
     OpLoopMerge %59 %50 None
     OpBranchConditional %cond %59 %80  ; break and outer continue

     %50 = OpLabel
     OpStore %var %uint_2
     OpBranch %30 ; inner backedge

     %59 = OpLabel ; inner merge
     OpStore %var %uint_3
     OpBranch %80

     %80 = OpLabel ; outer continue
     OpStore %var %uint_4
     OpBranch %20 ; outer backedge

     %99 = OpLabel
     OpStore %var %uint_5
     OpReturn

     OpFunctionEnd
  )";
    auto p = parser(test::Assemble(assembly));
    ASSERT_TRUE(p->BuildAndParseInternalModuleExceptFunctions()) << p->error();
    auto fe = p->function_emitter(100);
    EXPECT_FALSE(FlowClassifyCFGEdges(&fe));
    EXPECT_THAT(p->error(), Eq("Branch from block 30 to block 80 is an invalid exit from construct "
                               "starting at block 30; branch bypasses merge block 59"));
}

TEST_F(SpvParserCFGTest, ClassifyCFGEdges_Forward_IfToThen) {
    auto assembly = CommonTypes() + R"(
     %100 = OpFunction %void None %voidfn

     %10 = OpLabel
     OpSelectionMerge %99 None
     OpBranchConditional %cond %20 %99

     %20 = OpLabel
     OpBranch %99

     %99 = OpLabel
     OpReturn

     OpFunctionEnd
)";
    auto p = parser(test::Assemble(assembly));
    ASSERT_TRUE(p->BuildAndParseInternalModuleExceptFunctions()) << p->error();
    auto fe = p->function_emitter(100);
    EXPECT_TRUE(FlowClassifyCFGEdges(&fe));

    auto* bi = fe.GetBlockInfo(10);
    ASSERT_NE(bi, nullptr);
    EXPECT_EQ(bi->succ_edge.count(20), 1u);
    EXPECT_EQ(bi->succ_edge[20], EdgeKind::kForward);
}

TEST_F(SpvParserCFGTest, ClassifyCFGEdges_Forward_IfToElse) {
    auto assembly = CommonTypes() + R"(
     %100 = OpFunction %void None %voidfn

     %10 = OpLabel
     OpSelectionMerge %99 None
     OpBranchConditional %cond %99 %30

     %30 = OpLabel
     OpBranch %99

     %99 = OpLabel
     OpReturn

     OpFunctionEnd
)";
    auto p = parser(test::Assemble(assembly));
    ASSERT_TRUE(p->BuildAndParseInternalModuleExceptFunctions()) << p->error();
    auto fe = p->function_emitter(100);
    EXPECT_TRUE(FlowClassifyCFGEdges(&fe));

    auto* bi = fe.GetBlockInfo(10);
    ASSERT_NE(bi, nullptr);
    EXPECT_EQ(bi->succ_edge.count(30), 1u);
    EXPECT_EQ(bi->succ_edge[30], EdgeKind::kForward);
}

TEST_F(SpvParserCFGTest, ClassifyCFGEdges_Forward_SwitchToCase) {
    auto assembly = CommonTypes() + R"(
     %100 = OpFunction %void None %voidfn

     %10 = OpLabel
     OpSelectionMerge %99 None
     OpSwitch %selector %99 20 %20

     %20 = OpLabel
     OpBranch %99

     %99 = OpLabel
     OpReturn

     OpFunctionEnd
)";
    auto p = parser(test::Assemble(assembly));
    ASSERT_TRUE(p->BuildAndParseInternalModuleExceptFunctions()) << p->error();
    auto fe = p->function_emitter(100);
    EXPECT_TRUE(FlowClassifyCFGEdges(&fe));

    auto* bi = fe.GetBlockInfo(10);
    ASSERT_NE(bi, nullptr);
    EXPECT_EQ(bi->succ_edge.count(20), 1u);
    EXPECT_EQ(bi->succ_edge[20], EdgeKind::kForward);
}

TEST_F(SpvParserCFGTest, ClassifyCFGEdges_Forward_SwitchToDefaultNotMerge) {
    auto assembly = CommonTypes() + R"(
     %100 = OpFunction %void None %voidfn

     %10 = OpLabel
     OpSelectionMerge %99 None
     OpSwitch %selector %30 20 %20

     %20 = OpLabel
     OpBranch %99

     %30 = OpLabel
     OpBranch %99

     %99 = OpLabel
     OpReturn

     OpFunctionEnd
)";
    auto p = parser(test::Assemble(assembly));
    ASSERT_TRUE(p->BuildAndParseInternalModuleExceptFunctions()) << p->error();
    auto fe = p->function_emitter(100);
    EXPECT_TRUE(FlowClassifyCFGEdges(&fe));

    auto* bi = fe.GetBlockInfo(10);
    ASSERT_NE(bi, nullptr);
    EXPECT_EQ(bi->succ_edge.count(30), 1u);
    EXPECT_EQ(bi->succ_edge[30], EdgeKind::kForward);
}

TEST_F(SpvParserCFGTest, ClassifyCFGEdges_Forward_LoopHeadToBody) {
    auto assembly = CommonTypes() + R"(
     %100 = OpFunction %void None %voidfn

     %10 = OpLabel
     OpBranch %20

     %20 = OpLabel
     OpLoopMerge %99 %80 None
     OpBranchConditional %cond %30 %99

     %30 = OpLabel
     OpBranch %80

     %80 = OpLabel
     OpBranch %20

     %99 = OpLabel
     OpReturn

     OpFunctionEnd
)";
    auto p = parser(test::Assemble(assembly));
    ASSERT_TRUE(p->BuildAndParseInternalModuleExceptFunctions()) << p->error();
    auto fe = p->function_emitter(100);
    EXPECT_TRUE(FlowClassifyCFGEdges(&fe));

    auto* bi = fe.GetBlockInfo(20);
    ASSERT_NE(bi, nullptr);
    EXPECT_EQ(bi->succ_edge.count(30), 1u);
    EXPECT_EQ(bi->succ_edge[30], EdgeKind::kForward);
}

TEST_F(SpvParserCFGTest, ClassifyCFGEdges_DomViolation_BeforeIfToSelectionInterior) {
    auto assembly = CommonTypes() + R"(
     %100 = OpFunction %void None %voidfn

     %10 = OpLabel
     OpSelectionMerge %99 None
     OpBranchConditional %cond %20 %50 ;%50 is a bad branch

     %20 = OpLabel
     OpSelectionMerge %89 None
     OpBranchConditional %cond %50 %89

     %50 = OpLabel
     OpBranch %89

     %89 = OpLabel
     OpBranch %99

     %99 = OpLabel
     OpReturn

     OpFunctionEnd
)";
    auto p = parser(test::Assemble(assembly));
    ASSERT_TRUE(p->BuildAndParseInternalModuleExceptFunctions()) << p->error();
    auto fe = p->function_emitter(100);
    EXPECT_FALSE(FlowClassifyCFGEdges(&fe));
    EXPECT_THAT(p->error(),
                Eq("Branch from 10 to 50 bypasses header 20 (dominance rule violated)"));
}

TEST_F(SpvParserCFGTest, ClassifyCFGEdges_DomViolation_BeforeSwitchToSelectionInterior) {
    auto assembly = CommonTypes() + R"(
     %100 = OpFunction %void None %voidfn

     %10 = OpLabel
     OpSelectionMerge %99 None
     OpBranchConditional %cond %20 %50 ;%50 is a bad branch

     %20 = OpLabel
     OpSelectionMerge %89 None
     OpSwitch %selector %89 50 %50

     %50 = OpLabel
     OpBranch %89

     %89 = OpLabel
     OpBranch %99

     %99 = OpLabel
     OpReturn

     OpFunctionEnd
)";
    auto p = parser(test::Assemble(assembly));
    ASSERT_TRUE(p->BuildAndParseInternalModuleExceptFunctions()) << p->error();
    auto fe = p->function_emitter(100);
    EXPECT_FALSE(FlowClassifyCFGEdges(&fe));
    EXPECT_THAT(p->error(),
                Eq("Branch from 10 to 50 bypasses header 20 (dominance rule violated)"));
}

TEST_F(SpvParserCFGTest, ClassifyCFGEdges_DomViolation_BeforeLoopToLoopBodyInterior) {
    auto assembly = CommonTypes() + R"(
     %100 = OpFunction %void None %voidfn

     %10 = OpLabel
     OpSelectionMerge %99 None
     OpBranchConditional %cond %20 %50 ;%50 is a bad branch

     %20 = OpLabel
     OpLoopMerge %89 %80 None
     OpBranchConditional %cond %50 %89

     %50 = OpLabel
     OpBranch %89

     %80 = OpLabel
     OpBranch %20

     %89 = OpLabel
     OpBranch %99

     %99 = OpLabel
     OpReturn

     OpFunctionEnd
)";
    auto p = parser(test::Assemble(assembly));
    ASSERT_TRUE(p->BuildAndParseInternalModuleExceptFunctions()) << p->error();
    auto fe = p->function_emitter(100);
    EXPECT_FALSE(FlowClassifyCFGEdges(&fe));
    EXPECT_THAT(p->error(),
                // Weird error, but still we caught it.
                // Preferred: Eq("Branch from 10 to 50 bypasses header 20
                // (dominance rule violated)"))
                Eq("Branch from 10 to 50 bypasses continue target 80 (dominance "
                   "rule violated)"))
        << Dump(fe.block_order());
}

TEST_F(SpvParserCFGTest, ClassifyCFGEdges_DomViolation_BeforeContinueToContinueInterior) {
    auto assembly = CommonTypes() + R"(
     %100 = OpFunction %void None %voidfn

     %10 = OpLabel
     OpBranch %20

     %20 = OpLabel
     OpLoopMerge %99 %50 None
     OpBranchConditional %cond %30 %99

     %30 = OpLabel
     OpBranch %60

     %50 = OpLabel ; continue target
     OpBranch %60

     %60 = OpLabel
     OpBranch %20

     %89 = OpLabel
     OpBranch %99

     %99 = OpLabel
     OpReturn

     OpFunctionEnd
)";
    auto p = parser(test::Assemble(assembly));
    ASSERT_TRUE(p->BuildAndParseInternalModuleExceptFunctions()) << p->error();
    auto fe = p->function_emitter(100);
    EXPECT_FALSE(FlowClassifyCFGEdges(&fe));
    EXPECT_THAT(p->error(),
                Eq("Branch from block 30 to block 60 is an invalid exit from "
                   "construct starting at block 20; branch bypasses continue target 50"));
}

TEST_F(SpvParserCFGTest, ClassifyCFGEdges_DomViolation_AfterContinueToContinueInterior) {
    auto assembly = CommonTypes() + R"(
     %100 = OpFunction %void None %voidfn

     %10 = OpLabel
     OpBranch %20

     %20 = OpLabel
     OpLoopMerge %80 %50 None
     OpBranchConditional %cond %30 %80

     %30 = OpLabel
     OpBranch %50

     %50 = OpLabel
     OpBranch %60

     %60 = OpLabel
     OpBranch %20

     %80 = OpLabel
     OpBranch %60 ; bad branch
)";
    auto p = parser(test::Assemble(assembly));
    ASSERT_TRUE(p->BuildAndParseInternalModuleExceptFunctions()) << p->error();
    auto fe = p->function_emitter(100);
    EXPECT_FALSE(FlowClassifyCFGEdges(&fe));
    EXPECT_THAT(p->error(), Eq("Branch from block 50 to block 60 is an invalid exit from "
                               "construct starting at block 50; branch bypasses merge block 80"));
}

TEST_F(SpvParserCFGTest,
       FindSwitchCaseHeaders_DomViolation_SwitchCase_CantBeMergeForOtherConstruct) {
    auto assembly = CommonTypes() + R"(
     %100 = OpFunction %void None %voidfn

     %10 = OpLabel
     OpSelectionMerge %99 None
     OpSwitch %selector %99 20 %20 50 %50

     %20 = OpLabel
     OpSelectionMerge %50 None
     OpBranchConditional %cond %30 %50

     %30 = OpLabel
     OpBranch %50

     %50 = OpLabel ; case and merge block. Error
     OpBranch %99

     %99 = OpLabel
     OpReturn

     OpFunctionEnd
)";
    auto p = parser(test::Assemble(assembly));
    ASSERT_TRUE(p->BuildAndParseInternalModuleExceptFunctions()) << p->error();
    auto fe = p->function_emitter(100);
    EXPECT_FALSE(FlowFindSwitchCaseHeaders(&fe));
    EXPECT_THAT(p->error(), Eq("Block 50 is a case block for switch-selection header 10 and "
                               "also the merge block for 20 (violates dominance rule)"));
}

TEST_F(SpvParserCFGTest, ClassifyCFGEdges_DomViolation_SwitchDefault_CantBeMergeForOtherConstruct) {
    auto assembly = CommonTypes() + R"(
     %100 = OpFunction %void None %voidfn

     %10 = OpLabel
     OpSelectionMerge %99 None
     OpSwitch %selector %50 20 %20

     %20 = OpLabel
     OpSelectionMerge %50 None
     OpBranchConditional %cond %30 %50

     %30 = OpLabel
     OpBranch %50

     %50 = OpLabel ; default-case and merge block. Error
     OpBranch %99

     %99 = OpLabel
     OpReturn

     OpFunctionEnd
)";
    auto p = parser(test::Assemble(assembly));
    ASSERT_TRUE(p->BuildAndParseInternalModuleExceptFunctions()) << p->error();
    auto fe = p->function_emitter(100);
    EXPECT_FALSE(FlowFindSwitchCaseHeaders(&fe));
    EXPECT_THAT(p->error(), Eq("Block 50 is the default block for switch-selection header 10 "
                               "and also the merge block for 20 (violates dominance rule)"));
}

TEST_F(SpvParserCFGTest, ClassifyCFGEdges_TooManyBackedges) {
    auto assembly = CommonTypes() + R"(
     %100 = OpFunction %void None %voidfn

     %10 = OpLabel
     OpBranch %20

     %20 = OpLabel
     OpLoopMerge %99 %50 None
     OpBranchConditional %cond %30 %99

     %30 = OpLabel
     OpBranchConditional %cond2 %20 %50

     %50 = OpLabel
     OpBranch %20

     %99 = OpLabel
     OpReturn

     OpFunctionEnd
)";
    auto p = parser(test::Assemble(assembly));
    ASSERT_TRUE(p->BuildAndParseInternalModuleExceptFunctions()) << p->error();
    auto fe = p->function_emitter(100);
    EXPECT_FALSE(FlowClassifyCFGEdges(&fe));
    EXPECT_THAT(p->error(), Eq("Invalid backedge (30->20): 30 is not in a continue construct"));
}

TEST_F(SpvParserCFGTest, ClassifyCFGEdges_NeededMerge_BranchConditional) {
    auto assembly = CommonTypes() + R"(
     %100 = OpFunction %void None %voidfn

     %20 = OpLabel
     OpBranchConditional %cond %30 %40

     %30 = OpLabel
     OpBranch %99

     %40 = OpLabel
     OpBranch %99

     %99 = OpLabel
     OpReturn

     OpFunctionEnd
)";
    auto p = parser(test::Assemble(assembly));
    ASSERT_TRUE(p->BuildAndParseInternalModuleExceptFunctions()) << p->error();
    auto fe = p->function_emitter(100);
    EXPECT_FALSE(FlowClassifyCFGEdges(&fe));
    EXPECT_THAT(p->error(), Eq("Control flow diverges at block 20 (to 30, 40) but it is not "
                               "a structured header (it has no merge instruction)"));
}

TEST_F(SpvParserCFGTest, ClassifyCFGEdges_NeededMerge_Switch) {
    auto assembly = CommonTypes() + R"(
     %100 = OpFunction %void None %voidfn

     %10 = OpLabel
     OpSwitch %selector %99 20 %20 30 %30

     %20 = OpLabel
     OpBranch %99

     %30 = OpLabel
     OpBranch %99

     %99 = OpLabel
     OpReturn

     OpFunctionEnd
)";
    auto p = parser(test::Assemble(assembly));
    ASSERT_TRUE(p->BuildAndParseInternalModuleExceptFunctions()) << p->error();
    auto fe = p->function_emitter(100);
    EXPECT_FALSE(FlowClassifyCFGEdges(&fe));
    EXPECT_THAT(p->error(), Eq("Control flow diverges at block 10 (to 99, 20) but it is not "
                               "a structured header (it has no merge instruction)"));
}

TEST_F(SpvParserCFGTest, ClassifyCFGEdges_Pathological_Forward_LoopHeadSplitBody) {
    // In this case the branch-conditional in the loop header is really also a
    // selection header.
    auto assembly = CommonTypes() + R"(
     %100 = OpFunction %void None %voidfn

     %10 = OpLabel
     OpBranch %20

     %20 = OpLabel
     OpLoopMerge %99 %80 None
     OpBranchConditional %cond %30 %50 ; what to make of this?

     %30 = OpLabel
     OpBranch %99

     %50 = OpLabel
     OpBranch %99

     %80 = OpLabel
     OpBranch %20

     %99 = OpLabel
     OpReturn

     OpFunctionEnd
)";
    auto p = parser(test::Assemble(assembly));
    ASSERT_TRUE(p->BuildAndParseInternalModuleExceptFunctions()) << p->error();
    auto fe = p->function_emitter(100);
    EXPECT_TRUE(FlowClassifyCFGEdges(&fe));

    auto* bi = fe.GetBlockInfo(20);
    ASSERT_NE(bi, nullptr);
    EXPECT_EQ(bi->succ_edge.count(30), 1u);
    EXPECT_EQ(bi->succ_edge[30], EdgeKind::kForward);
    EXPECT_EQ(bi->succ_edge.count(50), 1u);
    EXPECT_EQ(bi->succ_edge[50], EdgeKind::kForward);
}

TEST_F(SpvParserCFGTest, ClassifyCFGEdges_Pathological_Forward_Premerge) {
    // Two arms of an if-selection converge early, before the merge block
    auto assembly = CommonTypes() + R"(
     %100 = OpFunction %void None %voidfn

     %10 = OpLabel
     OpSelectionMerge %99 None
     OpBranchConditional %cond %20 %30

     %20 = OpLabel
     OpBranch %50

     %30 = OpLabel
     OpBranch %50

     %50 = OpLabel ; this is an early merge!
     OpBranch %60

     %60 = OpLabel ; still early merge
     OpBranch %99

     %99 = OpLabel
     OpReturn

     OpFunctionEnd
)";
    auto p = parser(test::Assemble(assembly));
    ASSERT_TRUE(p->BuildAndParseInternalModuleExceptFunctions()) << p->error();
    auto fe = p->function_emitter(100);
    EXPECT_TRUE(FlowClassifyCFGEdges(&fe));

    auto* bi20 = fe.GetBlockInfo(20);
    ASSERT_NE(bi20, nullptr);
    EXPECT_EQ(bi20->succ_edge.count(50), 1u);
    EXPECT_EQ(bi20->succ_edge[50], EdgeKind::kForward);

    auto* bi30 = fe.GetBlockInfo(30);
    ASSERT_NE(bi30, nullptr);
    EXPECT_EQ(bi30->succ_edge.count(50), 1u);
    EXPECT_EQ(bi30->succ_edge[50], EdgeKind::kForward);

    auto* bi50 = fe.GetBlockInfo(50);
    ASSERT_NE(bi50, nullptr);
    EXPECT_EQ(bi50->succ_edge.count(60), 1u);
    EXPECT_EQ(bi50->succ_edge[60], EdgeKind::kForward);

    auto* bi60 = fe.GetBlockInfo(60);
    ASSERT_NE(bi60, nullptr);
    EXPECT_EQ(bi60->succ_edge.count(99), 1u);
    EXPECT_EQ(bi60->succ_edge[99], EdgeKind::kIfBreak);
}

TEST_F(SpvParserCFGTest, ClassifyCFGEdges_Pathological_Forward_Regardless) {
    // Both arms of an OpBranchConditional go to the same target.
    auto assembly = CommonTypes() + R"(
     %100 = OpFunction %void None %voidfn

     %10 = OpLabel
     OpSelectionMerge %99 None
     OpBranchConditional %cond %20 %20 ; same target!

     %20 = OpLabel
     OpBranch %99

     %99 = OpLabel
     OpReturn

     OpFunctionEnd
)";
    auto p = parser(test::Assemble(assembly));
    ASSERT_TRUE(p->BuildAndParseInternalModuleExceptFunctions()) << p->error();
    auto fe = p->function_emitter(100);
    EXPECT_TRUE(FlowClassifyCFGEdges(&fe));

    auto* bi10 = fe.GetBlockInfo(10);
    ASSERT_NE(bi10, nullptr);
    EXPECT_EQ(bi10->succ_edge.count(20), 1u);
    EXPECT_EQ(bi10->succ_edge[20], EdgeKind::kForward);

    auto* bi20 = fe.GetBlockInfo(20);
    ASSERT_NE(bi20, nullptr);
    EXPECT_EQ(bi20->succ_edge.count(99), 1u);
    EXPECT_EQ(bi20->succ_edge[99], EdgeKind::kIfBreak);
}

TEST_F(SpvParserCFGTest, FindIfSelectionInternalHeaders_NoIf) {
    auto assembly = CommonTypes() + R"(
     %100 = OpFunction %void None %voidfn

     %10 = OpLabel
     OpReturn

     OpFunctionEnd
)";
    auto p = parser(test::Assemble(assembly));
    ASSERT_TRUE(p->BuildAndParseInternalModuleExceptFunctions()) << p->error();
    auto fe = p->function_emitter(100);
    EXPECT_TRUE(FlowClassifyCFGEdges(&fe));

    auto* bi = fe.GetBlockInfo(10);
    ASSERT_NE(bi, nullptr);
    EXPECT_EQ(bi->true_head, 0u);
    EXPECT_EQ(bi->false_head, 0u);
    EXPECT_EQ(bi->premerge_head, 0u);
}

TEST_F(SpvParserCFGTest, FindIfSelectionInternalHeaders_ThenElse) {
    auto assembly = CommonTypes() + R"(
     %100 = OpFunction %void None %voidfn

     %10 = OpLabel
     OpSelectionMerge %99 None
     OpBranchConditional %cond %20 %30

     %20 = OpLabel
     OpBranch %99

     %30 = OpLabel
     OpBranch %99

     %99 = OpLabel
     OpReturn

     OpFunctionEnd
)";
    auto p = parser(test::Assemble(assembly));
    ASSERT_TRUE(p->BuildAndParseInternalModuleExceptFunctions()) << p->error();
    auto fe = p->function_emitter(100);
    EXPECT_TRUE(FlowFindIfSelectionInternalHeaders(&fe));

    auto* bi10 = fe.GetBlockInfo(10);
    ASSERT_NE(bi10, nullptr);
    EXPECT_EQ(bi10->true_head, 20u);
    EXPECT_EQ(bi10->false_head, 30u);
    EXPECT_EQ(bi10->premerge_head, 0u);

    auto* bi20 = fe.GetBlockInfo(20);
    ASSERT_NE(bi20, nullptr);
    EXPECT_EQ(bi20->true_head, 0u);
    EXPECT_EQ(bi20->false_head, 0u);
    EXPECT_EQ(bi20->premerge_head, 0u);

    auto* bi30 = fe.GetBlockInfo(30);
    ASSERT_NE(bi30, nullptr);
    EXPECT_EQ(bi30->true_head, 0u);
    EXPECT_EQ(bi30->false_head, 0u);
    EXPECT_EQ(bi30->premerge_head, 0u);

    auto* bi99 = fe.GetBlockInfo(99);
    ASSERT_NE(bi99, nullptr);
    EXPECT_EQ(bi99->true_head, 0u);
    EXPECT_EQ(bi99->false_head, 0u);
    EXPECT_EQ(bi99->premerge_head, 0u);
}

TEST_F(SpvParserCFGTest, FindIfSelectionInternalHeaders_IfOnly) {
    auto assembly = CommonTypes() + R"(
     %100 = OpFunction %void None %voidfn

     %10 = OpLabel
     OpSelectionMerge %99 None
     OpBranchConditional %cond %30 %99

     %30 = OpLabel
     OpBranch %99

     %99 = OpLabel
     OpReturn

     OpFunctionEnd
)";
    auto p = parser(test::Assemble(assembly));
    ASSERT_TRUE(p->BuildAndParseInternalModuleExceptFunctions()) << p->error();
    auto fe = p->function_emitter(100);
    EXPECT_TRUE(FlowFindIfSelectionInternalHeaders(&fe));

    auto* bi10 = fe.GetBlockInfo(10);
    ASSERT_NE(bi10, nullptr);
    EXPECT_EQ(bi10->true_head, 30u);
    EXPECT_EQ(bi10->false_head, 0u);
    EXPECT_EQ(bi10->premerge_head, 0u);

    auto* bi30 = fe.GetBlockInfo(30);
    ASSERT_NE(bi30, nullptr);
    EXPECT_EQ(bi30->true_head, 0u);
    EXPECT_EQ(bi30->false_head, 0u);
    EXPECT_EQ(bi30->premerge_head, 0u);

    auto* bi99 = fe.GetBlockInfo(99);
    ASSERT_NE(bi99, nullptr);
    EXPECT_EQ(bi99->true_head, 0u);
    EXPECT_EQ(bi99->false_head, 0u);
    EXPECT_EQ(bi99->premerge_head, 0u);
}

TEST_F(SpvParserCFGTest, FindIfSelectionInternalHeaders_ElseOnly) {
    auto assembly = CommonTypes() + R"(
     %100 = OpFunction %void None %voidfn

     %10 = OpLabel
     OpSelectionMerge %99 None
     OpBranchConditional %cond %99 %30

     %30 = OpLabel
     OpBranch %99

     %99 = OpLabel
     OpReturn

     OpFunctionEnd
)";
    auto p = parser(test::Assemble(assembly));
    ASSERT_TRUE(p->BuildAndParseInternalModuleExceptFunctions()) << p->error();
    auto fe = p->function_emitter(100);
    EXPECT_TRUE(FlowFindIfSelectionInternalHeaders(&fe));

    auto* bi10 = fe.GetBlockInfo(10);
    ASSERT_NE(bi10, nullptr);
    EXPECT_EQ(bi10->true_head, 0u);
    EXPECT_EQ(bi10->false_head, 30u);
    EXPECT_EQ(bi10->premerge_head, 0u);

    auto* bi30 = fe.GetBlockInfo(30);
    ASSERT_NE(bi30, nullptr);
    EXPECT_EQ(bi30->true_head, 0u);
    EXPECT_EQ(bi30->false_head, 0u);
    EXPECT_EQ(bi30->premerge_head, 0u);

    auto* bi99 = fe.GetBlockInfo(99);
    ASSERT_NE(bi99, nullptr);
    EXPECT_EQ(bi99->true_head, 0u);
    EXPECT_EQ(bi99->false_head, 0u);
    EXPECT_EQ(bi99->premerge_head, 0u);
}

TEST_F(SpvParserCFGTest, FindIfSelectionInternalHeaders_Regardless) {
    auto assembly = CommonTypes() + R"(
     %100 = OpFunction %void None %voidfn

     %10 = OpLabel
     OpSelectionMerge %99 None
     OpBranchConditional %cond %20 %20 ; same target

     %20 = OpLabel
     OpBranch %80

     %80 = OpLabel
     OpBranch %99

     %99 = OpLabel
     OpReturn

     OpFunctionEnd
)";
    auto p = parser(test::Assemble(assembly));
    ASSERT_TRUE(p->BuildAndParseInternalModuleExceptFunctions()) << p->error();
    auto fe = p->function_emitter(100);
    EXPECT_TRUE(FlowFindIfSelectionInternalHeaders(&fe));

    EXPECT_THAT(fe.block_order(), ElementsAre(10, 20, 80, 99));

    auto* bi10 = fe.GetBlockInfo(10);
    ASSERT_NE(bi10, nullptr);
    EXPECT_EQ(bi10->true_head, 20u);
    EXPECT_EQ(bi10->false_head, 20u);
    EXPECT_EQ(bi10->premerge_head, 0u);
}

TEST_F(SpvParserCFGTest, FindIfSelectionInternalHeaders_Premerge_Simple) {
    auto assembly = CommonTypes() + R"(
     %100 = OpFunction %void None %voidfn

     %10 = OpLabel
     OpSelectionMerge %99 None
     OpBranchConditional %cond %20 %30

     %20 = OpLabel
     OpBranch %80

     %30 = OpLabel
     OpBranch %80

     %80 = OpLabel ; premerge node
     OpBranch %99

     %99 = OpLabel
     OpReturn

     OpFunctionEnd
)";
    auto p = parser(test::Assemble(assembly));
    ASSERT_TRUE(p->BuildAndParseInternalModuleExceptFunctions()) << p->error();
    auto fe = p->function_emitter(100);
    EXPECT_TRUE(FlowFindIfSelectionInternalHeaders(&fe));

    EXPECT_THAT(fe.block_order(), ElementsAre(10, 20, 30, 80, 99));

    auto* bi10 = fe.GetBlockInfo(10);
    ASSERT_NE(bi10, nullptr);
    EXPECT_EQ(bi10->true_head, 20u);
    EXPECT_EQ(bi10->false_head, 30u);
    EXPECT_EQ(bi10->premerge_head, 80u);
}

TEST_F(SpvParserCFGTest, FindIfSelectionInternalHeaders_Premerge_ThenDirectToElse) {
    auto assembly = CommonTypes() + R"(
     %100 = OpFunction %void None %voidfn

     %10 = OpLabel
     OpSelectionMerge %99 None
     OpBranchConditional %cond %20 %30

     %20 = OpLabel
     OpBranch %30

     %30 = OpLabel
     OpBranch %80

     %80 = OpLabel ; premerge node
     OpBranch %99

     %99 = OpLabel
     OpReturn

     OpFunctionEnd
)";
    auto p = parser(test::Assemble(assembly));
    ASSERT_TRUE(p->BuildAndParseInternalModuleExceptFunctions()) << p->error();
    auto fe = p->function_emitter(100);
    EXPECT_TRUE(FlowFindIfSelectionInternalHeaders(&fe));

    EXPECT_THAT(fe.block_order(), ElementsAre(10, 20, 30, 80, 99));

    auto* bi10 = fe.GetBlockInfo(10);
    ASSERT_NE(bi10, nullptr);
    EXPECT_EQ(bi10->true_head, 20u);
    EXPECT_EQ(bi10->false_head, 30u);
    EXPECT_EQ(bi10->premerge_head, 30u);
}

TEST_F(SpvParserCFGTest, FindIfSelectionInternalHeaders_Premerge_ElseDirectToThen) {
    auto assembly = CommonTypes() + R"(
     %100 = OpFunction %void None %voidfn

     %10 = OpLabel
     OpSelectionMerge %99 None
     OpBranchConditional %cond %20 %30

     %20 = OpLabel
     OpBranch %80 ; branches to premerge

     %30 = OpLabel ; else
     OpBranch %20  ; branches to then

     %80 = OpLabel ; premerge node
     OpBranch %99

     %99 = OpLabel
     OpReturn

     OpFunctionEnd
)";
    auto p = parser(test::Assemble(assembly));
    ASSERT_TRUE(p->BuildAndParseInternalModuleExceptFunctions()) << p->error();
    auto fe = p->function_emitter(100);
    EXPECT_TRUE(FlowFindIfSelectionInternalHeaders(&fe));

    EXPECT_THAT(fe.block_order(), ElementsAre(10, 30, 20, 80, 99));

    auto* bi10 = fe.GetBlockInfo(10);
    ASSERT_NE(bi10, nullptr);
    EXPECT_EQ(bi10->true_head, 20u);
    EXPECT_EQ(bi10->false_head, 30u);
    EXPECT_EQ(bi10->premerge_head, 20u);
}

TEST_F(SpvParserCFGTest, FindIfSelectionInternalHeaders_Premerge_MultiCandidate_IsError) {
    auto assembly = CommonTypes() + R"(
     %100 = OpFunction %void None %voidfn

     %10 = OpLabel
     OpSelectionMerge %99 None
     OpBranchConditional %cond %20 %30

     %20 = OpLabel
     ; Try to force several branches down into "else" territory,
     ; but we error out earlier in the flow due to lack of merge
     ; instruction.
     OpBranchConditional %cond2  %70 %80

     %30 = OpLabel
     OpBranch %70

     %70 = OpLabel ; candidate premerge
     OpBranch %80

     %80 = OpLabel ; canddiate premerge
     OpBranch %99

     %99 = OpLabel
     OpReturn

     OpFunctionEnd
)";
    auto p = parser(test::Assemble(assembly));
    ASSERT_TRUE(p->BuildAndParseInternalModuleExceptFunctions()) << p->error();
    auto fe = p->function_emitter(100);
    // Error out sooner in the flow
    EXPECT_FALSE(FlowClassifyCFGEdges(&fe));
    EXPECT_THAT(p->error(), Eq("Control flow diverges at block 20 (to 70, 80) but it is not "
                               "a structured header (it has no merge instruction)"));
}

TEST_F(SpvParserCFGTest, ClassifyCFGEdges_IfBreak_FromThen_ForwardWithinThen) {
    // SPIR-V allows this unusual configuration.
    auto assembly = CommonTypes() + R"(
     %100 = OpFunction %void None %voidfn

     %10 = OpLabel
     OpSelectionMerge %99 None
     OpBranchConditional %cond %20 %99

     %20 = OpLabel
     OpBranchConditional %cond2 %99 %80 ; break with forward edge

     %80 = OpLabel ; still in then clause
     OpBranch %99

     %99 = OpLabel
     OpReturn
     OpFunctionEnd
)";
    auto p = parser(test::Assemble(assembly));
    ASSERT_TRUE(p->BuildAndParseInternalModuleExceptFunctions()) << p->error();
    auto fe = p->function_emitter(100);
    EXPECT_TRUE(FlowClassifyCFGEdges(&fe));
    EXPECT_THAT(fe.block_order(), ElementsAre(10, 20, 80, 99));

    auto* bi20 = fe.GetBlockInfo(20);
    ASSERT_NE(bi20, nullptr);
    EXPECT_EQ(bi20->succ_edge.count(80), 1u);
    EXPECT_EQ(bi20->succ_edge[80], EdgeKind::kForward);
    EXPECT_EQ(bi20->succ_edge.count(99), 1u);
    EXPECT_EQ(bi20->succ_edge[99], EdgeKind::kIfBreak);

    EXPECT_THAT(p->error(), Eq(""));
}

TEST_F(SpvParserCFGTest, ClassifyCFGEdges_IfBreak_FromElse_ForwardWithinElse) {
    // SPIR-V allows this unusual configuration.
    auto assembly = CommonTypes() + R"(
     %100 = OpFunction %void None %voidfn

     %10 = OpLabel
     OpSelectionMerge %99 None
     OpBranchConditional %cond %20 %30

     %20 = OpLabel
     OpBranch %99

     %30 = OpLabel ; else clause
     OpBranchConditional %cond2 %99 %80 ; break with forward edge

     %80 = OpLabel ; still in then clause
     OpBranch %99

     %99 = OpLabel
     OpReturn
     OpFunctionEnd
)";
    auto p = parser(test::Assemble(assembly));
    ASSERT_TRUE(p->BuildAndParseInternalModuleExceptFunctions()) << p->error();
    auto fe = p->function_emitter(100);
    EXPECT_TRUE(FlowClassifyCFGEdges(&fe));
    EXPECT_THAT(fe.block_order(), ElementsAre(10, 20, 30, 80, 99));

    auto* bi30 = fe.GetBlockInfo(30);
    ASSERT_NE(bi30, nullptr);
    EXPECT_EQ(bi30->succ_edge.count(80), 1u);
    EXPECT_EQ(bi30->succ_edge[80], EdgeKind::kForward);
    EXPECT_EQ(bi30->succ_edge.count(99), 1u);
    EXPECT_EQ(bi30->succ_edge[99], EdgeKind::kIfBreak);

    EXPECT_THAT(p->error(), Eq(""));
}

TEST_F(SpvParserCFGTest, ClassifyCFGEdges_IfBreak_WithForwardToPremerge) {
    auto assembly = CommonTypes() + R"(
     %100 = OpFunction %void None %voidfn

     %10 = OpLabel
     OpSelectionMerge %99 None
     OpBranchConditional %cond %20 %30

     %20 = OpLabel ; then
     OpBranchConditional %cond2 %99 %80 ; break with forward to premerge

     %30 = OpLabel ; else
     OpBranch %80

     %80 = OpLabel ; premerge node
     OpBranch %99

     %99 = OpLabel
     OpReturn
     OpFunctionEnd
)";
    auto p = parser(test::Assemble(assembly));
    ASSERT_TRUE(p->BuildAndParseInternalModuleExceptFunctions()) << p->error();
    auto fe = p->function_emitter(100);
    EXPECT_TRUE(FlowClassifyCFGEdges(&fe));
    EXPECT_THAT(fe.block_order(), ElementsAre(10, 20, 30, 80, 99));

    auto* bi20 = fe.GetBlockInfo(20);
    ASSERT_NE(bi20, nullptr);
    EXPECT_EQ(bi20->succ_edge.count(80), 1u);
    EXPECT_EQ(bi20->succ_edge[80], EdgeKind::kForward);
    EXPECT_EQ(bi20->succ_edge.count(99), 1u);
    EXPECT_EQ(bi20->succ_edge[99], EdgeKind::kIfBreak);

    EXPECT_THAT(p->error(), Eq(""));

    // TODO(crbug.com/tint/775): The SPIR-V reader errors out on this case.
    // Remove this when it's fixed.
    p->DeliberatelyInvalidSpirv();
}

TEST_F(SpvParserCFGTest,
       FindIfSelectionInternalHeaders_DomViolation_InteriorMerge_CantBeTrueHeader) {
    auto assembly = CommonTypes() + R"(
     %100 = OpFunction %void None %voidfn

     %10 = OpLabel
     OpSelectionMerge %99 None
     OpBranchConditional %cond %40 %20

     %20 = OpLabel
     OpSelectionMerge %40 None
     OpBranchConditional %cond2 %30 %40

     %30 = OpLabel
     OpBranch %40

     %40 = OpLabel ; inner merge, and true-head for outer if-selection
     OpBranch %99

     %99 = OpLabel ; outer merge
     OpReturn

     OpFunctionEnd
)";
    auto p = parser(test::Assemble(assembly));
    ASSERT_TRUE(p->BuildAndParseInternalModuleExceptFunctions()) << p->error();
    auto fe = p->function_emitter(100);
    EXPECT_FALSE(FlowFindIfSelectionInternalHeaders(&fe));
    EXPECT_THAT(p->error(),
                Eq("Block 40 is the true branch for if-selection header 10 and also the "
                   "merge block for header block 20 (violates dominance rule)"));
}

TEST_F(SpvParserCFGTest,
       FindIfSelectionInternalHeaders_DomViolation_InteriorMerge_CantBeFalseHeader) {
    auto assembly = CommonTypes() + R"(
     %100 = OpFunction %void None %voidfn

     %10 = OpLabel
     OpSelectionMerge %99 None
     OpBranchConditional %cond %20 %40

     %20 = OpLabel
     OpSelectionMerge %40 None
     OpBranchConditional %cond %30 %40

     %30 = OpLabel
     OpBranch %40

     %40 = OpLabel ; inner merge, and true-head for outer if-selection
     OpBranch %99

     %99 = OpLabel ; outer merge
     OpReturn

     OpFunctionEnd
)";
    auto p = parser(test::Assemble(assembly));
    ASSERT_TRUE(p->BuildAndParseInternalModuleExceptFunctions()) << p->error();
    auto fe = p->function_emitter(100);
    EXPECT_FALSE(FlowFindIfSelectionInternalHeaders(&fe));
    EXPECT_THAT(p->error(),
                Eq("Block 40 is the false branch for if-selection header 10 and also the "
                   "merge block for header block 20 (violates dominance rule)"));
}

TEST_F(SpvParserCFGTest, FindIfSelectionInternalHeaders_DomViolation_InteriorMerge_CantBePremerge) {
    auto assembly = CommonTypes() + R"(
     %100 = OpFunction %void None %voidfn

     %10 = OpLabel ; outer if-header
     OpSelectionMerge %99 None
     OpBranchConditional %cond %20 %50

     %20 = OpLabel
     OpBranch %70

     %50 = OpLabel ; inner if-header
     OpSelectionMerge %70 None
     OpBranchConditional %cond %60 %70

     %60 = OpLabel
     OpBranch %70

     %70 = OpLabel ; inner merge, and premerge for outer if-selection
     OpBranch %80

     %80 = OpLabel
     OpBranch %99

     %99 = OpLabel ; outer merge
     OpReturn

     OpFunctionEnd
)";
    auto p = parser(test::Assemble(assembly));
    ASSERT_TRUE(p->BuildAndParseInternalModuleExceptFunctions()) << p->error();
    auto fe = p->function_emitter(100);
    EXPECT_FALSE(FlowFindIfSelectionInternalHeaders(&fe));
    EXPECT_THAT(p->error(), Eq("Block 70 is the merge block for 50 but has alternate paths "
                               "reaching it, starting from blocks 20 and 50 which are the "
                               "true and false branches for the if-selection header block 10 "
                               "(violates dominance rule)"));
}

TEST_F(SpvParserCFGTest, FindIfSelectionInternalHeaders_TrueBranch_LoopBreak_Ok) {
    // crbug.com/tint/243
    auto assembly = CommonTypes() + R"(
     %100 = OpFunction %void None %voidfn

     %5 = OpLabel
     OpBranch %10

     %10 = OpLabel
     OpLoopMerge %99 %90 None
     OpBranch %20

     %20 = OpLabel
     OpSelectionMerge %40 None
     OpBranchConditional %cond %99 %30 ; true branch breaking is ok

     %30 = OpLabel
     OpBranch %40

     %40 = OpLabel ; selection merge
     OpBranch %90

     %90 = OpLabel ; continue target
     OpBranch %10 ; backedge

     %99 = OpLabel ; loop merge
     OpReturn

     OpFunctionEnd
)";
    auto p = parser(test::Assemble(assembly));
    ASSERT_TRUE(p->BuildAndParseInternalModuleExceptFunctions()) << p->error();
    auto fe = p->function_emitter(100);
    EXPECT_TRUE(FlowFindIfSelectionInternalHeaders(&fe));
    EXPECT_THAT(p->error(), Eq(""));
}

TEST_F(SpvParserCFGTest, FindIfSelectionInternalHeaders_TrueBranch_LoopContinue_Ok) {
    // crbug.com/tint/243
    auto assembly = CommonTypes() + R"(
     %100 = OpFunction %void None %voidfn

     %5 = OpLabel
     OpBranch %10

     %10 = OpLabel
     OpLoopMerge %99 %90 None
     OpBranch %20

     %20 = OpLabel
     OpSelectionMerge %40 None
     OpBranchConditional %cond %90 %30 ; true branch continue is ok

     %30 = OpLabel
     OpBranch %40

     %40 = OpLabel ; selection merge
     OpBranch %90

     %90 = OpLabel ; continue target
     OpBranch %10 ; backedge

     %99 = OpLabel ; loop merge
     OpReturn

     OpFunctionEnd
)";
    auto p = parser(test::Assemble(assembly));
    ASSERT_TRUE(p->BuildAndParseInternalModuleExceptFunctions()) << p->error();
    auto fe = p->function_emitter(100);
    EXPECT_TRUE(FlowFindIfSelectionInternalHeaders(&fe));
    EXPECT_THAT(p->error(), Eq(""));
}

TEST_F(SpvParserCFGTest, FindIfSelectionInternalHeaders_TrueBranch_SwitchBreak_Ok) {
    // crbug.com/tint/243
    auto assembly = CommonTypes() + R"(
     %100 = OpFunction %void None %voidfn

     %10 = OpLabel
     OpSelectionMerge %99 None
     OpSwitch %uint_20 %99 20 %20

     %20 = OpLabel
     OpSelectionMerge %40 None
     OpBranchConditional %cond %99 %30 ; true branch switch break is ok

     %30 = OpLabel
     OpBranch %40

     %40 = OpLabel ; if-selection merge
     OpBranch %99

     %99 = OpLabel ; switch merge
     OpReturn

     OpFunctionEnd
)";
    auto p = parser(test::Assemble(assembly));
    ASSERT_TRUE(p->BuildAndParseInternalModuleExceptFunctions()) << p->error();
    auto fe = p->function_emitter(100);
    EXPECT_TRUE(FlowFindIfSelectionInternalHeaders(&fe));
    EXPECT_THAT(p->error(), Eq(""));
}

TEST_F(SpvParserCFGTest, FindIfSelectionInternalHeaders_FalseBranch_LoopBreak_Ok) {
    // crbug.com/tint/243
    auto assembly = CommonTypes() + R"(
     %100 = OpFunction %void None %voidfn

     %5 = OpLabel
     OpBranch %10

     %10 = OpLabel
     OpLoopMerge %99 %90 None
     OpBranch %20

     %20 = OpLabel
     OpSelectionMerge %40 None
     OpBranchConditional %cond %30 %99 ; false branch breaking is ok

     %30 = OpLabel
     OpBranch %40

     %40 = OpLabel ; selection merge
     OpBranch %90

     %90 = OpLabel ; continue target
     OpBranch %10 ; backedge

     %99 = OpLabel ; loop merge
     OpReturn

     OpFunctionEnd
)";
    auto p = parser(test::Assemble(assembly));
    ASSERT_TRUE(p->BuildAndParseInternalModuleExceptFunctions()) << p->error();
    auto fe = p->function_emitter(100);
    EXPECT_TRUE(FlowFindIfSelectionInternalHeaders(&fe));
    EXPECT_THAT(p->error(), Eq(""));
}

TEST_F(SpvParserCFGTest, FindIfSelectionInternalHeaders_FalseBranch_LoopContinue_Ok) {
    // crbug.com/tint/243
    auto assembly = CommonTypes() + R"(
     %100 = OpFunction %void None %voidfn

     %5 = OpLabel
     OpBranch %10

     %10 = OpLabel
     OpLoopMerge %99 %90 None
     OpBranch %20

     %20 = OpLabel
     OpSelectionMerge %40 None
     OpBranchConditional %cond %30 %90 ; false branch continue is ok

     %30 = OpLabel
     OpBranch %40

     %40 = OpLabel ; selection merge
     OpBranch %90

     %90 = OpLabel ; continue target
     OpBranch %10 ; backedge

     %99 = OpLabel ; loop merge
     OpReturn

     OpFunctionEnd
)";
    auto p = parser(test::Assemble(assembly));
    ASSERT_TRUE(p->BuildAndParseInternalModuleExceptFunctions()) << p->error();
    auto fe = p->function_emitter(100);
    EXPECT_TRUE(FlowFindIfSelectionInternalHeaders(&fe));
    EXPECT_THAT(p->error(), Eq(""));
}

TEST_F(SpvParserCFGTest, FindIfSelectionInternalHeaders_FalseBranch_SwitchBreak_Ok) {
    // crbug.com/tint/243
    auto assembly = CommonTypes() + R"(
     %100 = OpFunction %void None %voidfn

     %10 = OpLabel
     OpSelectionMerge %99 None
     OpSwitch %uint_20 %99 20 %20

     %20 = OpLabel
     OpSelectionMerge %40 None
     OpBranchConditional %cond %30 %99 ; false branch switch break is ok

     %30 = OpLabel
     OpBranch %40

     %40 = OpLabel ; if-selection merge
     OpBranch %99

     %99 = OpLabel ; switch merge
     OpReturn

     OpFunctionEnd
)";
    auto p = parser(test::Assemble(assembly));
    ASSERT_TRUE(p->BuildAndParseInternalModuleExceptFunctions()) << p->error();
    auto fe = p->function_emitter(100);
    EXPECT_TRUE(FlowFindIfSelectionInternalHeaders(&fe));
    EXPECT_THAT(p->error(), Eq(""));
}

TEST_F(SpvParserCFGTest, EmitBody_IfBreak_FromThen_ForwardWithinThen) {
    // Exercises the hard case where we a single OpBranchConditional has both
    // IfBreak and Forward edges, within the true-branch clause.
    auto assembly = CommonTypes() + R"(
     %100 = OpFunction %void None %voidfn

     %10 = OpLabel
     OpStore %var %uint_1
     OpSelectionMerge %99 None
     OpBranchConditional %cond %20 %50

     %20 = OpLabel
     OpStore %var %uint_2
     OpBranchConditional %cond2 %99 %30 ; kIfBreak with kForward

     %30 = OpLabel ; still in then clause
     OpStore %var %uint_3
     OpBranch %99

     %50 = OpLabel ; else clause
     OpStore %var %uint_4
     OpBranch %99

     %99 = OpLabel
     OpStore %var %uint_5
     OpReturn
     OpFunctionEnd
)";
    auto p = parser(test::Assemble(assembly));
    ASSERT_TRUE(p->BuildAndParseInternalModuleExceptFunctions()) << p->error();
    auto fe = p->function_emitter(100);
    EXPECT_TRUE(fe.EmitBody()) << p->error();
    auto ast_body = fe.ast_body();
    auto got = test::ToString(p->program(), ast_body);
    auto* expect = R"(var_1 = 1u;
var guard10 = true;
if (false) {
  var_1 = 2u;
  if (true) {
    guard10 = false;
  }
  if (guard10) {
    var_1 = 3u;
    guard10 = false;
  }
} else {
  if (guard10) {
    var_1 = 4u;
    guard10 = false;
  }
}
var_1 = 5u;
return;
)";
    ASSERT_EQ(expect, got);
}

TEST_F(SpvParserCFGTest, EmitBody_IfBreak_FromElse_ForwardWithinElse) {
    // Exercises the hard case where we a single OpBranchConditional has both
    // IfBreak and Forward edges, within the false-branch clause.
    auto assembly = CommonTypes() + R"(
     %100 = OpFunction %void None %voidfn

     %10 = OpLabel
     OpStore %var %uint_1
     OpSelectionMerge %99 None
     OpBranchConditional %cond %20 %50

     %20 = OpLabel
     OpStore %var %uint_2
     OpBranch %99

     %50 = OpLabel ; else clause
     OpStore %var %uint_3
     OpBranchConditional %cond2 %99 %80 ; kIfBreak with kForward

     %80 = OpLabel ; still in then clause
     OpStore %var %uint_4
     OpBranch %99

     %99 = OpLabel
     OpStore %var %uint_5
     OpReturn
     OpFunctionEnd
)";
    auto p = parser(test::Assemble(assembly));
    ASSERT_TRUE(p->BuildAndParseInternalModuleExceptFunctions()) << p->error();
    auto fe = p->function_emitter(100);
    EXPECT_TRUE(fe.EmitBody()) << p->error();
    auto ast_body = fe.ast_body();
    auto got = test::ToString(p->program(), ast_body);
    auto* expect = R"(var_1 = 1u;
var guard10 = true;
if (false) {
  var_1 = 2u;
  guard10 = false;
} else {
  if (guard10) {
    var_1 = 3u;
    if (true) {
      guard10 = false;
    }
    if (guard10) {
      var_1 = 4u;
      guard10 = false;
    }
  }
}
var_1 = 5u;
return;
)";
    ASSERT_EQ(expect, got);
}

TEST_F(SpvParserCFGTest, EmitBody_IfBreak_FromThenWithForward_FromElseWithForward_AlsoPremerge) {
    // This is a combination of the previous two, but also adding a premerge.
    // We have IfBreak and Forward edges from the same OpBranchConditional, and
    // this occurs in the true-branch clause, the false-branch clause, and within
    // the premerge clause.  Flow guards have to be sprinkled in lots of places.
    auto assembly = CommonTypes() + R"(
     %100 = OpFunction %void None %voidfn

     %10 = OpLabel
     OpStore %var %uint_1
     OpSelectionMerge %99 None
     OpBranchConditional %cond %20 %50

     %20 = OpLabel ; then
     OpStore %var %uint_2
     OpBranchConditional %cond2 %21 %99 ; kForward and kIfBreak

     %21 = OpLabel ; still in then clause
     OpStore %var %uint_3
     OpBranch %80 ; to premerge

     %50 = OpLabel ; else clause
     OpStore %var %uint_4
     OpBranchConditional %cond2 %99 %51 ; kIfBreak with kForward

     %51 = OpLabel ; still in else clause
     OpStore %var %uint_5
     OpBranch %80 ; to premerge

     %80 = OpLabel ; premerge
     OpStore %var %uint_6
     OpBranchConditional %cond3 %81 %99

     %81 = OpLabel ; premerge
     OpStore %var %uint_7
     OpBranch %99

     %99 = OpLabel
     OpStore %var %uint_8
     OpReturn
     OpFunctionEnd
)";
    auto p = parser(test::Assemble(assembly));
    ASSERT_TRUE(p->BuildAndParseInternalModuleExceptFunctions()) << p->error();
    auto fe = p->function_emitter(100);
    EXPECT_TRUE(fe.EmitBody()) << p->error() << assembly;
    auto ast_body = fe.ast_body();
    auto got = test::ToString(p->program(), ast_body);
    auto* expect = R"(var_1 = 1u;
var guard10 = true;
if (false) {
  var_1 = 2u;
  if (true) {
  } else {
    guard10 = false;
  }
  if (guard10) {
    var_1 = 3u;
  }
} else {
  if (guard10) {
    var_1 = 4u;
    if (true) {
      guard10 = false;
    }
    if (guard10) {
      var_1 = 5u;
    }
  }
}
if (guard10) {
  var_1 = 6u;
  if (false) {
  } else {
    guard10 = false;
  }
  if (guard10) {
    var_1 = 7u;
    guard10 = false;
  }
}
var_1 = 8u;
return;
)";
    ASSERT_EQ(expect, got);
}

TEST_F(SpvParserCFGTest, BlockIsContinueForMoreThanOneHeader) {
    // This is disallowed by the rule:
    //    "a continue block is valid only for the innermost loop it is nested
    //    inside of"
    auto assembly = CommonTypes() + R"(
     %100 = OpFunction %void None %voidfn

     %10 = OpLabel
     OpBranch %20

     %20 = OpLabel ; outer loop
     OpLoopMerge %99 %50 None
     OpBranchConditional %cond %50 %99

     %50 = OpLabel ; continue target, but also single-block loop
     OpLoopMerge %80 %50 None
     OpBranchConditional %cond2 %50 %80

     %80 = OpLabel
     OpBranch %20 ; backedge for outer loop

     %99 = OpLabel
     OpReturn
     OpFunctionEnd
)";
    auto p = parser(test::Assemble(assembly));
    ASSERT_TRUE(p->BuildAndParseInternalModuleExceptFunctions()) << p->error();
    auto fe = p->function_emitter(100);
    fe.RegisterBasicBlocks();
    fe.ComputeBlockOrderAndPositions();
    EXPECT_TRUE(fe.VerifyHeaderContinueMergeOrder());
    EXPECT_FALSE(fe.RegisterMerges());
    EXPECT_THAT(p->error(), Eq("Block 50 declared as continue target for more "
                               "than one header: 20, 50"));
}

TEST_F(SpvParserCFGTest, EmitBody_If_Empty) {
    auto p = parser(test::Assemble(CommonTypes() + R"(
     %100 = OpFunction %void None %voidfn

     %10 = OpLabel
     OpSelectionMerge %99 None
     OpBranchConditional %cond %99 %99

     %99 = OpLabel
     OpReturn
     OpFunctionEnd
  )"));
    ASSERT_TRUE(p->BuildAndParseInternalModuleExceptFunctions()) << p->error();
    auto fe = p->function_emitter(100);
    EXPECT_TRUE(fe.EmitBody()) << p->error();

    auto ast_body = fe.ast_body();
    auto got = test::ToString(p->program(), ast_body);
    auto* expect = R"(if (false) {
}
return;
)";
    ASSERT_EQ(expect, got);
}

TEST_F(SpvParserCFGTest, EmitBody_If_Then_NoElse) {
    auto p = parser(test::Assemble(CommonTypes() + R"(
     %100 = OpFunction %void None %voidfn

     %10 = OpLabel
     OpStore %var %uint_0
     OpSelectionMerge %99 None
     OpBranchConditional %cond %30 %99

     %99 = OpLabel
     OpStore %var %999
     OpReturn

     %30 = OpLabel
     OpStore %var %uint_1
     OpBranch %99

     OpFunctionEnd
  )"));
    ASSERT_TRUE(p->BuildAndParseInternalModuleExceptFunctions()) << p->error();
    auto fe = p->function_emitter(100);
    EXPECT_TRUE(fe.EmitBody()) << p->error();

    auto ast_body = fe.ast_body();
    auto got = test::ToString(p->program(), ast_body);
    auto* expect = R"(var_1 = 0u;
if (false) {
  var_1 = 1u;
}
var_1 = 999u;
return;
)";
    ASSERT_EQ(expect, got);
}

TEST_F(SpvParserCFGTest, EmitBody_If_NoThen_Else) {
    auto p = parser(test::Assemble(CommonTypes() + R"(
     %100 = OpFunction %void None %voidfn

     %10 = OpLabel
     OpStore %var %uint_0
     OpSelectionMerge %99 None
     OpBranchConditional %cond %99 %30

     %99 = OpLabel
     OpStore %var %999
     OpReturn

     %30 = OpLabel
     OpStore %var %uint_1
     OpBranch %99

     OpFunctionEnd
  )"));
    ASSERT_TRUE(p->BuildAndParseInternalModuleExceptFunctions()) << p->error();
    auto fe = p->function_emitter(100);
    EXPECT_TRUE(fe.EmitBody()) << p->error();

    auto ast_body = fe.ast_body();
    auto got = test::ToString(p->program(), ast_body);
    auto* expect = R"(var_1 = 0u;
if (false) {
} else {
  var_1 = 1u;
}
var_1 = 999u;
return;
)";
    ASSERT_EQ(expect, got);
}

TEST_F(SpvParserCFGTest, EmitBody_If_Then_Else) {
    auto p = parser(test::Assemble(CommonTypes() + R"(
     %100 = OpFunction %void None %voidfn

     %10 = OpLabel
     OpStore %var %uint_0
     OpSelectionMerge %99 None
     OpBranchConditional %cond %30 %40

     %99 = OpLabel
     OpStore %var %999
     OpReturn

     %30 = OpLabel
     OpStore %var %uint_1
     OpBranch %99

     %40 = OpLabel
     OpStore %var %uint_2
     OpBranch %99

     OpFunctionEnd
  )"));
    ASSERT_TRUE(p->BuildAndParseInternalModuleExceptFunctions()) << p->error();
    auto fe = p->function_emitter(100);
    EXPECT_TRUE(fe.EmitBody()) << p->error();

    auto ast_body = fe.ast_body();
    auto got = test::ToString(p->program(), ast_body);
    auto* expect = R"(var_1 = 0u;
if (false) {
  var_1 = 1u;
} else {
  var_1 = 2u;
}
var_1 = 999u;
return;
)";
    ASSERT_EQ(expect, got);
}

TEST_F(SpvParserCFGTest, EmitBody_If_Then_Else_Premerge) {
    // TODO(dneto): This should get an extra if(true) around
    // the premerge code.
    // See https://bugs.chromium.org/p/tint/issues/detail?id=82
    auto p = parser(test::Assemble(CommonTypes() + R"(
     %100 = OpFunction %void None %voidfn

     %10 = OpLabel
     OpStore %var %uint_0
     OpSelectionMerge %99 None
     OpBranchConditional %cond %30 %40

     %80 = OpLabel ; premerge
     OpStore %var %uint_3
     OpBranch %99

     %99 = OpLabel
     OpStore %var %999
     OpReturn

     %30 = OpLabel
     OpStore %var %uint_1
     OpBranch %80

     %40 = OpLabel
     OpStore %var %uint_2
     OpBranch %80

     OpFunctionEnd
  )"));
    ASSERT_TRUE(p->BuildAndParseInternalModuleExceptFunctions()) << p->error();
    auto fe = p->function_emitter(100);
    EXPECT_TRUE(fe.EmitBody()) << p->error();

    auto ast_body = fe.ast_body();
    auto got = test::ToString(p->program(), ast_body);
    auto* expect = R"(var_1 = 0u;
if (false) {
  var_1 = 1u;
} else {
  var_1 = 2u;
}
if (true) {
  var_1 = 3u;
}
var_1 = 999u;
return;
)";
    ASSERT_EQ(expect, got);
}

TEST_F(SpvParserCFGTest, EmitBody_If_Then_Premerge) {
    // The premerge *is* the else.
    auto p = parser(test::Assemble(CommonTypes() + R"(
     %100 = OpFunction %void None %voidfn

     %10 = OpLabel
     OpStore %var %uint_0
     OpSelectionMerge %99 None
     OpBranchConditional %cond %30 %80

     %80 = OpLabel ; premerge
     OpStore %var %uint_3
     OpBranch %99

     %99 = OpLabel
     OpStore %var %999
     OpReturn

     %30 = OpLabel
     OpStore %var %uint_1
     OpBranch %80

     OpFunctionEnd
  )"));
    ASSERT_TRUE(p->BuildAndParseInternalModuleExceptFunctions()) << p->error();
    auto fe = p->function_emitter(100);
    EXPECT_TRUE(fe.EmitBody()) << p->error();

    auto ast_body = fe.ast_body();
    auto got = test::ToString(p->program(), ast_body);
    auto* expect = R"(var_1 = 0u;
if (false) {
  var_1 = 1u;
}
if (true) {
  var_1 = 3u;
}
var_1 = 999u;
return;
)";
    ASSERT_EQ(expect, got);
}

TEST_F(SpvParserCFGTest, EmitBody_If_Else_Premerge) {
    // The premerge *is* the then-clause.
    auto p = parser(test::Assemble(CommonTypes() + R"(
     %100 = OpFunction %void None %voidfn

     %10 = OpLabel
     OpStore %var %uint_0
     OpSelectionMerge %99 None
     OpBranchConditional %cond %80 %30

     %80 = OpLabel ; premerge
     OpStore %var %uint_3
     OpBranch %99

     %99 = OpLabel
     OpStore %var %999
     OpReturn

     %30 = OpLabel
     OpStore %var %uint_1
     OpBranch %80

     OpFunctionEnd
  )"));
    ASSERT_TRUE(p->BuildAndParseInternalModuleExceptFunctions()) << p->error();
    auto fe = p->function_emitter(100);
    EXPECT_TRUE(fe.EmitBody()) << p->error();

    auto ast_body = fe.ast_body();
    auto got = test::ToString(p->program(), ast_body);
    auto* expect = R"(var_1 = 0u;
if (false) {
} else {
  var_1 = 1u;
}
if (true) {
  var_1 = 3u;
}
var_1 = 999u;
return;
)";
    ASSERT_EQ(expect, got);
}

TEST_F(SpvParserCFGTest, EmitBody_If_Nest_If) {
    auto p = parser(test::Assemble(CommonTypes() + R"(
     %100 = OpFunction %void None %voidfn

     %10 = OpLabel
     OpStore %var %uint_0
     OpSelectionMerge %99 None
     OpBranchConditional %cond %30 %40

     %30 = OpLabel ;; inner if #1
     OpStore %var %uint_1
     OpSelectionMerge %39 None
     OpBranchConditional %cond2 %33 %39

     %33 = OpLabel
     OpStore %var %uint_2
     OpBranch %39

     %39 = OpLabel ;; inner merge
     OpStore %var %uint_3
     OpBranch %99

     %40 = OpLabel ;; inner if #2
     OpStore %var %uint_4
     OpSelectionMerge %49 None
     OpBranchConditional %cond2 %49 %43

     %43 = OpLabel
     OpStore %var %uint_5
     OpBranch %49

     %49 = OpLabel ;; 2nd inner merge
     OpStore %var %uint_6
     OpBranch %99

     %99 = OpLabel
     OpStore %var %999
     OpReturn

     OpFunctionEnd
  )"));
    ASSERT_TRUE(p->BuildAndParseInternalModuleExceptFunctions()) << p->error();
    auto fe = p->function_emitter(100);
    EXPECT_TRUE(fe.EmitBody()) << p->error();

    auto ast_body = fe.ast_body();
    auto got = test::ToString(p->program(), ast_body);
    auto* expect = R"(var_1 = 0u;
if (false) {
  var_1 = 1u;
  if (true) {
    var_1 = 2u;
  }
  var_1 = 3u;
} else {
  var_1 = 4u;
  if (true) {
  } else {
    var_1 = 5u;
  }
  var_1 = 6u;
}
var_1 = 999u;
return;
)";
    ASSERT_EQ(expect, got);
}

TEST_F(SpvParserCFGTest, EmitBody_Loop_SingleBlock_TrueBackedge) {
    auto p = parser(test::Assemble(CommonTypes() + R"(
     %100 = OpFunction %void None %voidfn

     %10 = OpLabel
     OpStore %var %uint_0
     OpBranch %20

     %20 = OpLabel
     OpStore %var %uint_1
     OpLoopMerge %99 %20 None
     OpBranchConditional %cond %20 %99

     %99 = OpLabel
     OpStore %var %999
     OpReturn

     OpFunctionEnd
  )"));
    ASSERT_TRUE(p->BuildAndParseInternalModuleExceptFunctions()) << p->error();
    auto fe = p->function_emitter(100);
    EXPECT_TRUE(fe.EmitBody()) << p->error();

    auto ast_body = fe.ast_body();
    auto got = test::ToString(p->program(), ast_body);
    auto* expect = R"(var_1 = 0u;
loop {
  var_1 = 1u;
  if (false) {
  } else {
    break;
  }
}
var_1 = 999u;
return;
)";
    ASSERT_EQ(expect, got);
}

TEST_F(SpvParserCFGTest, EmitBody_Loop_SingleBlock_FalseBackedge) {
    auto p = parser(test::Assemble(CommonTypes() + R"(
     %100 = OpFunction %void None %voidfn

     %10 = OpLabel
     OpStore %var %uint_0
     OpBranch %20

     %20 = OpLabel
     OpStore %var %uint_1
     OpLoopMerge %99 %20 None
     OpBranchConditional %cond %99 %20

     %99 = OpLabel
     OpStore %var %999
     OpReturn

     OpFunctionEnd
  )"));
    ASSERT_TRUE(p->BuildAndParseInternalModuleExceptFunctions()) << p->error();
    auto fe = p->function_emitter(100);
    EXPECT_TRUE(fe.EmitBody()) << p->error();

    auto ast_body = fe.ast_body();
    auto got = test::ToString(p->program(), ast_body);
    auto* expect = R"(var_1 = 0u;
loop {
  var_1 = 1u;
  if (false) {
    break;
  }
}
var_1 = 999u;
return;
)";
    ASSERT_EQ(expect, got);
}

TEST_F(SpvParserCFGTest, EmitBody_Loop_SingleBlock_BothBackedge) {
    auto p = parser(test::Assemble(CommonTypes() + R"(
     %100 = OpFunction %void None %voidfn

     %10 = OpLabel
     OpStore %var %uint_0
     OpBranch %20

     %20 = OpLabel
     OpStore %var %uint_1
     OpLoopMerge %99 %20 None
     OpBranchConditional %cond %20 %20

     %99 = OpLabel
     OpStore %var %999
     OpReturn

     OpFunctionEnd
  )"));
    ASSERT_TRUE(p->BuildAndParseInternalModuleExceptFunctions()) << p->error();
    auto fe = p->function_emitter(100);
    EXPECT_TRUE(fe.EmitBody()) << p->error();

    auto ast_body = fe.ast_body();
    auto got = test::ToString(p->program(), ast_body);
    auto* expect = R"(var_1 = 0u;
loop {
  var_1 = 1u;
}
var_1 = 999u;
return;
)";
    ASSERT_EQ(expect, got);
}

TEST_F(SpvParserCFGTest, EmitBody_Loop_SingleBlock_UnconditionalBackege) {
    auto p = parser(test::Assemble(CommonTypes() + R"(
     %100 = OpFunction %void None %voidfn

     %10 = OpLabel
     OpStore %var %uint_0
     OpBranch %20

     %20 = OpLabel
     OpStore %var %uint_1
     OpLoopMerge %99 %20 None
     OpBranch %20

     %99 = OpLabel
     OpStore %var %999
     OpReturn

     OpFunctionEnd
  )"));
    ASSERT_TRUE(p->BuildAndParseInternalModuleExceptFunctions()) << p->error();
    auto fe = p->function_emitter(100);
    EXPECT_TRUE(fe.EmitBody()) << p->error();

    auto ast_body = fe.ast_body();
    auto got = test::ToString(p->program(), ast_body);
    auto* expect = R"(var_1 = 0u;
loop {
  var_1 = 1u;
}
var_1 = 999u;
return;
)";
    ASSERT_EQ(expect, got);
}

TEST_F(SpvParserCFGTest, EmitBody_Loop_Unconditional_Body_SingleBlockContinue) {
    auto p = parser(test::Assemble(CommonTypes() + R"(
     %100 = OpFunction %void None %voidfn

     %10 = OpLabel
     OpStore %var %uint_0
     OpBranch %20

     %20 = OpLabel
     OpStore %var %uint_1
     OpLoopMerge %99 %50 None
     OpBranch %30

     %30 = OpLabel
     OpStore %var %uint_2
     OpBranch %50

     %50 = OpLabel
     OpStore %var %uint_3
     OpBranch %20

     %99 = OpLabel
     OpStore %var %999
     OpReturn

     OpFunctionEnd
  )"));
    ASSERT_TRUE(p->BuildAndParseInternalModuleExceptFunctions()) << p->error();
    auto fe = p->function_emitter(100);
    EXPECT_TRUE(fe.EmitBody()) << p->error();

    auto ast_body = fe.ast_body();
    auto got = test::ToString(p->program(), ast_body);
    auto* expect = R"(var_1 = 0u;
loop {
  var_1 = 1u;
  var_1 = 2u;

  continuing {
    var_1 = 3u;
  }
}
var_1 = 999u;
return;
)";
    ASSERT_EQ(expect, got);
}

TEST_F(SpvParserCFGTest, EmitBody_Loop_Unconditional_Body_MultiBlockContinue) {
    auto p = parser(test::Assemble(CommonTypes() + R"(
     %100 = OpFunction %void None %voidfn

     %10 = OpLabel
     OpStore %var %uint_0
     OpBranch %20

     %20 = OpLabel
     OpStore %var %uint_1
     OpLoopMerge %99 %50 None
     OpBranch %30

     %30 = OpLabel
     OpStore %var %uint_2
     OpBranch %50

     %50 = OpLabel
     OpStore %var %uint_3
     OpBranch %60

     %60 = OpLabel
     OpStore %var %uint_4
     OpBranch %20

     %99 = OpLabel
     OpStore %var %999
     OpReturn

     OpFunctionEnd
  )"));
    ASSERT_TRUE(p->BuildAndParseInternalModuleExceptFunctions()) << p->error();
    auto fe = p->function_emitter(100);
    EXPECT_TRUE(fe.EmitBody()) << p->error();

    auto ast_body = fe.ast_body();
    auto got = test::ToString(p->program(), ast_body);
    auto* expect = R"(var_1 = 0u;
loop {
  var_1 = 1u;
  var_1 = 2u;

  continuing {
    var_1 = 3u;
    var_1 = 4u;
  }
}
var_1 = 999u;
return;
)";
    ASSERT_EQ(expect, got);
}

TEST_F(SpvParserCFGTest, EmitBody_Loop_Unconditional_Body_ContinueNestIf) {
    auto p = parser(test::Assemble(CommonTypes() + R"(
     %100 = OpFunction %void None %voidfn

     %10 = OpLabel
     OpStore %var %uint_0
     OpBranch %20

     %20 = OpLabel
     OpStore %var %uint_1
     OpLoopMerge %99 %50 None
     OpBranch %30

     %30 = OpLabel
     OpStore %var %uint_2
     OpBranch %50

     %50 = OpLabel ; continue target; also if-header
     OpStore %var %uint_3
     OpSelectionMerge %80 None
     OpBranchConditional %cond2 %60 %80

     %60 = OpLabel
     OpStore %var %uint_4
     OpBranch %80

     %80 = OpLabel
     OpStore %var %uint_5
     OpBranch %20

     %99 = OpLabel
     OpStore %var %999
     OpReturn

     OpFunctionEnd
  )"));
    ASSERT_TRUE(p->BuildAndParseInternalModuleExceptFunctions()) << p->error();
    auto fe = p->function_emitter(100);
    EXPECT_TRUE(fe.EmitBody()) << p->error();

    auto ast_body = fe.ast_body();
    auto got = test::ToString(p->program(), ast_body);
    auto* expect = R"(var_1 = 0u;
loop {
  var_1 = 1u;
  var_1 = 2u;

  continuing {
    var_1 = 3u;
    if (true) {
      var_1 = 4u;
    }
    var_1 = 5u;
  }
}
var_1 = 999u;
return;
)";
    ASSERT_EQ(expect, got);
}

TEST_F(SpvParserCFGTest, EmitBody_Loop_MultiBlockContinueIsEntireLoop) {
    // Test case where both branches exit. e.g both go to merge.
    auto p = parser(test::Assemble(CommonTypes() + R"(
     %100 = OpFunction %void None %voidfn

     %10 = OpLabel
     OpStore %var %uint_0
     OpBranch %20

     %20 = OpLabel ; its own continue target
     OpStore %var %uint_1
     OpLoopMerge %99 %20 None
     OpBranch %80

     %80 = OpLabel
     OpStore %var %uint_2
     OpBranchConditional %cond %99 %20

     %99 = OpLabel
     OpStore %var %uint_3
     OpReturn

     OpFunctionEnd
  )"));
    ASSERT_TRUE(p->BuildAndParseInternalModuleExceptFunctions()) << p->error();
    auto fe = p->function_emitter(100);
    EXPECT_TRUE(fe.EmitBody()) << p->error();
    auto ast_body = fe.ast_body();
    auto got = test::ToString(p->program(), ast_body);
    auto* expect = R"(var_1 = 0u;
loop {
  var_1 = 1u;
  var_1 = 2u;
  if (false) {
    break;
  }
}
var_1 = 3u;
return;
)";
    ASSERT_EQ(expect, got);

    // Continue target does not structurally dominate the backedge block.
    p->DeliberatelyInvalidSpirv();
}

TEST_F(SpvParserCFGTest, EmitBody_Loop_Never) {
    // Test case where both branches exit. e.g both go to merge.
    auto p = parser(test::Assemble(CommonTypes() + R"(
     %100 = OpFunction %void None %voidfn

     %10 = OpLabel
     OpBranch %20

     %20 = OpLabel
     OpStore %var %uint_1
     OpLoopMerge %99 %80 None
     OpBranchConditional %cond %99 %99

     %80 = OpLabel ; continue target
     OpStore %var %uint_2
     OpBranch %20

     %99 = OpLabel
     OpStore %var %uint_3
     OpReturn

     OpFunctionEnd
  )"));
    ASSERT_TRUE(p->BuildAndParseInternalModuleExceptFunctions()) << p->error();
    auto fe = p->function_emitter(100);
    EXPECT_TRUE(fe.EmitBody()) << p->error();
    auto ast_body = fe.ast_body();
    auto got = test::ToString(p->program(), ast_body);
    auto* expect = R"(loop {
  var_1 = 1u;
  break;

  continuing {
    var_1 = 2u;
  }
}
var_1 = 3u;
return;
)";
    ASSERT_EQ(expect, got);
}

TEST_F(SpvParserCFGTest, EmitBody_Loop_HeaderBreakAndContinue) {
    // Header block branches to merge, and to an outer continue.
    // This is disallowed by the rule:
    //    "a continue block is valid only for the innermost loop it is nested
    //    inside of"
    // See test ClassifyCFGEdges_LoopContinue_FromNestedLoopHeader_IsError
}

TEST_F(SpvParserCFGTest, EmitBody_Loop_TrueToBody_FalseBreaks) {
    auto p = parser(test::Assemble(CommonTypes() + R"(
     %100 = OpFunction %void None %voidfn

     %10 = OpLabel
     OpBranch %20

     %20 = OpLabel
     OpStore %var %uint_1
     OpLoopMerge %99 %80 None
     OpBranchConditional %cond %30 %99

     %30 = OpLabel
     OpStore %var %uint_2
     OpBranch %80

     %80 = OpLabel ; continue target
     OpStore %var %uint_3
     OpBranch %20

     %99 = OpLabel
     OpStore %var %uint_4
     OpReturn

     OpFunctionEnd
  )"));
    ASSERT_TRUE(p->BuildAndParseInternalModuleExceptFunctions()) << p->error();
    auto fe = p->function_emitter(100);
    EXPECT_TRUE(fe.EmitBody()) << p->error();
    auto ast_body = fe.ast_body();
    auto got = test::ToString(p->program(), ast_body);
    auto* expect = R"(loop {
  var_1 = 1u;
  if (false) {
  } else {
    break;
  }
  var_1 = 2u;

  continuing {
    var_1 = 3u;
  }
}
var_1 = 4u;
return;
)";
    ASSERT_EQ(expect, got);
}

TEST_F(SpvParserCFGTest, EmitBody_Loop_FalseToBody_TrueBreaks) {
    auto p = parser(test::Assemble(CommonTypes() + R"(
     %100 = OpFunction %void None %voidfn

     %10 = OpLabel
     OpBranch %20

     %20 = OpLabel
     OpStore %var %uint_1
     OpLoopMerge %99 %80 None
     OpBranchConditional %cond %30 %99

     %30 = OpLabel
     OpStore %var %uint_2
     OpBranch %80

     %80 = OpLabel ; continue target
     OpStore %var %uint_3
     OpBranch %20

     %99 = OpLabel
     OpStore %var %uint_4
     OpReturn

     OpFunctionEnd
  )"));
    ASSERT_TRUE(p->BuildAndParseInternalModuleExceptFunctions()) << p->error();
    auto fe = p->function_emitter(100);
    EXPECT_TRUE(fe.EmitBody()) << p->error();
    auto ast_body = fe.ast_body();
    auto got = test::ToString(p->program(), ast_body);
    auto* expect = R"(loop {
  var_1 = 1u;
  if (false) {
  } else {
    break;
  }
  var_1 = 2u;

  continuing {
    var_1 = 3u;
  }
}
var_1 = 4u;
return;
)";
    ASSERT_EQ(expect, got);
}

TEST_F(SpvParserCFGTest, EmitBody_Loop_NestedIfContinue) {
    // By construction, it has to come from nested code.
    auto p = parser(test::Assemble(CommonTypes() + R"(
     %100 = OpFunction %void None %voidfn

     %10 = OpLabel
     OpBranch %20

     %20 = OpLabel
     OpLoopMerge %99 %80 None
     OpBranch %30

     %30 = OpLabel
     OpSelectionMerge %50 None
     OpBranchConditional %cond %40 %50

     %40 = OpLabel
     OpStore %var %uint_1
     OpBranch %80 ; continue edge

     %50 = OpLabel ; inner selection merge
     OpStore %var %uint_2
     OpBranch %80

     %80 = OpLabel ; continue target
     OpStore %var %uint_3
     OpBranch %20

     %99 = OpLabel
     OpReturn

     OpFunctionEnd
  )"));
    ASSERT_TRUE(p->BuildAndParseInternalModuleExceptFunctions()) << p->error();
    auto fe = p->function_emitter(100);
    EXPECT_TRUE(fe.EmitBody()) << p->error();
    auto ast_body = fe.ast_body();
    auto got = test::ToString(p->program(), ast_body);
    auto* expect = R"(loop {
  if (false) {
    var_1 = 1u;
    continue;
  }
  var_1 = 2u;

  continuing {
    var_1 = 3u;
  }
}
return;
)";
    ASSERT_EQ(expect, got);
}

TEST_F(SpvParserCFGTest, EmitBody_Loop_BodyAlwaysBreaks) {
    auto p = parser(test::Assemble(CommonTypes() + R"(
     %100 = OpFunction %void None %voidfn

     %10 = OpLabel
     OpBranch %20

     %20 = OpLabel
     OpLoopMerge %99 %80 None
     OpBranch %30

     %30 = OpLabel
     OpStore %var %uint_1
     OpBranch %99 ; break is here

     %80 = OpLabel
     OpStore %var %uint_2
     OpBranch %20 ; backedge

     %99 = OpLabel
     OpReturn

     OpFunctionEnd
  )"));
    ASSERT_TRUE(p->BuildAndParseInternalModuleExceptFunctions()) << p->error();
    auto fe = p->function_emitter(100);
    EXPECT_TRUE(fe.EmitBody()) << p->error();

    auto ast_body = fe.ast_body();
    auto got = test::ToString(p->program(), ast_body);
    auto* expect = R"(loop {
  var_1 = 1u;
  break;

  continuing {
    var_1 = 2u;
  }
}
return;
)";
    ASSERT_EQ(expect, got);
}

TEST_F(SpvParserCFGTest, EmitBody_Loop_BodyConditionallyBreaks_FromTrue) {
    // The else-branch has a continue but it's skipped because it's from a
    // block that immediately precedes the continue construct.
    auto p = parser(test::Assemble(CommonTypes() + R"(
     %100 = OpFunction %void None %voidfn

     %10 = OpLabel
     OpBranch %20

     %20 = OpLabel
     OpLoopMerge %99 %80 None
     OpBranch %30

     %30 = OpLabel
     OpStore %var %uint_1
     OpBranchConditional %cond %99 %80

     %80 = OpLabel
     OpStore %var %uint_2
     OpBranch %20 ; backedge

     %99 = OpLabel
     OpReturn

     OpFunctionEnd
  )"));
    ASSERT_TRUE(p->BuildAndParseInternalModuleExceptFunctions()) << p->error();
    auto fe = p->function_emitter(100);
    EXPECT_TRUE(fe.EmitBody()) << p->error();

    auto ast_body = fe.ast_body();
    auto got = test::ToString(p->program(), ast_body);
    auto* expect = R"(loop {
  var_1 = 1u;
  if (false) {
    break;
  }

  continuing {
    var_1 = 2u;
  }
}
return;
)";
    ASSERT_EQ(expect, got);
}

TEST_F(SpvParserCFGTest, EmitBody_Loop_BodyConditionallyBreaks_FromFalse) {
    // The else-branch has a continue but it's skipped because it's from a
    // block that immediately precedes the continue construct.
    auto p = parser(test::Assemble(CommonTypes() + R"(
     %100 = OpFunction %void None %voidfn

     %10 = OpLabel
     OpBranch %20

     %20 = OpLabel
     OpLoopMerge %99 %80 None
     OpBranch %30

     %30 = OpLabel
     OpStore %var %uint_1
     OpBranchConditional %cond %80 %99

     %80 = OpLabel
     OpStore %var %uint_2
     OpBranch %20 ; backedge

     %99 = OpLabel
     OpReturn

     OpFunctionEnd
  )"));
    ASSERT_TRUE(p->BuildAndParseInternalModuleExceptFunctions()) << p->error();
    auto fe = p->function_emitter(100);
    EXPECT_TRUE(fe.EmitBody()) << p->error();

    auto ast_body = fe.ast_body();
    auto got = test::ToString(p->program(), ast_body);
    auto* expect = R"(loop {
  var_1 = 1u;
  if (false) {
  } else {
    break;
  }

  continuing {
    var_1 = 2u;
  }
}
return;
)";
    ASSERT_EQ(expect, got);
}

TEST_F(SpvParserCFGTest, EmitBody_Loop_BodyConditionallyBreaks_FromTrue_Early) {
    auto p = parser(test::Assemble(CommonTypes() + R"(
     %100 = OpFunction %void None %voidfn

     %10 = OpLabel
     OpBranch %20

     %20 = OpLabel
     OpLoopMerge %99 %80 None
     OpBranch %30

     %30 = OpLabel
     OpStore %var %uint_1
     OpBranchConditional %cond %99 %70

     %70 = OpLabel
     OpStore %var %uint_3
     OpBranch %80

     %80 = OpLabel
     OpStore %var %uint_2
     OpBranch %20 ; backedge

     %99 = OpLabel
     OpReturn

     OpFunctionEnd
  )"));
    ASSERT_TRUE(p->BuildAndParseInternalModuleExceptFunctions()) << p->error();
    auto fe = p->function_emitter(100);
    EXPECT_TRUE(fe.EmitBody()) << p->error();

    auto ast_body = fe.ast_body();
    auto got = test::ToString(p->program(), ast_body);
    auto* expect = R"(loop {
  var_1 = 1u;
  if (false) {
    break;
  }
  var_1 = 3u;

  continuing {
    var_1 = 2u;
  }
}
return;
)";
    ASSERT_EQ(expect, got);
}

TEST_F(SpvParserCFGTest, EmitBody_Loop_BodyConditionallyBreaks_FromFalse_Early) {
    auto p = parser(test::Assemble(CommonTypes() + R"(
     %100 = OpFunction %void None %voidfn

     %10 = OpLabel
     OpBranch %20

     %20 = OpLabel
     OpLoopMerge %99 %80 None
     OpBranch %30

     %30 = OpLabel
     OpStore %var %uint_1
     OpBranchConditional %cond %70 %99

     %70 = OpLabel
     OpStore %var %uint_3
     OpBranch %80

     %80 = OpLabel
     OpStore %var %uint_2
     OpBranch %20 ; backedge

     %99 = OpLabel
     OpReturn

     OpFunctionEnd
  )"));
    ASSERT_TRUE(p->BuildAndParseInternalModuleExceptFunctions()) << p->error();
    auto fe = p->function_emitter(100);
    EXPECT_TRUE(fe.EmitBody()) << p->error();

    auto ast_body = fe.ast_body();
    auto got = test::ToString(p->program(), ast_body);
    auto* expect = R"(loop {
  var_1 = 1u;
  if (false) {
  } else {
    break;
  }
  var_1 = 3u;

  continuing {
    var_1 = 2u;
  }
}
return;
)";
    ASSERT_EQ(expect, got);
}

TEST_F(SpvParserCFGTest, EmitBody_Switch_DefaultIsMerge_NoCases) {
    auto p = parser(test::Assemble(CommonTypes() + R"(
     %100 = OpFunction %void None %voidfn

     %10 = OpLabel
     OpStore %var %uint_1
     OpSelectionMerge %99 None
     OpSwitch %selector %99

     %99 = OpLabel
     OpStore %var %uint_7
     OpReturn

     OpFunctionEnd
  )"));
    ASSERT_TRUE(p->BuildAndParseInternalModuleExceptFunctions()) << p->error();
    auto fe = p->function_emitter(100);
    EXPECT_TRUE(fe.EmitBody()) << p->error();

    auto ast_body = fe.ast_body();
    auto got = test::ToString(p->program(), ast_body);
    auto* expect = R"(var_1 = 1u;
switch(42u) {
  default: {
  }
}
var_1 = 7u;
return;
)";
    ASSERT_EQ(expect, got);
}

// First do no special control flow: no breaks, continues.
TEST_F(SpvParserCFGTest, EmitBody_Switch_DefaultIsMerge_OneCase) {
    auto p = parser(test::Assemble(CommonTypes() + R"(
     %100 = OpFunction %void None %voidfn

     %10 = OpLabel
     OpStore %var %uint_1
     OpSelectionMerge %99 None
     OpSwitch %selector %99 20 %20

     %20 = OpLabel
     OpStore %var %uint_20
     OpBranch %99

     %99 = OpLabel
     OpStore %var %uint_7
     OpReturn

     OpFunctionEnd
  )"));
    ASSERT_TRUE(p->BuildAndParseInternalModuleExceptFunctions()) << p->error();
    auto fe = p->function_emitter(100);
    EXPECT_TRUE(fe.EmitBody()) << p->error();

    auto ast_body = fe.ast_body();
    auto got = test::ToString(p->program(), ast_body);
    auto* expect = R"(var_1 = 1u;
switch(42u) {
  case 20u: {
    var_1 = 20u;
  }
  default: {
  }
}
var_1 = 7u;
return;
)";
    ASSERT_EQ(expect, got);
}

TEST_F(SpvParserCFGTest, EmitBody_Switch_DefaultIsMerge_TwoCases) {
    auto p = parser(test::Assemble(CommonTypes() + R"(
     %100 = OpFunction %void None %voidfn

     %10 = OpLabel
     OpStore %var %uint_1
     OpSelectionMerge %99 None
     OpSwitch %selector %99 20 %20 30 %30

     %20 = OpLabel
     OpStore %var %uint_20
     OpBranch %99

     %30 = OpLabel
     OpStore %var %uint_30
     OpBranch %99

     %99 = OpLabel
     OpStore %var %uint_7
     OpReturn

     OpFunctionEnd
  )"));
    ASSERT_TRUE(p->BuildAndParseInternalModuleExceptFunctions()) << p->error();
    auto fe = p->function_emitter(100);
    EXPECT_TRUE(fe.EmitBody()) << p->error();

    auto ast_body = fe.ast_body();
    auto got = test::ToString(p->program(), ast_body);
    auto* expect = R"(var_1 = 1u;
switch(42u) {
  case 30u: {
    var_1 = 30u;
  }
  case 20u: {
    var_1 = 20u;
  }
  default: {
  }
}
var_1 = 7u;
return;
)";
    ASSERT_EQ(expect, got);
}

TEST_F(SpvParserCFGTest, EmitBody_Switch_DefaultIsMerge_CasesWithDup) {
    auto p = parser(test::Assemble(CommonTypes() + R"(
     %100 = OpFunction %void None %voidfn

     %10 = OpLabel
     OpStore %var %uint_1
     OpSelectionMerge %99 None
     OpSwitch %selector %99 20 %20 30 %30 40 %20

     %20 = OpLabel
     OpStore %var %uint_20
     OpBranch %99

     %30 = OpLabel
     OpStore %var %uint_30
     OpBranch %99

     %99 = OpLabel
     OpStore %var %uint_7
     OpReturn

     OpFunctionEnd
  )"));
    ASSERT_TRUE(p->BuildAndParseInternalModuleExceptFunctions()) << p->error();
    auto fe = p->function_emitter(100);
    EXPECT_TRUE(fe.EmitBody()) << p->error();

    auto ast_body = fe.ast_body();
    auto got = test::ToString(p->program(), ast_body);
    auto* expect = R"(var_1 = 1u;
switch(42u) {
  case 30u: {
    var_1 = 30u;
  }
  case 20u, 40u: {
    var_1 = 20u;
  }
  default: {
  }
}
var_1 = 7u;
return;
)";
    ASSERT_EQ(expect, got);
}

TEST_F(SpvParserCFGTest, EmitBody_Switch_DefaultIsCase_NoDupCases) {
    // The default block is not the merge block. But not the same as a case
    // either.
    auto p = parser(test::Assemble(CommonTypes() + R"(
     %100 = OpFunction %void None %voidfn

     %10 = OpLabel
     OpStore %var %uint_1
     OpSelectionMerge %99 None
     OpSwitch %selector %30 20 %20 40 %40

     %20 = OpLabel
     OpStore %var %uint_20
     OpBranch %99

     %30 = OpLabel ; the named default block
     OpStore %var %uint_30
     OpBranch %99

     %40 = OpLabel
     OpStore %var %uint_40
     OpBranch %99

     %99 = OpLabel
     OpStore %var %uint_7
     OpReturn

     OpFunctionEnd
  )"));
    ASSERT_TRUE(p->BuildAndParseInternalModuleExceptFunctions()) << p->error();
    auto fe = p->function_emitter(100);
    EXPECT_TRUE(fe.EmitBody()) << p->error();

    auto ast_body = fe.ast_body();
    auto got = test::ToString(p->program(), ast_body);
    auto* expect = R"(var_1 = 1u;
switch(42u) {
  case 40u: {
    var_1 = 40u;
  }
  case 20u: {
    var_1 = 20u;
  }
  default: {
    var_1 = 30u;
  }
}
var_1 = 7u;
return;
)";
    ASSERT_EQ(expect, got);
}

TEST_F(SpvParserCFGTest, EmitBody_Switch_DefaultIsCase_WithDupCase) {
    // The default block is not the merge block and is the same as a case.
    // We emit the default case as part of the labeled case.
    auto p = parser(test::Assemble(CommonTypes() + R"(
     %100 = OpFunction %void None %voidfn

     %10 = OpLabel
     OpStore %var %uint_1
     OpSelectionMerge %99 None
     OpSwitch %selector %30 20 %20 30 %30 40 %40

     %20 = OpLabel
     OpStore %var %uint_20
     OpBranch %99

     %30 = OpLabel ; the named default block, also a case
     OpStore %var %uint_30
     OpBranch %99

     %40 = OpLabel
     OpStore %var %uint_40
     OpBranch %99

     %99 = OpLabel
     OpStore %var %uint_7
     OpReturn

     OpFunctionEnd
  )"));
    ASSERT_TRUE(p->BuildAndParseInternalModuleExceptFunctions()) << p->error();
    auto fe = p->function_emitter(100);
    EXPECT_TRUE(fe.EmitBody()) << p->error();

    auto ast_body = fe.ast_body();
    auto got = test::ToString(p->program(), ast_body);
    auto* expect = R"(var_1 = 1u;
switch(42u) {
  case 40u: {
    var_1 = 40u;
  }
  case 20u: {
    var_1 = 20u;
  }
  case 30u, default: {
    var_1 = 30u;
  }
}
var_1 = 7u;
return;
)";
    ASSERT_EQ(expect, got);
}

TEST_F(SpvParserCFGTest, EmitBody_Switch_Case_SintValue) {
    auto p = parser(test::Assemble(CommonTypes() + R"(
     %100 = OpFunction %void None %voidfn

     %10 = OpLabel
     OpStore %var %uint_1
     OpSelectionMerge %99 None
     ; SPIR-V assembler doesn't support negative literals in switch
     OpSwitch %signed_selector %99 20 %20 2000000000 %30 !4000000000 %40

     %20 = OpLabel
     OpStore %var %uint_20
     OpBranch %99

     %30 = OpLabel
     OpStore %var %uint_30
     OpBranch %99

     %40 = OpLabel
     OpStore %var %uint_40
     OpBranch %99

     %99 = OpLabel
     OpStore %var %uint_7
     OpReturn

     OpFunctionEnd
  )"));
    ASSERT_TRUE(p->BuildAndParseInternalModuleExceptFunctions()) << p->error();
    auto fe = p->function_emitter(100);
    EXPECT_TRUE(fe.EmitBody()) << p->error();

    auto ast_body = fe.ast_body();
    auto got = test::ToString(p->program(), ast_body);
    auto* expect = R"(var_1 = 1u;
switch(42i) {
  case -294967296i: {
    var_1 = 40u;
  }
  case 2000000000i: {
    var_1 = 30u;
  }
  case 20i: {
    var_1 = 20u;
  }
  default: {
  }
}
var_1 = 7u;
return;
)";
    ASSERT_EQ(expect, got);
}

TEST_F(SpvParserCFGTest, EmitBody_Switch_Case_UintValue) {
    auto p = parser(test::Assemble(CommonTypes() + R"(
     %100 = OpFunction %void None %voidfn

     %10 = OpLabel
     OpStore %var %uint_1
     OpSelectionMerge %99 None
     OpSwitch %selector %99 20 %20 2000000000 %30 50 %40

     %20 = OpLabel
     OpStore %var %uint_20
     OpBranch %99

     %30 = OpLabel
     OpStore %var %uint_30
     OpBranch %99

     %40 = OpLabel
     OpStore %var %uint_40
     OpBranch %99

     %99 = OpLabel
     OpStore %var %uint_7
     OpReturn

     OpFunctionEnd
  )"));
    ASSERT_TRUE(p->BuildAndParseInternalModuleExceptFunctions()) << p->error();
    auto fe = p->function_emitter(100);
    EXPECT_TRUE(fe.EmitBody()) << p->error();

    auto ast_body = fe.ast_body();
    auto got = test::ToString(p->program(), ast_body);
    auto* expect = R"(var_1 = 1u;
switch(42u) {
  case 50u: {
    var_1 = 40u;
  }
  case 2000000000u: {
    var_1 = 30u;
  }
  case 20u: {
    var_1 = 20u;
  }
  default: {
  }
}
var_1 = 7u;
return;
)";
    ASSERT_EQ(expect, got);
}

TEST_F(SpvParserCFGTest, EmitBody_Return_TopLevel) {
    auto p = parser(test::Assemble(CommonTypes() + R"(
     %100 = OpFunction %void None %voidfn

     %10 = OpLabel
     OpReturn

     OpFunctionEnd
  )"));
    ASSERT_TRUE(p->BuildAndParseInternalModuleExceptFunctions()) << p->error();
    auto fe = p->function_emitter(100);
    EXPECT_TRUE(fe.EmitBody()) << p->error();

    auto ast_body = fe.ast_body();
    auto got = test::ToString(p->program(), ast_body);
    auto* expect = R"(return;
)";
    ASSERT_EQ(expect, got);
}

TEST_F(SpvParserCFGTest, EmitBody_Return_InsideIf) {
    auto p = parser(test::Assemble(CommonTypes() + R"(
     %100 = OpFunction %void None %voidfn

     %10 = OpLabel
     OpSelectionMerge %99 None
     OpBranchConditional %cond %20 %99

     %20 = OpLabel
     OpReturn

     %99 = OpLabel
     OpReturn

     OpFunctionEnd
  )"));
    ASSERT_TRUE(p->BuildAndParseInternalModuleExceptFunctions()) << p->error();
    auto fe = p->function_emitter(100);
    EXPECT_TRUE(fe.EmitBody()) << p->error();

    auto ast_body = fe.ast_body();
    auto got = test::ToString(p->program(), ast_body);
    auto* expect = R"(if (false) {
  return;
}
return;
)";
    ASSERT_EQ(expect, got);
}

TEST_F(SpvParserCFGTest, EmitBody_Return_InsideLoop) {
    auto p = parser(test::Assemble(CommonTypes() + R"(
     %100 = OpFunction %void None %voidfn

     %10 = OpLabel
     OpBranch %20

     %20 = OpLabel
     OpLoopMerge %99 %80 None
     OpBranchConditional %cond %30 %30

     %30 = OpLabel
     OpReturn

     %80 = OpLabel
     OpBranch %20

     %99 = OpLabel
     OpReturn

     OpFunctionEnd
  )"));
    ASSERT_TRUE(p->BuildAndParseInternalModuleExceptFunctions()) << p->error();
    auto fe = p->function_emitter(100);
    EXPECT_TRUE(fe.EmitBody()) << p->error();

    auto ast_body = fe.ast_body();
    auto got = test::ToString(p->program(), ast_body);
    auto* expect = R"(loop {
  return;
}
return;
)";
    ASSERT_EQ(expect, got);
}

TEST_F(SpvParserCFGTest, EmitBody_ReturnValue_TopLevel) {
    auto p = parser(test::Assemble(CommonTypes() + R"(
     %200 = OpFunction %uint None %uintfn

     %210 = OpLabel
     OpReturnValue %uint_2

     OpFunctionEnd

     %100 = OpFunction %void None %voidfn

     %10 = OpLabel
     %11 = OpFunctionCall %uint %200
     OpReturn

     OpFunctionEnd
  )"));
    ASSERT_TRUE(p->BuildAndParseInternalModuleExceptFunctions()) << p->error();
    auto fe = p->function_emitter(200);
    EXPECT_TRUE(fe.EmitBody()) << p->error();

    auto ast_body = fe.ast_body();
    auto got = test::ToString(p->program(), ast_body);
    auto* expect = R"(return 2u;
)";
    ASSERT_EQ(expect, got);
}

TEST_F(SpvParserCFGTest, EmitBody_ReturnValue_InsideIf) {
    auto p = parser(test::Assemble(CommonTypes() + R"(
     %200 = OpFunction %uint None %uintfn

     %210 = OpLabel
     OpSelectionMerge %299 None
     OpBranchConditional %cond %220 %299

     %220 = OpLabel
     OpReturnValue %uint_2

     %299 = OpLabel
     OpReturnValue %uint_3

     OpFunctionEnd


     %100 = OpFunction %void None %voidfn

     %10 = OpLabel
     %11 = OpFunctionCall %uint %200
     OpReturn

     OpFunctionEnd
  )"));
    ASSERT_TRUE(p->BuildAndParseInternalModuleExceptFunctions()) << p->error();
    auto fe = p->function_emitter(200);
    EXPECT_TRUE(fe.EmitBody()) << p->error();

    auto ast_body = fe.ast_body();
    auto got = test::ToString(p->program(), ast_body);
    auto* expect = R"(if (false) {
  return 2u;
}
return 3u;
)";
    ASSERT_EQ(expect, got);
}

TEST_F(SpvParserCFGTest, EmitBody_ReturnValue_Loop) {
    auto p = parser(test::Assemble(CommonTypes() + R"(
     %200 = OpFunction %uint None %uintfn

     %210 = OpLabel
     OpBranch %220

     %220 = OpLabel
     OpLoopMerge %299 %280 None
     OpBranchConditional %cond %230 %230

     %230 = OpLabel
     OpReturnValue %uint_2

     %280 = OpLabel
     OpBranch %220

     %299 = OpLabel
     OpReturnValue %uint_3

     OpFunctionEnd


     %100 = OpFunction %void None %voidfn

     %10 = OpLabel
     %11 = OpFunctionCall %uint %200
     OpReturn

     OpFunctionEnd
  )"));
    ASSERT_TRUE(p->BuildAndParseInternalModuleExceptFunctions()) << p->error();
    auto fe = p->function_emitter(200);
    EXPECT_TRUE(fe.EmitBody()) << p->error();

    auto ast_body = fe.ast_body();
    auto got = test::ToString(p->program(), ast_body);
    auto* expect = R"(loop {
  return 2u;
}
return 3u;
)";
    ASSERT_EQ(expect, got);
}

TEST_F(SpvParserCFGTest, EmitBody_Kill_TopLevel) {
    auto p = parser(test::Assemble(CommonTypes() + R"(
     %100 = OpFunction %void None %voidfn

     %10 = OpLabel
     OpKill

     OpFunctionEnd
  )"));
    ASSERT_TRUE(p->BuildAndParseInternalModuleExceptFunctions()) << p->error();
    auto fe = p->function_emitter(100);
    EXPECT_TRUE(fe.EmitBody()) << p->error();

    auto ast_body = fe.ast_body();
    auto got = test::ToString(p->program(), ast_body);
    auto* expect = R"(discard;
)";
    ASSERT_EQ(expect, got);
}

TEST_F(SpvParserCFGTest, EmitBody_Kill_InsideIf) {
    auto p = parser(test::Assemble(CommonTypes() + R"(
     %100 = OpFunction %void None %voidfn

     %10 = OpLabel
     OpSelectionMerge %99 None
     OpBranchConditional %cond %20 %99

     %20 = OpLabel
     OpKill

     %99 = OpLabel
     OpKill

     OpFunctionEnd
  )"));
    ASSERT_TRUE(p->BuildAndParseInternalModuleExceptFunctions()) << p->error();
    auto fe = p->function_emitter(100);
    EXPECT_TRUE(fe.EmitBody()) << p->error();

    auto ast_body = fe.ast_body();
    auto got = test::ToString(p->program(), ast_body);
    auto* expect = R"(if (false) {
  discard;
}
discard;
)";
    ASSERT_EQ(expect, got);
}

TEST_F(SpvParserCFGTest, EmitBody_Kill_InsideLoop) {
    auto p = parser(test::Assemble(CommonTypes() + R"(
     %100 = OpFunction %void None %voidfn

     %10 = OpLabel
     OpBranch %20

     %20 = OpLabel
     OpLoopMerge %99 %80 None
     OpBranchConditional %cond %30 %30

     %30 = OpLabel
     OpKill

     %80 = OpLabel
     OpBranch %20

     %99 = OpLabel
     OpKill

     OpFunctionEnd
  )"));
    ASSERT_TRUE(p->BuildAndParseInternalModuleExceptFunctions()) << p->error();
    auto fe = p->function_emitter(100);
    EXPECT_TRUE(fe.EmitBody()) << p->error();

    auto ast_body = fe.ast_body();
    auto got = test::ToString(p->program(), ast_body);
    auto* expect = R"(loop {
  discard;
}
discard;
)";
    ASSERT_EQ(expect, got);
}

TEST_F(SpvParserCFGTest, EmitBody_Unreachable_TopLevel) {
    auto p = parser(test::Assemble(CommonTypes() + R"(
     %100 = OpFunction %void None %voidfn

     %10 = OpLabel
     OpUnreachable

     OpFunctionEnd
  )"));
    ASSERT_TRUE(p->BuildAndParseInternalModuleExceptFunctions()) << p->error();
    auto fe = p->function_emitter(100);
    EXPECT_TRUE(fe.EmitBody()) << p->error();

    auto ast_body = fe.ast_body();
    auto got = test::ToString(p->program(), ast_body);
    auto* expect = R"(return;
)";
    ASSERT_EQ(expect, got);
}

TEST_F(SpvParserCFGTest, EmitBody_Unreachable_InsideIf) {
    auto p = parser(test::Assemble(CommonTypes() + R"(
     %100 = OpFunction %void None %voidfn

     %10 = OpLabel
     OpSelectionMerge %99 None
     OpBranchConditional %cond %20 %99

     %20 = OpLabel
     OpUnreachable

     %99 = OpLabel
     OpReturn

     OpFunctionEnd
  )"));
    ASSERT_TRUE(p->BuildAndParseInternalModuleExceptFunctions()) << p->error();
    auto fe = p->function_emitter(100);
    EXPECT_TRUE(fe.EmitBody()) << p->error();

    auto ast_body = fe.ast_body();
    auto got = test::ToString(p->program(), ast_body);
    auto* expect = R"(if (false) {
  return;
}
return;
)";
    ASSERT_EQ(expect, got);
}

TEST_F(SpvParserCFGTest, EmitBody_Unreachable_InsideLoop) {
    auto p = parser(test::Assemble(CommonTypes() + R"(
     %100 = OpFunction %void None %voidfn

     %10 = OpLabel
     OpBranch %20

     %20 = OpLabel
     OpLoopMerge %99 %80 None
     OpBranchConditional %cond %30 %30

     %30 = OpLabel
     OpUnreachable

     %80 = OpLabel
     OpBranch %20

     %99 = OpLabel
     OpReturn

     OpFunctionEnd
  )"));
    ASSERT_TRUE(p->BuildAndParseInternalModuleExceptFunctions()) << p->error();
    auto fe = p->function_emitter(100);
    EXPECT_TRUE(fe.EmitBody()) << p->error();

    auto ast_body = fe.ast_body();
    auto got = test::ToString(p->program(), ast_body);
    auto* expect = R"(loop {
  return;
}
return;
)";
    ASSERT_EQ(expect, got);
}

TEST_F(SpvParserCFGTest, EmitBody_Unreachable_InNonVoidFunction) {
    auto p = parser(test::Assemble(CommonTypes() + R"(
     %200 = OpFunction %uint None %uintfn

     %210 = OpLabel
     OpUnreachable

     OpFunctionEnd

     %100 = OpFunction %void None %voidfn

     %10 = OpLabel
     %11 = OpFunctionCall %uint %200
     OpReturn

     OpFunctionEnd
  )"));
    ASSERT_TRUE(p->BuildAndParseInternalModuleExceptFunctions()) << p->error();
    auto fe = p->function_emitter(200);
    EXPECT_TRUE(fe.EmitBody()) << p->error();

    auto ast_body = fe.ast_body();
    auto got = test::ToString(p->program(), ast_body);
    auto* expect = R"(return 0u;
)";
    ASSERT_EQ(expect, got);
}

TEST_F(SpvParserCFGTest, EmitBody_Branch_BackEdge_MultiBlockLoop) {
    auto p = parser(test::Assemble(CommonTypes() + R"(
     %100 = OpFunction %void None %voidfn

     %10 = OpLabel
     OpBranch %20

     %20 = OpLabel
     OpLoopMerge %99 %80 None
     OpBranch %80

     %80 = OpLabel
     OpStore %var %uint_1
     OpBranch %20 ; here is one

     %99 = OpLabel
     OpReturn

     OpFunctionEnd
  )"));
    ASSERT_TRUE(p->BuildAndParseInternalModuleExceptFunctions()) << p->error();
    auto fe = p->function_emitter(100);
    EXPECT_TRUE(fe.EmitBody()) << p->error();

    auto ast_body = fe.ast_body();
    auto got = test::ToString(p->program(), ast_body);
    auto* expect = R"(loop {

  continuing {
    var_1 = 1u;
  }
}
return;
)";
    ASSERT_EQ(expect, got);
}

TEST_F(SpvParserCFGTest, EmitBody_Branch_BackEdge_SingleBlockLoop) {
    auto p = parser(test::Assemble(CommonTypes() + R"(
     %100 = OpFunction %void None %voidfn

     %10 = OpLabel
     OpBranch %20

     %20 = OpLabel
     OpStore %var %uint_1
     OpLoopMerge %99 %20 None
     OpBranch %20 ; backedge in single block loop

     %99 = OpLabel
     OpReturn

     OpFunctionEnd
  )"));
    ASSERT_TRUE(p->BuildAndParseInternalModuleExceptFunctions()) << p->error();
    auto fe = p->function_emitter(100);
    EXPECT_TRUE(fe.EmitBody()) << p->error();

    auto ast_body = fe.ast_body();
    auto got = test::ToString(p->program(), ast_body);
    auto* expect = R"(loop {
  var_1 = 1u;
}
return;
)";
    ASSERT_EQ(expect, got);
}

TEST_F(SpvParserCFGTest, EmitBody_Branch_SwitchBreak_LastInCase) {
    // When the break is last in its case, we omit it because it's implicit in
    // WGSL.
    auto p = parser(test::Assemble(CommonTypes() + R"(
     %100 = OpFunction %void None %voidfn

     %10 = OpLabel
     OpStore %var %uint_1
     OpSelectionMerge %99 None
     OpSwitch %selector %99 20 %20

     %20 = OpLabel
     OpStore %var %uint_20
     OpBranch %99 ; branch to merge. Last in case

     %99 = OpLabel
     OpStore %var %uint_7
     OpReturn

     OpFunctionEnd
  )"));
    ASSERT_TRUE(p->BuildAndParseInternalModuleExceptFunctions()) << p->error();
    auto fe = p->function_emitter(100);
    EXPECT_TRUE(fe.EmitBody()) << p->error();

    auto ast_body = fe.ast_body();
    auto got = test::ToString(p->program(), ast_body);
    auto* expect = R"(var_1 = 1u;
switch(42u) {
  case 20u: {
    var_1 = 20u;
  }
  default: {
  }
}
var_1 = 7u;
return;
)";
    ASSERT_EQ(expect, got);
}

TEST_F(SpvParserCFGTest, EmitBody_Branch_SwitchBreak_NotLastInCase) {
    // When the break is not last in its case, we must emit a 'break'
    auto p = parser(test::Assemble(CommonTypes() + R"(
     %100 = OpFunction %void None %voidfn

     %10 = OpLabel
     OpStore %var %uint_1
     OpSelectionMerge %99 None
     OpSwitch %selector %99 20 %20

     %20 = OpLabel
     OpStore %var %uint_20
     OpSelectionMerge %50 None
     OpBranchConditional %cond %40 %50

     %40 = OpLabel
     OpStore %var %uint_40
     OpBranch %99 ; branch to merge. Not last in case

     %50 = OpLabel ; inner merge
     OpStore %var %uint_50
     OpBranch %99

     %99 = OpLabel
     OpStore %var %uint_7
     OpReturn

     OpFunctionEnd
  )"));
    ASSERT_TRUE(p->BuildAndParseInternalModuleExceptFunctions()) << p->error();
    auto fe = p->function_emitter(100);
    EXPECT_TRUE(fe.EmitBody()) << p->error();

    auto ast_body = fe.ast_body();
    auto got = test::ToString(p->program(), ast_body);
    auto* expect = R"(var_1 = 1u;
switch(42u) {
  case 20u: {
    var_1 = 20u;
    if (false) {
      var_1 = 40u;
      break;
    }
    var_1 = 50u;
  }
  default: {
  }
}
var_1 = 7u;
return;
)";
    ASSERT_EQ(expect, got);
}

TEST_F(SpvParserCFGTest, EmitBody_Branch_LoopBreak_MultiBlockLoop_FromBody) {
    auto p = parser(test::Assemble(CommonTypes() + R"(
     %100 = OpFunction %void None %voidfn

     %10 = OpLabel
     OpBranch %20

     %20 = OpLabel
     OpLoopMerge %99 %80 None
     OpBranch %30

     %30 = OpLabel
     OpStore %var %uint_1
     OpBranch %99 ; break is here

     %80 = OpLabel
     OpStore %var %uint_2
     OpBranch %20 ; backedge

     %99 = OpLabel
     OpReturn

     OpFunctionEnd
  )"));
    ASSERT_TRUE(p->BuildAndParseInternalModuleExceptFunctions()) << p->error();
    auto fe = p->function_emitter(100);
    EXPECT_TRUE(fe.EmitBody()) << p->error();

    auto ast_body = fe.ast_body();
    auto got = test::ToString(p->program(), ast_body);
    auto* expect = R"(loop {
  var_1 = 1u;
  break;

  continuing {
    var_1 = 2u;
  }
}
return;
)";
    ASSERT_EQ(expect, got);
}

TEST_F(SpvParserCFGTest,
       EmitBody_Branch_LoopBreak_MultiBlockLoop_FromContinueConstructConditional) {
    // This case is invalid because the backedge block doesn't post-dominate the
    // continue target.
    auto p = parser(test::Assemble(CommonTypes() + R"(
     %100 = OpFunction %void None %voidfn

     %10 = OpLabel
     OpBranch %20

     %20 = OpLabel
     OpLoopMerge %99 %30 None
     OpBranch %30

     %30 = OpLabel ; continue target; also an if-header
     OpSelectionMerge %80 None
     OpBranchConditional %cond %40 %80

     %40 = OpLabel
     OpBranch %99 ; break, inside a nested if.

     %80 = OpLabel
     OpBranch %20 ; backedge

     %99 = OpLabel
     OpReturn

     OpFunctionEnd
  )"));
    ASSERT_TRUE(p->BuildAndParseInternalModuleExceptFunctions()) << p->error();
    auto fe = p->function_emitter(100);
    EXPECT_FALSE(fe.EmitBody()) << p->error();
    EXPECT_THAT(p->error(), Eq("Invalid exit (40->99) from continue construct: 40 is not the "
                               "last block in the continue construct starting at 30 "
                               "(violates post-dominance rule)"));
}

TEST_F(SpvParserCFGTest,
       EmitBody_Branch_LoopBreak_MultiBlockLoop_FromContinueConstructEnd_Unconditional) {
    auto p = parser(test::Assemble(CommonTypes() + R"(
     %100 = OpFunction %void None %voidfn

     %10 = OpLabel
     OpBranch %20

     %20 = OpLabel
     OpLoopMerge %99 %80 None
     OpBranch %80

     %80 = OpLabel ; continue target
     OpStore %var %uint_1
     OpBranch %99  ; should be a backedge
     ; This is invalid as there must be a backedge to the loop header.
     ; The SPIR-V allows this and understands how to emit it, even if it's not
     ; permitted by the SPIR-V validator.

     %99 = OpLabel
     OpReturn

     OpFunctionEnd
  )"));

    p->DeliberatelyInvalidSpirv();

    ASSERT_TRUE(p->BuildAndParseInternalModuleExceptFunctions()) << p->error();
    auto fe = p->function_emitter(100);
    EXPECT_TRUE(fe.EmitBody()) << p->error();
    auto ast_body = fe.ast_body();
    auto got = test::ToString(p->program(), ast_body);
    auto* expect = R"(loop {

  continuing {
    var_1 = 1u;
    break;
  }
}
return;
)";
    ASSERT_EQ(expect, got);
}

TEST_F(SpvParserCFGTest,
       EmitBody_Branch_LoopBreak_MultiBlockLoop_FromContinueConstructEnd_Conditional_BreakIf) {
    auto p = parser(test::Assemble(CommonTypes() + R"(
     %100 = OpFunction %void None %voidfn

     %10 = OpLabel
     OpBranch %20

     %20 = OpLabel
     OpLoopMerge %99 %80 None
     OpBranch %80

     %80 = OpLabel ; continue target
     OpStore %var %uint_1
     OpBranchConditional %cond %99 %20  ; exit, and backedge

     %99 = OpLabel
     OpReturn

     OpFunctionEnd
  )"));
    ASSERT_TRUE(p->BuildAndParseInternalModuleExceptFunctions()) << p->error();
    auto fe = p->function_emitter(100);
    EXPECT_TRUE(fe.EmitBody()) << p->error();
    auto ast_body = fe.ast_body();
    auto got = test::ToString(p->program(), ast_body);
    auto* expect = R"(loop {

  continuing {
    var_1 = 1u;
    break if false;
  }
}
return;
)";
    ASSERT_EQ(expect, got);
}

TEST_F(SpvParserCFGTest,
       EmitBody_Branch_LoopBreak_MultiBlockLoop_FromContinueConstructEnd_Conditional) {
    auto p = parser(test::Assemble(CommonTypes() + R"(
     %100 = OpFunction %void None %voidfn

     %10 = OpLabel
     OpBranch %20

     %20 = OpLabel
     OpLoopMerge %99 %80 None
     OpBranch %80

     %80 = OpLabel ; continue target
     OpStore %var %uint_1
     OpBranchConditional %cond %20 %99  ; backedge, and exit

     %99 = OpLabel
     OpReturn

     OpFunctionEnd
  )"));
    ASSERT_TRUE(p->BuildAndParseInternalModuleExceptFunctions()) << p->error();
    auto fe = p->function_emitter(100);
    EXPECT_TRUE(fe.EmitBody()) << p->error();
    auto ast_body = fe.ast_body();
    auto got = test::ToString(p->program(), ast_body);
    auto* expect = R"(loop {

  continuing {
    var_1 = 1u;
    break if !(false);
  }
}
return;
)";
    ASSERT_EQ(expect, got);
}

TEST_F(SpvParserCFGTest, EmitBody_Branch_LoopBreak_FromContinueConstructTail) {
    auto p = parser(test::Assemble(CommonTypes() + R"(
     %100 = OpFunction %void None %voidfn

     %10 = OpLabel
     OpBranch %20

     %20 = OpLabel
     OpLoopMerge %99 %50 None
     OpBranchConditional %cond %30 %99
     %30 = OpLabel
     OpBranch %50
     %50 = OpLabel
     OpBranch %60
     %60 = OpLabel
     OpBranchConditional %cond %20 %99
     %99 = OpLabel
     OpReturn

     OpFunctionEnd
  )"));
    ASSERT_TRUE(p->BuildAndParseInternalModuleExceptFunctions()) << p->error();
    auto fe = p->function_emitter(100);
    EXPECT_TRUE(fe.EmitBody()) << p->error();
    auto ast_body = fe.ast_body();
    auto got = test::ToString(p->program(), ast_body);
    auto* expect = R"(loop {
  if (false) {
  } else {
    break;
  }

  continuing {
    break if !(false);
  }
}
return;
)";
    ASSERT_EQ(expect, got);
}

TEST_F(SpvParserCFGTest, EmitBody_Branch_LoopContinue_LastInLoopConstruct) {
    auto p = parser(test::Assemble(CommonTypes() + R"(
     %100 = OpFunction %void None %voidfn

     %10 = OpLabel
     OpBranch %20

     %20 = OpLabel
     OpLoopMerge %99 %80 None
     OpBranch %30

     %30 = OpLabel
     OpStore %var %uint_1
     OpBranch %80 ; continue edge from last block before continue target

     %80 = OpLabel ; continue target
     OpStore %var %uint_2
     OpBranch %20

     %99 = OpLabel
     OpReturn

     OpFunctionEnd
  )"));
    ASSERT_TRUE(p->BuildAndParseInternalModuleExceptFunctions()) << p->error();
    auto fe = p->function_emitter(100);
    EXPECT_TRUE(fe.EmitBody()) << p->error();
    auto ast_body = fe.ast_body();
    auto got = test::ToString(p->program(), ast_body);
    auto* expect = R"(loop {
  var_1 = 1u;

  continuing {
    var_1 = 2u;
  }
}
return;
)";
    ASSERT_EQ(expect, got);
}

TEST_F(SpvParserCFGTest, EmitBody_Branch_LoopContinue_BeforeLast) {
    // By construction, it has to come from nested code.
    auto p = parser(test::Assemble(CommonTypes() + R"(
     %100 = OpFunction %void None %voidfn

     %10 = OpLabel
     OpBranch %20

     %20 = OpLabel
     OpLoopMerge %99 %80 None
     OpBranch %30

     %30 = OpLabel
     OpSelectionMerge %50 None
     OpBranchConditional %cond %40 %50

     %40 = OpLabel
     OpStore %var %uint_1
     OpBranch %80 ; continue edge

     %50 = OpLabel ; inner selection merge
     OpStore %var %uint_2
     OpBranch %80

     %80 = OpLabel ; continue target
     OpStore %var %uint_3
     OpBranch %20

     %99 = OpLabel
     OpReturn

     OpFunctionEnd
  )"));
    ASSERT_TRUE(p->BuildAndParseInternalModuleExceptFunctions()) << p->error();
    auto fe = p->function_emitter(100);
    EXPECT_TRUE(fe.EmitBody()) << p->error();
    auto ast_body = fe.ast_body();
    auto got = test::ToString(p->program(), ast_body);
    auto* expect = R"(loop {
  if (false) {
    var_1 = 1u;
    continue;
  }
  var_1 = 2u;

  continuing {
    var_1 = 3u;
  }
}
return;
)";
    ASSERT_EQ(expect, got);
}

TEST_F(SpvParserCFGTest, EmitBody_Branch_LoopContinue_FromSwitch) {
    auto p = parser(test::Assemble(CommonTypes() + R"(
     %100 = OpFunction %void None %voidfn

     %10 = OpLabel
     OpStore %var %uint_1
     OpBranch %20

     %20 = OpLabel
     OpStore %var %uint_2
     OpLoopMerge %99 %80 None
     OpBranch %30

     %30 = OpLabel
     OpStore %var %uint_3
     OpSelectionMerge %79 None
     OpSwitch %selector %79 40 %40

     %40 = OpLabel
     OpStore %var %uint_4
     OpBranch %80 ; continue edge

     %79 = OpLabel ; switch merge
     OpStore %var %uint_5
     OpBranch %80

     %80 = OpLabel ; continue target
     OpStore %var %uint_6
     OpBranch %20

     %99 = OpLabel
     OpStore %var %uint_7
     OpReturn

     OpFunctionEnd
  )"));
    ASSERT_TRUE(p->BuildAndParseInternalModuleExceptFunctions()) << p->error();
    auto fe = p->function_emitter(100);
    EXPECT_TRUE(fe.EmitBody()) << p->error();
    auto ast_body = fe.ast_body();
    auto got = test::ToString(p->program(), ast_body);
    auto* expect = R"(var_1 = 1u;
loop {
  var_1 = 2u;
  var_1 = 3u;
  switch(42u) {
    case 40u: {
      var_1 = 4u;
      continue;
    }
    default: {
    }
  }
  var_1 = 5u;

  continuing {
    var_1 = 6u;
  }
}
var_1 = 7u;
return;
)";
    ASSERT_EQ(expect, got);
}

TEST_F(SpvParserCFGTest, EmitBody_Branch_IfBreak_FromThen) {
    // When unconditional, the if-break must be last in the then clause.
    auto p = parser(test::Assemble(CommonTypes() + R"(
     %100 = OpFunction %void None %voidfn

     %10 = OpLabel
     OpSelectionMerge %99 None
     OpBranchConditional %cond %30 %99

     %30 = OpLabel
     OpStore %var %uint_1
     OpBranch %99

     %99 = OpLabel
     OpStore %var %uint_2
     OpReturn

     OpFunctionEnd
  )"));
    ASSERT_TRUE(p->BuildAndParseInternalModuleExceptFunctions()) << p->error();
    auto fe = p->function_emitter(100);
    EXPECT_TRUE(fe.EmitBody()) << p->error();
    auto ast_body = fe.ast_body();
    auto got = test::ToString(p->program(), ast_body);
    auto* expect = R"(if (false) {
  var_1 = 1u;
}
var_1 = 2u;
return;
)";
    ASSERT_EQ(expect, got);
}

TEST_F(SpvParserCFGTest, EmitBody_Branch_IfBreak_FromElse) {
    // When unconditional, the if-break must be last in the else clause.
    auto p = parser(test::Assemble(CommonTypes() + R"(
     %100 = OpFunction %void None %voidfn

     %10 = OpLabel
     OpSelectionMerge %99 None
     OpBranchConditional %cond %99 %30

     %30 = OpLabel
     OpStore %var %uint_1
     OpBranch %99

     %99 = OpLabel
     OpStore %var %uint_2
     OpReturn

     OpFunctionEnd
  )"));
    ASSERT_TRUE(p->BuildAndParseInternalModuleExceptFunctions()) << p->error();
    auto fe = p->function_emitter(100);
    EXPECT_TRUE(fe.EmitBody()) << p->error();
    auto ast_body = fe.ast_body();
    auto got = test::ToString(p->program(), ast_body);
    auto* expect = R"(if (false) {
} else {
  var_1 = 1u;
}
var_1 = 2u;
return;
)";
    ASSERT_EQ(expect, got);
}

TEST_F(SpvParserCFGTest, EmitBody_Branch_Forward) {
    auto p = parser(test::Assemble(CommonTypes() + R"(
     %100 = OpFunction %void None %voidfn

     %10 = OpLabel
     OpStore %var %uint_1
     OpBranch %99 ; forward

     %99 = OpLabel
     OpStore %var %uint_2
     OpReturn

     OpFunctionEnd
  )"));
    ASSERT_TRUE(p->BuildAndParseInternalModuleExceptFunctions()) << p->error();
    auto fe = p->function_emitter(100);
    EXPECT_TRUE(fe.EmitBody()) << p->error();
    auto ast_body = fe.ast_body();
    auto got = test::ToString(p->program(), ast_body);
    auto* expect = R"(var_1 = 1u;
var_1 = 2u;
return;
)";
    ASSERT_EQ(expect, got);
}

// Test matrix for normal OpBranchConditional:
//
//    kBack with:
//      kBack : TESTED dup general case
//      kSwitchBreak: invalid (invalid escape, or invalid backedge)
//      kLoopBreak: TESTED in single- and multi block loop configurations
//      kLoopContinue: invalid
//                      If single block loop, then the continue is backward
//                      If continue is forward, then it's a continue from a
//                      continue which is also invalid.
//      kIfBreak: invalid: loop and if must have distinct merge blocks
//      kForward: impossible; would be a loop break
//
//    kSwitchBreak with:
//      kBack : symmetry
//      kSwitchBreak: dup general case
//      kLoopBreak: invalid; only one kind of break allowed
//      kLoopContinue: TESTED
//      kIfBreak: invalid: switch and if must have distinct merge blocks
//      kForward: TESTED
//
//    kLoopBreak with:
//      kBack : symmetry
//      kSwitchBreak: symmetry
//      kLoopBreak: dup general case
//      kLoopContinue: TESTED
//      kIfBreak: invalid: switch and if must have distinct merge blocks
//      break kForward: TESTED
//
//    kLoopContinue with:
//      kBack : symmetry
//      kSwitchBreak: symmetry
//      kLoopBreak: symmetry
//      kLoopContinue: dup general case
//      kIfBreak: TESTED
//      kForward: TESTED
//
//    kIfBreak with:
//      kBack : symmetry
//      kSwitchBreak: symmetry
//      kLoopBreak: symmetry
//      kLoopContinue: symmetry
//      kIfBreak: dup general case
//      kForward: invalid: needs a merge instruction
//
//    kForward with:
//      kBack : symmetry
//      kSwitchBreak: symmetry
//      kLoopBreak: symmetry
//      kLoopContinue: symmetry
//      kIfBreak: symmetry
//      kForward: dup general case

TEST_F(SpvParserCFGTest, EmitBody_BranchConditional_Back_SingleBlock_Back) {
    auto p = parser(test::Assemble(CommonTypes() + R"(
     %100 = OpFunction %void None %voidfn

     %10 = OpLabel
     OpStore %var %uint_0
     OpBranch %20

     %20 = OpLabel
     OpStore %var %uint_1
     OpLoopMerge %99 %20 None
     OpBranchConditional %cond %20 %20

     %99 = OpLabel ; dead
     OpStore %var %uint_5
     OpReturn

     OpFunctionEnd
  )"));
    ASSERT_TRUE(p->BuildAndParseInternalModuleExceptFunctions()) << p->error();
    auto fe = p->function_emitter(100);
    EXPECT_TRUE(fe.EmitBody()) << p->error();
    auto ast_body = fe.ast_body();
    auto got = test::ToString(p->program(), ast_body);
    auto* expect = R"(var_1 = 0u;
loop {
  var_1 = 1u;
}
var_1 = 5u;
return;
)";
    ASSERT_EQ(expect, got);
}

TEST_F(SpvParserCFGTest, EmitBody_BranchConditional_Back_SingleBlock_LoopBreak_OnTrue) {
    auto p = parser(test::Assemble(CommonTypes() + R"(
     %100 = OpFunction %void None %voidfn

     %10 = OpLabel
     OpStore %var %uint_0
     OpBranch %20

     %20 = OpLabel
     OpStore %var %uint_1
     OpLoopMerge %99 %20 None
     OpBranchConditional %cond %99 %20

     %99 = OpLabel
     OpStore %var %uint_5
     OpReturn

     OpFunctionEnd
  )"));
    ASSERT_TRUE(p->BuildAndParseInternalModuleExceptFunctions()) << p->error();
    auto fe = p->function_emitter(100);
    EXPECT_TRUE(fe.EmitBody()) << p->error();
    auto ast_body = fe.ast_body();
    auto got = test::ToString(p->program(), ast_body);
    auto* expect = R"(var_1 = 0u;
loop {
  var_1 = 1u;
  if (false) {
    break;
  }
}
var_1 = 5u;
return;
)";
    ASSERT_EQ(expect, got);
}

TEST_F(SpvParserCFGTest, EmitBody_BranchConditional_Back_SingleBlock_LoopBreak_OnFalse) {
    auto p = parser(test::Assemble(CommonTypes() + R"(
     %100 = OpFunction %void None %voidfn

     %10 = OpLabel
     OpStore %var %uint_0
     OpBranch %20

     %20 = OpLabel
     OpStore %var %uint_1
     OpLoopMerge %99 %20 None
     OpBranchConditional %cond %20 %99

     %99 = OpLabel
     OpStore %var %uint_5
     OpReturn

     OpFunctionEnd
  )"));
    ASSERT_TRUE(p->BuildAndParseInternalModuleExceptFunctions()) << p->error();
    auto fe = p->function_emitter(100);
    EXPECT_TRUE(fe.EmitBody()) << p->error();
    auto ast_body = fe.ast_body();
    auto got = test::ToString(p->program(), ast_body);
    auto* expect = R"(var_1 = 0u;
loop {
  var_1 = 1u;
  if (false) {
  } else {
    break;
  }
}
var_1 = 5u;
return;
)";
    ASSERT_EQ(expect, got);
}

TEST_F(SpvParserCFGTest, EmitBody_BranchConditional_Back_MultiBlock_LoopBreak_OnTrue) {
    auto p = parser(test::Assemble(CommonTypes() + R"(
     %100 = OpFunction %void None %voidfn

     %10 = OpLabel
     OpStore %var %uint_0
     OpBranch %20

     %20 = OpLabel
     OpStore %var %uint_1
     OpLoopMerge %99 %80 None
     OpBranch %80

     %80 = OpLabel
     OpBranchConditional %cond %99 %20

     %99 = OpLabel
     OpStore %var %uint_5
     OpReturn

     OpFunctionEnd
  )"));
    ASSERT_TRUE(p->BuildAndParseInternalModuleExceptFunctions()) << p->error();
    auto fe = p->function_emitter(100);
    EXPECT_TRUE(fe.EmitBody()) << p->error();
    auto ast_body = fe.ast_body();
    auto got = test::ToString(p->program(), ast_body);
    auto* expect = R"(var_1 = 0u;
loop {
  var_1 = 1u;

  continuing {
    break if false;
  }
}
var_1 = 5u;
return;
)";
    ASSERT_EQ(expect, got);
}

TEST_F(SpvParserCFGTest, EmitBody_BranchConditional_Back_MultiBlock_LoopBreak_OnFalse) {
    auto p = parser(test::Assemble(CommonTypes() + R"(
     %100 = OpFunction %void None %voidfn

     %10 = OpLabel
     OpStore %var %uint_0
     OpBranch %20

     %20 = OpLabel
     OpStore %var %uint_1
     OpLoopMerge %99 %80 None
     OpBranch %80

     %80 = OpLabel
     OpBranchConditional %cond %20 %99

     %99 = OpLabel
     OpStore %var %uint_5
     OpReturn

     OpFunctionEnd
  )"));
    ASSERT_TRUE(p->BuildAndParseInternalModuleExceptFunctions()) << p->error();
    auto fe = p->function_emitter(100);
    EXPECT_TRUE(fe.EmitBody()) << p->error();
    auto ast_body = fe.ast_body();
    auto got = test::ToString(p->program(), ast_body);
    auto* expect = R"(var_1 = 0u;
loop {
  var_1 = 1u;

  continuing {
    break if !(false);
  }
}
var_1 = 5u;
return;
)";
    ASSERT_EQ(expect, got);
}

TEST_F(SpvParserCFGTest, EmitBody_BranchConditional_SwitchBreak_SwitchBreak_LastInCase) {
    // When the break is last in its case, we omit it because it's implicit in
    // WGSL.
    auto p = parser(test::Assemble(CommonTypes() + R"(
     %100 = OpFunction %void None %voidfn

     %10 = OpLabel
     OpStore %var %uint_1
     OpSelectionMerge %99 None
     OpSwitch %selector %99 20 %20

     %20 = OpLabel
     OpStore %var %uint_20
     OpBranchConditional %cond2 %99 %99 ; branch to merge. Last in case

     %99 = OpLabel
     OpStore %var %uint_7
     OpReturn

     OpFunctionEnd
  )"));
    ASSERT_TRUE(p->BuildAndParseInternalModuleExceptFunctions()) << p->error();
    auto fe = p->function_emitter(100);
    EXPECT_TRUE(fe.EmitBody()) << p->error();

    auto ast_body = fe.ast_body();
    auto got = test::ToString(p->program(), ast_body);
    auto* expect = R"(var_1 = 1u;
switch(42u) {
  case 20u: {
    var_1 = 20u;
  }
  default: {
  }
}
var_1 = 7u;
return;
)";
    ASSERT_EQ(expect, got);
}

TEST_F(SpvParserCFGTest, EmitBody_BranchConditional_SwitchBreak_SwitchBreak_NotLastInCase) {
    // When the break is not last in its case, we must emit a 'break'
    auto p = parser(test::Assemble(CommonTypes() + R"(
     %100 = OpFunction %void None %voidfn

     %10 = OpLabel
     OpStore %var %uint_1
     OpSelectionMerge %99 None
     OpSwitch %selector %99 20 %20

     %20 = OpLabel
     OpStore %var %uint_20
     OpSelectionMerge %50 None
     OpBranchConditional %cond %40 %50

     %40 = OpLabel
     OpStore %var %uint_40
     OpBranchConditional %cond2 %99 %99 ; branch to merge. Not last in case

     %50 = OpLabel ; inner merge
     OpStore %var %uint_50
     OpBranch %99

     %99 = OpLabel
     OpStore %var %uint_7
     OpReturn

     OpFunctionEnd
  )"));
    ASSERT_TRUE(p->BuildAndParseInternalModuleExceptFunctions()) << p->error();
    auto fe = p->function_emitter(100);
    EXPECT_TRUE(fe.EmitBody()) << p->error();

    auto ast_body = fe.ast_body();
    auto got = test::ToString(p->program(), ast_body);
    auto* expect = R"(var_1 = 1u;
switch(42u) {
  case 20u: {
    var_1 = 20u;
    if (false) {
      var_1 = 40u;
      break;
    }
    var_1 = 50u;
  }
  default: {
  }
}
var_1 = 7u;
return;
)";
    ASSERT_EQ(expect, got);
}

TEST_F(SpvParserCFGTest, EmitBody_BranchConditional_SwitchBreak_Continue_OnTrue) {
    auto p = parser(test::Assemble(CommonTypes() + R"(
     %100 = OpFunction %void None %voidfn

     %10 = OpLabel
     OpStore %var %uint_1
     OpBranch %20

     %20 = OpLabel
     OpStore %var %uint_2
     OpLoopMerge %99 %80 None
     OpBranch %30

     %30 = OpLabel
     OpStore %var %uint_3
     OpSelectionMerge %79 None
     OpSwitch %selector %79 40 %40

     %40 = OpLabel
     OpStore %var %uint_40
     OpBranchConditional %cond %80 %79 ; break; continue on true

     %79 = OpLabel
     OpStore %var %uint_6
     OpBranch %80

     %80 = OpLabel ; continue target
     OpStore %var %uint_7
     OpBranch %20

     %99 = OpLabel ; loop merge
     OpStore %var %uint_8
     OpReturn

     OpFunctionEnd
  )"));
    ASSERT_TRUE(p->BuildAndParseInternalModuleExceptFunctions()) << p->error();
    auto fe = p->function_emitter(100);
    EXPECT_TRUE(fe.EmitBody()) << p->error();

    auto ast_body = fe.ast_body();
    auto got = test::ToString(p->program(), ast_body);
    auto* expect = R"(var_1 = 1u;
loop {
  var_1 = 2u;
  var_1 = 3u;
  switch(42u) {
    case 40u: {
      var_1 = 40u;
      if (false) {
        continue;
      }
    }
    default: {
    }
  }
  var_1 = 6u;

  continuing {
    var_1 = 7u;
  }
}
var_1 = 8u;
return;
)";
    ASSERT_EQ(expect, got);
}

TEST_F(SpvParserCFGTest, EmitBody_BranchConditional_SwitchBreak_Continue_OnFalse) {
    auto p = parser(test::Assemble(CommonTypes() + R"(
     %100 = OpFunction %void None %voidfn

     %10 = OpLabel
     OpStore %var %uint_1
     OpBranch %20

     %20 = OpLabel
     OpStore %var %uint_2
     OpLoopMerge %99 %80 None
     OpBranch %30

     %30 = OpLabel
     OpStore %var %uint_3
     OpSelectionMerge %79 None
     OpSwitch %selector %79 40 %40

     %40 = OpLabel
     OpStore %var %uint_40
     OpBranchConditional %cond %79 %80 ; break; continue on false

     %79 = OpLabel
     OpStore %var %uint_6
     OpBranch %80

     %80 = OpLabel ; continue target
     OpStore %var %uint_7
     OpBranch %20

     %99 = OpLabel ; loop merge
     OpStore %var %uint_8
     OpReturn

     OpFunctionEnd
  )"));
    ASSERT_TRUE(p->BuildAndParseInternalModuleExceptFunctions()) << p->error();
    auto fe = p->function_emitter(100);
    EXPECT_TRUE(fe.EmitBody()) << p->error();

    auto ast_body = fe.ast_body();
    auto got = test::ToString(p->program(), ast_body);
    auto* expect = R"(var_1 = 1u;
loop {
  var_1 = 2u;
  var_1 = 3u;
  switch(42u) {
    case 40u: {
      var_1 = 40u;
      if (false) {
      } else {
        continue;
      }
    }
    default: {
    }
  }
  var_1 = 6u;

  continuing {
    var_1 = 7u;
  }
}
var_1 = 8u;
return;
)";
    ASSERT_EQ(expect, got);
}

TEST_F(SpvParserCFGTest, EmitBody_BranchConditional_SwitchBreak_Forward_OnTrue) {
    auto p = parser(test::Assemble(CommonTypes() + R"(
     %100 = OpFunction %void None %voidfn

     %10 = OpLabel
     OpStore %var %uint_1
     OpSelectionMerge %99 None
     OpSwitch %selector %99 20 %20

     %20 = OpLabel
     OpStore %var %uint_20
     OpBranchConditional %cond %30 %99 ; break; forward on true

     %30 = OpLabel
     OpStore %var %uint_30
     OpBranch %99

     %99 = OpLabel ; switch merge
     OpStore %var %uint_8
     OpReturn

     OpFunctionEnd
  )"));
    ASSERT_TRUE(p->BuildAndParseInternalModuleExceptFunctions()) << p->error();
    auto fe = p->function_emitter(100);
    EXPECT_TRUE(fe.EmitBody()) << p->error();
    auto ast_body = fe.ast_body();
    auto got = test::ToString(p->program(), ast_body);
    auto* expect = R"(var_1 = 1u;
switch(42u) {
  case 20u: {
    var_1 = 20u;
    if (false) {
    } else {
      break;
    }
    var_1 = 30u;
  }
  default: {
  }
}
var_1 = 8u;
return;
)";
    ASSERT_EQ(expect, got);
}

TEST_F(SpvParserCFGTest, EmitBody_BranchConditional_SwitchBreak_Forward_OnFalse) {
    auto p = parser(test::Assemble(CommonTypes() + R"(
     %100 = OpFunction %void None %voidfn

     %10 = OpLabel
     OpStore %var %uint_1
     OpSelectionMerge %99 None
     OpSwitch %selector %99 20 %20

     %20 = OpLabel
     OpStore %var %uint_20
     OpBranchConditional %cond %99 %30 ; break; forward on false

     %30 = OpLabel
     OpStore %var %uint_30
     OpBranch %99

     %99 = OpLabel ; switch merge
     OpStore %var %uint_8
     OpReturn

     OpFunctionEnd
  )"));
    ASSERT_TRUE(p->BuildAndParseInternalModuleExceptFunctions()) << p->error();
    auto fe = p->function_emitter(100);
    EXPECT_TRUE(fe.EmitBody()) << p->error();
    auto ast_body = fe.ast_body();
    auto got = test::ToString(p->program(), ast_body);
    auto* expect = R"(var_1 = 1u;
switch(42u) {
  case 20u: {
    var_1 = 20u;
    if (false) {
      break;
    }
    var_1 = 30u;
  }
  default: {
  }
}
var_1 = 8u;
return;
)";
    ASSERT_EQ(expect, got);
}

TEST_F(SpvParserCFGTest, EmitBody_BranchConditional_LoopBreak_SingleBlock_LoopBreak) {
    auto p = parser(test::Assemble(CommonTypes() + R"(
     %100 = OpFunction %void None %voidfn

     %10 = OpLabel
     OpStore %var %uint_0
     OpBranch %20

     %20 = OpLabel
     OpStore %var %uint_1
     OpLoopMerge %99 %80 None
     OpBranchConditional %cond %99 %99

     %80 = OpLabel ; continue target
     OpStore %var %uint_4
     OpBranch %20

     %99 = OpLabel
     OpStore %var %uint_5
     OpReturn

     OpFunctionEnd
  )"));
    ASSERT_TRUE(p->BuildAndParseInternalModuleExceptFunctions()) << p->error();
    auto fe = p->function_emitter(100);
    EXPECT_TRUE(fe.EmitBody()) << p->error();
    auto ast_body = fe.ast_body();
    auto got = test::ToString(p->program(), ast_body);
    auto* expect = R"(var_1 = 0u;
loop {
  var_1 = 1u;
  break;

  continuing {
    var_1 = 4u;
  }
}
var_1 = 5u;
return;
)";
    ASSERT_EQ(expect, got);
}

TEST_F(SpvParserCFGTest, EmitBody_BranchConditional_LoopBreak_MultiBlock_LoopBreak) {
    auto p = parser(test::Assemble(CommonTypes() + R"(
     %100 = OpFunction %void None %voidfn

     %10 = OpLabel
     OpStore %var %uint_0
     OpBranch %20

     %20 = OpLabel
     OpStore %var %uint_1
     OpLoopMerge %99 %80 None
     OpBranch %30

     %30 = OpLabel
     OpStore %var %uint_2
     OpBranchConditional %cond %99 %99

     %80 = OpLabel ; continue target
     OpStore %var %uint_4
     OpBranch %20

     %99 = OpLabel
     OpStore %var %uint_5
     OpReturn

     OpFunctionEnd
  )"));
    ASSERT_TRUE(p->BuildAndParseInternalModuleExceptFunctions()) << p->error();
    auto fe = p->function_emitter(100);
    EXPECT_TRUE(fe.EmitBody()) << p->error();
    auto ast_body = fe.ast_body();
    auto got = test::ToString(p->program(), ast_body);
    auto* expect = R"(var_1 = 0u;
loop {
  var_1 = 1u;
  var_1 = 2u;
  break;

  continuing {
    var_1 = 4u;
  }
}
var_1 = 5u;
return;
)";
    ASSERT_EQ(expect, got);
}

TEST_F(SpvParserCFGTest, EmitBody_BranchConditional_LoopBreak_Continue_OnTrue) {
    auto p = parser(test::Assemble(CommonTypes() + R"(
     %100 = OpFunction %void None %voidfn

     %10 = OpLabel
     OpStore %var %uint_0
     OpBranch %20

     %20 = OpLabel
     OpStore %var %uint_1
     OpLoopMerge %99 %80 None
     OpBranch %25

     ; Need this extra selection to make another block between
     ; %30 and the continue target, so we actually induce a Continue
     ; statement to exist.
     %25 = OpLabel
     OpSelectionMerge %40 None
     OpBranchConditional %cond2 %30 %40

     %30 = OpLabel
     OpStore %var %uint_2
; break; continue on true
     OpBranchConditional %cond %80 %99

     %40 = OpLabel
     OpStore %var %uint_3
     OpBranch %80

     %80 = OpLabel ; continue target
     OpStore %var %uint_4
     OpBranch %20

     %99 = OpLabel
     OpStore %var %uint_5
     OpReturn

     OpFunctionEnd
  )"));
    ASSERT_TRUE(p->BuildAndParseInternalModuleExceptFunctions()) << p->error();
    auto fe = p->function_emitter(100);
    EXPECT_TRUE(fe.EmitBody()) << p->error();
    auto ast_body = fe.ast_body();
    auto got = test::ToString(p->program(), ast_body);
    auto* expect = R"(var_1 = 0u;
loop {
  var_1 = 1u;
  if (true) {
    var_1 = 2u;
    if (false) {
      continue;
    } else {
      break;
    }
  }
  var_1 = 3u;

  continuing {
    var_1 = 4u;
  }
}
var_1 = 5u;
return;
)";
    ASSERT_EQ(expect, got);
}

TEST_F(SpvParserCFGTest, EmitBody_BranchConditional_LoopBreak_Continue_OnFalse) {
    auto p = parser(test::Assemble(CommonTypes() + R"(
     %100 = OpFunction %void None %voidfn

     %10 = OpLabel
     OpStore %var %uint_0
     OpBranch %20

     %20 = OpLabel
     OpStore %var %uint_1
     OpLoopMerge %99 %80 None
     OpBranch %25

     ; Need this extra selection to make another block between
     ; %30 and the continue target, so we actually induce a Continue
     ; statement to exist.
     %25 = OpLabel
     OpSelectionMerge %40 None
     OpBranchConditional %cond2 %30 %40

     %30 = OpLabel
     OpStore %var %uint_2
; break; continue on false
     OpBranchConditional %cond %99 %80

     %40 = OpLabel
     OpStore %var %uint_3
     OpBranch %80

     %80 = OpLabel ; continue target
     OpStore %var %uint_4
     OpBranch %20

     %99 = OpLabel
     OpStore %var %uint_5
     OpReturn

     OpFunctionEnd
  )"));
    ASSERT_TRUE(p->BuildAndParseInternalModuleExceptFunctions()) << p->error();
    auto fe = p->function_emitter(100);
    EXPECT_TRUE(fe.EmitBody()) << p->error();
    auto ast_body = fe.ast_body();
    auto got = test::ToString(p->program(), ast_body);
    auto* expect = R"(var_1 = 0u;
loop {
  var_1 = 1u;
  if (true) {
    var_1 = 2u;
    if (false) {
      break;
    } else {
      continue;
    }
  }
  var_1 = 3u;

  continuing {
    var_1 = 4u;
  }
}
var_1 = 5u;
return;
)";
    ASSERT_EQ(expect, got);
}

TEST_F(SpvParserCFGTest, EmitBody_BranchConditional_LoopBreak_Forward_OnTrue) {
    auto p = parser(test::Assemble(CommonTypes() + R"(
     %100 = OpFunction %void None %voidfn

     %10 = OpLabel
     OpStore %var %uint_0
     OpBranch %20

     %20 = OpLabel
     OpStore %var %uint_1
     OpLoopMerge %99 %80 None
     OpBranch %30

     %30 = OpLabel
     OpStore %var %uint_2
; break; forward on true
     OpBranchConditional %cond %40 %99

     %40 = OpLabel
     OpStore %var %uint_3
     OpBranch %80

     %80 = OpLabel ; continue target
     OpStore %var %uint_4
     OpBranch %20

     %99 = OpLabel
     OpStore %var %uint_5
     OpReturn

     OpFunctionEnd
  )"));
    ASSERT_TRUE(p->BuildAndParseInternalModuleExceptFunctions()) << p->error();
    auto fe = p->function_emitter(100);
    EXPECT_TRUE(fe.EmitBody()) << p->error();
    auto ast_body = fe.ast_body();
    auto got = test::ToString(p->program(), ast_body);
    auto* expect = R"(var_1 = 0u;
loop {
  var_1 = 1u;
  var_1 = 2u;
  if (false) {
  } else {
    break;
  }
  var_1 = 3u;

  continuing {
    var_1 = 4u;
  }
}
var_1 = 5u;
return;
)";
    ASSERT_EQ(expect, got);
}

TEST_F(SpvParserCFGTest, EmitBody_BranchConditional_LoopBreak_Forward_OnFalse) {
    auto p = parser(test::Assemble(CommonTypes() + R"(
     %100 = OpFunction %void None %voidfn

     %10 = OpLabel
     OpStore %var %uint_0
     OpBranch %20

     %20 = OpLabel
     OpStore %var %uint_1
     OpLoopMerge %99 %80 None
     OpBranch %30

     %30 = OpLabel
     OpStore %var %uint_2
; break; forward on false
     OpBranchConditional %cond %99 %40

     %40 = OpLabel
     OpStore %var %uint_3
     OpBranch %80

     %80 = OpLabel ; continue target
     OpStore %var %uint_4
     OpBranch %20

     %99 = OpLabel
     OpStore %var %uint_5
     OpReturn

     OpFunctionEnd
  )"));
    ASSERT_TRUE(p->BuildAndParseInternalModuleExceptFunctions()) << p->error();
    auto fe = p->function_emitter(100);
    EXPECT_TRUE(fe.EmitBody()) << p->error();
    auto ast_body = fe.ast_body();
    auto got = test::ToString(p->program(), ast_body);
    auto* expect = R"(var_1 = 0u;
loop {
  var_1 = 1u;
  var_1 = 2u;
  if (false) {
    break;
  }
  var_1 = 3u;

  continuing {
    var_1 = 4u;
  }
}
var_1 = 5u;
return;
)";
    ASSERT_EQ(expect, got);
}

TEST_F(SpvParserCFGTest, EmitBody_BranchConditional_Continue_Continue_FromHeader) {
    auto p = parser(test::Assemble(CommonTypes() + R"(
     %100 = OpFunction %void None %voidfn

     %10 = OpLabel
     OpStore %var %uint_0
     OpBranch %20

     %20 = OpLabel
     OpStore %var %uint_1
     OpLoopMerge %99 %80 None
     OpBranchConditional %cond %80 %80 ; to continue

     %80 = OpLabel ; continue target
     OpStore %var %uint_4
     OpBranch %20

     %99 = OpLabel
     OpStore %var %uint_5
     OpReturn

     OpFunctionEnd
  )"));
    ASSERT_TRUE(p->BuildAndParseInternalModuleExceptFunctions()) << p->error();
    auto fe = p->function_emitter(100);
    EXPECT_TRUE(fe.EmitBody()) << p->error();
    auto ast_body = fe.ast_body();
    auto got = test::ToString(p->program(), ast_body);
    auto* expect = R"(var_1 = 0u;
loop {
  var_1 = 1u;

  continuing {
    var_1 = 4u;
  }
}
var_1 = 5u;
return;
)";
    ASSERT_EQ(expect, got);
}

TEST_F(SpvParserCFGTest, EmitBody_BranchConditional_Continue_Continue_AfterHeader_Unconditional) {
    auto p = parser(test::Assemble(CommonTypes() + R"(
     %100 = OpFunction %void None %voidfn

     %10 = OpLabel
     OpStore %var %uint_0
     OpBranch %20

     %20 = OpLabel
     OpStore %var %uint_1
     OpLoopMerge %99 %80 None
     OpBranch %30

     %30 = OpLabel
     OpStore %var %uint_2
     OpBranchConditional %cond %80 %80 ; to continue

     %80 = OpLabel ; continue target
     OpStore %var %uint_4
     OpBranch %20

     %99 = OpLabel
     OpStore %var %uint_5
     OpReturn

     OpFunctionEnd
  )"));
    ASSERT_TRUE(p->BuildAndParseInternalModuleExceptFunctions()) << p->error();
    auto fe = p->function_emitter(100);
    EXPECT_TRUE(fe.EmitBody()) << p->error();
    auto ast_body = fe.ast_body();
    auto got = test::ToString(p->program(), ast_body);
    auto* expect = R"(var_1 = 0u;
loop {
  var_1 = 1u;
  var_1 = 2u;

  continuing {
    var_1 = 4u;
  }
}
var_1 = 5u;
return;
)";
    ASSERT_EQ(expect, got);
}

TEST_F(SpvParserCFGTest, EmitBody_BranchConditional_Continue_Continue_AfterHeader_Conditional) {
    // Create an intervening block so we actually require a "continue" statement
    auto p = parser(test::Assemble(CommonTypes() + R"(
     %100 = OpFunction %void None %voidfn

     %10 = OpLabel
     OpStore %var %uint_0
     OpBranch %20

     %20 = OpLabel
     OpStore %var %uint_1
     OpLoopMerge %99 %80 None
     OpBranch %30

     %30 = OpLabel
     OpStore %var %uint_2
     OpSelectionMerge %50 None
     OpBranchConditional %cond2 %40 %50

     %40 = OpLabel
     OpStore %var %uint_3
     OpBranchConditional %cond3 %80 %80 ; to continue

     %50 = OpLabel ; merge for selection
     OpStore %var %uint_4
     OpBranch %80

     %80 = OpLabel ; continue target
     OpStore %var %uint_5
     OpBranch %20

     %99 = OpLabel
     OpStore %var %uint_6
     OpReturn

     OpFunctionEnd
  )"));
    ASSERT_TRUE(p->BuildAndParseInternalModuleExceptFunctions()) << p->error();
    auto fe = p->function_emitter(100);
    EXPECT_TRUE(fe.EmitBody()) << p->error();
    auto ast_body = fe.ast_body();
    auto got = test::ToString(p->program(), ast_body);
    auto* expect = R"(var_1 = 0u;
loop {
  var_1 = 1u;
  var_1 = 2u;
  if (true) {
    var_1 = 3u;
    continue;
  }
  var_1 = 4u;

  continuing {
    var_1 = 5u;
  }
}
var_1 = 6u;
return;
)";
    ASSERT_EQ(expect, got);
}

TEST_F(SpvParserCFGTest,
       EmitBody_BranchConditional_Continue_Continue_AfterHeader_Conditional_EmptyContinuing) {
    // Like the previous tests, but with an empty continuing clause.
    auto p = parser(test::Assemble(CommonTypes() + R"(
     %100 = OpFunction %void None %voidfn

     %10 = OpLabel
     OpStore %var %uint_0
     OpBranch %20

     %20 = OpLabel
     OpStore %var %uint_1
     OpLoopMerge %99 %80 None
     OpBranch %30

     %30 = OpLabel
     OpStore %var %uint_2
     OpSelectionMerge %50 None
     OpBranchConditional %cond2 %40 %50

     %40 = OpLabel
     OpStore %var %uint_3
     OpBranchConditional %cond3 %80 %80 ; to continue

     %50 = OpLabel ; merge for selection
     OpStore %var %uint_4
     OpBranch %80

     %80 = OpLabel ; continue target
     ; no statements here.
     OpBranch %20

     %99 = OpLabel
     OpStore %var %uint_6
     OpReturn

     OpFunctionEnd
  )"));
    ASSERT_TRUE(p->BuildAndParseInternalModuleExceptFunctions()) << p->error();
    auto fe = p->function_emitter(100);
    EXPECT_TRUE(fe.EmitBody()) << p->error();
    auto ast_body = fe.ast_body();
    auto got = test::ToString(p->program(), ast_body);
    auto* expect = R"(var_1 = 0u;
loop {
  var_1 = 1u;
  var_1 = 2u;
  if (true) {
    var_1 = 3u;
    continue;
  }
  var_1 = 4u;
}
var_1 = 6u;
return;
)";
    ASSERT_EQ(expect, got);
}

TEST_F(SpvParserCFGTest, EmitBody_BranchConditional_LoopContinue_FromSwitch) {
    auto p = parser(test::Assemble(CommonTypes() + R"(
     %100 = OpFunction %void None %voidfn

     %10 = OpLabel
     OpStore %var %uint_1
     OpBranch %20

     %20 = OpLabel
     OpStore %var %uint_2
     OpLoopMerge %99 %80 None
     OpBranch %30

     %30 = OpLabel
     OpStore %var %uint_3
     OpSelectionMerge %79 None
     OpSwitch %selector %79 40 %40

     %40 = OpLabel
     OpStore %var %uint_4
     OpBranchConditional %cond2 %80 %80; dup continue edge

     %79 = OpLabel ; switch merge
     OpStore %var %uint_5
     OpBranch %80

     %80 = OpLabel ; continue target
     OpStore %var %uint_6
     OpBranch %20

     %99 = OpLabel
     OpStore %var %uint_7
     OpReturn

     OpFunctionEnd
  )"));
    ASSERT_TRUE(p->BuildAndParseInternalModuleExceptFunctions()) << p->error();
    auto fe = p->function_emitter(100);
    EXPECT_TRUE(fe.EmitBody()) << p->error();
    auto ast_body = fe.ast_body();
    auto got = test::ToString(p->program(), ast_body);
    auto* expect = R"(var_1 = 1u;
loop {
  var_1 = 2u;
  var_1 = 3u;
  switch(42u) {
    case 40u: {
      var_1 = 4u;
      continue;
    }
    default: {
    }
  }
  var_1 = 5u;

  continuing {
    var_1 = 6u;
  }
}
var_1 = 7u;
return;
)";
    ASSERT_EQ(expect, got);
}

TEST_F(SpvParserCFGTest, EmitBody_BranchConditional_Continue_IfBreak_OnTrue) {
    auto p = parser(test::Assemble(CommonTypes() + R"(
     %100 = OpFunction %void None %voidfn

     %10 = OpLabel
     OpStore %var %uint_0
     OpBranch %20

     %20 = OpLabel
     OpStore %var %uint_1
     OpLoopMerge %99 %80 None
     OpBranch %30

     %30 = OpLabel
     OpStore %var %uint_2
     OpSelectionMerge %50 None
     OpBranchConditional %cond2 %40 %50

     %40 = OpLabel
     OpStore %var %uint_3
 ; true to if's merge;  false to continue
     OpBranchConditional %cond3 %50 %80

     %50 = OpLabel ; merge for selection
     OpStore %var %uint_4
     OpBranch %80

     %80 = OpLabel ; continue target
     OpStore %var %uint_5
     OpBranch %20

     %99 = OpLabel
     OpStore %var %uint_6
     OpReturn

     OpFunctionEnd
  )"));
    ASSERT_TRUE(p->BuildAndParseInternalModuleExceptFunctions()) << p->error();
    auto fe = p->function_emitter(100);
    EXPECT_TRUE(fe.EmitBody()) << p->error();
    auto ast_body = fe.ast_body();
    auto got = test::ToString(p->program(), ast_body);
    auto* expect = R"(var_1 = 0u;
loop {
  var_1 = 1u;
  var_1 = 2u;
  if (true) {
    var_1 = 3u;
    if (false) {
    } else {
      continue;
    }
  }
  var_1 = 4u;

  continuing {
    var_1 = 5u;
  }
}
var_1 = 6u;
return;
)";
    ASSERT_EQ(expect, got);
}

TEST_F(SpvParserCFGTest, EmitBody_BranchConditional_Continue_IfBreak_OnFalse) {
    auto p = parser(test::Assemble(CommonTypes() + R"(
     %100 = OpFunction %void None %voidfn

     %10 = OpLabel
     OpStore %var %uint_0
     OpBranch %20

     %20 = OpLabel
     OpStore %var %uint_1
     OpLoopMerge %99 %80 None
     OpBranch %30

     %30 = OpLabel
     OpStore %var %uint_2
     OpSelectionMerge %50 None
     OpBranchConditional %cond2 %40 %50

     %40 = OpLabel
     OpStore %var %uint_3
 ; false to if's merge;  true to continue
     OpBranchConditional %cond3 %80 %50

     %50 = OpLabel ; merge for selection
     OpStore %var %uint_4
     OpBranch %80

     %80 = OpLabel ; continue target
     OpStore %var %uint_5
     OpBranch %20

     %99 = OpLabel
     OpStore %var %uint_6
     OpReturn

     OpFunctionEnd
  )"));
    ASSERT_TRUE(p->BuildAndParseInternalModuleExceptFunctions()) << p->error();
    auto fe = p->function_emitter(100);
    EXPECT_TRUE(fe.EmitBody()) << p->error();
    auto ast_body = fe.ast_body();
    auto got = test::ToString(p->program(), ast_body);
    auto* expect = R"(var_1 = 0u;
loop {
  var_1 = 1u;
  var_1 = 2u;
  if (true) {
    var_1 = 3u;
    if (false) {
      continue;
    }
  }
  var_1 = 4u;

  continuing {
    var_1 = 5u;
  }
}
var_1 = 6u;
return;
)";
    ASSERT_EQ(expect, got);
}

TEST_F(SpvParserCFGTest, EmitBody_BranchConditional_Continue_Forward_OnTrue) {
    auto p = parser(test::Assemble(CommonTypes() + R"(
     %100 = OpFunction %void None %voidfn

     %10 = OpLabel
     OpStore %var %uint_0
     OpBranch %20

     %20 = OpLabel
     OpStore %var %uint_1
     OpLoopMerge %99 %80 None
     OpBranch %30

     %30 = OpLabel
     OpStore %var %uint_2
; continue; forward on true
     OpBranchConditional %cond %40 %80

     %40 = OpLabel
     OpStore %var %uint_3
     OpBranch %80

     %80 = OpLabel ; continue target
     OpStore %var %uint_4
     OpBranch %20

     %99 = OpLabel
     OpStore %var %uint_5
     OpReturn

     OpFunctionEnd
  )"));
    ASSERT_TRUE(p->BuildAndParseInternalModuleExceptFunctions()) << p->error();
    auto fe = p->function_emitter(100);
    EXPECT_TRUE(fe.EmitBody()) << p->error();
    auto ast_body = fe.ast_body();
    auto got = test::ToString(p->program(), ast_body);
    auto* expect = R"(var_1 = 0u;
loop {
  var_1 = 1u;
  var_1 = 2u;
  if (false) {
  } else {
    continue;
  }
  var_1 = 3u;

  continuing {
    var_1 = 4u;
  }
}
var_1 = 5u;
return;
)";
    ASSERT_EQ(expect, got);
}

TEST_F(SpvParserCFGTest, EmitBody_BranchConditional_Continue_Forward_OnFalse) {
    auto p = parser(test::Assemble(CommonTypes() + R"(
     %100 = OpFunction %void None %voidfn

     %10 = OpLabel
     OpStore %var %uint_0
     OpBranch %20

     %20 = OpLabel
     OpStore %var %uint_1
     OpLoopMerge %99 %80 None
     OpBranch %30

     %30 = OpLabel
     OpStore %var %uint_2
; continue; forward on true
     OpBranchConditional %cond %80 %40

     %40 = OpLabel
     OpStore %var %uint_3
     OpBranch %80

     %80 = OpLabel ; continue target
     OpStore %var %uint_4
     OpBranch %20

     %99 = OpLabel
     OpStore %var %uint_5
     OpReturn

     OpFunctionEnd
  )"));
    ASSERT_TRUE(p->BuildAndParseInternalModuleExceptFunctions()) << p->error();
    auto fe = p->function_emitter(100);
    EXPECT_TRUE(fe.EmitBody()) << p->error();
    auto ast_body = fe.ast_body();
    auto got = test::ToString(p->program(), ast_body);
    auto* expect = R"(var_1 = 0u;
loop {
  var_1 = 1u;
  var_1 = 2u;
  if (false) {
    continue;
  }
  var_1 = 3u;

  continuing {
    var_1 = 4u;
  }
}
var_1 = 5u;
return;
)";
    ASSERT_EQ(expect, got);
}

TEST_F(SpvParserCFGTest, EmitBody_BranchConditional_IfBreak_IfBreak_Same) {
    auto p = parser(test::Assemble(CommonTypes() + R"(
     %100 = OpFunction %void None %voidfn

     %10 = OpLabel
     OpStore %var %uint_0
     OpSelectionMerge %99 None
     OpBranchConditional %cond %99 %99

     %20 = OpLabel ; dead
     OpStore %var %uint_1
     OpBranch %99

     %99 = OpLabel
     OpStore %var %uint_5
     OpReturn

     OpFunctionEnd
  )"));
    ASSERT_TRUE(p->BuildAndParseInternalModuleExceptFunctions()) << p->error();
    auto fe = p->function_emitter(100);
    EXPECT_TRUE(fe.EmitBody()) << p->error();
    auto ast_body = fe.ast_body();
    auto got = test::ToString(p->program(), ast_body);
    auto* expect = R"(var_1 = 0u;
if (false) {
}
var_1 = 5u;
return;
)";
    ASSERT_EQ(expect, got);
}

TEST_F(SpvParserCFGTest, EmitBody_BranchConditional_IfBreak_IfBreak_DifferentIsError) {
    auto p = parser(test::Assemble(CommonTypes() + R"(
     %100 = OpFunction %void None %voidfn

     %10 = OpLabel
     OpStore %var %uint_0
     OpSelectionMerge %99 None
     OpBranchConditional %cond %20 %99

     %20 = OpLabel
     OpStore %var %uint_1
     OpSelectionMerge %89 None
     OpBranchConditional %cond %30 %89

     %30 = OpLabel
     OpStore %var %uint_2
     OpBranchConditional %cond %89 %99 ; invalid divergence

     %89 = OpLabel ; inner if-merge
     OpBranch %99

     %99 = OpLabel ; outer if-merge
     OpStore %var %uint_5
     OpReturn

     OpFunctionEnd
  )"));
    ASSERT_TRUE(p->BuildAndParseInternalModuleExceptFunctions()) << p->error();
    auto fe = p->function_emitter(100);
    EXPECT_FALSE(FlowClassifyCFGEdges(&fe));
    EXPECT_THAT(p->error(), Eq("Branch from block 30 to block 99 is an invalid exit from construct "
                               "starting at block 20; branch bypasses merge block 89"));
}

TEST_F(SpvParserCFGTest, EmitBody_BranchConditional_Forward_Forward_Same) {
    auto p = parser(test::Assemble(CommonTypes() + R"(
     %100 = OpFunction %void None %voidfn

     %10 = OpLabel
     OpStore %var %uint_1
     OpBranchConditional %cond %99 %99; forward

     %99 = OpLabel
     OpStore %var %uint_2
     OpReturn

     OpFunctionEnd
  )"));
    ASSERT_TRUE(p->BuildAndParseInternalModuleExceptFunctions()) << p->error();
    auto fe = p->function_emitter(100);
    EXPECT_TRUE(fe.EmitBody()) << p->error();
    auto ast_body = fe.ast_body();
    auto got = test::ToString(p->program(), ast_body);
    auto* expect = R"(var_1 = 1u;
var_1 = 2u;
return;
)";
    ASSERT_EQ(expect, got);
}

TEST_F(SpvParserCFGTest, EmitBody_BranchConditional_Forward_Forward_Different_IsError) {
    auto p = parser(test::Assemble(CommonTypes() + R"(
     %100 = OpFunction %void None %voidfn

     %10 = OpLabel
     OpBranchConditional %cond %20 %99

     %20 = OpLabel
     OpReturn

     %99 = OpLabel
     OpStore %var %uint_2
     OpReturn

     OpFunctionEnd
  )"));
    ASSERT_TRUE(p->BuildAndParseInternalModuleExceptFunctions()) << p->error();
    auto fe = p->function_emitter(100);
    EXPECT_FALSE(fe.EmitBody());
    EXPECT_THAT(p->error(), Eq("Control flow diverges at block 10 (to 20, 99) but it is not "
                               "a structured header (it has no merge instruction)"));
}

TEST_F(SpvParserCFGTest, Switch_NotAsSelectionHeader_Simple) {
    auto assembly = CommonTypes() + R"(
     %100 = OpFunction %void None %voidfn

     %10 = OpLabel
     OpSwitch %uint_0 %99

     %99 = OpLabel
     OpReturn

     OpFunctionEnd
)";
    auto p = parser(test::Assemble(assembly));
    ASSERT_TRUE(p->BuildAndParseInternalModuleExceptFunctions()) << p->error();
    auto fe = p->function_emitter(100);
    EXPECT_FALSE(fe.EmitBody());
    EXPECT_THAT(p->error(), HasSubstr("invalid structured control flow: found an OpSwitch that "
                                      "is not preceded by an OpSelectionMerge:"));
}

TEST_F(SpvParserCFGTest, Switch_NotAsSelectionHeader_NonDefaultBranchesAreContinue) {
    // Adapted from SPIRV-Tools test MissingMergeOneUnseenTargetSwitchBad
    auto p = parser(test::Assemble(CommonTypes() + R"(
 %100 = OpFunction %void None %voidfn
 %entry = OpLabel
 OpBranch %loop
 %loop = OpLabel
 OpLoopMerge %merge %cont None
 OpBranchConditional %cond %merge %b1

 ; Here an OpSwitch is used with only one "unseen-so-far" target
 ; so it doesn't need an OpSelectionMerge.
 ; The %cont target can be implemented via "continue". So we can
 ; generate:
 ;    if ((selector != 1) && (selector != 3)) { continue; }
 %b1 = OpLabel
 OpSwitch %selector %b2 0 %b2 1 %cont 2 %b2 3 %cont

 %b2 = OpLabel ; the one unseen target
 OpBranch %cont
 %cont = OpLabel
 OpBranchConditional %cond2 %merge %loop
 %merge = OpLabel
 OpReturn
 OpFunctionEnd
   )"));
    ASSERT_TRUE(p->BuildAndParseInternalModuleExceptFunctions()) << p->error();
    auto fe = p->function_emitter(100);
    EXPECT_FALSE(fe.EmitBody());
    EXPECT_THAT(p->error(), HasSubstr("invalid structured control flow: found an OpSwitch that "
                                      "is not preceded by an OpSelectionMerge:"));
}

TEST_F(SpvParserCFGTest, Switch_NotAsSelectionHeader_DefaultBranchIsContinue) {
    // Adapted from SPIRV-Tools test MissingMergeOneUnseenTargetSwitchBad
    auto p = parser(test::Assemble(CommonTypes() + R"(
 %100 = OpFunction %void None %voidfn
 %entry = OpLabel
 OpBranch %loop
 %loop = OpLabel
 OpLoopMerge %merge %cont None
 OpBranchConditional %cond %merge %b1

 ; Here an OpSwitch is used with only one "unseen-so-far" target
 ; so it doesn't need an OpSelectionMerge.
 ; The %cont target can be implemented via "continue". So we can
 ; generate:
 ;    if (!(selector == 0 || selector == 2)) {continue;}
 %b1 = OpLabel
 OpSwitch %selector %cont 0 %b2 1 %cont 2 %b2

 %b2 = OpLabel ; the one unseen target
 OpBranch %cont
 %cont = OpLabel
 OpBranchConditional %cond2 %merge %loop
 %merge = OpLabel
 OpReturn
 OpFunctionEnd
   )"));
    ASSERT_TRUE(p->BuildAndParseInternalModuleExceptFunctions()) << p->error();
    auto fe = p->function_emitter(100);
    EXPECT_FALSE(fe.EmitBody());
    EXPECT_THAT(p->error(), HasSubstr("invalid structured control flow: found an OpSwitch that "
                                      "is not preceded by an OpSelectionMerge:"));
}

TEST_F(SpvParserCFGTest, SiblingLoopConstruct_Null) {
    auto assembly = CommonTypes() + R"(
     %100 = OpFunction %void None %voidfn
     %10 = OpLabel
     OpReturn
     OpFunctionEnd
)";
    auto p = parser(test::Assemble(assembly));
    ASSERT_TRUE(p->BuildAndParseInternalModuleExceptFunctions()) << p->error();
    auto fe = p->function_emitter(100);
    EXPECT_EQ(fe.SiblingLoopConstruct(nullptr), nullptr);
}

TEST_F(SpvParserCFGTest, SiblingLoopConstruct_NotAContinue) {
    auto assembly = CommonTypes() + R"(
     %100 = OpFunction %void None %voidfn

     %10 = OpLabel
     OpReturn

     OpFunctionEnd
)";
    auto p = parser(test::Assemble(assembly));
    ASSERT_TRUE(p->BuildAndParseInternalModuleExceptFunctions()) << p->error();
    auto fe = p->function_emitter(100);
    ASSERT_TRUE(FlowLabelControlFlowConstructs(&fe)) << p->error();
    const Construct* c = fe.GetBlockInfo(10)->construct;
    EXPECT_NE(c, nullptr);
    EXPECT_EQ(fe.SiblingLoopConstruct(c), nullptr);
}

TEST_F(SpvParserCFGTest, SiblingLoopConstruct_SingleBlockLoop) {
    auto assembly = CommonTypes() + R"(
     %100 = OpFunction %void None %voidfn

     %10 = OpLabel
     OpBranch %20

     %20 = OpLabel
     OpLoopMerge %99 %20 None
     OpBranchConditional %cond %20 %99

     %99 = OpLabel
     OpReturn

     OpFunctionEnd
)";
    auto p = parser(test::Assemble(assembly));
    ASSERT_TRUE(p->BuildAndParseInternalModuleExceptFunctions()) << p->error();
    auto fe = p->function_emitter(100);
    ASSERT_TRUE(FlowLabelControlFlowConstructs(&fe)) << p->error();
    const Construct* c = fe.GetBlockInfo(20)->construct;
    EXPECT_EQ(c->kind, Construct::kContinue);
    EXPECT_EQ(fe.SiblingLoopConstruct(c), nullptr);
}

TEST_F(SpvParserCFGTest, SiblingLoopConstruct_ContinueIsWholeMultiBlockLoop) {
    auto assembly = CommonTypes() + R"(
     %100 = OpFunction %void None %voidfn

     %10 = OpLabel
     OpBranch %20

     %20 = OpLabel
     OpLoopMerge %99 %20 None ; continue target is also loop header
     OpBranch %30

     %30 = OpLabel
     OpBranchConditional %cond %20 %99

     %99 = OpLabel
     OpReturn

     OpFunctionEnd
)";
    auto p = parser(test::Assemble(assembly));
    ASSERT_TRUE(p->BuildAndParseInternalModuleExceptFunctions()) << p->error() << assembly;
    auto fe = p->function_emitter(100);
    ASSERT_TRUE(FlowLabelControlFlowConstructs(&fe)) << p->error();
    const Construct* c = fe.GetBlockInfo(20)->construct;
    EXPECT_EQ(c->kind, Construct::kContinue);
    EXPECT_EQ(fe.SiblingLoopConstruct(c), nullptr);

    // SPIR-V 1.6 Rev 2 made this invalid SPIR-V.
    // Continue target is not structurally post dominated by the backedge block.
    // Don't dump this as an end-to-end test.
    p->DeliberatelyInvalidSpirv();
}

TEST_F(SpvParserCFGTest, SiblingLoopConstruct_HasSiblingLoop) {
    auto assembly = CommonTypes() + R"(
     %100 = OpFunction %void None %voidfn

     %10 = OpLabel
     OpBranch %20

     %20 = OpLabel
     OpLoopMerge %99 %30 None
     OpBranchConditional %cond %30 %99

     %30 = OpLabel ; continue target
     OpBranch %20

     %99 = OpLabel
     OpReturn

     OpFunctionEnd
)";
    auto p = parser(test::Assemble(assembly));
    ASSERT_TRUE(p->BuildAndParseInternalModuleExceptFunctions()) << p->error();
    auto fe = p->function_emitter(100);
    ASSERT_TRUE(FlowLabelControlFlowConstructs(&fe)) << p->error();
    const Construct* c = fe.GetBlockInfo(30)->construct;
    EXPECT_EQ(c->kind, Construct::kContinue);
    EXPECT_THAT(ToString(fe.SiblingLoopConstruct(c)),
                Eq("Construct{ Loop [1,2) begin_id:20 end_id:30 depth:1 "
                   "parent:Function@10 scope:[1,3) in-l:Loop@20 }"));
}

TEST_F(SpvParserCFGTest, EmitBody_IfSelection_TrueBranch_LoopBreak) {
    // crbug.com/tint/243
    auto assembly = CommonTypes() + R"(
     %100 = OpFunction %void None %voidfn

     %5 = OpLabel
     OpBranch %10

     %10 = OpLabel
     OpLoopMerge %99 %90 None
     OpBranch %20

     %20 = OpLabel
     OpSelectionMerge %40 None
     OpBranchConditional %cond %99 %30 ; true branch breaking is ok

     %30 = OpLabel
     OpBranch %40

     %40 = OpLabel ; selection merge
     OpBranch %90

     %90 = OpLabel ; continue target
     OpBranch %10 ; backedge

     %99 = OpLabel ; loop merge
     OpReturn
     OpFunctionEnd
)";
    auto p = parser(test::Assemble(assembly));
    ASSERT_TRUE(p->BuildAndParseInternalModuleExceptFunctions()) << p->error();
    auto fe = p->function_emitter(100);
    EXPECT_TRUE(fe.EmitBody()) << p->error();

    auto ast_body = fe.ast_body();
    auto got = test::ToString(p->program(), ast_body);
    auto* expect = R"(loop {
  if (false) {
    break;
  }
}
return;
)";
    ASSERT_EQ(expect, got);
}

TEST_F(SpvParserCFGTest, EmitBody_TrueBranch_LoopContinue) {
    // crbug.com/tint/243
    auto assembly = CommonTypes() + R"(
     %100 = OpFunction %void None %voidfn

     %5 = OpLabel
     OpBranch %10

     %10 = OpLabel
     OpLoopMerge %99 %90 None
     OpBranch %20

     %20 = OpLabel
     OpSelectionMerge %40 None
     OpBranchConditional %cond %90 %30 ; true branch continue is ok

     %30 = OpLabel
     OpBranch %40

     %40 = OpLabel ; selection merge
     OpBranch %90

     %90 = OpLabel ; continue target
     OpBranch %10 ; backedge

     %99 = OpLabel ; loop merge
     OpReturn

     OpFunctionEnd
)";
    auto p = parser(test::Assemble(assembly));
    ASSERT_TRUE(p->BuildAndParseInternalModuleExceptFunctions()) << p->error();
    auto fe = p->function_emitter(100);
    EXPECT_TRUE(fe.EmitBody()) << p->error();
    auto ast_body = fe.ast_body();
    auto got = test::ToString(p->program(), ast_body);
    auto* expect = R"(loop {
  if (false) {
    continue;
  }
}
return;
)";
    ASSERT_EQ(expect, got);
}

TEST_F(SpvParserCFGTest, EmitBody_TrueBranch_SwitchBreak) {
    // crbug.com/tint/243
    auto assembly = CommonTypes() + R"(
     %100 = OpFunction %void None %voidfn

     %10 = OpLabel
     OpSelectionMerge %99 None
     OpSwitch %uint_20 %99 20 %20

     %20 = OpLabel
     OpSelectionMerge %40 None
     OpBranchConditional %cond %99 %30 ; true branch switch break is ok

     %30 = OpLabel
     OpBranch %40

     %40 = OpLabel ; if-selection merge
     OpBranch %99

     %99 = OpLabel ; switch merge
     OpReturn

     OpFunctionEnd
)";
    auto p = parser(test::Assemble(assembly));
    ASSERT_TRUE(p->BuildAndParseInternalModuleExceptFunctions()) << p->error();
    auto fe = p->function_emitter(100);
    EXPECT_TRUE(fe.EmitBody()) << p->error();
    auto ast_body = fe.ast_body();
    auto got = test::ToString(p->program(), ast_body);
    auto* expect = R"(switch(20u) {
  case 20u: {
    if (false) {
      break;
    }
  }
  default: {
  }
}
return;
)";
    ASSERT_EQ(expect, got);
}

TEST_F(SpvParserCFGTest, EmitBody_FalseBranch_LoopBreak) {
    // crbug.com/tint/243
    auto assembly = CommonTypes() + R"(
     %100 = OpFunction %void None %voidfn

     %5 = OpLabel
     OpBranch %10

     %10 = OpLabel
     OpLoopMerge %99 %90 None
     OpBranch %20

     %20 = OpLabel
     OpSelectionMerge %40 None
     OpBranchConditional %cond %30 %99 ; false branch breaking is ok

     %30 = OpLabel
     OpBranch %40

     %40 = OpLabel ; selection merge
     OpBranch %90

     %90 = OpLabel ; continue target
     OpBranch %10 ; backedge

     %99 = OpLabel ; loop merge
     OpReturn

     OpFunctionEnd
)";
    auto p = parser(test::Assemble(assembly));
    ASSERT_TRUE(p->BuildAndParseInternalModuleExceptFunctions()) << p->error();
    auto fe = p->function_emitter(100);
    EXPECT_TRUE(fe.EmitBody()) << p->error();
    auto ast_body = fe.ast_body();
    auto got = test::ToString(p->program(), ast_body);
    auto* expect = R"(loop {
  if (false) {
  } else {
    break;
  }
}
return;
)";
    ASSERT_EQ(expect, got);
}

TEST_F(SpvParserCFGTest, EmitBody_FalseBranch_LoopContinue) {
    // crbug.com/tint/243
    auto assembly = CommonTypes() + R"(
     %100 = OpFunction %void None %voidfn

     %5 = OpLabel
     OpBranch %10

     %10 = OpLabel
     OpLoopMerge %99 %90 None
     OpBranch %20

     %20 = OpLabel
     OpSelectionMerge %40 None
     OpBranchConditional %cond %30 %90 ; false branch continue is ok

     %30 = OpLabel
     OpBranch %40

     %40 = OpLabel ; selection merge
     OpBranch %90

     %90 = OpLabel ; continue target
     OpBranch %10 ; backedge

     %99 = OpLabel ; loop merge
     OpReturn

     OpFunctionEnd
)";
    auto p = parser(test::Assemble(assembly));
    ASSERT_TRUE(p->BuildAndParseInternalModuleExceptFunctions()) << p->error();
    auto fe = p->function_emitter(100);
    EXPECT_TRUE(fe.EmitBody()) << p->error();
    auto ast_body = fe.ast_body();
    auto got = test::ToString(p->program(), ast_body);
    auto* expect = R"(loop {
  if (false) {
  } else {
    continue;
  }
}
return;
)";
    ASSERT_EQ(expect, got) << p->error();
}

TEST_F(SpvParserCFGTest, EmitBody_FalseBranch_SwitchBreak) {
    // crbug.com/tint/243
    auto assembly = CommonTypes() + R"(
     %100 = OpFunction %void None %voidfn

     %10 = OpLabel
     OpSelectionMerge %99 None
     OpSwitch %uint_20 %99 20 %20

     %20 = OpLabel
     OpSelectionMerge %40 None
     OpBranchConditional %cond %30 %99 ; false branch switch break is ok

     %30 = OpLabel
     OpBranch %40

     %40 = OpLabel ; if-selection merge
     OpBranch %99

     %99 = OpLabel ; switch merge
     OpReturn

     OpFunctionEnd
)";
    auto p = parser(test::Assemble(assembly));
    ASSERT_TRUE(p->BuildAndParseInternalModuleExceptFunctions()) << p->error();
    auto fe = p->function_emitter(100);
    EXPECT_TRUE(fe.EmitBody()) << p->error();
    auto ast_body = fe.ast_body();
    auto got = test::ToString(p->program(), ast_body);
    auto* expect = R"(switch(20u) {
  case 20u: {
    if (false) {
    } else {
      break;
    }
  }
  default: {
  }
}
return;
)";
    ASSERT_EQ(expect, got);
}

TEST_F(SpvParserCFGTest, EmitBody_LoopInternallyDiverge_Simple) {
    // crbug.com/tint/524
    auto assembly = CommonTypes() + R"(
     %100 = OpFunction %void None %voidfn
     %10 = OpLabel
     OpStore %var %uint_10
     OpBranch %20

     %20 = OpLabel
     OpStore %var %uint_20
     OpLoopMerge %99 %90 None
     OpBranchConditional %cond %30 %40 ; divergence

       %30 = OpLabel
       OpStore %var %uint_30
       OpBranch %90

       %40 = OpLabel
       OpStore %var %uint_40
       OpBranch %90

     %90 = OpLabel ; continue target
     OpStore %var %uint_90
     OpBranch %20

     %99 = OpLabel ; loop merge
     OpStore %var %uint_99
     OpReturn

     OpFunctionEnd
)";
    auto p = parser(test::Assemble(assembly));
    ASSERT_TRUE(p->BuildAndParseInternalModuleExceptFunctions()) << p->error();
    auto fe = p->function_emitter(100);
    EXPECT_TRUE(fe.EmitBody()) << p->error();
    auto ast_body = fe.ast_body();
    auto got = test::ToString(p->program(), ast_body);
    auto* expect = R"(var_1 = 10u;
loop {
  var_1 = 20u;
  if (false) {
    var_1 = 30u;
    continue;
  } else {
    var_1 = 40u;
  }

  continuing {
    var_1 = 90u;
  }
}
var_1 = 99u;
return;
)";
    ASSERT_EQ(expect, got) << got;
}

TEST_F(SpvParserCFGTest, EmitBody_ContinueFromSingleBlockLoopToOuterLoop_IsError) {
    // crbug.com/tint/793
    // This is invalid SPIR-V but the validator was only recently upgraded
    // to catch it.
    auto assembly = CommonTypes() + R"(
  %100 = OpFunction %void None %voidfn
  %5 = OpLabel
  OpBranch %10

  %10 = OpLabel ; outer loop header
  OpLoopMerge %99 %89 None
  OpBranchConditional %cond %99 %20

  %20 = OpLabel ; inner loop single block loop
  OpLoopMerge %79 %20 None

  ; true -> continue to outer loop
  ; false -> self-loop
  ; The true branch is invalid because a "continue block", i.e. the block
  ; containing the branch to the continue target, "is valid only for the
  ; innermost loop it is nested inside of".
  ; So it can't branch to the continue target of an outer loop.
  OpBranchConditional %cond %89 %20

  %79 = OpLabel ; merge for outer loop
  OpUnreachable

  %89 = OpLabel
  OpBranch %10 ; backedge for outer loop

  %99 = OpLabel ; merge for outer
  OpReturn

  OpFunctionEnd

)";
    auto p = parser(test::Assemble(assembly));
    EXPECT_FALSE(p->Parse());
    EXPECT_FALSE(p->success());
    EXPECT_THAT(p->error(), HasSubstr("block <ID> '20[%20]' exits the continue headed by <ID> "
                                      "'20[%20]', but not via a structured exit"))
        << p->error();
}

}  // namespace
}  // namespace tint::spirv::reader::ast_parser
