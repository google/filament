; SPIR-V
; Version: 1.0
; Generator: Khronos Glslang Reference Front End; 7
; Bound: 64
; Schema: 0
               OpCapability Shader
          %1 = OpExtInstImport "GLSL.std.450"
               OpMemoryModel Logical GLSL450
               OpEntryPoint Fragment %main "main" %pos_1 %_entryPointOutput
               OpExecutionMode %main OriginUpperLeft
               OpSource HLSL 500
               OpName %main "main"
               OpName %_main_vf4_ "@main(vf4;"
               OpName %pos "pos"
               OpName %Foo "Foo"
               OpMemberName %Foo 0 "a"
               OpMemberName %Foo 1 "b"
               OpName %foo "foo"
               OpName %Foo_0 "Foo"
               OpMemberName %Foo_0 0 "a"
               OpMemberName %Foo_0 1 "b"
               OpName %buf "buf"
               OpMemberName %buf 0 "results"
               OpMemberName %buf 1 "bar"
               OpName %_ ""
               OpName %pos_0 "pos"
               OpName %pos_1 "pos"
               OpName %_entryPointOutput "@entryPointOutput"
               OpName %param "param"
               OpMemberDecorate %Foo_0 0 Offset 0
               OpMemberDecorate %Foo_0 1 Offset 12
               OpDecorate %_arr_Foo_0_uint_16 ArrayStride 16
               OpMemberDecorate %buf 0 Offset 0
               OpMemberDecorate %buf 1 Offset 256
               OpDecorate %buf Block
               OpDecorate %_ DescriptorSet 0
               OpDecorate %_ Binding 0
               OpDecorate %pos_1 BuiltIn FragCoord
               OpDecorate %_entryPointOutput Location 0
       %void = OpTypeVoid
          %3 = OpTypeFunction %void
      %float = OpTypeFloat 32
    %v4float = OpTypeVector %float 4
%_ptr_Function_v4float = OpTypePointer Function %v4float
          %9 = OpTypeFunction %v4float %_ptr_Function_v4float
    %v3float = OpTypeVector %float 3
        %Foo = OpTypeStruct %v3float %float
%_ptr_Function_Foo = OpTypePointer Function %Foo
      %Foo_0 = OpTypeStruct %v3float %float
       %uint = OpTypeInt 32 0
    %uint_16 = OpConstant %uint 16
%_arr_Foo_0_uint_16 = OpTypeArray %Foo_0 %uint_16
        %buf = OpTypeStruct %_arr_Foo_0_uint_16 %v4float
%_ptr_Uniform_buf = OpTypePointer Uniform %buf
          %_ = OpVariable %_ptr_Uniform_buf Uniform
        %int = OpTypeInt 32 1
      %int_0 = OpConstant %int 0
     %uint_0 = OpConstant %uint 0
%_ptr_Function_float = OpTypePointer Function %float
     %int_16 = OpConstant %int 16
%_ptr_Uniform_Foo_0 = OpTypePointer Uniform %Foo_0
%_ptr_Function_v3float = OpTypePointer Function %v3float
      %int_1 = OpConstant %int 1
%_ptr_Uniform_v4float = OpTypePointer Uniform %v4float
    %float_0 = OpConstant %float 0
%_ptr_Input_v4float = OpTypePointer Input %v4float
      %pos_1 = OpVariable %_ptr_Input_v4float Input
%_ptr_Output_v4float = OpTypePointer Output %v4float
%_entryPointOutput = OpVariable %_ptr_Output_v4float Output
       %main = OpFunction %void None %3
          %5 = OpLabel
      %pos_0 = OpVariable %_ptr_Function_v4float Function
      %param = OpVariable %_ptr_Function_v4float Function
         %58 = OpLoad %v4float %pos_1
               OpStore %pos_0 %58
         %62 = OpLoad %v4float %pos_0
               OpStore %param %62
         %63 = OpFunctionCall %v4float %_main_vf4_ %param
               OpStore %_entryPointOutput %63
               OpReturn
               OpFunctionEnd
 %_main_vf4_ = OpFunction %v4float None %9
        %pos = OpFunctionParameter %_ptr_Function_v4float
         %12 = OpLabel
        %foo = OpVariable %_ptr_Function_Foo Function
         %28 = OpAccessChain %_ptr_Function_float %pos %uint_0
         %29 = OpLoad %float %28
         %30 = OpConvertFToS %int %29
         %32 = OpSMod %int %30 %int_16
         %34 = OpAccessChain %_ptr_Uniform_Foo_0 %_ %int_0 %32
         %35 = OpLoad %Foo_0 %34
         %36 = OpCompositeExtract %v3float %35 0
         %38 = OpAccessChain %_ptr_Function_v3float %foo %int_0
               OpStore %38 %36
         %39 = OpCompositeExtract %float %35 1
         %41 = OpAccessChain %_ptr_Function_float %foo %int_1
               OpStore %41 %39
         %42 = OpAccessChain %_ptr_Function_v3float %foo %int_0
         %43 = OpLoad %v3float %42
         %45 = OpAccessChain %_ptr_Uniform_v4float %_ %int_1
         %46 = OpLoad %v4float %45
         %47 = OpVectorShuffle %v3float %46 %46 0 1 2
         %48 = OpDot %float %43 %47
         %49 = OpAccessChain %_ptr_Function_float %foo %int_1
         %50 = OpLoad %float %49
         %52 = OpCompositeConstruct %v4float %48 %50 %float_0 %float_0
               OpReturnValue %52
               OpFunctionEnd
