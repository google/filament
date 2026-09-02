// RUN: %dxc -E Range -T ps_6_0 %s | FileCheck %s -check-prefix=CHK_RANGE

// RUN: %dxc -E VarOffset -T ps_6_0 -DOFFSETS=argOffsets %s | FileCheck %s -check-prefix=CHK_VAROFF
// RUN: %dxc -E VarOffset -T ps_6_0 -DOFFSETS=cbufOffsets %s | FileCheck %s -check-prefix=CHK_VAROFF
// RUN: %dxc -E VarOffset -T ps_6_0 -DOFFSETS=constOffsets %s | FileCheck %s -check-prefix=CHK_VAROFF
// RUN: %dxc -E VarOffset -T ps_6_0 -DOFFSETS=validOffsets %s | FileCheck %s -check-prefix=CHK_VALID

// RUN: %dxc -E VarOffset -T ps_6_7 -DOFFSETS=argOffsets %s | FileCheck %s -check-prefixes=CHK_VALID,SM67CHCK
// RUN: %dxc -E VarOffset -T ps_6_7 -DOFFSETS=cbufOffsets %s | FileCheck %s -check-prefixes=CHK_VALID,SM67CHCK
// RUN: %dxc -E VarOffset -T ps_6_7 -DOFFSETS=constOffsets %s | FileCheck %s -check-prefixes=CHK_VALID,SM67CHCK

//SM67CHK: Advanced Texture Ops

// CHK_RANGE: error: Offsets to texture access operations must be between -8 and 7.
// CHK_RANGE: error: Offsets to texture access operations must be between -8 and 7.
// CHK_RANGE: error: Offsets to texture access operations must be between -8 and 7.

// CHK_RANGE: error: Offsets to texture access operations must be between -8 and 7.
// CHK_RANGE: error: Offsets to texture access operations must be between -8 and 7.
// CHK_RANGE: error: Offsets to texture access operations must be between -8 and 7.

// CHK_VAROFF: Offsets to texture access operations must be immediate values
// CHK_VAROFF: Offsets to texture access operations must be immediate values
// CHK_VAROFF: Offsets to texture access operations must be immediate values

// CHK_VAROFF: Offsets to texture access operations must be immediate values
// CHK_VAROFF: Offsets to texture access operations must be immediate values
// CHK_VAROFF: Offsets to texture access operations must be immediate values


// Just make sure it compiles without errors
// CHK_VALID: define void
// CHK_VALID: ret void

Texture1D t1;
Texture2D t2;
Texture3D t3;
SamplerState s;
SamplerComparisonState sc;

float4 Range(float3 str : STR) : SV_TARGET
{
    float4 res = 0.0;
    res += t1.Sample(s, str.x, -10);
    res += t2.Sample(s, str.xy, int2(-18,19));
    res += t3.Sample(s, str, int3(-10,1,3));

    res += t1.Load(0, 90);
    res += t2.Load(1, int2(80, 90));
    res += t3.Load(2, int3(-1, -2, 11));

    return res;
}

#ifndef OFFSETS
#define OFFSETS argOffsets
#endif

uint3 cbufOffsets[4];

float4 VarOffset(float3 str : STR, uint3 argOffsets[4] : O, uint a : A) : SV_TARGET
{
    uint b = 3 + a;
    uint v = 3;
    const uint3 constOffsets[4] = {uint3(a,a,a), argOffsets[0], cbufOffsets[0], uint3(b,b,b)};
    uint3 validOffsets[4] = {uint3(v,v,v), uint3(1,1,1), uint3(2,2,2), uint3(3,3,3)};
    float4 res = 0.0;
    res += t2.Sample(s, str.x, OFFSETS[0].x);
    res += t2.Sample(s, str.xy, OFFSETS[0].xy);
    res += t3.Sample(s, str, OFFSETS[0]);

    res += t1.Load(0, OFFSETS[0].x);
    res += t2.Load(1, OFFSETS[0].xy);
    res += t3.Load(2, OFFSETS[0]);

    return res;
}

