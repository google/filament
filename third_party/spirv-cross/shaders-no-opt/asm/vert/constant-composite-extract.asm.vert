; SPIR-V
; Version: 1.0
; Generator: Khronos Glslang Reference Front End; 10
; Bound: 22
; Schema: 0
               OpCapability Shader
          %1 = OpExtInstImport "GLSL.std.450"
               OpMemoryModel Logical GLSL450
               OpEntryPoint Vertex %main "main" %_
               OpSource GLSL 450
               OpName %main "main"
               OpName %gl_PerVertex "gl_PerVertex"
               OpMemberName %gl_PerVertex 0 "gl_Position"
               OpMemberName %gl_PerVertex 1 "gl_PointSize"
               OpMemberName %gl_PerVertex 2 "gl_ClipDistance"
               OpMemberName %gl_PerVertex 3 "gl_CullDistance"
               OpName %_ ""
               OpMemberDecorate %gl_PerVertex 0 BuiltIn Position
               OpMemberDecorate %gl_PerVertex 1 BuiltIn PointSize
               OpMemberDecorate %gl_PerVertex 2 BuiltIn ClipDistance
               OpMemberDecorate %gl_PerVertex 3 BuiltIn CullDistance
               OpDecorate %gl_PerVertex Block
       %void = OpTypeVoid
          %3 = OpTypeFunction %void
      %float = OpTypeFloat 32
    %v4float = OpTypeVector %float 4
	%m4float = OpTypeMatrix %v4float 4
       %uint = OpTypeInt 32 0
     %uint_1 = OpConstant %uint 1
%_arr_float_uint_1 = OpTypeArray %float %uint_1
%gl_PerVertex = OpTypeStruct %v4float %float %_arr_float_uint_1 %_arr_float_uint_1
%_ptr_Output_gl_PerVertex = OpTypePointer Output %gl_PerVertex
          %_ = OpVariable %_ptr_Output_gl_PerVertex Output
        %int = OpTypeInt 32 1
      %int_0 = OpConstant %int 0
    %float_1 = OpConstant %float 1
    %float_2 = OpConstant %float 2
    %float_3 = OpConstant %float 3
    %float_4 = OpConstant %float 4
    %float_5 = OpConstant %float 5
    %float_6 = OpConstant %float 6
    %float_7 = OpConstant %float 7
    %float_8 = OpConstant %float 8
         %vec0 = OpConstantComposite %v4float %float_1 %float_2 %float_3 %float_4
         %vec1 = OpConstantComposite %v4float %float_5 %float_6 %float_7 %float_8
		 %cmat = OpConstantComposite %m4float %vec0 %vec1 %vec0 %vec1
%_ptr_Output_v4float = OpTypePointer Output %v4float
       %main = OpFunction %void None %3
          %5 = OpLabel
         %21 = OpAccessChain %_ptr_Output_v4float %_ %int_0
		 	%e0 = OpCompositeExtract %float %vec0 0
		 	%e1 = OpCompositeExtract %float %vec0 1
		 	%e2 = OpCompositeExtract %float %vec0 2
		 	%e3 = OpCompositeExtract %float %vec0 3
			%m13 = OpCompositeExtract %float %cmat 1 3
			%m21 = OpCompositeExtract %float %cmat 2 1
			%e_front = OpCompositeConstruct %v4float %e0 %e1 %e2 %e3
			%e_back = OpCompositeConstruct %v4float %e3 %e2 %m13 %m21
			%m0 = OpCompositeExtract %v4float %cmat 2
			%m1 = OpCompositeExtract %v4float %cmat 3
			%sum0 =	OpFAdd %v4float %m0 %m1
			%sum1 =	OpFAdd %v4float %e_front %e_back
			%sum = OpFAdd %v4float %sum0 %sum1
               OpStore %21 %sum
               OpReturn
               OpFunctionEnd
