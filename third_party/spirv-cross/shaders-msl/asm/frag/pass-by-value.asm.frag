; SPIR-V
; Version: 1.0
; Generator: Khronos Glslang Reference Front End; 6
; Bound: 32
; Schema: 0
               OpCapability Shader
          %1 = OpExtInstImport "GLSL.std.450"
               OpMemoryModel Logical GLSL450
               OpEntryPoint Fragment %main "main" %FragColor
               OpExecutionMode %main OriginUpperLeft
               OpSource GLSL 450
               OpName %main "main"
               OpName %add_value_f1_f1_ "add_value(f1;f1;"
               OpName %v "v"
               OpName %w "w"
               OpName %FragColor "FragColor"
               OpName %Registers "Registers"
               OpMemberName %Registers 0 "foo"
               OpName %registers "registers"
               OpDecorate %FragColor Location 0
               OpMemberDecorate %Registers 0 Offset 0
               OpDecorate %Registers Block
       %void = OpTypeVoid
          %3 = OpTypeFunction %void
      %float = OpTypeFloat 32
%_ptr_Function_float = OpTypePointer Function %float
          %8 = OpTypeFunction %float %float %float
%_ptr_Output_float = OpTypePointer Output %float
  %FragColor = OpVariable %_ptr_Output_float Output
   %float_10 = OpConstant %float 10
  %Registers = OpTypeStruct %float
%_ptr_PushConstant_Registers = OpTypePointer PushConstant %Registers
  %registers = OpVariable %_ptr_PushConstant_Registers PushConstant
        %int = OpTypeInt 32 1
      %int_0 = OpConstant %int 0
%_ptr_PushConstant_float = OpTypePointer PushConstant %float
       %main = OpFunction %void None %3
          %5 = OpLabel
         %29 = OpAccessChain %_ptr_PushConstant_float %registers %int_0
         %30 = OpLoad %float %29
         %31 = OpFunctionCall %float %add_value_f1_f1_ %float_10 %30
               OpStore %FragColor %31
               OpReturn
               OpFunctionEnd
%add_value_f1_f1_ = OpFunction %float None %8
          %v = OpFunctionParameter %float
          %w = OpFunctionParameter %float
         %12 = OpLabel
         %15 = OpFAdd %float %v %w
               OpReturnValue %15
               OpFunctionEnd
