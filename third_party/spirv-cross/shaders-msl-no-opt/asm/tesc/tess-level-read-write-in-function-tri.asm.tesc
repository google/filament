; SPIR-V
; Version: 1.0
; Generator: Khronos Glslang Reference Front End; 10
; Bound: 64
; Schema: 0
               OpCapability Tessellation
          %1 = OpExtInstImport "GLSL.std.450"
               OpMemoryModel Logical GLSL450
               OpEntryPoint TessellationControl %main "main" %gl_TessLevelInner %gl_TessLevelOuter %gl_out %gl_InvocationID
               OpExecutionMode %main OutputVertices 1
			   OpExecutionMode %main Triangles
               OpSource GLSL 450
               OpName %main "main"
               OpName %load_tess_level_in_func_ "load_tess_level_in_func("
               OpName %store_tess_level_in_func_ "store_tess_level_in_func("
               OpName %gl_TessLevelInner "gl_TessLevelInner"
               OpName %gl_TessLevelOuter "gl_TessLevelOuter"
               OpName %v "v"
               OpName %gl_PerVertex "gl_PerVertex"
               OpMemberName %gl_PerVertex 0 "gl_Position"
               OpMemberName %gl_PerVertex 1 "gl_PointSize"
               OpMemberName %gl_PerVertex 2 "gl_ClipDistance"
               OpMemberName %gl_PerVertex 3 "gl_CullDistance"
               OpName %gl_out "gl_out"
               OpName %gl_InvocationID "gl_InvocationID"
               OpDecorate %gl_TessLevelInner Patch
               OpDecorate %gl_TessLevelInner BuiltIn TessLevelInner
               OpDecorate %gl_TessLevelOuter Patch
               OpDecorate %gl_TessLevelOuter BuiltIn TessLevelOuter
               OpMemberDecorate %gl_PerVertex 0 BuiltIn Position
               OpMemberDecorate %gl_PerVertex 1 BuiltIn PointSize
               OpMemberDecorate %gl_PerVertex 2 BuiltIn ClipDistance
               OpMemberDecorate %gl_PerVertex 3 BuiltIn CullDistance
               OpDecorate %gl_PerVertex Block
               OpDecorate %gl_InvocationID BuiltIn InvocationId
       %void = OpTypeVoid
          %3 = OpTypeFunction %void
      %float = OpTypeFloat 32
          %7 = OpTypeFunction %float
       %uint = OpTypeInt 32 0
     %uint_2 = OpConstant %uint 2
%_arr_float_uint_2 = OpTypeArray %float %uint_2
%_ptr_Output__arr_float_uint_2 = OpTypePointer Output %_arr_float_uint_2
%gl_TessLevelInner = OpVariable %_ptr_Output__arr_float_uint_2 Output
        %int = OpTypeInt 32 1
      %int_0 = OpConstant %int 0
%_ptr_Output_float = OpTypePointer Output %float
     %uint_4 = OpConstant %uint 4
%_arr_float_uint_4 = OpTypeArray %float %uint_4
%_ptr_Output__arr_float_uint_4 = OpTypePointer Output %_arr_float_uint_4
%gl_TessLevelOuter = OpVariable %_ptr_Output__arr_float_uint_4 Output
      %int_1 = OpConstant %int 1
    %float_1 = OpConstant %float 1
    %float_2 = OpConstant %float 2
    %float_3 = OpConstant %float 3
    %float_4 = OpConstant %float 4
      %int_2 = OpConstant %int 2
    %float_5 = OpConstant %float 5
      %int_3 = OpConstant %int 3
    %float_6 = OpConstant %float 6
%_ptr_Function_float = OpTypePointer Function %float
    %v4float = OpTypeVector %float 4
     %uint_1 = OpConstant %uint 1
%_arr_float_uint_1 = OpTypeArray %float %uint_1
%gl_PerVertex = OpTypeStruct %v4float %float %_arr_float_uint_1 %_arr_float_uint_1
%_arr_gl_PerVertex_uint_1 = OpTypeArray %gl_PerVertex %uint_1
%_ptr_Output__arr_gl_PerVertex_uint_1 = OpTypePointer Output %_arr_gl_PerVertex_uint_1
     %gl_out = OpVariable %_ptr_Output__arr_gl_PerVertex_uint_1 Output
%_ptr_Input_int = OpTypePointer Input %int
%gl_InvocationID = OpVariable %_ptr_Input_int Input
%_ptr_Output_v4float = OpTypePointer Output %v4float
       %main = OpFunction %void None %3
          %5 = OpLabel
          %v = OpVariable %_ptr_Function_float Function
         %46 = OpFunctionCall %void %store_tess_level_in_func_
         %49 = OpFunctionCall %float %load_tess_level_in_func_
               OpStore %v %49
         %59 = OpLoad %int %gl_InvocationID
         %60 = OpLoad %float %v
         %61 = OpCompositeConstruct %v4float %60 %60 %60 %60
         %63 = OpAccessChain %_ptr_Output_v4float %gl_out %59 %int_0
               OpStore %63 %61
               OpReturn
               OpFunctionEnd
%load_tess_level_in_func_ = OpFunction %float None %7
          %9 = OpLabel
         %20 = OpAccessChain %_ptr_Output_float %gl_TessLevelInner %int_0
         %21 = OpLoad %float %20
         %27 = OpAccessChain %_ptr_Output_float %gl_TessLevelOuter %int_1
         %28 = OpLoad %float %27
         %29 = OpFAdd %float %21 %28
               OpReturnValue %29
               OpFunctionEnd
%store_tess_level_in_func_ = OpFunction %void None %3
         %11 = OpLabel
         %33 = OpAccessChain %_ptr_Output_float %gl_TessLevelInner %int_0
               OpStore %33 %float_1
         %35 = OpAccessChain %_ptr_Output_float %gl_TessLevelInner %int_1
               OpStore %35 %float_2
         %37 = OpAccessChain %_ptr_Output_float %gl_TessLevelOuter %int_0
               OpStore %37 %float_3
         %39 = OpAccessChain %_ptr_Output_float %gl_TessLevelOuter %int_1
               OpStore %39 %float_4
         %42 = OpAccessChain %_ptr_Output_float %gl_TessLevelOuter %int_2
               OpStore %42 %float_5
         %45 = OpAccessChain %_ptr_Output_float %gl_TessLevelOuter %int_3
               OpStore %45 %float_6
               OpReturn
               OpFunctionEnd
