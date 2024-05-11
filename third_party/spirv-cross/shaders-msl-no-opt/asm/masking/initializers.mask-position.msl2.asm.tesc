; SPIR-V
; Version: 1.0
; Generator: Khronos Glslang Reference Front End; 10
; Bound: 40
; Schema: 0
               OpCapability Tessellation
               OpCapability TessellationPointSize
          %1 = OpExtInstImport "GLSL.std.450"
               OpMemoryModel Logical GLSL450
               OpEntryPoint TessellationControl %main "main" %foo %gl_InvocationID %foo_patch %gl_out
               OpExecutionMode %main OutputVertices 4
               OpSource GLSL 450
               OpName %main "main"
               OpName %foo "foo"
               OpName %gl_InvocationID "gl_InvocationID"
               OpName %foo_patch "foo_patch"
               OpName %gl_PerVertex "gl_PerVertex"
               OpMemberName %gl_PerVertex 0 "gl_Position"
               OpMemberName %gl_PerVertex 1 "gl_PointSize"
               OpMemberName %gl_PerVertex 2 "gl_ClipDistance"
               OpMemberName %gl_PerVertex 3 "gl_CullDistance"
               OpName %gl_out "gl_out"
               OpDecorate %foo Location 0
               OpDecorate %gl_InvocationID BuiltIn InvocationId
               OpDecorate %foo_patch Patch
               OpDecorate %foo_patch Location 1
               OpMemberDecorate %gl_PerVertex 0 BuiltIn Position
               OpMemberDecorate %gl_PerVertex 1 BuiltIn PointSize
               OpMemberDecorate %gl_PerVertex 2 BuiltIn ClipDistance
               OpMemberDecorate %gl_PerVertex 3 BuiltIn CullDistance
               OpDecorate %gl_PerVertex Block
       %void = OpTypeVoid
          %3 = OpTypeFunction %void
      %float = OpTypeFloat 32
    %v4float = OpTypeVector %float 4
       %uint = OpTypeInt 32 0
     %uint_4 = OpConstant %uint 4
%_arr_v4float_uint_4 = OpTypeArray %v4float %uint_4
	%zero_foo = OpConstantNull %_arr_v4float_uint_4
%_ptr_Output__arr_v4float_uint_4 = OpTypePointer Output %_arr_v4float_uint_4
        %foo = OpVariable %_ptr_Output__arr_v4float_uint_4 Output %zero_foo
        %int = OpTypeInt 32 1
%_ptr_Input_int = OpTypePointer Input %int
%gl_InvocationID = OpVariable %_ptr_Input_int Input
    %float_1 = OpConstant %float 1
         %18 = OpConstantComposite %v4float %float_1 %float_1 %float_1 %float_1
%_ptr_Output_v4float = OpTypePointer Output %v4float
	%zero_foo_patch = OpConstantNull %v4float
  %foo_patch = OpVariable %_ptr_Output_v4float Output %zero_foo_patch
    %float_2 = OpConstant %float 2
         %23 = OpConstantComposite %v4float %float_2 %float_2 %float_2 %float_2
     %uint_1 = OpConstant %uint 1
%_arr_float_uint_1 = OpTypeArray %float %uint_1
%gl_PerVertex = OpTypeStruct %v4float %float %_arr_float_uint_1 %_arr_float_uint_1
%_arr_gl_PerVertex_uint_4 = OpTypeArray %gl_PerVertex %uint_4
%_ptr_Output__arr_gl_PerVertex_uint_4 = OpTypePointer Output %_arr_gl_PerVertex_uint_4
	%zero_gl_out = OpConstantNull %_arr_gl_PerVertex_uint_4
     %gl_out = OpVariable %_ptr_Output__arr_gl_PerVertex_uint_4 Output %zero_gl_out
      %int_0 = OpConstant %int 0
    %float_3 = OpConstant %float 3
         %33 = OpConstantComposite %v4float %float_3 %float_3 %float_3 %float_3
      %int_1 = OpConstant %int 1
    %float_4 = OpConstant %float 4
%_ptr_Output_float = OpTypePointer Output %float
       %main = OpFunction %void None %3
          %5 = OpLabel
         %16 = OpLoad %int %gl_InvocationID
         %20 = OpAccessChain %_ptr_Output_v4float %foo %16
               OpStore %20 %18
               OpStore %foo_patch %23
         %34 = OpAccessChain %_ptr_Output_v4float %gl_out %16 %int_0
               OpStore %34 %33
         %39 = OpAccessChain %_ptr_Output_float %gl_out %16 %int_1
               OpStore %39 %float_4
               OpReturn
               OpFunctionEnd
