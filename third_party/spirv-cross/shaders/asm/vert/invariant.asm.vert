; SPIR-V
; Version: 1.0
; Generator: Khronos Glslang Reference Front End; 3
; Bound: 18
; Schema: 0
               OpCapability Shader
          %1 = OpExtInstImport "GLSL.std.450"
               OpMemoryModel Logical GLSL450
               OpEntryPoint Vertex %main "main" %_entryPointOutput
               OpSource HLSL 500
               OpName %main "main"
               OpName %_main_ "@main("
               OpName %_entryPointOutput "@entryPointOutput"
               OpDecorate %_entryPointOutput Invariant
               OpDecorate %_entryPointOutput BuiltIn Position
       %void = OpTypeVoid
          %3 = OpTypeFunction %void
      %float = OpTypeFloat 32
    %v4float = OpTypeVector %float 4
          %8 = OpTypeFunction %v4float
    %float_1 = OpConstant %float 1
         %12 = OpConstantComposite %v4float %float_1 %float_1 %float_1 %float_1
%_ptr_Output_v4float = OpTypePointer Output %v4float
%_entryPointOutput = OpVariable %_ptr_Output_v4float Output
       %main = OpFunction %void None %3
          %5 = OpLabel
         %17 = OpFunctionCall %v4float %_main_
               OpStore %_entryPointOutput %17
               OpReturn
               OpFunctionEnd
     %_main_ = OpFunction %v4float None %8
         %10 = OpLabel
               OpReturnValue %12
               OpFunctionEnd
