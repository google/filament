; SPIR-V
; Version: 1.0
; Generator: Khronos Glslang Reference Front End; 2
; Bound: 63
; Schema: 0
               OpCapability Shader
               OpCapability SampledBuffer
               OpCapability ImageBuffer
          %1 = OpExtInstImport "GLSL.std.450"
               OpMemoryModel Logical GLSL450
               OpEntryPoint Fragment %main "main" %_entryPointOutput
               OpExecutionMode %main OriginUpperLeft
               OpSource HLSL 500
               OpName %main "main"
               OpName %_main_ "@main("
               OpName %storeTemp "storeTemp"
               OpName %RWIm "RWIm"
               OpName %v "v"
               OpName %RWBuf "RWBuf"
               OpName %ROIm "ROIm"
               OpName %ROBuf "ROBuf"
               OpName %_entryPointOutput "@entryPointOutput"
               OpDecorate %RWIm DescriptorSet 0
               OpDecorate %RWIm Binding 1
               OpDecorate %RWBuf DescriptorSet 0
               OpDecorate %RWBuf Binding 0
               OpDecorate %ROIm DescriptorSet 0
               OpDecorate %ROIm Binding 1
               OpDecorate %ROBuf DescriptorSet 0
               OpDecorate %ROBuf Binding 0
               OpDecorate %_entryPointOutput Location 0
       %void = OpTypeVoid
          %3 = OpTypeFunction %void
      %float = OpTypeFloat 32
    %v4float = OpTypeVector %float 4
          %8 = OpTypeFunction %v4float
%_ptr_Function_v4float = OpTypePointer Function %v4float
   %float_10 = OpConstant %float 10
  %float_0_5 = OpConstant %float 0.5
    %float_8 = OpConstant %float 8
    %float_2 = OpConstant %float 2
         %17 = OpConstantComposite %v4float %float_10 %float_0_5 %float_8 %float_2
         %18 = OpTypeImage %float 2D 0 0 0 2 Rgba32f
%_ptr_UniformConstant_18 = OpTypePointer UniformConstant %18
       %RWIm = OpVariable %_ptr_UniformConstant_18 UniformConstant
       %uint = OpTypeInt 32 0
     %v2uint = OpTypeVector %uint 2
    %uint_10 = OpConstant %uint 10
         %25 = OpConstantComposite %v2uint %uint_10 %uint_10
    %uint_30 = OpConstant %uint 30
         %30 = OpConstantComposite %v2uint %uint_30 %uint_30
         %32 = OpTypeImage %float Buffer 0 0 0 2 Rgba32f
%_ptr_UniformConstant_32 = OpTypePointer UniformConstant %32
      %RWBuf = OpVariable %_ptr_UniformConstant_32 UniformConstant
    %uint_80 = OpConstant %uint 80
         %38 = OpTypeImage %float 2D 0 0 0 1 Unknown
		 %SampledImage = OpTypeSampledImage %38
%_ptr_UniformConstant_38 = OpTypePointer UniformConstant %SampledImage
       %ROIm = OpVariable %_ptr_UniformConstant_38 UniformConstant
    %uint_50 = OpConstant %uint 50
    %uint_60 = OpConstant %uint 60
         %44 = OpConstantComposite %v2uint %uint_50 %uint_60
        %int = OpTypeInt 32 1
      %int_0 = OpConstant %int 0
         %50 = OpTypeImage %float Buffer 0 0 0 1 Rgba32f
%_ptr_UniformConstant_50 = OpTypePointer UniformConstant %50
      %ROBuf = OpVariable %_ptr_UniformConstant_50 UniformConstant
%_ptr_Output_v4float = OpTypePointer Output %v4float
%_entryPointOutput = OpVariable %_ptr_Output_v4float Output
       %main = OpFunction %void None %3
          %5 = OpLabel
         %62 = OpFunctionCall %v4float %_main_
               OpStore %_entryPointOutput %62
               OpReturn
               OpFunctionEnd
     %_main_ = OpFunction %v4float None %8
         %10 = OpLabel
  %storeTemp = OpVariable %_ptr_Function_v4float Function
          %v = OpVariable %_ptr_Function_v4float Function
               OpStore %storeTemp %17
         %21 = OpLoad %18 %RWIm
         %26 = OpLoad %v4float %storeTemp
               OpImageWrite %21 %25 %26
         %28 = OpLoad %18 %RWIm
         %31 = OpImageRead %v4float %28 %30
               OpStore %v %31
         %35 = OpLoad %32 %RWBuf
         %37 = OpLoad %v4float %v
               OpImageWrite %35 %uint_80 %37
         %41 = OpLoad %SampledImage %ROIm
		 %ROImage = OpImage %38 %41
         %47 = OpImageFetch %v4float %ROImage %44 Lod %int_0
         %48 = OpLoad %v4float %v
         %49 = OpFAdd %v4float %48 %47
               OpStore %v %49
         %53 = OpLoad %50 %ROBuf
         %54 = OpImageFetch %v4float %53 %uint_80
         %55 = OpLoad %v4float %v
         %56 = OpFAdd %v4float %55 %54
               OpStore %v %56
         %57 = OpLoad %v4float %v
               OpReturnValue %57
               OpFunctionEnd
