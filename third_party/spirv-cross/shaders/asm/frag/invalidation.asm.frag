; SPIR-V
; Version: 1.0
; Generator: Khronos Glslang Reference Front End; 1
; Bound: 28
; Schema: 0
               OpCapability Shader
          %1 = OpExtInstImport "GLSL.std.450"
               OpMemoryModel Logical GLSL450
               OpEntryPoint Fragment %4 "main" %v0 %v1 %FragColor
               OpExecutionMode %4 OriginUpperLeft
               OpSource GLSL 450
               OpName %4 "main"
               OpName %a "a"
               OpName %v0 "v0"
               OpName %b "b"
               OpName %v1 "v1"
               OpName %FragColor "FragColor"
               OpDecorate %v0 Location 0
               OpDecorate %v1 Location 1
               OpDecorate %FragColor Location 0
          %2 = OpTypeVoid
          %3 = OpTypeFunction %2
          %float = OpTypeFloat 32
          %pfloat = OpTypePointer Function %float
          %9 = OpTypePointer Input %float
         %v0 = OpVariable %9 Input
         %v1 = OpVariable %9 Input
         %25 = OpTypePointer Output %float
         %FragColor = OpVariable %25 Output
          %4 = OpFunction %2 None %3
          %5 = OpLabel
         %a = OpVariable %pfloat Function
         %b = OpVariable %pfloat Function
         %v0_tmp = OpLoad %float %v0
         %v1_tmp = OpLoad %float %v1
               OpStore %a %v0_tmp
               OpStore %b %v1_tmp

	     %a_tmp = OpLoad %float %a
	     %b_tmp = OpLoad %float %b
         %res = OpFAdd %float %a_tmp %b_tmp
		 %res1 = OpFMul %float %res %b_tmp
               OpStore %a %v1_tmp
               OpStore %FragColor %res1
               OpReturn
               OpFunctionEnd
