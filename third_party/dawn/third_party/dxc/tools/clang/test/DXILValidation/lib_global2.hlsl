// Check redefine global and resource with lib_global3.hlsl, lib_global4.hlsl.

float2x2 RotateMat(float2x2 m, uint x, uint y);
float2x2 MatRotate(float2x2 m, uint x, uint y);

RWStructuredBuffer<float2x2> fA;

[numthreads(8,8,1)]
void entry( uint2 tid : SV_DispatchThreadID, uint2 gid : SV_GroupID, uint2 gtid : SV_GroupThreadID, uint gidx : SV_GroupIndex )
{
    float2x2 f2x2 = fA[tid.x];

    f2x2 = RotateMat(f2x2, tid.x, tid.y);

    f2x2 = MatRotate(f2x2, tid.x, tid.y);

    fA[tid.y] = f2x2;
}