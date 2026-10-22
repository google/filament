               OpCapability Shader
               OpMemoryModel Logical GLSL450
               OpEntryPoint Vertex %VSMain "main" %gl_VertexIndex %gl_Position
               OpSource HLSL 600
               OpName %type_Float2Array "type.Float2Array"
               OpMemberName %type_Float2Array 0 "arr"
               OpName %Float2Array "Float2Array"
               OpName %VSMain "VSMain"
               OpName %param_var_i "param.var.i"
               OpName %src_VSMain "src.VSMain"
               OpName %i "i"
               OpName %bb_entry "bb.entry"
               OpDecorate %gl_VertexIndex BuiltIn VertexIndex
               OpDecorate %gl_Position BuiltIn Position
               OpDecorate %Float2Array DescriptorSet 0
               OpDecorate %Float2Array Binding 0
               OpDecorate %_arr_v2float_uint_3 ArrayStride 16
               OpMemberDecorate %type_Float2Array 0 Offset 0
               OpDecorate %type_Float2Array Block
        %int = OpTypeInt 32 1
      %int_0 = OpConstant %int 0
      %float = OpTypeFloat 32
    %float_0 = OpConstant %float 0
    %float_1 = OpConstant %float 1
       %uint = OpTypeInt 32 0
     %uint_3 = OpConstant %uint 3
    %v2float = OpTypeVector %float 2
%_arr_v2float_uint_3 = OpTypeArray %v2float %uint_3
%type_Float2Array = OpTypeStruct %_arr_v2float_uint_3
%_ptr_Uniform_type_Float2Array = OpTypePointer Uniform %type_Float2Array
%_ptr_Input_uint = OpTypePointer Input %uint
    %v4float = OpTypeVector %float 4
%_ptr_Output_v4float = OpTypePointer Output %v4float
       %void = OpTypeVoid
         %20 = OpTypeFunction %void
%_ptr_Function_uint = OpTypePointer Function %uint
         %27 = OpTypeFunction %v4float %_ptr_Function_uint
%_ptr_Uniform__arr_v2float_uint_3 = OpTypePointer Uniform %_arr_v2float_uint_3
%_ptr_Uniform_v2float = OpTypePointer Uniform %v2float
%Float2Array = OpVariable %_ptr_Uniform_type_Float2Array Uniform
%gl_VertexIndex = OpVariable %_ptr_Input_uint Input
%gl_Position = OpVariable %_ptr_Output_v4float Output
     %VSMain = OpFunction %void None %20
         %21 = OpLabel
%param_var_i = OpVariable %_ptr_Function_uint Function
         %24 = OpLoad %uint %gl_VertexIndex
               OpStore %param_var_i %24
         %25 = OpFunctionCall %v4float %src_VSMain %param_var_i
               OpStore %gl_Position %25
               OpReturn
               OpFunctionEnd
 %src_VSMain = OpFunction %v4float None %27
          %i = OpFunctionParameter %_ptr_Function_uint
   %bb_entry = OpLabel
         %30 = OpLoad %uint %i
         %32 = OpAccessChain %_ptr_Uniform__arr_v2float_uint_3 %Float2Array %int_0
         %34 = OpAccessChain %_ptr_Uniform_v2float %32 %30
         %35 = OpLoad %v2float %34
         %36 = OpCompositeExtract %float %35 0
         %37 = OpCompositeExtract %float %35 1
         %38 = OpCompositeConstruct %v4float %36 %37 %float_0 %float_1
               OpReturnValue %38
               OpFunctionEnd
