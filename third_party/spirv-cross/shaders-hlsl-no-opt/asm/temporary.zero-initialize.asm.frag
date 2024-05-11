; SPIR-V
; Version: 1.0
; Generator: Khronos Glslang Reference Front End; 8
; Bound: 65
; Schema: 0
               OpCapability Shader
          %1 = OpExtInstImport "GLSL.std.450"
               OpMemoryModel Logical GLSL450
               OpEntryPoint Fragment %main "main" %FragColor %vA %vB
               OpExecutionMode %main OriginUpperLeft
               OpSource ESSL 310
               OpName %main "main"
               OpName %FragColor "FragColor"
               OpName %vA "vA"
               OpName %vB "vB"
               OpDecorate %FragColor RelaxedPrecision
               OpDecorate %FragColor Location 0
               OpDecorate %vA RelaxedPrecision
               OpDecorate %vA Flat
               OpDecorate %vA Location 0
               OpDecorate %25 RelaxedPrecision
               OpDecorate %30 RelaxedPrecision
               OpDecorate %vB RelaxedPrecision
               OpDecorate %vB Flat
               OpDecorate %vB Location 1
               OpDecorate %38 RelaxedPrecision
               OpDecorate %40 RelaxedPrecision
               OpDecorate %49 RelaxedPrecision
               OpDecorate %51 RelaxedPrecision
               OpDecorate %53 RelaxedPrecision
               OpDecorate %56 RelaxedPrecision
               OpDecorate %64 RelaxedPrecision
               OpDecorate %58 RelaxedPrecision
               OpDecorate %57 RelaxedPrecision
               OpDecorate %60 RelaxedPrecision
       %void = OpTypeVoid
          %3 = OpTypeFunction %void
      %float = OpTypeFloat 32
    %v4float = OpTypeVector %float 4
%_ptr_Output_v4float = OpTypePointer Output %v4float
  %FragColor = OpVariable %_ptr_Output_v4float Output
    %float_0 = OpConstant %float 0
         %11 = OpConstantComposite %v4float %float_0 %float_0 %float_0 %float_0
        %int = OpTypeInt 32 1
      %int_0 = OpConstant %int 0
%_ptr_Input_int = OpTypePointer Input %int
         %vA = OpVariable %_ptr_Input_int Input
       %bool = OpTypeBool
     %int_20 = OpConstant %int 20
     %int_50 = OpConstant %int 50
         %vB = OpVariable %_ptr_Input_int Input
     %int_40 = OpConstant %int 40
     %int_60 = OpConstant %int 60
     %int_10 = OpConstant %int 10
    %float_1 = OpConstant %float 1
         %63 = OpConstantComposite %v4float %float_1 %float_1 %float_1 %float_1
       %main = OpFunction %void None %3
          %5 = OpLabel
               OpStore %FragColor %11
               OpBranch %17
         %17 = OpLabel
         %60 = OpPhi %int %int_0 %5 %58 %20
         %57 = OpPhi %int %int_0 %5 %56 %20
         %25 = OpLoad %int %vA
         %27 = OpSLessThan %bool %57 %25
               OpLoopMerge %19 %20 None
               OpBranchConditional %27 %18 %19
         %18 = OpLabel
         %30 = OpIAdd %int %25 %57
         %32 = OpIEqual %bool %30 %int_20
               OpSelectionMerge %34 None
               OpBranchConditional %32 %33 %36
         %33 = OpLabel
               OpBranch %34
         %36 = OpLabel
         %38 = OpLoad %int %vB
         %40 = OpIAdd %int %38 %57
         %42 = OpIEqual %bool %40 %int_40
         %64 = OpSelect %int %42 %int_60 %60
               OpBranch %34
         %34 = OpLabel
         %58 = OpPhi %int %int_50 %33 %64 %36
         %49 = OpIAdd %int %58 %int_10
         %51 = OpLoad %v4float %FragColor
         %53 = OpFAdd %v4float %51 %63
               OpStore %FragColor %53
               OpBranch %20
         %20 = OpLabel
         %56 = OpIAdd %int %57 %49
               OpBranch %17
         %19 = OpLabel
               OpReturn
               OpFunctionEnd
