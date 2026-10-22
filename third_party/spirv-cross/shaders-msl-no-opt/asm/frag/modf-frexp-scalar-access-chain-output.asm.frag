; SPIR-V
; Version: 1.0
; Generator: Khronos Glslang Reference Front End; 7
; Bound: 17
; Schema: 0
               OpCapability Shader
          %1 = OpExtInstImport "GLSL.std.450"
               OpMemoryModel Logical GLSL450
               OpEntryPoint Fragment %main "main"
               OpExecutionMode %main OriginUpperLeft
               OpSource ESSL 310
               OpName %main "main"
               OpName %col "col"
       %void = OpTypeVoid
          %3 = OpTypeFunction %void
      %float = OpTypeFloat 32
%_ptr_Function_float = OpTypePointer Function %float
%float_0_150000006 = OpConstant %float 0.150000006
    %v3float = OpTypeVector %float 3
%_ptr_Function_v3float = OpTypePointer Function %v3float
       %int = OpTypeInt 32 1
     %int_0 = OpConstant %int 0
     %int_1 = OpConstant %int 1
	 %v2int = OpTypeVector %int 2
%_ptr_Function_v2int = OpTypePointer Function %v2int
%_ptr_Function_int = OpTypePointer Function %int
       %main = OpFunction %void None %3
          %5 = OpLabel
        %col = OpVariable %_ptr_Function_v3float Function
        %icol = OpVariable %_ptr_Function_v2int Function
         %ptr_x = OpAccessChain %_ptr_Function_float %col %int_0
         %ptr_y = OpAccessChain %_ptr_Function_int %icol %int_1
         %16 = OpExtInst %float %1 Modf %float_0_150000006 %ptr_x
         %17 = OpExtInst %float %1 Frexp %float_0_150000006 %ptr_y
               OpReturn
               OpFunctionEnd
