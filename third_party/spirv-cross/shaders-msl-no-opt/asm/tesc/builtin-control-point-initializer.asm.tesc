; SPIR-V
; Version: 1.0
; Generator: Khronos Glslang Reference Front End; 10
; Bound: 35
; Schema: 0
               OpCapability Tessellation
          %1 = OpExtInstImport "GLSL.std.450"
               OpMemoryModel Logical GLSL450
               OpEntryPoint TessellationControl %main "main" %gl_out %gl_InvocationID %verts
               OpExecutionMode %main OutputVertices 4
               OpSource GLSL 450
               OpName %main "main"
               OpName %gl_PerVertex "gl_PerVertex"
               OpMemberName %gl_PerVertex 0 "gl_Position"
               OpMemberName %gl_PerVertex 1 "gl_PointSize"
               OpName %gl_out "gl_out"
               OpName %gl_InvocationID "gl_InvocationID"
               OpName %Verts "Verts"
               OpMemberName %Verts 0 "a"
               OpMemberName %Verts 1 "b"
               OpName %verts "verts"
               OpMemberDecorate %gl_PerVertex 0 BuiltIn Position
               OpMemberDecorate %gl_PerVertex 1 BuiltIn PointSize
               OpDecorate %gl_PerVertex Block
               OpDecorate %gl_InvocationID BuiltIn InvocationId
               OpDecorate %Verts Block
               OpDecorate %verts Location 0
       %void = OpTypeVoid
          %3 = OpTypeFunction %void
      %float = OpTypeFloat 32
    %v4float = OpTypeVector %float 4
       %uint = OpTypeInt 32 0
     %uint_1 = OpConstant %uint 1
%gl_PerVertex = OpTypeStruct %v4float %float
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
    %v2float = OpTypeVector %float 2
      %Verts = OpTypeStruct %float %v2float
%_arr_Verts_uint_4 = OpTypeArray %Verts %uint_4
%_ptr_Output__arr_Verts_uint_4 = OpTypePointer Output %_arr_Verts_uint_4
	%verts_zero = OpConstantNull %_arr_Verts_uint_4
      %verts = OpVariable %_ptr_Output__arr_Verts_uint_4 Output %verts_zero
%_ptr_Output_float = OpTypePointer Output %float
       %main = OpFunction %void None %3
          %5 = OpLabel
         %19 = OpLoad %int %gl_InvocationID
         %24 = OpAccessChain %_ptr_Output_v4float %gl_out %19 %int_0
               OpStore %24 %22
         %30 = OpLoad %int %gl_InvocationID
         %31 = OpLoad %int %gl_InvocationID
         %32 = OpConvertSToF %float %31
         %34 = OpAccessChain %_ptr_Output_float %verts %30 %int_0
               OpStore %34 %32
               OpReturn
               OpFunctionEnd
