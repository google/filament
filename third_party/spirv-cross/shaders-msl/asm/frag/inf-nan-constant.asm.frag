; SPIR-V
; Version: 1.0
; Generator: Khronos Glslang Reference Front End; 7
; Bound: 14
; Schema: 0
               OpCapability Shader
          %1 = OpExtInstImport "GLSL.std.450"
               OpMemoryModel Logical GLSL450
               OpEntryPoint Fragment %main "main" %FragColor
               OpExecutionMode %main OriginUpperLeft
               OpSource ESSL 310
               OpName %main "main"
               OpName %FragColor "FragColor"
               OpDecorate %FragColor Location 0
       %void = OpTypeVoid
          %3 = OpTypeFunction %void
      %float = OpTypeFloat 32
    %v3float = OpTypeVector %float 3
%_ptr_Output_v3float = OpTypePointer Output %v3float
  %FragColor = OpVariable %_ptr_Output_v3float Output
%float_0x1p_128 = OpConstant %float 0x1p+128
%float_n0x1p_128 = OpConstant %float -0x1p+128
%float_0x1_8p_128 = OpConstant %float 0x1.8p+128
         %13 = OpConstantComposite %v3float %float_0x1p_128 %float_n0x1p_128 %float_0x1_8p_128
       %main = OpFunction %void None %3
          %5 = OpLabel
               OpStore %FragColor %13
               OpReturn
               OpFunctionEnd
