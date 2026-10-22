; SPIR-V
; Version: 1.0
; Generator: Khronos Glslang Reference Front End; 10
; Bound: 52
; Schema: 0
               OpCapability Shader
          %1 = OpExtInstImport "GLSL.std.450"
               OpMemoryModel Logical GLSL450
               OpEntryPoint Fragment %main "main" %vColor %FragColor
               OpExecutionMode %main OriginUpperLeft
               OpSource ESSL 310
               OpName %main "main"
               OpName %a "a"
               OpName %vColor "vColor"
               OpName %b "b"
               OpName %i "i"
               OpName %FragColor "FragColor"
               OpDecorate %a RelaxedPrecision
               OpDecorate %vColor RelaxedPrecision
               OpDecorate %vColor Location 0
               OpDecorate %16 RelaxedPrecision
               OpDecorate %20 RelaxedPrecision
               OpDecorate %FragColor RelaxedPrecision
               OpDecorate %FragColor Location 0
               OpDecorate %37 RelaxedPrecision
               OpDecorate %38 RelaxedPrecision
               OpDecorate %39 RelaxedPrecision
               OpDecorate %43 RelaxedPrecision
               OpDecorate %44 RelaxedPrecision
               OpDecorate %45 RelaxedPrecision
               OpDecorate %49 RelaxedPrecision
       %void = OpTypeVoid
          %3 = OpTypeFunction %void
      %float = OpTypeFloat 32
%_ptr_Function_float = OpTypePointer Function %float
    %v4float = OpTypeVector %float 4
%_ptr_Input_v4float = OpTypePointer Input %v4float
     %vColor = OpVariable %_ptr_Input_v4float Input
       %uint = OpTypeInt 32 0
     %uint_0 = OpConstant %uint 0
%_ptr_Input_float = OpTypePointer Input %float
     %uint_1 = OpConstant %uint 1
        %int = OpTypeInt 32 1
%_ptr_Function_int = OpTypePointer Function %int
      %int_0 = OpConstant %int 0
      %int_4 = OpConstant %int 4
       %bool = OpTypeBool
%_ptr_Output_v4float = OpTypePointer Output %v4float
  %FragColor = OpVariable %_ptr_Output_v4float Output
    %float_1 = OpConstant %float 1
      %int_1 = OpConstant %int 1
       %main = OpFunction %void None %3
          %5 = OpLabel
          %a = OpVariable %_ptr_Function_float Function
          %b = OpVariable %_ptr_Function_float Function
          %i = OpVariable %_ptr_Function_int Function
         %15 = OpAccessChain %_ptr_Input_float %vColor %uint_0
         %16 = OpLoad %float %15
               OpStore %a %16
         %19 = OpAccessChain %_ptr_Input_float %vColor %uint_1
         %20 = OpLoad %float %19
               OpStore %b %20
               OpStore %i %int_0
               OpBranch %25
         %25 = OpLabel
               OpLoopMerge %27 %28 None
               OpBranch %29
         %29 = OpLabel
         %30 = OpLoad %int %i
         %33 = OpSLessThan %bool %30 %int_4
               OpBranchConditional %33 %26 %27
         %26 = OpLabel
         %37 = OpLoad %v4float %FragColor
         %38 = OpCompositeConstruct %v4float %float_1 %float_1 %float_1 %float_1
         %39 = OpFAdd %v4float %37 %38
               OpStore %FragColor %39
               OpBranch %28
         %28 = OpLabel
         %40 = OpLoad %int %i
         %42 = OpIAdd %int %40 %int_1
               OpStore %i %42
         %43 = OpLoad %float %a
         %44 = OpLoad %float %a
         %45 = OpFMul %float %43 %44
         %force_tmp = OpFMul %float %45 %44
         %46 = OpLoad %float %b
         %47 = OpFAdd %float %46 %force_tmp
               OpStore %b %47
               OpBranch %25
         %27 = OpLabel
         %48 = OpLoad %float %b
         %49 = OpLoad %v4float %FragColor
         %50 = OpCompositeConstruct %v4float %48 %48 %48 %48
         %51 = OpFAdd %v4float %49 %50
               OpStore %FragColor %51
               OpReturn
               OpFunctionEnd
