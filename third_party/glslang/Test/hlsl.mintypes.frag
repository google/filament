struct PS_OUTPUT
{
    float4 Color : SV_Target0;
};

uniform min16float  b1a, b1b;

PS_OUTPUT main()
{
    min16float  mf16;
    min16float1 mf16_1;
    min16float2 mf16_2;
    min16float3 mf16_3;
    min16float4 mf16_4;
    min16float2x2 mf16_2x2;
    min16float2x3 mf16_2x3;
    min16float2x4 mf16_2x4;
    min16float3x2 mf16_3x2;
    min16float3x3 mf16_3x3;
    min16float3x4 mf16_3x4;
    min16float4x2 mf16_4x2;
    min16float4x3 mf16_4x3;
    min16float4x4 mf16_4x4;

    min10float  mf10;
    min10float1 mf10_1;
    min10float2 mf10_2;
    min10float3 mf10_3;
    min10float4 mf10_4;
    min10float2x2 mf10_2x2;
    min10float2x3 mf10_2x3;
    min10float2x4 mf10_2x4;
    min10float3x2 mf10_3x2;
    min10float3x3 mf10_3x3;
    min10float3x4 mf10_3x4;
    min10float4x2 mf10_4x2;
    min10float4x3 mf10_4x3;
    min10float4x4 mf10_4x4;

    min16int  mi16;
    min16int1 mi16_1;
    min16int2 mi16_2;
    min16int3 mi16_3;
    min16int4 mi16_4;
    min16int2x2 mi16_2x2;
    min16int2x3 mi16_2x3;
    min16int2x4 mi16_2x4;
    min16int3x2 mi16_3x2;
    min16int3x3 mi16_3x3;
    min16int3x4 mi16_3x4;
    min16int4x2 mi16_4x2;
    min16int4x3 mi16_4x3;
    min16int4x4 mi16_4x4;

    min12int  mi12;
    min12int1 mi12_1;
    min12int2 mi12_2;
    min12int3 mi12_3;
    min12int4 mi12_4;
    min12int2x2 mi12_2x2;
    min12int2x3 mi12_2x3;
    min12int2x4 mi12_2x4;
    min12int3x2 mi12_3x2;
    min12int3x3 mi12_3x3;
    min12int3x4 mi12_3x4;
    min12int4x2 mi12_4x2;
    min12int4x3 mi12_4x3;
    min12int4x4 mi12_4x4;

    min16uint  mu16;
    min16uint1 mu16_1;
    min16uint2 mu16_2;
    min16uint3 mu16_3;
    min16uint4 mu16_4;
    min16uint2x2 mu16_2x2;
    min16uint2x3 mu16_2x3;
    min16uint2x4 mu16_2x4;
    min16uint3x2 mu16_3x2;
    min16uint3x3 mu16_3x3;
    min16uint3x4 mu16_3x4;
    min16uint4x2 mu16_4x2;
    min16uint4x3 mu16_4x3;
    min16uint4x4 mu16_4x4;

    mf16_2 + mf16;
    mf10_2 + mf10;
    mi16_2 + mi16;
    mi12_2 + mi12;
    mu16_2 + mu16;

    mul(mf16_2, mf16_2x4);
    mul(mf16_3, mf16_3x4);
    mul(mf16_4, mf16_4x4);
    mul(mf16_4x2, mf16_2);
    mul(mf16_4x3, mf16_3);
    mul(mf16_4x4, mf16_4);

    mul(mf10_2, mf10_2x4);
    mul(mf10_3, mf10_3x4);
    mul(mf10_4, mf10_4x4);
    mul(mf10_4x2, mf10_2);
    mul(mf10_4x3, mf10_3);
    mul(mf10_4x4, mf10_4);

    PS_OUTPUT psout;
    psout.Color = 0;
    return psout;
}
