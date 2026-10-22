; SPIR-V
; Option A test: same SpecId used for bool and uint
OpCapability Shader
OpMemoryModel Logical GLSL450
OpEntryPoint Fragment %main "main" %outColor
OpExecutionMode %main OriginUpperLeft

OpDecorate %outColor Location 0
OpDecorate %specBool SpecId 0
OpDecorate %specUint SpecId 0

%void = OpTypeVoid
%fn = OpTypeFunction %void
%float = OpTypeFloat 32
%v4float = OpTypeVector %float 4
%uint = OpTypeInt 32 0
%bool = OpTypeBool

%float_0 = OpConstant %float 0
%float_1 = OpConstant %float 1

%specBool = OpSpecConstantTrue %bool
%specUint = OpSpecConstant %uint 0

%ptr_Output_v4float = OpTypePointer Output %v4float
%outColor = OpVariable %ptr_Output_v4float Output

%main = OpFunction %void None %fn
%entry = OpLabel
%u_to_f = OpConvertUToF %float %specUint
%sel = OpSelect %float %specBool %float_1 %float_0
%sum = OpFAdd %float %u_to_f %sel
%vec = OpCompositeConstruct %v4float %sum %sum %sum %sum
OpStore %outColor %vec
OpReturn
OpFunctionEnd


