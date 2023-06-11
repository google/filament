; SPIR-V
; Version: 1.0
; Generator: Khronos Glslang Reference Front End; 8
; Bound: 30
; Schema: 0
               OpCapability Shader
               OpCapability SparseResidency
          %1 = OpExtInstImport "GLSL.std.450"
               OpMemoryModel Logical GLSL450
               OpEntryPoint Fragment %main "main" %vUV %FragColor
               OpExecutionMode %main OriginUpperLeft
               OpSource GLSL 450
               OpSourceExtension "GL_ARB_sparse_texture2"
               OpSourceExtension "GL_ARB_sparse_texture_clamp"
               OpName %main "main"
               OpName %ret "ret"
               OpName %uSamp "uSamp"
               OpName %vUV "vUV"
               OpName %texel "texel"
               OpName %ResType "ResType"
               OpName %FragColor "FragColor"
               OpDecorate %uSamp DescriptorSet 0
               OpDecorate %uSamp Binding 0
               OpDecorate %vUV Location 0
               OpDecorate %FragColor Location 0
       %void = OpTypeVoid
          %3 = OpTypeFunction %void
       %bool = OpTypeBool
%_ptr_Function_bool = OpTypePointer Function %bool
      %float = OpTypeFloat 32
         %10 = OpTypeImage %float 2D 0 0 0 1 Unknown
         %11 = OpTypeSampledImage %10
%_ptr_UniformConstant_11 = OpTypePointer UniformConstant %11
      %uSamp = OpVariable %_ptr_UniformConstant_11 UniformConstant
    %v2float = OpTypeVector %float 2
%_ptr_Input_v2float = OpTypePointer Input %v2float
        %vUV = OpVariable %_ptr_Input_v2float Input
    %v4float = OpTypeVector %float 4
%_ptr_Function_v4float = OpTypePointer Function %v4float
        %uint = OpTypeInt 32 0
    %ResType = OpTypeStruct %uint %v4float
%_ptr_Output_v4float = OpTypePointer Output %v4float
  %FragColor = OpVariable %_ptr_Output_v4float Output
       %main = OpFunction %void None %3
          %5 = OpLabel
        %ret = OpVariable %_ptr_Function_bool Function
      %texel = OpVariable %_ptr_Function_v4float Function
         %14 = OpLoad %11 %uSamp
         %18 = OpLoad %v2float %vUV
         %24 = OpImageSparseSampleImplicitLod %ResType %14 %18
         %25 = OpCompositeExtract %v4float %24 1
               OpStore %texel %25
         %26 = OpCompositeExtract %uint %24 0
         %27 = OpImageSparseTexelsResident %bool %26
               OpStore %ret %27
               OpReturn
               OpFunctionEnd
