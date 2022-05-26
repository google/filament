// Copyright (c) 2021 Google LLC
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

// Validation tests for decorations

#include <string>
#include <vector>

#include "gmock/gmock.h"
#include "test/test_fixture.h"
#include "test/unit_spirv.h"
#include "test/val/val_code_generator.h"
#include "test/val/val_fixtures.h"

namespace spvtools {
namespace val {
namespace {

using ::testing::Combine;
using ::testing::Eq;
using ::testing::HasSubstr;
using ::testing::Values;

using DecorationTest = spvtest::ValidateBase<bool>;

TEST_F(DecorationTest, WorkgroupSizeShader) {
  const std::string text = R"(
OpCapability Shader
OpCapability Linkage
OpMemoryModel Logical GLSL450
OpDecorate %ones BuiltIn WorkgroupSize
%int = OpTypeInt 32 0
%int3 = OpTypeVector %int 3
%int_1 = OpConstant %int 1
%ones = OpConstantComposite %int3 %int_1 %int_1 %int_1
)";

  CompileSuccessfully(text);
  EXPECT_EQ(SPV_SUCCESS, ValidateInstructions());
}

TEST_F(DecorationTest, WorkgroupSizeKernel) {
  const std::string text = R"(
OpCapability Kernel
OpCapability Linkage
OpMemoryModel Logical OpenCL
OpDecorate %var BuiltIn WorkgroupSize
%int = OpTypeInt 32 0
%int3 = OpTypeVector %int 3
%ptr = OpTypePointer Input %int3
%var = OpVariable %ptr Input
)";

  CompileSuccessfully(text);
  EXPECT_EQ(SPV_SUCCESS, ValidateInstructions());
}

using MemberOnlyDecorations = spvtest::ValidateBase<std::string>;

TEST_P(MemberOnlyDecorations, MemberDecoration) {
  const auto deco = GetParam();
  const std::string text = R"(
OpCapability Shader
OpCapability Linkage
OpMemoryModel Logical GLSL450
OpMemberDecorate %struct 0 )" +
                           deco + R"(
%float = OpTypeFloat 32
%float2 = OpTypeVector %float 2
%float2x2 = OpTypeMatrix %float2 2
%struct = OpTypeStruct %float2x2
)";

  CompileSuccessfully(text);
  EXPECT_EQ(SPV_SUCCESS, ValidateInstructions());
}

TEST_P(MemberOnlyDecorations, Decoration) {
  const auto deco = GetParam();
  const std::string text = R"(
OpCapability Shader
OpCapability Linkage
OpMemoryModel Logical GLSL450
OpDecorate %struct )" + deco +
                           R"(
%float = OpTypeFloat 32
%float2 = OpTypeVector %float 2
%float2x2 = OpTypeMatrix %float2 2
%struct = OpTypeStruct %float2x2
)";

  CompileSuccessfully(text);
  EXPECT_EQ(SPV_ERROR_INVALID_ID, ValidateInstructions());
  EXPECT_THAT(getDiagnosticString(),
              HasSubstr("can only be applied to structure members"));
}

INSTANTIATE_TEST_SUITE_P(ValidateMemberOnlyDecorations, MemberOnlyDecorations,
                         Values("RowMajor", "ColMajor", "MatrixStride 16"
                                // SPIR-V spec bug?
                                /*,"Offset 0"*/));

using NonMemberOnlyDecorations = spvtest::ValidateBase<std::string>;

TEST_P(NonMemberOnlyDecorations, MemberDecoration) {
  const auto deco = GetParam();
  const auto text = R"(
OpCapability Shader
OpCapability Kernel
OpCapability Linkage
OpCapability InputAttachment
OpCapability Addresses
OpCapability PhysicalStorageBufferAddresses
OpCapability ShaderNonUniform
OpExtension "SPV_KHR_no_integer_wrap_decoration"
OpExtension "SPV_KHR_physical_storage_buffer"
OpExtension "SPV_GOOGLE_hlsl_functionality1"
OpExtension "SPV_EXT_descriptor_indexing"
OpMemoryModel Logical GLSL450
OpMemberDecorate %struct 0 )" +
                    deco + R"(
%float = OpTypeFloat 32
%float2 = OpTypeVector %float 2
%float2x2 = OpTypeMatrix %float2 2
%struct = OpTypeStruct %float2x2
)";

  CompileSuccessfully(text, SPV_ENV_UNIVERSAL_1_3);
  EXPECT_EQ(SPV_ERROR_INVALID_ID, ValidateInstructions(SPV_ENV_UNIVERSAL_1_3));
  EXPECT_THAT(getDiagnosticString(),
              HasSubstr("cannot be applied to structure members"));
}

INSTANTIATE_TEST_SUITE_P(
    ValidateNonMemberOnlyDecorations, NonMemberOnlyDecorations,
    Values("SpecId 1", "Block", "BufferBlock", "ArrayStride 4", "GLSLShared",
           "GLSLPacked", "CPacked",
           // TODO: https://github.com/KhronosGroup/glslang/issues/703:
           // glslang applies Restrict to structure members.
           //"Restrict",
           "Aliased", "Constant", "Uniform", "SaturatedConversion", "Index 0",
           "Binding 0", "DescriptorSet 0", "FuncParamAttr Zext",
           "FPRoundingMode RTE", "FPFastMathMode None",
           "LinkageAttributes \"ext\" Import", "NoContraction",
           "InputAttachmentIndex 0", "Alignment 4", "MaxByteOffset 4",
           "AlignmentId %float", "MaxByteOffsetId %float", "NoSignedWrap",
           "NoUnsignedWrap", "NonUniform", "RestrictPointer", "AliasedPointer",
           "CounterBuffer %float"));

using StructDecorations = spvtest::ValidateBase<std::string>;

TEST_P(StructDecorations, Struct) {
  const std::string deco = GetParam();
  const std::string text = R"(
OpCapability Shader
OpCapability Kernel
OpCapability Linkage
OpMemoryModel Logical GLSL450
OpDecorate %struct )" + deco +
                           R"(
%struct = OpTypeStruct
)";

  CompileSuccessfully(text);
  EXPECT_EQ(SPV_SUCCESS, ValidateInstructions());
}

TEST_P(StructDecorations, OtherType) {
  const std::string deco = GetParam();
  const std::string text = R"(
OpCapability Shader
OpCapability Kernel
OpCapability Linkage
OpMemoryModel Logical GLSL450
OpDecorate %int )" + deco + R"(
%int = OpTypeInt 32 0
)";

  CompileSuccessfully(text);
  EXPECT_EQ(SPV_ERROR_INVALID_ID, ValidateInstructions());
  EXPECT_THAT(getDiagnosticString(), HasSubstr("must be a structure type"));
}

TEST_P(StructDecorations, Variable) {
  const std::string deco = GetParam();
  const std::string text = R"(
OpCapability Shader
OpCapability Kernel
OpCapability Linkage
OpMemoryModel Logical GLSL450
OpDecorate %var )" + deco + R"(
%int = OpTypeInt 32 0
%ptr = OpTypePointer Private %int
%var = OpVariable %ptr Private
)";

  CompileSuccessfully(text);
  EXPECT_EQ(SPV_ERROR_INVALID_ID, ValidateInstructions());
  EXPECT_THAT(getDiagnosticString(), HasSubstr("must be a structure type"));
}

TEST_P(StructDecorations, FunctionParameter) {
  const auto deco = GetParam();
  const std::string text = R"(
OpCapability Shader
OpCapability Kernel
OpCapability Linkage
OpMemoryModel Logical GLSL450
OpDecorate %func LinkageAttributes "import" Import
OpDecorate %param )" + deco +
                           R"(
%int = OpTypeInt 32 0
%void = OpTypeVoid
%fn = OpTypeFunction %void %int
%func = OpFunction %void None %fn
%param = OpFunctionParameter %int
OpFunctionEnd
)";

  CompileSuccessfully(text);
  EXPECT_EQ(SPV_ERROR_INVALID_ID, ValidateInstructions());
  EXPECT_THAT(getDiagnosticString(), HasSubstr("must be a structure type"));
}

TEST_P(StructDecorations, Constant) {
  const std::string deco = GetParam();
  const std::string text = R"(
OpCapability Shader
OpCapability Kernel
OpCapability Linkage
OpMemoryModel Logical GLSL450
OpDecorate %int_0 )" + deco +
                           R"(
%int = OpTypeInt 32 0
%int_0 = OpConstant %int 0
)";

  CompileSuccessfully(text);
  EXPECT_EQ(SPV_ERROR_INVALID_ID, ValidateInstructions());
  EXPECT_THAT(getDiagnosticString(), HasSubstr("must be a structure type"));
}

INSTANTIATE_TEST_SUITE_P(ValidateStructDecorations, StructDecorations,
                         Values("Block", "BufferBlock", "GLSLShared",
                                "GLSLPacked", "CPacked"));

using ArrayDecorations = spvtest::ValidateBase<std::string>;

TEST_P(ArrayDecorations, Array) {
  const auto deco = GetParam();
  const std::string text = R"(
OpCapability Shader
OpCapability Linkage
OpMemoryModel Logical GLSL450
OpDecorate %array )" + deco +
                           R"(
%int = OpTypeInt 32 0
%int_4 = OpConstant %int 4
%array = OpTypeArray %int %int_4
)";

  CompileSuccessfully(text);
  EXPECT_EQ(SPV_SUCCESS, ValidateInstructions());
}

TEST_P(ArrayDecorations, RuntimeArray) {
  const auto deco = GetParam();
  const std::string text = R"(
OpCapability Shader
OpCapability Linkage
OpMemoryModel Logical GLSL450
OpDecorate %array )" + deco +
                           R"(
%int = OpTypeInt 32 0
%array = OpTypeRuntimeArray %int
)";

  CompileSuccessfully(text);
  EXPECT_EQ(SPV_SUCCESS, ValidateInstructions());
}

TEST_P(ArrayDecorations, Pointer) {
  const auto deco = GetParam();
  const std::string text = R"(
OpCapability Shader
OpCapability Linkage
OpMemoryModel Logical GLSL450
OpDecorate %ptr )" + deco + R"(
%int = OpTypeInt 32 0
%ptr = OpTypePointer Workgroup %int
)";

  CompileSuccessfully(text);
  EXPECT_EQ(SPV_SUCCESS, ValidateInstructions());
}

TEST_P(ArrayDecorations, Struct) {
  const auto deco = GetParam();
  const std::string text = R"(
OpCapability Shader
OpCapability Linkage
OpMemoryModel Logical GLSL450
OpDecorate %struct )" + deco +
                           R"(
%int = OpTypeInt 32 0
%struct = OpTypeStruct %int
)";

  CompileSuccessfully(text);
  EXPECT_EQ(SPV_ERROR_INVALID_ID, ValidateInstructions());
  EXPECT_THAT(getDiagnosticString(),
              HasSubstr("must be an array or pointer type"));
}

TEST_P(ArrayDecorations, Variable) {
  const auto deco = GetParam();
  const std::string text = R"(
OpCapability Shader
OpCapability Linkage
OpMemoryModel Logical GLSL450
OpDecorate %var )" + deco + R"(
%int = OpTypeInt 32 0
%ptr = OpTypePointer Private %int
%var = OpVariable %ptr Private
)";

  CompileSuccessfully(text);
  EXPECT_EQ(SPV_ERROR_INVALID_ID, ValidateInstructions());
  EXPECT_THAT(getDiagnosticString(),
              HasSubstr("must be an array or pointer type"));
}

TEST_P(ArrayDecorations, FunctionParameter) {
  const auto deco = GetParam();
  const std::string text = R"(
OpCapability Shader
OpCapability Linkage
OpMemoryModel Logical GLSL450
OpDecorate %func LinkageAttributes "import" Import
OpDecorate %param )" + deco +
                           R"(
%int = OpTypeInt 32 0
%void = OpTypeVoid
%fn = OpTypeFunction %void %int
%func = OpFunction %void None %fn
%param = OpFunctionParameter %int
OpFunctionEnd
)";

  CompileSuccessfully(text);
  EXPECT_EQ(SPV_ERROR_INVALID_ID, ValidateInstructions());
  EXPECT_THAT(getDiagnosticString(),
              HasSubstr("must be an array or pointer type"));
}

TEST_P(ArrayDecorations, Constant) {
  const auto deco = GetParam();
  const std::string text = R"(
OpCapability Shader
OpCapability Linkage
OpMemoryModel Logical GLSL450
OpDecorate %null )" + deco +
                           R"(
%int = OpTypeInt 32 0
%int_4 = OpConstant %int 4
%array = OpTypeArray %int %int_4
%null = OpConstantNull %array
)";

  CompileSuccessfully(text);
  EXPECT_EQ(SPV_ERROR_INVALID_ID, ValidateInstructions());
  EXPECT_THAT(getDiagnosticString(),
              HasSubstr("must be an array or pointer type"));
}

INSTANTIATE_TEST_SUITE_P(ValidateArrayDecorations, ArrayDecorations,
                         Values("ArrayStride 4"));

using BuiltInDecorations = spvtest::ValidateBase<std::string>;

TEST_P(BuiltInDecorations, Variable) {
  const auto deco = GetParam();
  const std::string text = R"(
OpCapability Shader
OpCapability Linkage
OpMemoryModel Logical GLSL450
OpDecorate %var BuiltIn )" +
                           deco + R"(
%int = OpTypeInt 32 0
%ptr = OpTypePointer Input %int
%var = OpVariable %ptr Input
)";

  CompileSuccessfully(text);
  if (deco != "WorkgroupSize") {
    EXPECT_EQ(SPV_SUCCESS, ValidateInstructions());
  } else {
    EXPECT_EQ(SPV_ERROR_INVALID_ID, ValidateInstructions());
    EXPECT_THAT(getDiagnosticString(),
                HasSubstr("must be a constant for WorkgroupSize"));
  }
}

TEST_P(BuiltInDecorations, IntegerType) {
  const auto deco = GetParam();
  const std::string text = R"(
OpCapability Shader
OpCapability Linkage
OpMemoryModel Logical GLSL450
OpDecorate %int BuiltIn )" +
                           deco + R"(
%int = OpTypeInt 32 0
)";

  CompileSuccessfully(text);
  EXPECT_EQ(SPV_ERROR_INVALID_DATA, ValidateInstructions());
  EXPECT_THAT(getDiagnosticString(),
              HasSubstr("BuiltIns can only target variables, structure members "
                        "or constants"));
}

TEST_P(BuiltInDecorations, FunctionParameter) {
  const auto deco = GetParam();
  const std::string text = R"(
OpCapability Shader
OpCapability Linkage
OpMemoryModel Logical GLSL450
OpDecorate %func LinkageAttributes "import" Import
OpDecorate %param BuiltIn )" +
                           deco + R"(
%int = OpTypeInt 32 0
%void = OpTypeVoid
%fn = OpTypeFunction %void %int
%func = OpFunction %void None %fn
%param = OpFunctionParameter %int
OpFunctionEnd
)";

  CompileSuccessfully(text);
  EXPECT_EQ(SPV_ERROR_INVALID_DATA, ValidateInstructions());
  EXPECT_THAT(getDiagnosticString(),
              HasSubstr("BuiltIns can only target variables, structure members "
                        "or constants"));
}

TEST_P(BuiltInDecorations, Constant) {
  const auto deco = GetParam();
  const std::string text = R"(
OpCapability Shader
OpCapability Linkage
OpMemoryModel Logical GLSL450
OpDecorate %const BuiltIn )" +
                           deco + R"(
%int = OpTypeInt 32 0
%int3 = OpTypeVector %int 3
%int_1 = OpConstant %int 1
%const = OpConstantComposite %int3 %int_1 %int_1 %int_1
)";

  CompileSuccessfully(text);
  if (deco == "WorkgroupSize") {
    EXPECT_EQ(SPV_SUCCESS, ValidateInstructions());
  } else {
    EXPECT_EQ(SPV_ERROR_INVALID_ID, ValidateInstructions());
    EXPECT_THAT(getDiagnosticString(), HasSubstr("must be a variable"));
  }
}

TEST_P(BuiltInDecorations, SpecConstant) {
  const auto deco = GetParam();
  const std::string text = R"(
OpCapability Shader
OpCapability Linkage
OpMemoryModel Logical GLSL450
OpDecorate %const BuiltIn )" +
                           deco + R"(
%int = OpTypeInt 32 0
%int3 = OpTypeVector %int 3
%int_1 = OpConstant %int 1
%const = OpSpecConstantComposite %int3 %int_1 %int_1 %int_1
)";

  CompileSuccessfully(text);
  if (deco == "WorkgroupSize") {
    EXPECT_EQ(SPV_SUCCESS, ValidateInstructions());
  } else {
    EXPECT_EQ(SPV_ERROR_INVALID_ID, ValidateInstructions());
    EXPECT_THAT(getDiagnosticString(), HasSubstr("must be a variable"));
  }
}

INSTANTIATE_TEST_SUITE_P(ValidateBuiltInDecorations, BuiltInDecorations,
                         Values("Position", "PointSize", "VertexId",
                                "InstanceId", "FragCoord", "FrontFacing",
                                "NumWorkgroups", "WorkgroupSize",
                                "LocalInvocationId", "GlobalInvocationId"));

using MemoryObjectDecorations = spvtest::ValidateBase<std::string>;

TEST_P(MemoryObjectDecorations, Variable) {
  const auto deco = GetParam();
  const std::string text = R"(
OpCapability Shader
OpCapability Linkage
OpCapability SampleRateShading
OpCapability TransformFeedback
OpCapability GeometryStreams
OpCapability Tessellation
OpCapability PhysicalStorageBufferAddresses
OpExtension "SPV_KHR_physical_storage_buffer"
OpMemoryModel Logical GLSL450
OpDecorate %var )" + deco + R"(
%float = OpTypeFloat 32
%ptr = OpTypePointer Input %float
%var = OpVariable %ptr Input
)";

  CompileSuccessfully(text);
  EXPECT_EQ(SPV_SUCCESS, ValidateInstructions());
}

TEST_P(MemoryObjectDecorations, FunctionParameterGood) {
  const auto deco = GetParam();
  const std::string text = R"(
OpCapability Shader
OpCapability Linkage
OpCapability SampleRateShading
OpCapability TransformFeedback
OpCapability GeometryStreams
OpCapability Tessellation
OpCapability PhysicalStorageBufferAddresses
OpExtension "SPV_KHR_physical_storage_buffer"
OpMemoryModel Logical GLSL450
OpDecorate %func LinkageAttributes "import" Import
OpDecorate %param )" + deco +
                           R"(
%float = OpTypeFloat 32
%ptr = OpTypePointer Input %float
%void = OpTypeVoid
%fn = OpTypeFunction %void %ptr
%func = OpFunction %void None %fn
%param = OpFunctionParameter %ptr
OpFunctionEnd
)";

  CompileSuccessfully(text);
  EXPECT_EQ(SPV_SUCCESS, ValidateInstructions());
}

TEST_P(MemoryObjectDecorations, FunctionParameterNotAPointer) {
  const auto deco = GetParam();
  const std::string text = R"(
OpCapability Shader
OpCapability Linkage
OpCapability SampleRateShading
OpCapability TransformFeedback
OpCapability GeometryStreams
OpCapability Tessellation
OpCapability PhysicalStorageBufferAddresses
OpExtension "SPV_KHR_physical_storage_buffer"
OpMemoryModel Logical GLSL450
OpDecorate %func LinkageAttributes "import" Import
OpDecorate %param )" + deco +
                           R"(
%float = OpTypeFloat 32
%void = OpTypeVoid
%fn = OpTypeFunction %void %float
%func = OpFunction %void None %fn
%param = OpFunctionParameter %float
OpFunctionEnd
)";

  CompileSuccessfully(text);
  EXPECT_EQ(SPV_ERROR_INVALID_ID, ValidateInstructions());
  EXPECT_THAT(getDiagnosticString(), HasSubstr("must be a pointer type"));
}

TEST_P(MemoryObjectDecorations, FloatType) {
  const auto deco = GetParam();
  const std::string text = R"(
OpCapability Shader
OpCapability Linkage
OpCapability SampleRateShading
OpCapability TransformFeedback
OpCapability GeometryStreams
OpCapability Tessellation
OpCapability PhysicalStorageBufferAddresses
OpExtension "SPV_KHR_physical_storage_buffer"
OpMemoryModel Logical GLSL450
OpDecorate %float )" + deco +
                           R"(
%float = OpTypeFloat 32
)";

  CompileSuccessfully(text);
  EXPECT_EQ(SPV_ERROR_INVALID_ID, ValidateInstructions());
  EXPECT_THAT(getDiagnosticString(),
              HasSubstr("must be a memory object declaration"));
}

TEST_P(MemoryObjectDecorations, Constant) {
  const auto deco = GetParam();
  const std::string text = R"(
OpCapability Shader
OpCapability Linkage
OpCapability SampleRateShading
OpCapability TransformFeedback
OpCapability GeometryStreams
OpCapability Tessellation
OpCapability PhysicalStorageBufferAddresses
OpExtension "SPV_KHR_physical_storage_buffer"
OpMemoryModel Logical GLSL450
OpDecorate %const )" + deco +
                           R"(
%float = OpTypeFloat 32
%const = OpConstant %float 0
)";

  CompileSuccessfully(text);
  EXPECT_EQ(SPV_ERROR_INVALID_ID, ValidateInstructions());
  EXPECT_THAT(getDiagnosticString(),
              HasSubstr("must be a memory object declaration"));
}

// NonWritable and NonReadable are covered by other tests.
INSTANTIATE_TEST_SUITE_P(
    ValidateMemoryObjectDecorations, MemoryObjectDecorations,
    Values("NoPerspective", "Flat", "Patch", "Centroid", "Component 0",
           "Sample", "Restrict", "Aliased", "Volatile", "Coherent", "Stream 0",
           "XfbBuffer 1", "XfbStride 1", "AliasedPointer", "RestrictPointer"));

using VariableDecorations = spvtest::ValidateBase<std::string>;

TEST_P(VariableDecorations, Variable) {
  const auto deco = GetParam();
  const std::string text = R"(
OpCapability Shader
OpCapability Kernel
OpCapability Linkage
OpCapability InputAttachment
OpMemoryModel Logical GLSL450
OpDecorate %var )" + deco + R"(
%float = OpTypeFloat 32
%ptr = OpTypePointer Input %float
%var = OpVariable %ptr Input
)";

  CompileSuccessfully(text);
  EXPECT_EQ(SPV_SUCCESS, ValidateInstructions());
}

TEST_P(VariableDecorations, FunctionParameter) {
  const auto deco = GetParam();
  const std::string text = R"(
OpCapability Shader
OpCapability Kernel
OpCapability Linkage
OpCapability InputAttachment
OpMemoryModel Logical GLSL450
OpDecorate %func LinkageAttributes "import" Import
OpDecorate %param )" + deco +
                           R"(
%float = OpTypeFloat 32
%void = OpTypeVoid
%fn = OpTypeFunction %void %float
%func = OpFunction %void None %fn
%param = OpFunctionParameter %float
OpFunctionEnd
)";

  CompileSuccessfully(text);
  EXPECT_EQ(SPV_ERROR_INVALID_ID, ValidateInstructions());
  EXPECT_THAT(getDiagnosticString(), HasSubstr("must be a variable"));
}

TEST_P(VariableDecorations, FloatType) {
  const auto deco = GetParam();
  const std::string text = R"(
OpCapability Shader
OpCapability Kernel
OpCapability Linkage
OpCapability InputAttachment
OpMemoryModel Logical GLSL450
OpDecorate %float )" + deco +
                           R"(
%float = OpTypeFloat 32
)";

  CompileSuccessfully(text);
  EXPECT_EQ(SPV_ERROR_INVALID_ID, ValidateInstructions());
  EXPECT_THAT(getDiagnosticString(), HasSubstr("must be a variable"));
}

TEST_P(VariableDecorations, Constant) {
  const auto deco = GetParam();
  const std::string text = R"(
OpCapability Shader
OpCapability Kernel
OpCapability Linkage
OpCapability InputAttachment
OpMemoryModel Logical GLSL450
OpDecorate %const )" + deco +
                           R"(
%float = OpTypeFloat 32
%const = OpConstant %float 0
)";

  CompileSuccessfully(text);
  EXPECT_EQ(SPV_ERROR_INVALID_ID, ValidateInstructions());
  EXPECT_THAT(getDiagnosticString(), HasSubstr("must be a variable"));
}

INSTANTIATE_TEST_SUITE_P(ValidateVariableDecorations, VariableDecorations,
                         Values("Invariant", "Constant", "Location 0",
                                "Index 0", "Binding 0", "DescriptorSet 0"));

using VulkanIOStorageClass =
    spvtest::ValidateBase<std::tuple<std::string, std::string>>;

TEST_P(VulkanIOStorageClass, Invalid) {
  const auto deco = std::get<0>(GetParam());
  const auto sc = std::get<1>(GetParam());
  const std::string text = R"(
OpCapability Shader
OpExtension "SPV_KHR_storage_buffer_storage_class"
OpMemoryModel Logical GLSL450
OpEntryPoint Fragment %main "main"
OpExecutionMode %main OriginUpperLeft
OpDecorate %var )" + deco + R"( 0
%void = OpTypeVoid
%float = OpTypeFloat 32
%ptr = OpTypePointer )" +
                           sc +
                           R"( %float
%var = OpVariable %ptr )" + sc +
                           R"(
%void_fn = OpTypeFunction %void
%main = OpFunction %void None %void_fn
%entry = OpLabel
OpReturn
OpFunctionEnd
)";

  CompileSuccessfully(text, SPV_ENV_VULKAN_1_0);
  EXPECT_EQ(SPV_ERROR_INVALID_ID, ValidateInstructions(SPV_ENV_VULKAN_1_0));
  EXPECT_THAT(getDiagnosticString(),
              AnyVUID("VUID-StandaloneSpirv-Location-06672"));
  EXPECT_THAT(
      getDiagnosticString(),
      HasSubstr("decoration must not be applied to this storage class"));
}

INSTANTIATE_TEST_SUITE_P(ValidateVulkanIOStorageClass, VulkanIOStorageClass,
                         Combine(Values("Location", "Component"),
                                 Values("StorageBuffer", "Uniform",
                                        "UniformConstant", "Workgroup",
                                        "Private")));

using VulkanResourceStorageClass =
    spvtest::ValidateBase<std::tuple<std::string, std::string>>;

TEST_P(VulkanResourceStorageClass, Invalid) {
  const auto deco = std::get<0>(GetParam());
  const auto sc = std::get<1>(GetParam());
  const std::string text = R"(
OpCapability Shader
OpMemoryModel Logical GLSL450
OpEntryPoint Fragment %main "main"
OpExecutionMode %main OriginUpperLeft
OpDecorate %var )" + deco + R"( 0
%void = OpTypeVoid
%float = OpTypeFloat 32
%ptr = OpTypePointer )" +
                           sc +
                           R"( %float
%var = OpVariable %ptr )" + sc +
                           R"(
%void_fn = OpTypeFunction %void
%main = OpFunction %void None %void_fn
%entry = OpLabel
OpReturn
OpFunctionEnd
)";

  CompileSuccessfully(text, SPV_ENV_VULKAN_1_0);
  EXPECT_EQ(SPV_ERROR_INVALID_ID, ValidateInstructions(SPV_ENV_VULKAN_1_0));
  EXPECT_THAT(getDiagnosticString(),
              HasSubstr("VUID-StandaloneSpirv-DescriptorSet-06491"));
  EXPECT_THAT(getDiagnosticString(),
              HasSubstr("must be in the StorageBuffer, Uniform, or "
                        "UniformConstant storage class"));
}

INSTANTIATE_TEST_SUITE_P(ValidateVulkanResourceStorageClass,
                         VulkanResourceStorageClass,
                         Combine(Values("DescriptorSet", "Binding"),
                                 Values("Private", "Input", "Output",
                                        "Workgroup")));

using VulkanInterpolationStorageClass = spvtest::ValidateBase<std::string>;

TEST_P(VulkanInterpolationStorageClass, Input) {
  const auto deco = GetParam();
  const std::string text = R"(
OpCapability Shader
OpCapability SampleRateShading
OpMemoryModel Logical GLSL450
OpEntryPoint Fragment %main "main"
OpExecutionMode %main OriginUpperLeft
OpDecorate %var )" + deco + R"(
%void = OpTypeVoid
%float = OpTypeFloat 32
%void_fn = OpTypeFunction %void
%ptr = OpTypePointer Input %float
%var = OpVariable %ptr Input
%main = OpFunction %void None %void_fn
%entry = OpLabel
OpReturn
OpFunctionEnd
)";

  CompileSuccessfully(text, SPV_ENV_VULKAN_1_0);
  EXPECT_EQ(SPV_SUCCESS, ValidateInstructions(SPV_ENV_VULKAN_1_0));
}

TEST_P(VulkanInterpolationStorageClass, Output) {
  const auto deco = GetParam();
  const std::string text = R"(
OpCapability Shader
OpCapability SampleRateShading
OpMemoryModel Logical GLSL450
OpEntryPoint Vertex %main "main"
OpDecorate %var )" + deco + R"(
%void = OpTypeVoid
%float = OpTypeFloat 32
%void_fn = OpTypeFunction %void
%ptr = OpTypePointer Output %float
%var = OpVariable %ptr Output
%main = OpFunction %void None %void_fn
%entry = OpLabel
OpReturn
OpFunctionEnd
)";

  CompileSuccessfully(text, SPV_ENV_VULKAN_1_0);
  EXPECT_EQ(SPV_SUCCESS, ValidateInstructions(SPV_ENV_VULKAN_1_0));
}

TEST_P(VulkanInterpolationStorageClass, Private) {
  const auto deco = GetParam();
  const std::string text = R"(
OpCapability Shader
OpCapability SampleRateShading
OpMemoryModel Logical GLSL450
OpEntryPoint Fragment %main "main"
OpExecutionMode %main OriginUpperLeft
OpDecorate %var )" + deco + R"(
%void = OpTypeVoid
%float = OpTypeFloat 32
%void_fn = OpTypeFunction %void
%ptr = OpTypePointer Private %float
%var = OpVariable %ptr Private
%main = OpFunction %void None %void_fn
%entry = OpLabel
OpReturn
OpFunctionEnd
)";

  CompileSuccessfully(text, SPV_ENV_VULKAN_1_0);
  EXPECT_EQ(SPV_ERROR_INVALID_ID, ValidateInstructions(SPV_ENV_VULKAN_1_0));
  EXPECT_THAT(getDiagnosticString(),
              HasSubstr("storage class must be Input or Output"));
  EXPECT_THAT(getDiagnosticString(),
              HasSubstr("[VUID-StandaloneSpirv-Flat-04670"));
}

TEST_P(VulkanInterpolationStorageClass, Uniform) {
  const auto deco = GetParam();
  const std::string text = R"(
OpCapability Shader
OpCapability SampleRateShading
OpMemoryModel Logical GLSL450
OpEntryPoint Fragment %main "main"
OpExecutionMode %main OriginUpperLeft
OpDecorate %var )" + deco + R"(
OpDecorate %var Binding 0
OpDecorate %var DescriptorSet 0
%void = OpTypeVoid
%float = OpTypeFloat 32
%void_fn = OpTypeFunction %void
%ptr = OpTypePointer Uniform %float
%var = OpVariable %ptr Uniform
%main = OpFunction %void None %void_fn
%entry = OpLabel
OpReturn
OpFunctionEnd
)";

  CompileSuccessfully(text, SPV_ENV_VULKAN_1_0);
  EXPECT_EQ(SPV_ERROR_INVALID_ID, ValidateInstructions(SPV_ENV_VULKAN_1_0));
  EXPECT_THAT(getDiagnosticString(),
              HasSubstr("storage class must be Input or Output"));
  EXPECT_THAT(getDiagnosticString(),
              HasSubstr("[VUID-StandaloneSpirv-Flat-04670"));
}

TEST_P(VulkanInterpolationStorageClass, StorageBuffer) {
  const auto deco = GetParam();
  const std::string text = R"(
OpCapability Shader
OpCapability SampleRateShading
OpExtension "SPV_KHR_storage_buffer_storage_class"
OpMemoryModel Logical GLSL450
OpEntryPoint Fragment %main "main"
OpExecutionMode %main OriginUpperLeft
OpDecorate %var )" + deco + R"(
OpDecorate %var Binding 0
OpDecorate %var DescriptorSet 0
%void = OpTypeVoid
%float = OpTypeFloat 32
%void_fn = OpTypeFunction %void
%ptr = OpTypePointer StorageBuffer %float
%var = OpVariable %ptr StorageBuffer
%main = OpFunction %void None %void_fn
%entry = OpLabel
OpReturn
OpFunctionEnd
)";

  CompileSuccessfully(text, SPV_ENV_VULKAN_1_0);
  EXPECT_EQ(SPV_ERROR_INVALID_ID, ValidateInstructions(SPV_ENV_VULKAN_1_0));
  EXPECT_THAT(getDiagnosticString(),
              HasSubstr("storage class must be Input or Output"));
  EXPECT_THAT(getDiagnosticString(),
              HasSubstr("[VUID-StandaloneSpirv-Flat-04670"));
}

INSTANTIATE_TEST_SUITE_P(ValidateVulkanInterpolationStorageClass,
                         VulkanInterpolationStorageClass,
                         Values("Flat", "NoPerspective", "Centroid", "Sample"));

}  // namespace
}  // namespace val
}  // namespace spvtools
