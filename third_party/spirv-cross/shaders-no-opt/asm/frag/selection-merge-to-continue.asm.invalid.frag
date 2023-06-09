; SPIR-V
; Version: 1.0
; Generator: Khronos Glslang Reference Front End; 3
; Bound: 55
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
       %uint = OpTypeInt 32 0
     %uint_0 = OpConstant %uint 0
%_ptr_Input_float = OpTypePointer Input %float
   %float_20 = OpConstant %float 20
      %int_3 = OpConstant %int 3
      %int_1 = OpConstant %int 1
       %main = OpFunction %void None %3
          %5 = OpLabel
          %i = OpVariable %_ptr_Function_int Function
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
         %30 = OpAccessChain %_ptr_Input_float %v0 %uint_0
         %31 = OpLoad %float %30
         %33 = OpFOrdEqual %bool %31 %float_20
               OpSelectionMerge %19 None
               OpBranchConditional %33 %34 %44
         %34 = OpLabel
         %36 = OpLoad %int %i
         %38 = OpBitwiseAnd %int %36 %int_3
         %39 = OpAccessChain %_ptr_Input_float %v0 %38
         %40 = OpLoad %float %39
         %41 = OpLoad %v4float %FragColor
         %42 = OpCompositeConstruct %v4float %40 %40 %40 %40
         %43 = OpFAdd %v4float %41 %42
               OpStore %FragColor %43
               OpBranch %19
         %44 = OpLabel
         %45 = OpLoad %int %i
         %47 = OpBitwiseAnd %int %45 %int_1
         %48 = OpAccessChain %_ptr_Input_float %v0 %47
         %49 = OpLoad %float %48
         %50 = OpLoad %v4float %FragColor
         %51 = OpCompositeConstruct %v4float %49 %49 %49 %49
         %52 = OpFAdd %v4float %50 %51
               OpStore %FragColor %52
               OpBranch %19
         %19 = OpLabel
         %53 = OpLoad %int %i
         %54 = OpIAdd %int %53 %int_1
               OpStore %i %54
               OpBranch %16
         %18 = OpLabel
               OpReturn
               OpFunctionEnd
