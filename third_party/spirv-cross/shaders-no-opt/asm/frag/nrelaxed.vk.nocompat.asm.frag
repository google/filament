; SPIR-V
; Version: 1.3
; Generator: Unknown(30017); 21022
; Bound: 71
; Schema: 0
OpCapability Shader
%43 = OpExtInstImport "GLSL.std.450"
OpMemoryModel Logical GLSL450
OpEntryPoint Fragment %3 "main" %8 %9 %10 %12
OpExecutionMode %3 OriginUpperLeft
OpName %3 "main"
OpName %8 "A"
OpName %9 "B"
OpName %10 "C"
OpName %12 "SV_Target"
OpDecorate %8 RelaxedPrecision
OpDecorate %8 Location 0
OpDecorate %9 RelaxedPrecision
OpDecorate %9 Location 1
OpDecorate %10 RelaxedPrecision
OpDecorate %10 Location 2
OpDecorate %12 RelaxedPrecision
OpDecorate %12 Location 0
OpDecorate %44 RelaxedPrecision
OpDecorate %45 RelaxedPrecision
OpDecorate %46 RelaxedPrecision
OpDecorate %47 RelaxedPrecision
OpDecorate %48 RelaxedPrecision
OpDecorate %49 RelaxedPrecision
OpDecorate %50 RelaxedPrecision
OpDecorate %51 RelaxedPrecision
OpDecorate %52 RelaxedPrecision
OpDecorate %53 RelaxedPrecision
OpDecorate %54 RelaxedPrecision
OpDecorate %55 RelaxedPrecision
OpDecorate %56 RelaxedPrecision
OpDecorate %57 RelaxedPrecision
OpDecorate %58 RelaxedPrecision
OpDecorate %59 RelaxedPrecision
OpDecorate %60 RelaxedPrecision
OpDecorate %61 RelaxedPrecision
OpDecorate %62 RelaxedPrecision
OpDecorate %63 RelaxedPrecision
%1 = OpTypeVoid
%2 = OpTypeFunction %1
%5 = OpTypeFloat 32
%6 = OpTypeVector %5 4
%7 = OpTypePointer Input %6
%8 = OpVariable %7 Input
%9 = OpVariable %7 Input
%10 = OpVariable %7 Input
%11 = OpTypePointer Output %6
%12 = OpVariable %11 Output
%13 = OpTypePointer Input %5
%15 = OpTypeInt 32 0
%16 = OpConstant %15 0
%19 = OpConstant %15 1
%22 = OpConstant %15 2
%25 = OpConstant %15 3
%64 = OpTypePointer Output %5
%3 = OpFunction %1 None %2
%4 = OpLabel
OpBranch %69
%69 = OpLabel
%14 = OpAccessChain %13 %10 %16
%17 = OpLoad %5 %14
%18 = OpAccessChain %13 %10 %19
%20 = OpLoad %5 %18
%21 = OpAccessChain %13 %10 %22
%23 = OpLoad %5 %21
%24 = OpAccessChain %13 %10 %25
%26 = OpLoad %5 %24
%27 = OpAccessChain %13 %9 %16
%28 = OpLoad %5 %27
%29 = OpAccessChain %13 %9 %19
%30 = OpLoad %5 %29
%31 = OpAccessChain %13 %9 %22
%32 = OpLoad %5 %31
%33 = OpAccessChain %13 %9 %25
%34 = OpLoad %5 %33
%35 = OpAccessChain %13 %8 %16
%36 = OpLoad %5 %35
%37 = OpAccessChain %13 %8 %19
%38 = OpLoad %5 %37
%39 = OpAccessChain %13 %8 %22
%40 = OpLoad %5 %39
%41 = OpAccessChain %13 %8 %25
%42 = OpLoad %5 %41
%44 = OpExtInst %5 %43 NMin %36 %28
%45 = OpExtInst %5 %43 NMin %38 %30
%46 = OpExtInst %5 %43 NMin %40 %32
%47 = OpExtInst %5 %43 NMin %42 %34
%48 = OpExtInst %5 %43 NMax %36 %28
%49 = OpExtInst %5 %43 NMax %38 %30
%50 = OpExtInst %5 %43 NMax %40 %32
%51 = OpExtInst %5 %43 NMax %42 %34
%52 = OpExtInst %5 %43 NClamp %36 %48 %17
%53 = OpExtInst %5 %43 NClamp %38 %49 %20
%54 = OpExtInst %5 %43 NClamp %40 %50 %23
%55 = OpExtInst %5 %43 NClamp %42 %51 %26
%56 = OpFAdd %5 %48 %44
%57 = OpFAdd %5 %49 %45
%58 = OpFAdd %5 %50 %46
%59 = OpFAdd %5 %51 %47
%60 = OpFAdd %5 %56 %52
%61 = OpFAdd %5 %57 %53
%62 = OpFAdd %5 %58 %54
%63 = OpFAdd %5 %59 %55
%65 = OpAccessChain %64 %12 %16
OpStore %65 %60
%66 = OpAccessChain %64 %12 %19
OpStore %66 %61
%67 = OpAccessChain %64 %12 %22
OpStore %67 %62
%68 = OpAccessChain %64 %12 %25
OpStore %68 %63
OpReturn
OpFunctionEnd

