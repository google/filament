; SPIR-V
; Version: 1.0
; Generator: Khronos Glslang Reference Front End; 8
; Bound: 17
; Schema: 0
               OpCapability Shader
          %1 = OpExtInstImport "GLSL.std.450"
               OpMemoryModel Logical GLSL450
               OpEntryPoint Fragment %main "main" %FragColor
               OpExecutionMode %main OriginUpperLeft
               OpSource GLSL 450
               OpName %main "main"
               OpName %b "b"
               OpName %FragColor "FragColor"
               OpDecorate %FragColor Location 0
       %void = OpTypeVoid
          %3 = OpTypeFunction %void
      %float = OpTypeFloat 32
%_ptr_Private_float = OpTypePointer Private %float
   %float_10 = OpConstant %float 10
   %float_20 = OpConstant %float 20
          %b = OpVariable %_ptr_Private_float Private %float_10
%_ptr_Output_float = OpTypePointer Output %float
  %FragColor = OpVariable %_ptr_Output_float Output
       %main = OpFunction %void None %3
          %5 = OpLabel
               OpStore %b %float_20
         %15 = OpLoad %float %b
         %16 = OpFAdd %float %15 %15
               OpStore %FragColor %16
               OpReturn
               OpFunctionEnd
