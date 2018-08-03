; SPIR-V
; Version: 1.0
; Generator: Khronos Glslang Reference Front End; 1
; Bound: 70
; Schema: 0
               OpCapability Shader
          %1 = OpExtInstImport "GLSL.std.450"
               OpMemoryModel Logical GLSL450
               OpEntryPoint Fragment %main "main" %texCoords_1 %cascadeIndex_1 %fragDepth_1 %_entryPointOutput
               OpExecutionMode %main OriginUpperLeft
               OpSource HLSL 500
               OpName %main "main"
               OpName %_main_vf2_f1_f1_ "@main(vf2;f1;f1;"
               OpName %texCoords "texCoords"
               OpName %cascadeIndex "cascadeIndex"
               OpName %fragDepth "fragDepth"
               OpName %c "c"
               OpName %ShadowMap "ShadowMap"
               OpName %ShadowSamplerPCF "ShadowSamplerPCF"
               OpName %texCoords_0 "texCoords"
               OpName %texCoords_1 "texCoords"
               OpName %cascadeIndex_0 "cascadeIndex"
               OpName %cascadeIndex_1 "cascadeIndex"
               OpName %fragDepth_0 "fragDepth"
               OpName %fragDepth_1 "fragDepth"
               OpName %_entryPointOutput "@entryPointOutput"
               OpName %param "param"
               OpName %param_0 "param"
               OpName %param_1 "param"
               OpDecorate %ShadowMap DescriptorSet 0
               OpDecorate %ShadowSamplerPCF DescriptorSet 0
               OpDecorate %ShadowMap Binding 0
               OpDecorate %ShadowSamplerPCF Binding 1
               OpDecorate %texCoords_1 Location 0
               OpDecorate %cascadeIndex_1 Location 1
               OpDecorate %fragDepth_1 Location 2
               OpDecorate %_entryPointOutput Location 0
       %void = OpTypeVoid
          %3 = OpTypeFunction %void
      %float = OpTypeFloat 32
    %v2float = OpTypeVector %float 2
%_ptr_Function_v2float = OpTypePointer Function %v2float
%_ptr_Function_float = OpTypePointer Function %float
    %v4float = OpTypeVector %float 4
         %11 = OpTypeFunction %v4float %_ptr_Function_v2float %_ptr_Function_float %_ptr_Function_float
         %18 = OpTypeImage %float 2D 0 1 0 1 Unknown
%_ptr_UniformConstant_18 = OpTypePointer UniformConstant %18
  %ShadowMap = OpVariable %_ptr_UniformConstant_18 UniformConstant
         %22 = OpTypeSampler
%_ptr_UniformConstant_22 = OpTypePointer UniformConstant %22
%ShadowSamplerPCF = OpVariable %_ptr_UniformConstant_22 UniformConstant
         %26 = OpTypeImage %float 2D 1 1 0 1 Unknown
         %27 = OpTypeSampledImage %26
    %v3float = OpTypeVector %float 3
    %float_0 = OpConstant %float 0
%_ptr_Input_v2float = OpTypePointer Input %v2float
%texCoords_1 = OpVariable %_ptr_Input_v2float Input
%_ptr_Input_float = OpTypePointer Input %float
%cascadeIndex_1 = OpVariable %_ptr_Input_float Input
%fragDepth_1 = OpVariable %_ptr_Input_float Input
%_ptr_Output_v4float = OpTypePointer Output %v4float
%_entryPointOutput = OpVariable %_ptr_Output_v4float Output
       %main = OpFunction %void None %3
          %5 = OpLabel
%texCoords_0 = OpVariable %_ptr_Function_v2float Function
%cascadeIndex_0 = OpVariable %_ptr_Function_float Function
%fragDepth_0 = OpVariable %_ptr_Function_float Function
      %param = OpVariable %_ptr_Function_v2float Function
    %param_0 = OpVariable %_ptr_Function_float Function
    %param_1 = OpVariable %_ptr_Function_float Function
         %53 = OpLoad %v2float %texCoords_1
               OpStore %texCoords_0 %53
         %57 = OpLoad %float %cascadeIndex_1
               OpStore %cascadeIndex_0 %57
         %60 = OpLoad %float %fragDepth_1
               OpStore %fragDepth_0 %60
         %64 = OpLoad %v2float %texCoords_0
               OpStore %param %64
         %66 = OpLoad %float %cascadeIndex_0
               OpStore %param_0 %66
         %68 = OpLoad %float %fragDepth_0
               OpStore %param_1 %68
         %69 = OpFunctionCall %v4float %_main_vf2_f1_f1_ %param %param_0 %param_1
               OpStore %_entryPointOutput %69
               OpReturn
               OpFunctionEnd
%_main_vf2_f1_f1_ = OpFunction %v4float None %11
  %texCoords = OpFunctionParameter %_ptr_Function_v2float
%cascadeIndex = OpFunctionParameter %_ptr_Function_float
  %fragDepth = OpFunctionParameter %_ptr_Function_float
         %16 = OpLabel
          %c = OpVariable %_ptr_Function_float Function
         %21 = OpLoad %18 %ShadowMap
         %25 = OpLoad %22 %ShadowSamplerPCF
         %28 = OpSampledImage %27 %21 %25
         %29 = OpLoad %v2float %texCoords
         %30 = OpLoad %float %cascadeIndex
         %32 = OpCompositeExtract %float %29 0
         %33 = OpCompositeExtract %float %29 1
         %34 = OpCompositeConstruct %v3float %32 %33 %30
         %35 = OpLoad %float %fragDepth
         %36 = OpCompositeExtract %float %34 0
         %37 = OpCompositeExtract %float %34 1
         %38 = OpCompositeExtract %float %34 2
         %39 = OpCompositeConstruct %v4float %36 %37 %38 %35
         %41 = OpCompositeExtract %float %39 3
         %42 = OpImageSampleDrefExplicitLod %float %28 %39 %41 Lod %float_0
               OpStore %c %42
         %43 = OpLoad %float %c
         %44 = OpLoad %float %c
         %45 = OpLoad %float %c
         %46 = OpLoad %float %c
         %47 = OpCompositeConstruct %v4float %43 %44 %45 %46
               OpReturnValue %47
               OpFunctionEnd
