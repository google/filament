; SPIR-V
; Version: 1.0
; Generator: Khronos Glslang Reference Front End; 10
; Bound: 22
; Schema: 0
               OpCapability Shader
          %1 = OpExtInstImport "GLSL.std.450"
               OpMemoryModel Logical GLSL450
               OpEntryPoint Fragment %main "main" %FragColor %vUV
               OpExecutionMode %main OriginUpperLeft
               OpSource GLSL 450
               OpName %main "main"
               OpName %FragColor "FragColor"
               OpName %uSamp "uSamp"
               OpName %vUV "vUV"
               OpDecorate %FragColor Location 0
               OpDecorate %uSamp DescriptorSet 0
               OpDecorate %uSamp Binding 0
               OpDecorate %vUV Location 0
       %void = OpTypeVoid
          %3 = OpTypeFunction %void
      %float = OpTypeFloat 32
    %v4float = OpTypeVector %float 4
%_ptr_Output_v4float = OpTypePointer Output %v4float
  %FragColor = OpVariable %_ptr_Output_v4float Output
         %10 = OpTypeImage %float 2D 0 0 0 1 Unknown
         %11 = OpTypeSampledImage %10
%_ptr_UniformConstant_11 = OpTypePointer UniformConstant %11
      %uSamp = OpVariable %_ptr_UniformConstant_11 UniformConstant
    %v2float = OpTypeVector %float 2
%_ptr_Input_v2float = OpTypePointer Input %v2float
        %vUV = OpVariable %_ptr_Input_v2float Input
        %int = OpTypeInt 32 0
      %int_1 = OpConstant %int 1
       %main = OpFunction %void None %3
          %5 = OpLabel
         %14 = OpLoad %11 %uSamp
         %18 = OpLoad %v2float %vUV
         %21 = OpImageGather %v4float %14 %18 %int_1
               OpStore %FragColor %21
               OpReturn
               OpFunctionEnd
