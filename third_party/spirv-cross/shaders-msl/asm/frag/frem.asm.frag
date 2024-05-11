; SPIR-V
; Version: 1.0
; Generator: Khronos Glslang Reference Front End; 3
; Bound: 16
; Schema: 0
               OpCapability Shader
          %1 = OpExtInstImport "GLSL.std.450"
               OpMemoryModel Logical GLSL450
               OpEntryPoint Fragment %main "main" %FragColor %vA %vB
               OpExecutionMode %main OriginUpperLeft
               OpSource ESSL 310
               OpName %main "main"
               OpName %FragColor "FragColor"
               OpName %vA "vA"
               OpName %vB "vB"
               OpDecorate %FragColor RelaxedPrecision
               OpDecorate %FragColor Location 0
               OpDecorate %vA RelaxedPrecision
               OpDecorate %vA Location 0
               OpDecorate %12 RelaxedPrecision
               OpDecorate %vB RelaxedPrecision
               OpDecorate %vB Location 1
               OpDecorate %14 RelaxedPrecision
               OpDecorate %15 RelaxedPrecision
       %void = OpTypeVoid
          %3 = OpTypeFunction %void
      %float = OpTypeFloat 32
    %v4float = OpTypeVector %float 4
%_ptr_Output_v4float = OpTypePointer Output %v4float
  %FragColor = OpVariable %_ptr_Output_v4float Output
%_ptr_Input_v4float = OpTypePointer Input %v4float
         %vA = OpVariable %_ptr_Input_v4float Input
         %vB = OpVariable %_ptr_Input_v4float Input
       %main = OpFunction %void None %3
          %5 = OpLabel
         %12 = OpLoad %v4float %vA
         %14 = OpLoad %v4float %vB
         %15 = OpFRem %v4float %12 %14
               OpStore %FragColor %15
               OpReturn
               OpFunctionEnd
