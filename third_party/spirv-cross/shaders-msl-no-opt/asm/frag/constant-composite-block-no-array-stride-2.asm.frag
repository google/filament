OpCapability Shader
OpMemoryModel Logical GLSL450
OpEntryPoint Fragment %main "main" %frag_out
OpExecutionMode %main OriginUpperLeft
OpDecorate %frag_out Location 0
OpMemberDecorate %type 1 Offset 0
%void = OpTypeVoid
%float = OpTypeFloat 32
%uint = OpTypeInt 32 0
%uint_2 = OpConstant %uint 2
%const_1 = OpConstant %float 1.0
%const_2 = OpConstant %float 2.0
%const_3 = OpConstant %float 3.0
%const_4 = OpConstant %float 4.0
%const_5 = OpConstant %float 5.0
%const_6 = OpConstant %float 6.0
%arr_float_2 = OpTypeArray %float %uint_2
%const_arr0 = OpConstantComposite %arr_float_2 %const_1 %const_2
%const_arr1 = OpConstantComposite %arr_float_2 %const_3 %const_4
%const_arr2 = OpConstantComposite %arr_float_2 %const_5 %const_6
%type = OpTypeStruct %arr_float_2 %arr_float_2 %arr_float_2
%float_ptr = OpTypePointer Output %float
%const_var = OpConstantComposite %type %const_arr0 %const_arr1 %const_arr2
%type_ptr = OpTypePointer Function %type
%frag_out = OpVariable %float_ptr Output
%main_func = OpTypeFunction %void
%main = OpFunction %void None %main_func
%label = OpLabel
%var = OpVariable %type_ptr Function
OpStore %var %const_var
OpStore %frag_out %const_1
OpReturn
OpFunctionEnd
