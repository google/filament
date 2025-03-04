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
OpCapability GroupNonUniformPartitionedNV
OpCapability QuadControlKHR
OpExtension "SPV_NV_shader_subgroup_partitioned"
OpExtension "SPV_KHR_quad_control"
)";

  ss << capabilities_and_extensions;
  ss << "OpMemoryModel Logical GLSL450\n";
  ss << "OpEntryPoint " << execution_model << " %main \"main\"\n";
  if (execution_model == "GLCompute") {
    ss << "OpExecutionMode %main LocalSize 1 1 1\n";
  }

  ss << R"(
%void = OpTypeVoid
%func = OpTypeFunction %void
%bool = OpTypeBool
%u32 = OpTypeInt 32 0
%int = OpTypeInt 32 1
%float = OpTypeFloat 32
%u32vec4 = OpTypeVector %u32 4
%u32vec3 = OpTypeVector %u32 3
%v2bool = OpTypeVector %bool 2
%v4float = OpTypeVector %float 4
%struct = OpTypeStruct %int
%v4int = OpTypeVector %int 4

%true = OpConstantTrue %bool
%false = OpConstantFalse %bool

%u32_0 = OpConstant %u32 0
%int_0 = OpConstant %int 0

%float_0 = OpConstant %float 0

%u32vec4_null = OpConstantComposite %u32vec4 %u32_0 %u32_0 %u32_0 %u32_0
%u32vec3_null = OpConstantComposite %u32vec3 %u32_0 %u32_0 %u32_0
%v2bool_false = OpConstantNull %v2bool
%v4float_null = OpConstantNull %v4float
%struct_null = OpConstantNull %struct
%v4int_null = OpConstantComposite %v4int %int_0 %int_0 %int_0 %int_0

%u32_undef = OpUndef %u32

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

spv::Scope scopes[] = {spv::Scope::CrossDevice, spv::Scope::Device,
                       spv::Scope::Workgroup, spv::Scope::Subgroup,
                       spv::Scope::Invocation};

using ValidateGroupNonUniform = spvtest::ValidateBase<bool>;
using GroupNonUniform = spvtest::ValidateBase<
    std::tuple<std::string, std::string, spv::Scope, std::string, std::string>>;

std::string ConvertScope(spv::Scope scope) {
  switch (scope) {
    case spv::Scope::CrossDevice:
      return "%cross_device";
    case spv::Scope::Device:
      return "%device";
    case spv::Scope::Workgroup:
      return "%workgroup";
    case spv::Scope::Subgroup:
      return "%subgroup";
    case spv::Scope::Invocation:
      return "%invocation";
    default:
      return "";
  }
}

std::string ConvertMatch(const std::string& type) {
  if (type == "%bool") {
    return "%true";
  } else if (type == "%u32") {
    return "%u32_0";
  } else if (type == "%int") {
    return "%int_0";
  } else if (type == "%float") {
    return "%float_0";
  } else if (type == "%u32vec4") {
    return "%u32vec4_null";
  } else if (type == "%u32vec3") {
    return "%u32vec3_null";
  } else if (type == "%v2bool") {
    return "%v2bool_false";
  } else if (type == "%v4float") {
    return "%v4float_null";
  } else if (type == "%struct") {
    return "%struct_null";
  } else if (type == "%v4int") {
    return "%v4int_null";
  }

  return "INVALID";
}

TEST_P(GroupNonUniform, Vulkan1p1) {
  std::string opcode = std::get<0>(GetParam());
  std::string type = std::get<1>(GetParam());
  spv::Scope execution_scope = std::get<2>(GetParam());
  std::string args = std::get<3>(GetParam());
  std::string error = std::get<4>(GetParam());

  const std::string match = "match_res";
  size_t pos = std::string::npos;
  while ((pos = args.find(match)) != std::string::npos) {
    const std::string replace = ConvertMatch(type);
    args = args.substr(0, pos) + replace + args.substr(pos + match.size());
  }

  std::ostringstream sstr;
  sstr << "%result = " << opcode << " ";
  sstr << type << " ";
  if (opcode != "OpGroupNonUniformQuadAllKHR" &&
      opcode != "OpGroupNonUniformQuadAnyKHR") {
    sstr << ConvertScope(execution_scope) << " ";
  }
  sstr << args << "\n";

  CompileSuccessfully(GenerateShaderCode(sstr.str()), SPV_ENV_VULKAN_1_1);
  spv_result_t result = ValidateInstructions(SPV_ENV_VULKAN_1_1);
  if (error == "") {
    if (execution_scope == spv::Scope::Subgroup) {
      EXPECT_EQ(SPV_SUCCESS, result);
    } else {
      EXPECT_EQ(SPV_ERROR_INVALID_DATA, result);
      EXPECT_THAT(getDiagnosticString(),
                  AnyVUID("VUID-StandaloneSpirv-None-04642"));
      EXPECT_THAT(
          getDiagnosticString(),
          HasSubstr(
              "in Vulkan environment Execution scope is limited to Subgroup"));
    }
  } else {
    EXPECT_EQ(SPV_ERROR_INVALID_DATA, result);
    EXPECT_THAT(getDiagnosticString(), HasSubstr(error));
  }
}

TEST_P(GroupNonUniform, Spirv1p3) {
  std::string opcode = std::get<0>(GetParam());
  std::string type = std::get<1>(GetParam());
  spv::Scope execution_scope = std::get<2>(GetParam());
  std::string args = std::get<3>(GetParam());
  std::string error = std::get<4>(GetParam());

  const std::string match = "match_res";
  size_t pos = std::string::npos;
  while ((pos = args.find(match)) != std::string::npos) {
    const std::string replace = ConvertMatch(type);
    args = args.substr(0, pos) + replace + args.substr(pos + match.size());
  }

  std::ostringstream sstr;
  sstr << "%result = " << opcode << " ";
  sstr << type << " ";
  if (opcode != "OpGroupNonUniformQuadAllKHR" &&
      opcode != "OpGroupNonUniformQuadAnyKHR") {
    sstr << ConvertScope(execution_scope) << " ";
  }
  sstr << args << "\n";

  CompileSuccessfully(GenerateShaderCode(sstr.str()), SPV_ENV_UNIVERSAL_1_3);
  spv_result_t result = ValidateInstructions(SPV_ENV_UNIVERSAL_1_3);
  if (error == "") {
    if (execution_scope == spv::Scope::Subgroup ||
        execution_scope == spv::Scope::Workgroup) {
      EXPECT_EQ(SPV_SUCCESS, result);
    } else {
      EXPECT_EQ(SPV_ERROR_INVALID_DATA, result);
      EXPECT_THAT(
          getDiagnosticString(),
          HasSubstr("Execution scope is limited to Subgroup or Workgroup"));
    }
  } else {
    EXPECT_EQ(SPV_ERROR_INVALID_DATA, result);
    EXPECT_THAT(getDiagnosticString(), HasSubstr(error));
  }
}

INSTANTIATE_TEST_SUITE_P(GroupNonUniformElect, GroupNonUniform,
                         Combine(Values("OpGroupNonUniformElect"),
                                 Values("%bool"), ValuesIn(scopes), Values(""),
                                 Values("")));

INSTANTIATE_TEST_SUITE_P(GroupNonUniformVote, GroupNonUniform,
                         Combine(Values("OpGroupNonUniformAll",
                                        "OpGroupNonUniformAny",
                                        "OpGroupNonUniformAllEqual"),
                                 Values("%bool"), ValuesIn(scopes),
                                 Values("%true"), Values("")));

INSTANTIATE_TEST_SUITE_P(GroupNonUniformBroadcast, GroupNonUniform,
                         Combine(Values("OpGroupNonUniformBroadcast"),
                                 Values("%bool"), ValuesIn(scopes),
                                 Values("%true %u32_0"), Values("")));

INSTANTIATE_TEST_SUITE_P(GroupNonUniformBroadcastFirst, GroupNonUniform,
                         Combine(Values("OpGroupNonUniformBroadcastFirst"),
                                 Values("%bool"), ValuesIn(scopes),
                                 Values("%true"), Values("")));

INSTANTIATE_TEST_SUITE_P(GroupNonUniformBallot, GroupNonUniform,
                         Combine(Values("OpGroupNonUniformBallot"),
                                 Values("%u32vec4"), ValuesIn(scopes),
                                 Values("%true"), Values("")));

INSTANTIATE_TEST_SUITE_P(GroupNonUniformInverseBallot, GroupNonUniform,
                         Combine(Values("OpGroupNonUniformInverseBallot"),
                                 Values("%bool"), ValuesIn(scopes),
                                 Values("%u32vec4_null"), Values("")));

INSTANTIATE_TEST_SUITE_P(GroupNonUniformBallotBitExtract, GroupNonUniform,
                         Combine(Values("OpGroupNonUniformBallotBitExtract"),
                                 Values("%bool"), ValuesIn(scopes),
                                 Values("%u32vec4_null %u32_0"), Values("")));

INSTANTIATE_TEST_SUITE_P(GroupNonUniformBallotBitCount, GroupNonUniform,
                         Combine(Values("OpGroupNonUniformBallotBitCount"),
                                 Values("%u32"), ValuesIn(scopes),
                                 Values("Reduce %u32vec4_null"), Values("")));

INSTANTIATE_TEST_SUITE_P(GroupNonUniformBallotFind, GroupNonUniform,
                         Combine(Values("OpGroupNonUniformBallotFindLSB",
                                        "OpGroupNonUniformBallotFindMSB"),
                                 Values("%u32"), ValuesIn(scopes),
                                 Values("%u32vec4_null"), Values("")));

INSTANTIATE_TEST_SUITE_P(GroupNonUniformShuffle, GroupNonUniform,
                         Combine(Values("OpGroupNonUniformShuffle",
                                        "OpGroupNonUniformShuffleXor",
                                        "OpGroupNonUniformShuffleUp",
                                        "OpGroupNonUniformShuffleDown"),
                                 Values("%u32"), ValuesIn(scopes),
                                 Values("%u32_0 %u32_0"), Values("")));

INSTANTIATE_TEST_SUITE_P(
    GroupNonUniformIntegerArithmetic, GroupNonUniform,
    Combine(Values("OpGroupNonUniformIAdd", "OpGroupNonUniformIMul",
                   "OpGroupNonUniformSMin", "OpGroupNonUniformUMin",
                   "OpGroupNonUniformSMax", "OpGroupNonUniformUMax",
                   "OpGroupNonUniformBitwiseAnd", "OpGroupNonUniformBitwiseOr",
                   "OpGroupNonUniformBitwiseXor"),
            Values("%u32"), ValuesIn(scopes), Values("Reduce %u32_0"),
            Values("")));

INSTANTIATE_TEST_SUITE_P(
    GroupNonUniformFloatArithmetic, GroupNonUniform,
    Combine(Values("OpGroupNonUniformFAdd", "OpGroupNonUniformFMul",
                   "OpGroupNonUniformFMin", "OpGroupNonUniformFMax"),
            Values("%float"), ValuesIn(scopes), Values("Reduce %float_0"),
            Values("")));

INSTANTIATE_TEST_SUITE_P(GroupNonUniformLogicalArithmetic, GroupNonUniform,
                         Combine(Values("OpGroupNonUniformLogicalAnd",
                                        "OpGroupNonUniformLogicalOr",
                                        "OpGroupNonUniformLogicalXor"),
                                 Values("%bool"), ValuesIn(scopes),
                                 Values("Reduce %true"), Values("")));

INSTANTIATE_TEST_SUITE_P(GroupNonUniformQuad, GroupNonUniform,
                         Combine(Values("OpGroupNonUniformQuadBroadcast",
                                        "OpGroupNonUniformQuadSwap"),
                                 Values("%u32"), ValuesIn(scopes),
                                 Values("%u32_0 %u32_0"), Values("")));

INSTANTIATE_TEST_SUITE_P(GroupNonUniformBallotBitCountScope, GroupNonUniform,
                         Combine(Values("OpGroupNonUniformBallotBitCount"),
                                 Values("%u32"), ValuesIn(scopes),
                                 Values("Reduce %u32vec4_null"), Values("")));

INSTANTIATE_TEST_SUITE_P(
    GroupNonUniformBallotBitCountBadResultType, GroupNonUniform,
    Combine(
        Values("OpGroupNonUniformBallotBitCount"), Values("%float", "%int"),
        Values(spv::Scope::Subgroup), Values("Reduce %u32vec4_null"),
        Values("Expected Result Type to be an unsigned integer type scalar.")));

INSTANTIATE_TEST_SUITE_P(GroupNonUniformBallotBitCountBadValue, GroupNonUniform,
                         Combine(Values("OpGroupNonUniformBallotBitCount"),
                                 Values("%u32"), Values(spv::Scope::Subgroup),
                                 Values("Reduce %u32vec3_null", "Reduce %u32_0",
                                        "Reduce %float_0"),
                                 Values("Expected Value to be a vector of four "
                                        "components of integer type scalar")));

INSTANTIATE_TEST_SUITE_P(GroupNonUniformElectGood, GroupNonUniform,
                         Combine(Values("OpGroupNonUniformElect"),
                                 Values("%bool"), Values(spv::Scope::Subgroup),
                                 Values(""), Values("")));

INSTANTIATE_TEST_SUITE_P(
    GroupNonUniformElectBadResultType, GroupNonUniform,
    Combine(Values("OpGroupNonUniformElect"),
            Values("%void", "%u32", "%int", "%float", "%u32vec4", "%u32vec3",
                   "%v2bool", "%v4float", "%struct"),
            Values(spv::Scope::Subgroup), Values(""),
            Values("Result must be a boolean scalar type")));

INSTANTIATE_TEST_SUITE_P(GroupNonUniformAnyAllGood, GroupNonUniform,
                         Combine(Values("OpGroupNonUniformAny",
                                        "OpGroupNonUniformAll"),
                                 Values("%bool"), Values(spv::Scope::Subgroup),
                                 Values("%true", "%false"), Values("")));

INSTANTIATE_TEST_SUITE_P(
    GroupNonUniformAnyAllBadResultType, GroupNonUniform,
    Combine(Values("OpGroupNonUniformAny", "OpGroupNonUniformAll"),
            Values("%void", "%u32", "%int", "%float", "%u32vec4", "%u32vec3",
                   "%v2bool", "%v4float", "%struct"),
            Values(spv::Scope::Subgroup), Values("%true"),
            Values("Result must be a boolean scalar type")));

INSTANTIATE_TEST_SUITE_P(
    GroupNonUniformAnyAllBadOperand, GroupNonUniform,
    Combine(Values("OpGroupNonUniformAny", "OpGroupNonUniformAll"),
            Values("%bool"), Values(spv::Scope::Subgroup),
            Values("%u32_0", "%int_0", "%float_0", "%u32vec4_null",
                   "%u32vec3_null", "%v2bool_false", "%v4float_null",
                   "%struct_null"),
            Values("Predicate must be a boolean scalar type")));

INSTANTIATE_TEST_SUITE_P(GroupNonUniformAllEqualGood, GroupNonUniform,
                         Combine(Values("OpGroupNonUniformAllEqual"),
                                 Values("%bool"), Values(spv::Scope::Subgroup),
                                 Values("%true", "%false"), Values("")));

INSTANTIATE_TEST_SUITE_P(
    GroupNonUniformAllEqualBadResultType, GroupNonUniform,
    Combine(Values("OpGroupNonUniformAllEqual"),
            Values("%void", "%u32", "%int", "%float", "%u32vec4", "%u32vec3",
                   "%v2bool", "%v4float", "%struct"),
            Values(spv::Scope::Subgroup), Values("%true"),
            Values("Result must be a boolean scalar type")));

INSTANTIATE_TEST_SUITE_P(
    GroupNonUniformAllEqualBadOperand, GroupNonUniform,
    Combine(Values("OpGroupNonUniformAllEqual"), Values("%bool"),
            Values(spv::Scope::Subgroup), Values("%struct_null"),
            Values("Value must be a scalar or vector of integer, "
                   "floating-point, or boolean type")));

INSTANTIATE_TEST_SUITE_P(GroupNonUniformBroadcastGood, GroupNonUniform,
                         Combine(Values("OpGroupNonUniformBroadcast",
                                        "OpGroupNonUniformQuadBroadcast",
                                        "OpGroupNonUniformQuadSwap"),
                                 Values("%bool", "%u32", "%int", "%float",
                                        "%u32vec4", "%u32vec3", "%v2bool",
                                        "%v4float", "%v4int"),
                                 Values(spv::Scope::Subgroup),
                                 Values("match_res %u32_0"), Values("")));

INSTANTIATE_TEST_SUITE_P(
    GroupNonUniformBroadcastShuffleBadResultType, GroupNonUniform,
    Combine(Values("OpGroupNonUniformBroadcast", "OpGroupNonUniformShuffle",
                   "OpGroupNonUniformShuffleXor", "OpGroupNonUniformShuffleUp",
                   "OpGroupNonUniformShuffleDown",
                   "OpGroupNonUniformQuadBroadcast",
                   "OpGroupNonUniformQuadSwap"),
            Values("%void", "%struct"), Values(spv::Scope::Subgroup),
            Values("%u32_0 %u32_0"),
            Values("Result must be a scalar or vector of integer, "
                   "floating-point, or boolean type")));

INSTANTIATE_TEST_SUITE_P(
    GroupNonUniformBroadcastShuffleBadOperand1, GroupNonUniform,
    Combine(Values("OpGroupNonUniformBroadcast", "OpGroupNonUniformShuffle",
                   "OpGroupNonUniformShuffleXor", "OpGroupNonUniformShuffleUp",
                   "OpGroupNonUniformShuffleDown",
                   "OpGroupNonUniformQuadBroadcast",
                   "OpGroupNonUniformQuadSwap"),
            Values("%bool"), Values(spv::Scope::Subgroup),
            Values("%u32_0 %u32_0", "%int_0 %u32_0", "%float_0 %u32_0",
                   "%u32vec4_null %u32_0", "%u32vec3_null %u32_0",
                   "%v2bool_false %u32_0", "%v4float_null %u32_0",
                   "%struct_null %u32_0", "%v4int_null %u32_0"),
            Values("The type of Value must match the Result type")));

INSTANTIATE_TEST_SUITE_P(
    GroupNonUniformBroadcastShuffleBadOperand2, GroupNonUniform,
    Combine(Values("OpGroupNonUniformBroadcast", "OpGroupNonUniformShuffle",
                   "OpGroupNonUniformShuffleXor", "OpGroupNonUniformShuffleUp",
                   "OpGroupNonUniformShuffleDown",
                   "OpGroupNonUniformQuadBroadcast",
                   "OpGroupNonUniformQuadSwap"),
            Values("%bool"), Values(spv::Scope::Subgroup),
            Values("%true %true", "%true %int_0", "%true %float_0",
                   "%true %u32vec4_null", "%true %u32vec3_null",
                   "%true %v4float_null", "%true %v2bool_false",
                   "%true %struct_null", "%true %v4int_null"),
            Values("must be an unsigned integer scalar")));

INSTANTIATE_TEST_SUITE_P(GroupNonUniformBroadcastShuffleOperand2NotConstant,
                         GroupNonUniform,
                         Combine(Values("OpGroupNonUniformBroadcast",
                                        "OpGroupNonUniformQuadBroadcast",
                                        "OpGroupNonUniformQuadSwap"),
                                 Values("%bool"), Values(spv::Scope::Subgroup),
                                 Values("%true %u32_undef"),
                                 Values("must be a constant instruction")));

INSTANTIATE_TEST_SUITE_P(GroupNonUniformBroadcastFirstGood, GroupNonUniform,
                         Combine(Values("OpGroupNonUniformBroadcastFirst"),
                                 Values("%bool", "%u32", "%int", "%float",
                                        "%u32vec4", "%u32vec3", "%v2bool",
                                        "%v4float", "%v4int"),
                                 Values(spv::Scope::Subgroup),
                                 Values("match_res"), Values("")));

INSTANTIATE_TEST_SUITE_P(
    GroupNonUniformBroadcasFirsttBadResultType, GroupNonUniform,
    Combine(Values("OpGroupNonUniformBroadcastFirst"),
            Values("%void", "%struct"), Values(spv::Scope::Subgroup),
            Values("%u32_0"),
            Values("Result must be a scalar or vector of integer, "
                   "floating-point, or boolean type")));

INSTANTIATE_TEST_SUITE_P(
    GroupNonUniformBroadcastBadOperand, GroupNonUniform,
    Combine(Values("OpGroupNonUniformBroadcastFirst"), Values("%bool"),
            Values(spv::Scope::Subgroup),
            Values("%u32_0", "%int_0", "%float_0", "%u32vec4_null",
                   "%u32vec3_null", "%v2bool_false", "%v4float_null",
                   "%struct_null", "%v4int_null"),
            Values("The type of Value must match the Result type")));

INSTANTIATE_TEST_SUITE_P(GroupNonUniformBallotGood, GroupNonUniform,
                         Combine(Values("OpGroupNonUniformBallot"),
                                 Values("%u32vec4"),
                                 Values(spv::Scope::Subgroup),
                                 Values("%true", "%false"), Values("")));

INSTANTIATE_TEST_SUITE_P(
    GroupNonUniformBallotBadResultType, GroupNonUniform,
    Combine(Values("OpGroupNonUniformBallot"),
            Values("%void", "%bool", "%u32", "%int", "%float", "%u32vec3",
                   "%v2bool", "%v4float", "%struct", "%v4int"),
            Values(spv::Scope::Subgroup), Values("%true", "%false"),
            Values("Result must be a 4-component unsigned integer vector")));

INSTANTIATE_TEST_SUITE_P(GroupNonUniformBallotBadOperand, GroupNonUniform,
                         Combine(Values("OpGroupNonUniformBallot"),
                                 Values("%u32vec4"),
                                 Values(spv::Scope::Subgroup),
                                 Values("%u32_0", "%int_0", "%float_0",
                                        "%u32vec4_null", "%u32vec3_null",
                                        "%v2bool_false", "%v4float_null",
                                        "%struct_null", "%v4int_null"),
                                 Values("Predicate must be a boolean scalar")));

INSTANTIATE_TEST_SUITE_P(GroupNonUniformInverseBallotGood, GroupNonUniform,
                         Combine(Values("OpGroupNonUniformInverseBallot"),
                                 Values("%bool"), Values(spv::Scope::Subgroup),
                                 Values("%u32vec4_null"), Values("")));

INSTANTIATE_TEST_SUITE_P(
    GroupNonUniformInverseBallotBadResultType, GroupNonUniform,
    Combine(Values("OpGroupNonUniformInverseBallot"),
            Values("%void", "%u32", "%int", "%float", "%u32vec4", "%u32vec3",
                   "%v2bool", "%v4float", "%struct", "%v4int"),
            Values(spv::Scope::Subgroup), Values("%u32vec4_null"),
            Values("Result must be a boolean scalar")));

INSTANTIATE_TEST_SUITE_P(
    GroupNonUniformInverseBallotBadOperand, GroupNonUniform,
    Combine(Values("OpGroupNonUniformInverseBallot"), Values("%bool"),
            Values(spv::Scope::Subgroup),
            Values("%true", "%false", "%u32_0", "%int_0", "%float_0",
                   "%u32vec3_null", "%v2bool_false", "%v4float_null",
                   "%struct_null", "%v4int_null"),
            Values("Value must be a 4-component unsigned integer vector")));

INSTANTIATE_TEST_SUITE_P(GroupNonUniformBallotBitExtractGood, GroupNonUniform,
                         Combine(Values("OpGroupNonUniformBallotBitExtract"),
                                 Values("%bool"), Values(spv::Scope::Subgroup),
                                 Values("%u32vec4_null %u32_0"), Values("")));

INSTANTIATE_TEST_SUITE_P(
    GroupNonUniformBallotBitExtractBadResultType, GroupNonUniform,
    Combine(Values("OpGroupNonUniformBallotBitExtract"),
            Values("%void", "%u32", "%int", "%float", "%u32vec4", "%u32vec3",
                   "%v2bool", "%v4float", "%struct", "%v4int"),
            Values(spv::Scope::Subgroup), Values("%u32vec4_null %u32_0"),
            Values("Result must be a boolean scalar")));

INSTANTIATE_TEST_SUITE_P(
    GroupNonUniformBallotBitExtractBadOperand1, GroupNonUniform,
    Combine(Values("OpGroupNonUniformBallotBitExtract"), Values("%bool"),
            Values(spv::Scope::Subgroup),
            Values("%true %u32_0", "%false %u32_0", "%u32_0 %u32_0",
                   "%int_0 %u32_0", "%float_0 %u32_0", "%u32vec3_null %u32_0",
                   "%v2bool_false %u32_0", "%v4float_null %u32_0",
                   "%struct_null %u32_0", "%v4int_null %u32_0"),
            Values("Value must be a 4-component unsigned integer vector")));

INSTANTIATE_TEST_SUITE_P(
    GroupNonUniformBallotBitExtractBadOperand2, GroupNonUniform,
    Combine(Values("OpGroupNonUniformBallotBitExtract"), Values("%bool"),
            Values(spv::Scope::Subgroup),
            Values("%u32vec4_null %true", "%u32vec4_null %false",
                   "%u32vec4_null %int_0", "%u32vec4_null %float_0",
                   "%u32vec4_null %u32vec3_null", "%u32vec4_null %v2bool_false",
                   "%u32vec4_null %v4float_null", "%u32vec4_null %struct_null",
                   "%u32vec4_null %v4int_null"),
            Values("Id must be an unsigned integer scalar")));

INSTANTIATE_TEST_SUITE_P(GroupNonUniformBallotFindGood, GroupNonUniform,
                         Combine(Values("OpGroupNonUniformBallotFindLSB",
                                        "OpGroupNonUniformBallotFindMSB"),
                                 Values("%u32"), Values(spv::Scope::Subgroup),
                                 Values("%u32vec4_null"), Values("")));

INSTANTIATE_TEST_SUITE_P(
    GroupNonUniformBallotFindBadResultType, GroupNonUniform,
    Combine(Values("OpGroupNonUniformBallotFindLSB",
                   "OpGroupNonUniformBallotFindMSB"),
            Values("%void", "%bool", "%int", "%float", "%u32vec4", "%u32vec3",
                   "%v2bool", "%v4float", "%struct", "%v4int"),
            Values(spv::Scope::Subgroup), Values("%u32vec4_null"),
            Values("Result must be an unsigned integer scalar")));

INSTANTIATE_TEST_SUITE_P(
    GroupNonUniformBallotFindBadOperand, GroupNonUniform,
    Combine(Values("OpGroupNonUniformBallotFindLSB",
                   "OpGroupNonUniformBallotFindMSB"),
            Values("%u32"), Values(spv::Scope::Subgroup),
            Values("%true", "%false", "%u32_0", "%int_0", "%float_0",
                   "%u32vec3_null", "%v2bool_false", "%v4float_null",
                   "%struct_null", "%v4int_null"),
            Values("Value must be a 4-component unsigned integer vector")));

INSTANTIATE_TEST_SUITE_P(
    GroupNonUniformIntegerArithmeticGood, GroupNonUniform,
    Combine(Values("OpGroupNonUniformIAdd", "OpGroupNonUniformIMul",
                   "OpGroupNonUniformSMin", "OpGroupNonUniformSMax",
                   "OpGroupNonUniformBitwiseAnd", "OpGroupNonUniformBitwiseOr",
                   "OpGroupNonUniformBitwiseXor"),
            Values("%u32", "%int", "%u32vec4", "%u32vec3", "%v4int"),
            Values(spv::Scope::Subgroup),
            Values("Reduce match_res", "InclusiveScan match_res",
                   "ExclusiveScan match_res",
                   "ClusteredReduce match_res %u32_0",
                   "PartitionedReduceNV match_res %u32vec4_null",
                   "PartitionedInclusiveScanNV match_res %u32vec4_null",
                   "PartitionedExclusiveScanNV match_res %v4int_null"),
            Values("")));

INSTANTIATE_TEST_SUITE_P(
    GroupNonUniformIntegerArithmeticBadResultType, GroupNonUniform,
    Combine(Values("OpGroupNonUniformIAdd", "OpGroupNonUniformIMul",
                   "OpGroupNonUniformSMin", "OpGroupNonUniformSMax",
                   "OpGroupNonUniformBitwiseAnd", "OpGroupNonUniformBitwiseOr",
                   "OpGroupNonUniformBitwiseXor"),
            Values("%bool", "%float", "%v4float", "%struct"),
            Values(spv::Scope::Subgroup),
            Values("Reduce match_res", "InclusiveScan match_res",
                   "ExclusiveScan match_res",
                   "ClusteredReduce match_res %u32_0"),
            Values("Result must be an integer scalar or vector")));

INSTANTIATE_TEST_SUITE_P(
    GroupNonUniformIntegerArithmeticBadValue, GroupNonUniform,
    Combine(Values("OpGroupNonUniformIAdd", "OpGroupNonUniformIMul",
                   "OpGroupNonUniformSMin", "OpGroupNonUniformSMax",
                   "OpGroupNonUniformBitwiseAnd", "OpGroupNonUniformBitwiseOr",
                   "OpGroupNonUniformBitwiseXor"),
            Values("%int", "%u32vec4", "%u32vec3", "%v4int"),
            Values(spv::Scope::Subgroup),
            Values("Reduce %u32_0", "InclusiveScan %u32_0",
                   "ExclusiveScan %u32_0", "ClusteredReduce %u32_0 %u32_0"),
            Values("The type of Value must match the Result type")));

INSTANTIATE_TEST_SUITE_P(
    GroupNonUniformIntegerArithmeticMissingClusterSize, GroupNonUniform,
    Combine(
        Values("OpGroupNonUniformIAdd", "OpGroupNonUniformIMul",
               "OpGroupNonUniformSMin", "OpGroupNonUniformUMin",
               "OpGroupNonUniformSMax", "OpGroupNonUniformUMax",
               "OpGroupNonUniformBitwiseAnd", "OpGroupNonUniformBitwiseOr",
               "OpGroupNonUniformBitwiseXor"),
        Values("%u32"), Values(spv::Scope::Subgroup),
        Values("ClusteredReduce match_res"),
        Values(
            "ClusterSize must be present when Operation is ClusteredReduce")));

INSTANTIATE_TEST_SUITE_P(
    GroupNonUniformIntegerArithmeticMissingBallot, GroupNonUniform,
    Combine(
        Values("OpGroupNonUniformIAdd", "OpGroupNonUniformIMul",
               "OpGroupNonUniformSMin", "OpGroupNonUniformUMin",
               "OpGroupNonUniformSMax", "OpGroupNonUniformUMax",
               "OpGroupNonUniformBitwiseAnd", "OpGroupNonUniformBitwiseOr",
               "OpGroupNonUniformBitwiseXor"),
        Values("%u32"), Values(spv::Scope::Subgroup),
        Values("PartitionedReduceNV match_res",
               "PartitionedInclusiveScanNV match_res",
               "PartitionedExclusiveScanNV match_res"),
        Values("Ballot must be present when Operation is PartitionedReduceNV, "
               "PartitionedInclusiveScanNV, or PartitionedExclusiveScanNV")));

INSTANTIATE_TEST_SUITE_P(
    GroupNonUniformIntegerArithmeticBadClusterSizeType, GroupNonUniform,
    Combine(Values("OpGroupNonUniformIAdd", "OpGroupNonUniformIMul",
                   "OpGroupNonUniformSMin", "OpGroupNonUniformUMin",
                   "OpGroupNonUniformSMax", "OpGroupNonUniformUMax",
                   "OpGroupNonUniformBitwiseAnd", "OpGroupNonUniformBitwiseOr",
                   "OpGroupNonUniformBitwiseXor"),
            Values("%u32"), Values(spv::Scope::Subgroup),
            Values("ClusteredReduce match_res %true",
                   "ClusteredReduce match_res %false",
                   "ClusteredReduce match_res %int_0",
                   "ClusteredReduce match_res %float_0",
                   "ClusteredReduce match_res %u32vec4_null",
                   "ClusteredReduce match_res %u32vec3_null",
                   "ClusteredReduce match_res %v2bool_false",
                   "ClusteredReduce match_res %v4float_null",
                   "ClusteredReduce match_res %struct_null",
                   "ClusteredReduce match_res %v4int_null"),
            Values("ClusterSize must be an unsigned integer scalar")));

INSTANTIATE_TEST_SUITE_P(
    GroupNonUniformIntegerArithmeticBadBallotType, GroupNonUniform,
    Combine(Values("OpGroupNonUniformIAdd", "OpGroupNonUniformIMul",
                   "OpGroupNonUniformSMin", "OpGroupNonUniformUMin",
                   "OpGroupNonUniformSMax", "OpGroupNonUniformUMax",
                   "OpGroupNonUniformBitwiseAnd", "OpGroupNonUniformBitwiseOr",
                   "OpGroupNonUniformBitwiseXor"),
            Values("%u32"), Values(spv::Scope::Subgroup),
            Values("PartitionedReduceNV match_res %true",
                   "PartitionedReduceNV match_res %false",
                   "PartitionedReduceNV match_res %int_0",
                   "PartitionedReduceNV match_res %float_0",
                   "PartitionedReduceNV match_res %u32_0",
                   "PartitionedReduceNV match_res %u32vec3_null",
                   "PartitionedReduceNV match_res %v2bool_false",
                   "PartitionedReduceNV match_res %v4float_null",
                   "PartitionedReduceNV match_res %struct_null"),
            Values("Ballot must be a 4-component integer vector")));

INSTANTIATE_TEST_SUITE_P(
    GroupNonUniformIntegerArithmeticClusterSizeNotConstant, GroupNonUniform,
    Combine(Values("OpGroupNonUniformIAdd", "OpGroupNonUniformIMul",
                   "OpGroupNonUniformSMin", "OpGroupNonUniformUMin",
                   "OpGroupNonUniformSMax", "OpGroupNonUniformUMax",
                   "OpGroupNonUniformBitwiseAnd", "OpGroupNonUniformBitwiseOr",
                   "OpGroupNonUniformBitwiseXor"),
            Values("%u32"), Values(spv::Scope::Subgroup),
            Values("ClusteredReduce match_res %u32_undef"),
            Values("ClusterSize must be a constant instruction")));

INSTANTIATE_TEST_SUITE_P(
    GroupNonUniformUnsignedIntegerArithmeticGood, GroupNonUniform,
    Combine(Values("OpGroupNonUniformUMin", "OpGroupNonUniformUMax"),
            Values("%u32", "%u32vec4", "%u32vec3"),
            Values(spv::Scope::Subgroup),
            Values("Reduce match_res", "InclusiveScan match_res",
                   "ExclusiveScan match_res",
                   "ClusteredReduce match_res %u32_0"),
            Values("")));

INSTANTIATE_TEST_SUITE_P(
    GroupNonUniformUnsignedIntegerArithmeticBadResultType, GroupNonUniform,
    Combine(Values("OpGroupNonUniformUMin", "OpGroupNonUniformUMax"),
            Values("%bool", "%int", "%float", "%v4float", "%struct", "%v4int"),
            Values(spv::Scope::Subgroup),
            Values("Reduce match_res", "InclusiveScan match_res",
                   "ExclusiveScan match_res",
                   "ClusteredReduce match_res %u32_0"),
            Values("Result must be an unsigned integer scalar or vector")));

INSTANTIATE_TEST_SUITE_P(
    GroupNonUniformUnsignedIntegerArithmeticBadValue, GroupNonUniform,
    Combine(Values("OpGroupNonUniformUMin", "OpGroupNonUniformUMax"),
            Values("%u32vec4", "%u32vec3"), Values(spv::Scope::Subgroup),
            Values("Reduce %u32_0", "InclusiveScan %u32_0",
                   "ExclusiveScan %u32_0", "ClusteredReduce %u32_0 %u32_0"),
            Values("The type of Value must match the Result type")));

INSTANTIATE_TEST_SUITE_P(
    GroupNonUniformFloatArithmeticGood, GroupNonUniform,
    Combine(Values("OpGroupNonUniformFAdd", "OpGroupNonUniformFMul",
                   "OpGroupNonUniformFMin", "OpGroupNonUniformFMax"),
            Values("%float", "%v4float"), Values(spv::Scope::Subgroup),
            Values("Reduce match_res", "InclusiveScan match_res",
                   "ExclusiveScan match_res",
                   "ClusteredReduce match_res %u32_0"),
            Values("")));

INSTANTIATE_TEST_SUITE_P(
    GroupNonUniformFloatArithmeticBadResultType, GroupNonUniform,
    Combine(Values("OpGroupNonUniformFAdd", "OpGroupNonUniformFMul",
                   "OpGroupNonUniformFMin", "OpGroupNonUniformFMax"),
            Values("%bool", "%u32", "%int", "%u32vec4", "%u32vec3", "%struct",
                   "%v4int"),
            Values(spv::Scope::Subgroup),
            Values("Reduce match_res", "InclusiveScan match_res",
                   "ExclusiveScan match_res",
                   "ClusteredReduce match_res %u32_0"),
            Values("Result must be a floating-point scalar or vector")));

INSTANTIATE_TEST_SUITE_P(
    GroupNonUniformFloatArithmeticBadValue, GroupNonUniform,
    Combine(Values("OpGroupNonUniformFAdd", "OpGroupNonUniformFMul",
                   "OpGroupNonUniformFMin", "OpGroupNonUniformFMax"),
            Values("%v4float"), Values(spv::Scope::Subgroup),
            Values("Reduce %float_0", "InclusiveScan %float_0",
                   "ExclusiveScan %float_0", "ClusteredReduce %float_0 %u32_0"),
            Values("The type of Value must match the Result type")));

INSTANTIATE_TEST_SUITE_P(
    GroupNonUniformFloatArithmeticMissingClusterSize, GroupNonUniform,
    Combine(
        Values("OpGroupNonUniformFAdd", "OpGroupNonUniformFMul",
               "OpGroupNonUniformFMin", "OpGroupNonUniformFMax"),
        Values("%float"), Values(spv::Scope::Subgroup),
        Values("ClusteredReduce match_res"),
        Values(
            "ClusterSize must be present when Operation is ClusteredReduce")));

INSTANTIATE_TEST_SUITE_P(
    GroupNonUniformFloatArithmeticBadClusterSizeType, GroupNonUniform,
    Combine(Values("OpGroupNonUniformFAdd", "OpGroupNonUniformFMul",
                   "OpGroupNonUniformFMin", "OpGroupNonUniformFMax"),
            Values("%float"), Values(spv::Scope::Subgroup),
            Values("ClusteredReduce match_res %true",
                   "ClusteredReduce match_res %false",
                   "ClusteredReduce match_res %int_0",
                   "ClusteredReduce match_res %float_0",
                   "ClusteredReduce match_res %u32vec4_null",
                   "ClusteredReduce match_res %u32vec3_null",
                   "ClusteredReduce match_res %v2bool_false",
                   "ClusteredReduce match_res %v4float_null",
                   "ClusteredReduce match_res %struct_null",
                   "ClusteredReduce match_res %v4int_null"),
            Values("ClusterSize must be an unsigned integer scalar")));

INSTANTIATE_TEST_SUITE_P(
    GroupNonUniformFloatArithmeticClusterSizeNotConstant, GroupNonUniform,
    Combine(Values("OpGroupNonUniformFAdd", "OpGroupNonUniformFMul",
                   "OpGroupNonUniformFMin", "OpGroupNonUniformFMax"),
            Values("%float"), Values(spv::Scope::Subgroup),
            Values("ClusteredReduce match_res %u32_undef"),
            Values("ClusterSize must be a constant instruction")));

INSTANTIATE_TEST_SUITE_P(
    GroupNonUniformBooleanArithmeticGood, GroupNonUniform,
    Combine(Values("OpGroupNonUniformLogicalAnd", "OpGroupNonUniformLogicalOr",
                   "OpGroupNonUniformLogicalXor"),
            Values("%bool", "%v2bool"), Values(spv::Scope::Subgroup),
            Values("Reduce match_res", "InclusiveScan match_res",
                   "ExclusiveScan match_res",
                   "ClusteredReduce match_res %u32_0"),
            Values("")));

INSTANTIATE_TEST_SUITE_P(
    GroupNonUniformBooleanArithmeticBadResultType, GroupNonUniform,
    Combine(Values("OpGroupNonUniformLogicalAnd", "OpGroupNonUniformLogicalOr",
                   "OpGroupNonUniformLogicalXor"),
            Values("%u32", "%int", "%float", "%u32vec4", "%u32vec3", "%struct",
                   "%v4float", "%v4int"),
            Values(spv::Scope::Subgroup),
            Values("Reduce match_res", "InclusiveScan match_res",
                   "ExclusiveScan match_res",
                   "ClusteredReduce match_res %u32_0"),
            Values("Result must be a boolean scalar or vector")));

INSTANTIATE_TEST_SUITE_P(
    GroupNonUniformBooleanArithmeticBadValue, GroupNonUniform,
    Combine(Values("OpGroupNonUniformLogicalAnd", "OpGroupNonUniformLogicalOr",
                   "OpGroupNonUniformLogicalXor"),
            Values("%v2bool"), Values(spv::Scope::Subgroup),
            Values("Reduce %true", "InclusiveScan %true",
                   "ExclusiveScan %false", "ClusteredReduce %false %u32_0"),
            Values("The type of Value must match the Result type")));

INSTANTIATE_TEST_SUITE_P(
    GroupNonUniformBooleanArithmeticMissingClusterSize, GroupNonUniform,
    Combine(
        Values("OpGroupNonUniformLogicalAnd", "OpGroupNonUniformLogicalOr",
               "OpGroupNonUniformLogicalXor"),
        Values("%bool"), Values(spv::Scope::Subgroup),
        Values("ClusteredReduce match_res"),
        Values(
            "ClusterSize must be present when Operation is ClusteredReduce")));

INSTANTIATE_TEST_SUITE_P(
    GroupNonUniformBooleanArithmeticBadClusterSizeType, GroupNonUniform,
    Combine(Values("OpGroupNonUniformLogicalAnd", "OpGroupNonUniformLogicalOr",
                   "OpGroupNonUniformLogicalXor"),
            Values("%bool"), Values(spv::Scope::Subgroup),
            Values("ClusteredReduce match_res %true",
                   "ClusteredReduce match_res %false",
                   "ClusteredReduce match_res %int_0",
                   "ClusteredReduce match_res %float_0",
                   "ClusteredReduce match_res %u32vec4_null",
                   "ClusteredReduce match_res %u32vec3_null",
                   "ClusteredReduce match_res %v2bool_false",
                   "ClusteredReduce match_res %v4float_null",
                   "ClusteredReduce match_res %struct_null",
                   "ClusteredReduce match_res %v4int_null"),
            Values("ClusterSize must be an unsigned integer scalar")));

INSTANTIATE_TEST_SUITE_P(
    GroupNonUniformBooleanArithmeticClusterSizeNotConstant, GroupNonUniform,
    Combine(Values("OpGroupNonUniformLogicalAnd", "OpGroupNonUniformLogicalOr",
                   "OpGroupNonUniformLogicalXor"),
            Values("%bool"), Values(spv::Scope::Subgroup),
            Values("ClusteredReduce match_res %u32_undef"),
            Values("ClusterSize must be a constant instruction")));

// Subgroup scope is not actual parameter, but used for test expectations,
INSTANTIATE_TEST_SUITE_P(GroupNonUniformQuadAllKHR, GroupNonUniform,
                         Combine(Values("OpGroupNonUniformQuadAllKHR"),
                                 Values("%bool"), Values(spv::Scope::Subgroup),
                                 Values("%true"), Values("")));

// Subgroup scope is not actual parameter, but used for test expectations,
INSTANTIATE_TEST_SUITE_P(GroupNonUniformQuadAnyKHR, GroupNonUniform,
                         Combine(Values("OpGroupNonUniformQuadAnyKHR"),
                                 Values("%bool"), Values(spv::Scope::Subgroup),
                                 Values("%true"), Values("")));

TEST_F(ValidateGroupNonUniform, VulkanGroupNonUniformBallotBitCountOperation) {
  std::string test = R"(
OpCapability Shader
OpCapability GroupNonUniform
OpCapability GroupNonUniformBallot
OpCapability GroupNonUniformClustered
OpMemoryModel Logical GLSL450
OpEntryPoint GLCompute %main "main"
OpExecutionMode %main LocalSize 1 1 1
%void = OpTypeVoid
%func = OpTypeFunction %void
%u32 = OpTypeInt 32 0
%u32vec4 = OpTypeVector %u32 4
%u32_0 = OpConstant %u32 0
%u32vec4_null = OpConstantComposite %u32vec4 %u32_0 %u32_0 %u32_0 %u32_0
%subgroup = OpConstant %u32 3
%main = OpFunction %void None %func
%main_entry = OpLabel
%result = OpGroupNonUniformBallotBitCount %u32 %subgroup ClusteredReduce %u32vec4_null
OpReturn
OpFunctionEnd
)";

  CompileSuccessfully(test, SPV_ENV_VULKAN_1_1);
  ASSERT_EQ(SPV_ERROR_INVALID_DATA, ValidateInstructions(SPV_ENV_VULKAN_1_1));
  EXPECT_THAT(
      getDiagnosticString(),
      AnyVUID("VUID-StandaloneSpirv-OpGroupNonUniformBallotBitCount-04685"));
  EXPECT_THAT(
      getDiagnosticString(),
      HasSubstr(
          "In Vulkan: The OpGroupNonUniformBallotBitCount group operation must "
          "be only: Reduce, InclusiveScan, or ExclusiveScan."));
}

TEST_F(ValidateGroupNonUniform, BroadcastNonConstantSpv1p4) {
  const std::string text = R"(
OpCapability Shader
OpCapability GroupNonUniformBallot
OpMemoryModel Logical GLSL450
OpEntryPoint GLCompute %main "main" %var
OpExecutionMode %main LocalSize 1 1 1
OpDecorate %var DescriptorSet 0
OpDecorate %var Binding 0
OpDecorate %struct Block
OpMemberDecorate %struct 0 Offset 0
%void = OpTypeVoid
%int = OpTypeInt 32 0
%int_0 = OpConstant %int 0
%subgroup = OpConstant %int 3
%struct = OpTypeStruct %int
%ptr_struct = OpTypePointer StorageBuffer %struct
%ptr_int = OpTypePointer StorageBuffer %int
%var = OpVariable %ptr_struct StorageBuffer
%void_fn = OpTypeFunction %void
%main = OpFunction %void None %void_fn
%entry = OpLabel
%gep = OpAccessChain %ptr_int %var %int_0
%ld = OpLoad %int %gep
%broadcast = OpGroupNonUniformBroadcast %int %subgroup %int_0 %ld
OpReturn
OpFunctionEnd
)";

  CompileSuccessfully(text, SPV_ENV_UNIVERSAL_1_4);
  EXPECT_EQ(SPV_ERROR_INVALID_DATA,
            ValidateInstructions(SPV_ENV_UNIVERSAL_1_4));
  EXPECT_THAT(
      getDiagnosticString(),
      HasSubstr("Before SPIR-V 1.5, Id must be a constant instruction"));
}

TEST_F(ValidateGroupNonUniform, BroadcastNonConstantSpv1p5) {
  const std::string text = R"(
OpCapability Shader
OpCapability GroupNonUniformBallot
OpMemoryModel Logical GLSL450
OpEntryPoint GLCompute %main "main" %var
OpExecutionMode %main LocalSize 1 1 1
OpDecorate %var DescriptorSet 0
OpDecorate %var Binding 0
OpDecorate %struct Block
OpMemberDecorate %struct 0 Offset 0
%void = OpTypeVoid
%int = OpTypeInt 32 0
%int_0 = OpConstant %int 0
%subgroup = OpConstant %int 3
%struct = OpTypeStruct %int
%ptr_struct = OpTypePointer StorageBuffer %struct
%ptr_int = OpTypePointer StorageBuffer %int
%var = OpVariable %ptr_struct StorageBuffer
%void_fn = OpTypeFunction %void
%main = OpFunction %void None %void_fn
%entry = OpLabel
%gep = OpAccessChain %ptr_int %var %int_0
%ld = OpLoad %int %gep
%broadcast = OpGroupNonUniformBroadcast %int %subgroup %int_0 %ld
OpReturn
OpFunctionEnd
)";

  CompileSuccessfully(text, SPV_ENV_UNIVERSAL_1_5);
  EXPECT_EQ(SPV_SUCCESS, ValidateInstructions(SPV_ENV_UNIVERSAL_1_5));
}

TEST_F(ValidateGroupNonUniform, QuadBroadcastNonConstantSpv1p4) {
  const std::string text = R"(
OpCapability Shader
OpCapability GroupNonUniformQuad
OpMemoryModel Logical GLSL450
OpEntryPoint GLCompute %main "main" %var
OpExecutionMode %main LocalSize 1 1 1
OpDecorate %var DescriptorSet 0
OpDecorate %var Binding 0
OpDecorate %struct Block
OpMemberDecorate %struct 0 Offset 0
%void = OpTypeVoid
%int = OpTypeInt 32 0
%int_0 = OpConstant %int 0
%subgroup = OpConstant %int 3
%struct = OpTypeStruct %int
%ptr_struct = OpTypePointer StorageBuffer %struct
%ptr_int = OpTypePointer StorageBuffer %int
%var = OpVariable %ptr_struct StorageBuffer
%void_fn = OpTypeFunction %void
%main = OpFunction %void None %void_fn
%entry = OpLabel
%gep = OpAccessChain %ptr_int %var %int_0
%ld = OpLoad %int %gep
%broadcast = OpGroupNonUniformQuadBroadcast %int %subgroup %int_0 %ld
OpReturn
OpFunctionEnd
)";

  CompileSuccessfully(text, SPV_ENV_UNIVERSAL_1_4);
  EXPECT_EQ(SPV_ERROR_INVALID_DATA,
            ValidateInstructions(SPV_ENV_UNIVERSAL_1_4));
  EXPECT_THAT(
      getDiagnosticString(),
      HasSubstr("Before SPIR-V 1.5, Index must be a constant instruction"));
}

TEST_F(ValidateGroupNonUniform, QuadBroadcastNonConstantSpv1p5) {
  const std::string text = R"(
OpCapability Shader
OpCapability GroupNonUniformQuad
OpMemoryModel Logical GLSL450
OpEntryPoint GLCompute %main "main" %var
OpExecutionMode %main LocalSize 1 1 1
OpDecorate %var DescriptorSet 0
OpDecorate %var Binding 0
OpDecorate %struct Block
OpMemberDecorate %struct 0 Offset 0
%void = OpTypeVoid
%int = OpTypeInt 32 0
%int_0 = OpConstant %int 0
%subgroup = OpConstant %int 3
%struct = OpTypeStruct %int
%ptr_struct = OpTypePointer StorageBuffer %struct
%ptr_int = OpTypePointer StorageBuffer %int
%var = OpVariable %ptr_struct StorageBuffer
%void_fn = OpTypeFunction %void
%main = OpFunction %void None %void_fn
%entry = OpLabel
%gep = OpAccessChain %ptr_int %var %int_0
%ld = OpLoad %int %gep
%broadcast = OpGroupNonUniformQuadBroadcast %int %subgroup %int_0 %ld
OpReturn
OpFunctionEnd
)";

  CompileSuccessfully(text, SPV_ENV_UNIVERSAL_1_5);
  EXPECT_EQ(SPV_SUCCESS, ValidateInstructions(SPV_ENV_UNIVERSAL_1_5));
}

}  // namespace
}  // namespace val
}  // namespace spvtools
