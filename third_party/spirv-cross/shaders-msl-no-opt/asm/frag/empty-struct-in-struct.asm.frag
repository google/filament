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
               OpName %EmptyStruct2Test "EmptyStruct2Test"
               OpName %GetValue "GetValue"
               OpName %GetValue2 "GetValue"
               OpName %self "self"
               OpName %self2 "self"
               OpName %emptyStruct "emptyStruct"
               OpName %value "value"
               OpName %EntryPoint_Main "EntryPoint_Main"

%EmptyStructTest = OpTypeStruct
%EmptyStruct2Test = OpTypeStruct %EmptyStructTest
%_ptr_Function_EmptyStruct2Test = OpTypePointer Function %EmptyStruct2Test
      %float = OpTypeFloat 32
%_ptr_Function_float = OpTypePointer Function %float
          %5 = OpTypeFunction %float %_ptr_Function_EmptyStruct2Test
          %6 = OpTypeFunction %float %EmptyStruct2Test
       %void = OpTypeVoid
%_ptr_Function_void = OpTypePointer Function %void
          %8 = OpTypeFunction %void %_ptr_Function_EmptyStruct2Test
          %9 = OpTypeFunction %void
    %float_0 = OpConstant %float 0
	  %value4 = OpConstantNull %EmptyStruct2Test

   %GetValue = OpFunction %float None %5
       %self = OpFunctionParameter %_ptr_Function_EmptyStruct2Test
         %13 = OpLabel
               OpReturnValue %float_0
               OpFunctionEnd

   %GetValue2 = OpFunction %float None %6
       %self2 = OpFunctionParameter %EmptyStruct2Test
         %14 = OpLabel
               OpReturnValue %float_0
               OpFunctionEnd

%EntryPoint_Main = OpFunction %void None %9
         %37 = OpLabel
     %emptyStruct = OpVariable %_ptr_Function_EmptyStruct2Test Function
         %18 = OpVariable %_ptr_Function_EmptyStruct2Test Function
      %value = OpVariable %_ptr_Function_float Function
	  %value2 = OpCompositeConstruct %EmptyStructTest
	  %value3 = OpCompositeConstruct %EmptyStruct2Test %value2
         %22 = OpFunctionCall %float %GetValue %emptyStruct
         %23 = OpFunctionCall %float %GetValue2 %value3
         %24 = OpFunctionCall %float %GetValue2 %value4
               OpStore %value %22
               OpStore %value %23
               OpStore %value %24
               OpReturn
               OpFunctionEnd
