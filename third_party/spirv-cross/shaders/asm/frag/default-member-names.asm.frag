; SPIR-V
; Version: 1.0
; Generator: Khronos Glslang Reference Front End; 1
; Bound: 43
; Schema: 0
               OpCapability Shader
          %1 = OpExtInstImport "GLSL.std.450"
               OpMemoryModel Logical GLSL450
               OpEntryPoint Fragment %2 "main" %3
               OpExecutionMode %2 OriginUpperLeft
               OpDecorate %3 Location 0
       %void = OpTypeVoid
          %9 = OpTypeFunction %void
      %float = OpTypeFloat 32
    %v4float = OpTypeVector %float 4
         %12 = OpTypeFunction %v4float
  %_struct_5 = OpTypeStruct %float
  %_struct_6 = OpTypeStruct %float %float %float %float %float %float %float %float %float %float %float %float %_struct_5
%_ptr_Function__struct_6 = OpTypePointer Function %_struct_6
        %int = OpTypeInt 32 1
      %int_0 = OpConstant %int 0
%_ptr_Function_float = OpTypePointer Function %float
      %int_1 = OpConstant %int 1
      %int_2 = OpConstant %int 2
      %int_3 = OpConstant %int 3
%_ptr_Output_v4float = OpTypePointer Output %v4float
          %3 = OpVariable %_ptr_Output_v4float Output
%_ptr_Function_v4float = OpTypePointer Function %v4float
          %2 = OpFunction %void None %9
         %22 = OpLabel
         %23 = OpVariable %_ptr_Function__struct_6 Function
         %24 = OpAccessChain %_ptr_Function_float %23 %int_0
         %25 = OpLoad %float %24
         %26 = OpAccessChain %_ptr_Function_float %23 %int_1
         %27 = OpLoad %float %26
         %28 = OpAccessChain %_ptr_Function_float %23 %int_2
         %29 = OpLoad %float %28
         %30 = OpAccessChain %_ptr_Function_float %23 %int_3
         %31 = OpLoad %float %30
         %32 = OpCompositeConstruct %v4float %25 %27 %29 %31
               OpStore %3 %32
               OpReturn
               OpFunctionEnd
          %4 = OpFunction %v4float None %12
         %33 = OpLabel
          %7 = OpVariable %_ptr_Function__struct_6 Function
         %34 = OpAccessChain %_ptr_Function_float %7 %int_0
         %35 = OpLoad %float %34
         %36 = OpAccessChain %_ptr_Function_float %7 %int_1
         %37 = OpLoad %float %36
         %38 = OpAccessChain %_ptr_Function_float %7 %int_2
         %39 = OpLoad %float %38
         %40 = OpAccessChain %_ptr_Function_float %7 %int_3
         %41 = OpLoad %float %40
         %42 = OpCompositeConstruct %v4float %35 %37 %39 %41
               OpReturnValue %42
               OpFunctionEnd
