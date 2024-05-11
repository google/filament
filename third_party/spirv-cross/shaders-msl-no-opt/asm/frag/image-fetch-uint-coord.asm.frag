; SPIR-V
; Version: 1.0
; Generator: Google spiregg; 0
; Bound: 29
; Schema: 0
               OpCapability Shader
               OpMemoryModel Logical GLSL450
               OpEntryPoint Fragment %main "main" %in_var_TEXCOORD0 %out_var_SV_Target0
               OpExecutionMode %main OriginUpperLeft
               OpSource HLSL 600
               OpName %type_2d_image "type.2d.image"
               OpName %Tex "Tex"
               OpName %in_var_TEXCOORD0 "in.var.TEXCOORD0"
               OpName %out_var_SV_Target0 "out.var.SV_Target0"
               OpName %main "main"
               OpDecorate %in_var_TEXCOORD0 Flat
               OpDecorate %in_var_TEXCOORD0 Location 0
               OpDecorate %out_var_SV_Target0 Location 0
               OpDecorate %Tex DescriptorSet 0
               OpDecorate %Tex Binding 0
        %int = OpTypeInt 32 1
      %int_2 = OpConstant %int 2
      %float = OpTypeFloat 32
%type_2d_image = OpTypeImage %float 2D 2 0 0 1 Unknown
%_ptr_UniformConstant_type_2d_image = OpTypePointer UniformConstant %type_2d_image
       %uint = OpTypeInt 32 0
     %v3uint = OpTypeVector %uint 3
%_ptr_Input_v3uint = OpTypePointer Input %v3uint
    %v4float = OpTypeVector %float 4
%_ptr_Output_v4float = OpTypePointer Output %v4float
       %void = OpTypeVoid
         %16 = OpTypeFunction %void
        %Tex = OpVariable %_ptr_UniformConstant_type_2d_image UniformConstant
%in_var_TEXCOORD0 = OpVariable %_ptr_Input_v3uint Input
%out_var_SV_Target0 = OpVariable %_ptr_Output_v4float Output
       %main = OpFunction %void None %16
         %19 = OpLabel
         %20 = OpLoad %v3uint %in_var_TEXCOORD0
         %21 = OpCompositeExtract %uint %20 2
         %27 = OpLoad %type_2d_image %Tex
         %28 = OpImageFetch %v4float %27 %20 Lod %21
               OpStore %out_var_SV_Target0 %28
               OpReturn
               OpFunctionEnd
