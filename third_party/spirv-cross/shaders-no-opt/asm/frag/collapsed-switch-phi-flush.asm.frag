; SPIR-V
; Version: 1.0
; Generator: Khronos Glslang Reference Front End; 10
; Bound: 20
; Schema: 0
               OpCapability Shader
          %1 = OpExtInstImport "GLSL.std.450"
               OpMemoryModel Logical GLSL450
               OpEntryPoint Fragment %main "main" %FragColor %vIndex
               OpExecutionMode %main OriginUpperLeft
               OpSource GLSL 450
               OpName %main "main"
               OpName %FragColor "FragColor"
               OpName %vIndex "vIndex"
               OpDecorate %FragColor Location 0
               OpDecorate %vIndex Flat
               OpDecorate %vIndex Location 0
       %void = OpTypeVoid
          %3 = OpTypeFunction %void
        %int = OpTypeInt 32 1
      %int_0 = OpConstant %int 0
      %float = OpTypeFloat 32
    %v4float = OpTypeVector %float 4
%_ptr_Output_v4float = OpTypePointer Output %v4float
  %FragColor = OpVariable %_ptr_Output_v4float Output
    %float_1 = OpConstant %float 1
         %15 = OpConstantComposite %v4float %float_1 %float_1 %float_1 %float_1
%_ptr_Input_int = OpTypePointer Input %int
     %vIndex = OpVariable %_ptr_Input_int Input
       %main = OpFunction %void None %3
          %5 = OpLabel
               OpSelectionMerge %9 None
               OpSwitch %int_0 %9
          %9 = OpLabel
		  %tmp = OpPhi %v4float %15 %5
               OpStore %FragColor %tmp
               OpReturn
               OpFunctionEnd
