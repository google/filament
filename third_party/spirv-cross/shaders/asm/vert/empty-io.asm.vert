; SPIR-V
; Version: 1.0
; Generator: Khronos Glslang Reference Front End; 1
; Bound: 40
; Schema: 0
               OpCapability Shader
          %1 = OpExtInstImport "GLSL.std.450"
               OpMemoryModel Logical GLSL450
               OpEntryPoint Vertex %main "main" %position %_entryPointOutput_position %_entryPointOutput
               OpName %main "main"
               OpName %VSInput "VSInput"
               OpMemberName %VSInput 0 "position"
               OpName %VSOutput "VSOutput"
               OpMemberName %VSOutput 0 "position"
               OpName %_main_struct_VSInput_vf41_ "@main(struct-VSInput-vf41;"
               OpName %_input "_input"
               OpName %_out "_out"
               OpName %_input_0 "_input"
               OpName %position "position"
               OpName %_entryPointOutput_position "@entryPointOutput_position"
               OpName %param "param"
               OpName %VSOutput_0 "VSOutput"
               OpName %_entryPointOutput "@entryPointOutput"
               OpDecorate %position Location 0
               OpDecorate %_entryPointOutput_position BuiltIn Position
               OpDecorate %_entryPointOutput Location 0
       %void = OpTypeVoid
          %3 = OpTypeFunction %void
      %float = OpTypeFloat 32
    %v4float = OpTypeVector %float 4
    %VSInput = OpTypeStruct %v4float
%_ptr_Function_VSInput = OpTypePointer Function %VSInput
   %VSOutput = OpTypeStruct %v4float
         %11 = OpTypeFunction %VSOutput %_ptr_Function_VSInput
%_ptr_Function_VSOutput = OpTypePointer Function %VSOutput
        %int = OpTypeInt 32 1
         %18 = OpConstant %int 0
%_ptr_Function_v4float = OpTypePointer Function %v4float
%_ptr_Input_v4float = OpTypePointer Input %v4float
   %position = OpVariable %_ptr_Input_v4float Input
%_ptr_Output_v4float = OpTypePointer Output %v4float
%_entryPointOutput_position = OpVariable %_ptr_Output_v4float Output
 %VSOutput_0 = OpTypeStruct
%_ptr_Output_VSOutput_0 = OpTypePointer Output %VSOutput_0
%_entryPointOutput = OpVariable %_ptr_Output_VSOutput_0 Output
       %main = OpFunction %void None %3
          %5 = OpLabel
   %_input_0 = OpVariable %_ptr_Function_VSInput Function
      %param = OpVariable %_ptr_Function_VSInput Function
         %29 = OpLoad %v4float %position
         %30 = OpAccessChain %_ptr_Function_v4float %_input_0 %18
               OpStore %30 %29
         %34 = OpLoad %VSInput %_input_0
               OpStore %param %34
         %35 = OpFunctionCall %VSOutput %_main_struct_VSInput_vf41_ %param
         %36 = OpCompositeExtract %v4float %35 0
               OpStore %_entryPointOutput_position %36
               OpReturn
               OpFunctionEnd
%_main_struct_VSInput_vf41_ = OpFunction %VSOutput None %11
     %_input = OpFunctionParameter %_ptr_Function_VSInput
         %14 = OpLabel
       %_out = OpVariable %_ptr_Function_VSOutput Function
         %20 = OpAccessChain %_ptr_Function_v4float %_input %18
         %21 = OpLoad %v4float %20
         %22 = OpAccessChain %_ptr_Function_v4float %_out %18
               OpStore %22 %21
         %23 = OpLoad %VSOutput %_out
               OpReturnValue %23
               OpFunctionEnd
