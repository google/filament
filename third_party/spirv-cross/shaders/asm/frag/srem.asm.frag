; SPIR-V
; Version: 1.0
; Generator: Khronos Glslang Reference Front End; 2
; Bound: 23
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
               OpDecorate %vA Flat
               OpDecorate %vA Location 0
               OpDecorate %vB Flat
               OpDecorate %vB Location 1
       %void = OpTypeVoid
          %3 = OpTypeFunction %void
      %float = OpTypeFloat 32
    %v4float = OpTypeVector %float 4
%_ptr_Output_v4float = OpTypePointer Output %v4float
  %FragColor = OpVariable %_ptr_Output_v4float Output
        %int = OpTypeInt 32 1
      %v4int = OpTypeVector %int 4
%_ptr_Input_v4int = OpTypePointer Input %v4int
         %vA = OpVariable %_ptr_Input_v4int Input
         %vB = OpVariable %_ptr_Input_v4int Input
       %main = OpFunction %void None %3
          %5 = OpLabel
         %14 = OpLoad %v4int %vA
         %16 = OpLoad %v4int %vB
         %17 = OpLoad %v4int %vA
         %18 = OpLoad %v4int %vB
         %19 = OpSRem %v4int %17 %18
         %20 = OpConvertSToF %v4float %19
               OpStore %FragColor %20
               OpReturn
               OpFunctionEnd
