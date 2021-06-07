; SPIR-V
; Version: 1.3
; Generator: Google Tint Compiler; 0
; Bound: 29
; Schema: 0
                                OpCapability Shader
                                OpMemoryModel Logical GLSL450
                                OpEntryPoint Fragment %main "main" %fragColor %gl_SampleMask
                                OpExecutionMode %main OriginUpperLeft
                                OpName %fragColor "fragColor"
                                OpName %uBuffer "uBuffer"
                                OpMemberName %uBuffer 0 "color"
                                OpName %x_12 "x_12"
                                OpName %gl_SampleMask "gl_SampleMask"
                                OpName %main "main"
                                OpDecorate %fragColor Location 0
                                OpDecorate %uBuffer Block
                                OpMemberDecorate %uBuffer 0 Offset 0
                                OpDecorate %x_12 DescriptorSet 0
                                OpDecorate %x_12 Binding 0
                                OpDecorate %gl_SampleMask BuiltIn SampleMask
                       %float = OpTypeFloat 32
                     %v4float = OpTypeVector %float 4
         %_ptr_Output_v4float = OpTypePointer Output %v4float
                           %5 = OpConstantNull %v4float
                   %fragColor = OpVariable %_ptr_Output_v4float Output %5
                     %uBuffer = OpTypeStruct %v4float
        %_ptr_Uniform_uBuffer = OpTypePointer Uniform %uBuffer
                        %x_12 = OpVariable %_ptr_Uniform_uBuffer Uniform
                        %uint = OpTypeInt 32 0
                      %uint_1 = OpConstant %uint 1
            %_arr_uint_uint_1 = OpTypeArray %uint %uint_1
%_ptr_Output__arr_uint_uint_1 = OpTypePointer Output %_arr_uint_uint_1
                          %14 = OpConstantNull %_arr_uint_uint_1
               %gl_SampleMask = OpVariable %_ptr_Output__arr_uint_uint_1 Output %14
                        %void = OpTypeVoid
                          %15 = OpTypeFunction %void
                      %uint_0 = OpConstant %uint 0
        %_ptr_Uniform_v4float = OpTypePointer Uniform %v4float
                         %int = OpTypeInt 32 1
                       %int_0 = OpConstant %int 0
            %_ptr_Output_uint = OpTypePointer Output %uint
                       %int_6 = OpConstant %int 6
                        %main = OpFunction %void None %15
                          %18 = OpLabel
                          %21 = OpAccessChain %_ptr_Uniform_v4float %x_12 %uint_0
                          %22 = OpLoad %v4float %21
                                OpStore %fragColor %22
                          %26 = OpAccessChain %_ptr_Output_uint %gl_SampleMask %int_0
                          %27 = OpBitcast %uint %int_6
                                OpStore %26 %27
                            %loaded_scalar = OpLoad %uint %26
								OpStore %26 %loaded_scalar
                             %loaded = OpLoad %_arr_uint_uint_1 %gl_SampleMask
                                OpStore %gl_SampleMask %loaded
                                OpReturn
                                OpFunctionEnd
