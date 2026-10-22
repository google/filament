; SPIR-V
; Version: 1.0
; Generator: Khronos SPIR-V Tools Assembler; 0
; Bound: 14
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
       %void = OpTypeVoid
          %3 = OpTypeFunction %void
      %float = OpTypeFloat 32
    %v4float = OpTypeVector %float 4
    %float_1 = OpConstant %float 1
    %float_0 = OpConstant %float 0
         %10 = OpConstantComposite %v4float %float_1 %float_0 %float_0 %float_1
%_ptr_Output_v4float = OpTypePointer Output %v4float
 %_GLF_color = OpVariable %_ptr_Output_v4float Output
       %main = OpFunction %void None %3
          %5 = OpLabel
         %13 = OpExtInst %v4float %1 Modf %10 %_GLF_color
               OpReturn
               OpFunctionEnd
