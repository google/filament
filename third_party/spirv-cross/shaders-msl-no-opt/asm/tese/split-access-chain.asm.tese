               OpCapability Tessellation
         %94 = OpExtInstImport "GLSL.std.450"
               OpMemoryModel Logical GLSL450
               OpEntryPoint TessellationEvaluation %main "main" %in0 %o0
               OpExecutionMode %main Quads
               OpName %main "main"
               OpName %in0 "in0"
               OpName %o0 "o0"
               OpDecorate %in0 Location 0
               OpDecorate %o0 Location 0
       %void = OpTypeVoid
          %3 = OpTypeFunction %void
      %float = OpTypeFloat 32
    %v4float = OpTypeVector %float 4
       %uint = OpTypeInt 32 0
     %uint_0 = OpConstant %uint 0
     %uint_1 = OpConstant %uint 1
     %uint_2 = OpConstant %uint 2
%_arr_v4float_uint_1 = OpTypeArray %v4float %uint_1
%_ptr_Output_float = OpTypePointer Output %float
%_ptr_Input__arr_v4float_uint_1 = OpTypePointer Input %_arr_v4float_uint_1
      %in0 = OpVariable %_ptr_Input__arr_v4float_uint_1 Input
%_ptr_Output_v4float = OpTypePointer Output %v4float
         %o0 = OpVariable %_ptr_Output_float Output
%_ptr_Function_float = OpTypePointer Function %float
%_ptr_Input_v4float = OpTypePointer Input %v4float
%_ptr_Input_float = OpTypePointer Input %float
       %main = OpFunction %void None %3
          %4 = OpLabel
         %ac = OpAccessChain %_ptr_Input_v4float %in0 %uint_0
        %bac = OpInBoundsAccessChain %_ptr_Input_float %ac %uint_2
     %loaded = OpLoad %float %bac
               OpStore %o0 %loaded
               OpReturn
               OpFunctionEnd
