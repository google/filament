; SPIR-V
; Version: 1.0
; Generator: Khronos Glslang Reference Front End; 10
; Bound: 42
; Schema: 0
               OpCapability Tessellation
          %1 = OpExtInstImport "GLSL.std.450"
               OpMemoryModel Logical GLSL450
               OpEntryPoint TessellationControl %main "main" %gl_out %gl_InvocationID %_ %patches %v2 %v3 %verts
               OpExecutionMode %main OutputVertices 4
               OpSource GLSL 450
               OpName %main "main"
               OpName %gl_PerVertex "gl_PerVertex"
               OpMemberName %gl_PerVertex 0 "gl_Position"
               OpMemberName %gl_PerVertex 1 "gl_PointSize"
               OpMemberName %gl_PerVertex 2 "gl_ClipDistance"
               OpMemberName %gl_PerVertex 3 "gl_CullDistance"
               OpName %gl_out "gl_out"
               OpName %gl_InvocationID "gl_InvocationID"
               OpName %vert "vert"
               OpMemberName %vert 0 "v0"
               OpMemberName %vert 1 "v1"
               OpName %_ ""
               OpName %vert_patch "vert_patch"
               OpMemberName %vert_patch 0 "v2"
               OpMemberName %vert_patch 1 "v3"
               OpName %patches "patches"
               OpName %v2 "v2"
               OpName %v3 "v3"
               OpName %vert2 "vert2"
               OpMemberName %vert2 0 "v4"
               OpMemberName %vert2 1 "v5"
               OpName %verts "verts"
               OpMemberDecorate %gl_PerVertex 0 BuiltIn Position
               OpMemberDecorate %gl_PerVertex 1 BuiltIn PointSize
               OpMemberDecorate %gl_PerVertex 2 BuiltIn ClipDistance
               OpMemberDecorate %gl_PerVertex 3 BuiltIn CullDistance
               OpDecorate %gl_PerVertex Block
               OpDecorate %gl_InvocationID BuiltIn InvocationId
               OpMemberDecorate %vert 0 Patch
               OpMemberDecorate %vert 1 Patch
               OpDecorate %vert Block
               OpDecorate %_ Location 0
               OpMemberDecorate %vert_patch 0 Patch
               OpMemberDecorate %vert_patch 1 Patch
               OpDecorate %vert_patch Block
               OpDecorate %patches Location 2
               OpDecorate %v2 Patch
               OpDecorate %v2 Location 6
               OpDecorate %v3 Location 7
               OpDecorate %vert2 Block
               OpDecorate %verts Location 8
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
	%gl_out_zero = OpConstantNull %_arr_gl_PerVertex_uint_4
     %gl_out = OpVariable %_ptr_Output__arr_gl_PerVertex_uint_4 Output %gl_out_zero
        %int = OpTypeInt 32 1
%_ptr_Input_int = OpTypePointer Input %int
%gl_InvocationID = OpVariable %_ptr_Input_int Input
      %int_0 = OpConstant %int 0
    %float_1 = OpConstant %float 1
         %22 = OpConstantComposite %v4float %float_1 %float_1 %float_1 %float_1
%_ptr_Output_v4float = OpTypePointer Output %v4float
       %vert = OpTypeStruct %float %float
%_ptr_Output_vert = OpTypePointer Output %vert
		%__zero = OpConstantNull %vert
          %_ = OpVariable %_ptr_Output_vert Output %__zero
 %vert_patch = OpTypeStruct %float %float
     %uint_2 = OpConstant %uint 2
%_arr_vert_patch_uint_2 = OpTypeArray %vert_patch %uint_2
%_ptr_Output__arr_vert_patch_uint_2 = OpTypePointer Output %_arr_vert_patch_uint_2
	%patches_zero = OpConstantNull %_arr_vert_patch_uint_2
    %patches = OpVariable %_ptr_Output__arr_vert_patch_uint_2 Output %patches_zero
%_ptr_Output_float = OpTypePointer Output %float
		%v2_zero = OpConstantNull %float
         %v2 = OpVariable %_ptr_Output_float Output %v2_zero
%_arr_float_uint_4 = OpTypeArray %float %uint_4
%_ptr_Output__arr_float_uint_4 = OpTypePointer Output %_arr_float_uint_4
		%v3_zero = OpConstantNull %_arr_float_uint_4
         %v3 = OpVariable %_ptr_Output__arr_float_uint_4 Output %v3_zero
      %vert2 = OpTypeStruct %float %float
%_arr_vert2_uint_4 = OpTypeArray %vert2 %uint_4
%_ptr_Output__arr_vert2_uint_4 = OpTypePointer Output %_arr_vert2_uint_4
	%verts_zero = OpConstantNull %_arr_vert2_uint_4
      %verts = OpVariable %_ptr_Output__arr_vert2_uint_4 Output %verts_zero
       %main = OpFunction %void None %3
          %5 = OpLabel
         %19 = OpLoad %int %gl_InvocationID
         %24 = OpAccessChain %_ptr_Output_v4float %gl_out %19 %int_0
               OpStore %24 %22
               OpReturn
               OpFunctionEnd
