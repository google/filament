; SPIR-V
; Version: 1.0
; Generator: Khronos Glslang Reference Front End; 7
; Bound: 79
; Schema: 0
               OpCapability Tessellation
          %1 = OpExtInstImport "GLSL.std.450"
               OpMemoryModel Logical GLSL450
               OpEntryPoint TessellationEvaluation %main "main" %input_foo %input_bar %uv_1 %CPData_1 %_entryPointOutput_pos
               OpExecutionMode %main Quads
               OpSource HLSL 500
               OpName %main "main"
               OpName %HS_INPUT "HS_INPUT"
               OpMemberName %HS_INPUT 0 "foo"
               OpMemberName %HS_INPUT 1 "bar"
               OpName %ControlPoint "ControlPoint"
               OpMemberName %ControlPoint 0 "baz"
               OpName %DS_OUTPUT "DS_OUTPUT"
               OpMemberName %DS_OUTPUT 0 "pos"
               OpName %_main_struct_HS_INPUT_vf4_vf41_vf2_struct_ControlPoint_vf41_4__ "@main(struct-HS_INPUT-vf4-vf41;vf2;struct-ControlPoint-vf41[4];"
               OpName %input "input"
               OpName %uv "uv"
               OpName %CPData "CPData"
               OpName %o "o"
               OpName %input_0 "input"
               OpName %input_foo "input.foo"
               OpName %input_bar "input.bar"
               OpName %uv_0 "uv"
               OpName %uv_1 "uv"
               OpName %CPData_0 "CPData"
               OpName %CPData_1 "CPData"
               OpName %_entryPointOutput_pos "@entryPointOutput.pos"
               OpName %param "param"
               OpName %param_0 "param"
               OpName %param_1 "param"
               OpDecorate %input_foo Patch
               OpDecorate %input_foo Location 0
               OpDecorate %input_bar Patch
               OpDecorate %input_bar Location 1
               OpDecorate %uv_1 Patch
               OpDecorate %uv_1 BuiltIn TessCoord
               OpDecorate %CPData_1 Location 2
               OpDecorate %_entryPointOutput_pos BuiltIn Position
       %void = OpTypeVoid
          %3 = OpTypeFunction %void
      %float = OpTypeFloat 32
    %v4float = OpTypeVector %float 4
   %HS_INPUT = OpTypeStruct %v4float %v4float
%_ptr_Function_HS_INPUT = OpTypePointer Function %HS_INPUT
    %v2float = OpTypeVector %float 2
%_ptr_Function_v2float = OpTypePointer Function %v2float
%ControlPoint = OpTypeStruct %v4float
       %uint = OpTypeInt 32 0
     %uint_4 = OpConstant %uint 4
%_arr_ControlPoint_uint_4 = OpTypeArray %ControlPoint %uint_4
%_ptr_Function__arr_ControlPoint_uint_4 = OpTypePointer Function %_arr_ControlPoint_uint_4
  %DS_OUTPUT = OpTypeStruct %v4float
         %18 = OpTypeFunction %DS_OUTPUT %_ptr_Function_HS_INPUT %_ptr_Function_v2float %_ptr_Function__arr_ControlPoint_uint_4
%_ptr_Function_DS_OUTPUT = OpTypePointer Function %DS_OUTPUT
        %int = OpTypeInt 32 1
      %int_0 = OpConstant %int 0
%_ptr_Function_v4float = OpTypePointer Function %v4float
      %int_1 = OpConstant %int 1
      %int_3 = OpConstant %int 3
%_ptr_Input_v4float = OpTypePointer Input %v4float
  %input_foo = OpVariable %_ptr_Input_v4float Input
  %input_bar = OpVariable %_ptr_Input_v4float Input
    %v3float = OpTypeVector %float 3
%_ptr_Input_v3float = OpTypePointer Input %v3float
       %uv_1 = OpVariable %_ptr_Input_v3float Input
%_ptr_Input__arr_ControlPoint_uint_4 = OpTypePointer Input %_arr_ControlPoint_uint_4
   %CPData_1 = OpVariable %_ptr_Input__arr_ControlPoint_uint_4 Input
%_ptr_Output_v4float = OpTypePointer Output %v4float
%_entryPointOutput_pos = OpVariable %_ptr_Output_v4float Output
       %main = OpFunction %void None %3
          %5 = OpLabel
    %input_0 = OpVariable %_ptr_Function_HS_INPUT Function
       %uv_0 = OpVariable %_ptr_Function_v2float Function
   %CPData_0 = OpVariable %_ptr_Function__arr_ControlPoint_uint_4 Function
      %param = OpVariable %_ptr_Function_HS_INPUT Function
    %param_0 = OpVariable %_ptr_Function_v2float Function
    %param_1 = OpVariable %_ptr_Function__arr_ControlPoint_uint_4 Function
         %52 = OpLoad %v4float %input_foo
         %53 = OpAccessChain %_ptr_Function_v4float %input_0 %int_0
               OpStore %53 %52
         %55 = OpLoad %v4float %input_bar
         %56 = OpAccessChain %_ptr_Function_v4float %input_0 %int_1
               OpStore %56 %55
         %61 = OpLoad %v3float %uv_1
         %62 = OpCompositeExtract %float %61 0
         %63 = OpCompositeExtract %float %61 1
         %64 = OpCompositeConstruct %v2float %62 %63
               OpStore %uv_0 %64
         %68 = OpLoad %_arr_ControlPoint_uint_4 %CPData_1
               OpStore %CPData_0 %68
         %72 = OpLoad %HS_INPUT %input_0
               OpStore %param %72
         %74 = OpLoad %v2float %uv_0
               OpStore %param_0 %74
         %76 = OpLoad %_arr_ControlPoint_uint_4 %CPData_0
               OpStore %param_1 %76
         %77 = OpFunctionCall %DS_OUTPUT %_main_struct_HS_INPUT_vf4_vf41_vf2_struct_ControlPoint_vf41_4__ %param %param_0 %param_1
         %78 = OpCompositeExtract %v4float %77 0
               OpStore %_entryPointOutput_pos %78
               OpReturn
               OpFunctionEnd
%_main_struct_HS_INPUT_vf4_vf41_vf2_struct_ControlPoint_vf41_4__ = OpFunction %DS_OUTPUT None %18
      %input = OpFunctionParameter %_ptr_Function_HS_INPUT
         %uv = OpFunctionParameter %_ptr_Function_v2float
     %CPData = OpFunctionParameter %_ptr_Function__arr_ControlPoint_uint_4
         %23 = OpLabel
          %o = OpVariable %_ptr_Function_DS_OUTPUT Function
         %29 = OpAccessChain %_ptr_Function_v4float %input %int_0
         %30 = OpLoad %v4float %29
         %32 = OpAccessChain %_ptr_Function_v4float %input %int_1
         %33 = OpLoad %v4float %32
         %34 = OpFAdd %v4float %30 %33
         %35 = OpLoad %v2float %uv
         %36 = OpVectorShuffle %v4float %35 %35 0 1 0 1
         %37 = OpFAdd %v4float %34 %36
         %38 = OpAccessChain %_ptr_Function_v4float %CPData %int_0 %int_0
         %39 = OpLoad %v4float %38
         %40 = OpFAdd %v4float %37 %39
         %42 = OpAccessChain %_ptr_Function_v4float %CPData %int_3 %int_0
         %43 = OpLoad %v4float %42
         %44 = OpFAdd %v4float %40 %43
         %45 = OpAccessChain %_ptr_Function_v4float %o %int_0
               OpStore %45 %44
         %46 = OpLoad %DS_OUTPUT %o
               OpReturnValue %46
               OpFunctionEnd
