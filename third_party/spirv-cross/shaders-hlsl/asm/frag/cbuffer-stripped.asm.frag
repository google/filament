; SPIR-V
; Version: 1.0
; Generator: Khronos Glslang Reference Front End; 1
; Bound: 34
; Schema: 0
               OpCapability Shader
          %1 = OpExtInstImport "GLSL.std.450"
               OpMemoryModel Logical GLSL450
               OpEntryPoint Fragment %main "main" %_entryPointOutput
               OpExecutionMode %main OriginUpperLeft
               OpSource HLSL 500
               OpMemberDecorate %UBO 0 RowMajor
               OpMemberDecorate %UBO 0 Offset 0
               OpMemberDecorate %UBO 0 MatrixStride 16
               OpMemberDecorate %UBO 1 Offset 64
               OpDecorate %UBO Block
               OpDecorate %_ DescriptorSet 0
               OpDecorate %_ Binding 0
               OpDecorate %_entryPointOutput Location 0
       %void = OpTypeVoid
          %3 = OpTypeFunction %void
      %float = OpTypeFloat 32
    %v2float = OpTypeVector %float 2
          %8 = OpTypeFunction %v2float
%_ptr_Function_v2float = OpTypePointer Function %v2float
    %v4float = OpTypeVector %float 4
%mat2v4float = OpTypeMatrix %v4float 2
        %UBO = OpTypeStruct %mat2v4float %v4float
%_ptr_Uniform_UBO = OpTypePointer Uniform %UBO
          %_ = OpVariable %_ptr_Uniform_UBO Uniform
        %int = OpTypeInt 32 1
      %int_1 = OpConstant %int 1
%_ptr_Uniform_v4float = OpTypePointer Uniform %v4float
      %int_0 = OpConstant %int 0
%_ptr_Uniform_mat2v4float = OpTypePointer Uniform %mat2v4float
%_ptr_Output_v2float = OpTypePointer Output %v2float
%_entryPointOutput = OpVariable %_ptr_Output_v2float Output
       %main = OpFunction %void None %3
          %5 = OpLabel
         %33 = OpFunctionCall %v2float %_main_
               OpStore %_entryPointOutput %33
               OpReturn
               OpFunctionEnd
     %_main_ = OpFunction %v2float None %8
         %10 = OpLabel
         %a0 = OpVariable %_ptr_Function_v2float Function
         %21 = OpAccessChain %_ptr_Uniform_v4float %_ %int_1
         %22 = OpLoad %v4float %21
         %25 = OpAccessChain %_ptr_Uniform_mat2v4float %_ %int_0
         %26 = OpLoad %mat2v4float %25
         %27 = OpVectorTimesMatrix %v2float %22 %26
               OpStore %a0 %27
         %28 = OpLoad %v2float %a0
               OpReturnValue %28
               OpFunctionEnd
