; SPIR-V
; Version: 1.0
; Generator: Google spiregg; 0
; Bound: 30
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
               OpMemberDecorate %type_Foo 0 Offset 0
               OpMemberDecorate %type_Foo 0 MatrixStride 16
               OpMemberDecorate %type_Foo 0 RowMajor
               OpMemberDecorate %type_Foo 1 Offset 24
               OpDecorate %type_Foo Block
       %uint = OpTypeInt 32 0
     %uint_0 = OpConstant %uint 0
        %int = OpTypeInt 32 1
      %int_0 = OpConstant %int 0
     %uint_1 = OpConstant %uint 1
      %int_1 = OpConstant %int 1
      %float = OpTypeFloat 32
    %v2float = OpTypeVector %float 2
%mat2v2float = OpTypeMatrix %v2float 2
   %type_Foo = OpTypeStruct %mat2v2float %float
%_ptr_Uniform_type_Foo = OpTypePointer Uniform %type_Foo
%_ptr_Output_v2float = OpTypePointer Output %v2float
       %void = OpTypeVoid
         %17 = OpTypeFunction %void
%_ptr_Uniform_v2float = OpTypePointer Uniform %v2float
%_ptr_Uniform_float = OpTypePointer Uniform %float
        %Foo = OpVariable %_ptr_Uniform_type_Foo Uniform
%out_var_SV_Target = OpVariable %_ptr_Output_v2float Output
       %main = OpFunction %void None %17
         %20 = OpLabel
         %21 = OpAccessChain %_ptr_Uniform_v2float %Foo %int_0 %uint_0
         %22 = OpLoad %v2float %21
         %23 = OpAccessChain %_ptr_Uniform_v2float %Foo %int_0 %uint_1
         %24 = OpLoad %v2float %23
         %25 = OpFAdd %v2float %22 %24
         %26 = OpAccessChain %_ptr_Uniform_float %Foo %int_1
         %27 = OpLoad %float %26
         %28 = OpCompositeConstruct %v2float %27 %27
         %29 = OpFAdd %v2float %25 %28
               OpStore %out_var_SV_Target %29
               OpReturn
               OpFunctionEnd
