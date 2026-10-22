; SPIR-V
; Version: 1.0
; Generator: Khronos Glslang Reference Front End; 6
; Bound: 28
; Schema: 0
               OpCapability Shader
          %1 = OpExtInstImport "GLSL.std.450"
               OpMemoryModel Logical GLSL450
               OpEntryPoint Fragment %main "main" %vIndex %FragColor
               OpExecutionMode %main OriginUpperLeft
               OpSource ESSL 310
               OpName %main "main"
               OpName %vIndex "vIndex"
               OpName %FragColor "FragColor"
               OpDecorate %vIndex RelaxedPrecision
               OpDecorate %vIndex Flat
               OpDecorate %vIndex Location 0
               OpDecorate %13 RelaxedPrecision
               OpDecorate %FragColor RelaxedPrecision
               OpDecorate %FragColor Location 0
       %void = OpTypeVoid
          %3 = OpTypeFunction %void
      %float = OpTypeFloat 32
    %float_8 = OpConstant %float 8
        %int = OpTypeInt 32 1
%_ptr_Input_int = OpTypePointer Input %int
     %vIndex = OpVariable %_ptr_Input_int Input
    %float_1 = OpConstant %float 1
    %float_3 = OpConstant %float 3
%_ptr_Output_float = OpTypePointer Output %float
  %FragColor = OpVariable %_ptr_Output_float Output
       %main = OpFunction %void None %3
          %5 = OpLabel
         %13 = OpLoad %int %vIndex
               OpSelectionMerge %17 None
               OpSwitch %13 %15 0 %14 2 %14 1 %15 8 %17
         %15 = OpLabel
               OpBranch %17
         %14 = OpLabel
               OpBranch %17
         %17 = OpLabel
         %27 = OpPhi %float %float_3 %15 %float_1 %14 %float_8 %5
               OpStore %FragColor %27
               OpReturn
               OpFunctionEnd
