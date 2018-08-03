; SPIR-V
; Version: 1.0
; Generator: Khronos Glslang Reference Front End; 3
; Bound: 47
; Schema: 0
               OpCapability Shader
          %1 = OpExtInstImport "GLSL.std.450"
               OpMemoryModel Logical GLSL450
               OpEntryPoint Fragment %main "main" %counter %FragColor
               OpExecutionMode %main OriginUpperLeft
               OpSource GLSL 450
               OpName %main "main"
               OpName %counter "counter"
               OpName %FragColor "FragColor"
               OpDecorate %counter Flat
               OpDecorate %counter Location 0
               OpDecorate %FragColor Location 0
       %void = OpTypeVoid
          %3 = OpTypeFunction %void
      %float = OpTypeFloat 32
    %v4float = OpTypeVector %float 4
          %8 = OpTypeFunction %v4float
        %int = OpTypeInt 32 1
%_ptr_Input_int = OpTypePointer Input %int
    %counter = OpVariable %_ptr_Input_int Input
     %int_10 = OpConstant %int 10
       %bool = OpTypeBool
   %float_10 = OpConstant %float 10
         %21 = OpConstantComposite %v4float %float_10 %float_10 %float_10 %float_10
   %float_30 = OpConstant %float 30
         %25 = OpConstantComposite %v4float %float_30 %float_30 %float_30 %float_30
%_ptr_Output_v4float = OpTypePointer Output %v4float
  %FragColor = OpVariable %_ptr_Output_v4float Output
%_ptr_Function_v4float = OpTypePointer Function %v4float
      %false = OpConstantFalse %bool
         %44 = OpUndef %v4float
       %main = OpFunction %void None %3
          %5 = OpLabel
               OpBranch %33
         %33 = OpLabel
         %45 = OpPhi %v4float %44 %5 %44 %35
               OpLoopMerge %34 %35 None
               OpBranch %36
         %36 = OpLabel
         %37 = OpLoad %int %counter
         %38 = OpIEqual %bool %37 %int_10
               OpSelectionMerge %39 None
               OpBranchConditional %38 %40 %41
         %40 = OpLabel
               OpBranch %34
         %41 = OpLabel
               OpBranch %34
         %39 = OpLabel
               OpUnreachable
         %35 = OpLabel
               OpBranchConditional %false %33 %34
         %34 = OpLabel
         %46 = OpPhi %v4float %21 %40 %25 %41 %44 %35
               OpStore %FragColor %46
               OpReturn
               OpFunctionEnd
