; SPIR-V
; Version: 1.0
; Generator: Khronos Glslang Reference Front End; 2
; Bound: 113
; Schema: 0
               OpCapability Shader
          %1 = OpExtInstImport "GLSL.std.450"
               OpMemoryModel Logical GLSL450
               OpEntryPoint Fragment %main "main" %xIn_1 %_entryPointOutput
               OpExecutionMode %main OriginUpperLeft
               OpSource HLSL 500
               OpName %main "main"
               OpName %sample_fetch_t21_vi3_ "sample_fetch(t21;vi3;"
               OpName %tex "tex"
               OpName %UV "UV"
               OpName %sample_sampler_t21_vf2_ "sample_sampler(t21;vf2;"
               OpName %tex_0 "tex"
               OpName %UV_0 "UV"
               OpName %_main_vf4_ "@main(vf4;"
               OpName %xIn "xIn"
               OpName %Sampler "Sampler"
               OpName %coord "coord"
               OpName %value "value"
               OpName %SampledImage "SampledImage"
               OpName %param "param"
               OpName %param_0 "param"
               OpName %param_1 "param"
               OpName %param_2 "param"
               OpName %xIn_0 "xIn"
               OpName %xIn_1 "xIn"
               OpName %_entryPointOutput "@entryPointOutput"
               OpName %param_3 "param"
               OpDecorate %Sampler DescriptorSet 0
               OpDecorate %Sampler Binding 0
               OpDecorate %SampledImage DescriptorSet 0
               OpDecorate %SampledImage Binding 0
               OpDecorate %xIn_1 BuiltIn FragCoord
               OpDecorate %_entryPointOutput Location 0
       %void = OpTypeVoid
          %3 = OpTypeFunction %void
      %float = OpTypeFloat 32
          %7 = OpTypeImage %float 2D 0 0 0 1 Unknown
%_ptr_Function_7 = OpTypePointer Function %7
        %int = OpTypeInt 32 1
      %v3int = OpTypeVector %int 3
%_ptr_Function_v3int = OpTypePointer Function %v3int
    %v4float = OpTypeVector %float 4
         %13 = OpTypeFunction %v4float %_ptr_Function_7 %_ptr_Function_v3int
    %v2float = OpTypeVector %float 2
%_ptr_Function_v2float = OpTypePointer Function %v2float
         %20 = OpTypeFunction %v4float %_ptr_Function_7 %_ptr_Function_v2float
%_ptr_Function_v4float = OpTypePointer Function %v4float
         %26 = OpTypeFunction %v4float %_ptr_Function_v4float
      %v2int = OpTypeVector %int 2
       %uint = OpTypeInt 32 0
     %uint_2 = OpConstant %uint 2
%_ptr_Function_int = OpTypePointer Function %int
         %43 = OpTypeSampler
%_ptr_UniformConstant_43 = OpTypePointer UniformConstant %43
    %Sampler = OpVariable %_ptr_UniformConstant_43 UniformConstant
         %47 = OpTypeSampledImage %7
     %uint_0 = OpConstant %uint 0
%_ptr_Function_float = OpTypePointer Function %float
 %float_1280 = OpConstant %float 1280
     %uint_1 = OpConstant %uint 1
  %float_720 = OpConstant %float 720
      %int_0 = OpConstant %int 0
%_ptr_UniformConstant_7 = OpTypePointer UniformConstant %7
%SampledImage = OpVariable %_ptr_UniformConstant_7 UniformConstant
%_ptr_Input_v4float = OpTypePointer Input %v4float
      %xIn_1 = OpVariable %_ptr_Input_v4float Input
%_ptr_Output_v4float = OpTypePointer Output %v4float
%_entryPointOutput = OpVariable %_ptr_Output_v4float Output
       %main = OpFunction %void None %3
          %5 = OpLabel
      %xIn_0 = OpVariable %_ptr_Function_v4float Function
    %param_3 = OpVariable %_ptr_Function_v4float Function
        %107 = OpLoad %v4float %xIn_1
               OpStore %xIn_0 %107
        %111 = OpLoad %v4float %xIn_0
               OpStore %param_3 %111
        %112 = OpFunctionCall %v4float %_main_vf4_ %param_3
               OpStore %_entryPointOutput %112
               OpReturn
               OpFunctionEnd
%sample_fetch_t21_vi3_ = OpFunction %v4float None %13
        %tex = OpFunctionParameter %_ptr_Function_7
         %UV = OpFunctionParameter %_ptr_Function_v3int
         %17 = OpLabel
         %30 = OpLoad %7 %tex
         %32 = OpLoad %v3int %UV
         %33 = OpVectorShuffle %v2int %32 %32 0 1
         %37 = OpAccessChain %_ptr_Function_int %UV %uint_2
         %38 = OpLoad %int %37
         %39 = OpImageFetch %v4float %30 %33 Lod %38
               OpReturnValue %39
               OpFunctionEnd
%sample_sampler_t21_vf2_ = OpFunction %v4float None %20
      %tex_0 = OpFunctionParameter %_ptr_Function_7
       %UV_0 = OpFunctionParameter %_ptr_Function_v2float
         %24 = OpLabel
         %42 = OpLoad %7 %tex_0
         %46 = OpLoad %43 %Sampler
         %48 = OpSampledImage %47 %42 %46
         %49 = OpLoad %v2float %UV_0
         %50 = OpImageSampleImplicitLod %v4float %48 %49
               OpReturnValue %50
               OpFunctionEnd
 %_main_vf4_ = OpFunction %v4float None %26
        %xIn = OpFunctionParameter %_ptr_Function_v4float
         %29 = OpLabel
      %coord = OpVariable %_ptr_Function_v3int Function
      %value = OpVariable %_ptr_Function_v4float Function
      %param = OpVariable %_ptr_Function_7 Function
    %param_0 = OpVariable %_ptr_Function_v3int Function
    %param_1 = OpVariable %_ptr_Function_7 Function
    %param_2 = OpVariable %_ptr_Function_v2float Function
         %56 = OpAccessChain %_ptr_Function_float %xIn %uint_0
         %57 = OpLoad %float %56
         %59 = OpFMul %float %57 %float_1280
         %60 = OpConvertFToS %int %59
         %62 = OpAccessChain %_ptr_Function_float %xIn %uint_1
         %63 = OpLoad %float %62
         %65 = OpFMul %float %63 %float_720
         %66 = OpConvertFToS %int %65
         %68 = OpCompositeConstruct %v3int %60 %66 %int_0
               OpStore %coord %68
         %73 = OpLoad %7 %SampledImage
               OpStore %param %73
         %75 = OpLoad %v3int %coord
               OpStore %param_0 %75
         %76 = OpFunctionCall %v4float %sample_fetch_t21_vi3_ %param %param_0
               OpStore %value %76
         %77 = OpLoad %7 %SampledImage
         %78 = OpLoad %v3int %coord
         %79 = OpVectorShuffle %v2int %78 %78 0 1
         %80 = OpAccessChain %_ptr_Function_int %coord %uint_2
         %81 = OpLoad %int %80
         %82 = OpImageFetch %v4float %77 %79 Lod %81
         %83 = OpLoad %v4float %value
         %84 = OpFAdd %v4float %83 %82
               OpStore %value %84
         %86 = OpLoad %7 %SampledImage
               OpStore %param_1 %86
         %88 = OpLoad %v4float %xIn
         %89 = OpVectorShuffle %v2float %88 %88 0 1
               OpStore %param_2 %89
         %90 = OpFunctionCall %v4float %sample_sampler_t21_vf2_ %param_1 %param_2
         %91 = OpLoad %v4float %value
         %92 = OpFAdd %v4float %91 %90
               OpStore %value %92
         %93 = OpLoad %7 %SampledImage
         %94 = OpLoad %43 %Sampler
         %95 = OpSampledImage %47 %93 %94
         %96 = OpLoad %v4float %xIn
         %97 = OpVectorShuffle %v2float %96 %96 0 1
         %98 = OpImageSampleImplicitLod %v4float %95 %97
         %99 = OpLoad %v4float %value
        %100 = OpFAdd %v4float %99 %98
               OpStore %value %100
        %101 = OpLoad %v4float %value
               OpReturnValue %101
               OpFunctionEnd
