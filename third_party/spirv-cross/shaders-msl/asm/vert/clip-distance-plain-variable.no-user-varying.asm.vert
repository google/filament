; SPIR-V
; Version: 1.0
; Generator: Khronos Glslang Reference Front End; 8
; Bound: 56
; Schema: 0
               OpCapability Shader
               OpCapability ClipDistance
          %1 = OpExtInstImport "GLSL.std.450"
               OpMemoryModel Logical GLSL450
               OpEntryPoint Vertex %main "main" %pos_1 %_entryPointOutput_pos %_entryPointOutput_clip
               OpSource HLSL 500
               OpName %main "main"
               OpName %VSOut "VSOut"
               OpMemberName %VSOut 0 "pos"
               OpMemberName %VSOut 1 "clip"
               OpName %_main_vf4_ "@main(vf4;"
               OpName %pos "pos"
               OpName %vout "vout"
               OpName %pos_0 "pos"
               OpName %pos_1 "pos"
               OpName %flattenTemp "flattenTemp"
               OpName %param "param"
               OpName %_entryPointOutput_pos "@entryPointOutput.pos"
               OpName %_entryPointOutput_clip "@entryPointOutput.clip"
               OpDecorate %pos_1 Location 0
               OpDecorate %_entryPointOutput_pos BuiltIn Position
               OpDecorate %_entryPointOutput_clip BuiltIn ClipDistance
       %void = OpTypeVoid
          %3 = OpTypeFunction %void
      %float = OpTypeFloat 32
    %v4float = OpTypeVector %float 4
%_ptr_Function_v4float = OpTypePointer Function %v4float
    %v2float = OpTypeVector %float 2
      %VSOut = OpTypeStruct %v4float %v2float
         %11 = OpTypeFunction %VSOut %_ptr_Function_v4float
%_ptr_Function_VSOut = OpTypePointer Function %VSOut
        %int = OpTypeInt 32 1
      %int_0 = OpConstant %int 0
      %int_1 = OpConstant %int 1
%_ptr_Function_v2float = OpTypePointer Function %v2float
%_ptr_Input_v4float = OpTypePointer Input %v4float
      %pos_1 = OpVariable %_ptr_Input_v4float Input
%_ptr_Output_v4float = OpTypePointer Output %v4float
%_entryPointOutput_pos = OpVariable %_ptr_Output_v4float Output
       %uint = OpTypeInt 32 0
     %uint_2 = OpConstant %uint 2
%_arr_float_uint_2 = OpTypeArray %float %uint_2
%_ptr_Output__arr_float_uint_2 = OpTypePointer Output %_arr_float_uint_2
%_entryPointOutput_clip = OpVariable %_ptr_Output__arr_float_uint_2 Output
     %uint_0 = OpConstant %uint 0
%_ptr_Function_float = OpTypePointer Function %float
%_ptr_Output_float = OpTypePointer Output %float
     %uint_1 = OpConstant %uint 1
       %main = OpFunction %void None %3
          %5 = OpLabel
      %pos_0 = OpVariable %_ptr_Function_v4float Function
%flattenTemp = OpVariable %_ptr_Function_VSOut Function
      %param = OpVariable %_ptr_Function_v4float Function
         %32 = OpLoad %v4float %pos_1
               OpStore %pos_0 %32
         %35 = OpLoad %v4float %pos_0
               OpStore %param %35
         %36 = OpFunctionCall %VSOut %_main_vf4_ %param
               OpStore %flattenTemp %36
         %39 = OpAccessChain %_ptr_Function_v4float %flattenTemp %int_0
         %40 = OpLoad %v4float %39
               OpStore %_entryPointOutput_pos %40
         %48 = OpAccessChain %_ptr_Function_float %flattenTemp %int_1 %uint_0
         %49 = OpLoad %float %48
         %51 = OpAccessChain %_ptr_Output_float %_entryPointOutput_clip %int_0
               OpStore %51 %49
         %53 = OpAccessChain %_ptr_Function_float %flattenTemp %int_1 %uint_1
         %54 = OpLoad %float %53
         %55 = OpAccessChain %_ptr_Output_float %_entryPointOutput_clip %int_1
               OpStore %55 %54
               OpReturn
               OpFunctionEnd
 %_main_vf4_ = OpFunction %VSOut None %11
        %pos = OpFunctionParameter %_ptr_Function_v4float
         %14 = OpLabel
       %vout = OpVariable %_ptr_Function_VSOut Function
         %19 = OpLoad %v4float %pos
         %20 = OpAccessChain %_ptr_Function_v4float %vout %int_0
               OpStore %20 %19
         %22 = OpLoad %v4float %pos
         %23 = OpVectorShuffle %v2float %22 %22 0 1
         %25 = OpAccessChain %_ptr_Function_v2float %vout %int_1
               OpStore %25 %23
         %26 = OpLoad %VSOut %vout
               OpReturnValue %26
               OpFunctionEnd
