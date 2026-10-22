; SPIR-V
; Version: 1.0
; Generator: Khronos Glslang Reference Front End; 11
; Bound: 41
; Schema: 0
               OpCapability Shader
          %1 = OpExtInstImport "GLSL.std.450"
               OpMemoryModel Logical GLSL450
               OpEntryPoint Fragment %main "main" %b %FragColor
               OpExecutionMode %main OriginUpperLeft
               OpSource GLSL 450
               OpName %main "main"
               OpName %b "b"
               OpName %FragColor "FragColor"
               OpDecorate %b RelaxedPrecision
               OpDecorate %b Location 0
               OpDecorate %21 RelaxedPrecision
               OpDecorate %24 RelaxedPrecision
               OpDecorate %FragColor Location 0
               OpDecorate %39 RelaxedPrecision
       %void = OpTypeVoid
          %3 = OpTypeFunction %void
      %float = OpTypeFloat 32
    %float_0 = OpConstant %float 0
        %int = OpTypeInt 32 1
      %int_0 = OpConstant %int 0
%_ptr_Input_float = OpTypePointer Input %float
          %b = OpVariable %_ptr_Input_float Input
      %int_1 = OpConstant %int 1
      %int_4 = OpConstant %int 4
       %bool = OpTypeBool
    %float_4 = OpConstant %float 4
%_ptr_Output_float = OpTypePointer Output %float
  %FragColor = OpVariable %_ptr_Output_float Output
       %main = OpFunction %void None %3
          %5 = OpLabel
               OpBranch %14
         %14 = OpLabel
         %40 = OpPhi %int %int_0 %5 %27 %14
         %39 = OpPhi %float %float_0 %5 %24 %14
         %21 = OpLoad %float %b
         %24 = OpExtInst %float %1 Fma %39 %21 %21
         %27 = OpIAdd %int %40 %int_1
         %31 = OpSLessThan %bool %27 %int_4
               OpLoopMerge %16 %14 None
               OpBranchConditional %31 %14 %16
         %16 = OpLabel
         %38 = OpFMul %float %39 %float_4
               OpStore %FragColor %38
               OpReturn
               OpFunctionEnd
