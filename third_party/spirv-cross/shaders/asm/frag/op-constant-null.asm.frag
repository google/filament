; SPIR-V
; Version: 1.0
; Generator: Khronos Glslang Reference Front End; 1
; Bound: 45
; Schema: 0
               OpCapability Shader
          %1 = OpExtInstImport "GLSL.std.450"
               OpMemoryModel Logical GLSL450
               OpEntryPoint Fragment %main "main" %FragColor
               OpExecutionMode %main OriginUpperLeft
               OpSource ESSL 310
               OpName %main "main"
               OpName %a "a"
               OpName %b "b"
               OpName %c "c"
               OpName %D "D"
               OpMemberName %D 0 "a"
               OpMemberName %D 1 "b"
               OpName %d "d"
               OpName %e "e"
               OpName %FragColor "FragColor"
               OpDecorate %a RelaxedPrecision
               OpDecorate %b RelaxedPrecision
               OpDecorate %c RelaxedPrecision
               OpMemberDecorate %D 0 RelaxedPrecision
               OpMemberDecorate %D 1 RelaxedPrecision
               OpDecorate %e RelaxedPrecision
               OpDecorate %FragColor RelaxedPrecision
               OpDecorate %FragColor Location 0
               OpDecorate %44 RelaxedPrecision
               OpDecorate %float_1 RelaxedPrecision
               OpDecorate %14 RelaxedPrecision
               OpDecorate %23 RelaxedPrecision
               OpDecorate %41 RelaxedPrecision
       %void = OpTypeVoid
          %3 = OpTypeFunction %void
      %float = OpTypeFloat 32
%_ptr_Function_float = OpTypePointer Function %float
    %float_1 = OpConstantNull %float
    %v4float = OpTypeVector %float 4
%_ptr_Function_v4float = OpTypePointer Function %v4float
    %float_2 = OpConstantNull %float
         %14 = OpConstantNull %v4float
    %v3float = OpTypeVector %float 3
%mat2v3float = OpTypeMatrix %v3float 2
%_ptr_Function_mat2v3float = OpTypePointer Function %mat2v3float
    %float_4 = OpConstantNull %float
         %20 = OpConstantNull %v3float
    %float_5 = OpConstantNull %float
         %22 = OpConstantNull %v3float
         %23 = OpConstantNull %mat2v3float
          %D = OpTypeStruct %v4float %float
%_ptr_Function_D = OpTypePointer Function %D
         %27 = OpConstantNull %D
       %uint = OpTypeInt 32 0
     %uint_4 = OpConstant %uint 4
%_arr_v4float_uint_4 = OpTypeArray %v4float %uint_4
%_ptr_Function__arr_v4float_uint_4 = OpTypePointer Function %_arr_v4float_uint_4
   %float_10 = OpConstantNull %float
         %34 = OpConstantNull %v4float
   %float_11 = OpConstantNull %float
         %36 = OpConstantNull %v4float
   %float_12 = OpConstantNull %float
         %38 = OpConstantNull %v4float
   %float_13 = OpConstantNull %float
         %40 = OpConstantNull %v4float
         %41 = OpConstantNull %_arr_v4float_uint_4
%_ptr_Output_float = OpTypePointer Output %float
  %FragColor = OpVariable %_ptr_Output_float Output
       %main = OpFunction %void None %3
          %5 = OpLabel
          %a = OpVariable %_ptr_Function_float Function
          %b = OpVariable %_ptr_Function_v4float Function
          %c = OpVariable %_ptr_Function_mat2v3float Function
          %d = OpVariable %_ptr_Function_D Function
          %e = OpVariable %_ptr_Function__arr_v4float_uint_4 Function
               OpStore %a %float_1
               OpStore %b %14
               OpStore %c %23
               OpStore %d %27
               OpStore %e %41
         %44 = OpLoad %float %a
               OpStore %FragColor %44
               OpReturn
               OpFunctionEnd
