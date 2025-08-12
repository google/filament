; SPIR-V
; Version: 1.0
; Generator: Wine VKD3D Shader Compiler; 2
; Bound: 126
; Schema: 0
               OpCapability Tessellation
               OpMemoryModel Logical GLSL450
               OpEntryPoint TessellationControl %1 "main" %4 %30 %80 %101 %103 %108 %110 %115 %117
               OpExecutionMode %1 OutputVertices 3
               OpExecutionMode %1 Triangles
               OpExecutionMode %1 SpacingEqual
               OpExecutionMode %1 VertexOrderCw
               OpName %1 "main"
               OpName %11 "opc"
               OpName %14 "cb1_struct"
               OpName %16 "cb0_0"
               OpName %22 "vicp"
               OpName %23 "fork0"
               OpName %26 "vForkInstanceId"
               OpName %34 "r0"
               OpName %32 "fork0_epilogue"
               OpName %75 "fork1"
               OpName %81 "fork1_epilogue"
               OpName %101 "v0"
               OpName %103 "v1"
               OpName %108 "vicp0"
               OpName %110 "vocp0"
               OpName %115 "vicp1"
               OpName %117 "vocp1"
               OpDecorate %4 BuiltIn InvocationId
               OpDecorate %13 ArrayStride 16
               OpDecorate %14 Block
               OpMemberDecorate %14 0 Offset 0
               OpDecorate %16 DescriptorSet 0
               OpDecorate %16 Binding 0
               OpDecorate %30 BuiltIn TessLevelOuter
               OpDecorate %30 Patch
               OpDecorate %30 Patch
               OpDecorate %30 Patch
               OpDecorate %30 Patch
               OpDecorate %80 BuiltIn TessLevelInner
               OpDecorate %80 Patch
               OpDecorate %80 Patch
               OpDecorate %101 Location 0
               OpDecorate %103 Location 1
               OpDecorate %108 Location 2
               OpDecorate %110 Location 3
               OpDecorate %115 Location 4
               OpDecorate %117 Location 5
          %2 = OpTypeInt 32 1
          %3 = OpTypePointer Input %2
          %4 = OpVariable %3 Input
          %5 = OpTypeFloat 32
          %6 = OpTypeVector %5 4
          %7 = OpTypeInt 32 0
          %8 = OpConstant %7 4
          %9 = OpTypeArray %6 %8
         %10 = OpTypePointer Private %9
         %11 = OpVariable %10 Private
         %12 = OpConstant %7 1
         %13 = OpTypeArray %6 %12
         %14 = OpTypeStruct %13
         %15 = OpTypePointer Uniform %14
         %16 = OpVariable %15 Uniform
         %17 = OpConstant %7 3
         %18 = OpTypeArray %6 %17
         %19 = OpConstant %7 2
         %20 = OpTypeArray %18 %19
         %21 = OpTypePointer Private %20
         %22 = OpVariable %21 Private
         %24 = OpTypeVoid
         %25 = OpTypeFunction %24 %7
         %28 = OpTypeArray %5 %8
         %29 = OpTypePointer Output %28
         %30 = OpVariable %29 Output
         %31 = OpConstant %7 0
         %33 = OpTypePointer Function %6
         %36 = OpTypePointer Function %5
         %38 = OpTypePointer Uniform %6
         %40 = OpTypePointer Uniform %5
         %46 = OpTypePointer Private %6
         %48 = OpTypePointer Private %5
         %52 = OpVariable %46 Private
         %55 = OpVariable %46 Private
         %58 = OpVariable %46 Private
         %60 = OpTypeFunction %24 %46 %46 %46
         %69 = OpTypePointer Output %5
         %76 = OpTypeFunction %24
         %78 = OpTypeArray %5 %19
         %79 = OpTypePointer Output %78
         %80 = OpVariable %79 Output
         %89 = OpVariable %46 Private
         %91 = OpTypeFunction %24 %46
         %98 = OpTypePointer Private %18
        %100 = OpTypePointer Input %18
        %101 = OpVariable %100 Input
        %103 = OpVariable %100 Input
        %105 = OpTypeVector %5 3
        %106 = OpTypeArray %105 %17
        %107 = OpTypePointer Input %106
        %108 = OpVariable %107 Input
        %109 = OpTypePointer Output %106
        %110 = OpVariable %109 Output
        %111 = OpTypePointer Output %105
        %112 = OpTypePointer Input %105
        %115 = OpVariable %100 Input
        %116 = OpTypePointer Output %18
        %117 = OpVariable %116 Output
        %118 = OpTypePointer Output %6
        %119 = OpTypePointer Input %6
         %23 = OpFunction %24 None %25
         %26 = OpFunctionParameter %7
         %27 = OpLabel
         %34 = OpVariable %33 Function
         %35 = OpBitcast %5 %26
         %37 = OpInBoundsAccessChain %36 %34 %31
               OpStore %37 %35
         %39 = OpAccessChain %38 %16 %31 %31
         %41 = OpInBoundsAccessChain %40 %39 %31
         %42 = OpLoad %5 %41
         %43 = OpInBoundsAccessChain %36 %34 %31
         %44 = OpLoad %5 %43
         %45 = OpBitcast %2 %44
         %47 = OpAccessChain %46 %11 %45
         %49 = OpInBoundsAccessChain %48 %47 %31
               OpStore %49 %42
         %50 = OpAccessChain %46 %11 %31
         %51 = OpLoad %6 %50
               OpStore %52 %51
         %53 = OpAccessChain %46 %11 %12
         %54 = OpLoad %6 %53
               OpStore %55 %54
         %56 = OpAccessChain %46 %11 %19
         %57 = OpLoad %6 %56
               OpStore %58 %57
         %59 = OpFunctionCall %24 %32 %52 %55 %58
               OpReturn
               OpFunctionEnd
         %32 = OpFunction %24 None %60
         %61 = OpFunctionParameter %46
         %62 = OpFunctionParameter %46
         %63 = OpFunctionParameter %46
         %64 = OpLabel
         %65 = OpLoad %6 %61
         %66 = OpLoad %6 %62
         %67 = OpLoad %6 %63
         %68 = OpCompositeExtract %5 %65 0
         %70 = OpAccessChain %69 %30 %31
               OpStore %70 %68
         %71 = OpCompositeExtract %5 %66 0
         %72 = OpAccessChain %69 %30 %12
               OpStore %72 %71
         %73 = OpCompositeExtract %5 %67 0
         %74 = OpAccessChain %69 %30 %19
               OpStore %74 %73
               OpReturn
               OpFunctionEnd
         %75 = OpFunction %24 None %76
         %77 = OpLabel
         %82 = OpAccessChain %38 %16 %31 %31
         %83 = OpInBoundsAccessChain %40 %82 %31
         %84 = OpLoad %5 %83
         %85 = OpAccessChain %46 %11 %17
         %86 = OpInBoundsAccessChain %48 %85 %31
               OpStore %86 %84
         %87 = OpAccessChain %46 %11 %17
         %88 = OpLoad %6 %87
               OpStore %89 %88
         %90 = OpFunctionCall %24 %81 %89
               OpReturn
               OpFunctionEnd
         %81 = OpFunction %24 None %91
         %92 = OpFunctionParameter %46
         %93 = OpLabel
         %94 = OpLoad %6 %92
         %95 = OpCompositeExtract %5 %94 0
         %96 = OpAccessChain %69 %80 %31
               OpStore %96 %95
               OpReturn
               OpFunctionEnd
          %1 = OpFunction %24 None %76
         %97 = OpLabel
         %99 = OpInBoundsAccessChain %98 %22 %31
               OpCopyMemory %99 %101
        %102 = OpInBoundsAccessChain %98 %22 %12
               OpCopyMemory %102 %103
        %104 = OpLoad %2 %4
        %113 = OpAccessChain %111 %110 %104
        %114 = OpAccessChain %112 %108 %104
               OpCopyMemory %113 %114
        %120 = OpAccessChain %118 %117 %104
        %121 = OpAccessChain %119 %115 %104
               OpCopyMemory %120 %121
        %122 = OpFunctionCall %24 %23 %31
        %123 = OpFunctionCall %24 %23 %12
        %124 = OpFunctionCall %24 %23 %19
        %125 = OpFunctionCall %24 %75
               OpReturn
               OpFunctionEnd
