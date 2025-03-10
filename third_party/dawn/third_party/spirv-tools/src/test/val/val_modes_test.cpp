// Copyright (c) 2018 Google LLC.
// Modifications Copyright (C) 2024 Advanced Micro Devices, Inc. All rights
// reserved.
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
#include <vector>

#include "gmock/gmock.h"
#include "source/spirv_target_env.h"
#include "test/unit_spirv.h"
#include "test/val/val_fixtures.h"

namespace spvtools {
namespace val {
namespace {

using ::testing::Combine;
using ::testing::HasSubstr;
using ::testing::Values;
using ::testing::ValuesIn;

using ValidateMode = spvtest::ValidateBase<bool>;

const std::string kVoidFunction = R"(%void = OpTypeVoid
%void_fn = OpTypeFunction %void
%main = OpFunction %void None %void_fn
%entry = OpLabel
OpReturn
OpFunctionEnd
)";

TEST_F(ValidateMode, GLComputeNoMode) {
  const std::string spirv = R"(
OpCapability Shader
OpMemoryModel Logical GLSL450
OpEntryPoint GLCompute %main "main"
)" + kVoidFunction;

  CompileSuccessfully(spirv);
  EXPECT_THAT(SPV_SUCCESS, ValidateInstructions());
}

TEST_F(ValidateMode, GLComputeNoModeVulkan) {
  const std::string spirv = R"(
OpCapability Shader
OpMemoryModel Logical GLSL450
OpEntryPoint GLCompute %main "main"
)" + kVoidFunction;

  spv_target_env env = SPV_ENV_VULKAN_1_0;
  CompileSuccessfully(spirv, env);
  EXPECT_THAT(SPV_ERROR_INVALID_DATA, ValidateInstructions(env));
  EXPECT_THAT(getDiagnosticString(),
              AnyVUID("VUID-StandaloneSpirv-LocalSize-06426"));
  EXPECT_THAT(
      getDiagnosticString(),
      HasSubstr(
          "In the Vulkan environment, GLCompute execution model entry "
          "points require either the LocalSize or LocalSizeId execution mode "
          "or an object decorated with WorkgroupSize must be specified."));
}

TEST_F(ValidateMode, GLComputeNoModeVulkanWorkgroupSize) {
  const std::string spirv = R"(
OpCapability Shader
OpMemoryModel Logical GLSL450
OpEntryPoint GLCompute %main "main"
OpDecorate %int3_1 BuiltIn WorkgroupSize
%int = OpTypeInt 32 0
%int3 = OpTypeVector %int 3
%int_1 = OpConstant %int 1
%int3_1 = OpConstantComposite %int3 %int_1 %int_1 %int_1
)" + kVoidFunction;

  spv_target_env env = SPV_ENV_VULKAN_1_0;
  CompileSuccessfully(spirv, env);
  EXPECT_THAT(SPV_SUCCESS, ValidateInstructions(env));
}

TEST_F(ValidateMode, GLComputeZeroWorkgroupSize) {
  const std::string spirv = R"(
OpCapability Shader
OpMemoryModel Logical GLSL450
OpEntryPoint GLCompute %main "main"
OpDecorate %int3_1 BuiltIn WorkgroupSize
%int = OpTypeInt 32 0
%int3 = OpTypeVector %int 3
%int_0 = OpConstant %int 0
%int_1 = OpConstant %int 1
%int3_1 = OpConstantComposite %int3 %int_1 %int_0 %int_0
)" + kVoidFunction;

  CompileSuccessfully(spirv);
  EXPECT_THAT(SPV_ERROR_INVALID_DATA, ValidateInstructions());
  EXPECT_THAT(
      getDiagnosticString(),
      HasSubstr(
          "WorkgroupSize decorations must not have a static product of zero"));
}

TEST_F(ValidateMode, GLComputeZeroSpecWorkgroupSize) {
  const std::string spirv = R"(
OpCapability Shader
OpMemoryModel Logical GLSL450
OpEntryPoint GLCompute %main "main"
OpDecorate %int3_1 BuiltIn WorkgroupSize
%int = OpTypeInt 32 0
%int3 = OpTypeVector %int 3
%int_0 = OpSpecConstant %int 0
%int_1 = OpConstant %int 1
%int3_1 = OpConstantComposite %int3 %int_1 %int_0 %int_0
)" + kVoidFunction;

  CompileSuccessfully(spirv);
  EXPECT_EQ(SPV_SUCCESS, ValidateInstructions());
}

TEST_F(ValidateMode, GLComputeZeroSpecCompositeWorkgroupSize) {
  const std::string spirv = R"(
OpCapability Shader
OpMemoryModel Logical GLSL450
OpEntryPoint GLCompute %main "main"
OpDecorate %int3_1 BuiltIn WorkgroupSize
%int = OpTypeInt 32 0
%int3 = OpTypeVector %int 3
%int_0 = OpSpecConstant %int 0
%int_1 = OpSpecConstant %int 1
%int3_1 = OpSpecConstantComposite %int3 %int_1 %int_0 %int_0
)" + kVoidFunction;

  CompileSuccessfully(spirv);
  EXPECT_EQ(SPV_SUCCESS, ValidateInstructions());
}

TEST_F(ValidateMode, KernelZeroWorkgroupSizeConstant) {
  const std::string spirv = R"(
OpCapability Addresses
OpCapability Linkage
OpCapability Kernel
OpMemoryModel Physical32 OpenCL
OpEntryPoint Kernel %main "main"
OpDecorate %int3_1 BuiltIn WorkgroupSize
%int = OpTypeInt 32 0
%int3 = OpTypeVector %int 3
%int_0 = OpConstant %int 0
%int_1 = OpConstant %int 1
%int3_1 = OpConstantComposite %int3 %int_1 %int_0 %int_0
)" + kVoidFunction;

  CompileSuccessfully(spirv);
  EXPECT_THAT(SPV_ERROR_INVALID_ID, ValidateInstructions());
  EXPECT_THAT(getDiagnosticString(), HasSubstr("must be a variable"));
}

TEST_F(ValidateMode, KernelZeroWorkgroupSizeVariable) {
  const std::string spirv = R"(
OpCapability Addresses
OpCapability Linkage
OpCapability Kernel
OpMemoryModel Physical32 OpenCL
OpEntryPoint Kernel %main "main"
OpDecorate %var BuiltIn WorkgroupSize
%int = OpTypeInt 32 0
%int3 = OpTypeVector %int 3
%ptr = OpTypePointer Input %int3
%var = OpVariable %ptr Input
)" + kVoidFunction;

  CompileSuccessfully(spirv);
  EXPECT_EQ(SPV_SUCCESS, ValidateInstructions());
}

TEST_F(ValidateMode, GLComputeVulkanLocalSize) {
  const std::string spirv = R"(
OpCapability Shader
OpMemoryModel Logical GLSL450
OpEntryPoint GLCompute %main "main"
OpExecutionMode %main LocalSize 1 1 1
)" + kVoidFunction;

  spv_target_env env = SPV_ENV_VULKAN_1_0;
  CompileSuccessfully(spirv, env);
  EXPECT_THAT(SPV_SUCCESS, ValidateInstructions(env));
}

TEST_F(ValidateMode, GLComputeZeroLocalSize) {
  const std::string spirv = R"(
OpCapability Shader
OpMemoryModel Logical GLSL450
OpEntryPoint GLCompute %main "main"
OpExecutionMode %main LocalSize 1 1 0
)" + kVoidFunction;

  CompileSuccessfully(spirv);
  EXPECT_THAT(SPV_ERROR_INVALID_DATA, ValidateInstructions());
  EXPECT_THAT(
      getDiagnosticString(),
      HasSubstr("Local Size execution mode must not have a product of zero"));
}

TEST_F(ValidateMode, KernelZeroLocalSize) {
  const std::string spirv = R"(
OpCapability Addresses
OpCapability Linkage
OpCapability Kernel
OpMemoryModel Physical32 OpenCL
OpEntryPoint Kernel %main "main"
OpExecutionMode %main LocalSize 1 1 0
)" + kVoidFunction;

  CompileSuccessfully(spirv);
  EXPECT_THAT(SPV_ERROR_INVALID_DATA, ValidateInstructions());
  EXPECT_THAT(
      getDiagnosticString(),
      HasSubstr("Local Size execution mode must not have a product of zero"));
}

TEST_F(ValidateMode, GLComputeVulkanLocalSizeIdBad) {
  const std::string spirv = R"(
OpCapability Shader
OpMemoryModel Logical GLSL450
OpEntryPoint GLCompute %main "main"
OpExecutionModeId %main LocalSizeId %int_1 %int_1 %int_1
%int = OpTypeInt 32 0
%int_1 = OpConstant %int 1
)" + kVoidFunction;

  spv_target_env env = SPV_ENV_VULKAN_1_1;  // need SPIR-V 1.2
  CompileSuccessfully(spirv, env);
  EXPECT_THAT(SPV_ERROR_INVALID_DATA, ValidateInstructions(env));
  EXPECT_THAT(
      getDiagnosticString(),
      HasSubstr("LocalSizeId mode is not allowed by the current environment."));
}

TEST_F(ValidateMode, GLComputeVulkanLocalSizeIdGood) {
  const std::string spirv = R"(
OpCapability Shader
OpMemoryModel Logical GLSL450
OpEntryPoint GLCompute %main "main"
OpExecutionModeId %main LocalSizeId %int_1 %int_1 %int_1
%int = OpTypeInt 32 0
%int_1 = OpConstant %int 1
)" + kVoidFunction;

  spv_target_env env = SPV_ENV_VULKAN_1_1;  // need SPIR-V 1.2
  CompileSuccessfully(spirv, env);
  spvValidatorOptionsSetAllowLocalSizeId(getValidatorOptions(), true);
  EXPECT_THAT(SPV_SUCCESS, ValidateInstructions(env));
}

TEST_F(ValidateMode, GLComputeZeroLocalSizeId) {
  const std::string spirv = R"(
OpCapability Shader
OpMemoryModel Logical GLSL450
OpEntryPoint GLCompute %main "main"
OpExecutionModeId %main LocalSizeId %int_1 %int_0 %int_1
%int = OpTypeInt 32 0
%int_1 = OpConstant %int 1
%int_0 = OpConstant %int 0
)" + kVoidFunction;

  spv_target_env env = SPV_ENV_UNIVERSAL_1_3;
  CompileSuccessfully(spirv, env);
  EXPECT_THAT(SPV_ERROR_INVALID_DATA, ValidateInstructions(env));
  EXPECT_THAT(
      getDiagnosticString(),
      HasSubstr(
          "Local Size Id execution mode must not have a product of zero"));
}

TEST_F(ValidateMode, GLComputeZeroSpecLocalSizeId) {
  const std::string spirv = R"(
OpCapability Shader
OpMemoryModel Logical GLSL450
OpEntryPoint GLCompute %main "main"
OpExecutionModeId %main LocalSizeId %int_1 %int_0 %int_1
%int = OpTypeInt 32 0
%int_1 = OpConstant %int 1
%int_0 = OpSpecConstant %int 0
)" + kVoidFunction;

  spv_target_env env = SPV_ENV_UNIVERSAL_1_3;
  CompileSuccessfully(spirv, env);
  EXPECT_EQ(SPV_SUCCESS, ValidateInstructions(env));
}

TEST_F(ValidateMode, KernelZeroLocalSizeId) {
  const std::string spirv = R"(
OpCapability Addresses
OpCapability Linkage
OpCapability Kernel
OpMemoryModel Physical32 OpenCL
OpEntryPoint Kernel %main "main"
OpExecutionModeId %main LocalSizeId %int_1 %int_0 %int_1
%int = OpTypeInt 32 0
%int_1 = OpConstant %int 1
%int_0 = OpConstant %int 0
)" + kVoidFunction;

  spv_target_env env = SPV_ENV_UNIVERSAL_1_3;
  CompileSuccessfully(spirv, env);
  EXPECT_THAT(SPV_ERROR_INVALID_DATA, ValidateInstructions(env));
  EXPECT_THAT(
      getDiagnosticString(),
      HasSubstr(
          "Local Size Id execution mode must not have a product of zero"));
}

// https://github.com/KhronosGroup/SPIRV-Tools/issues/5939
TEST_F(ValidateMode, KernelZeroLocalSize64) {
  const std::string spirv = R"(
               OpCapability Kernel
               OpCapability Addresses
               OpCapability Int64
               OpCapability Linkage
               OpMemoryModel Physical64 OpenCL
               OpEntryPoint Kernel %test "test" %__spirv_BuiltInWorkgroupSize
               OpExecutionMode %test ContractionOff
               OpDecorate %__spirv_BuiltInWorkgroupSize Constant
               OpDecorate %__spirv_BuiltInWorkgroupSize LinkageAttributes "__spirv_BuiltInWorkgroupSize" Import
               OpDecorate %__spirv_BuiltInWorkgroupSize BuiltIn WorkgroupSize
       %void = OpTypeVoid
      %ulong = OpTypeInt 64 0
    %v3ulong = OpTypeVector %ulong 3
%_ptr_Input_v3ulong = OpTypePointer Input %v3ulong
          %8 = OpTypeFunction %void
%__spirv_BuiltInWorkgroupSize = OpVariable %_ptr_Input_v3ulong Input
       %test = OpFunction %void None %8
      %entry = OpLabel
         %11 = OpLoad %v3ulong %__spirv_BuiltInWorkgroupSize Aligned 1
         %12 = OpCompositeExtract %ulong %11 0
               OpReturn
               OpFunctionEnd
)";

  CompileSuccessfully(spirv);
  EXPECT_EQ(SPV_SUCCESS, ValidateInstructions());
}

TEST_F(ValidateMode, FragmentOriginLowerLeftVulkan) {
  const std::string spirv = R"(
OpCapability Shader
OpMemoryModel Logical GLSL450
OpEntryPoint Fragment %main "main"
OpExecutionMode %main OriginLowerLeft
)" + kVoidFunction;

  spv_target_env env = SPV_ENV_VULKAN_1_0;
  CompileSuccessfully(spirv, env);
  EXPECT_THAT(SPV_ERROR_INVALID_DATA, ValidateInstructions(env));
  EXPECT_THAT(getDiagnosticString(),
              AnyVUID("VUID-StandaloneSpirv-OriginLowerLeft-04653"));
  EXPECT_THAT(getDiagnosticString(),
              HasSubstr("In the Vulkan environment, the OriginLowerLeft "
                        "execution mode must not be used."));
}

TEST_F(ValidateMode, FragmentPixelCenterIntegerVulkan) {
  const std::string spirv = R"(
OpCapability Shader
OpMemoryModel Logical GLSL450
OpEntryPoint Fragment %main "main"
OpExecutionMode %main OriginUpperLeft
OpExecutionMode %main PixelCenterInteger
)" + kVoidFunction;

  spv_target_env env = SPV_ENV_VULKAN_1_0;
  CompileSuccessfully(spirv, env);
  EXPECT_THAT(SPV_ERROR_INVALID_DATA, ValidateInstructions(env));
  EXPECT_THAT(getDiagnosticString(),
              AnyVUID("VUID-StandaloneSpirv-PixelCenterInteger-04654"));
  EXPECT_THAT(getDiagnosticString(),
              HasSubstr("In the Vulkan environment, the PixelCenterInteger "
                        "execution mode must not be used."));
}

TEST_F(ValidateMode, GeometryNoOutputMode) {
  const std::string spirv = R"(
OpCapability Geometry
OpMemoryModel Logical GLSL450
OpEntryPoint Geometry %main "main"
OpExecutionMode %main InputPoints
)" + kVoidFunction;

  CompileSuccessfully(spirv);
  EXPECT_THAT(SPV_ERROR_INVALID_DATA, ValidateInstructions());
  EXPECT_THAT(getDiagnosticString(),
              HasSubstr("Geometry execution model entry points must specify "
                        "exactly one of OutputPoints, OutputLineStrip or "
                        "OutputTriangleStrip execution modes."));
}

TEST_F(ValidateMode, GeometryNoInputMode) {
  const std::string spirv = R"(
OpCapability Geometry
OpMemoryModel Logical GLSL450
OpEntryPoint Geometry %main "main"
OpExecutionMode %main OutputPoints
)" + kVoidFunction;

  CompileSuccessfully(spirv);
  EXPECT_THAT(SPV_ERROR_INVALID_DATA, ValidateInstructions());
  EXPECT_THAT(
      getDiagnosticString(),
      HasSubstr("Geometry execution model entry points must specify exactly "
                "one of InputPoints, InputLines, InputLinesAdjacency, "
                "Triangles or InputTrianglesAdjacency execution modes."));
}

TEST_F(ValidateMode, FragmentNoOrigin) {
  const std::string spirv = R"(
OpCapability Shader
OpMemoryModel Logical GLSL450
OpEntryPoint Fragment %main "main"
)" + kVoidFunction;

  CompileSuccessfully(spirv);
  EXPECT_THAT(SPV_ERROR_INVALID_DATA, ValidateInstructions());
  EXPECT_THAT(
      getDiagnosticString(),
      HasSubstr("Fragment execution model entry points require either an "
                "OriginUpperLeft or OriginLowerLeft execution mode."));
}

TEST_F(ValidateMode, FragmentBothOrigins) {
  const std::string spirv = R"(
OpCapability Shader
OpMemoryModel Logical GLSL450
OpEntryPoint Fragment %main "main"
OpExecutionMode %main OriginUpperLeft
OpExecutionMode %main OriginLowerLeft
)" + kVoidFunction;

  CompileSuccessfully(spirv);
  EXPECT_THAT(SPV_ERROR_INVALID_DATA, ValidateInstructions());
  EXPECT_THAT(
      getDiagnosticString(),
      HasSubstr("Fragment execution model entry points can only specify one of "
                "OriginUpperLeft or OriginLowerLeft execution modes."));
}

TEST_F(ValidateMode, FragmentDepthGreaterAndLess) {
  const std::string spirv = R"(
OpCapability Shader
OpMemoryModel Logical GLSL450
OpEntryPoint Fragment %main "main"
OpExecutionMode %main OriginUpperLeft
OpExecutionMode %main DepthGreater
OpExecutionMode %main DepthLess
)" + kVoidFunction;

  CompileSuccessfully(spirv);
  EXPECT_THAT(SPV_ERROR_INVALID_DATA, ValidateInstructions());
  EXPECT_THAT(getDiagnosticString(),
              HasSubstr("Fragment execution model entry points can specify at "
                        "most one of DepthGreater, DepthLess or DepthUnchanged "
                        "execution modes."));
}

TEST_F(ValidateMode, FragmentDepthGreaterAndUnchanged) {
  const std::string spirv = R"(
OpCapability Shader
OpMemoryModel Logical GLSL450
OpEntryPoint Fragment %main "main"
OpExecutionMode %main OriginUpperLeft
OpExecutionMode %main DepthGreater
OpExecutionMode %main DepthUnchanged
)" + kVoidFunction;

  CompileSuccessfully(spirv);
  EXPECT_THAT(SPV_ERROR_INVALID_DATA, ValidateInstructions());
  EXPECT_THAT(getDiagnosticString(),
              HasSubstr("Fragment execution model entry points can specify at "
                        "most one of DepthGreater, DepthLess or DepthUnchanged "
                        "execution modes."));
}

TEST_F(ValidateMode, FragmentDepthLessAndUnchanged) {
  const std::string spirv = R"(
OpCapability Shader
OpMemoryModel Logical GLSL450
OpEntryPoint Fragment %main "main"
OpExecutionMode %main OriginUpperLeft
OpExecutionMode %main DepthLess
OpExecutionMode %main DepthUnchanged
)" + kVoidFunction;

  CompileSuccessfully(spirv);
  EXPECT_THAT(SPV_ERROR_INVALID_DATA, ValidateInstructions());
  EXPECT_THAT(getDiagnosticString(),
              HasSubstr("Fragment execution model entry points can specify at "
                        "most one of DepthGreater, DepthLess or DepthUnchanged "
                        "execution modes."));
}

TEST_F(ValidateMode, FragmentAllDepths) {
  const std::string spirv = R"(
OpCapability Shader
OpMemoryModel Logical GLSL450
OpEntryPoint Fragment %main "main"
OpExecutionMode %main OriginUpperLeft
OpExecutionMode %main DepthGreater
OpExecutionMode %main DepthLess
OpExecutionMode %main DepthUnchanged
)" + kVoidFunction;

  CompileSuccessfully(spirv);
  EXPECT_THAT(SPV_ERROR_INVALID_DATA, ValidateInstructions());
  EXPECT_THAT(getDiagnosticString(),
              HasSubstr("Fragment execution model entry points can specify at "
                        "most one of DepthGreater, DepthLess or DepthUnchanged "
                        "execution modes."));
}

TEST_F(ValidateMode, TessellationControlSpacingEqualAndFractionalOdd) {
  const std::string spirv = R"(
OpCapability Tessellation
OpMemoryModel Logical GLSL450
OpEntryPoint TessellationControl %main "main"
OpExecutionMode %main SpacingEqual
OpExecutionMode %main SpacingFractionalOdd
)" + kVoidFunction;

  CompileSuccessfully(spirv);
  EXPECT_THAT(SPV_ERROR_INVALID_DATA, ValidateInstructions());
  EXPECT_THAT(getDiagnosticString(),
              HasSubstr("Tessellation execution model entry points can specify "
                        "at most one of SpacingEqual, SpacingFractionalOdd or "
                        "SpacingFractionalEven execution modes."));
}

TEST_F(ValidateMode, TessellationControlSpacingEqualAndSpacingFractionalEven) {
  const std::string spirv = R"(
OpCapability Tessellation
OpMemoryModel Logical GLSL450
OpEntryPoint TessellationControl %main "main"
OpExecutionMode %main SpacingEqual
OpExecutionMode %main SpacingFractionalEven
)" + kVoidFunction;

  CompileSuccessfully(spirv);
  EXPECT_THAT(SPV_ERROR_INVALID_DATA, ValidateInstructions());
  EXPECT_THAT(getDiagnosticString(),
              HasSubstr("Tessellation execution model entry points can specify "
                        "at most one of SpacingEqual, SpacingFractionalOdd or "
                        "SpacingFractionalEven execution modes."));
}

TEST_F(ValidateMode,
       TessellationControlSpacingFractionalOddAndSpacingFractionalEven) {
  const std::string spirv = R"(
OpCapability Tessellation
OpMemoryModel Logical GLSL450
OpEntryPoint TessellationControl %main "main"
OpExecutionMode %main SpacingFractionalOdd
OpExecutionMode %main SpacingFractionalEven
)" + kVoidFunction;

  CompileSuccessfully(spirv);
  EXPECT_THAT(SPV_ERROR_INVALID_DATA, ValidateInstructions());
  EXPECT_THAT(getDiagnosticString(),
              HasSubstr("Tessellation execution model entry points can specify "
                        "at most one of SpacingEqual, SpacingFractionalOdd or "
                        "SpacingFractionalEven execution modes."));
}

TEST_F(ValidateMode, TessellationControlAllSpacing) {
  const std::string spirv = R"(
OpCapability Tessellation
OpMemoryModel Logical GLSL450
OpEntryPoint TessellationControl %main "main"
OpExecutionMode %main SpacingEqual
OpExecutionMode %main SpacingFractionalOdd
OpExecutionMode %main SpacingFractionalEven
)" + kVoidFunction;

  CompileSuccessfully(spirv);
  EXPECT_THAT(SPV_ERROR_INVALID_DATA, ValidateInstructions());
  EXPECT_THAT(getDiagnosticString(),
              HasSubstr("Tessellation execution model entry points can specify "
                        "at most one of SpacingEqual, SpacingFractionalOdd or "
                        "SpacingFractionalEven execution modes."));
}

TEST_F(ValidateMode,
       TessellationEvaluationSpacingEqualAndSpacingFractionalOdd) {
  const std::string spirv = R"(
OpCapability Tessellation
OpMemoryModel Logical GLSL450
OpEntryPoint TessellationEvaluation %main "main"
OpExecutionMode %main SpacingEqual
OpExecutionMode %main SpacingFractionalOdd
)" + kVoidFunction;

  CompileSuccessfully(spirv);
  EXPECT_THAT(SPV_ERROR_INVALID_DATA, ValidateInstructions());
  EXPECT_THAT(getDiagnosticString(),
              HasSubstr("Tessellation execution model entry points can specify "
                        "at most one of SpacingEqual, SpacingFractionalOdd or "
                        "SpacingFractionalEven execution modes."));
}

TEST_F(ValidateMode,
       TessellationEvaluationSpacingEqualAndSpacingFractionalEven) {
  const std::string spirv = R"(
OpCapability Tessellation
OpMemoryModel Logical GLSL450
OpEntryPoint TessellationEvaluation %main "main"
OpExecutionMode %main SpacingEqual
OpExecutionMode %main SpacingFractionalEven
)" + kVoidFunction;

  CompileSuccessfully(spirv);
  EXPECT_THAT(SPV_ERROR_INVALID_DATA, ValidateInstructions());
  EXPECT_THAT(getDiagnosticString(),
              HasSubstr("Tessellation execution model entry points can specify "
                        "at most one of SpacingEqual, SpacingFractionalOdd or "
                        "SpacingFractionalEven execution modes."));
}

TEST_F(ValidateMode,
       TessellationEvaluationSpacingFractionalOddAndSpacingFractionalEven) {
  const std::string spirv = R"(
OpCapability Tessellation
OpMemoryModel Logical GLSL450
OpEntryPoint TessellationEvaluation %main "main"
OpExecutionMode %main SpacingFractionalOdd
OpExecutionMode %main SpacingFractionalEven
)" + kVoidFunction;

  CompileSuccessfully(spirv);
  EXPECT_THAT(SPV_ERROR_INVALID_DATA, ValidateInstructions());
  EXPECT_THAT(getDiagnosticString(),
              HasSubstr("Tessellation execution model entry points can specify "
                        "at most one of SpacingEqual, SpacingFractionalOdd or "
                        "SpacingFractionalEven execution modes."));
}

TEST_F(ValidateMode, TessellationEvaluationAllSpacing) {
  const std::string spirv = R"(
OpCapability Tessellation
OpMemoryModel Logical GLSL450
OpEntryPoint TessellationEvaluation %main "main"
OpExecutionMode %main SpacingEqual
OpExecutionMode %main SpacingFractionalOdd
OpExecutionMode %main SpacingFractionalEven
)" + kVoidFunction;

  CompileSuccessfully(spirv);
  EXPECT_THAT(SPV_ERROR_INVALID_DATA, ValidateInstructions());
  EXPECT_THAT(getDiagnosticString(),
              HasSubstr("Tessellation execution model entry points can specify "
                        "at most one of SpacingEqual, SpacingFractionalOdd or "
                        "SpacingFractionalEven execution modes."));
}

TEST_F(ValidateMode, TessellationControlBothVertex) {
  const std::string spirv = R"(
OpCapability Tessellation
OpMemoryModel Logical GLSL450
OpEntryPoint TessellationControl %main "main"
OpExecutionMode %main VertexOrderCw
OpExecutionMode %main VertexOrderCcw
)" + kVoidFunction;

  CompileSuccessfully(spirv);
  EXPECT_THAT(SPV_ERROR_INVALID_DATA, ValidateInstructions());
  EXPECT_THAT(
      getDiagnosticString(),
      HasSubstr("Tessellation execution model entry points can specify at most "
                "one of VertexOrderCw or VertexOrderCcw execution modes."));
}

TEST_F(ValidateMode, TessellationEvaluationBothVertex) {
  const std::string spirv = R"(
OpCapability Tessellation
OpMemoryModel Logical GLSL450
OpEntryPoint TessellationEvaluation %main "main"
OpExecutionMode %main VertexOrderCw
OpExecutionMode %main VertexOrderCcw
)" + kVoidFunction;

  CompileSuccessfully(spirv);
  EXPECT_THAT(SPV_ERROR_INVALID_DATA, ValidateInstructions());
  EXPECT_THAT(
      getDiagnosticString(),
      HasSubstr("Tessellation execution model entry points can specify at most "
                "one of VertexOrderCw or VertexOrderCcw execution modes."));
}

using ValidateModeGeometry = spvtest::ValidateBase<std::tuple<
    std::tuple<std::string, std::string, std::string, std::string, std::string>,
    std::tuple<std::string, std::string, std::string>>>;

TEST_P(ValidateModeGeometry, ExecutionMode) {
  std::vector<std::string> input_modes;
  std::vector<std::string> output_modes;
  input_modes.push_back(std::get<0>(std::get<0>(GetParam())));
  input_modes.push_back(std::get<1>(std::get<0>(GetParam())));
  input_modes.push_back(std::get<2>(std::get<0>(GetParam())));
  input_modes.push_back(std::get<3>(std::get<0>(GetParam())));
  input_modes.push_back(std::get<4>(std::get<0>(GetParam())));
  output_modes.push_back(std::get<0>(std::get<1>(GetParam())));
  output_modes.push_back(std::get<1>(std::get<1>(GetParam())));
  output_modes.push_back(std::get<2>(std::get<1>(GetParam())));

  std::ostringstream sstr;
  sstr << "OpCapability Geometry\n";
  sstr << "OpMemoryModel Logical GLSL450\n";
  sstr << "OpEntryPoint Geometry %main \"main\"\n";
  size_t num_input_modes = 0;
  for (auto input : input_modes) {
    if (!input.empty()) {
      num_input_modes++;
      sstr << "OpExecutionMode %main " << input << "\n";
    }
  }
  size_t num_output_modes = 0;
  for (auto output : output_modes) {
    if (!output.empty()) {
      num_output_modes++;
      sstr << "OpExecutionMode %main " << output << "\n";
    }
  }
  sstr << "%void = OpTypeVoid\n";
  sstr << "%void_fn = OpTypeFunction %void\n";
  sstr << "%int = OpTypeInt 32 0\n";
  sstr << "%int1 = OpConstant %int 1\n";
  sstr << "%main = OpFunction %void None %void_fn\n";
  sstr << "%entry = OpLabel\n";
  sstr << "OpReturn\n";
  sstr << "OpFunctionEnd\n";

  CompileSuccessfully(sstr.str());
  if (num_input_modes == 1 && num_output_modes == 1) {
    EXPECT_THAT(SPV_SUCCESS, ValidateInstructions());
  } else {
    EXPECT_THAT(SPV_ERROR_INVALID_DATA, ValidateInstructions());
    if (num_input_modes != 1) {
      EXPECT_THAT(getDiagnosticString(),
                  HasSubstr("Geometry execution model entry points must "
                            "specify exactly one of InputPoints, InputLines, "
                            "InputLinesAdjacency, Triangles or "
                            "InputTrianglesAdjacency execution modes."));
    } else {
      EXPECT_THAT(
          getDiagnosticString(),
          HasSubstr("Geometry execution model entry points must specify "
                    "exactly one of OutputPoints, OutputLineStrip or "
                    "OutputTriangleStrip execution modes."));
    }
  }
}

INSTANTIATE_TEST_SUITE_P(
    GeometryRequiredModes, ValidateModeGeometry,
    Combine(Combine(Values("InputPoints", ""), Values("InputLines", ""),
                    Values("InputLinesAdjacency", ""), Values("Triangles", ""),
                    Values("InputTrianglesAdjacency", "")),
            Combine(Values("OutputPoints", ""), Values("OutputLineStrip", ""),
                    Values("OutputTriangleStrip", ""))));

using ValidateModeExecution =
    spvtest::ValidateBase<std::tuple<spv_result_t, std::string, std::string,
                                     std::string, spv_target_env>>;

TEST_P(ValidateModeExecution, ExecutionMode) {
  const spv_result_t expectation = std::get<0>(GetParam());
  const std::string error = std::get<1>(GetParam());
  const std::string model = std::get<2>(GetParam());
  const std::string mode = std::get<3>(GetParam());
  const spv_target_env env = std::get<4>(GetParam());

  std::ostringstream sstr;
  sstr << "OpCapability Shader\n";
  sstr << "OpCapability Geometry\n";
  sstr << "OpCapability Tessellation\n";
  sstr << "OpCapability TransformFeedback\n";
  if (!spvIsVulkanEnv(env)) {
    sstr << "OpCapability Kernel\n";
    if (env == SPV_ENV_UNIVERSAL_1_3) {
      sstr << "OpCapability SubgroupDispatch\n";
    } else if (env == SPV_ENV_UNIVERSAL_1_5) {
      sstr << "OpCapability TileImageColorReadAccessEXT\n";
      sstr << "OpCapability TileImageDepthReadAccessEXT\n";
      sstr << "OpCapability TileImageStencilReadAccessEXT\n";
      sstr << "OpExtension \"SPV_EXT_shader_tile_image\"\n";
    }
  }
  sstr << "OpMemoryModel Logical GLSL450\n";
  sstr << "OpEntryPoint " << model << " %main \"main\"\n";
  if (mode.find("LocalSizeId") == 0 || mode.find("LocalSizeHintId") == 0 ||
      mode.find("SubgroupsPerWorkgroupId") == 0) {
    sstr << "OpExecutionModeId %main " << mode << "\n";
  } else {
    sstr << "OpExecutionMode %main " << mode << "\n";
  }
  if (model == "Geometry") {
    if (!(mode.find("InputPoints") == 0 || mode.find("InputLines") == 0 ||
          mode.find("InputLinesAdjacency") == 0 ||
          mode.find("Triangles") == 0 ||
          mode.find("InputTrianglesAdjacency") == 0)) {
      // Exactly one of the above modes is required for Geometry shaders.
      sstr << "OpExecutionMode %main InputPoints\n";
    }
    if (!(mode.find("OutputPoints") == 0 || mode.find("OutputLineStrip") == 0 ||
          mode.find("OutputTriangleStrip") == 0)) {
      // Exactly one of the above modes is required for Geometry shaders.
      sstr << "OpExecutionMode %main OutputPoints\n";
    }
  } else if (model == "Fragment") {
    if (!(mode.find("OriginUpperLeft") == 0 ||
          mode.find("OriginLowerLeft") == 0)) {
      // Exactly one of the above modes is required for Fragment shaders.
      sstr << "OpExecutionMode %main OriginUpperLeft\n";
    }
  }
  sstr << "%void = OpTypeVoid\n";
  sstr << "%void_fn = OpTypeFunction %void\n";
  sstr << "%int = OpTypeInt 32 0\n";
  sstr << "%int1 = OpConstant %int 1\n";
  sstr << "%main = OpFunction %void None %void_fn\n";
  sstr << "%entry = OpLabel\n";
  sstr << "OpReturn\n";
  sstr << "OpFunctionEnd\n";

  CompileSuccessfully(sstr.str(), env);
  EXPECT_THAT(expectation, ValidateInstructions(env));
  if (expectation != SPV_SUCCESS) {
    EXPECT_THAT(getDiagnosticString(), HasSubstr(error));
  }
}

INSTANTIATE_TEST_SUITE_P(
    ValidateModeGeometryOnlyGoodSpv10, ValidateModeExecution,
    Combine(Values(SPV_SUCCESS), Values(""), Values("Geometry"),
            Values("Invocations 3", "InputPoints", "InputLines",
                   "InputLinesAdjacency", "InputTrianglesAdjacency",
                   "OutputPoints", "OutputLineStrip", "OutputTriangleStrip"),
            Values(SPV_ENV_UNIVERSAL_1_0)));

INSTANTIATE_TEST_SUITE_P(
    ValidateModeGeometryOnlyBadSpv10, ValidateModeExecution,
    Combine(Values(SPV_ERROR_INVALID_DATA),
            Values("Execution mode can only be used with the Geometry "
                   "execution model."),
            Values("Fragment", "TessellationEvaluation", "TessellationControl",
                   "GLCompute", "Vertex", "Kernel"),
            Values("Invocations 3", "InputPoints", "InputLines",
                   "InputLinesAdjacency", "InputTrianglesAdjacency",
                   "OutputPoints", "OutputLineStrip", "OutputTriangleStrip"),
            Values(SPV_ENV_UNIVERSAL_1_0)));

INSTANTIATE_TEST_SUITE_P(
    ValidateModeTessellationOnlyGoodSpv10, ValidateModeExecution,
    Combine(Values(SPV_SUCCESS), Values(""),
            Values("TessellationControl", "TessellationEvaluation"),
            Values("SpacingEqual", "SpacingFractionalEven",
                   "SpacingFractionalOdd", "VertexOrderCw", "VertexOrderCcw",
                   "PointMode", "Quads", "Isolines"),
            Values(SPV_ENV_UNIVERSAL_1_0)));

INSTANTIATE_TEST_SUITE_P(
    ValidateModeTessellationOnlyBadSpv10, ValidateModeExecution,
    Combine(Values(SPV_ERROR_INVALID_DATA),
            Values("Execution mode can only be used with a tessellation "
                   "execution model."),
            Values("Fragment", "Geometry", "GLCompute", "Vertex", "Kernel"),
            Values("SpacingEqual", "SpacingFractionalEven",
                   "SpacingFractionalOdd", "VertexOrderCw", "VertexOrderCcw",
                   "PointMode", "Quads", "Isolines"),
            Values(SPV_ENV_UNIVERSAL_1_0)));

INSTANTIATE_TEST_SUITE_P(ValidateModeGeometryAndTessellationGoodSpv10,
                         ValidateModeExecution,
                         Combine(Values(SPV_SUCCESS), Values(""),
                                 Values("TessellationControl",
                                        "TessellationEvaluation", "Geometry"),
                                 Values("Triangles", "OutputVertices 3"),
                                 Values(SPV_ENV_UNIVERSAL_1_0)));

INSTANTIATE_TEST_SUITE_P(
    ValidateModeGeometryAndTessellationBadSpv10, ValidateModeExecution,
    Combine(Values(SPV_ERROR_INVALID_DATA),
            Values("Execution mode can only be used with a Geometry or "
                   "tessellation execution model."),
            Values("Fragment", "GLCompute", "Vertex", "Kernel"),
            Values("Triangles", "OutputVertices 3"),
            Values(SPV_ENV_UNIVERSAL_1_0)));

INSTANTIATE_TEST_SUITE_P(
    ValidateModeFragmentOnlyGoodSpv10, ValidateModeExecution,
    Combine(Values(SPV_SUCCESS), Values(""), Values("Fragment"),
            Values("PixelCenterInteger", "OriginUpperLeft", "OriginLowerLeft",
                   "EarlyFragmentTests", "DepthReplacing", "DepthLess",
                   "DepthUnchanged"),
            Values(SPV_ENV_UNIVERSAL_1_0)));

INSTANTIATE_TEST_SUITE_P(
    ValidateModeFragmentOnlyBadSpv10, ValidateModeExecution,
    Combine(Values(SPV_ERROR_INVALID_DATA),
            Values("Execution mode can only be used with the Fragment "
                   "execution model."),
            Values("Geometry", "TessellationControl", "TessellationEvaluation",
                   "GLCompute", "Vertex", "Kernel"),
            Values("PixelCenterInteger", "OriginUpperLeft", "OriginLowerLeft",
                   "EarlyFragmentTests", "DepthReplacing", "DepthGreater",
                   "DepthLess", "DepthUnchanged"),
            Values(SPV_ENV_UNIVERSAL_1_0)));

INSTANTIATE_TEST_SUITE_P(ValidateModeFragmentOnlyGoodSpv15,
                         ValidateModeExecution,
                         Combine(Values(SPV_SUCCESS), Values(""),
                                 Values("Fragment"),
                                 Values("NonCoherentColorAttachmentReadEXT",
                                        "NonCoherentDepthAttachmentReadEXT",
                                        "NonCoherentStencilAttachmentReadEXT"),
                                 Values(SPV_ENV_UNIVERSAL_1_5)));

INSTANTIATE_TEST_SUITE_P(
    ValidateModeFragmentOnlyBadSpv15, ValidateModeExecution,
    Combine(Values(SPV_ERROR_INVALID_DATA),
            Values("Execution mode can only be used with the Fragment "
                   "execution model."),
            Values("Geometry", "TessellationControl", "TessellationEvaluation",
                   "GLCompute", "Vertex", "Kernel"),
            Values("NonCoherentColorAttachmentReadEXT",
                   "NonCoherentDepthAttachmentReadEXT",
                   "NonCoherentStencilAttachmentReadEXT"),
            Values(SPV_ENV_UNIVERSAL_1_5)));

INSTANTIATE_TEST_SUITE_P(ValidateModeKernelOnlyGoodSpv13, ValidateModeExecution,
                         Combine(Values(SPV_SUCCESS), Values(""),
                                 Values("Kernel"),
                                 Values("LocalSizeHint 1 1 1", "VecTypeHint 4",
                                        "ContractionOff",
                                        "LocalSizeHintId %int1 %int1 %int1"),
                                 Values(SPV_ENV_UNIVERSAL_1_3)));

INSTANTIATE_TEST_SUITE_P(
    ValidateModeKernelOnlyBadSpv13, ValidateModeExecution,
    Combine(
        Values(SPV_ERROR_INVALID_DATA),
        Values(
            "Execution mode can only be used with the Kernel execution model."),
        Values("Geometry", "TessellationControl", "TessellationEvaluation",
               "GLCompute", "Vertex", "Fragment"),
        Values("LocalSizeHint 1 1 1", "VecTypeHint 4", "ContractionOff",
               "LocalSizeHintId %int1 %int1 %int1"),
        Values(SPV_ENV_UNIVERSAL_1_3)));

INSTANTIATE_TEST_SUITE_P(
    ValidateModeGLComputeAndKernelGoodSpv13, ValidateModeExecution,
    Combine(Values(SPV_SUCCESS), Values(""), Values("Kernel", "GLCompute"),
            Values("LocalSize 1 1 1", "LocalSizeId %int1 %int1 %int1"),
            Values(SPV_ENV_UNIVERSAL_1_3)));

INSTANTIATE_TEST_SUITE_P(
    ValidateModeGLComputeAndKernelBadSpv13, ValidateModeExecution,
    Combine(Values(SPV_ERROR_INVALID_DATA),
            Values("Execution mode can only be used with a Kernel or GLCompute "
                   "execution model."),
            Values("Geometry", "TessellationControl", "TessellationEvaluation",
                   "Fragment", "Vertex"),
            Values("LocalSize 1 1 1", "LocalSizeId %int1 %int1 %int1"),
            Values(SPV_ENV_UNIVERSAL_1_3)));

INSTANTIATE_TEST_SUITE_P(
    ValidateModeAllGoodSpv13, ValidateModeExecution,
    Combine(Values(SPV_SUCCESS), Values(""),
            Values("Kernel", "GLCompute", "Geometry", "TessellationControl",
                   "TessellationEvaluation", "Fragment", "Vertex"),
            Values("Xfb", "Initializer", "Finalizer", "SubgroupSize 1",
                   "SubgroupsPerWorkgroup 1", "SubgroupsPerWorkgroupId %int1"),
            Values(SPV_ENV_UNIVERSAL_1_3)));

TEST_F(ValidateModeExecution, MeshNVLocalSize) {
  const std::string spirv = R"(
OpCapability Shader
OpCapability MeshShadingNV
OpExtension "SPV_NV_mesh_shader"
OpMemoryModel Logical GLSL450
OpEntryPoint MeshNV %main "main"
OpExecutionMode %main LocalSize 1 1 1
)" + kVoidFunction;

  CompileSuccessfully(spirv);
  EXPECT_THAT(SPV_SUCCESS, ValidateInstructions());
}

TEST_F(ValidateModeExecution, TaskNVLocalSize) {
  const std::string spirv = R"(
OpCapability Shader
OpCapability MeshShadingNV
OpExtension "SPV_NV_mesh_shader"
OpMemoryModel Logical GLSL450
OpEntryPoint TaskNV %main "main"
OpExecutionMode %main LocalSize 1 1 1
)" + kVoidFunction;

  CompileSuccessfully(spirv);
  EXPECT_THAT(SPV_SUCCESS, ValidateInstructions());
}

TEST_F(ValidateModeExecution, MeshNVOutputPoints) {
  const std::string spirv = R"(
OpCapability Shader
OpCapability MeshShadingNV
OpExtension "SPV_NV_mesh_shader"
OpMemoryModel Logical GLSL450
OpEntryPoint MeshNV %main "main"
OpExecutionMode %main OutputPoints
)" + kVoidFunction;

  CompileSuccessfully(spirv);
  EXPECT_THAT(SPV_SUCCESS, ValidateInstructions());
}

TEST_F(ValidateModeExecution, MeshEXTOutputVertices) {
  const std::string spirv = R"(
OpCapability MeshShadingEXT
OpExtension "SPV_EXT_mesh_shader"
%1 = OpExtInstImport "GLSL.std.450"
OpMemoryModel Logical GLSL450
OpEntryPoint MeshEXT %main "main"
OpExecutionMode %main LocalSize 1 1 1
OpExecutionMode %main OutputVertices 3
OpExecutionMode %main OutputPrimitivesNV 1
OpExecutionMode %main OutputTrianglesNV
OpSource GLSL 460
OpSourceExtension "GL_EXT_mesh_shader"
OpName %main "main"
OpDecorate %gl_WorkGroupSize BuiltIn WorkgroupSize
%void = OpTypeVoid
%3 = OpTypeFunction %void
%uint = OpTypeInt 32 0
%uint_3 = OpConstant %uint 3
%uint_1 = OpConstant %uint 1
%v3uint = OpTypeVector %uint 3
%gl_WorkGroupSize = OpConstantComposite %v3uint %uint_1 %uint_1 %uint_1
%main = OpFunction %void None %3
%5 = OpLabel
OpSetMeshOutputsEXT %uint_3 %uint_1
OpReturn
OpFunctionEnd
)";

  CompileSuccessfully(spirv, SPV_ENV_UNIVERSAL_1_4);
  EXPECT_THAT(SPV_SUCCESS, ValidateInstructions(SPV_ENV_UNIVERSAL_1_4));
}

TEST_F(ValidateModeExecution, VulkanBadMeshEXTOutputVertices) {
  const std::string spirv = R"(
OpCapability MeshShadingEXT
OpExtension "SPV_EXT_mesh_shader"
%1 = OpExtInstImport "GLSL.std.450"
OpMemoryModel Logical GLSL450
OpEntryPoint MeshEXT %main "main"
OpExecutionMode %main LocalSize 1 1 1
OpExecutionMode %main OutputVertices 0
OpExecutionMode %main OutputPrimitivesNV 1
OpExecutionMode %main OutputTrianglesNV
OpSource GLSL 460
OpSourceExtension "GL_EXT_mesh_shader"
OpName %main "main"
OpDecorate %gl_WorkGroupSize BuiltIn WorkgroupSize
%void = OpTypeVoid
%3 = OpTypeFunction %void
%uint = OpTypeInt 32 0
%uint_3 = OpConstant %uint 3
%uint_1 = OpConstant %uint 1
%v3uint = OpTypeVector %uint 3
%gl_WorkGroupSize = OpConstantComposite %v3uint %uint_1 %uint_1 %uint_1
%main = OpFunction %void None %3
%5 = OpLabel
OpSetMeshOutputsEXT %uint_3 %uint_1
OpReturn
OpFunctionEnd
)";

  CompileSuccessfully(spirv, SPV_ENV_VULKAN_1_2);
  EXPECT_THAT(SPV_ERROR_INVALID_DATA, ValidateInstructions(SPV_ENV_VULKAN_1_2));
  EXPECT_THAT(getDiagnosticString(),
              AnyVUID("VUID-StandaloneSpirv-MeshEXT-07330"));
}

TEST_F(ValidateModeExecution, VulkanBadMeshEXTOutputOutputPrimitivesEXT) {
  const std::string spirv = R"(
OpCapability MeshShadingEXT
OpExtension "SPV_EXT_mesh_shader"
%1 = OpExtInstImport "GLSL.std.450"
OpMemoryModel Logical GLSL450
OpEntryPoint MeshEXT %main "main"
OpExecutionMode %main LocalSize 1 1 1
OpExecutionMode %main OutputVertices 1
OpExecutionMode %main OutputPrimitivesNV 0
OpExecutionMode %main OutputTrianglesNV
OpSource GLSL 460
OpSourceExtension "GL_EXT_mesh_shader"
OpName %main "main"
OpDecorate %gl_WorkGroupSize BuiltIn WorkgroupSize
%void = OpTypeVoid
%3 = OpTypeFunction %void
%uint = OpTypeInt 32 0
%uint_3 = OpConstant %uint 3
%uint_1 = OpConstant %uint 1
%v3uint = OpTypeVector %uint 3
%gl_WorkGroupSize = OpConstantComposite %v3uint %uint_1 %uint_1 %uint_1
%main = OpFunction %void None %3
%5 = OpLabel
OpSetMeshOutputsEXT %uint_3 %uint_1
OpReturn
OpFunctionEnd
)";

  CompileSuccessfully(spirv, SPV_ENV_VULKAN_1_2);
  EXPECT_THAT(SPV_ERROR_INVALID_DATA, ValidateInstructions(SPV_ENV_VULKAN_1_2));
  EXPECT_THAT(getDiagnosticString(),
              AnyVUID("VUID-StandaloneSpirv-MeshEXT-07331"));
}

TEST_F(ValidateModeExecution, MeshNVOutputVertices) {
  const std::string spirv = R"(
OpCapability Shader
OpCapability MeshShadingNV
OpExtension "SPV_NV_mesh_shader"
OpMemoryModel Logical GLSL450
OpEntryPoint MeshNV %main "main"
OpExecutionMode %main OutputVertices 42
)" + kVoidFunction;

  CompileSuccessfully(spirv);
  EXPECT_THAT(SPV_SUCCESS, ValidateInstructions());
}

TEST_F(ValidateModeExecution, MeshNVLocalSizeId) {
  const std::string spirv = R"(
OpCapability Shader
OpCapability MeshShadingNV
OpExtension "SPV_NV_mesh_shader"
OpMemoryModel Logical GLSL450
OpEntryPoint MeshNV %main "main"
OpExecutionModeId %main LocalSizeId %int_1 %int_1 %int_1
%int = OpTypeInt 32 0
%int_1 = OpConstant %int 1
)" + kVoidFunction;

  spv_target_env env = SPV_ENV_UNIVERSAL_1_3;
  CompileSuccessfully(spirv, env);
  EXPECT_THAT(SPV_SUCCESS, ValidateInstructions(env));
}

TEST_F(ValidateModeExecution, TaskNVLocalSizeId) {
  const std::string spirv = R"(
OpCapability Shader
OpCapability MeshShadingNV
OpExtension "SPV_NV_mesh_shader"
OpMemoryModel Logical GLSL450
OpEntryPoint TaskNV %main "main"
OpExecutionModeId %main LocalSizeId %int_1 %int_1 %int_1
%int = OpTypeInt 32 0
%int_1 = OpConstant %int 1
)" + kVoidFunction;

  spv_target_env env = SPV_ENV_UNIVERSAL_1_3;
  CompileSuccessfully(spirv, env);
  EXPECT_THAT(SPV_SUCCESS, ValidateInstructions(env));
}

TEST_F(ValidateModeExecution, ExecModeSubgroupsPerWorkgroupIdBad) {
  const std::string spirv = R"(
OpCapability Shader
OpCapability SubgroupDispatch
OpMemoryModel Logical GLSL450
OpEntryPoint Vertex %main "main"
OpExecutionMode %main SubgroupsPerWorkgroupId %int_1
%int = OpTypeInt 32 0
%int_1 = OpConstant %int 1
)" + kVoidFunction;

  spv_target_env env = SPV_ENV_UNIVERSAL_1_3;
  CompileSuccessfully(spirv, env);
  EXPECT_THAT(SPV_ERROR_INVALID_DATA, ValidateInstructions(env));
  EXPECT_THAT(getDiagnosticString(),
              HasSubstr("OpExecutionMode is only valid when the Mode operand "
                        "is an execution mode that takes no Extra Operands"));
}

TEST_F(ValidateModeExecution, ExecModeIdSubgroupsPerWorkgroupIdGood) {
  const std::string spirv = R"(
OpCapability Shader
OpCapability SubgroupDispatch
OpMemoryModel Logical GLSL450
OpEntryPoint Vertex %main "main"
OpExecutionModeId %main SubgroupsPerWorkgroupId %int_1
%int = OpTypeInt 32 0
%int_1 = OpConstant %int 1
)" + kVoidFunction;

  spv_target_env env = SPV_ENV_UNIVERSAL_1_3;
  CompileSuccessfully(spirv, env);
  EXPECT_THAT(SPV_SUCCESS, ValidateInstructions(env));
}

TEST_F(ValidateModeExecution, ExecModeIdSubgroupsPerWorkgroupIdNonConstantBad) {
  const std::string spirv = R"(
OpCapability Shader
OpCapability SubgroupDispatch
OpMemoryModel Logical GLSL450
OpEntryPoint Vertex %main "main"
OpExecutionModeId %main SubgroupsPerWorkgroupId %int_1
%int = OpTypeInt 32 0
%int_ptr = OpTypePointer Private %int
%int_1 = OpVariable %int_ptr Private
)" + kVoidFunction;

  spv_target_env env = SPV_ENV_UNIVERSAL_1_3;
  CompileSuccessfully(spirv, env);
  EXPECT_THAT(SPV_ERROR_INVALID_ID, ValidateInstructions(env));
  EXPECT_THAT(getDiagnosticString(),
              HasSubstr("For OpExecutionModeId all Extra Operand ids must be "
                        "constant instructions."));
}

TEST_F(ValidateModeExecution, ExecModeLocalSizeHintIdBad) {
  const std::string spirv = R"(
OpCapability Kernel
OpCapability Shader
OpMemoryModel Logical GLSL450
OpEntryPoint Kernel %main "main"
OpExecutionMode %main LocalSizeHintId %int_1 %int_1 %int_1
%int = OpTypeInt 32 0
%int_1 = OpConstant %int 1
)" + kVoidFunction;

  spv_target_env env = SPV_ENV_UNIVERSAL_1_3;
  CompileSuccessfully(spirv, env);
  EXPECT_THAT(SPV_ERROR_INVALID_DATA, ValidateInstructions(env));
  EXPECT_THAT(getDiagnosticString(),
              HasSubstr("OpExecutionMode is only valid when the Mode operand "
                        "is an execution mode that takes no Extra Operands"));
}

TEST_F(ValidateModeExecution, ExecModeIdLocalSizeHintIdGood) {
  const std::string spirv = R"(
OpCapability Kernel
OpCapability Shader
OpMemoryModel Logical GLSL450
OpEntryPoint Kernel %main "main"
OpExecutionModeId %main LocalSizeHintId %int_1 %int_1 %int_1
%int = OpTypeInt 32 0
%int_1 = OpConstant %int 1
)" + kVoidFunction;

  spv_target_env env = SPV_ENV_UNIVERSAL_1_3;
  CompileSuccessfully(spirv, env);
  EXPECT_THAT(SPV_SUCCESS, ValidateInstructions(env));
}

TEST_F(ValidateModeExecution, ExecModeIdLocalSizeHintIdNonConstantBad) {
  const std::string spirv = R"(
OpCapability Kernel
OpCapability Shader
OpMemoryModel Logical GLSL450
OpEntryPoint Vertex %main "main"
OpExecutionModeId %main LocalSizeHintId %int_1 %int_1 %int_1
%int = OpTypeInt 32 0
%int_ptr = OpTypePointer Private %int
%int_1 = OpVariable %int_ptr Private
)" + kVoidFunction;

  spv_target_env env = SPV_ENV_UNIVERSAL_1_3;
  CompileSuccessfully(spirv, env);
  EXPECT_THAT(SPV_ERROR_INVALID_ID, ValidateInstructions(env));
  EXPECT_THAT(getDiagnosticString(),
              HasSubstr("For OpExecutionModeId all Extra Operand ids must be "
                        "constant instructions."));
}

TEST_F(ValidateModeExecution, ExecModeLocalSizeIdBad) {
  const std::string spirv = R"(
OpCapability Kernel
OpCapability Shader
OpMemoryModel Logical GLSL450
OpEntryPoint Kernel %main "main"
OpExecutionMode %main LocalSizeId %int_1 %int_1 %int_1
%int = OpTypeInt 32 0
%int_1 = OpConstant %int 1
)" + kVoidFunction;

  spv_target_env env = SPV_ENV_UNIVERSAL_1_3;
  CompileSuccessfully(spirv, env);
  EXPECT_THAT(SPV_ERROR_INVALID_DATA, ValidateInstructions(env));
  EXPECT_THAT(getDiagnosticString(),
              HasSubstr("OpExecutionMode is only valid when the Mode operand "
                        "is an execution mode that takes no Extra Operands"));
}

TEST_F(ValidateModeExecution, ExecModeIdLocalSizeIdGood) {
  const std::string spirv = R"(
OpCapability Kernel
OpCapability Shader
OpMemoryModel Logical GLSL450
OpEntryPoint Kernel %main "main"
OpExecutionModeId %main LocalSizeId %int_1 %int_1 %int_1
%int = OpTypeInt 32 0
%int_1 = OpConstant %int 1
)" + kVoidFunction;

  spv_target_env env = SPV_ENV_UNIVERSAL_1_3;
  CompileSuccessfully(spirv, env);
  EXPECT_THAT(SPV_SUCCESS, ValidateInstructions(env));
}

TEST_F(ValidateModeExecution, ExecModeIdLocalSizeIdNonConstantBad) {
  const std::string spirv = R"(
OpCapability Kernel
OpCapability Shader
OpMemoryModel Logical GLSL450
OpEntryPoint Vertex %main "main"
OpExecutionModeId %main LocalSizeId %int_1 %int_1 %int_1
%int = OpTypeInt 32 0
%int_ptr = OpTypePointer Private %int
%int_1 = OpVariable %int_ptr Private
)" + kVoidFunction;

  spv_target_env env = SPV_ENV_UNIVERSAL_1_3;
  CompileSuccessfully(spirv, env);
  EXPECT_THAT(SPV_ERROR_INVALID_ID, ValidateInstructions(env));
  EXPECT_THAT(getDiagnosticString(),
              HasSubstr("For OpExecutionModeId all Extra Operand ids must be "
                        "constant instructions."));
}

using AllowMultipleExecutionModes = spvtest::ValidateBase<std::string>;

TEST_P(AllowMultipleExecutionModes, DifferentOperand) {
  const std::string mode = GetParam();
  const std::string spirv = R"(
OpCapability Shader
OpCapability DenormPreserve
OpCapability DenormFlushToZero
OpCapability SignedZeroInfNanPreserve
OpCapability RoundingModeRTE
OpCapability RoundingModeRTZ
OpExtension "SPV_KHR_float_controls"
OpMemoryModel Logical GLSL450
OpEntryPoint GLCompute %main "main"
OpExecutionMode %main LocalSize 1 1 1
OpExecutionMode %main )" + mode +
                            R"( 16
OpExecutionMode %main )" + mode +
                            R"( 32
%void = OpTypeVoid
%void_fn = OpTypeFunction %void
%main = OpFunction %void None %void_fn
%entry = OpLabel
OpReturn
OpFunctionEnd
)";

  CompileSuccessfully(spirv);
  EXPECT_EQ(SPV_SUCCESS, ValidateInstructions());
}

TEST_P(AllowMultipleExecutionModes, SameOperand) {
  const std::string mode = GetParam();
  const std::string spirv = R"(
OpCapability Shader
OpCapability DenormPreserve
OpCapability DenormFlushToZero
OpCapability SignedZeroInfNanPreserve
OpCapability RoundingModeRTE
OpCapability RoundingModeRTZ
OpExtension "SPV_KHR_float_controls"
OpMemoryModel Logical GLSL450
OpEntryPoint GLCompute %main "main"
OpExecutionMode %main LocalSize 1 1 1
OpExecutionMode %main )" + mode +
                            R"( 32
OpExecutionMode %main )" + mode +
                            R"( 32
%void = OpTypeVoid
%void_fn = OpTypeFunction %void
%main = OpFunction %void None %void_fn
%entry = OpLabel
OpReturn
OpFunctionEnd
)";

  CompileSuccessfully(spirv);
  EXPECT_EQ(SPV_ERROR_INVALID_ID, ValidateInstructions());
  EXPECT_THAT(getDiagnosticString(),
              HasSubstr("execution mode must not be specified multiple times "
                        "for the same entry point and operands"));
}

INSTANTIATE_TEST_SUITE_P(MultipleFloatControlsExecModes,
                         AllowMultipleExecutionModes,
                         Values("DenormPreserve", "DenormFlushToZero",
                                "SignedZeroInfNanPreserve", "RoundingModeRTE",
                                "RoundingModeRTZ"));

using MultipleExecModes = spvtest::ValidateBase<std::string>;

TEST_P(MultipleExecModes, DuplicateMode) {
  const std::string mode = GetParam();
  const std::string spirv = R"(
OpCapability Shader
OpMemoryModel Logical GLSL450
OpEntryPoint Fragment %main "main"
OpExecutionMode %main OriginUpperLeft
OpExecutionMode %main )" + mode +
                            R"(
OpExecutionMode %main )" + mode +
                            R"(
%void = OpTypeVoid
%void_fn = OpTypeFunction %void
%main = OpFunction %void None %void_fn
%entry = OpLabel
OpReturn
OpFunctionEnd
)";

  CompileSuccessfully(spirv);
  EXPECT_EQ(SPV_ERROR_INVALID_ID, ValidateInstructions());
  EXPECT_THAT(getDiagnosticString(),
              HasSubstr("execution mode must not be specified multiple times "
                        "per entry point"));
}

INSTANTIATE_TEST_SUITE_P(MultipleFragmentExecMode, MultipleExecModes,
                         Values("DepthReplacing", "DepthGreater", "DepthLess",
                                "DepthUnchanged"));

TEST_F(ValidateMode, FloatControls2FPFastMathDefaultSameOperand) {
  const std::string spirv = R"(
OpCapability Shader
OpCapability FloatControls2
OpExtension "SPV_KHR_float_controls2"
OpMemoryModel Logical GLSL450
OpEntryPoint GLCompute %main "main"
OpExecutionMode %main LocalSize 1 1 1
OpExecutionModeId %main FPFastMathDefault %float %none
OpExecutionModeId %main FPFastMathDefault %float %none
%void = OpTypeVoid
%float = OpTypeFloat 32
%int = OpTypeInt 32 0
%none = OpConstant %int 0
%void_fn = OpTypeFunction %void
%main = OpFunction %void None %void_fn
%entry = OpLabel
OpReturn
OpFunctionEnd
)";

  CompileSuccessfully(spirv, SPV_ENV_UNIVERSAL_1_2);
  EXPECT_EQ(SPV_ERROR_INVALID_ID, ValidateInstructions(SPV_ENV_UNIVERSAL_1_2));
  EXPECT_THAT(getDiagnosticString(),
              HasSubstr("execution mode must not be specified multiple times "
                        "for the same entry point and operands"));
}

TEST_F(ValidateMode, FloatControls2FPFastMathDefaultDifferentOperand) {
  const std::string spirv = R"(
OpCapability Shader
OpCapability Float16
OpCapability FloatControls2
OpExtension "SPV_KHR_float_controls2"
OpMemoryModel Logical GLSL450
OpEntryPoint GLCompute %main "main"
OpExecutionMode %main LocalSize 1 1 1
OpExecutionModeId %main FPFastMathDefault %float %none
OpExecutionModeId %main FPFastMathDefault %half %none
%void = OpTypeVoid
%float = OpTypeFloat 32
%int = OpTypeInt 32 0
%none = OpConstant %int 0
%half = OpTypeFloat 16
%void_fn = OpTypeFunction %void
%main = OpFunction %void None %void_fn
%entry = OpLabel
OpReturn
OpFunctionEnd
)";

  CompileSuccessfully(spirv, SPV_ENV_UNIVERSAL_1_2);
  EXPECT_EQ(SPV_SUCCESS, ValidateInstructions(SPV_ENV_UNIVERSAL_1_2));
}

TEST_F(ValidateMode, FragmentShaderInterlockVertexBad) {
  const std::string spirv = R"(
OpCapability Shader
OpCapability FragmentShaderPixelInterlockEXT
OpExtension "SPV_EXT_fragment_shader_interlock"
OpMemoryModel Logical GLSL450
OpEntryPoint Vertex %main "main"
OpExecutionMode %main PixelInterlockOrderedEXT
)" + kVoidFunction;

  CompileSuccessfully(spirv);
  EXPECT_THAT(SPV_ERROR_INVALID_DATA, ValidateInstructions());
  EXPECT_THAT(
      getDiagnosticString(),
      HasSubstr(
          "Execution mode can only be used with the Fragment execution model"));
}

TEST_F(ValidateMode, FragmentShaderInterlockTooManyModesBad) {
  const std::string spirv = R"(
OpCapability Shader
OpCapability FragmentShaderPixelInterlockEXT
OpCapability FragmentShaderSampleInterlockEXT
OpExtension "SPV_EXT_fragment_shader_interlock"
OpMemoryModel Logical GLSL450
OpEntryPoint Fragment %main "main"
OpExecutionMode %main OriginUpperLeft
OpExecutionMode %main PixelInterlockOrderedEXT
OpExecutionMode %main SampleInterlockOrderedEXT
)" + kVoidFunction;

  CompileSuccessfully(spirv);
  EXPECT_THAT(SPV_ERROR_INVALID_DATA, ValidateInstructions());
  EXPECT_THAT(
      getDiagnosticString(),
      HasSubstr("Fragment execution model entry points can specify at most "
                "one fragment shader interlock execution mode"));
}

TEST_F(ValidateMode, FragmentShaderInterlockNoModeBad) {
  const std::string spirv = R"(
OpCapability Shader
OpCapability FragmentShaderPixelInterlockEXT
OpExtension "SPV_EXT_fragment_shader_interlock"
OpMemoryModel Logical GLSL450
OpEntryPoint Fragment %main "main"
OpExecutionMode %main OriginUpperLeft
%void = OpTypeVoid
%void_fn = OpTypeFunction %void
%func = OpFunction %void None %void_fn
%entryf = OpLabel
OpBeginInvocationInterlockEXT
OpEndInvocationInterlockEXT
OpReturn
OpFunctionEnd
%main = OpFunction %void None %void_fn
%entry = OpLabel
%1 = OpFunctionCall %void %func
OpReturn
OpFunctionEnd
)";

  CompileSuccessfully(spirv);
  EXPECT_THAT(SPV_ERROR_INVALID_ID, ValidateInstructions());
  EXPECT_THAT(
      getDiagnosticString(),
      HasSubstr(
          "OpBeginInvocationInterlockEXT/OpEndInvocationInterlockEXT require a "
          "fragment shader interlock execution mode"));
}

TEST_F(ValidateMode, FragmentShaderInterlockGood) {
  const std::string spirv = R"(
OpCapability Shader
OpCapability FragmentShaderPixelInterlockEXT
OpExtension "SPV_EXT_fragment_shader_interlock"
OpMemoryModel Logical GLSL450
OpEntryPoint Fragment %main "main"
OpExecutionMode %main OriginUpperLeft
OpExecutionMode %main PixelInterlockOrderedEXT
%void = OpTypeVoid
%void_fn = OpTypeFunction %void
%func = OpFunction %void None %void_fn
%entryf = OpLabel
OpBeginInvocationInterlockEXT
OpEndInvocationInterlockEXT
OpReturn
OpFunctionEnd
%main = OpFunction %void None %void_fn
%entry = OpLabel
%1 = OpFunctionCall %void %func
OpReturn
OpFunctionEnd
)";

  CompileSuccessfully(spirv);
  EXPECT_THAT(SPV_SUCCESS, ValidateInstructions());
}

TEST_F(ValidateMode, FragmentShaderStencilRefFrontTooManyModesBad) {
  const std::string spirv = R"(
OpCapability Shader
OpCapability StencilExportEXT
OpExtension "SPV_AMD_shader_early_and_late_fragment_tests"
OpExtension "SPV_EXT_shader_stencil_export"
OpMemoryModel Logical GLSL450
OpEntryPoint Fragment %main "main"
OpExecutionMode %main OriginUpperLeft
OpExecutionMode %main EarlyAndLateFragmentTestsAMD
OpExecutionMode %main StencilRefLessFrontAMD
OpExecutionMode %main StencilRefGreaterFrontAMD
)" + kVoidFunction;

  CompileSuccessfully(spirv);
  EXPECT_THAT(SPV_ERROR_INVALID_DATA, ValidateInstructions());
  EXPECT_THAT(
      getDiagnosticString(),
      HasSubstr("Fragment execution model entry points can specify at most "
                "one of StencilRefUnchangedFrontAMD, "
                "StencilRefLessFrontAMD or StencilRefGreaterFrontAMD "
                "execution modes."));
}

TEST_F(ValidateMode, FragmentShaderStencilRefBackTooManyModesBad) {
  const std::string spirv = R"(
OpCapability Shader
OpCapability StencilExportEXT
OpExtension "SPV_AMD_shader_early_and_late_fragment_tests"
OpExtension "SPV_EXT_shader_stencil_export"
OpMemoryModel Logical GLSL450
OpEntryPoint Fragment %main "main"
OpExecutionMode %main OriginUpperLeft
OpExecutionMode %main EarlyAndLateFragmentTestsAMD
OpExecutionMode %main StencilRefLessBackAMD
OpExecutionMode %main StencilRefGreaterBackAMD
)" + kVoidFunction;

  CompileSuccessfully(spirv);
  EXPECT_THAT(SPV_ERROR_INVALID_DATA, ValidateInstructions());
  EXPECT_THAT(
      getDiagnosticString(),
      HasSubstr("Fragment execution model entry points can specify at most "
                "one of StencilRefUnchangedBackAMD, "
                "StencilRefLessBackAMD or StencilRefGreaterBackAMD "
                "execution modes."));
}

TEST_F(ValidateMode, FragmentShaderStencilRefFrontGood) {
  const std::string spirv = R"(
OpCapability Shader
OpCapability StencilExportEXT
OpExtension "SPV_AMD_shader_early_and_late_fragment_tests"
OpExtension "SPV_EXT_shader_stencil_export"
OpMemoryModel Logical GLSL450
OpEntryPoint Fragment %main "main"
OpExecutionMode %main OriginUpperLeft
OpExecutionMode %main EarlyAndLateFragmentTestsAMD
OpExecutionMode %main StencilRefLessFrontAMD
)" + kVoidFunction;

  CompileSuccessfully(spirv);
  EXPECT_THAT(SPV_SUCCESS, ValidateInstructions());
}

TEST_F(ValidateMode, FragmentShaderStencilRefBackGood) {
  const std::string spirv = R"(
OpCapability Shader
OpCapability StencilExportEXT
OpExtension "SPV_AMD_shader_early_and_late_fragment_tests"
OpExtension "SPV_EXT_shader_stencil_export"
OpMemoryModel Logical GLSL450
OpEntryPoint Fragment %main "main"
OpExecutionMode %main OriginUpperLeft
OpExecutionMode %main EarlyAndLateFragmentTestsAMD
OpExecutionMode %main StencilRefLessBackAMD
)" + kVoidFunction;

  CompileSuccessfully(spirv);
  EXPECT_THAT(SPV_SUCCESS, ValidateInstructions());
}

TEST_F(ValidateMode, FragmentShaderDemoteVertexBad) {
  const std::string spirv = R"(
OpCapability Shader
OpCapability DemoteToHelperInvocationEXT
OpExtension "SPV_EXT_demote_to_helper_invocation"
OpMemoryModel Logical GLSL450
OpEntryPoint Vertex %main "main"
%bool = OpTypeBool
%void = OpTypeVoid
%void_fn = OpTypeFunction %void
%main = OpFunction %void None %void_fn
%entry = OpLabel
OpDemoteToHelperInvocationEXT
%1 = OpIsHelperInvocationEXT %bool
OpReturn
OpFunctionEnd
)";

  CompileSuccessfully(spirv);
  EXPECT_THAT(SPV_ERROR_INVALID_ID, ValidateInstructions());
  EXPECT_THAT(
      getDiagnosticString(),
      HasSubstr(
          "OpDemoteToHelperInvocationEXT requires Fragment execution model"));
  EXPECT_THAT(
      getDiagnosticString(),
      HasSubstr("OpIsHelperInvocationEXT requires Fragment execution model"));
}

TEST_F(ValidateMode, FragmentShaderDemoteGood) {
  const std::string spirv = R"(
OpCapability Shader
OpCapability DemoteToHelperInvocationEXT
OpExtension "SPV_EXT_demote_to_helper_invocation"
OpMemoryModel Logical GLSL450
OpEntryPoint Fragment %main "main"
OpExecutionMode %main OriginUpperLeft
%bool = OpTypeBool
%void = OpTypeVoid
%void_fn = OpTypeFunction %void
%main = OpFunction %void None %void_fn
%entry = OpLabel
OpDemoteToHelperInvocationEXT
%1 = OpIsHelperInvocationEXT %bool
OpReturn
OpFunctionEnd
)";

  CompileSuccessfully(spirv);
  EXPECT_THAT(SPV_SUCCESS, ValidateInstructions());
}

TEST_F(ValidateMode, FragmentShaderDemoteBadType) {
  const std::string spirv = R"(
OpCapability Shader
OpCapability DemoteToHelperInvocationEXT
OpExtension "SPV_EXT_demote_to_helper_invocation"
OpMemoryModel Logical GLSL450
OpEntryPoint Fragment %main "main"
OpExecutionMode %main OriginUpperLeft
%u32 = OpTypeInt 32 0
%void = OpTypeVoid
%void_fn = OpTypeFunction %void
%main = OpFunction %void None %void_fn
%entry = OpLabel
OpDemoteToHelperInvocationEXT
%1 = OpIsHelperInvocationEXT %u32
OpReturn
OpFunctionEnd
)";

  CompileSuccessfully(spirv);
  EXPECT_THAT(SPV_ERROR_INVALID_DATA, ValidateInstructions());
  EXPECT_THAT(getDiagnosticString(),
              HasSubstr("Expected bool scalar type as Result Type"));
}

TEST_F(ValidateMode, LocalSizeIdVulkan1p3DoesNotRequireOption) {
  const std::string spirv = R"(
OpCapability Shader
OpMemoryModel Logical GLSL450
OpEntryPoint GLCompute %main "main"
OpExecutionModeId %main LocalSizeId %int_1 %int_1 %int_1
%void = OpTypeVoid
%int = OpTypeInt 32 0
%int_1 = OpConstant %int 1
%void_fn = OpTypeFunction %void
%main = OpFunction %void None %void_fn
%entry = OpLabel
OpReturn
OpFunctionEnd
)";

  CompileSuccessfully(spirv, SPV_ENV_VULKAN_1_3);
  EXPECT_EQ(SPV_SUCCESS, ValidateInstructions(SPV_ENV_VULKAN_1_3));
}

TEST_F(ValidateMode, MaximalReconvergenceRequiresExtension) {
  const std::string spirv = R"(
OpCapability Shader
OpMemoryModel Logical GLSL450
OpEntryPoint GLCompute %main "main"
OpExecutionMode %main LocalSize 1 1 1
OpExecutionMode %main MaximallyReconvergesKHR
%void = OpTypeVoid
%void_fn = OpTypeFunction %void
%main = OpFunction %void None %void_fn
%entry = OpLabel
OpReturn
OpFunctionEnd
)";

  CompileSuccessfully(spirv);
  EXPECT_EQ(SPV_ERROR_MISSING_EXTENSION, ValidateInstructions());
  EXPECT_THAT(getDiagnosticString(),
              HasSubstr("(6023) requires one of these extensions: "
                        "SPV_KHR_maximal_reconvergence "));
}

TEST_F(ValidateMode, FPFastMathDefaultNotExecutionModeId) {
  const std::string spirv = R"(
OpCapability Shader
OpCapability FloatControls2
OpExtension "SPV_KHR_float_controls2"
OpMemoryModel Logical GLSL450
OpEntryPoint GLCompute %main "main"
OpExecutionMode %main LocalSize 1 1 1
OpExecutionMode %main FPFastMathDefault %int_0 %int_0
%void = OpTypeVoid
%int = OpTypeInt 32 0
%int_0 = OpConstant %int 0
%void_fn = OpTypeFunction %void
%main = OpFunction %void None %void_fn
%entry = OpLabel
OpReturn
OpFunctionEnd
)";

  CompileSuccessfully(spirv);
  EXPECT_EQ(SPV_ERROR_INVALID_DATA, ValidateInstructions());
  EXPECT_THAT(getDiagnosticString(),
              HasSubstr("OpExecutionMode is only valid when the Mode operand "
                        "is an execution mode that takes no Extra Operands, or "
                        "takes Extra Operands that are not id operands"));
}

TEST_F(ValidateMode, FPFastMathDefaultNotAType) {
  const std::string spirv = R"(
OpCapability Shader
OpCapability FloatControls2
OpExtension "SPV_KHR_float_controls2"
OpMemoryModel Logical GLSL450
OpEntryPoint GLCompute %main "main"
OpExecutionMode %main LocalSize 1 1 1
OpExecutionModeId %main FPFastMathDefault %int_0 %int_0
%void = OpTypeVoid
%int = OpTypeInt 32 0
%int_0 = OpConstant %int 0
%void_fn = OpTypeFunction %void
%main = OpFunction %void None %void_fn
%entry = OpLabel
OpReturn
OpFunctionEnd
)";

  CompileSuccessfully(spirv, SPV_ENV_UNIVERSAL_1_3);
  EXPECT_EQ(SPV_ERROR_INVALID_ID, ValidateInstructions(SPV_ENV_UNIVERSAL_1_3));
  EXPECT_THAT(
      getDiagnosticString(),
      HasSubstr(
          "The Target Type operand must be a floating-point scalar type"));
}

TEST_F(ValidateMode, FPFastMathDefaultNotAFloatType) {
  const std::string spirv = R"(
OpCapability Shader
OpCapability FloatControls2
OpExtension "SPV_KHR_float_controls2"
OpMemoryModel Logical GLSL450
OpEntryPoint GLCompute %main "main"
OpExecutionMode %main LocalSize 1 1 1
OpExecutionModeId %main FPFastMathDefault %int %int_0
%void = OpTypeVoid
%int = OpTypeInt 32 0
%int_0 = OpConstant %int 0
%void_fn = OpTypeFunction %void
%main = OpFunction %void None %void_fn
%entry = OpLabel
OpReturn
OpFunctionEnd
)";

  CompileSuccessfully(spirv, SPV_ENV_UNIVERSAL_1_3);
  EXPECT_EQ(SPV_ERROR_INVALID_ID, ValidateInstructions(SPV_ENV_UNIVERSAL_1_3));
  EXPECT_THAT(
      getDiagnosticString(),
      HasSubstr(
          "The Target Type operand must be a floating-point scalar type"));
}

TEST_F(ValidateMode, FPFastMathDefaultNotAFloatScalarType) {
  const std::string spirv = R"(
OpCapability Shader
OpCapability FloatControls2
OpExtension "SPV_KHR_float_controls2"
OpMemoryModel Logical GLSL450
OpEntryPoint GLCompute %main "main"
OpExecutionMode %main LocalSize 1 1 1
OpExecutionModeId %main FPFastMathDefault %float2 %int_0
%void = OpTypeVoid
%int = OpTypeInt 32 0
%int_0 = OpConstant %int 0
%float = OpTypeFloat 32
%float2 = OpTypeVector %float 2
%void_fn = OpTypeFunction %void
%main = OpFunction %void None %void_fn
%entry = OpLabel
OpReturn
OpFunctionEnd
)";

  CompileSuccessfully(spirv, SPV_ENV_UNIVERSAL_1_3);
  EXPECT_EQ(SPV_ERROR_INVALID_ID, ValidateInstructions(SPV_ENV_UNIVERSAL_1_3));
  EXPECT_THAT(
      getDiagnosticString(),
      HasSubstr(
          "The Target Type operand must be a floating-point scalar type"));
}

TEST_F(ValidateMode, FPFastMathDefaultSpecConstant) {
  const std::string spirv = R"(
OpCapability Shader
OpCapability FloatControls2
OpExtension "SPV_KHR_float_controls2"
OpMemoryModel Logical GLSL450
OpEntryPoint GLCompute %main "main"
OpExecutionMode %main LocalSize 1 1 1
OpExecutionModeId %main FPFastMathDefault %float %int_0
%void = OpTypeVoid
%int = OpTypeInt 32 0
%int_0 = OpSpecConstant %int 0
%float = OpTypeFloat 32
%void_fn = OpTypeFunction %void
%main = OpFunction %void None %void_fn
%entry = OpLabel
OpReturn
OpFunctionEnd
)";

  CompileSuccessfully(spirv, SPV_ENV_UNIVERSAL_1_3);
  EXPECT_EQ(SPV_ERROR_INVALID_ID, ValidateInstructions(SPV_ENV_UNIVERSAL_1_3));
  EXPECT_THAT(getDiagnosticString(),
              HasSubstr("The Fast Math Default operand must be a "
                        "non-specialization constant"));
}

TEST_F(ValidateMode, FPFastMathDefaultInvalidMask) {
  const std::string spirv = R"(
OpCapability Shader
OpCapability FloatControls2
OpExtension "SPV_KHR_float_controls2"
OpMemoryModel Logical GLSL450
OpEntryPoint GLCompute %main "main"
OpExecutionMode %main LocalSize 1 1 1
OpExecutionModeId %main FPFastMathDefault %float %constant
%void = OpTypeVoid
%int = OpTypeInt 32 0
%constant = OpConstant %int 524288
%float = OpTypeFloat 32
%void_fn = OpTypeFunction %void
%main = OpFunction %void None %void_fn
%entry = OpLabel
OpReturn
OpFunctionEnd
)";

  CompileSuccessfully(spirv, SPV_ENV_UNIVERSAL_1_3);
  EXPECT_EQ(SPV_ERROR_INVALID_ID, ValidateInstructions(SPV_ENV_UNIVERSAL_1_3));
  EXPECT_THAT(
      getDiagnosticString(),
      HasSubstr("The Fast Math Default operand is an invalid bitmask value"));
}

TEST_F(ValidateMode, FPFastMathDefaultContainsFast) {
  const std::string spirv = R"(
OpCapability Shader
OpCapability FloatControls2
OpExtension "SPV_KHR_float_controls2"
OpMemoryModel Logical GLSL450
OpEntryPoint GLCompute %main "main"
OpExecutionMode %main LocalSize 1 1 1
OpExecutionModeId %main FPFastMathDefault %float %constant
%void = OpTypeVoid
%int = OpTypeInt 32 0
%constant = OpConstant %int 16
%float = OpTypeFloat 32
%void_fn = OpTypeFunction %void
%main = OpFunction %void None %void_fn
%entry = OpLabel
OpReturn
OpFunctionEnd
)";

  CompileSuccessfully(spirv, SPV_ENV_UNIVERSAL_1_3);
  EXPECT_EQ(SPV_ERROR_INVALID_ID, ValidateInstructions(SPV_ENV_UNIVERSAL_1_3));
  EXPECT_THAT(getDiagnosticString(),
              HasSubstr("The Fast Math Default operand must not include Fast"));
}

TEST_F(ValidateMode, FPFastMathDefaultAllowTransformMissingAllowReassoc) {
  const std::string spirv = R"(
OpCapability Shader
OpCapability FloatControls2
OpExtension "SPV_KHR_float_controls2"
OpMemoryModel Logical GLSL450
OpEntryPoint GLCompute %main "main"
OpExecutionMode %main LocalSize 1 1 1
OpExecutionModeId %main FPFastMathDefault %float %constant
%void = OpTypeVoid
%int = OpTypeInt 32 0
%constant = OpConstant %int 327680
%float = OpTypeFloat 32
%void_fn = OpTypeFunction %void
%main = OpFunction %void None %void_fn
%entry = OpLabel
OpReturn
OpFunctionEnd
)";

  CompileSuccessfully(spirv, SPV_ENV_UNIVERSAL_1_3);
  EXPECT_EQ(SPV_ERROR_INVALID_ID, ValidateInstructions(SPV_ENV_UNIVERSAL_1_3));
  EXPECT_THAT(
      getDiagnosticString(),
      HasSubstr("The Fast Math Default operand must include AllowContract and "
                "AllowReassoc when AllowTransform is specified"));
}

TEST_F(ValidateMode, FPFastMathDefaultAllowTransformMissingAllowContract) {
  const std::string spirv = R"(
OpCapability Shader
OpCapability FloatControls2
OpExtension "SPV_KHR_float_controls2"
OpMemoryModel Logical GLSL450
OpEntryPoint GLCompute %main "main"
OpExecutionMode %main LocalSize 1 1 1
OpExecutionModeId %main FPFastMathDefault %float %constant
%void = OpTypeVoid
%int = OpTypeInt 32 0
%constant = OpConstant %int 393216
%float = OpTypeFloat 32
%void_fn = OpTypeFunction %void
%main = OpFunction %void None %void_fn
%entry = OpLabel
OpReturn
OpFunctionEnd
)";

  CompileSuccessfully(spirv, SPV_ENV_UNIVERSAL_1_3);
  EXPECT_EQ(SPV_ERROR_INVALID_ID, ValidateInstructions(SPV_ENV_UNIVERSAL_1_3));
  EXPECT_THAT(
      getDiagnosticString(),
      HasSubstr("The Fast Math Default operand must include AllowContract and "
                "AllowReassoc when AllowTransform is specified"));
}

TEST_F(ValidateMode, FPFastMathDefaultAllowTransformMissingContractAndReassoc) {
  const std::string spirv = R"(
OpCapability Shader
OpCapability FloatControls2
OpExtension "SPV_KHR_float_controls2"
OpMemoryModel Logical GLSL450
OpEntryPoint GLCompute %main "main"
OpExecutionMode %main LocalSize 1 1 1
OpExecutionModeId %main FPFastMathDefault %float %constant
%void = OpTypeVoid
%int = OpTypeInt 32 0
%constant = OpConstant %int 262144
%float = OpTypeFloat 32
%void_fn = OpTypeFunction %void
%main = OpFunction %void None %void_fn
%entry = OpLabel
OpReturn
OpFunctionEnd
)";

  CompileSuccessfully(spirv, SPV_ENV_UNIVERSAL_1_3);
  EXPECT_EQ(SPV_ERROR_INVALID_ID, ValidateInstructions(SPV_ENV_UNIVERSAL_1_3));
  EXPECT_THAT(
      getDiagnosticString(),
      HasSubstr("The Fast Math Default operand must include AllowContract and "
                "AllowReassoc when AllowTransform is specified"));
}

TEST_F(ValidateMode, FPFastMathDefaultSignedZeroInfNanPreserve) {
  const std::string spirv = R"(
OpCapability Shader
OpCapability FloatControls2
OpCapability SignedZeroInfNanPreserve
OpExtension "SPV_KHR_float_controls2"
OpExtension "SPV_KHR_float_controls"
OpMemoryModel Logical GLSL450
OpEntryPoint GLCompute %main "main"
OpExecutionMode %main LocalSize 1 1 1
OpExecutionModeId %main FPFastMathDefault %float %constant
OpExecutionMode %main SignedZeroInfNanPreserve 32
%void = OpTypeVoid
%int = OpTypeInt 32 0
%constant = OpConstant %int 0
%float = OpTypeFloat 32
%void_fn = OpTypeFunction %void
%main = OpFunction %void None %void_fn
%entry = OpLabel
OpReturn
OpFunctionEnd
)";

  CompileSuccessfully(spirv, SPV_ENV_UNIVERSAL_1_3);
  EXPECT_EQ(SPV_ERROR_INVALID_DATA,
            ValidateInstructions(SPV_ENV_UNIVERSAL_1_3));
  EXPECT_THAT(
      getDiagnosticString(),
      HasSubstr("FPFastMathDefault and SignedZeroInfNanPreserve execution "
                "modes cannot be applied to the same entry point"));
}

TEST_F(ValidateMode, FPFastMathDefaultConractionOff) {
  const std::string spirv = R"(
OpCapability Kernel
OpCapability Addresses
OpCapability FloatControls2
OpCapability SignedZeroInfNanPreserve
OpExtension "SPV_KHR_float_controls2"
OpExtension "SPV_KHR_float_controls"
OpMemoryModel Physical64 OpenCL
OpEntryPoint Kernel %main "main"
OpExecutionModeId %main FPFastMathDefault %float %constant
OpExecutionMode %main ContractionOff
%void = OpTypeVoid
%int = OpTypeInt 32 0
%constant = OpConstant %int 0
%float = OpTypeFloat 32
%void_fn = OpTypeFunction %void
%main = OpFunction %void None %void_fn
%entry = OpLabel
OpReturn
OpFunctionEnd
)";

  CompileSuccessfully(spirv, SPV_ENV_UNIVERSAL_1_3);
  EXPECT_EQ(SPV_ERROR_INVALID_DATA,
            ValidateInstructions(SPV_ENV_UNIVERSAL_1_3));
  EXPECT_THAT(getDiagnosticString(),
              HasSubstr("FPFastMathDefault and ContractionOff execution modes "
                        "cannot be applied to the same entry point"));
}

TEST_F(ValidateMode, FPFastMathDefaultNoContractionNotInCallTree) {
  const std::string spirv = R"(
OpCapability Shader
OpCapability FloatControls2
OpExtension "SPV_KHR_float_controls2"
OpMemoryModel Logical GLSL450
OpEntryPoint GLCompute %main "main"
OpExecutionModeId %main FPFastMathDefault %float %constant
OpExecutionMode %main LocalSize 1 1 1
OpDecorate %add NoContraction
%void = OpTypeVoid
%int = OpTypeInt 32 0
%constant = OpConstant %int 0
%float = OpTypeFloat 32
%zero = OpConstant %float 0
%void_fn = OpTypeFunction %void
%main = OpFunction %void None %void_fn
%entry = OpLabel
OpReturn
OpFunctionEnd
%func = OpFunction %void None %void_fn
%func_entry = OpLabel
%add = OpFAdd %float %zero %zero
OpReturn
OpFunctionEnd
)";

  CompileSuccessfully(spirv, SPV_ENV_UNIVERSAL_1_3);
  EXPECT_EQ(SPV_SUCCESS, ValidateInstructions(SPV_ENV_UNIVERSAL_1_3));
}

TEST_F(ValidateMode, FPFastMathDefaultNoContractionInCallTree) {
  const std::string spirv = R"(
OpCapability Shader
OpCapability FloatControls2
OpExtension "SPV_KHR_float_controls2"
OpMemoryModel Logical GLSL450
OpEntryPoint GLCompute %main "main"
OpExecutionModeId %main FPFastMathDefault %float %constant
OpExecutionMode %main LocalSize 1 1 1
OpDecorate %add NoContraction
%void = OpTypeVoid
%int = OpTypeInt 32 0
%constant = OpConstant %int 0
%float = OpTypeFloat 32
%zero = OpConstant %float 0
%void_fn = OpTypeFunction %void
%main = OpFunction %void None %void_fn
%entry = OpLabel
%call = OpFunctionCall %void %func
OpReturn
OpFunctionEnd
%func = OpFunction %void None %void_fn
%func_entry = OpLabel
%add = OpFAdd %float %zero %zero
OpReturn
OpFunctionEnd
)";

  CompileSuccessfully(spirv, SPV_ENV_UNIVERSAL_1_3);
  EXPECT_EQ(SPV_ERROR_INVALID_DATA,
            ValidateInstructions(SPV_ENV_UNIVERSAL_1_3));
  EXPECT_THAT(getDiagnosticString(),
              HasSubstr("NoContraction cannot be used by an entry point with "
                        "the FPFastMathDefault execution mode"));
}

TEST_F(ValidateMode, FPFastMathDefaultNoContractionInCallTree2) {
  const std::string spirv = R"(
OpCapability Shader
OpCapability Kernel
OpCapability Addresses
OpCapability FloatControls2
OpExtension "SPV_KHR_float_controls2"
OpMemoryModel Physical64 OpenCL
OpEntryPoint Kernel %main "main"
OpExecutionModeId %main FPFastMathDefault %float %constant
OpDecorate %const NoContraction
%void = OpTypeVoid
%int = OpTypeInt 32 0
%constant = OpConstant %int 0
%float = OpTypeFloat 32
%zero = OpConstant %float 0
%const = OpSpecConstantOp %float FAdd %zero %zero
%void_fn = OpTypeFunction %void
%main = OpFunction %void None %void_fn
%entry = OpLabel
%call = OpFunctionCall %void %func
OpReturn
OpFunctionEnd
%func = OpFunction %void None %void_fn
%func_entry = OpLabel
%add = OpFAdd %float %const %zero
OpReturn
OpFunctionEnd
)";

  CompileSuccessfully(spirv, SPV_ENV_UNIVERSAL_1_3);
  EXPECT_EQ(SPV_ERROR_INVALID_DATA,
            ValidateInstructions(SPV_ENV_UNIVERSAL_1_3));
  EXPECT_THAT(getDiagnosticString(),
              HasSubstr("NoContraction cannot be used by an entry point with "
                        "the FPFastMathDefault execution mode"));
}

TEST_F(ValidateMode, FPFastMathDefaultFastMathFastNotInCallTree) {
  const std::string spirv = R"(
OpCapability Shader
OpCapability FloatControls2
OpExtension "SPV_KHR_float_controls2"
OpMemoryModel Logical GLSL450
OpEntryPoint GLCompute %main "main"
OpExecutionModeId %main FPFastMathDefault %float %constant
OpExecutionMode %main LocalSize 1 1 1
OpDecorate %add FPFastMathMode Fast
%void = OpTypeVoid
%int = OpTypeInt 32 0
%constant = OpConstant %int 0
%float = OpTypeFloat 32
%zero = OpConstant %float 0
%void_fn = OpTypeFunction %void
%main = OpFunction %void None %void_fn
%entry = OpLabel
OpReturn
OpFunctionEnd
%func = OpFunction %void None %void_fn
%func_entry = OpLabel
%add = OpFAdd %float %zero %zero
OpReturn
OpFunctionEnd
)";

  CompileSuccessfully(spirv, SPV_ENV_UNIVERSAL_1_3);
  EXPECT_EQ(SPV_SUCCESS, ValidateInstructions(SPV_ENV_UNIVERSAL_1_3));
}

TEST_F(ValidateMode, FPFastMathDefaultFastMathFastInCallTree) {
  const std::string spirv = R"(
OpCapability Shader
OpCapability FloatControls2
OpExtension "SPV_KHR_float_controls2"
OpMemoryModel Logical GLSL450
OpEntryPoint GLCompute %main "main"
OpExecutionModeId %main FPFastMathDefault %float %constant
OpExecutionMode %main LocalSize 1 1 1
OpDecorate %add FPFastMathMode Fast
%void = OpTypeVoid
%int = OpTypeInt 32 0
%constant = OpConstant %int 0
%float = OpTypeFloat 32
%zero = OpConstant %float 0
%void_fn = OpTypeFunction %void
%main = OpFunction %void None %void_fn
%entry = OpLabel
%call = OpFunctionCall %void %func
OpReturn
OpFunctionEnd
%func = OpFunction %void None %void_fn
%func_entry = OpLabel
%add = OpFAdd %float %zero %zero
OpReturn
OpFunctionEnd
)";

  CompileSuccessfully(spirv, SPV_ENV_UNIVERSAL_1_3);
  EXPECT_EQ(SPV_ERROR_INVALID_DATA,
            ValidateInstructions(SPV_ENV_UNIVERSAL_1_3));
  EXPECT_THAT(getDiagnosticString(),
              HasSubstr("FPFastMathMode Fast cannot be used by an entry point "
                        "with the FPFastMathDefault execution mode"));
}

TEST_F(ValidateMode, FPFastMathDefaultFastMathFastInCallTree2) {
  const std::string spirv = R"(
OpCapability Kernel
OpCapability Addresses
OpCapability FloatControls2
OpExtension "SPV_KHR_float_controls2"
OpMemoryModel Physical64 OpenCL
OpEntryPoint Kernel %main "main"
OpExecutionModeId %main FPFastMathDefault %float %constant
OpDecorate %const FPFastMathMode Fast
%void = OpTypeVoid
%int = OpTypeInt 32 0
%constant = OpConstant %int 0
%float = OpTypeFloat 32
%zero = OpConstant %float 0
%const = OpSpecConstantOp %float FAdd %zero %zero
%void_fn = OpTypeFunction %void
%main = OpFunction %void None %void_fn
%entry = OpLabel
%call = OpFunctionCall %void %func
OpReturn
OpFunctionEnd
%func = OpFunction %void None %void_fn
%func_entry = OpLabel
%add = OpFAdd %float %const %zero
OpReturn
OpFunctionEnd
)";

  CompileSuccessfully(spirv, SPV_ENV_UNIVERSAL_1_3);
  EXPECT_EQ(SPV_ERROR_INVALID_DATA,
            ValidateInstructions(SPV_ENV_UNIVERSAL_1_3));
  EXPECT_THAT(getDiagnosticString(),
              HasSubstr("FPFastMathMode Fast cannot be used by an entry point "
                        "with the FPFastMathDefault execution mode"));
}

TEST_F(ValidateMode, FragmentShaderRequireFullQuadsKHR) {
  const std::string spirv = R"(
OpCapability Shader
OpCapability GroupNonUniform
OpCapability GroupNonUniformVote
OpCapability GroupNonUniformBallot
OpCapability QuadControlKHR
OpExtension "SPV_KHR_quad_control"
%1 = OpExtInstImport "GLSL.std.450"
OpMemoryModel Logical GLSL450
OpEntryPoint GLCompute %4 "main"
OpExecutionMode %4 OriginUpperLeft
OpExecutionMode %4 RequireFullQuadsKHR
OpDecorate %17 Location 0
OpDecorate %31 BuiltIn HelperInvocation
OpDecorate %40 Location 0
OpDecorate %44 DescriptorSet 0
OpDecorate %44 Binding 0
%2 = OpTypeVoid
%3 = OpTypeFunction %2
%6 = OpTypeInt 32 0
%7 = OpTypeVector %6 4
%8 = OpTypePointer Function %7
%10 = OpTypeBool
%11 = OpConstantTrue %10
%12 = OpConstant %6 7
%14 = OpTypeFloat 32
%15 = OpTypeVector %14 4
%16 = OpTypePointer Output %15
%17 = OpVariable %16 Output
%18 = OpConstant %14 1
%19 = OpConstant %14 0
%20 = OpConstantComposite %15 %18 %19 %19 %18
%23 = OpConstant %6 4
%27 = OpConstant %6 1
%28 = OpTypePointer Output %14
%30 = OpTypePointer Input %10
%31 = OpVariable %30 Input
%36 = OpConstant %6 2
%38 = OpTypeVector %14 2
%39 = OpTypePointer Input %38
%40 = OpVariable %39 Input
%41 = OpTypeImage %14 2D 0 0 0 1 Unknown
%42 = OpTypeSampledImage %41
%43 = OpTypePointer UniformConstant %42
%44 = OpVariable %43 UniformConstant
%4 = OpFunction %2 None %3
%5 = OpLabel
%9 = OpVariable %8 Function
%13 = OpGroupNonUniformBallot %7 %12 %11
OpStore %9 %13
OpStore %17 %20
%21 = OpLoad %7 %9
%22 = OpGroupNonUniformBallotBitCount %6 %12 Reduce %21
%24 = OpIEqual %10 %22 %23
OpSelectionMerge %26 None
OpBranchConditional %24 %25 %26
%25 = OpLabel
%29 = OpAccessChain %28 %17 %27
OpStore %29 %18
OpBranch %26
%26 = OpLabel
%32 = OpLoad %10 %31
%33 = OpGroupNonUniformAny %10 %12 %32
OpSelectionMerge %35 None
OpBranchConditional %33 %34 %35
%34 = OpLabel
%37 = OpAccessChain %28 %17 %36
OpStore %37 %18
OpBranch %35
%35 = OpLabel
OpReturn
OpFunctionEnd
)";

  CompileSuccessfully(spirv, SPV_ENV_UNIVERSAL_1_3);
  EXPECT_THAT(SPV_ERROR_INVALID_DATA,
              ValidateInstructions(SPV_ENV_UNIVERSAL_1_3));
  EXPECT_THAT(
      getDiagnosticString(),
      HasSubstr(
          "Execution mode can only be used with the Fragment execution model"));
}

TEST_F(ValidateMode, FragmentShaderQuadDerivativesKHR) {
  const std::string spirv = R"(
OpCapability Shader
OpCapability GroupNonUniform
OpCapability GroupNonUniformVote
OpCapability QuadControlKHR
OpExtension "SPV_KHR_quad_control"
%1 = OpExtInstImport "GLSL.std.450"
OpMemoryModel Logical GLSL450
OpEntryPoint GLCompute %4 "main"
OpExecutionMode %4 OriginUpperLeft
OpExecutionMode %4 QuadDerivativesKHR
OpDecorate %12 BuiltIn FragCoord
OpDecorate %41 Location 0
OpDecorate %45 DescriptorSet 0
OpDecorate %45 Binding 0
OpDecorate %49 Location 0
%2 = OpTypeVoid
%3 = OpTypeFunction %2
%6 = OpTypeBool
%7 = OpTypePointer Function %6
%9 = OpTypeFloat 32
%10 = OpTypeVector %9 4
%11 = OpTypePointer Input %10
%12 = OpVariable %11 Input
%13 = OpTypeInt 32 0
%14 = OpConstant %13 1
%15 = OpTypePointer Input %9
%18 = OpConstant %9 8.5
%21 = OpConstant %9 0.100000001
%25 = OpConstant %13 0
%28 = OpConstant %9 3.5
%30 = OpConstant %9 6
%36 = OpConstant %13 7
%40 = OpTypePointer Output %10
%41 = OpVariable %40 Output
%42 = OpTypeImage %9 2D 0 0 0 1 Unknown
%43 = OpTypeSampledImage %42
%44 = OpTypePointer UniformConstant %43
%45 = OpVariable %44 UniformConstant
%47 = OpTypeVector %9 2
%48 = OpTypePointer Input %47
%49 = OpVariable %48 Input
%53 = OpConstant %9 0.899999976
%54 = OpConstant %9 0.200000003
%55 = OpConstant %9 1
%56 = OpConstantComposite %10 %53 %54 %54 %55
%4 = OpFunction %2 None %3
%5 = OpLabel
%8 = OpVariable %7 Function
%16 = OpAccessChain %15 %12 %14
%17 = OpLoad %9 %16
%19 = OpFSub %9 %17 %18
%20 = OpExtInst %9 %1 FAbs %19
%22 = OpFOrdLessThan %6 %20 %21
OpSelectionMerge %24 None
OpBranchConditional %22 %23 %24
%23 = OpLabel
%26 = OpAccessChain %15 %12 %25
%27 = OpLoad %9 %26
%29 = OpFSub %9 %27 %28
%31 = OpFMod %9 %29 %30
%33 = OpFOrdLessThan %6 %31 %21
OpBranch %24
%24 = OpLabel
%34 = OpPhi %6 %22 %5 %33 %23
OpStore %8 %34
%35 = OpLoad %6 %8
%37 = OpGroupNonUniformAny %6 %36 %35
OpSelectionMerge %39 None
OpBranchConditional %37 %38 %52
%38 = OpLabel
%46 = OpLoad %43 %45
%50 = OpLoad %47 %49
%51 = OpImageSampleImplicitLod %10 %46 %50
OpStore %41 %51
OpBranch %39
%52 = OpLabel
OpStore %41 %56
OpBranch %39
%39 = OpLabel
OpReturn
OpFunctionEnd
)";

  CompileSuccessfully(spirv, SPV_ENV_UNIVERSAL_1_3);
  EXPECT_THAT(SPV_ERROR_INVALID_DATA,
              ValidateInstructions(SPV_ENV_UNIVERSAL_1_3));
  EXPECT_THAT(
      getDiagnosticString(),
      HasSubstr(
          "Execution mode can only be used with the Fragment execution model"));
}

const std::string kNodeShaderPrelude = R"(
OpCapability Shader
OpCapability ShaderEnqueueAMDX
OpExtension "SPV_AMDX_shader_enqueue"
OpMemoryModel Logical GLSL450
OpEntryPoint GLCompute %main "main"
OpEntryPoint GLCompute %other "other"
)";

const std::string kNodeShaderPostlude = R"(
%uint = OpTypeInt 32 0
%uint_0 = OpConstant %uint 0
%uint_1 = OpConstant %uint 1
%node0 = OpConstantStringAMDX "node0"
%node1 = OpConstantStringAMDX "node1"
%node2 = OpConstantStringAMDX "node2"
%S = OpTypeStruct
%_payloadarr_S = OpTypeNodePayloadArrayAMDX %S
%_payloadarr_S_0 = OpTypeNodePayloadArrayAMDX %S
%bool = OpTypeBool
%true = OpConstantTrue %bool
%void = OpTypeVoid
%void_fn = OpTypeFunction %void
%main = OpFunction %void None %void_fn
%entry = OpLabel
OpReturn
OpFunctionEnd
%other = OpFunction %void None %void_fn
%entry0 = OpLabel
OpReturn
OpFunctionEnd
)";

TEST_F(ValidateMode, NodeShader) {
  const std::string spirv = kNodeShaderPrelude + R"(
OpExecutionModeId %main ShaderIndexAMDX %uint_0
OpExecutionModeId %main IsApiEntryAMDX %true
OpExecutionModeId %main MaxNodeRecursionAMDX %uint_1
OpExecutionModeId %main MaxNumWorkgroupsAMDX %uint_1 %uint_1 %uint_1
OpExecutionModeId %main SharesInputWithAMDX %node0 %uint_0
OpExecutionModeId %other ShaderIndexAMDX %uint_0
OpExecutionModeId %other StaticNumWorkgroupsAMDX %uint_1 %uint_1 %uint_1
OpDecorateId %_payloadarr_S PayloadNodeNameAMDX %node1
OpDecorateId %_payloadarr_S_0 PayloadNodeNameAMDX %node2
OpDecorateId %_payloadarr_S PayloadNodeBaseIndexAMDX %uint_0
OpDecorateId %_payloadarr_S PayloadNodeArraySizeAMDX %uint_1
OpDecorateId %_payloadarr_S NodeSharesPayloadLimitsWithAMDX %_payloadarr_S_0
)" + kNodeShaderPostlude;

  spv_target_env env = SPV_ENV_UNIVERSAL_1_3;
  CompileSuccessfully(spirv, env);
  EXPECT_THAT(SPV_SUCCESS, ValidateInstructions(env));
}

TEST_F(ValidateMode, NodeShaderModeShaderIndex) {
  const std::string spirv = kNodeShaderPrelude + R"(
OpExecutionMode %main ShaderIndexAMDX %uint_0
OpExecutionModeId %main IsApiEntryAMDX %true
OpExecutionModeId %main MaxNodeRecursionAMDX %uint_1
OpExecutionModeId %main MaxNumWorkgroupsAMDX %uint_1 %uint_1 %uint_1
OpExecutionModeId %main SharesInputWithAMDX %node0 %uint_0
OpExecutionMode %other ShaderIndexAMDX %uint_0
OpExecutionModeId %other StaticNumWorkgroupsAMDX %uint_1 %uint_1 %uint_1
OpDecorateId %_payloadarr_S PayloadNodeNameAMDX %node1
OpDecorateId %_payloadarr_S_0 PayloadNodeNameAMDX %node2
OpDecorateId %_payloadarr_S PayloadNodeBaseIndexAMDX %uint_0
OpDecorateId %_payloadarr_S PayloadNodeArraySizeAMDX %uint_1
OpDecorateId %_payloadarr_S NodeSharesPayloadLimitsWithAMDX %_payloadarr_S_0
)" + kNodeShaderPostlude;

  spv_target_env env = SPV_ENV_UNIVERSAL_1_3;
  CompileSuccessfully(spirv, env);
  EXPECT_THAT(SPV_ERROR_INVALID_DATA, ValidateInstructions(env));
  EXPECT_THAT(
      getDiagnosticString(),
      HasSubstr("OpExecutionMode is only valid when the Mode operand is an "
                "execution mode that takes no Extra Operands, or takes Extra "
                "Operands that are not id operands"));
}

TEST_F(ValidateMode, NodeShaderModeIsApiEntry) {
  const std::string spirv = kNodeShaderPrelude + R"(
OpExecutionModeId %main ShaderIndexAMDX %uint_0
OpExecutionMode %main IsApiEntryAMDX %true
OpExecutionModeId %main MaxNodeRecursionAMDX %uint_1
OpExecutionModeId %main MaxNumWorkgroupsAMDX %uint_1 %uint_1 %uint_1
OpExecutionModeId %main SharesInputWithAMDX %node0 %uint_0
OpExecutionModeId %other ShaderIndexAMDX %uint_0
OpExecutionModeId %other StaticNumWorkgroupsAMDX %uint_1 %uint_1 %uint_1
OpDecorateId %_payloadarr_S PayloadNodeNameAMDX %node1
OpDecorateId %_payloadarr_S_0 PayloadNodeNameAMDX %node2
OpDecorateId %_payloadarr_S PayloadNodeBaseIndexAMDX %uint_0
OpDecorateId %_payloadarr_S PayloadNodeArraySizeAMDX %uint_1
OpDecorateId %_payloadarr_S NodeSharesPayloadLimitsWithAMDX %_payloadarr_S_0
)" + kNodeShaderPostlude;

  spv_target_env env = SPV_ENV_UNIVERSAL_1_3;
  CompileSuccessfully(spirv, env);
  EXPECT_THAT(SPV_ERROR_INVALID_DATA, ValidateInstructions(env));
  EXPECT_THAT(
      getDiagnosticString(),
      HasSubstr("OpExecutionMode is only valid when the Mode operand is an "
                "execution mode that takes no Extra Operands, or takes Extra "
                "Operands that are not id operands"));
}

TEST_F(ValidateMode, NodeShaderModeMaxNodeRecursion) {
  const std::string spirv = kNodeShaderPrelude + R"(
OpExecutionModeId %main ShaderIndexAMDX %uint_0
OpExecutionModeId %main IsApiEntryAMDX %true
OpExecutionMode %main MaxNodeRecursionAMDX %uint_1
OpExecutionModeId %main MaxNumWorkgroupsAMDX %uint_1 %uint_1 %uint_1
OpExecutionModeId %main SharesInputWithAMDX %node0 %uint_0
OpExecutionModeId %other ShaderIndexAMDX %uint_0
OpExecutionModeId %other StaticNumWorkgroupsAMDX %uint_1 %uint_1 %uint_1
OpDecorateId %_payloadarr_S PayloadNodeNameAMDX %node1
OpDecorateId %_payloadarr_S_0 PayloadNodeNameAMDX %node2
OpDecorateId %_payloadarr_S PayloadNodeBaseIndexAMDX %uint_0
OpDecorateId %_payloadarr_S PayloadNodeArraySizeAMDX %uint_1
OpDecorateId %_payloadarr_S NodeSharesPayloadLimitsWithAMDX %_payloadarr_S_0
)" + kNodeShaderPostlude;

  spv_target_env env = SPV_ENV_UNIVERSAL_1_3;
  CompileSuccessfully(spirv, env);
  EXPECT_THAT(SPV_ERROR_INVALID_DATA, ValidateInstructions(env));
  EXPECT_THAT(
      getDiagnosticString(),
      HasSubstr("OpExecutionMode is only valid when the Mode operand is an "
                "execution mode that takes no Extra Operands, or takes Extra "
                "Operands that are not id operands"));
}

TEST_F(ValidateMode, NodeShaderModeMaxNumWorkgroups) {
  const std::string spirv = kNodeShaderPrelude + R"(
OpExecutionModeId %main ShaderIndexAMDX %uint_0
OpExecutionModeId %main IsApiEntryAMDX %true
OpExecutionModeId %main MaxNodeRecursionAMDX %uint_1
OpExecutionMode %main MaxNumWorkgroupsAMDX %uint_1 %uint_1 %uint_1
OpExecutionModeId %main SharesInputWithAMDX %node0 %uint_0
OpExecutionModeId %other ShaderIndexAMDX %uint_0
OpExecutionModeId %other StaticNumWorkgroupsAMDX %uint_1 %uint_1 %uint_1
OpDecorateId %_payloadarr_S PayloadNodeNameAMDX %node1
OpDecorateId %_payloadarr_S_0 PayloadNodeNameAMDX %node2
OpDecorateId %_payloadarr_S PayloadNodeBaseIndexAMDX %uint_0
OpDecorateId %_payloadarr_S PayloadNodeArraySizeAMDX %uint_1
OpDecorateId %_payloadarr_S NodeSharesPayloadLimitsWithAMDX %_payloadarr_S_0
)" + kNodeShaderPostlude;

  spv_target_env env = SPV_ENV_UNIVERSAL_1_3;
  CompileSuccessfully(spirv, env);
  EXPECT_THAT(SPV_ERROR_INVALID_DATA, ValidateInstructions(env));
  EXPECT_THAT(
      getDiagnosticString(),
      HasSubstr("OpExecutionMode is only valid when the Mode operand is an "
                "execution mode that takes no Extra Operands, or takes Extra "
                "Operands that are not id operands"));
}

TEST_F(ValidateMode, NodeShaderModeStaticNumWorkgroups) {
  const std::string spirv = kNodeShaderPrelude + R"(
OpExecutionModeId %main ShaderIndexAMDX %uint_0
OpExecutionModeId %main IsApiEntryAMDX %true
OpExecutionModeId %main MaxNodeRecursionAMDX %uint_1
OpExecutionModeId %main MaxNumWorkgroupsAMDX %uint_1 %uint_1 %uint_1
OpExecutionModeId %main SharesInputWithAMDX %node0 %uint_0
OpExecutionModeId %other ShaderIndexAMDX %uint_0
OpExecutionMode %other StaticNumWorkgroupsAMDX %uint_1 %uint_1 %uint_1
OpDecorateId %_payloadarr_S PayloadNodeNameAMDX %node1
OpDecorateId %_payloadarr_S_0 PayloadNodeNameAMDX %node2
OpDecorateId %_payloadarr_S PayloadNodeBaseIndexAMDX %uint_0
OpDecorateId %_payloadarr_S PayloadNodeArraySizeAMDX %uint_1
OpDecorateId %_payloadarr_S NodeSharesPayloadLimitsWithAMDX %_payloadarr_S_0
)" + kNodeShaderPostlude;

  spv_target_env env = SPV_ENV_UNIVERSAL_1_3;
  CompileSuccessfully(spirv, env);
  EXPECT_THAT(SPV_ERROR_INVALID_DATA, ValidateInstructions(env));
  EXPECT_THAT(
      getDiagnosticString(),
      HasSubstr("OpExecutionMode is only valid when the Mode operand is an "
                "execution mode that takes no Extra Operands, or takes Extra "
                "Operands that are not id operands"));
}

TEST_F(ValidateMode, NodeShaderModeSharesInputWith) {
  const std::string spirv = kNodeShaderPrelude + R"(
OpExecutionModeId %main ShaderIndexAMDX %uint_0
OpExecutionModeId %main IsApiEntryAMDX %true
OpExecutionModeId %main MaxNodeRecursionAMDX %uint_1
OpExecutionModeId %main MaxNumWorkgroupsAMDX %uint_1 %uint_1 %uint_1
OpExecutionMode %main SharesInputWithAMDX %node0 %uint_0
OpExecutionModeId %other ShaderIndexAMDX %uint_0
OpExecutionModeId %other StaticNumWorkgroupsAMDX %uint_1 %uint_1 %uint_1
OpDecorateId %_payloadarr_S PayloadNodeNameAMDX %node1
OpDecorateId %_payloadarr_S_0 PayloadNodeNameAMDX %node2
OpDecorateId %_payloadarr_S PayloadNodeBaseIndexAMDX %uint_0
OpDecorateId %_payloadarr_S PayloadNodeArraySizeAMDX %uint_1
OpDecorateId %_payloadarr_S NodeSharesPayloadLimitsWithAMDX %_payloadarr_S_0
)" + kNodeShaderPostlude;

  spv_target_env env = SPV_ENV_UNIVERSAL_1_3;
  CompileSuccessfully(spirv, env);
  EXPECT_THAT(SPV_ERROR_INVALID_DATA, ValidateInstructions(env));
  EXPECT_THAT(
      getDiagnosticString(),
      HasSubstr("OpExecutionMode is only valid when the Mode operand is an "
                "execution mode that takes no Extra Operands, or takes Extra "
                "Operands that are not id operands"));
}

}  // namespace
}  // namespace val
}  // namespace spvtools
