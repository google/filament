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

#include "spirv-tools/optimizer.hpp"

#include <sstream>
#include <string>
#include <vector>

#include "gmock/gmock.h"
#include "spirv-tools/libspirv.hpp"
#include "test/opt/pass_fixture.h"

namespace spvtools {
namespace opt {
namespace {

using ::testing::Eq;

// Return a string that contains the minimum instructions needed to form
// a valid module.  Other instructions can be appended to this string.
std::string Header() {
  return R"(OpCapability Shader
OpCapability Linkage
OpMemoryModel Logical GLSL450
)";
}

TEST(Optimizer, CanRunNullPassWithDistinctInputOutputVectors) {
  SpirvTools tools(SPV_ENV_UNIVERSAL_1_0);
  std::vector<uint32_t> binary_in;
  tools.Assemble(Header() + "OpName %foo \"foo\"\n%foo = OpTypeVoid",
                 &binary_in);

  Optimizer opt(SPV_ENV_UNIVERSAL_1_0);
  opt.RegisterPass(CreateNullPass());
  std::vector<uint32_t> binary_out;
  opt.Run(binary_in.data(), binary_in.size(), &binary_out);

  std::string disassembly;
  tools.Disassemble(binary_out.data(), binary_out.size(), &disassembly);
  EXPECT_THAT(disassembly,
              Eq(Header() + "OpName %foo \"foo\"\n%foo = OpTypeVoid\n"));
}

TEST(Optimizer, CanRunTransformingPassWithDistinctInputOutputVectors) {
  SpirvTools tools(SPV_ENV_UNIVERSAL_1_0);
  std::vector<uint32_t> binary_in;
  tools.Assemble(Header() + "OpName %foo \"foo\"\n%foo = OpTypeVoid",
                 &binary_in);

  Optimizer opt(SPV_ENV_UNIVERSAL_1_0);
  opt.RegisterPass(CreateStripDebugInfoPass());
  std::vector<uint32_t> binary_out;
  opt.Run(binary_in.data(), binary_in.size(), &binary_out);

  std::string disassembly;
  tools.Disassemble(binary_out.data(), binary_out.size(), &disassembly);
  EXPECT_THAT(disassembly, Eq(Header() + "%void = OpTypeVoid\n"));
}

TEST(Optimizer, CanRunNullPassWithAliasedVectors) {
  SpirvTools tools(SPV_ENV_UNIVERSAL_1_0);
  std::vector<uint32_t> binary;
  tools.Assemble("OpName %foo \"foo\"\n%foo = OpTypeVoid", &binary);

  Optimizer opt(SPV_ENV_UNIVERSAL_1_0);
  opt.RegisterPass(CreateNullPass());
  opt.Run(binary.data(), binary.size(), &binary);  // This is the key.

  std::string disassembly;
  tools.Disassemble(binary.data(), binary.size(), &disassembly);
  EXPECT_THAT(disassembly, Eq("OpName %foo \"foo\"\n%foo = OpTypeVoid\n"));
}

TEST(Optimizer, CanRunNullPassWithAliasedVectorDataButDifferentSize) {
  SpirvTools tools(SPV_ENV_UNIVERSAL_1_0);
  std::vector<uint32_t> binary;
  tools.Assemble(Header() + "OpName %foo \"foo\"\n%foo = OpTypeVoid", &binary);

  Optimizer opt(SPV_ENV_UNIVERSAL_1_0);
  opt.RegisterPass(CreateNullPass());
  auto orig_size = binary.size();
  // Now change the size.  Add a word that will be ignored
  // by the optimizer.
  binary.push_back(42);
  EXPECT_THAT(orig_size + 1, Eq(binary.size()));
  opt.Run(binary.data(), orig_size, &binary);  // This is the key.
  // The binary vector should have been rewritten.
  EXPECT_THAT(binary.size(), Eq(orig_size));

  std::string disassembly;
  tools.Disassemble(binary.data(), binary.size(), &disassembly);
  EXPECT_THAT(disassembly,
              Eq(Header() + "OpName %foo \"foo\"\n%foo = OpTypeVoid\n"));
}

TEST(Optimizer, CanRunTransformingPassWithAliasedVectors) {
  SpirvTools tools(SPV_ENV_UNIVERSAL_1_0);
  std::vector<uint32_t> binary;
  tools.Assemble(Header() + "OpName %foo \"foo\"\n%foo = OpTypeVoid", &binary);

  Optimizer opt(SPV_ENV_UNIVERSAL_1_0);
  opt.RegisterPass(CreateStripDebugInfoPass());
  opt.Run(binary.data(), binary.size(), &binary);  // This is the key

  std::string disassembly;
  tools.Disassemble(binary.data(), binary.size(), &disassembly);
  EXPECT_THAT(disassembly, Eq(Header() + "%void = OpTypeVoid\n"));
}

TEST(Optimizer, CanValidateFlags) {
  Optimizer opt(SPV_ENV_UNIVERSAL_1_0);
  EXPECT_FALSE(opt.FlagHasValidForm("bad-flag"));
  EXPECT_TRUE(opt.FlagHasValidForm("-O"));
  EXPECT_TRUE(opt.FlagHasValidForm("-Os"));
  EXPECT_FALSE(opt.FlagHasValidForm("-O2"));
  EXPECT_TRUE(opt.FlagHasValidForm("--this_flag"));
}

TEST(Optimizer, CanRegisterPassesFromFlags) {
  SpirvTools tools(SPV_ENV_UNIVERSAL_1_0);
  Optimizer opt(SPV_ENV_UNIVERSAL_1_0);

  spv_message_level_t msg_level;
  const char* msg_fname;
  spv_position_t msg_position;
  const char* msg;
  auto examine_message = [&msg_level, &msg_fname, &msg_position, &msg](
                             spv_message_level_t ml, const char* f,
                             const spv_position_t& p, const char* m) {
    msg_level = ml;
    msg_fname = f;
    msg_position = p;
    msg = m;
  };
  opt.SetMessageConsumer(examine_message);

  std::vector<std::string> pass_flags = {
      "--strip-debug",
      "--strip-nonsemantic",
      "--set-spec-const-default-value=23:42 21:12",
      "--if-conversion",
      "--freeze-spec-const",
      "--inline-entry-points-exhaustive",
      "--inline-entry-points-opaque",
      "--convert-local-access-chains",
      "--eliminate-dead-code-aggressive",
      "--eliminate-insert-extract",
      "--eliminate-local-single-block",
      "--eliminate-local-single-store",
      "--merge-blocks",
      "--merge-return",
      "--eliminate-dead-branches",
      "--eliminate-dead-functions",
      "--eliminate-local-multi-store",
      "--eliminate-dead-const",
      "--eliminate-dead-inserts",
      "--eliminate-dead-variables",
      "--fold-spec-const-op-composite",
      "--loop-unswitch",
      "--scalar-replacement=300",
      "--scalar-replacement",
      "--strength-reduction",
      "--unify-const",
      "--flatten-decorations",
      "--compact-ids",
      "--cfg-cleanup",
      "--local-redundancy-elimination",
      "--loop-invariant-code-motion",
      "--reduce-load-size",
      "--redundancy-elimination",
      "--private-to-local",
      "--remove-duplicates",
      "--workaround-1209",
      "--replace-invalid-opcode",
      "--simplify-instructions",
      "--ssa-rewrite",
      "--copy-propagate-arrays",
      "--loop-fission=20",
      "--loop-fusion=2",
      "--loop-unroll",
      "--vector-dce",
      "--loop-unroll-partial=3",
      "--loop-peeling",
      "--ccp",
      "-O",
      "-Os",
      "--legalize-hlsl"};
  EXPECT_TRUE(opt.RegisterPassesFromFlags(pass_flags));

  // Test some invalid flags.
  EXPECT_FALSE(opt.RegisterPassFromFlag("-O2"));
  EXPECT_EQ(msg_level, SPV_MSG_ERROR);

  EXPECT_FALSE(opt.RegisterPassFromFlag("-loop-unroll"));
  EXPECT_EQ(msg_level, SPV_MSG_ERROR);

  EXPECT_FALSE(opt.RegisterPassFromFlag("--set-spec-const-default-value"));
  EXPECT_EQ(msg_level, SPV_MSG_ERROR);

  EXPECT_FALSE(opt.RegisterPassFromFlag("--scalar-replacement=s"));
  EXPECT_EQ(msg_level, SPV_MSG_ERROR);

  EXPECT_FALSE(opt.RegisterPassFromFlag("--loop-fission=-4"));
  EXPECT_EQ(msg_level, SPV_MSG_ERROR);

  EXPECT_FALSE(opt.RegisterPassFromFlag("--loop-fusion=xx"));
  EXPECT_EQ(msg_level, SPV_MSG_ERROR);

  EXPECT_FALSE(opt.RegisterPassFromFlag("--loop-unroll-partial"));
  EXPECT_EQ(msg_level, SPV_MSG_ERROR);
}


TEST(Optimizer, RemoveNop) {
  // Test that OpNops are removed even if no optimizations are run.
  const std::string before = R"(OpCapability Shader
OpCapability Linkage
OpMemoryModel Logical GLSL450
%void = OpTypeVoid
%2 = OpTypeFunction %void
%3 = OpFunction %void None %2
%4 = OpLabel
OpNop
OpReturn
OpFunctionEnd
)";

  const std::string after = R"(OpCapability Shader
OpCapability Linkage
OpMemoryModel Logical GLSL450
%void = OpTypeVoid
%2 = OpTypeFunction %void
%3 = OpFunction %void None %2
%4 = OpLabel
OpReturn
OpFunctionEnd
)";

  std::vector<uint32_t> binary;
  {
    SpirvTools tools(SPV_ENV_VULKAN_1_1);
    tools.Assemble(before, &binary);
  }

  Optimizer opt(SPV_ENV_VULKAN_1_1);

  std::vector<uint32_t> optimized;
  class ValidatorOptions validator_options;
  ASSERT_TRUE(opt.Run(binary.data(), binary.size(), &optimized,
                      validator_options, true))
      << before << "\n";
  std::string disassembly;
  {
    SpirvTools tools(SPV_ENV_VULKAN_1_1);
    tools.Disassemble(optimized.data(), optimized.size(), &disassembly);
  }

  EXPECT_EQ(after, disassembly)
      << "Was expecting the OpNop to have been removed.";
}

TEST(Optimizer, AvoidIntegrityCheckForExtraLineInfo) {
  // Test that it avoids the integrity check when no optimizations are run and
  // OpLines are propagated.
  const std::string before = R"(OpCapability Shader
OpCapability Linkage
OpMemoryModel Logical GLSL450
%1 = OpString "Test"
%void = OpTypeVoid
%3 = OpTypeFunction %void
%uint = OpTypeInt 32 0
%_ptr_Function_uint = OpTypePointer Function %uint
%6 = OpFunction %void None %3
%7 = OpLabel
OpLine %1 10 0
%8 = OpVariable %_ptr_Function_uint Function
OpLine %1 10 0
%9 = OpVariable %_ptr_Function_uint Function
OpLine %1 20 0
OpReturn
OpFunctionEnd
)";

  const std::string after = R"(OpCapability Shader
OpCapability Linkage
OpMemoryModel Logical GLSL450
%1 = OpString "Test"
%void = OpTypeVoid
%3 = OpTypeFunction %void
%uint = OpTypeInt 32 0
%_ptr_Function_uint = OpTypePointer Function %uint
%6 = OpFunction %void None %3
%7 = OpLabel
OpLine %1 10 0
%8 = OpVariable %_ptr_Function_uint Function
%9 = OpVariable %_ptr_Function_uint Function
OpLine %1 20 0
OpReturn
OpFunctionEnd
)";

  std::vector<uint32_t> binary;
  SpirvTools tools(SPV_ENV_VULKAN_1_1);
  tools.Assemble(before, &binary);

  Optimizer opt(SPV_ENV_VULKAN_1_1);

  std::vector<uint32_t> optimized;
  class ValidatorOptions validator_options;
  ASSERT_TRUE(opt.Run(binary.data(), binary.size(), &optimized,
                      validator_options, true))
      << before << "\n";

  std::string disassembly;
  tools.Disassemble(optimized.data(), optimized.size(), &disassembly);

  EXPECT_EQ(after, disassembly)
      << "Was expecting the OpLine to have been propagated.";
}

TEST(Optimizer, AvoidIntegrityCheckForDebugScope) {
  // Test that it avoids the integrity check when the code contains DebugScope.
  const std::string before = R"(OpCapability Shader
%1 = OpExtInstImport "OpenCL.DebugInfo.100"
OpMemoryModel Logical GLSL450
OpEntryPoint Fragment %main "main"
OpExecutionMode %main OriginUpperLeft
%3 = OpString "simple_vs.hlsl"
OpSource HLSL 600 %3
OpName %main "main"
%void = OpTypeVoid
%5 = OpTypeFunction %void
%6 = OpExtInst %void %1 DebugSource %3
%7 = OpExtInst %void %1 DebugCompilationUnit 2 4 %6 HLSL
%main = OpFunction %void None %5
%14 = OpLabel
%26 = OpExtInst %void %1 DebugScope %7
OpReturn
%27 = OpExtInst %void %1 DebugNoScope
OpFunctionEnd
)";

  const std::string after = R"(OpCapability Shader
%1 = OpExtInstImport "OpenCL.DebugInfo.100"
OpMemoryModel Logical GLSL450
OpEntryPoint Fragment %main "main"
OpExecutionMode %main OriginUpperLeft
%3 = OpString "simple_vs.hlsl"
OpSource HLSL 600 %3
OpName %main "main"
%void = OpTypeVoid
%5 = OpTypeFunction %void
%6 = OpExtInst %void %1 DebugSource %3
%7 = OpExtInst %void %1 DebugCompilationUnit 2 4 %6 HLSL
%main = OpFunction %void None %5
%8 = OpLabel
%11 = OpExtInst %void %1 DebugScope %7
OpReturn
%12 = OpExtInst %void %1 DebugNoScope
OpFunctionEnd
)";

  std::vector<uint32_t> binary;
  SpirvTools tools(SPV_ENV_VULKAN_1_1);
  tools.Assemble(before, &binary);

  Optimizer opt(SPV_ENV_VULKAN_1_1);

  std::vector<uint32_t> optimized;
  ASSERT_TRUE(opt.Run(binary.data(), binary.size(), &optimized))
      << before << "\n";

  std::string disassembly;
  tools.Disassemble(optimized.data(), optimized.size(), &disassembly);

  EXPECT_EQ(after, disassembly)
      << "Was expecting the result id of DebugScope to have been changed.";
}

TEST(Optimizer, CheckDefaultPerformancePassesLargeStructScalarization) {
  std::string start = R"(OpCapability Shader
%1 = OpExtInstImport "GLSL.std.450"
OpMemoryModel Logical GLSL450
OpEntryPoint Vertex %4 "main" %46 %48
OpSource GLSL 430
OpName %4 "main"
OpDecorate %44 Block
OpMemberDecorate %44 0 BuiltIn Position
OpMemberDecorate %44 1 BuiltIn PointSize
OpMemberDecorate %44 2 BuiltIn ClipDistance
OpDecorate %48 Location 0
%2 = OpTypeVoid
%3 = OpTypeFunction %2
%6 = OpTypeFloat 32
%7 = OpTypeVector %6 4
%8 = OpTypePointer Function %7
%9 = OpTypeStruct %7)";

  // add 200 float members to the struct
  for (int i = 0; i < 200; i++) {
    start += " %6";
  }

  start += R"(
%10 = OpTypeFunction %9 %8
%14 = OpTypeFunction %6 %9
%18 = OpTypePointer Function %9
%20 = OpTypeInt 32 1
%21 = OpConstant %20 0
%24 = OpConstant %20 1
%25 = OpTypeInt 32 0
%26 = OpConstant %25 1
%27 = OpTypePointer Function %6
%43 = OpTypeArray %6 %26
%44 = OpTypeStruct %7 %6 %43
%45 = OpTypePointer Output %44
%46 = OpVariable %45 Output
%47 = OpTypePointer Input %7
%48 = OpVariable %47 Input
%54 = OpTypePointer Output %7
%4 = OpFunction %2 None %3
%5 = OpLabel
%49 = OpVariable %8 Function
%50 = OpLoad %7 %48
OpStore %49 %50
%51 = OpFunctionCall %9 %12 %49
%52 = OpFunctionCall %6 %16 %51
%53 = OpCompositeConstruct %7 %52 %52 %52 %52
%55 = OpAccessChain %54 %46 %21
OpStore %55 %53
OpReturn
OpFunctionEnd
%12 = OpFunction %9 None %10
%11 = OpFunctionParameter %8
%13 = OpLabel
%19 = OpVariable %18 Function
%22 = OpLoad %7 %11
%23 = OpAccessChain %8 %19 %21
OpStore %23 %22
%28 = OpAccessChain %27 %11 %26
%29 = OpLoad %6 %28
%30 = OpConvertFToS %20 %29
%31 = OpAccessChain %27 %19 %21 %30
%32 = OpLoad %6 %31
%33 = OpAccessChain %27 %19 %24
OpStore %33 %32
%34 = OpLoad %9 %19
OpReturnValue %34
OpFunctionEnd
%16 = OpFunction %6 None %14
%15 = OpFunctionParameter %9
%17 = OpLabel
%37 = OpCompositeExtract %6 %15 1
%38 = OpConvertFToS %20 %37
%39 = OpCompositeExtract %7 %15 0
%40 = OpVectorExtractDynamic %6 %39 %38
OpReturnValue %40
OpFunctionEnd)";

  std::vector<uint32_t> binary;
  SpirvTools tools(SPV_ENV_VULKAN_1_3);
  tools.Assemble(start, &binary,
                 SPV_TEXT_TO_BINARY_OPTION_PRESERVE_NUMERIC_IDS);

  std::string test_disassembly;
  std::string default_disassembly;

  {
    Optimizer opt(SPV_ENV_VULKAN_1_3);
    opt.RegisterPerformancePasses();

    std::vector<uint32_t> optimized;
    ASSERT_TRUE(opt.Run(binary.data(), binary.size(), &optimized))
        << start << "\n";

    tools.Disassemble(optimized.data(), optimized.size(), &default_disassembly,
                      SPV_BINARY_TO_TEXT_OPTION_NO_HEADER);
  }

  {
    // default passes should not benefit from additional scalar replacement
    Optimizer opt(SPV_ENV_VULKAN_1_3);
    opt.RegisterPerformancePasses()
        .RegisterPass(CreateScalarReplacementPass(201))
        .RegisterPass(CreateAggressiveDCEPass());

    std::vector<uint32_t> optimized;
    ASSERT_TRUE(opt.Run(binary.data(), binary.size(), &optimized))
        << start << "\n";

    tools.Disassemble(optimized.data(), optimized.size(), &test_disassembly,
                      SPV_BINARY_TO_TEXT_OPTION_NO_HEADER);
  }

  EXPECT_EQ(test_disassembly, default_disassembly);
}

TEST(Optimizer, KeepDebugBuildIdentifierAfterDCE) {
  // Test that DebugBuildIdentifier is not removed after DCE.
  const std::string before = R"(
OpCapability Shader
OpExtension "SPV_KHR_non_semantic_info"
%1 = OpExtInstImport "NonSemantic.Shader.DebugInfo.100"
OpMemoryModel Logical GLSL450
OpEntryPoint GLCompute %main "main"
OpExecutionMode %main LocalSize 8 8 1
%4 = OpString "8937d8f571cf7b58d86d9d66196024f5d04e3186"
%7 = OpString ""
%9 = OpString ""
OpSource Slang 1
%19 = OpString ""
%24 = OpString ""
%25 = OpString ""
OpName %main "main"
%void = OpTypeVoid
%uint = OpTypeInt 32 0
%uint_0 = OpConstant %uint 0
%uint_11 = OpConstant %uint 11
%uint_5 = OpConstant %uint 5
%uint_100 = OpConstant %uint 100
%15 = OpTypeFunction %void
%uint_6 = OpConstant %uint 6
%uint_7 = OpConstant %uint 7
%uint_1 = OpConstant %uint 1
%uint_2 = OpConstant %uint 2
%3 = OpExtInst %void %1 DebugBuildIdentifier %4 %uint_0
%8 = OpExtInst %void %1 DebugSource %9 %7
%13 = OpExtInst %void %1 DebugCompilationUnit %uint_100 %uint_5 %8 %uint_11
%17 = OpExtInst %void %1 DebugTypeFunction %uint_0 %void
%18 = OpExtInst %void %1 DebugFunction %19 %17 %8 %uint_5 %uint_6 %13 %19 %uint_0 %uint_5
%23 = OpExtInst %void %1 DebugEntryPoint %18 %13 %24 %25
%main = OpFunction %void None %15
%16 = OpLabel
%21 = OpExtInst %void %1 DebugFunctionDefinition %18 %main
%32 = OpExtInst %void %1 DebugScope %18
%26 = OpExtInst %void %1 DebugLine %8 %uint_7 %uint_7 %uint_1 %uint_2
OpReturn
%33 = OpExtInst %void %1 DebugNoScope
OpFunctionEnd
  )";

  std::vector<uint32_t> binary;
  SpirvTools tools(SPV_ENV_VULKAN_1_3);
  tools.Assemble(before, &binary,
                 SPV_TEXT_TO_BINARY_OPTION_PRESERVE_NUMERIC_IDS);

  Optimizer opt(SPV_ENV_VULKAN_1_3);
  opt.RegisterPerformancePasses().RegisterPass(CreateAggressiveDCEPass());

  std::vector<uint32_t> optimized;
  ASSERT_TRUE(opt.Run(binary.data(), binary.size(), &optimized))
      << before << "\n";

  std::string after;
  tools.Disassemble(optimized.data(), optimized.size(), &after,
                    SPV_BINARY_TO_TEXT_OPTION_NO_HEADER);

  // Test that the DebugBuildIdentifier is not removed after DCE.
  size_t dbi_pos = after.find("DebugBuildIdentifier");
  EXPECT_NE(dbi_pos, std::string::npos)
      << "Was expecting the DebugBuildIdentifier to have been kept.";
  std::string string_id;
  std::string flags_id;
  if (dbi_pos != std::string::npos) {
    std::stringstream ss(after.substr(dbi_pos));
    std::string temp;
    char percent;
    ss >> temp;  // Consume "DebugBuildIdentifier"
    ss >> percent >> string_id;
    ss >> percent >> flags_id;
  }

  EXPECT_FALSE(string_id.empty())
      << "Could not find string id for DebugBuildIdentifier.";
  EXPECT_FALSE(flags_id.empty())
      << "Could not find flags id for DebugBuildIdentifier.";

  bool found =
      (after.find("%" + string_id + " = OpString") != std::string::npos);
  EXPECT_TRUE(found)
      << "Was expecting the DebugBuildIdentifier string to have been kept.";
  found = (after.find("%" + flags_id + " = OpConstant") != std::string::npos);
  EXPECT_TRUE(found)
      << "Was expecting the DebugBuildIdentifier constant to have been kept.";
}

}  // namespace
}  // namespace opt
}  // namespace spvtools
