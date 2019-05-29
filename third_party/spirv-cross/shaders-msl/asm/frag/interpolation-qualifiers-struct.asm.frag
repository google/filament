; SPIR-V
; Version: 1.3
; Generator: Khronos Glslang Reference Front End; 6
; Bound: 51
; Schema: 0
               OpCapability Shader
               OpCapability SampleRateShading
          %1 = OpExtInstImport "GLSL.std.450"
               OpMemoryModel Logical GLSL450
               OpEntryPoint Fragment %main "main" %FragColor %inp
               OpExecutionMode %main OriginUpperLeft
               OpSource GLSL 450
               OpName %main "main"
               OpName %FragColor "FragColor"
               OpName %Input "Input"
               OpMemberName %Input 0 "v0"
               OpMemberName %Input 1 "v1"
               OpMemberName %Input 2 "v2"
               OpMemberName %Input 3 "v3"
               OpMemberName %Input 4 "v4"
               OpMemberName %Input 5 "v5"
               OpMemberName %Input 6 "v6"
               OpName %inp "inp"
               OpDecorate %FragColor Location 0
               OpDecorate %inp Location 0
               OpMemberDecorate %Input 1 NoPerspective
               OpMemberDecorate %Input 2 Centroid
               OpMemberDecorate %Input 3 Centroid
               OpMemberDecorate %Input 3 NoPerspective
               OpMemberDecorate %Input 4 Sample
               OpMemberDecorate %Input 5 Sample
               OpMemberDecorate %Input 5 NoPerspective
               OpMemberDecorate %Input 6 Flat
       %void = OpTypeVoid
          %3 = OpTypeFunction %void
      %float = OpTypeFloat 32
    %v4float = OpTypeVector %float 4
%_ptr_Output_v4float = OpTypePointer Output %v4float
  %FragColor = OpVariable %_ptr_Output_v4float Output
    %v2float = OpTypeVector %float 2
    %v3float = OpTypeVector %float 3
      %Input = OpTypeStruct %v2float %v2float %v3float %v4float %float %float %float
%_ptr_Input_Input = OpTypePointer Input %Input
        %inp = OpVariable %_ptr_Input_Input Input
        %int = OpTypeInt 32 1
      %int_0 = OpConstant %int 0
       %uint = OpTypeInt 32 0
     %uint_0 = OpConstant %uint 0
%_ptr_Input_float = OpTypePointer Input %float
      %int_1 = OpConstant %int 1
     %uint_1 = OpConstant %uint 1
      %int_2 = OpConstant %int 2
%_ptr_Input_v3float = OpTypePointer Input %v3float
      %int_3 = OpConstant %int 3
     %uint_3 = OpConstant %uint 3
      %int_4 = OpConstant %int 4
      %int_5 = OpConstant %int 5
      %int_6 = OpConstant %int 6
       %main = OpFunction %void None %3
          %5 = OpLabel
         %20 = OpAccessChain %_ptr_Input_float %inp %int_0 %uint_0
         %21 = OpLoad %float %20
         %24 = OpAccessChain %_ptr_Input_float %inp %int_1 %uint_1
         %25 = OpLoad %float %24
         %26 = OpFAdd %float %21 %25
         %29 = OpAccessChain %_ptr_Input_v3float %inp %int_2
         %30 = OpLoad %v3float %29
         %31 = OpVectorShuffle %v2float %30 %30 0 1
         %34 = OpAccessChain %_ptr_Input_float %inp %int_3 %uint_3
         %35 = OpLoad %float %34
         %37 = OpAccessChain %_ptr_Input_float %inp %int_4
         %38 = OpLoad %float %37
         %39 = OpFMul %float %35 %38
         %41 = OpAccessChain %_ptr_Input_float %inp %int_5
         %42 = OpLoad %float %41
         %43 = OpFAdd %float %39 %42
         %45 = OpAccessChain %_ptr_Input_float %inp %int_6
         %46 = OpLoad %float %45
         %47 = OpFSub %float %43 %46
         %48 = OpCompositeExtract %float %31 0
         %49 = OpCompositeExtract %float %31 1
         %50 = OpCompositeConstruct %v4float %26 %48 %49 %47
               OpStore %FragColor %50
               OpReturn
               OpFunctionEnd
