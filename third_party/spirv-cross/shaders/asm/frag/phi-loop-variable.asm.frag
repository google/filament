; SPIR-V
; Version: 1.0
; Generator: Khronos Glslang Reference Front End; 1
; Bound: 59
; Schema: 0
               OpCapability Shader
          %1 = OpExtInstImport "GLSL.std.450"
               OpMemoryModel Logical GLSL450
               OpEntryPoint Fragment %4 "main"
               OpExecutionMode %4 OriginUpperLeft
       %void = OpTypeVoid
          %3 = OpTypeFunction %void
      %float = OpTypeFloat 32
    %v2float = OpTypeVector %float 2
%mat2v2float = OpTypeMatrix %v2float 2
%_ptr_Function_mat2v2float = OpTypePointer Function %mat2v2float
    %v3float = OpTypeVector %float 3
         %11 = OpTypeFunction %v3float %_ptr_Function_mat2v2float
%_ptr_Function_v3float = OpTypePointer Function %v3float
    %float_1 = OpConstant %float 1
         %18 = OpConstantComposite %v3float %float_1 %float_1 %float_1
        %int = OpTypeInt 32 1
%_ptr_Function_int = OpTypePointer Function %int
     %int_35 = OpConstant %int 35
      %int_0 = OpConstant %int 0
       %bool = OpTypeBool
      %int_1 = OpConstant %int 1
          %4 = OpFunction %void None %3
          %5 = OpLabel
               OpBranch %48
         %48 = OpLabel
         %58 = OpPhi %int %int_35 %5 %56 %50
               OpLoopMerge %49 %50 None
               OpBranch %51
         %51 = OpLabel
         %53 = OpSGreaterThanEqual %bool %58 %int_0
               OpBranchConditional %53 %54 %49
         %54 = OpLabel
               OpBranch %50
         %50 = OpLabel
         %56 = OpISub %int %58 %int_1
               OpBranch %48
         %49 = OpLabel
               OpReturn
               OpFunctionEnd
         %13 = OpFunction %v3float None %11
         %12 = OpFunctionParameter %_ptr_Function_mat2v2float
         %14 = OpLabel
         %16 = OpVariable %_ptr_Function_v3float Function
         %21 = OpVariable %_ptr_Function_int Function
               OpStore %16 %18
               OpStore %21 %int_35
               OpBranch %23
         %23 = OpLabel
               OpLoopMerge %25 %26 None
               OpBranch %27
         %27 = OpLabel
         %28 = OpLoad %int %21
         %31 = OpSGreaterThanEqual %bool %28 %int_0
               OpBranchConditional %31 %24 %25
         %24 = OpLabel
               OpBranch %26
         %26 = OpLabel
         %32 = OpLoad %int %21
         %34 = OpISub %int %32 %int_1
               OpStore %21 %34
               OpBranch %23
         %25 = OpLabel
         %35 = OpLoad %v3float %16
               OpReturnValue %35
               OpFunctionEnd
