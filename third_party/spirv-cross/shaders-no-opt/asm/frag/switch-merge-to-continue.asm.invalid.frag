; SPIR-V
; Version: 1.0
; Generator: Khronos Glslang Reference Front End; 3
; Bound: 57
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
       %uint = OpTypeInt 32 0
     %uint_0 = OpConstant %uint 0
%_ptr_Output_float = OpTypePointer Output %float
    %float_3 = OpConstant %float 3
     %uint_1 = OpConstant %uint 1
     %uint_2 = OpConstant %uint 2
      %int_1 = OpConstant %int 1
%_ptr_Input_v4float = OpTypePointer Input %v4float
         %v0 = OpVariable %_ptr_Input_v4float Input
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
         %25 = OpLoad %int %i
               OpSelectionMerge %19 None
               OpSwitch %25 %28 0 %26 1 %27
         %28 = OpLabel
         %46 = OpAccessChain %_ptr_Output_float %FragColor %uint_2
         %47 = OpLoad %float %46
         %48 = OpFAdd %float %47 %float_3
         %49 = OpAccessChain %_ptr_Output_float %FragColor %uint_2
               OpStore %49 %48
               OpBranch %19
         %26 = OpLabel
         %33 = OpAccessChain %_ptr_Output_float %FragColor %uint_0
         %34 = OpLoad %float %33
         %35 = OpFAdd %float %34 %float_1
         %36 = OpAccessChain %_ptr_Output_float %FragColor %uint_0
               OpStore %36 %35
               OpBranch %19
         %27 = OpLabel
         %40 = OpAccessChain %_ptr_Output_float %FragColor %uint_1
         %41 = OpLoad %float %40
         %42 = OpFAdd %float %41 %float_3
         %43 = OpAccessChain %_ptr_Output_float %FragColor %uint_1
               OpStore %43 %42
               OpBranch %19
         %19 = OpLabel
         %52 = OpLoad %int %i
         %54 = OpIAdd %int %52 %int_1
               OpStore %i %54
               OpBranch %16
         %18 = OpLabel
               OpReturn
               OpFunctionEnd
