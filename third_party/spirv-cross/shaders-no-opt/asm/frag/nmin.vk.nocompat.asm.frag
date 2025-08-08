; SPIR-V
; Version: 1.3
; Generator: Unknown(30017); 21022
; Bound: 92
; Schema: 0
OpCapability Shader
OpCapability Float16
OpCapability Float64
OpCapability DenormPreserve
OpExtension "SPV_KHR_float_controls"
%34 = OpExtInstImport "GLSL.std.450"
OpMemoryModel Logical GLSL450
OpEntryPoint Fragment %3 "main" %8 %9 %11
OpExecutionMode %3 OriginUpperLeft
OpExecutionMode %3 DenormPreserve 16
OpExecutionMode %3 DenormPreserve 64
OpName %3 "main"
OpName %8 "A"
OpName %9 "B"
OpName %11 "SV_Target"
OpDecorate %8 Location 0
OpDecorate %9 Location 1
OpDecorate %11 Location 0
%1 = OpTypeVoid
%2 = OpTypeFunction %1
%5 = OpTypeFloat 32
%6 = OpTypeVector %5 4
%7 = OpTypePointer Input %6
%8 = OpVariable %7 Input
%9 = OpVariable %7 Input
%10 = OpTypePointer Output %6
%11 = OpVariable %10 Output
%12 = OpTypePointer Input %5
%14 = OpTypeInt 32 0
%15 = OpConstant %14 0
%18 = OpConstant %14 1
%21 = OpConstant %14 2
%24 = OpConstant %14 3
%39 = OpTypeFloat 16
%52 = OpTypeFloat 64
%85 = OpTypePointer Output %5
%3 = OpFunction %1 None %2
%4 = OpLabel
OpBranch %90
%90 = OpLabel
%13 = OpAccessChain %12 %9 %15
%16 = OpLoad %5 %13
%17 = OpAccessChain %12 %9 %18
%19 = OpLoad %5 %17
%20 = OpAccessChain %12 %9 %21
%22 = OpLoad %5 %20
%23 = OpAccessChain %12 %9 %24
%25 = OpLoad %5 %23
%26 = OpAccessChain %12 %8 %15
%27 = OpLoad %5 %26
%28 = OpAccessChain %12 %8 %18
%29 = OpLoad %5 %28
%30 = OpAccessChain %12 %8 %21
%31 = OpLoad %5 %30
%32 = OpAccessChain %12 %8 %24
%33 = OpLoad %5 %32
%35 = OpExtInst %5 %34 NMin %27 %16
%36 = OpExtInst %5 %34 NMin %29 %19
%37 = OpExtInst %5 %34 NMin %31 %22
%38 = OpExtInst %5 %34 NMin %33 %25
%40 = OpFConvert %39 %16
%41 = OpFConvert %39 %19
%42 = OpFConvert %39 %22
%43 = OpFConvert %39 %25
%44 = OpFConvert %39 %27
%45 = OpFConvert %39 %29
%46 = OpFConvert %39 %31
%47 = OpFConvert %39 %33
%48 = OpExtInst %39 %34 NMin %44 %40
%49 = OpExtInst %39 %34 NMin %45 %41
%50 = OpExtInst %39 %34 NMin %46 %42
%51 = OpExtInst %39 %34 NMin %47 %43
%53 = OpFConvert %52 %16
%54 = OpFConvert %52 %19
%55 = OpFConvert %52 %22
%56 = OpFConvert %52 %25
%57 = OpFConvert %52 %27
%58 = OpFConvert %52 %29
%59 = OpFConvert %52 %31
%60 = OpFConvert %52 %33
%61 = OpExtInst %52 %34 NMin %57 %53
%62 = OpExtInst %52 %34 NMin %58 %54
%63 = OpExtInst %52 %34 NMin %59 %55
%64 = OpExtInst %52 %34 NMin %60 %56
%65 = OpFConvert %5 %48
%66 = OpFConvert %5 %49
%67 = OpFConvert %5 %50
%68 = OpFConvert %5 %51
%69 = OpFAdd %5 %65 %35
%70 = OpFAdd %5 %66 %36
%71 = OpFAdd %5 %67 %37
%72 = OpFAdd %5 %68 %38
%73 = OpFConvert %52 %69
%74 = OpFConvert %52 %70
%75 = OpFConvert %52 %71
%76 = OpFConvert %52 %72
%77 = OpFAdd %52 %61 %73
%78 = OpFAdd %52 %62 %74
%79 = OpFAdd %52 %63 %75
%80 = OpFAdd %52 %64 %76
%81 = OpFConvert %5 %77
%82 = OpFConvert %5 %78
%83 = OpFConvert %5 %79
%84 = OpFConvert %5 %80
%86 = OpAccessChain %85 %11 %15
OpStore %86 %81
%87 = OpAccessChain %85 %11 %18
OpStore %87 %82
%88 = OpAccessChain %85 %11 %21
OpStore %88 %83
%89 = OpAccessChain %85 %11 %24
OpStore %89 %84
OpReturn
OpFunctionEnd

