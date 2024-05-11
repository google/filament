               OpCapability Shader
          %1 = OpExtInstImport "GLSL.std.450"
               OpMemoryModel Logical GLSL450
               OpEntryPoint Fragment %main "main"
               OpExecutionMode %main OriginUpperLeft
               OpSource GLSL 450
               OpName %main "main"
       %void = OpTypeVoid
          %3 = OpTypeFunction %void
        %int = OpTypeInt 32 1
      %int_0 = OpConstant %int 0
     %int_16 = OpConstant %int 16
       %bool = OpTypeBool
      %int_1 = OpConstant %int 1
       %main = OpFunction %void None %3
          %5 = OpLabel
               OpBranch %8
          %8 = OpLabel
         %10 = OpPhi %int %12 %7 %int_0 %5
               OpLoopMerge %6 %7 None
               OpBranch %11
         %11 = OpLabel
         %16 = OpINotEqual %bool %10 %int_16
               OpBranchConditional %16 %19 %18
         %18 = OpLabel
               OpBranch %6
         %19 = OpLabel
               OpBranch %17
         %17 = OpLabel
         %21 = OpIAdd %int %10 %int_1
               OpBranch %7
          %7 = OpLabel
         %12 = OpPhi %int %21 %17
               OpBranch %8
          %6 = OpLabel
               OpReturn
               OpFunctionEnd
