; SPIR-V
; Version: 1.0
; Generator: Khronos Glslang Reference Front End; 7
; Bound: 54
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
        %int = OpTypeInt 32 1
      %int_0 = OpConstant %int 0
      %float = OpTypeFloat 32
    %v4float = OpTypeVector %float 4
%_ptr_Input_v4float = OpTypePointer Input %v4float
%gl_FragCoord = OpVariable %_ptr_Input_v4float Input
       %uint = OpTypeInt 32 0
     %uint_0 = OpConstant %uint 0
%_ptr_Input_float = OpTypePointer Input %float
       %bool = OpTypeBool
    %v2float = OpTypeVector %float 2
%_ptr_Output_v4float = OpTypePointer Output %v4float
 %_GLF_color = OpVariable %_ptr_Output_v4float Output
    %float_1 = OpConstant %float 1
         %52 = OpUndef %v2float
       %main = OpFunction %void None %3
          %5 = OpLabel
               OpSelectionMerge %9 None
               OpSwitch %int_0 %8
          %8 = OpLabel
         %17 = OpAccessChain %_ptr_Input_float %gl_FragCoord %uint_0
         %18 = OpLoad %float %17
         %22 = OpFOrdNotEqual %bool %18 %18
               OpSelectionMerge %24 None
               OpBranchConditional %22 %23 %24
         %23 = OpLabel
               OpBranch %9
         %24 = OpLabel
         %33 = OpCompositeExtract %float %52 1
         %51 = OpCompositeInsert %v2float %33 %52 1
               OpBranch %9
          %9 = OpLabel
         %53 = OpPhi %v2float %52 %23 %51 %24
         %42 = OpCompositeExtract %float %53 0
         %43 = OpCompositeExtract %float %53 1
         %48 = OpCompositeConstruct %v4float %42 %43 %float_1 %float_1
               OpStore %_GLF_color %48
               OpReturn
               OpFunctionEnd
