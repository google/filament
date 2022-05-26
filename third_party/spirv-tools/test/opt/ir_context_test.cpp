// Copyright (c) 2017 Google Inc.
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

#include "source/opt/ir_context.h"

#include <algorithm>
#include <memory>
#include <string>
#include <utility>

#include "OpenCLDebugInfo100.h"
#include "gmock/gmock.h"
#include "gtest/gtest.h"
#include "source/opt/pass.h"
#include "test/opt/pass_fixture.h"
#include "test/opt/pass_utils.h"

static const uint32_t kDebugDeclareOperandVariableIndex = 5;
static const uint32_t kDebugValueOperandValueIndex = 5;

namespace spvtools {
namespace opt {
namespace {

using Analysis = IRContext::Analysis;
using ::testing::Each;
using ::testing::UnorderedElementsAre;

class NoopPassPreservesNothing : public Pass {
 public:
  NoopPassPreservesNothing(Status s) : Pass(), status_to_return_(s) {}

  const char* name() const override { return "noop-pass"; }
  Status Process() override { return status_to_return_; }

 private:
  Status status_to_return_;
};

class NoopPassPreservesAll : public Pass {
 public:
  NoopPassPreservesAll(Status s) : Pass(), status_to_return_(s) {}

  const char* name() const override { return "noop-pass"; }
  Status Process() override { return status_to_return_; }

  Analysis GetPreservedAnalyses() override {
    return Analysis(IRContext::kAnalysisEnd - 1);
  }

 private:
  Status status_to_return_;
};

class NoopPassPreservesFirst : public Pass {
 public:
  NoopPassPreservesFirst(Status s) : Pass(), status_to_return_(s) {}

  const char* name() const override { return "noop-pass"; }
  Status Process() override { return status_to_return_; }

  Analysis GetPreservedAnalyses() override { return IRContext::kAnalysisBegin; }

 private:
  Status status_to_return_;
};

using IRContextTest = PassTest<::testing::Test>;

TEST_F(IRContextTest, IndividualValidAfterBuild) {
  std::unique_ptr<Module> module(new Module());
  IRContext localContext(SPV_ENV_UNIVERSAL_1_2, std::move(module),
                         spvtools::MessageConsumer());

  for (Analysis i = IRContext::kAnalysisBegin; i < IRContext::kAnalysisEnd;
       i <<= 1) {
    localContext.BuildInvalidAnalyses(i);
    EXPECT_TRUE(localContext.AreAnalysesValid(i));
  }
}

TEST_F(IRContextTest, DontRebuildValidAnalysis) {
  std::unique_ptr<Module> module(new Module());
  IRContext localContext(SPV_ENV_UNIVERSAL_1_2, std::move(module),
                         spvtools::MessageConsumer());

  auto* oldCfg = localContext.cfg();
  auto* oldDefUse = localContext.get_def_use_mgr();
  localContext.BuildInvalidAnalyses(IRContext::kAnalysisCFG |
                                    IRContext::kAnalysisDefUse);
  auto* newCfg = localContext.cfg();
  auto* newDefUse = localContext.get_def_use_mgr();
  EXPECT_EQ(oldCfg, newCfg);
  EXPECT_EQ(oldDefUse, newDefUse);
}

TEST_F(IRContextTest, AllValidAfterBuild) {
  std::unique_ptr<Module> module = MakeUnique<Module>();
  IRContext localContext(SPV_ENV_UNIVERSAL_1_2, std::move(module),
                         spvtools::MessageConsumer());

  Analysis built_analyses = IRContext::kAnalysisNone;
  for (Analysis i = IRContext::kAnalysisBegin; i < IRContext::kAnalysisEnd;
       i <<= 1) {
    localContext.BuildInvalidAnalyses(i);
    built_analyses |= i;
  }
  EXPECT_TRUE(localContext.AreAnalysesValid(built_analyses));
}

TEST_F(IRContextTest, AllValidAfterPassNoChange) {
  std::unique_ptr<Module> module = MakeUnique<Module>();
  IRContext localContext(SPV_ENV_UNIVERSAL_1_2, std::move(module),
                         spvtools::MessageConsumer());

  Analysis built_analyses = IRContext::kAnalysisNone;
  for (Analysis i = IRContext::kAnalysisBegin; i < IRContext::kAnalysisEnd;
       i <<= 1) {
    localContext.BuildInvalidAnalyses(i);
    built_analyses |= i;
  }

  NoopPassPreservesNothing pass(Pass::Status::SuccessWithoutChange);
  Pass::Status s = pass.Run(&localContext);
  EXPECT_EQ(s, Pass::Status::SuccessWithoutChange);
  EXPECT_TRUE(localContext.AreAnalysesValid(built_analyses));
}

TEST_F(IRContextTest, NoneValidAfterPassWithChange) {
  std::unique_ptr<Module> module = MakeUnique<Module>();
  IRContext localContext(SPV_ENV_UNIVERSAL_1_2, std::move(module),
                         spvtools::MessageConsumer());

  for (Analysis i = IRContext::kAnalysisBegin; i < IRContext::kAnalysisEnd;
       i <<= 1) {
    localContext.BuildInvalidAnalyses(i);
  }

  NoopPassPreservesNothing pass(Pass::Status::SuccessWithChange);
  Pass::Status s = pass.Run(&localContext);
  EXPECT_EQ(s, Pass::Status::SuccessWithChange);
  for (Analysis i = IRContext::kAnalysisBegin; i < IRContext::kAnalysisEnd;
       i <<= 1) {
    EXPECT_FALSE(localContext.AreAnalysesValid(i));
  }
}

TEST_F(IRContextTest, AllPreservedAfterPassWithChange) {
  std::unique_ptr<Module> module = MakeUnique<Module>();
  IRContext localContext(SPV_ENV_UNIVERSAL_1_2, std::move(module),
                         spvtools::MessageConsumer());

  for (Analysis i = IRContext::kAnalysisBegin; i < IRContext::kAnalysisEnd;
       i <<= 1) {
    localContext.BuildInvalidAnalyses(i);
  }

  NoopPassPreservesAll pass(Pass::Status::SuccessWithChange);
  Pass::Status s = pass.Run(&localContext);
  EXPECT_EQ(s, Pass::Status::SuccessWithChange);
  for (Analysis i = IRContext::kAnalysisBegin; i < IRContext::kAnalysisEnd;
       i <<= 1) {
    EXPECT_TRUE(localContext.AreAnalysesValid(i));
  }
}

TEST_F(IRContextTest, PreserveFirstOnlyAfterPassWithChange) {
  std::unique_ptr<Module> module = MakeUnique<Module>();
  IRContext localContext(SPV_ENV_UNIVERSAL_1_2, std::move(module),
                         spvtools::MessageConsumer());

  for (Analysis i = IRContext::kAnalysisBegin; i < IRContext::kAnalysisEnd;
       i <<= 1) {
    localContext.BuildInvalidAnalyses(i);
  }

  NoopPassPreservesFirst pass(Pass::Status::SuccessWithChange);
  Pass::Status s = pass.Run(&localContext);
  EXPECT_EQ(s, Pass::Status::SuccessWithChange);
  EXPECT_TRUE(localContext.AreAnalysesValid(IRContext::kAnalysisBegin));
  for (Analysis i = IRContext::kAnalysisBegin << 1; i < IRContext::kAnalysisEnd;
       i <<= 1) {
    EXPECT_FALSE(localContext.AreAnalysesValid(i));
  }
}

TEST_F(IRContextTest, KillMemberName) {
  const std::string text = R"(
              OpCapability Shader
          %1 = OpExtInstImport "GLSL.std.450"
               OpMemoryModel Logical GLSL450
               OpEntryPoint Fragment %2 "main"
               OpExecutionMode %2 OriginUpperLeft
               OpSource GLSL 430
               OpName %3 "stuff"
               OpMemberName %3 0 "refZ"
               OpMemberDecorate %3 0 Offset 0
               OpDecorate %3 Block
          %4 = OpTypeFloat 32
          %3 = OpTypeStruct %4
          %5 = OpTypeVoid
          %6 = OpTypeFunction %5
          %2 = OpFunction %5 None %6
          %7 = OpLabel
               OpReturn
               OpFunctionEnd
)";

  std::unique_ptr<IRContext> context =
      BuildModule(SPV_ENV_UNIVERSAL_1_2, nullptr, text);

  // Build the decoration manager.
  context->get_decoration_mgr();

  // Delete the OpTypeStruct.  Should delete the OpName, OpMemberName, and
  // OpMemberDecorate associated with it.
  context->KillDef(3);

  // Make sure all of the name are removed.
  for (auto& inst : context->debugs2()) {
    EXPECT_EQ(inst.opcode(), SpvOpNop);
  }

  // Make sure all of the decorations are removed.
  for (auto& inst : context->annotations()) {
    EXPECT_EQ(inst.opcode(), SpvOpNop);
  }
}

TEST_F(IRContextTest, KillGroupDecoration) {
  const std::string text = R"(
               OpCapability Shader
          %1 = OpExtInstImport "GLSL.std.450"
               OpMemoryModel Logical GLSL450
               OpEntryPoint Fragment %2 "main"
               OpExecutionMode %2 OriginUpperLeft
               OpSource GLSL 430
               OpDecorate %3 Restrict
          %3 = OpDecorationGroup
               OpGroupDecorate %3 %4 %5
          %6 = OpTypeFloat 32
          %7 = OpTypePointer Function %6
          %8 = OpTypeStruct %6
          %9 = OpTypeVoid
         %10 = OpTypeFunction %9
          %2 = OpFunction %9 None %10
         %11 = OpLabel
          %4 = OpVariable %7 Function
          %5 = OpVariable %7 Function
               OpReturn
               OpFunctionEnd
)";

  std::unique_ptr<IRContext> context =
      BuildModule(SPV_ENV_UNIVERSAL_1_2, nullptr, text);

  // Build the decoration manager.
  context->get_decoration_mgr();

  // Delete the second variable.
  context->KillDef(5);

  // The three decorations instructions should still be there.  The first two
  // should be the same, but the third should have %5 removed.

  // Check the OpDecorate instruction
  auto inst = context->annotation_begin();
  EXPECT_EQ(inst->opcode(), SpvOpDecorate);
  EXPECT_EQ(inst->GetSingleWordInOperand(0), 3);

  // Check the OpDecorationGroup Instruction
  ++inst;
  EXPECT_EQ(inst->opcode(), SpvOpDecorationGroup);
  EXPECT_EQ(inst->result_id(), 3);

  // Check that %5 is no longer part of the group.
  ++inst;
  EXPECT_EQ(inst->opcode(), SpvOpGroupDecorate);
  EXPECT_EQ(inst->NumInOperands(), 2);
  EXPECT_EQ(inst->GetSingleWordInOperand(0), 3);
  EXPECT_EQ(inst->GetSingleWordInOperand(1), 4);

  // Check that we are at the end.
  ++inst;
  EXPECT_EQ(inst, context->annotation_end());
}

TEST_F(IRContextTest, TakeNextUniqueIdIncrementing) {
  const uint32_t NUM_TESTS = 1000;
  IRContext localContext(SPV_ENV_UNIVERSAL_1_2, nullptr);
  for (uint32_t i = 1; i < NUM_TESTS; ++i)
    EXPECT_EQ(i, localContext.TakeNextUniqueId());
}

TEST_F(IRContextTest, KillGroupDecorationWitNoDecorations) {
  const std::string text = R"(
               OpCapability Shader
          %1 = OpExtInstImport "GLSL.std.450"
               OpMemoryModel Logical GLSL450
               OpEntryPoint Fragment %2 "main"
               OpExecutionMode %2 OriginUpperLeft
               OpSource GLSL 430
          %3 = OpDecorationGroup
               OpGroupDecorate %3 %4 %5
          %6 = OpTypeFloat 32
          %7 = OpTypePointer Function %6
          %8 = OpTypeStruct %6
          %9 = OpTypeVoid
         %10 = OpTypeFunction %9
          %2 = OpFunction %9 None %10
         %11 = OpLabel
          %4 = OpVariable %7 Function
          %5 = OpVariable %7 Function
               OpReturn
               OpFunctionEnd
)";

  std::unique_ptr<IRContext> context =
      BuildModule(SPV_ENV_UNIVERSAL_1_2, nullptr, text);

  // Build the decoration manager.
  context->get_decoration_mgr();

  // Delete the second variable.
  context->KillDef(5);

  // The two decoration instructions should still be there.  The first one
  // should be the same, but the second should have %5 removed.

  // Check the OpDecorationGroup Instruction
  auto inst = context->annotation_begin();
  EXPECT_EQ(inst->opcode(), SpvOpDecorationGroup);
  EXPECT_EQ(inst->result_id(), 3);

  // Check that %5 is no longer part of the group.
  ++inst;
  EXPECT_EQ(inst->opcode(), SpvOpGroupDecorate);
  EXPECT_EQ(inst->NumInOperands(), 2);
  EXPECT_EQ(inst->GetSingleWordInOperand(0), 3);
  EXPECT_EQ(inst->GetSingleWordInOperand(1), 4);

  // Check that we are at the end.
  ++inst;
  EXPECT_EQ(inst, context->annotation_end());
}

TEST_F(IRContextTest, KillDecorationGroup) {
  const std::string text = R"(
               OpCapability Shader
          %1 = OpExtInstImport "GLSL.std.450"
               OpMemoryModel Logical GLSL450
               OpEntryPoint Fragment %2 "main"
               OpExecutionMode %2 OriginUpperLeft
               OpSource GLSL 430
          %3 = OpDecorationGroup
               OpGroupDecorate %3 %4 %5
          %6 = OpTypeFloat 32
          %7 = OpTypePointer Function %6
          %8 = OpTypeStruct %6
          %9 = OpTypeVoid
         %10 = OpTypeFunction %9
          %2 = OpFunction %9 None %10
         %11 = OpLabel
          %4 = OpVariable %7 Function
          %5 = OpVariable %7 Function
               OpReturn
               OpFunctionEnd
)";

  std::unique_ptr<IRContext> context =
      BuildModule(SPV_ENV_UNIVERSAL_1_2, nullptr, text);

  // Build the decoration manager.
  context->get_decoration_mgr();

  // Delete the second variable.
  context->KillDef(3);

  // Check the OpDecorationGroup Instruction is still there.
  EXPECT_TRUE(context->annotations().empty());
}

TEST_F(IRContextTest, KillFunctionFromDebugFunction) {
  const std::string text = R"(
               OpCapability Shader
          %1 = OpExtInstImport "OpenCL.DebugInfo.100"
               OpMemoryModel Logical GLSL450
               OpEntryPoint Fragment %2 "main"
               OpExecutionMode %2 OriginUpperLeft
          %3 = OpString "ps.hlsl"
          %4 = OpString "foo"
               OpSource HLSL 600
       %void = OpTypeVoid
          %6 = OpTypeFunction %void
          %7 = OpExtInst %void %1 DebugSource %3
          %8 = OpExtInst %void %1 DebugCompilationUnit 1 4 %7 HLSL
          %9 = OpExtInst %void %1 DebugTypeFunction FlagIsProtected|FlagIsPrivate %void
         %10 = OpExtInst %void %1 DebugFunction %4 %9 %7 1 1 %8 %4 FlagIsProtected|FlagIsPrivate 1 %11
          %2 = OpFunction %void None %6
         %12 = OpLabel
               OpReturn
               OpFunctionEnd
         %11 = OpFunction %void None %6
         %13 = OpLabel
               OpReturn
               OpFunctionEnd
)";

  std::unique_ptr<IRContext> context =
      BuildModule(SPV_ENV_UNIVERSAL_1_2, nullptr, text);

  // Delete the second variable.
  context->KillDef(11);

  // Get DebugInfoNone id.
  uint32_t debug_info_none_id = 0;
  for (auto it = context->ext_inst_debuginfo_begin();
       it != context->ext_inst_debuginfo_end(); ++it) {
    if (it->GetOpenCL100DebugOpcode() == OpenCLDebugInfo100DebugInfoNone) {
      debug_info_none_id = it->result_id();
    }
  }
  EXPECT_NE(0, debug_info_none_id);

  // Check the Function operand of DebugFunction is DebugInfoNone.
  const uint32_t kDebugFunctionOperandFunctionIndex = 13;
  bool checked = false;
  for (auto it = context->ext_inst_debuginfo_begin();
       it != context->ext_inst_debuginfo_end(); ++it) {
    if (it->GetOpenCL100DebugOpcode() == OpenCLDebugInfo100DebugFunction) {
      EXPECT_FALSE(checked);
      EXPECT_EQ(it->GetOperand(kDebugFunctionOperandFunctionIndex).words[0],
                debug_info_none_id);
      checked = true;
    }
  }
  EXPECT_TRUE(checked);
}

TEST_F(IRContextTest, KillVariableFromDebugGlobalVariable) {
  const std::string text = R"(
               OpCapability Shader
          %1 = OpExtInstImport "OpenCL.DebugInfo.100"
               OpMemoryModel Logical GLSL450
               OpEntryPoint Fragment %2 "main"
               OpExecutionMode %2 OriginUpperLeft
          %3 = OpString "ps.hlsl"
          %4 = OpString "foo"
          %5 = OpString "int"
               OpSource HLSL 600
       %uint = OpTypeInt 32 0
    %uint_32 = OpConstant %uint 32
%_ptr_Private_uint = OpTypePointer Private %uint
       %void = OpTypeVoid
         %10 = OpTypeFunction %void
         %11 = OpVariable %_ptr_Private_uint Private
         %12 = OpExtInst %void %1 DebugSource %3
         %13 = OpExtInst %void %1 DebugCompilationUnit 1 4 %12 HLSL
         %14 = OpExtInst %void %1 DebugTypeBasic %5 %uint_32 Signed
         %15 = OpExtInst %void %1 DebugGlobalVariable %4 %14 %12 1 12 %13 %4 %11 FlagIsDefinition
          %2 = OpFunction %void None %10
         %16 = OpLabel
               OpReturn
               OpFunctionEnd
)";

  std::unique_ptr<IRContext> context =
      BuildModule(SPV_ENV_UNIVERSAL_1_2, nullptr, text);

  // Delete the second variable.
  context->KillDef(11);

  // Get DebugInfoNone id.
  uint32_t debug_info_none_id = 0;
  for (auto it = context->ext_inst_debuginfo_begin();
       it != context->ext_inst_debuginfo_end(); ++it) {
    if (it->GetOpenCL100DebugOpcode() == OpenCLDebugInfo100DebugInfoNone) {
      debug_info_none_id = it->result_id();
    }
  }
  EXPECT_NE(0, debug_info_none_id);

  // Check the Function operand of DebugFunction is DebugInfoNone.
  const uint32_t kDebugGlobalVariableOperandVariableIndex = 11;
  bool checked = false;
  for (auto it = context->ext_inst_debuginfo_begin();
       it != context->ext_inst_debuginfo_end(); ++it) {
    if (it->GetOpenCL100DebugOpcode() ==
        OpenCLDebugInfo100DebugGlobalVariable) {
      EXPECT_FALSE(checked);
      EXPECT_EQ(
          it->GetOperand(kDebugGlobalVariableOperandVariableIndex).words[0],
          debug_info_none_id);
      checked = true;
    }
  }
  EXPECT_TRUE(checked);
}

TEST_F(IRContextTest, BasicVisitFromEntryPoint) {
  // Make sure we visit the entry point, and the function it calls.
  // Do not visit Dead or Exported.
  const std::string text = R"(
               OpCapability Shader
               OpMemoryModel Logical GLSL450
               OpEntryPoint Fragment %10 "main"
               OpName %10 "main"
               OpName %Dead "Dead"
               OpName %11 "Constant"
               OpName %ExportedFunc "ExportedFunc"
               OpDecorate %ExportedFunc LinkageAttributes "ExportedFunc" Export
       %void = OpTypeVoid
          %6 = OpTypeFunction %void
         %10 = OpFunction %void None %6
         %14 = OpLabel
         %15 = OpFunctionCall %void %11
         %16 = OpFunctionCall %void %11
               OpReturn
               OpFunctionEnd
         %11 = OpFunction %void None %6
         %18 = OpLabel
               OpReturn
               OpFunctionEnd
       %Dead = OpFunction %void None %6
         %19 = OpLabel
               OpReturn
               OpFunctionEnd
%ExportedFunc = OpFunction %void None %7
         %20 = OpLabel
         %21 = OpFunctionCall %void %11
               OpReturn
               OpFunctionEnd
)";
  // clang-format on

  std::unique_ptr<IRContext> localContext =
      BuildModule(SPV_ENV_UNIVERSAL_1_1, nullptr, text,
                  SPV_TEXT_TO_BINARY_OPTION_PRESERVE_NUMERIC_IDS);
  EXPECT_NE(nullptr, localContext) << "Assembling failed for shader:\n"
                                   << text << std::endl;
  std::vector<uint32_t> processed;
  Pass::ProcessFunction mark_visited = [&processed](Function* fp) {
    processed.push_back(fp->result_id());
    return false;
  };
  localContext->ProcessEntryPointCallTree(mark_visited);
  EXPECT_THAT(processed, UnorderedElementsAre(10, 11));
}

TEST_F(IRContextTest, BasicVisitReachable) {
  // Make sure we visit the entry point, exported function, and the function
  // they call. Do not visit Dead.
  const std::string text = R"(
               OpCapability Shader
               OpMemoryModel Logical GLSL450
               OpEntryPoint Fragment %10 "main"
               OpName %10 "main"
               OpName %Dead "Dead"
               OpName %11 "Constant"
               OpName %12 "ExportedFunc"
               OpName %13 "Constant2"
               OpDecorate %12 LinkageAttributes "ExportedFunc" Export
       %void = OpTypeVoid
          %6 = OpTypeFunction %void
         %10 = OpFunction %void None %6
         %14 = OpLabel
         %15 = OpFunctionCall %void %11
         %16 = OpFunctionCall %void %11
               OpReturn
               OpFunctionEnd
         %11 = OpFunction %void None %6
         %18 = OpLabel
               OpReturn
               OpFunctionEnd
       %Dead = OpFunction %void None %6
         %19 = OpLabel
               OpReturn
               OpFunctionEnd
         %12 = OpFunction %void None %6
         %20 = OpLabel
         %21 = OpFunctionCall %void %13
               OpReturn
               OpFunctionEnd
         %13 = OpFunction %void None %6
         %22 = OpLabel
               OpReturn
               OpFunctionEnd
)";
  // clang-format on

  std::unique_ptr<IRContext> localContext =
      BuildModule(SPV_ENV_UNIVERSAL_1_1, nullptr, text,
                  SPV_TEXT_TO_BINARY_OPTION_PRESERVE_NUMERIC_IDS);
  EXPECT_NE(nullptr, localContext) << "Assembling failed for shader:\n"
                                   << text << std::endl;

  std::vector<uint32_t> processed;
  Pass::ProcessFunction mark_visited = [&processed](Function* fp) {
    processed.push_back(fp->result_id());
    return false;
  };
  localContext->ProcessReachableCallTree(mark_visited);
  EXPECT_THAT(processed, UnorderedElementsAre(10, 11, 12, 13));
}

TEST_F(IRContextTest, BasicVisitOnlyOnce) {
  // Make sure we visit %12 only once, even if it is called from two different
  // functions.
  const std::string text = R"(
               OpCapability Shader
               OpMemoryModel Logical GLSL450
               OpEntryPoint Fragment %10 "main"
               OpName %10 "main"
               OpName %Dead "Dead"
               OpName %11 "Constant"
               OpName %12 "ExportedFunc"
               OpDecorate %12 LinkageAttributes "ExportedFunc" Export
       %void = OpTypeVoid
          %6 = OpTypeFunction %void
         %10 = OpFunction %void None %6
         %14 = OpLabel
         %15 = OpFunctionCall %void %11
         %16 = OpFunctionCall %void %12
               OpReturn
               OpFunctionEnd
         %11 = OpFunction %void None %6
         %18 = OpLabel
         %19 = OpFunctionCall %void %12
               OpReturn
               OpFunctionEnd
       %Dead = OpFunction %void None %6
         %20 = OpLabel
               OpReturn
               OpFunctionEnd
         %12 = OpFunction %void None %6
         %21 = OpLabel
               OpReturn
               OpFunctionEnd
)";
  // clang-format on

  std::unique_ptr<IRContext> localContext =
      BuildModule(SPV_ENV_UNIVERSAL_1_1, nullptr, text,
                  SPV_TEXT_TO_BINARY_OPTION_PRESERVE_NUMERIC_IDS);
  EXPECT_NE(nullptr, localContext) << "Assembling failed for shader:\n"
                                   << text << std::endl;

  std::vector<uint32_t> processed;
  Pass::ProcessFunction mark_visited = [&processed](Function* fp) {
    processed.push_back(fp->result_id());
    return false;
  };
  localContext->ProcessReachableCallTree(mark_visited);
  EXPECT_THAT(processed, UnorderedElementsAre(10, 11, 12));
}

TEST_F(IRContextTest, BasicDontVisitExportedVariable) {
  // Make sure we only visit functions and not exported variables.
  const std::string text = R"(
               OpCapability Shader
               OpMemoryModel Logical GLSL450
               OpEntryPoint Fragment %10 "main"
               OpExecutionMode %10 OriginUpperLeft
               OpSource GLSL 150
               OpName %10 "main"
               OpName %12 "export_var"
               OpDecorate %12 LinkageAttributes "export_var" Export
       %void = OpTypeVoid
          %6 = OpTypeFunction %void
      %float = OpTypeFloat 32
  %float_1 = OpConstant %float 1
         %12 = OpVariable %float Output
         %10 = OpFunction %void None %6
         %14 = OpLabel
               OpStore %12 %float_1
               OpReturn
               OpFunctionEnd
)";
  // clang-format on

  std::unique_ptr<IRContext> localContext =
      BuildModule(SPV_ENV_UNIVERSAL_1_1, nullptr, text,
                  SPV_TEXT_TO_BINARY_OPTION_PRESERVE_NUMERIC_IDS);
  EXPECT_NE(nullptr, localContext) << "Assembling failed for shader:\n"
                                   << text << std::endl;

  std::vector<uint32_t> processed;
  Pass::ProcessFunction mark_visited = [&processed](Function* fp) {
    processed.push_back(fp->result_id());
    return false;
  };
  localContext->ProcessReachableCallTree(mark_visited);
  EXPECT_THAT(processed, UnorderedElementsAre(10));
}

TEST_F(IRContextTest, IdBoundTestAtLimit) {
  const std::string text = R"(
OpCapability Shader
OpCapability Linkage
OpMemoryModel Logical GLSL450
%1 = OpTypeVoid
%2 = OpTypeFunction %1
%3 = OpFunction %1 None %2
%4 = OpLabel
OpReturn
OpFunctionEnd)";

  std::unique_ptr<IRContext> context =
      BuildModule(SPV_ENV_UNIVERSAL_1_1, nullptr, text,
                  SPV_TEXT_TO_BINARY_OPTION_PRESERVE_NUMERIC_IDS);
  uint32_t current_bound = context->module()->id_bound();
  context->set_max_id_bound(current_bound);
  uint32_t next_id_bound = context->TakeNextId();
  EXPECT_EQ(next_id_bound, 0);
  EXPECT_EQ(current_bound, context->module()->id_bound());
  next_id_bound = context->TakeNextId();
  EXPECT_EQ(next_id_bound, 0);
}

TEST_F(IRContextTest, IdBoundTestBelowLimit) {
  const std::string text = R"(
OpCapability Shader
OpCapability Linkage
OpMemoryModel Logical GLSL450
%1 = OpTypeVoid
%2 = OpTypeFunction %1
%3 = OpFunction %1 None %2
%4 = OpLabel
OpReturn
OpFunctionEnd)";

  std::unique_ptr<IRContext> context =
      BuildModule(SPV_ENV_UNIVERSAL_1_1, nullptr, text,
                  SPV_TEXT_TO_BINARY_OPTION_PRESERVE_NUMERIC_IDS);
  uint32_t current_bound = context->module()->id_bound();
  context->set_max_id_bound(current_bound + 100);
  uint32_t next_id_bound = context->TakeNextId();
  EXPECT_EQ(next_id_bound, current_bound);
  EXPECT_EQ(current_bound + 1, context->module()->id_bound());
  next_id_bound = context->TakeNextId();
  EXPECT_EQ(next_id_bound, current_bound + 1);
}

TEST_F(IRContextTest, IdBoundTestNearLimit) {
  const std::string text = R"(
OpCapability Shader
OpCapability Linkage
OpMemoryModel Logical GLSL450
%1 = OpTypeVoid
%2 = OpTypeFunction %1
%3 = OpFunction %1 None %2
%4 = OpLabel
OpReturn
OpFunctionEnd)";

  std::unique_ptr<IRContext> context =
      BuildModule(SPV_ENV_UNIVERSAL_1_1, nullptr, text,
                  SPV_TEXT_TO_BINARY_OPTION_PRESERVE_NUMERIC_IDS);
  uint32_t current_bound = context->module()->id_bound();
  context->set_max_id_bound(current_bound + 1);
  uint32_t next_id_bound = context->TakeNextId();
  EXPECT_EQ(next_id_bound, current_bound);
  EXPECT_EQ(current_bound + 1, context->module()->id_bound());
  next_id_bound = context->TakeNextId();
  EXPECT_EQ(next_id_bound, 0);
}

TEST_F(IRContextTest, IdBoundTestUIntMax) {
  const std::string text = R"(
OpCapability Shader
OpCapability Linkage
OpMemoryModel Logical GLSL450
%1 = OpTypeVoid
%2 = OpTypeFunction %1
%3 = OpFunction %1 None %2
%4294967294 = OpLabel ; ID is UINT_MAX-1
OpReturn
OpFunctionEnd)";

  std::unique_ptr<IRContext> context =
      BuildModule(SPV_ENV_UNIVERSAL_1_1, nullptr, text,
                  SPV_TEXT_TO_BINARY_OPTION_PRESERVE_NUMERIC_IDS);
  uint32_t current_bound = context->module()->id_bound();

  // Expecting |BuildModule| to preserve the numeric ids.
  EXPECT_EQ(current_bound, std::numeric_limits<uint32_t>::max());

  context->set_max_id_bound(current_bound);
  uint32_t next_id_bound = context->TakeNextId();
  EXPECT_EQ(next_id_bound, 0);
  EXPECT_EQ(current_bound, context->module()->id_bound());
}

TEST_F(IRContextTest, CfgAndDomAnalysis) {
  const std::string text = R"(
OpCapability Shader
OpCapability Linkage
OpMemoryModel Logical GLSL450
%1 = OpTypeVoid
%2 = OpTypeFunction %1
%3 = OpFunction %1 None %2
%4 = OpLabel
OpReturn
OpFunctionEnd)";

  std::unique_ptr<IRContext> ctx =
      BuildModule(SPV_ENV_UNIVERSAL_1_1, nullptr, text,
                  SPV_TEXT_TO_BINARY_OPTION_PRESERVE_NUMERIC_IDS);

  // Building the dominator analysis should build the CFG.
  ASSERT_TRUE(ctx->module()->begin() != ctx->module()->end());
  ctx->GetDominatorAnalysis(&*ctx->module()->begin());

  EXPECT_TRUE(ctx->AreAnalysesValid(IRContext::kAnalysisCFG));
  EXPECT_TRUE(ctx->AreAnalysesValid(IRContext::kAnalysisDominatorAnalysis));

  // Invalidating the CFG analysis should invalidate the dominator analysis.
  ctx->InvalidateAnalyses(IRContext::kAnalysisCFG);
  EXPECT_FALSE(ctx->AreAnalysesValid(IRContext::kAnalysisCFG));
  EXPECT_FALSE(ctx->AreAnalysesValid(IRContext::kAnalysisDominatorAnalysis));
}

TEST_F(IRContextTest, AsanErrorTest) {
  std::string shader = R"(
               OpCapability Shader
          %1 = OpExtInstImport "GLSL.std.450"
               OpMemoryModel Logical GLSL450
               OpEntryPoint Fragment %4 "main"
               OpExecutionMode %4 OriginUpperLeft
               OpSource ESSL 310
               OpName %4 "main"
               OpName %8 "x"
               OpName %10 "y"
               OpDecorate %8 RelaxedPrecision
               OpDecorate %10 RelaxedPrecision
               OpDecorate %11 RelaxedPrecision
          %2 = OpTypeVoid
          %3 = OpTypeFunction %2
          %6 = OpTypeInt 32 1
          %7 = OpTypePointer Function %6
          %9 = OpConstant %6 1
          %4 = OpFunction %2 None %3
          %5 = OpLabel
          %8 = OpVariable %7 Function
         %10 = OpVariable %7 Function
               OpStore %8 %9
         %11 = OpLoad %6 %8
	       OpBranch %20
	 %20 = OpLabel
	 %21 = OpPhi %6 %11 %5
         OpStore %10 %21
         OpReturn
         OpFunctionEnd
  )";

  const auto env = SPV_ENV_UNIVERSAL_1_3;
  const auto consumer = nullptr;
  const auto context = BuildModule(
      env, consumer, shader, SPV_TEXT_TO_BINARY_OPTION_PRESERVE_NUMERIC_IDS);

  opt::Function* fun =
      context->cfg()->block(5)->GetParent();  // Computes the CFG analysis
  opt::DominatorAnalysis* dom = nullptr;
  dom = context->GetDominatorAnalysis(fun);  // Computes the dominator analysis,
                                             // which depends on the CFG
                                             // analysis
  context->InvalidateAnalysesExceptFor(
      opt::IRContext::Analysis::kAnalysisDominatorAnalysis);  // Invalidates the
                                                              // CFG analysis
  dom = context->GetDominatorAnalysis(
      fun);  // Recompute the CFG analysis because the Dominator tree uses it.
  auto bb = dom->ImmediateDominator(5);
  std::cout
      << bb->id();  // Make sure asan does not complain about use after free.
}

TEST_F(IRContextTest, DebugInstructionReplaceSingleUse) {
  const std::string text = R"(
OpCapability Shader
OpCapability Linkage
%1 = OpExtInstImport "OpenCL.DebugInfo.100"
OpMemoryModel Logical GLSL450
%2 = OpString "test"
%3 = OpTypeVoid
%4 = OpTypeFunction %3
%5 = OpTypeFloat 32
%6 = OpTypePointer Function %5
%7 = OpConstant %5 0
%8 = OpTypeInt 32 0
%9 = OpConstant %8 32
%10 = OpExtInst %3 %1 DebugExpression
%11 = OpExtInst %3 %1 DebugSource %2
%12 = OpExtInst %3 %1 DebugCompilationUnit 1 4 %11 HLSL
%13 = OpExtInst %3 %1 DebugTypeFunction FlagIsProtected|FlagIsPrivate %3
%14 = OpExtInst %3 %1 DebugFunction %2 %13 %11 0 0 %12 %2 FlagIsProtected|FlagIsPrivate 0 %17
%15 = OpExtInst %3 %1 DebugTypeBasic %2 %9 Float
%16 = OpExtInst %3 %1 DebugLocalVariable %2 %15 %11 0 0 %14 FlagIsLocal
%17 = OpFunction %3 None %4
%18 = OpLabel
%19 = OpExtInst %3 %1 DebugScope %14
%20 = OpVariable %6 Function
%26 = OpVariable %6 Function
OpBranch %21
%21 = OpLabel
%22 = OpPhi %5 %7 %18
OpBranch %23
%23 = OpLabel
OpLine %2 0 0
OpStore %20 %7
%24 = OpExtInst %3 %1 DebugValue %16 %22 %10
%25 = OpExtInst %3 %1 DebugDeclare %16 %26 %10
OpReturn
OpFunctionEnd)";

  std::unique_ptr<IRContext> ctx =
      BuildModule(SPV_ENV_UNIVERSAL_1_1, nullptr, text,
                  SPV_TEXT_TO_BINARY_OPTION_PRESERVE_NUMERIC_IDS);
  ctx->BuildInvalidAnalyses(IRContext::kAnalysisDebugInfo);
  NoopPassPreservesAll pass(Pass::Status::SuccessWithChange);
  pass.Run(ctx.get());
  EXPECT_TRUE(ctx->AreAnalysesValid(IRContext::kAnalysisDebugInfo));

  auto* dbg_value = ctx->get_def_use_mgr()->GetDef(24);
  EXPECT_TRUE(dbg_value->GetSingleWordOperand(kDebugValueOperandValueIndex) ==
              22);
  EXPECT_TRUE(ctx->ReplaceAllUsesWith(22, 7));
  dbg_value = ctx->get_def_use_mgr()->GetDef(24);
  EXPECT_TRUE(dbg_value->GetSingleWordOperand(kDebugValueOperandValueIndex) ==
              7);

  auto* dbg_decl = ctx->get_def_use_mgr()->GetDef(25);
  EXPECT_TRUE(
      dbg_decl->GetSingleWordOperand(kDebugDeclareOperandVariableIndex) == 26);
  EXPECT_TRUE(ctx->ReplaceAllUsesWith(26, 20));
  dbg_decl = ctx->get_def_use_mgr()->GetDef(25);
  EXPECT_TRUE(
      dbg_decl->GetSingleWordOperand(kDebugDeclareOperandVariableIndex) == 20);
}

TEST_F(IRContextTest, DebugInstructionReplaceAllUses) {
  const std::string text = R"(
OpCapability Shader
OpCapability Linkage
%1 = OpExtInstImport "OpenCL.DebugInfo.100"
OpMemoryModel Logical GLSL450
%2 = OpString "test"
%3 = OpTypeVoid
%4 = OpTypeFunction %3
%5 = OpTypeFloat 32
%6 = OpTypePointer Function %5
%7 = OpConstant %5 0
%8 = OpTypeInt 32 0
%9 = OpConstant %8 32
%10 = OpExtInst %3 %1 DebugExpression
%11 = OpExtInst %3 %1 DebugSource %2
%12 = OpExtInst %3 %1 DebugCompilationUnit 1 4 %11 HLSL
%13 = OpExtInst %3 %1 DebugTypeFunction FlagIsProtected|FlagIsPrivate %3
%14 = OpExtInst %3 %1 DebugFunction %2 %13 %11 0 0 %12 %2 FlagIsProtected|FlagIsPrivate 0 %17
%15 = OpExtInst %3 %1 DebugTypeBasic %2 %9 Float
%16 = OpExtInst %3 %1 DebugLocalVariable %2 %15 %11 0 0 %14 FlagIsLocal
%27 = OpExtInst %3 %1 DebugLocalVariable %2 %15 %11 1 0 %14 FlagIsLocal
%17 = OpFunction %3 None %4
%18 = OpLabel
%19 = OpExtInst %3 %1 DebugScope %14
%20 = OpVariable %6 Function
%26 = OpVariable %6 Function
OpBranch %21
%21 = OpLabel
%22 = OpPhi %5 %7 %18
OpBranch %23
%23 = OpLabel
OpLine %2 0 0
OpStore %20 %7
%24 = OpExtInst %3 %1 DebugValue %16 %22 %10
%25 = OpExtInst %3 %1 DebugDeclare %16 %26 %10
%28 = OpExtInst %3 %1 DebugValue %27 %22 %10
%29 = OpExtInst %3 %1 DebugDeclare %27 %26 %10
OpReturn
OpFunctionEnd)";

  std::unique_ptr<IRContext> ctx =
      BuildModule(SPV_ENV_UNIVERSAL_1_1, nullptr, text,
                  SPV_TEXT_TO_BINARY_OPTION_PRESERVE_NUMERIC_IDS);
  ctx->BuildInvalidAnalyses(IRContext::kAnalysisDebugInfo);
  NoopPassPreservesAll pass(Pass::Status::SuccessWithChange);
  pass.Run(ctx.get());
  EXPECT_TRUE(ctx->AreAnalysesValid(IRContext::kAnalysisDebugInfo));

  auto* dbg_value0 = ctx->get_def_use_mgr()->GetDef(24);
  auto* dbg_value1 = ctx->get_def_use_mgr()->GetDef(28);
  EXPECT_TRUE(dbg_value0->GetSingleWordOperand(kDebugValueOperandValueIndex) ==
              22);
  EXPECT_TRUE(dbg_value1->GetSingleWordOperand(kDebugValueOperandValueIndex) ==
              22);
  EXPECT_TRUE(ctx->ReplaceAllUsesWith(22, 7));
  dbg_value0 = ctx->get_def_use_mgr()->GetDef(24);
  dbg_value1 = ctx->get_def_use_mgr()->GetDef(28);
  EXPECT_TRUE(dbg_value0->GetSingleWordOperand(kDebugValueOperandValueIndex) ==
              7);
  EXPECT_TRUE(dbg_value1->GetSingleWordOperand(kDebugValueOperandValueIndex) ==
              7);

  auto* dbg_decl0 = ctx->get_def_use_mgr()->GetDef(25);
  auto* dbg_decl1 = ctx->get_def_use_mgr()->GetDef(29);
  EXPECT_TRUE(
      dbg_decl0->GetSingleWordOperand(kDebugDeclareOperandVariableIndex) == 26);
  EXPECT_TRUE(
      dbg_decl1->GetSingleWordOperand(kDebugDeclareOperandVariableIndex) == 26);
  EXPECT_TRUE(ctx->ReplaceAllUsesWith(26, 20));
  dbg_decl0 = ctx->get_def_use_mgr()->GetDef(25);
  dbg_decl1 = ctx->get_def_use_mgr()->GetDef(29);
  EXPECT_TRUE(
      dbg_decl0->GetSingleWordOperand(kDebugDeclareOperandVariableIndex) == 20);
  EXPECT_TRUE(
      dbg_decl1->GetSingleWordOperand(kDebugDeclareOperandVariableIndex) == 20);
}

TEST_F(IRContextTest, DebugInstructionReplaceDebugScopeAndDebugInlinedAt) {
  const std::string text = R"(
OpCapability Shader
OpCapability Linkage
%1 = OpExtInstImport "OpenCL.DebugInfo.100"
OpMemoryModel Logical GLSL450
%2 = OpString "test"
%3 = OpTypeVoid
%4 = OpTypeFunction %3
%5 = OpTypeFloat 32
%6 = OpTypePointer Function %5
%7 = OpConstant %5 0
%8 = OpTypeInt 32 0
%9 = OpConstant %8 32
%10 = OpExtInst %3 %1 DebugExpression
%11 = OpExtInst %3 %1 DebugSource %2
%12 = OpExtInst %3 %1 DebugCompilationUnit 1 4 %11 HLSL
%13 = OpExtInst %3 %1 DebugTypeFunction FlagIsProtected|FlagIsPrivate %3
%14 = OpExtInst %3 %1 DebugFunction %2 %13 %11 0 0 %12 %2 FlagIsProtected|FlagIsPrivate 0 %17
%15 = OpExtInst %3 %1 DebugInfoNone
%16 = OpExtInst %3 %1 DebugFunction %2 %13 %11 10 10 %12 %2 FlagIsProtected|FlagIsPrivate 0 %15
%25 = OpExtInst %3 %1 DebugInlinedAt 0 %14
%26 = OpExtInst %3 %1 DebugInlinedAt 2 %14
%17 = OpFunction %3 None %4
%18 = OpLabel
%19 = OpExtInst %3 %1 DebugScope %14
%20 = OpVariable %6 Function
OpBranch %21
%21 = OpLabel
%24 = OpExtInst %3 %1 DebugScope %16
%22 = OpPhi %5 %7 %18
OpBranch %23
%23 = OpLabel
%27 = OpExtInst %3 %1 DebugScope %16 %25
OpLine %2 0 0
%28 = OpFAdd %5 %7 %7
OpStore %20 %28
OpReturn
OpFunctionEnd)";

  std::unique_ptr<IRContext> ctx =
      BuildModule(SPV_ENV_UNIVERSAL_1_1, nullptr, text,
                  SPV_TEXT_TO_BINARY_OPTION_PRESERVE_NUMERIC_IDS);
  ctx->BuildInvalidAnalyses(IRContext::kAnalysisDebugInfo);
  NoopPassPreservesAll pass(Pass::Status::SuccessWithChange);
  pass.Run(ctx.get());
  EXPECT_TRUE(ctx->AreAnalysesValid(IRContext::kAnalysisDebugInfo));

  auto* inst0 = ctx->get_def_use_mgr()->GetDef(20);
  auto* inst1 = ctx->get_def_use_mgr()->GetDef(22);
  auto* inst2 = ctx->get_def_use_mgr()->GetDef(28);
  EXPECT_EQ(inst0->GetDebugScope().GetLexicalScope(), 14);
  EXPECT_EQ(inst1->GetDebugScope().GetLexicalScope(), 16);
  EXPECT_EQ(inst2->GetDebugScope().GetLexicalScope(), 16);
  EXPECT_EQ(inst2->GetDebugInlinedAt(), 25);

  EXPECT_TRUE(ctx->ReplaceAllUsesWith(14, 12));
  EXPECT_TRUE(ctx->ReplaceAllUsesWith(16, 14));
  EXPECT_TRUE(ctx->ReplaceAllUsesWith(25, 26));
  EXPECT_EQ(inst0->GetDebugScope().GetLexicalScope(), 12);
  EXPECT_EQ(inst1->GetDebugScope().GetLexicalScope(), 14);
  EXPECT_EQ(inst2->GetDebugScope().GetLexicalScope(), 14);
  EXPECT_EQ(inst2->GetDebugInlinedAt(), 26);
}

TEST_F(IRContextTest, AddDebugValueAfterReplaceUse) {
  const std::string text = R"(
OpCapability Shader
OpCapability Linkage
%1 = OpExtInstImport "OpenCL.DebugInfo.100"
OpMemoryModel Logical GLSL450
%2 = OpString "test"
%3 = OpTypeVoid
%4 = OpTypeFunction %3
%5 = OpTypeFloat 32
%6 = OpTypePointer Function %5
%7 = OpConstant %5 0
%8 = OpTypeInt 32 0
%9 = OpConstant %8 32
%10 = OpExtInst %3 %1 DebugExpression
%11 = OpExtInst %3 %1 DebugSource %2
%12 = OpExtInst %3 %1 DebugCompilationUnit 1 4 %11 HLSL
%13 = OpExtInst %3 %1 DebugTypeFunction FlagIsProtected|FlagIsPrivate %3
%14 = OpExtInst %3 %1 DebugFunction %2 %13 %11 0 0 %12 %2 FlagIsProtected|FlagIsPrivate 0 %17
%15 = OpExtInst %3 %1 DebugTypeBasic %2 %9 Float
%16 = OpExtInst %3 %1 DebugLocalVariable %2 %15 %11 0 0 %14 FlagIsLocal
%17 = OpFunction %3 None %4
%18 = OpLabel
%19 = OpExtInst %3 %1 DebugScope %14
%20 = OpVariable %6 Function
%26 = OpVariable %6 Function
OpBranch %21
%21 = OpLabel
%27 = OpExtInst %3 %1 DebugScope %14
%22 = OpPhi %5 %7 %18
OpBranch %23
%23 = OpLabel
%28 = OpExtInst %3 %1 DebugScope %14
OpLine %2 0 0
OpStore %20 %7
%24 = OpExtInst %3 %1 DebugValue %16 %22 %10
%25 = OpExtInst %3 %1 DebugDeclare %16 %26 %10
OpReturn
OpFunctionEnd)";

  std::unique_ptr<IRContext> ctx =
      BuildModule(SPV_ENV_UNIVERSAL_1_1, nullptr, text,
                  SPV_TEXT_TO_BINARY_OPTION_PRESERVE_NUMERIC_IDS);
  ctx->BuildInvalidAnalyses(IRContext::kAnalysisDebugInfo);
  NoopPassPreservesAll pass(Pass::Status::SuccessWithChange);
  pass.Run(ctx.get());
  EXPECT_TRUE(ctx->AreAnalysesValid(IRContext::kAnalysisDebugInfo));

  // Replace all uses of result it '26' with '20'
  auto* dbg_decl = ctx->get_def_use_mgr()->GetDef(25);
  EXPECT_EQ(dbg_decl->GetSingleWordOperand(kDebugDeclareOperandVariableIndex),
            26);
  EXPECT_TRUE(ctx->ReplaceAllUsesWith(26, 20));
  dbg_decl = ctx->get_def_use_mgr()->GetDef(25);
  EXPECT_EQ(dbg_decl->GetSingleWordOperand(kDebugDeclareOperandVariableIndex),
            20);

  // No DebugValue should be added because result id '26' is not used for
  // DebugDeclare.
  ctx->get_debug_info_mgr()->AddDebugValueIfVarDeclIsVisible(dbg_decl, 26, 22,
                                                             dbg_decl, nullptr);
  EXPECT_EQ(dbg_decl->NextNode()->opcode(), SpvOpReturn);

  // DebugValue should be added because result id '20' is used for DebugDeclare.
  ctx->get_debug_info_mgr()->AddDebugValueIfVarDeclIsVisible(dbg_decl, 20, 22,
                                                             dbg_decl, nullptr);
  EXPECT_EQ(dbg_decl->NextNode()->GetOpenCL100DebugOpcode(),
            OpenCLDebugInfo100DebugValue);

  // Replace all uses of result it '20' with '26'
  EXPECT_EQ(dbg_decl->GetSingleWordOperand(kDebugDeclareOperandVariableIndex),
            20);
  EXPECT_TRUE(ctx->ReplaceAllUsesWith(20, 26));
  EXPECT_EQ(dbg_decl->GetSingleWordOperand(kDebugDeclareOperandVariableIndex),
            26);

  // No DebugValue should be added because result id '20' is not used for
  // DebugDeclare.
  ctx->get_debug_info_mgr()->AddDebugValueIfVarDeclIsVisible(dbg_decl, 20, 7,
                                                             dbg_decl, nullptr);
  Instruction* dbg_value = dbg_decl->NextNode();
  EXPECT_EQ(dbg_value->GetOpenCL100DebugOpcode(), OpenCLDebugInfo100DebugValue);
  EXPECT_EQ(dbg_value->GetSingleWordOperand(kDebugValueOperandValueIndex), 22);

  // DebugValue should be added because result id '26' is used for DebugDeclare.
  ctx->get_debug_info_mgr()->AddDebugValueIfVarDeclIsVisible(dbg_decl, 26, 7,
                                                             dbg_decl, nullptr);
  dbg_value = dbg_decl->NextNode();
  EXPECT_EQ(dbg_value->GetOpenCL100DebugOpcode(), OpenCLDebugInfo100DebugValue);
  EXPECT_EQ(dbg_value->GetSingleWordOperand(kDebugValueOperandValueIndex), 7);
}

}  // namespace
}  // namespace opt
}  // namespace spvtools
