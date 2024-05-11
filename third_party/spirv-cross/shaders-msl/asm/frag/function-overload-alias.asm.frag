; SPIR-V
; Version: 1.0
; Generator: Khronos Glslang Reference Front End; 3
; Bound: 76
; Schema: 0
               OpCapability Shader
          %1 = OpExtInstImport "GLSL.std.450"
               OpMemoryModel Logical GLSL450
               OpEntryPoint Fragment %main "main" %FragColor
               OpExecutionMode %main OriginUpperLeft
               OpSource ESSL 310
               OpName %main "main"
               OpName %foobar_vf4_ "foo"
               OpName %a "foo"
               OpName %foobar_vf3_ "foo"
               OpName %a_0 "foo"
               OpName %foobaz_vf4_ "foo"
               OpName %a_1 "foo"
               OpName %foobaz_vf2_ "foo"
               OpName %a_2 "foo"
               OpName %a_3 "foo"
               OpName %param "foo"
               OpName %b "foo"
               OpName %param_0 "foo"
               OpName %c "foo"
               OpName %param_1 "foo"
               OpName %d "foo"
               OpName %param_2 "foo"
               OpName %FragColor "FragColor"
               OpDecorate %foobar_vf4_ RelaxedPrecision
               OpDecorate %a RelaxedPrecision
               OpDecorate %foobar_vf3_ RelaxedPrecision
               OpDecorate %a_0 RelaxedPrecision
               OpDecorate %foobaz_vf4_ RelaxedPrecision
               OpDecorate %a_1 RelaxedPrecision
               OpDecorate %foobaz_vf2_ RelaxedPrecision
               OpDecorate %a_2 RelaxedPrecision
               OpDecorate %28 RelaxedPrecision
               OpDecorate %30 RelaxedPrecision
               OpDecorate %31 RelaxedPrecision
               OpDecorate %34 RelaxedPrecision
               OpDecorate %35 RelaxedPrecision
               OpDecorate %36 RelaxedPrecision
               OpDecorate %37 RelaxedPrecision
               OpDecorate %40 RelaxedPrecision
               OpDecorate %42 RelaxedPrecision
               OpDecorate %43 RelaxedPrecision
               OpDecorate %46 RelaxedPrecision
               OpDecorate %47 RelaxedPrecision
               OpDecorate %48 RelaxedPrecision
               OpDecorate %49 RelaxedPrecision
               OpDecorate %a_3 RelaxedPrecision
               OpDecorate %55 RelaxedPrecision
               OpDecorate %b RelaxedPrecision
               OpDecorate %59 RelaxedPrecision
               OpDecorate %c RelaxedPrecision
               OpDecorate %62 RelaxedPrecision
               OpDecorate %d RelaxedPrecision
               OpDecorate %66 RelaxedPrecision
               OpDecorate %FragColor RelaxedPrecision
               OpDecorate %FragColor Location 0
               OpDecorate %69 RelaxedPrecision
               OpDecorate %70 RelaxedPrecision
               OpDecorate %71 RelaxedPrecision
               OpDecorate %72 RelaxedPrecision
               OpDecorate %73 RelaxedPrecision
               OpDecorate %74 RelaxedPrecision
               OpDecorate %75 RelaxedPrecision
       %void = OpTypeVoid
          %3 = OpTypeFunction %void
      %float = OpTypeFloat 32
    %v4float = OpTypeVector %float 4
%_ptr_Function_v4float = OpTypePointer Function %v4float
          %9 = OpTypeFunction %v4float %_ptr_Function_v4float
    %v3float = OpTypeVector %float 3
%_ptr_Function_v3float = OpTypePointer Function %v3float
         %15 = OpTypeFunction %v4float %_ptr_Function_v3float
    %v2float = OpTypeVector %float 2
%_ptr_Function_v2float = OpTypePointer Function %v2float
         %24 = OpTypeFunction %v4float %_ptr_Function_v2float
    %float_1 = OpConstant %float 1
    %float_2 = OpConstant %float 2
         %53 = OpConstantComposite %v4float %float_1 %float_1 %float_1 %float_1
         %57 = OpConstantComposite %v3float %float_1 %float_1 %float_1
         %64 = OpConstantComposite %v2float %float_1 %float_1
%_ptr_Output_v4float = OpTypePointer Output %v4float
  %FragColor = OpVariable %_ptr_Output_v4float Output
       %main = OpFunction %void None %3
          %5 = OpLabel
        %a_3 = OpVariable %_ptr_Function_v4float Function
      %param = OpVariable %_ptr_Function_v4float Function
          %b = OpVariable %_ptr_Function_v4float Function
    %param_0 = OpVariable %_ptr_Function_v3float Function
          %c = OpVariable %_ptr_Function_v4float Function
    %param_1 = OpVariable %_ptr_Function_v4float Function
          %d = OpVariable %_ptr_Function_v4float Function
    %param_2 = OpVariable %_ptr_Function_v2float Function
               OpStore %param %53
         %55 = OpFunctionCall %v4float %foobar_vf4_ %param
               OpStore %a_3 %55
               OpStore %param_0 %57
         %59 = OpFunctionCall %v4float %foobar_vf3_ %param_0
               OpStore %b %59
               OpStore %param_1 %53
         %62 = OpFunctionCall %v4float %foobaz_vf4_ %param_1
               OpStore %c %62
               OpStore %param_2 %64
         %66 = OpFunctionCall %v4float %foobaz_vf2_ %param_2
               OpStore %d %66
         %69 = OpLoad %v4float %a_3
         %70 = OpLoad %v4float %b
         %71 = OpFAdd %v4float %69 %70
         %72 = OpLoad %v4float %c
         %73 = OpFAdd %v4float %71 %72
         %74 = OpLoad %v4float %d
         %75 = OpFAdd %v4float %73 %74
               OpStore %FragColor %75
               OpReturn
               OpFunctionEnd
%foobar_vf4_ = OpFunction %v4float None %9
          %a = OpFunctionParameter %_ptr_Function_v4float
         %12 = OpLabel
         %28 = OpLoad %v4float %a
         %30 = OpCompositeConstruct %v4float %float_1 %float_1 %float_1 %float_1
         %31 = OpFAdd %v4float %28 %30
               OpReturnValue %31
               OpFunctionEnd
%foobar_vf3_ = OpFunction %v4float None %15
        %a_0 = OpFunctionParameter %_ptr_Function_v3float
         %18 = OpLabel
         %34 = OpLoad %v3float %a_0
         %35 = OpVectorShuffle %v4float %34 %34 0 1 2 2
         %36 = OpCompositeConstruct %v4float %float_1 %float_1 %float_1 %float_1
         %37 = OpFAdd %v4float %35 %36
               OpReturnValue %37
               OpFunctionEnd
%foobaz_vf4_ = OpFunction %v4float None %9
        %a_1 = OpFunctionParameter %_ptr_Function_v4float
         %21 = OpLabel
         %40 = OpLoad %v4float %a_1
         %42 = OpCompositeConstruct %v4float %float_2 %float_2 %float_2 %float_2
         %43 = OpFAdd %v4float %40 %42
               OpReturnValue %43
               OpFunctionEnd
%foobaz_vf2_ = OpFunction %v4float None %24
        %a_2 = OpFunctionParameter %_ptr_Function_v2float
         %27 = OpLabel
         %46 = OpLoad %v2float %a_2
         %47 = OpVectorShuffle %v4float %46 %46 0 1 0 1
         %48 = OpCompositeConstruct %v4float %float_2 %float_2 %float_2 %float_2
         %49 = OpFAdd %v4float %47 %48
               OpReturnValue %49
               OpFunctionEnd
