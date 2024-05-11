; SPIR-V
; Version: 1.0
; Generator: Khronos Glslang Reference Front End; 10
; Bound: 40
; Schema: 0
               OpCapability Shader
          %1 = OpExtInstImport "GLSL.std.450"
               OpMemoryModel Logical GLSL450
               OpEntryPoint Fragment %main "main" %FragColor %foos %bars
               OpExecutionMode %main OriginUpperLeft
               OpSource GLSL 450
               OpName %main "main"
               OpName %FragColor "FragColor"
               OpName %Foo "Foo"
               OpMemberName %Foo 0 "a"
               OpMemberName %Foo 1 "b"
               OpName %foos "ALIAS"
               OpName %bars "ALIAS"
               OpDecorate %FragColor Location 0
               OpDecorate %foos Location 1
               OpDecorate %bars Location 10
       %void = OpTypeVoid
          %3 = OpTypeFunction %void
      %float = OpTypeFloat 32
    %v4float = OpTypeVector %float 4
%_ptr_Output_v4float = OpTypePointer Output %v4float
  %FragColor = OpVariable %_ptr_Output_v4float Output
        %Foo = OpTypeStruct %float %float
       %uint = OpTypeInt 32 0
     %uint_4 = OpConstant %uint 4
%_arr_Foo_uint_4 = OpTypeArray %Foo %uint_4
%_ptr_Input__arr_Foo_uint_4 = OpTypePointer Input %_arr_Foo_uint_4
       %foos = OpVariable %_ptr_Input__arr_Foo_uint_4 Input
        %int = OpTypeInt 32 1
      %int_0 = OpConstant %int 0
%_ptr_Input_float = OpTypePointer Input %float
     %uint_0 = OpConstant %uint 0
%_ptr_Output_float = OpTypePointer Output %float
      %int_1 = OpConstant %int 1
     %uint_1 = OpConstant %uint 1
      %int_2 = OpConstant %int 2
     %uint_2 = OpConstant %uint 2
       %bars = OpVariable %_ptr_Input__arr_Foo_uint_4 Input
      %int_3 = OpConstant %int 3
     %uint_3 = OpConstant %uint 3
       %main = OpFunction %void None %3
          %5 = OpLabel
         %19 = OpAccessChain %_ptr_Input_float %foos %int_0 %int_0
         %20 = OpLoad %float %19
         %23 = OpAccessChain %_ptr_Output_float %FragColor %uint_0
               OpStore %23 %20
         %25 = OpAccessChain %_ptr_Input_float %foos %int_1 %int_1
         %26 = OpLoad %float %25
         %28 = OpAccessChain %_ptr_Output_float %FragColor %uint_1
               OpStore %28 %26
         %30 = OpAccessChain %_ptr_Input_float %foos %int_2 %int_0
         %31 = OpLoad %float %30
         %33 = OpAccessChain %_ptr_Output_float %FragColor %uint_2
               OpStore %33 %31
         %36 = OpAccessChain %_ptr_Input_float %bars %int_3 %int_1
         %37 = OpLoad %float %36
         %39 = OpAccessChain %_ptr_Output_float %FragColor %uint_3
               OpStore %39 %37
               OpReturn
               OpFunctionEnd
