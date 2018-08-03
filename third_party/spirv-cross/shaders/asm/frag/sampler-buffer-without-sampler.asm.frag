; SPIR-V
; Version: 1.0
; Generator: Khronos Glslang Reference Front End; 1
; Bound: 36
; Schema: 0
               OpCapability Shader
               OpCapability SampledBuffer
               OpCapability ImageBuffer
          %1 = OpExtInstImport "GLSL.std.450"
               OpMemoryModel Logical GLSL450
               OpEntryPoint Fragment %main "main" %_entryPointOutput
               OpExecutionMode %main OriginUpperLeft
               OpName %main "main"
               OpName %_main_ "@main("
               OpName %storeTemp "storeTemp"
               OpName %RWTex "RWTex"
               OpName %Tex "Tex"
               OpName %_entryPointOutput "@entryPointOutput"
               OpDecorate %RWTex DescriptorSet 0
               OpDecorate %Tex DescriptorSet 0
               OpDecorate %RWTex Binding 0
               OpDecorate %Tex Binding 1
               OpDecorate %_entryPointOutput Location 0
       %void = OpTypeVoid
          %3 = OpTypeFunction %void
      %float = OpTypeFloat 32
    %v4float = OpTypeVector %float 4
          %8 = OpTypeFunction %v4float
%_ptr_Function_v4float = OpTypePointer Function %v4float
         %13 = OpConstant %float 1
         %14 = OpConstant %float 2
         %15 = OpConstant %float 3
         %16 = OpConstant %float 4
         %17 = OpConstantComposite %v4float %13 %14 %15 %16
         %18 = OpTypeImage %float Buffer 0 0 0 2 Rgba32f
%_ptr_UniformConstant_18 = OpTypePointer UniformConstant %18
      %RWTex = OpVariable %_ptr_UniformConstant_18 UniformConstant
        %int = OpTypeInt 32 1
         %23 = OpConstant %int 20
         %25 = OpTypeImage %float Buffer 0 0 0 1 Rgba32f
%_ptr_UniformConstant_25 = OpTypePointer UniformConstant %25
        %Tex = OpVariable %_ptr_UniformConstant_25 UniformConstant
         %29 = OpConstant %int 10
%_ptr_Output_v4float = OpTypePointer Output %v4float
%_entryPointOutput = OpVariable %_ptr_Output_v4float Output
       %main = OpFunction %void None %3
          %5 = OpLabel
         %35 = OpFunctionCall %v4float %_main_
               OpStore %_entryPointOutput %35
               OpReturn
               OpFunctionEnd
     %_main_ = OpFunction %v4float None %8
         %10 = OpLabel
  %storeTemp = OpVariable %_ptr_Function_v4float Function
               OpStore %storeTemp %17
         %21 = OpLoad %18 %RWTex
         %24 = OpLoad %v4float %storeTemp
               OpImageWrite %21 %23 %24
         %28 = OpLoad %25 %Tex
         %30 = OpImageFetch %v4float %28 %29
               OpReturnValue %30
               OpFunctionEnd
