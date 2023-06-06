; SPIR-V
; Version: 1.0
; Generator: Google spiregg; 0
; Bound: 29
; Schema: 0
               OpCapability Shader
               OpMemoryModel Logical GLSL450
               OpEntryPoint Fragment %main "main" %out_var_SV_Target
               OpExecutionMode %main OriginUpperLeft
               OpSource HLSL 600
               OpName %type_Foo "type.Foo"
               OpMemberName %type_Foo 0 "a"
               OpMemberName %type_Foo 1 "b"
               OpName %Foo "Foo"
               OpName %out_var_SV_Target "out.var.SV_Target"
               OpName %main "main"
               OpDecorate %out_var_SV_Target Location 0
               OpDecorate %Foo DescriptorSet 0
               OpDecorate %Foo Binding 0
               OpDecorate %_arr_v3float_uint_2 ArrayStride 16
               OpMemberDecorate %type_Foo 0 Offset 0
               OpMemberDecorate %type_Foo 1 Offset 28
               OpDecorate %type_Foo Block
        %int = OpTypeInt 32 1
      %int_0 = OpConstant %int 0
      %int_1 = OpConstant %int 1
       %uint = OpTypeInt 32 0
     %uint_2 = OpConstant %uint 2
      %float = OpTypeFloat 32
    %v3float = OpTypeVector %float 3
%_arr_v3float_uint_2 = OpTypeArray %v3float %uint_2
   %type_Foo = OpTypeStruct %_arr_v3float_uint_2 %float
%_ptr_Uniform_type_Foo = OpTypePointer Uniform %type_Foo
%_ptr_Output_v3float = OpTypePointer Output %v3float
       %void = OpTypeVoid
         %16 = OpTypeFunction %void
%_ptr_Uniform_v3float = OpTypePointer Uniform %v3float
%_ptr_Uniform_float = OpTypePointer Uniform %float
        %Foo = OpVariable %_ptr_Uniform_type_Foo Uniform
%out_var_SV_Target = OpVariable %_ptr_Output_v3float Output
       %main = OpFunction %void None %16
         %19 = OpLabel
         %20 = OpAccessChain %_ptr_Uniform_v3float %Foo %int_0 %int_0
         %21 = OpLoad %v3float %20
         %22 = OpAccessChain %_ptr_Uniform_v3float %Foo %int_0 %int_1
         %23 = OpLoad %v3float %22
         %24 = OpFAdd %v3float %21 %23
         %25 = OpAccessChain %_ptr_Uniform_float %Foo %int_1
         %26 = OpLoad %float %25
         %27 = OpCompositeConstruct %v3float %26 %26 %26
         %28 = OpFAdd %v3float %24 %27
               OpStore %out_var_SV_Target %28
               OpReturn
               OpFunctionEnd
