; SPIR-V
; Version: 1.3
; Generator: Google spiregg; 0
; Bound: 36
; Schema: 0
               OpCapability Shader
               OpExtension "SPV_GOOGLE_hlsl_functionality1"
               OpExtension "SPV_GOOGLE_user_type"
               OpMemoryModel Logical GLSL450
               OpEntryPoint Fragment %psMain "main" %gl_FragCoord %in_var_TEXCOORD0 %out_var_SV_Target0
               OpExecutionMode %psMain OriginUpperLeft
               OpSource HLSL 500
               OpName %type_2d_image "type.2d.image"
               OpName %g_texture "g_texture"
               OpName %type_sampler "type.sampler"
               OpName %g_sampler "g_sampler"
               OpName %g_comp "g_comp"
               OpName %in_var_TEXCOORD0 "in.var.TEXCOORD0"
               OpName %out_var_SV_Target0 "out.var.SV_Target0"
               OpName %psMain "psMain"
               OpName %type_sampled_image "type.sampled.image"
               OpDecorate %gl_FragCoord BuiltIn FragCoord
               OpDecorateString %gl_FragCoord UserSemantic "SV_Position"
               OpDecorateString %in_var_TEXCOORD0 UserSemantic "TEXCOORD0"
               OpDecorateString %out_var_SV_Target0 UserSemantic "SV_Target0"
               OpDecorate %in_var_TEXCOORD0 Location 0
               OpDecorate %out_var_SV_Target0 Location 0
               OpDecorate %g_texture DescriptorSet 0
               OpDecorate %g_texture Binding 0
               OpDecorate %g_sampler DescriptorSet 0
               OpDecorate %g_sampler Binding 0
               OpDecorate %g_comp DescriptorSet 0
               OpDecorate %g_comp Binding 1
               OpDecorateString %g_texture UserTypeGOOGLE "texture2d"
      %float = OpTypeFloat 32
  %float_0_5 = OpConstant %float 0.5
        %int = OpTypeInt 32 1
      %int_0 = OpConstant %int 0
      %int_1 = OpConstant %int 1
      %v2int = OpTypeVector %int 2
         %16 = OpConstantComposite %v2int %int_0 %int_0
%type_2d_image = OpTypeImage %float 2D 2 0 0 1 Unknown
%_ptr_UniformConstant_type_2d_image = OpTypePointer UniformConstant %type_2d_image
%type_sampler = OpTypeSampler
%_ptr_UniformConstant_type_sampler = OpTypePointer UniformConstant %type_sampler
    %v4float = OpTypeVector %float 4
%_ptr_Input_v4float = OpTypePointer Input %v4float
    %v2float = OpTypeVector %float 2
%_ptr_Input_v2float = OpTypePointer Input %v2float
%_ptr_Output_v4float = OpTypePointer Output %v4float
       %void = OpTypeVoid
         %25 = OpTypeFunction %void
%type_sampled_image = OpTypeSampledImage %type_2d_image
%g_texture = OpVariable %_ptr_UniformConstant_type_2d_image UniformConstant
  %g_sampler = OpVariable %_ptr_UniformConstant_type_sampler UniformConstant
     %g_comp = OpVariable %_ptr_UniformConstant_type_sampler UniformConstant
%gl_FragCoord = OpVariable %_ptr_Input_v4float Input
%in_var_TEXCOORD0 = OpVariable %_ptr_Input_v2float Input
%out_var_SV_Target0 = OpVariable %_ptr_Output_v4float Output
     %psMain = OpFunction %void None %25
         %26 = OpLabel
         %27 = OpLoad %v2float %in_var_TEXCOORD0
         %28 = OpLoad %type_2d_image %g_texture
         %29 = OpLoad %type_sampler %g_comp
         %30 = OpSampledImage %type_sampled_image %28 %29
         %32 = OpLoad %type_sampler %g_sampler
         %33 = OpSampledImage %type_sampled_image %28 %32
         %31 = OpImageGather %v4float %33 %27 %int_1 ConstOffset %16
         %34 = OpImageGather %v4float %33 %27 %int_0 ConstOffset %16
         %35 = OpFMul %v4float %34 %31
               OpStore %out_var_SV_Target0 %35
               OpReturn
               OpFunctionEnd

