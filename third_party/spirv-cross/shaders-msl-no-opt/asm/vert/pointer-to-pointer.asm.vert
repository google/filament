OpCapability Shader
OpCapability VariablePointers
OpCapability VariablePointersStorageBuffer
OpMemoryModel Logical GLSL450

OpEntryPoint Vertex %fn_vert "main"

%F = OpTypeFloat 32
%PF = OpTypePointer StorageBuffer %F
%PPF = OpTypePointer Private %PF
%PPPF = OpTypePointer Function %PPF

%V = OpTypeVoid
%Fn0V = OpTypeFunction %V

%FnArg = OpTypeFunction %V %PPPF

%uPPF = OpUndef %PPF

%fn_ptr = OpFunction %V None %FnArg
	%arg = OpFunctionParameter %PPPF
	%fn_ptr_bb0 = OpLabel
	OpReturn
OpFunctionEnd

%fn_vert = OpFunction %V None %Fn0V
	%fn_vert_bb0 = OpLabel
	%VPPPF = OpVariable %PPPF Function
	OpStore %VPPPF %uPPF
	%VV = OpFunctionCall %V %fn_ptr %VPPPF
	OpReturn
OpFunctionEnd


