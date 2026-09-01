// Copyright 2026 LunarG Inc.
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

#include "gmock/gmock.h"
#include "test/val/val_fixtures.h"

namespace spvtools {
namespace val {
namespace {

using ::testing::HasSubstr;

using ValidateGroup = spvtest::ValidateBase<bool>;

std::string GenerateShaderCode(const std::string& body, bool is_64_bit = true) {
  std::ostringstream ss;
  ss << R"(
OpCapability Kernel
OpCapability Addresses
OpCapability Linkage
OpCapability Groups
OpCapability Float64
OpCapability Int64
)";
  if (is_64_bit) {
    ss << "OpMemoryModel Physical64 OpenCL";
  } else {
    ss << "OpMemoryModel Physical32 OpenCL";
  }
  ss << R"(
OpEntryPoint Kernel %main "main"
%bool = OpTypeBool
%float = OpTypeFloat 32
%float64 = OpTypeFloat 64
%uint = OpTypeInt 32 0
%uint64 = OpTypeInt 64 0
%null_uint = OpConstantNull %uint
%uint_0 = OpConstant %uint 0
%uint_1 = OpConstant %uint 1
%uint_2 = OpConstant %uint 2
%uint64_1 = OpConstant %uint64 1
%float_2 = OpConstant %float 2
%uint_array = OpTypeArray %uint %uint_2
%void = OpTypeVoid
%event = OpTypeEvent
%null_event = OpConstantNull %event

%workgroup_float_ptr = OpTypePointer Workgroup %float
%workgroup_float_var = OpVariable %workgroup_float_ptr Workgroup
%workgroup_bool_ptr = OpTypePointer Workgroup %bool
%workgroup_bool_var = OpVariable %workgroup_bool_ptr Workgroup
%cross_float_ptr = OpTypePointer CrossWorkgroup %float
%cross_float_var = OpVariable %cross_float_ptr CrossWorkgroup
%cross_uint_ptr = OpTypePointer CrossWorkgroup %uint
%cross_uint_var = OpVariable %cross_uint_ptr CrossWorkgroup
%uniform_float_ptr = OpTypePointer UniformConstant %float
%uniform_float_var = OpVariable %uniform_float_ptr UniformConstant
%func_event_ptr = OpTypePointer Function %event

%fn = OpTypeFunction %void
%true = OpConstantTrue %bool
%main = OpFunction %void None %fn
%label = OpLabel
)";

  ss << body;

  ss << R"(
OpReturn
OpFunctionEnd)";
  return ss.str();
}

TEST_F(ValidateGroup, AllAnyGood) {
  const std::string ss = R"(
    %x = OpGroupAll %bool %uint_2 %true
    %y = OpGroupAny %bool %uint_2 %true
  )";
  CompileSuccessfully(GenerateShaderCode(ss));
  EXPECT_EQ(SPV_SUCCESS, ValidateInstructions());
}

TEST_F(ValidateGroup, FloatGood) {
  const std::string ss = R"(
    %a = OpGroupFAdd %float %uint_2 Reduce %float_2
    %b = OpGroupFMin %float %uint_2 Reduce %float_2
    %c = OpGroupFMax %float %uint_2 Reduce %float_2
  )";
  CompileSuccessfully(GenerateShaderCode(ss));
  EXPECT_EQ(SPV_SUCCESS, ValidateInstructions());
}

TEST_F(ValidateGroup, IntGood) {
  const std::string ss = R"(
    %a = OpGroupIAdd %uint %uint_2 Reduce %uint_2
    %b = OpGroupSMin %uint %uint_2 Reduce %uint_2
    %c = OpGroupSMax %uint %uint_2 Reduce %uint_2
    %d = OpGroupUMin %uint %uint_2 Reduce %uint_2
    %e = OpGroupUMax %uint %uint_2 Reduce %uint_2
  )";
  CompileSuccessfully(GenerateShaderCode(ss));
  EXPECT_EQ(SPV_SUCCESS, ValidateInstructions());
}

TEST_F(ValidateGroup, BroadcastGood) {
  const std::string ss = R"(
    %a = OpGroupBroadcast %uint %uint_2 %uint_2 %uint_0
    %b = OpGroupBroadcast %float %uint_2 %float_2 %uint_0
  )";
  CompileSuccessfully(GenerateShaderCode(ss));
  EXPECT_EQ(SPV_SUCCESS, ValidateInstructions());
}

TEST_F(ValidateGroup, AllResult) {
  const std::string ss = R"(
    %x = OpGroupAll %uint %uint_2 %true
  )";
  CompileSuccessfully(GenerateShaderCode(ss));
  EXPECT_EQ(SPV_ERROR_INVALID_DATA, ValidateInstructions());
  EXPECT_THAT(getDiagnosticString(),
              HasSubstr("Result must be a boolean scalar type"));
}

TEST_F(ValidateGroup, AllPredicate) {
  const std::string ss = R"(
    %x = OpGroupAll %bool %uint_2 %uint_2
  )";
  CompileSuccessfully(GenerateShaderCode(ss));
  EXPECT_EQ(SPV_ERROR_INVALID_DATA, ValidateInstructions());
  EXPECT_THAT(getDiagnosticString(),
              HasSubstr("Predicate must be a boolean scalar type"));
}

TEST_F(ValidateGroup, AnyResult) {
  const std::string ss = R"(
    %x = OpGroupAny %uint %uint_2 %true
  )";
  CompileSuccessfully(GenerateShaderCode(ss));
  EXPECT_EQ(SPV_ERROR_INVALID_DATA, ValidateInstructions());
  EXPECT_THAT(getDiagnosticString(),
              HasSubstr("Result must be a boolean scalar type"));
}

TEST_F(ValidateGroup, AnyPredicate) {
  const std::string ss = R"(
    %x = OpGroupAny %bool %uint_2 %uint_2
  )";
  CompileSuccessfully(GenerateShaderCode(ss));
  EXPECT_EQ(SPV_ERROR_INVALID_DATA, ValidateInstructions());
  EXPECT_THAT(getDiagnosticString(),
              HasSubstr("Predicate must be a boolean scalar type"));
}

TEST_F(ValidateGroup, FAddWithInt) {
  const std::string ss = R"(
    %a = OpGroupFAdd %uint %uint_2 Reduce %uint_2
  )";
  CompileSuccessfully(GenerateShaderCode(ss));
  EXPECT_EQ(SPV_ERROR_INVALID_DATA, ValidateInstructions());
  EXPECT_THAT(getDiagnosticString(),
              HasSubstr("Result must be a scalar or vector of float type"));
}

TEST_F(ValidateGroup, FMaxWidthMismatch) {
  const std::string ss = R"(
    %a = OpGroupFAdd %float64 %uint_2 Reduce %float_2
  )";
  CompileSuccessfully(GenerateShaderCode(ss));
  EXPECT_EQ(SPV_ERROR_INVALID_DATA, ValidateInstructions());
  EXPECT_THAT(getDiagnosticString(),
              HasSubstr("The type of X must match the Result type"));
}

TEST_F(ValidateGroup, IAddWithFloat) {
  const std::string ss = R"(
     %a = OpGroupIAdd %float %uint_2 Reduce %float_2
  )";
  CompileSuccessfully(GenerateShaderCode(ss));
  EXPECT_EQ(SPV_ERROR_INVALID_DATA, ValidateInstructions());
  EXPECT_THAT(getDiagnosticString(),
              HasSubstr("Result must be a scalar or vector of integer type"));
}

TEST_F(ValidateGroup, UMinWithArray) {
  const std::string ss = R"(
    %a = OpGroupUMin %uint_array %uint_2 Reduce %float_2
  )";
  CompileSuccessfully(GenerateShaderCode(ss));
  EXPECT_EQ(SPV_ERROR_INVALID_DATA, ValidateInstructions());
  EXPECT_THAT(getDiagnosticString(),
              HasSubstr("Result must be a scalar or vector of integer type"));
}

TEST_F(ValidateGroup, SMaxWidthMismatch) {
  const std::string ss = R"(
    %c = OpGroupSMax %uint64 %uint_2 Reduce %uint_2
  )";
  CompileSuccessfully(GenerateShaderCode(ss));
  EXPECT_EQ(SPV_ERROR_INVALID_DATA, ValidateInstructions());
  EXPECT_THAT(getDiagnosticString(),
              HasSubstr("The type of X must match the Result type"));
}

TEST_F(ValidateGroup, BroadcastArray) {
  const std::string ss = R"(
    %a = OpGroupBroadcast %uint_array %uint_2 %uint_2 %uint_0
  )";
  CompileSuccessfully(GenerateShaderCode(ss));
  EXPECT_EQ(SPV_ERROR_INVALID_DATA, ValidateInstructions());
  EXPECT_THAT(getDiagnosticString(),
              HasSubstr("Result must be a scalar or vector of integer, "
                        "floating-point, or boolean type"));
}

TEST_F(ValidateGroup, BroadcastMismatch) {
  const std::string ss = R"(
    %b = OpGroupBroadcast %uint %uint_2 %float_2 %uint_0
  )";
  CompileSuccessfully(GenerateShaderCode(ss));
  EXPECT_EQ(SPV_ERROR_INVALID_DATA, ValidateInstructions());
  EXPECT_THAT(getDiagnosticString(),
              HasSubstr("The type of Value must match the Result type"));
}

TEST_F(ValidateGroup, AsyncCopyWaitEventsGood) {
  const std::string ss = R"(
               OpCapability Kernel
               OpCapability Addresses
               OpCapability Int64
               OpCapability Int8
               OpCapability Linkage
               OpMemoryModel Physical64 OpenCL
               OpEntryPoint Kernel %async_example "async_example"
               OpExecutionMode %async_example ContractionOff
               OpDecorate %26 Alignment 4
               OpDecorate %29 Alignment 8
               OpDecorate %async_example_local_data Alignment 4
      %float = OpTypeFloat 32
%_ptr_CrossWorkgroup_float = OpTypePointer CrossWorkgroup %float
       %void = OpTypeVoid
          %5 = OpTypeFunction %void %_ptr_CrossWorkgroup_float
%spirv_Event = OpTypeEvent
%_ptr_Workgroup_float = OpTypePointer Workgroup %float
      %ulong = OpTypeInt 64 0
       %uint = OpTypeInt 32 0
%_ptr_Function_spirv_Event = OpTypePointer Function %spirv_Event
    %uint_64 = OpConstant %uint 64
%_arr_float_uint_64 = OpTypeArray %float %uint_64
%_ptr_Workgroup__arr_float_uint_64 = OpTypePointer Workgroup %_arr_float_uint_64
    %ulong_1 = OpConstant %ulong 1
   %ulong_64 = OpConstant %ulong 64
     %uint_1 = OpConstant %uint 1
     %uint_2 = OpConstant %uint 2
      %uchar = OpTypeInt 8 0
%_ptr_Function_uchar = OpTypePointer Function %uchar
%async_example_local_data = OpVariable %_ptr_Workgroup__arr_float_uint_64 Workgroup
         %24 = OpConstantNull %spirv_Event
%async_example = OpFunction %void None %5
         %26 = OpFunctionParameter %_ptr_CrossWorkgroup_float
         %54 = OpLabel
         %29 = OpVariable %_ptr_Function_spirv_Event Function
         %30 = OpBitcast %_ptr_Workgroup_float %async_example_local_data
         %31 = OpBitcast %_ptr_Function_uchar %29
         %32 = OpGroupAsyncCopy %spirv_Event %uint_2 %30 %26 %ulong_64 %ulong_1 %24
               OpStore %29 %32 Aligned 8
               OpGroupWaitEvents %uint_2 %uint_1 %29
               OpReturn
               OpFunctionEnd
  )";
  CompileSuccessfully(ss);
  EXPECT_EQ(SPV_SUCCESS, ValidateInstructions());
}

TEST_F(ValidateGroup, AsyncCopyResultType) {
  const std::string ss = R"(
    %a = OpGroupAsyncCopy %uint %uint_2 %workgroup_float_var %cross_float_var %uint64_1 %uint64_1 %null_event
  )";
  CompileSuccessfully(GenerateShaderCode(ss));
  EXPECT_EQ(SPV_ERROR_INVALID_DATA, ValidateInstructions());
  EXPECT_THAT(getDiagnosticString(),
              HasSubstr("The result type must be OpTypeEvent"));
}

TEST_F(ValidateGroup, AsyncCopyDestinationPointer) {
  const std::string ss = R"(
    %a = OpGroupAsyncCopy %event %uint_2 %null_uint %cross_float_var %uint64_1 %uint64_1 %null_event
  )";
  CompileSuccessfully(GenerateShaderCode(ss));
  EXPECT_EQ(SPV_ERROR_INVALID_DATA, ValidateInstructions());
  EXPECT_THAT(getDiagnosticString(),
              HasSubstr("Expected Destination to be a pointer"));
}

TEST_F(ValidateGroup, AsyncCopyDestinationUniform) {
  const std::string ss = R"(
    %a = OpGroupAsyncCopy %event %uint_2 %uniform_float_var %cross_float_var %uint64_1 %uint64_1 %null_event
  )";
  CompileSuccessfully(GenerateShaderCode(ss));
  EXPECT_EQ(SPV_ERROR_INVALID_DATA, ValidateInstructions());
  EXPECT_THAT(getDiagnosticString(),
              HasSubstr("Expected Destination to be a pointer with storage "
                        "class Workgroup or CrossWorkgroup"));
}

TEST_F(ValidateGroup, AsyncCopyDestinationBool) {
  const std::string ss = R"(
      %a = OpGroupAsyncCopy %event %uint_2 %workgroup_bool_var %cross_float_var %uint64_1 %uint64_1 %null_event
  )";
  CompileSuccessfully(GenerateShaderCode(ss));
  EXPECT_EQ(SPV_ERROR_INVALID_DATA, ValidateInstructions());
  EXPECT_THAT(getDiagnosticString(),
              HasSubstr("Expected Destination to be a pointer to scalar or "
                        "vector of floating-point type or integer type"));
}

TEST_F(ValidateGroup, AsyncCopyDestinationSourceTypes) {
  const std::string ss = R"(
      %a = OpGroupAsyncCopy %event %uint_2 %workgroup_float_var %cross_uint_var %uint64_1 %uint64_1 %null_event
  )";
  CompileSuccessfully(GenerateShaderCode(ss));
  EXPECT_EQ(SPV_ERROR_INVALID_DATA, ValidateInstructions());
  EXPECT_THAT(getDiagnosticString(),
              HasSubstr("Expected Destination and Source to be the same type"));
}

TEST_F(ValidateGroup, AsyncCopyBothWorkgroup) {
  const std::string ss = R"(
      %a = OpGroupAsyncCopy %event %uint_2 %workgroup_float_var %workgroup_float_var %uint64_1 %uint64_1 %null_event
  )";
  CompileSuccessfully(GenerateShaderCode(ss));
  EXPECT_EQ(SPV_ERROR_INVALID_DATA, ValidateInstructions());
  EXPECT_THAT(getDiagnosticString(),
              HasSubstr("If Destination storage class is Workgroup, then the "
                        "Source storage class must be CrossWorkgroup."));
}

TEST_F(ValidateGroup, AsyncCopyBothCrossWorkgroup) {
  const std::string ss = R"(
      %a = OpGroupAsyncCopy %event %uint_2 %cross_float_var %cross_float_var %uint64_1 %uint64_1 %null_event
  )";
  CompileSuccessfully(GenerateShaderCode(ss));
  EXPECT_EQ(SPV_ERROR_INVALID_DATA, ValidateInstructions());
  EXPECT_THAT(getDiagnosticString(),
              HasSubstr("If Destination storage class is CrossWorkgroup, then "
                        "the Source storage class must be Workgroup"));
}

TEST_F(ValidateGroup, AsyncCopyEventType) {
  const std::string ss = R"(
      %a = OpGroupAsyncCopy %event %uint_2 %workgroup_float_var %cross_float_var %uint64_1 %uint64_1 %null_uint
  )";
  CompileSuccessfully(GenerateShaderCode(ss));
  EXPECT_EQ(SPV_ERROR_INVALID_DATA, ValidateInstructions());
  EXPECT_THAT(getDiagnosticString(),
              HasSubstr("Expected Event to be type OpTypeEvent"));
}

TEST_F(ValidateGroup, AsyncCopyNumElement32Bit) {
  const std::string ss = R"(
      %a = OpGroupAsyncCopy %event %uint_2 %workgroup_float_var %cross_float_var %uint_1 %uint64_1 %null_event
  )";
  CompileSuccessfully(GenerateShaderCode(ss));
  EXPECT_EQ(SPV_ERROR_INVALID_DATA, ValidateInstructions());
  EXPECT_THAT(getDiagnosticString(),
              HasSubstr("NumElements must be a 64-bit int scalar when "
                        "Addressing Model is Physical64"));
}

TEST_F(ValidateGroup, AsyncCopyStride32Bit) {
  const std::string ss = R"(
      %a = OpGroupAsyncCopy %event %uint_2 %workgroup_float_var %cross_float_var %uint64_1 %uint_1 %null_event
  )";
  CompileSuccessfully(GenerateShaderCode(ss));
  EXPECT_EQ(SPV_ERROR_INVALID_DATA, ValidateInstructions());
  EXPECT_THAT(getDiagnosticString(),
              HasSubstr("Stride must be a 64-bit int scalar when Addressing "
                        "Model is Physical64"));
}

TEST_F(ValidateGroup, AsyncCopyNumElement64Bit) {
  const std::string ss = R"(
      %a = OpGroupAsyncCopy %event %uint_2 %workgroup_float_var %cross_float_var %uint64_1 %uint_1 %null_event
  )";
  CompileSuccessfully(GenerateShaderCode(ss, false));
  EXPECT_EQ(SPV_ERROR_INVALID_DATA, ValidateInstructions());
  EXPECT_THAT(getDiagnosticString(),
              HasSubstr("NumElements must be a 32-bit int scalar when "
                        "Addressing Model is Physical32"));
}

TEST_F(ValidateGroup, AsyncCopyStride64Bit) {
  const std::string ss = R"(
      %a = OpGroupAsyncCopy %event %uint_2 %workgroup_float_var %cross_float_var %uint_1 %uint64_1 %null_event
  )";
  CompileSuccessfully(GenerateShaderCode(ss, false));
  EXPECT_EQ(SPV_ERROR_INVALID_DATA, ValidateInstructions());
  EXPECT_THAT(getDiagnosticString(),
              HasSubstr("Stride must be a 32-bit int scalar when Addressing "
                        "Model is Physical32"));
}

TEST_F(ValidateGroup, GroupWaitEventsNumEvents) {
  const std::string ss = R"(
    %a = OpVariable %func_event_ptr Function
    OpGroupWaitEvents %uint_2 %uint64_1 %a
  )";
  CompileSuccessfully(GenerateShaderCode(ss));
  EXPECT_EQ(SPV_ERROR_INVALID_DATA, ValidateInstructions());
  EXPECT_THAT(getDiagnosticString(),
              HasSubstr("Expected Num Events to be a 32-bit int scalar"));
}

TEST_F(ValidateGroup, GroupWaitEventsEventList) {
  const std::string ss = R"(
    OpGroupWaitEvents %uint_2 %uint_1 %null_uint
  )";
  CompileSuccessfully(GenerateShaderCode(ss));
  EXPECT_EQ(SPV_ERROR_INVALID_DATA, ValidateInstructions());
  EXPECT_THAT(getDiagnosticString(),
              HasSubstr("Expected Events List to be a pointer"));
}

TEST_F(ValidateGroup, GroupWaitEventsEventListType) {
  const std::string ss = R"(
    OpGroupWaitEvents %uint_2 %uint_1 %uniform_float_var
  )";
  CompileSuccessfully(GenerateShaderCode(ss));
  EXPECT_EQ(SPV_ERROR_INVALID_DATA, ValidateInstructions());
  EXPECT_THAT(getDiagnosticString(),
              HasSubstr("Expected Events List to be a pointer to OpTypeEvent"));
}

}  // namespace
}  // namespace val
}  // namespace spvtools
