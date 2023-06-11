; SPIR-V
; Version: 1.0
; Generator: Khronos Glslang Reference Front End; 10
; Bound: 25
; Schema: 0
               OpCapability Shader
          %1 = OpExtInstImport "GLSL.std.450"
               OpMemoryModel Logical GLSL450
               OpEntryPoint Fragment %main "main" %FragColor
               OpExecutionMode %main OriginUpperLeft
               OpSource GLSL 450
               OpName %main "main"
               OpName %FragColor "FragColor"
               OpName %UBOs "UBOs"
               OpMemberName %UBOs 0 "v"
               OpName %ubos "ubos"
               OpDecorate %FragColor Location 0
               OpMemberDecorate %UBOs 0 Offset 0
               OpDecorate %UBOs Block
               OpDecorate %ubos DescriptorSet 0
               OpDecorate %ubos Binding 0
       %void = OpTypeVoid
          %3 = OpTypeFunction %void
      %float = OpTypeFloat 32
    %v4float = OpTypeVector %float 4
%_ptr_Output_v4float = OpTypePointer Output %v4float
  %FragColor = OpVariable %_ptr_Output_v4float Output
       %UBOs = OpTypeStruct %v4float
       %uint = OpTypeInt 32 0
     %uint_2 = OpConstant %uint 2
%_arr_UBOs_uint_2 = OpTypeArray %UBOs %uint_2
%_ptr_Uniform__arr_UBOs_uint_2 = OpTypePointer Uniform %_arr_UBOs_uint_2
%_ptr_Uniform_UBOs = OpTypePointer Uniform %UBOs
       %ubos = OpVariable %_ptr_Uniform__arr_UBOs_uint_2 Uniform
        %int = OpTypeInt 32 1
      %int_0 = OpConstant %int 0
%_ptr_Uniform_v4float = OpTypePointer Uniform %v4float
      %int_1 = OpConstant %int 1
       %main = OpFunction %void None %3
          %5 = OpLabel
         %ptr0_partial = OpAccessChain %_ptr_Uniform_UBOs %ubos %int_0
		 %ptr0 = OpAccessChain %_ptr_Uniform_v4float %ptr0_partial %int_0
         %ptr1_partial = OpAccessChain %_ptr_Uniform_UBOs %ubos %int_1
		 %ptr1 = OpAccessChain %_ptr_Uniform_v4float %ptr1_partial %int_0
         %20 = OpLoad %v4float %ptr0
         %23 = OpLoad %v4float %ptr1
         %24 = OpFAdd %v4float %20 %23
               OpStore %FragColor %24
               OpReturn
               OpFunctionEnd
