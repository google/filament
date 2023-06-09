; SPIR-V
; Version: 1.0
; Generator: Khronos Glslang Reference Front End; 1
; Bound: 50
; Schema: 0
               OpCapability Shader
          %1 = OpExtInstImport "GLSL.std.450"
               OpMemoryModel Logical GLSL450
               OpEntryPoint Fragment %fragmentProgram "main" %_entryPointOutput
               OpExecutionMode %fragmentProgram OriginUpperLeft
               OpSource HLSL 500
               OpName %fragmentProgram "fragmentProgram"
               OpName %_fragmentProgram_ "@fragmentProgram("
               OpName %uv "uv"
               OpName %_entryPointOutput "@entryPointOutput"
               OpDecorate %_entryPointOutput Location 0
       %void = OpTypeVoid
          %3 = OpTypeFunction %void
      %float = OpTypeFloat 32
    %v4float = OpTypeVector %float 4
          %8 = OpTypeFunction %v4float
    %v2float = OpTypeVector %float 2
%_ptr_Function_v2float = OpTypePointer Function %v2float
    %float_0 = OpConstant %float 0
         %15 = OpConstantComposite %v2float %float_0 %float_0
       %uint = OpTypeInt 32 0
     %uint_0 = OpConstant %uint 0
%_ptr_Function_float = OpTypePointer Function %float
       %bool = OpTypeBool
    %float_1 = OpConstant %float 1
         %26 = OpConstantComposite %v4float %float_1 %float_0 %float_0 %float_1
         %29 = OpConstantComposite %v4float %float_1 %float_1 %float_0 %float_1
%_ptr_Output_v4float = OpTypePointer Output %v4float
%_entryPointOutput = OpVariable %_ptr_Output_v4float Output
%_ptr_Function_v4float = OpTypePointer Function %v4float
      %false = OpConstantFalse %bool
%fragmentProgram = OpFunction %void None %3
          %5 = OpLabel
         %35 = OpVariable %_ptr_Function_v2float Function
         %37 = OpVariable %_ptr_Function_v4float Function
               OpBranch %38
         %38 = OpLabel
               OpLoopMerge %39 %40 None
               OpBranch %41
         %41 = OpLabel
               OpStore %35 %15
         %42 = OpAccessChain %_ptr_Function_float %35 %uint_0
         %43 = OpLoad %float %42
         %44 = OpFOrdNotEqual %bool %43 %float_0
               OpSelectionMerge %45 None
               OpBranchConditional %44 %46 %47
         %46 = OpLabel
               OpStore %37 %26
               OpBranch %39
         %47 = OpLabel
               OpStore %37 %29
               OpBranch %39
         %45 = OpLabel
         %48 = OpUndef %v4float
               OpStore %37 %48
               OpBranch %39
         %40 = OpLabel
               OpBranchConditional %false %38 %39
         %39 = OpLabel
         %34 = OpLoad %v4float %37
               OpStore %_entryPointOutput %34
               OpReturn
               OpFunctionEnd
%_fragmentProgram_ = OpFunction %v4float None %8
         %10 = OpLabel
         %uv = OpVariable %_ptr_Function_v2float Function
               OpStore %uv %15
         %19 = OpAccessChain %_ptr_Function_float %uv %uint_0
         %20 = OpLoad %float %19
         %22 = OpFOrdNotEqual %bool %20 %float_0
               OpSelectionMerge %24 None
               OpBranchConditional %22 %23 %28
         %23 = OpLabel
               OpReturnValue %26
         %28 = OpLabel
               OpReturnValue %29
         %24 = OpLabel
         %31 = OpUndef %v4float
               OpReturnValue %31
               OpFunctionEnd
