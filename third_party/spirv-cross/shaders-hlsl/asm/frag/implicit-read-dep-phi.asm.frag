; SPIR-V
; Version: 1.0
; Generator: Khronos Glslang Reference Front End; 3
; Bound: 60
; Schema: 0
               OpCapability Shader
          %1 = OpExtInstImport "GLSL.std.450"
               OpMemoryModel Logical GLSL450
               OpEntryPoint Fragment %main "main" %v0 %FragColor
               OpExecutionMode %main OriginUpperLeft
               OpSource GLSL 450
               OpName %main "main"
               OpName %phi "phi"
               OpName %i "i"
               OpName %v0 "v0"
               OpName %FragColor "FragColor"
               OpName %uImage "uImage"
               OpDecorate %v0 Location 0
               OpDecorate %FragColor Location 0
               OpDecorate %uImage DescriptorSet 0
               OpDecorate %uImage Binding 0
       %void = OpTypeVoid
          %3 = OpTypeFunction %void
      %float = OpTypeFloat 32
%_ptr_Function_float = OpTypePointer Function %float
    %float_1 = OpConstant %float 1
        %int = OpTypeInt 32 1
%_ptr_Function_int = OpTypePointer Function %int
      %int_0 = OpConstant %int 0
      %int_4 = OpConstant %int 4
       %bool = OpTypeBool
    %v4float = OpTypeVector %float 4
%_ptr_Input_v4float = OpTypePointer Input %v4float
         %v0 = OpVariable %_ptr_Input_v4float Input
%_ptr_Input_float = OpTypePointer Input %float
    %float_0 = OpConstant %float 0
%_ptr_Output_v4float = OpTypePointer Output %v4float
  %FragColor = OpVariable %_ptr_Output_v4float Output
         %36 = OpTypeImage %float 2D 0 0 0 1 Unknown
         %37 = OpTypeSampledImage %36
%_ptr_UniformConstant_37 = OpTypePointer UniformConstant %37
     %uImage = OpVariable %_ptr_UniformConstant_37 UniformConstant
    %v2float = OpTypeVector %float 2
       %uint = OpTypeInt 32 0
     %uint_0 = OpConstant %uint 0
    %float_2 = OpConstant %float 2
      %int_1 = OpConstant %int 1
	  %float_1_vec = OpConstantComposite %v4float %float_1 %float_2 %float_1 %float_2
       %main = OpFunction %void None %3
          %5 = OpLabel
          %i = OpVariable %_ptr_Function_int Function
               OpStore %i %int_0
               OpBranch %loop_header
         %loop_header = OpLabel
        %phi = OpPhi %float %float_1 %5 %phi_plus_2 %continue_block
		%tex_phi = OpPhi %v4float %float_1_vec %5 %texture_load_result %continue_block
               OpLoopMerge %merge_block %continue_block None
               OpBranch %loop_body
         %loop_body = OpLabel
               OpStore %FragColor %tex_phi
         %19 = OpLoad %int %i
         %22 = OpSLessThan %bool %19 %int_4
               OpBranchConditional %22 %15 %merge_block
         %15 = OpLabel
         %26 = OpLoad %int %i
         %28 = OpAccessChain %_ptr_Input_float %v0 %26
         %29 = OpLoad %float %28
         %31 = OpFOrdGreaterThan %bool %29 %float_0
               OpBranchConditional %31 %continue_block %merge_block
         %continue_block = OpLabel
         %40 = OpLoad %37 %uImage
         %43 = OpCompositeConstruct %v2float %phi %phi
         %texture_load_result = OpImageSampleExplicitLod %v4float %40 %43 Lod %float_0
         %phi_plus_2 = OpFAdd %float %phi %float_2
         %54 = OpLoad %int %i
         %56 = OpIAdd %int %54 %int_1
               OpStore %i %56
               OpBranch %loop_header
         %merge_block = OpLabel
               OpReturn
               OpFunctionEnd
