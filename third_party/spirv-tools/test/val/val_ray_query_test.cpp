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

// Tests ray query instructions from SPV_KHR_ray_query.

#include <sstream>
#include <string>

#include "gmock/gmock.h"
#include "spirv-tools/libspirv.h"
#include "test/val/val_fixtures.h"

namespace spvtools {
namespace val {
namespace {

using ::testing::HasSubstr;
using ::testing::Values;

using ValidateRayQuery = spvtest::ValidateBase<bool>;

std::string GenerateShaderCode(
    const std::string& body,
    const std::string& capabilities_and_extensions = "",
    const std::string& declarations = "") {
  std::ostringstream ss;
  ss << R"(
OpCapability Shader
OpCapability Int64
OpCapability Float64
OpCapability RayQueryKHR
OpExtension "SPV_KHR_ray_query"
)";

  ss << capabilities_and_extensions;

  ss << R"(
OpMemoryModel Logical GLSL450
OpEntryPoint GLCompute %main "main"
OpExecutionMode %main LocalSize 1 1 1

OpDecorate %top_level_as DescriptorSet 0
OpDecorate %top_level_as Binding 0

%void = OpTypeVoid
%func = OpTypeFunction %void
%bool = OpTypeBool
%f32 = OpTypeFloat 32
%f64 = OpTypeFloat 64
%u32 = OpTypeInt 32 0
%s32 = OpTypeInt 32 1
%u64 = OpTypeInt 64 0
%s64 = OpTypeInt 64 1
%type_rq = OpTypeRayQueryKHR
%type_as = OpTypeAccelerationStructureKHR

%s32vec2 = OpTypeVector %s32 2
%u32vec2 = OpTypeVector %u32 2
%f32vec2 = OpTypeVector %f32 2
%u32vec3 = OpTypeVector %u32 3
%s32vec3 = OpTypeVector %s32 3
%f32vec3 = OpTypeVector %f32 3
%u32vec4 = OpTypeVector %u32 4
%s32vec4 = OpTypeVector %s32 4
%f32vec4 = OpTypeVector %f32 4

%mat4x3 = OpTypeMatrix %f32vec3 4

%f32_0 = OpConstant %f32 0
%f64_0 = OpConstant %f64 0
%s32_0 = OpConstant %s32 0
%u32_0 = OpConstant %u32 0
%u64_0 = OpConstant %u64 0

%u32vec3_0 = OpConstantComposite %u32vec3 %u32_0 %u32_0 %u32_0
%f32vec3_0 = OpConstantComposite %f32vec3 %f32_0 %f32_0 %f32_0
%f32vec4_0 = OpConstantComposite %f32vec4 %f32_0 %f32_0 %f32_0 %f32_0

%ptr_rq = OpTypePointer Private %type_rq
%ray_query = OpVariable %ptr_rq Private

%ptr_as = OpTypePointer UniformConstant %type_as
%top_level_as = OpVariable %ptr_as UniformConstant

%ptr_function_u32 = OpTypePointer Function %u32
%ptr_function_f32 = OpTypePointer Function %f32
%ptr_function_f32vec3 = OpTypePointer Function %f32vec3
)";

  ss << declarations;

  ss << R"(
%main = OpFunction %void None %func
%main_entry = OpLabel
)";

  ss << body;

  ss << R"(
OpReturn
OpFunctionEnd)";
  return ss.str();
}

std::string RayQueryResult(std::string opcode) {
  if (opcode.compare("OpRayQueryProceedKHR") == 0 ||
      opcode.compare("OpRayQueryGetIntersectionTypeKHR") == 0 ||
      opcode.compare("OpRayQueryGetRayTMinKHR") == 0 ||
      opcode.compare("OpRayQueryGetRayFlagsKHR") == 0 ||
      opcode.compare("OpRayQueryGetIntersectionTKHR") == 0 ||
      opcode.compare("OpRayQueryGetIntersectionInstanceCustomIndexKHR") == 0 ||
      opcode.compare("OpRayQueryGetIntersectionInstanceIdKHR") == 0 ||
      opcode.compare("OpRayQueryGetIntersectionInstanceShaderBindingTableRecord"
                     "OffsetKHR") == 0 ||
      opcode.compare("OpRayQueryGetIntersectionGeometryIndexKHR") == 0 ||
      opcode.compare("OpRayQueryGetIntersectionPrimitiveIndexKHR") == 0 ||
      opcode.compare("OpRayQueryGetIntersectionBarycentricsKHR") == 0 ||
      opcode.compare("OpRayQueryGetIntersectionFrontFaceKHR") == 0 ||
      opcode.compare("OpRayQueryGetIntersectionCandidateAABBOpaqueKHR") == 0 ||
      opcode.compare("OpRayQueryGetIntersectionObjectRayDirectionKHR") == 0 ||
      opcode.compare("OpRayQueryGetIntersectionObjectRayOriginKHR") == 0 ||
      opcode.compare("OpRayQueryGetWorldRayDirectionKHR") == 0 ||
      opcode.compare("OpRayQueryGetWorldRayOriginKHR") == 0 ||
      opcode.compare("OpRayQueryGetIntersectionObjectToWorldKHR") == 0 ||
      opcode.compare("OpRayQueryGetIntersectionWorldToObjectKHR") == 0) {
    return "%result =";
  }
  return "";
}

std::string RayQueryResultType(std::string opcode, bool valid) {
  if (opcode.compare("OpRayQueryGetIntersectionTypeKHR") == 0 ||
      opcode.compare("OpRayQueryGetRayFlagsKHR") == 0 ||
      opcode.compare("OpRayQueryGetIntersectionInstanceCustomIndexKHR") == 0 ||
      opcode.compare("OpRayQueryGetIntersectionInstanceIdKHR") == 0 ||
      opcode.compare("OpRayQueryGetIntersectionInstanceShaderBindingTableRecord"
                     "OffsetKHR") == 0 ||
      opcode.compare("OpRayQueryGetIntersectionGeometryIndexKHR") == 0 ||
      opcode.compare("OpRayQueryGetIntersectionPrimitiveIndexKHR") == 0) {
    return valid ? "%u32" : "%f64";
  }

  if (opcode.compare("OpRayQueryGetRayTMinKHR") == 0 ||
      opcode.compare("OpRayQueryGetIntersectionTKHR") == 0) {
    return valid ? "%f32" : "%f64";
  }

  if (opcode.compare("OpRayQueryGetIntersectionBarycentricsKHR") == 0) {
    return valid ? "%f32vec2" : "%f64";
  }

  if (opcode.compare("OpRayQueryGetIntersectionObjectRayDirectionKHR") == 0 ||
      opcode.compare("OpRayQueryGetIntersectionObjectRayOriginKHR") == 0 ||
      opcode.compare("OpRayQueryGetWorldRayDirectionKHR") == 0 ||
      opcode.compare("OpRayQueryGetWorldRayOriginKHR") == 0) {
    return valid ? "%f32vec3" : "%f64";
  }

  if (opcode.compare("OpRayQueryProceedKHR") == 0 ||
      opcode.compare("OpRayQueryGetIntersectionFrontFaceKHR") == 0 ||
      opcode.compare("OpRayQueryGetIntersectionCandidateAABBOpaqueKHR") == 0) {
    return valid ? "%bool" : "%f64";
  }

  if (opcode.compare("OpRayQueryGetIntersectionObjectToWorldKHR") == 0 ||
      opcode.compare("OpRayQueryGetIntersectionWorldToObjectKHR") == 0) {
    return valid ? "%mat4x3" : "%f64";
  }
  return "";
}

std::string RayQueryIntersection(std::string opcode, bool valid) {
  if (opcode.compare("OpRayQueryGetIntersectionTypeKHR") == 0 ||
      opcode.compare("OpRayQueryGetIntersectionTKHR") == 0 ||
      opcode.compare("OpRayQueryGetIntersectionInstanceCustomIndexKHR") == 0 ||
      opcode.compare("OpRayQueryGetIntersectionInstanceIdKHR") == 0 ||
      opcode.compare("OpRayQueryGetIntersectionInstanceShaderBindingTableRecord"
                     "OffsetKHR") == 0 ||
      opcode.compare("OpRayQueryGetIntersectionGeometryIndexKHR") == 0 ||
      opcode.compare("OpRayQueryGetIntersectionPrimitiveIndexKHR") == 0 ||
      opcode.compare("OpRayQueryGetIntersectionBarycentricsKHR") == 0 ||
      opcode.compare("OpRayQueryGetIntersectionFrontFaceKHR") == 0 ||
      opcode.compare("OpRayQueryGetIntersectionObjectRayDirectionKHR") == 0 ||
      opcode.compare("OpRayQueryGetIntersectionObjectRayOriginKHR") == 0 ||
      opcode.compare("OpRayQueryGetIntersectionObjectToWorldKHR") == 0 ||
      opcode.compare("OpRayQueryGetIntersectionWorldToObjectKHR") == 0) {
    return valid ? "%s32_0" : "%f32_0";
  }
  return "";
}

using RayQueryCommon = spvtest::ValidateBase<std::string>;

TEST_P(RayQueryCommon, Success) {
  std::string opcode = GetParam();
  std::ostringstream ss;
  ss << RayQueryResult(opcode);
  ss << " " << opcode << " ";
  ss << RayQueryResultType(opcode, true);
  ss << " %ray_query ";
  ss << RayQueryIntersection(opcode, true);
  CompileSuccessfully(GenerateShaderCode(ss.str()).c_str());
  EXPECT_EQ(SPV_SUCCESS, ValidateInstructions());
}

TEST_P(RayQueryCommon, BadQuery) {
  std::string opcode = GetParam();
  std::ostringstream ss;
  ss << RayQueryResult(opcode);
  ss << " " << opcode << " ";
  ss << RayQueryResultType(opcode, true);
  ss << " %top_level_as ";
  ss << RayQueryIntersection(opcode, true);
  CompileSuccessfully(GenerateShaderCode(ss.str()).c_str());
  EXPECT_EQ(SPV_ERROR_INVALID_DATA, ValidateInstructions());
  EXPECT_THAT(getDiagnosticString(),
              HasSubstr("Ray Query must be a pointer to OpTypeRayQueryKHR"));
}

TEST_P(RayQueryCommon, BadResult) {
  std::string opcode = GetParam();
  std::string result_type = RayQueryResultType(opcode, false);
  if (!result_type.empty()) {
    std::ostringstream ss;
    ss << RayQueryResult(opcode);
    ss << " " << opcode << " ";
    ss << result_type;
    ss << " %ray_query ";
    ss << RayQueryIntersection(opcode, true);
    CompileSuccessfully(GenerateShaderCode(ss.str()).c_str());
    EXPECT_EQ(SPV_ERROR_INVALID_DATA, ValidateInstructions());

    std::string correct_result_type = RayQueryResultType(opcode, true);
    if (correct_result_type.compare("%u32") == 0) {
      EXPECT_THAT(
          getDiagnosticString(),
          HasSubstr("expected Result Type to be 32-bit int scalar type"));
    } else if (correct_result_type.compare("%f32") == 0) {
      EXPECT_THAT(
          getDiagnosticString(),
          HasSubstr("expected Result Type to be 32-bit float scalar type"));
    } else if (correct_result_type.compare("%f32vec2") == 0) {
      EXPECT_THAT(getDiagnosticString(),
                  HasSubstr("expected Result Type to be 32-bit float "
                            "2-component vector type"));
    } else if (correct_result_type.compare("%f32vec3") == 0) {
      EXPECT_THAT(getDiagnosticString(),
                  HasSubstr("expected Result Type to be 32-bit float "
                            "3-component vector type"));
    } else if (correct_result_type.compare("%bool") == 0) {
      EXPECT_THAT(getDiagnosticString(),
                  HasSubstr("expected Result Type to be bool scalar type"));
    } else if (correct_result_type.compare("%mat4x3") == 0) {
      EXPECT_THAT(getDiagnosticString(),
                  HasSubstr("expected matrix type as Result Type"));
    }
  }
}

TEST_P(RayQueryCommon, BadIntersection) {
  std::string opcode = GetParam();
  std::string intersection = RayQueryIntersection(opcode, false);
  if (!intersection.empty()) {
    std::ostringstream ss;
    ss << RayQueryResult(opcode);
    ss << " " << opcode << " ";
    ss << RayQueryResultType(opcode, true);
    ss << " %ray_query ";
    ss << intersection;
    CompileSuccessfully(GenerateShaderCode(ss.str()).c_str());
    EXPECT_EQ(SPV_ERROR_INVALID_DATA, ValidateInstructions());
    EXPECT_THAT(
        getDiagnosticString(),
        HasSubstr(
            "expected Intersection ID to be a constant 32-bit int scalar"));
  }
}

INSTANTIATE_TEST_SUITE_P(
    ValidateRayQueryCommon, RayQueryCommon,
    Values("OpRayQueryTerminateKHR", "OpRayQueryConfirmIntersectionKHR",
           "OpRayQueryProceedKHR", "OpRayQueryGetIntersectionTypeKHR",
           "OpRayQueryGetRayTMinKHR", "OpRayQueryGetRayFlagsKHR",
           "OpRayQueryGetWorldRayDirectionKHR",
           "OpRayQueryGetWorldRayOriginKHR", "OpRayQueryGetIntersectionTKHR",
           "OpRayQueryGetIntersectionInstanceCustomIndexKHR",
           "OpRayQueryGetIntersectionInstanceIdKHR",
           "OpRayQueryGetIntersectionInstanceShaderBindingTableRecordOffsetKHR",
           "OpRayQueryGetIntersectionGeometryIndexKHR",
           "OpRayQueryGetIntersectionPrimitiveIndexKHR",
           "OpRayQueryGetIntersectionBarycentricsKHR",
           "OpRayQueryGetIntersectionFrontFaceKHR",
           "OpRayQueryGetIntersectionCandidateAABBOpaqueKHR",
           "OpRayQueryGetIntersectionObjectRayDirectionKHR",
           "OpRayQueryGetIntersectionObjectRayOriginKHR",
           "OpRayQueryGetIntersectionObjectToWorldKHR",
           "OpRayQueryGetIntersectionWorldToObjectKHR"));

// tests various Intersection operand types
TEST_F(ValidateRayQuery, IntersectionSuccess) {
  const std::string body = R"(
%result_1 = OpRayQueryGetIntersectionFrontFaceKHR %bool %ray_query %s32_0
%result_2 = OpRayQueryGetIntersectionFrontFaceKHR %bool %ray_query %u32_0
)";

  CompileSuccessfully(GenerateShaderCode(body).c_str());
  EXPECT_EQ(SPV_SUCCESS, ValidateInstructions());
}

TEST_F(ValidateRayQuery, IntersectionVector) {
  const std::string body = R"(
%result = OpRayQueryGetIntersectionFrontFaceKHR %bool %ray_query %u32vec3_0
)";

  CompileSuccessfully(GenerateShaderCode(body).c_str());
  EXPECT_EQ(SPV_ERROR_INVALID_DATA, ValidateInstructions());
  EXPECT_THAT(
      getDiagnosticString(),
      HasSubstr("expected Intersection ID to be a constant 32-bit int scalar"));
}

TEST_F(ValidateRayQuery, IntersectionNonConstantVariable) {
  const std::string body = R"(
%var = OpVariable %ptr_function_u32 Function
%result = OpRayQueryGetIntersectionFrontFaceKHR %bool %ray_query %var
)";

  CompileSuccessfully(GenerateShaderCode(body).c_str());
  EXPECT_EQ(SPV_ERROR_INVALID_DATA, ValidateInstructions());
  EXPECT_THAT(
      getDiagnosticString(),
      HasSubstr("expected Intersection ID to be a constant 32-bit int scalar"));
}

TEST_F(ValidateRayQuery, IntersectionNonConstantLoad) {
  const std::string body = R"(
%var = OpVariable %ptr_function_u32 Function
%load = OpLoad %u32 %var
%result = OpRayQueryGetIntersectionFrontFaceKHR %bool %ray_query %load
)";

  CompileSuccessfully(GenerateShaderCode(body).c_str());
  EXPECT_EQ(SPV_ERROR_INVALID_DATA, ValidateInstructions());
  EXPECT_THAT(
      getDiagnosticString(),
      HasSubstr("expected Intersection ID to be a constant 32-bit int scalar"));
}

TEST_F(ValidateRayQuery, InitializeSuccess) {
  const std::string body = R"(
%var_u32 = OpVariable %ptr_function_u32 Function
%var_f32 = OpVariable %ptr_function_f32 Function
%var_f32vec3 = OpVariable %ptr_function_f32vec3 Function

%as = OpLoad %type_as %top_level_as
OpRayQueryInitializeKHR %ray_query %as %u32_0 %u32_0 %f32vec3_0 %f32_0 %f32vec3_0 %f32_0

%_u32 = OpLoad %u32 %var_u32
%_f32 = OpLoad %f32 %var_f32
%_f32vec3 = OpLoad %f32vec3 %var_f32vec3
OpRayQueryInitializeKHR %ray_query %as %_u32 %_u32 %_f32vec3 %_f32 %_f32vec3 %_f32
)";

  CompileSuccessfully(GenerateShaderCode(body).c_str());
  EXPECT_EQ(SPV_SUCCESS, ValidateInstructions());
}

TEST_F(ValidateRayQuery, InitializeFunctionSuccess) {
  const std::string declaration = R"(
%rq_ptr = OpTypePointer Private %type_rq
%rq_func_type = OpTypeFunction %void %rq_ptr
%rq_var_1 = OpVariable %rq_ptr Private
%rq_var_2 = OpVariable %rq_ptr Private
)";

  const std::string body = R"(
%fcall_1 = OpFunctionCall %void %rq_func %rq_var_1
%as_1 = OpLoad %type_as %top_level_as
OpRayQueryInitializeKHR %rq_var_1 %as_1 %u32_0 %u32_0 %f32vec3_0 %f32_0 %f32vec3_0 %f32_0
%fcall_2 = OpFunctionCall %void %rq_func %rq_var_2
OpReturn
OpFunctionEnd
%rq_func = OpFunction %void None %rq_func_type
%rq_param = OpFunctionParameter %rq_ptr
%label = OpLabel
%as_2 = OpLoad %type_as %top_level_as
OpRayQueryInitializeKHR %rq_param %as_2 %u32_0 %u32_0 %f32vec3_0 %f32_0 %f32vec3_0 %f32_0
)";

  CompileSuccessfully(GenerateShaderCode(body, "", declaration).c_str());
  EXPECT_EQ(SPV_SUCCESS, ValidateInstructions());
}

TEST_F(ValidateRayQuery, InitializeBadRayQuery) {
  const std::string body = R"(
%load = OpLoad %type_as %top_level_as
OpRayQueryInitializeKHR %top_level_as %load %u32_0 %u32_0 %f32vec3_0 %f32_0 %f32vec3_0 %f32_0
)";

  CompileSuccessfully(GenerateShaderCode(body).c_str());
  EXPECT_EQ(SPV_ERROR_INVALID_DATA, ValidateInstructions());
  EXPECT_THAT(getDiagnosticString(),
              HasSubstr("Ray Query must be a pointer to OpTypeRayQueryKHR"));
}

TEST_F(ValidateRayQuery, InitializeBadAS) {
  const std::string body = R"(
OpRayQueryInitializeKHR %ray_query %ray_query %u32_0 %u32_0 %f32vec3_0 %f32_0 %f32vec3_0 %f32_0
)";

  CompileSuccessfully(GenerateShaderCode(body).c_str());
  EXPECT_EQ(SPV_ERROR_INVALID_DATA, ValidateInstructions());
  EXPECT_THAT(getDiagnosticString(),
              HasSubstr("Expected Acceleration Structure to be of type "
                        "OpTypeAccelerationStructureKHR"));
}

TEST_F(ValidateRayQuery, InitializeBadRayFlags64) {
  const std::string body = R"(
%load = OpLoad %type_as %top_level_as
OpRayQueryInitializeKHR %ray_query %load %u64_0 %u32_0 %f32vec3_0 %f32_0 %f32vec3_0 %f32_0
)";

  CompileSuccessfully(GenerateShaderCode(body).c_str());
  EXPECT_EQ(SPV_ERROR_INVALID_DATA, ValidateInstructions());
  EXPECT_THAT(getDiagnosticString(),
              HasSubstr("Ray Flags must be a 32-bit int scalar"));
}

TEST_F(ValidateRayQuery, InitializeBadRayFlagsVector) {
  const std::string body = R"(
%load = OpLoad %type_as %top_level_as
OpRayQueryInitializeKHR %ray_query %load %u32vec2 %u32_0 %f32vec3_0 %f32_0 %f32vec3_0 %f32_0
)";

  CompileSuccessfully(GenerateShaderCode(body).c_str());
  EXPECT_EQ(SPV_ERROR_INVALID_ID, ValidateInstructions());
  EXPECT_THAT(getDiagnosticString(),
              HasSubstr("Operand '15[%v2uint]' cannot be a type"));
}

TEST_F(ValidateRayQuery, InitializeBadCullMask) {
  const std::string body = R"(
%load = OpLoad %type_as %top_level_as
OpRayQueryInitializeKHR %ray_query %load %u32_0 %f32_0 %f32vec3_0 %f32_0 %f32vec3_0 %f32_0
)";

  CompileSuccessfully(GenerateShaderCode(body).c_str());
  EXPECT_EQ(SPV_ERROR_INVALID_DATA, ValidateInstructions());
  EXPECT_THAT(getDiagnosticString(),
              HasSubstr("Cull Mask must be a 32-bit int scalar"));
}

TEST_F(ValidateRayQuery, InitializeBadRayOriginVec4) {
  const std::string body = R"(
%load = OpLoad %type_as %top_level_as
OpRayQueryInitializeKHR %ray_query %load %u32_0 %u32_0 %f32vec4_0 %f32_0 %f32vec3_0 %f32_0
)";

  CompileSuccessfully(GenerateShaderCode(body).c_str());
  EXPECT_EQ(SPV_ERROR_INVALID_DATA, ValidateInstructions());
  EXPECT_THAT(
      getDiagnosticString(),
      HasSubstr("Ray Origin must be a 32-bit float 3-component vector"));
}

TEST_F(ValidateRayQuery, InitializeBadRayOriginFloat) {
  const std::string body = R"(
%var_f32 = OpVariable %ptr_function_f32 Function
%_f32 = OpLoad %f32 %var_f32
%load = OpLoad %type_as %top_level_as
OpRayQueryInitializeKHR %ray_query %load %u32_0 %u32_0 %_f32 %f32_0 %f32vec3_0 %f32_0
)";

  CompileSuccessfully(GenerateShaderCode(body).c_str());
  EXPECT_EQ(SPV_ERROR_INVALID_DATA, ValidateInstructions());
  EXPECT_THAT(
      getDiagnosticString(),
      HasSubstr("Ray Origin must be a 32-bit float 3-component vector"));
}

TEST_F(ValidateRayQuery, InitializeBadRayOriginInt) {
  const std::string body = R"(
%load = OpLoad %type_as %top_level_as
OpRayQueryInitializeKHR %ray_query %load %u32_0 %u32_0 %u32vec3_0 %f32_0 %f32vec3_0 %f32_0
)";

  CompileSuccessfully(GenerateShaderCode(body).c_str());
  EXPECT_EQ(SPV_ERROR_INVALID_DATA, ValidateInstructions());
  EXPECT_THAT(
      getDiagnosticString(),
      HasSubstr("Ray Origin must be a 32-bit float 3-component vector"));
}

TEST_F(ValidateRayQuery, InitializeBadRayTMin) {
  const std::string body = R"(
%load = OpLoad %type_as %top_level_as
OpRayQueryInitializeKHR %ray_query %load %u32_0 %u32_0 %f32vec3_0 %u32_0 %f32vec3_0 %f32_0
)";

  CompileSuccessfully(GenerateShaderCode(body).c_str());
  EXPECT_EQ(SPV_ERROR_INVALID_DATA, ValidateInstructions());
  EXPECT_THAT(getDiagnosticString(),
              HasSubstr("Ray TMin must be a 32-bit float scalar"));
}

TEST_F(ValidateRayQuery, InitializeBadRayDirection) {
  const std::string body = R"(
%load = OpLoad %type_as %top_level_as
OpRayQueryInitializeKHR %ray_query %load %u32_0 %u32_0 %f32vec3_0 %f32_0 %f32vec4_0 %f32_0
)";

  CompileSuccessfully(GenerateShaderCode(body).c_str());
  EXPECT_EQ(SPV_ERROR_INVALID_DATA, ValidateInstructions());
  EXPECT_THAT(
      getDiagnosticString(),
      HasSubstr("Ray Direction must be a 32-bit float 3-component vector"));
}

TEST_F(ValidateRayQuery, InitializeBadRayTMax) {
  const std::string body = R"(
%load = OpLoad %type_as %top_level_as
OpRayQueryInitializeKHR %ray_query %load %u32_0 %u32_0 %f32vec3_0 %f32_0 %f32vec3_0 %f64_0
)";

  CompileSuccessfully(GenerateShaderCode(body).c_str());
  EXPECT_EQ(SPV_ERROR_INVALID_DATA, ValidateInstructions());
  EXPECT_THAT(getDiagnosticString(),
              HasSubstr("Ray TMax must be a 32-bit float scalar"));
}

TEST_F(ValidateRayQuery, GenerateIntersectionSuccess) {
  const std::string body = R"(
%var = OpVariable %ptr_function_f32 Function
%load = OpLoad %f32 %var
OpRayQueryGenerateIntersectionKHR %ray_query %f32_0
OpRayQueryGenerateIntersectionKHR %ray_query %load
)";

  CompileSuccessfully(GenerateShaderCode(body).c_str());
  EXPECT_EQ(SPV_SUCCESS, ValidateInstructions());
}

TEST_F(ValidateRayQuery, GenerateIntersectionBadRayQuery) {
  const std::string body = R"(
OpRayQueryGenerateIntersectionKHR %top_level_as %f32_0
)";

  CompileSuccessfully(GenerateShaderCode(body).c_str());
  EXPECT_EQ(SPV_ERROR_INVALID_DATA, ValidateInstructions());
  EXPECT_THAT(getDiagnosticString(),
              HasSubstr("Ray Query must be a pointer to OpTypeRayQueryKHR"));
}

TEST_F(ValidateRayQuery, GenerateIntersectionBadHitT) {
  const std::string body = R"(
OpRayQueryGenerateIntersectionKHR %ray_query %u32_0
)";

  CompileSuccessfully(GenerateShaderCode(body).c_str());
  EXPECT_EQ(SPV_ERROR_INVALID_DATA, ValidateInstructions());
  EXPECT_THAT(getDiagnosticString(),
              HasSubstr("Hit T must be a 32-bit float scalar"));
}

TEST_F(ValidateRayQuery, RayQueryArraySuccess) {
  // This shader is slightly different to the ones above, so it doesn't reuse
  // the shader code generator.
  const std::string shader = R"(
                       OpCapability Shader
                       OpCapability RayQueryKHR
                       OpExtension "SPV_KHR_ray_query"
                       OpMemoryModel Logical GLSL450
                       OpEntryPoint GLCompute %main "main"
                       OpExecutionMode %main LocalSize 1 1 1
                       OpSource GLSL 460
                       OpDecorate %topLevelAS DescriptorSet 0
                       OpDecorate %topLevelAS Binding 0
                       OpDecorate %gl_WorkGroupSize BuiltIn WorkgroupSize
               %void = OpTypeVoid
               %func = OpTypeFunction %void
          %ray_query = OpTypeRayQueryKHR
               %uint = OpTypeInt 32 0
             %uint_2 = OpConstant %uint 2
    %ray_query_array = OpTypeArray %ray_query %uint_2
%ptr_ray_query_array = OpTypePointer Private %ray_query_array
         %rayQueries = OpVariable %ptr_ray_query_array Private
                %int = OpTypeInt 32 1
              %int_0 = OpConstant %int 0
      %ptr_ray_query = OpTypePointer Private %ray_query
       %accel_struct = OpTypeAccelerationStructureKHR
   %ptr_accel_struct = OpTypePointer UniformConstant %accel_struct
         %topLevelAS = OpVariable %ptr_accel_struct UniformConstant
             %uint_0 = OpConstant %uint 0
           %uint_255 = OpConstant %uint 255
              %float = OpTypeFloat 32
            %v3float = OpTypeVector %float 3
            %float_0 = OpConstant %float 0
          %vec3_zero = OpConstantComposite %v3float %float_0 %float_0 %float_0
            %float_1 = OpConstant %float 1
      %vec3_xy_0_z_1 = OpConstantComposite %v3float %float_0 %float_0 %float_1
           %float_10 = OpConstant %float 10
             %v3uint = OpTypeVector %uint 3
             %uint_1 = OpConstant %uint 1
   %gl_WorkGroupSize = OpConstantComposite %v3uint %uint_1 %uint_1 %uint_1
               %main = OpFunction %void None %func
         %main_label = OpLabel
    %first_ray_query = OpAccessChain %ptr_ray_query %rayQueries %int_0
     %topLevelAS_val = OpLoad %accel_struct %topLevelAS
                       OpRayQueryInitializeKHR %first_ray_query %topLevelAS_val %uint_0 %uint_255 %vec3_zero %float_0 %vec3_xy_0_z_1 %float_10
                       OpReturn
                       OpFunctionEnd
)";
  CompileSuccessfully(shader);
  EXPECT_EQ(SPV_SUCCESS, ValidateInstructions());
}

}  // namespace
}  // namespace val
}  // namespace spvtools
