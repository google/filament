; SPIR-V
; Version: 1.0
; Generator: Google spiregg; 0
; Bound: 39
; Schema: 0
               OpCapability Shader
               OpMemoryModel Logical GLSL450
               OpEntryPoint Vertex %vs_main "main" %gl_Position
               OpSource HLSL 600
               OpName %vs_main "vs_main"
               OpDecorate %gl_Position BuiltIn Position
        %int = OpTypeInt 32 1
      %int_0 = OpConstant %int 0
      %int_2 = OpConstant %int 2
      %float = OpTypeFloat 32
    %float_0 = OpConstant %float 0
      %int_1 = OpConstant %int 1
    %float_3 = OpConstant %float 3
       %uint = OpTypeInt 32 0
     %uint_0 = OpConstant %uint 0
    %v4float = OpTypeVector %float 4
%_ptr_Output_v4float = OpTypePointer Output %v4float
       %void = OpTypeVoid
         %15 = OpTypeFunction %void
     %uint_2 = OpConstant %uint 2
%_arr_float_uint_2 = OpTypeArray %float %uint_2
%_ptr_Function__arr_float_uint_2 = OpTypePointer Function %_arr_float_uint_2
%_ptr_Function_float = OpTypePointer Function %float
       %bool = OpTypeBool
%gl_Position = OpVariable %_ptr_Output_v4float Output
         %21 = OpUndef %float
    %vs_main = OpFunction %void None %15
         %22 = OpLabel
         %23 = OpVariable %_ptr_Function__arr_float_uint_2 Function
               OpBranch %24
         %24 = OpLabel
         %25 = OpPhi %int %int_0 %22 %26 %27
         %28 = OpSLessThan %bool %25 %int_2
               OpLoopMerge %29 %27 None
               OpBranchConditional %28 %27 %29
         %27 = OpLabel
         %30 = OpAccessChain %_ptr_Function_float %23 %25
               OpStore %30 %float_0
         %26 = OpIAdd %int %25 %int_1
               OpBranch %24
         %29 = OpLabel
         %31 = OpLoad %_arr_float_uint_2 %23
         %32 = OpBitcast %uint %float_3
         %33 = OpINotEqual %bool %32 %uint_0
               OpSelectionMerge %34 None
               OpBranchConditional %33 %35 %34
         %35 = OpLabel
         %36 = OpCompositeExtract %float %31 0
               OpBranch %34
         %34 = OpLabel
         %37 = OpPhi %float %21 %29 %36 %35
         %38 = OpCompositeConstruct %v4float %float_0 %float_0 %float_0 %37
               OpStore %gl_Position %38
               OpReturn
               OpFunctionEnd
