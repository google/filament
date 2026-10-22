; SPIR-V
; Version: 1.0
; Generator: Google spiregg; 0
; Bound: 66
; Schema: 0
               OpCapability Shader
               OpMemoryModel Logical GLSL450
               OpEntryPoint Fragment %main "main" %in_var_COLOR %out_var_SV_TARGET
               OpExecutionMode %main OriginUpperLeft
               OpSource HLSL 600
               OpName %type_Test1Cbuf "type.Test1Cbuf"
               OpMemberName %type_Test1Cbuf 0 "test1"
               OpName %Test1 "Test1"
               OpMemberName %Test1 0 "a"
               OpMemberName %Test1 1 "b"
               OpName %StraddleResolve "StraddleResolve"
               OpMemberName %StraddleResolve 0 "A"
               OpMemberName %StraddleResolve 1 "B"
               OpMemberName %StraddleResolve 2 "C"
               OpMemberName %StraddleResolve 3 "D"
               OpName %Test1Cbuf "Test1Cbuf"
               OpName %type_Test2Cbuf "type.Test2Cbuf"
               OpMemberName %type_Test2Cbuf 0 "test2"
               OpName %Test2 "Test2"
               OpMemberName %Test2 0 "a"
               OpMemberName %Test2 1 "b"
               OpMemberName %Test2 2 "c"
               OpMemberName %Test2 3 "d"
               OpName %Test2Cbuf "Test2Cbuf"
               OpName %type_Test3Cbuf "type.Test3Cbuf"
               OpMemberName %type_Test3Cbuf 0 "test3"
               OpName %Test3 "Test3"
               OpMemberName %Test3 0 "c23"
               OpMemberName %Test3 1 "dummy0"
               OpMemberName %Test3 2 "r23"
               OpMemberName %Test3 3 "dummy1"
               OpMemberName %Test3 4 "c32"
               OpMemberName %Test3 5 "dummy2"
               OpMemberName %Test3 6 "r32"
               OpMemberName %Test3 7 "dummy3"
               OpName %MatrixStraddle2x3c "MatrixStraddle2x3c"
               OpMemberName %MatrixStraddle2x3c 0 "m"
               OpName %MatrixStraddle2x3r "MatrixStraddle2x3r"
               OpMemberName %MatrixStraddle2x3r 0 "m"
               OpName %MatrixStraddle3x2c "MatrixStraddle3x2c"
               OpMemberName %MatrixStraddle3x2c 0 "m"
               OpName %MatrixStraddle3x2r "MatrixStraddle3x2r"
               OpMemberName %MatrixStraddle3x2r 0 "m"
               OpName %Test3Cbuf "Test3Cbuf"
               OpName %type_Test4Cbuf "type.Test4Cbuf"
               OpMemberName %type_Test4Cbuf 0 "test4"
               OpName %Test4 "Test4"
               OpMemberName %Test4 0 "c23"
               OpMemberName %Test4 1 "dummy0"
               OpMemberName %Test4 2 "r23"
               OpMemberName %Test4 3 "dummy1"
               OpName %Test4Cbuf "Test4Cbuf"
               OpName %in_var_COLOR "in.var.COLOR"
               OpName %out_var_SV_TARGET "out.var.SV_TARGET"
               OpName %main "main"
               OpDecorate %in_var_COLOR Location 0
               OpDecorate %out_var_SV_TARGET Location 0
               OpDecorate %Test1Cbuf DescriptorSet 0
               OpDecorate %Test1Cbuf Binding 0
               OpDecorate %Test2Cbuf DescriptorSet 0
               OpDecorate %Test2Cbuf Binding 1
               OpDecorate %Test3Cbuf DescriptorSet 0
               OpDecorate %Test3Cbuf Binding 2
               OpDecorate %Test4Cbuf DescriptorSet 0
               OpDecorate %Test4Cbuf Binding 3
               OpMemberDecorate %StraddleResolve 0 Offset 0
               OpMemberDecorate %StraddleResolve 1 Offset 16
               OpMemberDecorate %StraddleResolve 2 Offset 32
               OpMemberDecorate %StraddleResolve 3 Offset 48
               OpMemberDecorate %Test1 0 Offset 0
               OpMemberDecorate %Test1 1 Offset 60
               OpMemberDecorate %type_Test1Cbuf 0 Offset 0
               OpDecorate %type_Test1Cbuf Block
               OpDecorate %_arr_StraddleResolve_uint_2 ArrayStride 64
               OpDecorate %_arr__arr_StraddleResolve_uint_2_uint_3 ArrayStride 128
               OpMemberDecorate %Test2 0 Offset 0
               OpMemberDecorate %Test2 1 Offset 124
               OpMemberDecorate %Test2 2 Offset 128
               OpMemberDecorate %Test2 3 Offset 508
               OpMemberDecorate %type_Test2Cbuf 0 Offset 0
               OpDecorate %type_Test2Cbuf Block
               OpMemberDecorate %MatrixStraddle2x3c 0 Offset 0
               OpMemberDecorate %MatrixStraddle2x3c 0 MatrixStride 16
               OpMemberDecorate %MatrixStraddle2x3c 0 RowMajor
               OpMemberDecorate %MatrixStraddle2x3r 0 Offset 0
               OpMemberDecorate %MatrixStraddle2x3r 0 MatrixStride 16
               OpMemberDecorate %MatrixStraddle2x3r 0 ColMajor
               OpMemberDecorate %MatrixStraddle3x2c 0 Offset 0
               OpMemberDecorate %MatrixStraddle3x2c 0 MatrixStride 16
               OpMemberDecorate %MatrixStraddle3x2c 0 RowMajor
               OpMemberDecorate %MatrixStraddle3x2r 0 Offset 0
               OpMemberDecorate %MatrixStraddle3x2r 0 MatrixStride 16
               OpMemberDecorate %MatrixStraddle3x2r 0 ColMajor
               OpMemberDecorate %Test3 0 Offset 0
               OpMemberDecorate %Test3 1 Offset 40
               OpMemberDecorate %Test3 2 Offset 48
               OpMemberDecorate %Test3 3 Offset 76
               OpMemberDecorate %Test3 4 Offset 80
               OpMemberDecorate %Test3 5 Offset 108
               OpMemberDecorate %Test3 6 Offset 112
               OpMemberDecorate %Test3 7 Offset 152
               OpMemberDecorate %type_Test3Cbuf 0 Offset 0
               OpDecorate %type_Test3Cbuf Block
               OpDecorate %_arr_MatrixStraddle2x3c_uint_3 ArrayStride 48
               OpDecorate %_arr__arr_MatrixStraddle2x3c_uint_3_uint_2 ArrayStride 144
               OpDecorate %_arr_MatrixStraddle2x3r_uint_3 ArrayStride 32
               OpDecorate %_arr__arr_MatrixStraddle2x3r_uint_3_uint_2 ArrayStride 96
               OpMemberDecorate %Test4 0 Offset 0
               OpMemberDecorate %Test4 1 Offset 280
               OpMemberDecorate %Test4 2 Offset 288
               OpMemberDecorate %Test4 3 Offset 476
               OpMemberDecorate %type_Test4Cbuf 0 Offset 0
               OpDecorate %type_Test4Cbuf Block
        %int = OpTypeInt 32 1
      %int_0 = OpConstant %int 0
      %int_1 = OpConstant %int 1
      %float = OpTypeFloat 32
    %v3float = OpTypeVector %float 3
%StraddleResolve = OpTypeStruct %v3float %v3float %v3float %v3float
      %Test1 = OpTypeStruct %StraddleResolve %float
%type_Test1Cbuf = OpTypeStruct %Test1
%_ptr_Uniform_type_Test1Cbuf = OpTypePointer Uniform %type_Test1Cbuf
       %uint = OpTypeInt 32 0
     %uint_2 = OpConstant %uint 2
%_arr_StraddleResolve_uint_2 = OpTypeArray %StraddleResolve %uint_2
     %uint_3 = OpConstant %uint 3
%_arr__arr_StraddleResolve_uint_2_uint_3 = OpTypeArray %_arr_StraddleResolve_uint_2 %uint_3
      %Test2 = OpTypeStruct %_arr_StraddleResolve_uint_2 %float %_arr__arr_StraddleResolve_uint_2_uint_3 %float
%type_Test2Cbuf = OpTypeStruct %Test2
%_ptr_Uniform_type_Test2Cbuf = OpTypePointer Uniform %type_Test2Cbuf
%mat2v3float = OpTypeMatrix %v3float 2
%MatrixStraddle2x3c = OpTypeStruct %mat2v3float
%MatrixStraddle2x3r = OpTypeStruct %mat2v3float
    %v2float = OpTypeVector %float 2
%mat3v2float = OpTypeMatrix %v2float 3
%MatrixStraddle3x2c = OpTypeStruct %mat3v2float
%MatrixStraddle3x2r = OpTypeStruct %mat3v2float
      %Test3 = OpTypeStruct %MatrixStraddle2x3c %float %MatrixStraddle2x3r %float %MatrixStraddle3x2c %float %MatrixStraddle3x2r %float
%type_Test3Cbuf = OpTypeStruct %Test3
%_ptr_Uniform_type_Test3Cbuf = OpTypePointer Uniform %type_Test3Cbuf
%_arr_MatrixStraddle2x3c_uint_3 = OpTypeArray %MatrixStraddle2x3c %uint_3
%_arr__arr_MatrixStraddle2x3c_uint_3_uint_2 = OpTypeArray %_arr_MatrixStraddle2x3c_uint_3 %uint_2
%_arr_MatrixStraddle2x3r_uint_3 = OpTypeArray %MatrixStraddle2x3r %uint_3
%_arr__arr_MatrixStraddle2x3r_uint_3_uint_2 = OpTypeArray %_arr_MatrixStraddle2x3r_uint_3 %uint_2
      %Test4 = OpTypeStruct %_arr__arr_MatrixStraddle2x3c_uint_3_uint_2 %float %_arr__arr_MatrixStraddle2x3r_uint_3_uint_2 %float
%type_Test4Cbuf = OpTypeStruct %Test4
%_ptr_Uniform_type_Test4Cbuf = OpTypePointer Uniform %type_Test4Cbuf
    %v4float = OpTypeVector %float 4
%_ptr_Input_v4float = OpTypePointer Input %v4float
%_ptr_Output_v4float = OpTypePointer Output %v4float
       %void = OpTypeVoid
         %46 = OpTypeFunction %void
%_ptr_Uniform_float = OpTypePointer Uniform %float
  %Test1Cbuf = OpVariable %_ptr_Uniform_type_Test1Cbuf Uniform
  %Test2Cbuf = OpVariable %_ptr_Uniform_type_Test2Cbuf Uniform
  %Test3Cbuf = OpVariable %_ptr_Uniform_type_Test3Cbuf Uniform
  %Test4Cbuf = OpVariable %_ptr_Uniform_type_Test4Cbuf Uniform
%in_var_COLOR = OpVariable %_ptr_Input_v4float Input
%out_var_SV_TARGET = OpVariable %_ptr_Output_v4float Output
       %main = OpFunction %void None %46
         %48 = OpLabel
         %49 = OpLoad %v4float %in_var_COLOR
         %50 = OpAccessChain %_ptr_Uniform_float %Test1Cbuf %int_0 %int_1
         %51 = OpLoad %float %50
         %52 = OpCompositeConstruct %v4float %51 %51 %51 %51
         %53 = OpFAdd %v4float %49 %52
         %54 = OpAccessChain %_ptr_Uniform_float %Test2Cbuf %int_0 %int_1
         %55 = OpLoad %float %54
         %56 = OpCompositeConstruct %v4float %55 %55 %55 %55
         %57 = OpFAdd %v4float %53 %56
         %58 = OpAccessChain %_ptr_Uniform_float %Test3Cbuf %int_0 %int_1
         %59 = OpLoad %float %58
         %60 = OpCompositeConstruct %v4float %59 %59 %59 %59
         %61 = OpFAdd %v4float %57 %60
         %62 = OpAccessChain %_ptr_Uniform_float %Test4Cbuf %int_0 %int_1
         %63 = OpLoad %float %62
         %64 = OpCompositeConstruct %v4float %63 %63 %63 %63
         %65 = OpFAdd %v4float %61 %64
               OpStore %out_var_SV_TARGET %65
               OpReturn
               OpFunctionEnd
