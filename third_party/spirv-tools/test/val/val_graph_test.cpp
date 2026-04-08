// Copyright (c) 2023-2025 Arm Ltd.
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

#include "gmock/gmock.h"
#include "test/unit_spirv.h"
#include "test/val/val_fixtures.h"

// TODO(kpet) Tests with spec constants

namespace spvtools {
namespace val {
namespace {

using ::testing::ContainsRegex;
using ::testing::HasSubstr;

using ValidateGraph = spvtest::ValidateBase<std::string>;

constexpr spv_target_env SPVENV = SPV_ENV_VULKAN_1_3;

std::string GenerateModule(const std::string& src) {
  const std::string boilerplate = R"(
                     OpCapability Shader
                     OpCapability VulkanMemoryModel
                     OpCapability Int8
                     OpCapability GraphARM
                     OpCapability TensorsARM
                     OpCapability RuntimeDescriptorArray
                     OpExtension "SPV_ARM_graph"
                     OpExtension "SPV_ARM_tensors"
                     OpMemoryModel Logical Vulkan
                     OpDecorate %var_int8tensor DescriptorSet 0
                     OpDecorate %var_int8tensor Binding 0
                     OpDecorate %var_int32tensor DescriptorSet 0
                     OpDecorate %var_int32tensor Binding 1
                     OpDecorate %var_int8tensor_array3 DescriptorSet 0
                     OpDecorate %var_int8tensor_array3 Binding 2
                     OpDecorate %var_int8tensor_runtime_array DescriptorSet 0
                     OpDecorate %var_int8tensor_runtime_array Binding 3
             %void = OpTypeVoid
             %uint = OpTypeInt 32 0
             %int8 = OpTypeInt 8 1
            %int32 = OpTypeInt 32 1
            %float = OpTypeFloat 32
           %uint_0 = OpConstant %uint 0
           %uint_1 = OpConstant %uint 1
           %uint_2 = OpConstant %uint 2
           %uint_3 = OpConstant %uint 3
           %uint_4 = OpConstant %uint 4
          %float_1 = OpConstant %float 1.0
       %int8tensor = OpTypeTensorARM %int8 %uint_4
     %int8r3tensor = OpTypeTensorARM %int8 %uint_3
      %int32tensor = OpTypeTensorARM %int32 %uint_4
%int8tensor_array3 = OpTypeArray %int8tensor %uint_3
%int32tensor_array3 = OpTypeArray %int32tensor %uint_3
%int8tensor_runtime_array = OpTypeRuntimeArray %int8tensor
%int32tensor_runtime_array = OpTypeRuntimeArray %int32tensor
%ptr_Input_int8tensor = OpTypePointer Input %int8tensor
%var_int8tensor_wrong_storage_class = OpVariable %ptr_Input_int8tensor Input
%ptr_UniformConstant_int8tensor = OpTypePointer UniformConstant %int8tensor
%ptr_UniformConstant_int32tensor = OpTypePointer UniformConstant %int32tensor
%ptr_UniformConstant_int8tensor_array3 = OpTypePointer UniformConstant %int8tensor_array3
%ptr_UniformConstant_int8tensor_runtime_array = OpTypePointer UniformConstant %int8tensor_runtime_array
   %var_int8tensor = OpVariable %ptr_UniformConstant_int8tensor UniformConstant
   %var_int32tensor = OpVariable %ptr_UniformConstant_int32tensor UniformConstant
%var_int8tensor_array3 = OpVariable %ptr_UniformConstant_int8tensor_array3 UniformConstant
%var_int8tensor_runtime_array = OpVariable %ptr_UniformConstant_int8tensor_runtime_array UniformConstant
)";
  return boilerplate + src;
}

std::string GenerateModuleWithGraphEntryPoint(const std::string& header) {
  const std::string src = R"(
%default_graph_type = OpTypeGraphARM 1 %int8tensor %int8tensor
              OpGraphEntryPointARM %default_graph "default_entry_point" %var_int8tensor %var_int8tensor
     %default_graph = OpGraphARM %default_graph_type
        %in = OpGraphInputARM %int8tensor %uint_0
              OpGraphSetOutputARM %in %uint_0
              OpGraphEndARM
    )";
  return GenerateModule(header) + src;
}

//
// Layout tests
//

TEST_F(ValidateGraph, InvalidNoGraphEntryPoint) {
  std::string spvasm = GenerateModule("");
  CompileSuccessfully(spvasm, SPVENV);
  EXPECT_EQ(SPV_ERROR_INVALID_BINARY, ValidateInstructions(SPVENV));
  EXPECT_THAT(getDiagnosticString(),
              HasSubstr("No OpGraphEntryPointARM instruction was found but the "
                        "GraphARM capability is declared."));
}

TEST_F(ValidateGraph, InvalidGraphInGraph) {
  const std::string src = R"(
       %graph_type = OpTypeGraphARM 1 %int8tensor %int8tensor
            %graph = OpGraphARM %graph_type
           %graph2 = OpGraphARM %graph_type
)";
  std::string spvasm = GenerateModule(src);

  CompileSuccessfully(spvasm, SPVENV);
  EXPECT_EQ(SPV_ERROR_INVALID_LAYOUT, ValidateInstructions(SPVENV));
  EXPECT_THAT(getDiagnosticString(),
              HasSubstr("Cannot define a graph in a graph"));
}

TEST_F(ValidateGraph, InvalidGraphEndOutsideGraph) {
  const std::string src = R"(
                     OpGraphEndARM
)";
  std::string spvasm = GenerateModule(src);

  CompileSuccessfully(spvasm, SPVENV);
  EXPECT_EQ(SPV_ERROR_INVALID_LAYOUT, ValidateInstructions(SPVENV));
  EXPECT_THAT(getDiagnosticString(),
              HasSubstr("GraphEndARM must be preceded by at least one "
                        "OpGraphSetOutputARM instruction"));
}

TEST_F(ValidateGraph, InvalidGraphEntryPointInsideGraph) {
  const std::string src = R"(
       %graph_type = OpTypeGraphARM 1 %int8tensor %int8tensor
            %graph = OpGraphARM %graph_type
               %in = OpGraphInputARM %int8tensor %uint_0
                     OpGraphEntryPointARM %graph "main" %in %in
)";
  std::string spvasm = GenerateModule(src);

  CompileSuccessfully(spvasm, SPVENV);
  EXPECT_EQ(SPV_ERROR_INVALID_LAYOUT, ValidateInstructions(SPVENV));
  EXPECT_THAT(
      getDiagnosticString(),
      HasSubstr(
          "GraphEntryPointARM cannot appear in the definition of a graph"));
}

TEST_F(ValidateGraph, InvalidNonGraphInstructionInGraphSection) {
  const std::string src = R"(
       %graph_type = OpTypeGraphARM 1 %int8tensor %int8tensor
            %ftype = OpTypeFunction %void %void
            %graph = OpGraphARM %graph_type
               %in = OpGraphInputARM %int8tensor %uint_0
                     OpGraphSetOutputARM %in %uint_0
                     OpGraphEndARM
               %fn = OpFunction %void None %ftype
)";
  std::string spvasm = GenerateModule(src);

  CompileSuccessfully(spvasm, SPVENV);
  EXPECT_EQ(SPV_ERROR_INVALID_LAYOUT, ValidateInstructions(SPVENV));
  EXPECT_THAT(
      getDiagnosticString(),
      HasSubstr("Function cannot appear in the graph definitions section"));
}

TEST_F(ValidateGraph, InvalidGraphInputOusideGraph) {
  const std::string src = R"(
               %in = OpGraphInputARM %int8tensor %uint_0
)";
  std::string spvasm = GenerateModule(src);

  CompileSuccessfully(spvasm, SPVENV);
  EXPECT_EQ(SPV_ERROR_INVALID_LAYOUT, ValidateInstructions(SPVENV));
  EXPECT_THAT(getDiagnosticString(),
              HasSubstr("OpGraphInputARM must immediately follow an OpGraphARM "
                        "or OpGraphInputARM instruction."));
}

TEST_F(ValidateGraph, InvalidGraphSetOutputsOusideGraph) {
  const std::string src = R"(
                     OpGraphSetOutputARM %uint_0 %uint_0
)";
  std::string spvasm = GenerateModule(src);

  CompileSuccessfully(spvasm, SPVENV);
  EXPECT_EQ(SPV_ERROR_INVALID_LAYOUT, ValidateInstructions(SPVENV));
  EXPECT_THAT(getDiagnosticString(),
              HasSubstr("OpGraphSetOutputARM must immediately precede an "
                        "OpGraphEndARM or OpGraphSetOutputARM instruction"));
}

TEST_F(ValidateGraph, ValidGraphConstantOusideGraph) {
  const std::string src = R"(
%graph_type = OpTypeGraphARM 1 %int8tensor %int8tensor
       %cst = OpGraphConstantARM %int8tensor 1
              OpGraphEntryPointARM %graph "longname" %var_int8tensor %var_int8tensor
     %graph = OpGraphARM %graph_type
        %in = OpGraphInputARM %int8tensor %uint_0
              OpGraphSetOutputARM %in %uint_0
              OpGraphEndARM
)";
  std::string spvasm = GenerateModule(src);

  CompileSuccessfully(spvasm, SPVENV);
  EXPECT_EQ(SPV_SUCCESS, ValidateInstructions(SPVENV));
}

TEST_F(ValidateGraph, ValidGraphWithNoInputsNoBody) {
  const std::string src = R"(
              %cst = OpGraphConstantARM %int8tensor 1
       %graph_type = OpTypeGraphARM 0 %int8tensor
                     OpGraphEntryPointARM %graph "longname" %var_int8tensor
            %graph = OpGraphARM %graph_type
                     OpGraphSetOutputARM %cst %uint_0
                     OpGraphEndARM
)";
  std::string spvasm = GenerateModule(src);

  CompileSuccessfully(spvasm, SPVENV);
  EXPECT_EQ(SPV_SUCCESS, ValidateInstructions(SPVENV));
}

TEST_F(ValidateGraph, InvalidGraphWithDisallowedBodyInstructions) {
  const std::string src = R"(
       %graph_type = OpTypeGraphARM 1 %int8tensor %int8tensor
                     OpGraphEntryPointARM %graph "longname" %var_int8tensor %var_int8tensor
            %graph = OpGraphARM %graph_type
               %in = OpGraphInputARM %int8tensor %uint_0
              %val = OpCompositeExtract %int8r3tensor %in 0
              %out = OpCompositeInsert %int8tensor %val %in 0
                     OpGraphSetOutputARM %out %uint_0
                     OpGraphEndARM
)";
  std::string spvasm = GenerateModule(src);

  CompileSuccessfully(spvasm, SPVENV);
  EXPECT_EQ(SPV_ERROR_INVALID_LAYOUT, ValidateInstructions(SPVENV));
  EXPECT_THAT(
      getDiagnosticString(),
      HasSubstr(
          "OpCompositeInsert cannot appear in the graph definitions section"));
}

TEST_F(ValidateGraph, InvalidInstructionOutsideGraphAfterGraph) {
  const std::string src = R"(
%graph_type = OpTypeGraphARM 1 %int8tensor %int8tensor
       %cst = OpGraphConstantARM %int8tensor 1
              OpGraphEntryPointARM %graph "longname" %var_int8tensor %var_int8tensor
     %graph = OpGraphARM %graph_type
        %in = OpGraphInputARM %int8tensor %uint_0
              OpGraphSetOutputARM %in %uint_0
              OpGraphEndARM
       %val = OpCompositeExtract %int8r3tensor %cst 0
)";
  std::string spvasm = GenerateModule(src);

  CompileSuccessfully(spvasm, SPVENV);
  EXPECT_EQ(SPV_ERROR_INVALID_LAYOUT, ValidateInstructions(SPVENV));
  EXPECT_THAT(getDiagnosticString(),
              HasSubstr("OpCompositeExtract must appear in a graph body"));
}

//
// Type tests
//
TEST_F(ValidateGraph, ValidGraphTypeOneTensorOutputNoInputs) {
  const std::string src = R"(
       %graph_type = OpTypeGraphARM 0 %int8tensor
)";
  std::string spvasm = GenerateModuleWithGraphEntryPoint(src);

  CompileSuccessfully(spvasm, SPVENV);
  EXPECT_EQ(SPV_SUCCESS, ValidateInstructions(SPVENV));
}

TEST_F(ValidateGraph, ValidGraphTypeOneTensorArrayOutputNoInputs) {
  const std::string src = R"(
       %graph_type = OpTypeGraphARM 0 %int8tensor_array3
)";
  std::string spvasm = GenerateModuleWithGraphEntryPoint(src);

  CompileSuccessfully(spvasm, SPVENV);
  EXPECT_EQ(SPV_SUCCESS, ValidateInstructions(SPVENV));
}

TEST_F(ValidateGraph, ValidGraphTypeOneTensorRuntimeArrayOutputNoInputs) {
  const std::string src = R"(
       %graph_type = OpTypeGraphARM 0 %int8tensor_runtime_array
)";
  std::string spvasm = GenerateModuleWithGraphEntryPoint(src);

  CompileSuccessfully(spvasm, SPVENV);
  EXPECT_EQ(SPV_SUCCESS, ValidateInstructions(SPVENV));
}

TEST_F(ValidateGraph, ValidGraphTypeOneTensorOutputOneTensorInput) {
  const std::string src = R"(
       %graph_type = OpTypeGraphARM 1 %int8tensor %int32tensor
)";
  std::string spvasm = GenerateModuleWithGraphEntryPoint(src);

  CompileSuccessfully(spvasm, SPVENV);
  EXPECT_EQ(SPV_SUCCESS, ValidateInstructions(SPVENV));
}

TEST_F(ValidateGraph, ValidGraphTypeOneTensorOutputOneTensorArrayInput) {
  const std::string src = R"(
       %graph_type = OpTypeGraphARM 1 %int8tensor_array3 %int32tensor
)";
  std::string spvasm = GenerateModuleWithGraphEntryPoint(src);

  CompileSuccessfully(spvasm, SPVENV);
  EXPECT_EQ(SPV_SUCCESS, ValidateInstructions(SPVENV));
}

TEST_F(ValidateGraph, ValidGraphTypeOneTensorOutputOneTensorRuntimeArrayInput) {
  const std::string src = R"(
       %graph_type = OpTypeGraphARM 1 %int8tensor_runtime_array %int32tensor
)";
  std::string spvasm = GenerateModuleWithGraphEntryPoint(src);

  CompileSuccessfully(spvasm, SPVENV);
  EXPECT_EQ(SPV_SUCCESS, ValidateInstructions(SPVENV));
}

TEST_F(ValidateGraph,
       ValidGraphTypeOneTensorOutputOneTensorInputOneTensorArrayInput) {
  const std::string src = R"(
       %graph_type = OpTypeGraphARM 2 %int8tensor %int8tensor_array3 %int32tensor
)";
  std::string spvasm = GenerateModuleWithGraphEntryPoint(src);

  CompileSuccessfully(spvasm, SPVENV);
  EXPECT_EQ(SPV_SUCCESS, ValidateInstructions(SPVENV));
}

TEST_F(
    ValidateGraph,
    ValidGraphTypeOneTensorOutputOneTensorRuntimeArrayInputOneTensorArrayInput) {
  const std::string src = R"(
       %graph_type = OpTypeGraphARM 1 %int8tensor_runtime_array %int8tensor_array3 %int32tensor
)";
  std::string spvasm = GenerateModuleWithGraphEntryPoint(src);

  CompileSuccessfully(spvasm, SPVENV);
  EXPECT_EQ(SPV_SUCCESS, ValidateInstructions(SPVENV));
}

TEST_F(ValidateGraph, ValidGraphTypeOneTensorArrayOutputOneTensorInput) {
  const std::string src = R"(
       %graph_type = OpTypeGraphARM 1 %int8tensor %int32tensor_array3
)";
  std::string spvasm = GenerateModuleWithGraphEntryPoint(src);

  CompileSuccessfully(spvasm, SPVENV);
  EXPECT_EQ(SPV_SUCCESS, ValidateInstructions(SPVENV));
}

TEST_F(ValidateGraph, ValidGraphTypeOneTensorRuntimeArrayOutputOneTensorInput) {
  const std::string src = R"(
       %graph_type = OpTypeGraphARM 1 %int8tensor %int32tensor_runtime_array
)";
  std::string spvasm = GenerateModuleWithGraphEntryPoint(src);

  CompileSuccessfully(spvasm, SPVENV);
  EXPECT_EQ(SPV_SUCCESS, ValidateInstructions(SPVENV));
}

TEST_F(ValidateGraph,
       ValidGraphTypeOneTensorRuntimeArrayOutputOneTensorOuputOneTensorInput) {
  const std::string src = R"(
       %graph_type = OpTypeGraphARM 1 %int8tensor %int32tensor_runtime_array %int32tensor
)";
  std::string spvasm = GenerateModuleWithGraphEntryPoint(src);

  CompileSuccessfully(spvasm, SPVENV);
  EXPECT_EQ(SPV_SUCCESS, ValidateInstructions(SPVENV));
}

TEST_F(
    ValidateGraph,
    ValidGraphTypeOneTensorRuntimeArrayOutputOneTensorArrayOutputOneTensorOuputOneTensorInput) {
  const std::string src = R"(
       %graph_type = OpTypeGraphARM 1 %int8tensor %int32tensor_runtime_array %int32tensor_array3 %int32tensor
)";
  std::string spvasm = GenerateModuleWithGraphEntryPoint(src);

  CompileSuccessfully(spvasm, SPVENV);
  EXPECT_EQ(SPV_SUCCESS, ValidateInstructions(SPVENV));
}
TEST_F(ValidateGraph,
       ValidGraphTypeOneTensorRuntimeArrayOutputOnteTensorOuputOneTensorInput) {
  const std::string src = R"(
       %graph_type = OpTypeGraphARM 1 %int8tensor_array3 %int32tensor_array3
)";
  std::string spvasm = GenerateModuleWithGraphEntryPoint(src);

  CompileSuccessfully(spvasm, SPVENV);
  EXPECT_EQ(SPV_SUCCESS, ValidateInstructions(SPVENV));
}

TEST_F(ValidateGraph, InvalidGraphTypeWithNotEnoughIOTypes) {
  const std::string src = R"(
       %graph_type = OpTypeGraphARM 10 %int8tensor %int8tensor
)";
  std::string spvasm = GenerateModule(src);

  CompileSuccessfully(spvasm, SPVENV);
  EXPECT_EQ(SPV_ERROR_INVALID_DATA, ValidateInstructions(SPVENV));
  EXPECT_THAT(
      getDiagnosticString(),
      HasSubstr("2 I/O types were provided but the graph has 10 inputs"));
}

TEST_F(ValidateGraph, InvalidGraphTypeWithNonGraphInterfaceTypeIO) {
  const std::string src = R"(
       %graph_type = OpTypeGraphARM 1 %int8tensor %uint
)";
  std::string spvasm = GenerateModule(src);

  CompileSuccessfully(spvasm, SPVENV);
  EXPECT_EQ(SPV_ERROR_INVALID_DATA, ValidateInstructions(SPVENV));
  EXPECT_THAT(getDiagnosticString(),
              ContainsRegex("I/O type '.*' is not a Graph Interface Type.*"));
}

TEST_F(ValidateGraph, InvalidGraphTypeWithOneInputZeroOutputs) {
  const std::string src = R"(
       %graph_type = OpTypeGraphARM 1 %int8tensor
)";
  std::string spvasm = GenerateModule(src);

  CompileSuccessfully(spvasm, SPVENV);
  EXPECT_EQ(SPV_ERROR_INVALID_DATA, ValidateInstructions(SPVENV));
  EXPECT_THAT(getDiagnosticString(),
              HasSubstr("A graph type must have at least one output"));
}

TEST_F(ValidateGraph, InvalidGraphTypeWithZeroInputsZeroOutputs) {
  const std::string src = R"(
       %graph_type = OpTypeGraphARM 0
)";
  std::string spvasm = GenerateModule(src);

  CompileSuccessfully(spvasm, SPVENV);
  EXPECT_EQ(SPV_ERROR_INVALID_DATA, ValidateInstructions(SPVENV));
  EXPECT_THAT(getDiagnosticString(),
              HasSubstr("A graph type must have at least one output"));
}

//
// Constant tests
//
TEST_F(ValidateGraph, ValidGraphConstantTensorUnranked) {
  const std::string src = R"(
   %tensor_type =  OpTypeTensorARM %uint
           %cst = OpGraphConstantARM %tensor_type 25
)";
  std::string spvasm = GenerateModuleWithGraphEntryPoint(src);

  CompileSuccessfully(spvasm, SPVENV);
  EXPECT_EQ(SPV_SUCCESS, ValidateInstructions(SPVENV));
}

TEST_F(ValidateGraph, ValidGraphConstantTensorRanked) {
  const std::string src = R"(
   %tensor_type =  OpTypeTensorARM %uint %uint_4
           %cst = OpGraphConstantARM %tensor_type 25
)";
  std::string spvasm = GenerateModuleWithGraphEntryPoint(src);

  CompileSuccessfully(spvasm, SPVENV);
  EXPECT_EQ(SPV_SUCCESS, ValidateInstructions(SPVENV));
}

TEST_F(ValidateGraph, ValidGraphConstantTensorShaped) {
  const std::string src = R"(
   %uint_array2 = OpTypeArray %uint %uint_2
  %tensor_shape = OpConstantComposite %uint_array2 %uint_1 %uint_4
   %tensor_type = OpTypeTensorARM %uint %uint_2 %tensor_shape
           %cst = OpGraphConstantARM %tensor_type 25
)";
  std::string spvasm = GenerateModuleWithGraphEntryPoint(src);

  CompileSuccessfully(spvasm, SPVENV);
  EXPECT_EQ(SPV_SUCCESS, ValidateInstructions(SPVENV));
}

TEST_F(ValidateGraph, InvalidGraphConstantDuplicateIDs) {
  const std::string src = R"(
           %cst = OpGraphConstantARM %int8tensor 25
          %cst2 = OpGraphConstantARM %int32tensor 25
)";
  std::string spvasm = GenerateModuleWithGraphEntryPoint(src);

  CompileSuccessfully(spvasm, SPVENV);
  EXPECT_EQ(SPV_ERROR_INVALID_DATA, ValidateInstructions(SPVENV));
  EXPECT_THAT(getDiagnosticString(),
              HasSubstr("No two OpGraphConstantARM instructions may have the "
                        "same GraphConstantID"));
}

TEST_F(ValidateGraph, InvalidGraphConstantWithNonTensorType) {
  const std::string src = R"(
           %cst = OpGraphConstantARM %uint 25
)";
  std::string spvasm = GenerateModule(src);

  CompileSuccessfully(spvasm, SPVENV);
  EXPECT_EQ(SPV_ERROR_INVALID_DATA, ValidateInstructions(SPVENV));
  EXPECT_THAT(
      getDiagnosticString(),
      HasSubstr(
          "GraphConstantARM must have a Result Type that is a tensor type"));
}

//
// EntryPoint tests
//
TEST_F(ValidateGraph, InvalidModuleWithNoGraphEntryPoint) {
  std::string spvasm = GenerateModule("");

  CompileSuccessfully(spvasm, SPVENV);
  EXPECT_EQ(SPV_ERROR_INVALID_BINARY, ValidateInstructions(SPVENV));
  EXPECT_THAT(getDiagnosticString(),
              HasSubstr("No OpGraphEntryPointARM instruction was found but the "
                        "GraphARM capability is declared."));
}
TEST_F(ValidateGraph, InvalidGraphEntryPointNotAGraph) {
  const std::string src = R"(
           OpGraphEntryPointARM %uint_0 "longname"
)";
  std::string spvasm = GenerateModule(src);

  CompileSuccessfully(spvasm, SPVENV);
  EXPECT_EQ(SPV_ERROR_INVALID_DATA, ValidateInstructions(SPVENV));
  EXPECT_THAT(
      getDiagnosticString(),
      HasSubstr(
          "GraphEntryPointARM Graph must be a OpGraphARM but found Constant"));
}

TEST_F(ValidateGraph, InvalidGraphEntryPointInterfaceIDNotOpVariable) {
  const std::string src = R"(
%graph_type = OpTypeGraphARM 1 %int8tensor %int8tensor
              OpGraphEntryPointARM %graph "longname" %uint_0 %uint_0
     %graph = OpGraphARM %graph_type
        %in = OpGraphInputARM %int8tensor %uint_0
              OpGraphSetOutputARM %in %uint_0
              OpGraphEndARM
)";
  std::string spvasm = GenerateModule(src);

  CompileSuccessfully(spvasm, SPVENV);
  EXPECT_EQ(SPV_ERROR_INVALID_DATA, ValidateInstructions(SPVENV));
  EXPECT_THAT(getDiagnosticString(),
              ContainsRegex(
                  "GraphEntryPointARM Interface ID '.*' must "
                  "come from OpVariable with UniformConstant Storage Class.*"));
}

TEST_F(ValidateGraph, InvalidGraphEntryPointInterfaceIDWrongStorageClass) {
  const std::string src = R"(
%graph_type = OpTypeGraphARM 1 %int8tensor %int8tensor
              OpGraphEntryPointARM %graph "longname" %var_int8tensor_wrong_storage_class %var_int8tensor_wrong_storage_class
     %graph = OpGraphARM %graph_type
        %in = OpGraphInputARM %int8tensor %uint_0
              OpGraphSetOutputARM %in %uint_0
              OpGraphEndARM
)";
  std::string spvasm = GenerateModule(src);

  CompileSuccessfully(spvasm, SPVENV);
  EXPECT_EQ(SPV_ERROR_INVALID_DATA, ValidateInstructions(SPVENV));
  EXPECT_THAT(getDiagnosticString(),
              ContainsRegex(
                  "GraphEntryPointARM Interface ID '.*' must "
                  "come from OpVariable with UniformConstant Storage Class.*"));
}

TEST_F(ValidateGraph, InvalidGraphEntryPointNotEnoughInterfaceIDs) {
  const std::string src = R"(
%graph_type = OpTypeGraphARM 1 %int8tensor %int8tensor
              OpGraphEntryPointARM %graph "longname" %var_int8tensor
     %graph = OpGraphARM %graph_type
        %in = OpGraphInputARM %int8tensor %uint_0
              OpGraphSetOutputARM %in %uint_0
              OpGraphEndARM
)";
  std::string spvasm = GenerateModule(src);

  CompileSuccessfully(spvasm, SPVENV);
  EXPECT_EQ(SPV_ERROR_INVALID_DATA, ValidateInstructions(SPVENV));
  EXPECT_THAT(
      getDiagnosticString(),
      ContainsRegex(
          "GraphEntryPointARM Interface list contains 1 IDs but Graph's type "
          "'.*' has 2 inputs and outputs.*"));
}

TEST_F(ValidateGraph, InvalidGraphEntryPointTooManyInterfaceIDs) {
  const std::string src = R"(
%graph_type = OpTypeGraphARM 1 %int8tensor %int8tensor
              OpGraphEntryPointARM %graph "longname" %var_int8tensor %var_int8tensor %var_int8tensor
     %graph = OpGraphARM %graph_type
        %in = OpGraphInputARM %int8tensor %uint_0
              OpGraphSetOutputARM %in %uint_0
              OpGraphEndARM
)";
  std::string spvasm = GenerateModule(src);

  CompileSuccessfully(spvasm, SPVENV);
  EXPECT_EQ(SPV_ERROR_INVALID_DATA, ValidateInstructions(SPVENV));
  EXPECT_THAT(
      getDiagnosticString(),
      ContainsRegex(
          "GraphEntryPointARM Interface list contains 3 IDs but Graph's type "
          "'.*' has 2 inputs and outputs.*"));
}

TEST_F(ValidateGraph,
       InvalidGraphEntryPointInterfaceIDTypeMismatchesGraphIOType) {
  const std::string src = R"(
%graph_type = OpTypeGraphARM 1 %int8tensor %int8tensor
              OpGraphEntryPointARM %graph "longname" %var_int8tensor %var_int32tensor
     %graph = OpGraphARM %graph_type
        %in = OpGraphInputARM %int8tensor %uint_0
              OpGraphSetOutputARM %in %uint_0
              OpGraphEndARM
)";
  std::string spvasm = GenerateModule(src);

  CompileSuccessfully(spvasm, SPVENV);
  EXPECT_EQ(SPV_ERROR_INVALID_DATA, ValidateInstructions(SPVENV));
  EXPECT_THAT(getDiagnosticString(),
              ContainsRegex("GraphEntryPointARM Interface ID type "
                            "'.*' must match the type of the "
                            "corresponding graph I/O '.*'.*"));
}

//
// Graph tests
//
TEST_F(ValidateGraph, InvalidGraphResultType) {
  const std::string src = R"(
           OpGraphEntryPointARM %graph "foo"
  %graph = OpGraphARM %uint
     %in = OpGraphInputARM %int8tensor %uint_0
           OpGraphSetOutputARM %in %uint_0
           OpGraphEndARM
)";
  std::string spvasm = GenerateModule(src);

  CompileSuccessfully(spvasm, SPVENV);
  EXPECT_EQ(SPV_ERROR_INVALID_DATA, ValidateInstructions(SPVENV));
  EXPECT_THAT(getDiagnosticString(),
              HasSubstr("GraphARM Result Type must be an OpTypeGraphARM"));
}

//
// Input tests
//
TEST_F(ValidateGraph, ValidTensorInput) {
  const std::string src = R"(
%graph_type = OpTypeGraphARM 1 %int8tensor %int8tensor
              OpGraphEntryPointARM %graph "longname" %var_int8tensor %var_int8tensor
     %graph = OpGraphARM %graph_type
        %in = OpGraphInputARM %int8tensor %uint_0
              OpGraphSetOutputARM %in %uint_0
              OpGraphEndARM
)";
  std::string spvasm = GenerateModule(src);

  CompileSuccessfully(spvasm, SPVENV);
  EXPECT_EQ(SPV_SUCCESS, ValidateInstructions(SPVENV));
}

TEST_F(ValidateGraph, ValidTensorArrayInputCompositeExtract) {
  const std::string src = R"(
%graph_type = OpTypeGraphARM 1 %int8tensor_array3 %int8tensor
              OpGraphEntryPointARM %graph "longname" %var_int8tensor_array3 %var_int8tensor
     %graph = OpGraphARM %graph_type
%in_tensors = OpGraphInputARM %int8tensor_array3 %uint_0
       %out = OpCompositeExtract %int8tensor %in_tensors 1
              OpGraphSetOutputARM %out %uint_0
              OpGraphEndARM
)";
  std::string spvasm = GenerateModule(src);

  CompileSuccessfully(spvasm, SPVENV);
  EXPECT_EQ(SPV_SUCCESS, ValidateInstructions(SPVENV));
}

TEST_F(ValidateGraph, ValidTensorArrayInputWithElementIndex) {
  const std::string src = R"(
%graph_type = OpTypeGraphARM 1 %int8tensor_array3 %int8tensor
              OpGraphEntryPointARM %graph "longname" %var_int8tensor_array3 %var_int8tensor
     %graph = OpGraphARM %graph_type
       %out = OpGraphInputARM %int8tensor %uint_0 %uint_1
              OpGraphSetOutputARM %out %uint_0
              OpGraphEndARM
)";
  std::string spvasm = GenerateModule(src);

  CompileSuccessfully(spvasm, SPVENV);
  EXPECT_EQ(SPV_SUCCESS, ValidateInstructions(SPVENV));
}

TEST_F(ValidateGraph, ValidTensorRuntimeArrayInputCompositeExtract) {
  const std::string src = R"(
%graph_type = OpTypeGraphARM 1 %int8tensor_runtime_array %int8tensor
              OpGraphEntryPointARM %graph "longname" %var_int8tensor_runtime_array %var_int8tensor
     %graph = OpGraphARM %graph_type
%in_tensors = OpGraphInputARM %int8tensor_runtime_array %uint_0
       %out = OpCompositeExtract %int8tensor %in_tensors 1
              OpGraphSetOutputARM %out %uint_0
              OpGraphEndARM
)";
  std::string spvasm = GenerateModule(src);

  CompileSuccessfully(spvasm, SPVENV);
  EXPECT_EQ(SPV_SUCCESS, ValidateInstructions(SPVENV));
}

TEST_F(ValidateGraph, ValidTensorRuntimeArrayInputWithElementIndex) {
  const std::string src = R"(
%graph_type = OpTypeGraphARM 1 %int8tensor_runtime_array %int8tensor
              OpGraphEntryPointARM %graph "longname" %var_int8tensor_runtime_array %var_int8tensor
     %graph = OpGraphARM %graph_type
       %out = OpGraphInputARM %int8tensor %uint_0 %uint_1
              OpGraphSetOutputARM %out %uint_0
              OpGraphEndARM
)";
  std::string spvasm = GenerateModule(src);

  CompileSuccessfully(spvasm, SPVENV);
  EXPECT_EQ(SPV_SUCCESS, ValidateInstructions(SPVENV));
}

TEST_F(ValidateGraph, InvalidGraphInputIndexWrongType) {
  const std::string src = R"(
%graph_type = OpTypeGraphARM 1 %int8tensor %int8tensor
              OpGraphEntryPointARM %graph "longname" %var_int8tensor %var_int8tensor
     %graph = OpGraphARM %graph_type
        %in = OpGraphInputARM %int8tensor %float_1
              OpGraphSetOutputARM %in %uint_0
              OpGraphEndARM
)";
  std::string spvasm = GenerateModule(src);

  CompileSuccessfully(spvasm, SPVENV);
  EXPECT_EQ(SPV_ERROR_INVALID_DATA, ValidateInstructions(SPVENV));
  EXPECT_THAT(getDiagnosticString(),
              HasSubstr("GraphInputARM InputIndex must be a 32-bit integer"));
}

TEST_F(ValidateGraph, InvalidGraphInputElementIndexWrongType) {
  const std::string src = R"(
%graph_type = OpTypeGraphARM 1 %int8tensor_array3 %int8tensor
              OpGraphEntryPointARM %graph "longname" %var_int8tensor_array3 %var_int8tensor
     %graph = OpGraphARM %graph_type
        %in = OpGraphInputARM %int8tensor %uint_0 %float_1
              OpGraphSetOutputARM %in %uint_0
              OpGraphEndARM
)";
  std::string spvasm = GenerateModule(src);

  CompileSuccessfully(spvasm, SPVENV);
  EXPECT_EQ(SPV_ERROR_INVALID_DATA, ValidateInstructions(SPVENV));
  EXPECT_THAT(getDiagnosticString(),
              HasSubstr("GraphInputARM ElementIndex must be a 32-bit integer"));
}

TEST_F(ValidateGraph, InvalidGraphInputIndexDuplicate) {
  const std::string src = R"(
%graph_type = OpTypeGraphARM 1 %int8tensor %int8tensor
              OpGraphEntryPointARM %graph "longname" %var_int8tensor %var_int8tensor
     %graph = OpGraphARM %graph_type
        %in = OpGraphInputARM %int8tensor %uint_0
       %in2 = OpGraphInputARM %int8tensor %uint_0
              OpGraphSetOutputARM %in %uint_0
              OpGraphEndARM
)";
  std::string spvasm = GenerateModule(src);

  CompileSuccessfully(spvasm, SPVENV);
  EXPECT_EQ(SPV_ERROR_INVALID_DATA, ValidateInstructions(SPVENV));
  EXPECT_THAT(getDiagnosticString(),
              HasSubstr("Two OpGraphInputARM instructions with the same "
                        "InputIndex must not be part of the same "
                        "graph definition unless ElementIndex is present in "
                        "both with different values."));
}

TEST_F(ValidateGraph, InvalidGraphInputAndElementIndexDuplicate) {
  const std::string src = R"(
%graph_type = OpTypeGraphARM 1 %int8tensor_array3 %int8tensor
              OpGraphEntryPointARM %graph "longname" %var_int8tensor_array3 %var_int8tensor
     %graph = OpGraphARM %graph_type
        %in = OpGraphInputARM %int8tensor %uint_0 %uint_1
       %in2 = OpGraphInputARM %int8tensor %uint_0 %uint_1
              OpGraphSetOutputARM %in %uint_0
              OpGraphEndARM
)";
  std::string spvasm = GenerateModule(src);

  CompileSuccessfully(spvasm, SPVENV);
  EXPECT_EQ(SPV_ERROR_INVALID_DATA, ValidateInstructions(SPVENV));
  EXPECT_THAT(getDiagnosticString(),
              HasSubstr("Two OpGraphInputARM instructions with the same "
                        "InputIndex must not be part of the same "
                        "graph definition unless ElementIndex is present in "
                        "both with different values."));
}

TEST_F(ValidateGraph,
       InvalidGraphInputIndexDuplicateWithAndWithoutElementIndex) {
  const std::string src = R"(
%graph_type = OpTypeGraphARM 1 %int8tensor_array3 %int8tensor
              OpGraphEntryPointARM %graph "longname" %var_int8tensor_array3 %var_int8tensor
     %graph = OpGraphARM %graph_type
        %in = OpGraphInputARM %int8tensor_array3 %uint_0
       %in2 = OpGraphInputARM %int8tensor %uint_0 %uint_1
              OpGraphSetOutputARM %in2 %uint_0
              OpGraphEndARM
)";
  std::string spvasm = GenerateModule(src);

  CompileSuccessfully(spvasm, SPVENV);
  EXPECT_EQ(SPV_ERROR_INVALID_DATA, ValidateInstructions(SPVENV));
  EXPECT_THAT(getDiagnosticString(),
              HasSubstr("Two OpGraphInputARM instructions with the same "
                        "InputIndex must not be part of the same "
                        "graph definition unless ElementIndex is present in "
                        "both with different values."));
}

TEST_F(ValidateGraph, ValidGraphInputIndexDuplicateWithDifferentElementIndex) {
  const std::string src = R"(
%graph_type = OpTypeGraphARM 1 %int8tensor_runtime_array %int8tensor
              OpGraphEntryPointARM %graph "longname" %var_int8tensor_runtime_array %var_int8tensor
     %graph = OpGraphARM %graph_type
        %in = OpGraphInputARM %int8tensor %uint_0 %uint_2
       %in2 = OpGraphInputARM %int8tensor %uint_0 %uint_1
              OpGraphSetOutputARM %in2 %uint_0
              OpGraphEndARM
)";
  std::string spvasm = GenerateModule(src);

  CompileSuccessfully(spvasm, SPVENV);
  EXPECT_EQ(SPV_SUCCESS, ValidateInstructions(SPVENV));
}

TEST_F(ValidateGraph, InvalidGraphInputElementIndexWithNonArrayInput) {
  const std::string src = R"(
%graph_type = OpTypeGraphARM 1 %int8tensor %int8tensor
              OpGraphEntryPointARM %graph "longname" %var_int8tensor %var_int8tensor
     %graph = OpGraphARM %graph_type
        %in = OpGraphInputARM %int8tensor %uint_0 %uint_1
              OpGraphSetOutputARM %in %uint_0
              OpGraphEndARM
)";
  std::string spvasm = GenerateModule(src);

  CompileSuccessfully(spvasm, SPVENV);
  EXPECT_EQ(SPV_ERROR_INVALID_DATA, ValidateInstructions(SPVENV));
  EXPECT_THAT(
      getDiagnosticString(),
      HasSubstr("OpGraphInputARM ElementIndex not allowed when the graph input "
                "selected by "
                "InputIndex is not an OpTypeArray or OpTypeRuntimeArray"));
}

TEST_F(ValidateGraph, InvalidGraphInputElementIndexOutOfRange) {
  const std::string src = R"(
%graph_type = OpTypeGraphARM 1 %int8tensor_array3 %int8tensor
              OpGraphEntryPointARM %graph "longname" %var_int8tensor_array3 %var_int8tensor
     %graph = OpGraphARM %graph_type
        %in = OpGraphInputARM %int8tensor %uint_0 %uint_3
              OpGraphSetOutputARM %in %uint_0
              OpGraphEndARM
)";
  std::string spvasm = GenerateModule(src);

  CompileSuccessfully(spvasm, SPVENV);
  EXPECT_EQ(SPV_ERROR_INVALID_DATA, ValidateInstructions(SPVENV));
  EXPECT_THAT(getDiagnosticString(),
              ContainsRegex("OpGraphInputARM ElementIndex out of range. The "
                            "type of the graph input being accessed '.*' is an "
                            "array of 3 elements but ElementIndex is 3.*"));
}

TEST_F(ValidateGraph, InvalidGraphInputIndexOutOfRange) {
  const std::string src = R"(
%graph_type = OpTypeGraphARM 2 %int8tensor %int8tensor %int8tensor
              OpGraphEntryPointARM %graph "longname" %var_int8tensor %var_int8tensor %var_int8tensor
     %graph = OpGraphARM %graph_type
       %in0 = OpGraphInputARM %int8tensor %uint_0
       %in1 = OpGraphInputARM %int8tensor %uint_1
       %in2 = OpGraphInputARM %int8tensor %uint_2
              OpGraphSetOutputARM %in0 %uint_0
              OpGraphEndARM
)";
  std::string spvasm = GenerateModule(src);

  CompileSuccessfully(spvasm, SPVENV);
  EXPECT_EQ(SPV_ERROR_INVALID_DATA, ValidateInstructions(SPVENV));
  EXPECT_THAT(getDiagnosticString(),
              ContainsRegex("Type '.*' for graph '.*' "
                            "has 2 inputs but found an OpGraphInputARM "
                            "instruction with an InputIndex that is 2.*"));
}

TEST_F(ValidateGraph, InvalidGraphFirstInputTypeDoesNotMatchGraphType) {
  const std::string src = R"(
%graph_type = OpTypeGraphARM 2 %int8tensor %int8tensor %int8tensor
              OpGraphEntryPointARM %graph "longname" %var_int8tensor %var_int8tensor %var_int8tensor
     %graph = OpGraphARM %graph_type
       %in0 = OpGraphInputARM %int32tensor %uint_0
       %in1 = OpGraphInputARM %int8tensor %uint_1
              OpGraphSetOutputARM %in1 %uint_0
              OpGraphEndARM
)";
  std::string spvasm = GenerateModule(src);

  CompileSuccessfully(spvasm, SPVENV);
  EXPECT_EQ(SPV_ERROR_INVALID_DATA, ValidateInstructions(SPVENV));
  EXPECT_THAT(getDiagnosticString(),
              ContainsRegex("Result Type '.*' of graph input instruction "
                            "'.*' does not match the type "
                            "'.*' of input 0 in the graph type.*"));
}

TEST_F(ValidateGraph, InvalidGraphLastInputTypeDoesNotMatchGraphType) {
  const std::string src = R"(
%graph_type = OpTypeGraphARM 2 %int8tensor %int8tensor %int8tensor
              OpGraphEntryPointARM %graph "longname" %var_int8tensor %var_int8tensor %var_int8tensor
     %graph = OpGraphARM %graph_type
       %in0 = OpGraphInputARM %int8tensor %uint_0
       %in1 = OpGraphInputARM %int32tensor %uint_1
              OpGraphSetOutputARM %in0 %uint_0
              OpGraphEndARM
)";
  std::string spvasm = GenerateModule(src);

  CompileSuccessfully(spvasm, SPVENV);
  EXPECT_EQ(SPV_ERROR_INVALID_DATA, ValidateInstructions(SPVENV));
  EXPECT_THAT(getDiagnosticString(),
              ContainsRegex("Result Type '.*' of graph input instruction "
                            "'.*' does not match the type "
                            "'.*' of input 1 in the graph type.*"));
}

TEST_F(ValidateGraph, InvalidGraphInputAfterNonGraphInputOrGraph) {
  const std::string src = R"(
%graph_type = OpTypeGraphARM 2 %int8tensor_array3 %int8tensor %int8tensor
              OpGraphEntryPointARM %graph "longname" %var_int8tensor_array3 %var_int8tensor %var_int8tensor
     %graph = OpGraphARM %graph_type
        %in = OpGraphInputARM %int8tensor_array3 %uint_0
       %val = OpCompositeExtract %int8tensor %in 0
       %in2 = OpGraphInputARM %int8tensor %uint_1
              OpGraphSetOutputARM %val %uint_0
              OpGraphEndARM
)";
  std::string spvasm = GenerateModule(src);

  CompileSuccessfully(spvasm, SPVENV);
  EXPECT_EQ(SPV_ERROR_INVALID_LAYOUT, ValidateInstructions(SPVENV));
  EXPECT_THAT(getDiagnosticString(),
              HasSubstr("OpGraphInputARM must immediately follow an OpGraphARM "
                        "or OpGraphInputARM instruction"));
}

//
// Output tests
//
TEST_F(ValidateGraph, ValidTensorOutput) {
  const std::string src = R"(
%graph_type = OpTypeGraphARM 1 %int8tensor %int8tensor
              OpGraphEntryPointARM %graph "longname" %var_int8tensor %var_int8tensor
     %graph = OpGraphARM %graph_type
        %in = OpGraphInputARM %int8tensor %uint_0
              OpGraphSetOutputARM %in %uint_0
              OpGraphEndARM
)";
  std::string spvasm = GenerateModule(src);

  CompileSuccessfully(spvasm, SPVENV);
  EXPECT_EQ(SPV_SUCCESS, ValidateInstructions(SPVENV));
}

TEST_F(ValidateGraph, ValidTensorOutputWholeArray) {
  const std::string src = R"(
%graph_type = OpTypeGraphARM 1 %int8tensor_array3 %int8tensor_array3
              OpGraphEntryPointARM %graph "longname" %var_int8tensor_array3 %var_int8tensor_array3
     %graph = OpGraphARM %graph_type
        %in = OpGraphInputARM %int8tensor_array3 %uint_0
              OpGraphSetOutputARM %in %uint_0
              OpGraphEndARM
)";
  std::string spvasm = GenerateModule(src);

  CompileSuccessfully(spvasm, SPVENV);
  EXPECT_EQ(SPV_SUCCESS, ValidateInstructions(SPVENV));
}

TEST_F(ValidateGraph, ValidTensorOutputArraySingleElement) {
  const std::string src = R"(
%graph_type = OpTypeGraphARM 1 %int8tensor %int8tensor_array3
              OpGraphEntryPointARM %graph "longname" %var_int8tensor %var_int8tensor_array3
     %graph = OpGraphARM %graph_type
        %in = OpGraphInputARM %int8tensor %uint_0
              OpGraphSetOutputARM %in %uint_0 %uint_1
              OpGraphEndARM
)";
  std::string spvasm = GenerateModule(src);

  CompileSuccessfully(spvasm, SPVENV);
  EXPECT_EQ(SPV_SUCCESS, ValidateInstructions(SPVENV));
}

TEST_F(ValidateGraph, ValidTensorOutputArrayMultipleElements) {
  const std::string src = R"(
%graph_type = OpTypeGraphARM 1 %int8tensor %int8tensor_array3
              OpGraphEntryPointARM %graph "longname" %var_int8tensor %var_int8tensor_array3
     %graph = OpGraphARM %graph_type
        %in = OpGraphInputARM %int8tensor %uint_0
              OpGraphSetOutputARM %in %uint_0 %uint_1
              OpGraphSetOutputARM %in %uint_0 %uint_2
              OpGraphEndARM
)";
  std::string spvasm = GenerateModule(src);

  CompileSuccessfully(spvasm, SPVENV);
  EXPECT_EQ(SPV_SUCCESS, ValidateInstructions(SPVENV));
}

TEST_F(ValidateGraph, ValidTensorOutputWholeRuntimeArray) {
  const std::string src = R"(
%graph_type = OpTypeGraphARM 1 %int8tensor_runtime_array %int8tensor_runtime_array
              OpGraphEntryPointARM %graph "longname" %var_int8tensor_runtime_array %var_int8tensor_runtime_array
     %graph = OpGraphARM %graph_type
        %in = OpGraphInputARM %int8tensor_runtime_array %uint_0
              OpGraphSetOutputARM %in %uint_0
              OpGraphEndARM
)";
  std::string spvasm = GenerateModule(src);

  CompileSuccessfully(spvasm, SPVENV);
  EXPECT_EQ(SPV_SUCCESS, ValidateInstructions(SPVENV));
}

TEST_F(ValidateGraph, ValidTensorOutputRuntimeArraySingleElement) {
  const std::string src = R"(
%graph_type = OpTypeGraphARM 1 %int8tensor %int8tensor_runtime_array
              OpGraphEntryPointARM %graph "longname" %var_int8tensor %var_int8tensor_runtime_array
     %graph = OpGraphARM %graph_type
        %in = OpGraphInputARM %int8tensor %uint_0
              OpGraphSetOutputARM %in %uint_0 %uint_1
              OpGraphEndARM
)";
  std::string spvasm = GenerateModule(src);

  CompileSuccessfully(spvasm, SPVENV);
  EXPECT_EQ(SPV_SUCCESS, ValidateInstructions(SPVENV));
}

TEST_F(ValidateGraph, ValidTensorOutputRuntimeArrayMultipleElements) {
  const std::string src = R"(
%graph_type = OpTypeGraphARM 1 %int8tensor %int8tensor_runtime_array
              OpGraphEntryPointARM %graph "longname" %var_int8tensor %var_int8tensor_runtime_array
     %graph = OpGraphARM %graph_type
        %in = OpGraphInputARM %int8tensor %uint_0
              OpGraphSetOutputARM %in %uint_0 %uint_1
              OpGraphSetOutputARM %in %uint_0 %uint_2
              OpGraphEndARM
)";
  std::string spvasm = GenerateModule(src);

  CompileSuccessfully(spvasm, SPVENV);
  EXPECT_EQ(SPV_SUCCESS, ValidateInstructions(SPVENV));
}

TEST_F(ValidateGraph, InvalidGraphOutputIndexWrongType) {
  const std::string src = R"(
%graph_type = OpTypeGraphARM 1 %int8tensor %int8tensor
              OpGraphEntryPointARM %graph "longname" %var_int8tensor %var_int8tensor
     %graph = OpGraphARM %graph_type
        %in = OpGraphInputARM %int8tensor %uint_0
              OpGraphSetOutputARM %in %float_1
              OpGraphEndARM
)";
  std::string spvasm = GenerateModule(src);

  CompileSuccessfully(spvasm, SPVENV);
  EXPECT_EQ(SPV_ERROR_INVALID_DATA, ValidateInstructions(SPVENV));
  EXPECT_THAT(
      getDiagnosticString(),
      HasSubstr("GraphSetOutputARM OutputIndex must be a 32-bit integer"));
}

TEST_F(ValidateGraph, InvalidGraphOutputElementIndexWrongType) {
  const std::string src = R"(
%graph_type = OpTypeGraphARM 1 %int8tensor %int8tensor_array3
              OpGraphEntryPointARM %graph "longname" %var_int8tensor %var_int8tensor_array3
     %graph = OpGraphARM %graph_type
        %in = OpGraphInputARM %int8tensor %uint_0
              OpGraphSetOutputARM %in %uint_0 %float_1
              OpGraphEndARM
)";
  std::string spvasm = GenerateModule(src);

  CompileSuccessfully(spvasm, SPVENV);
  EXPECT_EQ(SPV_ERROR_INVALID_DATA, ValidateInstructions(SPVENV));
  EXPECT_THAT(
      getDiagnosticString(),
      HasSubstr("GraphSetOutputARM ElementIndex must be a 32-bit integer"));
}

TEST_F(ValidateGraph, InvalidGraphOutputIndexDuplicate) {
  const std::string src = R"(
%graph_type = OpTypeGraphARM 1 %int8tensor %int8tensor
              OpGraphEntryPointARM %graph "longname" %var_int8tensor %var_int8tensor
     %graph = OpGraphARM %graph_type
        %in = OpGraphInputARM %int8tensor %uint_0
              OpGraphSetOutputARM %in %uint_0
              OpGraphSetOutputARM %in %uint_0
              OpGraphEndARM
)";
  std::string spvasm = GenerateModule(src);

  CompileSuccessfully(spvasm, SPVENV);
  EXPECT_EQ(SPV_ERROR_INVALID_DATA, ValidateInstructions(SPVENV));
  EXPECT_THAT(getDiagnosticString(),
              HasSubstr("Two OpGraphSetOutputARM instructions with the same "
                        "OutputIndex must not be part of the same "
                        "graph definition unless ElementIndex is present in "
                        "both with different values."));
}

TEST_F(ValidateGraph, InvalidGraphOutputAndElementIndexDuplicate) {
  const std::string src = R"(
%graph_type = OpTypeGraphARM 1 %int8tensor %int8tensor_array3
              OpGraphEntryPointARM %graph "longname" %var_int8tensor %var_int8tensor_array3
     %graph = OpGraphARM %graph_type
        %in = OpGraphInputARM %int8tensor %uint_0
              OpGraphSetOutputARM %in %uint_0 %uint_1
              OpGraphSetOutputARM %in %uint_0 %uint_1
              OpGraphEndARM
)";
  std::string spvasm = GenerateModule(src);

  CompileSuccessfully(spvasm, SPVENV);
  EXPECT_EQ(SPV_ERROR_INVALID_DATA, ValidateInstructions(SPVENV));
  EXPECT_THAT(getDiagnosticString(),
              HasSubstr("Two OpGraphSetOutputARM instructions with the same "
                        "OutputIndex must not be part of the same "
                        "graph definition unless ElementIndex is present in "
                        "both with different values."));
}

TEST_F(ValidateGraph,
       InvalidGraphOutputIndexDuplicateWithAndWithoutElementIndex) {
  const std::string src = R"(
%graph_type = OpTypeGraphARM 1  %int8tensor_array3 %int8tensor_array3
              OpGraphEntryPointARM %graph "longname" %var_int8tensor_array3 %var_int8tensor_array3
     %graph = OpGraphARM %graph_type
        %in = OpGraphInputARM %int8tensor_array3 %uint_0
 %in_single = OpCompositeExtract %int8tensor %in 0
              OpGraphSetOutputARM %in %uint_0
              OpGraphSetOutputARM %in_single %uint_0 %uint_1
              OpGraphEndARM
)";
  std::string spvasm = GenerateModule(src);

  CompileSuccessfully(spvasm, SPVENV);
  EXPECT_EQ(SPV_ERROR_INVALID_DATA, ValidateInstructions(SPVENV));
  EXPECT_THAT(getDiagnosticString(),
              HasSubstr("Two OpGraphSetOutputARM instructions with the same "
                        "OutputIndex must not be part of the same "
                        "graph definition unless ElementIndex is present in "
                        "both with different values."));
}

TEST_F(ValidateGraph, ValidGraphOutputIndexDuplicateWithDifferentElementIndex) {
  const std::string src = R"(
%graph_type = OpTypeGraphARM 1 %int8tensor %int8tensor_runtime_array
              OpGraphEntryPointARM %graph "longname" %var_int8tensor %var_int8tensor_runtime_array
     %graph = OpGraphARM %graph_type
        %in = OpGraphInputARM %int8tensor %uint_0
              OpGraphSetOutputARM %in %uint_0 %uint_1
              OpGraphSetOutputARM %in %uint_0 %uint_4
              OpGraphEndARM
)";
  std::string spvasm = GenerateModule(src);

  CompileSuccessfully(spvasm, SPVENV);
  EXPECT_EQ(SPV_SUCCESS, ValidateInstructions(SPVENV));
}

TEST_F(ValidateGraph, InvalidGraphOutputElementIndexWithNonArrayOutput) {
  const std::string src = R"(
%graph_type = OpTypeGraphARM 1 %int8tensor %int8tensor
              OpGraphEntryPointARM %graph "longname" %var_int8tensor %var_int8tensor
     %graph = OpGraphARM %graph_type
        %in = OpGraphInputARM %int8tensor %uint_0
              OpGraphSetOutputARM %in %uint_0 %uint_0
              OpGraphEndARM
)";
  std::string spvasm = GenerateModule(src);

  CompileSuccessfully(spvasm, SPVENV);
  EXPECT_EQ(SPV_ERROR_INVALID_DATA, ValidateInstructions(SPVENV));
  EXPECT_THAT(
      getDiagnosticString(),
      HasSubstr("OpGraphSetOutputARM ElementIndex not allowed when the graph "
                "output selected by "
                "OutputIndex is not an OpTypeArray or OpTypeRuntimeArray"));
}

TEST_F(ValidateGraph, InvalidGraphOutputElementIndexOutOfRange) {
  const std::string src = R"(
%graph_type = OpTypeGraphARM 1 %int8tensor %int8tensor_array3
              OpGraphEntryPointARM %graph "longname" %var_int8tensor %var_int8tensor_array3
     %graph = OpGraphARM %graph_type
        %in = OpGraphInputARM %int8tensor %uint_0
              OpGraphSetOutputARM %in %uint_0 %uint_3
              OpGraphEndARM
)";
  std::string spvasm = GenerateModule(src);

  CompileSuccessfully(spvasm, SPVENV);
  EXPECT_EQ(SPV_ERROR_INVALID_DATA, ValidateInstructions(SPVENV));
  EXPECT_THAT(
      getDiagnosticString(),
      ContainsRegex("OpGraphSetOutputARM ElementIndex out of range. The "
                    "type of the graph output being accessed '.*' is an "
                    "array of 3 elements but ElementIndex is 3.*"));
}

TEST_F(ValidateGraph, InvalidGraphOutputNotBeforeEndOrOutput) {
  const std::string src = R"(
%graph_type = OpTypeGraphARM 1 %int8tensor_array3 %int8tensor_array3 %int8tensor
              OpGraphEntryPointARM %graph "longname" %var_int8tensor_array3 %var_int8tensor_array3 %var_int8tensor
     %graph = OpGraphARM %graph_type
        %in = OpGraphInputARM %int8tensor_array3 %uint_0
              OpGraphSetOutputARM %in %uint_0
       %val = OpCompositeExtract %int8tensor %in 0
              OpGraphSetOutputARM %val %uint_1
              OpGraphEndARM
)";
  std::string spvasm = GenerateModule(src);

  CompileSuccessfully(spvasm, SPVENV);
  EXPECT_EQ(SPV_ERROR_INVALID_LAYOUT, ValidateInstructions(SPVENV));
  EXPECT_THAT(
      getDiagnosticString(),
      HasSubstr(
          "CompositeExtract cannot appear after a graph output instruction"));
}

TEST_F(ValidateGraph, InvalidGraphOutputIndexOutOfRange) {
  const std::string src = R"(
%graph_type = OpTypeGraphARM 1 %int8tensor %int8tensor %int8tensor
              OpGraphEntryPointARM %graph "longname" %var_int8tensor %var_int8tensor %var_int8tensor
     %graph = OpGraphARM %graph_type
        %in = OpGraphInputARM %int8tensor %uint_0
              OpGraphSetOutputARM %in %uint_2
              OpGraphEndARM
)";
  std::string spvasm = GenerateModule(src);

  CompileSuccessfully(spvasm, SPVENV);
  EXPECT_EQ(SPV_ERROR_INVALID_DATA, ValidateInstructions(SPVENV));
  EXPECT_THAT(getDiagnosticString(),
              HasSubstr("GraphSetOutputARM setting OutputIndex 2 but graph "
                        "only has 2 outputs."));
}

TEST_F(ValidateGraph, InvalidGraphFirstOutputValueTypeDoesNotMatchOutputType) {
  const std::string src = R"(
%graph_type = OpTypeGraphARM 1 %int32tensor %int8tensor %int8tensor
              OpGraphEntryPointARM %graph "longname" %var_int32tensor %var_int8tensor %var_int8tensor
     %graph = OpGraphARM %graph_type
        %in = OpGraphInputARM %int32tensor %uint_0
              OpGraphSetOutputARM %in %uint_0
              OpGraphEndARM
)";
  std::string spvasm = GenerateModule(src);

  CompileSuccessfully(spvasm, SPVENV);
  EXPECT_EQ(SPV_ERROR_INVALID_DATA, ValidateInstructions(SPVENV));
  EXPECT_THAT(
      getDiagnosticString(),
      ContainsRegex(
          "The type '.*' of Value provided to the graph output instruction "
          "'.*' does not match the type '.*' of output 0 in the graph type.*"));
}

TEST_F(ValidateGraph, InvalidGraphLastOutputValueTypeDoesNotMatchOutputType) {
  const std::string src = R"(
%graph_type = OpTypeGraphARM 1 %int32tensor %int32tensor %int8tensor
              OpGraphEntryPointARM %graph "longname" %var_int32tensor %var_int32tensor %var_int8tensor
     %graph = OpGraphARM %graph_type
        %in = OpGraphInputARM %int32tensor %uint_0
              OpGraphSetOutputARM %in %uint_1
              OpGraphEndARM
)";
  std::string spvasm = GenerateModule(src);

  CompileSuccessfully(spvasm, SPVENV);
  EXPECT_EQ(SPV_ERROR_INVALID_DATA, ValidateInstructions(SPVENV));
  EXPECT_THAT(
      getDiagnosticString(),
      ContainsRegex(
          "The type '.*' of Value provided to the graph output instruction "
          "'.*' does not match the type '.*' of output 1 in the graph type.*"));
}

TEST_F(ValidateGraph, InvalidGraphNoSetOuputBeforeEnd) {
  const std::string src = R"(
%graph_type = OpTypeGraphARM 1 %int8tensor %int8tensor
              OpGraphEntryPointARM %graph "longname" %var_int8tensor %var_int8tensor
     %graph = OpGraphARM %graph_type
        %in = OpGraphInputARM %int8tensor %uint_0
              OpGraphEndARM
)";
  std::string spvasm = GenerateModule(src);

  CompileSuccessfully(spvasm, SPVENV);
  EXPECT_EQ(SPV_ERROR_INVALID_LAYOUT, ValidateInstructions(SPVENV));
  EXPECT_THAT(getDiagnosticString(),
              HasSubstr("GraphEndARM must be preceded by at least one "
                        "OpGraphSetOutputARM instruction"));
}

//
// End tests
//
TEST_F(ValidateGraph, InvalidGraphNoEnd) {
  const std::string src = R"(
%graph_type = OpTypeGraphARM 1 %int8tensor %int8tensor
              OpGraphEntryPointARM %graph "longname" %var_int8tensor %var_int8tensor
     %graph = OpGraphARM %graph_type
        %in = OpGraphInputARM %int8tensor %uint_0
)";
  std::string spvasm = GenerateModule(src);

  CompileSuccessfully(spvasm, SPVENV);
  EXPECT_EQ(SPV_ERROR_INVALID_LAYOUT, ValidateInstructions(SPVENV));
  EXPECT_THAT(getDiagnosticString(),
              HasSubstr("Missing OpGraphEndARM at end of module"));
}

}  // namespace
}  // namespace val
}  // namespace spvtools
