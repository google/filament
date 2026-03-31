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

// Tests instructions from SPV_NV_shader_invocation_reorder and
// SPV_EXT_shader_invocation_reorder.

#include <sstream>
#include <string>

#include "gmock/gmock.h"
#include "test/val/val_fixtures.h"

namespace spvtools {
namespace val {
namespace {

using ::testing::HasSubstr;
using ::testing::Values;

using ValidateRayTracingReorderNV = spvtest::ValidateBase<bool>;

std::string GenerateReorderThreadCode(const std::string& body = "",
                                      const std::string& declarations = "",
                                      const std::string& extensions = "",
                                      const std::string& capabilities = "") {
  std::ostringstream ss;
  ss << R"(
            OpCapability RayTracingKHR
            OpCapability ShaderInvocationReorderNV
         )";
  ss << capabilities;
  ss << R"(
            OpExtension "SPV_KHR_ray_tracing"
            OpExtension "SPV_NV_shader_invocation_reorder"
          )";
  ss << extensions;
  ss << R"(
       %1 = OpExtInstImport "GLSL.std.450"
            OpMemoryModel Logical GLSL450
            OpEntryPoint RayGenerationNV %main "main" %hObj
            OpSourceExtension "GL_EXT_ray_tracing"
            OpSourceExtension "GL_NV_shader_invocation_reorder"
            OpName %main "main"
    %void = OpTypeVoid
       %3 = OpTypeFunction %void
       %6 = OpTypeHitObjectNV
%_ptr_Private_6 = OpTypePointer Private %6
    %hObj = OpVariable %_ptr_Private_6 Private
    )";
  ss << declarations;

  ss << R"(
   %main  = OpFunction %void None %3
     %5   = OpLabel 
    )";

  ss << body;

  ss << R"(
            OpReturn
            OpFunctionEnd
    )";
  return ss.str();
}

TEST_F(ValidateRayTracingReorderNV, ReorderThreadWithHintNV) {
  const std::string declarations = R"(
             %uint = OpTypeInt 32 0
           %uint_4 = OpConstant %uint 4
  )";

  const std::string body = R"(
    OpReorderThreadWithHintNV %uint_4 %uint_4
  )";

  CompileSuccessfully(GenerateReorderThreadCode(body, declarations).c_str(),
                      SPV_ENV_VULKAN_1_2);
  EXPECT_EQ(SPV_SUCCESS, ValidateInstructions(SPV_ENV_VULKAN_1_2));
}

TEST_F(ValidateRayTracingReorderNV, ReorderThreadWithHitObjectNV) {
  const std::string declarations = R"(
      %uint = OpTypeInt 32 0
    %uint_4 = OpConstant %uint 4
    %uint_2 = OpConstant %uint 2
  )";

  const std::string body = R"(
             OpReorderThreadWithHitObjectNV %hObj
             OpReorderThreadWithHitObjectNV %hObj %uint_4 %uint_2
  )";

  CompileSuccessfully(GenerateReorderThreadCode(body, declarations).c_str(),
                      SPV_ENV_VULKAN_1_2);
  EXPECT_EQ(SPV_SUCCESS, ValidateInstructions(SPV_ENV_VULKAN_1_2));
}

std::string GenerateReorderShaderCode(
    const std::string& body = "", const std::string& declarations = "",
    const std::string execution_model = "RayGenerationKHR") {
  std::ostringstream ss;
  ss << R"(
               OpCapability RayTracingKHR
               OpCapability ShaderInvocationReorderNV
               OpExtension "SPV_KHR_ray_tracing"
               OpExtension "SPV_NV_shader_invocation_reorder"
          %1 = OpExtInstImport "GLSL.std.450"
               OpMemoryModel Logical GLSL450
               OpEntryPoint )"
     << execution_model
     << R"( %main "main" %attr %_ %hObj %payload %__0 %as %__1
               OpSource GLSL 460
               OpSourceExtension "GL_EXT_ray_tracing"
               OpSourceExtension "GL_NV_shader_invocation_reorder"
               OpName %main "main"
               OpName %attr "attr"
               OpName %hBlock "hBlock"
               OpMemberName %hBlock 0 "attrval"
               OpName %_ ""
               OpName %hObj "hObj"
               OpName %payload "payload"
               OpName %pBlock "pBlock"
               OpMemberName %pBlock 0 "val1"
               OpMemberName %pBlock 1 "val2"
               OpName %__0 ""
               OpName %as "as"
               OpName %block "block"
               OpMemberName %block 0 "op"
               OpName %__1 ""
               OpDecorate %hBlock Block
               OpDecorate %pBlock Block
               OpDecorate %as DescriptorSet 0
               OpDecorate %as Binding 0
               OpMemberDecorate %block 0 Offset 0
               OpDecorate %block Block
               OpDecorate %__1 DescriptorSet 0
               OpDecorate %__1 Binding 1
       %void = OpTypeVoid
          %3 = OpTypeFunction %void
      %float = OpTypeFloat 32
    %v2float = OpTypeVector %float 2
%_ptr_HitObjectAttributeNV_v2float = OpTypePointer HitObjectAttributeNV %v2float
       %attr = OpVariable %_ptr_HitObjectAttributeNV_v2float HitObjectAttributeNV
    %float_1 = OpConstant %float 1
         %11 = OpConstantComposite %v2float %float_1 %float_1
     %hBlock = OpTypeStruct %float
%_ptr_HitObjectAttributeNV_hBlock = OpTypePointer HitObjectAttributeNV %hBlock
          %_ = OpVariable %_ptr_HitObjectAttributeNV_hBlock HitObjectAttributeNV
        %int = OpTypeInt 32 1
      %int_0 = OpConstant %int 0
    %float_2 = OpConstant %float 2
%_ptr_HitObjectAttributeNV_float = OpTypePointer HitObjectAttributeNV %float
             %20 = OpTypeHitObjectNV
    %_ptr_Private_20 = OpTypePointer Private %20
           %hObj = OpVariable %_ptr_Private_20 Private
             %23 = OpTypeAccelerationStructureKHR
    %_ptr_UniformConstant_23 = OpTypePointer UniformConstant %23
             %as = OpVariable %_ptr_UniformConstant_23 UniformConstant
    %v4float = OpTypeVector %float 4
%_ptr_RayPayloadNV_v4float = OpTypePointer RayPayloadNV %v4float
    %payload = OpVariable %_ptr_RayPayloadNV_v4float RayPayloadNV
     %pBlock = OpTypeStruct %v2float %v2float
%_ptr_RayPayloadNV_pBlock = OpTypePointer RayPayloadNV %pBlock
        %__0 = OpVariable %_ptr_RayPayloadNV_pBlock RayPayloadNV
      %block = OpTypeStruct %float
%_ptr_StorageBuffer_block = OpTypePointer StorageBuffer %block
        %__1 = OpVariable %_ptr_StorageBuffer_block StorageBuffer
       )";

  ss << declarations;

  ss << R"(
        %main = OpFunction %void None %3
          %5 = OpLabel 
         )";

  ss << body;

  ss << R"(
               OpReturn
               OpFunctionEnd)";
  return ss.str();
}

TEST_F(ValidateRayTracingReorderNV, HitObjectTraceRayNV) {
  const std::string declarations = R"(
       %uint = OpTypeInt 32 0
     %uint_1 = OpConstant %uint 1
    %v3float = OpTypeVector %float 3
  %float_0_5 = OpConstant %float 0.5
         %31 = OpConstantComposite %v3float %float_0_5 %float_0_5 %float_0_5
         %32 = OpConstantComposite %v3float %float_1 %float_1 %float_1
      %int_1 = OpConstant %int 1
  )";

  const std::string body = R"(
  OpStore %attr %11
  %26 = OpLoad %23 %as
  OpHitObjectTraceRayNV %hObj %26 %uint_1 %uint_1 %uint_1 %uint_1 %uint_1 %31 %float_0_5 %32 %float_1 %payload
  )";

  CompileSuccessfully(GenerateReorderShaderCode(body, declarations).c_str(),
                      SPV_ENV_VULKAN_1_2);
  EXPECT_EQ(SPV_SUCCESS, ValidateInstructions(SPV_ENV_VULKAN_1_2));
}

TEST_F(ValidateRayTracingReorderNV, HitObjectTraceRayMotionNV) {
  const std::string declarations = R"(
        %uint = OpTypeInt 32 0
      %uint_1 = OpConstant %uint 1
     %v3float = OpTypeVector %float 3
   %float_0_5 = OpConstant %float 0.5
          %31 = OpConstantComposite %v3float %float_0_5 %float_0_5 %float_0_5
          %32 = OpConstantComposite %v3float %float_1 %float_1 %float_1
    %float_10 = OpConstant %float 10
       %int_2 = OpConstant %int 2
  )";

  const std::string body = R"(
      OpStore %attr %11
      %26 = OpLoad %23 %as
      OpHitObjectTraceRayMotionNV %hObj %26 %uint_1 %uint_1 %uint_1 %uint_1 %uint_1 %31 %float_0_5 %32 %float_1 %float_10 %__0
  )";

  CompileSuccessfully(GenerateReorderShaderCode(body, declarations).c_str(),
                      SPV_ENV_VULKAN_1_2);
  EXPECT_EQ(SPV_SUCCESS, ValidateInstructions(SPV_ENV_VULKAN_1_2));
}

TEST_F(ValidateRayTracingReorderNV, HitObjectRecordHitNV) {
  const std::string declarations = R"(
        %int_1 = OpConstant %int 1
        %uint = OpTypeInt 32 0
      %uint_2 = OpConstant %uint 2
     %v3float = OpTypeVector %float 3
   %float_0_5 = OpConstant %float 0.5
          %31 = OpConstantComposite %v3float %float_0_5 %float_0_5 %float_0_5
          %32 = OpConstantComposite %v3float %float_1 %float_1 %float_1
    %float_10 = OpConstant %float 10
       %int_2 = OpConstant %int 2
  )";

  const std::string body = R"(
      OpStore %attr %11
      %26 = OpLoad %23 %as
      OpHitObjectRecordHitNV %hObj %26 %int_1 %int_1 %int_1 %uint_2 %uint_2 %uint_2 %31 %float_1 %32 %float_2 %attr
  )";

  CompileSuccessfully(GenerateReorderShaderCode(body, declarations).c_str(),
                      SPV_ENV_VULKAN_1_2);
  EXPECT_EQ(SPV_SUCCESS, ValidateInstructions(SPV_ENV_VULKAN_1_2));
}

TEST_F(ValidateRayTracingReorderNV, HitObjectRecordHitWithIndexNV) {
  const std::string declarations = R"(
        %int_1 = OpConstant %int 1
        %uint = OpTypeInt 32 0
      %uint_2 = OpConstant %uint 2
     %v3float = OpTypeVector %float 3
   %float_0_5 = OpConstant %float 0.5
          %31 = OpConstantComposite %v3float %float_0_5 %float_0_5 %float_0_5
          %32 = OpConstantComposite %v3float %float_1 %float_1 %float_1
    %float_10 = OpConstant %float 10
       %int_2 = OpConstant %int 2
  )";

  const std::string body = R"(
      OpStore %attr %11
      %26 = OpLoad %23 %as
      OpHitObjectRecordHitWithIndexNV %hObj %26 %int_1 %int_1 %int_1 %uint_2 %uint_2 %31 %float_1 %32 %float_2 %_
  )";

  CompileSuccessfully(GenerateReorderShaderCode(body, declarations).c_str(),
                      SPV_ENV_VULKAN_1_2);
  EXPECT_EQ(SPV_SUCCESS, ValidateInstructions(SPV_ENV_VULKAN_1_2));
}

TEST_F(ValidateRayTracingReorderNV, HitObjectRecordEmptyNV) {
  const std::string body = R"(
      OpHitObjectRecordEmptyNV %hObj
  )";

  CompileSuccessfully(GenerateReorderShaderCode(body).c_str(),
                      SPV_ENV_VULKAN_1_2);
  EXPECT_EQ(SPV_SUCCESS, ValidateInstructions(SPV_ENV_VULKAN_1_2));
}

TEST_F(ValidateRayTracingReorderNV, HitObjectRecordMissNV) {
  const std::string declarations = R"(
         %uint = OpTypeInt 32 0
       %uint_1 = OpConstant %uint 1
      %v3float = OpTypeVector %float 3
    %float_0_5 = OpConstant %float 0.5
           %29 = OpConstantComposite %v3float %float_0_5 %float_0_5 %float_0_5
    %float_1_5 = OpConstant %float 1.5
           %31 = OpConstantComposite %v3float %float_1_5 %float_1_5 %float_1_5
      %float_5 = OpConstant %float 5
  )";

  const std::string body = R"(
      OpHitObjectRecordMissNV %hObj %uint_1 %29 %float_2 %31 %float_5
  )";

  CompileSuccessfully(GenerateReorderShaderCode(body, declarations).c_str(),
                      SPV_ENV_VULKAN_1_2);
  EXPECT_EQ(SPV_SUCCESS, ValidateInstructions(SPV_ENV_VULKAN_1_2));
}

TEST_F(ValidateRayTracingReorderNV, HitObjectIsHitNV) {
  const std::string declarations = R"(
        %bool = OpTypeBool
        %_ptr_StorageBuffer_float = OpTypePointer StorageBuffer %float
  )";

  const std::string body = R"(
      %26 = OpHitObjectIsHitNV %bool %hObj
            OpSelectionMerge %28 None
            OpBranchConditional %26 %27 %28
      %27 = OpLabel
      %33 = OpAccessChain %_ptr_StorageBuffer_float %__1 %int_0
            OpStore %33 %float_1
            OpBranch %28
      %28 = OpLabel
  )";

  CompileSuccessfully(GenerateReorderShaderCode(body, declarations).c_str(),
                      SPV_ENV_VULKAN_1_2);
  EXPECT_EQ(SPV_SUCCESS, ValidateInstructions(SPV_ENV_VULKAN_1_2));
}

TEST_F(ValidateRayTracingReorderNV, HitObjectGetRayTMaxNV) {
  const std::string declarations = R"(
        %_ptr_Function_float = OpTypePointer Function %float
  )";

  const std::string body = R"(
      %tmin = OpVariable %_ptr_Function_float Function
      %12 = OpHitObjectGetRayTMaxNV %float %hObj
      OpStore %tmin %12
  )";

  CompileSuccessfully(GenerateReorderShaderCode(body, declarations).c_str(),
                      SPV_ENV_VULKAN_1_2);
  EXPECT_EQ(SPV_SUCCESS, ValidateInstructions(SPV_ENV_VULKAN_1_2));
}

TEST_F(ValidateRayTracingReorderNV, HitObjectGetRayTMinNV) {
  const std::string declarations = R"(
        %_ptr_Function_float = OpTypePointer Function %float
  )";

  const std::string body = R"(
      %tmin = OpVariable %_ptr_Function_float Function
      %12 = OpHitObjectGetRayTMinNV %float %hObj
      OpStore %tmin %12
  )";

  CompileSuccessfully(GenerateReorderShaderCode(body, declarations).c_str(),
                      SPV_ENV_VULKAN_1_2);
  EXPECT_EQ(SPV_SUCCESS, ValidateInstructions(SPV_ENV_VULKAN_1_2));
}

TEST_F(ValidateRayTracingReorderNV, HitObjectGetWorldRayOriginNV) {
  const std::string declarations = R"(
        %v3float = OpTypeVector %float 3
        %_ptr_Function_v3float = OpTypePointer Function %v3float
  )";

  const std::string body = R"(
      %orig = OpVariable %_ptr_Function_v3float Function
      %13 = OpHitObjectGetWorldRayOriginNV %v3float %hObj
      OpStore %orig %13
  )";

  CompileSuccessfully(GenerateReorderShaderCode(body, declarations).c_str(),
                      SPV_ENV_VULKAN_1_2);
  EXPECT_EQ(SPV_SUCCESS, ValidateInstructions(SPV_ENV_VULKAN_1_2));
}

TEST_F(ValidateRayTracingReorderNV, HitObjectGetObjectRayOriginNV) {
  const std::string declarations = R"(
        %v3float = OpTypeVector %float 3
        %_ptr_Function_v3float = OpTypePointer Function %v3float
  )";

  const std::string body = R"(
      %oorig = OpVariable %_ptr_Function_v3float Function
      %13 = OpHitObjectGetObjectRayOriginNV %v3float %hObj
      OpStore %oorig %13
  )";

  CompileSuccessfully(GenerateReorderShaderCode(body, declarations).c_str(),
                      SPV_ENV_VULKAN_1_2);
  EXPECT_EQ(SPV_SUCCESS, ValidateInstructions(SPV_ENV_VULKAN_1_2));
}

TEST_F(ValidateRayTracingReorderNV, HitObjectGetWorldRayDirectionNV) {
  const std::string declarations = R"(
        %v3float = OpTypeVector %float 3
        %_ptr_Function_v3float = OpTypePointer Function %v3float
  )";

  const std::string body = R"(
      %dir = OpVariable %_ptr_Function_v3float Function
      %13 = OpHitObjectGetWorldRayDirectionNV %v3float %hObj
      OpStore %dir %13
  )";

  CompileSuccessfully(GenerateReorderShaderCode(body, declarations).c_str(),
                      SPV_ENV_VULKAN_1_2);
  EXPECT_EQ(SPV_SUCCESS, ValidateInstructions(SPV_ENV_VULKAN_1_2));
}

TEST_F(ValidateRayTracingReorderNV, HitObjectGetObjectRayDirectionNV) {
  const std::string declarations = R"(
        %v3float = OpTypeVector %float 3
        %_ptr_Function_v3float = OpTypePointer Function %v3float
  )";

  const std::string body = R"(
      %odir = OpVariable %_ptr_Function_v3float Function
      %13 = OpHitObjectGetObjectRayDirectionNV %v3float %hObj
      OpStore %odir %13
  )";

  CompileSuccessfully(GenerateReorderShaderCode(body, declarations).c_str(),
                      SPV_ENV_VULKAN_1_2);
  EXPECT_EQ(SPV_SUCCESS, ValidateInstructions(SPV_ENV_VULKAN_1_2));
}

TEST_F(ValidateRayTracingReorderNV, HitObjectGetObjectToWorldNV) {
  const std::string declarations = R"(
        %v3float = OpTypeVector %float 3
        %mat4v3float = OpTypeMatrix %v3float 4
        %_ptr_Function_mat4v3float = OpTypePointer Function %mat4v3float
  )";

  const std::string body = R"(
      %otw = OpVariable %_ptr_Function_mat4v3float Function
      %14 = OpHitObjectGetObjectToWorldNV %mat4v3float %hObj
      OpStore %otw %14
  )";

  CompileSuccessfully(GenerateReorderShaderCode(body, declarations).c_str(),
                      SPV_ENV_VULKAN_1_2);
  EXPECT_EQ(SPV_SUCCESS, ValidateInstructions(SPV_ENV_VULKAN_1_2));
}

TEST_F(ValidateRayTracingReorderNV, HitObjectGetWorldToObjectNV) {
  const std::string declarations = R"(
        %v3float = OpTypeVector %float 3
        %mat4v3float = OpTypeMatrix %v3float 4
        %_ptr_Function_mat4v3float = OpTypePointer Function %mat4v3float
  )";

  const std::string body = R"(
      %wto = OpVariable %_ptr_Function_mat4v3float Function
      %14 = OpHitObjectGetWorldToObjectNV %mat4v3float %hObj
      OpStore %wto %14
  )";

  CompileSuccessfully(GenerateReorderShaderCode(body, declarations).c_str(),
                      SPV_ENV_VULKAN_1_2);
  EXPECT_EQ(SPV_SUCCESS, ValidateInstructions(SPV_ENV_VULKAN_1_2));
}

TEST_F(ValidateRayTracingReorderNV, HitObjectGetInstanceCustomIndexNV) {
  const std::string declarations = R"(
        %_ptr_Function_int = OpTypePointer Function %int
  )";

  const std::string body = R"(
      %id = OpVariable %_ptr_Function_int Function
      %12 = OpHitObjectGetInstanceCustomIndexNV %int %hObj
      OpStore %id %12
  )";

  CompileSuccessfully(GenerateReorderShaderCode(body, declarations).c_str(),
                      SPV_ENV_VULKAN_1_2);
  EXPECT_EQ(SPV_SUCCESS, ValidateInstructions(SPV_ENV_VULKAN_1_2));
}

TEST_F(ValidateRayTracingReorderNV, HitObjectGetInstanceIdNV) {
  const std::string declarations = R"(
        %_ptr_Function_int = OpTypePointer Function %int
  )";

  const std::string body = R"(
      %id = OpVariable %_ptr_Function_int Function
      %12 = OpHitObjectGetInstanceIdNV %int %hObj
      OpStore %id %12
  )";

  CompileSuccessfully(GenerateReorderShaderCode(body, declarations).c_str(),
                      SPV_ENV_VULKAN_1_2);
  EXPECT_EQ(SPV_SUCCESS, ValidateInstructions(SPV_ENV_VULKAN_1_2));
}

TEST_F(ValidateRayTracingReorderNV, HitObjectGetPrimitiveIndexNV) {
  const std::string declarations = R"(
        %_ptr_Function_int = OpTypePointer Function %int
  )";

  const std::string body = R"(
      %id = OpVariable %_ptr_Function_int Function
      %12 = OpHitObjectGetPrimitiveIndexNV %int %hObj
      OpStore %id %12
  )";

  CompileSuccessfully(GenerateReorderShaderCode(body, declarations).c_str(),
                      SPV_ENV_VULKAN_1_2);
  EXPECT_EQ(SPV_SUCCESS, ValidateInstructions(SPV_ENV_VULKAN_1_2));
}

TEST_F(ValidateRayTracingReorderNV, HitObjectGetGeometryIndexNV) {
  const std::string declarations = R"(
        %_ptr_Function_int = OpTypePointer Function %int
  )";

  const std::string body = R"(
      %id = OpVariable %_ptr_Function_int Function
      %12 = OpHitObjectGetGeometryIndexNV %int %hObj
      OpStore %id %12
  )";

  CompileSuccessfully(GenerateReorderShaderCode(body, declarations).c_str(),
                      SPV_ENV_VULKAN_1_2);
  EXPECT_EQ(SPV_SUCCESS, ValidateInstructions(SPV_ENV_VULKAN_1_2));
}

TEST_F(ValidateRayTracingReorderNV, HitObjectGetHitKindNV) {
  const std::string declarations = R"(
    %uint = OpTypeInt 32 0
    %_ptr_Function_uint = OpTypePointer Function %uint
  )";

  const std::string body = R"(
    %uid = OpVariable %_ptr_Function_uint Function
    %12 = OpHitObjectGetHitKindNV %uint %hObj
    OpStore %uid %12
  )";

  CompileSuccessfully(GenerateReorderShaderCode(body, declarations).c_str(),
                      SPV_ENV_VULKAN_1_2);
  EXPECT_EQ(SPV_SUCCESS, ValidateInstructions(SPV_ENV_VULKAN_1_2));
}

TEST_F(ValidateRayTracingReorderNV, HitObjectGetAttributesNV) {
  const std::string body = R"(
    OpHitObjectGetAttributesNV %hObj %attr
  )";

  CompileSuccessfully(GenerateReorderShaderCode(body).c_str(),
                      SPV_ENV_VULKAN_1_2);
  EXPECT_EQ(SPV_SUCCESS, ValidateInstructions(SPV_ENV_VULKAN_1_2));
}

TEST_F(ValidateRayTracingReorderNV, HitObjectGetShaderRecordBufferHandleNV) {
  const std::string declarations = R"(
           %uint = OpTypeInt 32 0
         %v2uint = OpTypeVector %uint 2
    %_ptr_Function_v2uint = OpTypePointer Function %v2uint
  )";

  const std::string body = R"(
    %handle = OpVariable %_ptr_Function_v2uint Function
        %13 = OpHitObjectGetShaderRecordBufferHandleNV %v2uint %hObj
              OpStore %handle %13
  )";

  CompileSuccessfully(GenerateReorderShaderCode(body, declarations).c_str(),
                      SPV_ENV_VULKAN_1_2);
  EXPECT_EQ(SPV_SUCCESS, ValidateInstructions(SPV_ENV_VULKAN_1_2));
}

TEST_F(ValidateRayTracingReorderNV,
       HitObjectGetShaderBindingTableRecordIndexNV) {
  const std::string declarations = R"(
    %uint = OpTypeInt 32 0
    %_ptr_Function_uint = OpTypePointer Function %uint
  )";

  const std::string body = R"(
    %rid = OpVariable %_ptr_Function_uint Function
     %12 = OpHitObjectGetShaderBindingTableRecordIndexNV %uint %hObj
           OpStore %rid %12
  )";

  CompileSuccessfully(GenerateReorderShaderCode(body, declarations).c_str(),
                      SPV_ENV_VULKAN_1_2);
  EXPECT_EQ(SPV_SUCCESS, ValidateInstructions(SPV_ENV_VULKAN_1_2));
}

TEST_F(ValidateRayTracingReorderNV, ClusterASNV) {
  const std::string cap = R"(
               OpCapability RayTracingClusterAccelerationStructureNV
                            )";

  const std::string ext = R"(
               OpExtension "SPV_NV_cluster_acceleration_structure"
                           )";

  const std::string declarations = R"(
        %int = OpTypeInt 32 1
        %_ptr_Function_int = OpTypePointer Function %int
  )";

  const std::string body = R"(
      %id = OpVariable %_ptr_Function_int Function
      %12 = OpHitObjectGetClusterIdNV %int %hObj
      OpStore %id %12
  )";

  CompileSuccessfully(
      GenerateReorderThreadCode(body, declarations, ext, cap).c_str(),
      SPV_ENV_VULKAN_1_2);
  EXPECT_EQ(SPV_SUCCESS, ValidateInstructions(SPV_ENV_VULKAN_1_2));
}

TEST_F(ValidateRayTracingReorderNV, LSSGetSpherePositionNV) {
  const std::string cap = R"(
               OpCapability RayTracingSpheresGeometryNV
                            )";

  const std::string ext = R"(
               OpExtension "SPV_NV_linear_swept_spheres"
                           )";

  const std::string declarations = R"(
        %float = OpTypeFloat 32
        %v3float = OpTypeVector %float 3
        %_ptr_Function_v3float = OpTypePointer Function %v3float
  )";

  const std::string body = R"(
      %pos = OpVariable %_ptr_Function_v3float Function
      %result = OpHitObjectGetSpherePositionNV %v3float %hObj
      OpStore %pos %result
  )";

  CompileSuccessfully(
      GenerateReorderThreadCode(body, declarations, ext, cap).c_str(),
      SPV_ENV_VULKAN_1_2);
  EXPECT_EQ(SPV_SUCCESS, ValidateInstructions(SPV_ENV_VULKAN_1_2));
}

TEST_F(ValidateRayTracingReorderNV, LSSGetLSSPositionsNV) {
  const std::string cap = R"(
               OpCapability RayTracingSpheresGeometryNV
               OpCapability RayTracingLinearSweptSpheresGeometryNV
                            )";

  const std::string ext = R"(
               OpExtension "SPV_NV_linear_swept_spheres"
                           )";

  const std::string declarations = R"(
        %float = OpTypeFloat 32
        %uint = OpTypeInt 32 0
        %v3float = OpTypeVector %float 3
        %uint_2 = OpConstant %uint 2
        %arr = OpTypeArray %v3float %uint_2
        %_ptr_Function_v3float = OpTypePointer Function %arr
  )";

  const std::string body = R"(
      %lsspos = OpVariable %_ptr_Function_v3float Function
      %result = OpHitObjectGetLSSPositionsNV %arr %hObj
      OpStore %lsspos %result
  )";

  CompileSuccessfully(
      GenerateReorderThreadCode(body, declarations, ext, cap).c_str(),
      SPV_ENV_VULKAN_1_2);
  EXPECT_EQ(SPV_SUCCESS, ValidateInstructions(SPV_ENV_VULKAN_1_2));
}

TEST_F(ValidateRayTracingReorderNV, LSSGetSphereRadiusNV) {
  const std::string cap = R"(
               OpCapability RayTracingSpheresGeometryNV
                            )";

  const std::string ext = R"(
               OpExtension "SPV_NV_linear_swept_spheres"
                           )";

  const std::string declarations = R"(
        %float = OpTypeFloat 32
        %_ptr_Function_float = OpTypePointer Function %float
  )";

  const std::string body = R"(
      %rad = OpVariable %_ptr_Function_float Function
      %result = OpHitObjectGetSphereRadiusNV %float %hObj
      OpStore %rad %result
  )";

  CompileSuccessfully(
      GenerateReorderThreadCode(body, declarations, ext, cap).c_str(),
      SPV_ENV_VULKAN_1_2);
  EXPECT_EQ(SPV_SUCCESS, ValidateInstructions(SPV_ENV_VULKAN_1_2));
}

TEST_F(ValidateRayTracingReorderNV, LSSGetLSSRadiiNV) {
  const std::string cap = R"(
               OpCapability RayTracingLinearSweptSpheresGeometryNV
                            )";

  const std::string ext = R"(
               OpExtension "SPV_NV_linear_swept_spheres"
                           )";

  const std::string declarations = R"(
        %float = OpTypeFloat 32
        %uint = OpTypeInt 32 0
        %uint_2 = OpConstant %uint 2
        %arr = OpTypeArray %float %uint_2
        %_ptr_Function_float = OpTypePointer Function %arr
  )";

  const std::string body = R"(
      %rad = OpVariable %_ptr_Function_float Function
      %result = OpHitObjectGetLSSRadiiNV %arr %hObj
      OpStore %rad %result
  )";

  CompileSuccessfully(
      GenerateReorderThreadCode(body, declarations, ext, cap).c_str(),
      SPV_ENV_VULKAN_1_2);
  EXPECT_EQ(SPV_SUCCESS, ValidateInstructions(SPV_ENV_VULKAN_1_2));
}

TEST_F(ValidateRayTracingReorderNV, LSSIsSphereHitNV) {
  const std::string cap = R"(
               OpCapability RayTracingSpheresGeometryNV
                            )";

  const std::string ext = R"(
               OpExtension "SPV_NV_linear_swept_spheres"
                           )";

  const std::string declarations = R"(
        %bool = OpTypeBool
        %_ptr_Function_bool = OpTypePointer Function %bool
  )";

  const std::string body = R"(
      %ishit = OpVariable %_ptr_Function_bool Function
      %result = OpHitObjectIsSphereHitNV %bool %hObj
      OpStore %ishit %result
  )";

  CompileSuccessfully(
      GenerateReorderThreadCode(body, declarations, ext, cap).c_str(),
      SPV_ENV_VULKAN_1_2);
  EXPECT_EQ(SPV_SUCCESS, ValidateInstructions(SPV_ENV_VULKAN_1_2));
}

TEST_F(ValidateRayTracingReorderNV, LSSIsLSSHitNV) {
  const std::string cap = R"(
               OpCapability RayTracingLinearSweptSpheresGeometryNV
                            )";

  const std::string ext = R"(
               OpExtension "SPV_NV_linear_swept_spheres"
                           )";

  const std::string declarations = R"(
        %bool = OpTypeBool
        %_ptr_Function_bool = OpTypePointer Function %bool
  )";

  const std::string body = R"(
      %ishit = OpVariable %_ptr_Function_bool Function
      %result = OpHitObjectIsLSSHitNV %bool %hObj
      OpStore %ishit %result
  )";

  CompileSuccessfully(
      GenerateReorderThreadCode(body, declarations, ext, cap).c_str(),
      SPV_ENV_VULKAN_1_2);
  EXPECT_EQ(SPV_SUCCESS, ValidateInstructions(SPV_ENV_VULKAN_1_2));
}

// EXT Extension Tests
using ValidateRayTracingReorderEXT = spvtest::ValidateBase<bool>;

std::string GenerateReorderThreadCodeEXT(const std::string& body = "",
                                         const std::string& declarations = "",
                                         const std::string& extensions = "",
                                         const std::string& capabilities = "") {
  std::ostringstream ss;
  ss << R"(
            OpCapability RayTracingKHR
            OpCapability ShaderInvocationReorderEXT
         )";
  ss << capabilities;
  ss << R"(
            OpExtension "SPV_KHR_ray_tracing"
            OpExtension "SPV_EXT_shader_invocation_reorder"
          )";
  ss << extensions;
  ss << R"(
       %1 = OpExtInstImport "GLSL.std.450"
            OpMemoryModel Logical GLSL450
            OpEntryPoint RayGenerationNV %main "main" %hObj
            OpSourceExtension "GL_EXT_ray_tracing"
            OpSourceExtension "GL_EXT_shader_invocation_reorder"
            OpName %main "main"
    %void = OpTypeVoid
       %3 = OpTypeFunction %void
       %6 = OpTypeHitObjectEXT
%_ptr_Private_6 = OpTypePointer Private %6
    %hObj = OpVariable %_ptr_Private_6 Private
    )";
  ss << declarations;

  ss << R"(
   %main  = OpFunction %void None %3
     %5   = OpLabel 
    )";

  ss << body;

  ss << R"(
            OpReturn
            OpFunctionEnd
    )";
  return ss.str();
}

std::string GenerateReorderShaderCodeEXT(const std::string& body = "",
                                         const std::string& declarations = "",
                                         const std::string& extensions = "",
                                         const std::string& capabilties = "") {
  std::ostringstream ss;
  ss << R"(
               OpCapability RayTracingKHR
               OpCapability ShaderInvocationReorderEXT
         )";
  ss << capabilties;
  ss << R"(    OpExtension "SPV_KHR_ray_tracing"
               OpExtension "SPV_EXT_shader_invocation_reorder"
          )";
  ss << extensions;
  ss << R"(
          %1 = OpExtInstImport "GLSL.std.450"
               OpMemoryModel Logical GLSL450
               OpEntryPoint RayGenerationKHR %main "main" %attr %_ %hObj %payload %__0 %as %__1
               OpSource GLSL 460
               OpSourceExtension "GL_EXT_ray_tracing"
               OpSourceExtension "GL_EXT_shader_invocation_reorder"
               OpName %main "main"
               OpName %attr "attr"
               OpName %hBlock "hBlock"
               OpMemberName %hBlock 0 "attrval"
               OpName %_ ""
               OpName %hObj "hObj"
               OpName %payload "payload"
               OpName %pBlock "pBlock"
               OpMemberName %pBlock 0 "val1"
               OpMemberName %pBlock 1 "val2"
               OpName %__0 ""
               OpName %as "as"
               OpName %block "block"
               OpMemberName %block 0 "op"
               OpName %__1 ""
               OpDecorate %hBlock Block
               OpDecorate %pBlock Block
               OpDecorate %as DescriptorSet 0
               OpDecorate %as Binding 0
               OpMemberDecorate %block 0 Offset 0
               OpDecorate %block Block
               OpDecorate %__1 DescriptorSet 0
               OpDecorate %__1 Binding 1
       %void = OpTypeVoid
          %3 = OpTypeFunction %void
      %float = OpTypeFloat 32
    %v2float = OpTypeVector %float 2
%_ptr_HitObjectAttributeEXT_v2float = OpTypePointer HitObjectAttributeEXT %v2float
       %attr = OpVariable %_ptr_HitObjectAttributeEXT_v2float HitObjectAttributeEXT
    %float_1 = OpConstant %float 1
         %11 = OpConstantComposite %v2float %float_1 %float_1
     %hBlock = OpTypeStruct %float
%_ptr_HitObjectAttributeEXT_hBlock = OpTypePointer HitObjectAttributeEXT %hBlock
          %_ = OpVariable %_ptr_HitObjectAttributeEXT_hBlock HitObjectAttributeEXT
        %int = OpTypeInt 32 1
      %int_0 = OpConstant %int 0
    %float_2 = OpConstant %float 2
%_ptr_HitObjectAttributeEXT_float = OpTypePointer HitObjectAttributeEXT %float
             %20 = OpTypeHitObjectEXT
    %_ptr_Private_20 = OpTypePointer Private %20
           %hObj = OpVariable %_ptr_Private_20 Private
             %23 = OpTypeAccelerationStructureKHR
    %_ptr_UniformConstant_23 = OpTypePointer UniformConstant %23
             %as = OpVariable %_ptr_UniformConstant_23 UniformConstant
    %v4float = OpTypeVector %float 4
%_ptr_RayPayloadKHR_v4float = OpTypePointer RayPayloadKHR %v4float
    %payload = OpVariable %_ptr_RayPayloadKHR_v4float RayPayloadKHR
     %pBlock = OpTypeStruct %v2float %v2float
%_ptr_RayPayloadKHR_pBlock = OpTypePointer RayPayloadKHR %pBlock
        %__0 = OpVariable %_ptr_RayPayloadKHR_pBlock RayPayloadKHR
      %block = OpTypeStruct %float
%_ptr_StorageBuffer_block = OpTypePointer StorageBuffer %block
        %__1 = OpVariable %_ptr_StorageBuffer_block StorageBuffer
       )";

  ss << declarations;

  ss << R"(
        %main = OpFunction %void None %3
          %5 = OpLabel 
         )";

  ss << body;

  ss << R"(
               OpReturn
               OpFunctionEnd)";
  return ss.str();
}

TEST_F(ValidateRayTracingReorderEXT, ReorderThreadWithHintEXT) {
  const std::string declarations = R"(
             %uint = OpTypeInt 32 0
           %uint_4 = OpConstant %uint 4
  )";

  const std::string body = R"(
    OpReorderThreadWithHintEXT %uint_4 %uint_4
  )";

  CompileSuccessfully(GenerateReorderThreadCodeEXT(body, declarations).c_str(),
                      SPV_ENV_VULKAN_1_2);
  EXPECT_EQ(SPV_SUCCESS, ValidateInstructions(SPV_ENV_VULKAN_1_2));
}

TEST_F(ValidateRayTracingReorderEXT, ReorderThreadWithHitObjectEXT) {
  const std::string declarations = R"(
      %uint = OpTypeInt 32 0
    %uint_4 = OpConstant %uint 4
    %uint_2 = OpConstant %uint 2
  )";

  const std::string body = R"(
             OpReorderThreadWithHitObjectEXT %hObj
             OpReorderThreadWithHitObjectEXT %hObj %uint_4 %uint_2
  )";

  CompileSuccessfully(GenerateReorderThreadCodeEXT(body, declarations).c_str(),
                      SPV_ENV_VULKAN_1_2);
  EXPECT_EQ(SPV_SUCCESS, ValidateInstructions(SPV_ENV_VULKAN_1_2));
}

TEST_F(ValidateRayTracingReorderEXT, HitObjectTraceRayEXT) {
  const std::string declarations = R"(
       %uint = OpTypeInt 32 0
     %uint_1 = OpConstant %uint 1
    %v3float = OpTypeVector %float 3
  %float_0_5 = OpConstant %float 0.5
         %31 = OpConstantComposite %v3float %float_0_5 %float_0_5 %float_0_5
         %32 = OpConstantComposite %v3float %float_1 %float_1 %float_1
      %int_1 = OpConstant %int 1
  )";

  const std::string body = R"(
  OpStore %attr %11
  %26 = OpLoad %23 %as
  OpHitObjectTraceRayEXT %hObj %26 %uint_1 %uint_1 %uint_1 %uint_1 %uint_1 %31 %float_0_5 %32 %float_1 %payload
  )";

  CompileSuccessfully(GenerateReorderShaderCodeEXT(body, declarations).c_str(),
                      SPV_ENV_VULKAN_1_2);
  EXPECT_EQ(SPV_SUCCESS, ValidateInstructions(SPV_ENV_VULKAN_1_2));
}

TEST_F(ValidateRayTracingReorderEXT, HitObjectTraceRayMotionEXT) {
  const std::string declarations = R"(
        %uint = OpTypeInt 32 0
      %uint_1 = OpConstant %uint 1
     %v3float = OpTypeVector %float 3
   %float_0_5 = OpConstant %float 0.5
          %31 = OpConstantComposite %v3float %float_0_5 %float_0_5 %float_0_5
          %32 = OpConstantComposite %v3float %float_1 %float_1 %float_1
    %float_10 = OpConstant %float 10
       %int_2 = OpConstant %int 2
  )";

  const std::string body = R"(
      OpStore %attr %11
      %26 = OpLoad %23 %as
      OpHitObjectTraceRayMotionEXT %hObj %26 %uint_1 %uint_1 %uint_1 %uint_1 %uint_1 %31 %float_0_5 %32 %float_1 %float_10 %__0
  )";

  CompileSuccessfully(GenerateReorderShaderCodeEXT(body, declarations).c_str(),
                      SPV_ENV_VULKAN_1_2);
  EXPECT_EQ(SPV_SUCCESS, ValidateInstructions(SPV_ENV_VULKAN_1_2));
}

TEST_F(ValidateRayTracingReorderEXT, HitObjectRecordEmptyEXT) {
  const std::string body = R"(
      OpHitObjectRecordEmptyEXT %hObj
  )";

  CompileSuccessfully(GenerateReorderShaderCodeEXT(body).c_str(),
                      SPV_ENV_VULKAN_1_2);
  EXPECT_EQ(SPV_SUCCESS, ValidateInstructions(SPV_ENV_VULKAN_1_2));
}

TEST_F(ValidateRayTracingReorderEXT, HitObjectRecordMissEXT) {
  const std::string declarations = R"(
         %uint = OpTypeInt 32 0
       %uint_1 = OpConstant %uint 1
      %v3float = OpTypeVector %float 3
    %float_0_5 = OpConstant %float 0.5
           %29 = OpConstantComposite %v3float %float_0_5 %float_0_5 %float_0_5
    %float_1_5 = OpConstant %float 1.5
           %31 = OpConstantComposite %v3float %float_1_5 %float_1_5 %float_1_5
      %float_5 = OpConstant %float 5
  )";

  const std::string body = R"(
      OpHitObjectRecordMissEXT %hObj %uint_1 %uint_1 %29 %float_2 %31 %float_5
  )";

  CompileSuccessfully(GenerateReorderShaderCodeEXT(body, declarations).c_str(),
                      SPV_ENV_VULKAN_1_2);
  EXPECT_EQ(SPV_SUCCESS, ValidateInstructions(SPV_ENV_VULKAN_1_2));
}

TEST_F(ValidateRayTracingReorderEXT, HitObjectIsHitEXT) {
  const std::string declarations = R"(
        %bool = OpTypeBool
        %_ptr_StorageBuffer_float = OpTypePointer StorageBuffer %float
  )";

  const std::string body = R"(
      %26 = OpHitObjectIsHitEXT %bool %hObj
            OpSelectionMerge %28 None
            OpBranchConditional %26 %27 %28
      %27 = OpLabel
      %33 = OpAccessChain %_ptr_StorageBuffer_float %__1 %int_0
            OpStore %33 %float_1
            OpBranch %28
      %28 = OpLabel
  )";

  CompileSuccessfully(GenerateReorderShaderCodeEXT(body, declarations).c_str(),
                      SPV_ENV_VULKAN_1_2);
  EXPECT_EQ(SPV_SUCCESS, ValidateInstructions(SPV_ENV_VULKAN_1_2));
}

TEST_F(ValidateRayTracingReorderEXT, HitObjectIsMissEXT) {
  const std::string declarations = R"(
        %bool = OpTypeBool
        %_ptr_StorageBuffer_float = OpTypePointer StorageBuffer %float
  )";

  const std::string body = R"(
      %26 = OpHitObjectIsMissEXT %bool %hObj
            OpSelectionMerge %28 None
            OpBranchConditional %26 %27 %28
      %27 = OpLabel
      %33 = OpAccessChain %_ptr_StorageBuffer_float %__1 %int_0
            OpStore %33 %float_1
            OpBranch %28
      %28 = OpLabel
  )";

  CompileSuccessfully(GenerateReorderShaderCodeEXT(body, declarations).c_str(),
                      SPV_ENV_VULKAN_1_2);
  EXPECT_EQ(SPV_SUCCESS, ValidateInstructions(SPV_ENV_VULKAN_1_2));
}

TEST_F(ValidateRayTracingReorderEXT, HitObjectIsEmptyEXT) {
  const std::string declarations = R"(
        %bool = OpTypeBool
        %_ptr_StorageBuffer_float = OpTypePointer StorageBuffer %float
  )";

  const std::string body = R"(
      %26 = OpHitObjectIsEmptyEXT %bool %hObj
            OpSelectionMerge %28 None
            OpBranchConditional %26 %27 %28
      %27 = OpLabel
      %33 = OpAccessChain %_ptr_StorageBuffer_float %__1 %int_0
            OpStore %33 %float_1
            OpBranch %28
      %28 = OpLabel
  )";

  CompileSuccessfully(GenerateReorderShaderCodeEXT(body, declarations).c_str(),
                      SPV_ENV_VULKAN_1_2);
  EXPECT_EQ(SPV_SUCCESS, ValidateInstructions(SPV_ENV_VULKAN_1_2));
}

TEST_F(ValidateRayTracingReorderEXT, HitObjectGetGeometryIndexEXT) {
  const std::string declarations = R"(
        %_ptr_Function_int = OpTypePointer Function %int
  )";

  const std::string body = R"(
      %id = OpVariable %_ptr_Function_int Function
      %12 = OpHitObjectGetGeometryIndexEXT %int %hObj
      OpStore %id %12
  )";

  CompileSuccessfully(GenerateReorderShaderCodeEXT(body, declarations).c_str(),
                      SPV_ENV_VULKAN_1_2);
  EXPECT_EQ(SPV_SUCCESS, ValidateInstructions(SPV_ENV_VULKAN_1_2));
}

TEST_F(ValidateRayTracingReorderEXT, HitObjectGetPrimitiveIndexEXT) {
  const std::string declarations = R"(
        %_ptr_Function_int = OpTypePointer Function %int
  )";

  const std::string body = R"(
      %id = OpVariable %_ptr_Function_int Function
      %12 = OpHitObjectGetPrimitiveIndexEXT %int %hObj
      OpStore %id %12
  )";

  CompileSuccessfully(GenerateReorderShaderCodeEXT(body, declarations).c_str(),
                      SPV_ENV_VULKAN_1_2);
  EXPECT_EQ(SPV_SUCCESS, ValidateInstructions(SPV_ENV_VULKAN_1_2));
}

TEST_F(ValidateRayTracingReorderEXT, HitObjectGetInstanceIdEXT) {
  const std::string declarations = R"(
        %_ptr_Function_int = OpTypePointer Function %int
  )";

  const std::string body = R"(
      %id = OpVariable %_ptr_Function_int Function
      %12 = OpHitObjectGetInstanceIdEXT %int %hObj
      OpStore %id %12
  )";

  CompileSuccessfully(GenerateReorderShaderCodeEXT(body, declarations).c_str(),
                      SPV_ENV_VULKAN_1_2);
  EXPECT_EQ(SPV_SUCCESS, ValidateInstructions(SPV_ENV_VULKAN_1_2));
}

TEST_F(ValidateRayTracingReorderEXT, HitObjectGetInstanceCustomIndexEXT) {
  const std::string declarations = R"(
        %_ptr_Function_int = OpTypePointer Function %int
  )";

  const std::string body = R"(
      %id = OpVariable %_ptr_Function_int Function
      %12 = OpHitObjectGetInstanceCustomIndexEXT %int %hObj
      OpStore %id %12
  )";

  CompileSuccessfully(GenerateReorderShaderCodeEXT(body, declarations).c_str(),
                      SPV_ENV_VULKAN_1_2);
  EXPECT_EQ(SPV_SUCCESS, ValidateInstructions(SPV_ENV_VULKAN_1_2));
}

TEST_F(ValidateRayTracingReorderEXT, HitObjectGetHitKindEXT) {
  const std::string declarations = R"(
    %uint = OpTypeInt 32 0
    %_ptr_Function_uint = OpTypePointer Function %uint
  )";

  const std::string body = R"(
    %uid = OpVariable %_ptr_Function_uint Function
    %12 = OpHitObjectGetHitKindEXT %uint %hObj
    OpStore %uid %12
  )";

  CompileSuccessfully(GenerateReorderShaderCodeEXT(body, declarations).c_str(),
                      SPV_ENV_VULKAN_1_2);
  EXPECT_EQ(SPV_SUCCESS, ValidateInstructions(SPV_ENV_VULKAN_1_2));
}

TEST_F(ValidateRayTracingReorderEXT, HitObjectGetCurrentTimeEXT) {
  const std::string declarations = R"(
        %_ptr_Function_float = OpTypePointer Function %float
  )";

  const std::string body = R"(
      %time = OpVariable %_ptr_Function_float Function
      %12 = OpHitObjectGetCurrentTimeEXT %float %hObj
      OpStore %time %12
  )";

  CompileSuccessfully(GenerateReorderShaderCodeEXT(body, declarations).c_str(),
                      SPV_ENV_VULKAN_1_2);
  EXPECT_EQ(SPV_SUCCESS, ValidateInstructions(SPV_ENV_VULKAN_1_2));
}

TEST_F(ValidateRayTracingReorderEXT, HitObjectGetObjectRayOriginEXT) {
  const std::string declarations = R"(
        %v3float = OpTypeVector %float 3
        %_ptr_Function_v3float = OpTypePointer Function %v3float
  )";

  const std::string body = R"(
      %oorig = OpVariable %_ptr_Function_v3float Function
      %13 = OpHitObjectGetObjectRayOriginEXT %v3float %hObj
      OpStore %oorig %13
  )";

  CompileSuccessfully(GenerateReorderShaderCodeEXT(body, declarations).c_str(),
                      SPV_ENV_VULKAN_1_2);
  EXPECT_EQ(SPV_SUCCESS, ValidateInstructions(SPV_ENV_VULKAN_1_2));
}

TEST_F(ValidateRayTracingReorderEXT, HitObjectGetObjectRayDirectionEXT) {
  const std::string declarations = R"(
        %v3float = OpTypeVector %float 3
        %_ptr_Function_v3float = OpTypePointer Function %v3float
  )";

  const std::string body = R"(
      %odir = OpVariable %_ptr_Function_v3float Function
      %13 = OpHitObjectGetObjectRayDirectionEXT %v3float %hObj
      OpStore %odir %13
  )";

  CompileSuccessfully(GenerateReorderShaderCodeEXT(body, declarations).c_str(),
                      SPV_ENV_VULKAN_1_2);
  EXPECT_EQ(SPV_SUCCESS, ValidateInstructions(SPV_ENV_VULKAN_1_2));
}

TEST_F(ValidateRayTracingReorderEXT, HitObjectGetRayTMaxEXT) {
  const std::string declarations = R"(
        %_ptr_Function_float = OpTypePointer Function %float
  )";

  const std::string body = R"(
      %tmax = OpVariable %_ptr_Function_float Function
      %12 = OpHitObjectGetRayTMaxEXT %float %hObj
      OpStore %tmax %12
  )";

  CompileSuccessfully(GenerateReorderShaderCodeEXT(body, declarations).c_str(),
                      SPV_ENV_VULKAN_1_2);
  EXPECT_EQ(SPV_SUCCESS, ValidateInstructions(SPV_ENV_VULKAN_1_2));
}

TEST_F(ValidateRayTracingReorderEXT, HitObjectGetRayTMinEXT) {
  const std::string declarations = R"(
        %_ptr_Function_float = OpTypePointer Function %float
  )";

  const std::string body = R"(
      %tmin = OpVariable %_ptr_Function_float Function
      %12 = OpHitObjectGetRayTMinEXT %float %hObj
      OpStore %tmin %12
  )";

  CompileSuccessfully(GenerateReorderShaderCodeEXT(body, declarations).c_str(),
                      SPV_ENV_VULKAN_1_2);
  EXPECT_EQ(SPV_SUCCESS, ValidateInstructions(SPV_ENV_VULKAN_1_2));
}

TEST_F(ValidateRayTracingReorderEXT, HitObjectGetRayFlagsEXT) {
  const std::string declarations = R"(
        %_ptr_Function_int = OpTypePointer Function %int
  )";

  const std::string body = R"(
      %flags = OpVariable %_ptr_Function_int Function
      %12 = OpHitObjectGetRayFlagsEXT %int %hObj
      OpStore %flags %12
  )";

  CompileSuccessfully(GenerateReorderShaderCodeEXT(body, declarations).c_str(),
                      SPV_ENV_VULKAN_1_2);
  EXPECT_EQ(SPV_SUCCESS, ValidateInstructions(SPV_ENV_VULKAN_1_2));
}

TEST_F(ValidateRayTracingReorderEXT, HitObjectGetWorldRayOriginEXT) {
  const std::string declarations = R"(
        %v3float = OpTypeVector %float 3
        %_ptr_Function_v3float = OpTypePointer Function %v3float
  )";

  const std::string body = R"(
      %orig = OpVariable %_ptr_Function_v3float Function
      %13 = OpHitObjectGetWorldRayOriginEXT %v3float %hObj
      OpStore %orig %13
  )";

  CompileSuccessfully(GenerateReorderShaderCodeEXT(body, declarations).c_str(),
                      SPV_ENV_VULKAN_1_2);
  EXPECT_EQ(SPV_SUCCESS, ValidateInstructions(SPV_ENV_VULKAN_1_2));
}

TEST_F(ValidateRayTracingReorderEXT, HitObjectGetWorldRayDirectionEXT) {
  const std::string declarations = R"(
        %v3float = OpTypeVector %float 3
        %_ptr_Function_v3float = OpTypePointer Function %v3float
  )";

  const std::string body = R"(
      %dir = OpVariable %_ptr_Function_v3float Function
      %13 = OpHitObjectGetWorldRayDirectionEXT %v3float %hObj
      OpStore %dir %13
  )";

  CompileSuccessfully(GenerateReorderShaderCodeEXT(body, declarations).c_str(),
                      SPV_ENV_VULKAN_1_2);
  EXPECT_EQ(SPV_SUCCESS, ValidateInstructions(SPV_ENV_VULKAN_1_2));
}

TEST_F(ValidateRayTracingReorderEXT, HitObjectGetObjectToWorldEXT) {
  const std::string declarations = R"(
        %v3float = OpTypeVector %float 3
        %mat4v3float = OpTypeMatrix %v3float 4
        %_ptr_Function_mat4v3float = OpTypePointer Function %mat4v3float
  )";

  const std::string body = R"(
      %otw = OpVariable %_ptr_Function_mat4v3float Function
      %14 = OpHitObjectGetObjectToWorldEXT %mat4v3float %hObj
      OpStore %otw %14
  )";

  CompileSuccessfully(GenerateReorderShaderCodeEXT(body, declarations).c_str(),
                      SPV_ENV_VULKAN_1_2);
  EXPECT_EQ(SPV_SUCCESS, ValidateInstructions(SPV_ENV_VULKAN_1_2));
}

TEST_F(ValidateRayTracingReorderEXT, HitObjectGetWorldToObjectEXT) {
  const std::string declarations = R"(
        %v3float = OpTypeVector %float 3
        %mat4v3float = OpTypeMatrix %v3float 4
        %_ptr_Function_mat4v3float = OpTypePointer Function %mat4v3float
  )";

  const std::string body = R"(
      %wto = OpVariable %_ptr_Function_mat4v3float Function
      %14 = OpHitObjectGetWorldToObjectEXT %mat4v3float %hObj
      OpStore %wto %14
  )";

  CompileSuccessfully(GenerateReorderShaderCodeEXT(body, declarations).c_str(),
                      SPV_ENV_VULKAN_1_2);
  EXPECT_EQ(SPV_SUCCESS, ValidateInstructions(SPV_ENV_VULKAN_1_2));
}

TEST_F(ValidateRayTracingReorderEXT, HitObjectGetShaderRecordBufferHandleEXT) {
  const std::string declarations = R"(
           %uint = OpTypeInt 32 0
         %v2uint = OpTypeVector %uint 2
    %_ptr_Function_v2uint = OpTypePointer Function %v2uint
  )";

  const std::string body = R"(
    %handle = OpVariable %_ptr_Function_v2uint Function
        %13 = OpHitObjectGetShaderRecordBufferHandleEXT %v2uint %hObj
              OpStore %handle %13
  )";

  CompileSuccessfully(GenerateReorderShaderCodeEXT(body, declarations).c_str(),
                      SPV_ENV_VULKAN_1_2);
  EXPECT_EQ(SPV_SUCCESS, ValidateInstructions(SPV_ENV_VULKAN_1_2));
}

TEST_F(ValidateRayTracingReorderEXT,
       HitObjectGetShaderBindingTableRecordIndexEXT) {
  const std::string declarations = R"(
    %uint = OpTypeInt 32 0
    %_ptr_Function_uint = OpTypePointer Function %uint
  )";

  const std::string body = R"(
    %rid = OpVariable %_ptr_Function_uint Function
     %12 = OpHitObjectGetShaderBindingTableRecordIndexEXT %uint %hObj
           OpStore %rid %12
  )";

  CompileSuccessfully(GenerateReorderShaderCodeEXT(body, declarations).c_str(),
                      SPV_ENV_VULKAN_1_2);
  EXPECT_EQ(SPV_SUCCESS, ValidateInstructions(SPV_ENV_VULKAN_1_2));
}

TEST_F(ValidateRayTracingReorderEXT,
       HitObjectSetShaderBindingTableRecordIndexEXT) {
  const std::string declarations = R"(
    %uint = OpTypeInt 32 0
    %uint_5 = OpConstant %uint 5
  )";

  const std::string body = R"(
    OpHitObjectSetShaderBindingTableRecordIndexEXT %hObj %uint_5
  )";

  CompileSuccessfully(GenerateReorderShaderCodeEXT(body, declarations).c_str(),
                      SPV_ENV_VULKAN_1_2);
  EXPECT_EQ(SPV_SUCCESS, ValidateInstructions(SPV_ENV_VULKAN_1_2));
}

TEST_F(ValidateRayTracingReorderEXT, HitObjectGetAttributesEXT) {
  const std::string body = R"(
    OpHitObjectGetAttributesEXT %hObj %attr
  )";

  CompileSuccessfully(GenerateReorderShaderCodeEXT(body).c_str(),
                      SPV_ENV_VULKAN_1_2);
  EXPECT_EQ(SPV_SUCCESS, ValidateInstructions(SPV_ENV_VULKAN_1_2));
}

TEST_F(ValidateRayTracingReorderEXT, HitObjectExecuteShaderEXT) {
  const std::string body = R"(
    OpHitObjectExecuteShaderEXT %hObj %payload
  )";

  CompileSuccessfully(GenerateReorderShaderCodeEXT(body).c_str(),
                      SPV_ENV_VULKAN_1_2);
  EXPECT_EQ(SPV_SUCCESS, ValidateInstructions(SPV_ENV_VULKAN_1_2));
}

TEST_F(ValidateRayTracingReorderEXT,
       HitObjectGetIntersectionTriangleVertexPositionsEXT) {
  const std::string declarations = R"(
        %v3float = OpTypeVector %float 3
        %uint = OpTypeInt 32 0
        %uint_3 = OpConstant %uint 3
        %arr_3_v3float = OpTypeArray %v3float %uint_3
        %_ptr_Function_arr_3_v3float = OpTypePointer Function %arr_3_v3float
  )";

  const std::string body = R"(
      %vertices = OpVariable %_ptr_Function_arr_3_v3float Function
      %result = OpHitObjectGetIntersectionTriangleVertexPositionsEXT %arr_3_v3float %hObj
      OpStore %vertices %result
  )";

  CompileSuccessfully(GenerateReorderShaderCodeEXT(body, declarations).c_str(),
                      SPV_ENV_VULKAN_1_2);
  EXPECT_EQ(SPV_SUCCESS, ValidateInstructions(SPV_ENV_VULKAN_1_2));
}

TEST_F(ValidateRayTracingReorderEXT, HitObjectRecordFromQueryEXT) {
  const std::string cap = R"(
               OpCapability RayQueryKHR
                            )";
  const std::string extensions = R"(
               OpExtension "SPV_KHR_ray_query"
               )";
  const std::string declarations = R"(
        %uint = OpTypeInt 32 0
        %uint_5 = OpConstant %uint 5
        %rayquery_type = OpTypeRayQueryKHR
        %_ptr_Function_rayquery = OpTypePointer Function %rayquery_type
  )";

  const std::string body = R"(
      %ray_query = OpVariable %_ptr_Function_rayquery Function
      OpHitObjectRecordFromQueryEXT %hObj %ray_query %uint_5 %attr
  )";

  CompileSuccessfully(
      GenerateReorderShaderCodeEXT(body, declarations, extensions, cap).c_str(),
      SPV_ENV_VULKAN_1_2);
  EXPECT_EQ(SPV_SUCCESS, ValidateInstructions(SPV_ENV_VULKAN_1_2));
}

TEST_F(ValidateRayTracingReorderEXT, HitObjectRecordMissMotionEXT) {
  const std::string declarations = R"(
         %uint = OpTypeInt 32 0
       %uint_1 = OpConstant %uint 1
      %v3float = OpTypeVector %float 3
    %float_0_5 = OpConstant %float 0.5
           %29 = OpConstantComposite %v3float %float_0_5 %float_0_5 %float_0_5
    %float_1_5 = OpConstant %float 1.5
           %31 = OpConstantComposite %v3float %float_1_5 %float_1_5 %float_1_5
      %float_5 = OpConstant %float 5
     %float_10 = OpConstant %float 10
  )";

  const std::string body = R"(
      OpHitObjectRecordMissMotionEXT %hObj %uint_1 %uint_1 %29 %float_2 %31 %float_5 %float_10
  )";

  CompileSuccessfully(GenerateReorderShaderCodeEXT(body, declarations).c_str(),
                      SPV_ENV_VULKAN_1_2);
  EXPECT_EQ(SPV_SUCCESS, ValidateInstructions(SPV_ENV_VULKAN_1_2));
}

// Fused Hit Object Instructions Tests

TEST_F(ValidateRayTracingReorderEXT, HitObjectReorderExecuteShaderEXT) {
  const std::string declarations = R"(
      %uint = OpTypeInt 32 0
    %uint_4 = OpConstant %uint 4
    %uint_2 = OpConstant %uint 2
  )";

  const std::string body = R"(
      OpHitObjectReorderExecuteShaderEXT %hObj %payload
      OpHitObjectReorderExecuteShaderEXT %hObj %payload %uint_4 %uint_2
  )";

  CompileSuccessfully(GenerateReorderShaderCodeEXT(body, declarations).c_str(),
                      SPV_ENV_VULKAN_1_2);
  EXPECT_EQ(SPV_SUCCESS, ValidateInstructions(SPV_ENV_VULKAN_1_2));
}

TEST_F(ValidateRayTracingReorderEXT, HitObjectTraceReorderExecuteEXT) {
  const std::string declarations = R"(
       %uint = OpTypeInt 32 0
     %uint_1 = OpConstant %uint 1
     %uint_4 = OpConstant %uint 4
     %uint_2 = OpConstant %uint 2
    %v3float = OpTypeVector %float 3
  %float_0_5 = OpConstant %float 0.5
         %31 = OpConstantComposite %v3float %float_0_5 %float_0_5 %float_0_5
         %32 = OpConstantComposite %v3float %float_1 %float_1 %float_1
      %int_1 = OpConstant %int 1
  )";

  const std::string body = R"(
  OpStore %attr %11
  %26 = OpLoad %23 %as
  OpHitObjectTraceReorderExecuteEXT %hObj %26 %uint_1 %uint_1 %uint_1 %uint_1 %uint_1 %31 %float_0_5 %32 %float_1 %payload
  OpHitObjectTraceReorderExecuteEXT %hObj %26 %uint_1 %uint_1 %uint_1 %uint_1 %uint_1 %31 %float_0_5 %32 %float_1 %payload %uint_4 %uint_2
  )";

  CompileSuccessfully(GenerateReorderShaderCodeEXT(body, declarations).c_str(),
                      SPV_ENV_VULKAN_1_2);
  EXPECT_EQ(SPV_SUCCESS, ValidateInstructions(SPV_ENV_VULKAN_1_2));
}

TEST_F(ValidateRayTracingReorderEXT, HitObjectTraceMotionReorderExecuteEXT) {
  const std::string declarations = R"(
        %uint = OpTypeInt 32 0
      %uint_1 = OpConstant %uint 1
      %uint_4 = OpConstant %uint 4
      %uint_2 = OpConstant %uint 2
     %v3float = OpTypeVector %float 3
   %float_0_5 = OpConstant %float 0.5
          %31 = OpConstantComposite %v3float %float_0_5 %float_0_5 %float_0_5
          %32 = OpConstantComposite %v3float %float_1 %float_1 %float_1
    %float_10 = OpConstant %float 10
       %int_2 = OpConstant %int 2
  )";

  const std::string body = R"(
      OpStore %attr %11
      %26 = OpLoad %23 %as
      OpHitObjectTraceMotionReorderExecuteEXT %hObj %26 %uint_1 %uint_1 %uint_1 %uint_1 %uint_1 %31 %float_0_5 %32 %float_1 %float_10 %__0
      OpHitObjectTraceMotionReorderExecuteEXT %hObj %26 %uint_1 %uint_1 %uint_1 %uint_1 %uint_1 %31 %float_0_5 %32 %float_1 %float_10 %__0 %uint_4 %uint_2
  )";

  CompileSuccessfully(GenerateReorderShaderCodeEXT(body, declarations).c_str(),
                      SPV_ENV_VULKAN_1_2);
  EXPECT_EQ(SPV_SUCCESS, ValidateInstructions(SPV_ENV_VULKAN_1_2));
}

}  // namespace
}  // namespace val
}  // namespace spvtools
