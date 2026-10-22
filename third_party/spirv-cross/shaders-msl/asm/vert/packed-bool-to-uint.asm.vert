; SPIR-V
; Version: 1.0
; Generator: Khronos Glslang Reference Front End; 7
; Bound: 62
; Schema: 0
               OpCapability Shader
          %1 = OpExtInstImport "GLSL.std.450"
               OpMemoryModel Logical GLSL450
               OpEntryPoint Vertex %main "main" %_ %gl_VertexIndex %a_position
               OpSource GLSL 450
               OpName %main "main"
               OpName %gl_PerVertex "gl_PerVertex"
               OpMemberName %gl_PerVertex 0 "gl_Position"
               OpMemberName %gl_PerVertex 1 "gl_PointSize"
               OpMemberName %gl_PerVertex 2 "gl_ClipDistance"
               OpMemberName %gl_PerVertex 3 "gl_CullDistance"
               OpName %_ ""
               OpName %Struct "Struct"
               OpMemberName %Struct 0 "flags"
               OpName %defaultUniformsVS "defaultUniformsVS"
               OpMemberName %defaultUniformsVS 0 "flags"
               OpMemberName %defaultUniformsVS 1 "uquad"
               OpMemberName %defaultUniformsVS 2 "umatrix"
               OpName %__0 ""
               OpName %gl_VertexIndex "gl_VertexIndex"
               OpName %a_position "a_position"
               OpMemberDecorate %gl_PerVertex 0 BuiltIn Position
               OpMemberDecorate %gl_PerVertex 1 BuiltIn PointSize
               OpMemberDecorate %gl_PerVertex 2 BuiltIn ClipDistance
               OpMemberDecorate %gl_PerVertex 3 BuiltIn CullDistance
               OpDecorate %gl_PerVertex Block
               OpDecorate %_arr_uint_uint_1 ArrayStride 16
               OpMemberDecorate %Struct 0 Offset 0
               OpDecorate %_arr_v2float_uint_4 ArrayStride 16
               OpMemberDecorate %defaultUniformsVS 0 Offset 0
               OpMemberDecorate %defaultUniformsVS 1 Offset 16
               OpMemberDecorate %defaultUniformsVS 2 ColMajor
               OpMemberDecorate %defaultUniformsVS 2 Offset 80
               OpMemberDecorate %defaultUniformsVS 2 MatrixStride 16
               OpDecorate %defaultUniformsVS Block
               OpDecorate %__0 DescriptorSet 0
               OpDecorate %__0 Binding 0
               OpDecorate %gl_VertexIndex BuiltIn VertexIndex
               OpDecorate %a_position Location 0
       %void = OpTypeVoid
          %3 = OpTypeFunction %void
      %float = OpTypeFloat 32
    %v4float = OpTypeVector %float 4
       %uint = OpTypeInt 32 0
     %uint_1 = OpConstant %uint 1
%_arr_float_uint_1 = OpTypeArray %float %uint_1
%gl_PerVertex = OpTypeStruct %v4float %float %_arr_float_uint_1 %_arr_float_uint_1
%_ptr_Output_gl_PerVertex = OpTypePointer Output %gl_PerVertex
          %_ = OpVariable %_ptr_Output_gl_PerVertex Output
        %int = OpTypeInt 32 1
      %int_0 = OpConstant %int 0
%_arr_uint_uint_1 = OpTypeArray %uint %uint_1
     %Struct = OpTypeStruct %_arr_uint_uint_1
    %v2float = OpTypeVector %float 2
     %uint_4 = OpConstant %uint 4
%_arr_v2float_uint_4 = OpTypeArray %v2float %uint_4
%mat4v4float = OpTypeMatrix %v4float 4
%defaultUniformsVS = OpTypeStruct %Struct %_arr_v2float_uint_4 %mat4v4float
%_ptr_Uniform_defaultUniformsVS = OpTypePointer Uniform %defaultUniformsVS
        %__0 = OpVariable %_ptr_Uniform_defaultUniformsVS Uniform
      %int_2 = OpConstant %int 2
%_ptr_Uniform_mat4v4float = OpTypePointer Uniform %mat4v4float
      %int_1 = OpConstant %int 1
%_ptr_Input_int = OpTypePointer Input %int
%gl_VertexIndex = OpVariable %_ptr_Input_int Input
%_ptr_Uniform_v2float = OpTypePointer Uniform %v2float
%_ptr_Input_v4float = OpTypePointer Input %v4float
 %a_position = OpVariable %_ptr_Input_v4float Input
     %uint_2 = OpConstant %uint 2
%_ptr_Input_float = OpTypePointer Input %float
     %uint_3 = OpConstant %uint 3
%_ptr_Output_v4float = OpTypePointer Output %v4float
%_ptr_Uniform_uint = OpTypePointer Uniform %uint
       %bool = OpTypeBool
     %uint_0 = OpConstant %uint 0
    %float_0 = OpConstant %float 0
%_ptr_Output_float = OpTypePointer Output %float
       %main = OpFunction %void None %3
          %5 = OpLabel
         %27 = OpAccessChain %_ptr_Uniform_mat4v4float %__0 %int_2
         %28 = OpLoad %mat4v4float %27
         %32 = OpLoad %int %gl_VertexIndex
         %34 = OpAccessChain %_ptr_Uniform_v2float %__0 %int_1 %32
         %35 = OpLoad %v2float %34
         %40 = OpAccessChain %_ptr_Input_float %a_position %uint_2
         %41 = OpLoad %float %40
         %43 = OpAccessChain %_ptr_Input_float %a_position %uint_3
         %44 = OpLoad %float %43
         %45 = OpCompositeExtract %float %35 0
         %46 = OpCompositeExtract %float %35 1
         %47 = OpCompositeConstruct %v4float %45 %46 %41 %44
         %48 = OpMatrixTimesVector %v4float %28 %47
         %50 = OpAccessChain %_ptr_Output_v4float %_ %int_0
               OpStore %50 %48
         %52 = OpAccessChain %_ptr_Uniform_uint %__0 %int_0 %int_0 %int_0
         %53 = OpLoad %uint %52
         %56 = OpINotEqual %bool %53 %uint_0
               OpSelectionMerge %58 None
               OpBranchConditional %56 %57 %58
         %57 = OpLabel
         %61 = OpAccessChain %_ptr_Output_float %_ %int_0 %uint_2
               OpStore %61 %float_0
               OpBranch %58
         %58 = OpLabel
               OpReturn
               OpFunctionEnd
