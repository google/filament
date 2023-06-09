; SPIR-V
; Version: 1.0
; Generator: Khronos Glslang Reference Front End; 6
; Bound: 19
; Schema: 0
               OpCapability Shader
               OpCapability ImageQuery
          %1 = OpExtInstImport "GLSL.std.450"
               OpMemoryModel Logical GLSL450
               OpEntryPoint Fragment %main "main" %Size
               OpExecutionMode %main OriginUpperLeft
               OpSource GLSL 450
               OpName %main "main"
               OpName %Size "Size"
               OpName %uTexture "uTexture"
               OpDecorate %Size Location 0
               OpDecorate %uTexture DescriptorSet 0
               OpDecorate %uTexture Binding 0
       %void = OpTypeVoid
          %3 = OpTypeFunction %void
        %int = OpTypeInt 32 1
      %v2int = OpTypeVector %int 2
%_ptr_Output_v2int = OpTypePointer Output %v2int
       %Size = OpVariable %_ptr_Output_v2int Output
      %float = OpTypeFloat 32
         %11 = OpTypeImage %float 2D 0 0 0 1 Unknown
         %12 = OpTypeSampledImage %11
%_ptr_UniformConstant_12 = OpTypePointer UniformConstant %12
   %uTexture = OpVariable %_ptr_UniformConstant_12 UniformConstant
      %int_0 = OpConstant %int 0
      %int_1 = OpConstant %int 1
       %main = OpFunction %void None %3
          %5 = OpLabel
         %15 = OpLoad %12 %uTexture
         %17 = OpImage %11 %15
         %18 = OpImageQuerySizeLod %v2int %17 %int_0
         %19 = OpImageQuerySizeLod %v2int %17 %int_1
		 %20 = OpIAdd %v2int %18 %19
               OpStore %Size %20
               OpReturn
               OpFunctionEnd
