// dxc -Tps_6_0 cbuffer-hard-packing.hlsl -spirv -fvk-use-dx-layout -Fo /tmp/test.spv && spirv-dis -o cbuffer-hard-packing.asm.frag /tmp/test.spv

struct PSInput
{
	float4 color : COLOR;
};

struct StraddleResolve
{
    float3 A;
    float3 B;
    float3 C;
    float3 D;
};

struct MatrixStraddle2x3c
{
	column_major float2x3 m;
};

struct MatrixStraddle2x3r
{
	row_major float2x3 m;
};

struct MatrixStraddle3x2c
{
	column_major float3x2 m;
};

struct MatrixStraddle3x2r
{
	row_major float3x2 m;
};

struct Test1
{
	StraddleResolve a;
	float b;
};

struct Test2
{
	StraddleResolve a[2];
	float b;
	StraddleResolve c[3][2];
	float d;
};

struct Test3
{
	MatrixStraddle2x3c c23;
	float dummy0;
	MatrixStraddle2x3r r23;
	float dummy1;
	MatrixStraddle3x2c c32;
	float dummy2;
	MatrixStraddle3x2r r32;
	float dummy3;
};

struct Test4
{
	MatrixStraddle2x3c c23[2][3];
	float dummy0;
	MatrixStraddle2x3r r23[2][3];
	float dummy1;
};


// Wrap in inner struct to ensure we get exact behavior since we cannot lean on packoffset() in this style.
cbuffer Test1Cbuf
{
	Test1 test1;
};

cbuffer Test2Cbuf
{
	Test2 test2;
};

cbuffer Test3Cbuf
{
	Test3 test3;
};

cbuffer Test4Cbuf
{
	Test4 test4;
};

float4 main(PSInput input) : SV_TARGET
{
	return input.color + test1.b + test2.b + test3.dummy0 + test4.dummy0;
}
