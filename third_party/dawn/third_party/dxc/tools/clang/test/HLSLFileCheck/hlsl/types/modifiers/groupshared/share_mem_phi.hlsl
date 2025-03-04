// RUN: %dxc -E main -T cs_6_0 %s

groupshared float b;
groupshared float a;

RWStructuredBuffer<float> fB;

[numthreads(8,8,1)]
void main( uint2 tid : SV_DispatchThreadID, uint2 gid : SV_GroupID, uint2 gtid : SV_GroupThreadID, uint gidx : SV_GroupIndex )
{
    if (tid.x == 0 && tid.y == 0) {
       b = 1;
       a = 2;
    }

    float x = 0;
    float y = tid.y;
    if (tid.x > tid.y) {
       x = a;
       y = sin(y);
    }
    else
       x = b;

    fB[tid.x] = x + y;
}