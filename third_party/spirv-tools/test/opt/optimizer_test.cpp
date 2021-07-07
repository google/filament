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

#include <string>
#include <vector>

#include "gmock/gmock.h"
#include "spirv-tools/libspirv.hpp"
#include "spirv-tools/optimizer.hpp"
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
      "--strip-reflect",
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

}  // namespace
}  // namespace opt
}  // namespace spvtools
