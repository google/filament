; SPIR-V
; Version: 1.3
; Generator: Unknown(30017); 21022
; Bound: 31
; Schema: 0
               OpCapability Shader
               OpCapability GroupNonUniformBallot
               OpMemoryModel Logical GLSL450
               OpEntryPoint Fragment %main "main" %INDEX %SV_Target
               OpExecutionMode %main OriginUpperLeft
               OpName %main "main"
               OpName %INDEX "INDEX"
               OpName %SV_Target "SV_Target"
               OpDecorate %INDEX Flat
               OpDecorate %INDEX Location 0
               OpDecorate %SV_Target Location 0
       %void = OpTypeVoid
          %2 = OpTypeFunction %void
       %uint = OpTypeInt 32 0
%_ptr_Input_uint = OpTypePointer Input %uint
      %INDEX = OpVariable %_ptr_Input_uint Input
     %v4uint = OpTypeVector %uint 4
%_ptr_Output_v4uint = OpTypePointer Output %v4uint
  %SV_Target = OpVariable %_ptr_Output_v4uint Output
       %bool = OpTypeBool
   %uint_100 = OpConstant %uint 100
     %uint_3 = OpConstant %uint 3
%_ptr_Output_uint = OpTypePointer Output %uint
     %uint_0 = OpConstant %uint 0
     %uint_1 = OpConstant %uint 1
     %uint_2 = OpConstant %uint 2
       %main = OpFunction %void None %2
          %4 = OpLabel
               OpBranch %29
         %29 = OpLabel
         %11 = OpLoad %uint %INDEX
         %13 = OpULessThan %bool %11 %uint_100
         %15 = OpGroupNonUniformBallot %v4uint %uint_3 %13
         %17 = OpCompositeExtract %uint %15 0
         %18 = OpCompositeExtract %uint %15 1
         %19 = OpCompositeExtract %uint %15 2
         %20 = OpCompositeExtract %uint %15 3
         %22 = OpAccessChain %_ptr_Output_uint %SV_Target %uint_0
               OpStore %22 %17
         %24 = OpAccessChain %_ptr_Output_uint %SV_Target %uint_1
               OpStore %24 %18
         %26 = OpAccessChain %_ptr_Output_uint %SV_Target %uint_2
               OpStore %26 %19
         %28 = OpAccessChain %_ptr_Output_uint %SV_Target %uint_3
               OpStore %28 %20
               OpReturn
               OpFunctionEnd
