; SPIR-V
; Version: 1.0
; Generator: Khronos Glslang Reference Front End; 7
; Bound: 55
; Schema: 0
               OpCapability Shader
          %1 = OpExtInstImport "GLSL.std.450"
               OpMemoryModel Logical GLSL450
               OpEntryPoint Fragment %main "main" %FragColor
               OpExecutionMode %main OriginUpperLeft
               OpSource GLSL 450
               OpName %main "main"
               OpName %UBO "UBO"
               OpMemberName %UBO 0 "uCount"
               OpMemberName %UBO 1 "uJ"
               OpMemberName %UBO 2 "uK"
               OpName %_ ""
               OpName %FragColor "FragColor"
               OpMemberDecorate %UBO 0 Offset 0
               OpMemberDecorate %UBO 1 Offset 4
               OpMemberDecorate %UBO 2 Offset 8
               OpDecorate %UBO Block
               OpDecorate %_ DescriptorSet 0
               OpDecorate %_ Binding 0
               OpDecorate %FragColor Location 0
       %void = OpTypeVoid
          %3 = OpTypeFunction %void
        %int = OpTypeInt 32 1
        %UBO = OpTypeStruct %int %int %int
%_ptr_Uniform_UBO = OpTypePointer Uniform %UBO
          %_ = OpVariable %_ptr_Uniform_UBO Uniform
      %int_1 = OpConstant %int 1
%_ptr_Uniform_int = OpTypePointer Uniform %int
      %int_2 = OpConstant %int 2
      %int_0 = OpConstant %int 0
       %bool = OpTypeBool
      %float = OpTypeFloat 32
%_ptr_Output_float = OpTypePointer Output %float
  %FragColor = OpVariable %_ptr_Output_float Output
       %main = OpFunction %void None %3
          %5 = OpLabel
         %14 = OpAccessChain %_ptr_Uniform_int %_ %int_1
         %15 = OpLoad %int %14
         %18 = OpAccessChain %_ptr_Uniform_int %_ %int_2
         %19 = OpLoad %int %18
               OpBranch %22
         %22 = OpLabel
         %54 = OpPhi %int %19 %5 %53 %23
         %53 = OpPhi %int %15 %5 %54 %23
         %52 = OpPhi %int %int_0 %5 %37 %23
         %28 = OpAccessChain %_ptr_Uniform_int %_ %int_0
         %29 = OpLoad %int %28
         %31 = OpSLessThan %bool %52 %29
               OpLoopMerge %24 %23 None
               OpBranchConditional %31 %inbetween %24
         %inbetween = OpLabel
               OpBranch %23
         %23 = OpLabel
         %37 = OpIAdd %int %52 %int_1
               OpBranch %22
         %24 = OpLabel
         %43 = OpISub %int %53 %54
         %44 = OpConvertSToF %float %43
         %49 = OpIMul %int %15 %19
         %50 = OpConvertSToF %float %49
         %51 = OpFMul %float %44 %50
               OpStore %FragColor %51
               OpReturn
               OpFunctionEnd
