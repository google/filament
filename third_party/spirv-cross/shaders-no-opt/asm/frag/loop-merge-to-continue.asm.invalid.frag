; SPIR-V
; Version: 1.0
; Generator: Khronos Glslang Reference Front End; 3
; Bound: 51
; Schema: 0
               OpCapability Shader
          %1 = OpExtInstImport "GLSL.std.450"
               OpMemoryModel Logical GLSL450
               OpEntryPoint Fragment %main "main" %FragColor %v0
               OpExecutionMode %main OriginUpperLeft
               OpSource GLSL 450
               OpName %main "main"
               OpName %FragColor "FragColor"
               OpName %i "i"
               OpName %j "j"
               OpName %v0 "v0"
               OpDecorate %FragColor Location 0
               OpDecorate %v0 Location 0
       %void = OpTypeVoid
          %3 = OpTypeFunction %void
      %float = OpTypeFloat 32
    %v4float = OpTypeVector %float 4
%_ptr_Output_v4float = OpTypePointer Output %v4float
  %FragColor = OpVariable %_ptr_Output_v4float Output
    %float_1 = OpConstant %float 1
         %11 = OpConstantComposite %v4float %float_1 %float_1 %float_1 %float_1
        %int = OpTypeInt 32 1
%_ptr_Function_int = OpTypePointer Function %int
      %int_0 = OpConstant %int 0
      %int_4 = OpConstant %int 4
       %bool = OpTypeBool
%_ptr_Input_v4float = OpTypePointer Input %v4float
         %v0 = OpVariable %_ptr_Input_v4float Input
      %int_3 = OpConstant %int 3
%_ptr_Input_float = OpTypePointer Input %float
      %int_1 = OpConstant %int 1
       %main = OpFunction %void None %3
          %5 = OpLabel
          %i = OpVariable %_ptr_Function_int Function
          %j = OpVariable %_ptr_Function_int Function
               OpStore %FragColor %11
               OpStore %i %int_0
               OpBranch %16
         %16 = OpLabel
               OpLoopMerge %18 %19 None
               OpBranch %20
         %20 = OpLabel
         %21 = OpLoad %int %i
         %24 = OpSLessThan %bool %21 %int_4
               OpBranchConditional %24 %17 %18
         %17 = OpLabel
               OpStore %j %int_0
               OpBranch %26
         %26 = OpLabel
               OpLoopMerge %19 %29 None
               OpBranch %30
         %30 = OpLabel
         %31 = OpLoad %int %j
         %32 = OpSLessThan %bool %31 %int_4
               OpBranchConditional %32 %27 %19
         %27 = OpLabel
         %35 = OpLoad %int %i
         %36 = OpLoad %int %j
         %37 = OpIAdd %int %35 %36
         %39 = OpBitwiseAnd %int %37 %int_3
         %41 = OpAccessChain %_ptr_Input_float %v0 %39
         %42 = OpLoad %float %41
         %43 = OpLoad %v4float %FragColor
         %44 = OpCompositeConstruct %v4float %42 %42 %42 %42
         %45 = OpFAdd %v4float %43 %44
               OpStore %FragColor %45
               OpBranch %29
         %29 = OpLabel
         %46 = OpLoad %int %j
         %48 = OpIAdd %int %46 %int_1
               OpStore %j %48
               OpBranch %26
         %19 = OpLabel
         %49 = OpLoad %int %i
         %50 = OpIAdd %int %49 %int_1
               OpStore %i %50
               OpBranch %16
         %18 = OpLabel
               OpReturn
               OpFunctionEnd
