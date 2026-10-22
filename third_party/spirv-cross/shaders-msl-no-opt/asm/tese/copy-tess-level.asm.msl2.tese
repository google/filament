; SPIR-V
; Version: 1.0
; Generator: Google spiregg; 0
; Bound: 35
; Schema: 0
               OpCapability Tessellation
               OpMemoryModel Logical GLSL450
               OpEntryPoint TessellationEvaluation %Domain "main" %gl_TessLevelOuter %gl_TessLevelInner %in_var_CUSTOM_VALUE %gl_TessCoord %out_var_CUSTOM_VALUE
               OpExecutionMode %Domain Quads
               OpSource HLSL 600
               OpName %in_var_CUSTOM_VALUE "in.var.CUSTOM_VALUE"
               OpName %out_var_CUSTOM_VALUE "out.var.CUSTOM_VALUE"
               OpName %Domain "Domain"
               OpDecorate %gl_TessLevelOuter BuiltIn TessLevelOuter
               OpDecorate %gl_TessLevelOuter Patch
               OpDecorate %gl_TessLevelInner BuiltIn TessLevelInner
               OpDecorate %gl_TessLevelInner Patch
               OpDecorate %gl_TessCoord BuiltIn TessCoord
               OpDecorate %gl_TessCoord Patch
               OpDecorate %in_var_CUSTOM_VALUE Location 0
               OpDecorate %out_var_CUSTOM_VALUE Location 0
       %uint = OpTypeInt 32 0
     %uint_4 = OpConstant %uint 4
      %float = OpTypeFloat 32
%_arr_float_uint_4 = OpTypeArray %float %uint_4
%_ptr_Input__arr_float_uint_4 = OpTypePointer Input %_arr_float_uint_4
     %uint_2 = OpConstant %uint 2
%_arr_float_uint_2 = OpTypeArray %float %uint_2
%_ptr_Input__arr_float_uint_2 = OpTypePointer Input %_arr_float_uint_2
    %v4float = OpTypeVector %float 4
%_arr_v4float_uint_4 = OpTypeArray %v4float %uint_4
%_ptr_Input__arr_v4float_uint_4 = OpTypePointer Input %_arr_v4float_uint_4
    %v3float = OpTypeVector %float 3
%_ptr_Input_v3float = OpTypePointer Input %v3float
%_ptr_Output_v4float = OpTypePointer Output %v4float
       %void = OpTypeVoid
         %22 = OpTypeFunction %void
%gl_TessLevelOuter = OpVariable %_ptr_Input__arr_float_uint_4 Input
%gl_TessLevelInner = OpVariable %_ptr_Input__arr_float_uint_2 Input
%in_var_CUSTOM_VALUE = OpVariable %_ptr_Input__arr_v4float_uint_4 Input
%gl_TessCoord = OpVariable %_ptr_Input_v3float Input
%out_var_CUSTOM_VALUE = OpVariable %_ptr_Output_v4float Output
     %Domain = OpFunction %void None %22
         %23 = OpLabel
         %24 = OpLoad %_arr_float_uint_4 %gl_TessLevelOuter
         %25 = OpLoad %_arr_float_uint_2 %gl_TessLevelInner
         %26 = OpCompositeExtract %float %24 0
         %27 = OpCompositeExtract %float %24 1
         %28 = OpCompositeExtract %float %24 2
         %29 = OpCompositeExtract %float %24 3
         %30 = OpCompositeExtract %float %25 0
         %31 = OpCompositeExtract %float %25 1
         %32 = OpFAdd %float %26 %30
         %33 = OpFAdd %float %27 %31
         %34 = OpCompositeConstruct %v4float %32 %33 %28 %29
               OpStore %out_var_CUSTOM_VALUE %34
               OpReturn
               OpFunctionEnd
