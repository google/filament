// RUN: %dxc -E GatherRange -T ps_6_0 %s | FileCheck %s

// RUN: %dxc -E Gather1 -T ps_6_0 -DOFFSETS=argOffsets %s | FileCheck %s
// RUN: %dxc -E Gather1 -T ps_6_0 -DOFFSETS=cbufOffsets %s | FileCheck %s
// RUN: %dxc -E Gather1 -T ps_6_0 -DOFFSETS=constOffsets %s | FileCheck %s
// RUN: %dxc -E Gather1 -T ps_6_0 -DOFFSETS=validOffsets %s | FileCheck %s

// RUN: %dxc -E Gather4 -T ps_6_0 -DOFFSETS=argOffsets %s | FileCheck %s -check-prefix=CHK_VALID4
// RUN: %dxc -E Gather4 -T ps_6_0 -DOFFSETS=cbufOffsets %s | FileCheck %s -check-prefix=CHK_VALID4
// RUN: %dxc -E Gather4 -T ps_6_0 -DOFFSETS=constOffsets %s | FileCheck %s -check-prefix=CHK_VALID4
// RUN: %dxc -E Gather4 -T ps_6_0 -DOFFSETS=validOffsets %s | FileCheck %s -check-prefix=CHK_VALID4

// CHECK:  @dx.op.textureGather.f32(i32 73, %dx.types.Handle %{{.+}}, %dx.types.Handle %{{.+}}, float %{{.+}}, float %{{.+}}, float undef, float undef, i32 {{%?.+}}, i32 {{%?.+}}, i32 0)
// CHECK:  @dx.op.textureGather.f32(i32 73, %dx.types.Handle %{{.+}}, %dx.types.Handle %{{.+}}, float %{{.+}}, float %{{.+}}, float undef, float undef, i32 {{%?.+}}, i32 {{%?.+}}, i32 0)
// CHECK:  @dx.op.textureGatherCmp.f32(i32 74, %dx.types.Handle %{{.+}}, %dx.types.Handle %{{.+}}, float %{{.+}}, float %{{.+}}, float undef, float undef, i32 {{%?.+}}, i32 {{%?.+}}, i32 0,
// CHECK:  @dx.op.textureGatherCmp.f32(i32 74, %dx.types.Handle %{{.+}}, %dx.types.Handle %{{.+}}, float %{{.+}}, float %{{.+}}, float undef, float undef, i32 {{%?.+}}, i32 {{%?.+}}, i32 0,


Texture1D t1;
Texture2D t2;
Texture3D t3;
SamplerState s;
SamplerComparisonState sc;

float4 GatherRange(float3 str : STR) : SV_TARGET
{
    float4 res = 0.0;
    res += t2.Gather     (s, str.xy, int2(9,8));
    res += t2.GatherRed  (s, str.xy, int2(-9,-8));
    res += t2.GatherCmp     (sc, str.xy, 0.0, int2(999999, -999999));
    res += t2.GatherCmpRed  (sc, str.xy, 0.0, int2(0, 10));

    return res;
}

#ifndef OFFSETS
#define OFFSETS argOffsets
#endif

uint3 cbufOffsets[4];

float4 Gather1(float3 str : STR, uint3 argOffsets[4] : O, uint a : A) : SV_TARGET
{
    uint b = 3 + a;
    uint v = 3;
    const uint3 constOffsets[4] = {uint3(a,a,a), argOffsets[0], cbufOffsets[0], uint3(b,b,b)};
    uint3 validOffsets[4] = {uint3(v,v,v), uint3(1,1,1), uint3(2,2,2), uint3(3,3,3)};
    float4 res = 0.0;
    res += t2.Gather     (s, str.xy, OFFSETS[0].xy);
    res += t2.GatherRed  (s, str.xy, OFFSETS[1].xy);
    res += t2.GatherCmp     (sc, str.xy, 0.0, OFFSETS[0].xy);
    res += t2.GatherCmpRed  (sc, str.xy, 0.0, OFFSETS[1].xy);

    return res;
}

float4 Gather4(float3 str : STR, uint3 argOffsets[4] : O, uint a : A) : SV_TARGET
{
    uint b = 3 + a;
    uint v = 4;
    const uint3 constOffsets[4] = {uint3(a,a,a), argOffsets[0], cbufOffsets[0], uint3(b,b,b)};
    uint3 validOffsets[4] = {uint3(v,v,v), uint3(1,1,1), uint3(2,2,2), uint3(3,3,3)};
    float4 res = 0.0;

// CHK_VALID4:  @dx.op.textureGather.f32(i32 73, %dx.types.Handle %{{.+}}, %dx.types.Handle %{{.+}}, float %{{.+}}, float %{{.+}}, float undef, float undef, i32 0, i32 0, i32 0)
// CHK_VALID4:  @dx.op.textureGather.f32(i32 73, %dx.types.Handle %{{.+}}, %dx.types.Handle %{{.+}}, float %{{.+}}, float %{{.+}}, float undef, float undef, i32 1, i32 1, i32 0)
// CHK_VALID4:  @dx.op.textureGather.f32(i32 73, %dx.types.Handle %{{.+}}, %dx.types.Handle %{{.+}}, float %{{.+}}, float %{{.+}}, float undef, float undef, i32 2, i32 2, i32 0)
// CHK_VALID4:  @dx.op.textureGather.f32(i32 73, %dx.types.Handle %{{.+}}, %dx.types.Handle %{{.+}}, float %{{.+}}, float %{{.+}}, float undef, float undef, i32 -11, i32 1, i32 0)

    res += t2.GatherRed  (s, str.xy, int2(0,0), int2(1,1), int2(2,2), int2(-11, 1));

// CHK_VALID4:  @dx.op.textureGather.f32(i32 73, %dx.types.Handle %{{.+}}, %dx.types.Handle %{{.+}}, float %{{.+}}, float %{{.+}}, float undef, float undef, i32 0, i32 0, i32 1)
// CHK_VALID4:  @dx.op.textureGather.f32(i32 73, %dx.types.Handle %{{.+}}, %dx.types.Handle %{{.+}}, float %{{.+}}, float %{{.+}}, float undef, float undef, i32 1, i32 1, i32 1)
// CHK_VALID4:  @dx.op.textureGather.f32(i32 73, %dx.types.Handle %{{.+}}, %dx.types.Handle %{{.+}}, float %{{.+}}, float %{{.+}}, float undef, float undef, i32 0, i32 -9, i32 1)
// CHK_VALID4:  @dx.op.textureGather.f32(i32 73, %dx.types.Handle %{{.+}}, %dx.types.Handle %{{.+}}, float %{{.+}}, float %{{.+}}, float undef, float undef, i32 3, i32 3, i32 1)

    res += t2.GatherGreen(s, str.xy, int2(0,0), int2(1,1), int2(0,-9), int2(3,3));

// CHK_VALID4:  @dx.op.textureGather.f32(i32 73, %dx.types.Handle %{{.+}}, %dx.types.Handle %{{.+}}, float %{{.+}}, float %{{.+}}, float undef, float undef, i32 0, i32 0, i32 2)
// CHK_VALID4:  @dx.op.textureGather.f32(i32 73, %dx.types.Handle %{{.+}}, %dx.types.Handle %{{.+}}, float %{{.+}}, float %{{.+}}, float undef, float undef, i32 3, i32 33, i32 2)
// CHK_VALID4:  @dx.op.textureGather.f32(i32 73, %dx.types.Handle %{{.+}}, %dx.types.Handle %{{.+}}, float %{{.+}}, float %{{.+}}, float undef, float undef, i32 2, i32 2, i32 2)
// CHK_VALID4:  @dx.op.textureGather.f32(i32 73, %dx.types.Handle %{{.+}}, %dx.types.Handle %{{.+}}, float %{{.+}}, float %{{.+}}, float undef, float undef, i32 3, i32 3, i32 2)

    res += t2.GatherBlue (s, str.xy, int2(0,0), int2(3,33), int2(2,2), int2(3,3));

// CHK_VALID4:  @dx.op.textureGather.f32(i32 73, %dx.types.Handle %{{.+}}, %dx.types.Handle %{{.+}}, float %{{.+}}, float %{{.+}}, float undef, float undef, i32 11, i32 1, i32 3)
// CHK_VALID4:  @dx.op.textureGather.f32(i32 73, %dx.types.Handle %{{.+}}, %dx.types.Handle %{{.+}}, float %{{.+}}, float %{{.+}}, float undef, float undef, i32 1, i32 1, i32 3)
// CHK_VALID4:  @dx.op.textureGather.f32(i32 73, %dx.types.Handle %{{.+}}, %dx.types.Handle %{{.+}}, float %{{.+}}, float %{{.+}}, float undef, float undef, i32 2, i32 2, i32 3)
// CHK_VALID4:  @dx.op.textureGather.f32(i32 73, %dx.types.Handle %{{.+}}, %dx.types.Handle %{{.+}}, float %{{.+}}, float %{{.+}}, float undef, float undef, i32 3, i32 3, i32 3)

    res += t2.GatherAlpha(s, str.xy, int2(11,1), int2(1,1), int2(2,2), int2(3,3));

// CHK_VALID4:  @dx.op.textureGatherCmp.f32(i32 74, %dx.types.Handle %{{.+}}, %dx.types.Handle %{{.+}}, float %{{.+}}, float %{{.+}}, float undef, float undef, i32 0, i32 0, i32 0,
// CHK_VALID4:  @dx.op.textureGatherCmp.f32(i32 74, %dx.types.Handle %{{.+}}, %dx.types.Handle %{{.+}}, float %{{.+}}, float %{{.+}}, float undef, float undef, i32 1, i32 1, i32 0,
// CHK_VALID4:  @dx.op.textureGatherCmp.f32(i32 74, %dx.types.Handle %{{.+}}, %dx.types.Handle %{{.+}}, float %{{.+}}, float %{{.+}}, float undef, float undef, i32 2, i32 2, i32 0,
// CHK_VALID4:  @dx.op.textureGatherCmp.f32(i32 74, %dx.types.Handle %{{.+}}, %dx.types.Handle %{{.+}}, float %{{.+}}, float %{{.+}}, float undef, float undef, i32 3, i32 -9, i32 0,

    res += t2.GatherCmpRed  (sc, str.xy, 0.0, int2(0,0), int2(1,1), int2(2,2), int2(3,-9));

// CHK_VALID4:  @dx.op.textureGatherCmp.f32(i32 74, %dx.types.Handle %{{.+}}, %dx.types.Handle %{{.+}}, float %{{.+}}, float %{{.+}}, float undef, float undef, i32 0, i32 0, i32 1,
// CHK_VALID4:  @dx.op.textureGatherCmp.f32(i32 74, %dx.types.Handle %{{.+}}, %dx.types.Handle %{{.+}}, float %{{.+}}, float %{{.+}}, float undef, float undef, i32 1, i32 1, i32 1,
// CHK_VALID4:  @dx.op.textureGatherCmp.f32(i32 74, %dx.types.Handle %{{.+}}, %dx.types.Handle %{{.+}}, float %{{.+}}, float %{{.+}}, float undef, float undef, i32 10, i32 5, i32 1,
// CHK_VALID4:  @dx.op.textureGatherCmp.f32(i32 74, %dx.types.Handle %{{.+}}, %dx.types.Handle %{{.+}}, float %{{.+}}, float %{{.+}}, float undef, float undef, i32 3, i32 3, i32 1,

    res += t2.GatherCmpGreen(sc, str.xy, 0.0, int2(0,0), int2(1,1), int2(10, 5), int2(3,3));

// CHK_VALID4:  @dx.op.textureGatherCmp.f32(i32 74, %dx.types.Handle %{{.+}}, %dx.types.Handle %{{.+}}, float %{{.+}}, float %{{.+}}, float undef, float undef, i32 0, i32 0, i32 2,
// CHK_VALID4:  @dx.op.textureGatherCmp.f32(i32 74, %dx.types.Handle %{{.+}}, %dx.types.Handle %{{.+}}, float %{{.+}}, float %{{.+}}, float undef, float undef, i32 -11, i32 6, i32 2,
// CHK_VALID4:  @dx.op.textureGatherCmp.f32(i32 74, %dx.types.Handle %{{.+}}, %dx.types.Handle %{{.+}}, float %{{.+}}, float %{{.+}}, float undef, float undef, i32 2, i32 2, i32 2,
// CHK_VALID4:  @dx.op.textureGatherCmp.f32(i32 74, %dx.types.Handle %{{.+}}, %dx.types.Handle %{{.+}}, float %{{.+}}, float %{{.+}}, float undef, float undef, i32 3, i32 3, i32 2,

    res += t2.GatherCmpBlue (sc, str.xy, 0.0, int2(0,0), int2(-11,6), int2(2,2), int2(3,3));

// CHK_VALID4:  @dx.op.textureGatherCmp.f32(i32 74, %dx.types.Handle %{{.+}}, %dx.types.Handle %{{.+}}, float %{{.+}}, float %{{.+}}, float undef, float undef, i32 9, i32 9, i32 3,
// CHK_VALID4:  @dx.op.textureGatherCmp.f32(i32 74, %dx.types.Handle %{{.+}}, %dx.types.Handle %{{.+}}, float %{{.+}}, float %{{.+}}, float undef, float undef, i32 1, i32 1, i32 3,
// CHK_VALID4:  @dx.op.textureGatherCmp.f32(i32 74, %dx.types.Handle %{{.+}}, %dx.types.Handle %{{.+}}, float %{{.+}}, float %{{.+}}, float undef, float undef, i32 2, i32 2, i32 3,
// CHK_VALID4:  @dx.op.textureGatherCmp.f32(i32 74, %dx.types.Handle %{{.+}}, %dx.types.Handle %{{.+}}, float %{{.+}}, float %{{.+}}, float undef, float undef, i32 3, i32 3, i32 3,

    res += t2.GatherCmpAlpha(sc, str.xy, 0.0, int2(9,9), int2(1,1), int2(2,2), int2(3,3));

// CHK_VALID4:  @dx.op.textureGather.f32(i32 73, %dx.types.Handle %{{.+}}, %dx.types.Handle %{{.+}}, float %{{.+}}, float %{{.+}}, float undef, float undef, i32 0, i32 1, i32 0)
// CHK_VALID4:  @dx.op.textureGather.f32(i32 73, %dx.types.Handle %{{.+}}, %dx.types.Handle %{{.+}}, float %{{.+}}, float %{{.+}}, float undef, float undef, i32 2, i32 3, i32 0)
// CHK_VALID4:  @dx.op.textureGather.f32(i32 73, %dx.types.Handle %{{.+}}, %dx.types.Handle %{{.+}}, float %{{.+}}, float %{{.+}}, float undef, float undef, i32 4, i32 5, i32 0)
// CHK_VALID4:  @dx.op.textureGather.f32(i32 73, %dx.types.Handle %{{.+}}, %dx.types.Handle %{{.+}}, float %{{.+}}, float %{{.+}}, float undef, float undef, i32 {{%?.+}}, i32 {{%?.+}}, i32 0)

    res += t2.GatherRed  (s, str.xy, int2(0,1), int2(2,3), int2(4,5), OFFSETS[3].xy);

// CHK_VALID4:  @dx.op.textureGather.f32(i32 73, %dx.types.Handle %{{.+}}, %dx.types.Handle %{{.+}}, float %{{.+}}, float %{{.+}}, float undef, float undef, i32 1, i32 2, i32 1)
// CHK_VALID4:  @dx.op.textureGather.f32(i32 73, %dx.types.Handle %{{.+}}, %dx.types.Handle %{{.+}}, float %{{.+}}, float %{{.+}}, float undef, float undef, i32 3, i32 4, i32 1)
// CHK_VALID4:  @dx.op.textureGather.f32(i32 73, %dx.types.Handle %{{.+}}, %dx.types.Handle %{{.+}}, float %{{.+}}, float %{{.+}}, float undef, float undef, i32 {{%?.+}}, i32 {{%?.+}}, i32 1)
// CHK_VALID4:  @dx.op.textureGather.f32(i32 73, %dx.types.Handle %{{.+}}, %dx.types.Handle %{{.+}}, float %{{.+}}, float %{{.+}}, float undef, float undef, i32 7, i32 0, i32 1)

    res += t2.GatherGreen(s, str.xy, int2(1,2), int2(3,4), OFFSETS[2].xy, int2(7,0));

// CHK_VALID4:  @dx.op.textureGather.f32(i32 73, %dx.types.Handle %{{.+}}, %dx.types.Handle %{{.+}}, float %{{.+}}, float %{{.+}}, float undef, float undef, i32 2, i32 3, i32 2)
// CHK_VALID4:  @dx.op.textureGather.f32(i32 73, %dx.types.Handle %{{.+}}, %dx.types.Handle %{{.+}}, float %{{.+}}, float %{{.+}}, float undef, float undef, i32 {{%?.+}}, i32 {{%?.+}}, i32 2)
// CHK_VALID4:  @dx.op.textureGather.f32(i32 73, %dx.types.Handle %{{.+}}, %dx.types.Handle %{{.+}}, float %{{.+}}, float %{{.+}}, float undef, float undef, i32 6, i32 7, i32 2)
// CHK_VALID4:  @dx.op.textureGather.f32(i32 73, %dx.types.Handle %{{.+}}, %dx.types.Handle %{{.+}}, float %{{.+}}, float %{{.+}}, float undef, float undef, i32 0, i32 1, i32 2)

    res += t2.GatherBlue (s, str.xy, int2(2,3), OFFSETS[1].xy, int2(6,7), int2(0,1));

// CHK_VALID4:  @dx.op.textureGather.f32(i32 73, %dx.types.Handle %{{.+}}, %dx.types.Handle %{{.+}}, float %{{.+}}, float %{{.+}}, float undef, float undef, i32 {{%?.+}}, i32 {{%?.+}}, i32 3)
// CHK_VALID4:  @dx.op.textureGather.f32(i32 73, %dx.types.Handle %{{.+}}, %dx.types.Handle %{{.+}}, float %{{.+}}, float %{{.+}}, float undef, float undef, i32 5, i32 6, i32 3)
// CHK_VALID4:  @dx.op.textureGather.f32(i32 73, %dx.types.Handle %{{.+}}, %dx.types.Handle %{{.+}}, float %{{.+}}, float %{{.+}}, float undef, float undef, i32 7, i32 0, i32 3)
// CHK_VALID4:  @dx.op.textureGather.f32(i32 73, %dx.types.Handle %{{.+}}, %dx.types.Handle %{{.+}}, float %{{.+}}, float %{{.+}}, float undef, float undef, i32 1, i32 2, i32 3)

    res += t2.GatherAlpha(s, str.xy, OFFSETS[0].xy, int2(5,6), int2(7,0), int2(1,2));

// CHK_VALID4:  @dx.op.textureGatherCmp.f32(i32 74, %dx.types.Handle %{{.+}}, %dx.types.Handle %{{.+}}, float %{{.+}}, float %{{.+}}, float undef, float undef, i32 4, i32 5, i32 0,
// CHK_VALID4:  @dx.op.textureGatherCmp.f32(i32 74, %dx.types.Handle %{{.+}}, %dx.types.Handle %{{.+}}, float %{{.+}}, float %{{.+}}, float undef, float undef, i32 6, i32 7, i32 0,
// CHK_VALID4:  @dx.op.textureGatherCmp.f32(i32 74, %dx.types.Handle %{{.+}}, %dx.types.Handle %{{.+}}, float %{{.+}}, float %{{.+}}, float undef, float undef, i32 0, i32 1, i32 0,
// CHK_VALID4:  @dx.op.textureGatherCmp.f32(i32 74, %dx.types.Handle {{.+}}, %dx.types.Handle %{{.+}}, float %{{.+}}, float %{{.+}}, float undef, float undef, i32 {{%?.+}}, i32 {{%?.+}}, i32 0,

    res += t2.GatherCmpRed  (sc, str.xy, 0.0, int2(4,5), int2(6,7), int2(0,1), OFFSETS[3].xy);

// CHK_VALID4:  @dx.op.textureGatherCmp.f32(i32 74, %dx.types.Handle %{{.+}}, %dx.types.Handle %{{.+}}, float %{{.+}}, float %{{.+}}, float undef, float undef, i32 5, i32 6, i32 1,
// CHK_VALID4:  @dx.op.textureGatherCmp.f32(i32 74, %dx.types.Handle %{{.+}}, %dx.types.Handle %{{.+}}, float %{{.+}}, float %{{.+}}, float undef, float undef, i32 7, i32 0, i32 1,
// CHK_VALID4:  @dx.op.textureGatherCmp.f32(i32 74, %dx.types.Handle {{.+}}, %dx.types.Handle %{{.+}}, float %{{.+}}, float %{{.+}}, float undef, float undef, i32 {{%?.+}}, i32 {{%?.+}}, i32 1,
// CHK_VALID4:  @dx.op.textureGatherCmp.f32(i32 74, %dx.types.Handle %{{.+}}, %dx.types.Handle %{{.+}}, float %{{.+}}, float %{{.+}}, float undef, float undef, i32 3, i32 4, i32 1,

    res += t2.GatherCmpGreen(sc, str.xy, 0.0, int2(5,6), int2(7,0), OFFSETS[2].xy, int2(3,4));

// CHK_VALID4:  @dx.op.textureGatherCmp.f32(i32 74, %dx.types.Handle %{{.+}}, %dx.types.Handle %{{.+}}, float %{{.+}}, float %{{.+}}, float undef, float undef, i32 6, i32 7, i32 2,
// CHK_VALID4:  @dx.op.textureGatherCmp.f32(i32 74, %dx.types.Handle {{.+}}, %dx.types.Handle %{{.+}}, float %{{.+}}, float %{{.+}}, float undef, float undef, i32 {{%?.+}}, i32 {{%?.+}}, i32 2,
// CHK_VALID4:  @dx.op.textureGatherCmp.f32(i32 74, %dx.types.Handle %{{.+}}, %dx.types.Handle %{{.+}}, float %{{.+}}, float %{{.+}}, float undef, float undef, i32 2, i32 3, i32 2,
// CHK_VALID4:  @dx.op.textureGatherCmp.f32(i32 74, %dx.types.Handle {{.+}}, %dx.types.Handle %{{.+}}, float %{{.+}}, float %{{.+}}, float undef, float undef, i32 4, i32 5, i32 2,

    res += t2.GatherCmpBlue (sc, str.xy, 0.0, int2(6,7), OFFSETS[1].xy, int2(2,3), int2(4,5));

// CHK_VALID4:  @dx.op.textureGatherCmp.f32(i32 74, %dx.types.Handle {{.+}}, %dx.types.Handle %{{.+}}, float %{{.+}}, float %{{.+}}, float undef, float undef, i32 {{%?.+}}, i32 {{%?.+}}, i32 3,
// CHK_VALID4:  @dx.op.textureGatherCmp.f32(i32 74, %dx.types.Handle %{{.+}}, %dx.types.Handle %{{.+}}, float %{{.+}}, float %{{.+}}, float undef, float undef, i32 1, i32 2, i32 3,
// CHK_VALID4:  @dx.op.textureGatherCmp.f32(i32 74, %dx.types.Handle %{{.+}}, %dx.types.Handle %{{.+}}, float %{{.+}}, float %{{.+}}, float undef, float undef, i32 3, i32 4, i32 3,
// CHK_VALID4:  @dx.op.textureGatherCmp.f32(i32 74, %dx.types.Handle {{.+}}, %dx.types.Handle %{{.+}}, float %{{.+}}, float %{{.+}}, float undef, float undef, i32 5, i32 6, i32 3,

    res += t2.GatherCmpAlpha(sc, str.xy, 0.0, OFFSETS[0].xy, int2(1,2), int2(3,4), int2(5,6));

    return res;
}
