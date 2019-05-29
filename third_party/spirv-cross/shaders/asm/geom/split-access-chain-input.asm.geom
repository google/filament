; SPIR-V
; Version: 1.0
; Generator: Khronos Glslang Reference Front End; 3
; Bound: 23
; Schema: 0
               OpCapability Geometry
          %1 = OpExtInstImport "GLSL.std.450"
               OpMemoryModel Logical GLSL450
               OpEntryPoint Geometry %main "main" %gl_in %position
               OpExecutionMode %main Triangles
               OpExecutionMode %main Invocations 1
               OpExecutionMode %main OutputTriangleStrip
               OpExecutionMode %main OutputVertices 3
               OpSource GLSL 440
               OpName %main "main"
               OpName %position "position"
               OpName %gl_PerVertex "gl_PerVertex"
               OpMemberName %gl_PerVertex 0 "gl_Position"
               OpMemberName %gl_PerVertex 1 "gl_PointSize"
               OpMemberName %gl_PerVertex 2 "gl_ClipDistance"
               OpName %gl_in "gl_in"
               OpMemberDecorate %gl_PerVertex 0 BuiltIn Position
               OpMemberDecorate %gl_PerVertex 1 BuiltIn PointSize
               OpMemberDecorate %gl_PerVertex 2 BuiltIn ClipDistance
               OpDecorate %gl_PerVertex Block
			   OpDecorate %position BuiltIn Position
       %void = OpTypeVoid
          %3 = OpTypeFunction %void
      %float = OpTypeFloat 32
    %v4float = OpTypeVector %float 4
%_ptr_Function_v4float = OpTypePointer Output %v4float
       %uint = OpTypeInt 32 0
     %uint_1 = OpConstant %uint 1
%_arr_float_uint_1 = OpTypeArray %float %uint_1
%gl_PerVertex = OpTypeStruct %v4float %float %_arr_float_uint_1
     %uint_3 = OpConstant %uint 3
%_arr_gl_PerVertex_uint_3 = OpTypeArray %gl_PerVertex %uint_3
%ptr_Input_gl_PerVertex = OpTypePointer Input %gl_PerVertex
%_ptr_Input__arr_gl_PerVertex_uint_3 = OpTypePointer Input %_arr_gl_PerVertex_uint_3
      %gl_in = OpVariable %_ptr_Input__arr_gl_PerVertex_uint_3 Input
   %position = OpVariable %_ptr_Function_v4float Output
        %int = OpTypeInt 32 1
      %int_0 = OpConstant %int 0
%_ptr_Input_v4float = OpTypePointer Input %v4float
       %main = OpFunction %void None %3
          %5 = OpLabel
         %21 = OpAccessChain %ptr_Input_gl_PerVertex %gl_in %int_0
         %22 = OpAccessChain %_ptr_Input_v4float %21 %int_0
         %23 = OpLoad %v4float %22
               OpStore %position %23
               OpReturn
               OpFunctionEnd
