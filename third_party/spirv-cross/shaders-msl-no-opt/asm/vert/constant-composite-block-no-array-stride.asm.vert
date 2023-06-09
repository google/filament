; SPIR-V
; Version: 1.0
; Generator: Khronos SPIR-V Tools Assembler; 0
; Bound: 121
; Schema: 0
               OpCapability Shader
               OpMemoryModel Logical GLSL450
               OpEntryPoint Vertex %1 "main" %2 %3 %4 %5 %gl_VertexIndex %gl_InstanceIndex
               OpMemberDecorate %_struct_8 0 BuiltIn Position
               OpMemberDecorate %_struct_8 1 BuiltIn PointSize
               OpMemberDecorate %_struct_8 2 BuiltIn ClipDistance
               OpMemberDecorate %_struct_8 3 BuiltIn CullDistance
               OpDecorate %_struct_8 Block
               OpDecorate %3 Location 0
               OpDecorate %4 Location 1
               OpDecorate %5 Location 1
               OpDecorate %gl_VertexIndex BuiltIn VertexIndex
               OpDecorate %gl_InstanceIndex BuiltIn InstanceIndex
               OpDecorate %9 ArrayStride 4
               OpDecorate %10 Offset 0
          %9 = OpDecorationGroup
         %10 = OpDecorationGroup
               OpDecorate %11 RelaxedPrecision
               OpDecorate %12 RelaxedPrecision
               OpDecorate %12 Flat
               OpDecorate %12 Restrict
         %13 = OpDecorationGroup
         %11 = OpDecorationGroup
         %12 = OpDecorationGroup
               OpGroupMemberDecorate %10 %_struct_14 0 %_struct_15 0
       %void = OpTypeVoid
       %bool = OpTypeBool
        %int = OpTypeInt 32 1
       %uint = OpTypeInt 32 0
      %float = OpTypeFloat 32
      %v2int = OpTypeVector %int 2
     %v2uint = OpTypeVector %uint 2
    %v2float = OpTypeVector %float 2
      %v3int = OpTypeVector %int 3
     %v3uint = OpTypeVector %uint 3
    %v3float = OpTypeVector %float 3
      %v4int = OpTypeVector %int 4
     %v4uint = OpTypeVector %uint 4
    %v4float = OpTypeVector %float 4
     %v4bool = OpTypeVector %bool 4
         %31 = OpTypeFunction %v4float %v4float
         %32 = OpTypeFunction %bool
         %33 = OpTypeFunction %void
%_ptr_Input_float = OpTypePointer Input %float
%_ptr_Input_int = OpTypePointer Input %int
%_ptr_Input_uint = OpTypePointer Input %uint
%_ptr_Input_v2float = OpTypePointer Input %v2float
%_ptr_Input_v2int = OpTypePointer Input %v2int
%_ptr_Input_v2uint = OpTypePointer Input %v2uint
%_ptr_Input_v3float = OpTypePointer Input %v3float
%_ptr_Input_v4float = OpTypePointer Input %v4float
%_ptr_Input_v4int = OpTypePointer Input %v4int
%_ptr_Input_v4uint = OpTypePointer Input %v4uint
%_ptr_Output_float = OpTypePointer Output %float
%_ptr_Output_int = OpTypePointer Output %int
%_ptr_Output_uint = OpTypePointer Output %uint
%_ptr_Output_v2float = OpTypePointer Output %v2float
%_ptr_Output_v2int = OpTypePointer Output %v2int
%_ptr_Output_v2uint = OpTypePointer Output %v2uint
%_ptr_Output_v4float = OpTypePointer Output %v4float
%_ptr_Output_v4int = OpTypePointer Output %v4int
%_ptr_Output_v4uint = OpTypePointer Output %v4uint
%_ptr_Function_float = OpTypePointer Function %float
%_ptr_Function_int = OpTypePointer Function %int
%_ptr_Function_v4float = OpTypePointer Function %v4float
    %float_1 = OpConstant %float 1
    %float_0 = OpConstant %float 0
  %float_0_5 = OpConstant %float 0.5
   %float_n1 = OpConstant %float -1
    %float_7 = OpConstant %float 7
    %float_8 = OpConstant %float 8
      %int_0 = OpConstant %int 0
      %int_1 = OpConstant %int 1
      %int_2 = OpConstant %int 2
      %int_3 = OpConstant %int 3
      %int_4 = OpConstant %int 4
     %uint_0 = OpConstant %uint 0
     %uint_1 = OpConstant %uint 1
     %uint_2 = OpConstant %uint 2
     %uint_3 = OpConstant %uint 3
    %uint_32 = OpConstant %uint 32
     %uint_4 = OpConstant %uint 4
%uint_2147483647 = OpConstant %uint 2147483647
         %74 = OpConstantComposite %v4float %float_1 %float_1 %float_1 %float_1
         %75 = OpConstantComposite %v4float %float_1 %float_0 %float_0 %float_1
         %76 = OpConstantComposite %v4float %float_0_5 %float_0_5 %float_0_5 %float_0_5
%_arr_float_uint_1 = OpTypeArray %float %uint_1
%_arr_float_uint_2 = OpTypeArray %float %uint_2
%_arr_v4float_uint_3 = OpTypeArray %v4float %uint_3
%_arr_float_uint_4 = OpTypeArray %float %uint_4
%_arr_v4float_uint_32 = OpTypeArray %v4float %uint_32
%_ptr_Input__arr_v4float_uint_3 = OpTypePointer Input %_arr_v4float_uint_3
%_ptr_Input__arr_v4float_uint_32 = OpTypePointer Input %_arr_v4float_uint_32
%_ptr_Output__arr_float_uint_2 = OpTypePointer Output %_arr_float_uint_2
%_ptr_Output__arr_v4float_uint_3 = OpTypePointer Output %_arr_v4float_uint_3
%_ptr_Output__arr_float_uint_4 = OpTypePointer Output %_arr_float_uint_4
  %_struct_8 = OpTypeStruct %v4float %float %_arr_float_uint_1 %_arr_float_uint_1
%_ptr_Output__struct_8 = OpTypePointer Output %_struct_8
          %2 = OpVariable %_ptr_Output__struct_8 Output
          %3 = OpVariable %_ptr_Input_v4float Input
          %4 = OpVariable %_ptr_Output_v4float Output
          %5 = OpVariable %_ptr_Input_v4float Input
%gl_VertexIndex = OpVariable %_ptr_Input_int Input
%gl_InstanceIndex = OpVariable %_ptr_Input_int Input
%_arr_float_uint_3 = OpTypeArray %float %uint_3
 %_struct_14 = OpTypeStruct %_arr_float_uint_3
 %_struct_15 = OpTypeStruct %_arr_float_uint_3
%_ptr_Function__struct_14 = OpTypePointer Function %_struct_14
%_ptr_Function__struct_15 = OpTypePointer Function %_struct_15
    %float_2 = OpConstant %float 2
   %float_n2 = OpConstant %float -2
         %93 = OpConstantComposite %_arr_float_uint_3 %float_1 %float_2 %float_1
         %94 = OpConstantComposite %_arr_float_uint_3 %float_n1 %float_n2 %float_n1
         %95 = OpConstantComposite %_struct_14 %93
         %96 = OpConstantComposite %_struct_15 %94
          %1 = OpFunction %void None %33
         %97 = OpLabel
         %98 = OpLoad %v4float %3
         %99 = OpAccessChain %_ptr_Output_v4float %2 %int_0
               OpStore %99 %98
        %100 = OpLoad %v4float %5
        %101 = OpFunctionCall %v4float %102 %100
               OpStore %4 %101
               OpReturn
               OpFunctionEnd
        %103 = OpFunction %bool None %32
        %104 = OpLabel
        %105 = OpLoad %int %gl_VertexIndex
        %106 = OpIEqual %bool %105 %int_0
               OpReturnValue %106
               OpFunctionEnd
        %102 = OpFunction %v4float None %31
        %107 = OpFunctionParameter %v4float
        %108 = OpLabel
        %109 = OpVariable %_ptr_Function_v4float Function
        %110 = OpVariable %_ptr_Function__struct_14 Function
        %111 = OpVariable %_ptr_Function__struct_15 Function
               OpStore %109 %107
               OpStore %110 %95
               OpStore %111 %96
        %112 = OpAccessChain %_ptr_Function_float %110 %int_0 %int_2
        %113 = OpLoad %float %112
        %114 = OpAccessChain %_ptr_Function_float %111 %int_0 %int_2
        %115 = OpLoad %float %114
        %116 = OpFAdd %float %113 %115
        %117 = OpAccessChain %_ptr_Function_float %109 %int_1
        %118 = OpLoad %float %117
        %119 = OpFAdd %float %116 %118
               OpStore %117 %119
        %120 = OpLoad %v4float %109
               OpReturnValue %120
               OpFunctionEnd
