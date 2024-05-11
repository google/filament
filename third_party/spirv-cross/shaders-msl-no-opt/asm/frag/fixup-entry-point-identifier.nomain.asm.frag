; SPIR-V
; Version: 1.0
; Generator: Khronos Glslang Reference Front End; 10
; Bound: 12
; Schema: 0
               OpCapability Shader
          %1 = OpExtInstImport "GLSL.std.450"
               OpMemoryModel Logical GLSL450
               OpEntryPoint Fragment %foo "_5ma@@in" %FragColor
               OpExecutionMode %foo OriginUpperLeft
               OpSource GLSL 450
               OpName %foo "FOO"
               OpName %FragColor "FragColor"
               OpDecorate %FragColor Location 0
       %void = OpTypeVoid
          %3 = OpTypeFunction %void
      %float = OpTypeFloat 32
    %v4float = OpTypeVector %float 4
%_ptr_Output_v4float = OpTypePointer Output %v4float
  %FragColor = OpVariable %_ptr_Output_v4float Output
    %float_1 = OpConstant %float 1
         %11 = OpConstantComposite %v4float %float_1 %float_1 %float_1 %float_1
       %foo = OpFunction %void None %3
          %5 = OpLabel
               OpStore %FragColor %11
               OpReturn
               OpFunctionEnd
