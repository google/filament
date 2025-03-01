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

TEST_F(ValidateMeshShading, VulkanBasicMeshAndTaskSuccess) {
  const std::string body = R"(
               OpCapability MeshShadingEXT
               OpExtension "SPV_EXT_mesh_shader"
               OpExtension "SPV_NV_mesh_shader"
               OpMemoryModel Logical GLSL450
               OpEntryPoint MeshEXT %mainMesh "mainMesh"
               OpEntryPoint TaskEXT %mainTask "mainTask"
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
              HasSubstr("There can be at most one OpVariable with storage "
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

}  // namespace
}  // namespace val
}  // namespace spvtools
