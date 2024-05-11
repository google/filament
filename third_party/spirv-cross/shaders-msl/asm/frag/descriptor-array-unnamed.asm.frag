; SPIR-V
; Version: 1.0
; Generator: Khronos Glslang Reference Front End; 7
; Bound: 39
; Schema: 0
               OpCapability Shader
          %1 = OpExtInstImport "GLSL.std.450"
               OpMemoryModel Logical GLSL450
               OpEntryPoint Fragment %main "main" %FragColor
               OpExecutionMode %main OriginUpperLeft
               OpSource GLSL 450
               OpDecorate %FragColor Location 0
               OpMemberDecorate %SSBO 0 NonWritable
               OpMemberDecorate %SSBO 0 Offset 0
               OpDecorate %SSBO BufferBlock
               OpDecorate %ssbos DescriptorSet 0
               OpDecorate %ssbos Binding 5
               OpMemberDecorate %Registers 0 Offset 0
               OpDecorate %Registers Block
               OpMemberDecorate %UBO 0 Offset 0
               OpDecorate %UBO Block
               OpDecorate %ubos DescriptorSet 0
               OpDecorate %ubos Binding 1
       %void = OpTypeVoid
          %3 = OpTypeFunction %void
      %float = OpTypeFloat 32
    %v4float = OpTypeVector %float 4
%_ptr_Output_v4float = OpTypePointer Output %v4float
  %FragColor = OpVariable %_ptr_Output_v4float Output
       %SSBO = OpTypeStruct %v4float
       %uint = OpTypeInt 32 0
     %uint_4 = OpConstant %uint 4
%_arr_SSBO_uint_4 = OpTypeArray %SSBO %uint_4
%_ptr_Uniform__arr_SSBO_uint_4 = OpTypePointer Uniform %_arr_SSBO_uint_4
      %ssbos = OpVariable %_ptr_Uniform__arr_SSBO_uint_4 Uniform
        %int = OpTypeInt 32 1
  %Registers = OpTypeStruct %int
%_ptr_PushConstant_Registers = OpTypePointer PushConstant %Registers
  %registers = OpVariable %_ptr_PushConstant_Registers PushConstant
      %int_0 = OpConstant %int 0
%_ptr_PushConstant_int = OpTypePointer PushConstant %int
%_ptr_Uniform_v4float = OpTypePointer Uniform %v4float
        %UBO = OpTypeStruct %v4float
%_arr_UBO_uint_4 = OpTypeArray %UBO %uint_4
%_ptr_Uniform__arr_UBO_uint_4 = OpTypePointer Uniform %_arr_UBO_uint_4
       %ubos = OpVariable %_ptr_Uniform__arr_UBO_uint_4 Uniform
%float_0_200000003 = OpConstant %float 0.200000003
         %36 = OpConstantComposite %v4float %float_0_200000003 %float_0_200000003 %float_0_200000003 %float_0_200000003
       %main = OpFunction %void None %3
          %5 = OpLabel
         %22 = OpAccessChain %_ptr_PushConstant_int %registers %int_0
         %23 = OpLoad %int %22
         %25 = OpAccessChain %_ptr_Uniform_v4float %ssbos %23 %int_0
         %26 = OpLoad %v4float %25
         %31 = OpAccessChain %_ptr_PushConstant_int %registers %int_0
         %32 = OpLoad %int %31
         %33 = OpAccessChain %_ptr_Uniform_v4float %ubos %32 %int_0
         %34 = OpLoad %v4float %33
         %37 = OpFMul %v4float %34 %36
         %38 = OpFAdd %v4float %26 %37
               OpStore %FragColor %38
               OpReturn
               OpFunctionEnd
