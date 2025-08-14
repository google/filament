// RUN: %dxc -T hs_6_0 -E HS -fspv-debug=vulkan-with-source -fcgl %s -spirv | FileCheck %s

// CHECK:            [[set:%[0-9]+]] = OpExtInstImport "NonSemantic.Shader.DebugInfo.100"
// CHECK: [[hsConstantName:%[0-9]+]] = OpString "hsConstant"
// CHECK:    [[dbgFunction:%[0-9]+]] = OpExtInst %void [[set]] DebugFunction [[hsConstantName]]
// CHECK:   [[lexicalBlock:%[0-9]+]] = OpExtInst %void [[set]] DebugLexicalBlock {{%[0-9]+}} %uint_23 %uint_1 [[dbgFunction]]
// CHECK:                %hsConstant = OpFunction %HS_OUTPUT None {{%[0-9]+}}
// CHECK:                {{%[0-9]+}} = OpExtInst %void [[set]] DebugFunctionDefinition [[dbgFunction]] %hsConstant
// CHECK:                {{%[0-9]+}} = OpExtInst %void [[set]] DebugScope [[lexicalBlock]]

struct HS_OUTPUT
{
	float edges[4] : SV_TessFactor;
	float inside[2]: SV_InsideTessFactor;
};

struct VS_OUTPUT
{
    float4 pos	: POSITION;
};

HS_OUTPUT hsConstant( InputPatch<VS_OUTPUT, 4> ip, uint pid : SV_PrimitiveID )
{	
	return (HS_OUTPUT)0;
}

[domain("quad")]
[partitioning("integer")]
[outputcontrolpoints(4)]
[outputtopology("triangle_cw")]
[patchconstantfunc("hsConstant")]
VS_OUTPUT HS( InputPatch<VS_OUTPUT, 4> ip, uint cpid : SV_OutputControlPointID)
{
	return (VS_OUTPUT)0;
}
