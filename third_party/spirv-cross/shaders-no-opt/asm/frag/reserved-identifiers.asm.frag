; SPIR-V
; Version: 1.0
; Generator: Khronos Glslang Reference Front End; 10
; Bound: 24
; Schema: 0
               OpCapability Shader
          %1 = OpExtInstImport "GLSL.std.450"
               OpMemoryModel Logical GLSL450
               OpEntryPoint Fragment %main "main" %spvFoo %SPIRV_Cross_blah %_40 %_m40 %_underscore_foo_bar_meep
               OpExecutionMode %main OriginUpperLeft
               OpSource GLSL 450
               OpName %main "main"
               OpName %spvFoo "spvFoo"
               OpName %SPIRV_Cross_blah "SPIRV_Cross_blah"
               OpName %_40 "_40Bar"
               OpName %_m40 "_m40"
               OpName %_underscore_foo_bar_meep "__underscore_foo__bar_meep__"
               OpDecorate %spvFoo Location 0
               OpDecorate %SPIRV_Cross_blah Location 1
               OpDecorate %_40 Location 2
               OpDecorate %_m40 Location 3
               OpDecorate %_underscore_foo_bar_meep Location 4
       %void = OpTypeVoid
          %3 = OpTypeFunction %void
      %float = OpTypeFloat 32
    %v4float = OpTypeVector %float 4
%_ptr_Output_v4float = OpTypePointer Output %v4float
     %spvFoo = OpVariable %_ptr_Output_v4float Output
    %float_0 = OpConstant %float 0
         %11 = OpConstantComposite %v4float %float_0 %float_0 %float_0 %float_0
%SPIRV_Cross_blah = OpVariable %_ptr_Output_v4float Output
    %float_1 = OpConstant %float 1
         %14 = OpConstantComposite %v4float %float_1 %float_1 %float_1 %float_1
        %_40 = OpVariable %_ptr_Output_v4float Output
    %float_2 = OpConstant %float 2
         %17 = OpConstantComposite %v4float %float_2 %float_2 %float_2 %float_2
       %_m40 = OpVariable %_ptr_Output_v4float Output
    %float_3 = OpConstant %float 3
         %20 = OpConstantComposite %v4float %float_3 %float_3 %float_3 %float_3
%_underscore_foo_bar_meep = OpVariable %_ptr_Output_v4float Output
    %float_4 = OpConstant %float 4
         %23 = OpConstantComposite %v4float %float_4 %float_4 %float_4 %float_4
       %main = OpFunction %void None %3
          %5 = OpLabel
               OpStore %spvFoo %11
               OpStore %SPIRV_Cross_blah %14
               OpStore %_40 %17
               OpStore %_m40 %20
               OpStore %_underscore_foo_bar_meep %23
               OpReturn
               OpFunctionEnd
