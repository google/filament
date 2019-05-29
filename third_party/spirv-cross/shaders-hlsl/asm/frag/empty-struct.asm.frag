; SPIR-V
; Version: 1.2
; Generator: Khronos; 0
; Bound: 43
; Schema: 0
               OpCapability Shader
               OpMemoryModel Logical GLSL450
               OpEntryPoint Fragment %EntryPoint_Main "main"
               OpExecutionMode %EntryPoint_Main OriginUpperLeft
               OpSource Unknown 100
               OpName %EmptyStructTest "EmptyStructTest"
               OpName %GetValue "GetValue"
               OpName %GetValue2 "GetValue"
               OpName %self "self"
               OpName %self2 "self"
               OpName %emptyStruct "emptyStruct"
               OpName %value "value"
               OpName %EntryPoint_Main "EntryPoint_Main"

%EmptyStructTest = OpTypeStruct
%_ptr_Function_EmptyStructTest = OpTypePointer Function %EmptyStructTest
      %float = OpTypeFloat 32
%_ptr_Function_float = OpTypePointer Function %float
          %5 = OpTypeFunction %float %_ptr_Function_EmptyStructTest
          %6 = OpTypeFunction %float %EmptyStructTest
       %void = OpTypeVoid
%_ptr_Function_void = OpTypePointer Function %void
          %8 = OpTypeFunction %void %_ptr_Function_EmptyStructTest
          %9 = OpTypeFunction %void
    %float_0 = OpConstant %float 0

   %GetValue = OpFunction %float None %5
       %self = OpFunctionParameter %_ptr_Function_EmptyStructTest
         %13 = OpLabel
               OpReturnValue %float_0
               OpFunctionEnd

   %GetValue2 = OpFunction %float None %6
       %self2 = OpFunctionParameter %EmptyStructTest
         %14 = OpLabel
               OpReturnValue %float_0
               OpFunctionEnd

%EntryPoint_Main = OpFunction %void None %9
         %37 = OpLabel
     %emptyStruct = OpVariable %_ptr_Function_EmptyStructTest Function
         %18 = OpVariable %_ptr_Function_EmptyStructTest Function
      %value = OpVariable %_ptr_Function_float Function
	  %value2 = OpCompositeConstruct %EmptyStructTest
         %22 = OpFunctionCall %float %GetValue %emptyStruct
         %23 = OpFunctionCall %float %GetValue2 %value2
               OpStore %value %22
               OpStore %value %23
               OpReturn
               OpFunctionEnd
