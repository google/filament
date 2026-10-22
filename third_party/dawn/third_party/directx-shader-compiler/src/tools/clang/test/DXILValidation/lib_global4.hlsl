// Check redefine global and resource with lib_global2.hlsl, lib_global3.hlsl.

RWTexture1D<float> buf0;

groupshared column_major float2x2 dataC[8*8];

float2x2 RotateMat(float2x2 m, uint x, uint y) {
    buf0[x] = y;
    dataC[x%(8*8)] = m;
    GroupMemoryBarrierWithGroupSync();
    float2x2 f2x2 = dataC[8*8-1-y%(8*8)];
    return f2x2;
}