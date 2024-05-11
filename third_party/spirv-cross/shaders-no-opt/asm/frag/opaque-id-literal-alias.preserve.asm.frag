; SPIR-V
; Version: 1.0
; Generator: Khronos Glslang Reference Front End; 7
; Bound: 50
; Schema: 0
               OpCapability Shader
          %1 = OpExtInstImport "GLSL.std.450"
               OpMemoryModel Logical GLSL450
               OpEntryPoint Fragment %main "main" %FragColor %gl_FragCoord %vUV
               OpExecutionMode %main OriginUpperLeft
               OpSource GLSL 450
               OpName %main "main"
               OpName %FragColor "FragColor"
               OpName %gl_FragCoord "gl_FragCoord"
               OpName %uSampled "uSampled"
               OpName %vUV "vUV"
               OpDecorate %FragColor Location 0
               OpDecorate %gl_FragCoord BuiltIn FragCoord
               OpDecorate %uSampled DescriptorSet 0
               OpDecorate %uSampled Binding 0
               OpDecorate %vUV Location 0
       %void = OpTypeVoid
          %3 = OpTypeFunction %void
      %float = OpTypeFloat 32
    %v4float = OpTypeVector %float 4
%_ptr_Output_v4float = OpTypePointer Output %v4float
  %FragColor = OpVariable %_ptr_Output_v4float Output
    %float_0 = OpConstant %float 0
         %11 = OpConstantComposite %v4float %float_0 %float_0 %float_0 %float_0
%_ptr_Input_v4float = OpTypePointer Input %v4float
%gl_FragCoord = OpVariable %_ptr_Input_v4float Input
       %uint = OpTypeInt 32 0
     %uint_0 = OpConstant %uint 0
%_ptr_Input_float = OpTypePointer Input %float
   %float_10 = OpConstant %float 10
       %bool = OpTypeBool
         %24 = OpTypeImage %float 2D 0 0 1 1 Unknown
         %25 = OpTypeSampledImage %24
%_ptr_UniformConstant_25 = OpTypePointer UniformConstant %25
   %uSampled = OpVariable %_ptr_UniformConstant_25 UniformConstant
    %v2float = OpTypeVector %float 2
%_ptr_Input_v2float = OpTypePointer Input %v2float
        %vUV = OpVariable %_ptr_Input_v2float Input
        %int = OpTypeInt 32 1
      %v2int = OpTypeVector %int 2
      %int_0 = OpConstant %int 0
      %int_1 = OpConstant %int 1
       %main = OpFunction %void None %3
          %5 = OpLabel
               OpStore %FragColor %11
         %17 = OpAccessChain %_ptr_Input_float %gl_FragCoord %uint_0
         %18 = OpLoad %float %17
         %21 = OpFOrdLessThan %bool %18 %float_10
               OpSelectionMerge %23 None
               OpBranchConditional %21 %22 %41
         %22 = OpLabel
         %28 = OpLoad %25 %uSampled
         %32 = OpLoad %v2float %vUV
         %35 = OpConvertFToS %v2int %32
         %64 = OpImage %24 %28
         %38 = OpImageFetch %v4float %64 %35 Sample %int_0
         %39 = OpLoad %v4float %FragColor
         %40 = OpFAdd %v4float %39 %38
               OpStore %FragColor %40
               OpBranch %23
         %41 = OpLabel
         %42 = OpLoad %25 %uSampled
         %43 = OpLoad %v2float %vUV
         %44 = OpConvertFToS %v2int %43
         %46 = OpImage %24 %42
         %47 = OpImageFetch %v4float %46 %44 Sample %int_1
         %48 = OpLoad %v4float %FragColor
         %49 = OpFAdd %v4float %48 %47
               OpStore %FragColor %49
               OpBranch %23
         %23 = OpLabel
               OpReturn
               OpFunctionEnd
