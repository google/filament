; SPIR-V
; Version: 1.0
; Generator: Khronos Glslang Reference Front End; 1
; Bound: 58
; Schema: 0
               OpCapability Shader
               OpCapability ClipDistance
               OpCapability CullDistance
          %1 = OpExtInstImport "GLSL.std.450"
               OpMemoryModel Logical GLSL450
               OpEntryPoint Vertex %4 "main" %52 %output
               OpSource GLSL 450
               OpName %4 "main"
               OpName %9 "pos"
               OpName %50 "gl_PerVertex"
               OpMemberName %50 0 "gl_Position"
               OpMemberName %50 1 "gl_PointSize"
               OpMemberName %50 2 "gl_ClipDistance"
               OpMemberName %50 3 "gl_CullDistance"
               OpName %52 ""
               OpDecorate %13 SpecId 201
               OpDecorate %24 SpecId 202
               OpMemberDecorate %50 0 BuiltIn Position
               OpMemberDecorate %50 1 BuiltIn PointSize
               OpMemberDecorate %50 2 BuiltIn ClipDistance
               OpMemberDecorate %50 3 BuiltIn CullDistance
               OpDecorate %50 Block
               OpDecorate %57 SpecId 200
			   OpDecorate %output Flat
			   OpDecorate %output Location 0
          %2 = OpTypeVoid
          %3 = OpTypeFunction %2
          %6 = OpTypeFloat 32
          %7 = OpTypeVector %6 4
          %8 = OpTypePointer Function %7
         %10 = OpConstant %6 0
         %11 = OpConstantComposite %7 %10 %10 %10 %10
         %12 = OpTypeInt 32 1
		 %int_ptr = OpTypePointer Output %12
         %13 = OpSpecConstant %12 -10
         %14 = OpConstant %12 2
         %15 = OpSpecConstantOp %12 IAdd %13 %14
         %17 = OpTypeInt 32 0
         %18 = OpConstant %17 1
         %19 = OpTypePointer Function %6
         %24 = OpSpecConstant %17 100
         %25 = OpConstant %17 5
         %26 = OpSpecConstantOp %17 UMod %24 %25
         %28 = OpConstant %17 2
         %33 = OpConstant %12 20
         %34 = OpConstant %12 30
         %35 = OpTypeVector %12 4
         %36 = OpSpecConstantComposite %35 %33 %34 %15 %15
         %40 = OpTypeVector %12 2
         %41 = OpSpecConstantOp %40 VectorShuffle %36 %36 1 0
		 %foo = OpSpecConstantOp %12 CompositeExtract %36 1
         %42 = OpTypeVector %6 2
         %49 = OpTypeArray %6 %18
         %50 = OpTypeStruct %7 %6 %49 %49
         %51 = OpTypePointer Output %50
         %52 = OpVariable %51 Output
		 %output = OpVariable %int_ptr Output
         %53 = OpConstant %12 0
         %55 = OpTypePointer Output %7
         %57 = OpSpecConstant %6 3.14159
          %4 = OpFunction %2 None %3
          %5 = OpLabel
          %9 = OpVariable %8 Function
               OpStore %9 %11
         %16 = OpConvertSToF %6 %15
         %20 = OpAccessChain %19 %9 %18
         %21 = OpLoad %6 %20
         %22 = OpFAdd %6 %21 %16
         %23 = OpAccessChain %19 %9 %18
               OpStore %23 %22
         %27 = OpConvertUToF %6 %26
         %29 = OpAccessChain %19 %9 %28
         %30 = OpLoad %6 %29
         %31 = OpFAdd %6 %30 %27
         %32 = OpAccessChain %19 %9 %28
               OpStore %32 %31
         %37 = OpConvertSToF %7 %36
         %38 = OpLoad %7 %9
         %39 = OpFAdd %7 %38 %37
               OpStore %9 %39
         %43 = OpConvertSToF %42 %41
         %44 = OpLoad %7 %9
         %45 = OpVectorShuffle %42 %44 %44 0 1
         %46 = OpFAdd %42 %45 %43
         %47 = OpLoad %7 %9
         %48 = OpVectorShuffle %7 %47 %46 4 5 2 3
               OpStore %9 %48
         %54 = OpLoad %7 %9
         %56 = OpAccessChain %55 %52 %53
               OpStore %56 %54
			   OpStore %output %foo
               OpReturn
               OpFunctionEnd
