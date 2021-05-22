; SPIR-V
; Version: 1.0
; Generator: Khronos Glslang Reference Front End; 10
; Bound: 59
; Schema: 0
               OpCapability Shader
               OpCapability ShaderNonUniform
               OpCapability RuntimeDescriptorArray
               OpCapability StorageBufferArrayNonUniformIndexing
               OpExtension "SPV_EXT_descriptor_indexing"
          %1 = OpExtInstImport "GLSL.std.450"
               OpMemoryModel Logical GLSL450
               OpEntryPoint Fragment %main "main" %vIndex %FragColor
               OpExecutionMode %main OriginUpperLeft
               OpSource GLSL 450
               OpSourceExtension "GL_EXT_nonuniform_qualifier"
               OpSourceExtension "GL_EXT_samplerless_texture_functions"
               OpName %main "main"
               OpName %i "i"
               OpName %vIndex "vIndex"
               OpName %SSBO "SSBO"
               OpMemberName %SSBO 0 "counter"
               OpMemberName %SSBO 1 "v"
               OpName %ssbos "ssbos"
               OpName %FragColor "FragColor"
               OpDecorate %vIndex Flat
               OpDecorate %vIndex Location 0
               OpDecorate %_runtimearr_v4float ArrayStride 16
               OpMemberDecorate %SSBO 0 Offset 0
               OpMemberDecorate %SSBO 1 Offset 16
               OpDecorate %SSBO BufferBlock
               OpDecorate %ssbos DescriptorSet 0
               OpDecorate %ssbos Binding 3
               OpDecorate %32 NonUniform
               OpDecorate %39 NonUniform
               OpDecorate %49 NonUniform
               OpDecorate %FragColor Location 0
       %void = OpTypeVoid
          %3 = OpTypeFunction %void
        %int = OpTypeInt 32 1
%_ptr_Function_int = OpTypePointer Function %int
%_ptr_Input_int = OpTypePointer Input %int
     %vIndex = OpVariable %_ptr_Input_int Input
       %uint = OpTypeInt 32 0
      %float = OpTypeFloat 32
    %v4float = OpTypeVector %float 4
%_runtimearr_v4float = OpTypeRuntimeArray %v4float
       %SSBO = OpTypeStruct %uint %_runtimearr_v4float
%_runtimearr_SSBO = OpTypeRuntimeArray %SSBO
%_ptr_Uniform__runtimearr_SSBO = OpTypePointer Uniform %_runtimearr_SSBO
      %ssbos = OpVariable %_ptr_Uniform__runtimearr_SSBO Uniform
     %int_60 = OpConstant %int 60
      %int_1 = OpConstant %int 1
     %int_70 = OpConstant %int 70
   %float_20 = OpConstant %float 20
         %30 = OpConstantComposite %v4float %float_20 %float_20 %float_20 %float_20
%_ptr_Uniform_v4float = OpTypePointer Uniform %v4float
    %int_100 = OpConstant %int 100
      %int_0 = OpConstant %int 0
%_ptr_Uniform_uint = OpTypePointer Uniform %uint
   %uint_100 = OpConstant %uint 100
     %uint_1 = OpConstant %uint 1
     %uint_0 = OpConstant %uint 0
%_ptr_Output_v4float = OpTypePointer Output %v4float
  %FragColor = OpVariable %_ptr_Output_v4float Output
%_ptr_Uniform_SSBO = OpTypePointer Uniform %SSBO
     %uint_2 = OpConstant %uint 2
%_ptr_Output_float = OpTypePointer Output %float
       %main = OpFunction %void None %3
          %5 = OpLabel
          %i = OpVariable %_ptr_Function_int Function
         %11 = OpLoad %int %vIndex
               OpStore %i %11
         %20 = OpLoad %int %i
         %22 = OpIAdd %int %20 %int_60
         %23 = OpCopyObject %int %22
         %25 = OpLoad %int %i
         %27 = OpIAdd %int %25 %int_70
         %28 = OpCopyObject %int %27
         %32 = OpAccessChain %_ptr_Uniform_v4float %ssbos %23 %int_1 %28
               OpStore %32 %30
         %33 = OpLoad %int %i
         %35 = OpIAdd %int %33 %int_100
         %36 = OpCopyObject %int %35
         %39 = OpAccessChain %_ptr_Uniform_uint %ssbos %36 %int_0
         %43 = OpAtomicIAdd %uint %39 %uint_1 %uint_0 %uint_100
         %46 = OpLoad %int %i
         %47 = OpCopyObject %int %46
         %49 = OpAccessChain %_ptr_Uniform_SSBO %ssbos %47
         %50 = OpArrayLength %uint %49 1
         %51 = OpBitcast %int %50
         %52 = OpConvertSToF %float %51
         %55 = OpAccessChain %_ptr_Output_float %FragColor %uint_2
         %56 = OpLoad %float %55
         %57 = OpFAdd %float %56 %52
         %58 = OpAccessChain %_ptr_Output_float %FragColor %uint_2
               OpStore %58 %57
               OpReturn
               OpFunctionEnd
