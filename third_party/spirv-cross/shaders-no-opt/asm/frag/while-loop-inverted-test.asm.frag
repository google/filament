; SPIR-V
; Version: 1.0
; Generator: Khronos Glslang Reference Front End; 7
; Bound: 29
; Schema: 0
               OpCapability Shader
          %1 = OpExtInstImport "GLSL.std.450"
               OpMemoryModel Logical GLSL450
               OpEntryPoint Fragment %main "main"
               OpExecutionMode %main OriginUpperLeft
               OpSource GLSL 450
               OpName %main "main"
               OpName %i "i"
               OpName %j "j"
       %void = OpTypeVoid
          %3 = OpTypeFunction %void
        %int = OpTypeInt 32 1
%_ptr_Function_int = OpTypePointer Function %int
      %int_0 = OpConstant %int 0
     %int_20 = OpConstant %int 20
       %bool = OpTypeBool
      %int_1 = OpConstant %int 1
       %main = OpFunction %void None %3
          %5 = OpLabel
          %i = OpVariable %_ptr_Function_int Function
          %j = OpVariable %_ptr_Function_int Function
               OpStore %i %int_0
               OpStore %j %int_0
               OpBranch %11
         %11 = OpLabel
               OpLoopMerge %13 %14 None
               OpBranch %15
         %15 = OpLabel
         %16 = OpLoad %int %i
         %19 = OpIEqual %bool %16 %int_20
               OpBranchConditional %19 %13 %12
         %12 = OpLabel
         %20 = OpLoad %int %j
         %21 = OpLoad %int %i
         %22 = OpIAdd %int %20 %21
         %24 = OpIAdd %int %22 %int_1
         %25 = OpLoad %int %j
         %26 = OpIMul %int %24 %25
               OpStore %j %26
         %27 = OpLoad %int %i
         %28 = OpIAdd %int %27 %int_1
               OpStore %i %28
               OpBranch %14
         %14 = OpLabel
               OpBranch %11
         %13 = OpLabel
               OpReturn
               OpFunctionEnd
