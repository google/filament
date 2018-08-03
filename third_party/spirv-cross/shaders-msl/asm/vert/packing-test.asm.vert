; SPIR-V
; Version: 1.0
; Generator: Khronos Glslang Reference Front End; 3
; Bound: 18
; Schema: 0
               OpCapability Shader
          %1 = OpExtInstImport "GLSL.std.450"
               OpMemoryModel Logical GLSL450
               OpEntryPoint Vertex %main "main"
               OpSource HLSL 500
               OpName %main "main"
               OpName %TestStruct "TestStruct"
               OpMemberName %TestStruct 0 "transforms"
               OpName %CB0 "CB0"
               OpMemberName %CB0 0 "CB0"
               OpName %_ ""
               OpDecorate %_arr_mat4v4float_uint_6 ArrayStride 64
               OpMemberDecorate %TestStruct 0 RowMajor
               OpMemberDecorate %TestStruct 0 Offset 0
               OpMemberDecorate %TestStruct 0 MatrixStride 16
               OpDecorate %_arr_TestStruct_uint_16 ArrayStride 384
               OpMemberDecorate %CB0 0 Offset 0
               OpDecorate %CB0 Block
               OpDecorate %_ DescriptorSet 0
               OpDecorate %_ Binding 0
       %void = OpTypeVoid
          %3 = OpTypeFunction %void
      %float = OpTypeFloat 32
    %v4float = OpTypeVector %float 4
%mat4v4float = OpTypeMatrix %v4float 4
       %uint = OpTypeInt 32 0
     %uint_6 = OpConstant %uint 6
%_arr_mat4v4float_uint_6 = OpTypeArray %mat4v4float %uint_6
 %TestStruct = OpTypeStruct %_arr_mat4v4float_uint_6
    %uint_16 = OpConstant %uint 16
%_arr_TestStruct_uint_16 = OpTypeArray %TestStruct %uint_16
        %CB0 = OpTypeStruct %_arr_TestStruct_uint_16
%_ptr_Uniform_CB0 = OpTypePointer Uniform %CB0
          %_ = OpVariable %_ptr_Uniform_CB0 Uniform
       %main = OpFunction %void None %3
          %5 = OpLabel
               OpReturn
               OpFunctionEnd
