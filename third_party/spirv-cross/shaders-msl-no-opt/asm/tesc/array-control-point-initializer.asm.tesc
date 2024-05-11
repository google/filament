; SPIR-V
; Version: 1.0
; Generator: Khronos Glslang Reference Front End; 10
; Bound: 48
; Schema: 0
               OpCapability Tessellation
          %1 = OpExtInstImport "GLSL.std.450"
               OpMemoryModel Logical GLSL450
               OpEntryPoint TessellationControl %main "main" %gl_out %gl_InvocationID %foo
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
               OpName %foo "foo"
               OpMemberDecorate %gl_PerVertex 0 BuiltIn Position
               OpMemberDecorate %gl_PerVertex 1 BuiltIn PointSize
               OpMemberDecorate %gl_PerVertex 2 BuiltIn ClipDistance
               OpMemberDecorate %gl_PerVertex 3 BuiltIn CullDistance
               OpDecorate %gl_PerVertex Block
               OpDecorate %gl_InvocationID BuiltIn InvocationId
               OpDecorate %foo Location 0
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
     %uint_3 = OpConstant %uint 3
%_arr_float_uint_3 = OpTypeArray %float %uint_3
%_arr__arr_float_uint_3_uint_4 = OpTypeArray %_arr_float_uint_3 %uint_4
%_ptr_Output__arr__arr_float_uint_3_uint_4 = OpTypePointer Output %_arr__arr_float_uint_3_uint_4
%foo_zero = OpConstantNull %_arr__arr_float_uint_3_uint_4
        %foo = OpVariable %_ptr_Output__arr__arr_float_uint_3_uint_4 Output %foo_zero
%_ptr_Output_float = OpTypePointer Output %float
      %int_1 = OpConstant %int 1
      %int_2 = OpConstant %int 2
    %float_2 = OpConstant %float 2
       %main = OpFunction %void None %3
          %5 = OpLabel
         %19 = OpLoad %int %gl_InvocationID
         %24 = OpAccessChain %_ptr_Output_v4float %gl_out %19 %int_0
               OpStore %24 %22
         %30 = OpLoad %int %gl_InvocationID
         %31 = OpLoad %int %gl_InvocationID
         %32 = OpConvertSToF %float %31
         %34 = OpAccessChain %_ptr_Output_float %foo %30 %int_0
               OpStore %34 %32
         %35 = OpLoad %int %gl_InvocationID
         %37 = OpLoad %int %gl_InvocationID
         %38 = OpConvertSToF %float %37
         %39 = OpFAdd %float %38 %float_1
         %40 = OpAccessChain %_ptr_Output_float %foo %35 %int_1
               OpStore %40 %39
         %41 = OpLoad %int %gl_InvocationID
         %43 = OpLoad %int %gl_InvocationID
         %44 = OpConvertSToF %float %43
         %46 = OpFAdd %float %44 %float_2
         %47 = OpAccessChain %_ptr_Output_float %foo %41 %int_2
               OpStore %47 %46
               OpReturn
               OpFunctionEnd
