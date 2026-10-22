; SPIR-V
; Version: 1.0
; Generator: Khronos Glslang Reference Front End; 10
; Bound: 43
; Schema: 0
               OpCapability Tessellation
          %1 = OpExtInstImport "GLSL.std.450"
               OpMemoryModel Logical GLSL450
               OpEntryPoint TessellationControl %main "main" %gl_TessLevelInner %gl_TessLevelOuter %gl_out %gl_InvocationID
               OpExecutionMode %main OutputVertices 1
			   OpExecutionMode %main Triangles
               OpSource GLSL 450
               OpName %main "main"
               OpName %gl_TessLevelInner "gl_TessLevelInner"
               OpName %gl_TessLevelOuter "gl_TessLevelOuter"
               OpName %inner "inner"
               OpName %outer "outer"
               OpName %gl_PerVertex "gl_PerVertex"
               OpMemberName %gl_PerVertex 0 "gl_Position"
               OpMemberName %gl_PerVertex 1 "gl_PointSize"
               OpMemberName %gl_PerVertex 2 "gl_ClipDistance"
               OpMemberName %gl_PerVertex 3 "gl_CullDistance"
               OpName %gl_out "gl_out"
               OpName %gl_InvocationID "gl_InvocationID"
               OpDecorate %gl_TessLevelInner Patch
               OpDecorate %gl_TessLevelInner BuiltIn TessLevelInner
               OpDecorate %gl_TessLevelOuter Patch
               OpDecorate %gl_TessLevelOuter BuiltIn TessLevelOuter
               OpMemberDecorate %gl_PerVertex 0 BuiltIn Position
               OpMemberDecorate %gl_PerVertex 1 BuiltIn PointSize
               OpMemberDecorate %gl_PerVertex 2 BuiltIn ClipDistance
               OpMemberDecorate %gl_PerVertex 3 BuiltIn CullDistance
               OpDecorate %gl_PerVertex Block
               OpDecorate %gl_InvocationID BuiltIn InvocationId
       %void = OpTypeVoid
          %3 = OpTypeFunction %void
      %float = OpTypeFloat 32
       %uint = OpTypeInt 32 0
     %uint_2 = OpConstant %uint 2
%_arr_float_uint_2 = OpTypeArray %float %uint_2
%_ptr_Output__arr_float_uint_2 = OpTypePointer Output %_arr_float_uint_2
%gl_TessLevelInner = OpVariable %_ptr_Output__arr_float_uint_2 Output
    %float_1 = OpConstant %float 1
    %float_2 = OpConstant %float 2
         %14 = OpConstantComposite %_arr_float_uint_2 %float_1 %float_2
     %uint_4 = OpConstant %uint 4
%_arr_float_uint_4 = OpTypeArray %float %uint_4
%_ptr_Output__arr_float_uint_4 = OpTypePointer Output %_arr_float_uint_4
%gl_TessLevelOuter = OpVariable %_ptr_Output__arr_float_uint_4 Output
    %float_3 = OpConstant %float 3
    %float_4 = OpConstant %float 4
         %21 = OpConstantComposite %_arr_float_uint_4 %float_1 %float_2 %float_3 %float_4
%_ptr_Function__arr_float_uint_2 = OpTypePointer Function %_arr_float_uint_2
%_ptr_Function__arr_float_uint_4 = OpTypePointer Function %_arr_float_uint_4
    %v4float = OpTypeVector %float 4
     %uint_1 = OpConstant %uint 1
%_arr_float_uint_1 = OpTypeArray %float %uint_1
%gl_PerVertex = OpTypeStruct %v4float %float %_arr_float_uint_1 %_arr_float_uint_1
%_arr_gl_PerVertex_uint_1 = OpTypeArray %gl_PerVertex %uint_1
%_ptr_Output__arr_gl_PerVertex_uint_1 = OpTypePointer Output %_arr_gl_PerVertex_uint_1
     %gl_out = OpVariable %_ptr_Output__arr_gl_PerVertex_uint_1 Output
        %int = OpTypeInt 32 1
%_ptr_Input_int = OpTypePointer Input %int
%gl_InvocationID = OpVariable %_ptr_Input_int Input
      %int_0 = OpConstant %int 0
         %40 = OpConstantComposite %v4float %float_1 %float_1 %float_1 %float_1
%_ptr_Output_v4float = OpTypePointer Output %v4float
       %main = OpFunction %void None %3
          %5 = OpLabel
      %inner = OpVariable %_ptr_Function__arr_float_uint_2 Function
      %outer = OpVariable %_ptr_Function__arr_float_uint_4 Function
               OpStore %gl_TessLevelInner %14
               OpStore %gl_TessLevelOuter %21
         %24 = OpLoad %_arr_float_uint_2 %gl_TessLevelInner
               OpStore %inner %24
         %27 = OpLoad %_arr_float_uint_4 %gl_TessLevelOuter
               OpStore %outer %27
         %38 = OpLoad %int %gl_InvocationID
         %42 = OpAccessChain %_ptr_Output_v4float %gl_out %38 %int_0
               OpStore %42 %40
               OpReturn
               OpFunctionEnd
