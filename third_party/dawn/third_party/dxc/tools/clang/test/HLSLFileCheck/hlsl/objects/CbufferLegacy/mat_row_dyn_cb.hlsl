// RUN: %dxc -E main -T cs_6_0 %s | FileCheck %s

// One row should be 4 elements, loaded into array for component indexing
// CHECK: alloca [4 x i32]

cbuffer C
{
    row_major int3x4 m;
};

RWStructuredBuffer<int> output;

[shader("compute")]
[numthreads(12,1,1)]
void main(uint3 tid : SV_DispatchThreadID)
{
    uint i = tid.x;

    output[i] = m[i / 4][i % 4];
}
