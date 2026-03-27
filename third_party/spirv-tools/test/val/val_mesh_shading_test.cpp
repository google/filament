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

// Tests instructions from SPV_EXT_mesh_shader

#include <string>

#include "gmock/gmock.h"
#include "test/val/val_fixtures.h"

namespace spvtools {
namespace val {
namespace {

using ::testing::HasSubstr;
using ::testing::Values;

using ValidateMeshShading = spvtest::ValidateBase<bool>;

TEST_F(ValidateMeshShading, EmitMeshTasksEXTNotLastInstructionUniversal) {
  const std::string body = R"(
               OpCapability MeshShadingEXT
               OpExtension "SPV_EXT_mesh_shader"
               OpMemoryModel Logical GLSL450
               OpEntryPoint TaskEXT %main "main" %p
               OpExecutionModeId %main LocalSizeId %uint_1 %uint_1 %uint_1
       %void = OpTypeVoid
       %func = OpTypeFunction %void
       %uint = OpTypeInt 32 0
     %uint_1 = OpConstant %uint 1
      %float = OpTypeFloat 32
  %arr_float = OpTypeArray %float %uint_1
    %Payload = OpTypeStruct %arr_float
%ptr_Payload = OpTypePointer TaskPayloadWorkgroupEXT %Payload
          %p = OpVariable %ptr_Payload TaskPayloadWorkgroupEXT
       %main = OpFunction %void None %func
     %label1 = OpLabel
               OpEmitMeshTasksEXT %uint_1 %uint_1 %uint_1 %p
               OpBranch %label2
     %label2 = OpLabel
               OpReturn
               OpFunctionEnd
)";

  CompileSuccessfully(body, SPV_ENV_UNIVERSAL_1_4);
  EXPECT_EQ(SPV_ERROR_INVALID_LAYOUT,
            ValidateInstructions(SPV_ENV_UNIVERSAL_1_4));
  EXPECT_THAT(getDiagnosticString(),
              HasSubstr("Branch must appear in a block"));
}

TEST_F(ValidateMeshShading, EmitMeshTasksEXTNotLastInstructionVulkan) {
  const std::string body = R"(
               OpCapability MeshShadingEXT
               OpExtension "SPV_EXT_mesh_shader"
               OpMemoryModel Logical GLSL450
               OpEntryPoint TaskEXT %main "main" %p
               OpExecutionModeId %main LocalSizeId %uint_1 %uint_1 %uint_1
       %void = OpTypeVoid
       %func = OpTypeFunction %void
       %uint = OpTypeInt 32 0
     %uint_1 = OpConstant %uint 1
      %float = OpTypeFloat 32
  %arr_float = OpTypeArray %float %uint_1
    %Payload = OpTypeStruct %arr_float
%ptr_Payload = OpTypePointer TaskPayloadWorkgroupEXT %Payload
          %p = OpVariable %ptr_Payload TaskPayloadWorkgroupEXT
       %main = OpFunction %void None %func
     %label1 = OpLabel
               OpEmitMeshTasksEXT %uint_1 %uint_1 %uint_1 %p
               OpReturn
               OpFunctionEnd
)";

  CompileSuccessfully(body, SPV_ENV_VULKAN_1_2);
  EXPECT_EQ(SPV_ERROR_INVALID_LAYOUT, ValidateInstructions(SPV_ENV_VULKAN_1_2));
  EXPECT_THAT(getDiagnosticString(),
              HasSubstr("Return must appear in a block"));
}

TEST_F(ValidateMeshShading, BasicTaskSuccess) {
  const std::string body = R"(
               OpCapability MeshShadingEXT
               OpExtension "SPV_EXT_mesh_shader"
               OpMemoryModel Logical GLSL450
               OpEntryPoint TaskEXT %main "main"
       %void = OpTypeVoid
       %func = OpTypeFunction %void
       %main = OpFunction %void None %func
      %label = OpLabel
               OpReturn
               OpFunctionEnd
)";

  CompileSuccessfully(body, SPV_ENV_UNIVERSAL_1_5);
  EXPECT_EQ(SPV_SUCCESS, ValidateInstructions(SPV_ENV_UNIVERSAL_1_5));
}

TEST_F(ValidateMeshShading, BasicMeshSuccess) {
  const std::string body = R"(
               OpCapability MeshShadingEXT
               OpExtension "SPV_EXT_mesh_shader"
               OpMemoryModel Logical GLSL450
               OpEntryPoint MeshEXT %main "main"
               OpExecutionMode %main OutputVertices 1
               OpExecutionMode %main OutputPrimitivesEXT 1
               OpExecutionMode %main OutputTrianglesEXT
       %void = OpTypeVoid
       %func = OpTypeFunction %void
       %main = OpFunction %void None %func
      %label = OpLabel
               OpReturn
               OpFunctionEnd
)";

  CompileSuccessfully(body, SPV_ENV_UNIVERSAL_1_5);
  EXPECT_EQ(SPV_SUCCESS, ValidateInstructions(SPV_ENV_UNIVERSAL_1_5));
}

// https://godbolt.org/z/Kvb1rsceP
TEST_F(ValidateMeshShading, BasicMeshBuiltinSuccess) {
  const std::string body = R"(
               OpCapability MeshShadingEXT
               OpExtension "SPV_EXT_mesh_shader"
               OpMemoryModel Logical GLSL450
               OpEntryPoint MeshEXT %main "main" %gl_MeshVerticesEXT %vertexOutput %gl_MeshPrimitivesEXT %gl_LocalInvocationIndex %gl_PrimitiveTriangleIndicesEXT
               OpExecutionModeId %main LocalSizeId %uint_1 %uint_1 %uint_1
               OpExecutionMode %main OutputVertices 3
               OpExecutionMode %main OutputPrimitivesEXT 1
               OpExecutionMode %main OutputTrianglesEXT
               OpDecorate %gl_MeshPerVertexEXT Block
               OpMemberDecorate %gl_MeshPerVertexEXT 0 BuiltIn Position
               OpMemberDecorate %gl_MeshPerVertexEXT 1 BuiltIn PointSize
               OpMemberDecorate %gl_MeshPerVertexEXT 2 BuiltIn ClipDistance
               OpMemberDecorate %gl_MeshPerVertexEXT 3 BuiltIn CullDistance
               OpDecorate %VertexOutput Block
               OpDecorate %vertexOutput Location 0
               OpDecorate %gl_MeshPerPrimitiveEXT Block
               OpMemberDecorate %gl_MeshPerPrimitiveEXT 0 BuiltIn PrimitiveId
               OpMemberDecorate %gl_MeshPerPrimitiveEXT 0 PerPrimitiveEXT
               OpMemberDecorate %gl_MeshPerPrimitiveEXT 1 BuiltIn Layer
               OpMemberDecorate %gl_MeshPerPrimitiveEXT 1 PerPrimitiveEXT
               OpMemberDecorate %gl_MeshPerPrimitiveEXT 2 BuiltIn ViewportIndex
               OpMemberDecorate %gl_MeshPerPrimitiveEXT 2 PerPrimitiveEXT
               OpMemberDecorate %gl_MeshPerPrimitiveEXT 3 BuiltIn CullPrimitiveEXT
               OpMemberDecorate %gl_MeshPerPrimitiveEXT 3 PerPrimitiveEXT
               OpDecorate %gl_LocalInvocationIndex BuiltIn LocalInvocationIndex
               OpDecorate %gl_PrimitiveTriangleIndicesEXT BuiltIn PrimitiveTriangleIndicesEXT
       %void = OpTypeVoid
          %4 = OpTypeFunction %void
       %uint = OpTypeInt 32 0
     %uint_1 = OpConstant %uint 1
     %uint_3 = OpConstant %uint 3
      %float = OpTypeFloat 32
    %v4float = OpTypeVector %float 4
%_arr_float_uint_1 = OpTypeArray %float %uint_1
%gl_MeshPerVertexEXT = OpTypeStruct %v4float %float %_arr_float_uint_1 %_arr_float_uint_1
%_arr_gl_MeshPerVertexEXT_uint_3 = OpTypeArray %gl_MeshPerVertexEXT %uint_3
%_ptr_Output__arr_gl_MeshPerVertexEXT_uint_3 = OpTypePointer Output %_arr_gl_MeshPerVertexEXT_uint_3
%gl_MeshVerticesEXT = OpVariable %_ptr_Output__arr_gl_MeshPerVertexEXT_uint_3 Output
        %int = OpTypeInt 32 1
      %int_0 = OpConstant %int 0
    %float_0 = OpConstant %float 0
         %20 = OpConstantComposite %v4float %float_0 %float_0 %float_0 %float_0
%_ptr_Output_v4float = OpTypePointer Output %v4float
      %int_1 = OpConstant %int 1
      %int_2 = OpConstant %int 2
%VertexOutput = OpTypeStruct %v4float
%_arr_VertexOutput_uint_3 = OpTypeArray %VertexOutput %uint_3
%_ptr_Output__arr_VertexOutput_uint_3 = OpTypePointer Output %_arr_VertexOutput_uint_3
%vertexOutput = OpVariable %_ptr_Output__arr_VertexOutput_uint_3 Output
       %bool = OpTypeBool
%gl_MeshPerPrimitiveEXT = OpTypeStruct %int %int %int %bool
%_arr_gl_MeshPerPrimitiveEXT_uint_1 = OpTypeArray %gl_MeshPerPrimitiveEXT %uint_1
%_ptr_Output__arr_gl_MeshPerPrimitiveEXT_uint_1 = OpTypePointer Output %_arr_gl_MeshPerPrimitiveEXT_uint_1
%gl_MeshPrimitivesEXT = OpVariable %_ptr_Output__arr_gl_MeshPerPrimitiveEXT_uint_1 Output
%_ptr_Input_uint = OpTypePointer Input %uint
%gl_LocalInvocationIndex = OpVariable %_ptr_Input_uint Input
      %int_3 = OpConstant %int 3
      %false = OpConstantFalse %bool
%_ptr_Output_bool = OpTypePointer Output %bool
     %v3uint = OpTypeVector %uint 3
%_arr_v3uint_uint_1 = OpTypeArray %v3uint %uint_1
%_ptr_Output__arr_v3uint_uint_1 = OpTypePointer Output %_arr_v3uint_uint_1
%gl_PrimitiveTriangleIndicesEXT = OpVariable %_ptr_Output__arr_v3uint_uint_1 Output
     %uint_0 = OpConstant %uint 0
         %52 = OpConstantComposite %v3uint %uint_0 %uint_0 %uint_0
%_ptr_Output_v3uint = OpTypePointer Output %v3uint
       %main = OpFunction %void None %4
          %6 = OpLabel
               OpSetMeshOutputsEXT %uint_3 %uint_1
         %22 = OpAccessChain %_ptr_Output_v4float %gl_MeshVerticesEXT %int_0 %int_0
               OpStore %22 %20
         %24 = OpAccessChain %_ptr_Output_v4float %gl_MeshVerticesEXT %int_1 %int_0
               OpStore %24 %20
         %26 = OpAccessChain %_ptr_Output_v4float %gl_MeshVerticesEXT %int_2 %int_0
               OpStore %26 %20
         %31 = OpAccessChain %_ptr_Output_v4float %vertexOutput %int_0 %int_0
               OpStore %31 %20
         %32 = OpAccessChain %_ptr_Output_v4float %vertexOutput %int_1 %int_0
               OpStore %32 %20
         %33 = OpAccessChain %_ptr_Output_v4float %vertexOutput %int_2 %int_0
               OpStore %33 %20
         %41 = OpLoad %uint %gl_LocalInvocationIndex
         %45 = OpAccessChain %_ptr_Output_bool %gl_MeshPrimitivesEXT %41 %int_3
               OpStore %45 %false
         %50 = OpLoad %uint %gl_LocalInvocationIndex
         %54 = OpAccessChain %_ptr_Output_v3uint %gl_PrimitiveTriangleIndicesEXT %50
               OpStore %54 %52
               OpReturn
               OpFunctionEnd
)";

  CompileSuccessfully(body, SPV_ENV_VULKAN_1_3);
  EXPECT_EQ(SPV_SUCCESS, ValidateInstructions(SPV_ENV_VULKAN_1_3));
}

TEST_F(ValidateMeshShading, VulkanBasicMeshAndTaskSuccess) {
  const std::string body = R"(
               OpCapability MeshShadingEXT
               OpExtension "SPV_EXT_mesh_shader"
               OpExtension "SPV_NV_mesh_shader"
               OpMemoryModel Logical GLSL450
               OpEntryPoint MeshEXT %mainMesh "mainMesh"
               OpEntryPoint TaskEXT %mainTask "mainTask"
               OpExecutionMode %mainMesh LocalSize 1 1 1
               OpExecutionMode %mainTask LocalSize 1 1 1
               OpExecutionMode %mainMesh OutputVertices 1
               OpExecutionMode %mainMesh OutputPrimitivesEXT 1
               OpExecutionMode %mainMesh OutputTrianglesEXT
       %void = OpTypeVoid
       %func = OpTypeFunction %void
   %mainMesh = OpFunction %void None %func
  %labelMesh = OpLabel
               OpReturn
               OpFunctionEnd
   %mainTask = OpFunction %void None %func
  %labelTask = OpLabel
               OpReturn
               OpFunctionEnd
)";

  CompileSuccessfully(body, SPV_ENV_VULKAN_1_2);
  EXPECT_EQ(SPV_SUCCESS, ValidateInstructions(SPV_ENV_VULKAN_1_2));
}

TEST_F(ValidateMeshShading, VulkanBasicMeshAndTaskBad) {
  const std::string body = R"(
               OpCapability MeshShadingEXT
               OpCapability MeshShadingNV
               OpExtension "SPV_EXT_mesh_shader"
               OpExtension "SPV_NV_mesh_shader"
               OpMemoryModel Logical GLSL450
               OpEntryPoint MeshEXT %mainMesh "mainMesh"
               OpEntryPoint TaskNV %mainTask "mainTask"
               OpExecutionMode %mainMesh OutputVertices 1
               OpExecutionMode %mainMesh OutputPrimitivesEXT 1
               OpExecutionMode %mainMesh OutputTrianglesEXT
       %void = OpTypeVoid
       %func = OpTypeFunction %void
   %mainMesh = OpFunction %void None %func
  %labelMesh = OpLabel
               OpReturn
               OpFunctionEnd
   %mainTask = OpFunction %void None %func
  %labelTask = OpLabel
               OpReturn
               OpFunctionEnd
)";

  CompileSuccessfully(body, SPV_ENV_VULKAN_1_2);
  EXPECT_EQ(SPV_ERROR_INVALID_LAYOUT, ValidateInstructions(SPV_ENV_VULKAN_1_2));
  EXPECT_THAT(getDiagnosticString(),
              AnyVUID("VUID-StandaloneSpirv-MeshEXT-07102"));
  EXPECT_THAT(getDiagnosticString(),
              HasSubstr("Module can't mix MeshEXT/TaskEXT with MeshNV/TaskNV "
                        "Execution Model."));
}

TEST_F(ValidateMeshShading, MeshMissingOutputVertices) {
  const std::string body = R"(
               OpCapability MeshShadingEXT
               OpExtension "SPV_EXT_mesh_shader"
               OpMemoryModel Logical GLSL450
               OpEntryPoint MeshEXT %main "main"
               OpExecutionMode %main OutputPrimitivesEXT 1
               OpExecutionMode %main OutputTrianglesEXT
       %void = OpTypeVoid
       %func = OpTypeFunction %void
       %main = OpFunction %void None %func
      %label = OpLabel
               OpReturn
               OpFunctionEnd
)";

  CompileSuccessfully(body, SPV_ENV_UNIVERSAL_1_5);
  EXPECT_EQ(SPV_ERROR_INVALID_DATA,
            ValidateInstructions(SPV_ENV_UNIVERSAL_1_5));
  EXPECT_THAT(
      getDiagnosticString(),
      HasSubstr("MeshEXT execution model entry points must specify both "
                "OutputPrimitivesEXT and OutputVertices Execution Modes."));
}

TEST_F(ValidateMeshShading, MeshMissingOutputPrimitivesEXT) {
  const std::string body = R"(
               OpCapability MeshShadingEXT
               OpExtension "SPV_EXT_mesh_shader"
               OpMemoryModel Logical GLSL450
               OpEntryPoint MeshEXT %main "main"
               OpExecutionMode %main OutputVertices 1
               OpExecutionMode %main OutputTrianglesEXT
       %void = OpTypeVoid
       %func = OpTypeFunction %void
       %main = OpFunction %void None %func
      %label = OpLabel
               OpReturn
               OpFunctionEnd
)";

  CompileSuccessfully(body, SPV_ENV_UNIVERSAL_1_5);
  EXPECT_EQ(SPV_ERROR_INVALID_DATA,
            ValidateInstructions(SPV_ENV_UNIVERSAL_1_5));
  EXPECT_THAT(
      getDiagnosticString(),
      HasSubstr("MeshEXT execution model entry points must specify both "
                "OutputPrimitivesEXT and OutputVertices Execution Modes."));
}

TEST_F(ValidateMeshShading, MeshMissingOutputType) {
  const std::string body = R"(
               OpCapability MeshShadingEXT
               OpExtension "SPV_EXT_mesh_shader"
               OpMemoryModel Logical GLSL450
               OpEntryPoint MeshEXT %main "main"
               OpExecutionMode %main OutputVertices 1
               OpExecutionMode %main OutputPrimitivesEXT 1
       %void = OpTypeVoid
       %func = OpTypeFunction %void
       %main = OpFunction %void None %func
      %label = OpLabel
               OpReturn
               OpFunctionEnd
)";

  CompileSuccessfully(body, SPV_ENV_UNIVERSAL_1_5);
  EXPECT_EQ(SPV_ERROR_INVALID_DATA,
            ValidateInstructions(SPV_ENV_UNIVERSAL_1_5));
  EXPECT_THAT(getDiagnosticString(),
              HasSubstr("MeshEXT execution model entry points must specify "
                        "exactly one of OutputPoints, OutputLinesEXT, or "
                        "OutputTrianglesEXT Execution Modes."));
}

TEST_F(ValidateMeshShading, MeshMultipleOutputType) {
  const std::string body = R"(
               OpCapability MeshShadingEXT
               OpExtension "SPV_EXT_mesh_shader"
               OpMemoryModel Logical GLSL450
               OpEntryPoint MeshEXT %main "main"
               OpExecutionMode %main OutputVertices 1
               OpExecutionMode %main OutputPrimitivesEXT 1
               OpExecutionMode %main OutputLinesEXT
               OpExecutionMode %main OutputTrianglesEXT
       %void = OpTypeVoid
       %func = OpTypeFunction %void
       %main = OpFunction %void None %func
      %label = OpLabel
               OpReturn
               OpFunctionEnd
)";

  CompileSuccessfully(body, SPV_ENV_UNIVERSAL_1_5);
  EXPECT_EQ(SPV_ERROR_INVALID_DATA,
            ValidateInstructions(SPV_ENV_UNIVERSAL_1_5));
  EXPECT_THAT(getDiagnosticString(),
              HasSubstr("MeshEXT execution model entry points must specify "
                        "exactly one of OutputPoints, OutputLinesEXT, or "
                        "OutputTrianglesEXT Execution Modes."));
}

TEST_F(ValidateMeshShading, BadExecutionModelOutputLinesEXT) {
  const std::string body = R"(
               OpCapability MeshShadingEXT
               OpExtension "SPV_EXT_mesh_shader"
               OpMemoryModel Logical GLSL450
               OpEntryPoint GLCompute %main "main"
               OpExecutionMode %main OutputLinesEXT
       %void = OpTypeVoid
       %func = OpTypeFunction %void
       %main = OpFunction %void None %func
      %label = OpLabel
               OpReturn
               OpFunctionEnd
)";

  CompileSuccessfully(body, SPV_ENV_UNIVERSAL_1_5);
  EXPECT_EQ(SPV_ERROR_INVALID_DATA,
            ValidateInstructions(SPV_ENV_UNIVERSAL_1_5));
  EXPECT_THAT(getDiagnosticString(),
              HasSubstr("Execution mode can only be used with the MeshEXT or "
                        "MeshNV execution model."));
}

TEST_F(ValidateMeshShading, BadExecutionModelOutputTrianglesEXT) {
  const std::string body = R"(
               OpCapability MeshShadingEXT
               OpExtension "SPV_EXT_mesh_shader"
               OpMemoryModel Logical GLSL450
               OpEntryPoint GLCompute %main "main"
               OpExecutionMode %main OutputTrianglesEXT
       %void = OpTypeVoid
       %func = OpTypeFunction %void
       %main = OpFunction %void None %func
      %label = OpLabel
               OpReturn
               OpFunctionEnd
)";

  CompileSuccessfully(body, SPV_ENV_UNIVERSAL_1_5);
  EXPECT_EQ(SPV_ERROR_INVALID_DATA,
            ValidateInstructions(SPV_ENV_UNIVERSAL_1_5));
  EXPECT_THAT(getDiagnosticString(),
              HasSubstr("Execution mode can only be used with the MeshEXT or "
                        "MeshNV execution model."));
}

TEST_F(ValidateMeshShading, BadExecutionModelOutputPrimitivesEXT) {
  const std::string body = R"(
               OpCapability MeshShadingEXT
               OpExtension "SPV_EXT_mesh_shader"
               OpMemoryModel Logical GLSL450
               OpEntryPoint GLCompute %main "main"
               OpExecutionMode %main OutputPrimitivesEXT 1
       %void = OpTypeVoid
       %func = OpTypeFunction %void
       %main = OpFunction %void None %func
      %label = OpLabel
               OpReturn
               OpFunctionEnd
)";

  CompileSuccessfully(body, SPV_ENV_UNIVERSAL_1_5);
  EXPECT_EQ(SPV_ERROR_INVALID_DATA,
            ValidateInstructions(SPV_ENV_UNIVERSAL_1_5));
  EXPECT_THAT(getDiagnosticString(),
              HasSubstr("Execution mode can only be used with the MeshEXT or "
                        "MeshNV execution model."));
}

TEST_F(ValidateMeshShading, OpEmitMeshTasksBadGroupCountSignedInt) {
  const std::string body = R"(
               OpCapability MeshShadingEXT
               OpExtension "SPV_EXT_mesh_shader"
               OpMemoryModel Logical GLSL450
               OpEntryPoint TaskEXT %main "main"
       %void = OpTypeVoid
       %func = OpTypeFunction %void
        %int = OpTypeInt 32 1
       %uint = OpTypeInt 32 0
      %int_1 = OpConstant %int 1
     %uint_1 = OpConstant %uint 1
       %main = OpFunction %void None %func
       %label = OpLabel
               OpEmitMeshTasksEXT %int_1 %uint_1 %uint_1
               OpFunctionEnd
)";

  CompileSuccessfully(body, SPV_ENV_UNIVERSAL_1_5);
  EXPECT_EQ(SPV_ERROR_INVALID_DATA,
            ValidateInstructions(SPV_ENV_UNIVERSAL_1_5));
  EXPECT_THAT(getDiagnosticString(),
              HasSubstr("Group Count X must be a 32-bit unsigned int scalar"));
}

TEST_F(ValidateMeshShading, OpEmitMeshTasksBadGroupCountVector) {
  const std::string body = R"(
               OpCapability MeshShadingEXT
               OpExtension "SPV_EXT_mesh_shader"
               OpMemoryModel Logical GLSL450
               OpEntryPoint TaskEXT %main "main"
       %void = OpTypeVoid
       %func = OpTypeFunction %void
       %uint = OpTypeInt 32 0
     %v2uint = OpTypeVector %uint 2
%_ptr_v2uint = OpTypePointer Function %v2uint
     %uint_1 = OpConstant %uint 1
  %composite = OpConstantComposite %v2uint %uint_1 %uint_1
  %_ptr_uint = OpTypePointer Function %uint
       %main = OpFunction %void None %func
      %label = OpLabel
          %x = OpVariable %_ptr_v2uint Function
               OpStore %x %composite
         %13 = OpAccessChain %_ptr_uint %x %uint_1
         %14 = OpLoad %uint %13
               OpEmitMeshTasksEXT %14 %composite %uint_1
               OpFunctionEnd
)";

  CompileSuccessfully(body, SPV_ENV_UNIVERSAL_1_5);
  EXPECT_EQ(SPV_ERROR_INVALID_DATA,
            ValidateInstructions(SPV_ENV_UNIVERSAL_1_5));
  EXPECT_THAT(getDiagnosticString(),
              HasSubstr("Group Count Y must be a 32-bit unsigned int scalar"));
}

TEST_F(ValidateMeshShading, OpEmitMeshTasksBadPayload) {
  const std::string body = R"(
               OpCapability MeshShadingEXT
               OpExtension "SPV_EXT_mesh_shader"
               OpMemoryModel Logical GLSL450
               OpEntryPoint TaskEXT %main "main" %payload
       %void = OpTypeVoid
       %func = OpTypeFunction %void
       %uint = OpTypeInt 32 0
       %task = OpTypeStruct %uint
%_ptr_Uniform = OpTypePointer Uniform %task
    %payload = OpVariable %_ptr_Uniform Uniform
     %uint_1 = OpConstant %uint 1
       %main = OpFunction %void None %func
      %label = OpLabel
               OpEmitMeshTasksEXT %uint_1 %uint_1 %uint_1 %payload
               OpFunctionEnd
)";

  CompileSuccessfully(body, SPV_ENV_UNIVERSAL_1_5);
  EXPECT_EQ(SPV_ERROR_INVALID_DATA,
            ValidateInstructions(SPV_ENV_UNIVERSAL_1_5));
  EXPECT_THAT(getDiagnosticString(),
              HasSubstr("Payload OpVariable must have a storage class of "
                        "TaskPayloadWorkgroupEXT"));
}

TEST_F(ValidateMeshShading, TaskPayloadWorkgroupBadExecutionModel) {
  const std::string body = R"(
               OpCapability MeshShadingEXT
               OpExtension "SPV_EXT_mesh_shader"
               OpMemoryModel Logical GLSL450
               OpEntryPoint GLCompute %main "main" %payload
       %void = OpTypeVoid
       %func = OpTypeFunction %void
       %uint = OpTypeInt 32 0
%_ptr_TaskPayloadWorkgroupEXT = OpTypePointer TaskPayloadWorkgroupEXT %uint
    %payload = OpVariable %_ptr_TaskPayloadWorkgroupEXT TaskPayloadWorkgroupEXT
       %main = OpFunction %void None %func
      %label = OpLabel
       %load = OpLoad %uint %payload
               OpReturn
               OpFunctionEnd
)";

  CompileSuccessfully(body, SPV_ENV_UNIVERSAL_1_5);
  EXPECT_EQ(SPV_ERROR_INVALID_ID, ValidateInstructions(SPV_ENV_UNIVERSAL_1_5));
  EXPECT_THAT(getDiagnosticString(),
              HasSubstr("TaskPayloadWorkgroupEXT Storage Class is limited to "
                        "TaskEXT and MeshKHR execution model"));
}

TEST_F(ValidateMeshShading, BadMultipleTaskPayloadWorkgroupEXT) {
  const std::string body = R"(
               OpCapability MeshShadingEXT
               OpExtension "SPV_EXT_mesh_shader"
               OpMemoryModel Logical GLSL450
               OpEntryPoint TaskEXT %main "main" %payload %payload1
       %void = OpTypeVoid
       %func = OpTypeFunction %void
       %uint = OpTypeInt 32 0
%_ptr_TaskPayloadWorkgroupEXT = OpTypePointer TaskPayloadWorkgroupEXT %uint
    %payload = OpVariable %_ptr_TaskPayloadWorkgroupEXT TaskPayloadWorkgroupEXT
    %payload1 = OpVariable %_ptr_TaskPayloadWorkgroupEXT TaskPayloadWorkgroupEXT
       %main = OpFunction %void None %func
      %label = OpLabel
       %load = OpLoad %uint %payload
               OpReturn
               OpFunctionEnd
)";
  CompileSuccessfully(body, SPV_ENV_UNIVERSAL_1_5);
  EXPECT_EQ(SPV_ERROR_INVALID_ID, ValidateInstructions(SPV_ENV_UNIVERSAL_1_5));
  EXPECT_THAT(getDiagnosticString(),
              HasSubstr("There can be at most one "
                        "OpVariable with storage "
                        "class TaskPayloadWorkgroupEXT associated with "
                        "an OpEntryPoint"));
}

TEST_F(ValidateMeshShading, TaskPayloadWorkgroupTaskExtExecutionModel) {
  const std::string body = R"(
               OpCapability MeshShadingEXT
               OpExtension "SPV_EXT_mesh_shader"
               OpMemoryModel Logical GLSL450
               OpEntryPoint TaskEXT %main "main" %payload
       %void = OpTypeVoid
       %func = OpTypeFunction %void
       %uint = OpTypeInt 32 0
%_ptr_TaskPayloadWorkgroupEXT = OpTypePointer TaskPayloadWorkgroupEXT %uint
    %payload = OpVariable %_ptr_TaskPayloadWorkgroupEXT TaskPayloadWorkgroupEXT
       %main = OpFunction %void None %func
      %label = OpLabel
       %load = OpLoad %uint %payload
               OpReturn
               OpFunctionEnd
)";

  CompileSuccessfully(body, SPV_ENV_UNIVERSAL_1_5);
  EXPECT_EQ(SPV_SUCCESS, ValidateInstructions(SPV_ENV_UNIVERSAL_1_5));
}

TEST_F(ValidateMeshShading, TaskPayloadWorkgroupMeshExtExecutionModel) {
  const std::string body = R"(
               OpCapability MeshShadingEXT
               OpExtension "SPV_EXT_mesh_shader"
               OpMemoryModel Logical GLSL450
               OpEntryPoint MeshEXT %main "main" %payload
               OpExecutionMode %main OutputVertices 1
               OpExecutionMode %main OutputPrimitivesEXT 1
               OpExecutionMode %main OutputTrianglesEXT
       %void = OpTypeVoid
       %func = OpTypeFunction %void
       %uint = OpTypeInt 32 0
%_ptr_TaskPayloadWorkgroupEXT = OpTypePointer TaskPayloadWorkgroupEXT %uint
    %payload = OpVariable %_ptr_TaskPayloadWorkgroupEXT TaskPayloadWorkgroupEXT
       %main = OpFunction %void None %func
      %label = OpLabel
       %load = OpLoad %uint %payload
               OpReturn
               OpFunctionEnd
)";

  CompileSuccessfully(body, SPV_ENV_UNIVERSAL_1_5);
  EXPECT_EQ(SPV_SUCCESS, ValidateInstructions(SPV_ENV_UNIVERSAL_1_5));
}

TEST_F(ValidateMeshShading, OpSetMeshOutputsBadVertexCount) {
  const std::string body = R"(
               OpCapability MeshShadingEXT
               OpExtension "SPV_EXT_mesh_shader"
               OpMemoryModel Logical GLSL450
               OpEntryPoint MeshEXT %main "main"
               OpExecutionMode %main OutputVertices 1
               OpExecutionMode %main OutputPrimitivesNV 1
               OpExecutionMode %main OutputTrianglesNV
       %void = OpTypeVoid
          %3 = OpTypeFunction %void
        %int = OpTypeInt 32 1
       %uint = OpTypeInt 32 0
  %_ptr_int = OpTypePointer Function %int
      %int_1 = OpConstant %int 1
     %uint_1 = OpConstant %uint 1
       %main = OpFunction %void None %3
          %5 = OpLabel
          %x = OpVariable %_ptr_int Function
               OpStore %x %int_1
       %load = OpLoad %int %x
               OpSetMeshOutputsEXT %load %uint_1
               OpReturn
               OpFunctionEnd
)";

  CompileSuccessfully(body, SPV_ENV_UNIVERSAL_1_5);
  EXPECT_EQ(SPV_ERROR_INVALID_DATA,
            ValidateInstructions(SPV_ENV_UNIVERSAL_1_5));
  EXPECT_THAT(getDiagnosticString(),
              HasSubstr("Vertex Count must be a 32-bit unsigned int scalar"));
}

TEST_F(ValidateMeshShading, OpSetMeshOutputsBadPrimitiveCount) {
  const std::string body = R"(
               OpCapability MeshShadingEXT
               OpExtension "SPV_EXT_mesh_shader"
               OpMemoryModel Logical GLSL450
               OpEntryPoint MeshEXT %main "main"
               OpExecutionMode %main OutputVertices 1
               OpExecutionMode %main OutputPrimitivesNV 1
               OpExecutionMode %main OutputTrianglesNV
       %void = OpTypeVoid
          %3 = OpTypeFunction %void
        %int = OpTypeInt 32 1
       %uint = OpTypeInt 32 0
      %int_1 = OpConstant %int 1
     %uint_1 = OpConstant %uint 1
       %main = OpFunction %void None %3
          %5 = OpLabel
               OpSetMeshOutputsEXT %uint_1 %int_1
               OpReturn
               OpFunctionEnd
)";

  CompileSuccessfully(body, SPV_ENV_UNIVERSAL_1_5);
  EXPECT_EQ(SPV_ERROR_INVALID_DATA,
            ValidateInstructions(SPV_ENV_UNIVERSAL_1_5));
  EXPECT_THAT(
      getDiagnosticString(),
      HasSubstr("Primitive Count must be a 32-bit unsigned int scalar"));
}

TEST_F(ValidateMeshShading, OpSetMeshOutputsBadExecutionModel) {
  const std::string body = R"(
               OpCapability MeshShadingEXT
               OpExtension "SPV_EXT_mesh_shader"
               OpMemoryModel Logical GLSL450
               OpEntryPoint TaskEXT %main "main"
       %void = OpTypeVoid
          %3 = OpTypeFunction %void
       %uint = OpTypeInt 32 0
     %uint_1 = OpConstant %uint 1
       %main = OpFunction %void None %3
          %5 = OpLabel
               OpSetMeshOutputsEXT %uint_1 %uint_1
               OpReturn
               OpFunctionEnd
)";

  CompileSuccessfully(body, SPV_ENV_UNIVERSAL_1_5);
  EXPECT_EQ(SPV_ERROR_INVALID_ID, ValidateInstructions(SPV_ENV_UNIVERSAL_1_5));
  EXPECT_THAT(
      getDiagnosticString(),
      HasSubstr("OpSetMeshOutputsEXT requires MeshEXT execution model"));
}

TEST_F(ValidateMeshShading, OpSetMeshOutputsZeroSuccess) {
  const std::string body = R"(
               OpCapability MeshShadingEXT
               OpExtension "SPV_EXT_mesh_shader"
               OpMemoryModel Logical GLSL450
               OpEntryPoint MeshEXT %main "main"
               OpExecutionMode %main OutputVertices 1
               OpExecutionMode %main OutputPrimitivesNV 1
               OpExecutionMode %main OutputTrianglesNV
       %void = OpTypeVoid
          %3 = OpTypeFunction %void
       %uint = OpTypeInt 32 0
     %uint_0 = OpConstant %uint 0
       %main = OpFunction %void None %3
          %5 = OpLabel
               OpSetMeshOutputsEXT %uint_0 %uint_0
               OpReturn
               OpFunctionEnd
)";

  CompileSuccessfully(body, SPV_ENV_UNIVERSAL_1_5);
  EXPECT_EQ(SPV_SUCCESS, ValidateInstructions(SPV_ENV_UNIVERSAL_1_5));
}

TEST_F(ValidateMeshShading, OpEmitMeshTasksZeroSuccess) {
  const std::string body = R"(
               OpCapability MeshShadingEXT
               OpExtension "SPV_EXT_mesh_shader"
               OpMemoryModel Logical GLSL450
               OpEntryPoint TaskEXT %main "main"
       %void = OpTypeVoid
       %func = OpTypeFunction %void
       %uint = OpTypeInt 32 0
     %uint_0 = OpConstant %uint 1
       %main = OpFunction %void None %func
      %label = OpLabel
               OpEmitMeshTasksEXT %uint_0 %uint_0 %uint_0
               OpFunctionEnd
)";

  CompileSuccessfully(body, SPV_ENV_UNIVERSAL_1_5);
  EXPECT_EQ(SPV_SUCCESS, ValidateInstructions(SPV_ENV_UNIVERSAL_1_5));
}

TEST_F(ValidateMeshShading, MeshOutputScalar) {
  const std::string body = R"(
          OpCapability MeshShadingEXT
          OpExtension "SPV_EXT_mesh_shader"
          OpMemoryModel Logical GLSL450
          OpEntryPoint MeshEXT %main "main" %x
          OpExecutionModeId %main LocalSizeId %uint_1 %uint_1 %uint_1
          OpExecutionMode %main OutputVertices 3
          OpExecutionMode %main OutputPrimitivesEXT 1
          OpExecutionMode %main OutputTrianglesEXT
          OpDecorate %x Location 0
  %void = OpTypeVoid
     %4 = OpTypeFunction %void
  %uint = OpTypeInt 32 0
%uint_1 = OpConstant %uint 1
%uint_3 = OpConstant %uint 3
%o_ptr = OpTypePointer Output %uint
     %x = OpVariable %o_ptr Output
  %main = OpFunction %void None %4
     %6 = OpLabel
          OpSetMeshOutputsEXT %uint_3 %uint_1
          OpReturn
          OpFunctionEnd
)";

  CompileSuccessfully(body, SPV_ENV_VULKAN_1_3);
  EXPECT_EQ(SPV_ERROR_INVALID_DATA, ValidateInstructions(SPV_ENV_VULKAN_1_3));
  EXPECT_THAT(getDiagnosticString(),
              HasSubstr("In the MeshEXT Execution Mode, all Output Variables "
                        "must contain an Array."));
}

TEST_F(ValidateMeshShading, MeshOutputScalarStruct) {
  const std::string body = R"(
               OpCapability MeshShadingEXT
               OpExtension "SPV_EXT_mesh_shader"
               OpMemoryModel Logical GLSL450
               OpEntryPoint MeshEXT %main "main" %vertexOutput
               OpExecutionModeId %main LocalSizeId %uint_1 %uint_1 %uint_1
               OpExecutionMode %main OutputVertices 3
               OpExecutionMode %main OutputPrimitivesEXT 1
               OpExecutionMode %main OutputTrianglesEXT
               OpDecorate %VertexOutput Block
               OpMemberDecorate %VertexOutput 0 Location 0
       %void = OpTypeVoid
          %4 = OpTypeFunction %void
       %uint = OpTypeInt 32 0
     %uint_1 = OpConstant %uint 1
     %uint_3 = OpConstant %uint 3
      %float = OpTypeFloat 32
    %v4float = OpTypeVector %float 4
%VertexOutput = OpTypeStruct %v4float
%_ptr_Output_uint_3 = OpTypePointer Output %VertexOutput
%vertexOutput = OpVariable %_ptr_Output_uint_3 Output
        %int = OpTypeInt 32 1
      %int_0 = OpConstant %int 0
    %float_0 = OpConstant %float 0
         %19 = OpConstantComposite %v4float %float_0 %float_0 %float_0 %float_0
%_ptr_Output_v4float = OpTypePointer Output %v4float
       %main = OpFunction %void None %4
          %6 = OpLabel
               OpSetMeshOutputsEXT %uint_3 %uint_1
         %21 = OpAccessChain %_ptr_Output_v4float %vertexOutput %int_0
               OpStore %21 %19
               OpReturn
               OpFunctionEnd
)";

  CompileSuccessfully(body, SPV_ENV_VULKAN_1_3);
  EXPECT_EQ(SPV_ERROR_INVALID_DATA, ValidateInstructions(SPV_ENV_VULKAN_1_3));
  EXPECT_THAT(getDiagnosticString(),
              HasSubstr("In the MeshEXT Execution Mode, all Output Variables "
                        "must contain an Array."));
}

TEST_F(ValidateMeshShading, BadPerPrimitiveEXTStorageClassInMeshEXT) {
  const std::string body = R"(
              OpCapability MeshShadingEXT
              OpExtension "SPV_EXT_mesh_shader"
         %1 = OpExtInstImport "GLSL.std.450"
              OpMemoryModel Logical GLSL450
              OpEntryPoint MeshEXT %main "main" %gl_LocalInvocationID %blk %triangleNormal
              OpExecutionMode %main LocalSize 32 1 1
              OpExecutionMode %main OutputVertices 81
              OpExecutionMode %main OutputPrimitivesNV 32
              OpExecutionMode %main OutputTrianglesNV
              OpSource GLSL 450
              OpSourceExtension "GL_EXT_mesh_shader"
              OpName %main "main"
              OpName %iid "iid"
              OpName %gl_LocalInvocationID "gl_LocalInvocationID"
              OpName %myblock "myblock"
              OpMemberName %myblock 0 "f"
              OpName %blk "blk"
              OpName %triangleNormal "triangleNormal"
              OpDecorate %gl_LocalInvocationID BuiltIn LocalInvocationId
              OpMemberDecorate %myblock 0 PerPrimitiveEXT
              OpDecorate %myblock Block
              OpDecorate %blk Location 0
              OpDecorate %triangleNormal PerPrimitiveEXT
              OpDecorate %triangleNormal Location 0
              OpDecorate %gl_WorkGroupSize BuiltIn WorkgroupSize
      %void = OpTypeVoid
         %3 = OpTypeFunction %void
      %uint = OpTypeInt 32 0
%_ptr_Function_uint = OpTypePointer Function %uint
    %v3uint = OpTypeVector %uint 3
%_ptr_Input_v3uint = OpTypePointer Input %v3uint
%gl_LocalInvocationID = OpVariable %_ptr_Input_v3uint Input
    %uint_0 = OpConstant %uint 0
%_ptr_Input_uint = OpTypePointer Input %uint
     %float = OpTypeFloat 32
   %myblock = OpTypeStruct %float
   %uint_32 = OpConstant %uint 32
%_arr_myblock_uint_32 = OpTypeArray %myblock %uint_32
%_ptr_Output__arr_myblock_uint_32 = OpTypePointer Output %_arr_myblock_uint_32
       %blk = OpVariable %_ptr_Output__arr_myblock_uint_32 Output
       %int = OpTypeInt 32 1
     %int_0 = OpConstant %int 0
  %float_11 = OpConstant %float 11
%_ptr_Output_float = OpTypePointer Output %float
   %v3float = OpTypeVector %float 3
%_arr_v3float_uint_32 = OpTypeArray %v3float %uint_32
%_ptr_Output__arr_v3float_uint_32 = OpTypePointer Input %_arr_v3float_uint_32
%triangleNormal = OpVariable %_ptr_Output__arr_v3float_uint_32 Input
        %33 = OpConstantComposite %v3float %float_11 %float_11 %float_11
%_ptr_Output_v3float = OpTypePointer Output %v3float
    %uint_1 = OpConstant %uint 1
%gl_WorkGroupSize = OpConstantComposite %v3uint %uint_32 %uint_1 %uint_1
      %main = OpFunction %void None %3
         %5 = OpLabel
       %iid = OpVariable %_ptr_Function_uint Function
        %14 = OpAccessChain %_ptr_Input_uint %gl_LocalInvocationID %uint_0
        %15 = OpLoad %uint %14
              OpStore %iid %15
        %22 = OpLoad %uint %iid
        %27 = OpAccessChain %_ptr_Output_float %blk %22 %int_0
              OpStore %27 %float_11
              OpReturn
              OpFunctionEnd
)";

  CompileSuccessfully(body, SPV_ENV_UNIVERSAL_1_5);
  EXPECT_EQ(SPV_ERROR_INVALID_DATA,
            ValidateInstructions(SPV_ENV_UNIVERSAL_1_5));
  EXPECT_THAT(getDiagnosticString(),
              HasSubstr("PerPrimitiveEXT decoration must be applied only to "
                        "variables in the Output Storage Class in the Storage "
                        "Class in the MeshEXT Execution Model."));
}

TEST_F(ValidateMeshShading, VulkanPerPrimitiveEXTStorageClassInMeshEXT) {
  const std::string body = R"(
      OpCapability MeshShadingEXT
      OpExtension "SPV_EXT_mesh_shader"
 %1 = OpExtInstImport "GLSL.std.450"
      OpMemoryModel Logical GLSL450
      OpEntryPoint MeshEXT %main "main" %gl_LocalInvocationID %blk %triangleNormal
      OpExecutionMode %main LocalSize 32 1 1
      OpExecutionMode %main OutputVertices 81
      OpExecutionMode %main OutputPrimitivesNV 32
      OpExecutionMode %main OutputTrianglesNV
      OpSource GLSL 450
      OpSourceExtension "GL_EXT_mesh_shader"
      OpName %main "main"
      OpName %iid "iid"
      OpName %gl_LocalInvocationID "gl_LocalInvocationID"
      OpName %myblock "myblock"
      OpMemberName %myblock 0 "f"
      OpName %blk "blk"
      OpName %triangleNormal "triangleNormal"
      OpDecorate %gl_LocalInvocationID BuiltIn LocalInvocationId
      OpMemberDecorate %myblock 0 PerPrimitiveEXT
      OpDecorate %myblock Block
      OpDecorate %blk Location 0
      OpDecorate %triangleNormal PerPrimitiveEXT
      OpDecorate %triangleNormal Location 0
      OpDecorate %gl_WorkGroupSize BuiltIn WorkgroupSize
%void = OpTypeVoid
 %3 = OpTypeFunction %void
%uint = OpTypeInt 32 0
%_ptr_Function_uint = OpTypePointer Function %uint
%v3uint = OpTypeVector %uint 3
%_ptr_Input_v3uint = OpTypePointer Input %v3uint
%gl_LocalInvocationID = OpVariable %_ptr_Input_v3uint Input
%uint_0 = OpConstant %uint 0
%_ptr_Input_uint = OpTypePointer Input %uint
%float = OpTypeFloat 32
%myblock = OpTypeStruct %float
%uint_32 = OpConstant %uint 32
%_arr_myblock_uint_32 = OpTypeArray %myblock %uint_32
%_ptr_Output__arr_myblock_uint_32 = OpTypePointer Output %_arr_myblock_uint_32
%blk = OpVariable %_ptr_Output__arr_myblock_uint_32 Output
%int = OpTypeInt 32 1
%int_0 = OpConstant %int 0
%float_11 = OpConstant %float 11
%_ptr_Output_float = OpTypePointer Output %float
%v3float = OpTypeVector %float 3
%_arr_v3float_uint_32 = OpTypeArray %v3float %uint_32
%_ptr_Output__arr_v3float_uint_32 = OpTypePointer Input %_arr_v3float_uint_32
%triangleNormal = OpVariable %_ptr_Output__arr_v3float_uint_32 Input
%33 = OpConstantComposite %v3float %float_11 %float_11 %float_11
%_ptr_Output_v3float = OpTypePointer Output %v3float
%uint_1 = OpConstant %uint 1
%gl_WorkGroupSize = OpConstantComposite %v3uint %uint_32 %uint_1 %uint_1
%main = OpFunction %void None %3
 %5 = OpLabel
%iid = OpVariable %_ptr_Function_uint Function
%14 = OpAccessChain %_ptr_Input_uint %gl_LocalInvocationID %uint_0
%15 = OpLoad %uint %14
      OpStore %iid %15
%22 = OpLoad %uint %iid
%27 = OpAccessChain %_ptr_Output_float %blk %22 %int_0
      OpStore %27 %float_11
      OpReturn
      OpFunctionEnd
)";

  CompileSuccessfully(body, SPV_ENV_VULKAN_1_2);
  EXPECT_EQ(SPV_ERROR_INVALID_DATA, ValidateInstructions(SPV_ENV_VULKAN_1_2));
  EXPECT_THAT(getDiagnosticString(),
              AnyVUID("VUID-PrimitiveId-PrimitiveId-04336"));
}

TEST_F(ValidateMeshShading, BadPerPrimitiveEXTStorageClassInFrag) {
  const std::string body = R"(
             OpCapability Shader
             OpCapability MeshShadingEXT
             OpExtension "SPV_EXT_mesh_shader"
        %1 = OpExtInstImport "GLSL.std.450"
             OpMemoryModel Logical GLSL450
             OpEntryPoint Fragment %main "main" %triangleNormal
             OpExecutionMode %main OriginUpperLeft
             OpSource GLSL 450
             OpSourceExtension "GL_EXT_mesh_shader"
             OpName %main "main"
             OpName %triangleNormal "triangleNormal"
             OpDecorate %triangleNormal PerPrimitiveNV
             OpDecorate %triangleNormal Location 0
     %void = OpTypeVoid
        %3 = OpTypeFunction %void
    %float = OpTypeFloat 32
  %v3float = OpTypeVector %float 3
     %uint = OpTypeInt 32 0
   %uint_3 = OpConstant %uint 3
%_arr_v3float_uint_3 = OpTypeArray %v3float %uint_3
%_ptr_Input__arr_v3float_uint_3 = OpTypePointer Output %_arr_v3float_uint_3
%triangleNormal = OpVariable %_ptr_Input__arr_v3float_uint_3 Output
      %int = OpTypeInt 32 1
    %int_0 = OpConstant %int 0
%_ptr_Input_v3float = OpTypePointer Input %v3float
     %main = OpFunction %void None %3
        %5 = OpLabel
       %18 = OpAccessChain %_ptr_Input_v3float %triangleNormal %int_0
       %19 = OpLoad %v3float %18
             OpReturn
             OpFunctionEnd
)";

  CompileSuccessfully(body, SPV_ENV_UNIVERSAL_1_5);
  EXPECT_EQ(SPV_ERROR_INVALID_DATA,
            ValidateInstructions(SPV_ENV_UNIVERSAL_1_5));
  EXPECT_THAT(getDiagnosticString(),
              HasSubstr("PerPrimitiveEXT decoration must be applied only to "
                        "variables in the Input Storage Class in the Fragment "
                        "Execution Model."));
}

TEST_F(ValidateMeshShading, PerPrimitiveEXTStorageClassInFrag) {
  const std::string body = R"(
                OpCapability Shader
                OpCapability MeshShadingEXT
                OpExtension "SPV_EXT_mesh_shader"
           %1 = OpExtInstImport "GLSL.std.450"
                OpMemoryModel Logical GLSL450
                OpEntryPoint Fragment %main "main" %res3 %triangleNormal
                OpExecutionMode %main OriginUpperLeft
                OpSource GLSL 450
                OpSourceExtension "GL_EXT_mesh_shader"
                OpName %main "main"
                OpName %res3 "res3"
                OpName %triangleNormal "triangleNormal"
                OpDecorate %res3 Location 0
                OpDecorate %triangleNormal PerPrimitiveNV
                OpDecorate %triangleNormal Location 0
        %void = OpTypeVoid
           %3 = OpTypeFunction %void
       %float = OpTypeFloat 32
     %v3float = OpTypeVector %float 3
 %_ptr_Output_v3float = OpTypePointer Output %v3float
        %res3 = OpVariable %_ptr_Output_v3float Output
        %uint = OpTypeInt 32 0
      %uint_3 = OpConstant %uint 3
 %_arr_v3float_uint_3 = OpTypeArray %v3float %uint_3
 %_ptr_Input__arr_v3float_uint_3 = OpTypePointer Input %_arr_v3float_uint_3
 %triangleNormal = OpVariable %_ptr_Input__arr_v3float_uint_3 Input
         %int = OpTypeInt 32 1
       %int_0 = OpConstant %int 0
 %_ptr_Input_v3float = OpTypePointer Input %v3float
        %main = OpFunction %void None %3
           %5 = OpLabel
          %18 = OpAccessChain %_ptr_Input_v3float %triangleNormal %int_0
          %19 = OpLoad %v3float %18
                OpStore %res3 %19
                OpReturn
                OpFunctionEnd
)";

  CompileSuccessfully(body, SPV_ENV_UNIVERSAL_1_5);
  EXPECT_EQ(SPV_SUCCESS, ValidateInstructions(SPV_ENV_UNIVERSAL_1_5));
}

TEST_F(ValidateMeshShading, PerPrimitiveEXTStorageClassInMeshEXT) {
  const std::string body = R"(
               OpCapability MeshShadingEXT
               OpExtension "SPV_EXT_mesh_shader"
          %1 = OpExtInstImport "GLSL.std.450"
               OpMemoryModel Logical GLSL450
               OpEntryPoint MeshEXT %main "main" %gl_LocalInvocationID %blk %triangleNormal
               OpExecutionMode %main LocalSize 32 1 1
               OpExecutionMode %main OutputVertices 81
               OpExecutionMode %main OutputPrimitivesNV 32
               OpExecutionMode %main OutputTrianglesNV
               OpSource GLSL 450
               OpSourceExtension "GL_EXT_mesh_shader"
               OpName %main "main"
               OpName %iid "iid"
               OpName %gl_LocalInvocationID "gl_LocalInvocationID"
               OpName %myblock "myblock"
               OpMemberName %myblock 0 "f"
               OpName %blk "blk"
               OpName %triangleNormal "triangleNormal"
               OpDecorate %gl_LocalInvocationID BuiltIn LocalInvocationId
               OpMemberDecorate %myblock 0 PerPrimitiveNV
               OpDecorate %myblock Block
               OpDecorate %blk Location 0
               OpDecorate %triangleNormal PerPrimitiveNV
               OpDecorate %triangleNormal Location 0
               OpDecorate %gl_WorkGroupSize BuiltIn WorkgroupSize
       %void = OpTypeVoid
          %3 = OpTypeFunction %void
       %uint = OpTypeInt 32 0
%_ptr_Function_uint = OpTypePointer Function %uint
     %v3uint = OpTypeVector %uint 3
%_ptr_Input_v3uint = OpTypePointer Input %v3uint
%gl_LocalInvocationID = OpVariable %_ptr_Input_v3uint Input
     %uint_0 = OpConstant %uint 0
%_ptr_Input_uint = OpTypePointer Input %uint
      %float = OpTypeFloat 32
    %myblock = OpTypeStruct %float
    %uint_32 = OpConstant %uint 32
%_arr_myblock_uint_32 = OpTypeArray %myblock %uint_32
%_ptr_Output__arr_myblock_uint_32 = OpTypePointer Output %_arr_myblock_uint_32
        %blk = OpVariable %_ptr_Output__arr_myblock_uint_32 Output
        %int = OpTypeInt 32 1
      %int_0 = OpConstant %int 0
   %float_11 = OpConstant %float 11
%_ptr_Output_float = OpTypePointer Output %float
    %v3float = OpTypeVector %float 3
%_arr_v3float_uint_32 = OpTypeArray %v3float %uint_32
%_ptr_Output__arr_v3float_uint_32 = OpTypePointer Output %_arr_v3float_uint_32
%triangleNormal = OpVariable %_ptr_Output__arr_v3float_uint_32 Output
         %33 = OpConstantComposite %v3float %float_11 %float_11 %float_11
%_ptr_Output_v3float = OpTypePointer Output %v3float
     %uint_1 = OpConstant %uint 1
%gl_WorkGroupSize = OpConstantComposite %v3uint %uint_32 %uint_1 %uint_1
       %main = OpFunction %void None %3
          %5 = OpLabel
        %iid = OpVariable %_ptr_Function_uint Function
         %14 = OpAccessChain %_ptr_Input_uint %gl_LocalInvocationID %uint_0
         %15 = OpLoad %uint %14
               OpStore %iid %15
         %22 = OpLoad %uint %iid
         %27 = OpAccessChain %_ptr_Output_float %blk %22 %int_0
               OpStore %27 %float_11
         %32 = OpLoad %uint %iid
         %35 = OpAccessChain %_ptr_Output_v3float %triangleNormal %32
               OpStore %35 %33
               OpReturn
               OpFunctionEnd
)";

  CompileSuccessfully(body, SPV_ENV_UNIVERSAL_1_5);
  EXPECT_EQ(SPV_SUCCESS, ValidateInstructions(SPV_ENV_UNIVERSAL_1_5));
}

TEST_F(ValidateMeshShading, MeshWriteOutput) {
  const std::string body = R"(
               OpCapability MeshShadingEXT
               OpExtension "SPV_EXT_mesh_shader"
               OpMemoryModel Logical GLSL450
               OpEntryPoint MeshEXT %main "main" %x %y
               OpExecutionModeId %main LocalSizeId %uint_1 %uint_1 %uint_1
               OpExecutionMode %main OutputVertices 3
               OpExecutionMode %main OutputPrimitivesEXT 1
               OpExecutionMode %main OutputTrianglesEXT
               OpDecorate %x Location 1
               OpDecorate %y Location 0
       %void = OpTypeVoid
          %4 = OpTypeFunction %void
       %uint = OpTypeInt 32 0
     %uint_1 = OpConstant %uint 1
     %uint_3 = OpConstant %uint 3
%_arr_uint_uint_3 = OpTypeArray %uint %uint_3
%_ptr_Output__arr_uint_uint_3 = OpTypePointer Output %_arr_uint_uint_3
          %x = OpVariable %_ptr_Output__arr_uint_uint_3 Output
        %int = OpTypeInt 32 1
      %int_0 = OpConstant %int 0
%_ptr_Output_uint = OpTypePointer Output %uint
      %float = OpTypeFloat 32
    %v4float = OpTypeVector %float 4
%_arr_v4float_uint_3 = OpTypeArray %v4float %uint_3
%_ptr_Output__arr_v4float_uint_3 = OpTypePointer Output %_arr_v4float_uint_3
          %y = OpVariable %_ptr_Output__arr_v4float_uint_3 Output
    %float_0 = OpConstant %float 0
         %23 = OpConstantComposite %v4float %float_0 %float_0 %float_0 %float_0
%_ptr_Output_v4float = OpTypePointer Output %v4float
       %main = OpFunction %void None %4
          %6 = OpLabel
         %16 = OpAccessChain %_ptr_Output_uint %x %int_0
               OpStore %16 %uint_1
         %25 = OpAccessChain %_ptr_Output_v4float %y %int_0
               OpStore %25 %23
               OpSetMeshOutputsEXT %uint_3 %uint_1
               OpReturn
               OpFunctionEnd
)";

  CompileSuccessfully(body, SPV_ENV_VULKAN_1_3);
  EXPECT_EQ(SPV_SUCCESS, ValidateInstructions(SPV_ENV_VULKAN_1_3));
}

TEST_F(ValidateMeshShading, MeshReadOutputInt) {
  const std::string body = R"(
               OpCapability MeshShadingEXT
               OpExtension "SPV_EXT_mesh_shader"
               OpMemoryModel Logical GLSL450
               OpEntryPoint MeshEXT %main "main" %x
               OpExecutionModeId %main LocalSizeId %uint_1 %uint_1 %uint_1
               OpExecutionMode %main OutputVertices 3
               OpExecutionMode %main OutputPrimitivesEXT 1
               OpExecutionMode %main OutputTrianglesEXT
               OpDecorate %x Location 0
       %void = OpTypeVoid
          %4 = OpTypeFunction %void
       %uint = OpTypeInt 32 0
     %uint_1 = OpConstant %uint 1
%_ptr_Function_uint = OpTypePointer Function %uint
     %uint_3 = OpConstant %uint 3
%_arr_uint_uint_3 = OpTypeArray %uint %uint_3
%_ptr_Output__arr_uint_uint_3 = OpTypePointer Output %_arr_uint_uint_3
          %x = OpVariable %_ptr_Output__arr_uint_uint_3 Output
        %int = OpTypeInt 32 1
      %int_1 = OpConstant %int 1
%_ptr_Output_uint = OpTypePointer Output %uint
       %main = OpFunction %void None %4
          %6 = OpLabel
          %y = OpVariable %_ptr_Function_uint Function
         %18 = OpAccessChain %_ptr_Output_uint %x %int_1
         %19 = OpLoad %uint %18
               OpStore %y %19
               OpSetMeshOutputsEXT %uint_3 %uint_1
               OpReturn
               OpFunctionEnd
)";

  CompileSuccessfully(body, SPV_ENV_VULKAN_1_3);
  EXPECT_EQ(SPV_ERROR_INVALID_ID, ValidateInstructions(SPV_ENV_VULKAN_1_3));
  EXPECT_THAT(getDiagnosticString(),
              AnyVUID("VUID-StandaloneSpirv-MeshEXT-07107"));
  EXPECT_THAT(getDiagnosticString(),
              HasSubstr("The Output Storage Class in a Mesh Execution Model "
                        "must not be read from"));
}

TEST_F(ValidateMeshShading, MeshReadOutputVec) {
  const std::string body = R"(
               OpCapability MeshShadingEXT
               OpExtension "SPV_EXT_mesh_shader"
          %2 = OpExtInstImport "GLSL.std.450"
               OpMemoryModel Logical GLSL450
               OpEntryPoint MeshEXT %main "main" %x
               OpExecutionModeId %main LocalSizeId %uint_1 %uint_1 %uint_1
               OpExecutionMode %main OutputVertices 3
               OpExecutionMode %main OutputPrimitivesEXT 1
               OpExecutionMode %main OutputTrianglesEXT
               OpDecorate %x Location 0
       %void = OpTypeVoid
          %4 = OpTypeFunction %void
       %uint = OpTypeInt 32 0
     %uint_1 = OpConstant %uint 1
      %float = OpTypeFloat 32
    %v4float = OpTypeVector %float 4
%_ptr_Function_v4float = OpTypePointer Function %v4float
     %uint_3 = OpConstant %uint 3
%_arr_v4float_uint_3 = OpTypeArray %v4float %uint_3
%_ptr_Output__arr_v4float_uint_3 = OpTypePointer Output %_arr_v4float_uint_3
          %x = OpVariable %_ptr_Output__arr_v4float_uint_3 Output
        %int = OpTypeInt 32 1
      %int_1 = OpConstant %int 1
%_ptr_Output_v4float = OpTypePointer Output %v4float
       %main = OpFunction %void None %4
          %6 = OpLabel
          %y = OpVariable %_ptr_Function_v4float Function
         %20 = OpAccessChain %_ptr_Output_v4float %x %int_1
         %21 = OpLoad %v4float %20
               OpStore %y %21
               OpSetMeshOutputsEXT %uint_3 %uint_1
               OpReturn
               OpFunctionEnd
)";

  CompileSuccessfully(body, SPV_ENV_VULKAN_1_3);
  EXPECT_EQ(SPV_ERROR_INVALID_ID, ValidateInstructions(SPV_ENV_VULKAN_1_3));
  EXPECT_THAT(getDiagnosticString(),
              AnyVUID("VUID-StandaloneSpirv-MeshEXT-07107"));
  EXPECT_THAT(getDiagnosticString(),
              HasSubstr("The Output Storage Class in a Mesh Execution Model "
                        "must not be read from"));
}

TEST_F(ValidateMeshShading, MeshReadOutputStruct) {
  const std::string body = R"(
               OpCapability MeshShadingEXT
               OpExtension "SPV_EXT_mesh_shader"
          %2 = OpExtInstImport "GLSL.std.450"
               OpMemoryModel Logical GLSL450
               OpEntryPoint MeshEXT %main "main" %x_0
               OpExecutionModeId %main LocalSizeId %uint_1 %uint_1 %uint_1
               OpExecutionMode %main OutputVertices 3
               OpExecutionMode %main OutputPrimitivesEXT 1
               OpExecutionMode %main OutputTrianglesEXT
               OpDecorate %x_0 Location 0
       %void = OpTypeVoid
          %4 = OpTypeFunction %void
       %uint = OpTypeInt 32 0
     %uint_1 = OpConstant %uint 1
%_ptr_Function_uint = OpTypePointer Function %uint
        %Bar = OpTypeStruct %uint
        %Foo = OpTypeStruct %Bar
     %uint_3 = OpConstant %uint 3
%_arr_Foo_uint_3 = OpTypeArray %Foo %uint_3
%_ptr_Output__arr_Foo_uint_3 = OpTypePointer Output %_arr_Foo_uint_3
        %x_0 = OpVariable %_ptr_Output__arr_Foo_uint_3 Output
        %int = OpTypeInt 32 1
      %int_2 = OpConstant %int 2
      %int_0 = OpConstant %int 0
%_ptr_Output_uint = OpTypePointer Output %uint
       %main = OpFunction %void None %4
          %6 = OpLabel
          %x = OpVariable %_ptr_Function_uint Function
         %21 = OpAccessChain %_ptr_Output_uint %x_0 %int_2 %int_0 %int_0
         %22 = OpLoad %uint %21
               OpStore %x %22
               OpSetMeshOutputsEXT %uint_3 %uint_1
               OpReturn
               OpFunctionEnd
)";

  CompileSuccessfully(body, SPV_ENV_VULKAN_1_3);
  EXPECT_EQ(SPV_ERROR_INVALID_ID, ValidateInstructions(SPV_ENV_VULKAN_1_3));
  EXPECT_THAT(getDiagnosticString(),
              AnyVUID("VUID-StandaloneSpirv-MeshEXT-07107"));
  EXPECT_THAT(getDiagnosticString(),
              HasSubstr("The Output Storage Class in a Mesh Execution Model "
                        "must not be read from"));
}

TEST_F(ValidateMeshShading, MeshReadOutputIntUntyped) {
  const std::string body = R"(
               OpCapability MeshShadingEXT
               OpCapability UntypedPointersKHR
               OpExtension "SPV_EXT_mesh_shader"
               OpExtension "SPV_KHR_untyped_pointers"
               OpMemoryModel Logical GLSL450
               OpEntryPoint MeshEXT %main "main" %x
               OpExecutionModeId %main LocalSizeId %uint_1 %uint_1 %uint_1
               OpExecutionMode %main OutputVertices 3
               OpExecutionMode %main OutputPrimitivesEXT 1
               OpExecutionMode %main OutputTrianglesEXT
               OpDecorate %x Location 0
       %void = OpTypeVoid
          %4 = OpTypeFunction %void
       %uint = OpTypeInt 32 0
     %uint_1 = OpConstant %uint 1
%_ptr_Function_uint = OpTypePointer Function %uint
     %uint_3 = OpConstant %uint 3
%_arr_uint_uint_3 = OpTypeArray %uint %uint_3
%_ptr_Output__arr_uint_uint_3 = OpTypePointer Output %_arr_uint_uint_3
          %x = OpVariable %_ptr_Output__arr_uint_uint_3 Output
        %int = OpTypeInt 32 1
      %int_1 = OpConstant %int 1
%_ptr_Output_uint = OpTypeUntypedPointerKHR Output
       %main = OpFunction %void None %4
          %6 = OpLabel
          %y = OpVariable %_ptr_Function_uint Function
         %18 = OpUntypedAccessChainKHR %_ptr_Output_uint %_arr_uint_uint_3 %x %int_1
         %19 = OpLoad %uint %18
               OpStore %y %19
               OpSetMeshOutputsEXT %uint_3 %uint_1
               OpReturn
               OpFunctionEnd
)";

  CompileSuccessfully(body, SPV_ENV_VULKAN_1_3);
  EXPECT_EQ(SPV_ERROR_INVALID_ID, ValidateInstructions(SPV_ENV_VULKAN_1_3));
  EXPECT_THAT(getDiagnosticString(),
              HasSubstr("In Vulkan, untyped pointers can only be used in an "
                        "explicitly laid out storage class"));
}

}  // namespace
}  // namespace val
}  // namespace spvtools
