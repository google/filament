; SPIR-V
; Version: 1.0
; Generator: Khronos Glslang Reference Front End; 6
; Bound: 26
; Schema: 0
               OpCapability Shader
          %1 = OpExtInstImport "GLSL.std.450"
               OpMemoryModel Logical GLSL450
               OpEntryPoint Fragment %main "main" %FragColor %gl_FragCoord
               OpExecutionMode %main OriginUpperLeft
               OpSource GLSL 450
               OpName %main "main"
               OpName %FragColor "FragColor"
               OpName %uTexture "uTexture"
               OpName %gl_FragCoord "gl_FragCoord"
               OpDecorate %FragColor Location 0
               OpDecorate %uTexture DescriptorSet 0
               OpDecorate %uTexture Binding 0
               OpDecorate %gl_FragCoord BuiltIn FragCoord
       %void = OpTypeVoid
          %3 = OpTypeFunction %void
      %float = OpTypeFloat 32
    %v4float = OpTypeVector %float 4
%_ptr_Output_v4float = OpTypePointer Output %v4float
  %FragColor = OpVariable %_ptr_Output_v4float Output
         %10 = OpTypeImage %float 2D 0 0 0 1 Unknown
         %11 = OpTypeSampledImage %10
%_ptr_UniformConstant_11 = OpTypePointer UniformConstant %11
   %uTexture = OpVariable %_ptr_UniformConstant_11 UniformConstant
%_ptr_Input_v4float = OpTypePointer Input %v4float
%gl_FragCoord = OpVariable %_ptr_Input_v4float Input
    %v2float = OpTypeVector %float 2
        %int = OpTypeInt 32 1
      %v2int = OpTypeVector %int 2
      %int_0 = OpConstant %int 0
       %main = OpFunction %void None %3
          %5 = OpLabel
         %14 = OpLoad %11 %uTexture
         %18 = OpLoad %v4float %gl_FragCoord
         %19 = OpVectorShuffle %v2float %18 %18 0 1
         %22 = OpConvertFToS %v2int %19
         %24 = OpImage %10 %14
         %25 = OpImageFetch %v4float %24 %22
               OpStore %FragColor %25
               OpReturn
               OpFunctionEnd
