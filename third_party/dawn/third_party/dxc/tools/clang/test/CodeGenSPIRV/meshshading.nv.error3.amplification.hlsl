// RUN: not %dxc -T as_6_5 -E main -fcgl  %s -spirv  2>&1 | FileCheck %s

// CHECK:  16:19: error: invalid usage of semantic 'USER_IN' in shader profile as

struct MeshPayload {
    float4 pos;
};

groupshared MeshPayload pld;

#define NUM_THREADS 128

[numthreads(NUM_THREADS, 1, 1)]
void main(
        in uint tig : SV_GroupIndex,
        in float3 userAttrIn : USER_IN)
{
}
