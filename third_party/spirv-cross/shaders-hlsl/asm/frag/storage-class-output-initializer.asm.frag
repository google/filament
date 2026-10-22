; SPIR-V
; Version: 1.0
; Generator: Khronos Glslang Reference Front End; 7
; Bound: 25
; Schema: 0
               OpCapability Shader
          %1 = OpExtInstImport "GLSL.std.450"
               OpMemoryModel Logical GLSL450
               OpEntryPoint Fragment %main "main" %FragColors %FragColor
               OpExecutionMode %main OriginUpperLeft
               OpSource GLSL 450
               OpName %main "main"
               OpName %FragColors "FragColors"
               OpName %FragColor "FragColor"
               OpDecorate %FragColors Location 0
               OpDecorate %FragColor Location 2
       %void = OpTypeVoid
          %3 = OpTypeFunction %void
      %float = OpTypeFloat 32
    %v4float = OpTypeVector %float 4
       %uint = OpTypeInt 32 0
     %uint_2 = OpConstant %uint 2
%_arr_v4float_uint_2 = OpTypeArray %v4float %uint_2
%_ptr_Output__arr_v4float_uint_2 = OpTypePointer Output %_arr_v4float_uint_2
    %float_1 = OpConstant %float 1
    %float_2 = OpConstant %float 2
    %float_3 = OpConstant %float 3
    %float_4 = OpConstant %float 4
         %17 = OpConstantComposite %v4float %float_1 %float_2 %float_3 %float_4
   %float_10 = OpConstant %float 10
         %19 = OpConstantComposite %v4float %float_10 %float_10 %float_10 %float_10
         %20 = OpConstantComposite %_arr_v4float_uint_2 %17 %19
%_ptr_Output_v4float = OpTypePointer Output %v4float
    %float_5 = OpConstant %float 5
         %24 = OpConstantComposite %v4float %float_5 %float_5 %float_5 %float_5
 %FragColors = OpVariable %_ptr_Output__arr_v4float_uint_2 Output %20
  %FragColor = OpVariable %_ptr_Output_v4float Output %24
       %main = OpFunction %void None %3
          %5 = OpLabel
               OpReturn
               OpFunctionEnd
