; SPIR-V
; Version: 1.0
; Generator: Khronos Glslang Reference Front End; 10
; Bound: 30
; Schema: 0
               OpCapability Shader
          %1 = OpExtInstImport "GLSL.std.450"
               OpMemoryModel Logical GLSL450
               OpEntryPoint Fragment %main "main" %vInput %FragColor
               OpExecutionMode %main OriginUpperLeft
               OpSource ESSL 310
               OpName %main "main"
               OpName %vInput "vInput"
               OpName %FragColor "FragColor"
			   OpName %phi "PHI"
               OpDecorate %vInput RelaxedPrecision
               OpDecorate %vInput Location 0
               OpDecorate %FragColor RelaxedPrecision
               OpDecorate %FragColor Location 0
			   OpDecorate %b0 RelaxedPrecision
			   OpDecorate %b1 RelaxedPrecision
			   OpDecorate %b2 RelaxedPrecision
			   OpDecorate %b3 RelaxedPrecision
			   OpDecorate %c1 RelaxedPrecision
			   OpDecorate %c3 RelaxedPrecision
			   OpDecorate %d4_mp RelaxedPrecision
       %void = OpTypeVoid
          %3 = OpTypeFunction %void
      %float = OpTypeFloat 32
    %v4float = OpTypeVector %float 4
%_ptr_Function_v4float = OpTypePointer Function %v4float
%_ptr_Input_v4float = OpTypePointer Input %v4float
     %vInput = OpVariable %_ptr_Input_v4float Input
    %float_1 = OpConstant %float 1
       %uint = OpTypeInt 32 0
     %uint_0 = OpConstant %uint 0
%_ptr_Function_float = OpTypePointer Function %float
    %float_2 = OpConstant %float 2
     %uint_1 = OpConstant %uint 1
    %float_3 = OpConstant %float 3
     %uint_2 = OpConstant %uint 2
    %float_4 = OpConstant %float 4
     %uint_3 = OpConstant %uint 3
	 %v4float_arr2 = OpTypeArray %v4float %uint_2
	 %v44float = OpTypeMatrix %v4float 4
%_ptr_Output_v4float = OpTypePointer Output %v4float
	%v4undef = OpUndef %v4float
	%v4const = OpConstantNull %v4float
	%v4arrconst = OpConstantNull %v4float_arr2
	%v44const = OpConstantNull %v44float
  %FragColor = OpVariable %_ptr_Output_v4float Output
       %main = OpFunction %void None %3
          %5 = OpLabel

         %loaded0 = OpLoad %v4float %vInput

		; Basic case (highp).
         %a0 = OpCompositeInsert %v4float %float_1 %loaded0 0
         %a1 = OpCompositeInsert %v4float %float_2 %a0 1
         %a2 = OpCompositeInsert %v4float %float_3 %a1 2
         %a3 = OpCompositeInsert %v4float %float_4 %a2 3
		 	OpStore %FragColor %a3

		; Basic case (mediump).
         %b0 = OpCompositeInsert %v4float %float_1 %loaded0 0
         %b1 = OpCompositeInsert %v4float %float_2 %b0 1
         %b2 = OpCompositeInsert %v4float %float_3 %b1 2
         %b3 = OpCompositeInsert %v4float %float_4 %b2 3
		 	OpStore %FragColor %b3

		; Mix relaxed precision.
         %c0 = OpCompositeInsert %v4float %float_1 %loaded0 0
         %c1 = OpCompositeInsert %v4float %float_2 %c0 1
         %c2 = OpCompositeInsert %v4float %float_3 %c1 2
         %c3 = OpCompositeInsert %v4float %float_4 %c2 3
		 	OpStore %FragColor %c3

		; SSA use after insert
         %d0 = OpCompositeInsert %v4float %float_1 %loaded0 0
         %d1 = OpCompositeInsert %v4float %float_2 %d0 1
         %d2 = OpCompositeInsert %v4float %float_3 %d1 2
         %d3 = OpCompositeInsert %v4float %float_4 %d2 3
		 %d4 = OpFAdd %v4float %d3 %d0
		 	OpStore %FragColor %d4
		 %d4_mp = OpFAdd %v4float %d3 %d1
		 	OpStore %FragColor %d4_mp

		; Verify Insert behavior on Undef.
		  %e0 = OpCompositeInsert %v4float %float_1 %v4undef 0
		  %e1 = OpCompositeInsert %v4float %float_2 %e0 1
		  %e2 = OpCompositeInsert %v4float %float_3 %e1 2
		  %e3 = OpCompositeInsert %v4float %float_4 %e2 3
		 	OpStore %FragColor %e3

		; Verify Insert behavior on Constant.
		  %f0 = OpCompositeInsert %v4float %float_1 %v4const 0
		 	OpStore %FragColor %f0

		; Verify Insert behavior on Array.
		  %g0 = OpCompositeInsert %v4float_arr2 %float_1 %v4arrconst 1 2
		  %g1 = OpCompositeInsert %v4float_arr2 %float_2 %g0 0 3
		  %g2 = OpCompositeExtract %v4float %g1 0
		 	OpStore %FragColor %g2
		  %g3 = OpCompositeExtract %v4float %g1 1
		 	OpStore %FragColor %g3

		; Verify Insert behavior on Matrix.
		  %h0 = OpCompositeInsert %v44float %float_1 %v44const 1 2
		  %h1 = OpCompositeInsert %v44float %float_2 %h0 2 3
		  %h2 = OpCompositeExtract %v4float %h1 0
		 	OpStore %FragColor %h2
		  %h3 = OpCompositeExtract %v4float %h1 1
		 	OpStore %FragColor %h3
		  %h4 = OpCompositeExtract %v4float %h1 2
		 	OpStore %FragColor %h4
		  %h5 = OpCompositeExtract %v4float %h1 3
		 	OpStore %FragColor %h5

		; Verify that we cannot RMW PHI variables.
		OpBranch %next
		%next = OpLabel
		%phi = OpPhi %v4float %d2 %5
         %i0 = OpCompositeInsert %v4float %float_4 %phi 3
		 	OpStore %FragColor %i0

               OpReturn
               OpFunctionEnd
