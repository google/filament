; SPIR-V
; Version: 1.0
; Generator: Wine VKD3D Shader Compiler; 0
; Bound: 67
; Schema: 0
               OpCapability Shader
               OpMemoryModel Logical GLSL450
               OpEntryPoint Fragment %main "main" %8 %16 %22 %28 %33 %o0
			   OpExecutionMode %main OriginUpperLeft
               OpName %main "main"
               OpName %v1 "v1"
               OpName %v2 "v2"
               OpName %o0 "o0"
               OpName %r0 "r0"
               OpDecorate %8 Location 1
               OpDecorate %16 Location 1
               OpDecorate %16 Component 2
               OpDecorate %22 Location 2
               OpDecorate %22 Flat
               OpDecorate %28 Location 2
               OpDecorate %28 Component 1
               OpDecorate %28 Flat
               OpDecorate %33 Location 2
               OpDecorate %33 Component 2
               OpDecorate %33 Flat
               OpDecorate %o0 Location 0
       %void = OpTypeVoid
          %2 = OpTypeFunction %void
      %float = OpTypeFloat 32
    %v2float = OpTypeVector %float 2
%_ptr_Input_v2float = OpTypePointer Input %v2float
          %8 = OpVariable %_ptr_Input_v2float Input
    %v4float = OpTypeVector %float 4
%_ptr_Private_v4float = OpTypePointer Private %v4float
         %v1 = OpVariable %_ptr_Private_v4float Private
%_ptr_Input_float = OpTypePointer Input %float
         %16 = OpVariable %_ptr_Input_float Input
%_ptr_Private_float = OpTypePointer Private %float
       %uint = OpTypeInt 32 0
     %uint_2 = OpConstant %uint 2
         %22 = OpVariable %_ptr_Input_float Input
         %v2 = OpVariable %_ptr_Private_v4float Private
     %uint_0 = OpConstant %uint 0
%_ptr_Input_uint = OpTypePointer Input %uint
         %28 = OpVariable %_ptr_Input_uint Input
     %uint_1 = OpConstant %uint 1
         %33 = OpVariable %_ptr_Input_uint Input
%_ptr_Output_v4float = OpTypePointer Output %v4float
         %o0 = OpVariable %_ptr_Output_v4float Output
%_ptr_Function_v4float = OpTypePointer Function %v4float
        %int = OpTypeInt 32 1
%_ptr_Function_float = OpTypePointer Function %float
%_ptr_Output_float = OpTypePointer Output %float
       %main = OpFunction %void None %2
          %4 = OpLabel
         %r0 = OpVariable %_ptr_Function_v4float Function
         %12 = OpLoad %v2float %8
         %13 = OpLoad %v4float %v1
         %14 = OpVectorShuffle %v4float %13 %12 4 5 2 3
               OpStore %v1 %14
         %17 = OpLoad %float %16
         %21 = OpInBoundsAccessChain %_ptr_Private_float %v1 %uint_2
               OpStore %21 %17
         %24 = OpLoad %float %22
         %26 = OpInBoundsAccessChain %_ptr_Private_float %v2 %uint_0
               OpStore %26 %24
         %29 = OpLoad %uint %28
         %30 = OpBitcast %float %29
         %32 = OpInBoundsAccessChain %_ptr_Private_float %v2 %uint_1
               OpStore %32 %30
         %34 = OpLoad %uint %33
         %35 = OpBitcast %float %34
         %36 = OpInBoundsAccessChain %_ptr_Private_float %v2 %uint_2
               OpStore %36 %35
         %42 = OpInBoundsAccessChain %_ptr_Private_float %v2 %uint_1
         %43 = OpLoad %float %42
         %44 = OpBitcast %int %43
         %45 = OpInBoundsAccessChain %_ptr_Private_float %v2 %uint_2
         %46 = OpLoad %float %45
         %47 = OpBitcast %int %46
         %48 = OpIAdd %int %44 %47
         %49 = OpBitcast %float %48
         %51 = OpInBoundsAccessChain %_ptr_Function_float %r0 %uint_0
               OpStore %51 %49
         %52 = OpInBoundsAccessChain %_ptr_Function_float %r0 %uint_0
         %53 = OpLoad %float %52
         %54 = OpBitcast %uint %53
         %55 = OpConvertUToF %float %54
         %57 = OpInBoundsAccessChain %_ptr_Output_float %o0 %uint_1
               OpStore %57 %55
         %58 = OpInBoundsAccessChain %_ptr_Private_float %v1 %uint_1
         %59 = OpLoad %float %58
         %60 = OpInBoundsAccessChain %_ptr_Private_float %v2 %uint_0
         %61 = OpLoad %float %60
         %62 = OpFAdd %float %59 %61
         %63 = OpInBoundsAccessChain %_ptr_Output_float %o0 %uint_0
               OpStore %63 %62
         %64 = OpLoad %v4float %v1
         %65 = OpLoad %v4float %o0
         %66 = OpVectorShuffle %v4float %65 %64 0 1 6 4
               OpStore %o0 %66
               OpReturn
               OpFunctionEnd
