; SPIR-V
; Version: 1.0
; Generator: Khronos Glslang Reference Front End; 7
; Bound: 28
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
      %int_1 = OpConstant %int 1
     %int_20 = OpConstant %int 20
       %bool = OpTypeBool
       %main = OpFunction %void None %3
          %5 = OpLabel
          %i = OpVariable %_ptr_Function_int Function
          %j = OpVariable %_ptr_Function_int Function
               OpStore %i %int_0
               OpStore %j %int_0
               OpBranch %11
         %11 = OpLabel
               OpLoopMerge %13 %14 None
               OpBranch %12
         %12 = OpLabel
         %15 = OpLoad %int %j
         %16 = OpLoad %int %i
         %17 = OpIAdd %int %15 %16
         %19 = OpIAdd %int %17 %int_1
         %20 = OpLoad %int %j
         %21 = OpIMul %int %19 %20
               OpStore %j %21
         %22 = OpLoad %int %i
         %23 = OpIAdd %int %22 %int_1
               OpStore %i %23
               OpBranch %14
         %14 = OpLabel
         %24 = OpLoad %int %i
         %27 = OpIEqual %bool %24 %int_20
               OpBranchConditional %27 %13 %11
         %13 = OpLabel
               OpReturn
               OpFunctionEnd
