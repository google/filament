; SPIR-V
; Version: 1.0
; Generator: Khronos SPIR-V Tools Assembler; 0
; Bound: 68
; Schema: 0
               OpCapability Shader
          %1 = OpExtInstImport "GLSL.std.450"
               OpMemoryModel Logical GLSL450
               OpEntryPoint Fragment %main "main" %gl_FragCoord %_GLF_color
               OpExecutionMode %main OriginUpperLeft
               OpSource ESSL 310
               OpName %main "main"
               OpName %gl_FragCoord "gl_FragCoord"
               OpName %_GLF_color "_GLF_color"
               OpName %m "m"
               OpDecorate %gl_FragCoord BuiltIn FragCoord
               OpDecorate %_GLF_color Location 0
       %void = OpTypeVoid
          %7 = OpTypeFunction %void
      %float = OpTypeFloat 32
    %v4float = OpTypeVector %float 4
%_ptr_Input_v4float = OpTypePointer Input %v4float
%gl_FragCoord = OpVariable %_ptr_Input_v4float Input
       %uint = OpTypeInt 32 0
     %uint_0 = OpConstant %uint 0
%_ptr_Input_float = OpTypePointer Input %float
   %float_10 = OpConstant %float 10
       %bool = OpTypeBool
%_ptr_Output_v4float = OpTypePointer Output %v4float
 %_GLF_color = OpVariable %_ptr_Output_v4float Output
    %float_1 = OpConstant %float 1
    %float_0 = OpConstant %float 0
         %19 = OpConstantComposite %v4float %float_1 %float_0 %float_0 %float_1
%mat4v4float = OpTypeMatrix %v4float 4
         %21 = OpConstantComposite %v4float %float_1 %float_1 %float_1 %float_1
         %22 = OpConstantComposite %mat4v4float %21 %21 %21 %21
     %uint_4 = OpConstant %uint 4
%_arr_mat4v4float_uint_4 = OpTypeArray %mat4v4float %uint_4
%_ptr_Function__arr_mat4v4float_uint_4 = OpTypePointer Function %_arr_mat4v4float_uint_4
        %int = OpTypeInt 32 1
      %int_0 = OpConstant %int 0
      %int_4 = OpConstant %int 4
    %v2float = OpTypeVector %float 2
         %30 = OpConstantComposite %v2float %float_1 %float_1
      %int_1 = OpConstant %int 1
     %uint_3 = OpConstant %uint 3
%_ptr_Function_float = OpTypePointer Function %float
         %34 = OpConstantComposite %_arr_mat4v4float_uint_4 %22 %22 %22 %22
       %main = OpFunction %void None %7
         %35 = OpLabel
          %m = OpVariable %_ptr_Function__arr_mat4v4float_uint_4 Function
               OpBranch %36
         %36 = OpLabel
               OpLoopMerge %37 %38 None
               OpBranch %39
         %39 = OpLabel
         %40 = OpAccessChain %_ptr_Input_float %gl_FragCoord %uint_0
         %41 = OpLoad %float %40
         %42 = OpFOrdLessThan %bool %41 %float_10
               OpSelectionMerge %43 None
               OpBranchConditional %42 %44 %43
         %44 = OpLabel
               OpStore %_GLF_color %19
               OpBranch %37
         %43 = OpLabel
               OpStore %m %34
               OpBranch %45
         %45 = OpLabel
         %46 = OpPhi %int %int_0 %43 %47 %48
         %49 = OpSLessThan %bool %46 %int_4
               OpLoopMerge %50 %48 None
               OpBranchConditional %49 %51 %50
         %51 = OpLabel
               OpBranch %52
         %52 = OpLabel
         %53 = OpPhi %int %int_0 %51 %54 %55
         %56 = OpSLessThan %bool %53 %int_4
               OpLoopMerge %57 %55 None
               OpBranchConditional %56 %58 %57
         %58 = OpLabel
         %59 = OpSelect %int %56 %int_1 %int_0
         %60 = OpAccessChain %_ptr_Function_float %m %59 %46 %uint_3
         %61 = OpLoad %float %60
         %62 = OpCompositeConstruct %v2float %61 %61
         %63 = OpFDiv %v2float %30 %62
         %64 = OpExtInst %float %1 Distance %30 %63
         %65 = OpFOrdLessThan %bool %64 %float_1
               OpSelectionMerge %66 None
               OpBranchConditional %65 %67 %55
         %67 = OpLabel
               OpStore %_GLF_color %21
               OpBranch %55
         %66 = OpLabel
               OpBranch %55
         %55 = OpLabel
         %54 = OpIAdd %int %53 %int_1
               OpBranch %52
         %57 = OpLabel
               OpBranch %48
         %48 = OpLabel
         %47 = OpIAdd %int %46 %int_1
               OpBranch %45
         %50 = OpLabel
               OpBranch %37
         %38 = OpLabel
               OpBranch %36
         %37 = OpLabel
               OpReturn
               OpFunctionEnd
