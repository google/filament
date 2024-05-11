; SPIR-V
; Version: 1.0
; Generator: Khronos Glslang Reference Front End; 1
; Bound: 38
; Schema: 0
               OpCapability Shader
          %1 = OpExtInstImport "GLSL.std.450"
               OpMemoryModel Logical GLSL450
               OpEntryPoint Fragment %main "main" %_entryPointOutput
               OpExecutionMode %main OriginUpperLeft
               OpSource HLSL 500
               OpName %main "main"
               OpName %_main_ "@main("
               OpName %pointLightShadowMap "pointLightShadowMap"
               OpName %shadowSamplerPCF "shadowSamplerPCF"
               OpName %_entryPointOutput "@entryPointOutput"
               OpDecorate %pointLightShadowMap DescriptorSet 0
               OpDecorate %shadowSamplerPCF DescriptorSet 0
               OpDecorate %pointLightShadowMap Binding 0
               OpDecorate %shadowSamplerPCF Binding 1
               OpDecorate %_entryPointOutput Location 0
       %void = OpTypeVoid
          %3 = OpTypeFunction %void
      %float = OpTypeFloat 32
          %7 = OpTypeFunction %float
         %10 = OpTypeImage %float Cube 0 0 0 1 Unknown
%_ptr_UniformConstant_10 = OpTypePointer UniformConstant %10
%pointLightShadowMap = OpVariable %_ptr_UniformConstant_10 UniformConstant
         %14 = OpTypeSampler
%_ptr_UniformConstant_14 = OpTypePointer UniformConstant %14
%shadowSamplerPCF = OpVariable %_ptr_UniformConstant_14 UniformConstant
         %18 = OpTypeImage %float Cube 1 0 0 1 Unknown
         %19 = OpTypeSampledImage %18
    %v3float = OpTypeVector %float 3
  %float_0_1 = OpConstant %float 0.1
         %23 = OpConstantComposite %v3float %float_0_1 %float_0_1 %float_0_1
  %float_0_5 = OpConstant %float 0.5
    %v4float = OpTypeVector %float 4
    %float_0 = OpConstant %float 0
%_ptr_Output_float = OpTypePointer Output %float
%_entryPointOutput = OpVariable %_ptr_Output_float Output
       %main = OpFunction %void None %3
          %5 = OpLabel
         %37 = OpFunctionCall %float %_main_
               OpStore %_entryPointOutput %37
               OpReturn
               OpFunctionEnd
     %_main_ = OpFunction %float None %7
          %9 = OpLabel
         %13 = OpLoad %10 %pointLightShadowMap
         %17 = OpLoad %14 %shadowSamplerPCF
         %20 = OpSampledImage %19 %13 %17
         %26 = OpCompositeExtract %float %23 0
         %27 = OpCompositeExtract %float %23 1
         %28 = OpCompositeExtract %float %23 2
         %29 = OpCompositeConstruct %v4float %26 %27 %28 %float_0_5
         %31 = OpCompositeExtract %float %29 3
         %32 = OpImageSampleDrefExplicitLod %float %20 %29 %31 Lod %float_0
               OpReturnValue %32
               OpFunctionEnd
