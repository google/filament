; SPIR-V
; Version: 1.0
; Generator: Khronos Glslang Reference Front End; 1
; Bound: 50
; Schema: 0
               OpCapability Shader
          %1 = OpExtInstImport "GLSL.std.450"
               OpMemoryModel Logical GLSL450
               OpEntryPoint Fragment %main "main" %FragColor %vUV %vLod
               OpExecutionMode %main OriginUpperLeft
               OpName %main "main"
               OpName %FragColor "FragColor"
               OpName %uShadow2DArray "uShadow2DArray"
               OpName %vUV "vUV"
               OpName %vLod "vLod"
               OpDecorate %FragColor Location 0
               OpDecorate %uShadow2DArray DescriptorSet 0
               OpDecorate %uShadow2DArray Binding 0
               OpDecorate %vUV Location 0
               OpDecorate %vLod Location 1
       %void = OpTypeVoid
          %3 = OpTypeFunction %void
      %float = OpTypeFloat 32
    %v4float = OpTypeVector %float 4
%_ptr_Output_v4float = OpTypePointer Output %v4float
  %FragColor = OpVariable %_ptr_Output_v4float Output
         %10 = OpTypeImage %float 2D 1 1 0 1 Unknown
         %11 = OpTypeSampledImage %10
%_ptr_UniformConstant_11 = OpTypePointer UniformConstant %11
%uShadow2DArray = OpVariable %_ptr_UniformConstant_11 UniformConstant
%_ptr_Input_v4float = OpTypePointer Input %v4float
        %vUV = OpVariable %_ptr_Input_v4float Input
%_ptr_Input_float = OpTypePointer Input %float
       %vLod = OpVariable %_ptr_Input_float Input
       %main = OpFunction %void None %3
          %5 = OpLabel
         %14 = OpLoad %11 %uShadow2DArray
         %17 = OpLoad %v4float %vUV
         %19 = OpLoad %float %vLod
         %20 = OpCompositeExtract %float %17 3
         %21 = OpImageSampleDrefExplicitLod %float %14 %17 %20 Lod %19
         %22 = OpCompositeConstruct %v4float %21 %21 %21 %21
               OpStore %FragColor %22
               OpReturn
               OpFunctionEnd
