; SPIR-V
; Version: 1.0
; Generator: Khronos Glslang Reference Front End; 10
; Bound: 27
; Schema: 0
               OpCapability Shader
          %1 = OpExtInstImport "GLSL.std.450"
               OpMemoryModel Logical GLSL450
               OpEntryPoint Fragment %main "main" %vColor %FragColor
               OpExecutionMode %main OriginUpperLeft
               OpSource ESSL 310
               OpName %main "main"
               OpName %b "b"
               OpName %vColor "vColor"
               OpName %FragColor "FragColor"
               OpDecorate %b RelaxedPrecision
               OpDecorate %vColor RelaxedPrecision
               OpDecorate %vColor Location 0
               OpDecorate %FragColor RelaxedPrecision
               OpDecorate %FragColor Location 0
       %void = OpTypeVoid
          %3 = OpTypeFunction %void
      %float = OpTypeFloat 32
%_ptr_Function_float = OpTypePointer Function %float
%_ptr_Input_float = OpTypePointer Input %float
     %vColor = OpVariable %_ptr_Input_float Input
       %bool = OpTypeBool
      %false = OpConstantFalse %bool
%_ptr_Output_float = OpTypePointer Output %float
  %FragColor = OpVariable %_ptr_Output_float Output
       %main = OpFunction %void None %3
          %5 = OpLabel
               OpBranch %6
          %6 = OpLabel
               OpLoopMerge %8 %9 None
               OpBranch %7
          %7 = OpLabel
		  	%15 = OpLoad %float %vColor
          %b = OpFMul %float %15 %15
               OpBranch %9
          %9 = OpLabel
               OpBranchConditional %false %6 %8
          %8 = OpLabel
         %bb = OpFMul %float %b %b
               OpStore %FragColor %bb
               OpReturn
               OpFunctionEnd
