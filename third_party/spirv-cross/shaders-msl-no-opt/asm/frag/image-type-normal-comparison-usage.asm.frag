; SPIR-V
; Version: 1.0
; Generator: Google spiregg; 0
; Bound: 43
; Schema: 0
               OpCapability Shader
               OpMemoryModel Logical GLSL450
               OpEntryPoint Fragment %main "main" %in_var_TEXCOORD0 %out_var_SV_Target0
               OpExecutionMode %main OriginUpperLeft
               OpSource HLSL 600
               OpName %type_2d_image "type.2d.image"
               OpName %ShadowMap "ShadowMap"
               OpName %type_sampler "type.sampler"
               OpName %SampleNormal "SampleNormal"
               OpName %SampleShadow "SampleShadow"
               OpName %in_var_TEXCOORD0 "in.var.TEXCOORD0"
               OpName %out_var_SV_Target0 "out.var.SV_Target0"
               OpName %main "main"
               OpName %type_sampled_image "type.sampled.image"
               OpDecorate %in_var_TEXCOORD0 Location 0
               OpDecorate %out_var_SV_Target0 Location 0
               OpDecorate %ShadowMap DescriptorSet 0
               OpDecorate %ShadowMap Binding 0
               OpDecorate %SampleNormal DescriptorSet 0
               OpDecorate %SampleNormal Binding 0
               OpDecorate %SampleShadow DescriptorSet 0
               OpDecorate %SampleShadow Binding 1
      %float = OpTypeFloat 32
  %float_0_5 = OpConstant %float 0.5
    %float_1 = OpConstant %float 1
    %float_0 = OpConstant %float 0
    %v4float = OpTypeVector %float 4
%type_2d_image = OpTypeImage %float 2D 2 0 0 1 Unknown
%_ptr_UniformConstant_type_2d_image = OpTypePointer UniformConstant %type_2d_image
%type_sampler = OpTypeSampler
%_ptr_UniformConstant_type_sampler = OpTypePointer UniformConstant %type_sampler
    %v2float = OpTypeVector %float 2
%_ptr_Input_v2float = OpTypePointer Input %v2float
%_ptr_Output_v4float = OpTypePointer Output %v4float
       %void = OpTypeVoid
         %21 = OpTypeFunction %void
       %bool = OpTypeBool
%type_sampled_image = OpTypeSampledImage %type_2d_image
  %ShadowMap = OpVariable %_ptr_UniformConstant_type_2d_image UniformConstant
%SampleNormal = OpVariable %_ptr_UniformConstant_type_sampler UniformConstant
%SampleShadow = OpVariable %_ptr_UniformConstant_type_sampler UniformConstant
%in_var_TEXCOORD0 = OpVariable %_ptr_Input_v2float Input
%out_var_SV_Target0 = OpVariable %_ptr_Output_v4float Output
       %main = OpFunction %void None %21
         %23 = OpLabel
         %24 = OpLoad %v2float %in_var_TEXCOORD0
         %25 = OpCompositeExtract %float %24 0
         %26 = OpFOrdGreaterThan %bool %25 %float_0_5
               OpSelectionMerge %27 None
               OpBranchConditional %26 %28 %29
         %28 = OpLabel
         %30 = OpLoad %type_2d_image %ShadowMap
         %31 = OpLoad %type_sampler %SampleNormal
         %32 = OpSampledImage %type_sampled_image %30 %31
         %33 = OpImageSampleImplicitLod %v4float %32 %24 None
         %34 = OpCompositeExtract %float %33 0
         %35 = OpFOrdLessThanEqual %bool %34 %float_0_5
         %36 = OpSelect %float %35 %float_1 %float_0
               OpBranch %27
         %29 = OpLabel
         %37 = OpLoad %type_2d_image %ShadowMap
         %38 = OpLoad %type_sampler %SampleShadow
         %39 = OpSampledImage %type_sampled_image %37 %38
         %40 = OpImageSampleDrefExplicitLod %float %39 %24 %float_0_5 Lod %float_0
               OpBranch %27
         %27 = OpLabel
         %41 = OpPhi %float %36 %28 %40 %29
         %42 = OpCompositeConstruct %v4float %41 %41 %41 %float_1
               OpStore %out_var_SV_Target0 %42
               OpReturn
               OpFunctionEnd
