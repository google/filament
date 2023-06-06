; SPIR-V
; Version: 1.0
; Generator: Khronos Glslang Reference Front End; 10
; Bound: 65
; Schema: 0
               OpCapability Shader
          %1 = OpExtInstImport "GLSL.std.450"
               OpMemoryModel Logical GLSL450
               OpEntryPoint Fragment %main "main" %FragColor
               OpExecutionMode %main OriginUpperLeft
               OpSource GLSL 450
               OpName %main "main"
               OpName %SSBO "SSBO"
               OpMemberName %SSBO 0 "values0"
               OpName %_ ""
               OpName %SSBO1 "SSBO1"
               OpMemberName %SSBO1 0 "values1"
               OpName %__0 ""
               OpName %FragColor "FragColor"
               OpDecorate %_runtimearr_float ArrayStride 4
               OpMemberDecorate %SSBO 0 NonWritable
               OpMemberDecorate %SSBO 0 Offset 0
               OpDecorate %SSBO BufferBlock
               OpDecorate %_ DescriptorSet 0
               OpDecorate %_ Binding 0
               OpDecorate %_runtimearr_float_0 ArrayStride 4
               OpMemberDecorate %SSBO1 0 NonWritable
               OpMemberDecorate %SSBO1 0 Offset 0
               OpDecorate %SSBO1 BufferBlock
               OpDecorate %__0 DescriptorSet 0
               OpDecorate %__0 Binding 1
               OpDecorate %FragColor Location 0
       %void = OpTypeVoid
          %3 = OpTypeFunction %void
      %float = OpTypeFloat 32
    %v2float = OpTypeVector %float 2
    %float_0 = OpConstant %float 0
         %11 = OpConstantComposite %v2float %float_0 %float_0
        %int = OpTypeInt 32 1
      %int_0 = OpConstant %int 0
     %int_16 = OpConstant %int 16
       %bool = OpTypeBool
%_runtimearr_float = OpTypeRuntimeArray %float
       %SSBO = OpTypeStruct %_runtimearr_float
%_ptr_Uniform_SSBO = OpTypePointer Uniform %SSBO
          %_ = OpVariable %_ptr_Uniform_SSBO Uniform
%_ptr_Uniform_float = OpTypePointer Uniform %float
%_runtimearr_float_0 = OpTypeRuntimeArray %float
      %SSBO1 = OpTypeStruct %_runtimearr_float_0
%_ptr_Uniform_SSBO1 = OpTypePointer Uniform %SSBO1
        %__0 = OpVariable %_ptr_Uniform_SSBO1 Uniform
      %int_1 = OpConstant %int 1
%_ptr_Output_v2float = OpTypePointer Output %v2float
  %FragColor = OpVariable %_ptr_Output_v2float Output
       %main = OpFunction %void None %3
          %5 = OpLabel
               OpBranch %17
         %17 = OpLabel
         %61 = OpPhi %v2float %11 %5 %d %cont
         %60 = OpPhi %int %int_0 %5 %49 %cont
         %25 = OpSLessThan %bool %60 %int_16
               OpLoopMerge %19 %cont None
               OpBranchConditional %25 %pre18 %19
	   %pre18 = OpLabel
			   OpBranch %18
         %18 = OpLabel
         %32 = OpAccessChain %_ptr_Uniform_float %_ %int_0 %60
         %43 = OpAccessChain %_ptr_Uniform_float %__0 %int_0 %60
         %33 = OpLoad %float %32
         %44 = OpLoad %float %43
		 %a = OpFMul %v2float %61 %61
         %b = OpCompositeInsert %v2float %33 %a 0
         %c = OpCompositeInsert %v2float %44 %b 1
		 OpBranch %cont
		 %cont = OpLabel
		 %d = OpFAdd %v2float %61 %c
         %49 = OpIAdd %int %60 %int_1
               OpBranch %17
         %19 = OpLabel
               OpStore %FragColor %61
               OpReturn
               OpFunctionEnd
