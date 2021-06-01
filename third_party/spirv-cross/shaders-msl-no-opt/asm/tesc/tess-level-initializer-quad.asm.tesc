; SPIR-V
; Version: 1.0
; Generator: Khronos Glslang Reference Front End; 10
; Bound: 47
; Schema: 0
               OpCapability Tessellation
          %1 = OpExtInstImport "GLSL.std.450"
               OpMemoryModel Logical GLSL450
               OpEntryPoint TessellationControl %main "main" %gl_out %gl_InvocationID %gl_TessLevelInner %gl_TessLevelOuter
               OpExecutionMode %main OutputVertices 4
			   OpExecutionMode %main Quads
               OpSource GLSL 450
               OpName %main "main"
               OpName %gl_PerVertex "gl_PerVertex"
               OpMemberName %gl_PerVertex 0 "gl_Position"
               OpMemberName %gl_PerVertex 1 "gl_PointSize"
               OpMemberName %gl_PerVertex 2 "gl_ClipDistance"
               OpMemberName %gl_PerVertex 3 "gl_CullDistance"
               OpName %gl_out "gl_out"
               OpName %gl_InvocationID "gl_InvocationID"
               OpName %gl_TessLevelInner "gl_TessLevelInner"
               OpName %gl_TessLevelOuter "gl_TessLevelOuter"
               OpMemberDecorate %gl_PerVertex 0 BuiltIn Position
               OpMemberDecorate %gl_PerVertex 1 BuiltIn PointSize
               OpMemberDecorate %gl_PerVertex 2 BuiltIn ClipDistance
               OpMemberDecorate %gl_PerVertex 3 BuiltIn CullDistance
               OpDecorate %gl_PerVertex Block
               OpDecorate %gl_InvocationID BuiltIn InvocationId
               OpDecorate %gl_TessLevelInner Patch
               OpDecorate %gl_TessLevelInner BuiltIn TessLevelInner
               OpDecorate %gl_TessLevelOuter Patch
               OpDecorate %gl_TessLevelOuter BuiltIn TessLevelOuter
       %void = OpTypeVoid
          %3 = OpTypeFunction %void
      %float = OpTypeFloat 32
    %v4float = OpTypeVector %float 4
       %uint = OpTypeInt 32 0
     %uint_1 = OpConstant %uint 1
%_arr_float_uint_1 = OpTypeArray %float %uint_1
%gl_PerVertex = OpTypeStruct %v4float %float %_arr_float_uint_1 %_arr_float_uint_1
     %uint_4 = OpConstant %uint 4
%_arr_gl_PerVertex_uint_4 = OpTypeArray %gl_PerVertex %uint_4
%_ptr_Output__arr_gl_PerVertex_uint_4 = OpTypePointer Output %_arr_gl_PerVertex_uint_4
     %gl_out = OpVariable %_ptr_Output__arr_gl_PerVertex_uint_4 Output
        %int = OpTypeInt 32 1
%_ptr_Input_int = OpTypePointer Input %int
%gl_InvocationID = OpVariable %_ptr_Input_int Input
      %int_0 = OpConstant %int 0
    %float_1 = OpConstant %float 1
         %22 = OpConstantComposite %v4float %float_1 %float_1 %float_1 %float_1
%_ptr_Output_v4float = OpTypePointer Output %v4float
     %uint_2 = OpConstant %uint 2
%_arr_float_uint_2 = OpTypeArray %float %uint_2
%_ptr_Output__arr_float_uint_2 = OpTypePointer Output %_arr_float_uint_2
	%inner_zero = OpConstantNull %_arr_float_uint_2
%gl_TessLevelInner = OpVariable %_ptr_Output__arr_float_uint_2 Output %inner_zero
%_ptr_Output_float = OpTypePointer Output %float
      %int_1 = OpConstant %int 1
    %float_2 = OpConstant %float 2
%_arr_float_uint_4 = OpTypeArray %float %uint_4
%_ptr_Output__arr_float_uint_4 = OpTypePointer Output %_arr_float_uint_4
	%outer_zero = OpConstantNull %_arr_float_uint_4
%gl_TessLevelOuter = OpVariable %_ptr_Output__arr_float_uint_4 Output %outer_zero
    %float_3 = OpConstant %float 3
    %float_4 = OpConstant %float 4
      %int_2 = OpConstant %int 2
    %float_5 = OpConstant %float 5
      %int_3 = OpConstant %int 3
    %float_6 = OpConstant %float 6
       %main = OpFunction %void None %3
          %5 = OpLabel
         %19 = OpLoad %int %gl_InvocationID
         %24 = OpAccessChain %_ptr_Output_v4float %gl_out %19 %int_0
               OpStore %24 %22
         %30 = OpAccessChain %_ptr_Output_float %gl_TessLevelInner %int_0
               OpStore %30 %float_1
         %33 = OpAccessChain %_ptr_Output_float %gl_TessLevelInner %int_1
               OpStore %33 %float_2
         %38 = OpAccessChain %_ptr_Output_float %gl_TessLevelOuter %int_0
               OpStore %38 %float_3
         %40 = OpAccessChain %_ptr_Output_float %gl_TessLevelOuter %int_1
               OpStore %40 %float_4
         %43 = OpAccessChain %_ptr_Output_float %gl_TessLevelOuter %int_2
               OpStore %43 %float_5
         %46 = OpAccessChain %_ptr_Output_float %gl_TessLevelOuter %int_3
               OpStore %46 %float_6
               OpReturn
               OpFunctionEnd
