; SPIR-V
; Version: 1.0
; Generator: Khronos Glslang Reference Front End; 7
; Bound: 22
; Schema: 0
               OpCapability Shader
               OpCapability Float64
          %1 = OpExtInstImport "GLSL.std.450"
               OpMemoryModel Logical GLSL450
               OpEntryPoint Fragment %main "main" %FragColor %vTmp
               OpExecutionMode %main OriginUpperLeft
               OpSource GLSL 450
               OpName %main "main"
               OpName %FragColor "FragColor"
               OpName %vTmp "vTmp"
               OpDecorate %FragColor Location 0
               OpDecorate %vTmp Flat
               OpDecorate %vTmp Location 0
       %void = OpTypeVoid
          %3 = OpTypeFunction %void
      %float = OpTypeFloat 32
    %v3float = OpTypeVector %float 3
%_ptr_Output_v3float = OpTypePointer Output %v3float
  %FragColor = OpVariable %_ptr_Output_v3float Output
     %double = OpTypeFloat 64
   %v3double = OpTypeVector %double 3
%double_0x1p_1024 = OpConstant %double 0x1p+1024
%double_n0x1p_1024 = OpConstant %double -0x1p+1024
%double_0x1_8p_1024 = OpConstant %double 0x1.8p+1024
         %15 = OpConstantComposite %v3double %double_0x1p_1024 %double_n0x1p_1024 %double_0x1_8p_1024
%_ptr_Input_double = OpTypePointer Input %double
       %vTmp = OpVariable %_ptr_Input_double Input
       %main = OpFunction %void None %3
          %5 = OpLabel
         %18 = OpLoad %double %vTmp
         %19 = OpCompositeConstruct %v3double %18 %18 %18
         %20 = OpFAdd %v3double %15 %19
         %21 = OpFConvert %v3float %20
               OpStore %FragColor %21
               OpReturn
               OpFunctionEnd
