; SPIR-V
; Version: 1.0
; Generator: Khronos Glslang Reference Front End; 7
; Bound: 27
; Schema: 0
               OpCapability Shader
          %1 = OpExtInstImport "GLSL.std.450"
               OpMemoryModel Logical GLSL450
               OpEntryPoint Fragment %main "main" %FragColor %vColor
               OpExecutionMode %main OriginUpperLeft
               OpSource GLSL 450
               OpName %main "main"
               OpName %FragColor "FragColor"
               OpName %vColor "vColor"
               OpName %omap_r "omap_r"
               OpName %omap_g "omap_g"
               OpName %omap_b "omap_b"
               OpName %omap_a "omap_a"
               OpDecorate %FragColor Location 0
               OpDecorate %vColor Location 0
               OpDecorate %omap_r SpecId 0
               OpDecorate %omap_g SpecId 1
               OpDecorate %omap_b SpecId 2
               OpDecorate %omap_a SpecId 3
       %void = OpTypeVoid
          %3 = OpTypeFunction %void
      %float = OpTypeFloat 32
    %v4float = OpTypeVector %float 4
%_ptr_Output_v4float = OpTypePointer Output %v4float
  %FragColor = OpVariable %_ptr_Output_v4float Output
%_ptr_Input_v4float = OpTypePointer Input %v4float
     %vColor = OpVariable %_ptr_Input_v4float Input
        %int = OpTypeInt 32 1
     %omap_r = OpSpecConstant %int 0
%_ptr_Input_float = OpTypePointer Input %float
     %omap_g = OpSpecConstant %int 1
     %omap_b = OpSpecConstant %int 2
     %omap_a = OpSpecConstant %int 3
       %main = OpFunction %void None %3
          %5 = OpLabel
		  %loaded = OpLoad %v4float %vColor
         %r = OpVectorExtractDynamic %float %loaded %omap_r
         %g = OpVectorExtractDynamic %float %loaded %omap_g
         %b = OpVectorExtractDynamic %float %loaded %omap_b
         %a = OpVectorExtractDynamic %float %loaded %omap_a
         %rgba = OpCompositeConstruct %v4float %r %g %b %a
               OpStore %FragColor %rgba
               OpReturn
               OpFunctionEnd
