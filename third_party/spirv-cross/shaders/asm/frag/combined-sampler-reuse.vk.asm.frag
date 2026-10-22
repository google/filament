; SPIR-V
; Version: 1.0
; Generator: Khronos Glslang Reference Front End; 6
; Bound: 36
; Schema: 0
               OpCapability Shader
          %1 = OpExtInstImport "GLSL.std.450"
               OpMemoryModel Logical GLSL450
               OpEntryPoint Fragment %main "main" %FragColor %vUV
               OpExecutionMode %main OriginUpperLeft
               OpSource GLSL 450
               OpName %main "main"
               OpName %FragColor "FragColor"
               OpName %uTex "uTex"
               OpName %uSampler "uSampler"
               OpName %vUV "vUV"
               OpDecorate %FragColor Location 0
               OpDecorate %uTex DescriptorSet 0
               OpDecorate %uTex Binding 1
               OpDecorate %uSampler DescriptorSet 0
               OpDecorate %uSampler Binding 0
               OpDecorate %vUV Location 0
       %void = OpTypeVoid
          %3 = OpTypeFunction %void
      %float = OpTypeFloat 32
    %v4float = OpTypeVector %float 4
%_ptr_Output_v4float = OpTypePointer Output %v4float
  %FragColor = OpVariable %_ptr_Output_v4float Output
         %10 = OpTypeImage %float 2D 0 0 0 1 Unknown
%_ptr_UniformConstant_10 = OpTypePointer UniformConstant %10
       %uTex = OpVariable %_ptr_UniformConstant_10 UniformConstant
         %14 = OpTypeSampler
%_ptr_UniformConstant_14 = OpTypePointer UniformConstant %14
   %uSampler = OpVariable %_ptr_UniformConstant_14 UniformConstant
         %18 = OpTypeSampledImage %10
    %v2float = OpTypeVector %float 2
%_ptr_Input_v2float = OpTypePointer Input %v2float
        %vUV = OpVariable %_ptr_Input_v2float Input
        %int = OpTypeInt 32 1
      %v2int = OpTypeVector %int 2
      %int_1 = OpConstant %int 1
         %32 = OpConstantComposite %v2int %int_1 %int_1
       %main = OpFunction %void None %3
          %5 = OpLabel
         %13 = OpLoad %10 %uTex
         %17 = OpLoad %14 %uSampler
         %19 = OpSampledImage %18 %13 %17
         %23 = OpLoad %v2float %vUV
         %24 = OpImageSampleImplicitLod %v4float %19 %23
               OpStore %FragColor %24
         %28 = OpLoad %v2float %vUV
         %33 = OpImageSampleImplicitLod %v4float %19 %28 ConstOffset %32
         %34 = OpLoad %v4float %FragColor
         %35 = OpFAdd %v4float %34 %33
               OpStore %FragColor %35
               OpReturn
               OpFunctionEnd
