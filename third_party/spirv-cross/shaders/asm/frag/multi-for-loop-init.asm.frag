; SPIR-V
; Version: 1.0
; Generator: Khronos Glslang Reference Front End; 3
; Bound: 52
; Schema: 0
               OpCapability Shader
          %1 = OpExtInstImport "GLSL.std.450"
               OpMemoryModel Logical GLSL450
               OpEntryPoint Fragment %main "main" %FragColor %counter %ucounter
               OpExecutionMode %main OriginUpperLeft
               OpSource ESSL 310
               OpName %main "main"
               OpName %FragColor "FragColor"
               OpName %i "i"
               OpName %j "j"
               OpName %counter "counter"
               OpName %ucounter "ucounter"
               OpDecorate %FragColor RelaxedPrecision
               OpDecorate %FragColor Location 0
               OpDecorate %i RelaxedPrecision
               OpDecorate %j RelaxedPrecision
               OpDecorate %23 RelaxedPrecision
               OpDecorate %27 RelaxedPrecision
               OpDecorate %31 RelaxedPrecision
               OpDecorate %32 RelaxedPrecision
               OpDecorate %33 RelaxedPrecision
               OpDecorate %34 RelaxedPrecision
               OpDecorate %35 RelaxedPrecision
               OpDecorate %36 RelaxedPrecision
               OpDecorate %37 RelaxedPrecision
               OpDecorate %38 RelaxedPrecision
               OpDecorate %39 RelaxedPrecision
               OpDecorate %40 RelaxedPrecision
               OpDecorate %counter RelaxedPrecision
               OpDecorate %counter Flat
               OpDecorate %counter Location 0
               OpDecorate %43 RelaxedPrecision
               OpDecorate %44 RelaxedPrecision
               OpDecorate %45 RelaxedPrecision
               OpDecorate %46 RelaxedPrecision
               OpDecorate %47 RelaxedPrecision
               OpDecorate %48 RelaxedPrecision
               OpDecorate %ucounter RelaxedPrecision
               OpDecorate %ucounter Flat
               OpDecorate %ucounter Location 1
       %void = OpTypeVoid
          %3 = OpTypeFunction %void
      %float = OpTypeFloat 32
    %v4float = OpTypeVector %float 4
%_ptr_Output_v4float = OpTypePointer Output %v4float
  %FragColor = OpVariable %_ptr_Output_v4float Output
    %float_0 = OpConstant %float 0
         %11 = OpConstantComposite %v4float %float_0 %float_0 %float_0 %float_0
        %int = OpTypeInt 32 1
%_ptr_Function_int = OpTypePointer Function %int
       %uint = OpTypeInt 32 0
%_ptr_Function_uint = OpTypePointer Function %uint
      %int_0 = OpConstant %int 0
      %int_1 = OpConstant %uint 1
     %int_10 = OpConstant %int 10
       %bool = OpTypeBool
     %int_20 = OpConstant %uint 20
%_ptr_Input_int = OpTypePointer Input %int
    %counter = OpVariable %_ptr_Input_int Input
%_ptr_Input_uint = OpTypePointer Input %uint
   %ucounter = OpVariable %_ptr_Input_uint Input
       %main = OpFunction %void None %3
          %5 = OpLabel
          %i = OpVariable %_ptr_Function_int Function
          %j = OpVariable %_ptr_Function_uint Function
               OpStore %FragColor %11
               OpStore %i %int_0
               OpStore %j %int_1
               OpBranch %18
         %18 = OpLabel
               OpLoopMerge %20 %21 None
               OpBranch %22
         %22 = OpLabel
         %23 = OpLoad %int %i
         %26 = OpSLessThan %bool %23 %int_10
         %27 = OpLoad %uint %j
         %29 = OpSLessThan %bool %27 %int_20
         %30 = OpLogicalAnd %bool %26 %29
               OpBranchConditional %30 %19 %20
         %19 = OpLabel
         %31 = OpLoad %int %i
         %32 = OpConvertSToF %float %31
         %33 = OpCompositeConstruct %v4float %32 %32 %32 %32
         %34 = OpLoad %v4float %FragColor
         %35 = OpFAdd %v4float %34 %33
               OpStore %FragColor %35
         %36 = OpLoad %uint %j
         %37 = OpConvertUToF %float %36
         %38 = OpCompositeConstruct %v4float %37 %37 %37 %37
         %39 = OpLoad %v4float %FragColor
         %40 = OpFAdd %v4float %39 %38
               OpStore %FragColor %40
               OpBranch %21
         %21 = OpLabel
         %43 = OpLoad %int %counter
         %44 = OpLoad %int %i
         %45 = OpIAdd %int %44 %43
               OpStore %i %45
         %46 = OpLoad %int %counter
         %47 = OpLoad %uint %j
         %48 = OpIAdd %uint %47 %46
               OpStore %j %48
               OpBranch %18
         %20 = OpLabel
               OpReturn
               OpFunctionEnd
