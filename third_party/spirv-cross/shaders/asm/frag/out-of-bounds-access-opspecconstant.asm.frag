; SPIR-V
; Version: 1.0
; Generator: Google Shaderc over Glslang; 10
; Bound: 21
; Schema: 0
               OpCapability Shader
          %1 = OpExtInstImport "GLSL.std.450"
               OpMemoryModel Logical GLSL450
               OpEntryPoint Fragment %main "main"
               OpExecutionMode %main OriginUpperLeft
               OpSource ESSL 320
               OpSourceExtension "GL_GOOGLE_cpp_style_line_directive"
               OpSourceExtension "GL_GOOGLE_include_directive"
               OpName %main "main"
               OpName %v "v"
               OpDecorate %v RelaxedPrecision
       %void = OpTypeVoid
          %3 = OpTypeFunction %void
      %float = OpTypeFloat 32
    %v3float = OpTypeVector %float 3
%_ptr_Function_v3float = OpTypePointer Function %v3float
    %float_0 = OpConstant %float 0
         %11 = OpConstantComposite %v3float %float_0 %float_0 %float_0
       %bool = OpTypeBool
      %false = OpConstantFalse %bool
   %float_99 = OpConstant %float 99
       %uint = OpTypeInt 32 0
%uint_spec_3 = OpSpecConstant %uint 3
%_ptr_Function_float = OpTypePointer Function %float
       %main = OpFunction %void None %3
          %5 = OpLabel
          %v = OpVariable %_ptr_Function_v3float Function
               OpStore %v %11
               OpSelectionMerge %15 None
               OpBranchConditional %false %14 %15
         %14 = OpLabel
         %20 = OpAccessChain %_ptr_Function_float %v %uint_spec_3
               OpStore %20 %float_99
               OpBranch %15
         %15 = OpLabel
               OpReturn
               OpFunctionEnd
