; SPIR-V
; Version: 1.0
; Generator: Khronos Glslang Reference Front End; 10
; Bound: 15
; Schema: 0
               OpCapability Shader
          %1 = OpExtInstImport "GLSL.std.450"
               OpMemoryModel Logical GLSL450
               OpEntryPoint Fragment %main "main" %FragColor0 %FragColor1 %FragColor2 %FragColor3 %V4
               OpExecutionMode %main OriginUpperLeft
               OpSource ESSL 320
               OpName %main "main"
               OpName %FragColor0 "FragColor0"
               OpName %FragColor1 "FragColor1"
               OpName %FragColor2 "FragColor2"
               OpName %FragColor3 "FragColor3"
               OpName %V4 "V4"
			   OpName %V4_value0 "V4_value0"
			   OpName %V1_value0 "V1_value0"
			   OpName %V1_value1 "V1_value1"
			   OpName %V1_value2 "V1_value2"
			   OpName %float_0_weird "float_0_weird"
			   OpName %ubo "ubo"
			   OpName %ubo_mp0 "ubo_mp0"
			   OpName %ubo_hp0 "ubo_hp0"
			   OpName %block "UBO"
			   OpName %phi_mp "phi_mp"
			   OpName %phi_hp "phi_hp"
			   OpMemberName %block 0 "mediump_float"
			   OpMemberName %block 1 "highp_float"
               OpDecorate %FragColor0 RelaxedPrecision
               OpDecorate %FragColor0 Location 0
               OpDecorate %FragColor1 RelaxedPrecision
               OpDecorate %FragColor1 Location 1
               OpDecorate %FragColor2 RelaxedPrecision
               OpDecorate %FragColor2 Location 2
               OpDecorate %FragColor3 RelaxedPrecision
               OpDecorate %FragColor3 Location 3
               OpDecorate %V4 RelaxedPrecision
               OpDecorate %V4 Location 0
			   OpDecorate %V4_add RelaxedPrecision
			   OpDecorate %V4_mul RelaxedPrecision
			   OpDecorate %V1_add RelaxedPrecision
			   OpDecorate %V1_mul RelaxedPrecision
			   OpDecorate %phi_mp RelaxedPrecision
			   OpDecorate %mp_to_mp RelaxedPrecision
			   OpDecorate %hp_to_mp RelaxedPrecision
			   OpDecorate %V1_add_composite RelaxedPrecision
			   OpDecorate %V1_mul_composite RelaxedPrecision
			   OpDecorate %V4_sin1 RelaxedPrecision
			   OpDecorate %float_0_weird RelaxedPrecision
			   OpDecorate %ubo Binding 0
			   OpDecorate %ubo DescriptorSet 0
			   OpDecorate %block Block
			   OpMemberDecorate %block 0 Offset 0
			   OpMemberDecorate %block 1 Offset 4
			   OpMemberDecorate %block 0 RelaxedPrecision
       %void = OpTypeVoid
          %3 = OpTypeFunction %void
      %float = OpTypeFloat 32
	  %block = OpTypeStruct %float %float
	  %block_ptr = OpTypePointer Uniform %block
	  %ubo_float_ptr = OpTypePointer Uniform %float
	  %ubo = OpVariable %block_ptr Uniform
      %uint = OpTypeInt 32 0
	  %uint_0 = OpConstant %uint 0
	  %uint_1 = OpConstant %uint 1
	  %uint_2 = OpConstant %uint 2
	  %uint_3 = OpConstant %uint 3
	  %float_3 = OpConstant %float 3.0
    %v4float = OpTypeVector %float 4
	  %float_3_splat = OpConstantComposite %v4float %float_3 %float_3 %float_3 %float_3
%_ptr_Output_v4float = OpTypePointer Output %v4float
  %FragColor0 = OpVariable %_ptr_Output_v4float Output
  %FragColor1 = OpVariable %_ptr_Output_v4float Output
  %FragColor2 = OpVariable %_ptr_Output_v4float Output
  %FragColor3 = OpVariable %_ptr_Output_v4float Output
%_ptr_Input_v4float = OpTypePointer Input %v4float
%_ptr_Input_float = OpTypePointer Input %float
         %V4 = OpVariable %_ptr_Input_v4float Input
       %main = OpFunction %void None %3
          %5 = OpLabel

		; Inherits precision in GLSL
         %V4_value0 = OpLoad %v4float %V4

		; Inherits precision in GLSL
		 %ptr_V4x = OpAccessChain %_ptr_Input_float %V4 %uint_0

		; Inherits precision in GLSL
         %V1_value0 = OpLoad %float %ptr_V4x
         %V1_value1 = OpCompositeExtract %float %V4_value0 2
         %V1_value2 = OpCopyObject %float %V1_value1

		 %mp_ptr = OpAccessChain %ubo_float_ptr %ubo %uint_0
		 %hp_ptr = OpAccessChain %ubo_float_ptr %ubo %uint_1
		 %ubo_mp0 = OpLoad %float %mp_ptr
		 %ubo_hp0 = OpLoad %float %hp_ptr

		; Stays mediump
         %V4_add = OpFAdd %v4float %V4_value0 %float_3_splat
		; Must promote to highp
         %V4_sub = OpFSub %v4float %V4_value0 %float_3_splat
		; Relaxed, truncate inputs.
         %V4_mul = OpFMul %v4float %V4_sub %float_3_splat
		 OpStore %FragColor0 %V4_add
		 OpStore %FragColor1 %V4_sub
		 OpStore %FragColor2 %V4_mul

		; Same as V4 tests.
         %V1_add = OpFAdd %float %V1_value0 %float_3
		 %float_0_weird = OpFSub %float %float_3 %ubo_hp0
         %V1_sub = OpFSub %float %V1_value0 %float_0_weird
         %V1_mul = OpFMul %float %V1_sub %ubo_hp0
		 %V1_result = OpCompositeConstruct %v4float %V1_add %V1_sub %V1_mul %float_3
		 OpStore %FragColor3 %V1_result

		; Same as V4 tests, but composite forwarding.
         %V1_add_composite = OpFAdd %float %V1_value1 %ubo_mp0
         %V1_sub_composite = OpFSub %float %V1_value2 %ubo_mp0
         %V1_mul_composite = OpFMul %float %V1_sub_composite %ubo_hp0
		 %V1_result_composite = OpCompositeConstruct %v4float %V1_add_composite %V1_sub_composite %V1_mul_composite %float_3
		 OpStore %FragColor3 %V1_result_composite

		 ; Must promote input to highp.
		 %V4_sin0 = OpExtInst %v4float %1 Sin %V4_value0
		 OpStore %FragColor0 %V4_sin0
		 ; Can keep mediump input.
		 %V4_sin1 = OpExtInst %v4float %1 Sin %V4_value0
		 OpStore %FragColor1 %V4_sin1

		OpBranch %next
		%next = OpLabel
			%phi_mp = OpPhi %float %V1_add %5
			%phi_hp = OpPhi %float %V1_sub %5

			; Consume PHIs in different precision contexts
			%mp_to_mp = OpFAdd %float %phi_mp %phi_mp
			%mp_to_hp = OpFAdd %float %phi_mp %phi_mp
			%hp_to_mp = OpFAdd %float %phi_hp %phi_hp
			%hp_to_hp = OpFAdd %float %phi_hp %phi_hp
			%complete = OpCompositeConstruct %v4float %mp_to_mp %mp_to_hp %hp_to_mp %hp_to_hp
			OpStore %FragColor2 %complete

               OpReturn
               OpFunctionEnd
