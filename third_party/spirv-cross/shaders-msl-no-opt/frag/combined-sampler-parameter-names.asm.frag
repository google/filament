; SPIR-V
; Version: 1.0
; Generator: Khronos Glslang Reference Front End; 11
; Bound: 26
; Schema: 0
               OpCapability Shader
          %1 = OpExtInstImport "GLSL.std.450"
               OpMemoryModel Logical GLSL450
               OpEntryPoint Fragment %main "main" %UV %FragColor
               OpExecutionMode %main OriginUpperLeft
               OpSource GLSL 450
               OpDecorate %uSamp DescriptorSet 0
               OpDecorate %uSamp Binding 0
               OpDecorate %UV Location 0
               OpDecorate %FragColor Location 0
       %void = OpTypeVoid
          %3 = OpTypeFunction %void
      %float = OpTypeFloat 32
    %v4float = OpTypeVector %float 4
          %8 = OpTypeFunction %v4float
         %11 = OpTypeImage %float 2D 0 0 0 1 Unknown
         %12 = OpTypeSampledImage %11
%_ptr_UniformConstant_12 = OpTypePointer UniformConstant %12
      %uSamp = OpVariable %_ptr_UniformConstant_12 UniformConstant
    %v2float = OpTypeVector %float 2
%_ptr_Input_v2float = OpTypePointer Input %v2float
         %UV = OpVariable %_ptr_Input_v2float Input
%_ptr_Output_v4float = OpTypePointer Output %v4float
  %FragColor = OpVariable %_ptr_Output_v4float Output
       %main = OpFunction %void None %3
          %5 = OpLabel
         %25 = OpFunctionCall %v4float %samp_
               OpStore %FragColor %25
               OpReturn
               OpFunctionEnd
      %samp_ = OpFunction %v4float None %8
         %10 = OpLabel
         %15 = OpLoad %12 %uSamp
         %19 = OpLoad %v2float %UV
         %20 = OpImageSampleImplicitLod %v4float %15 %19
               OpReturnValue %20
               OpFunctionEnd
