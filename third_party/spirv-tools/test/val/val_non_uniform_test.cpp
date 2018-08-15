// Copyright (c) 2018 Google LLC.
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

#include <sstream>
#include <string>
#include <tuple>

#include "gmock/gmock.h"
#include "test/unit_spirv.h"
#include "test/val/val_fixtures.h"

namespace spvtools {
namespace val {
namespace {

using ::testing::Combine;
using ::testing::HasSubstr;
using ::testing::Values;
using ::testing::ValuesIn;

std::string GenerateShaderCode(
    const std::string& body,
    const std::string& capabilities_and_extensions = "",
    const std::string& execution_model = "GLCompute") {
  std::ostringstream ss;
  ss << R"(
OpCapability Shader
OpCapability GroupNonUniform
OpCapability GroupNonUniformVote
OpCapability GroupNonUniformBallot
OpCapability GroupNonUniformShuffle
OpCapability GroupNonUniformShuffleRelative
OpCapability GroupNonUniformArithmetic
OpCapability GroupNonUniformClustered
OpCapability GroupNonUniformQuad
)";

  ss << capabilities_and_extensions;
  ss << "OpMemoryModel Logical GLSL450\n";
  ss << "OpEntryPoint " << execution_model << " %main \"main\"\n";

  ss << R"(
%void = OpTypeVoid
%func = OpTypeFunction %void
%bool = OpTypeBool
%u32 = OpTypeInt 32 0
%float = OpTypeFloat 32
%u32vec4 = OpTypeVector %u32 4

%true = OpConstantTrue %bool
%false = OpConstantFalse %bool

%u32_0 = OpConstant %u32 0

%float_0 = OpConstant %float 0

%u32vec4_null = OpConstantComposite %u32vec4 %u32_0 %u32_0 %u32_0 %u32_0

%cross_device = OpConstant %u32 0
%device = OpConstant %u32 1
%workgroup = OpConstant %u32 2
%subgroup = OpConstant %u32 3
%invocation = OpConstant %u32 4

%reduce = OpConstant %u32 0
%inclusive_scan = OpConstant %u32 1
%exclusive_scan = OpConstant %u32 2
%clustered_reduce = OpConstant %u32 3

%main = OpFunction %void None %func
%main_entry = OpLabel
)";

  ss << body;

  ss << R"(
OpReturn
OpFunctionEnd)";

  return ss.str();
}

SpvScope scopes[] = {SpvScopeCrossDevice, SpvScopeDevice, SpvScopeWorkgroup,
                     SpvScopeSubgroup, SpvScopeInvocation};

using GroupNonUniformScope = spvtest::ValidateBase<
    std::tuple<std::string, std::string, SpvScope, std::string>>;

std::string ConvertScope(SpvScope scope) {
  switch (scope) {
    case SpvScopeCrossDevice:
      return "%cross_device";
    case SpvScopeDevice:
      return "%device";
    case SpvScopeWorkgroup:
      return "%workgroup";
    case SpvScopeSubgroup:
      return "%subgroup";
    case SpvScopeInvocation:
      return "%invocation";
    default:
      return "";
  }
}

TEST_P(GroupNonUniformScope, Vulkan1p1) {
  std::string opcode = std::get<0>(GetParam());
  std::string type = std::get<1>(GetParam());
  SpvScope execution_scope = std::get<2>(GetParam());
  std::string args = std::get<3>(GetParam());

  std::ostringstream sstr;
  sstr << "%result = " << opcode << " ";
  sstr << type << " ";
  sstr << ConvertScope(execution_scope) << " ";
  sstr << args << "\n";

  CompileSuccessfully(GenerateShaderCode(sstr.str()), SPV_ENV_VULKAN_1_1);
  spv_result_t result = ValidateInstructions(SPV_ENV_VULKAN_1_1);
  if (execution_scope == SpvScopeSubgroup) {
    EXPECT_EQ(SPV_SUCCESS, result);
  } else {
    EXPECT_EQ(SPV_ERROR_INVALID_DATA, result);
    EXPECT_THAT(
        getDiagnosticString(),
        HasSubstr(
            "in Vulkan environment Execution scope is limited to Subgroup"));
  }
}

TEST_P(GroupNonUniformScope, Spirv1p3) {
  std::string opcode = std::get<0>(GetParam());
  std::string type = std::get<1>(GetParam());
  SpvScope execution_scope = std::get<2>(GetParam());
  std::string args = std::get<3>(GetParam());

  std::ostringstream sstr;
  sstr << "%result = " << opcode << " ";
  sstr << type << " ";
  sstr << ConvertScope(execution_scope) << " ";
  sstr << args << "\n";

  CompileSuccessfully(GenerateShaderCode(sstr.str()), SPV_ENV_UNIVERSAL_1_3);
  spv_result_t result = ValidateInstructions(SPV_ENV_UNIVERSAL_1_3);
  if (execution_scope == SpvScopeSubgroup ||
      execution_scope == SpvScopeWorkgroup) {
    EXPECT_EQ(SPV_SUCCESS, result);
  } else {
    EXPECT_EQ(SPV_ERROR_INVALID_DATA, result);
    EXPECT_THAT(
        getDiagnosticString(),
        HasSubstr("Execution scope is limited to Subgroup or Workgroup"));
  }
}

INSTANTIATE_TEST_CASE_P(GroupNonUniformElect, GroupNonUniformScope,
                        Combine(Values("OpGroupNonUniformElect"),
                                Values("%bool"), ValuesIn(scopes), Values("")));

INSTANTIATE_TEST_CASE_P(GroupNonUniformVote, GroupNonUniformScope,
                        Combine(Values("OpGroupNonUniformAll",
                                       "OpGroupNonUniformAny",
                                       "OpGroupNonUniformAllEqual"),
                                Values("%bool"), ValuesIn(scopes),
                                Values("%true")));

INSTANTIATE_TEST_CASE_P(GroupNonUniformBroadcast, GroupNonUniformScope,
                        Combine(Values("OpGroupNonUniformBroadcast"),
                                Values("%bool"), ValuesIn(scopes),
                                Values("%true %u32_0")));

INSTANTIATE_TEST_CASE_P(GroupNonUniformBroadcastFirst, GroupNonUniformScope,
                        Combine(Values("OpGroupNonUniformBroadcastFirst"),
                                Values("%bool"), ValuesIn(scopes),
                                Values("%true")));

INSTANTIATE_TEST_CASE_P(GroupNonUniformBallot, GroupNonUniformScope,
                        Combine(Values("OpGroupNonUniformBallot"),
                                Values("%u32vec4"), ValuesIn(scopes),
                                Values("%true")));

INSTANTIATE_TEST_CASE_P(GroupNonUniformInverseBallot, GroupNonUniformScope,
                        Combine(Values("OpGroupNonUniformInverseBallot"),
                                Values("%bool"), ValuesIn(scopes),
                                Values("%u32vec4_null")));

INSTANTIATE_TEST_CASE_P(GroupNonUniformBallotBitExtract, GroupNonUniformScope,
                        Combine(Values("OpGroupNonUniformBallotBitExtract"),
                                Values("%bool"), ValuesIn(scopes),
                                Values("%u32vec4_null %u32_0")));

INSTANTIATE_TEST_CASE_P(GroupNonUniformBallotBitCount, GroupNonUniformScope,
                        Combine(Values("OpGroupNonUniformBallotBitCount"),
                                Values("%u32"), ValuesIn(scopes),
                                Values("Reduce %u32vec4_null")));

INSTANTIATE_TEST_CASE_P(GroupNonUniformBallotFind, GroupNonUniformScope,
                        Combine(Values("OpGroupNonUniformBallotFindLSB",
                                       "OpGroupNonUniformBallotFindMSB"),
                                Values("%u32"), ValuesIn(scopes),
                                Values("%u32vec4_null")));

INSTANTIATE_TEST_CASE_P(GroupNonUniformShuffle, GroupNonUniformScope,
                        Combine(Values("OpGroupNonUniformShuffle",
                                       "OpGroupNonUniformShuffleXor",
                                       "OpGroupNonUniformShuffleUp",
                                       "OpGroupNonUniformShuffleDown"),
                                Values("%u32"), ValuesIn(scopes),
                                Values("%u32_0 %u32_0")));

INSTANTIATE_TEST_CASE_P(
    GroupNonUniformIntegerArithmetic, GroupNonUniformScope,
    Combine(Values("OpGroupNonUniformIAdd", "OpGroupNonUniformIMul",
                   "OpGroupNonUniformSMin", "OpGroupNonUniformUMin",
                   "OpGroupNonUniformSMax", "OpGroupNonUniformUMax",
                   "OpGroupNonUniformBitwiseAnd", "OpGroupNonUniformBitwiseOr",
                   "OpGroupNonUniformBitwiseXor"),
            Values("%u32"), ValuesIn(scopes), Values("Reduce %u32_0")));

INSTANTIATE_TEST_CASE_P(
    GroupNonUniformFloatArithmetic, GroupNonUniformScope,
    Combine(Values("OpGroupNonUniformFAdd", "OpGroupNonUniformFMul",
                   "OpGroupNonUniformFMin", "OpGroupNonUniformFMax"),
            Values("%float"), ValuesIn(scopes), Values("Reduce %float_0")));

INSTANTIATE_TEST_CASE_P(GroupNonUniformLogicalArithmetic, GroupNonUniformScope,
                        Combine(Values("OpGroupNonUniformLogicalAnd",
                                       "OpGroupNonUniformLogicalOr",
                                       "OpGroupNonUniformLogicalXor"),
                                Values("%bool"), ValuesIn(scopes),
                                Values("Reduce %true")));

INSTANTIATE_TEST_CASE_P(GroupNonUniformQuad, GroupNonUniformScope,
                        Combine(Values("OpGroupNonUniformQuadBroadcast",
                                       "OpGroupNonUniformQuadSwap"),
                                Values("%u32"), ValuesIn(scopes),
                                Values("%u32_0 %u32_0")));

}  // namespace
}  // namespace val
}  // namespace spvtools
