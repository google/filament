; SPIR-V
; Version: 1.0
; Generator: Wine VKD3D Shader Compiler; 0
; Bound: 54
; Schema: 0
               OpCapability Shader
               OpMemoryModel Logical GLSL450
               OpEntryPoint Fragment %main "main" %o0
			   OpExecutionMode %main OriginUpperLeft
               OpName %main "main"
               OpName %t0 "t0"
               OpName %o0 "o0"
               OpName %r0 "r0"
               OpName %push_cb "push_cb"
               OpMemberName %push_cb 0 "cb0"
               OpName %dummy_sampler "dummy_sampler"
               OpDecorate %t0 DescriptorSet 0
               OpDecorate %t0 Binding 2
               OpDecorate %o0 Location 0
               OpDecorate %_arr_v4float_uint_1 ArrayStride 16
               OpDecorate %push_cb Block
               OpMemberDecorate %push_cb 0 Offset 0
               OpDecorate %dummy_sampler DescriptorSet 0
               OpDecorate %dummy_sampler Binding 4
       %void = OpTypeVoid
          %2 = OpTypeFunction %void
      %float = OpTypeFloat 32
          %6 = OpTypeImage %float 2D 0 0 0 1 Unknown
%_ptr_UniformConstant_6 = OpTypePointer UniformConstant %6
         %t0 = OpVariable %_ptr_UniformConstant_6 UniformConstant
    %v4float = OpTypeVector %float 4
%_ptr_Output_v4float = OpTypePointer Output %v4float
         %o0 = OpVariable %_ptr_Output_v4float Output
%_ptr_Function_v4float = OpTypePointer Function %v4float
       %uint = OpTypeInt 32 0
     %uint_1 = OpConstant %uint 1
%_arr_v4float_uint_1 = OpTypeArray %v4float %uint_1
    %push_cb = OpTypeStruct %_arr_v4float_uint_1
%_ptr_PushConstant_push_cb = OpTypePointer PushConstant %push_cb
         %19 = OpVariable %_ptr_PushConstant_push_cb PushConstant
     %uint_0 = OpConstant %uint 0
%_ptr_PushConstant_v4float = OpTypePointer PushConstant %v4float
%_ptr_PushConstant_float = OpTypePointer PushConstant %float
        %int = OpTypeInt 32 1
    %v2float = OpTypeVector %float 2
    %float_0 = OpConstant %float 0
         %30 = OpConstantComposite %v2float %float_0 %float_0
         %33 = OpTypeSampler
%_ptr_UniformConstant_33 = OpTypePointer UniformConstant %33
%dummy_sampler = OpVariable %_ptr_UniformConstant_33 UniformConstant
         %38 = OpTypeSampledImage %6
      %v2int = OpTypeVector %int 2
%_ptr_Function_float = OpTypePointer Function %float
     %uint_3 = OpConstant %uint 3
     %int_n1 = OpConstant %int -1
     %int_n2 = OpConstant %int -2
         %52 = OpConstantComposite %v2int %int_n1 %int_n2
       %main = OpFunction %void None %2
          %4 = OpLabel
         %r0 = OpVariable %_ptr_Function_v4float Function
         %23 = OpAccessChain %_ptr_PushConstant_v4float %19 %uint_0 %uint_0
         %25 = OpLoad %v4float %23
         %26 = OpLoad %v4float %r0
         %27 = OpVectorShuffle %v4float %26 %25 6 7 2 3
               OpStore %r0 %27
         %31 = OpLoad %v4float %r0
         %32 = OpVectorShuffle %v4float %31 %30 0 1 4 5
               OpStore %r0 %32
         %36 = OpLoad %6 %t0
         %37 = OpLoad %33 %dummy_sampler
         %39 = OpSampledImage %38 %36 %37
         %40 = OpImage %6 %39
         %41 = OpLoad %v4float %r0
         %42 = OpVectorShuffle %v2float %41 %41 0 1
         %44 = OpBitcast %v2int %42
         %47 = OpInBoundsAccessChain %_ptr_Function_float %r0 %uint_3
         %48 = OpLoad %float %47
         %49 = OpBitcast %int %48
         %54 = OpImageFetch %v4float %40 %44 Lod|ConstOffset %49 %52
               OpStore %o0 %54
               OpReturn
               OpFunctionEnd
