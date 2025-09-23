// Copyright (c) 2015-2016 The Khronos Group Inc.
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

// Validation tests for OpVariable storage class

#include <sstream>
#include <string>
#include <tuple>

#include "gmock/gmock.h"
#include "test/val/val_fixtures.h"

namespace spvtools {
namespace val {
namespace {

using ::testing::HasSubstr;
using ::testing::Values;
using ValidateStorage = spvtest::ValidateBase<std::string>;
using ValidateStorageExecutionModel = spvtest::ValidateBase<std::string>;

TEST_F(ValidateStorage, FunctionStorageInsideFunction) {
  char str[] = R"(
          OpCapability Shader
          OpCapability Linkage
          OpMemoryModel Logical GLSL450
%intt   = OpTypeInt 32 1
%voidt  = OpTypeVoid
%vfunct = OpTypeFunction %voidt
%ptrt   = OpTypePointer Function %intt
%func   = OpFunction %voidt None %vfunct
%funcl  = OpLabel
%var    = OpVariable %ptrt Function
          OpReturn
          OpFunctionEnd
)";

  CompileSuccessfully(str);
  ASSERT_EQ(SPV_SUCCESS, ValidateInstructions());
}

TEST_F(ValidateStorage, FunctionStorageOutsideFunction) {
  char str[] = R"(
          OpCapability Shader
          OpCapability Linkage
          OpMemoryModel Logical GLSL450
%intt   = OpTypeInt 32 1
%voidt  = OpTypeVoid
%vfunct = OpTypeFunction %voidt
%ptrt   = OpTypePointer Function %intt
%var    = OpVariable %ptrt Function
%func   = OpFunction %voidt None %vfunct
%funcl  = OpLabel
          OpReturn
          OpFunctionEnd
)";

  CompileSuccessfully(str);
  ASSERT_EQ(SPV_ERROR_INVALID_LAYOUT, ValidateInstructions());
  EXPECT_THAT(getDiagnosticString(),
              HasSubstr("Variables can not have a function[7] storage class "
                        "outside of a function"));
}

TEST_F(ValidateStorage, OtherStorageOutsideFunction) {
  char str[] = R"(
              OpCapability Shader
              OpCapability Kernel
              OpCapability AtomicStorage
              OpCapability Linkage
              OpMemoryModel Logical GLSL450
%intt       = OpTypeInt 32 0
%voidt      = OpTypeVoid
%vfunct     = OpTypeFunction %voidt
%uniconptrt = OpTypePointer UniformConstant %intt
%unicon     = OpVariable %uniconptrt UniformConstant
%inputptrt  = OpTypePointer Input %intt
%input      = OpVariable %inputptrt Input
%unifptrt   = OpTypePointer Uniform %intt
%unif       = OpVariable %unifptrt Uniform
%outputptrt = OpTypePointer Output %intt
%output     = OpVariable %outputptrt Output
%wgroupptrt = OpTypePointer Workgroup %intt
%wgroup     = OpVariable %wgroupptrt Workgroup
%xwgrpptrt  = OpTypePointer CrossWorkgroup %intt
%xwgrp      = OpVariable %xwgrpptrt CrossWorkgroup
%privptrt   = OpTypePointer Private %intt
%priv       = OpVariable %privptrt Private
%pushcoptrt = OpTypePointer PushConstant %intt
%pushco     = OpVariable %pushcoptrt PushConstant
%atomcptrt  = OpTypePointer AtomicCounter %intt
%atomct     = OpVariable %atomcptrt AtomicCounter
%imageptrt  = OpTypePointer Image %intt
%image      = OpVariable %imageptrt Image
%func       = OpFunction %voidt None %vfunct
%funcl      = OpLabel
              OpReturn
              OpFunctionEnd
)";

  CompileSuccessfully(str);
  ASSERT_EQ(SPV_SUCCESS, ValidateInstructions());
}

// clang-format off
TEST_P(ValidateStorage, OtherStorageInsideFunction) {
  std::stringstream ss;
  ss << R"(
          OpCapability Shader
          OpCapability Kernel
          OpCapability AtomicStorage
          OpCapability Linkage
          OpMemoryModel Logical GLSL450
%intt   = OpTypeInt 32 0
%voidt  = OpTypeVoid
%vfunct = OpTypeFunction %voidt
%ptrt   = OpTypePointer Function %intt
%func   = OpFunction %voidt None %vfunct
%funcl  = OpLabel
%var    = OpVariable %ptrt )" << GetParam() << R"(
          OpReturn
          OpFunctionEnd
)";

  CompileSuccessfully(ss.str());
  ASSERT_EQ(SPV_ERROR_INVALID_LAYOUT, ValidateInstructions());
  EXPECT_THAT(getDiagnosticString(), HasSubstr(
      "Variables must have a function[7] storage class inside of a function"));
}

INSTANTIATE_TEST_SUITE_P(MatrixOp, ValidateStorage,
                        ::testing::Values(
                             "Input",
                             "Uniform",
                             "Output",
                             "Workgroup",
                             "CrossWorkgroup",
                             "Private",
                             "PushConstant",
                             "AtomicCounter",
                             "Image"));
// clang-format on

TEST_F(ValidateStorage, GenericVariableOutsideFunction) {
  const auto str = R"(
          OpCapability Kernel
          OpCapability Linkage
          OpCapability GenericPointer
          OpMemoryModel Logical OpenCL
%intt   = OpTypeInt 32 0
%ptrt   = OpTypePointer Function %intt
%var    = OpVariable %ptrt Generic
)";
  CompileSuccessfully(str);
  ASSERT_EQ(SPV_ERROR_INVALID_BINARY, ValidateInstructions());
  EXPECT_THAT(getDiagnosticString(),
              HasSubstr("Variable storage class cannot be Generic"));
}

TEST_F(ValidateStorage, GenericVariableInsideFunction) {
  const auto str = R"(
          OpCapability Shader
          OpCapability Linkage
          OpCapability GenericPointer
          OpMemoryModel Logical GLSL450
%intt   = OpTypeInt 32 1
%voidt  = OpTypeVoid
%vfunct = OpTypeFunction %voidt
%ptrt   = OpTypePointer Function %intt
%func   = OpFunction %voidt None %vfunct
%funcl  = OpLabel
%var    = OpVariable %ptrt Generic
          OpReturn
          OpFunctionEnd
)";
  CompileSuccessfully(str);
  EXPECT_EQ(SPV_ERROR_INVALID_BINARY, ValidateInstructions());
  EXPECT_THAT(getDiagnosticString(),
              HasSubstr("Variable storage class cannot be Generic"));
}

TEST_F(ValidateStorage, RelaxedLogicalPointerFunctionParam) {
  const auto str = R"(
          OpCapability Shader
          OpCapability Linkage
          OpMemoryModel Logical GLSL450
%intt   = OpTypeInt 32 1
%voidt  = OpTypeVoid
%ptrt   = OpTypePointer Function %intt
%vfunct = OpTypeFunction %voidt
%vifunct = OpTypeFunction %voidt %ptrt
%wgroupptrt = OpTypePointer Workgroup %intt
%wgroup = OpVariable %wgroupptrt Workgroup
%main   = OpFunction %voidt None %vfunct
%mainl  = OpLabel
%ret    = OpFunctionCall %voidt %func %wgroup
          OpReturn
          OpFunctionEnd
%func   = OpFunction %voidt None %vifunct
%arg    = OpFunctionParameter %ptrt
%funcl  = OpLabel
          OpReturn
          OpFunctionEnd
)";
  CompileSuccessfully(str);
  getValidatorOptions()->before_hlsl_legalization = true;
  ASSERT_EQ(SPV_SUCCESS, ValidateInstructions());
}

TEST_F(ValidateStorage, RelaxedLogicalPointerFunctionParamBad) {
  const auto str = R"(
          OpCapability Shader
          OpCapability Linkage
          OpMemoryModel Logical GLSL450
%floatt = OpTypeFloat 32
%intt   = OpTypeInt 32 1
%voidt  = OpTypeVoid
%ptrt   = OpTypePointer Function %intt
%vfunct = OpTypeFunction %voidt
%vifunct = OpTypeFunction %voidt %ptrt
%wgroupptrt = OpTypePointer Workgroup %floatt
%wgroup = OpVariable %wgroupptrt Workgroup
%main   = OpFunction %voidt None %vfunct
%mainl  = OpLabel
%ret    = OpFunctionCall %voidt %func %wgroup
          OpReturn
          OpFunctionEnd
%func   = OpFunction %voidt None %vifunct
%arg    = OpFunctionParameter %ptrt
%funcl  = OpLabel
          OpReturn
          OpFunctionEnd
)";
  CompileSuccessfully(str);
  getValidatorOptions()->relax_logical_pointer = true;
  ASSERT_EQ(SPV_ERROR_INVALID_ID, ValidateInstructions());
  EXPECT_THAT(getDiagnosticString(),
              HasSubstr("OpFunctionCall Argument <id> '"));
}

TEST_F(ValidateStorage, TileAttachmentQCOMBad1) {
  const std::string spirv = R"(
               OpCapability Shader
               OpCapability Sampled1D
               OpCapability TileShadingQCOM
               OpExtension "SPV_QCOM_tile_shading"
               OpMemoryModel Logical GLSL450
               OpEntryPoint Fragment %main "main"
               OpExecutionMode %main OriginUpperLeft
               OpSource GLSL 450
               OpDecorate %color1 Binding 2
               OpDecorate %color1 DescriptorSet 0
       %void = OpTypeVoid
        %int = OpTypeInt 32 1
         %44 = OpTypeImage %int 1D 0 0 0 2 Rgba32i
%_ptr_TileAttachmentQCOM_44 = OpTypePointer TileAttachmentQCOM %44
     %color1 = OpVariable %_ptr_TileAttachmentQCOM_44 TileAttachmentQCOM
          %3 = OpTypeFunction %void
       %main = OpFunction %void None %3
          %5 = OpLabel
               OpReturn
               OpFunctionEnd
  )";

  spv_target_env env = SPV_ENV_VULKAN_1_4;
  CompileSuccessfully(spirv, env);
  EXPECT_THAT(SPV_ERROR_INVALID_DATA, ValidateInstructions(env));
  EXPECT_THAT(getDiagnosticString(),
              HasSubstr("Any OpTypeImage variable in the TileAttachmentQCOM "
                        "Storage Class must have 2D as its dimension"));
}

TEST_F(ValidateStorage, TileAttachmentQCOMBad2) {
  const std::string spirv = R"(
               OpCapability Shader
               OpCapability TileShadingQCOM
               OpExtension "SPV_QCOM_tile_shading"
               OpMemoryModel Logical GLSL450
               OpEntryPoint Fragment %main "main"
               OpExecutionMode %main OriginUpperLeft
               OpSource GLSL 450
               OpDecorate %color1 Binding 2
       %void = OpTypeVoid 
        %int = OpTypeInt 32 1
         %44 = OpTypeImage %int 2D 0 0 0 2 Rgba32i
%_ptr_TileAttachmentQCOM_44 = OpTypePointer TileAttachmentQCOM %44
     %color1 = OpVariable %_ptr_TileAttachmentQCOM_44 TileAttachmentQCOM
          %3 = OpTypeFunction %void
       %main = OpFunction %void None %3
          %5 = OpLabel
               OpReturn
               OpFunctionEnd
  )";

  spv_target_env env = SPV_ENV_VULKAN_1_4;
  CompileSuccessfully(spirv, env);
  EXPECT_THAT(SPV_ERROR_INVALID_ID, ValidateInstructions(env));
  EXPECT_THAT(getDiagnosticString(),
              HasSubstr("Any variable in the TileAttachmentQCOM Storage Class "
                        "must be decorated with DescriptorSet and Binding"));
}

TEST_F(ValidateStorage, TileAttachmentQCOMBad3) {
  const std::string spirv = R"(
               OpCapability Shader
               OpCapability TileShadingQCOM
               OpExtension "SPV_QCOM_tile_shading"
               OpMemoryModel Logical GLSL450
               OpEntryPoint Fragment %main "main"
               OpExecutionMode %main OriginUpperLeft
               OpSource GLSL 450
               OpDecorate %color1 DescriptorSet 0
       %void = OpTypeVoid 
        %int = OpTypeInt 32 1
         %44 = OpTypeImage %int 2D 0 0 0 2 Rgba32i
%_ptr_TileAttachmentQCOM_44 = OpTypePointer TileAttachmentQCOM %44
     %color1 = OpVariable %_ptr_TileAttachmentQCOM_44 TileAttachmentQCOM
          %3 = OpTypeFunction %void
       %main = OpFunction %void None %3
          %5 = OpLabel
               OpReturn
               OpFunctionEnd
  )";

  spv_target_env env = SPV_ENV_VULKAN_1_4;
  CompileSuccessfully(spirv, env);
  EXPECT_THAT(SPV_ERROR_INVALID_ID, ValidateInstructions(env));
  EXPECT_THAT(getDiagnosticString(),
              HasSubstr("Any variable in the TileAttachmentQCOM Storage Class "
                        "must be decorated with DescriptorSet and Binding"));
}

TEST_F(ValidateStorage, TileAttachmentQCOMBad4) {
  const std::string spirv = R"(
               OpCapability Shader
               OpCapability TileShadingQCOM
               OpExtension "SPV_QCOM_tile_shading"
          %1 = OpExtInstImport "GLSL.std.450"
               OpMemoryModel Logical GLSL450
               OpEntryPoint Fragment %main "main"
               OpExecutionMode %main OriginUpperLeft
               OpSource GLSL 450
               OpDecorate %color1 Binding 2
               OpDecorate %color1 DescriptorSet 0
       %void = OpTypeVoid
        %int = OpTypeInt 32 1
         %44 = OpTypeImage %int 2D 0 0 0 0 Rgba32i
%_ptr_TileAttachmentQCOM_44 = OpTypePointer TileAttachmentQCOM %44
     %color1 = OpVariable %_ptr_TileAttachmentQCOM_44 TileAttachmentQCOM
          %3 = OpTypeFunction %void
       %main = OpFunction %void None %3
          %5 = OpLabel
               OpReturn
               OpFunctionEnd
  )";

  spv_target_env env = SPV_ENV_VULKAN_1_4;
  CompileSuccessfully(spirv, env);
  EXPECT_THAT(SPV_ERROR_INVALID_DATA, ValidateInstructions(env));
  EXPECT_THAT(getDiagnosticString(),
              AnyVUID("VUID-StandaloneSpirv-OpTypeImage-04657"));
  EXPECT_THAT(getDiagnosticString(),
              HasSubstr("Sampled must be 1 or 2 in the Vulkan environment"));
}

TEST_F(ValidateStorage, TileAttachmentQCOMBad5) {
  const std::string spirv = R"(
               OpCapability Shader
               OpCapability ImageQuery
               OpCapability TileShadingQCOM
               OpExtension "SPV_QCOM_tile_shading"
               OpMemoryModel Logical GLSL450
               OpEntryPoint Fragment %main "main"
               OpExecutionMode %main OriginUpperLeft
               OpDecorate %color1 Binding 2
               OpDecorate %color1 DescriptorSet 0
       %void = OpTypeVoid 
          %3 = OpTypeFunction %void
        %int = OpTypeInt 32 1
      %v2int = OpTypeVector %int 2 
      %int_2 = OpConstant %int 2
         %44 = OpTypeImage %int 2D 0 0 0 2 Rgba32i
%_ptr_TileAttachmentQCOM_44 = OpTypePointer TileAttachmentQCOM %44
     %color1 = OpVariable %_ptr_TileAttachmentQCOM_44 TileAttachmentQCOM
       %main = OpFunction %void None %3
          %5 = OpLabel
        %154 = OpLoad %44 %color1
        %156 = OpImageQuerySizeLod %v2int %154 %int_2
               OpReturn
               OpFunctionEnd
  )";

  spv_target_env env = SPV_ENV_VULKAN_1_4;
  CompileSuccessfully(spirv, env);
  EXPECT_THAT(SPV_ERROR_INVALID_DATA, ValidateInstructions(env));
  EXPECT_THAT(
      getDiagnosticString(),
      HasSubstr("Any variable in the TileAttachmentQCOM Storage Class "
                "must not be consumed by an OpImageQuery* instruction"));
}

TEST_F(ValidateStorage, TileAttachmentQCOMBad6) {
  const std::string spirv = R"(
               OpCapability Shader
               OpMemoryModel Logical GLSL450
               OpEntryPoint Fragment %main "main"
               OpExecutionMode %main OriginUpperLeft
               OpDecorate %color1 Binding 2
               OpDecorate %color1 DescriptorSet 0
       %void = OpTypeVoid
        %int = OpTypeInt 32 1
         %44 = OpTypeImage %int 2D 0 0 0 2 Rgba32i
%_ptr_TileAttachmentQCOM_44 = OpTypePointer TileAttachmentQCOM %44
     %color1 = OpVariable %_ptr_TileAttachmentQCOM_44 TileAttachmentQCOM
          %3 = OpTypeFunction %void
       %main = OpFunction %void None %3
          %5 = OpLabel
               OpReturn
               OpFunctionEnd
  )";

  spv_target_env env = SPV_ENV_VULKAN_1_4;
  CompileSuccessfully(spirv, env);
  EXPECT_THAT(SPV_ERROR_INVALID_CAPABILITY, ValidateInstructions(env));
  EXPECT_THAT(getDiagnosticString(),
              HasSubstr("requires one of these capabilities: TileShadingQCOM"));
}

std::string GenerateExecutionModelCode(const std::string& execution_model,
                                       const std::string& storage_class,
                                       bool store) {
  const std::string mode = (execution_model.compare("GLCompute") == 0)
                               ? "OpExecutionMode %func LocalSize 1 1 1"
                               : "";
  const std::string operation =
      (store) ? "OpStore %var %int0" : "%load = OpLoad %intt %var";
  std::ostringstream ss;
  ss << R"(
              OpCapability Shader
              OpCapability RayTracingKHR
              OpExtension "SPV_KHR_ray_tracing"
              OpMemoryModel Logical GLSL450
              OpEntryPoint )"
     << execution_model << R"( %func "func" %var
              )"
     << mode << R"(
              OpDecorate %var Location 0
%intt       = OpTypeInt 32 0
%int0       = OpConstant %intt 0
%voidt      = OpTypeVoid
%vfunct     = OpTypeFunction %voidt
%ptr        = OpTypePointer )"
     << storage_class << R"( %intt
%var        = OpVariable %ptr )"
     << storage_class << R"(
%func       = OpFunction %voidt None %vfunct
%funcl      = OpLabel
              )"
     << operation << R"(
              OpReturn
              OpFunctionEnd
)";

  return ss.str();
}

TEST_P(ValidateStorageExecutionModel, VulkanOutsideStoreFailure) {
  std::string execution_model = GetParam();
  CompileSuccessfully(
      GenerateExecutionModelCode(execution_model, "Output", true).c_str(),
      SPV_ENV_VULKAN_1_0);
  ASSERT_EQ(SPV_ERROR_INVALID_ID, ValidateInstructions(SPV_ENV_VULKAN_1_0));
  EXPECT_THAT(getDiagnosticString(),
              AnyVUID("VUID-StandaloneSpirv-None-04644"));
  EXPECT_THAT(
      getDiagnosticString(),
      HasSubstr("in Vulkan environment, Output Storage Class must not be used "
                "in GLCompute, RayGenerationKHR, IntersectionKHR, AnyHitKHR, "
                "ClosestHitKHR, MissKHR, or CallableKHR execution models"));
}

TEST_P(ValidateStorageExecutionModel, CallableDataStore) {
  std::string execution_model = GetParam();
  CompileSuccessfully(
      GenerateExecutionModelCode(execution_model, "CallableDataKHR", true)
          .c_str(),
      SPV_ENV_VULKAN_1_2);
  if (execution_model.compare("RayGenerationKHR") == 0 ||
      execution_model.compare("ClosestHitKHR") == 0 ||
      execution_model.compare("CallableKHR") == 0 ||
      execution_model.compare("MissKHR") == 0) {
    ASSERT_EQ(SPV_SUCCESS, ValidateInstructions(SPV_ENV_VULKAN_1_2));
  } else {
    ASSERT_EQ(SPV_ERROR_INVALID_ID, ValidateInstructions(SPV_ENV_VULKAN_1_2));
    EXPECT_THAT(getDiagnosticString(),
                AnyVUID("VUID-StandaloneSpirv-CallableDataKHR-04704"));
    EXPECT_THAT(
        getDiagnosticString(),
        HasSubstr(
            "CallableDataKHR Storage Class is limited to RayGenerationKHR, "
            "ClosestHitKHR, CallableKHR, and MissKHR execution model"));
  }
}

TEST_P(ValidateStorageExecutionModel, CallableDataLoad) {
  std::string execution_model = GetParam();
  CompileSuccessfully(
      GenerateExecutionModelCode(execution_model, "CallableDataKHR", false)
          .c_str(),
      SPV_ENV_VULKAN_1_2);
  if (execution_model.compare("RayGenerationKHR") == 0 ||
      execution_model.compare("ClosestHitKHR") == 0 ||
      execution_model.compare("CallableKHR") == 0 ||
      execution_model.compare("MissKHR") == 0) {
    ASSERT_EQ(SPV_SUCCESS, ValidateInstructions(SPV_ENV_VULKAN_1_2));
  } else {
    ASSERT_EQ(SPV_ERROR_INVALID_ID, ValidateInstructions(SPV_ENV_VULKAN_1_2));
    EXPECT_THAT(getDiagnosticString(),
                AnyVUID("VUID-StandaloneSpirv-CallableDataKHR-04704"));
    EXPECT_THAT(
        getDiagnosticString(),
        HasSubstr(
            "CallableDataKHR Storage Class is limited to RayGenerationKHR, "
            "ClosestHitKHR, CallableKHR, and MissKHR execution model"));
  }
}

TEST_P(ValidateStorageExecutionModel, IncomingCallableDataStore) {
  std::string execution_model = GetParam();
  CompileSuccessfully(GenerateExecutionModelCode(
                          execution_model, "IncomingCallableDataKHR", true)
                          .c_str(),
                      SPV_ENV_VULKAN_1_2);
  if (execution_model.compare("CallableKHR") == 0) {
    ASSERT_EQ(SPV_SUCCESS, ValidateInstructions(SPV_ENV_VULKAN_1_2));
  } else {
    ASSERT_EQ(SPV_ERROR_INVALID_ID, ValidateInstructions(SPV_ENV_VULKAN_1_2));
    EXPECT_THAT(getDiagnosticString(),
                AnyVUID("VUID-StandaloneSpirv-IncomingCallableDataKHR-04705"));
    EXPECT_THAT(getDiagnosticString(),
                HasSubstr("IncomingCallableDataKHR Storage Class is limited to "
                          "CallableKHR execution model"));
  }
}

TEST_P(ValidateStorageExecutionModel, IncomingCallableDataLoad) {
  std::string execution_model = GetParam();
  CompileSuccessfully(GenerateExecutionModelCode(
                          execution_model, "IncomingCallableDataKHR", false)
                          .c_str(),
                      SPV_ENV_VULKAN_1_2);
  if (execution_model.compare("CallableKHR") == 0) {
    ASSERT_EQ(SPV_SUCCESS, ValidateInstructions(SPV_ENV_VULKAN_1_2));
  } else {
    ASSERT_EQ(SPV_ERROR_INVALID_ID, ValidateInstructions(SPV_ENV_VULKAN_1_2));
    EXPECT_THAT(getDiagnosticString(),
                AnyVUID("VUID-StandaloneSpirv-IncomingCallableDataKHR-04705"));
    EXPECT_THAT(getDiagnosticString(),
                HasSubstr("IncomingCallableDataKHR Storage Class is limited to "
                          "CallableKHR execution model"));
  }
}

TEST_P(ValidateStorageExecutionModel, RayPayloadStore) {
  std::string execution_model = GetParam();
  CompileSuccessfully(
      GenerateExecutionModelCode(execution_model, "RayPayloadKHR", true)
          .c_str(),
      SPV_ENV_VULKAN_1_2);
  if (execution_model.compare("RayGenerationKHR") == 0 ||
      execution_model.compare("ClosestHitKHR") == 0 ||
      execution_model.compare("MissKHR") == 0) {
    ASSERT_EQ(SPV_SUCCESS, ValidateInstructions(SPV_ENV_VULKAN_1_2));
  } else {
    ASSERT_EQ(SPV_ERROR_INVALID_ID, ValidateInstructions(SPV_ENV_VULKAN_1_2));
    EXPECT_THAT(getDiagnosticString(),
                AnyVUID("VUID-StandaloneSpirv-RayPayloadKHR-04698"));
    EXPECT_THAT(
        getDiagnosticString(),
        HasSubstr("RayPayloadKHR Storage Class is limited to RayGenerationKHR, "
                  "ClosestHitKHR, and MissKHR execution model"));
  }
}

TEST_P(ValidateStorageExecutionModel, RayPayloadLoad) {
  std::string execution_model = GetParam();
  CompileSuccessfully(
      GenerateExecutionModelCode(execution_model, "RayPayloadKHR", false)
          .c_str(),
      SPV_ENV_VULKAN_1_2);
  if (execution_model.compare("RayGenerationKHR") == 0 ||
      execution_model.compare("ClosestHitKHR") == 0 ||
      execution_model.compare("MissKHR") == 0) {
    ASSERT_EQ(SPV_SUCCESS, ValidateInstructions(SPV_ENV_VULKAN_1_2));
  } else {
    ASSERT_EQ(SPV_ERROR_INVALID_ID, ValidateInstructions(SPV_ENV_VULKAN_1_2));
    EXPECT_THAT(getDiagnosticString(),
                AnyVUID("VUID-StandaloneSpirv-RayPayloadKHR-04698"));
    EXPECT_THAT(
        getDiagnosticString(),
        HasSubstr("RayPayloadKHR Storage Class is limited to RayGenerationKHR, "
                  "ClosestHitKHR, and MissKHR execution model"));
  }
}

TEST_P(ValidateStorageExecutionModel, HitAttributeStore) {
  std::string execution_model = GetParam();
  CompileSuccessfully(
      GenerateExecutionModelCode(execution_model, "HitAttributeKHR", true)
          .c_str(),
      SPV_ENV_VULKAN_1_2);
  if (execution_model.compare("IntersectionKHR") == 0) {
    ASSERT_EQ(SPV_SUCCESS, ValidateInstructions(SPV_ENV_VULKAN_1_2));
  } else if (execution_model.compare("AnyHitKHR") == 0 ||
             execution_model.compare("ClosestHitKHR") == 0) {
    ASSERT_EQ(SPV_ERROR_INVALID_ID, ValidateInstructions(SPV_ENV_VULKAN_1_2));
    EXPECT_THAT(getDiagnosticString(),
                AnyVUID("VUID-StandaloneSpirv-HitAttributeKHR-04703"));
    EXPECT_THAT(getDiagnosticString(),
                HasSubstr("HitAttributeKHR Storage Class variables are read "
                          "only with AnyHitKHR and ClosestHitKHR"));
  } else {
    ASSERT_EQ(SPV_ERROR_INVALID_ID, ValidateInstructions(SPV_ENV_VULKAN_1_2));
    EXPECT_THAT(getDiagnosticString(),
                AnyVUID("VUID-StandaloneSpirv-HitAttributeKHR-04701"));
    EXPECT_THAT(
        getDiagnosticString(),
        HasSubstr(
            "HitAttributeKHR Storage Class is limited to IntersectionKHR, "
            "AnyHitKHR, sand ClosestHitKHR execution model"));
  }
}

TEST_P(ValidateStorageExecutionModel, HitAttributeLoad) {
  std::string execution_model = GetParam();
  CompileSuccessfully(
      GenerateExecutionModelCode(execution_model, "HitAttributeKHR", false)
          .c_str(),
      SPV_ENV_VULKAN_1_2);
  if (execution_model.compare("IntersectionKHR") == 0 ||
      execution_model.compare("AnyHitKHR") == 0 ||
      execution_model.compare("ClosestHitKHR") == 0) {
    ASSERT_EQ(SPV_SUCCESS, ValidateInstructions(SPV_ENV_VULKAN_1_2));
  } else {
    ASSERT_EQ(SPV_ERROR_INVALID_ID, ValidateInstructions(SPV_ENV_VULKAN_1_2));
    EXPECT_THAT(getDiagnosticString(),
                AnyVUID("VUID-StandaloneSpirv-HitAttributeKHR-04701"));
    EXPECT_THAT(
        getDiagnosticString(),
        HasSubstr(
            "HitAttributeKHR Storage Class is limited to IntersectionKHR, "
            "AnyHitKHR, sand ClosestHitKHR execution model"));
  }
}

TEST_P(ValidateStorageExecutionModel, IncomingRayPayloadStore) {
  std::string execution_model = GetParam();
  CompileSuccessfully(
      GenerateExecutionModelCode(execution_model, "IncomingRayPayloadKHR", true)
          .c_str(),
      SPV_ENV_VULKAN_1_2);
  if (execution_model.compare("AnyHitKHR") == 0 ||
      execution_model.compare("ClosestHitKHR") == 0 ||
      execution_model.compare("MissKHR") == 0) {
    ASSERT_EQ(SPV_SUCCESS, ValidateInstructions(SPV_ENV_VULKAN_1_2));
  } else {
    ASSERT_EQ(SPV_ERROR_INVALID_ID, ValidateInstructions(SPV_ENV_VULKAN_1_2));
    EXPECT_THAT(getDiagnosticString(),
                AnyVUID("VUID-StandaloneSpirv-IncomingRayPayloadKHR-04699"));
    EXPECT_THAT(
        getDiagnosticString(),
        HasSubstr("IncomingRayPayloadKHR Storage Class is limited to "
                  "AnyHitKHR, ClosestHitKHR, and MissKHR execution model"));
  }
}

TEST_P(ValidateStorageExecutionModel, IncomingRayPayloadLoad) {
  std::string execution_model = GetParam();
  CompileSuccessfully(GenerateExecutionModelCode(execution_model,
                                                 "IncomingRayPayloadKHR", false)
                          .c_str(),
                      SPV_ENV_VULKAN_1_2);
  if (execution_model.compare("AnyHitKHR") == 0 ||
      execution_model.compare("ClosestHitKHR") == 0 ||
      execution_model.compare("MissKHR") == 0) {
    ASSERT_EQ(SPV_SUCCESS, ValidateInstructions(SPV_ENV_VULKAN_1_2));
  } else {
    ASSERT_EQ(SPV_ERROR_INVALID_ID, ValidateInstructions(SPV_ENV_VULKAN_1_2));
    EXPECT_THAT(getDiagnosticString(),
                AnyVUID("VUID-StandaloneSpirv-IncomingRayPayloadKHR-04699"));
    EXPECT_THAT(
        getDiagnosticString(),
        HasSubstr("IncomingRayPayloadKHR Storage Class is limited to "
                  "AnyHitKHR, ClosestHitKHR, and MissKHR execution model"));
  }
}

TEST_P(ValidateStorageExecutionModel, ShaderRecordBufferStore) {
  std::string execution_model = GetParam();
  CompileSuccessfully(
      GenerateExecutionModelCode(execution_model, "ShaderRecordBufferKHR", true)
          .c_str(),
      SPV_ENV_VULKAN_1_2);
  ASSERT_EQ(SPV_ERROR_INVALID_ID, ValidateInstructions(SPV_ENV_VULKAN_1_2));
  EXPECT_THAT(
      getDiagnosticString(),
      HasSubstr("ShaderRecordBufferKHR Storage Class variables are read only"));
}

TEST_P(ValidateStorageExecutionModel, ShaderRecordBufferLoad) {
  std::string execution_model = GetParam();
  CompileSuccessfully(GenerateExecutionModelCode(execution_model,
                                                 "ShaderRecordBufferKHR", false)
                          .c_str(),
                      SPV_ENV_VULKAN_1_2);
  if (execution_model.compare("RayGenerationKHR") == 0 ||
      execution_model.compare("IntersectionKHR") == 0 ||
      execution_model.compare("AnyHitKHR") == 0 ||
      execution_model.compare("ClosestHitKHR") == 0 ||
      execution_model.compare("CallableKHR") == 0 ||
      execution_model.compare("MissKHR") == 0) {
    ASSERT_EQ(SPV_SUCCESS, ValidateInstructions(SPV_ENV_VULKAN_1_2));
  } else {
    ASSERT_EQ(SPV_ERROR_INVALID_ID, ValidateInstructions(SPV_ENV_VULKAN_1_2));
    EXPECT_THAT(getDiagnosticString(),
                AnyVUID("VUID-StandaloneSpirv-ShaderRecordBufferKHR-07119"));
    EXPECT_THAT(
        getDiagnosticString(),
        HasSubstr("ShaderRecordBufferKHR Storage Class is limited to "
                  "RayGenerationKHR, IntersectionKHR, AnyHitKHR, "
                  "ClosestHitKHR, CallableKHR, and MissKHR execution model"));
  }
}

INSTANTIATE_TEST_SUITE_P(MatrixExecutionModel, ValidateStorageExecutionModel,
                         ::testing::Values("RayGenerationKHR",
                                           "IntersectionKHR", "AnyHitKHR",
                                           "ClosestHitKHR", "MissKHR",
                                           "CallableKHR", "GLCompute"));

}  // namespace
}  // namespace val
}  // namespace spvtools
