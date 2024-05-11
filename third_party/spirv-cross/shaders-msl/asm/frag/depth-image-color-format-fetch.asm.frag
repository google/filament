; SPIR-V
; Version: 1.0
; Generator: Khronos SPIR-V Tools Assembler; 0
; Bound: 132
; Schema: 0
               OpCapability Shader
               OpMemoryModel Logical GLSL450
               OpEntryPoint Fragment %1 "main" %2 %3 %4
               OpExecutionMode %1 OriginUpperLeft
               OpDecorate %3 Location 0
               OpDecorate %2 Location 1
               OpDecorate %4 BuiltIn FragCoord
               OpDecorate %5 ArrayStride 4
               OpDecorate %6 ArrayStride 16
               OpMemberDecorate %7 0 Offset 0
               OpDecorate %7 BufferBlock
               OpDecorate %8 DescriptorSet 0
               OpDecorate %8 Binding 0
               OpDecorate %9 DescriptorSet 0
               OpDecorate %9 Binding 1
               OpDecorate %10 DescriptorSet 0
               OpDecorate %10 Binding 2
         %11 = OpTypeVoid
         %12 = OpTypeBool
         %13 = OpTypeInt 32 1
         %14 = OpTypeInt 32 0
         %16 = OpTypeFloat 32
         %17 = OpTypeVector %13 2
         %18 = OpTypeVector %14 2
         %19 = OpTypeVector %16 2
         %20 = OpTypeVector %13 3
         %21 = OpTypeVector %14 3
         %22 = OpTypeVector %16 3
         %23 = OpTypeVector %13 4
         %24 = OpTypeVector %14 4
         %25 = OpTypeVector %16 4
         %26 = OpTypeVector %12 4
         %27 = OpTypeFunction %25 %25
         %28 = OpTypeFunction %12
         %29 = OpTypeFunction %11
         %30 = OpTypePointer Input %16
         %31 = OpTypePointer Input %13
         %32 = OpTypePointer Input %14
         %33 = OpTypePointer Input %19
         %34 = OpTypePointer Input %17
         %35 = OpTypePointer Input %18
         %38 = OpTypePointer Input %22
         %40 = OpTypePointer Input %25
         %41 = OpTypePointer Input %23
         %42 = OpTypePointer Input %24
         %43 = OpTypePointer Output %16
         %44 = OpTypePointer Output %13
         %45 = OpTypePointer Output %14
         %46 = OpTypePointer Output %19
         %47 = OpTypePointer Output %17
         %48 = OpTypePointer Output %18
         %49 = OpTypePointer Output %25
         %50 = OpTypePointer Output %23
         %51 = OpTypePointer Output %24
         %52 = OpTypePointer Function %16
         %53 = OpTypePointer Function %13
         %54 = OpTypePointer Function %25
         %55 = OpConstant %16 1
         %56 = OpConstant %16 0
         %57 = OpConstant %16 0.5
         %58 = OpConstant %16 -1
         %59 = OpConstant %16 7
         %60 = OpConstant %16 8
         %61 = OpConstant %13 0
         %62 = OpConstant %13 1
         %63 = OpConstant %13 2
         %64 = OpConstant %13 3
         %65 = OpConstant %13 4
         %66 = OpConstant %14 0
         %67 = OpConstant %14 1
         %68 = OpConstant %14 2
         %69 = OpConstant %14 3
         %70 = OpConstant %14 32
         %71 = OpConstant %14 4
         %72 = OpConstant %14 2147483647
         %73 = OpConstantComposite %25 %55 %55 %55 %55
         %74 = OpConstantComposite %25 %55 %56 %56 %55
         %75 = OpConstantComposite %25 %57 %57 %57 %57
         %76 = OpTypeArray %16 %67
         %77 = OpTypeArray %16 %68
         %78 = OpTypeArray %25 %69
         %79 = OpTypeArray %16 %71
         %80 = OpTypeArray %25 %70
         %81 = OpTypePointer Input %78
         %82 = OpTypePointer Input %80
         %83 = OpTypePointer Output %77
         %84 = OpTypePointer Output %78
         %85 = OpTypePointer Output %79
          %4 = OpVariable %40 Input
          %3 = OpVariable %49 Output
          %2 = OpVariable %40 Input
         %86 = OpConstant %14 64
         %87 = OpConstant %13 64
         %88 = OpConstant %13 8
         %89 = OpConstantComposite %19 %60 %60
          %5 = OpTypeArray %16 %86
          %6 = OpTypeArray %25 %86
         %90 = OpTypePointer Uniform %16
         %91 = OpTypePointer Uniform %25
          %7 = OpTypeStruct %6
         %92 = OpTypePointer Uniform %7
         %10 = OpVariable %92 Uniform
         %93 = OpTypeImage %16 2D 1 0 0 1 Rgba32f
         %94 = OpTypePointer UniformConstant %93
          %8 = OpVariable %94 UniformConstant
         %95 = OpTypeSampler
         %96 = OpTypePointer UniformConstant %95
          %9 = OpVariable %96 UniformConstant
         %97 = OpTypeSampledImage %93
         %98 = OpTypeFunction %11 %13
          %1 = OpFunction %11 None %29
         %99 = OpLabel
        %100 = OpLoad %25 %2
        %101 = OpFunctionCall %25 %102 %100
               OpStore %3 %101
               OpReturn
               OpFunctionEnd
        %103 = OpFunction %12 None %28
        %104 = OpLabel
        %105 = OpAccessChain %30 %4 %61
        %106 = OpAccessChain %30 %4 %62
        %107 = OpLoad %16 %105
        %108 = OpLoad %16 %106
        %109 = OpFOrdEqual %12 %107 %57
        %110 = OpFOrdEqual %12 %108 %57
        %111 = OpLogicalAnd %12 %109 %110
               OpReturnValue %111
               OpFunctionEnd
        %112 = OpFunction %11 None %98
        %113 = OpFunctionParameter %13
        %114 = OpLabel
        %115 = OpSRem %13 %113 %88
        %116 = OpSDiv %13 %113 %88
        %117 = OpCompositeConstruct %17 %115 %116
        %118 = OpConvertSToF %19 %117
        %119 = OpFDiv %19 %118 %89
        %120 = OpLoad %93 %8
        %121 = OpImageFetch %25 %120 %117
         %36 = OpAccessChain %91 %10 %61 %113
               OpStore %36 %121
               OpReturn
               OpFunctionEnd
        %102 = OpFunction %25 None %27
        %122 = OpFunctionParameter %25
        %123 = OpLabel
        %124 = OpVariable %53 Function
               OpStore %124 %61
               OpBranch %125
        %125 = OpLabel
         %15 = OpLoad %13 %124
        %126 = OpSLessThan %12 %15 %87
               OpLoopMerge %127 %128 None
               OpBranchConditional %126 %129 %127
        %129 = OpLabel
        %130 = OpLoad %13 %124
        %131 = OpFunctionCall %11 %112 %130
               OpBranch %128
        %128 = OpLabel
         %37 = OpLoad %13 %124
         %39 = OpIAdd %13 %37 %62
               OpStore %124 %39
               OpBranch %125
        %127 = OpLabel
               OpReturnValue %122
               OpFunctionEnd
