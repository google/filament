; SPIR-V
; Version: 1.0
; Generator: Khronos Glslang Reference Front End; 6
; Bound: 51
; Schema: 0
               OpCapability Shader
          %1 = OpExtInstImport "GLSL.std.450"
               OpMemoryModel Logical GLSL450
               OpEntryPoint Fragment %main "main" %FragColor
               OpExecutionMode %main OriginUpperLeft
               OpSource GLSL 450
               OpName %main "main"
               OpName %FragColor "FragColor"
               OpDecorate %FragColor Location 0
       %void = OpTypeVoid
          %3 = OpTypeFunction %void
      %float = OpTypeFloat 32
    %float_0 = OpConstant %float 0
        %int = OpTypeInt 32 1
      %int_0 = OpConstant %int 0
     %int_16 = OpConstant %int 16
       %bool = OpTypeBool
    %float_1 = OpConstant %float 1
      %int_1 = OpConstant %int 1
    %float_2 = OpConstant %float 2
    %v4float = OpTypeVector %float 4
%_ptr_Output_v4float = OpTypePointer Output %v4float
  %FragColor = OpVariable %_ptr_Output_v4float Output
       %main = OpFunction %void None %3
          %5 = OpLabel
               OpBranch %14
         %14 = OpLabel
         %50 = OpPhi %float %float_0 %5 %25 %15
         %47 = OpPhi %int %int_0 %5 %28 %15
         %22 = OpSLessThan %bool %47 %int_16
               OpLoopMerge %16 %15 None
               OpBranchConditional %22 %body1 %16
      %body1 = OpLabel
         %25 = OpFAdd %float %50 %float_1
         %28 = OpIAdd %int %47 %int_1
			   OpBranch %15
         %15 = OpLabel
               OpBranch %14
         %16 = OpLabel
         %46 = OpCompositeConstruct %v4float %50 %50 %50 %50
               OpStore %FragColor %46
               OpReturn
               OpFunctionEnd
