// Copyright (c) 2022 The Khronos Group Inc.
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
#include "test/val/val_fixtures.h"

namespace spvtools {
namespace val {
namespace {

using ::testing::HasSubstr;
using ::testing::Values;
using ::testing::ValuesIn;

struct Case {
  std::vector<std::string> caps;
  bool shader;
  std::string result_type;
  std::string scope;
  std::string delta;
  std::string cluster_size;
  std::string expected_error;  // empty for no error.
};

inline std::ostream& operator<<(std::ostream& out, Case c) {
  out << "\nSPV_KHR_subgroup_rotate Case{{";
  for (auto& cap : c.caps) {
    out << cap;
  }
  out << "} ";
  out << (c.shader ? "shader " : "kernel ");
  out << c.result_type + " ";
  out << c.scope + " ";
  out << c.delta + " ";
  out << c.cluster_size + " ";
  out << "err'" << c.expected_error << "'";
  out << "}";
  return out;
}

std::string AssemblyForCase(const Case& c) {
  std::ostringstream ss;

  if (c.shader) {
    ss << "OpCapability Shader\n";
  } else {
    ss << "OpCapability Kernel\n";
    ss << "OpCapability Addresses\n";
  }
  for (auto& cap : c.caps) {
    ss << "OpCapability " << cap << "\n";
  }
  ss << "OpExtension \"SPV_KHR_subgroup_rotate\"\n";

  if (c.shader) {
    ss << "OpMemoryModel Logical GLSL450\n";
    ss << "OpEntryPoint GLCompute %main \"main\"\n";
  } else {
    ss << "OpMemoryModel Physical32 OpenCL\n";
    ss << "OpEntryPoint Kernel %main \"main\"\n";
  }

  ss << R"(
    %void    = OpTypeVoid
    %void_fn = OpTypeFunction %void
    %u32 = OpTypeInt 32 0
    %float = OpTypeFloat 32
    %ptr = OpTypePointer Function %u32
  )";

  if (c.shader) {
    ss << "%i32 = OpTypeInt 32 1\n";
  }

  ss << R"(
    %u32_0 = OpConstant %u32 0
    %u32_1 = OpConstant %u32 1
    %u32_15 = OpConstant %u32 15
    %u32_16 = OpConstant %u32 16
    %u32_undef = OpUndef %u32
    %u32_spec_1 = OpSpecConstant %u32 1
    %u32_spec_16 = OpSpecConstant %u32 16
    %f32_1 = OpConstant %float 1.0
    %subgroup = OpConstant %u32 3
    %workgroup = OpConstant %u32 2
    %invalid_scope = OpConstant %u32 1
    %val = OpConstant %u32 42
  )";

  if (c.shader) {
    ss << "%i32_1 = OpConstant %i32 1\n";
  }

  ss << R"(
    %main = OpFunction %void None %void_fn
    %entry = OpLabel
  )";

  ss << "%unused = OpGroupNonUniformRotateKHR ";
  ss << c.result_type + " ";
  ss << c.scope;
  ss << " %val ";
  ss << c.delta;
  ss << " " + c.cluster_size;
  ss << "\n";

  ss << R"(
    OpReturn
    OpFunctionEnd
  )";

  return ss.str();
}

using ValidateSpvKHRSubgroupRotate = spvtest::ValidateBase<Case>;

TEST_P(ValidateSpvKHRSubgroupRotate, Base) {
  const auto& c = GetParam();
  const auto& assembly = AssemblyForCase(c);
  CompileSuccessfully(assembly);
  if (c.expected_error.empty()) {
    EXPECT_EQ(SPV_SUCCESS, ValidateInstructions()) << getDiagnosticString();
  } else {
    EXPECT_NE(SPV_SUCCESS, ValidateInstructions());
    EXPECT_THAT(getDiagnosticString(), HasSubstr(c.expected_error));
  }
}

INSTANTIATE_TEST_SUITE_P(
    Valid, ValidateSpvKHRSubgroupRotate,
    ::testing::Values(
        Case{
            {"GroupNonUniformRotateKHR"}, false, "%u32", "%subgroup", "%u32_1"},
        Case{{"GroupNonUniformRotateKHR"}, true, "%u32", "%subgroup", "%u32_1"},
        Case{{"GroupNonUniformRotateKHR"},
             false,
             "%u32",
             "%subgroup",
             "%u32_1",
             "%u32_16"},
        Case{{"GroupNonUniformRotateKHR"},
             true,
             "%u32",
             "%subgroup",
             "%u32_1",
             "%u32_16"},
        Case{{"GroupNonUniformRotateKHR"},
             false,
             "%u32",
             "%subgroup",
             "%u32_spec_1",
             "%u32_16"},
        Case{{"GroupNonUniformRotateKHR"},
             true,
             "%u32",
             "%subgroup",
             "%u32_1",
             "%u32_spec_16"},
        Case{{"GroupNonUniformRotateKHR"},
             false,
             "%u32",
             "%workgroup",
             "%u32_1"},
        Case{
            {"GroupNonUniformRotateKHR"}, true, "%u32", "%workgroup", "%u32_1"},
        Case{{"GroupNonUniformRotateKHR"},
             false,
             "%u32",
             "%workgroup",
             "%u32_spec_1"},
        Case{{"GroupNonUniformRotateKHR"},
             true,
             "%u32",
             "%workgroup",
             "%u32_spec_1"}));

INSTANTIATE_TEST_SUITE_P(
    RequiresCapability, ValidateSpvKHRSubgroupRotate,
    ::testing::Values(Case{{},
                           false,
                           "%u32",
                           "%subgroup",
                           "%u32_1",
                           "",
                           "Opcode GroupNonUniformRotateKHR requires one of "
                           "these capabilities: "
                           "GroupNonUniformRotateKHR"},
                      Case{{},
                           true,
                           "%u32",
                           "%subgroup",
                           "%u32_1",
                           "",
                           "Opcode GroupNonUniformRotateKHR requires one of "
                           "these capabilities: "
                           "GroupNonUniformRotateKHR"}));

TEST_F(ValidateSpvKHRSubgroupRotate, RequiresExtension) {
  const std::string str = R"(
    OpCapability GroupNonUniformRotateKHR
)";
  CompileSuccessfully(str.c_str());
  EXPECT_NE(SPV_SUCCESS, ValidateInstructions());
  EXPECT_THAT(
      getDiagnosticString(),
      HasSubstr(
          "1st operand of Capability: operand GroupNonUniformRotateKHR(6026) "
          "requires one of these extensions: SPV_KHR_subgroup_rotate"));
}

INSTANTIATE_TEST_SUITE_P(
    InvalidExecutionScope, ValidateSpvKHRSubgroupRotate,
    ::testing::Values(
        Case{{"GroupNonUniformRotateKHR"},
             false,
             "%u32",
             "%invalid_scope",
             "%u32_1",
             "",
             "Execution scope is limited to Subgroup or Workgroup"},
        Case{{"GroupNonUniformRotateKHR"},
             true,
             "%u32",
             "%invalid_scope",
             "%u32_1",
             "",
             "Execution scope is limited to Subgroup or Workgroup"}));

INSTANTIATE_TEST_SUITE_P(
    InvalidResultType, ValidateSpvKHRSubgroupRotate,
    ::testing::Values(Case{{"GroupNonUniformRotateKHR"},
                           false,
                           "%ptr",
                           "%subgroup",
                           "%u32_1",
                           "",
                           "Expected Result Type to be a scalar or vector of "
                           "floating-point, integer or boolean type"},
                      Case{{"GroupNonUniformRotateKHR"},
                           true,
                           "%ptr",
                           "%subgroup",
                           "%u32_1",
                           "",
                           "Expected Result Type to be a scalar or vector of "
                           "floating-point, integer or boolean type"}));

INSTANTIATE_TEST_SUITE_P(
    MismatchedResultAndValueTypes, ValidateSpvKHRSubgroupRotate,
    ::testing::Values(
        Case{{"GroupNonUniformRotateKHR"},
             false,
             "%float",
             "%subgroup",
             "%u32_1",
             "",
             "Result Type must be the same as the type of Value"},
        Case{{"GroupNonUniformRotateKHR"},
             true,
             "%float",
             "%subgroup",
             "%u32_1",
             "",
             "Result Type must be the same as the type of Value"}));

INSTANTIATE_TEST_SUITE_P(
    InvalidDelta, ValidateSpvKHRSubgroupRotate,
    ::testing::Values(Case{{"GroupNonUniformRotateKHR"},
                           false,
                           "%u32",
                           "%subgroup",
                           "%f32_1",
                           "",
                           "Delta must be a scalar of integer type, whose "
                           "Signedness operand is 0"},
                      Case{{"GroupNonUniformRotateKHR"},
                           true,
                           "%u32",
                           "%subgroup",
                           "%f32_1",
                           "",
                           "Delta must be a scalar of integer type, whose "
                           "Signedness operand is 0"},
                      Case{{"GroupNonUniformRotateKHR"},
                           true,
                           "%u32",
                           "%subgroup",
                           "%i32_1",
                           "",
                           "Delta must be a scalar of integer type, whose "
                           "Signedness operand is 0"}));

INSTANTIATE_TEST_SUITE_P(
    InvalidClusterSize, ValidateSpvKHRSubgroupRotate,
    ::testing::Values(
        Case{{"GroupNonUniformRotateKHR"},
             false,
             "%u32",
             "%subgroup",
             "%u32_1",
             "%f32_1",
             "ClusterSize must be a scalar of integer type, whose Signedness "
             "operand is 0"},
        Case{{"GroupNonUniformRotateKHR"},
             true,
             "%u32",
             "%subgroup",
             "%u32_1",
             "%i32_1",
             "ClusterSize must be a scalar of integer type, whose Signedness "
             "operand is 0"},
        Case{{"GroupNonUniformRotateKHR"},
             true,
             "%u32",
             "%subgroup",
             "%u32_1",
             "%u32_0",
             "Behavior is undefined unless ClusterSize is at least 1 and a "
             "power of 2"},
        Case{{"GroupNonUniformRotateKHR"},
             true,
             "%u32",
             "%subgroup",
             "%u32_1",
             "%u32_15",
             "Behavior is undefined unless ClusterSize is at least 1 and a "
             "power of 2"},
        Case{{"GroupNonUniformRotateKHR"},
             true,
             "%u32",
             "%subgroup",
             "%u32_1",
             "%u32_undef",
             "ClusterSize must come from a constant instruction"}));

}  // namespace
}  // namespace val
}  // namespace spvtools
