; SPIR-V
; Version: 1.0
; Generator: Khronos Glslang Reference Front End; 2
; Bound: 131
; Schema: 0
               OpCapability Shader
          %1 = OpExtInstImport "GLSL.std.450"
               OpMemoryModel Logical GLSL450
               OpEntryPoint Fragment %main "main" %fragWorld_1 %_entryPointOutput
               OpExecutionMode %main OriginUpperLeft
               OpSource HLSL 500
               OpName %main "main"
               OpName %GetClip2TexMatrix_ "GetClip2TexMatrix("
               OpName %GetCascade_vf3_ "GetCascade(vf3;"
               OpName %fragWorldPosition "fragWorldPosition"
               OpName %_main_vf3_ "@main(vf3;"
               OpName %fragWorld "fragWorld"
               OpName %Foo "Foo"
               OpMemberName %Foo 0 "lightVP"
               OpMemberName %Foo 1 "shadowCascadesNum"
               OpMemberName %Foo 2 "test"
               OpName %_ ""
               OpName %cascadeIndex "cascadeIndex"
               OpName %worldToShadowMap "worldToShadowMap"
               OpName %fragShadowMapPos "fragShadowMapPos"
               OpName %param "param"
               OpName %fragWorld_0 "fragWorld"
               OpName %fragWorld_1 "fragWorld"
               OpName %_entryPointOutput "@entryPointOutput"
               OpName %param_0 "param"
               OpDecorate %_arr_mat4v4float_uint_64 ArrayStride 64
               OpMemberDecorate %Foo 0 RowMajor
               OpMemberDecorate %Foo 0 Offset 0
               OpMemberDecorate %Foo 0 MatrixStride 16
               OpMemberDecorate %Foo 1 Offset 4096
               OpMemberDecorate %Foo 2 Offset 4100
               OpDecorate %Foo Block
               OpDecorate %_ DescriptorSet 0
               OpDecorate %_ Binding 0
               OpDecorate %fragWorld_1 Location 0
               OpDecorate %_entryPointOutput Location 0
       %void = OpTypeVoid
          %3 = OpTypeFunction %void
      %float = OpTypeFloat 32
    %v4float = OpTypeVector %float 4
%mat4v4float = OpTypeMatrix %v4float 4
          %9 = OpTypeFunction %mat4v4float
    %v3float = OpTypeVector %float 3
%_ptr_Function_v3float = OpTypePointer Function %v3float
        %int = OpTypeInt 32 1
         %15 = OpTypeFunction %int %_ptr_Function_v3float
       %uint = OpTypeInt 32 0
    %uint_64 = OpConstant %uint 64
%_arr_mat4v4float_uint_64 = OpTypeArray %mat4v4float %uint_64
        %Foo = OpTypeStruct %_arr_mat4v4float_uint_64 %uint %int
%_ptr_Uniform_Foo = OpTypePointer Uniform %Foo
          %_ = OpVariable %_ptr_Uniform_Foo Uniform
      %int_2 = OpConstant %int 2
%_ptr_Uniform_int = OpTypePointer Uniform %int
      %int_0 = OpConstant %int 0
       %bool = OpTypeBool
  %float_0_5 = OpConstant %float 0.5
    %float_0 = OpConstant %float 0
         %39 = OpConstantComposite %v4float %float_0_5 %float_0 %float_0 %float_0
         %40 = OpConstantComposite %v4float %float_0 %float_0_5 %float_0 %float_0
         %41 = OpConstantComposite %v4float %float_0 %float_0 %float_0_5 %float_0
    %float_1 = OpConstant %float 1
         %43 = OpConstantComposite %v4float %float_0 %float_0 %float_0 %float_1
         %44 = OpConstantComposite %mat4v4float %39 %40 %41 %43
         %46 = OpConstantComposite %v4float %float_1 %float_0 %float_0 %float_0
         %47 = OpConstantComposite %v4float %float_0 %float_1 %float_0 %float_0
         %48 = OpConstantComposite %v4float %float_0 %float_0 %float_1 %float_0
         %49 = OpConstantComposite %mat4v4float %46 %47 %48 %43
%_ptr_Function_uint = OpTypePointer Function %uint
     %uint_0 = OpConstant %uint 0
      %int_1 = OpConstant %int 1
%_ptr_Uniform_uint = OpTypePointer Uniform %uint
%_ptr_Function_mat4v4float = OpTypePointer Function %mat4v4float
%_ptr_Uniform_mat4v4float = OpTypePointer Uniform %mat4v4float
%_ptr_Function_v4float = OpTypePointer Function %v4float
     %uint_2 = OpConstant %uint 2
%_ptr_Function_float = OpTypePointer Function %float
     %uint_1 = OpConstant %uint 1
     %int_n1 = OpConstant %int -1
%_ptr_Input_v3float = OpTypePointer Input %v3float
%fragWorld_1 = OpVariable %_ptr_Input_v3float Input
%_ptr_Output_int = OpTypePointer Output %int
%_entryPointOutput = OpVariable %_ptr_Output_int Output
       %main = OpFunction %void None %3
          %5 = OpLabel
%fragWorld_0 = OpVariable %_ptr_Function_v3float Function
    %param_0 = OpVariable %_ptr_Function_v3float Function
        %125 = OpLoad %v3float %fragWorld_1
               OpStore %fragWorld_0 %125
        %129 = OpLoad %v3float %fragWorld_0
               OpStore %param_0 %129
        %130 = OpFunctionCall %int %_main_vf3_ %param_0
               OpStore %_entryPointOutput %130
               OpReturn
               OpFunctionEnd
%GetClip2TexMatrix_ = OpFunction %mat4v4float None %9
         %11 = OpLabel
         %30 = OpAccessChain %_ptr_Uniform_int %_ %int_2
         %31 = OpLoad %int %30
         %34 = OpIEqual %bool %31 %int_0
               OpSelectionMerge %36 None
               OpBranchConditional %34 %35 %36
         %35 = OpLabel
               OpReturnValue %44
         %36 = OpLabel
               OpReturnValue %49
               OpFunctionEnd
%GetCascade_vf3_ = OpFunction %int None %15
%fragWorldPosition = OpFunctionParameter %_ptr_Function_v3float
         %18 = OpLabel
%cascadeIndex = OpVariable %_ptr_Function_uint Function
%worldToShadowMap = OpVariable %_ptr_Function_mat4v4float Function
%fragShadowMapPos = OpVariable %_ptr_Function_v4float Function
               OpStore %cascadeIndex %uint_0
               OpBranch %55
         %55 = OpLabel
               OpLoopMerge %57 %58 Unroll
               OpBranch %59
         %59 = OpLabel
         %60 = OpLoad %uint %cascadeIndex
         %63 = OpAccessChain %_ptr_Uniform_uint %_ %int_1
         %64 = OpLoad %uint %63
         %65 = OpULessThan %bool %60 %64
               OpBranchConditional %65 %56 %57
         %56 = OpLabel
         %68 = OpFunctionCall %mat4v4float %GetClip2TexMatrix_
         %69 = OpLoad %uint %cascadeIndex
         %71 = OpAccessChain %_ptr_Uniform_mat4v4float %_ %int_0 %69
         %72 = OpLoad %mat4v4float %71
         %73 = OpMatrixTimesMatrix %mat4v4float %68 %72
               OpStore %worldToShadowMap %73
         %76 = OpLoad %mat4v4float %worldToShadowMap
         %77 = OpLoad %v3float %fragWorldPosition
         %78 = OpCompositeExtract %float %77 0
         %79 = OpCompositeExtract %float %77 1
         %80 = OpCompositeExtract %float %77 2
         %81 = OpCompositeConstruct %v4float %78 %79 %80 %float_1
         %82 = OpMatrixTimesVector %v4float %76 %81
               OpStore %fragShadowMapPos %82
         %85 = OpAccessChain %_ptr_Function_float %fragShadowMapPos %uint_2
         %86 = OpLoad %float %85
         %87 = OpFOrdGreaterThanEqual %bool %86 %float_0
         %88 = OpAccessChain %_ptr_Function_float %fragShadowMapPos %uint_2
         %89 = OpLoad %float %88
         %90 = OpFOrdLessThanEqual %bool %89 %float_1
         %91 = OpLogicalAnd %bool %87 %90
         %92 = OpAccessChain %_ptr_Function_float %fragShadowMapPos %uint_0
         %93 = OpLoad %float %92
         %95 = OpAccessChain %_ptr_Function_float %fragShadowMapPos %uint_1
         %96 = OpLoad %float %95
         %97 = OpExtInst %float %1 FMax %93 %96
         %98 = OpFOrdLessThanEqual %bool %97 %float_1
         %99 = OpLogicalAnd %bool %91 %98
        %100 = OpAccessChain %_ptr_Function_float %fragShadowMapPos %uint_0
        %101 = OpLoad %float %100
        %102 = OpAccessChain %_ptr_Function_float %fragShadowMapPos %uint_1
        %103 = OpLoad %float %102
        %104 = OpExtInst %float %1 FMin %101 %103
        %105 = OpFOrdGreaterThanEqual %bool %104 %float_0
        %106 = OpLogicalAnd %bool %99 %105
               OpSelectionMerge %108 None
               OpBranchConditional %106 %107 %108
        %107 = OpLabel
        %109 = OpLoad %uint %cascadeIndex
        %110 = OpBitcast %int %109
               OpReturnValue %110
        %108 = OpLabel
               OpBranch %58
         %58 = OpLabel
        %112 = OpLoad %uint %cascadeIndex
        %113 = OpIAdd %uint %112 %int_1
               OpStore %cascadeIndex %113
               OpBranch %55
         %57 = OpLabel
               OpReturnValue %int_n1
               OpFunctionEnd
 %_main_vf3_ = OpFunction %int None %15
  %fragWorld = OpFunctionParameter %_ptr_Function_v3float
         %21 = OpLabel
      %param = OpVariable %_ptr_Function_v3float Function
        %118 = OpLoad %v3float %fragWorld
               OpStore %param %118
        %119 = OpFunctionCall %int %GetCascade_vf3_ %param
               OpReturnValue %119
               OpFunctionEnd
