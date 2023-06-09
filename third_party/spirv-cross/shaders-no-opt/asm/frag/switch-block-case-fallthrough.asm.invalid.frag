; SPIR-V
; Version: 1.0
; Generator: Khronos Glslang Reference Front End; 7
; Bound: 29
; Schema: 0
               OpCapability Shader
          %1 = OpExtInstImport "GLSL.std.450"
               OpMemoryModel Logical GLSL450
               OpEntryPoint Fragment %main "main" %vIndex %FragColor
               OpExecutionMode %main OriginUpperLeft
               OpSource GLSL 450
               OpName %main "main"
               OpName %vIndex "vIndex"
               OpName %FragColor "FragColor"
               OpName %i "i"
               OpName %j "j"
               OpDecorate %vIndex Flat
               OpDecorate %vIndex Location 0
               OpDecorate %FragColor Location 0
       %void = OpTypeVoid
          %3 = OpTypeFunction %void
        %int = OpTypeInt 32 1
       %bool = OpTypeBool
 %int_0 = OpConstant %int 0
 %int_1 = OpConstant %int 1
 %int_2 = OpConstant %int 2
 %int_3 = OpConstant %int 3
%_ptr_Input_int = OpTypePointer Input %int
     %vIndex = OpVariable %_ptr_Input_int Input
      %float = OpTypeFloat 32
    %v4float = OpTypeVector %float 4
%_ptr_Output_v4float = OpTypePointer Output %v4float
  %FragColor = OpVariable %_ptr_Output_v4float Output
%_ptr_Function_int = OpTypePointer Function %int
       %main = OpFunction %void None %3
          %header = OpLabel
          %i = OpVariable %_ptr_Function_int Function
          %j = OpVariable %_ptr_Function_int Function
          %9 = OpLoad %int %vIndex
               OpSelectionMerge %switch_merge None
               OpSwitch %9 %default_case 100 %default_case 0 %case_0 1 %case_1 11 %case_1 2 %case_2 3 %case_3 4 %case_4 5 %case_5

         %case_0 = OpLabel
               OpBranch %default_case

         %default_case = OpLabel
               %default_case_phi = OpPhi %int %int_2 %header %int_3 %case_0
               ; Test what happens when a case block dominates access to a variable.
               OpStore %j %default_case_phi
               OpBranch %case_1

         %case_1 = OpLabel
               ; Test phi nodes between case labels.
               %case_1_phi = OpPhi %int %int_0 %default_case %int_1 %header
               OpStore %j %case_1_phi
               OpBranch %case_2

         %case_2 = OpLabel
               OpBranch %switch_merge

         %case_3 = OpLabel
            ; Conditionally branch to another case block. This is really dumb, but it is apparently legal.
            %case_3_cond = OpSGreaterThan %bool %9 %int_3
            OpBranchConditional %case_3_cond %case_4 %switch_merge

         %case_4 = OpLabel
            ; When emitted from case 3, we should *not* see fallthrough behavior.
            OpBranch %case_5

         %case_5 = OpLabel
            OpStore %i %int_0
            OpBranch %switch_merge

         %switch_merge = OpLabel
         %26 = OpLoad %int %i
         %27 = OpConvertSToF %float %26
         %28 = OpCompositeConstruct %v4float %27 %27 %27 %27
               OpStore %FragColor %28
               OpReturn
               OpFunctionEnd
