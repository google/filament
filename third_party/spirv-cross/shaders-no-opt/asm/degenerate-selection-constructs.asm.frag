; SPIR-V
; Version: 1.0
; Generator: Khronos SPIR-V Tools Assembler; 0
; Bound: 816
; Schema: 0
               OpCapability Shader
          %1 = OpExtInstImport "GLSL.std.450"
               OpMemoryModel Logical GLSL450
               OpEntryPoint Fragment %main "main" %gl_FragCoord %_GLF_color
               OpExecutionMode %main OriginUpperLeft
               OpSource ESSL 320
               OpName %main "main"
               OpName %checkSwap_f1_f1_ "checkSwap(f1;f1;"
               OpName %a "a"
               OpName %b "b"
               OpName %gl_FragCoord "gl_FragCoord"
               OpName %buf1 "buf1"
               OpMemberName %buf1 0 "resolution"
               OpName %_ ""
               OpName %i "i"
               OpName %data "data"
               OpName %buf0 "buf0"
               OpMemberName %buf0 0 "injectionSwitch"
               OpName %__0 ""
               OpName %i_0 "i"
               OpName %j "j"
               OpName %doSwap "doSwap"
               OpName %param "param"
               OpName %param_0 "param"
               OpName %temp "temp"
               OpName %_GLF_color "_GLF_color"
               OpDecorate %gl_FragCoord BuiltIn FragCoord
               OpMemberDecorate %buf1 0 Offset 0
               OpDecorate %buf1 Block
               OpDecorate %_ DescriptorSet 0
               OpDecorate %_ Binding 1
               OpMemberDecorate %buf0 0 Offset 0
               OpDecorate %buf0 Block
               OpDecorate %__0 DescriptorSet 0
               OpDecorate %__0 Binding 0
               OpDecorate %_GLF_color Location 0
       %void = OpTypeVoid
          %3 = OpTypeFunction %void
      %float = OpTypeFloat 32
%_ptr_Function_float = OpTypePointer Function %float
       %bool = OpTypeBool
          %9 = OpTypeFunction %bool %_ptr_Function_float %_ptr_Function_float
    %v4float = OpTypeVector %float 4
%_ptr_Input_v4float = OpTypePointer Input %v4float
%gl_FragCoord = OpVariable %_ptr_Input_v4float Input
       %uint = OpTypeInt 32 0
     %uint_1 = OpConstant %uint 1
%_ptr_Input_float = OpTypePointer Input %float
    %v2float = OpTypeVector %float 2
       %buf1 = OpTypeStruct %v2float
%_ptr_Uniform_buf1 = OpTypePointer Uniform %buf1
          %_ = OpVariable %_ptr_Uniform_buf1 Uniform
        %int = OpTypeInt 32 1
      %int_0 = OpConstant %int 0
%_ptr_Uniform_float = OpTypePointer Uniform %float
    %float_2 = OpConstant %float 2
%_ptr_Function_bool = OpTypePointer Function %bool
%_ptr_Function_int = OpTypePointer Function %int
     %int_10 = OpConstant %int 10
    %uint_10 = OpConstant %uint 10
%_arr_float_uint_10 = OpTypeArray %float %uint_10
%_ptr_Function__arr_float_uint_10 = OpTypePointer Function %_arr_float_uint_10
       %buf0 = OpTypeStruct %v2float
%_ptr_Uniform_buf0 = OpTypePointer Uniform %buf0
        %__0 = OpVariable %_ptr_Uniform_buf0 Uniform
      %int_1 = OpConstant %int 1
      %int_9 = OpConstant %int 9
     %uint_0 = OpConstant %uint 0
%_ptr_Output_v4float = OpTypePointer Output %v4float
 %_GLF_color = OpVariable %_ptr_Output_v4float Output
   %float_10 = OpConstant %float 10
      %int_5 = OpConstant %int 5
    %float_1 = OpConstant %float 1
    %float_0 = OpConstant %float 0
      %false = OpConstantFalse %bool
       %true = OpConstantTrue %bool
       %main = OpFunction %void None %3
          %5 = OpLabel
          %i = OpVariable %_ptr_Function_int Function
       %data = OpVariable %_ptr_Function__arr_float_uint_10 Function
        %i_0 = OpVariable %_ptr_Function_int Function
          %j = OpVariable %_ptr_Function_int Function
     %doSwap = OpVariable %_ptr_Function_bool Function
      %param = OpVariable %_ptr_Function_float Function
    %param_0 = OpVariable %_ptr_Function_float Function
       %temp = OpVariable %_ptr_Function_float Function
               OpStore %i %int_0
               OpBranch %50
         %50 = OpLabel
               OpLoopMerge %52 %53 None
               OpBranch %54
         %54 = OpLabel
         %55 = OpLoad %int %i
         %57 = OpSLessThan %bool %55 %int_10
               OpBranchConditional %57 %51 %52
         %51 = OpLabel
         %62 = OpLoad %int %i
         %63 = OpLoad %int %i
         %64 = OpISub %int %int_10 %63
         %65 = OpConvertSToF %float %64
         %69 = OpAccessChain %_ptr_Uniform_float %__0 %int_0 %uint_1
         %70 = OpLoad %float %69
         %71 = OpFMul %float %65 %70
         %72 = OpAccessChain %_ptr_Function_float %data %62
               OpStore %72 %71
               OpBranch %53
         %53 = OpLabel
         %73 = OpLoad %int %i
         %75 = OpIAdd %int %73 %int_1
               OpStore %i %75
               OpBranch %50
         %52 = OpLabel
               OpStore %i_0 %int_0
               OpBranch %77
         %77 = OpLabel
               OpLoopMerge %79 %80 None
               OpBranch %81
         %81 = OpLabel
         %82 = OpLoad %int %i_0
         %84 = OpSLessThan %bool %82 %int_9
               OpBranchConditional %84 %78 %79
         %78 = OpLabel
               OpStore %j %int_0
               OpBranch %86
         %86 = OpLabel
               OpLoopMerge %88 %89 None
               OpBranch %90
         %90 = OpLabel
         %91 = OpLoad %int %j
         %92 = OpSLessThan %bool %91 %int_10
               OpBranchConditional %92 %87 %88
         %87 = OpLabel
         %93 = OpLoad %int %j
         %94 = OpLoad %int %i_0
         %95 = OpIAdd %int %94 %int_1
         %96 = OpSLessThan %bool %93 %95
               OpSelectionMerge %98 None
               OpBranchConditional %96 %97 %98
         %97 = OpLabel
               OpBranch %89
         %98 = OpLabel
        %101 = OpLoad %int %i_0
        %102 = OpLoad %int %j
        %104 = OpAccessChain %_ptr_Function_float %data %101
        %105 = OpLoad %float %104
               OpStore %param %105
        %107 = OpAccessChain %_ptr_Function_float %data %102
        %108 = OpLoad %float %107
               OpStore %param_0 %108
        %109 = OpFunctionCall %bool %checkSwap_f1_f1_ %param %param_0
               OpStore %doSwap %109
        %110 = OpLoad %bool %doSwap
               OpSelectionMerge %112 None
               OpBranchConditional %110 %111 %112
        %111 = OpLabel
        %114 = OpLoad %int %i_0
        %115 = OpAccessChain %_ptr_Function_float %data %114
        %116 = OpLoad %float %115
               OpStore %temp %116
        %117 = OpLoad %int %i_0
        %118 = OpLoad %int %j
        %119 = OpAccessChain %_ptr_Function_float %data %118
        %120 = OpLoad %float %119
        %121 = OpAccessChain %_ptr_Function_float %data %117
               OpStore %121 %120
        %122 = OpLoad %int %j
        %123 = OpLoad %float %temp
        %124 = OpAccessChain %_ptr_Function_float %data %122
               OpStore %124 %123
               OpBranch %112
        %112 = OpLabel
               OpBranch %89
         %89 = OpLabel
        %125 = OpLoad %int %j
        %126 = OpIAdd %int %125 %int_1
               OpStore %j %126
               OpBranch %86
         %88 = OpLabel
               OpBranch %80
         %80 = OpLabel
        %127 = OpLoad %int %i_0
        %128 = OpIAdd %int %127 %int_1
               OpStore %i_0 %128
               OpBranch %77
         %79 = OpLabel
        %130 = OpAccessChain %_ptr_Input_float %gl_FragCoord %uint_0
        %131 = OpLoad %float %130
        %132 = OpAccessChain %_ptr_Uniform_float %_ %int_0 %uint_0
        %133 = OpLoad %float %132
        %134 = OpFDiv %float %133 %float_2
        %135 = OpFOrdLessThan %bool %131 %134
               OpSelectionMerge %137 None
               OpBranchConditional %135 %136 %153
        %136 = OpLabel
        %140 = OpAccessChain %_ptr_Function_float %data %int_0
        %141 = OpLoad %float %140
        %143 = OpFDiv %float %141 %float_10
        %145 = OpAccessChain %_ptr_Function_float %data %int_5
        %146 = OpLoad %float %145
        %147 = OpFDiv %float %146 %float_10
        %148 = OpAccessChain %_ptr_Function_float %data %int_9
        %149 = OpLoad %float %148
        %150 = OpFDiv %float %149 %float_10
        %152 = OpCompositeConstruct %v4float %143 %147 %150 %float_1
               OpStore %_GLF_color %152
               OpBranch %137
        %153 = OpLabel
        %154 = OpAccessChain %_ptr_Function_float %data %int_5
        %155 = OpLoad %float %154
        %156 = OpFDiv %float %155 %float_10
        %157 = OpAccessChain %_ptr_Function_float %data %int_9
        %158 = OpLoad %float %157
        %159 = OpFDiv %float %158 %float_10
        %160 = OpAccessChain %_ptr_Function_float %data %int_0
        %161 = OpLoad %float %160
        %162 = OpFDiv %float %161 %float_10
        %163 = OpCompositeConstruct %v4float %156 %159 %162 %float_1
               OpStore %_GLF_color %163
               OpBranch %137
        %137 = OpLabel
               OpReturn
               OpFunctionEnd
%checkSwap_f1_f1_ = OpFunction %bool None %9
          %a = OpFunctionParameter %_ptr_Function_float
          %b = OpFunctionParameter %_ptr_Function_float
         %13 = OpLabel
         %35 = OpVariable %_ptr_Function_bool Function
         %20 = OpAccessChain %_ptr_Input_float %gl_FragCoord %uint_1
         %21 = OpLoad %float %20
         %29 = OpAccessChain %_ptr_Uniform_float %_ %int_0 %uint_1
         %30 = OpLoad %float %29
         %32 = OpFDiv %float %30 %float_2
         %33 = OpFOrdLessThan %bool %21 %32
               OpBranch %36
         %36 = OpLabel
               OpSelectionMerge %351 None
               OpBranchConditional %33 %352 %354
        %352 = OpLabel
        %353 = OpLoad %float %a
               OpBranch %351
        %354 = OpLabel
        %355 = OpCopyObject %float %float_0
               OpBranch %351
        %351 = OpLabel
         %38 = OpPhi %float %353 %352 %355 %354
               OpSelectionMerge %386 None
               OpBranchConditional %false %385 %385
        %385 = OpLabel
               OpSelectionMerge %356 None
               OpBranchConditional %33 %357 %359
        %357 = OpLabel
        %358 = OpLoad %float %b
               OpBranch %356
        %359 = OpLabel
        %360 = OpCopyObject %float %float_0
               OpBranch %356
        %356 = OpLabel
         %39 = OpPhi %float %358 %357 %360 %359
         %40 = OpFOrdGreaterThan %bool %38 %39
               OpBranch %362
        %362 = OpLabel
               OpSelectionMerge %479 None
               OpBranchConditional %33 %480 %479
        %480 = OpLabel
               OpStore %35 %40
               OpBranch %479
        %479 = OpLabel
               OpBranchConditional %true %361 %386
        %361 = OpLabel
               OpBranch %386
        %386 = OpLabel
               OpBranch %41
         %41 = OpLabel
               OpSelectionMerge %363 None
               OpBranchConditional %33 %366 %364
        %364 = OpLabel
        %365 = OpLoad %float %a
               OpBranch %363
        %366 = OpLabel
        %367 = OpCopyObject %float %float_0
               OpBranch %363
        %363 = OpLabel
         %42 = OpPhi %float %365 %364 %367 %366
               OpSelectionMerge %368 None
               OpBranchConditional %33 %371 %369
        %369 = OpLabel
        %370 = OpLoad %float %b
               OpBranch %368
        %371 = OpLabel
        %372 = OpCopyObject %float %float_0
               OpBranch %368
        %368 = OpLabel
         %43 = OpPhi %float %370 %369 %372 %371
         %44 = OpFOrdLessThan %bool %42 %43
               OpSelectionMerge %373 None
               OpBranchConditional %33 %373 %374
        %374 = OpLabel
               OpStore %35 %44
               OpBranch %373
        %373 = OpLabel
               OpBranch %37
         %37 = OpLabel
         %45 = OpLoad %bool %35
               OpReturnValue %45
               OpFunctionEnd
