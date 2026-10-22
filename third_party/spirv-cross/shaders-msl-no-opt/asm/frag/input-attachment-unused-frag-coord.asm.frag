; SPIR-V
; Version: 1.0
; Generator: Khronos Glslang Reference Front End; 8
; Bound: 35
; Schema: 0
               OpCapability Shader
               OpCapability InputAttachment
          %1 = OpExtInstImport "GLSL.std.450"
               OpMemoryModel Logical GLSL450
               OpEntryPoint Fragment %main "main" %FragColor %gl_FragCoord
               OpExecutionMode %main OriginUpperLeft
               OpSource ESSL 310
               OpName %main "main"
               OpName %load_subpasses_IP1_ "load_subpasses(IP1;"
               OpName %uInput "uInput"
               OpName %FragColor "FragColor"
               OpName %uSubpass0 "uSubpass0"
               OpName %uSubpass1 "uSubpass1"
               OpName %gl_FragCoord "gl_FragCoord"
               OpDecorate %load_subpasses_IP1_ RelaxedPrecision
               OpDecorate %uInput RelaxedPrecision
               OpDecorate %14 RelaxedPrecision
               OpDecorate %19 RelaxedPrecision
               OpDecorate %FragColor RelaxedPrecision
               OpDecorate %FragColor Location 0
               OpDecorate %uSubpass0 RelaxedPrecision
               OpDecorate %uSubpass0 DescriptorSet 0
               OpDecorate %uSubpass0 Binding 0
               OpDecorate %uSubpass0 InputAttachmentIndex 0
               OpDecorate %25 RelaxedPrecision
               OpDecorate %26 RelaxedPrecision
               OpDecorate %uSubpass1 RelaxedPrecision
               OpDecorate %uSubpass1 DescriptorSet 0
               OpDecorate %uSubpass1 Binding 1
               OpDecorate %uSubpass1 InputAttachmentIndex 1
               OpDecorate %28 RelaxedPrecision
               OpDecorate %29 RelaxedPrecision
               OpDecorate %gl_FragCoord BuiltIn FragCoord
       %void = OpTypeVoid
          %3 = OpTypeFunction %void
      %float = OpTypeFloat 32
          %7 = OpTypeImage %float SubpassData 0 0 0 2 Unknown
%_ptr_UniformConstant_7 = OpTypePointer UniformConstant %7
    %v4float = OpTypeVector %float 4
         %10 = OpTypeFunction %v4float %_ptr_UniformConstant_7
        %int = OpTypeInt 32 1
      %int_0 = OpConstant %int 0
      %v2int = OpTypeVector %int 2
         %18 = OpConstantComposite %v2int %int_0 %int_0
%_ptr_Output_v4float = OpTypePointer Output %v4float
  %FragColor = OpVariable %_ptr_Output_v4float Output
  %uSubpass0 = OpVariable %_ptr_UniformConstant_7 UniformConstant
  %uSubpass1 = OpVariable %_ptr_UniformConstant_7 UniformConstant
%_ptr_Input_v4float = OpTypePointer Input %v4float
%gl_FragCoord = OpVariable %_ptr_Input_v4float Input
       %main = OpFunction %void None %3
          %5 = OpLabel
         %25 = OpLoad %7 %uSubpass0
         %26 = OpImageRead %v4float %25 %18
         %28 = OpFunctionCall %v4float %load_subpasses_IP1_ %uSubpass1
         %29 = OpFAdd %v4float %26 %28
         ;%32 = OpLoad %v4float %gl_FragCoord
         ;%33 = OpVectorShuffle %v4float %32 %32 0 1 0 1
         ;%34 = OpFAdd %v4float %29 %33
               OpStore %FragColor %29
               OpReturn
               OpFunctionEnd
%load_subpasses_IP1_ = OpFunction %v4float None %10
     %uInput = OpFunctionParameter %_ptr_UniformConstant_7
         %13 = OpLabel
         %14 = OpLoad %7 %uInput
         %19 = OpImageRead %v4float %14 %18
               OpReturnValue %19
               OpFunctionEnd
