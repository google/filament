// Copyright (c) 2019 The Khronos Group Inc.
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

// Validation tests for OpenCL env specific checks

#include <string>

#include "gmock/gmock.h"
#include "test/val/val_fixtures.h"

namespace spvtools {
namespace val {
namespace {

using testing::Eq;
using testing::HasSubstr;

using ValidateOpenCL = spvtest::ValidateBase<bool>;

TEST_F(ValidateOpenCL, NonPhysicalAddressingModelBad) {
  std::string spirv = R"(
     OpCapability Kernel
     OpMemoryModel Logical OpenCL
)";

  CompileSuccessfully(spirv);

  EXPECT_EQ(SPV_ERROR_INVALID_DATA, ValidateInstructions(SPV_ENV_OPENCL_1_2));
  EXPECT_THAT(getDiagnosticString(),
              HasSubstr("Addressing model must be Physical32 or Physical64 "
                        "in the OpenCL environment.\n  OpMemoryModel Logical "
                        "OpenCL\n"));
}

TEST_F(ValidateOpenCL, NonOpenCLMemoryModelBad) {
  std::string spirv = R"(
     OpCapability Kernel
     OpCapability Addresses
     OpCapability VulkanMemoryModelKHR
     OpExtension "SPV_KHR_vulkan_memory_model"
     OpMemoryModel Physical32 VulkanKHR
)";

  CompileSuccessfully(spirv);

  EXPECT_EQ(SPV_ERROR_INVALID_DATA, ValidateInstructions(SPV_ENV_OPENCL_1_2));
  EXPECT_THAT(
      getDiagnosticString(),
      HasSubstr("Memory model must be OpenCL in the OpenCL environment."));
}

TEST_F(ValidateOpenCL, NonVoidSampledTypeImageBad) {
  std::string spirv = R"(
    OpCapability Addresses
    OpCapability Kernel
    OpMemoryModel Physical32 OpenCL
    %1 = OpTypeInt 32 0
    %2 = OpTypeImage %1 2D 0 0 0 0 Unknown ReadOnly
)";

  CompileSuccessfully(spirv);

  EXPECT_EQ(SPV_ERROR_INVALID_DATA, ValidateInstructions(SPV_ENV_OPENCL_1_2));
  EXPECT_THAT(
      getDiagnosticString(),
      HasSubstr("Sampled Type must be OpTypeVoid in the OpenCL environment."
                "\n  %2 = OpTypeImage %uint 2D 0 0 0 0 Unknown ReadOnly\n"));
}

TEST_F(ValidateOpenCL, NonZeroMSImageBad) {
  std::string spirv = R"(
    OpCapability Addresses
    OpCapability Kernel
    OpMemoryModel Physical32 OpenCL
    %1 = OpTypeVoid
    %2 = OpTypeImage %1 2D 0 0 1 0 Unknown ReadOnly
)";

  CompileSuccessfully(spirv);

  EXPECT_EQ(SPV_ERROR_INVALID_DATA, ValidateInstructions(SPV_ENV_OPENCL_1_2));
  EXPECT_THAT(
      getDiagnosticString(),
      HasSubstr("MS must be 0 in the OpenCL environment."
                "\n  %2 = OpTypeImage %void 2D 0 0 1 0 Unknown ReadOnly\n"));
}

TEST_F(ValidateOpenCL, Non1D2DArrayedImageBad) {
  std::string spirv = R"(
    OpCapability Addresses
    OpCapability Kernel
    OpMemoryModel Physical32 OpenCL
    %1 = OpTypeVoid
    %2 = OpTypeImage %1 3D 0 1 0 0 Unknown ReadOnly
)";

  CompileSuccessfully(spirv);

  EXPECT_EQ(SPV_ERROR_INVALID_DATA, ValidateInstructions(SPV_ENV_OPENCL_1_2));
  EXPECT_THAT(
      getDiagnosticString(),
      HasSubstr("In the OpenCL environment, Arrayed may only be set to 1 "
                "when Dim is either 1D or 2D."
                "\n  %2 = OpTypeImage %void 3D 0 1 0 0 Unknown ReadOnly\n"));
}

TEST_F(ValidateOpenCL, NonZeroSampledImageBad) {
  std::string spirv = R"(
    OpCapability Addresses
    OpCapability Kernel
    OpMemoryModel Physical32 OpenCL
    %1 = OpTypeVoid
    %2 = OpTypeImage %1 3D 0 0 0 1 Unknown ReadOnly
)";

  CompileSuccessfully(spirv);

  EXPECT_EQ(SPV_ERROR_INVALID_DATA, ValidateInstructions(SPV_ENV_OPENCL_1_2));
  EXPECT_THAT(
      getDiagnosticString(),
      HasSubstr("Sampled must be 0 in the OpenCL environment."
                "\n  %2 = OpTypeImage %void 3D 0 0 0 1 Unknown ReadOnly\n"));
}

TEST_F(ValidateOpenCL, NoAccessQualifierImageBad) {
  std::string spirv = R"(
    OpCapability Addresses
    OpCapability Kernel
    OpMemoryModel Physical32 OpenCL
    %1 = OpTypeVoid
    %2 = OpTypeImage %1 3D 0 0 0 0 Unknown
)";

  CompileSuccessfully(spirv);

  EXPECT_EQ(SPV_ERROR_INVALID_DATA, ValidateInstructions(SPV_ENV_OPENCL_1_2));
  EXPECT_THAT(getDiagnosticString(),
              HasSubstr("In the OpenCL environment, the optional "
                        "Access Qualifier must be present."
                        "\n  %2 = OpTypeImage %void 3D 0 0 0 0 Unknown\n"));
}

TEST_F(ValidateOpenCL, ImageWriteWithOptionalImageOperandsBad) {
  std::string spirv = R"(
    OpCapability Addresses
    OpCapability Kernel
    OpCapability ImageBasic
    OpMemoryModel Physical64 OpenCL
    OpEntryPoint Kernel %5 "test"
    %uint = OpTypeInt 32 0
    %uint_7 = OpConstant %uint 7
    %uint_3 = OpConstant %uint 3
    %uint_1 = OpConstant %uint 1
    %uint_2 = OpConstant %uint 2
    %uint_4 = OpConstant %uint 4
    %void = OpTypeVoid
    %3 = OpTypeImage %void 2D 0 0 0 0 Unknown WriteOnly
    %4 = OpTypeFunction %void %3
    %v2uint = OpTypeVector %uint 2
    %v4uint = OpTypeVector %uint 4
    %12 = OpConstantComposite %v2uint %uint_7 %uint_3
    %17 = OpConstantComposite %v4uint %uint_1 %uint_2 %uint_3 %uint_4
    %5 = OpFunction %void None %4
    %img = OpFunctionParameter %3
    %entry = OpLabel
    OpImageWrite %img %12 %17 ConstOffset %12
    OpReturn
    OpFunctionEnd
)";

  CompileSuccessfully(spirv);

  EXPECT_EQ(SPV_ERROR_INVALID_DATA, ValidateInstructions(SPV_ENV_OPENCL_1_2));
  EXPECT_THAT(getDiagnosticString(),
              HasSubstr("Optional Image Operands are not allowed in the "
                        "OpenCL environment."
                        "\n  OpImageWrite %15 %13 %14 ConstOffset %13\n"));
}

TEST_F(ValidateOpenCL, ImageReadWithConstOffsetBad) {
  std::string spirv = R"(
               OpCapability Addresses
               OpCapability Kernel
               OpCapability ImageBasic
               OpMemoryModel Physical64 OpenCL
               OpEntryPoint Kernel %5 "image_kernel"
               OpName %img "img"
               OpName %coord "coord"
               OpName %call "call"
       %uint = OpTypeInt 32 0
     %uint_7 = OpConstant %uint 7
     %uint_3 = OpConstant %uint 3
       %void = OpTypeVoid
          %3 = OpTypeImage %void 2D 0 0 0 0 Unknown ReadOnly
          %4 = OpTypeFunction %void %3
     %v4uint = OpTypeVector %uint 4
     %v2uint = OpTypeVector %uint 2
      %coord = OpConstantComposite %v2uint %uint_7 %uint_3
          %5 = OpFunction %void None %4
        %img = OpFunctionParameter %3
      %entry = OpLabel
       %call = OpImageRead %v4uint %img %coord ConstOffset %coord
               OpReturn
               OpFunctionEnd
)";

  CompileSuccessfully(spirv);

  EXPECT_EQ(SPV_ERROR_INVALID_DATA, ValidateInstructions(SPV_ENV_OPENCL_1_2));
  EXPECT_THAT(
      getDiagnosticString(),
      HasSubstr(
          "ConstOffset image operand not allowed in the OpenCL environment."
          "\n  %call = OpImageRead %v4uint %img %coord ConstOffset %coord\n"));
}

TEST_F(ValidateOpenCL, ImageRead_NonDepthScalarFloatResult_Bad) {
  std::string spirv = R"(
               OpCapability Addresses
               OpCapability Kernel
               OpCapability ImageBasic
               OpMemoryModel Physical64 OpenCL
               OpEntryPoint Kernel %5 "image_kernel"
               OpName %img "img"
               OpName %coord "coord"
               OpName %call "call"
       %uint = OpTypeInt 32 0
     %v2uint = OpTypeVector %uint 2
      %coord = OpConstantNull %v2uint
       %void = OpTypeVoid
      %float = OpTypeFloat 32
          %3 = OpTypeImage %void 2D 0 0 0 0 Unknown ReadOnly
          %4 = OpTypeFunction %void %3
          %5 = OpFunction %void None %4
        %img = OpFunctionParameter %3
      %entry = OpLabel
       %call = OpImageRead %float %img %coord
               OpReturn
               OpFunctionEnd
)";

  CompileSuccessfully(spirv);

  EXPECT_EQ(SPV_ERROR_INVALID_DATA, ValidateInstructions(SPV_ENV_OPENCL_1_2));
  EXPECT_THAT(getDiagnosticString(),
              HasSubstr("Expected Result Type to have 4 components"));
}

TEST_F(ValidateOpenCL, ImageRead_NonDepthScalarIntResult_Bad) {
  std::string spirv = R"(
               OpCapability Addresses
               OpCapability Kernel
               OpCapability ImageBasic
               OpMemoryModel Physical64 OpenCL
               OpEntryPoint Kernel %5 "image_kernel"
               OpName %img "img"
               OpName %coord "coord"
               OpName %call "call"
       %uint = OpTypeInt 32 0
     %v2uint = OpTypeVector %uint 2
      %coord = OpConstantNull %v2uint
       %void = OpTypeVoid
      %float = OpTypeFloat 32
          %3 = OpTypeImage %void 2D 0 0 0 0 Unknown ReadOnly
          %4 = OpTypeFunction %void %3
          %5 = OpFunction %void None %4
        %img = OpFunctionParameter %3
      %entry = OpLabel
       %call = OpImageRead %uint %img %coord
               OpReturn
               OpFunctionEnd
)";

  CompileSuccessfully(spirv);

  EXPECT_EQ(SPV_ERROR_INVALID_DATA, ValidateInstructions(SPV_ENV_OPENCL_1_2));
  EXPECT_THAT(getDiagnosticString(),
              HasSubstr("Expected Result Type to have 4 components"));
}

TEST_F(ValidateOpenCL, ImageRead_NonDepthVector3FloatResult_Bad) {
  std::string spirv = R"(
               OpCapability Addresses
               OpCapability Kernel
               OpCapability ImageBasic
               OpMemoryModel Physical64 OpenCL
               OpEntryPoint Kernel %5 "image_kernel"
               OpName %img "img"
               OpName %coord "coord"
               OpName %call "call"
       %uint = OpTypeInt 32 0
     %v2uint = OpTypeVector %uint 2
      %coord = OpConstantNull %v2uint
       %void = OpTypeVoid
      %float = OpTypeFloat 32
    %v3float = OpTypeVector %float 3
          %3 = OpTypeImage %void 2D 0 0 0 0 Unknown ReadOnly
          %4 = OpTypeFunction %void %3
          %5 = OpFunction %void None %4
        %img = OpFunctionParameter %3
      %entry = OpLabel
       %call = OpImageRead %v3float %img %coord
               OpReturn
               OpFunctionEnd
)";

  CompileSuccessfully(spirv);

  EXPECT_EQ(SPV_ERROR_INVALID_DATA, ValidateInstructions(SPV_ENV_OPENCL_1_2));
  EXPECT_THAT(getDiagnosticString(),
              HasSubstr("Expected Result Type to have 4 components"));
}

TEST_F(ValidateOpenCL, ImageRead_NonDepthVector4FloatResult_Ok) {
  std::string spirv = R"(
               OpCapability Addresses
               OpCapability Kernel
               OpCapability ImageBasic
               OpMemoryModel Physical64 OpenCL
               OpEntryPoint Kernel %5 "image_kernel"
               OpName %img "img"
               OpName %coord "coord"
               OpName %call "call"
       %uint = OpTypeInt 32 0
     %v2uint = OpTypeVector %uint 2
      %coord = OpConstantNull %v2uint
       %void = OpTypeVoid
      %float = OpTypeFloat 32
    %v4float = OpTypeVector %float 4
          %3 = OpTypeImage %void 2D 0 0 0 0 Unknown ReadOnly
          %4 = OpTypeFunction %void %3
          %5 = OpFunction %void None %4
        %img = OpFunctionParameter %3
      %entry = OpLabel
       %call = OpImageRead %v4float %img %coord
               OpReturn
               OpFunctionEnd
)";

  CompileSuccessfully(spirv);

  EXPECT_EQ(SPV_SUCCESS, ValidateInstructions(SPV_ENV_OPENCL_1_2));
  EXPECT_THAT(getDiagnosticString(), Eq(""));
}

TEST_F(ValidateOpenCL, ImageRead_NonDepthVector4IntResult_Ok) {
  std::string spirv = R"(
               OpCapability Addresses
               OpCapability Kernel
               OpCapability ImageBasic
               OpMemoryModel Physical64 OpenCL
               OpEntryPoint Kernel %5 "image_kernel"
               OpName %img "img"
               OpName %coord "coord"
               OpName %call "call"
       %uint = OpTypeInt 32 0
     %v2uint = OpTypeVector %uint 2
      %coord = OpConstantNull %v2uint
       %void = OpTypeVoid
     %v4uint = OpTypeVector %uint 4
          %3 = OpTypeImage %void 2D 0 0 0 0 Unknown ReadOnly
          %4 = OpTypeFunction %void %3
          %5 = OpFunction %void None %4
        %img = OpFunctionParameter %3
      %entry = OpLabel
       %call = OpImageRead %v4uint %img %coord
               OpReturn
               OpFunctionEnd
)";

  CompileSuccessfully(spirv);

  EXPECT_EQ(SPV_SUCCESS, ValidateInstructions(SPV_ENV_OPENCL_1_2));
  EXPECT_THAT(getDiagnosticString(), Eq(""));
}

TEST_F(ValidateOpenCL, ImageRead_DepthScalarFloatResult_Ok) {
  std::string spirv = R"(
               OpCapability Addresses
               OpCapability Kernel
               OpCapability ImageBasic
               OpMemoryModel Physical64 OpenCL
               OpEntryPoint Kernel %5 "image_kernel"
               OpName %img "img"
               OpName %coord "coord"
               OpName %call "call"
       %uint = OpTypeInt 32 0
     %v2uint = OpTypeVector %uint 2
      %coord = OpConstantNull %v2uint
       %void = OpTypeVoid
      %float = OpTypeFloat 32
          %3 = OpTypeImage %void 2D 1 0 0 0 Unknown ReadOnly
          %4 = OpTypeFunction %void %3
          %5 = OpFunction %void None %4
        %img = OpFunctionParameter %3
      %entry = OpLabel
       %call = OpImageRead %float %img %coord
               OpReturn
               OpFunctionEnd
)";

  CompileSuccessfully(spirv);

  EXPECT_EQ(SPV_SUCCESS, ValidateInstructions(SPV_ENV_OPENCL_1_2));
  EXPECT_THAT(getDiagnosticString(), Eq(""));
}

TEST_F(ValidateOpenCL, ImageRead_DepthScalarIntResult_Bad) {
  std::string spirv = R"(
               OpCapability Addresses
               OpCapability Kernel
               OpCapability ImageBasic
               OpMemoryModel Physical64 OpenCL
               OpEntryPoint Kernel %5 "image_kernel"
               OpName %img "img"
               OpName %coord "coord"
               OpName %call "call"
       %uint = OpTypeInt 32 0
     %v2uint = OpTypeVector %uint 2
      %coord = OpConstantNull %v2uint
       %void = OpTypeVoid
      %float = OpTypeFloat 32
          %3 = OpTypeImage %void 2D 1 0 0 0 Unknown ReadOnly
          %4 = OpTypeFunction %void %3
          %5 = OpFunction %void None %4
        %img = OpFunctionParameter %3
      %entry = OpLabel
       %call = OpImageRead %uint %img %coord
               OpReturn
               OpFunctionEnd
)";

  CompileSuccessfully(spirv);

  EXPECT_EQ(SPV_ERROR_INVALID_DATA, ValidateInstructions(SPV_ENV_OPENCL_1_2));
  EXPECT_THAT(getDiagnosticString(),
              HasSubstr("Expected Result Type from a depth image "
                        "read to result in a scalar float value"));
}

TEST_F(ValidateOpenCL, ImageRead_DepthVectorFloatResult_Bad) {
  std::string spirv = R"(
               OpCapability Addresses
               OpCapability Kernel
               OpCapability ImageBasic
               OpMemoryModel Physical64 OpenCL
               OpEntryPoint Kernel %5 "image_kernel"
               OpName %img "img"
               OpName %coord "coord"
               OpName %call "call"
       %uint = OpTypeInt 32 0
     %v2uint = OpTypeVector %uint 2
      %coord = OpConstantNull %v2uint
       %void = OpTypeVoid
      %float = OpTypeFloat 32
    %v4float = OpTypeVector %float 4
          %3 = OpTypeImage %void 2D 1 0 0 0 Unknown ReadOnly
          %4 = OpTypeFunction %void %3
          %5 = OpFunction %void None %4
        %img = OpFunctionParameter %3
      %entry = OpLabel
       %call = OpImageRead %v4float %img %coord
               OpReturn
               OpFunctionEnd
)";

  CompileSuccessfully(spirv);

  EXPECT_EQ(SPV_ERROR_INVALID_DATA, ValidateInstructions(SPV_ENV_OPENCL_1_2));
  EXPECT_THAT(getDiagnosticString(),
              HasSubstr("Expected Result Type from a depth image "
                        "read to result in a scalar float value"));
}

TEST_F(ValidateOpenCL, ImageSampleExplicitLodWithConstOffsetBad) {
  std::string spirv = R"(
               OpCapability Addresses
               OpCapability Kernel
               OpCapability ImageBasic
               OpCapability LiteralSampler
               OpMemoryModel Physical64 OpenCL
               OpEntryPoint Kernel %5 "image_kernel"
               OpName %img "img"
               OpName %coord "coord"
               OpName %call "call"
       %uint = OpTypeInt 32 0
     %v2uint = OpTypeVector %uint 2
      %coord = OpConstantNull %v2uint
       %void = OpTypeVoid
          %3 = OpTypeImage %void 2D 0 0 0 0 Unknown ReadOnly
          %4 = OpTypeFunction %void %3
          %8 = OpTypeSampler
         %10 = OpTypeSampledImage %3
     %v4uint = OpTypeVector %uint 4
      %float = OpTypeFloat 32
          %9 = OpConstantSampler %8 None 0 Nearest
    %float_0 = OpConstant %float 0
          %5 = OpFunction %void None %4
          %6 = OpFunctionParameter %3
      %entry = OpLabel
        %img = OpSampledImage %10 %6 %9
       %call = OpImageSampleExplicitLod %v4uint %img %coord
                                        Lod|ConstOffset %float_0 %coord
               OpReturn
               OpFunctionEnd
)";

  CompileSuccessfully(spirv);

  EXPECT_EQ(SPV_ERROR_INVALID_DATA, ValidateInstructions(SPV_ENV_OPENCL_1_2));
  EXPECT_THAT(
      getDiagnosticString(),
      HasSubstr(
          "ConstOffset image operand not allowed in the OpenCL environment."
          "\n  %call = OpImageSampleExplicitLod %v4uint %img "
          "%coord Lod|ConstOffset %float_0 %coord\n"));
}

}  // namespace
}  // namespace val
}  // namespace spvtools
