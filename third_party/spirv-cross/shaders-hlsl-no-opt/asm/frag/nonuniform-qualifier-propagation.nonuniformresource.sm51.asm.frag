; SPIR-V
; Version: 1.0
; Generator: Khronos Glslang Reference Front End; 7
; Bound: 93
; Schema: 0
               OpCapability Shader
               OpCapability ShaderNonUniformEXT
               OpCapability RuntimeDescriptorArrayEXT
               OpCapability UniformBufferArrayNonUniformIndexingEXT
               OpCapability SampledImageArrayNonUniformIndexingEXT
               OpCapability StorageBufferArrayNonUniformIndexingEXT
               OpExtension "SPV_EXT_descriptor_indexing"
          %1 = OpExtInstImport "GLSL.std.450"
               OpMemoryModel Logical GLSL450
               OpEntryPoint Fragment %main "main" %vIndex %FragColor %vUV
               OpExecutionMode %main OriginUpperLeft
               OpSource GLSL 450
               OpSourceExtension "GL_EXT_nonuniform_qualifier"
               OpName %main "main"
               OpName %i "i"
               OpName %vIndex "vIndex"
               OpName %FragColor "FragColor"
               OpName %uSamplers "uSamplers"
               OpName %uSamps "uSamps"
               OpName %vUV "vUV"
               OpName %uCombinedSamplers "uCombinedSamplers"
               OpName %UBO "UBO"
               OpMemberName %UBO 0 "v"
               OpName %ubos "ubos"
               OpName %SSBO "SSBO"
               OpMemberName %SSBO 0 "v"
               OpName %ssbos "ssbos"
               OpDecorate %vIndex Flat
               OpDecorate %vIndex Location 0
               OpDecorate %FragColor Location 0
               OpDecorate %uSamplers DescriptorSet 0
               OpDecorate %uSamplers Binding 0

               OpDecorate %sampled_image NonUniformEXT
               OpDecorate %combined_sampler NonUniformEXT
               OpDecorate %ubo_ptr_copy NonUniformEXT
               OpDecorate %ssbo_ptr_copy NonUniformEXT

               OpDecorate %uSamps DescriptorSet 1
               OpDecorate %uSamps Binding 0
               OpDecorate %vUV Location 1
               OpDecorate %uCombinedSamplers DescriptorSet 0
               OpDecorate %uCombinedSamplers Binding 4
               OpDecorate %_arr_v4float_uint_64 ArrayStride 16
               OpMemberDecorate %UBO 0 Offset 0
               OpDecorate %UBO Block
               OpDecorate %ubos DescriptorSet 2
               OpDecorate %ubos Binding 0
               OpDecorate %_runtimearr_v4float ArrayStride 16
               OpMemberDecorate %SSBO 0 NonWritable
               OpMemberDecorate %SSBO 0 Offset 0
               OpDecorate %SSBO BufferBlock
               OpDecorate %ssbos DescriptorSet 3
               OpDecorate %ssbos Binding 0
       %void = OpTypeVoid
          %3 = OpTypeFunction %void
        %int = OpTypeInt 32 1
%_ptr_Function_int = OpTypePointer Function %int
%_ptr_Input_int = OpTypePointer Input %int
     %vIndex = OpVariable %_ptr_Input_int Input
      %float = OpTypeFloat 32
    %v4float = OpTypeVector %float 4
%_ptr_Output_v4float = OpTypePointer Output %v4float
  %FragColor = OpVariable %_ptr_Output_v4float Output
         %16 = OpTypeImage %float 2D 0 0 0 1 Unknown
%_runtimearr_16 = OpTypeRuntimeArray %16
%_ptr_UniformConstant__runtimearr_16 = OpTypePointer UniformConstant %_runtimearr_16
  %uSamplers = OpVariable %_ptr_UniformConstant__runtimearr_16 UniformConstant
     %int_10 = OpConstant %int 10
%_ptr_UniformConstant_16 = OpTypePointer UniformConstant %16
         %27 = OpTypeSampler
%_runtimearr_27 = OpTypeRuntimeArray %27
%_ptr_UniformConstant__runtimearr_27 = OpTypePointer UniformConstant %_runtimearr_27
     %uSamps = OpVariable %_ptr_UniformConstant__runtimearr_27 UniformConstant
     %int_40 = OpConstant %int 40
%_ptr_UniformConstant_27 = OpTypePointer UniformConstant %27
         %38 = OpTypeSampledImage %16
    %v2float = OpTypeVector %float 2
%_ptr_Input_v2float = OpTypePointer Input %v2float
        %vUV = OpVariable %_ptr_Input_v2float Input
%_runtimearr_38 = OpTypeRuntimeArray %38
%_ptr_UniformConstant__runtimearr_38 = OpTypePointer UniformConstant %_runtimearr_38
%uCombinedSamplers = OpVariable %_ptr_UniformConstant__runtimearr_38 UniformConstant
%_ptr_UniformConstant_38 = OpTypePointer UniformConstant %38
       %uint = OpTypeInt 32 0
    %uint_64 = OpConstant %uint 64
%_arr_v4float_uint_64 = OpTypeArray %v4float %uint_64
        %UBO = OpTypeStruct %_arr_v4float_uint_64
%_runtimearr_UBO = OpTypeRuntimeArray %UBO
%_ptr_Uniform__runtimearr_UBO = OpTypePointer Uniform %_runtimearr_UBO
       %ubos = OpVariable %_ptr_Uniform__runtimearr_UBO Uniform
     %int_20 = OpConstant %int 20
      %int_0 = OpConstant %int 0
%_ptr_Uniform_v4float = OpTypePointer Uniform %v4float
%_runtimearr_v4float = OpTypeRuntimeArray %v4float
       %SSBO = OpTypeStruct %_runtimearr_v4float
%_runtimearr_SSBO = OpTypeRuntimeArray %SSBO
%_ptr_Uniform__runtimearr_SSBO = OpTypePointer Uniform %_runtimearr_SSBO
      %ssbos = OpVariable %_ptr_Uniform__runtimearr_SSBO Uniform
     %int_50 = OpConstant %int 50
     %int_60 = OpConstant %int 60
       %main = OpFunction %void None %3
          %5 = OpLabel
          %i = OpVariable %_ptr_Function_int Function
         %11 = OpLoad %int %vIndex
               OpStore %i %11
         %20 = OpLoad %int %i
         %22 = OpIAdd %int %20 %int_10
         %23 = OpCopyObject %int %22
         %25 = OpAccessChain %_ptr_UniformConstant_16 %uSamplers %23
         %26 = OpLoad %16 %25
         %31 = OpLoad %int %i
         %33 = OpIAdd %int %31 %int_40
         %34 = OpCopyObject %int %33
         %36 = OpAccessChain %_ptr_UniformConstant_27 %uSamps %34
         %37 = OpLoad %27 %36
         %sampled_image = OpSampledImage %38 %26 %37
         %43 = OpLoad %v2float %vUV
         %44 = OpImageSampleImplicitLod %v4float %sampled_image %43
               OpStore %FragColor %44
         %48 = OpLoad %int %i
         %49 = OpIAdd %int %48 %int_10
         %50 = OpCopyObject %int %49
         %52 = OpAccessChain %_ptr_UniformConstant_38 %uCombinedSamplers %50
         %combined_sampler = OpLoad %38 %52
         %54 = OpLoad %v2float %vUV
         %55 = OpImageSampleImplicitLod %v4float %combined_sampler %54
               OpStore %FragColor %55
         %63 = OpLoad %int %i
         %65 = OpIAdd %int %63 %int_20
         %66 = OpCopyObject %int %65
         %68 = OpLoad %int %i
         %69 = OpIAdd %int %68 %int_40
         %70 = OpCopyObject %int %69
         %ubo_ptr = OpAccessChain %_ptr_Uniform_v4float %ubos %66 %int_0 %70
         %ubo_ptr_copy = OpCopyObject %_ptr_Uniform_v4float %ubo_ptr
         %73 = OpLoad %v4float %ubo_ptr_copy
         %74 = OpLoad %v4float %FragColor
         %75 = OpFAdd %v4float %74 %73
               OpStore %FragColor %75
         %81 = OpLoad %int %i
         %83 = OpIAdd %int %81 %int_50
         %84 = OpCopyObject %int %83
         %85 = OpLoad %int %i
         %87 = OpIAdd %int %85 %int_60
         %88 = OpCopyObject %int %87
         %ssbo_ptr = OpAccessChain %_ptr_Uniform_v4float %ssbos %84 %int_0 %88
         %ssbo_ptr_copy = OpCopyObject %_ptr_Uniform_v4float %ssbo_ptr
         %90 = OpLoad %v4float %ssbo_ptr_copy
         %91 = OpLoad %v4float %FragColor
         %92 = OpFAdd %v4float %91 %90
               OpStore %FragColor %92
               OpReturn
               OpFunctionEnd
