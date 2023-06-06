; SPIR-V
; Version: 1.2
; Generator: Khronos; 0
; Bound: 51
; Schema: 0
               OpCapability Shader
               OpMemoryModel Logical GLSL450
               OpEntryPoint Fragment %EntryPoint_Main "main"
               OpExecutionMode %EntryPoint_Main OriginUpperLeft
               OpSource Unknown 100
               OpName %mat3 "mat3"
               OpName %constituent "constituent"
               OpName %constituent_0 "constituent"
               OpName %constituent_1 "constituent"
               OpName %constituent_2 "constituent"
               OpName %constituent_3 "constituent"
               OpName %constituent_4 "constituent"
               OpName %constituent_5 "constituent"
               OpName %constituent_6 "constituent"
               OpName %EntryPoint_Main "EntryPoint_Main"
       %void = OpTypeVoid
%_ptr_Function_void = OpTypePointer Function %void
      %float = OpTypeFloat 32
        %int = OpTypeInt 32 1
    %v3float = OpTypeVector %float 3
%mat3v3float = OpTypeMatrix %v3float 3
%_ptr_Function_mat3v3float = OpTypePointer Function %mat3v3float
         %14 = OpTypeFunction %void
      %int_0 = OpConstant %int 0
      %int_1 = OpConstant %int 1
%EntryPoint_Main = OpFunction %void None %14
         %45 = OpLabel
     %mat3 = OpVariable %_ptr_Function_mat3v3float Function
%constituent = OpConvertSToF %float %int_0
%constituent_0 = OpCompositeConstruct %v3float %constituent %constituent %constituent
%constituent_1 = OpCompositeConstruct %v3float %constituent %constituent %constituent
%constituent_2 = OpCompositeConstruct %v3float %constituent %constituent %constituent
         %25 = OpCompositeConstruct %mat3v3float %constituent_0 %constituent_1 %constituent_2
               OpStore %mat3 %25
%constituent_3 = OpConvertSToF %float %int_1
%constituent_4 = OpCompositeConstruct %v3float %constituent_3 %constituent_3 %constituent_3
%constituent_5 = OpCompositeConstruct %v3float %constituent_3 %constituent_3 %constituent_3
%constituent_6 = OpCompositeConstruct %v3float %constituent_3 %constituent_3 %constituent_3
         %30 = OpCompositeConstruct %mat3v3float %constituent_4 %constituent_5 %constituent_6
               OpStore %mat3 %30
               OpReturn
               OpFunctionEnd
