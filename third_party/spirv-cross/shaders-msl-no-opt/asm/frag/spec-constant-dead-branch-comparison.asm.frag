; SPIR-V
; Version: 1.0
; Generator: Khronos SPIR-V Tools Assembler; 0
; Bound: 55
; Schema: 0
               OpCapability Shader
               OpMemoryModel Logical GLSL450
               OpEntryPoint Fragment %main "main" %in_var_TEXCOORD0 %out_var_SV_Target0
               OpExecutionMode %main OriginUpperLeft
               OpSource HLSL 600
               OpName %type_2d_image "type.2d.image"
               OpName %ShadowMap "ShadowMap"
               OpName %type_sampler "type.sampler"
               OpName %Sampler "Sampler"
               OpName %get_sampler_type "get_sampler_type"
               OpName %in_var_TEXCOORD0 "in.var.TEXCOORD0"
               OpName %out_var_SV_Target0 "out.var.SV_Target0"
               OpName %main "main"
               OpName %type_sampled_image "type.sampled.image"
               OpDecorate %in_var_TEXCOORD0 Location 0
               OpDecorate %out_var_SV_Target0 Location 0
               OpDecorate %ShadowMap DescriptorSet 0
               OpDecorate %ShadowMap Binding 0
               OpDecorate %Sampler DescriptorSet 0
               OpDecorate %Sampler Binding 1
               OpDecorate %SamplerType SpecId 0
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
       %uint = OpTypeInt 32 0
%SamplerType = OpSpecConstant %uint 0
         %25 = OpTypeFunction %uint
       %bool = OpTypeBool
     %uint_3 = OpConstant %uint 3
%type_sampled_image = OpTypeSampledImage %type_2d_image
  %ShadowMap = OpVariable %_ptr_UniformConstant_type_2d_image UniformConstant
    %Sampler = OpVariable %_ptr_UniformConstant_type_sampler UniformConstant
%in_var_TEXCOORD0 = OpVariable %_ptr_Input_v2float Input
%out_var_SV_Target0 = OpVariable %_ptr_Output_v4float Output
%get_sampler_type = OpFunction %uint None %25
         %30 = OpLabel
               OpReturnValue %SamplerType
               OpFunctionEnd
       %main = OpFunction %void None %21
         %32 = OpLabel
         %33 = OpLoad %v2float %in_var_TEXCOORD0
         %34 = OpFunctionCall %uint %get_sampler_type
         %35 = OpIEqual %bool %34 %uint_3
               OpSelectionMerge %36 None
               OpBranchConditional %35 %37 %36
         %37 = OpLabel
         %38 = OpLoad %type_2d_image %ShadowMap
         %39 = OpLoad %type_sampler %Sampler
         %40 = OpSampledImage %type_sampled_image %38 %39
         %41 = OpImageSampleDrefExplicitLod %float %40 %33 %float_0_5 Lod %float_0
               OpBranch %36
         %36 = OpLabel
         %42 = OpLoad %type_2d_image %ShadowMap
         %43 = OpLoad %type_sampler %Sampler
         %44 = OpSampledImage %type_sampled_image %42 %43
         %45 = OpImageSampleImplicitLod %v4float %44 %33 None
               OpStore %out_var_SV_Target0 %45
               OpReturn
               OpFunctionEnd
