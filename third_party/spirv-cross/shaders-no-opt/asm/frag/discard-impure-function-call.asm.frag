; SPIR-V
; Version: 1.0
; Generator: Khronos Glslang Reference Front End; 7
; Bound: 34
; Schema: 0
               OpCapability Shader
          %1 = OpExtInstImport "GLSL.std.450"
               OpMemoryModel Logical GLSL450
               OpEntryPoint Fragment %main "main" %vA %FragColor
               OpExecutionMode %main OriginUpperLeft
               OpSource GLSL 450
               OpName %main "main"
               OpName %foobar_i1_ "foobar(i1;"
               OpName %a "a"
               OpName %a_0 "a"
               OpName %vA "vA"
               OpName %param "param"
               OpName %FragColor "FragColor"
               OpDecorate %vA Flat
               OpDecorate %vA Location 0
               OpDecorate %FragColor Location 0
       %void = OpTypeVoid
          %3 = OpTypeFunction %void
        %int = OpTypeInt 32 1
%_ptr_Function_int = OpTypePointer Function %int
      %float = OpTypeFloat 32
    %v4float = OpTypeVector %float 4
         %10 = OpTypeFunction %v4float %_ptr_Function_int
      %int_0 = OpConstant %int 0
       %bool = OpTypeBool
   %float_10 = OpConstant %float 10
         %22 = OpConstantComposite %v4float %float_10 %float_10 %float_10 %float_10
%_ptr_Function_v4float = OpTypePointer Function %v4float
%_ptr_Input_int = OpTypePointer Input %int
         %vA = OpVariable %_ptr_Input_int Input
%_ptr_Output_v4float = OpTypePointer Output %v4float
  %FragColor = OpVariable %_ptr_Output_v4float Output
       %main = OpFunction %void None %3
          %5 = OpLabel
        %a_0 = OpVariable %_ptr_Function_v4float Function
      %param = OpVariable %_ptr_Function_int Function
         %30 = OpLoad %int %vA
               OpStore %param %30
         %31 = OpFunctionCall %v4float %foobar_i1_ %param
               OpStore %FragColor %22
               OpReturn
               OpFunctionEnd
 %foobar_i1_ = OpFunction %v4float None %10
          %a = OpFunctionParameter %_ptr_Function_int
         %13 = OpLabel
         %14 = OpLoad %int %a
         %17 = OpSLessThan %bool %14 %int_0
               OpSelectionMerge %19 None
               OpBranchConditional %17 %18 %19
         %18 = OpLabel
               OpKill
         %19 = OpLabel
               OpReturnValue %22
               OpFunctionEnd
