               OpCapability Shader
          %1 = OpExtInstImport "GLSL.std.450"
               OpMemoryModel Logical GLSL450
               OpEntryPoint Vertex %main "main" %a
               OpSource ESSL 310
               OpName %main "main"
               OpName %a "a"
               OpDecorate %a Location 0
       %void = OpTypeVoid
          %3 = OpTypeFunction %void
      %float = OpTypeFloat 32
%_ptr_Output_float = OpTypePointer Output %float
          %a = OpVariable %_ptr_Output_float Output
    %float_5 = OpConstant %float 5
    %float_2 = OpConstant %float 2
    %float_1 = OpConstant %float 1
       %main = OpFunction %void None %3
          %5 = OpLabel
               OpStore %a %float_5
         %10 = OpLoad %float %a
         %14 = OpExtInst %float %1 Fma %10 %float_2 %float_1
               OpStore %a %14
               OpReturn
               OpFunctionEnd
