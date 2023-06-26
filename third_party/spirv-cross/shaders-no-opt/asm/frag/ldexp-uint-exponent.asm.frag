; SPIR-V
; Version: 1.0
; Generator: Khronos Glslang Reference Front End; 7
; Bound: 20
; Schema: 0
               OpCapability Shader
          %1 = OpExtInstImport "GLSL.std.450"
               OpMemoryModel Logical GLSL450
               OpEntryPoint Fragment %main "main" %_GLF_color
               OpExecutionMode %main OriginUpperLeft
               OpSource ESSL 310
               OpName %main "main"
               OpName %_GLF_color "_GLF_color"
               OpDecorate %_GLF_color Location 0
               OpDecorate %18 RelaxedPrecision
       %void = OpTypeVoid
          %3 = OpTypeFunction %void
      %float = OpTypeFloat 32
    %v4float = OpTypeVector %float 4
%_ptr_Output_v4float = OpTypePointer Output %v4float
 %_GLF_color = OpVariable %_ptr_Output_v4float Output
    %float_1 = OpConstant %float 1
         %11 = OpConstantComposite %v4float %float_1 %float_1 %float_1 %float_1
       %uint = OpTypeInt 32 0
     %v4uint = OpTypeVector %uint 4
     %uint_1 = OpConstant %uint 1
         %15 = OpConstantComposite %v4uint %uint_1 %uint_1 %uint_1 %uint_1
        %int = OpTypeInt 32 1
      %v4int = OpTypeVector %int 4
       %main = OpFunction %void None %3
          %5 = OpLabel
         %18 = OpBitCount %v4uint %15
         %19 = OpExtInst %v4float %1 Ldexp %11 %18
               OpStore %_GLF_color %19
               OpReturn
               OpFunctionEnd
