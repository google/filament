; SPIR-V
; Version: 1.0
; Generator: Khronos Glslang Reference Front End; 10
; Bound: 33
; Schema: 0
               OpCapability Shader
          %1 = OpExtInstImport "GLSL.std.450"
               OpMemoryModel Logical GLSL450
               OpEntryPoint Fragment %main "main" %FragColor
               OpExecutionMode %main OriginUpperLeft
               OpSource GLSL 450
               OpName %main "main"
               OpName %FragColor "FragColor"
               OpName %Foo "Foo"
               OpMemberName %Foo 0 "m"
               OpMemberName %Foo 1 "v"
               OpName %FooUBO "FooUBO"
               OpMemberName %FooUBO 0 "foo"
               OpName %_ ""
               OpName %Bar "Bar"
               OpMemberName %Bar 0 "m"
               OpMemberName %Bar 1 "v"
               OpName %BarUBO "BarUBO"
               OpMemberName %BarUBO 0 "bar"
               OpName %__0 ""
               OpDecorate %FragColor Location 0
               OpDecorate %_arr_mat3v3float_uint_2 ArrayStride 48
               OpMemberDecorate %Foo 0 ColMajor
               OpMemberDecorate %Foo 0 Offset 0
               OpMemberDecorate %Foo 0 MatrixStride 16
               OpMemberDecorate %Foo 1 Offset 92
               OpMemberDecorate %FooUBO 0 Offset 0
               OpDecorate %FooUBO Block
               OpDecorate %_ DescriptorSet 0
               OpDecorate %_ Binding 0
               OpMemberDecorate %Bar 0 ColMajor
               OpMemberDecorate %Bar 0 Offset 0
               OpMemberDecorate %Bar 0 MatrixStride 16
               OpMemberDecorate %Bar 1 Offset 44
               OpMemberDecorate %BarUBO 0 Offset 0
               OpDecorate %BarUBO Block
               OpDecorate %__0 DescriptorSet 0
               OpDecorate %__0 Binding 1
       %void = OpTypeVoid
          %3 = OpTypeFunction %void
      %float = OpTypeFloat 32
    %v4float = OpTypeVector %float 4
%_ptr_Output_v4float = OpTypePointer Output %v4float
  %FragColor = OpVariable %_ptr_Output_v4float Output
    %v3float = OpTypeVector %float 3
%mat3v3float = OpTypeMatrix %v3float 3
       %uint = OpTypeInt 32 0
     %uint_2 = OpConstant %uint 2
%_arr_mat3v3float_uint_2 = OpTypeArray %mat3v3float %uint_2
        %Foo = OpTypeStruct %_arr_mat3v3float_uint_2 %float
     %FooUBO = OpTypeStruct %Foo
%_ptr_Uniform_FooUBO = OpTypePointer Uniform %FooUBO
          %_ = OpVariable %_ptr_Uniform_FooUBO Uniform
        %int = OpTypeInt 32 1
      %int_0 = OpConstant %int 0
      %int_1 = OpConstant %int 1
%_ptr_Uniform_float = OpTypePointer Uniform %float
        %Bar = OpTypeStruct %mat3v3float %float
     %BarUBO = OpTypeStruct %Bar
%_ptr_Uniform_BarUBO = OpTypePointer Uniform %BarUBO
        %__0 = OpVariable %_ptr_Uniform_BarUBO Uniform
       %main = OpFunction %void None %3
          %5 = OpLabel
         %23 = OpAccessChain %_ptr_Uniform_float %_ %int_0 %int_1
         %24 = OpLoad %float %23
         %29 = OpAccessChain %_ptr_Uniform_float %__0 %int_0 %int_1
         %30 = OpLoad %float %29
         %31 = OpFAdd %float %24 %30
         %32 = OpCompositeConstruct %v4float %31 %31 %31 %31
               OpStore %FragColor %32
               OpReturn
               OpFunctionEnd
