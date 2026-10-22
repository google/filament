; SPIR-V
; Version: 1.0
; Generator: Khronos Glslang Reference Front End; 6
; Bound: 111
; Schema: 0
               OpCapability Shader
          %1 = OpExtInstImport "GLSL.std.450"
               OpMemoryModel Logical GLSL450
               OpEntryPoint Fragment %main "main" %FragColor %index
               OpExecutionMode %main OriginUpperLeft
               OpSource ESSL 310
               OpName %main "main"
               OpName %FragColor "FragColor"
               OpName %index "index"
               OpName %indexable "indexable"
               OpName %indexable_0 "indexable"
               OpName %indexable_1 "indexable"
               OpName %foo "foo"
               OpName %foobar "foobar"
               OpName %baz "baz"
               OpDecorate %FragColor RelaxedPrecision
               OpDecorate %FragColor Location 0
               OpDecorate %index RelaxedPrecision
               OpDecorate %index Flat
               OpDecorate %index Location 0
               OpDecorate %20 RelaxedPrecision
               OpDecorate %25 RelaxedPrecision
               OpDecorate %26 RelaxedPrecision
               OpDecorate %32 RelaxedPrecision
               OpDecorate %34 RelaxedPrecision
               OpDecorate %37 RelaxedPrecision
               OpDecorate %38 RelaxedPrecision
               OpDecorate %39 RelaxedPrecision
               OpDecorate %41 RelaxedPrecision
               OpDecorate %42 RelaxedPrecision
               OpDecorate %45 RelaxedPrecision
               OpDecorate %46 RelaxedPrecision
               OpDecorate %47 RelaxedPrecision
               OpDecorate %foo RelaxedPrecision
               OpDecorate %61 RelaxedPrecision
               OpDecorate %66 RelaxedPrecision
               OpDecorate %68 RelaxedPrecision
               OpDecorate %71 RelaxedPrecision
               OpDecorate %72 RelaxedPrecision
               OpDecorate %73 RelaxedPrecision
               OpDecorate %75 RelaxedPrecision
               OpDecorate %76 RelaxedPrecision
               OpDecorate %79 RelaxedPrecision
               OpDecorate %80 RelaxedPrecision
               OpDecorate %81 RelaxedPrecision
               OpDecorate %foobar RelaxedPrecision
               OpDecorate %83 RelaxedPrecision
               OpDecorate %90 RelaxedPrecision
               OpDecorate %91 RelaxedPrecision
               OpDecorate %93 RelaxedPrecision
               OpDecorate %94 RelaxedPrecision
               OpDecorate %95 RelaxedPrecision
               OpDecorate %baz RelaxedPrecision
               OpDecorate %105 RelaxedPrecision
               OpDecorate %106 RelaxedPrecision
               OpDecorate %108 RelaxedPrecision
               OpDecorate %109 RelaxedPrecision
               OpDecorate %110 RelaxedPrecision
               OpDecorate %16 RelaxedPrecision
       %void = OpTypeVoid
          %3 = OpTypeFunction %void
      %float = OpTypeFloat 32
%_ptr_Output_float = OpTypePointer Output %float
  %FragColor = OpVariable %_ptr_Output_float Output
       %uint = OpTypeInt 32 0
    %uint_16 = OpConstant %uint 16
%_arr_float_uint_16 = OpTypeArray %float %uint_16
    %float_1 = OpConstant %float 1
    %float_2 = OpConstant %float 2
    %float_3 = OpConstant %float 3
    %float_4 = OpConstant %float 4
         %16 = OpConstantComposite %_arr_float_uint_16 %float_1 %float_2 %float_3 %float_4 %float_1 %float_2 %float_3 %float_4 %float_1 %float_2 %float_3 %float_4 %float_1 %float_2 %float_3 %float_4
        %int = OpTypeInt 32 1
%_ptr_Input_int = OpTypePointer Input %int
      %index = OpVariable %_ptr_Input_int Input
%_ptr_Function__arr_float_uint_16 = OpTypePointer Function %_arr_float_uint_16
%_ptr_Function_float = OpTypePointer Function %float
     %int_10 = OpConstant %int 10
       %bool = OpTypeBool
      %int_1 = OpConstant %int 1
    %v4float = OpTypeVector %float 4
     %uint_4 = OpConstant %uint 4
%_arr_v4float_uint_4 = OpTypeArray %v4float %uint_4
%_ptr_Function__arr_v4float_uint_4 = OpTypePointer Function %_arr_v4float_uint_4
    %float_0 = OpConstant %float 0
         %54 = OpConstantComposite %v4float %float_0 %float_0 %float_0 %float_0
         %55 = OpConstantComposite %v4float %float_1 %float_1 %float_1 %float_1
    %float_8 = OpConstant %float 8
         %57 = OpConstantComposite %v4float %float_8 %float_8 %float_8 %float_8
    %float_5 = OpConstant %float 5
         %59 = OpConstantComposite %v4float %float_5 %float_5 %float_5 %float_5
         %60 = OpConstantComposite %_arr_v4float_uint_4 %54 %55 %57 %59
     %int_30 = OpConstant %int 30
      %int_3 = OpConstant %int 3
     %uint_1 = OpConstant %uint 1
     %uint_0 = OpConstant %uint 0
   %float_20 = OpConstant %float 20
     %uint_2 = OpConstant %uint 2
         %97 = OpConstantComposite %v4float %float_20 %float_20 %float_20 %float_20
   %float_30 = OpConstant %float 30
         %99 = OpConstantComposite %v4float %float_30 %float_30 %float_30 %float_30
   %float_50 = OpConstant %float 50
        %101 = OpConstantComposite %v4float %float_50 %float_50 %float_50 %float_50
   %float_60 = OpConstant %float 60
        %103 = OpConstantComposite %v4float %float_60 %float_60 %float_60 %float_60
        %104 = OpConstantComposite %_arr_v4float_uint_4 %97 %99 %101 %103
       %main = OpFunction %void None %3
          %5 = OpLabel
  %indexable = OpVariable %_ptr_Function__arr_float_uint_16 Function %16
%indexable_0 = OpVariable %_ptr_Function__arr_float_uint_16 Function %16
%indexable_1 = OpVariable %_ptr_Function__arr_float_uint_16 Function %16
        %foo = OpVariable %_ptr_Function__arr_v4float_uint_4 Function %60
     %foobar = OpVariable %_ptr_Function__arr_v4float_uint_4 Function %60
        %baz = OpVariable %_ptr_Function__arr_v4float_uint_4 Function %60
         %20 = OpLoad %int %index
         %24 = OpAccessChain %_ptr_Function_float %indexable %20
         %25 = OpLoad %float %24
               OpStore %FragColor %25
         %26 = OpLoad %int %index
         %29 = OpSLessThan %bool %26 %int_10
               OpSelectionMerge %31 None
               OpBranchConditional %29 %30 %40
         %30 = OpLabel
         %32 = OpLoad %int %index
         %34 = OpBitwiseXor %int %32 %int_1
         %36 = OpAccessChain %_ptr_Function_float %indexable_0 %34
         %37 = OpLoad %float %36
         %38 = OpLoad %float %FragColor
         %39 = OpFAdd %float %38 %37
               OpStore %FragColor %39
               OpBranch %31
         %40 = OpLabel
         %41 = OpLoad %int %index
         %42 = OpBitwiseAnd %int %41 %int_1
         %44 = OpAccessChain %_ptr_Function_float %indexable_1 %42
         %45 = OpLoad %float %44
         %46 = OpLoad %float %FragColor
         %47 = OpFAdd %float %46 %45
               OpStore %FragColor %47
               OpBranch %31
         %31 = OpLabel
         %61 = OpLoad %int %index
         %63 = OpSGreaterThan %bool %61 %int_30
               OpSelectionMerge %65 None
               OpBranchConditional %63 %64 %74
         %64 = OpLabel
         %66 = OpLoad %int %index
         %68 = OpBitwiseAnd %int %66 %int_3
         %70 = OpAccessChain %_ptr_Function_float %foo %68 %uint_1
         %71 = OpLoad %float %70
         %72 = OpLoad %float %FragColor
         %73 = OpFAdd %float %72 %71
               OpStore %FragColor %73
               OpBranch %65
         %74 = OpLabel
         %75 = OpLoad %int %index
         %76 = OpBitwiseAnd %int %75 %int_1
         %78 = OpAccessChain %_ptr_Function_float %foo %76 %uint_0
         %79 = OpLoad %float %78
         %80 = OpLoad %float %FragColor
         %81 = OpFAdd %float %80 %79
               OpStore %FragColor %81
               OpBranch %65
         %65 = OpLabel
         %83 = OpLoad %int %index
         %84 = OpSGreaterThan %bool %83 %int_30
               OpSelectionMerge %86 None
               OpBranchConditional %84 %85 %86
         %85 = OpLabel
         %89 = OpAccessChain %_ptr_Function_float %foobar %int_1 %uint_2
               OpStore %89 %float_20
               OpBranch %86
         %86 = OpLabel
         %90 = OpLoad %int %index
         %91 = OpBitwiseAnd %int %90 %int_3
         %92 = OpAccessChain %_ptr_Function_float %foobar %91 %uint_2
         %93 = OpLoad %float %92
         %94 = OpLoad %float %FragColor
         %95 = OpFAdd %float %94 %93
               OpStore %FragColor %95
               OpStore %baz %104
        %105 = OpLoad %int %index
        %106 = OpBitwiseAnd %int %105 %int_3
        %107 = OpAccessChain %_ptr_Function_float %baz %106 %uint_2
        %108 = OpLoad %float %107
        %109 = OpLoad %float %FragColor
        %110 = OpFAdd %float %109 %108
               OpStore %FragColor %110
               OpReturn
               OpFunctionEnd
