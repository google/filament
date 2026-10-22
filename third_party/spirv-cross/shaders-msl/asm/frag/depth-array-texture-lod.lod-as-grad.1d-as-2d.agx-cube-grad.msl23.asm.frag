; SPIR-V
; Version: 1.3
; Generator: Khronos Glslang Reference Front End; 11
; Bound: 45
; Schema: 0
               OpCapability Shader
               OpCapability SampledCubeArray
          %1 = OpExtInstImport "GLSL.std.450"
               OpMemoryModel Logical GLSL450
               OpEntryPoint Fragment %main "main" %o_color %v_texCoord %v_drefLodBias
               OpExecutionMode %main OriginUpperLeft

               ; Debug Information
               OpSource GLSL 450
               OpName %main "main"  ; id %4
               OpName %o_color "o_color"  ; id %9
               OpName %u_sampler "u_sampler"  ; id %13
               OpName %v_texCoord "v_texCoord"  ; id %16
               OpName %v_drefLodBias "v_drefLodBias"  ; id %21
               OpName %buf0 "buf0"  ; id %39
               OpMemberName %buf0 0 "u_scale"
               OpName %_ ""  ; id %41
               OpName %buf1 "buf1"  ; id %42
               OpMemberName %buf1 0 "u_bias"
               OpName %__0 ""  ; id %44

               ; Annotations
               OpDecorate %o_color RelaxedPrecision
               OpDecorate %o_color Location 0
               OpDecorate %u_sampler DescriptorSet 0
               OpDecorate %u_sampler Binding 0
               OpDecorate %v_texCoord Location 0
               OpDecorate %v_drefLodBias Location 1
               OpMemberDecorate %buf0 0 Offset 0
               OpDecorate %buf0 Block
               OpDecorate %_ DescriptorSet 0
               OpDecorate %_ Binding 1
               OpMemberDecorate %buf1 0 Offset 0
               OpDecorate %buf1 Block
               OpDecorate %__0 DescriptorSet 0
               OpDecorate %__0 Binding 2

               ; Types, variables and constants
       %void = OpTypeVoid
          %3 = OpTypeFunction %void
      %float = OpTypeFloat 32
    %v4float = OpTypeVector %float 4
%_ptr_Output_v4float = OpTypePointer Output %v4float
    %o_color = OpVariable %_ptr_Output_v4float Output
         %10 = OpTypeImage %float Cube 1 1 0 1 Unknown
         %11 = OpTypeSampledImage %10
%_ptr_UniformConstant_11 = OpTypePointer UniformConstant %11
  %u_sampler = OpVariable %_ptr_UniformConstant_11 UniformConstant
%_ptr_Input_v4float = OpTypePointer Input %v4float
 %v_texCoord = OpVariable %_ptr_Input_v4float Input
    %v2float = OpTypeVector %float 2
%_ptr_Input_v2float = OpTypePointer Input %v2float
%v_drefLodBias = OpVariable %_ptr_Input_v2float Input
       %uint = OpTypeInt 32 0
     %uint_0 = OpConstant %uint 0
%_ptr_Input_float = OpTypePointer Input %float
    %v3float = OpTypeVector %float 3
     %uint_1 = OpConstant %uint 1
    %float_0 = OpConstant %float 0
    %float_1 = OpConstant %float 1
       %buf0 = OpTypeStruct %v4float
%_ptr_Uniform_buf0 = OpTypePointer Uniform %buf0
          %_ = OpVariable %_ptr_Uniform_buf0 Uniform
       %buf1 = OpTypeStruct %v4float
%_ptr_Uniform_buf1 = OpTypePointer Uniform %buf1
        %__0 = OpVariable %_ptr_Uniform_buf1 Uniform

               ; Function main
       %main = OpFunction %void None %3
          %5 = OpLabel
         %14 = OpLoad %11 %u_sampler
         %18 = OpLoad %v4float %v_texCoord
         %25 = OpAccessChain %_ptr_Input_float %v_drefLodBias %uint_0
         %26 = OpLoad %float %25
         %32 = OpAccessChain %_ptr_Input_float %v_drefLodBias %uint_1
         %33 = OpLoad %float %32
         %35 = OpImageSampleDrefExplicitLod %float %14 %18 %26 Lod %33
         %38 = OpCompositeConstruct %v4float %35 %float_0 %float_0 %float_1
               OpStore %o_color %38
               OpReturn
               OpFunctionEnd
