; SPIR-V
; Version: 1.0
; Generator: Khronos Glslang Reference Front End; 6
; Bound: 63
; Schema: 0
               OpCapability Shader
          %1 = OpExtInstImport "GLSL.std.450"
               OpMemoryModel Logical GLSL450
               OpEntryPoint Fragment %main "main" %FragColor
               OpExecutionMode %main OriginUpperLeft
               OpSource GLSL 450
               OpName %main "main"
               OpName %sample_from_func_s21_4__ "sample_from_func(s21[4];"
               OpName %uSampler "uSampler"
               OpName %sample_one_from_func_s21_ "sample_one_from_func(s21;"
               OpName %uSampler_0 "uSampler"
               OpName %Registers "Registers"
               OpMemberName %Registers 0 "index"
               OpName %registers "registers"
               OpName %FragColor "FragColor"
               OpName %uSampler_1 "uSampler"
               OpMemberDecorate %Registers 0 Offset 0
               OpDecorate %Registers Block
               OpDecorate %FragColor Location 0
               OpDecorate %uSampler_1 DescriptorSet 0
               OpDecorate %uSampler_1 Binding 0
       %void = OpTypeVoid
          %3 = OpTypeFunction %void
      %float = OpTypeFloat 32
          %7 = OpTypeImage %float 2D 0 0 0 1 Unknown
       %uint = OpTypeInt 32 0
     %uint_4 = OpConstant %uint 4
%_arr_8_uint_4 = OpTypeArray %7 %uint_4
%_ptr_UniformConstant__arr_8_uint_4 = OpTypePointer UniformConstant %_arr_8_uint_4
    %v4float = OpTypeVector %float 4
         %14 = OpTypeFunction %v4float %_ptr_UniformConstant__arr_8_uint_4
%_ptr_UniformConstant_8 = OpTypePointer UniformConstant %7
         %19 = OpTypeFunction %v4float %_ptr_UniformConstant_8
        %int = OpTypeInt 32 1
  %Registers = OpTypeStruct %int
%_ptr_PushConstant_Registers = OpTypePointer PushConstant %Registers
  %registers = OpVariable %_ptr_PushConstant_Registers PushConstant
      %int_0 = OpConstant %int 0
%_ptr_PushConstant_int = OpTypePointer PushConstant %int
      %v2int = OpTypeVector %int 2
      %int_4 = OpConstant %int 4
         %35 = OpConstantComposite %v2int %int_4 %int_4
%_ptr_Output_v4float = OpTypePointer Output %v4float
  %FragColor = OpVariable %_ptr_Output_v4float Output
 %uSampler_1 = OpVariable %_ptr_UniformConstant__arr_8_uint_4 UniformConstant
     %int_10 = OpConstant %int 10
         %53 = OpConstantComposite %v2int %int_10 %int_10
       %main = OpFunction %void None %3
          %5 = OpLabel
         %48 = OpAccessChain %_ptr_PushConstant_int %registers %int_0
         %49 = OpLoad %int %48
         %50 = OpAccessChain %_ptr_UniformConstant_8 %uSampler_1 %49
         %51 = OpLoad %7 %50
         %55 = OpImageFetch %v4float %51 %53 Lod %int_0
         %56 = OpFunctionCall %v4float %sample_from_func_s21_4__ %uSampler_1
         %57 = OpFAdd %v4float %55 %56
         %58 = OpAccessChain %_ptr_PushConstant_int %registers %int_0
         %59 = OpLoad %int %58
         %60 = OpAccessChain %_ptr_UniformConstant_8 %uSampler_1 %59
         %61 = OpFunctionCall %v4float %sample_one_from_func_s21_ %60
         %62 = OpFAdd %v4float %57 %61
               OpStore %FragColor %62
               OpReturn
               OpFunctionEnd
%sample_from_func_s21_4__ = OpFunction %v4float None %14
   %uSampler = OpFunctionParameter %_ptr_UniformConstant__arr_8_uint_4
         %17 = OpLabel
         %29 = OpAccessChain %_ptr_PushConstant_int %registers %int_0
         %30 = OpLoad %int %29
         %31 = OpAccessChain %_ptr_UniformConstant_8 %uSampler %30
         %32 = OpLoad %7 %31
         %37 = OpImageFetch %v4float %32 %35 Lod %int_0
               OpReturnValue %37
               OpFunctionEnd
%sample_one_from_func_s21_ = OpFunction %v4float None %19
 %uSampler_0 = OpFunctionParameter %_ptr_UniformConstant_8
         %22 = OpLabel
         %40 = OpLoad %7 %uSampler_0
         %42 = OpImageFetch %v4float %40 %35 Lod %int_0
               OpReturnValue %42
               OpFunctionEnd
