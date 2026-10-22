; SPIR-V
; Version: 1.0
; Generator: Khronos Glslang Reference Front End; 7
; Bound: 132
; Schema: 0
               OpCapability Shader
          %1 = OpExtInstImport "GLSL.std.450"
               OpMemoryModel Logical GLSL450
               OpEntryPoint Fragment %main "main" %A %B %FragColor
               OpExecutionMode %main OriginUpperLeft
               OpSource GLSL 450
               OpName %main "main"
               OpName %test_vector_ "test_vector("
               OpName %test_scalar_ "test_scalar("
               OpName %le "le"
               OpName %A "A"
               OpName %B "B"
               OpName %leq "leq"
               OpName %ge "ge"
               OpName %geq "geq"
               OpName %eq "eq"
               OpName %neq "neq"
               OpName %le_0 "le"
               OpName %leq_0 "leq"
               OpName %ge_0 "ge"
               OpName %geq_0 "geq"
               OpName %eq_0 "eq"
               OpName %neq_0 "neq"
               OpName %FragColor "FragColor"
               OpDecorate %A Location 0
               OpDecorate %B Location 1
               OpDecorate %FragColor Location 0
       %void = OpTypeVoid
          %3 = OpTypeFunction %void
      %float = OpTypeFloat 32
    %v4float = OpTypeVector %float 4
          %8 = OpTypeFunction %v4float
         %11 = OpTypeFunction %float
       %bool = OpTypeBool
     %v4bool = OpTypeVector %bool 4
%_ptr_Function_v4bool = OpTypePointer Function %v4bool
%_ptr_Input_v4float = OpTypePointer Input %v4float
          %A = OpVariable %_ptr_Input_v4float Input
          %B = OpVariable %_ptr_Input_v4float Input
    %float_0 = OpConstant %float 0
    %float_1 = OpConstant %float 1
         %47 = OpConstantComposite %v4float %float_0 %float_0 %float_0 %float_0
         %48 = OpConstantComposite %v4float %float_1 %float_1 %float_1 %float_1
%_ptr_Function_bool = OpTypePointer Function %bool
       %uint = OpTypeInt 32 0
     %uint_0 = OpConstant %uint 0
%_ptr_Input_float = OpTypePointer Input %float
%_ptr_Output_v4float = OpTypePointer Output %v4float
  %FragColor = OpVariable %_ptr_Output_v4float Output
       %main = OpFunction %void None %3
          %5 = OpLabel
        %128 = OpFunctionCall %v4float %test_vector_
        %129 = OpFunctionCall %float %test_scalar_
        %130 = OpCompositeConstruct %v4float %129 %129 %129 %129
        %131 = OpFAdd %v4float %128 %130
               OpStore %FragColor %131
               OpReturn
               OpFunctionEnd
%test_vector_ = OpFunction %v4float None %8
         %10 = OpLabel
         %le = OpVariable %_ptr_Function_v4bool Function
        %leq = OpVariable %_ptr_Function_v4bool Function
         %ge = OpVariable %_ptr_Function_v4bool Function
        %geq = OpVariable %_ptr_Function_v4bool Function
         %eq = OpVariable %_ptr_Function_v4bool Function
        %neq = OpVariable %_ptr_Function_v4bool Function
         %20 = OpLoad %v4float %A
         %22 = OpLoad %v4float %B
         %23 = OpFUnordLessThan %v4bool %20 %22
               OpStore %le %23
         %25 = OpLoad %v4float %A
         %26 = OpLoad %v4float %B
         %27 = OpFUnordLessThanEqual %v4bool %25 %26
               OpStore %leq %27
         %29 = OpLoad %v4float %A
         %30 = OpLoad %v4float %B
         %31 = OpFUnordGreaterThan %v4bool %29 %30
               OpStore %ge %31
         %33 = OpLoad %v4float %A
         %34 = OpLoad %v4float %B
         %35 = OpFUnordGreaterThanEqual %v4bool %33 %34
               OpStore %geq %35
         %37 = OpLoad %v4float %A
         %38 = OpLoad %v4float %B
         %39 = OpFUnordEqual %v4bool %37 %38
               OpStore %eq %39
         %ordered = OpFOrdNotEqual %v4bool %37 %38
               OpStore %neq %ordered
         %41 = OpLoad %v4float %A
         %42 = OpLoad %v4float %B
         %43 = OpFUnordNotEqual %v4bool %41 %42
               OpStore %neq %43
         %44 = OpLoad %v4bool %le
         %49 = OpSelect %v4float %44 %48 %47
         %50 = OpLoad %v4bool %leq
         %51 = OpSelect %v4float %50 %48 %47
         %52 = OpFAdd %v4float %49 %51
         %53 = OpLoad %v4bool %ge
         %54 = OpSelect %v4float %53 %48 %47
         %55 = OpFAdd %v4float %52 %54
         %56 = OpLoad %v4bool %geq
         %57 = OpSelect %v4float %56 %48 %47
         %58 = OpFAdd %v4float %55 %57
         %59 = OpLoad %v4bool %eq
         %60 = OpSelect %v4float %59 %48 %47
         %61 = OpFAdd %v4float %58 %60
         %62 = OpLoad %v4bool %neq
         %63 = OpSelect %v4float %62 %48 %47
         %64 = OpFAdd %v4float %61 %63
               OpReturnValue %64
               OpFunctionEnd
%test_scalar_ = OpFunction %float None %11
         %13 = OpLabel
       %le_0 = OpVariable %_ptr_Function_bool Function
      %leq_0 = OpVariable %_ptr_Function_bool Function
       %ge_0 = OpVariable %_ptr_Function_bool Function
      %geq_0 = OpVariable %_ptr_Function_bool Function
       %eq_0 = OpVariable %_ptr_Function_bool Function
      %neq_0 = OpVariable %_ptr_Function_bool Function
         %72 = OpAccessChain %_ptr_Input_float %A %uint_0
         %73 = OpLoad %float %72
         %74 = OpAccessChain %_ptr_Input_float %B %uint_0
         %75 = OpLoad %float %74
         %76 = OpFUnordLessThan %bool %73 %75
               OpStore %le_0 %76
         %78 = OpAccessChain %_ptr_Input_float %A %uint_0
         %79 = OpLoad %float %78
         %80 = OpAccessChain %_ptr_Input_float %B %uint_0
         %81 = OpLoad %float %80
         %82 = OpFUnordLessThanEqual %bool %79 %81
               OpStore %leq_0 %82
         %84 = OpAccessChain %_ptr_Input_float %A %uint_0
         %85 = OpLoad %float %84
         %86 = OpAccessChain %_ptr_Input_float %B %uint_0
         %87 = OpLoad %float %86
         %88 = OpFUnordGreaterThan %bool %85 %87
               OpStore %ge_0 %88
         %90 = OpAccessChain %_ptr_Input_float %A %uint_0
         %91 = OpLoad %float %90
         %92 = OpAccessChain %_ptr_Input_float %B %uint_0
         %93 = OpLoad %float %92
         %94 = OpFUnordGreaterThanEqual %bool %91 %93
               OpStore %geq_0 %94
         %96 = OpAccessChain %_ptr_Input_float %A %uint_0
         %97 = OpLoad %float %96
         %98 = OpAccessChain %_ptr_Input_float %B %uint_0
         %99 = OpLoad %float %98
        %100 = OpFUnordEqual %bool %97 %99
               OpStore %eq_0 %100
        %102 = OpAccessChain %_ptr_Input_float %A %uint_0
        %103 = OpLoad %float %102
        %104 = OpAccessChain %_ptr_Input_float %B %uint_0
        %105 = OpLoad %float %104
        %106 = OpFUnordNotEqual %bool %103 %105
               OpStore %neq_0 %106
        %107 = OpLoad %bool %le_0
        %108 = OpSelect %float %107 %float_1 %float_0
        %109 = OpLoad %bool %leq_0
        %110 = OpSelect %float %109 %float_1 %float_0
        %111 = OpFAdd %float %108 %110
        %112 = OpLoad %bool %ge_0
        %113 = OpSelect %float %112 %float_1 %float_0
        %114 = OpFAdd %float %111 %113
        %115 = OpLoad %bool %geq_0
        %116 = OpSelect %float %115 %float_1 %float_0
        %117 = OpFAdd %float %114 %116
        %118 = OpLoad %bool %eq_0
        %119 = OpSelect %float %118 %float_1 %float_0
        %120 = OpFAdd %float %117 %119
        %121 = OpLoad %bool %neq_0
        %122 = OpSelect %float %121 %float_1 %float_0
        %123 = OpFAdd %float %120 %122
               OpReturnValue %123
               OpFunctionEnd
