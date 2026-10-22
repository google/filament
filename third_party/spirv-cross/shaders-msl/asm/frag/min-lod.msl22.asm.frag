; SPIR-V
; Version: 1.0
; Generator: Khronos Glslang Reference Front End; 7
; Bound: 21
; Schema: 0
               OpCapability Shader
			   OpCapability MinLod
          %1 = OpExtInstImport "GLSL.std.450"
               OpMemoryModel Logical GLSL450
               OpEntryPoint Fragment %main "main" %FragColor %vUV
               OpExecutionMode %main OriginUpperLeft
               OpSource GLSL 450
               OpName %main "main"
               OpName %FragColor "FragColor"
               OpName %uSampler "uSampler"
               OpName %vUV "vUV"
               OpDecorate %FragColor Location 0
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
         %11 = OpTypeSampledImage %10
%_ptr_UniformConstant_11 = OpTypePointer UniformConstant %11
   %uSampler = OpVariable %_ptr_UniformConstant_11 UniformConstant
    %v2float = OpTypeVector %float 2
%_ptr_Input_v2float = OpTypePointer Input %v2float
        %vUV = OpVariable %_ptr_Input_v2float Input
    %float_4 = OpConstant %float 4
       %main = OpFunction %void None %3
          %5 = OpLabel
         %14 = OpLoad %11 %uSampler
         %18 = OpLoad %v2float %vUV
         %20 = OpImageSampleImplicitLod %v4float %14 %18 MinLod %float_4
               OpStore %FragColor %20
               OpReturn
               OpFunctionEnd
