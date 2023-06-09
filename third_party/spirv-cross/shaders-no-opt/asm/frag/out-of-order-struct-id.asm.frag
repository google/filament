; SPIR-V
; Version: 1.0
; Generator: Khronos Glslang Reference Front End; 7
; Bound: 24
; Schema: 0
               OpCapability Shader
          %1 = OpExtInstImport "GLSL.std.450"
               OpMemoryModel Logical GLSL450
               OpEntryPoint Fragment %main "main" %FragColor
               OpExecutionMode %main OriginUpperLeft
               OpSource GLSL 450
               OpName %main "main"
               OpName %FragColor "FragColor"
               OpName %80 "Foo"
               OpMemberName %80 0 "a"
               OpName %79 "Bar"
               OpMemberName %79 0 "foo"
               OpMemberName %79 1 "foo2"
               OpName %UBO "UBO"
               OpMemberName %UBO 0 "bar"
               OpName %_ ""
               OpDecorate %FragColor Location 0
               OpMemberDecorate %80 0 Offset 0
               OpMemberDecorate %79 0 Offset 0
               OpMemberDecorate %79 1 Offset 16
               OpMemberDecorate %UBO 0 Offset 0
               OpDecorate %UBO Block
               OpDecorate %_ DescriptorSet 0
               OpDecorate %_ Binding 0
       %void = OpTypeVoid
          %3 = OpTypeFunction %void
      %float = OpTypeFloat 32
    %v4float = OpTypeVector %float 4
%_ptr_Output_v4float = OpTypePointer Output %v4float
  %FragColor = OpVariable %_ptr_Output_v4float Output
        %80 = OpTypeStruct %v4float
        %79 = OpTypeStruct %80 %80
        %UBO = OpTypeStruct %79
%_ptr_Uniform_UBO = OpTypePointer Uniform %UBO
          %_ = OpVariable %_ptr_Uniform_UBO Uniform
        %int = OpTypeInt 32 1
      %int_0 = OpConstant %int 0
%_ptr_Uniform_v4float = OpTypePointer Uniform %v4float
      %int_1 = OpConstant %int 1
       %main = OpFunction %void None %3
          %5 = OpLabel
         %18 = OpAccessChain %_ptr_Uniform_v4float %_ %int_0 %int_0 %int_0
         %19 = OpLoad %v4float %18
         %21 = OpAccessChain %_ptr_Uniform_v4float %_ %int_0 %int_1 %int_0
         %22 = OpLoad %v4float %21
         %23 = OpFAdd %v4float %19 %22
               OpStore %FragColor %23
               OpReturn
               OpFunctionEnd
