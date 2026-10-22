; SPIR-V
; Version: 1.0
; Generator: Khronos Glslang Reference Front End; 3
; Bound: 42
; Schema: 0
               OpCapability Geometry
          %1 = OpExtInstImport "GLSL.std.450"
               OpMemoryModel Logical GLSL450
               OpEntryPoint Geometry %main "main" %gl_in %_
               OpExecutionMode %main Triangles
               OpExecutionMode %main Invocations 1
               OpExecutionMode %main OutputTriangleStrip
               OpExecutionMode %main OutputVertices 5
               OpSource GLSL 440
               OpName %main "main"
               OpName %Data "Data"
               OpMemberName %Data 0 "ApiPerspectivePosition"
               OpName %Copy_struct_Data_vf41_3__ "Copy(struct-Data-vf41[3];"
               OpName %inputStream "inputStream"
               OpName %gl_PerVertex "gl_PerVertex"
               OpMemberName %gl_PerVertex 0 "gl_Position"
               OpMemberName %gl_PerVertex 1 "gl_PointSize"
               OpMemberName %gl_PerVertex 2 "gl_ClipDistance"
               OpName %gl_in "gl_in"
               OpName %inputStream_0 "inputStream"
               OpName %param "param"
               OpName %gl_PerVertex_0 "gl_PerVertex"
               OpMemberName %gl_PerVertex_0 0 "gl_Position"
               OpMemberName %gl_PerVertex_0 1 "gl_PointSize"
               OpMemberName %gl_PerVertex_0 2 "gl_ClipDistance"
               OpName %_ ""
               OpMemberDecorate %gl_PerVertex 0 BuiltIn Position
               OpMemberDecorate %gl_PerVertex 1 BuiltIn PointSize
               OpMemberDecorate %gl_PerVertex 2 BuiltIn ClipDistance
               OpDecorate %gl_PerVertex Block
               OpMemberDecorate %gl_PerVertex_0 0 BuiltIn Position
               OpMemberDecorate %gl_PerVertex_0 1 BuiltIn PointSize
               OpMemberDecorate %gl_PerVertex_0 2 BuiltIn ClipDistance
               OpDecorate %gl_PerVertex_0 Block
       %void = OpTypeVoid
          %3 = OpTypeFunction %void
      %float = OpTypeFloat 32
    %v4float = OpTypeVector %float 4
       %Data = OpTypeStruct %v4float
       %uint = OpTypeInt 32 0
     %uint_3 = OpConstant %uint 3
%_arr_Data_uint_3 = OpTypeArray %Data %uint_3
%_ptr_Function__Data = OpTypePointer Function %Data
%_ptr_Function__arr_Data_uint_3 = OpTypePointer Function %_arr_Data_uint_3
         %13 = OpTypeFunction %void %_ptr_Function__arr_Data_uint_3
        %int = OpTypeInt 32 1
      %int_0 = OpConstant %int 0
     %uint_1 = OpConstant %uint 1
%_arr_float_uint_1 = OpTypeArray %float %uint_1
%gl_PerVertex = OpTypeStruct %v4float %float %_arr_float_uint_1
%_arr_gl_PerVertex_uint_3 = OpTypeArray %gl_PerVertex %uint_3
%_ptr_Input__arr_gl_PerVertex_uint_3 = OpTypePointer Input %_arr_gl_PerVertex_uint_3
      %gl_in = OpVariable %_ptr_Input__arr_gl_PerVertex_uint_3 Input
%_ptr_Input_v4float = OpTypePointer Input %v4float
%_ptr_Function_v4float = OpTypePointer Function %v4float
%gl_PerVertex_0 = OpTypeStruct %v4float %float %_arr_float_uint_1
%_ptr_Output_gl_PerVertex_0 = OpTypePointer Output %gl_PerVertex_0
          %_ = OpVariable %_ptr_Output_gl_PerVertex_0 Output
%_ptr_Output_v4float = OpTypePointer Output %v4float
       %main = OpFunction %void None %3
          %5 = OpLabel
%inputStream_0 = OpVariable %_ptr_Function__arr_Data_uint_3 Function
      %param = OpVariable %_ptr_Function__arr_Data_uint_3 Function
         %32 = OpLoad %_arr_Data_uint_3 %inputStream_0
               OpStore %param %32
         %33 = OpFunctionCall %void %Copy_struct_Data_vf41_3__ %param
         %34 = OpLoad %_arr_Data_uint_3 %param
               OpStore %inputStream_0 %34
         %59 = OpAccessChain %_ptr_Function__Data %inputStream_0 %int_0
         %38 = OpAccessChain %_ptr_Function_v4float %59 %int_0
         %39 = OpLoad %v4float %38
         %41 = OpAccessChain %_ptr_Output_v4float %_ %int_0
               OpStore %41 %39
               OpReturn
               OpFunctionEnd
%Copy_struct_Data_vf41_3__ = OpFunction %void None %13
%inputStream = OpFunctionParameter %_ptr_Function__arr_Data_uint_3
         %16 = OpLabel
         %26 = OpAccessChain %_ptr_Input_v4float %gl_in %int_0 %int_0
         %27 = OpLoad %v4float %26
         %28 = OpAccessChain %_ptr_Function__Data %inputStream %int_0
         %29 = OpAccessChain %_ptr_Function_v4float %28 %int_0
               OpStore %29 %27
               OpReturn
               OpFunctionEnd
