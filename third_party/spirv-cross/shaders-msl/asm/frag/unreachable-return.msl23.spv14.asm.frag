; SPIR-V
; Version: 1.5
; Generator: Khronos Glslang Reference Front End; 11
; Bound: 46
; Schema: 0
               OpCapability Shader
               OpCapability DemoteToHelperInvocation
               OpExtension "SPV_EXT_demote_to_helper_invocation"
          %1 = OpExtInstImport "GLSL.std.450"
               OpMemoryModel Logical GLSL450
               OpEntryPoint Fragment %main "main" %gl_FragCoord %frag_clr %buff
               OpExecutionMode %main OriginUpperLeft
               OpSource GLSL 450
               OpName %main "main"
               OpName %frag_coord "frag_coord"
               OpName %gl_FragCoord "gl_FragCoord"
               OpName %buff_idx "buff_idx"
               OpName %frag_clr "frag_clr"
               OpName %buff_t "buff_t"
               OpMemberName %buff_t 0 "m0"
               OpName %buff "buff"
               OpDecorate %gl_FragCoord BuiltIn FragCoord
               OpDecorate %frag_clr Location 0
               OpDecorate %_arr_int_uint_1024 ArrayStride 4
               OpDecorate %buff_t Block
               OpMemberDecorate %buff_t 0 Offset 0
               OpDecorate %buff Binding 0
               OpDecorate %buff DescriptorSet 0
       %void = OpTypeVoid
          %3 = OpTypeFunction %void
        %int = OpTypeInt 32 1
      %v2int = OpTypeVector %int 2
%_ptr_Function_v2int = OpTypePointer Function %v2int
      %float = OpTypeFloat 32
    %v4float = OpTypeVector %float 4
%_ptr_Input_v4float = OpTypePointer Input %v4float
%gl_FragCoord = OpVariable %_ptr_Input_v4float Input
      %v4int = OpTypeVector %int 4
%_ptr_Function_int = OpTypePointer Function %int
       %uint = OpTypeInt 32 0
     %uint_1 = OpConstant %uint 1
     %int_32 = OpConstant %int 32
     %uint_0 = OpConstant %uint 0
%_ptr_Output_v4float = OpTypePointer Output %v4float
   %frag_clr = OpVariable %_ptr_Output_v4float Output
    %float_0 = OpConstant %float 0
    %float_1 = OpConstant %float 1
         %34 = OpConstantComposite %v4float %float_0 %float_0 %float_1 %float_1
  %uint_1024 = OpConstant %uint 1024
%_arr_int_uint_1024 = OpTypeArray %int %uint_1024
     %buff_t = OpTypeStruct %_arr_int_uint_1024
%_ptr_StorageBuffer_buff_t = OpTypePointer StorageBuffer %buff_t
       %buff = OpVariable %_ptr_StorageBuffer_buff_t StorageBuffer
      %int_0 = OpConstant %int 0
      %int_1 = OpConstant %int 1
%_ptr_StorageBuffer_int = OpTypePointer StorageBuffer %int
       %main = OpFunction %void None %3
          %5 = OpLabel
 %frag_coord = OpVariable %_ptr_Function_v2int Function
   %buff_idx = OpVariable %_ptr_Function_int Function
         %14 = OpLoad %v4float %gl_FragCoord
         %16 = OpConvertFToS %v4int %14
         %17 = OpVectorShuffle %v2int %16 %16 0 1
               OpStore %frag_coord %17
         %22 = OpAccessChain %_ptr_Function_int %frag_coord %uint_1
         %23 = OpLoad %int %22
         %25 = OpIMul %int %23 %int_32
         %27 = OpAccessChain %_ptr_Function_int %frag_coord %uint_0
         %28 = OpLoad %int %27
         %29 = OpIAdd %int %25 %28
               OpStore %buff_idx %29
               OpStore %frag_clr %34
         %41 = OpLoad %int %buff_idx
         %44 = OpAccessChain %_ptr_StorageBuffer_int %buff %int_0 %41
               OpStore %44 %int_1
               OpDemoteToHelperInvocation
               OpUnreachable
               OpFunctionEnd
