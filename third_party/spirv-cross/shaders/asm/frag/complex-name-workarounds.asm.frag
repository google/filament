; SPIR-V
; Version: 1.0
; Generator: Khronos Glslang Reference Front End; 6
; Bound: 47
; Schema: 0
               OpCapability Shader
          %1 = OpExtInstImport "GLSL.std.450"
               OpMemoryModel Logical GLSL450
               OpEntryPoint Fragment %main "main" %a %b %FragColor
               OpExecutionMode %main OriginUpperLeft
               OpSource GLSL 450
               OpName %main "main"
               OpName %func__vf4_ "fu__nc_"
               OpName %a_ "a_"
               OpName %func_2_vf4_ "fu__nc_"
               OpName %a_2 "___"
               OpName %c0 "___"
               OpName %a "__"
               OpName %b "a"
               OpName %param "b"
               OpName %c1 "b"
               OpName %param_0 "b"
               OpName %FragColor "b"
               OpDecorate %a Location 0
               OpDecorate %b Location 1
               OpDecorate %FragColor Location 0
       %void = OpTypeVoid
          %3 = OpTypeFunction %void
      %float = OpTypeFloat 32
    %v4float = OpTypeVector %float 4
%_ptr_Function_v4float = OpTypePointer Function %v4float
          %9 = OpTypeFunction %v4float %_ptr_Function_v4float
%_ptr_Input_v4float = OpTypePointer Input %v4float
          %a = OpVariable %_ptr_Input_v4float Input
          %b = OpVariable %_ptr_Input_v4float Input
%_ptr_Output_v4float = OpTypePointer Output %v4float
  %FragColor = OpVariable %_ptr_Output_v4float Output
       %main = OpFunction %void None %3
          %5 = OpLabel
         %c0 = OpVariable %_ptr_Function_v4float Function
      %param = OpVariable %_ptr_Function_v4float Function
         %c1 = OpVariable %_ptr_Function_v4float Function
    %param_0 = OpVariable %_ptr_Function_v4float Function
         %25 = OpLoad %v4float %a
         %27 = OpLoad %v4float %b
         %28 = OpFAdd %v4float %25 %27
         %30 = OpLoad %v4float %a
               OpStore %param %30
         %31 = OpFunctionCall %v4float %func__vf4_ %param
         %32 = OpFAdd %v4float %28 %31
               OpStore %c0 %32
         %34 = OpLoad %v4float %a
         %35 = OpLoad %v4float %b
         %36 = OpFSub %v4float %34 %35
         %38 = OpLoad %v4float %b
               OpStore %param_0 %38
         %39 = OpFunctionCall %v4float %func_2_vf4_ %param_0
         %40 = OpFAdd %v4float %36 %39
               OpStore %c1 %40
         %43 = OpLoad %v4float %c0
               OpStore %FragColor %43
         %44 = OpLoad %v4float %c1
               OpStore %FragColor %44
         %45 = OpLoad %v4float %c0
               OpStore %FragColor %45
         %46 = OpLoad %v4float %c1
               OpStore %FragColor %46
               OpReturn
               OpFunctionEnd
 %func__vf4_ = OpFunction %v4float None %9
         %a_ = OpFunctionParameter %_ptr_Function_v4float
         %12 = OpLabel
         %16 = OpLoad %v4float %a_
               OpReturnValue %16
               OpFunctionEnd
%func_2_vf4_ = OpFunction %v4float None %9
        %a_2 = OpFunctionParameter %_ptr_Function_v4float
         %15 = OpLabel
         %19 = OpLoad %v4float %a_2
               OpReturnValue %19
               OpFunctionEnd
