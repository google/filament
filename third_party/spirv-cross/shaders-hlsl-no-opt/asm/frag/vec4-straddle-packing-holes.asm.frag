; SPIR-V
; Version: 1.0
; Generator: Google spiregg; 0
; Bound: 29
; Schema: 0
               OpCapability Shader
               OpMemoryModel Logical GLSL450
               OpEntryPoint Fragment %PSMain "main" %out_var_SV_Target
               OpExecutionMode %PSMain OriginUpperLeft
               OpSource HLSL 600
               OpName %type_Test "type.Test"
               OpMemberName %type_Test 0 "V0_xyz_"
               OpMemberName %type_Test 1 "V1___zw"
               OpName %Test "Test"
               OpName %out_var_SV_Target "out.var.SV_Target"
               OpName %PSMain "PSMain"
               OpDecorate %out_var_SV_Target Location 0
               OpDecorate %Test DescriptorSet 0
               OpDecorate %Test Binding 0
               OpMemberDecorate %type_Test 0 Offset 0
               OpMemberDecorate %type_Test 1 Offset 24
               OpDecorate %type_Test Block
        %int = OpTypeInt 32 1
      %int_0 = OpConstant %int 0
      %float = OpTypeFloat 32
    %v3float = OpTypeVector %float 3
    %v2float = OpTypeVector %float 2
  %type_Test = OpTypeStruct %v3float %v2float
%_ptr_Uniform_type_Test = OpTypePointer Uniform %type_Test
    %v4float = OpTypeVector %float 4
%_ptr_Output_v4float = OpTypePointer Output %v4float
       %void = OpTypeVoid
         %15 = OpTypeFunction %void
%_ptr_Uniform_v3float = OpTypePointer Uniform %v3float
%_ptr_Uniform_float = OpTypePointer Uniform %float
       %Test = OpVariable %_ptr_Uniform_type_Test Uniform
%out_var_SV_Target = OpVariable %_ptr_Output_v4float Output
       %uint = OpTypeInt 32 0
     %uint_1 = OpConstant %uint 1
     %PSMain = OpFunction %void None %15
         %18 = OpLabel
         %19 = OpAccessChain %_ptr_Uniform_v3float %Test %int_0
         %20 = OpLoad %v3float %19
         %21 = OpAccessChain %_ptr_Uniform_float %Test %uint_1 %int_0
         %22 = OpLoad %float %21
         %23 = OpCompositeExtract %float %20 0
         %24 = OpCompositeExtract %float %20 1
         %25 = OpCompositeExtract %float %20 2
         %26 = OpCompositeConstruct %v4float %23 %24 %25 %22
               OpStore %out_var_SV_Target %26
               OpReturn
               OpFunctionEnd
