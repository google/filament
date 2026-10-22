; SPIR-V
; Version: 1.0
; Generator: Khronos Glslang Reference Front End; 6
; Bound: 36
; Schema: 0
               OpCapability Shader
               OpExtension "SPV_GOOGLE_decorate_string"
               OpExtension "SPV_GOOGLE_hlsl_functionality1"
          %1 = OpExtInstImport "GLSL.std.450"
               OpMemoryModel Logical GLSL450
               OpEntryPoint Vertex %main "main" %_entryPointOutput_p %_entryPointOutput_c
               OpSource HLSL 500
               OpName %main "main"
               OpName %VOut "VOut"
               OpMemberName %VOut 0 "p"
               OpMemberName %VOut 1 "c"
               OpName %_main_ "@main("
               OpName %v "v"
               OpName %flattenTemp "flattenTemp"
               OpName %_entryPointOutput_p "@entryPointOutput.p"
               OpName %_entryPointOutput_c "@entryPointOutput.c"
               OpMemberDecorateStringGOOGLE %VOut 0 HlslSemanticGOOGLE "SV_POSITION"
               OpMemberDecorateStringGOOGLE %VOut 1 HlslSemanticGOOGLE "COLOR"
               OpDecorate %_entryPointOutput_p BuiltIn Position
               OpDecorateStringGOOGLE %_entryPointOutput_p HlslSemanticGOOGLE "SV_POSITION"
               OpDecorate %_entryPointOutput_c Location 0
               OpDecorateStringGOOGLE %_entryPointOutput_c HlslSemanticGOOGLE "COLOR"
       %void = OpTypeVoid
          %3 = OpTypeFunction %void
      %float = OpTypeFloat 32
    %v4float = OpTypeVector %float 4
       %VOut = OpTypeStruct %v4float %v4float
          %9 = OpTypeFunction %VOut
%_ptr_Function_VOut = OpTypePointer Function %VOut
        %int = OpTypeInt 32 1
      %int_0 = OpConstant %int 0
    %float_1 = OpConstant %float 1
         %17 = OpConstantComposite %v4float %float_1 %float_1 %float_1 %float_1
%_ptr_Function_v4float = OpTypePointer Function %v4float
      %int_1 = OpConstant %int 1
    %float_2 = OpConstant %float 2
         %22 = OpConstantComposite %v4float %float_2 %float_2 %float_2 %float_2
%_ptr_Output_v4float = OpTypePointer Output %v4float
%_entryPointOutput_p = OpVariable %_ptr_Output_v4float Output
%_entryPointOutput_c = OpVariable %_ptr_Output_v4float Output
       %main = OpFunction %void None %3
          %5 = OpLabel
%flattenTemp = OpVariable %_ptr_Function_VOut Function
         %28 = OpFunctionCall %VOut %_main_
               OpStore %flattenTemp %28
         %31 = OpAccessChain %_ptr_Function_v4float %flattenTemp %int_0
         %32 = OpLoad %v4float %31
               OpStore %_entryPointOutput_p %32
         %34 = OpAccessChain %_ptr_Function_v4float %flattenTemp %int_1
         %35 = OpLoad %v4float %34
               OpStore %_entryPointOutput_c %35
               OpReturn
               OpFunctionEnd
     %_main_ = OpFunction %VOut None %9
         %11 = OpLabel
          %v = OpVariable %_ptr_Function_VOut Function
         %19 = OpAccessChain %_ptr_Function_v4float %v %int_0
               OpStore %19 %17
         %23 = OpAccessChain %_ptr_Function_v4float %v %int_1
               OpStore %23 %22
         %24 = OpLoad %VOut %v
               OpReturnValue %24
               OpFunctionEnd
