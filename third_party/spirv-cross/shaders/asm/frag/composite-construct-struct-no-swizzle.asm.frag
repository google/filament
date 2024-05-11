; SPIR-V
; Version: 1.0
; Generator: Khronos Glslang Reference Front End; 3
; Bound: 39
; Schema: 0
               OpCapability Shader
          %1 = OpExtInstImport "GLSL.std.450"
               OpMemoryModel Logical GLSL450
               OpEntryPoint Fragment %main "main" %foo %FooOut
               OpExecutionMode %main OriginUpperLeft
               OpSource ESSL 310
               OpName %main "main"
               OpName %foo "foo"
               OpName %SwizzleTest "SwizzleTest"
               OpMemberName %SwizzleTest 0 "a"
               OpMemberName %SwizzleTest 1 "b"
               OpName %FooOut "FooOut"
               OpDecorate %foo RelaxedPrecision
               OpDecorate %foo Location 0
               OpDecorate %12 RelaxedPrecision
               OpMemberDecorate %SwizzleTest 0 RelaxedPrecision
               OpMemberDecorate %SwizzleTest 1 RelaxedPrecision
               OpDecorate %FooOut RelaxedPrecision
               OpDecorate %FooOut Location 0
               OpDecorate %34 RelaxedPrecision
       %void = OpTypeVoid
          %3 = OpTypeFunction %void
      %float = OpTypeFloat 32
    %v2float = OpTypeVector %float 2
%_ptr_Function_v2float = OpTypePointer Function %v2float
%_ptr_Input_v2float = OpTypePointer Input %v2float
        %foo = OpVariable %_ptr_Input_v2float Input
%SwizzleTest = OpTypeStruct %float %float
%_ptr_Function_SwizzleTest = OpTypePointer Function %SwizzleTest
       %uint = OpTypeInt 32 0
%_ptr_Function_float = OpTypePointer Function %float
%_ptr_Output_float = OpTypePointer Output %float
     %FooOut = OpVariable %_ptr_Output_float Output
        %int = OpTypeInt 32 1
       %main = OpFunction %void None %3
          %5 = OpLabel
         %12 = OpLoad %v2float %foo
         %36 = OpCompositeExtract %float %12 0
         %38 = OpCompositeExtract %float %12 1
		 %test0 = OpCompositeConstruct %SwizzleTest %36 %38
		 %new0 = OpCompositeExtract %float %test0 0
		 %new1 = OpCompositeExtract %float %test0 1
         %34 = OpFAdd %float %new0 %new1
               OpStore %FooOut %34
               OpReturn
               OpFunctionEnd
