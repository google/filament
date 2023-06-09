; SPIR-V
; Version: 1.3
; Generator: Khronos Glslang Reference Front End; 7
; Bound: 122
; Schema: 0
               OpCapability Shader
          %1 = OpExtInstImport "GLSL.std.450"
               OpMemoryModel Logical GLSL450
               OpEntryPoint Fragment %main "main" %c %d %e %f %g %h %FragColor
               OpExecutionMode %main OriginUpperLeft
               OpSource GLSL 460
               OpName %main "main"
               OpName %t0 "t0"
               OpName %a "a"
               OpName %t1 "t1"
               OpName %b "b"
               OpName %c1 "c1"
               OpName %c2 "c2"
               OpName %c3 "c3"
               OpName %c4 "c4"
               OpName %c5 "c5"
               OpName %c6 "c6"
               OpName %c7 "c7"
               OpName %c "c"
               OpName %d "d"
               OpName %c8 "c8"
               OpName %c9 "c9"
               OpName %c10 "c10"
               OpName %c11 "c11"
               OpName %c12 "c12"
               OpName %c13 "c13"
               OpName %e "e"
               OpName %f "f"
               OpName %c14 "c14"
               OpName %c15 "c15"
               OpName %c16 "c16"
               OpName %c17 "c17"
               OpName %c18 "c18"
               OpName %c19 "c19"
               OpName %g "g"
               OpName %h "h"
               OpName %c20 "c20"
               OpName %c21 "c21"
               OpName %c22 "c22"
               OpName %c23 "c23"
               OpName %c24 "c24"
               OpName %FragColor "FragColor"
               OpDecorate %a SpecId 1
               OpDecorate %b SpecId 2
               OpDecorate %c Location 2
               OpDecorate %d Location 3
               OpDecorate %e Location 4
               OpDecorate %f Location 5
               OpDecorate %g Location 6
               OpDecorate %h Location 7
               OpDecorate %FragColor Location 0
       %void = OpTypeVoid
          %3 = OpTypeFunction %void
      %float = OpTypeFloat 32
%_ptr_Function_float = OpTypePointer Function %float
          %a = OpSpecConstant %float 1
          %b = OpSpecConstant %float 2
       %bool = OpTypeBool
%_ptr_Function_bool = OpTypePointer Function %bool
     %v2bool = OpTypeVector %bool 2
%_ptr_Function_v2bool = OpTypePointer Function %v2bool
    %v2float = OpTypeVector %float 2
%_ptr_Input_v2float = OpTypePointer Input %v2float
          %c = OpVariable %_ptr_Input_v2float Input
          %d = OpVariable %_ptr_Input_v2float Input
     %v3bool = OpTypeVector %bool 3
%_ptr_Function_v3bool = OpTypePointer Function %v3bool
    %v3float = OpTypeVector %float 3
%_ptr_Input_v3float = OpTypePointer Input %v3float
          %e = OpVariable %_ptr_Input_v3float Input
          %f = OpVariable %_ptr_Input_v3float Input
     %v4bool = OpTypeVector %bool 4
%_ptr_Function_v4bool = OpTypePointer Function %v4bool
    %v4float = OpTypeVector %float 4
%_ptr_Input_v4float = OpTypePointer Input %v4float
          %g = OpVariable %_ptr_Input_v4float Input
          %h = OpVariable %_ptr_Input_v4float Input
%_ptr_Output_v4float = OpTypePointer Output %v4float
  %FragColor = OpVariable %_ptr_Output_v4float Output
       %main = OpFunction %void None %3
          %5 = OpLabel
         %t0 = OpVariable %_ptr_Function_float Function
         %t1 = OpVariable %_ptr_Function_float Function
         %c1 = OpVariable %_ptr_Function_bool Function
         %c2 = OpVariable %_ptr_Function_bool Function
         %c3 = OpVariable %_ptr_Function_bool Function
         %c4 = OpVariable %_ptr_Function_bool Function
         %c5 = OpVariable %_ptr_Function_bool Function
         %c6 = OpVariable %_ptr_Function_bool Function
         %c7 = OpVariable %_ptr_Function_v2bool Function
         %c8 = OpVariable %_ptr_Function_v2bool Function
         %c9 = OpVariable %_ptr_Function_v2bool Function
        %c10 = OpVariable %_ptr_Function_v2bool Function
        %c11 = OpVariable %_ptr_Function_v2bool Function
        %c12 = OpVariable %_ptr_Function_v2bool Function
        %c13 = OpVariable %_ptr_Function_v3bool Function
        %c14 = OpVariable %_ptr_Function_v3bool Function
        %c15 = OpVariable %_ptr_Function_v3bool Function
        %c16 = OpVariable %_ptr_Function_v3bool Function
        %c17 = OpVariable %_ptr_Function_v3bool Function
        %c18 = OpVariable %_ptr_Function_v3bool Function
        %c19 = OpVariable %_ptr_Function_v4bool Function
        %c20 = OpVariable %_ptr_Function_v4bool Function
        %c21 = OpVariable %_ptr_Function_v4bool Function
        %c22 = OpVariable %_ptr_Function_v4bool Function
        %c23 = OpVariable %_ptr_Function_v4bool Function
        %c24 = OpVariable %_ptr_Function_v4bool Function
               OpStore %t0 %a
               OpStore %t1 %b
         %15 = OpFUnordEqual %bool %a %b
               OpStore %c1 %15
         %ordered = OpFOrdNotEqual %bool %a %b
               OpStore %c1 %ordered
         %17 = OpFUnordNotEqual %bool %a %b
               OpStore %c2 %17
         %19 = OpFUnordLessThan %bool %a %b
               OpStore %c3 %19
         %21 = OpFUnordGreaterThan %bool %a %b
               OpStore %c4 %21
         %23 = OpFUnordLessThanEqual %bool %a %b
               OpStore %c5 %23
         %25 = OpFUnordGreaterThanEqual %bool %a %b
               OpStore %c6 %25
         %32 = OpLoad %v2float %c
         %34 = OpLoad %v2float %d
         %35 = OpFUnordEqual %v2bool %32 %34
               OpStore %c7 %35
         %37 = OpLoad %v2float %c
         %38 = OpLoad %v2float %d
         %39 = OpFUnordNotEqual %v2bool %37 %38
               OpStore %c8 %39
         %41 = OpLoad %v2float %c
         %42 = OpLoad %v2float %d
         %43 = OpFUnordLessThan %v2bool %41 %42
               OpStore %c9 %43
         %45 = OpLoad %v2float %c
         %46 = OpLoad %v2float %d
         %47 = OpFUnordGreaterThan %v2bool %45 %46
               OpStore %c10 %47
         %49 = OpLoad %v2float %c
         %50 = OpLoad %v2float %d
         %51 = OpFUnordLessThanEqual %v2bool %49 %50
               OpStore %c11 %51
         %53 = OpLoad %v2float %c
         %54 = OpLoad %v2float %d
         %55 = OpFUnordGreaterThanEqual %v2bool %53 %54
               OpStore %c12 %55
         %62 = OpLoad %v3float %e
         %64 = OpLoad %v3float %f
         %65 = OpFUnordEqual %v3bool %62 %64
               OpStore %c13 %65
         %67 = OpLoad %v3float %e
         %68 = OpLoad %v3float %f
         %69 = OpFUnordNotEqual %v3bool %67 %68
               OpStore %c14 %69
         %71 = OpLoad %v3float %e
         %72 = OpLoad %v3float %f
         %73 = OpFUnordLessThan %v3bool %71 %72
               OpStore %c15 %73
         %75 = OpLoad %v3float %e
         %76 = OpLoad %v3float %f
         %77 = OpFUnordGreaterThan %v3bool %75 %76
               OpStore %c16 %77
         %79 = OpLoad %v3float %e
         %80 = OpLoad %v3float %f
         %81 = OpFUnordLessThanEqual %v3bool %79 %80
               OpStore %c17 %81
         %83 = OpLoad %v3float %e
         %84 = OpLoad %v3float %f
         %85 = OpFUnordGreaterThanEqual %v3bool %83 %84
               OpStore %c18 %85
         %92 = OpLoad %v4float %g
         %94 = OpLoad %v4float %h
         %95 = OpFUnordEqual %v4bool %92 %94
               OpStore %c19 %95
         %97 = OpLoad %v4float %g
         %98 = OpLoad %v4float %h
         %99 = OpFUnordNotEqual %v4bool %97 %98
               OpStore %c20 %99
        %101 = OpLoad %v4float %g
        %102 = OpLoad %v4float %h
        %103 = OpFUnordLessThan %v4bool %101 %102
               OpStore %c21 %103
        %105 = OpLoad %v4float %g
        %106 = OpLoad %v4float %h
        %107 = OpFUnordGreaterThan %v4bool %105 %106
               OpStore %c22 %107
        %109 = OpLoad %v4float %g
        %110 = OpLoad %v4float %h
        %111 = OpFUnordLessThanEqual %v4bool %109 %110
               OpStore %c23 %111
        %113 = OpLoad %v4float %g
        %114 = OpLoad %v4float %h
        %115 = OpFUnordGreaterThanEqual %v4bool %113 %114
               OpStore %c24 %115
        %118 = OpLoad %float %t0
        %119 = OpLoad %float %t1
        %120 = OpFAdd %float %118 %119
        %121 = OpCompositeConstruct %v4float %120 %120 %120 %120
               OpStore %FragColor %121
               OpReturn
               OpFunctionEnd
