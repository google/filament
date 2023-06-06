; SPIR-V
; Version: 1.0
; Generator: Khronos Glslang Reference Front End; 6
; Bound: 44
; Schema: 0
               OpCapability Shader
          %1 = OpExtInstImport "GLSL.std.450"
               OpMemoryModel Logical GLSL450
               OpEntryPoint Fragment %main "main" %vUV %FragColor
               OpExecutionMode %main OriginUpperLeft
               OpSource GLSL 450
               OpName %main "main"
               OpName %sample_combined_ "sample_combined("
               OpName %sample_separate_ "sample_separate("
               OpName %uShadow "uShadow"
               OpName %vUV "vUV"
               OpName %uTexture "uTexture"
               OpName %uSampler "uSampler"
               OpName %FragColor "FragColor"
               OpDecorate %uShadow DescriptorSet 0
               OpDecorate %uShadow Binding 0
               OpDecorate %vUV Location 0
               OpDecorate %uTexture DescriptorSet 0
               OpDecorate %uTexture Binding 1
               OpDecorate %uSampler DescriptorSet 0
               OpDecorate %uSampler Binding 2
               OpDecorate %FragColor Location 0
       %void = OpTypeVoid
          %3 = OpTypeFunction %void
      %float = OpTypeFloat 32
          %7 = OpTypeFunction %float
         %12 = OpTypeImage %float 2D 2 0 0 1 Unknown
         %13 = OpTypeSampledImage %12
%_ptr_UniformConstant_13 = OpTypePointer UniformConstant %13
    %uShadow = OpVariable %_ptr_UniformConstant_13 UniformConstant
    %v3float = OpTypeVector %float 3
%_ptr_Input_v3float = OpTypePointer Input %v3float
        %vUV = OpVariable %_ptr_Input_v3float Input
%_ptr_UniformConstant_25 = OpTypePointer UniformConstant %12
   %uTexture = OpVariable %_ptr_UniformConstant_25 UniformConstant
         %29 = OpTypeSampler
%_ptr_UniformConstant_29 = OpTypePointer UniformConstant %29
   %uSampler = OpVariable %_ptr_UniformConstant_29 UniformConstant
%_ptr_Output_float = OpTypePointer Output %float
  %FragColor = OpVariable %_ptr_Output_float Output
       %main = OpFunction %void None %3
          %5 = OpLabel
         %41 = OpFunctionCall %float %sample_combined_
         %42 = OpFunctionCall %float %sample_separate_
         %43 = OpFAdd %float %41 %42
               OpStore %FragColor %43
               OpReturn
               OpFunctionEnd
%sample_combined_ = OpFunction %float None %7
          %9 = OpLabel
         %16 = OpLoad %13 %uShadow
         %20 = OpLoad %v3float %vUV
         %21 = OpCompositeExtract %float %20 2
         %22 = OpImageSampleDrefImplicitLod %float %16 %20 %21
               OpReturnValue %22
               OpFunctionEnd
%sample_separate_ = OpFunction %float None %7
         %11 = OpLabel
         %28 = OpLoad %12 %uTexture
         %32 = OpLoad %29 %uSampler
         %33 = OpSampledImage %13 %28 %32
         %34 = OpLoad %v3float %vUV
         %35 = OpCompositeExtract %float %34 2
         %36 = OpImageSampleDrefImplicitLod %float %33 %34 %35
               OpReturnValue %36
               OpFunctionEnd
