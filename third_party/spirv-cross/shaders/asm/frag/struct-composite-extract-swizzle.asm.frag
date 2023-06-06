; SPIR-V
; Version: 1.0
; Generator: Khronos Glslang Reference Front End; 1
; Bound: 34
; Schema: 0
               OpCapability Shader
          %1 = OpExtInstImport "GLSL.std.450"
               OpMemoryModel Logical GLSL450
               OpEntryPoint Fragment %main "main" %FragColor
               OpExecutionMode %main OriginUpperLeft
               OpSource ESSL 310
               OpName %main "main"
               OpName %FragColor "FragColor"
               OpName %uSampler "uSampler"
               OpName %Foo "Foo"
               OpMemberName %Foo 0 "var1"
               OpMemberName %Foo 1 "var2"
               OpName %foo "foo"
               OpDecorate %FragColor RelaxedPrecision
               OpDecorate %FragColor Location 0
               OpDecorate %uSampler RelaxedPrecision
               OpDecorate %uSampler DescriptorSet 0
               OpDecorate %uSampler Binding 0
               OpDecorate %14 RelaxedPrecision
               OpMemberDecorate %Foo 0 RelaxedPrecision
               OpMemberDecorate %Foo 1 RelaxedPrecision
               OpDecorate %27 RelaxedPrecision
               OpDecorate %28 RelaxedPrecision
       %void = OpTypeVoid
          %3 = OpTypeFunction %void
      %float = OpTypeFloat 32
    %v4float = OpTypeVector %float 4
%_ptr_Output_v4float = OpTypePointer Output %v4float
  %FragColor = OpVariable %_ptr_Output_v4float Output
         %10 = OpTypeImage %float 2D 0 0 0 1 Unknown
         %11 = OpTypeSampledImage %10
%_ptr_UniformConstant_11 = OpTypePointer UniformConstant %11
   %uSampler = OpVariable %_ptr_UniformConstant_11 UniformConstant
        %Foo = OpTypeStruct %float %float
%_ptr_Function_Foo = OpTypePointer Function %Foo
        %int = OpTypeInt 32 1
%_ptr_Function_float = OpTypePointer Function %float
    %v2float = OpTypeVector %float 2
         %33 = OpUndef %Foo
       %main = OpFunction %void None %3
          %5 = OpLabel
        %foo = OpVariable %_ptr_Function_Foo Function
         %14 = OpLoad %11 %uSampler
         %30 = OpCompositeExtract %float %33 0
         %32 = OpCompositeExtract %float %33 1
         %27 = OpCompositeConstruct %v2float %30 %32
         %28 = OpImageSampleImplicitLod %v4float %14 %27
               OpStore %FragColor %28
               OpReturn
               OpFunctionEnd
