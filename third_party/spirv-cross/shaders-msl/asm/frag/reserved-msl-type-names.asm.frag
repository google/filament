               OpCapability Shader
          %1 = OpExtInstImport "GLSL.std.450"
               OpMemoryModel Logical GLSL450
               OpEntryPoint Fragment %main "main" %FragColor %vUV
               OpExecutionMode %main OriginUpperLeft
               OpSource GLSL 450
               OpName %main "main"
               OpName %FragColor "FragColor"
               OpName %texA "sampler"
               OpName %vUV "vUV"
               OpName %texB "depth2d"
               OpDecorate %FragColor Location 0
               OpDecorate %texA Binding 0
               OpDecorate %texA DescriptorSet 0
               OpDecorate %vUV Location 0
               OpDecorate %texB Binding 1
               OpDecorate %texB DescriptorSet 0
       %void = OpTypeVoid
          %3 = OpTypeFunction %void
      %float = OpTypeFloat 32
    %v4float = OpTypeVector %float 4
%_ptr_Output_v4float = OpTypePointer Output %v4float
  %FragColor = OpVariable %_ptr_Output_v4float Output
         %10 = OpTypeImage %float 2D 0 0 0 1 Unknown
         %11 = OpTypeSampledImage %10
%_ptr_UniformConstant_11 = OpTypePointer UniformConstant %11
       %texA = OpVariable %_ptr_UniformConstant_11 UniformConstant
    %v2float = OpTypeVector %float 2
%_ptr_Input_v2float = OpTypePointer Input %v2float
        %vUV = OpVariable %_ptr_Input_v2float Input
       %texB = OpVariable %_ptr_UniformConstant_11 UniformConstant
       %main = OpFunction %void None %3
          %5 = OpLabel
         %14 = OpLoad %11 %texA
         %18 = OpLoad %v2float %vUV
         %19 = OpImageSampleImplicitLod %v4float %14 %18
         %21 = OpLoad %11 %texB
         %22 = OpLoad %v2float %vUV
         %23 = OpImageSampleImplicitLod %v4float %21 %22
         %24 = OpFAdd %v4float %19 %23
               OpStore %FragColor %24
               OpReturn
               OpFunctionEnd
