; SPIR-V
; Version: 1.0
; Generator: Khronos Glslang Reference Front End; 7
; Bound: 45
; Schema: 0
               OpCapability Shader
               OpCapability FragmentShaderPixelInterlockEXT
               OpExtension "SPV_EXT_fragment_shader_interlock"
          %1 = OpExtInstImport "GLSL.std.450"
               OpMemoryModel Logical GLSL450
               OpEntryPoint Fragment %main "main" %gl_FragCoord
               OpExecutionMode %main OriginUpperLeft
               OpExecutionMode %main PixelInterlockOrderedEXT
               OpSource GLSL 450
               OpSourceExtension "GL_ARB_fragment_shader_interlock"
               OpName %main "main"
               OpName %callee2_ "callee2("
               OpName %callee_ "callee("
               OpName %SSBO1 "SSBO1"
               OpMemberName %SSBO1 0 "values1"
               OpName %_ ""
               OpName %gl_FragCoord "gl_FragCoord"
               OpName %SSBO0 "SSBO0"
               OpMemberName %SSBO0 0 "values0"
               OpName %__0 ""
               OpDecorate %_runtimearr_uint ArrayStride 4
               OpMemberDecorate %SSBO1 0 Offset 0
               OpDecorate %SSBO1 BufferBlock
               OpDecorate %_ DescriptorSet 0
               OpDecorate %_ Binding 1
               OpDecorate %gl_FragCoord BuiltIn FragCoord
               OpDecorate %_runtimearr_uint_0 ArrayStride 4
               OpMemberDecorate %SSBO0 0 Offset 0
               OpDecorate %SSBO0 BufferBlock
               OpDecorate %__0 DescriptorSet 0
               OpDecorate %__0 Binding 0
       %void = OpTypeVoid
          %3 = OpTypeFunction %void
       %uint = OpTypeInt 32 0
%_runtimearr_uint = OpTypeRuntimeArray %uint
      %SSBO1 = OpTypeStruct %_runtimearr_uint
%_ptr_Uniform_SSBO1 = OpTypePointer Uniform %SSBO1
          %_ = OpVariable %_ptr_Uniform_SSBO1 Uniform
        %int = OpTypeInt 32 1
      %int_0 = OpConstant %int 0
      %float = OpTypeFloat 32
    %v4float = OpTypeVector %float 4
%_ptr_Input_v4float = OpTypePointer Input %v4float
%gl_FragCoord = OpVariable %_ptr_Input_v4float Input
     %uint_0 = OpConstant %uint 0
%_ptr_Input_float = OpTypePointer Input %float
     %uint_1 = OpConstant %uint 1
%_ptr_Uniform_uint = OpTypePointer Uniform %uint
%_runtimearr_uint_0 = OpTypeRuntimeArray %uint
      %SSBO0 = OpTypeStruct %_runtimearr_uint_0
%_ptr_Uniform_SSBO0 = OpTypePointer Uniform %SSBO0
        %__0 = OpVariable %_ptr_Uniform_SSBO0 Uniform
       %main = OpFunction %void None %3
          %5 = OpLabel
         %44 = OpFunctionCall %void %callee_
               OpReturn
               OpFunctionEnd
   %callee2_ = OpFunction %void None %3
          %7 = OpLabel
         %23 = OpAccessChain %_ptr_Input_float %gl_FragCoord %uint_0
         %24 = OpLoad %float %23
         %25 = OpConvertFToS %int %24
         %28 = OpAccessChain %_ptr_Uniform_uint %_ %int_0 %25
         %29 = OpLoad %uint %28
         %30 = OpIAdd %uint %29 %uint_1
         %31 = OpAccessChain %_ptr_Uniform_uint %_ %int_0 %25
               OpStore %31 %30
               OpReturn
               OpFunctionEnd
    %callee_ = OpFunction %void None %3
          %9 = OpLabel
         %36 = OpAccessChain %_ptr_Input_float %gl_FragCoord %uint_0
         %37 = OpLoad %float %36
         %38 = OpConvertFToS %int %37
         %39 = OpAccessChain %_ptr_Uniform_uint %__0 %int_0 %38
         %40 = OpLoad %uint %39
         %41 = OpIAdd %uint %40 %uint_1
         %42 = OpAccessChain %_ptr_Uniform_uint %__0 %int_0 %38
               OpStore %42 %41
               OpBeginInvocationInterlockEXT
         %43 = OpFunctionCall %void %callee2_
               OpEndInvocationInterlockEXT
               OpReturn
               OpFunctionEnd
