; SPIR-V
; Version: 1.0
; Generator: Khronos Glslang Reference Front End; 10
; Bound: 61
; Schema: 0
               OpCapability Shader
          %1 = OpExtInstImport "GLSL.std.450"
               OpMemoryModel Logical GLSL450
               OpEntryPoint Fragment %main "main" %FragColor %gl_FragCoord
               OpExecutionMode %main OriginUpperLeft
               OpSource GLSL 450
               OpName %main "main"
               OpName %FragColor "FragColor"
               OpName %uSamp "uSamp"
               OpName %gl_FragCoord "gl_FragCoord"
               OpDecorate %FragColor Location 0
               OpDecorate %uSamp DescriptorSet 0
               OpDecorate %uSamp Binding 0
               OpDecorate %gl_FragCoord BuiltIn FragCoord
       %void = OpTypeVoid
          %3 = OpTypeFunction %void
      %float = OpTypeFloat 32
    %v4float = OpTypeVector %float 4
%_ptr_Output_v4float = OpTypePointer Output %v4float
  %FragColor = OpVariable %_ptr_Output_v4float Output
         %10 = OpTypeImage %float 2D 0 0 1 1 Unknown
         %11 = OpTypeSampledImage %10
%_ptr_UniformConstant_11 = OpTypePointer UniformConstant %11
      %uSamp = OpVariable %_ptr_UniformConstant_11 UniformConstant
%_ptr_Input_v4float = OpTypePointer Input %v4float
%gl_FragCoord = OpVariable %_ptr_Input_v4float Input
    %v2float = OpTypeVector %float 2
       %uint = OpTypeInt 32 0
       %int = OpTypeInt 32 1
	   %v2int = OpTypeVector %int 2
     %uint_0 = OpConstant %uint 0
%_ptr_Output_float = OpTypePointer Output %float
     %uint_1 = OpConstant %uint 1
     %uint_2 = OpConstant %uint 2
     %uint_3 = OpConstant %uint 3
       %main = OpFunction %void None %3
          %5 = OpLabel
         %14 = OpLoad %11 %uSamp
         %18 = OpLoad %v4float %gl_FragCoord
         %19 = OpVectorShuffle %v2float %18 %18 0 1
         %22 = OpConvertFToS %v2int %19
         %24 = OpImage %10 %14
         %25 = OpImageFetch %v4float %24 %22 Sample %uint_0
         %28 = OpCompositeExtract %float %25 0
         %30 = OpAccessChain %_ptr_Output_float %FragColor %uint_0
               OpStore %30 %28
         %36 = OpImage %10 %14
         %37 = OpImageFetch %v4float %36 %22 Sample %uint_1
         %38 = OpCompositeExtract %float %37 0
         %40 = OpAccessChain %_ptr_Output_float %FragColor %uint_1
               OpStore %40 %38
         %46 = OpImage %10 %14
         %47 = OpImageFetch %v4float %46 %22 Sample %uint_2
         %48 = OpCompositeExtract %float %47 0
         %50 = OpAccessChain %_ptr_Output_float %FragColor %uint_2
               OpStore %50 %48
         %56 = OpImage %10 %14
         %57 = OpImageFetch %v4float %56 %22 Sample %uint_3
         %58 = OpCompositeExtract %float %57 0
         %60 = OpAccessChain %_ptr_Output_float %FragColor %uint_3
               OpStore %60 %58
               OpReturn
               OpFunctionEnd
