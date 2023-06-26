; SPIR-V
; Version: 1.0
; Generator: Khronos Glslang Reference Front End; 7
; Bound: 29
; Schema: 0
               OpCapability Shader
          %1 = OpExtInstImport "GLSL.std.450"
               OpMemoryModel Logical GLSL450
               OpEntryPoint Fragment %main "main" %FragColor %vFloat
               OpExecutionMode %main OriginUpperLeft
               OpSource GLSL 450
               OpName %main "main"
               OpName %FragColor "FragColor"
               OpName %vFloat "vFloat"
               OpName %undef "undef"
               OpDecorate %FragColor Location 0
               OpDecorate %vFloat Location 0
       %void = OpTypeVoid
          %3 = OpTypeFunction %void
      %float = OpTypeFloat 32
    %v4float = OpTypeVector %float 4
%_ptr_Output_v4float = OpTypePointer Output %v4float
  %FragColor = OpVariable %_ptr_Output_v4float Output
%_ptr_Input_v4float = OpTypePointer Input %v4float
     %vFloat = OpVariable %_ptr_Input_v4float Input
    %v2float = OpTypeVector %float 2
%_ptr_Private_v4float = OpTypePointer Private %v4float
      %undef = OpUndef %v4float
       %uint = OpTypeInt 32 0
     %uint_2 = OpConstant %uint 2
%_ptr_Private_float = OpTypePointer Private %float
     %uint_3 = OpConstant %uint 3
%_ptr_Input_float = OpTypePointer Input %float
       %main = OpFunction %void None %3
          %5 = OpLabel
         %13 = OpLoad %v4float %vFloat
         %26 = OpVectorShuffle %v4float %13 %undef 4 1 0xffffffff 3
         %27 = OpVectorShuffle %v4float %13 %13 2 1 0xffffffff 3
         %28 = OpFAdd %v4float %26 %27
               OpStore %FragColor %28
               OpReturn
               OpFunctionEnd
