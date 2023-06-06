; SPIR-V
; Version: 1.0
; Generator: Khronos Glslang Reference Front End; 7
; Bound: 25
; Schema: 0
               OpCapability Shader
               OpCapability StorageInputOutput16
               OpCapability Float16
               OpExtension "SPV_KHR_16bit_storage"
          %1 = OpExtInstImport "GLSL.std.450"
               OpMemoryModel Logical GLSL450
               OpEntryPoint Fragment %main "main" %FragColor %UV
               OpExecutionMode %main OriginUpperLeft
               OpSource GLSL 450
               OpSourceExtension "GL_EXT_shader_explicit_arithmetic_types_float16"
               OpName %main "main"
               OpName %FragColor "FragColor"
               OpName %uTexture "uTexture"
               OpName %UV "UV"
               OpDecorate %FragColor Location 0
               OpDecorate %uTexture DescriptorSet 0
               OpDecorate %uTexture Binding 0
               OpDecorate %UV Location 0
       %void = OpTypeVoid
          %3 = OpTypeFunction %void
       %half = OpTypeFloat 16
       %float = OpTypeFloat 32
     %v4half = OpTypeVector %half 4
     %v4float = OpTypeVector %float 4
%_ptr_Output_v4half = OpTypePointer Output %v4half
  %FragColor = OpVariable %_ptr_Output_v4half Output
         %11 = OpTypeImage %float 2D 0 0 0 1 Unknown
         %12 = OpTypeSampledImage %11
%_ptr_UniformConstant_12 = OpTypePointer UniformConstant %12
   %uTexture = OpVariable %_ptr_UniformConstant_12 UniformConstant
     %v2half = OpTypeVector %half 2
%_ptr_Input_v2half = OpTypePointer Input %v2half
         %UV = OpVariable %_ptr_Input_v2half Input
       %main = OpFunction %void None %3
          %5 = OpLabel
         %15 = OpLoad %12 %uTexture
         %19 = OpLoad %v2half %UV
         %23 = OpImageSampleImplicitLod %v4float %15 %19
		 %24 = OpFConvert %v4half %23
               OpStore %FragColor %24
               OpReturn
               OpFunctionEnd
