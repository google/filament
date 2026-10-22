; SPIR-V
; Version: 1.0
; Generator: Google spiregg; 0
; Bound: 32
; Schema: 0
               OpCapability Shader
               OpMemoryModel Logical GLSL450
               OpEntryPoint Fragment %main "main" %in_var_TEXCOORD0 %out_var_SV_Target
               OpExecutionMode %main OriginUpperLeft
               OpSource HLSL 600
               OpName %type_2d_image "type.2d.image"
               OpName %g_Texture "g_Texture"
               OpName %type_sampler "type.sampler"
               OpName %g_Sampler "g_Sampler"
               OpName %g_CompareSampler "g_CompareSampler"
               OpName %in_var_TEXCOORD0 "in.var.TEXCOORD0"
               OpName %out_var_SV_Target "out.var.SV_Target"
               OpName %main "main"
               OpName %type_sampled_image "type.sampled.image"
               OpDecorate %in_var_TEXCOORD0 Location 0
               OpDecorate %out_var_SV_Target Location 0
               OpDecorate %g_Texture DescriptorSet 0
               OpDecorate %g_Texture Binding 0
               OpDecorate %g_Sampler DescriptorSet 0
               OpDecorate %g_Sampler Binding 0
               OpDecorate %g_CompareSampler DescriptorSet 0
               OpDecorate %g_CompareSampler Binding 1
      %float = OpTypeFloat 32
  %float_0_5 = OpConstant %float 0.5
    %float_0 = OpConstant %float 0
%type_2d_image = OpTypeImage %float 2D 2 0 0 1 Unknown
%_ptr_UniformConstant_type_2d_image = OpTypePointer UniformConstant %type_2d_image
%type_sampler = OpTypeSampler
%_ptr_UniformConstant_type_sampler = OpTypePointer UniformConstant %type_sampler
    %v2float = OpTypeVector %float 2
%_ptr_Input_v2float = OpTypePointer Input %v2float
%_ptr_Output_float = OpTypePointer Output %float
       %void = OpTypeVoid
         %19 = OpTypeFunction %void
%type_sampled_image = OpTypeSampledImage %type_2d_image
    %v4float = OpTypeVector %float 4
  %g_Texture = OpVariable %_ptr_UniformConstant_type_2d_image UniformConstant
  %g_Sampler = OpVariable %_ptr_UniformConstant_type_sampler UniformConstant
%g_CompareSampler = OpVariable %_ptr_UniformConstant_type_sampler UniformConstant
%in_var_TEXCOORD0 = OpVariable %_ptr_Input_v2float Input
%out_var_SV_Target = OpVariable %_ptr_Output_float Output
       %main = OpFunction %void None %19
         %21 = OpLabel
         %22 = OpLoad %v2float %in_var_TEXCOORD0
         %23 = OpLoad %type_2d_image %g_Texture
         %24 = OpLoad %type_sampler %g_Sampler
         %25 = OpSampledImage %type_sampled_image %23 %24
         %26 = OpImageSampleImplicitLod %v4float %25 %22 None
         %27 = OpCompositeExtract %float %26 0
         %28 = OpLoad %type_sampler %g_CompareSampler
         %29 = OpSampledImage %type_sampled_image %23 %28
         %30 = OpImageSampleDrefExplicitLod %float %29 %22 %float_0_5 Lod %float_0
         %31 = OpFAdd %float %27 %30
               OpStore %out_var_SV_Target %31
               OpReturn
               OpFunctionEnd
