; SPIR-V
; Version: 1.0
; Generator: Khronos Glslang Reference Front End; 10
; Bound: 37
; Schema: 0
               OpCapability Shader
          %1 = OpExtInstImport "GLSL.std.450"
               OpMemoryModel Logical GLSL450
               OpEntryPoint Fragment %main "main" %FragColor
               OpExecutionMode %main OriginUpperLeft
               OpSource GLSL 450
               OpName %main "main"
               OpName %ACOS_f1_ "mat3"
               OpName %a "a"
               OpName %ACOS_i1_ "gl_Foo"
               OpName %a_0 "a"
               OpName %FragColor "FragColor"
               OpName %param "param"
               OpName %param_0 "param"
               OpDecorate %FragColor Location 0
       %void = OpTypeVoid
          %3 = OpTypeFunction %void
      %float = OpTypeFloat 32
%_ptr_Function_float = OpTypePointer Function %float
          %8 = OpTypeFunction %float %_ptr_Function_float
        %int = OpTypeInt 32 1
%_ptr_Function_int = OpTypePointer Function %int
         %14 = OpTypeFunction %float %_ptr_Function_int
    %float_1 = OpConstant %float 1
%_ptr_Output_float = OpTypePointer Output %float
  %FragColor = OpVariable %_ptr_Output_float Output
    %float_2 = OpConstant %float 2
      %int_4 = OpConstant %int 4
       %main = OpFunction %void None %3
          %5 = OpLabel
      %param = OpVariable %_ptr_Function_float Function
    %param_0 = OpVariable %_ptr_Function_int Function
               OpStore %param %float_2
         %32 = OpFunctionCall %float %ACOS_f1_ %param
               OpStore %param_0 %int_4
         %35 = OpFunctionCall %float %ACOS_i1_ %param_0
         %36 = OpFAdd %float %32 %35
               OpStore %FragColor %36
               OpReturn
               OpFunctionEnd
   %ACOS_f1_ = OpFunction %float None %8
          %a = OpFunctionParameter %_ptr_Function_float
         %11 = OpLabel
         %18 = OpLoad %float %a
         %20 = OpFAdd %float %18 %float_1
               OpReturnValue %20
               OpFunctionEnd
   %ACOS_i1_ = OpFunction %float None %14
        %a_0 = OpFunctionParameter %_ptr_Function_int
         %17 = OpLabel
         %23 = OpLoad %int %a_0
         %24 = OpConvertSToF %float %23
         %25 = OpFAdd %float %24 %float_1
               OpReturnValue %25
               OpFunctionEnd
