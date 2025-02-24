// RUN: not %dxc -T hs_6_0 -E main -fcgl  %s -spirv  2>&1 | FileCheck %s

// CHECK: 32:11: error: SV_OutputControlPointID semantic must be provided in hull shader

struct HS_QUAD_INPUT {
	float2 Origin: TEXCOORD0;
	float2 Size: TEXCOORD1;
};

struct HS_OUTPUT {
	float Dummy : TEXCOORD0;
};

struct QUAD_PATCH_DATA {
	float Edges[4]: SV_TessFactor;
	float Inside[2]: SV_InsideTessFactor;

	float2 Origin: TEXCOORD0;
	float2 Size: TEXCOORD1;
};


QUAD_PATCH_DATA hsPatchConstant(InputPatch<HS_QUAD_INPUT, 1> Input) {
	return (QUAD_PATCH_DATA)0;

}
[domain("quad")]
[outputtopology("triangle_ccw")]
[partitioning("fractional_odd")]
[outputcontrolpoints(1)]
[patchconstantfunc("hsPatchConstant")]
HS_OUTPUT main(InputPatch<HS_QUAD_INPUT, 1> Input)
{
	return (HS_OUTPUT)0;
}

