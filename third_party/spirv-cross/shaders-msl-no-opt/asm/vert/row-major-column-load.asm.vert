; SPIR-V
; Version: 1.0
; Generator: Khronos Glslang Reference Front End; 10
; Bound: 20
; Schema: 0
               OpCapability Shader
               OpMemoryModel Logical GLSL450
               OpEntryPoint Vertex %main "main" %_Ret_Val
               OpSource HLSL 600
               OpName %_Block0T "_Block0T"
               OpMemberName %_Block0T 0 "World"
               OpName %_Block0 "_Block0"
               OpName %_Ret_Val "_Ret_Val"
               OpName %main "main"
               OpDecorate %_Block0T Block
               OpMemberDecorate %_Block0T 0 Offset 0
               OpMemberDecorate %_Block0T 0 RowMajor
               OpMemberDecorate %_Block0T 0 MatrixStride 16
               OpDecorate %_Block0 Binding 0
               OpDecorate %_Block0 DescriptorSet 0
               OpDecorate %_Ret_Val Location 0
       %float = OpTypeFloat 32
     %v3float = OpTypeVector %float 3
 %mat4v3float = OpTypeMatrix %v3float 4
    %_Block0T = OpTypeStruct %mat4v3float
%_ptr_Uniform__Block0T = OpTypePointer Uniform %_Block0T
     %_Block0 = OpVariable %_ptr_Uniform__Block0T Uniform
%_ptr_Uniform_v3float = OpTypePointer Uniform %v3float
%_ptr_Output_v3float = OpTypePointer Output %v3float
    %_Ret_Val = OpVariable %_ptr_Output_v3float Output
        %void = OpTypeVoid
         %int = OpTypeInt 32 1
       %int_0 = OpConstant %int 0
       %int_3 = OpConstant %int 3
   %func_type = OpTypeFunction %void
       %main = OpFunction %void None %func_type
      %entry = OpLabel
        %ptr = OpAccessChain %_ptr_Uniform_v3float %_Block0 %int_0 %int_3
        %val = OpLoad %v3float %ptr Volatile
               OpStore %_Ret_Val %val
               OpReturn
               OpFunctionEnd
