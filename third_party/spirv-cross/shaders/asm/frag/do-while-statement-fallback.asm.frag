; SPIR-V
; Version: 1.0
; Generator: Khronos Glslang Reference Front End; 7
; Bound: 35
; Schema: 0
               OpCapability Shader
          %1 = OpExtInstImport "GLSL.std.450"
               OpMemoryModel Logical GLSL450
               OpEntryPoint Fragment %main "main" %FragColor
               OpExecutionMode %main OriginUpperLeft
               OpSource GLSL 450
               OpName %main "main"
               OpName %foo "foo"
               OpName %FragColor "FragColor"
               OpDecorate %FragColor Location 0
       %void = OpTypeVoid
          %3 = OpTypeFunction %void
      %float = OpTypeFloat 32
%_ptr_Function_float = OpTypePointer Function %float
    %float_1 = OpConstant %float 1
    %float_2 = OpConstant %float 2
       %bool = OpTypeBool
      %false = OpConstantFalse %bool
    %float_3 = OpConstant %float 3
    %float_4 = OpConstant %float 4
    %float_5 = OpConstant %float 5
%_ptr_Output_float = OpTypePointer Output %float
  %FragColor = OpVariable %_ptr_Output_float Output
       %main = OpFunction %void None %3
          %5 = OpLabel
        %foo = OpVariable %_ptr_Function_float Function
               OpStore %foo %float_1
               OpBranch %10
         %10 = OpLabel
               OpLoopMerge %12 %13 None
               OpBranch %11
         %11 = OpLabel
               OpBranch %13
         %13 = OpLabel
               OpStore %foo %float_2
               OpBranchConditional %false %10 %12
         %12 = OpLabel
               OpBranch %17
         %17 = OpLabel
               OpLoopMerge %19 %20 None
               OpBranch %18
         %18 = OpLabel
               OpBranch %20
         %20 = OpLabel
               OpStore %foo %float_3
               OpBranchConditional %false %17 %19
         %19 = OpLabel
               OpBranch %22
         %22 = OpLabel
               OpLoopMerge %24 %25 None
               OpBranch %23
         %23 = OpLabel
               OpBranch %25
         %25 = OpLabel
               OpStore %foo %float_4
               OpBranchConditional %false %22 %24
         %24 = OpLabel
               OpBranch %27
         %27 = OpLabel
               OpLoopMerge %29 %30 None
               OpBranch %28
         %28 = OpLabel
               OpBranch %30
         %30 = OpLabel
               OpStore %foo %float_5
               OpBranchConditional %false %27 %29
         %29 = OpLabel
         %34 = OpLoad %float %foo
               OpStore %FragColor %34
               OpReturn
               OpFunctionEnd
