; SPIR-V
; Version: 1.0
; Generator: Khronos Glslang Reference Front End; 10
; Bound: 23
; Schema: 0
               OpCapability Shader
               OpCapability MultiView
               OpExtension "SPV_KHR_multiview"
          %1 = OpExtInstImport "GLSL.std.450"
               OpMemoryModel Logical GLSL450
               OpEntryPoint Vertex %main "main" %_ %gl_ViewIndex
               OpEntryPoint Vertex %main2 "main2" %_ %gl_ViewIndex2
               OpSource GLSL 450
               OpSourceExtension "GL_EXT_multiview"
               OpName %main "main"
               OpName %gl_PerVertex "gl_PerVertex"
               OpMemberName %gl_PerVertex 0 "gl_Position"
               OpMemberName %gl_PerVertex 1 "gl_PointSize"
               OpMemberName %gl_PerVertex 2 "gl_ClipDistance"
               OpMemberName %gl_PerVertex 3 "gl_CullDistance"
               OpName %_ ""
               OpName %gl_ViewIndex "gl_ViewIndex"
               OpMemberDecorate %gl_PerVertex 0 BuiltIn Position
               OpMemberDecorate %gl_PerVertex 1 BuiltIn PointSize
               OpMemberDecorate %gl_PerVertex 2 BuiltIn ClipDistance
               OpMemberDecorate %gl_PerVertex 3 BuiltIn CullDistance
               OpDecorate %gl_PerVertex Block
               OpDecorate %gl_ViewIndex BuiltIn ViewIndex
               OpDecorate %gl_ViewIndex2 BuiltIn ViewIndex
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
%_ptr_Input_int = OpTypePointer Input %int
%gl_ViewIndex = OpVariable %_ptr_Input_int Input
%gl_ViewIndex2 = OpVariable %_ptr_Input_int Input
%_ptr_Output_v4float = OpTypePointer Output %v4float

       %main = OpFunction %void None %3
          %5 = OpLabel
         %18 = OpLoad %int %gl_ViewIndex
         %19 = OpConvertSToF %float %18
         %20 = OpCompositeConstruct %v4float %19 %19 %19 %19
         %22 = OpAccessChain %_ptr_Output_v4float %_ %int_0
               OpStore %22 %20
               OpReturn
               OpFunctionEnd

       %main2 = OpFunction %void None %3
          %100 = OpLabel
         %101 = OpLoad %int %gl_ViewIndex2
         %102 = OpConvertSToF %float %101
         %103 = OpCompositeConstruct %v4float %102 %102 %102 %102
         %104 = OpAccessChain %_ptr_Output_v4float %_ %int_0
               OpStore %104 %103

               OpReturn
               OpFunctionEnd
