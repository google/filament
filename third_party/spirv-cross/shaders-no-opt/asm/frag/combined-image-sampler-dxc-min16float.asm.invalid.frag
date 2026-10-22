; SPIR-V
; Version: 1.0
; Generator: Google spiregg; 0
; Bound: 48
; Schema: 0
               OpCapability Shader
               OpMemoryModel Logical GLSL450
               OpEntryPoint Fragment %PSMain "main" %in_var_COLOR %in_var_TEXCOORD0 %out_var_SV_TARGET
               OpExecutionMode %PSMain OriginUpperLeft
			   ; Not actually ESSL, but makes testing easier.
               OpSource ESSL 310 
               OpName %type_2d_image "type.2d.image"
               OpName %tex "tex"
               OpName %type_sampler "type.sampler"
               OpName %Samp "Samp"
               OpName %in_var_COLOR "in.var.COLOR"
               OpName %in_var_TEXCOORD0 "in.var.TEXCOORD0"
               OpName %out_var_SV_TARGET "out.var.SV_TARGET"
               OpName %PSMain "PSMain"
               OpName %PSInput "PSInput"
               OpMemberName %PSInput 0 "color"
               OpMemberName %PSInput 1 "uv"
               OpName %param_var_input "param.var.input"
               OpName %src_PSMain "src.PSMain"
               OpName %input "input"
               OpName %bb_entry "bb.entry"
               OpName %a "a"
               OpName %type_sampled_image "type.sampled.image"
               OpDecorate %in_var_COLOR Location 0
               OpDecorate %in_var_TEXCOORD0 Location 1
               OpDecorate %out_var_SV_TARGET Location 0
               OpDecorate %tex DescriptorSet 0
               OpDecorate %tex Binding 0
               OpDecorate %Samp DescriptorSet 0
               OpDecorate %Samp Binding 1
               OpDecorate %tex RelaxedPrecision
               OpDecorate %a RelaxedPrecision
               OpDecorate %38 RelaxedPrecision
               OpDecorate %45 RelaxedPrecision
               OpDecorate %47 RelaxedPrecision
        %int = OpTypeInt 32 1
      %int_0 = OpConstant %int 0
      %int_1 = OpConstant %int 1
      %float = OpTypeFloat 32
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
         %21 = OpTypeFunction %void
    %PSInput = OpTypeStruct %v4float %v2float
%_ptr_Function_PSInput = OpTypePointer Function %PSInput
         %31 = OpTypeFunction %v4float %_ptr_Function_PSInput
%_ptr_Function_v4float = OpTypePointer Function %v4float
%_ptr_Function_v2float = OpTypePointer Function %v2float
%type_sampled_image = OpTypeSampledImage %type_2d_image
        %tex = OpVariable %_ptr_UniformConstant_type_2d_image UniformConstant
       %Samp = OpVariable %_ptr_UniformConstant_type_sampler UniformConstant
%in_var_COLOR = OpVariable %_ptr_Input_v4float Input
%in_var_TEXCOORD0 = OpVariable %_ptr_Input_v2float Input
%out_var_SV_TARGET = OpVariable %_ptr_Output_v4float Output
     %PSMain = OpFunction %void None %21
         %22 = OpLabel
%param_var_input = OpVariable %_ptr_Function_PSInput Function
         %26 = OpLoad %v4float %in_var_COLOR
         %27 = OpLoad %v2float %in_var_TEXCOORD0
         %28 = OpCompositeConstruct %PSInput %26 %27
               OpStore %param_var_input %28
         %29 = OpFunctionCall %v4float %src_PSMain %param_var_input
               OpStore %out_var_SV_TARGET %29
               OpReturn
               OpFunctionEnd
 %src_PSMain = OpFunction %v4float None %31
      %input = OpFunctionParameter %_ptr_Function_PSInput
   %bb_entry = OpLabel
          %a = OpVariable %_ptr_Function_v4float Function
         %36 = OpAccessChain %_ptr_Function_v4float %input %int_0
         %37 = OpLoad %v4float %36
         %38 = OpLoad %type_2d_image %tex
         %39 = OpLoad %type_sampler %Samp
         %41 = OpAccessChain %_ptr_Function_v2float %input %int_1
         %42 = OpLoad %v2float %41
         %44 = OpSampledImage %type_sampled_image %38 %39
         %45 = OpImageSampleImplicitLod %v4float %44 %42 None
         %46 = OpFMul %v4float %37 %45
               OpStore %a %46
         %47 = OpLoad %v4float %a
               OpReturnValue %47
               OpFunctionEnd

