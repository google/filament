               OpCapability Shader
          %1 = OpExtInstImport "GLSL.std.450"
               OpMemoryModel Logical GLSL450
               OpEntryPoint Fragment %main "main" %FragColor
               OpExecutionMode %main OriginUpperLeft
               OpSource GLSL 450
               OpName %main "main"
               OpName %FragColor "FragColor"
               OpDecorate %FragColor Location 0
       %void = OpTypeVoid
          %3 = OpTypeFunction %void
       %bool = OpTypeBool
      %false = OpConstantFalse %bool
	  %uint = OpTypeInt 32 0
	  %uint_1 = OpConstant %uint 1
	  %uint_2 = OpConstant %uint 2
      %true = OpConstantTrue %bool
      %float = OpTypeFloat 32
    %v4float = OpTypeVector %float 4
%_ptr_Output_v4float = OpTypePointer Output %v4float
		 %s = OpTypeStruct %float
		 %arr = OpTypeArray %float %uint_2
%_ptr_Function_s = OpTypePointer Function %s
%_ptr_Function_arr = OpTypePointer Function %arr
  %FragColor = OpVariable %_ptr_Output_v4float Output
    %float_0 = OpConstant %float 0
    %float_1 = OpConstant %float 1
         %17 = OpConstantComposite %v4float %float_1 %float_1 %float_0 %float_1
         %18 = OpConstantComposite %v4float %float_0 %float_0 %float_0 %float_1
         %19 = OpConstantComposite %v4float %float_1 %float_1 %float_1 %float_1
         %20 = OpConstantComposite %v4float %float_0 %float_0 %float_0 %float_0
		 %s0 = OpConstantComposite %s %float_0
		 %s1 = OpConstantComposite %s %float_1
     %v4bool = OpTypeVector %bool 4
	 	%b4	= OpConstantComposite %v4bool %false %true %false %true
		%arr1 = OpConstantComposite %arr %float_0 %float_1
		%arr2 = OpConstantComposite %arr %float_1 %float_0
       %main = OpFunction %void None %3
          %5 = OpLabel
		  %ss = OpVariable %_ptr_Function_s Function
		  %arrvar = OpVariable %_ptr_Function_arr Function
		  ; Not trivial
         %21 = OpSelect %v4float %false %17 %18
               OpStore %FragColor %21
		  ; Trivial
         %22 = OpSelect %v4float %false %19 %20
               OpStore %FragColor %22
			; Vector not trivial
         %23 = OpSelect %v4float %b4 %17 %18
               OpStore %FragColor %23
			; Vector trivial
         %24 = OpSelect %v4float %b4 %19 %20
               OpStore %FragColor %24
		  ; Struct selection
         %sout = OpSelect %s %false %s0 %s1
               OpStore %ss %sout
		; Array selection
         %arrout = OpSelect %arr %true %arr1 %arr2
               OpStore %arrvar %arrout

               OpReturn
               OpFunctionEnd
