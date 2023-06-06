; SPIR-V
; Version: 1.0
; Generator: Khronos Glslang Reference Front End; 10
; Bound: 44
; Schema: 0
               OpCapability Tessellation
               OpCapability TessellationPointSize
          %1 = OpExtInstImport "GLSL.std.450"
               OpMemoryModel Logical GLSL450
               OpEntryPoint TessellationControl %main "main" %c %gl_InvocationID %p %gl_out
               OpExecutionMode %main OutputVertices 4
               OpSource GLSL 450
               OpName %main "main"
               OpName %C "C"
               OpMemberName %C 0 "v"
               OpName %c "c"
               OpName %gl_InvocationID "gl_InvocationID"
               OpName %P "P"
               OpMemberName %P 0 "v"
               OpName %p "p"
               OpName %gl_PerVertex "gl_PerVertex"
               OpMemberName %gl_PerVertex 0 "gl_Position"
               OpMemberName %gl_PerVertex 1 "gl_PointSize"
               OpMemberName %gl_PerVertex 2 "gl_ClipDistance"
               OpMemberName %gl_PerVertex 3 "gl_CullDistance"
               OpName %gl_out "gl_out"
               OpDecorate %C Block
               OpDecorate %c Location 0
               OpDecorate %gl_InvocationID BuiltIn InvocationId
               OpMemberDecorate %P 0 Patch
               OpDecorate %P Block
               OpDecorate %p Location 1
               OpMemberDecorate %gl_PerVertex 0 BuiltIn Position
               OpMemberDecorate %gl_PerVertex 1 BuiltIn PointSize
               OpMemberDecorate %gl_PerVertex 2 BuiltIn ClipDistance
               OpMemberDecorate %gl_PerVertex 3 BuiltIn CullDistance
               OpDecorate %gl_PerVertex Block
       %void = OpTypeVoid
          %3 = OpTypeFunction %void
      %float = OpTypeFloat 32
    %v4float = OpTypeVector %float 4
          %C = OpTypeStruct %v4float
       %uint = OpTypeInt 32 0
     %uint_4 = OpConstant %uint 4
%_arr_C_uint_4 = OpTypeArray %C %uint_4
%_ptr_Output__arr_C_uint_4 = OpTypePointer Output %_arr_C_uint_4
		%zero_c = OpConstantNull %_arr_C_uint_4
          %c = OpVariable %_ptr_Output__arr_C_uint_4 Output %zero_c
        %int = OpTypeInt 32 1
%_ptr_Input_int = OpTypePointer Input %int
%gl_InvocationID = OpVariable %_ptr_Input_int Input
      %int_0 = OpConstant %int 0
    %float_1 = OpConstant %float 1
         %20 = OpConstantComposite %v4float %float_1 %float_1 %float_1 %float_1
%_ptr_Output_v4float = OpTypePointer Output %v4float
          %P = OpTypeStruct %v4float
%_ptr_Output_P = OpTypePointer Output %P
		%zero_p = OpConstantNull %P
          %p = OpVariable %_ptr_Output_P Output %zero_p
    %float_2 = OpConstant %float 2
         %27 = OpConstantComposite %v4float %float_2 %float_2 %float_2 %float_2
     %uint_1 = OpConstant %uint 1
%_arr_float_uint_1 = OpTypeArray %float %uint_1
%gl_PerVertex = OpTypeStruct %v4float %float %_arr_float_uint_1 %_arr_float_uint_1
%_arr_gl_PerVertex_uint_4 = OpTypeArray %gl_PerVertex %uint_4
%_ptr_Output__arr_gl_PerVertex_uint_4 = OpTypePointer Output %_arr_gl_PerVertex_uint_4
	%zero_gl_out = OpConstantNull %_arr_gl_PerVertex_uint_4
     %gl_out = OpVariable %_ptr_Output__arr_gl_PerVertex_uint_4 Output %zero_gl_out
    %float_3 = OpConstant %float 3
         %37 = OpConstantComposite %v4float %float_3 %float_3 %float_3 %float_3
      %int_1 = OpConstant %int 1
    %float_4 = OpConstant %float 4
%_ptr_Output_float = OpTypePointer Output %float
       %main = OpFunction %void None %3
          %5 = OpLabel
         %17 = OpLoad %int %gl_InvocationID
         %22 = OpAccessChain %_ptr_Output_v4float %c %17 %int_0
               OpStore %22 %20
         %28 = OpAccessChain %_ptr_Output_v4float %p %int_0
               OpStore %28 %27
         %38 = OpAccessChain %_ptr_Output_v4float %gl_out %17 %int_0
               OpStore %38 %37
         %43 = OpAccessChain %_ptr_Output_float %gl_out %17 %int_1
               OpStore %43 %float_4
               OpReturn
               OpFunctionEnd
