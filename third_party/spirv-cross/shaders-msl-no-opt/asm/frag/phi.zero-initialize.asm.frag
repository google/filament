; SPIR-V
; Version: 1.0
; Generator: Khronos Glslang Reference Front End; 8
; Bound: 40
; Schema: 0
               OpCapability Shader
          %1 = OpExtInstImport "GLSL.std.450"
               OpMemoryModel Logical GLSL450
               OpEntryPoint Fragment %main "main" %vColor %FragColor
               OpExecutionMode %main OriginUpperLeft
               OpSource GLSL 450
               OpName %main "main"
               OpName %vColor "vColor"
               OpName %uninit_function_int "uninit_function_int"
               OpName %FragColor "FragColor"
               OpName %uninit_int "uninit_int"
               OpName %uninit_vector "uninit_vector"
               OpName %uninit_matrix "uninit_matrix"
               OpName %Foo "Foo"
               OpMemberName %Foo 0 "a"
               OpName %uninit_foo "uninit_foo"
               OpDecorate %vColor Location 0
               OpDecorate %FragColor Location 0
       %void = OpTypeVoid
          %3 = OpTypeFunction %void
      %float = OpTypeFloat 32
    %v4float = OpTypeVector %float 4
%_ptr_Input_v4float = OpTypePointer Input %v4float
     %vColor = OpVariable %_ptr_Input_v4float Input
       %uint = OpTypeInt 32 0
     %uint_0 = OpConstant %uint 0
%_ptr_Input_float = OpTypePointer Input %float
   %float_10 = OpConstant %float 10
       %bool = OpTypeBool
        %int = OpTypeInt 32 1
%_ptr_Function_int = OpTypePointer Function %int
     %int_10 = OpConstant %int 10
     %int_20 = OpConstant %int 20
%_ptr_Output_v4float = OpTypePointer Output %v4float
  %FragColor = OpVariable %_ptr_Output_v4float Output
%_ptr_Private_int = OpTypePointer Private %int
 %uninit_int = OpUndef %int
      %v4int = OpTypeVector %int 4
%_ptr_Private_v4int = OpTypePointer Private %v4int
%uninit_vector = OpUndef %v4int
%mat4v4float = OpTypeMatrix %v4float 4
%_ptr_Private_mat4v4float = OpTypePointer Private %mat4v4float
%uninit_matrix = OpUndef %mat4v4float
        %Foo = OpTypeStruct %int
%_ptr_Private_Foo = OpTypePointer Private %Foo
 %uninit_foo = OpUndef %Foo
       %main = OpFunction %void None %3
          %5 = OpLabel
%uninit_function_int = OpVariable %_ptr_Function_int Function
         %13 = OpAccessChain %_ptr_Input_float %vColor %uint_0
         %14 = OpLoad %float %13
         %17 = OpFOrdGreaterThan %bool %14 %float_10
               OpSelectionMerge %19 None
               OpBranchConditional %17 %18 %24
         %18 = OpLabel
               OpBranch %19
         %24 = OpLabel
               OpBranch %19
         %19 = OpLabel
		 %27 = OpPhi %int %int_10 %18 %int_20 %24
         %28 = OpLoad %v4float %vColor
               OpStore %FragColor %28
               OpReturn
               OpFunctionEnd
