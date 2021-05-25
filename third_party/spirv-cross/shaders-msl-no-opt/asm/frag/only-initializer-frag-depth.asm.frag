; SPIR-V
; Version: 1.0
; Generator: Khronos Glslang Reference Front End; 10
; Bound: 10
; Schema: 0
               OpCapability Shader
          %1 = OpExtInstImport "GLSL.std.450"
               OpMemoryModel Logical GLSL450
               OpEntryPoint Fragment %main "main" %gl_FragDepth
               OpExecutionMode %main OriginUpperLeft
               OpExecutionMode %main DepthReplacing
               OpSource GLSL 450
               OpName %main "main"
               OpName %gl_FragDepth "gl_FragDepth"
               OpDecorate %gl_FragDepth BuiltIn FragDepth
       %void = OpTypeVoid
          %3 = OpTypeFunction %void
      %float = OpTypeFloat 32
%_ptr_Output_float = OpTypePointer Output %float
  %float_0_5 = OpConstant %float 0.5
%gl_FragDepth = OpVariable %_ptr_Output_float Output %float_0_5
       %main = OpFunction %void None %3
          %5 = OpLabel
               OpReturn
               OpFunctionEnd
