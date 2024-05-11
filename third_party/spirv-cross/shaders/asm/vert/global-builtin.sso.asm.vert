; SPIR-V
; Version: 1.0
; Generator: Khronos Glslang Reference Front End; 1
; Bound: 40
; Schema: 0
               OpCapability Shader
          %1 = OpExtInstImport "GLSL.std.450"
               OpMemoryModel Logical GLSL450
               OpEntryPoint Vertex %main "main" %_entryPointOutput %_entryPointOutput_pos
               OpSource HLSL 500
               OpName %main "main"
               OpName %VSOut "VSOut"
               OpMemberName %VSOut 0 "a"
               OpMemberName %VSOut 1 "pos"
               OpName %_main_ "@main("
               OpName %vout "vout"
               OpName %flattenTemp "flattenTemp"
               OpName %VSOut_0 "VSOut"
               OpMemberName %VSOut_0 0 "a"
               OpName %_entryPointOutput "@entryPointOutput"
               OpName %_entryPointOutput_pos "@entryPointOutput_pos"
               OpDecorate %_entryPointOutput Location 0
               OpDecorate %_entryPointOutput_pos BuiltIn Position
       %void = OpTypeVoid
          %3 = OpTypeFunction %void
      %float = OpTypeFloat 32
    %v4float = OpTypeVector %float 4
      %VSOut = OpTypeStruct %float %v4float
          %9 = OpTypeFunction %VSOut
%_ptr_Function_VSOut = OpTypePointer Function %VSOut
        %int = OpTypeInt 32 1
      %int_0 = OpConstant %int 0
   %float_40 = OpConstant %float 40
%_ptr_Function_float = OpTypePointer Function %float
      %int_1 = OpConstant %int 1
    %float_1 = OpConstant %float 1
         %21 = OpConstantComposite %v4float %float_1 %float_1 %float_1 %float_1
%_ptr_Function_v4float = OpTypePointer Function %v4float
    %VSOut_0 = OpTypeStruct %float
%_ptr_Output_VSOut_0 = OpTypePointer Output %VSOut_0
%_entryPointOutput = OpVariable %_ptr_Output_VSOut_0 Output
%_ptr_Output_float = OpTypePointer Output %float
%_ptr_Output_v4float = OpTypePointer Output %v4float
%_entryPointOutput_pos = OpVariable %_ptr_Output_v4float Output
       %main = OpFunction %void None %3
          %5 = OpLabel
%flattenTemp = OpVariable %_ptr_Function_VSOut Function
         %28 = OpFunctionCall %VSOut %_main_
               OpStore %flattenTemp %28
         %32 = OpAccessChain %_ptr_Function_float %flattenTemp %int_0
         %33 = OpLoad %float %32
         %35 = OpAccessChain %_ptr_Output_float %_entryPointOutput %int_0
               OpStore %35 %33
         %38 = OpAccessChain %_ptr_Function_v4float %flattenTemp %int_1
         %39 = OpLoad %v4float %38
               OpStore %_entryPointOutput_pos %39
               OpReturn
               OpFunctionEnd
     %_main_ = OpFunction %VSOut None %9
         %11 = OpLabel
       %vout = OpVariable %_ptr_Function_VSOut Function
         %18 = OpAccessChain %_ptr_Function_float %vout %int_0
               OpStore %18 %float_40
         %23 = OpAccessChain %_ptr_Function_v4float %vout %int_1
               OpStore %23 %21
         %24 = OpLoad %VSOut %vout
               OpReturnValue %24
               OpFunctionEnd
