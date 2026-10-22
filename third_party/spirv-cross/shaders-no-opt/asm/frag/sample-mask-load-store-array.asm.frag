; SPIR-V
; Version: 1.0
; Generator: Khronos Glslang Reference Front End; 11
; Bound: 30
; Schema: 0
               OpCapability Shader
          %1 = OpExtInstImport "GLSL.std.450"
               OpMemoryModel Logical GLSL450
               OpEntryPoint Fragment %main "main" %gl_SampleMaskIn %gl_SampleMask
               OpExecutionMode %main OriginUpperLeft
               OpSource GLSL 450
               OpName %main "main"
               OpName %copy_sample_mask "copy_sample_mask"
               OpName %gl_SampleMaskIn "gl_SampleMaskIn"
               OpName %out_sample_mask "out_sample_mask"
               OpName %gl_SampleMask "gl_SampleMask"
               OpDecorate %gl_SampleMaskIn Flat
               OpDecorate %gl_SampleMaskIn BuiltIn SampleMask
               OpDecorate %gl_SampleMask BuiltIn SampleMask
       %void = OpTypeVoid
          %3 = OpTypeFunction %void
        %int = OpTypeInt 32 1
       %uint = OpTypeInt 32 0
     %uint_1 = OpConstant %uint 1
%_arr_int_uint_1 = OpTypeArray %int %uint_1
%_ptr_Function__arr_int_uint_1 = OpTypePointer Function %_arr_int_uint_1
      %int_0 = OpConstant %int 0
%_ptr_Input__arr_int_uint_1 = OpTypePointer Input %_arr_int_uint_1
%gl_SampleMaskIn = OpVariable %_ptr_Input__arr_int_uint_1 Input
%_ptr_Input_int = OpTypePointer Input %int
%_ptr_Function_int = OpTypePointer Function %int
%_ptr_Output__arr_int_uint_1 = OpTypePointer Output %_arr_int_uint_1
%gl_SampleMask = OpVariable %_ptr_Output__arr_int_uint_1 Output
%_ptr_Output_int = OpTypePointer Output %int
       %main = OpFunction %void None %3
          %5 = OpLabel
%copy_sample_mask = OpVariable %_ptr_Function__arr_int_uint_1 Function
%out_sample_mask = OpVariable %_ptr_Function__arr_int_uint_1 Function

         %loaded_sample_mask_in = OpLoad %_arr_int_uint_1 %gl_SampleMaskIn
		   OpStore %copy_sample_mask %loaded_sample_mask_in
		%loaded_copy = OpLoad %_arr_int_uint_1 %copy_sample_mask
		   OpStore %gl_SampleMask %loaded_copy
               OpReturn
               OpFunctionEnd
