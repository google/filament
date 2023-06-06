; SPIR-V
; Version: 1.3
; Generator: Khronos Glslang Reference Front End; 7
; Bound: 72
; Schema: 0
               OpCapability Tessellation
          %1 = OpExtInstImport "GLSL.std.450"
               OpMemoryModel Logical GLSL450
               OpEntryPoint TessellationEvaluation %main "main" %_ %gl_TessCoord %gl_TessLevelInner %gl_TessLevelOuter
               OpExecutionMode %main Quads
               OpExecutionMode %main SpacingFractionalEven
               OpExecutionMode %main VertexOrderCw
               OpSource ESSL 310
               OpSourceExtension "GL_EXT_shader_io_blocks"
               OpSourceExtension "GL_EXT_tessellation_shader"
               OpMemberDecorate %gl_PerVertex 0 BuiltIn Position
               OpMemberDecorate %gl_PerVertex 1 BuiltIn PointSize
               OpDecorate %gl_PerVertex Block
               OpDecorate %gl_TessCoord BuiltIn TessCoord
               OpDecorate %gl_TessLevelInner Patch
               OpDecorate %gl_TessLevelInner BuiltIn TessLevelInner
               OpDecorate %gl_TessLevelOuter Patch
               OpDecorate %gl_TessLevelOuter BuiltIn TessLevelOuter
       %void = OpTypeVoid
          %3 = OpTypeFunction %void
      %float = OpTypeFloat 32
    %v4float = OpTypeVector %float 4
%gl_PerVertex = OpTypeStruct %v4float %float
%_ptr_Output_gl_PerVertex = OpTypePointer Output %gl_PerVertex
          %_ = OpVariable %_ptr_Output_gl_PerVertex Output
        %int = OpTypeInt 32 1
      %int_0 = OpConstant %int 0
    %v3float = OpTypeVector %float 3
%_ptr_Input_v3float = OpTypePointer Input %v3float
%gl_TessCoord = OpVariable %_ptr_Input_v3float Input
       %uint = OpTypeInt 32 0
     %uint_0 = OpConstant %uint 0
%_ptr_Input_float = OpTypePointer Input %float
     %uint_2 = OpConstant %uint 2
%_arr_float_uint_2 = OpTypeArray %float %uint_2
%_ptr_Input__arr_float_uint_2 = OpTypePointer Input %_arr_float_uint_2
%gl_TessLevelInner = OpVariable %_ptr_Input__arr_float_uint_2 Input
     %uint_4 = OpConstant %uint 4
%_arr_float_uint_4 = OpTypeArray %float %uint_4
%_ptr_Input__arr_float_uint_4 = OpTypePointer Input %_arr_float_uint_4
%gl_TessLevelOuter = OpVariable %_ptr_Input__arr_float_uint_4 Input
    %float_1 = OpConstant %float 1
      %int_2 = OpConstant %int 2
     %uint_1 = OpConstant %uint 1
      %int_1 = OpConstant %int 1
      %int_3 = OpConstant %int 3
    %float_0 = OpConstant %float 0
%_ptr_Output_v4float = OpTypePointer Output %v4float
       %main = OpFunction %void None %3
          %5 = OpLabel
         %19 = OpAccessChain %_ptr_Input_float %gl_TessCoord %uint_0
         %20 = OpLoad %float %19
         %25 = OpAccessChain %_ptr_Input_float %gl_TessLevelInner %int_0
         %26 = OpLoad %float %25
         %27 = OpFMul %float %20 %26
         %32 = OpAccessChain %_ptr_Input_float %gl_TessLevelOuter %int_0
         %33 = OpLoad %float %32
         %34 = OpFMul %float %27 %33
         %36 = OpAccessChain %_ptr_Input_float %gl_TessCoord %uint_0
         %37 = OpLoad %float %36
         %38 = OpFSub %float %float_1 %37
         %39 = OpAccessChain %_ptr_Input_float %gl_TessLevelInner %int_0
         %40 = OpLoad %float %39
         %41 = OpFMul %float %38 %40
         %43 = OpAccessChain %_ptr_Input_float %gl_TessLevelOuter %int_2
         %44 = OpLoad %float %43
         %45 = OpFMul %float %41 %44
         %46 = OpFAdd %float %34 %45
         %48 = OpAccessChain %_ptr_Input_float %gl_TessCoord %uint_1
         %49 = OpLoad %float %48
         %51 = OpAccessChain %_ptr_Input_float %gl_TessLevelInner %int_1
         %52 = OpLoad %float %51
         %53 = OpFMul %float %49 %52
         %54 = OpAccessChain %_ptr_Input_float %gl_TessLevelOuter %int_1
         %55 = OpLoad %float %54
         %56 = OpFMul %float %53 %55
         %57 = OpAccessChain %_ptr_Input_float %gl_TessCoord %uint_1
         %58 = OpLoad %float %57
         %59 = OpFSub %float %float_1 %58
         %60 = OpAccessChain %_ptr_Input_float %gl_TessLevelInner %int_1
         %61 = OpLoad %float %60
         %62 = OpFMul %float %59 %61
         %64 = OpAccessChain %_ptr_Input_float %gl_TessLevelOuter %int_3
         %65 = OpLoad %float %64
         %66 = OpFMul %float %62 %65
         %67 = OpFAdd %float %56 %66
         %69 = OpCompositeConstruct %v4float %46 %67 %float_0 %float_1
         %71 = OpAccessChain %_ptr_Output_v4float %_ %int_0
               OpStore %71 %69
               OpReturn
               OpFunctionEnd
