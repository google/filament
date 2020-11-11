; SPIR-V
; Version: 1.0
; Generator: Google spiregg; 0
; Bound: 43
; Schema: 0
               OpCapability Shader
               OpMemoryModel Logical GLSL450
               OpEntryPoint Fragment %frag "main" %gl_FragCoord %out_var_SV_Target
               OpExecutionMode %frag OriginUpperLeft
               OpSource HLSL 600
               OpName %type__Globals "type.$Globals"
               OpMemberName %type__Globals 0 "_BorderWidths"
               OpName %_Globals "$Globals"
               OpName %out_var_SV_Target "out.var.SV_Target"
               OpName %frag "frag"
               OpDecorate %gl_FragCoord BuiltIn FragCoord
               OpDecorate %out_var_SV_Target Location 0
               OpDecorate %_Globals DescriptorSet 0
               OpDecorate %_Globals Binding 0
               OpDecorate %_arr_float_uint_4 ArrayStride 16
               OpMemberDecorate %type__Globals 0 Offset 0
               OpDecorate %type__Globals Block
        %int = OpTypeInt 32 1
      %int_0 = OpConstant %int 0
      %int_1 = OpConstant %int 1
      %float = OpTypeFloat 32
    %float_0 = OpConstant %float 0
      %int_2 = OpConstant %int 2
       %uint = OpTypeInt 32 0
    %float_1 = OpConstant %float 1
     %uint_4 = OpConstant %uint 4
%_arr_float_uint_4 = OpTypeArray %float %uint_4
%type__Globals = OpTypeStruct %_arr_float_uint_4
%_ptr_Uniform_type__Globals = OpTypePointer Uniform %type__Globals
    %v4float = OpTypeVector %float 4
%_ptr_Input_v4float = OpTypePointer Input %v4float
%_ptr_Output_v4float = OpTypePointer Output %v4float
       %void = OpTypeVoid
         %21 = OpTypeFunction %void
    %v2float = OpTypeVector %float 2
%_ptr_Uniform_float = OpTypePointer Uniform %float
       %bool = OpTypeBool
   %_Globals = OpVariable %_ptr_Uniform_type__Globals Uniform
%gl_FragCoord = OpVariable %_ptr_Input_v4float Input
%out_var_SV_Target = OpVariable %_ptr_Output_v4float Output
       %frag = OpFunction %void None %21
         %25 = OpLabel
         %26 = OpLoad %v4float %gl_FragCoord
         %27 = OpAccessChain %_ptr_Uniform_float %_Globals %int_0 %int_0
         %28 = OpLoad %float %27
         %29 = OpAccessChain %_ptr_Uniform_float %_Globals %int_0 %int_1
         %30 = OpLoad %float %29
         %31 = OpCompositeConstruct %v2float %28 %30
         %32 = OpCompositeExtract %float %26 0
         %33 = OpFOrdGreaterThan %bool %32 %float_0
               OpSelectionMerge %34 None
               OpBranchConditional %33 %35 %34
         %35 = OpLabel
         %36 = OpAccessChain %_ptr_Uniform_float %_Globals %int_0 %int_2
         %37 = OpLoad %float %36
         %38 = OpCompositeInsert %v2float %37 %31 0
               OpBranch %34
         %34 = OpLabel
         %39 = OpPhi %v2float %31 %25 %38 %35
         %40 = OpCompositeExtract %float %39 0
         %41 = OpCompositeExtract %float %39 1
         %42 = OpCompositeConstruct %v4float %40 %41 %float_0 %float_1
               OpStore %out_var_SV_Target %42
               OpReturn
               OpFunctionEnd
