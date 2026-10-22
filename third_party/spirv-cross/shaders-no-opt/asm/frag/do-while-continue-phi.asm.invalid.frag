; SPIR-V
; Version: 1.0
; Generator: Khronos Glslang Reference Front End; 7
; Bound: 42
; Schema: 0
               OpCapability Shader
          %1 = OpExtInstImport "GLSL.std.450"
               OpMemoryModel Logical GLSL450
               OpEntryPoint Fragment %main "main" %gl_FragCoord %_GLF_color
               OpExecutionMode %main OriginUpperLeft
               OpSource ESSL 310
               OpName %main "main"
               OpName %gl_FragCoord "gl_FragCoord"
               OpName %_GLF_color "_GLF_color"
               OpDecorate %gl_FragCoord BuiltIn FragCoord
               OpDecorate %_GLF_color Location 0
       %void = OpTypeVoid
          %3 = OpTypeFunction %void
      %float = OpTypeFloat 32
    %v4float = OpTypeVector %float 4
%_ptr_Input_v4float = OpTypePointer Input %v4float
%gl_FragCoord = OpVariable %_ptr_Input_v4float Input
       %uint = OpTypeInt 32 0
     %uint_0 = OpConstant %uint 0
%_ptr_Input_float = OpTypePointer Input %float
       %bool = OpTypeBool
      %false = OpConstantFalse %bool
%_ptr_Output_v4float = OpTypePointer Output %v4float
 %_GLF_color = OpVariable %_ptr_Output_v4float Output
    %float_1 = OpConstant %float 1
    %float_0 = OpConstant %float 0
         %31 = OpConstantComposite %v4float %float_1 %float_0 %float_0 %float_1
       %true = OpConstantTrue %bool
       %main = OpFunction %void None %3
          %5 = OpLabel
               OpBranch %33
         %33 = OpLabel
               OpLoopMerge %32 %35 None
               OpBranch %6
          %6 = OpLabel
               OpLoopMerge %8 %24 None
               OpBranch %7
          %7 = OpLabel
         %17 = OpAccessChain %_ptr_Input_float %gl_FragCoord %uint_0
         %18 = OpLoad %float %17
         %22 = OpFOrdNotEqual %bool %18 %18
               OpSelectionMerge %24 None
               OpBranchConditional %22 %23 %24
         %23 = OpLabel
               OpBranch %8
         %24 = OpLabel
               OpBranchConditional %false %6 %8
          %8 = OpLabel
         %41 = OpPhi %bool %true %23 %false %24
               OpSelectionMerge %39 None
               OpBranchConditional %41 %32 %39
         %39 = OpLabel
               OpStore %_GLF_color %31
               OpBranch %32
         %35 = OpLabel
               OpBranch %33
         %32 = OpLabel
               OpReturn
               OpFunctionEnd
