; SPIR-V
; Version: 1.3
; Generator: Unknown(30017); 21022
; Bound: 121
; Schema: 0
OpCapability Shader
OpCapability Float16
OpCapability Float64
OpCapability DenormPreserve
OpExtension "SPV_KHR_float_controls"
%43 = OpExtInstImport "GLSL.std.450"
OpMemoryModel Logical GLSL450
OpEntryPoint Fragment %3 "main" %8 %9 %10 %12
OpExecutionMode %3 OriginUpperLeft
OpExecutionMode %3 DenormPreserve 16
OpExecutionMode %3 DenormPreserve 64
OpName %3 "main"
OpName %8 "A"
OpName %9 "B"
OpName %10 "C"
OpName %12 "SV_Target"
OpDecorate %8 Location 0
OpDecorate %9 Location 1
OpDecorate %10 Location 2
OpDecorate %12 Location 0
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
%52 = OpTypeFloat 16
%73 = OpTypeFloat 64
%114 = OpTypePointer Output %5
%3 = OpFunction %1 None %2
%4 = OpLabel
OpBranch %119
%119 = OpLabel
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
%48 = OpExtInst %5 %43 NClamp %36 %28 %17
%49 = OpExtInst %5 %43 NClamp %38 %30 %20
%50 = OpExtInst %5 %43 NClamp %40 %32 %23
%51 = OpExtInst %5 %43 NClamp %42 %34 %26
%53 = OpFConvert %52 %17
%54 = OpFConvert %52 %20
%55 = OpFConvert %52 %23
%56 = OpFConvert %52 %26
%57 = OpFConvert %52 %28
%58 = OpFConvert %52 %30
%59 = OpFConvert %52 %32
%60 = OpFConvert %52 %34
%61 = OpFConvert %52 %36
%62 = OpFConvert %52 %38
%63 = OpFConvert %52 %40
%64 = OpFConvert %52 %42
%69 = OpExtInst %52 %43 NClamp %61 %57 %53
%70 = OpExtInst %52 %43 NClamp %62 %58 %54
%71 = OpExtInst %52 %43 NClamp %63 %59 %55
%72 = OpExtInst %52 %43 NClamp %64 %60 %56
%74 = OpFConvert %73 %17
%75 = OpFConvert %73 %20
%76 = OpFConvert %73 %23
%77 = OpFConvert %73 %26
%78 = OpFConvert %73 %28
%79 = OpFConvert %73 %30
%80 = OpFConvert %73 %32
%81 = OpFConvert %73 %34
%82 = OpFConvert %73 %36
%83 = OpFConvert %73 %38
%84 = OpFConvert %73 %40
%85 = OpFConvert %73 %42
%90 = OpExtInst %73 %43 NClamp %82 %78 %74
%91 = OpExtInst %73 %43 NClamp %83 %79 %75
%92 = OpExtInst %73 %43 NClamp %84 %80 %76
%93 = OpExtInst %73 %43 NClamp %85 %81 %77
%94 = OpFConvert %5 %69
%95 = OpFConvert %5 %70
%96 = OpFConvert %5 %71
%97 = OpFConvert %5 %72
%98 = OpFAdd %5 %94 %48
%99 = OpFAdd %5 %95 %49
%100 = OpFAdd %5 %96 %50
%101 = OpFAdd %5 %97 %51
%102 = OpFConvert %73 %98
%103 = OpFConvert %73 %99
%104 = OpFConvert %73 %100
%105 = OpFConvert %73 %101
%106 = OpFAdd %73 %90 %102
%107 = OpFAdd %73 %91 %103
%108 = OpFAdd %73 %92 %104
%109 = OpFAdd %73 %93 %105
%110 = OpFConvert %5 %106
%111 = OpFConvert %5 %107
%112 = OpFConvert %5 %108
%113 = OpFConvert %5 %109
%115 = OpAccessChain %114 %12 %16
OpStore %115 %110
%116 = OpAccessChain %114 %12 %19
OpStore %116 %111
%117 = OpAccessChain %114 %12 %22
OpStore %117 %112
%118 = OpAccessChain %114 %12 %25
OpStore %118 %113
OpReturn
OpFunctionEnd

