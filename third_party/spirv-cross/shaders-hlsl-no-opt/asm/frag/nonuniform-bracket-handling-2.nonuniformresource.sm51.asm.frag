; SPIR-V
; Version: 1.0
; Generator: Khronos Glslang Reference Front End; 10
; Bound: 53
; Schema: 0
               OpCapability Shader
               OpCapability ShaderNonUniform
               OpCapability RuntimeDescriptorArray
               OpCapability SampledImageArrayNonUniformIndexing
               OpExtension "SPV_EXT_descriptor_indexing"
          %1 = OpExtInstImport "GLSL.std.450"
               OpMemoryModel Logical GLSL450
               OpEntryPoint Fragment %main "main" %FragColor %vUV %gl_FragCoord
               OpExecutionMode %main OriginUpperLeft
               OpSource GLSL 450
               OpSourceExtension "GL_EXT_nonuniform_qualifier"
               OpName %main "main"
               OpName %FragColor "FragColor"
               OpName %uSamplers "uSamplers"
               OpName %SSBO "SSBO"
               OpMemberName %SSBO 0 "indices"
               OpName %_ ""
               OpName %vUV "vUV"
               OpName %uSampler "uSampler"
               OpName %gl_FragCoord "gl_FragCoord"
               OpDecorate %FragColor Location 0
               OpDecorate %uSamplers DescriptorSet 0
               OpDecorate %uSamplers Binding 0
               OpDecorate %_runtimearr_uint ArrayStride 4
               OpMemberDecorate %SSBO 0 NonWritable
               OpMemberDecorate %SSBO 0 Offset 0
               OpDecorate %SSBO BufferBlock
               OpDecorate %_ DescriptorSet 2
               OpDecorate %_ Binding 0
               OpDecorate %26 NonUniform
               OpDecorate %28 NonUniform
               OpDecorate %29 NonUniform
               OpDecorate %vUV Location 0
               OpDecorate %uSampler DescriptorSet 1
               OpDecorate %uSampler Binding 1
               OpDecorate %38 NonUniform
               OpDecorate %gl_FragCoord BuiltIn FragCoord
       %void = OpTypeVoid
          %3 = OpTypeFunction %void
      %float = OpTypeFloat 32
    %v4float = OpTypeVector %float 4
%_ptr_Output_v4float = OpTypePointer Output %v4float
  %FragColor = OpVariable %_ptr_Output_v4float Output
         %10 = OpTypeImage %float 2D 0 0 0 1 Unknown
         %11 = OpTypeSampledImage %10
%_runtimearr_11 = OpTypeRuntimeArray %11
%_ptr_UniformConstant__runtimearr_11 = OpTypePointer UniformConstant %_runtimearr_11
  %uSamplers = OpVariable %_ptr_UniformConstant__runtimearr_11 UniformConstant
       %uint = OpTypeInt 32 0
%_runtimearr_uint = OpTypeRuntimeArray %uint
       %SSBO = OpTypeStruct %_runtimearr_uint
%_ptr_Uniform_SSBO = OpTypePointer Uniform %SSBO
          %_ = OpVariable %_ptr_Uniform_SSBO Uniform
        %int = OpTypeInt 32 1
      %int_0 = OpConstant %int 0
     %int_10 = OpConstant %int 10
%_ptr_Uniform_uint = OpTypePointer Uniform %uint
%_ptr_UniformConstant_11 = OpTypePointer UniformConstant %11
    %v2float = OpTypeVector %float 2
%_ptr_Input_v2float = OpTypePointer Input %v2float
        %vUV = OpVariable %_ptr_Input_v2float Input
    %float_0 = OpConstant %float 0
   %uSampler = OpVariable %_ptr_UniformConstant_11 UniformConstant
%_ptr_Input_v4float = OpTypePointer Input %v4float
%gl_FragCoord = OpVariable %_ptr_Input_v4float Input
     %uint_1 = OpConstant %uint 1
%_ptr_Input_float = OpTypePointer Input %float
       %main = OpFunction %void None %3
          %5 = OpLabel
         %24 = OpAccessChain %_ptr_Uniform_uint %_ %int_0 %int_10
         %26 = OpLoad %uint %24
         %28 = OpAccessChain %_ptr_UniformConstant_11 %uSamplers %26
         %29 = OpLoad %11 %28
         %33 = OpLoad %v2float %vUV
         %35 = OpImageSampleExplicitLod %v4float %29 %33 Lod %float_0
               OpStore %FragColor %35
         %37 = OpLoad %11 %uSampler
         %38 = OpCopyObject %11 %37
         %39 = OpLoad %v2float %vUV
         %44 = OpAccessChain %_ptr_Input_float %gl_FragCoord %uint_1
         %45 = OpLoad %float %44
         %46 = OpConvertFToS %int %45
         %47 = OpAccessChain %_ptr_Uniform_uint %_ %int_0 %46
         %48 = OpLoad %uint %47
         %49 = OpConvertUToF %float %48
         %50 = OpImageSampleExplicitLod %v4float %38 %39 Lod %49
         %51 = OpLoad %v4float %FragColor
         %52 = OpFAdd %v4float %51 %50
               OpStore %FragColor %52
               OpReturn
               OpFunctionEnd
