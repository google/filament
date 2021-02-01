; SPIR-V
; Version: 1.3
; Generator: Khronos Glslang Reference Front End; 7
; Bound: 19
; Schema: 0
               OpCapability Shader
               OpCapability DemoteToHelperInvocationEXT
               OpExtension "SPV_EXT_demote_to_helper_invocation"
          %1 = OpExtInstImport "GLSL.std.450"
               OpMemoryModel Logical GLSL450
               OpEntryPoint Fragment %main "main" %FragColor
               OpExecutionMode %main OriginUpperLeft
               OpSource GLSL 450
               OpSourceExtension "GL_EXT_demote_to_helper_invocation"
               OpName %main "main"
               OpName %FragColor "FragColor"
               OpDecorate %FragColor Location 0
       %void = OpTypeVoid
          %3 = OpTypeFunction %void
       %bool = OpTypeBool
%_ptr_Function_bool = OpTypePointer Function %bool
      %float = OpTypeFloat 32
    %v4float = OpTypeVector %float 4
%_ptr_Output_v4float = OpTypePointer Output %v4float
  %FragColor = OpVariable %_ptr_Output_v4float Output
    %float_1 = OpConstant %float 1
    %float_0 = OpConstant %float 0
         %19 = OpConstantComposite %v4float %float_1 %float_0 %float_0 %float_1
       %main = OpFunction %void None %3
          %5 = OpLabel
          %9 = OpIsHelperInvocationEXT %bool
               OpDemoteToHelperInvocationEXT
         %10 = OpLogicalNot %bool %9
               OpSelectionMerge %12 None
               OpBranchConditional %10 %11 %12
         %11 = OpLabel
               OpStore %FragColor %19
               OpBranch %12
         %12 = OpLabel
               OpReturn
               OpFunctionEnd
